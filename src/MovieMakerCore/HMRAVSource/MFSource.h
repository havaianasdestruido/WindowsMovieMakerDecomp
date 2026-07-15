/*
 * MFSource.h
 *
 * MFSource - Media Foundation source wrapper. Wraps an IMFSourceReader
 * for file-based media input and IMFMediaSource for source-resolver-based
 * scenarios. Provides sample-level access to audio/video streams.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef MFSOURCE_H
#define MFSOURCE_H

#include "AVSource.h"
#include <mfreadwrite.h>
#include <mfapi.h>

namespace HMRAVSource
{

// ============================================================================
// MFSourceReaderCallback
// ============================================================================
// Callback implementation for asynchronous MF source reader operations.
// Receives ReadSample, Flush, and status-change notifications.
//
class MFSourceReaderCallback :
    public IMFSourceReaderCallback
{
public:
    MFSourceReaderCallback();
    virtual ~MFSourceReaderCallback();

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IMFSourceReaderCallback
    STDMETHOD(OnReadSample)(
        HRESULT hrStatus,
        DWORD dwStreamIndex,
        DWORD dwStreamFlags,
        LONGLONG llTimestamp,
        IMFSample* pSample
    ) override;

    STDMETHOD(OnFlush)(DWORD dwStreamIndex) override;

    STDMETHOD(OnEvent)(DWORD dwStreamIndex, IMFMediaEvent* pEvent) override;

    // Caller access
    HRESULT WaitSample(DWORD dwTimeoutMs = 5000);
    HRESULT GetLastSample(IMFSample** ppSample, DWORD* pdwStreamIndex, DWORD* pdwStreamFlags, LONGLONG* pllTimestamp);
    void Reset();

private:
    LONG                        m_cRef;
    HANDLE                      m_hEvent;
    CComPtr<IMFSample>          m_spLastSample;
    DWORD                       m_dwLastStreamIndex;
    DWORD                       m_dwLastStreamFlags;
    LONGLONG                    m_llLastTimestamp;
    HRESULT                     m_hrLastStatus;
};

// ============================================================================
// MFSource
// ============================================================================
// Media Foundation source wrapper. Uses IMFSourceReader to open media files
// and read samples from audio/video streams. Supports both synchronous and
// asynchronous reading modes.
//
class MFSource : public AVSource
{
public:
    MFSource();
    virtual ~MFSource();

    // AVSource overrides
    HRESULT Open(const AVSourceDesc& desc) override;
    HRESULT Close() override;

    HRESULT SetPositionHns(LONGLONG llPosition) override;

    HRESULT Start() override;
    HRESULT Stop() override;

    HRESULT ReadSample(IMFSample** ppSample, DWORD dwStreamIndex) override;
    HRESULT Flush() override;

    HRESULT GetEvent(IMFMediaEvent** ppEvent) override;
    HRESULT BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState) override;
    HRESULT EndGetEvent(IMFMediaEvent* pEvent, IMFMediaEvent** ppNextEvent) override;

    // MF-specific queries
    HRESULT GetSourceReader(IMFSourceReader** ppReader);
    DWORD GetStreamCount() const throw();
    HRESULT GetStreamMediaType(DWORD dwStreamIndex, IMFMediaType** ppMediaType);

    // Video-specific
    HRESULT GetVideoFrameSize(UINT* pWidth, UINT* pHeight);
    HRESULT SetVideoFrameSize(UINT width, UINT height);

    // Audio-specific
    HRESULT GetAudioFormat(WAVEFORMATEX* pWfx);
    HRESULT SetAudioFormat(const WAVEFORMATEX* pWfx);

    // Native format access
    HRESULT GetSelectedMediaType(DWORD dwStreamIndex, IMFMediaType** ppType);
    HRESULT SetSelectedMediaType(DWORD dwStreamIndex, IMFMediaType* pType);

private:
    CComPtr<IMFSourceReader>        m_spReader;
    CComPtr<MFSourceReaderCallback>    m_spCallback;
    DWORD                           m_dwVideoStreamIndex;
    DWORD                           m_dwAudioStreamIndex;
    DWORD                           m_dwStreamCount;
    bool                            m_fAsyncMode;

    HRESULT CreateSourceReader(const AVSourceDesc& desc);
    HRESULT EnumerateStreams();
    HRESULT ConfigureVideoStream(UINT width, UINT height);
    HRESULT ConfigureAudioStream();
    HRESULT SeekToPosition(LONGLONG llPosition);
};

} // namespace HMRAVSource

#endif // MFSOURCE_H
