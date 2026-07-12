/*
 * Mixer.h
 *
 * Audio mixer for multi-track mixing. Combines multiple audio streams
 * into a single output, with per-track volume and pan control.
 *
 * RTTI classes:
 *   ?AVMixer@@, ?AVMixerBuffer@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef MIXER_H
#define MIXER_H

#include "../HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// MixerTrackInfo
// ============================================================================
struct MixerTrackInfo
{
    DWORD   dwTrackIndex;
    float   flVolume;           // 0.0 - 1.0
    float   flPan;              // -1.0 (left) to 1.0 (right)
    bool    fMuted;
    bool    fSolo;
    float   flGainDb;           // gain in dB

    MixerTrackInfo()
        : dwTrackIndex(0)
        , flVolume(1.0f)
        , flPan(0.0f)
        , fMuted(false)
        , fSolo(false)
        , flGainDb(0.0f)
    {
    }
};

// ============================================================================
// MixerBuffer
// ============================================================================
// Internal mixing buffer that accumulates mixed audio samples. Uses
// floating-point accumulation for high-quality mixing.
//
class AVSOURCE_API MixerBuffer
{
public:
    MixerBuffer();
    ~MixerBuffer();

    HRESULT Initialize(DWORD dwSampleRate, DWORD dwChannels, DWORD dwMaxFrames);
    HRESULT Shutdown();

    HRESULT Clear();
    HRESULT AddSamples(const float* pSamples, DWORD dwFrameCount, float flVolume);
    HRESULT AddSamplesWithPan(const float* pSamples, DWORD dwFrameCount, float flVolume, float flPan);

    // Retrieve mixed result
    HRESULT GetResultAsFloat(float* pOutput, DWORD dwFrameCount) const;
    HRESULT GetResultAsInt16(short* pOutput, DWORD dwFrameCount) const;
    HRESULT GetResultAsInt32(int* pOutput, DWORD dwFrameCount) const;

    DWORD GetFrameCount() const throw();
    DWORD GetSampleRate() const throw();
    DWORD GetChannels() const throw();

private:
    ATL::CAtlArray<float>   m_arrAccumulator;
    DWORD                   m_dwSampleRate;
    DWORD                   m_dwChannels;
    DWORD                   m_dwMaxFrames;
    DWORD                   m_dwFrameCount;
};

// ============================================================================
// Mixer
// ============================================================================
// Multi-track audio mixer. Accepts audio input from multiple tracks,
// applies per-track volume/pan/mute, and produces a mixed output.
//
class AVSOURCE_API Mixer
{
public:
    Mixer();
    ~Mixer();

    // Lifecycle
    HRESULT Initialize(DWORD dwSampleRate, DWORD dwChannels, DWORD dwTrackCount);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Track configuration
    HRESULT SetTrackVolume(DWORD dwTrack, float flVolume);
    float GetTrackVolume(DWORD dwTrack) const throw();

    HRESULT SetTrackPan(DWORD dwTrack, float flPan);
    float GetTrackPan(DWORD dwTrack) const throw();

    HRESULT SetTrackMute(DWORD dwTrack, bool fMute);
    bool IsTrackMuted(DWORD dwTrack) const throw();

    HRESULT SetTrackSolo(DWORD dwTrack, bool fSolo);
    bool IsTrackSolo(DWORD dwTrack) const throw();

    HRESULT SetTrackGain(DWORD dwTrack, float flGainDb);
    float GetTrackGain(DWORD dwTrack) const throw();

    // Master volume
    HRESULT SetMasterVolume(float flVolume);
    float GetMasterVolume() const throw();

    // Mixing
    HRESULT MixFrame(const float* pSamples, DWORD dwTrack, DWORD dwFrameCount);
    HRESULT GetMixedOutput(float* pOutput, DWORD dwFrameCount);
    HRESULT GetMixedOutputAsInt16(short* pOutput, DWORD dwFrameCount);
    HRESULT ResetMixBuffer();

    // Track count
    DWORD GetTrackCount() const throw();

    // Format
    DWORD GetSampleRate() const throw();
    DWORD GetChannels() const throw();

private:
    MixerBuffer         m_mixBuffer;
    ATL::CAtlArray<MixerTrackInfo> m_arrTracks;
    DWORD               m_dwSampleRate;
    DWORD               m_dwChannels;
    float               m_flMasterVolume;
    bool                m_fInitialized;
};

} // namespace HMRAVSource

#endif // MIXER_H
