/*
 * AVSource.h
 *
 * AVSource - Base class for all audio/video source objects in the
 * HMRAVSource pipeline. Defines the common interface for opening,
 * querying, and reading from media sources.
 *
 * Subclasses: MFSource (Media Foundation), DShowSource (DirectShow).
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AVSOURCE_H
#define AVSOURCE_H

#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// AVSourceDesc - Parameters for opening a source
// ============================================================================
struct AVSourceDesc
{
    ATL::CString    strFilePath;
    AVSourceType    type = AVSourceTypeFile;
    bool            fEnableAudio = true;
    bool            fEnableVideo = true;
    DWORD           dwPreferredVideoWidth = 0;
    DWORD           dwPreferredVideoHeight = 0;
    double          dblPreferredFrameRate = 0.0;
};

// ============================================================================
// AVSourceInfo - Information about an open source
// ============================================================================
struct AVSourceInfo
{
    AVMediaType     mediaType = AVMediaTypeUnknown;
    DWORD           dwWidth = 0;
    DWORD           dwHeight = 0;
    double          dblFrameRate = 0.0;
    LONGLONG        llDurationHns = 0;
    DWORD           dwAudioSampleRate = 0;
    DWORD           dwAudioChannels = 0;
    DWORD           dwAudioBitsPerSample = 0;
    ATL::CString    strVideoCodec;
    ATL::CString    strAudioCodec;
    bool            fHasAudio = false;
    bool            fHasVideo = false;
};

// ============================================================================
// AVSource
// ============================================================================
// Base class for audio/video sources. Encapsulates the common interface for
// media source operations: open, close, seek, and state queries. Does not
// own the actual pipeline - subclasses provide MF or DirectShow backends.
//
class AVSOURCE_API AVSource
{
public:
    AVSource();
    virtual ~AVSource();

    // Identity
    AVSourceType GetType() const throw();
    DWORD GetSourceId() const throw();

    // Open / Close
    virtual HRESULT Open(const AVSourceDesc& desc);
    virtual HRESULT Close();
    virtual bool IsOpen() const throw();

    // State
    AVSourceState GetState() const throw();
    HRESULT GetLastResult() const throw();

    // Source info (valid after Open)
    const AVSourceInfo& GetInfo() const;

    // Duration
    LONGLONG GetDurationHns() const throw();

    // Position (hundred-nanoseconds)
    LONGLONG GetPositionHns() const throw();
    virtual HRESULT SetPositionHns(LONGLONG llPosition);

    // Start / Stop
    virtual HRESULT Start();
    virtual HRESULT Stop();
    virtual HRESULT Pause();
    virtual HRESULT Resume();

    // Media type negotiation
    virtual HRESULT GetNativeVideoType(GUID* pSubtype, UINT* pWidth, UINT* pHeight);
    virtual HRESULT GetNativeAudioType(GUID* pSubtype, DWORD* pSampleRate, DWORD* pChannels);

    // Sample reading (for transcode pipeline)
    virtual HRESULT ReadSample(IMFSample** ppSample, DWORD dwStreamIndex);
    virtual HRESULT Flush();

    // Event handling
    virtual HRESULT GetEvent(IMFMediaEvent** ppEvent);
    virtual HRESULT BeginGetEvent(IMFMediaEventCallback* pCallback, IUnknown* punkState);
    virtual HRESULT EndGetEvent(IMFMediaEvent* pEvent, IMFMediaEvent** ppNextEvent);

    // Error info
    ATL::CString GetLastErrorDescription() const;

    // Reference counting
    void AddRef();
    void Release();

protected:
    AVSourceType    m_type;
    AVSourceState   m_state;
    HRESULT         m_hrLastResult;
    DWORD           m_dwSourceId;
    AVSourceInfo    m_info;
    LONGLONG        m_llDurationHns;
    LONGLONG        m_llPositionHns;
    LONG            m_cRef;

    void SetState(AVSourceState state);
    void SetLastError(HRESULT hr);
    void ResetInfo();

    static DWORD s_dwNextSourceId;
};

} // namespace HMRAVSource

#endif // AVSOURCE_H
