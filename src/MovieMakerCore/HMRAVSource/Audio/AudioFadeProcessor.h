/*
 * AudioFadeProcessor.h
 *
 * Audio fade-in/fade-out processor. Applies linear and equal-power
 * fade curves to audio sample buffers based on time position within
 * a clip.
 *
 * RTTI classes:
 *   ?AVAudioFadeProcessor@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AUDIOFADEPROCESSOR_H
#define AUDIOFADEPROCESSOR_H

#include "../HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// AudioFadeType
// ============================================================================
enum AudioFadeType
{
    AudioFadeTypeLinear         = 0,    // linear gain ramp
    AudioFadeTypeEqualPower     = 1,    // equal-power (constant energy)
    AudioFadeTypeExponential    = 2     // exponential curve
};

// ============================================================================
// AudioFadeProcessor
// ============================================================================
// Applies fade-in and fade-out gain curves to audio sample buffers.
// Tracks time position to compute the correct gain at any point
// within a clip's fade regions.
//
class AVSOURCE_API AudioFadeProcessor
{
public:
    AudioFadeProcessor();
    ~AudioFadeProcessor();

    // Lifecycle
    HRESULT Initialize(DWORD dwSampleRate, DWORD dwChannels, DWORD dwBitsPerSample);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Fade configuration
    void SetFadeInDurationHns(LONGLONG llDurationHns) throw();
    LONGLONG GetFadeInDurationHns() const throw();

    void SetFadeOutDurationHns(LONGLONG llDurationHns) throw();
    LONGLONG GetFadeOutDurationHns() const throw();

    void SetTotalDurationHns(LONGLONG llDurationHns) throw();
    LONGLONG GetTotalDurationHns() const throw();

    // Fade type
    void SetFadeInType(AudioFadeType type) throw();
    AudioFadeType GetFadeInType() const throw();

    void SetFadeOutType(AudioFadeType type) throw();
    AudioFadeType GetFadeOutType() const throw();

    // Processing
    HRESULT ProcessBufferFloat(float* pBuffer, DWORD dwFrameCount,
                               LONGLONG llPositionHns);

    HRESULT ProcessBufferInt16(short* pBuffer, DWORD dwFrameCount,
                               LONGLONG llPositionHns);

    HRESULT ProcessBufferInt32(int* pBuffer, DWORD dwFrameCount,
                               LONGLONG llPositionHns);

    // Single-sample processing
    float ComputeGain(LONGLONG llPositionHns) const throw();

    // Reset
    void Reset() throw();

private:
    DWORD           m_dwSampleRate;
    DWORD           m_dwChannels;
    DWORD           m_dwBitsPerSample;
    LONGLONG        m_llFadeInDurationHns;
    LONGLONG        m_llFadeOutDurationHns;
    LONGLONG        m_llTotalDurationHns;
    AudioFadeType   m_fadeInType;
    AudioFadeType   m_fadeOutType;
    bool            m_fInitialized;

    float ComputeFadeGain(LONGLONG llPositionHns, bool fFadingIn,
                          LONGLONG llFadeDurationHns) const throw();
};

} // namespace HMRAVSource

#endif // AUDIOFADEPROCESSOR_H
