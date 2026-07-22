/*
 * AudioDuckingProcessor.h
 *
 * Audio ducking processor. Applies gain reduction to a background
 * music track based on real-time narration/dialogue level analysis.
 * Works in conjunction with DuckingTrackDataSource for level monitoring.
 *
 * RTTI classes:
 *   ?AVAudioDuckingProcessor@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AUDIODUCKINGPROCESSOR_H
#define AUDIODUCKINGPROCESSOR_H

#include "../HMRAVSource.h"
#include "AudioAnalysis.h"

namespace HMRAVSource
{

// ============================================================================
// AudioDuckingProcessor
// ============================================================================
// Applies audio ducking to background music audio buffers. Monitors
// narration/dialogue levels via a linked DuckingTrackDataSource and
// smoothly attenuates the background track when speech is detected.
//
class AVSOURCE_API AudioDuckingProcessor
{
public:
    AudioDuckingProcessor();
    ~AudioDuckingProcessor();

    // Lifecycle
    HRESULT Initialize(DWORD dwSampleRate, DWORD dwChannels);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Link the narration-level monitor
    void SetDuckingSource(DuckingTrackDataSource* pSource) throw();
    DuckingTrackDataSource* GetDuckingSource() const throw();

    // Ducking parameters
    void SetThresholdDb(float flDb) throw();
    float GetThresholdDb() const throw();

    void SetDuckLevel(float flLevel) throw();
    float GetDuckLevel() const throw();

    void SetAttackTimeMs(DWORD dwMs) throw();
    DWORD GetAttackTimeMs() const throw();

    void SetReleaseTimeMs(DWORD dwMs) throw();
    DWORD GetReleaseTimeMs() const throw();

    // Fade times for ducking transitions
    void SetFadeInMs(DWORD dwMs) throw();
    DWORD GetFadeInMs() const throw();

    void SetFadeOutMs(DWORD dwMs) throw();
    DWORD GetFadeOutMs() const throw();

    // Processing - applies ducking gain to background audio
    HRESULT ProcessBufferFloat(float* pBuffer, DWORD dwFrameCount,
                               LONGLONG llPositionHns);

    HRESULT ProcessBufferInt16(short* pBuffer, DWORD dwFrameCount,
                               LONGLONG llPositionHns);

    // Update narration level - call after feeding narration samples
    HRESULT UpdateNarrationLevel(const float* pNarrationSamples, DWORD dwFrameCount);

    // Current state
    bool IsDuckingActive() const throw();
    float GetCurrentGainReduction() const throw();

    // Reset
    void Reset() throw();

private:
    DWORD                   m_dwSampleRate;
    DWORD                   m_dwChannels;
    float                   m_flThresholdDb;
    float                   m_flDuckLevel;
    DWORD                   m_dwAttackTimeMs;
    DWORD                   m_dwReleaseTimeMs;
    DWORD                   m_dwFadeInMs;
    DWORD                   m_dwFadeOutMs;
    float                   m_flCurrentDuckLevel;
    float                   m_flTargetDuckLevel;
    bool                    m_fDuckingActive;
    bool                    m_fInitialized;

    DuckingTrackDataSource* m_pDuckingSource;

    float ComputeGainReduction() const throw();
    float ComputeSmoothedDuck(float flTargetDuck, DWORD dwFrameCount) throw();
};

} // namespace HMRAVSource

#endif // AUDIODUCKINGPROCESSOR_H
