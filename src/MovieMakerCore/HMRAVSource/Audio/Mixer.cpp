#include "pch.h"
/*
 * Mixer.cpp
 *
 * Implementation of the Mixer and MixerBuffer classes for multi-track
 * audio mixing.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "Mixer.h"
#include <cmath>

namespace HMRAVSource
{

// ============================================================================
// MixerBuffer implementation
// ============================================================================

MixerBuffer::MixerBuffer()
    : m_dwSampleRate(0)
    , m_dwChannels(0)
    , m_dwMaxFrames(0)
    , m_dwFrameCount(0)
{
}

MixerBuffer::~MixerBuffer()
{
    Shutdown();
}

HRESULT MixerBuffer::Initialize(DWORD dwSampleRate, DWORD dwChannels, DWORD dwMaxFrames)
{
    m_dwSampleRate = dwSampleRate;
    m_dwChannels = dwChannels;
    m_dwMaxFrames = dwMaxFrames;
    m_dwFrameCount = 0;

    size_t totalSamples = static_cast<size_t>(dwMaxFrames) * dwChannels;
    m_arrAccumulator.SetCount(totalSamples);
    memset(m_arrAccumulator.GetData(), 0, totalSamples * sizeof(float));
    return S_OK;
}

HRESULT MixerBuffer::Shutdown()
{
    m_arrAccumulator.RemoveAll();
    m_dwMaxFrames = 0;
    m_dwFrameCount = 0;
    return S_OK;
}

HRESULT MixerBuffer::Clear()
{
    memset(m_arrAccumulator.GetData(), 0, m_arrAccumulator.GetCount() * sizeof(float));
    m_dwFrameCount = 0;
    return S_OK;
}

HRESULT MixerBuffer::AddSamples(const float* pSamples, DWORD dwFrameCount, float flVolume)
{
    if (!pSamples) return E_INVALIDARG;

    DWORD framesToMix = std::min(dwFrameCount, m_dwMaxFrames - m_dwFrameCount);
    float* pDest = m_arrAccumulator.GetData() + (static_cast<size_t>(m_dwFrameCount) * m_dwChannels);

    for (DWORD f = 0; f < framesToMix; ++f)
    {
        for (DWORD c = 0; c < m_dwChannels; ++c)
        {
            DWORD idx = f * m_dwChannels + c;
            pDest[idx] += pSamples[idx] * flVolume;
        }
    }

    m_dwFrameCount += framesToMix;
    return S_OK;
}

HRESULT MixerBuffer::AddSamplesWithPan(const float* pSamples, DWORD dwFrameCount, float flVolume, float flPan)
{
    if (!pSamples) return E_INVALIDARG;

    DWORD framesToMix = std::min(dwFrameCount, m_dwMaxFrames - m_dwFrameCount);
    float* pDest = m_arrAccumulator.GetData() + (static_cast<size_t>(m_dwFrameCount) * m_dwChannels);

    // Pan law: equal power panning
    float flLeftGain = flVolume * cosf((flPan + 1.0f) * 0.25f * 3.14159265f);
    float flRightGain = flVolume * sinf((flPan + 1.0f) * 0.25f * 3.14159265f);

    for (DWORD f = 0; f < framesToMix; ++f)
    {
        if (m_dwChannels >= 2)
        {
            DWORD idx = f * m_dwChannels;
            pDest[idx]     += pSamples[idx]     * flLeftGain;
            pDest[idx + 1] += pSamples[idx + 1] * flRightGain;
        }
        else
        {
            pDest[f] += pSamples[f] * flVolume;
        }
    }

    m_dwFrameCount += framesToMix;
    return S_OK;
}

HRESULT MixerBuffer::GetResultAsFloat(float* pOutput, DWORD dwFrameCount) const
{
    if (!pOutput) return E_INVALIDARG;

    DWORD framesToCopy = std::min(dwFrameCount, m_dwFrameCount);
    size_t cbCopy = static_cast<size_t>(framesToCopy) * m_dwChannels * sizeof(float);
    memcpy(pOutput, m_arrAccumulator.GetData(), cbCopy);

    if (framesToCopy < dwFrameCount)
        memset(pOutput + framesToCopy * m_dwChannels, 0,
               (dwFrameCount - framesToCopy) * m_dwChannels * sizeof(float));

    return S_OK;
}

HRESULT MixerBuffer::GetResultAsInt16(short* pOutput, DWORD dwFrameCount) const
{
    if (!pOutput) return E_INVALIDARG;

    DWORD framesToCopy = std::min(dwFrameCount, m_dwFrameCount);
    const float* pSrc = m_arrAccumulator.GetData();

    for (DWORD i = 0; i < framesToCopy * m_dwChannels; ++i)
    {
        float flSample = pSrc[i];
        if (flSample > 1.0f) flSample = 1.0f;
        if (flSample < -1.0f) flSample = -1.0f;
        pOutput[i] = static_cast<short>(flSample * 32767.0f);
    }

    return S_OK;
}

HRESULT MixerBuffer::GetResultAsInt32(int* pOutput, DWORD dwFrameCount) const
{
    if (!pOutput) return E_INVALIDARG;

    DWORD framesToCopy = std::min(dwFrameCount, m_dwFrameCount);
    const float* pSrc = m_arrAccumulator.GetData();

    for (DWORD i = 0; i < framesToCopy * m_dwChannels; ++i)
    {
        float flSample = pSrc[i];
        if (flSample > 1.0f) flSample = 1.0f;
        if (flSample < -1.0f) flSample = -1.0f;
        pOutput[i] = static_cast<int>(flSample * 2147483647.0f);
    }

    return S_OK;
}

DWORD MixerBuffer::GetFrameCount() const throw() { return m_dwFrameCount; }
DWORD MixerBuffer::GetSampleRate() const throw() { return m_dwSampleRate; }
DWORD MixerBuffer::GetChannels() const throw() { return m_dwChannels; }

// ============================================================================
// Mixer implementation
// ============================================================================

Mixer::Mixer()
    : m_dwSampleRate(0)
    , m_dwChannels(0)
    , m_flMasterVolume(1.0f)
    , m_fInitialized(false)
{
}

Mixer::~Mixer()
{
    Shutdown();
}

HRESULT Mixer::Initialize(DWORD dwSampleRate, DWORD dwChannels, DWORD dwTrackCount)
{
    if (m_fInitialized) return S_FALSE;
    if (dwTrackCount == 0) return E_INVALIDARG;

    m_dwSampleRate = dwSampleRate;
    m_dwChannels = dwChannels;

    m_arrTracks.SetCount(dwTrackCount);
    for (DWORD i = 0; i < dwTrackCount; ++i)
        m_arrTracks.GetAt(i).dwTrackIndex = i;

    HRESULT hr = m_mixBuffer.Initialize(dwSampleRate, dwChannels, dwSampleRate * 10); // 10 second buffer
    if (FAILED(hr)) return hr;

    m_fInitialized = true;
    return S_OK;
}

HRESULT Mixer::Shutdown()
{
    if (!m_fInitialized) return S_FALSE;

    m_mixBuffer.Shutdown();
    m_arrTracks.RemoveAll();
    m_fInitialized = false;
    return S_OK;
}

bool Mixer::IsInitialized() const throw() { return m_fInitialized; }

HRESULT Mixer::SetTrackVolume(DWORD dwTrack, float flVolume)
{
    if (dwTrack >= m_arrTracks.GetCount()) return E_INVALIDARG;
    if (flVolume < 0.0f) flVolume = 0.0f;
    if (flVolume > 2.0f) flVolume = 2.0f;
    m_arrTracks.GetAt(dwTrack).flVolume = flVolume;
    return S_OK;
}

float Mixer::GetTrackVolume(DWORD dwTrack) const throw()
{
    if (dwTrack >= m_arrTracks.GetCount()) return 0.0f;
    return m_arrTracks.GetAt(dwTrack).flVolume;
}

HRESULT Mixer::SetTrackPan(DWORD dwTrack, float flPan)
{
    if (dwTrack >= m_arrTracks.GetCount()) return E_INVALIDARG;
    if (flPan < -1.0f) flPan = -1.0f;
    if (flPan > 1.0f) flPan = 1.0f;
    m_arrTracks.GetAt(dwTrack).flPan = flPan;
    return S_OK;
}

float Mixer::GetTrackPan(DWORD dwTrack) const throw()
{
    if (dwTrack >= m_arrTracks.GetCount()) return 0.0f;
    return m_arrTracks.GetAt(dwTrack).flPan;
}

HRESULT Mixer::SetTrackMute(DWORD dwTrack, bool fMute)
{
    if (dwTrack >= m_arrTracks.GetCount()) return E_INVALIDARG;
    m_arrTracks.GetAt(dwTrack).fMuted = fMute;
    return S_OK;
}

bool Mixer::IsTrackMuted(DWORD dwTrack) const throw()
{
    if (dwTrack >= m_arrTracks.GetCount()) return false;
    return m_arrTracks.GetAt(dwTrack).fMuted;
}

HRESULT Mixer::SetTrackSolo(DWORD dwTrack, bool fSolo)
{
    if (dwTrack >= m_arrTracks.GetCount()) return E_INVALIDARG;
    m_arrTracks.GetAt(dwTrack).fSolo = fSolo;
    return S_OK;
}

bool Mixer::IsTrackSolo(DWORD dwTrack) const throw()
{
    if (dwTrack >= m_arrTracks.GetCount()) return false;
    return m_arrTracks.GetAt(dwTrack).fSolo;
}

HRESULT Mixer::SetTrackGain(DWORD dwTrack, float flGainDb)
{
    if (dwTrack >= m_arrTracks.GetCount()) return E_INVALIDARG;
    m_arrTracks.GetAt(dwTrack).flGainDb = flGainDb;
    return S_OK;
}

float Mixer::GetTrackGain(DWORD dwTrack) const throw()
{
    if (dwTrack >= m_arrTracks.GetCount()) return 0.0f;
    return m_arrTracks.GetAt(dwTrack).flGainDb;
}

HRESULT Mixer::SetMasterVolume(float flVolume)
{
    if (flVolume < 0.0f) flVolume = 0.0f;
    if (flVolume > 2.0f) flVolume = 2.0f;
    m_flMasterVolume = flVolume;
    return S_OK;
}

float Mixer::GetMasterVolume() const throw() { return m_flMasterVolume; }

HRESULT Mixer::MixFrame(const float* pSamples, DWORD dwTrack, DWORD dwFrameCount)
{
    if (!m_fInitialized || !pSamples) return E_INVALIDARG;
    if (dwTrack >= m_arrTracks.GetCount()) return E_INVALIDARG;

    const MixerTrackInfo& track = m_arrTracks.GetAt(dwTrack);

    // Check solo/mute
    bool fAnySolo = false;
    for (size_t i = 0; i < m_arrTracks.GetCount(); ++i)
    {
        if (m_arrTracks.GetAt(i).fSolo) { fAnySolo = true; break; }
    }

    if (fAnySolo && !track.fSolo) return S_OK;
    if (track.fMuted) return S_OK;

    float flVolume = track.flVolume * m_flMasterVolume;
    float flGainLinear = powf(10.0f, track.flGainDb / 20.0f);
    flVolume *= flGainLinear;

    return m_mixBuffer.AddSamplesWithPan(pSamples, dwFrameCount, flVolume, track.flPan);
}

HRESULT Mixer::GetMixedOutput(float* pOutput, DWORD dwFrameCount)
{
    if (!m_fInitialized || !pOutput) return E_INVALIDARG;
    return m_mixBuffer.GetResultAsFloat(pOutput, dwFrameCount);
}

HRESULT Mixer::GetMixedOutputAsInt16(short* pOutput, DWORD dwFrameCount)
{
    if (!m_fInitialized || !pOutput) return E_INVALIDARG;
    return m_mixBuffer.GetResultAsInt16(pOutput, dwFrameCount);
}

HRESULT Mixer::ResetMixBuffer()
{
    return m_mixBuffer.Clear();
}

DWORD Mixer::GetTrackCount() const throw() { return static_cast<DWORD>(m_arrTracks.GetCount()); }
DWORD Mixer::GetSampleRate() const throw() { return m_dwSampleRate; }
DWORD Mixer::GetChannels() const throw() { return m_dwChannels; }

} // namespace HMRAVSource
