/*
 * AudioChannelMapper.h
 *
 * Audio channel mapping and conversion. Provides mono/stereo
 * downmix/upmix, surround-to-stereo fold-down, and custom channel
 * remapping for audio processing pipelines.
 *
 * RTTI classes:
 *   ?AVAudioChannelMapper@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AUDIOCHANNELMAPPER_H
#define AUDIOCHANNELMAPPER_H

#include "../HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// AudioChannelMapper
// ============================================================================
// Converts audio buffers between different channel configurations.
// Supports mono-to-stereo upmix, stereo-to-mono downmix, and
// multi-channel fold-down to stereo with standard ITU-R BS.775
// downmix coefficients.
//
class AVSOURCE_API AudioChannelMapper
{
public:
    AudioChannelMapper();
    ~AudioChannelMapper();

    // Lifecycle
    HRESULT Initialize(DWORD dwInputChannels, DWORD dwOutputChannels, DWORD dwSampleRate);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Format queries
    DWORD GetInputChannels() const throw();
    DWORD GetOutputChannels() const throw();
    DWORD GetSampleRate() const throw();

    // Processing
    HRESULT ProcessBufferFloat(const float* pInput, float* pOutput, DWORD dwFrameCount);
    HRESULT ProcessBufferInt16(const short* pInput, short* pOutput, DWORD dwFrameCount);

    // In-place processing (output overwrites input; requires dwInputChannels >= dwOutputChannels)
    HRESULT ProcessBufferFloatInPlace(float* pBuffer, DWORD dwFrameCount);
    HRESULT ProcessBufferInt16InPlace(short* pBuffer, DWORD dwFrameCount);

    // Custom channel weights (for non-standard mappings)
    // dwOutputChannel selects which output to configure, arrWeights[0..dwInputChannels-1]
    HRESULT SetChannelWeights(DWORD dwOutputChannel, const float* arrWeights, DWORD dwInputChannelCount);

    // Convenience channel counts
    DWORD GetBytesPerFrame() const throw();

private:
    DWORD   m_dwInputChannels;
    DWORD   m_dwOutputChannels;
    DWORD   m_dwSampleRate;
    bool    m_fInitialized;

    // Default weight matrix [output][input]
    static const DWORD kMaxChannels = 8;
    float   m_arrWeights[kMaxChannels][kMaxChannels];

    void InitializeDefaultWeights();
};

} // namespace HMRAVSource

#endif // AUDIOCHANNELMAPPER_H
