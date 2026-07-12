/*
 * AVCaptureSession.h
 *
 * AVCaptureSession / AudioCaptureSession - Capture session management.
 * Coordinates video and audio capture within a unified session, managing
 * start/stop, timing, and sample coordination.
 *
 * RTTI: ?AVAVCaptureSession@@, ?AVAudioCaptureSession@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AVCAPTURESESSION_H
#define AVCAPTURESESSION_H

#include "HMRAVSource.h"
#include "VideoCapture.h"
#include "AudioCapture.h"

namespace HMRAVSource
{

// ============================================================================
// AVCaptureSessionDesc
// ============================================================================
struct AVCaptureSessionDesc
{
    VideoCaptureDeviceInfo   videoDevice;
    AudioCaptureDeviceInfo   audioDevice;
    VideoCaptureConfig       videoConfig;
    AudioCaptureConfig       audioConfig;
    ATL::CString             strOutputPath;
    bool                     fCaptureVideo = true;
    bool                     fCaptureAudio = true;
    LONGLONG                 llMaxDurationHns = 0;   // 0 = unlimited
};

// ============================================================================
// AVCaptureSessionStats
// ============================================================================
struct AVCaptureSessionStats
{
    DWORD       dwVideoFramesCaptured = 0;
    DWORD       dwAudioSamplesCaptured = 0;
    LONGLONG    llElapsedHns = 0;
    LONGLONG    llDiskSpaceUsed = 0;
    bool        fActive = false;
};

// ============================================================================
// AVCaptureSession
// ============================================================================
// Manages a combined video+audio capture session. Coordinates the webcam
// and microphone captures, provides session-level timing, and manages
// the output file lifecycle.
//
// From RTTI: ?AVAVCaptureSession@@
//
class AVSOURCE_API AVCaptureSession
{
public:
    AVCaptureSession();
    virtual ~AVCaptureSession();

    // Lifecycle
    HRESULT Initialize(const AVCaptureSessionDesc& desc);
    HRESULT Shutdown();

    // Session control
    HRESULT StartSession();
    HRESULT StopSession();
    HRESULT PauseSession();
    HRESULT ResumeSession();

    // State
    CaptureState GetState() const throw();
    bool IsActive() const throw();

    // Statistics
    AVCaptureSessionStats GetStats() const;

    // Individual captures
    VideoCapture* GetVideoCapture() { return m_pVideoCapture; }
    AudioCapture* GetAudioCapture() { return m_pAudioCapture; }

    // Duration
    LONGLONG GetSessionDurationHns() const throw();
    void SetMaxDurationHns(LONGLONG llMax) throw();

    // Output path
    ATL::CString GetOutputPath() const;

    // Callbacks
    using StatsCallback = std::function<void(const AVCaptureSessionStats&)>;
    using ErrorCallback = std::function<void(HRESULT)>;
    using CompleteCallback = std::function<void(HRESULT)>;

    void SetStatsCallback(StatsCallback cb) { m_statsCb = cb; }
    void SetErrorCallback(ErrorCallback cb) { m_errorCb = cb; }
    void SetCompleteCallback(CompleteCallback cb) { m_completeCb = cb; }

protected:
    VideoCapture*       m_pVideoCapture;
    AudioCapture*       m_pAudioCapture;
    AVCaptureSessionDesc m_desc;
    CaptureState        m_state;
    AVCaptureSessionStats m_stats;
    LONGLONG            m_llStartTimeHns;
    LONGLONG            m_llMaxDurationHns;
    bool                m_fActive;

    StatsCallback       m_statsCb;
    ErrorCallback       m_errorCb;
    CompleteCallback    m_completeCb;

    void UpdateStats();
    void CheckMaxDuration();
    void NotifyStats();
};

// ============================================================================
// AudioCaptureSession
// ============================================================================
// Simplified capture session for audio-only recording (narration).
// Wraps AudioCapture with session-level timing and file output.
//
// From RTTI: ?AVAudioCaptureSession@@
//
class AVSOURCE_API AudioCaptureSession
{
public:
    AudioCaptureSession();
    ~AudioCaptureSession();

    // Lifecycle
    HRESULT Initialize(const AudioCaptureDeviceInfo& deviceInfo,
                       const AudioCaptureConfig& config,
                       LPCWSTR pszOutputPath);
    HRESULT Shutdown();

    // Session control
    HRESULT StartSession();
    HRESULT StopSession();
    HRESULT PauseSession();
    HRESULT ResumeSession();

    // State
    CaptureState GetState() const throw();
    bool IsActive() const throw();

    // Duration
    LONGLONG GetSessionDurationHns() const throw();

    // AudioCapture access
    AudioCapture* GetAudioCapture() { return m_pAudioCapture; }

    // Callbacks
    using CompleteCallback = std::function<void(HRESULT, LPCWSTR)>;
    void SetCompleteCallback(CompleteCallback cb) { m_completeCb = cb; }

private:
    AudioCapture*       m_pAudioCapture;
    AudioCaptureConfig  m_config;
    ATL::CString        m_strOutputPath;
    CaptureState        m_state;
    bool                m_fActive;
    LONGLONG            m_llStartTimeHns;
    CompleteCallback    m_completeCb;
};

} // namespace HMRAVSource

#endif // AVCAPTURESESSION_H
