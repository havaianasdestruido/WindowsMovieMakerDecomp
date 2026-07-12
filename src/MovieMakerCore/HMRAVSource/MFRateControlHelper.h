/*
 * MFRateControlHelper.h
 *
 * MFRateControlHelper - Media Foundation rate control for playback
 * speed adjustment. Handles both thinning (dropping frames) and
 * non-thinning (time-stretching) rate modes for video playback.
 *
 * RTTI: ?AVMFRateControlHelper@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef MFRATECONTROLHELPER_H
#define MFRATECONTROLHELPER_H

#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// RateMode
// ============================================================================
enum RateMode
{
    RateModeNormal         = 0,    // 1.0x playback
    RateModeThinning       = 1,    // Frame dropping for fast playback
    RateModeNonThinning    = 2,    // Time stretching (no frame drops)
    RateModeReverse        = 3     // Reverse playback
};

// ============================================================================
// RateChangeEvent
// ============================================================================
struct RateChangeEvent
{
    double  dblOldRate;
    double  dblNewRate;
    RateMode oldMode;
    RateMode newMode;
    HRESULT hrResult;
};

// ============================================================================
// MFRateControlHelper
// ============================================================================
// Provides rate control functionality for Media Foundation playback.
// Manages playback rate changes, thinning vs non-thinning modes,
// and provides rate-related query helpers for the source reader.
//
class AVSOURCE_API MFRateControlHelper
{
public:
    MFRateControlHelper();
    ~MFRateControlHelper();

    // Lifecycle
    HRESULT Initialize(IMFSourceReader* pReader);
    HRESULT Initialize(IMFMediaSession* pSession);
    HRESULT Shutdown();

    // Rate control
    HRESULT SetRate(double dblRate);
    HRESULT SetRate(double dblRate, bool fThin);
    double GetRate() const throw();
    HRESULT GetSupportedRate(double dblRate, bool fThin, bool* pfSupported);

    // Rate bounds
    HRESULT GetRateBounds(float* pflMinRate, float* pflMaxRate);
    bool IsRateSupported(double dblRate) const throw();
    bool IsThinMode() const throw();

    // Mode
    RateMode GetRateMode() const throw();
    void SetRateMode(RateMode mode) throw();

    // Smooth rate transitions
    HRESULT SetRateSmooth(double dblTargetRate, LONGLONG llTransitionDurationMs);
    HRESULT CancelSmoothTransition();

    // Rate history
    struct RateHistoryEntry
    {
        double      dblRate;
        RateMode    mode;
        LONGLONG    llTimestampMs;
    };

    DWORD GetRateChangeCount() const throw();
    RateHistoryEntry GetRateChangeByIndex(DWORD dwIndex) const;
    void ClearRateHistory();

    // Callbacks
    using RateChangeCallback = std::function<void(const RateChangeEvent&)>;
    void SetRateChangeCallback(RateChangeCallback cb) { m_rateChangeCb = cb; }

private:
    IMFSourceReader*    m_pReader;
    IMFMediaSession*    m_pSession;
    double              m_dblCurrentRate;
    RateMode            m_currentMode;
    bool                m_fInitialized;

    ATL::CAtlArray<RateHistoryEntry> m_rateHistory;
    RateChangeCallback  m_rateChangeCb;

    // Smooth transition state
    bool                m_fSmoothTransition;
    double              m_dblTargetRate;
    LONGLONG            m_llTransitionDurationMs;
    LONGLONG            m_llTransitionStartMs;

    HRESULT ApplyRateViaSourceReader(double dblRate, bool fThin);
    HRESULT ApplyRateViaSession(double dblRate);
    void RecordRateChange(double dblRate, RateMode mode);
    void FireRateChangeEvent(double dblOldRate, double dblNewRate,
                             RateMode oldMode, RateMode newMode, HRESULT hr);
    RateMode DetermineRateMode(double dblRate, bool fThin) const;
};

} // namespace HMRAVSource

#endif // MFRATECONTROLHELPER_H
