/*
 * AudioAnalysis.h
 *
 * Audio analysis for ducking and RMS measurement. Provides real-time
 * audio level analysis used for audio ducking and waveform display.
 *
 * RTTI classes:
 *   ?AVAudioRMSData@@, ?AVWaveformCallback@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AUDIOANALYSIS_H
#define AUDIOANALYSIS_H

#include "../HMRAVSource.h"
#include "Waveform.h"

namespace HMRAVSource
{

// ============================================================================
// AudioRMSData
// ============================================================================
// Stores RMS (Root Mean Square) audio level data for a time window.
// Used by the ducking system to detect dialogue/narration levels.
//
class AVSOURCE_API AudioRMSData
{
public:
    AudioRMSData();
    ~AudioRMSData();

    // Lifecycle
    HRESULT Initialize(DWORD dwSampleRate, DWORD dwChannels, DWORD dwWindowSizeMs = 50);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Feed audio samples
    HRESULT FeedSamples(const float* pSamples, DWORD dwFrameCount);
    HRESULT FeedSamplesInt16(const short* pSamples, DWORD dwSampleCount);

    // Query current levels
    float GetCurrentRmsLevel() const throw();
    float GetCurrentPeakLevel() const throw();
    float GetCurrentRmsLevelDb() const throw();
    float GetCurrentPeakLevelDb() const throw();

    // Windowed RMS (over the analysis window)
    float GetWindowRmsLevel() const throw();
    float GetWindowPeakLevel() const throw();

    // Threshold detection
    bool IsAboveThreshold(float flThresholdDb) const throw();
    DWORD GetTimeAboveThresholdMs() const throw();

    // Time above threshold tracking
    void SetSampleRateForThresholdTracking(DWORD dwSampleRate);

    // Reset
    void Reset();

    // Configuration
    DWORD GetWindowSizeMs() const throw();
    void SetWindowSizeMs(DWORD dwMs) throw();

    // Statistics
    float GetMaxRmsLevel() const throw();
    float GetAverageRmsLevel() const throw();

    // File-based analysis
    HRESULT AnalyzeFile(LPCWSTR pszFilePath);
    float GetPeakLevel() const throw();
    float GetRMSLevel() const throw();
    const ATL::CAtlArray<float>& GetWaveformData() const throw();

private:
    void UpdateWindowLevel();

    ATL::CAtlArray<float>   m_arrWaveformData;

    ATL::CAtlArray<float>   m_arrWindowBuffer;
    DWORD                   m_dwWindowSizeSamples;
    DWORD                   m_dwSamplePosition;
    DWORD                   m_dwSampleRate;
    DWORD                   m_dwChannels;
    DWORD                   m_dwWindowSizeMs;

    float                   m_flCurrentRms;
    float                   m_flCurrentPeak;
    float                   m_flWindowRms;
    float                   m_flWindowPeak;
    float                   m_flMaxRms;
    float                   m_flTotalRms;
    DWORD                   m_dwSampleCount;
    DWORD                   m_dwTimeAboveThresholdSamples;
    bool                    m_fInitialized;
};

// ============================================================================
// WaveformCallback
// ============================================================================
// Callback interface for receiving waveform data updates. Implemented
// by the UI layer to receive real-time waveform data for visualization.
//
class AVSOURCE_API WaveformCallback
{
public:
    WaveformCallback();
    virtual ~WaveformCallback();

    // Called when new waveform data is available
    virtual void OnWaveformUpdate(float flRmsLevel, float flPeakLevel, LONGLONG llTimestampHns);

    // Called when waveform analysis is complete
    virtual void OnWaveformComplete();

    // Called on error during waveform analysis
    virtual void OnWaveformError(HRESULT hrError);

    // Enable/disable callbacks
    bool IsEnabled() const throw();
    void SetEnabled(bool fEnabled) throw();

private:
    bool m_fEnabled;
};

// ============================================================================
// AudioDuckingTrackDataSource
// ============================================================================
// Provides audio level data for the ducking system. Monitors a narration
// or dialogue track and signals when the ducking system should reduce
// background music volume.
//
class AVSOURCE_API DuckingTrackDataSource
{
public:
    DuckingTrackDataSource();
    ~DuckingTrackDataSource();

    // Lifecycle
    HRESULT Initialize(DWORD dwSampleRate, DWORD dwChannels);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Feed audio data
    HRESULT ProcessBuffer(const float* pBuffer, DWORD dwFrameCount);

    // Ducking state
    bool IsDuckingActive() const throw();
    float GetCurrentDuckLevel() const throw();   // 0.0 = no ducking, 1.0 = full duck

    // Configuration
    float GetThresholdDb() const throw();
    void SetThresholdDb(float flDb) throw();

    DWORD GetAttackTimeMs() const throw();
    void SetAttackTimeMs(DWORD dwMs) throw();

    DWORD GetReleaseTimeMs() const throw();
    void SetReleaseTimeMs(DWORD dwMs) throw();

    float GetMaxDuckLevel() const throw();
    void SetMaxDuckLevel(float flLevel) throw();

    // RMS data access
    AudioRMSData* GetRmsData();
    const AudioRMSData* GetRmsData() const;

private:
    AudioRMSData    m_rmsData;
    float           m_flThresholdDb;
    DWORD           m_dwAttackTimeMs;
    DWORD           m_dwReleaseTimeMs;
    float           m_flMaxDuckLevel;
    float           m_flCurrentDuckLevel;
    DWORD           m_dwSampleRate;
    bool            m_fDuckingActive;
    bool            m_fInitialized;
};

} // namespace HMRAVSource

#endif // AUDIOANALYSIS_H
