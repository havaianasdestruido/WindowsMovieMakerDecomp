/*
 * StreamSink.h
 *
 * StreamSink - Media Foundation stream sink for writing encoded samples
 * to the output file. Wraps IMFSinkWriter and manages individual stream
 * sinks for audio and video output.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef STREAMSINK_H
#define STREAMSINK_H

#include "HMRAVSource.h"
#include "EncodeProfile.h"
#include <mfreadwrite.h>
#include <mfapi.h>

namespace HMRAVSource
{

// ============================================================================
// StreamSinkDesc
// ============================================================================
struct StreamSinkDesc
{
    ATL::CString        strOutputPath;
    EncodeProfile       profile;
    DWORD               dwVideoStreamIndex = 0;
    DWORD               dwAudioStreamIndex = 1;
    bool                fHasVideo = true;
    bool                fHasAudio = true;
};

// ============================================================================
// StreamSinkStats
// ============================================================================
struct StreamSinkStats
{
    DWORD       dwFramesWritten = 0;
    DWORD       dwAudioSamplesWritten = 0;
    LONGLONG    llTotalBytesWritten = 0;
    LONGLONG    llDurationWrittenHns = 0;
    double      dblEncodingFps = 0.0;
    bool        fComplete = false;
};

// ============================================================================
// StreamSink
// ============================================================================
// Wraps an IMFSinkWriter to write encoded audio/video samples to an output
// file. Manages stream configuration, format negotiation, and the write
// pipeline. Supports both synchronous and asynchronous writing.
//
class AVSOURCE_API StreamSink
{
public:
    StreamSink();
    ~StreamSink();

    // Lifecycle
    HRESULT Initialize(const StreamSinkDesc& desc);
    HRESULT Shutdown();

    // Writing
    HRESULT BeginWriting();
    HRESULT WriteVideoSample(IMFSample* pSample);
    HRESULT WriteAudioSample(IMFSample* pSample);
    HRESULT EndWriting();

    // Format
    HRESULT SetInputVideoType(IMFMediaType* pType);
    HRESULT SetInputAudioType(IMFMediaType* pType);
    HRESULT SetOutputVideoType(IMFMediaType* pType);
    HRESULT SetOutputAudioType(IMFMediaType* pType);

    // Query
    StreamSinkState GetState() const throw();
    const StreamSinkStats& GetStats() const;

    // Flush
    HRESULT Flush();

    // Progress
    using ProgressCallback = std::function<void(const StreamSinkStats&)>;
    void SetProgressCallback(ProgressCallback cb) { m_progressCb = cb; }

    // Output file
    ATL::CString GetOutputPath() const;

    // Duration tracking
    void SetTotalDurationHns(LONGLONG llDuration) throw();
    LONGLONG GetTotalDurationHns() const throw();

private:
    CComPtr<IMFSinkWriter>      m_spSinkWriter;
    StreamSinkDesc              m_desc;
    StreamSinkState             m_state;
    StreamSinkStats             m_stats;
    DWORD                       m_dwVideoStreamIndex;
    DWORD                       m_dwAudioStreamIndex;
    LONGLONG                    m_llTotalDurationHns;
    ProgressCallback            m_progressCb;

    HRESULT ConfigureVideoStream();
    HRESULT ConfigureAudioStream();
    HRESULT FinalizeWriter();
    void UpdateStats(IMFSample* pSample, bool fVideo);
    void FireProgress();
};

} // namespace HMRAVSource

#endif // STREAMSINK_H
