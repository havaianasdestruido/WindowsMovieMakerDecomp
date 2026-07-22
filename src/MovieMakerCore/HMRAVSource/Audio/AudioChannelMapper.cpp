#include "pch.h"
#include "AudioChannelMapper.h"
#include <cmath>

namespace HMRAVSource
{

AudioChannelMapper::AudioChannelMapper()
    : m_dwInputChannels(0)
    , m_dwOutputChannels(0)
    , m_dwSampleRate(0)
    , m_fInitialized(false)
{
    memset(m_arrWeights, 0, sizeof(m_arrWeights));
}

AudioChannelMapper::~AudioChannelMapper()
{
    Shutdown();
}

HRESULT AudioChannelMapper::Initialize(DWORD dwInputChannels, DWORD dwOutputChannels, DWORD dwSampleRate)
{
    if (m_fInitialized) return S_FALSE;
    if (dwInputChannels == 0 || dwOutputChannels == 0) return E_INVALIDARG;
    if (dwInputChannels > kMaxChannels || dwOutputChannels > kMaxChannels) return E_INVALIDARG;

    m_dwInputChannels = dwInputChannels;
    m_dwOutputChannels = dwOutputChannels;
    m_dwSampleRate = dwSampleRate;
    m_fInitialized = true;

    InitializeDefaultWeights();
    return S_OK;
}

HRESULT AudioChannelMapper::Shutdown()
{
    m_dwInputChannels = 0;
    m_dwOutputChannels = 0;
    m_fInitialized = false;
    memset(m_arrWeights, 0, sizeof(m_arrWeights));
    return S_OK;
}

bool AudioChannelMapper::IsInitialized() const throw() { return m_fInitialized; }

DWORD AudioChannelMapper::GetInputChannels() const throw() { return m_dwInputChannels; }
DWORD AudioChannelMapper::GetOutputChannels() const throw() { return m_dwOutputChannels; }
DWORD AudioChannelMapper::GetSampleRate() const throw() { return m_dwSampleRate; }

DWORD AudioChannelMapper::GetBytesPerFrame() const throw()
{
    return m_dwOutputChannels * sizeof(float);
}

// ============================================================================
// InitializeDefaultWeights
// Standard downmix/upmix weight matrices.
// ============================================================================
void AudioChannelMapper::InitializeDefaultWeights()
{
    memset(m_arrWeights, 0, sizeof(m_arrWeights));

    // Identity (same channel count): 1:1 mapping
    if (m_dwInputChannels == m_dwOutputChannels)
    {
        for (DWORD i = 0; i < m_dwInputChannels; ++i)
            m_arrWeights[i][i] = 1.0f;
        return;
    }

    // Mono -> Stereo (center to both channels)
    if (m_dwInputChannels == 1 && m_dwOutputChannels == 2)
    {
        m_arrWeights[0][0] = 1.0f;  // L = center
        m_arrWeights[1][0] = 1.0f;  // R = center
        return;
    }

    // Stereo -> Mono (equal-power downmix)
    if (m_dwInputChannels == 2 && m_dwOutputChannels == 1)
    {
        m_arrWeights[0][0] = 0.707f;  // L contribution
        m_arrWeights[0][1] = 0.707f;  // R contribution
        return;
    }

    // 5.1 -> Stereo (standard ITU-R BS.775 fold-down)
    if (m_dwInputChannels == 6 && m_dwOutputChannels == 2)
    {
        float flScale = 0.707f;
        m_arrWeights[0][0] = 1.0f;            // L from L
        m_arrWeights[0][1] = 0.0f;            // L from R
        m_arrWeights[0][2] = flScale;         // L from C (center contributes to both)
        m_arrWeights[0][3] = 0.0f;            // L from LFE
        m_arrWeights[0][4] = flScale * 0.707f; // L from Ls (rear left -> front)
        m_arrWeights[0][5] = 0.0f;            // L from Rs

        m_arrWeights[1][0] = 0.0f;            // R from L
        m_arrWeights[1][1] = 1.0f;            // R from R
        m_arrWeights[1][2] = flScale;         // R from C
        m_arrWeights[1][3] = 0.0f;            // R from LFE
        m_arrWeights[1][4] = 0.0f;            // R from Ls
        m_arrWeights[1][5] = flScale * 0.707f; // R from Rs
        return;
    }

    // Mono -> 5.1 (center channel fill)
    if (m_dwInputChannels == 1 && m_dwOutputChannels == 6)
    {
        m_arrWeights[2][0] = 1.0f;  // C = center
        return;
    }

    // Stereo -> 5.1 (stereo upmix to 5.1)
    if (m_dwInputChannels == 2 && m_dwOutputChannels == 6)
    {
        m_arrWeights[0][0] = 1.0f;  // L from L
        m_arrWeights[0][1] = 0.0f;  // L from R
        m_arrWeights[1][0] = 0.0f;  // R from L
        m_arrWeights[1][1] = 1.0f;  // R from R
        return;
    }

    // Generic: route by index where possible, zero-fill the rest
    DWORD dwMin = m_dwInputChannels < m_dwOutputChannels ? m_dwInputChannels : m_dwOutputChannels;
    for (DWORD i = 0; i < dwMin; ++i)
        m_arrWeights[i][i] = 1.0f;
}

// ============================================================================
// SetChannelWeights
// ============================================================================
HRESULT AudioChannelMapper::SetChannelWeights(DWORD dwOutputChannel,
                                               const float* arrWeights,
                                               DWORD dwInputChannelCount)
{
    if (!m_fInitialized) return E_FAIL;
    if (!arrWeights) return E_POINTER;
    if (dwOutputChannel >= m_dwOutputChannels) return E_INVALIDARG;
    if (dwInputChannelCount != m_dwInputChannels) return E_INVALIDARG;

    for (DWORD i = 0; i < m_dwInputChannels; ++i)
        m_arrWeights[dwOutputChannel][i] = arrWeights[i];

    return S_OK;
}

// ============================================================================
// ProcessBufferFloat
// ============================================================================
HRESULT AudioChannelMapper::ProcessBufferFloat(const float* pInput, float* pOutput,
                                                DWORD dwFrameCount)
{
    if (!m_fInitialized || !pInput || !pOutput) return E_INVALIDARG;
    if (dwFrameCount == 0) return S_OK;

    for (DWORD f = 0; f < dwFrameCount; ++f)
    {
        for (DWORD o = 0; o < m_dwOutputChannels; ++o)
        {
            float flSum = 0.0f;
            for (DWORD i = 0; i < m_dwInputChannels; ++i)
            {
                flSum += pInput[f * m_dwInputChannels + i] * m_arrWeights[o][i];
            }
            pOutput[f * m_dwOutputChannels + o] = flSum;
        }
    }

    return S_OK;
}

// ============================================================================
// ProcessBufferInt16
// ============================================================================
HRESULT AudioChannelMapper::ProcessBufferInt16(const short* pInput, short* pOutput,
                                                DWORD dwFrameCount)
{
    if (!m_fInitialized || !pInput || !pOutput) return E_INVALIDARG;
    if (dwFrameCount == 0) return S_OK;

    for (DWORD f = 0; f < dwFrameCount; ++f)
    {
        for (DWORD o = 0; o < m_dwOutputChannels; ++o)
        {
            float flSum = 0.0f;
            for (DWORD i = 0; i < m_dwInputChannels; ++i)
            {
                flSum += static_cast<float>(pInput[f * m_dwInputChannels + i]) *
                         m_arrWeights[o][i];
            }
            // Clamp to int16 range
            if (flSum > 32767.0f) flSum = 32767.0f;
            if (flSum < -32768.0f) flSum = -32768.0f;
            pOutput[f * m_dwOutputChannels + o] = static_cast<short>(flSum);
        }
    }

    return S_OK;
}

// ============================================================================
// ProcessBufferFloatInPlace - requires input channels >= output channels
// ============================================================================
HRESULT AudioChannelMapper::ProcessBufferFloatInPlace(float* pBuffer, DWORD dwFrameCount)
{
    if (!m_fInitialized || !pBuffer) return E_INVALIDARG;
    if (dwFrameCount == 0) return S_OK;

    // Only safe when input channels >= output channels (downmix)
    if (m_dwInputChannels < m_dwOutputChannels) return E_UNEXPECTED;

    DWORD dwInputFrameBytes = m_dwInputChannels * sizeof(float);
    DWORD dwOutputFrameBytes = m_dwOutputChannels * sizeof(float);

    for (DWORD f = 0; f < dwFrameCount; ++f)
    {
        float arrInput[kMaxChannels] = {};
        float arrOutput[kMaxChannels] = {};

        BYTE* pFrame = reinterpret_cast<BYTE*>(pBuffer) + f * dwInputFrameBytes;
        memcpy(arrInput, pFrame, dwInputFrameBytes);

        for (DWORD o = 0; o < m_dwOutputChannels; ++o)
        {
            float flSum = 0.0f;
            for (DWORD i = 0; i < m_dwInputChannels; ++i)
                flSum += arrInput[i] * m_arrWeights[o][i];
            arrOutput[o] = flSum;
        }

        memcpy(pFrame, arrOutput, dwOutputFrameBytes);
    }

    return S_OK;
}

// ============================================================================
// ProcessBufferInt16InPlace - requires input channels >= output channels
// ============================================================================
HRESULT AudioChannelMapper::ProcessBufferInt16InPlace(short* pBuffer, DWORD dwFrameCount)
{
    if (!m_fInitialized || !pBuffer) return E_INVALIDARG;
    if (dwFrameCount == 0) return S_OK;

    if (m_dwInputChannels < m_dwOutputChannels) return E_UNEXPECTED;

    DWORD dwInputFrameSamples = m_dwInputChannels;
    DWORD dwOutputFrameSamples = m_dwOutputChannels;

    for (DWORD f = 0; f < dwFrameCount; ++f)
    {
        short arrInput[kMaxChannels] = {};
        short arrOutput[kMaxChannels] = {};

        memcpy(arrInput, pBuffer + f * dwInputFrameSamples,
               dwInputFrameSamples * sizeof(short));

        for (DWORD o = 0; o < m_dwOutputChannels; ++o)
        {
            float flSum = 0.0f;
            for (DWORD i = 0; i < m_dwInputChannels; ++i)
                flSum += static_cast<float>(arrInput[i]) * m_arrWeights[o][i];

            if (flSum > 32767.0f) flSum = 32767.0f;
            if (flSum < -32768.0f) flSum = -32768.0f;
            arrOutput[o] = static_cast<short>(flSum);
        }

        memcpy(pBuffer + f * dwInputFrameSamples, arrOutput,
               dwOutputFrameSamples * sizeof(short));
    }

    return S_OK;
}

} // namespace HMRAVSource
