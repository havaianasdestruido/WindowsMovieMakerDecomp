#include "pch.h"
/*
 * Waveform.cpp
 *
 * Implementation of the Waveform class for audio visualization data.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "Waveform.h"

namespace HMRAVSource
{

Waveform::Waveform()
    : m_dwSampleRate(0)
    , m_dwChannels(0)
    , m_llDurationHns(0)
    , m_flMaxRmsLevel(0.0f)
    , m_flMaxPeakLevel(0.0f)
    , m_fInitialized(false)
{
}

Waveform::~Waveform()
{
    Shutdown();
}

HRESULT Waveform::Initialize(DWORD dwSampleRate, DWORD dwChannels, LONGLONG llDurationHns)
{
    if (dwSampleRate == 0 || dwChannels == 0)
        return E_INVALIDARG;

    m_dwSampleRate = dwSampleRate;
    m_dwChannels = dwChannels;
    m_llDurationHns = llDurationHns;
    m_fInitialized = true;
    return S_OK;
}

HRESULT Waveform::Shutdown()
{
    m_arrSamples.RemoveAll();
    m_dwSampleRate = 0;
    m_dwChannels = 0;
    m_llDurationHns = 0;
    m_flMaxRmsLevel = 0.0f;
    m_flMaxPeakLevel = 0.0f;
    m_fInitialized = false;
    return S_OK;
}

bool Waveform::IsInitialized() const throw() { return m_fInitialized; }

size_t Waveform::GetSampleCount() const throw() { return m_arrSamples.GetCount(); }

const WaveformSample* Waveform::GetSample(size_t nIndex) const
{
    if (nIndex >= m_arrSamples.GetCount()) return nullptr;
    return &m_arrSamples.GetAt(nIndex);
}

float Waveform::GetRmsLevelAt(size_t nIndex) const throw()
{
    if (nIndex >= m_arrSamples.GetCount()) return 0.0f;
    return m_arrSamples.GetAt(nIndex).flRmsLevel;
}

float Waveform::GetPeakLevelAt(size_t nIndex) const throw()
{
    if (nIndex >= m_arrSamples.GetCount()) return 0.0f;
    return m_arrSamples.GetAt(nIndex).flPeakLevel;
}

LONGLONG Waveform::GetTimeAt(size_t nIndex) const throw()
{
    if (nIndex >= m_arrSamples.GetCount()) return 0;
    return m_arrSamples.GetAt(nIndex).llTimeHns;
}

const WaveformSample* Waveform::GetSampleData() const throw()
{
    if (m_arrSamples.GetCount() == 0) return nullptr;
    return m_arrSamples.GetData();
}

DWORD Waveform::GetSamplesPerSecond() const throw()
{
    return m_dwSampleRate > 0 ? m_dwSampleRate : 10;
}

HRESULT Waveform::AddSample(const WaveformSample& sample)
{
    m_arrSamples.Add(sample);
    if (sample.flRmsLevel > m_flMaxRmsLevel) m_flMaxRmsLevel = sample.flRmsLevel;
    if (sample.flPeakLevel > m_flMaxPeakLevel) m_flMaxPeakLevel = sample.flPeakLevel;
    return S_OK;
}

HRESULT Waveform::AddSamples(const WaveformSample* pSamples, size_t cSamples)
{
    if (!pSamples) return E_INVALIDARG;
    for (size_t i = 0; i < cSamples; ++i)
        AddSample(pSamples[i]);
    return S_OK;
}

void Waveform::RemoveAllSamples()
{
    m_arrSamples.RemoveAll();
    m_flMaxRmsLevel = 0.0f;
    m_flMaxPeakLevel = 0.0f;
}

float Waveform::GetMaxRmsLevel() const throw() { return m_flMaxRmsLevel; }
float Waveform::GetMaxPeakLevel() const throw() { return m_flMaxPeakLevel; }

float Waveform::GetAverageRmsLevel() const throw()
{
    if (m_arrSamples.GetCount() == 0) return 0.0f;
    float flSum = 0.0f;
    for (size_t i = 0; i < m_arrSamples.GetCount(); ++i)
        flSum += m_arrSamples.GetAt(i).flRmsLevel;
    return flSum / static_cast<float>(m_arrSamples.GetCount());
}

LONGLONG Waveform::GetStartTimeHns() const throw()
{
    if (m_arrSamples.GetCount() == 0) return 0;
    return m_arrSamples.GetAt(0).llTimeHns;
}

LONGLONG Waveform::GetEndTimeHns() const throw()
{
    if (m_arrSamples.GetCount() == 0) return 0;
    return m_arrSamples.GetAt(m_arrSamples.GetCount() - 1).llTimeHns;
}

LONGLONG Waveform::GetDurationHns() const throw()
{
    return m_llDurationHns;
}

size_t Waveform::FindSampleByTime(LONGLONG llTimeHns) const throw()
{
    if (m_arrSamples.GetCount() == 0) return 0;

    // Binary search
    size_t nLow = 0;
    size_t nHigh = m_arrSamples.GetCount() - 1;

    while (nLow < nHigh)
    {
        size_t nMid = (nLow + nHigh) / 2;
        if (m_arrSamples.GetAt(nMid).llTimeHns < llTimeHns)
            nLow = nMid + 1;
        else
            nHigh = nMid;
    }
    return nLow;
}

HRESULT Waveform::GetDisplaySamples(float* pOutput, size_t cOutputSamples,
                                     LONGLONG llStartTime, LONGLONG llEndTime) const
{
    if (!pOutput || cOutputSamples == 0)
        return E_INVALIDARG;

    if (m_arrSamples.GetCount() == 0)
    {
        for (size_t i = 0; i < cOutputSamples; ++i)
            pOutput[i] = 0.0f;
        return S_OK;
    }

    LONGLONG llRange = llEndTime - llStartTime;
    if (llRange <= 0)
    {
        for (size_t i = 0; i < cOutputSamples; ++i)
            pOutput[i] = 0.0f;
        return S_OK;
    }

    for (size_t i = 0; i < cOutputSamples; ++i)
    {
        LONGLONG llTargetTime = llStartTime + (llRange * static_cast<LONGLONG>(i)) / static_cast<LONGLONG>(cOutputSamples);
        size_t nSampleIndex = FindSampleByTime(llTargetTime);
        pOutput[i] = GetRmsLevelAt(nSampleIndex);
    }

    return S_OK;
}

DWORD Waveform::GetSampleRate() const throw() { return m_dwSampleRate; }
DWORD Waveform::GetChannels() const throw() { return m_dwChannels; }

HRESULT Waveform::CacheSection(LONGLONG llStartTimeHns, LONGLONG llEndTimeHns, DWORD dwSamplesPerSecond)
{
    UNREFERENCED_PARAMETER(llStartTimeHns);
    UNREFERENCED_PARAMETER(llEndTimeHns);
    UNREFERENCED_PARAMETER(dwSamplesPerSecond);
    return S_OK;
}

bool Waveform::IsSectionCached(LONGLONG llStartTimeHns, LONGLONG llEndTimeHns) const throw()
{
    UNREFERENCED_PARAMETER(llStartTimeHns);
    UNREFERENCED_PARAMETER(llEndTimeHns);
    return false;
}

} // namespace HMRAVSource
