/*
 * AudioOutput.h
 *
 * Audio output renderer with WASAPI (Windows Audio Session API).
 * Manages the audio output device, format negotiation, and sample
 * rendering to the default audio endpoint.
 *
 * RTTI classes:
 *   ?AVAudioOutput@@, ?AVAudioOutputCommandParam@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include "../HMRAVSource.h"
#include <mmdeviceapi.h>
#include <Audioclient.h>

namespace HMRAVSource
{

// ============================================================================
// AudioOutputFormat
// ============================================================================
struct AudioOutputFormat
{
    WAVEFORMATEX    wfx;
    DWORD           dwBufferSizeMs;

    AudioOutputFormat()
    {
        memset(&wfx, 0, sizeof(wfx));
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 44100;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
        dwBufferSizeMs = 50;
    }
};

// ============================================================================
// AudioOutputCommandParam
// ============================================================================
// Parameters for audio output commands (play, stop, pause, etc.)
//
struct AudioOutputCommandParam
{
    enum CommandType
    {
        CmdPlay         = 0,
        CmdStop         = 1,
        CmdPause        = 2,
        CmdResume       = 3,
        CmdFlush        = 4,
        CmdSetVolume    = 5,
        CmdSetMute      = 6,
        CmdSetPosition  = 7
    };

    CommandType     command;
    float           flVolume;       // 0.0 - 1.0
    bool            fMute;
    LONGLONG        llPositionHns;

    AudioOutputCommandParam()
        : command(CmdPlay)
        , flVolume(1.0f)
        , fMute(false)
        , llPositionHns(0)
    {
    }
};

// ============================================================================
// AudioOutput
// ============================================================================
// Manages audio output rendering through WASAPI. Handles device
// enumeration, format negotiation, and sample writing to the audio
// endpoint for playback during preview.
//
class AVSOURCE_API AudioOutput
{
public:
    AudioOutput();
    ~AudioOutput();

    // Lifecycle
    HRESULT Initialize(const AudioOutputFormat& format);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Device management
    HRESULT SetDefaultDevice();
    HRESULT SetDevice(LPCWSTR pszDeviceId);
    ATL::CString GetDeviceName() const;
    ATL::CString GetDeviceId() const;

    // Playback control
    HRESULT Play();
    HRESULT Stop();
    HRESULT Pause();
    HRESULT Resume();
    HRESULT Flush();

    // Sample writing
    HRESULT WriteSample(const BYTE* pData, DWORD cbData);
    HRESULT WriteSampleFromMF(IMFSample* pSample);

    // Volume
    HRESULT SetVolume(float flVolume);
    float GetVolume() const throw();
    HRESULT SetMute(bool fMute);
    bool IsMuted() const throw();

    // Position
    HRESULT SetPosition(LONGLONG llPositionHns);
    LONGLONG GetPosition() const throw();

    // State
    enum AudioOutputState
    {
        StateStopped    = 0,
        StatePlaying    = 1,
        StatePaused     = 2,
        StateError      = 3
    };

    AudioOutputState GetState() const throw();

    // Format
    const AudioOutputFormat& GetFormat() const;
    HRESULT SetFormat(const AudioOutputFormat& format);

    // Buffer info
    DWORD GetBufferDurationMs() const throw();
    DWORD GetBufferPaddingMs() const throw();

    // Command dispatch
    HRESULT ExecuteCommand(const AudioOutputCommandParam& param);

private:
    HRESULT CreateAudioClient();
    HRESULT StartSession();
    HRESULT StopSession();

    CComPtr<IMMDeviceEnumerator>    m_spDeviceEnumerator;
    CComPtr<IMMDevice>              m_spDevice;
    CComPtr<IAudioClient>           m_spAudioClient;
    CComPtr<IAudioRenderClient>     m_spRenderClient;

    AudioOutputFormat               m_format;
    AudioOutputState                m_state;
    float                           m_flVolume;
    bool                            m_fMute;
    LONGLONG                        m_llPositionHns;
    DWORD                           m_dwBufferFrameCount;
    bool                            m_fInitialized;
};

} // namespace HMRAVSource

#endif // AUDIOOUTPUT_H
