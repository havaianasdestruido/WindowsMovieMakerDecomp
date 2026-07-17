#include "pch.h"
/*
 * AudioBoost.cpp
 *
 * Implementation of the AudioBoost class for volume boost and
 * normalization.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "AudioBoost.h"
#include <cmath>

namespace HMRAVSource
{

AudioBoost::AudioBoost()
    : m_mode(AudioBoostModeNone)
    , m_flFixedGainDb(6.0f)
    , m_flTargetPeakLevel(0.95f)
    , m_flTargetLoudnessLu(-14.0f)
    , m_flCompressionThresholdDb(-20.0f)
    , m_flCompressionRatio(4.0f)
    , m_dwAttackMs(10)
    , m_dwReleaseMs(100)
    , m_flMeasuredPeak(0.0f)
    , m_flMeasuredRms(0.0f)
    , m_flMeasuredLoudness(0.0f)
    , m_flAppliedGainDb(0.0f)
    , m_dwSampleRate(0)
    , m_dwChannels(0)
    , m_fInitialized(false)
    , m_fNormalize(false)
{
}

AudioBoost::~AudioBoost()
{
    Shutdown();
}

HRESULT AudioBoost::Initialize(DWORD dwSampleRate, DWORD dwChannels)
{
    if (m_fInitialized) return S_FALSE;
    if (dwSampleRate == 0 || dwChannels == 0) return E_INVALIDARG;

    m_dwSampleRate = dwSampleRate;
    m_dwChannels = dwChannels;
    m_fInitialized = true;
    return S_OK;
}

HRESULT AudioBoost::Shutdown()
{
    m_fInitialized = false;
    return S_OK;
}

bool AudioBoost::IsInitialized() const throw() { return m_fInitialized; }

AudioBoostMode AudioBoost::GetMode() const throw() { return m_mode; }
void AudioBoost::SetMode(AudioBoostMode mode) throw() { m_mode = mode; }

float AudioBoost::GetFixedGainDb() const throw() { return m_flFixedGainDb; }
void AudioBoost::SetFixedGainDb(float flDb) throw() { m_flFixedGainDb = flDb; }

float AudioBoost::GetTargetPeakLevel() const throw() { return m_flTargetPeakLevel; }
void AudioBoost::SetTargetPeakLevel(float flLevel) throw()
{
    if (flLevel < 0.0f) flLevel = 0.0f;
    if (flLevel > 1.0f) flLevel = 1.0f;
    m_flTargetPeakLevel = flLevel;
}

float AudioBoost::GetTargetLoudnessLu() const throw() { return m_flTargetLoudnessLu; }
void AudioBoost::SetTargetLoudnessLu(float flLu) throw() { m_flTargetLoudnessLu = flLu; }

float AudioBoost::GetCompressionThresholdDb() const throw() { return m_flCompressionThresholdDb; }
void AudioBoost::SetCompressionThresholdDb(float flDb) throw() { m_flCompressionThresholdDb = flDb; }

float AudioBoost::GetCompressionRatio() const throw() { return m_flCompressionRatio; }
void AudioBoost::SetCompressionRatio(float flRatio) throw()
{
    if (flRatio < 1.0f) flRatio = 1.0f;
    m_flCompressionRatio = flRatio;
}

DWORD AudioBoost::GetAttackMs() const throw() { return m_dwAttackMs; }
void AudioBoost::SetAttackMs(DWORD dwMs) throw() { m_dwAttackMs = dwMs; }

DWORD AudioBoost::GetReleaseMs() const throw() { return m_dwReleaseMs; }
void AudioBoost::SetReleaseMs(DWORD dwMs) throw() { m_dwReleaseMs = dwMs; }

HRESULT AudioBoost::ProcessBuffer(float* pBuffer, DWORD dwFrameCount)
{
    if (!m_fInitialized || !pBuffer)
        return E_INVALIDARG;

    if (m_mode == AudioBoostModeNone)
        return S_OK;

    // Measure current peak and RMS
    float flPeak = 0.0f;
    float flSum = 0.0f;
    DWORD dwTotalSamples = dwFrameCount * m_dwChannels;
    for (DWORD i = 0; i < dwTotalSamples; ++i)
    {
        float flAbs = fabsf(pBuffer[i]);
        if (flAbs > flPeak) flPeak = flAbs;
        flSum += flAbs * flAbs;
    }
    m_flMeasuredPeak = flPeak;
    m_flMeasuredRms = (dwTotalSamples > 0) ? sqrtf(flSum / static_cast<float>(dwTotalSamples)) : 0.0f;
    m_flMeasuredLoudness = (m_flMeasuredRms > 0.0f) ? 20.0f * log10f(m_flMeasuredRms) : -100.0f;

    // Compute and apply gain based on mode
    float flLinearGain = ComputeLinearGain(flPeak);

    if (m_mode == AudioBoostModeCompress)
    {
        // Simple soft-knee compression
        float flThresholdLinear = powf(10.0f, m_flCompressionThresholdDb / 20.0f);
        for (DWORD i = 0; i < dwTotalSamples; ++i)
        {
            float flAbs = fabsf(pBuffer[i]);
            if (flAbs > flThresholdLinear)
            {
                float flOver = flAbs / flThresholdLinear;
                float flCompressed = powf(flOver, 1.0f / m_flCompressionRatio);
                float flGain = (flThresholdLinear * flCompressed) / flAbs;
                pBuffer[i] *= flGain;
            }
            else
            {
                pBuffer[i] *= flLinearGain;
            }
        }
    }
    else
    {
        ApplyGain(pBuffer, dwFrameCount, flLinearGain);
    }

    m_flAppliedGainDb = 20.0f * log10f(flLinearGain);
    return S_OK;
}

HRESULT AudioBoost::ProcessBufferInt16(short* pBuffer, DWORD dwSampleCount)
{
    if (!m_fInitialized || !pBuffer)
        return E_INVALIDARG;

    // Convert to float, process, convert back
    DWORD dwFrameCount = dwSampleCount / m_dwChannels;
    std::vector<float> floatBuf(dwSampleCount);

    for (DWORD i = 0; i < dwSampleCount; ++i)
        floatBuf[i] = static_cast<float>(pBuffer[i]) / 32767.0f;

    HRESULT hr = ProcessBuffer(floatBuf.data(), dwFrameCount);
    if (FAILED(hr)) return hr;

    for (DWORD i = 0; i < dwSampleCount; ++i)
    {
        float flSample = floatBuf[i];
        if (flSample > 1.0f) flSample = 1.0f;
        if (flSample < -1.0f) flSample = -1.0f;
        pBuffer[i] = static_cast<short>(flSample * 32767.0f);
    }

    return S_OK;
}

float AudioBoost::GetMeasuredPeakLevel() const throw() { return m_flMeasuredPeak; }
float AudioBoost::GetMeasuredRmsLevel() const throw() { return m_flMeasuredRms; }
float AudioBoost::GetMeasuredLoudness() const throw() { return m_flMeasuredLoudness; }
float AudioBoost::GetAppliedGainDb() const throw() { return m_flAppliedGainDb; }

void AudioBoost::SetBoostLevel(float flLevel)
{
    if (flLevel < 0.0f) flLevel = 0.0f;
    if (flLevel > 2.0f) flLevel = 2.0f;
    m_flFixedGainDb = 20.0f * log10f(flLevel > 0.0f ? flLevel : 0.0001f);
    m_mode = AudioBoostModeFixedGain;
}

float AudioBoost::ProcessSample(float flSample)
{
    if (m_mode == AudioBoostModeNone)
        return flSample;

    float flLinearGain = ComputeLinearGain(fabsf(flSample));
    float flResult = flSample * flLinearGain;

    if (m_mode == AudioBoostModeCompress)
    {
        float flThresholdLinear = powf(10.0f, m_flCompressionThresholdDb / 20.0f);
        float flAbs = fabsf(flSample);
        if (flAbs > flThresholdLinear)
        {
            float flOver = flAbs / flThresholdLinear;
            float flCompressed = powf(flOver, 1.0f / m_flCompressionRatio);
            float flGain = (flThresholdLinear * flCompressed) / (flAbs > 0.0f ? flAbs : 1.0f);
            flResult = flSample * flGain;
        }
    }

    if (flResult > 1.0f) flResult = 1.0f;
    if (flResult < -1.0f) flResult = -1.0f;
    return flResult;
}

void AudioBoost::SetNormalize(bool fEnabled)
{
    m_fNormalize = fEnabled;
    if (fEnabled && m_mode == AudioBoostModeNone)
        m_mode = AudioBoostModePeakNormalize;
    else if (!fEnabled && m_mode == AudioBoostModePeakNormalize)
        m_mode = AudioBoostModeNone;
}

void AudioBoost::ResetAnalysis()
{
    m_flMeasuredPeak = 0.0f;
    m_flMeasuredRms = 0.0f;
    m_flMeasuredLoudness = 0.0f;
    m_flAppliedGainDb = 0.0f;
}

float AudioBoost::ComputeLinearGain(float flInputPeak)
{
    if (flInputPeak <= 0.0f) return 1.0f;

    switch (m_mode)
    {
    case AudioBoostModeFixedGain:
        return powf(10.0f, m_flFixedGainDb / 20.0f);

    case AudioBoostModePeakNormalize:
    {
        if (flInputPeak >= m_flTargetPeakLevel) return 1.0f;
        return m_flTargetPeakLevel / flInputPeak;
    }

    case AudioBoostModeLoudnessNorm:
    {
        // Simplified loudness normalization
        float flCurrentDb = 20.0f * log10f(flInputPeak);
        float flTargetDb = m_flTargetLoudnessLu;
        float flGainDb = flTargetDb - flCurrentDb;
        return powf(10.0f, flGainDb / 20.0f);
    }

    default:
        return 1.0f;
    }
}

void AudioBoost::ApplyGain(float* pBuffer, DWORD dwFrameCount, float flLinearGain)
{
    DWORD dwTotalSamples = dwFrameCount * m_dwChannels;
    for (DWORD i = 0; i < dwTotalSamples; ++i)
        pBuffer[i] *= flLinearGain;
}

} // namespace HMRAVSource
