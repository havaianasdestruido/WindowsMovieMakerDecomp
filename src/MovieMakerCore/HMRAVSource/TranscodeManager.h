/*
 * TranscodeManager.h
 *
 * TranscodeManager - Orchestrates the full transcode pipeline: reads from
 * an AVSource, applies encoding via MF transforms, and writes to a
 * StreamSink. Manages the transcode session lifecycle, progress reporting,
 * and cancellation.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef TRANSCODEMANAGER_H
#define TRANSCODEMANAGER_H

#include "HMRAVSource.h"
#include "AVSource.h"
#include "EncodeProfile.h"
#include "StreamSink.h"

namespace HMRAVSource
{

// ============================================================================
// TranscodeParams
// ============================================================================
struct TranscodeParams
{
    ATL::CString        strInputPath;
    ATL::CString        strOutputPath;
    EncodeProfile       profile;
    LONGLONG            llStartHns = 0;
    LONGLONG            llEndHns = 0;           // 0 = full duration
    bool                fIncludeVideo = true;
    bool                fIncludeAudio = true;
    bool                fHardwareAcceleration = true;
    DWORD               dwNumThreads = 0;       // 0 = auto
};

// ============================================================================
// TranscodeProgress
// ============================================================================
struct TranscodeProgress
{
    LONGLONG    llInputDurationHns = 0;
    LONGLONG    llOutputDurationHns = 0;
    LONGLONG    llCurrentPositionHns = 0;
    DWORD       dwFramesEncoded = 0;
    DWORD       dwTotalFrames = 0;
    float       fPercentComplete = 0.0f;
    double      dblEncodingFps = 0.0f;
    double      dblElapsedSeconds = 0.0f;
    double      dblEstimatedSecondsRemaining = 0.0f;
    bool        fComplete = false;
    bool        fCancelled = false;
    bool        fError = false;
    HRESULT     hrError = S_OK;
};

// ============================================================================
// TranscodeManager
// ============================================================================
// Manages the full transcode pipeline. Connects an AVSource (input) to a
// StreamSink (output) through the Media Foundation transform chain.
// Provides progress callbacks, cancellation, and error reporting.
//
class AVSOURCE_API TranscodeManager
{
public:
    TranscodeManager();
    ~TranscodeManager();

    // Lifecycle
    HRESULT Initialize();
    HRESULT Shutdown();

    // Transcode
    HRESULT BeginTranscode(const TranscodeParams& params);
    HRESULT TranscodeFrame();
    HRESULT EndTranscode();
    void    CancelTranscode();

    // Status
    TranscodeProgress GetProgress() const;
    bool IsTranscoding() const throw();

    // Callbacks
    using ProgressCallback = std::function<void(const TranscodeProgress&)>;
    using ErrorCallback = std::function<void(HRESULT, LPCWSTR)>;
    using CompleteCallback = std::function<void(HRESULT, LPCWSTR)>;

    void SetProgressCallback(ProgressCallback cb) { m_progressCb = cb; }
    void SetErrorCallback(ErrorCallback cb) { m_errorCb = cb; }
    void SetCompleteCallback(CompleteCallback cb) { m_completeCb = cb; }

    // Multi-frame encode (drives the full pipeline)
    HRESULT TranscodeAllFrames();

    // Individual step
    HRESULT ProcessNextFrame();

    // Pause / Resume
    HRESULT PauseTranscode();
    HRESULT ResumeTranscode();

    // Source access
    AVSource* GetSource() const { return m_spSource; }
    StreamSink* GetSink() const { return m_spSink; }

private:
    AVSource*               m_spSource;
    StreamSink*             m_spSink;
    TranscodeParams         m_params;
    TranscodeProgress       m_progress;
    TranscodeState          m_state;
    bool                    m_fTranscoding;
    bool                    m_fPaused;

    ProgressCallback        m_progressCb;
    ErrorCallback           m_errorCb;
    CompleteCallback        m_completeCb;

    CComPtr<IMFTransform>   m_spVideoTransform;
    CComPtr<IMFTransform>   m_spAudioTransform;

    HRESULT CreateSource();
    HRESULT CreateSink();
    HRESULT ConfigureTransforms();
    HRESULT ProcessVideoFrame(IMFSample* pSample);
    HRESULT ProcessAudioSample(IMFSample* pSample);
    void    UpdateProgress(LONGLONG llPosition);
    void    FireError(HRESULT hr);
    void    FireComplete(HRESULT hr);
};

} // namespace HMRAVSource

#endif // TRANSCODEMANAGER_H
