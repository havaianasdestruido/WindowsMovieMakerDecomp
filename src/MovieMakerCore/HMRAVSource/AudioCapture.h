/*
 * AudioCapture.h
 *
 * AudioCapture - Audio capture source (narration/microphone). Wraps
 * Media Foundation capture APIs for audio-only capture.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include "HMRAVSourceTypes.h"
#include "AVSource.h"
#include <mfapi.h>
#include <mfidl.h>

namespace HMRAVSource
{

// ============================================================================
// AudioCaptureDeviceInfo
// ============================================================================
struct AudioCaptureDeviceInfo
{
    ATL::CString    strDeviceName;
    ATL::CString    strDeviceId;
    DWORD           dwMaxChannels = 2;
    DWORD           dwMaxSampleRate = 44100;
    DWORD           dwDefaultSampleRate = 44100;
    DWORD           dwDefaultChannels = 1;
    bool            fHasAggregate = false;
};

// ============================================================================
// AudioCaptureConfig
// ============================================================================
struct AudioCaptureConfig
{
    DWORD   dwSampleRate = 44100;
    DWORD   dwChannels = 1;
    DWORD   dwBitsPerSample = 16;
    bool    fEnableAggregation = false;     // mix system audio + mic
};

// ============================================================================
// AudioCapture
// ============================================================================
// Manages audio capture from a microphone for narration recording.
// Uses Media Foundation capture APIs for device enumeration and audio
// sample capture.
//
class AVSOURCE_API AudioCapture
{
public:
    AudioCapture();
    ~AudioCapture();

    // Device enumeration
    static HRESULT EnumDevices(ATL::CAtlArray<AudioCaptureDeviceInfo>& devices);
    static HRESULT GetDefaultDevice(AudioCaptureDeviceInfo* pInfo);

    // Lifecycle
    HRESULT Initialize(const AudioCaptureDeviceInfo& deviceInfo);
    HRESULT Shutdown();

    // Capture
    HRESULT StartCapture();
    HRESULT StopCapture();
    HRESULT IsCapturing() const throw();

    // Pause / Resume
    HRESULT PauseCapture();
    HRESULT ResumeCapture();

    // Configuration
    HRESULT SetConfig(const AudioCaptureConfig& config);
    AudioCaptureConfig GetConfig() const throw();

    // State
    CaptureState GetState() const throw();

    // Sample reading
    HRESULT ReadSample(IMFSample** ppSample);

    // Volume
    HRESULT SetVolume(float flVolume);
    HRESULT GetVolume(float* pflVolume);

    // Mute
    HRESULT SetMute(bool fMute);
    bool IsMuted() const throw();

    // Device info
    AudioCaptureDeviceInfo GetDeviceInfo() const;

    // Sample callback
    using SampleCallback = std::function<void(IMFSample*)>;
    void SetSampleCallback(SampleCallback cb) { m_sampleCb = cb; }

private:
    CComPtr<IMFMediaSource>     m_spMediaSource;
    CComPtr<IMFSourceReader>    m_spReader;

    AudioCaptureDeviceInfo       m_deviceInfo;
    AudioCaptureConfig           m_config;
    CaptureState                 m_state;
    float                        m_flVolume;
    bool                         m_fMuted;

    SampleCallback               m_sampleCb;

    HRESULT CreateAudioSource();
    HRESULT ConfigureReader();
};

} // namespace HMRAVSource

#endif // AUDIOCAPTURE_H
