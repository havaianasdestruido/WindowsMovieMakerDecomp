#include "pch.h"
/*
 * AudioAnalysis.cpp
 *
 * Implementation of AudioRMSData, WaveformCallback, and
 * DuckingTrackDataSource for audio analysis and ducking.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "AudioAnalysis.h"
#include <cmath>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

namespace HMRAVSource
{

// ============================================================================
// AudioRMSData implementation
// ============================================================================

AudioRMSData::AudioRMSData()
    : m_dwWindowSizeSamples(0)
    , m_dwSamplePosition(0)
    , m_dwSampleRate(0)
    , m_dwChannels(0)
    , m_dwWindowSizeMs(50)
    , m_flCurrentRms(0.0f)
    , m_flCurrentPeak(0.0f)
    , m_flWindowRms(0.0f)
    , m_flWindowPeak(0.0f)
    , m_flMaxRms(0.0f)
    , m_flTotalRms(0.0f)
    , m_dwSampleCount(0)
    , m_dwTimeAboveThresholdSamples(0)
    , m_fInitialized(false)
{
}

AudioRMSData::~AudioRMSData()
{
    Shutdown();
}

HRESULT AudioRMSData::Initialize(DWORD dwSampleRate, DWORD dwChannels, DWORD dwWindowSizeMs)
{
    if (m_fInitialized) return S_FALSE;
    if (dwSampleRate == 0 || dwChannels == 0) return E_INVALIDARG;

    m_dwSampleRate = dwSampleRate;
    m_dwChannels = dwChannels;
    m_dwWindowSizeMs = dwWindowSizeMs > 0 ? dwWindowSizeMs : 50;

    m_dwWindowSizeSamples = (dwSampleRate * m_dwWindowSizeMs / 1000) * dwChannels;
    m_arrWindowBuffer.SetCount(m_dwWindowSizeSamples);
    memset(m_arrWindowBuffer.GetData(), 0, m_dwWindowSizeSamples * sizeof(float));

    m_fInitialized = true;
    return S_OK;
}

HRESULT AudioRMSData::Shutdown()
{
    m_arrWindowBuffer.RemoveAll();
    m_dwWindowSizeSamples = 0;
    m_fInitialized = false;
    return S_OK;
}

bool AudioRMSData::IsInitialized() const throw() { return m_fInitialized; }

HRESULT AudioRMSData::FeedSamples(const float* pSamples, DWORD dwFrameCount)
{
    if (!m_fInitialized || !pSamples) return E_INVALIDARG;

    DWORD dwTotalSamples = dwFrameCount * m_dwChannels;

    // Find peak in this batch
    float flBatchPeak = 0.0f;
    float flBatchSum = 0.0f;

    for (DWORD i = 0; i < dwTotalSamples; ++i)
    {
        float flAbs = fabsf(pSamples[i]);
        if (flAbs > flBatchPeak) flBatchPeak = flAbs;
        flBatchSum += flAbs * flAbs;

        // Copy into window buffer
        m_arrWindowBuffer.GetAt(m_dwSamplePosition) = pSamples[i];
        m_dwSamplePosition = (m_dwSamplePosition + 1) % m_dwWindowSizeSamples;
    }

    m_flCurrentPeak = flBatchPeak;

    if (dwTotalSamples > 0)
    {
        m_flCurrentRms = sqrtf(flBatchSum / static_cast<float>(dwTotalSamples));
    }

    // Track time above threshold (use -30 dB as default threshold)
    if (m_flCurrentRms > powf(10.0f, -30.0f / 20.0f))
    {
        m_dwTimeAboveThresholdSamples += dwTotalSamples;
    }

    // Update running statistics
    if (m_flCurrentRms > m_flMaxRms)
        m_flMaxRms = m_flCurrentRms;

    m_flTotalRms += m_flCurrentRms;
    m_dwSampleCount++;

    UpdateWindowLevel();

    return S_OK;
}

HRESULT AudioRMSData::FeedSamplesInt16(const short* pSamples, DWORD dwSampleCount)
{
    if (!m_fInitialized || !pSamples) return E_INVALIDARG;

    std::vector<float> floatSamples(dwSampleCount);
    for (DWORD i = 0; i < dwSampleCount; ++i)
        floatSamples[i] = static_cast<float>(pSamples[i]) / 32767.0f;

    return FeedSamples(floatSamples.data(), dwSampleCount / m_dwChannels);
}

float AudioRMSData::GetCurrentRmsLevel() const throw() { return m_flCurrentRms; }
float AudioRMSData::GetCurrentPeakLevel() const throw() { return m_flCurrentPeak; }

float AudioRMSData::GetCurrentRmsLevelDb() const throw()
{
    if (m_flCurrentRms <= 0.0f) return -100.0f;
    return 20.0f * log10f(m_flCurrentRms);
}

float AudioRMSData::GetCurrentPeakLevelDb() const throw()
{
    if (m_flCurrentPeak <= 0.0f) return -100.0f;
    return 20.0f * log10f(m_flCurrentPeak);
}

float AudioRMSData::GetWindowRmsLevel() const throw() { return m_flWindowRms; }
float AudioRMSData::GetWindowPeakLevel() const throw() { return m_flWindowPeak; }

bool AudioRMSData::IsAboveThreshold(float flThresholdDb) const throw()
{
    return GetCurrentRmsLevelDb() > flThresholdDb;
}

DWORD AudioRMSData::GetTimeAboveThresholdMs() const throw()
{
    if (m_dwSampleRate == 0 || m_dwChannels == 0) return 0;
    DWORD dwTotalSamplesPerMs = (m_dwSampleRate * m_dwChannels) / 1000;
    if (dwTotalSamplesPerMs == 0) return 0;
    return m_dwTimeAboveThresholdSamples / dwTotalSamplesPerMs;
}

void AudioRMSData::Reset()
{
    memset(m_arrWindowBuffer.GetData(), 0, m_dwWindowSizeSamples * sizeof(float));
    m_dwSamplePosition = 0;
    m_flCurrentRms = 0.0f;
    m_flCurrentPeak = 0.0f;
    m_flWindowRms = 0.0f;
    m_flWindowPeak = 0.0f;
    m_dwTimeAboveThresholdSamples = 0;
    m_flMaxRms = 0.0f;
    m_flTotalRms = 0.0f;
    m_dwSampleCount = 0;
    m_arrWaveformData.RemoveAll();
}

DWORD AudioRMSData::GetWindowSizeMs() const throw() { return m_dwWindowSizeMs; }

void AudioRMSData::SetWindowSizeMs(DWORD dwMs) throw()
{
    m_dwWindowSizeMs = dwMs;
    m_dwWindowSizeSamples = (m_dwSampleRate * dwMs / 1000) * m_dwChannels;
    m_arrWindowBuffer.SetCount(m_dwWindowSizeSamples);
    memset(m_arrWindowBuffer.GetData(), 0, m_dwWindowSizeSamples * sizeof(float));
}

float AudioRMSData::GetMaxRmsLevel() const throw() { return m_flMaxRms; }

void AudioRMSData::SetSampleRateForThresholdTracking(DWORD dwSampleRate)
{
    m_dwSampleRate = dwSampleRate;
}

float AudioRMSData::GetAverageRmsLevel() const throw()
{
    if (m_dwSampleCount == 0) return 0.0f;
    return m_flTotalRms / static_cast<float>(m_dwSampleCount);
}

void AudioRMSData::UpdateWindowLevel()
{
    float flSum = 0.0f;
    float flPeak = 0.0f;

    for (DWORD i = 0; i < m_dwWindowSizeSamples; ++i)
    {
        float flAbs = fabsf(m_arrWindowBuffer.GetAt(i));
        if (flAbs > flPeak) flPeak = flAbs;
        flSum += flAbs * flAbs;
    }

    m_flWindowPeak = flPeak;
    if (m_dwWindowSizeSamples > 0)
        m_flWindowRms = sqrtf(flSum / static_cast<float>(m_dwWindowSizeSamples));
}

// ============================================================================
// WaveformCallback implementation
// ============================================================================

WaveformCallback::WaveformCallback()
    : m_fEnabled(true)
{
}

WaveformCallback::~WaveformCallback()
{
}

void WaveformCallback::OnWaveformUpdate(float flRmsLevel, float flPeakLevel, LONGLONG llTimestampHns)
{
    UNREFERENCED_PARAMETER(flRmsLevel);
    UNREFERENCED_PARAMETER(flPeakLevel);
    UNREFERENCED_PARAMETER(llTimestampHns);
}

void WaveformCallback::OnWaveformComplete()
{
}

void WaveformCallback::OnWaveformError(HRESULT hrError)
{
    UNREFERENCED_PARAMETER(hrError);
}

bool WaveformCallback::IsEnabled() const throw() { return m_fEnabled; }
void WaveformCallback::SetEnabled(bool fEnabled) throw() { m_fEnabled = fEnabled; }

// ============================================================================
// DuckingTrackDataSource implementation
// ============================================================================

DuckingTrackDataSource::DuckingTrackDataSource()
    : m_flThresholdDb(-30.0f)
    , m_dwAttackTimeMs(300)
    , m_dwReleaseTimeMs(500)
    , m_flMaxDuckLevel(0.5f)
    , m_flCurrentDuckLevel(0.0f)
    , m_dwSampleRate(44100)
    , m_fDuckingActive(false)
    , m_fInitialized(false)
{
}

DuckingTrackDataSource::~DuckingTrackDataSource()
{
    Shutdown();
}

HRESULT DuckingTrackDataSource::Initialize(DWORD dwSampleRate, DWORD dwChannels)
{
    if (m_fInitialized) return S_FALSE;

    m_dwSampleRate = dwSampleRate > 0 ? dwSampleRate : 44100;

    HRESULT hr = m_rmsData.Initialize(dwSampleRate, dwChannels);
    if (FAILED(hr)) return hr;

    m_fInitialized = true;
    return S_OK;
}

HRESULT DuckingTrackDataSource::Shutdown()
{
    if (!m_fInitialized) return S_FALSE;

    m_rmsData.Shutdown();
    m_fInitialized = false;
    return S_OK;
}

bool DuckingTrackDataSource::IsInitialized() const throw() { return m_fInitialized; }

HRESULT DuckingTrackDataSource::ProcessBuffer(const float* pBuffer, DWORD dwFrameCount)
{
    if (!m_fInitialized || !pBuffer) return E_INVALIDARG;

    HRESULT hr = m_rmsData.FeedSamples(pBuffer, dwFrameCount);
    if (FAILED(hr)) return hr;

    // Determine if ducking should be active
    float flLevelDb = m_rmsData.GetCurrentRmsLevelDb();
    bool fAboveThreshold = flLevelDb > m_flThresholdDb;

    DWORD dwFramesPerMs = m_dwSampleRate / 1000;
    if (dwFramesPerMs == 0) dwFramesPerMs = 44;

    if (fAboveThreshold)
    {
        // Increase duck level (attack)
        float flTargetDuck = m_flMaxDuckLevel;
        float flStepPerFrame = 1.0f / static_cast<float>(m_dwAttackTimeMs * dwFramesPerMs);
        m_flCurrentDuckLevel += flStepPerFrame * static_cast<float>(dwFrameCount);
        if (m_flCurrentDuckLevel > flTargetDuck)
            m_flCurrentDuckLevel = flTargetDuck;

        m_fDuckingActive = true;
    }
    else
    {
        // Decrease duck level (release)
        float flStepPerFrame = 1.0f / static_cast<float>(m_dwReleaseTimeMs * dwFramesPerMs);
        m_flCurrentDuckLevel -= flStepPerFrame * static_cast<float>(dwFrameCount);
        if (m_flCurrentDuckLevel < 0.0f)
        {
            m_flCurrentDuckLevel = 0.0f;
            m_fDuckingActive = false;
        }
    }

    return S_OK;
}

bool DuckingTrackDataSource::IsDuckingActive() const throw() { return m_fDuckingActive; }
float DuckingTrackDataSource::GetCurrentDuckLevel() const throw() { return m_flCurrentDuckLevel; }

float DuckingTrackDataSource::GetThresholdDb() const throw() { return m_flThresholdDb; }
void DuckingTrackDataSource::SetThresholdDb(float flDb) throw() { m_flThresholdDb = flDb; }

DWORD DuckingTrackDataSource::GetAttackTimeMs() const throw() { return m_dwAttackTimeMs; }
void DuckingTrackDataSource::SetAttackTimeMs(DWORD dwMs) throw() { m_dwAttackTimeMs = dwMs; }

DWORD DuckingTrackDataSource::GetReleaseTimeMs() const throw() { return m_dwReleaseTimeMs; }
void DuckingTrackDataSource::SetReleaseTimeMs(DWORD dwMs) throw() { m_dwReleaseTimeMs = dwMs; }

float DuckingTrackDataSource::GetMaxDuckLevel() const throw() { return m_flMaxDuckLevel; }
void DuckingTrackDataSource::SetMaxDuckLevel(float flLevel) throw()
{
    if (flLevel < 0.0f) flLevel = 0.0f;
    if (flLevel > 1.0f) flLevel = 1.0f;
    m_flMaxDuckLevel = flLevel;
}

AudioRMSData* DuckingTrackDataSource::GetRmsData() { return &m_rmsData; }
const AudioRMSData* DuckingTrackDataSource::GetRmsData() const { return &m_rmsData; }

// ============================================================================
// File-based audio analysis
// ============================================================================

HRESULT AudioRMSData::AnalyzeFile(LPCWSTR pszFilePath)
{
    if (!pszFilePath || !m_fInitialized) return E_INVALIDARG;

    m_arrWaveformData.RemoveAll();
    m_flMaxRms = 0.0f;
    m_flTotalRms = 0.0f;
    m_dwSampleCount = 0;

    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, nullptr, &spReader);
    if (FAILED(hr)) return hr;

    CComPtr<IMFMediaType> spAudioType;
    hr = spReader->GetNativeMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        0, &spAudioType);
    if (FAILED(hr)) return hr;

    UINT32 samplesPerSec = 0;
    spAudioType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSec);
    UINT32 channels = 0;
    spAudioType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);

    if (samplesPerSec == 0) samplesPerSec = m_dwSampleRate;
    if (channels == 0) channels = m_dwChannels;
    if (samplesPerSec == 0 || channels == 0) return E_FAIL;

    DWORD samplesPerWindow = samplesPerSec * m_dwWindowSizeMs / 1000;
    if (samplesPerWindow == 0) samplesPerWindow = 1;

    std::vector<float> windowAccumulator(samplesPerWindow * channels, 0.0f);
    DWORD windowSampleIndex = 0;

    while (true)
    {
        CComPtr<IMFSample> spSample;
        DWORD dwStreamFlags = 0;
        hr = spReader->ReadSample(
            static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM),
            0, nullptr, &dwStreamFlags, nullptr, &spSample);

        if (FAILED(hr)) break;
        if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) break;
        if (!spSample) continue;

        CComPtr<IMFMediaBuffer> spBuffer;
        hr = spSample->ConvertToContiguousBuffer(&spBuffer);
        if (FAILED(hr)) continue;

        BYTE* pData = nullptr;
        DWORD cbData = 0;
        hr = spBuffer->Lock(&pData, nullptr, &cbData);
        if (FAILED(hr)) continue;

        DWORD dwSampleCount = cbData / (channels * sizeof(float));
        const float* pFloatData = reinterpret_cast<const float*>(pData);

        for (DWORD s = 0; s < dwSampleCount; ++s)
        {
            for (DWORD c = 0; c < channels; ++c)
            {
                windowAccumulator[windowSampleIndex * channels + c] += pFloatData[s * channels + c];
            }
            windowSampleIndex++;

            if (windowSampleIndex >= samplesPerWindow)
            {
                float flSum = 0.0f;
                float flPeak = 0.0f;
                DWORD totalWindowSamples = samplesPerWindow * channels;

                for (DWORD i = 0; i < totalWindowSamples; ++i)
                {
                    float flAbs = fabsf(windowAccumulator[i]);
                    if (flAbs > flPeak) flPeak = flAbs;
                    flSum += flAbs * flAbs;
                }

                float flRms = (totalWindowSamples > 0)
                    ? sqrtf(flSum / static_cast<float>(totalWindowSamples))
                    : 0.0f;

                m_arrWaveformData.Add(flRms);

                if (flRms > m_flMaxRms) m_flMaxRms = flRms;
                m_flTotalRms += flRms;
                m_dwSampleCount++;

                memset(windowAccumulator.data(), 0, totalWindowSamples * sizeof(float));
                windowSampleIndex = 0;
            }
        }

        spBuffer->Unlock();
    }

    return S_OK;
}

float AudioRMSData::GetPeakLevel() const throw()
{
    return m_flCurrentPeak;
}

float AudioRMSData::GetRMSLevel() const throw()
{
    return m_flCurrentRms;
}

const ATL::CAtlArray<float>& AudioRMSData::GetWaveformData() const throw()
{
    return m_arrWaveformData;
}

} // namespace HMRAVSource
