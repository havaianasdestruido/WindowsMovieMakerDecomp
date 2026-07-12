/*
 * AudioResampler.h
 *
 * Audio resampling helper. Wraps the Media Foundation audio resampler
 * for sample rate and channel count conversion.
 *
 * RTTI classes:
 *   ?AVAudioResamplerHelper@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AUDIORESAMPLER_H
#define AUDIORESAMPLER_H

#include "../HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// AudioResamplerHelper
// ============================================================================
// Wraps the Media Foundation audio resampler MFT for converting between
// sample rates and channel configurations. Used for normalizing audio
// before mixing or output.
//
class AVSOURCE_API AudioResamplerHelper
{
public:
    AudioResamplerHelper();
    ~AudioResamplerHelper();

    // Lifecycle
    HRESULT Initialize(DWORD dwInputSampleRate, DWORD dwInputChannels,
                       DWORD dwOutputSampleRate, DWORD dwOutputChannels);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Processing
    HRESULT ProcessInput(const BYTE* pData, DWORD cbData, LONGLONG llTimestampHns);
    HRESULT ProcessOutput(BYTE* pData, DWORD cbMaxData, DWORD* pcbWritten, LONGLONG* pllTimestampHns);
    HRESULT Flush();

    // Format queries
    DWORD GetInputSampleRate() const throw();
    DWORD GetInputChannels() const throw();
    DWORD GetOutputSampleRate() const throw();
    DWORD GetOutputChannels() const throw();

    // Latency
    DWORD GetLatencyFrames() const throw();

    // Reset
    HRESULT Reset();

private:
    HRESULT CreateResamplerMft();
    HRESULT ConfigureMediaType();

    CComPtr<IMFTransform>       m_spResampler;
    CComPtr<IMFMediaEvent>      m_spEvent;

    DWORD   m_dwInputSampleRate;
    DWORD   m_dwInputChannels;
    DWORD   m_dwOutputSampleRate;
    DWORD   m_dwOutputChannels;
    DWORD   m_dwLatencyFrames;
    bool    m_fInitialized;
};

} // namespace HMRAVSource

#endif // AUDIORESAMPLER_H
