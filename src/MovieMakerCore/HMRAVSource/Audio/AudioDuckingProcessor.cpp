#include "pch.h"
#include "AudioDuckingProcessor.h"
#include <cmath>

namespace HMRAVSource
{

AudioDuckingProcessor::AudioDuckingProcessor()
    : m_dwSampleRate(0)
    , m_dwChannels(0)
    , m_flThresholdDb(-30.0f)
    , m_flDuckLevel(0.5f)
    , m_dwAttackTimeMs(300)
    , m_dwReleaseTimeMs(500)
    , m_dwFadeInMs(300)
    , m_dwFadeOutMs(500)
    , m_flCurrentDuckLevel(0.0f)
    , m_flTargetDuckLevel(0.0f)
    , m_fDuckingActive(false)
    , m_fInitialized(false)
    , m_pDuckingSource(nullptr)
{
}

AudioDuckingProcessor::~AudioDuckingProcessor()
{
    Shutdown();
}

HRESULT AudioDuckingProcessor::Initialize(DWORD dwSampleRate, DWORD dwChannels)
{
    if (m_fInitialized) return S_FALSE;
    if (dwSampleRate == 0 || dwChannels == 0) return E_INVALIDARG;

    m_dwSampleRate = dwSampleRate;
    m_dwChannels = dwChannels;
    m_fInitialized = true;
    return S_OK;
}

HRESULT AudioDuckingProcessor::Shutdown()
{
    m_fInitialized = false;
    m_pDuckingSource = nullptr;
    m_flCurrentDuckLevel = 0.0f;
    m_flTargetDuckLevel = 0.0f;
    m_fDuckingActive = false;
    return S_OK;
}

bool AudioDuckingProcessor::IsInitialized() const throw() { return m_fInitialized; }

void AudioDuckingProcessor::SetDuckingSource(DuckingTrackDataSource* pSource) throw()
{
    m_pDuckingSource = pSource;
}

DuckingTrackDataSource* AudioDuckingProcessor::GetDuckingSource() const throw()
{
    return m_pDuckingSource;
}

void AudioDuckingProcessor::SetThresholdDb(float flDb) throw() { m_flThresholdDb = flDb; }
float AudioDuckingProcessor::GetThresholdDb() const throw() { return m_flThresholdDb; }

void AudioDuckingProcessor::SetDuckLevel(float flLevel) throw()
{
    if (flLevel < 0.0f) flLevel = 0.0f;
    if (flLevel > 1.0f) flLevel = 1.0f;
    m_flDuckLevel = flLevel;
}

float AudioDuckingProcessor::GetDuckLevel() const throw() { return m_flDuckLevel; }

void AudioDuckingProcessor::SetAttackTimeMs(DWORD dwMs) throw() { m_dwAttackTimeMs = dwMs; }
DWORD AudioDuckingProcessor::GetAttackTimeMs() const throw() { return m_dwAttackTimeMs; }

void AudioDuckingProcessor::SetReleaseTimeMs(DWORD dwMs) throw() { m_dwReleaseTimeMs = dwMs; }
DWORD AudioDuckingProcessor::GetReleaseTimeMs() const throw() { return m_dwReleaseTimeMs; }

void AudioDuckingProcessor::SetFadeInMs(DWORD dwMs) throw() { m_dwFadeInMs = dwMs; }
DWORD AudioDuckingProcessor::GetFadeInMs() const throw() { return m_dwFadeInMs; }

void AudioDuckingProcessor::SetFadeOutMs(DWORD dwMs) throw() { m_dwFadeOutMs = dwMs; }
DWORD AudioDuckingProcessor::GetFadeOutMs() const throw() { return m_dwFadeOutMs; }

bool AudioDuckingProcessor::IsDuckingActive() const throw() { return m_fDuckingActive; }

float AudioDuckingProcessor::GetCurrentGainReduction() const throw()
{
    return m_flCurrentDuckLevel;
}

void AudioDuckingProcessor::Reset() throw()
{
    m_flCurrentDuckLevel = 0.0f;
    m_flTargetDuckLevel = 0.0f;
    m_fDuckingActive = false;
}

// ============================================================================
// ComputeGainReduction - converts duck level [0..1] to linear gain [1..0]
// ============================================================================
float AudioDuckingProcessor::ComputeGainReduction() const throw()
{
    // duckLevel 0.0 = full volume, duckLevel 1.0 = silence
    float flGain = 1.0f - (m_flCurrentDuckLevel * m_flDuckLevel);
    if (flGain < 0.0f) flGain = 0.0f;
    if (flGain > 1.0f) flGain = 1.0f;
    return flGain;
}

// ============================================================================
// ComputeSmoothedDuck - smoothly ramps duck level toward target
// ============================================================================
float AudioDuckingProcessor::ComputeSmoothedDuck(float flTargetDuck, DWORD dwFrameCount) throw()
{
    float flStepPerFrame;
    DWORD dwFramesPerMs = m_dwSampleRate / 1000;
    if (dwFramesPerMs == 0) dwFramesPerMs = 44;

    if (flTargetDuck > m_flCurrentDuckLevel)
    {
        // Attack (increasing duck)
        DWORD dwRampFrames = m_dwFadeInMs * dwFramesPerMs;
        if (dwRampFrames == 0) dwRampFrames = 1;
        flStepPerFrame = 1.0f / static_cast<float>(dwRampFrames);
    }
    else
    {
        // Release (decreasing duck)
        DWORD dwRampFrames = m_dwFadeOutMs * dwFramesPerMs;
        if (dwRampFrames == 0) dwRampFrames = 1;
        flStepPerFrame = -1.0f / static_cast<float>(dwRampFrames);
    }

    float flNewLevel = m_flCurrentDuckLevel + flStepPerFrame * static_cast<float>(dwFrameCount);

    if (flTargetDuck > m_flCurrentDuckLevel && flNewLevel > flTargetDuck)
        flNewLevel = flTargetDuck;
    else if (flTargetDuck < m_flCurrentDuckLevel && flNewLevel < flTargetDuck)
        flNewLevel = flTargetDuck;

    if (flNewLevel < 0.0f) flNewLevel = 0.0f;
    if (flNewLevel > 1.0f) flNewLevel = 1.0f;

    return flNewLevel;
}

// ============================================================================
// UpdateNarrationLevel
// ============================================================================
HRESULT AudioDuckingProcessor::UpdateNarrationLevel(const float* pNarrationSamples,
                                                     DWORD dwFrameCount)
{
    if (!m_fInitialized) return E_FAIL;

    if (m_pDuckingSource)
    {
        HRESULT hr = m_pDuckingSource->ProcessBuffer(pNarrationSamples, dwFrameCount);
        if (FAILED(hr)) return hr;

        if (m_pDuckingSource->IsDuckingActive())
        {
            m_flTargetDuckLevel = m_pDuckingSource->GetCurrentDuckLevel();
            m_fDuckingActive = true;
        }
        else
        {
            m_flTargetDuckLevel = 0.0f;
        }
    }
    else
    {
        // Fallback: determine ducking from threshold check on the input itself
        float flPeak = 0.0f;
        DWORD dwTotalSamples = dwFrameCount * m_dwChannels;
        for (DWORD i = 0; i < dwTotalSamples; ++i)
        {
            float flAbs = fabsf(pNarrationSamples[i]);
            if (flAbs > flPeak) flPeak = flAbs;
        }

        float flLevelDb = (flPeak > 0.0f) ? 20.0f * log10f(flPeak) : -100.0f;

        if (flLevelDb > m_flThresholdDb)
        {
            m_flTargetDuckLevel = 1.0f;
            m_fDuckingActive = true;
        }
        else
        {
            m_flTargetDuckLevel = 0.0f;
        }
    }

    return S_OK;
}

// ============================================================================
// ProcessBufferFloat
// ============================================================================
HRESULT AudioDuckingProcessor::ProcessBufferFloat(float* pBuffer, DWORD dwFrameCount,
                                                   LONGLONG llPositionHns)
{
    UNREFERENCED_PARAMETER(llPositionHns);

    if (!m_fInitialized || !pBuffer) return E_INVALIDARG;
    if (dwFrameCount == 0) return S_OK;

    m_flCurrentDuckLevel = ComputeSmoothedDuck(m_flTargetDuckLevel, dwFrameCount);

    float flGain = ComputeGainReduction();

    if (flGain >= 1.0f)
        return S_OK;

    DWORD dwTotalSamples = dwFrameCount * m_dwChannels;
    for (DWORD i = 0; i < dwTotalSamples; ++i)
        pBuffer[i] *= flGain;

    if (m_flCurrentDuckLevel < 0.001f && m_flTargetDuckLevel < 0.001f)
        m_fDuckingActive = false;

    return S_OK;
}

// ============================================================================
// ProcessBufferInt16
// ============================================================================
HRESULT AudioDuckingProcessor::ProcessBufferInt16(short* pBuffer, DWORD dwFrameCount,
                                                   LONGLONG llPositionHns)
{
    UNREFERENCED_PARAMETER(llPositionHns);

    if (!m_fInitialized || !pBuffer) return E_INVALIDARG;
    if (dwFrameCount == 0) return S_OK;

    m_flCurrentDuckLevel = ComputeSmoothedDuck(m_flTargetDuckLevel, dwFrameCount);

    float flGain = ComputeGainReduction();

    if (flGain >= 1.0f)
        return S_OK;

    DWORD dwTotalSamples = dwFrameCount * m_dwChannels;
    for (DWORD i = 0; i < dwTotalSamples; ++i)
        pBuffer[i] = static_cast<short>(pBuffer[i] * flGain);

    if (m_flCurrentDuckLevel < 0.001f && m_flTargetDuckLevel < 0.001f)
        m_fDuckingActive = false;

    return S_OK;
}

} // namespace HMRAVSource
