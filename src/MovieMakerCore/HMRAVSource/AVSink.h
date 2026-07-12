/*
 * AVSink.h
 *
 * AVSink - Audio/video output sink that receives decoded samples from
 * the source reader pipeline. Routes video samples to a video renderer
 * and audio samples to an audio output device for preview playback.
 *
 * RTTI: ?AVAVSink@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AVSINK_H
#define AVSINK_H

#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// AVSinkDesc
// ============================================================================
struct AVSinkDesc
{
    HWND            hVideoWnd;
    DWORD           dwVideoWidth;
    DWORD           dwVideoHeight;
    DWORD           dwAudioSampleRate;
    DWORD           dwAudioChannels;
    DWORD           dwAudioBitsPerSample;
    float           flVolume;
    bool            fEnableVideo;
    bool            fEnableAudio;

    AVSinkDesc()
        : hVideoWnd(nullptr)
        , dwVideoWidth(0)
        , dwVideoHeight(0)
        , dwAudioSampleRate(44100)
        , dwAudioChannels(2)
        , dwAudioBitsPerSample(16)
        , flVolume(1.0f)
        , fEnableVideo(true)
        , fEnableAudio(true)
    {
    }
};

// ============================================================================
// AVSinkState
// ============================================================================
enum AVSinkInternalState
{
    AVSinkStateUninitialized = 0,
    AVSinkStateReady         = 1,
    AVSinkStateRunning       = 2,
    AVSinkStatePaused        = 3,
    AVSinkStateError         = 4
};

// ============================================================================
// AVSink
// ============================================================================
// Audio/video output sink. Receives decoded samples from the pipeline and
// routes them to video rendering (EVR) or audio output (WASAPI). Manages
// sample queues, timing synchronization, and format negotiation.
//
class AVSOURCE_API AVSink
{
public:
    AVSink();
    virtual ~AVSink();

    // Lifecycle
    HRESULT Initialize(const AVSinkDesc& desc);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Sample reception
    HRESULT ProcessVideoSample(IMFSample* pSample);
    HRESULT ProcessAudioSample(IMFSample* pSample);

    // Flush
    HRESULT Flush();

    // State
    AVSinkInternalState GetState() const throw();

    // Volume
    HRESULT SetVolume(float flVolume);
    float GetVolume() const throw();

    // Video window
    HRESULT SetVideoWindow(HWND hWnd);
    HWND GetVideoWindow() const throw();

    // Format changes
    HRESULT SetVideoFormat(DWORD dwWidth, DWORD dwHeight, const GUID* pSubtype);
    HRESULT SetAudioFormat(DWORD dwSampleRate, DWORD dwChannels, DWORD dwBitsPerSample);

    // Statistics
    DWORD GetVideoFramesProcessed() const throw();
    DWORD GetAudioSamplesProcessed() const throw();

    // Callback for rendering events
    using RenderCallback = std::function<void(HRESULT, LONGLONG)>;
    void SetRenderCallback(RenderCallback cb) { m_renderCb = cb; }

private:
    AVSinkInternalState     m_state;
    HWND                    m_hVideoWnd;
    float                   m_flVolume;
    bool                    m_fEnableVideo;
    bool                    m_fEnableAudio;
    DWORD                   m_dwVideoWidth;
    DWORD                   m_dwVideoHeight;
    DWORD                   m_dwAudioSampleRate;
    DWORD                   m_dwAudioChannels;
    DWORD                   m_dwAudioBitsPerSample;
    DWORD                   m_dwVideoFramesProcessed;
    DWORD                   m_dwAudioSamplesProcessed;

    CComPtr<IMFVideoDisplayControl> m_spVideoDisplay;
    CComPtr<IUnknown>               m_spAudioRenderer;

    RenderCallback          m_renderCb;

    HRESULT CreateVideoRenderer();
    HRESULT CreateAudioRenderer();
    HRESULT ConfigureVideoRenderer();
    HRESULT ConfigureAudioRenderer();
    HRESULT ProcessVideoFrameInternal(IMFSample* pSample);
    HRESULT ProcessAudioBufferInternal(IMFSample* pSample);
};

} // namespace HMRAVSource

#endif // AVSINK_H
