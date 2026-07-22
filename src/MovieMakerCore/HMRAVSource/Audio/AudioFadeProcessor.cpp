#include "pch.h"
#include "AudioFadeProcessor.h"
#include <cmath>

namespace HMRAVSource
{

AudioFadeProcessor::AudioFadeProcessor()
    : m_dwSampleRate(0)
    , m_dwChannels(0)
    , m_dwBitsPerSample(16)
    , m_llFadeInDurationHns(0)
    , m_llFadeOutDurationHns(0)
    , m_llTotalDurationHns(0)
    , m_fadeInType(AudioFadeTypeLinear)
    , m_fadeOutType(AudioFadeTypeLinear)
    , m_fInitialized(false)
{
}

AudioFadeProcessor::~AudioFadeProcessor()
{
    Shutdown();
}

HRESULT AudioFadeProcessor::Initialize(DWORD dwSampleRate, DWORD dwChannels, DWORD dwBitsPerSample)
{
    if (m_fInitialized) return S_FALSE;
    if (dwSampleRate == 0 || dwChannels == 0) return E_INVALIDARG;

    m_dwSampleRate = dwSampleRate;
    m_dwChannels = dwChannels;
    m_dwBitsPerSample = dwBitsPerSample;
    m_fInitialized = true;
    return S_OK;
}

HRESULT AudioFadeProcessor::Shutdown()
{
    m_fInitialized = false;
    m_llFadeInDurationHns = 0;
    m_llFadeOutDurationHns = 0;
    m_llTotalDurationHns = 0;
    return S_OK;
}

bool AudioFadeProcessor::IsInitialized() const throw() { return m_fInitialized; }

void AudioFadeProcessor::SetFadeInDurationHns(LONGLONG llDurationHns) throw()
{
    m_llFadeInDurationHns = llDurationHns >= 0 ? llDurationHns : 0;
}

LONGLONG AudioFadeProcessor::GetFadeInDurationHns() const throw()
{
    return m_llFadeInDurationHns;
}

void AudioFadeProcessor::SetFadeOutDurationHns(LONGLONG llDurationHns) throw()
{
    m_llFadeOutDurationHns = llDurationHns >= 0 ? llDurationHns : 0;
}

LONGLONG AudioFadeProcessor::GetFadeOutDurationHns() const throw()
{
    return m_llFadeOutDurationHns;
}

void AudioFadeProcessor::SetTotalDurationHns(LONGLONG llDurationHns) throw()
{
    m_llTotalDurationHns = llDurationHns >= 0 ? llDurationHns : 0;
}

LONGLONG AudioFadeProcessor::GetTotalDurationHns() const throw()
{
    return m_llTotalDurationHns;
}

void AudioFadeProcessor::SetFadeInType(AudioFadeType type) throw() { m_fadeInType = type; }
AudioFadeType AudioFadeProcessor::GetFadeInType() const throw() { return m_fadeInType; }

void AudioFadeProcessor::SetFadeOutType(AudioFadeType type) throw() { m_fadeOutType = type; }
AudioFadeType AudioFadeProcessor::GetFadeOutType() const throw() { return m_fadeOutType; }

void AudioFadeProcessor::Reset() throw()
{
    m_llFadeInDurationHns = 0;
    m_llFadeOutDurationHns = 0;
    m_llTotalDurationHns = 0;
}

// ============================================================================
// ComputeFadeGain - returns gain [0.0 .. 1.0] for a position within a fade
// ============================================================================
float AudioFadeProcessor::ComputeFadeGain(LONGLONG llPositionHns, bool fFadingIn,
                                           LONGLONG llFadeDurationHns) const throw()
{
    if (llFadeDurationHns <= 0 || llPositionHns < 0)
        return 1.0f;

    double dblProgress = static_cast<double>(llPositionHns) /
                         static_cast<double>(llFadeDurationHns);
    if (dblProgress > 1.0) dblProgress = 1.0;

    AudioFadeType fadeType = fFadingIn ? m_fadeInType : m_fadeOutType;
    if (!fFadingIn)
        dblProgress = 1.0 - dblProgress;

    switch (fadeType)
    {
    case AudioFadeTypeLinear:
        return static_cast<float>(dblProgress);

    case AudioFadeTypeEqualPower:
    {
        float flRad = static_cast<float>(dblProgress) * 3.14159265f * 0.5f;
        return sinf(flRad);
    }

    case AudioFadeTypeExponential:
    {
        if (dblProgress <= 0.0) return 0.0f;
        return powf(10.0f, (1.0f - static_cast<float>(dblProgress)) * -60.0f / 20.0f);
    }

    default:
        return static_cast<float>(dblProgress);
    }
}

// ============================================================================
// ComputeGain - master gain computation for a given time position
// ============================================================================
float AudioFadeProcessor::ComputeGain(LONGLONG llPositionHns) const throw()
{
    if (!m_fInitialized) return 1.0f;

    // Fade-in region
    if (m_llFadeInDurationHns > 0 && llPositionHns < m_llFadeInDurationHns)
    {
        return ComputeFadeGain(llPositionHns, true, m_llFadeInDurationHns);
    }

    // Fade-out region
    if (m_llFadeOutDurationHns > 0 && m_llTotalDurationHns > 0)
    {
        LONGLONG llFadeOutStart = m_llTotalDurationHns - m_llFadeOutDurationHns;
        if (llPositionHns >= llFadeOutStart)
        {
            LONGLONG llFadeOutPosition = llPositionHns - llFadeOutStart;
            return ComputeFadeGain(llFadeOutPosition, false, m_llFadeOutDurationHns);
        }
    }

    return 1.0f;
}

// ============================================================================
// ProcessBufferFloat
// ============================================================================
HRESULT AudioFadeProcessor::ProcessBufferFloat(float* pBuffer, DWORD dwFrameCount,
                                                LONGLONG llPositionHns)
{
    if (!m_fInitialized || !pBuffer) return E_INVALIDARG;
    if (dwFrameCount == 0) return S_OK;

    LONGLONG llHnsPerFrame = 10000000LL / static_cast<LONGLONG>(m_dwSampleRate);
    if (llHnsPerFrame <= 0) llHnsPerFrame = 1;

    LONGLONG llCurrentHns = llPositionHns;

    for (DWORD f = 0; f < dwFrameCount; ++f)
    {
        float flGain = ComputeGain(llCurrentHns);

        for (DWORD c = 0; c < m_dwChannels; ++c)
        {
            DWORD idx = f * m_dwChannels + c;
            pBuffer[idx] *= flGain;
        }

        llCurrentHns += llHnsPerFrame;
    }

    return S_OK;
}

// ============================================================================
// ProcessBufferInt16
// ============================================================================
HRESULT AudioFadeProcessor::ProcessBufferInt16(short* pBuffer, DWORD dwFrameCount,
                                                LONGLONG llPositionHns)
{
    if (!m_fInitialized || !pBuffer) return E_INVALIDARG;
    if (dwFrameCount == 0) return S_OK;

    LONGLONG llHnsPerFrame = 10000000LL / static_cast<LONGLONG>(m_dwSampleRate);
    if (llHnsPerFrame <= 0) llHnsPerFrame = 1;

    LONGLONG llCurrentHns = llPositionHns;

    for (DWORD f = 0; f < dwFrameCount; ++f)
    {
        float flGain = ComputeGain(llCurrentHns);

        for (DWORD c = 0; c < m_dwChannels; ++c)
        {
            DWORD idx = f * m_dwChannels + c;
            pBuffer[idx] = static_cast<short>(pBuffer[idx] * flGain);
        }

        llCurrentHns += llHnsPerFrame;
    }

    return S_OK;
}

// ============================================================================
// ProcessBufferInt32
// ============================================================================
HRESULT AudioFadeProcessor::ProcessBufferInt32(int* pBuffer, DWORD dwFrameCount,
                                                LONGLONG llPositionHns)
{
    if (!m_fInitialized || !pBuffer) return E_INVALIDARG;
    if (dwFrameCount == 0) return S_OK;

    LONGLONG llHnsPerFrame = 10000000LL / static_cast<LONGLONG>(m_dwSampleRate);
    if (llHnsPerFrame <= 0) llHnsPerFrame = 1;

    LONGLONG llCurrentHns = llPositionHns;

    for (DWORD f = 0; f < dwFrameCount; ++f)
    {
        float flGain = ComputeGain(llCurrentHns);

        for (DWORD c = 0; c < m_dwChannels; ++c)
        {
            DWORD idx = f * m_dwChannels + c;
            pBuffer[idx] = static_cast<int>(pBuffer[idx] * flGain);
        }

        llCurrentHns += llHnsPerFrame;
    }

    return S_OK;
}

} // namespace HMRAVSource
