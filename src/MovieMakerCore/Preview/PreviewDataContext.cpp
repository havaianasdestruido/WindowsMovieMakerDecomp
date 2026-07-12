/*
 * PreviewDataContext.cpp
 *
 * Implementation of the preview DirectUI data context. Bridges the
 * PreviewPresenterWrapper rendering state to DirectUI property bindings
 * for the preview panel in the Sundance main window.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "PreviewDataContext.h"
#include "PreviewPresenter.h"

// ============================================================================
// Property name constants
// ============================================================================
namespace
{
    const WCHAR kPropCurrentPosition[]  = L"CurrentPosition";
    const WCHAR kPropTotalDuration[]    = L"TotalDuration";
    const WCHAR kPropIsPlaying[]        = L"IsPlaying";
    const WCHAR kPropIsPaused[]         = L"IsPaused";
    const WCHAR kPropIsStopped[]        = L"IsStopped";
    const WCHAR kPropVolume[]           = L"Volume";
    const WCHAR kPropPlaybackSpeed[]    = L"PlaybackSpeed";
    const WCHAR kPropCurrentFrameUrl[]  = L"CurrentFrameUrl";
    const WCHAR kPropHasPreview[]       = L"HasPreview";

    enum PropertyDispId
    {
        dispidCurrentPosition   = 101,
        dispidTotalDuration     = 102,
        dispidIsPlaying         = 103,
        dispidIsPaused          = 104,
        dispidIsStopped         = 105,
        dispidVolume            = 106,
        dispidPlaybackSpeed     = 107,
        dispidCurrentFrameUrl   = 108,
        dispidHasPreview        = 109
    };

    struct PropertyMapping
    {
        LPCWSTR pwszName;
        DISPID  dispid;
    };

    const PropertyMapping g_propertyMap[] =
    {
        { kPropCurrentPosition,  dispidCurrentPosition },
        { kPropTotalDuration,    dispidTotalDuration },
        { kPropIsPlaying,        dispidIsPlaying },
        { kPropIsPaused,         dispidIsPaused },
        { kPropIsStopped,        dispidIsStopped },
        { kPropVolume,           dispidVolume },
        { kPropPlaybackSpeed,    dispidPlaybackSpeed },
        { kPropCurrentFrameUrl,  dispidCurrentFrameUrl },
        { kPropHasPreview,       dispidHasPreview },
    };

    const int g_cProperties = ARRAYSIZE(g_propertyMap);
}

// ============================================================================
// Construction / destruction
// ============================================================================
PreviewDataContext::PreviewDataContext()
    : m_fIsPlaying(false)
    , m_fIsPaused(false)
    , m_fIsStopped(true)
    , m_fHasPreview(false)
    , m_dblCurrentPosition(0.0)
    , m_dblTotalDuration(0.0)
    , m_dblVolume(1.0)
    , m_dblPlaybackSpeed(1.0)
    , m_bstrCurrentFrameUrl(nullptr)
    , m_pPresenter(nullptr)
{
}

PreviewDataContext::~PreviewDataContext()
{
    if (m_bstrCurrentFrameUrl)
    {
        SysFreeString(m_bstrCurrentFrameUrl);
        m_bstrCurrentFrameUrl = nullptr;
    }
}

// ============================================================================
// CreatePreviewDataContext
// ============================================================================
HRESULT CreatePreviewDataContext(PreviewDataContext** ppContext)
{
    if (!ppContext)
        return E_POINTER;

    *ppContext = nullptr;

    CComObject<PreviewDataContext>* pObj = nullptr;
    HRESULT hr = CComObject<PreviewDataContext>::CreateInstance(&pObj);
    if (FAILED(hr))
        return hr;

    pObj->AddRef();
    *ppContext = pObj;

    return S_OK;
}

// ============================================================================
// SetPresenter / GetPresenter
// ============================================================================
void PreviewDataContext::SetPresenter(PreviewPresenterWrapper* pPresenter)
{
    m_pPresenter = pPresenter;
    m_fHasPreview = (pPresenter != nullptr);
    RefreshAllProperties();
}

PreviewPresenterWrapper* PreviewDataContext::GetPresenter() const throw()
{
    return m_pPresenter;
}

// ============================================================================
// GetProperty
// ============================================================================
HRESULT PreviewDataContext::GetProperty(LPCWSTR pszName, VARIANT* pvarValue)
{
    if (!pszName || !pvarValue)
        return E_INVALIDARG;

    VariantInit(pvarValue);

    if (wcscmp(pszName, kPropCurrentPosition) == 0)
    {
        RefreshPositionState();
        pvarValue->vt = VT_R8;
        pvarValue->dblVal = m_dblCurrentPosition;
        return S_OK;
    }

    if (wcscmp(pszName, kPropTotalDuration) == 0)
    {
        RefreshPositionState();
        pvarValue->vt = VT_R8;
        pvarValue->dblVal = m_dblTotalDuration;
        return S_OK;
    }

    if (wcscmp(pszName, kPropIsPlaying) == 0)
    {
        RefreshPlaybackState();
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = m_fIsPlaying ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropIsPaused) == 0)
    {
        RefreshPlaybackState();
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = m_fIsPaused ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropIsStopped) == 0)
    {
        RefreshPlaybackState();
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = m_fIsStopped ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropVolume) == 0)
    {
        pvarValue->vt = VT_R8;
        pvarValue->dblVal = m_dblVolume;
        return S_OK;
    }

    if (wcscmp(pszName, kPropPlaybackSpeed) == 0)
    {
        pvarValue->vt = VT_R8;
        pvarValue->dblVal = m_dblPlaybackSpeed;
        return S_OK;
    }

    if (wcscmp(pszName, kPropCurrentFrameUrl) == 0)
    {
        pvarValue->vt = VT_BSTR;
        pvarValue->bstrVal = SysAllocString(m_bstrCurrentFrameUrl ? m_bstrCurrentFrameUrl : L"");
        if (!pvarValue->bstrVal)
            return E_OUTOFMEMORY;
        return S_OK;
    }

    if (wcscmp(pszName, kPropHasPreview) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = m_fHasPreview ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    return DISP_E_UNKNOWNNAME;
}

// ============================================================================
// SetProperty
// ============================================================================
HRESULT PreviewDataContext::SetProperty(LPCWSTR pszName, const VARIANT* varValue)
{
    if (!pszName || !varValue)
        return E_INVALIDARG;

    if (wcscmp(pszName, kPropVolume) == 0)
    {
        VARIANT varConverted;
        VariantInit(&varConverted);
        HRESULT hr = VariantChangeType(&varConverted, const_cast<VARIANT*>(varValue), 0, VT_R8);
        if (SUCCEEDED(hr))
        {
            m_dblVolume = varConverted.dblVal;
            if (m_pPresenter)
                m_pPresenter->SetVolume(m_dblVolume);
            FirePropertyChanged(kPropVolume);
        }
        return hr;
    }

    if (wcscmp(pszName, kPropPlaybackSpeed) == 0)
    {
        VARIANT varConverted;
        VariantInit(&varConverted);
        HRESULT hr = VariantChangeType(&varConverted, const_cast<VARIANT*>(varValue), 0, VT_R8);
        if (SUCCEEDED(hr))
        {
            m_dblPlaybackSpeed = varConverted.dblVal;
            if (m_pPresenter)
                m_pPresenter->SetPlaybackSpeed(m_dblPlaybackSpeed);
            FirePropertyChanged(kPropPlaybackSpeed);
        }
        return hr;
    }

    return E_NOTIMPL;
}

// ============================================================================
// RefreshAllProperties
// ============================================================================
void PreviewDataContext::RefreshAllProperties()
{
    RefreshPlaybackState();
    RefreshPositionState();
}

// ============================================================================
// RefreshPlaybackState
// ============================================================================
void PreviewDataContext::RefreshPlaybackState()
{
    if (!m_pPresenter)
    {
        m_fIsPlaying = false;
        m_fIsPaused = false;
        m_fIsStopped = true;
        return;
    }

    PreviewPresenterWrapper::PreviewState state = m_pPresenter->GetState();
    m_fIsPlaying = (state == PreviewPresenterWrapper::PreviewStatePlaying);
    m_fIsPaused = (state == PreviewPresenterWrapper::PreviewStatePaused);
    m_fIsStopped = (state == PreviewPresenterWrapper::PreviewStateStopped);
}

// ============================================================================
// RefreshPositionState
// ============================================================================
void PreviewDataContext::RefreshPositionState()
{
    if (!m_pPresenter)
    {
        m_dblCurrentPosition = 0.0;
        m_dblTotalDuration = 0.0;
        return;
    }

    m_dblCurrentPosition = m_pPresenter->GetNormalizedPosition();
    LONGLONG llTotal = m_pPresenter->GetCurrentPositionHns();
    UNREFERENCED_PARAMETER(llTotal);
}

// ============================================================================
// Notification handlers
// ============================================================================
void PreviewDataContext::OnPlaybackStateChanged()
{
    RefreshPlaybackState();
    FirePropertyChanged(kPropIsPlaying);
    FirePropertyChanged(kPropIsPaused);
    FirePropertyChanged(kPropIsStopped);
}

void PreviewDataContext::OnPositionChanged(LONGLONG llPositionHns)
{
    UNREFERENCED_PARAMETER(llPositionHns);
    RefreshPositionState();
    FirePropertyChanged(kPropCurrentPosition);
}

void PreviewDataContext::OnVolumeChanged(double dblVolume)
{
    m_dblVolume = dblVolume;
    FirePropertyChanged(kPropVolume);
}

// ============================================================================
// FirePropertyChanged
// ============================================================================
void PreviewDataContext::FirePropertyChanged(LPCWSTR pszPropertyName)
{
    UNREFERENCED_PARAMETER(pszPropertyName);
}

// ============================================================================
// IDispatch::GetIDsOfNames
// ============================================================================
HRESULT STDMETHODCALLTYPE PreviewDataContext::GetIDsOfNames(
    REFIID /*riid*/,
    LPOLESTR* rgszNames,
    UINT cNames,
    LCID /*lcid*/,
    DISPID* rgDispId)
{
    if (!rgszNames || !rgDispId || cNames == 0)
        return E_INVALIDARG;

    for (UINT i = 0; i < cNames; ++i)
    {
        rgDispId[i] = DISPID_UNKNOWN;

        for (int j = 0; j < g_cProperties; ++j)
        {
            if (_wcsicmp(rgszNames[i], g_propertyMap[j].pwszName) == 0)
            {
                rgDispId[i] = g_propertyMap[j].dispid;
                break;
            }
        }
    }

    return S_OK;
}

// ============================================================================
// IDispatch::GetTypeInfo
// ============================================================================
HRESULT STDMETHODCALLTYPE PreviewDataContext::GetTypeInfo(
    UINT /*iTInfo*/,
    LCID /*lcid*/,
    ITypeInfo** ppTInfo)
{
    if (!ppTInfo)
        return E_POINTER;

    *ppTInfo = nullptr;
    return E_NOTIMPL;
}

// ============================================================================
// IDispatch::GetTypeInfoCount
// ============================================================================
HRESULT STDMETHODCALLTYPE PreviewDataContext::GetTypeInfoCount(UINT* pctinfo)
{
    if (!pctinfo)
        return E_POINTER;

    *pctinfo = 0;
    return S_OK;
}

// ============================================================================
// IDispatch::Invoke
// ============================================================================
HRESULT STDMETHODCALLTYPE PreviewDataContext::Invoke(
    DISPID dispIdMember,
    REFIID /*riid*/,
    LCID /*lcid*/,
    WORD wFlags,
    DISPPARAMS* pDispParams,
    VARIANT* pVarResult,
    EXCEPINFO* /*pExcepInfo*/,
    UINT* /*puArgErr*/)
{
    if (!pVarResult)
        return E_POINTER;

    VariantInit(pVarResult);

    bool fWriting = (wFlags & DISPATCH_PROPERTYPUT) || (wFlags & DISPATCH_PROPERTYPUTREF);

    if (fWriting && pDispParams && pDispParams->cArgs == 1)
    {
        switch (dispIdMember)
        {
        case dispidVolume:
            return SetProperty(kPropVolume, &pDispParams->rgvarg[0]);
        case dispidPlaybackSpeed:
            return SetProperty(kPropPlaybackSpeed, &pDispParams->rgvarg[0]);
        default:
            return DISP_E_MEMBERNOTFOUND;
        }
    }

    switch (dispIdMember)
    {
    case dispidCurrentPosition:
        RefreshPositionState();
        pVarResult->vt = VT_R8;
        pVarResult->dblVal = m_dblCurrentPosition;
        return S_OK;

    case dispidTotalDuration:
        RefreshPositionState();
        pVarResult->vt = VT_R8;
        pVarResult->dblVal = m_dblTotalDuration;
        return S_OK;

    case dispidIsPlaying:
        RefreshPlaybackState();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = m_fIsPlaying ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;

    case dispidIsPaused:
        RefreshPlaybackState();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = m_fIsPaused ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;

    case dispidIsStopped:
        RefreshPlaybackState();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = m_fIsStopped ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;

    case dispidVolume:
        pVarResult->vt = VT_R8;
        pVarResult->dblVal = m_dblVolume;
        return S_OK;

    case dispidPlaybackSpeed:
        pVarResult->vt = VT_R8;
        pVarResult->dblVal = m_dblPlaybackSpeed;
        return S_OK;

    case dispidCurrentFrameUrl:
        pVarResult->vt = VT_BSTR;
        pVarResult->bstrVal = SysAllocString(m_bstrCurrentFrameUrl ? m_bstrCurrentFrameUrl : L"");
        return pVarResult->bstrVal ? S_OK : E_OUTOFMEMORY;

    case dispidHasPreview:
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = m_fHasPreview ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;

    default:
        return DISP_E_MEMBERNOTFOUND;
    }
}
