/*
 * AudioBoost.h
 *
 * Audio volume boost/normalization. Provides dynamic range compression,
 * peak normalization, and loudness normalization for audio tracks.
 *
 * RTTI classes:
 *   ?AVAudioBoost@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AUDIOBOOST_H
#define AUDIOBOOST_H

#include "../HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// AudioBoostMode
// ============================================================================
enum AudioBoostMode
{
    AudioBoostModeNone          = 0,
    AudioBoostModeFixedGain     = 1,    // apply fixed dB gain
    AudioBoostModePeakNormalize = 2,    // normalize to peak level
    AudioBoostModeLoudnessNorm  = 3,    // ITU-R BS.1770 loudness
    AudioBoostModeCompress      = 4     // dynamic range compression
};

// ============================================================================
// AudioBoost
// ============================================================================
// Provides audio volume boost and normalization for audio tracks.
// Supports fixed gain, peak normalization, loudness normalization,
// and dynamic range compression.
//
class AVSOURCE_API AudioBoost
{
public:
    AudioBoost();
    ~AudioBoost();

    // Lifecycle
    HRESULT Initialize(DWORD dwSampleRate, DWORD dwChannels);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Mode
    AudioBoostMode GetMode() const throw();
    void SetMode(AudioBoostMode mode) throw();

    // Fixed gain
    float GetFixedGainDb() const throw();
    void SetFixedGainDb(float flDb) throw();

    // Target levels
    float GetTargetPeakLevel() const throw();      // 0.0 - 1.0
    void SetTargetPeakLevel(float flLevel) throw();

    float GetTargetLoudnessLu() const throw();     // LUFS
    void SetTargetLoudnessLu(float flLu) throw();

    // Compression parameters
    float GetCompressionThresholdDb() const throw();
    void SetCompressionThresholdDb(float flDb) throw();

    float GetCompressionRatio() const throw();
    void SetCompressionRatio(float flRatio) throw();

    DWORD GetAttackMs() const throw();
    void SetAttackMs(DWORD dwMs) throw();

    DWORD GetReleaseMs() const throw();
    void SetReleaseMs(DWORD dwMs) throw();

    // Processing
    HRESULT ProcessBuffer(float* pBuffer, DWORD dwFrameCount);
    HRESULT ProcessBufferInt16(short* pBuffer, DWORD dwSampleCount);

    // Convenience
    void SetBoostLevel(float flLevel);
    float ProcessSample(float flSample);
    void SetNormalize(bool fEnabled);

    // Analysis results
    float GetMeasuredPeakLevel() const throw();
    float GetMeasuredRmsLevel() const throw();
    float GetMeasuredLoudness() const throw();
    float GetAppliedGainDb() const throw();

    // Reset analysis
    void ResetAnalysis();

private:
    float ComputeLinearGain(float flInputPeak);
    void ApplyGain(float* pBuffer, DWORD dwFrameCount, float flLinearGain);

    AudioBoostMode  m_mode;
    float           m_flFixedGainDb;
    float           m_flTargetPeakLevel;
    float           m_flTargetLoudnessLu;
    float           m_flCompressionThresholdDb;
    float           m_flCompressionRatio;
    DWORD           m_dwAttackMs;
    DWORD           m_dwReleaseMs;

    float           m_flMeasuredPeak;
    float           m_flMeasuredRms;
    float           m_flMeasuredLoudness;
    float           m_flAppliedGainDb;

    DWORD           m_dwSampleRate;
    DWORD           m_dwChannels;
    bool            m_fInitialized;
    bool            m_fNormalize;
};

} // namespace HMRAVSource

#endif // AUDIOBOOST_H
