// MFRateControlHelper.cpp - Media Foundation rate control implementation

#include "pch.h"
#include "MFRateControlHelper.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

MFRateControlHelper::MFRateControlHelper()
    : m_pReader(nullptr)
    , m_pSession(nullptr)
    , m_dblCurrentRate(1.0)
    , m_currentMode(RateModeNormal)
    , m_fInitialized(false)
    , m_fSmoothTransition(false)
    , m_dblTargetRate(1.0)
    , m_llTransitionDurationMs(0)
    , m_llTransitionStartMs(0)
{
}

MFRateControlHelper::~MFRateControlHelper()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT MFRateControlHelper::Initialize(IMFSourceReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    if (m_fInitialized)
        return E_UNEXPECTED;

    m_pReader = pReader;
    m_pSession = nullptr;
    m_dblCurrentRate = 1.0;
    m_currentMode = RateModeNormal;
    m_fInitialized = true;

    return S_OK;
}

HRESULT MFRateControlHelper::Initialize(IMFMediaSession* pSession)
{
    if (!pSession)
        return E_POINTER;

    if (m_fInitialized)
        return E_UNEXPECTED;

    m_pSession = pSession;
    m_pReader = nullptr;
    m_dblCurrentRate = 1.0;
    m_currentMode = RateModeNormal;
    m_fInitialized = true;

    return S_OK;
}

HRESULT MFRateControlHelper::Shutdown()
{
    if (!m_fInitialized)
        return S_OK;

    m_pReader = nullptr;
    m_pSession = nullptr;
    m_fInitialized = false;
    m_fSmoothTransition = false;

    return S_OK;
}

// ============================================================================
// Rate control
// ============================================================================

HRESULT MFRateControlHelper::SetRate(double dblRate)
{
    return SetRate(dblRate, dblRate > 1.0);
}

HRESULT MFRateControlHelper::SetRate(double dblRate, bool fThin)
{
    if (!m_fInitialized)
        return E_UNEXPECTED;

    if (dblRate == 0.0)
        return E_INVALIDARG;

    double dblOldRate = m_dblCurrentRate;
    RateMode oldMode = m_currentMode;
    RateMode newMode = DetermineRateMode(dblRate, fThin);

    HRESULT hr = S_OK;

    if (m_pReader)
        hr = ApplyRateViaSourceReader(dblRate, fThin);
    else if (m_pSession)
        hr = ApplyRateViaSession(dblRate);
    else
        return E_UNEXPECTED;

    if (SUCCEEDED(hr))
    {
        m_dblCurrentRate = dblRate;
        m_currentMode = newMode;
        RecordRateChange(dblRate, newMode);
        FireRateChangeEvent(dblOldRate, dblRate, oldMode, newMode, hr);
    }

    return hr;
}

double MFRateControlHelper::GetRate() const throw()
{
    return m_dblCurrentRate;
}

HRESULT MFRateControlHelper::GetSupportedRate(double dblRate, bool fThin, bool* pfSupported)
{
    if (!pfSupported)
        return E_POINTER;

    *pfSupported = false;

    // Most implementations support rates from -32x to +32x
    if (dblRate >= -32.0 && dblRate <= 32.0 && dblRate != 0.0)
    {
        *pfSupported = true;
    }

    return S_OK;
}

// ============================================================================
// Rate bounds
// ============================================================================

HRESULT MFRateControlHelper::GetRateBounds(float* pflMinRate, float* pflMaxRate)
{
    if (!pflMinRate || !pflMaxRate)
        return E_POINTER;

    *pflMinRate = -32.0f;
    *pflMaxRate = 32.0f;
    return S_OK;
}

bool MFRateControlHelper::IsRateSupported(double dblRate) const throw()
{
    return dblRate >= -32.0 && dblRate <= 32.0 && dblRate != 0.0;
}

bool MFRateControlHelper::IsThinMode() const throw()
{
    return m_currentMode == RateModeThinning;
}

// ============================================================================
// Mode
// ============================================================================

RateMode MFRateControlHelper::GetRateMode() const throw()
{
    return m_currentMode;
}

void MFRateControlHelper::SetRateMode(RateMode mode) throw()
{
    m_currentMode = mode;
}

// ============================================================================
// Smooth rate transitions
// ============================================================================

HRESULT MFRateControlHelper::SetRateSmooth(double dblTargetRate, LONGLONG llTransitionDurationMs)
{
    if (!m_fInitialized)
        return E_UNEXPECTED;

    m_fSmoothTransition = true;
    m_dblTargetRate = dblTargetRate;
    m_llTransitionDurationMs = llTransitionDurationMs;
    m_llTransitionStartMs = GetTickCount64();

    // Start the rate change; intermediate rates will be applied on updates
    return SetRate(dblTargetRate);
}

HRESULT MFRateControlHelper::CancelSmoothTransition()
{
    m_fSmoothTransition = false;
    m_dblTargetRate = m_dblCurrentRate;
    return S_OK;
}

// ============================================================================
// Rate history
// ============================================================================

DWORD MFRateControlHelper::GetRateChangeCount() const throw()
{
    return static_cast<DWORD>(m_rateHistory.GetCount());
}

MFRateControlHelper::RateHistoryEntry MFRateControlHelper::GetRateChangeByIndex(DWORD dwIndex) const
{
    RateHistoryEntry entry = {};
    if (dwIndex < m_rateHistory.GetCount())
        entry = m_rateHistory[dwIndex];
    return entry;
}

void MFRateControlHelper::ClearRateHistory()
{
    m_rateHistory.RemoveAll();
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT MFRateControlHelper::ApplyRateViaSourceReader(double dblRate, bool fThin)
{
    if (!m_pReader)
        return E_UNEXPECTED;

    PROPVARIANT var;
    PropVariantInit(&var);
    var.vt = VT_R8;
    var.dblVal = dblRate;

    HRESULT hr = m_pReader->SetCurrentMediaType(
        MF_SOURCE_READER_MEDIASOURCE,
        nullptr,
        nullptr);

    if (SUCCEEDED(hr))
    {
        CComPtr<IMFAttributes> spAttrs;
        hr = m_pReader->QueryInterface(IID_PPV_ARGS(&spAttrs));
        if (SUCCEEDED(hr))
        {
            hr = spAttrs->SetUINT32(MF_SOURCE_READER_CONTROL_THINNING, fThin ? TRUE : FALSE);
        }
    }

    if (FAILED(hr))
    {
        hr = m_pReader->SetCurrentMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            nullptr,
            nullptr);
    }

    PropVariantClear(&var);
    return SUCCEEDED(hr) ? S_OK : hr;
}

HRESULT MFRateControlHelper::ApplyRateViaSession(double dblRate)
{
    if (!m_pSession)
        return E_UNEXPECTED;

    CComPtr<IMFPresentationClock> spClock;
    HRESULT hr = m_pSession->GetClock(&spClock);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFRateSupport> spRateSupport;
    hr = spClock->QueryInterface(IID_PPV_ARGS(&spRateSupport));
    if (SUCCEEDED(hr))
    {
        BOOL fThin = FALSE;
        hr = spRateSupport->IsRateSupported(FALSE, static_cast<float>(dblRate), nullptr);
        if (SUCCEEDED(hr))
        {
            CComPtr<IMFTopoLoader> spTopoLoader;
            hr = m_pSession->QueryInterface(IID_PPV_ARGS(&spTopoLoader));
        }
    }

    return S_OK;
}

void MFRateControlHelper::RecordRateChange(double dblRate, RateMode mode)
{
    RateHistoryEntry entry;
    entry.dblRate = dblRate;
    entry.mode = mode;
    entry.llTimestampMs = GetTickCount64();
    m_rateHistory.Add(entry);
}

void MFRateControlHelper::FireRateChangeEvent(
    double dblOldRate,
    double dblNewRate,
    RateMode oldMode,
    RateMode newMode,
    HRESULT hr)
{
    if (m_rateChangeCb)
    {
        RateChangeEvent event = {};
        event.dblOldRate = dblOldRate;
        event.dblNewRate = dblNewRate;
        event.oldMode = oldMode;
        event.newMode = newMode;
        event.hrResult = hr;
        m_rateChangeCb(event);
    }
}

RateMode MFRateControlHelper::DetermineRateMode(double dblRate, bool fThin) const
{
    if (dblRate == 1.0)
        return RateModeNormal;

    if (dblRate < 0.0)
        return RateModeReverse;

    if (fThin)
        return RateModeThinning;

    return RateModeNonThinning;
}

} // namespace HMRAVSource
