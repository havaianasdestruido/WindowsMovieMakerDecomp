/*
 * StreamSinkHost.h
 *
 * StreamSinkHost - Manages multiple stream sinks for a single output.
 * Coordinates video and audio stream sinks, handles stream selection,
 * format negotiation, and provides a unified interface for writing.
 *
 * AudioStreamSink - Audio-specific stream sink configuration and
 * sample handling with volume and mixing support.
 *
 * RTTI: ?AVStreamSinkHost@@, ?AVAudioStreamSink@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef STREAMSINKHOST_H
#define STREAMSINKHOST_H

#include "HMRAVSource.h"
#include "EncodeProfile.h"
#include "StreamSink.h"

namespace HMRAVSource
{

// ============================================================================
// AudioStreamSink
// ============================================================================
// Audio-specific stream sink configuration. Manages audio format
// negotiation, volume scaling, and sample conversion for the audio
// output stream.
//
class AVSOURCE_API AudioStreamSink
{
public:
    AudioStreamSink();
    ~AudioStreamSink();

    // Lifecycle
    HRESULT Initialize(DWORD dwStreamIndex, const EncodeAudioParams& audioParams);
    HRESULT Shutdown();

    // Sample writing
    HRESULT WriteSample(IMFSample* pSample);

    // Format
    HRESULT SetInputMediaType(IMFMediaType* pType);
    HRESULT SetOutputMediaType(IMFMediaType* pType);
    HRESULT GetInputMediaType(IMFMediaType** ppType);
    HRESULT GetOutputMediaType(IMFMediaType** ppType);

    // Volume
    HRESULT SetVolume(float flVolume);
    float GetVolume() const throw();

    // Mute
    HRESULT SetMute(bool fMute);
    bool IsMuted() const throw();

    // State
    StreamSinkState GetState() const throw();
    DWORD GetStreamIndex() const throw();

    // Statistics
    DWORD GetSamplesWritten() const throw();
    LONGLONG GetDurationWrittenHns() const throw();

    // Format query
    DWORD GetSampleRate() const throw();
    DWORD GetChannels() const throw();
    DWORD GetBitsPerSample() const throw();

private:
    DWORD               m_dwStreamIndex;
    StreamSinkState     m_state;
    EncodeAudioParams   m_audioParams;
    float               m_flVolume;
    bool                m_fMute;
    DWORD               m_dwSamplesWritten;
    LONGLONG            m_llDurationWrittenHns;

    CComPtr<IMFMediaType> m_spInputType;
    CComPtr<IMFMediaType> m_spOutputType;

    HRESULT ConvertAndScaleSample(IMFSample* pSample, IMFSample** ppConverted);
};

// ============================================================================
// StreamSinkHost
// ============================================================================
// Manages multiple stream sinks (video + audio) for a single transcode
// output. Coordinates stream configuration, sample routing, and the
// overall write lifecycle across all active streams.
//
class AVSOURCE_API StreamSinkHost
{
public:
    StreamSinkHost();
    ~StreamSinkHost();

    // Lifecycle
    HRESULT Initialize(LPCWSTR pszOutputPath, const EncodeProfile& profile);
    HRESULT Shutdown();

    // Stream management
    HRESULT AddVideoStream(const EncodeVideoParams& videoParams);
    HRESULT AddAudioStream(const EncodeAudioParams& audioParams);
    HRESULT RemoveStream(DWORD dwStreamIndex);

    // Writing
    HRESULT BeginWriting();
    HRESULT WriteVideoSample(IMFSample* pSample);
    HRESULT WriteAudioSample(IMFSample* pSample, DWORD dwStreamIndex);
    HRESULT WriteAudioSampleToDefault(IMFSample* pSample);
    HRESULT EndWriting();

    // Stream access
    DWORD GetStreamCount() const throw();
    AudioStreamSink* GetAudioSink(DWORD dwStreamIndex);
    DWORD GetDefaultAudioStreamIndex() const throw();
    DWORD GetVideoStreamIndex() const throw();

    // State
    StreamSinkState GetState() const throw();
    bool IsWriting() const throw();

    // Statistics
    const StreamSinkStats& GetStats() const;
    void ResetStats();

    // Format overrides
    HRESULT SetInputVideoType(IMFMediaType* pType);
    HRESULT SetInputAudioType(DWORD dwStreamIndex, IMFMediaType* pType);
    HRESULT SetOutputVideoType(IMFMediaType* pType);
    HRESULT SetOutputAudioType(DWORD dwStreamIndex, IMFMediaType* pType);

    // Progress callback
    using ProgressCallback = std::function<void(const StreamSinkStats&)>;
    void SetProgressCallback(ProgressCallback cb) { m_progressCb = cb; }

    // Output
    ATL::CString GetOutputPath() const;

private:
    CComPtr<IMFSinkWriter>          m_spSinkWriter;
    ATL::CString                    m_strOutputPath;
    EncodeProfile                   m_profile;
    StreamSinkState                 m_state;
    StreamSinkStats                 m_stats;
    DWORD                           m_dwVideoStreamIndex;
    DWORD                           m_dwDefaultAudioIndex;
    ATL::CAtlArray<AudioStreamSink> m_audioSinks;
    ProgressCallback                m_progressCb;

    HRESULT CreateSinkWriter();
    HRESULT NegotiateVideoType();
    HRESULT NegotiateAudioType(AudioStreamSink& audioSink);
    void UpdateStats(IMFSample* pSample, bool fVideo);
    void FireProgress();
};

} // namespace HMRAVSource

#endif // STREAMSINKHOST_H
