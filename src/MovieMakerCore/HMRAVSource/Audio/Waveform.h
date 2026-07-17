/*
 * Waveform.h
 *
 * Audio waveform data for visualization. Stores RMS sample data
 * at regular intervals for waveform display in the timeline.
 *
 * RTTI classes:
 *   ?AVWaveform@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "../HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// WaveformSample
// ============================================================================
struct WaveformSample
{
    float   flRmsLevel;     // RMS amplitude (0.0-1.0)
    float   flPeakLevel;    // peak amplitude (0.0-1.0)
    LONGLONG llTimeHns;     // sample time in hundred-nanoseconds
};

// ============================================================================
// Waveform
// ============================================================================
// Stores audio waveform data for visualization. Provides the sample data
// used to draw waveform overlays on audio/music tracks in the timeline.
//
class AVSOURCE_API Waveform
{
public:
    Waveform();
    ~Waveform();

    // Lifecycle
    HRESULT Initialize(DWORD dwSampleRate, DWORD dwChannels, LONGLONG llDurationHns);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Sample data
    size_t GetSampleCount() const throw();
    const WaveformSample* GetSample(size_t nIndex) const;
    float GetRmsLevelAt(size_t nIndex) const throw();
    float GetPeakLevelAt(size_t nIndex) const throw();
    LONGLONG GetTimeAt(size_t nIndex) const throw();

    // Bulk data access
    const WaveformSample* GetSampleData() const throw();
    DWORD GetSamplesPerSecond() const throw();

    // Add samples
    HRESULT AddSample(const WaveformSample& sample);
    HRESULT AddSamples(const WaveformSample* pSamples, size_t cSamples);
    void RemoveAllSamples();

    // Query
    float GetMaxRmsLevel() const throw();
    float GetMaxPeakLevel() const throw();
    float GetAverageRmsLevel() const throw();

    // Time range
    LONGLONG GetStartTimeHns() const throw();
    LONGLONG GetEndTimeHns() const throw();
    LONGLONG GetDurationHns() const throw();

    // Find sample by time
    size_t FindSampleByTime(LONGLONG llTimeHns) const throw();

    // Downsampling for display
    HRESULT GetDisplaySamples(float* pOutput, size_t cOutputSamples,
                              LONGLONG llStartTime, LONGLONG llEndTime) const;

    // Sample rate and channels
    DWORD GetSampleRate() const throw();
    DWORD GetChannels() const throw();

    // Section caching support (for CachedWFSection integration)
    HRESULT CacheSection(LONGLONG llStartTimeHns, LONGLONG llEndTimeHns, DWORD dwSamplesPerSecond);
    bool IsSectionCached(LONGLONG llStartTimeHns, LONGLONG llEndTimeHns) const throw();

private:
    struct CachedSection
    {
        LONGLONG llStartTimeHns;
        LONGLONG llEndTimeHns;
        DWORD    dwSamplesPerSecond;
    };

    ATL::CAtlArray<WaveformSample>  m_arrSamples;
    ATL::CAtlArray<CachedSection>   m_arrCachedSections;
    DWORD                           m_dwSampleRate;
    DWORD                           m_dwChannels;
    LONGLONG                        m_llDurationHns;
    float                           m_flMaxRmsLevel;
    float                           m_flMaxPeakLevel;
    bool                            m_fInitialized;
};

} // namespace HMRAVSource

#endif // WAVEFORM_H
