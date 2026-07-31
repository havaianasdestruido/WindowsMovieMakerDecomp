#include "pch.h"
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
    , m_llCurrentPositionHns(0)
    , m_llTotalDurationHns(0)
    , m_pPresenter(nullptr)
    , m_pProject(nullptr)
{
    InitializeCriticalSection(&m_csLock);
}

PreviewDataContext::~PreviewDataContext()
{
    if (m_bstrCurrentFrameUrl)
    {
        SysFreeString(m_bstrCurrentFrameUrl);
        m_bstrCurrentFrameUrl = nullptr;
    }

    DeleteCriticalSection(&m_csLock);
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
    EnterCriticalSection(&m_csLock);
    m_pPresenter = pPresenter;
    m_fHasPreview = (pPresenter != nullptr);
    RefreshPlaybackState();
    RefreshPositionState();
    LeaveCriticalSection(&m_csLock);

    FirePropertyChanged(kPropHasPreview);
    FirePropertyChanged(kPropIsPlaying);
    FirePropertyChanged(kPropIsPaused);
    FirePropertyChanged(kPropIsStopped);
    FirePropertyChanged(kPropCurrentPosition);
    FirePropertyChanged(kPropTotalDuration);
}

PreviewPresenterWrapper* PreviewDataContext::GetPresenter() const throw()
{
    EnterCriticalSection(&m_csLock);
    PreviewPresenterWrapper* pPresenter = m_pPresenter;
    LeaveCriticalSection(&m_csLock);
    return pPresenter;
}

// ============================================================================
// SetProject / GetProject
// ============================================================================
void PreviewDataContext::SetProject(StoryboardManager::MovieProject* pProject)
{
    EnterCriticalSection(&m_csLock);
    m_pProject = pProject;
    RefreshPositionState();
    LeaveCriticalSection(&m_csLock);

    FirePropertyChanged(kPropCurrentPosition);
    FirePropertyChanged(kPropTotalDuration);
}

StoryboardManager::MovieProject* PreviewDataContext::GetProject() const throw()
{
    EnterCriticalSection(&m_csLock);
    StoryboardManager::MovieProject* pProject = m_pProject;
    LeaveCriticalSection(&m_csLock);
    return pProject;
}

// ============================================================================
// GetProperty
// ============================================================================
HRESULT PreviewDataContext::GetProperty(LPCWSTR pszName, VARIANT* pvarValue)
{
    if (!pszName || !pvarValue)
        return E_INVALIDARG;

    VariantInit(pvarValue);

    EnterCriticalSection(&m_csLock);

    HRESULT hr = S_OK;

    if (wcscmp(pszName, kPropCurrentPosition) == 0)
    {
        RefreshPositionState();
        pvarValue->vt = VT_R8;
        pvarValue->dblVal = m_dblCurrentPosition;
    }
    else if (wcscmp(pszName, kPropTotalDuration) == 0)
    {
        RefreshPositionState();
        pvarValue->vt = VT_R8;
        pvarValue->dblVal = m_dblTotalDuration;
    }
    else if (wcscmp(pszName, kPropIsPlaying) == 0)
    {
        RefreshPlaybackState();
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = m_fIsPlaying ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropIsPaused) == 0)
    {
        RefreshPlaybackState();
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = m_fIsPaused ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropIsStopped) == 0)
    {
        RefreshPlaybackState();
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = m_fIsStopped ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropVolume) == 0)
    {
        pvarValue->vt = VT_R8;
        pvarValue->dblVal = m_dblVolume;
    }
    else if (wcscmp(pszName, kPropPlaybackSpeed) == 0)
    {
        pvarValue->vt = VT_R8;
        pvarValue->dblVal = m_dblPlaybackSpeed;
    }
    else if (wcscmp(pszName, kPropCurrentFrameUrl) == 0)
    {
        pvarValue->vt = VT_BSTR;
        pvarValue->bstrVal = SysAllocString(m_bstrCurrentFrameUrl ? m_bstrCurrentFrameUrl : L"");
        if (!pvarValue->bstrVal)
            hr = E_OUTOFMEMORY;
    }
    else if (wcscmp(pszName, kPropHasPreview) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = m_fHasPreview ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
        hr = DISP_E_UNKNOWNNAME;
    }

    LeaveCriticalSection(&m_csLock);
    return hr;
}

// ============================================================================
// SetProperty
// ============================================================================
HRESULT PreviewDataContext::SetProperty(LPCWSTR pszName, const VARIANT* varValue)
{
    if (!pszName || !varValue)
        return E_INVALIDARG;

    HRESULT hr = DISP_E_MEMBERNOTFOUND;

    if (wcscmp(pszName, kPropVolume) == 0)
    {
        VARIANT varConverted;
        VariantInit(&varConverted);
        hr = VariantChangeType(&varConverted, const_cast<VARIANT*>(varValue), 0, VT_R8);
        if (SUCCEEDED(hr))
        {
            EnterCriticalSection(&m_csLock);
            m_dblVolume = varConverted.dblVal;
            if (m_pPresenter)
                m_pPresenter->SetVolume(m_dblVolume);
            LeaveCriticalSection(&m_csLock);

            FirePropertyChanged(kPropVolume);
        }
    }
    else if (wcscmp(pszName, kPropPlaybackSpeed) == 0)
    {
        VARIANT varConverted;
        VariantInit(&varConverted);
        hr = VariantChangeType(&varConverted, const_cast<VARIANT*>(varValue), 0, VT_R8);
        if (SUCCEEDED(hr))
        {
            EnterCriticalSection(&m_csLock);
            m_dblPlaybackSpeed = varConverted.dblVal;
            if (m_pPresenter)
                m_pPresenter->SetPlaybackSpeed(m_dblPlaybackSpeed);
            LeaveCriticalSection(&m_csLock);

            FirePropertyChanged(kPropPlaybackSpeed);
        }
    }

    return hr;
}

// ============================================================================
// RefreshAllProperties
// ============================================================================
void PreviewDataContext::RefreshAllProperties()
{
    EnterCriticalSection(&m_csLock);
    RefreshPlaybackState();
    RefreshPositionState();
    LeaveCriticalSection(&m_csLock);
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
        m_llCurrentPositionHns = 0;
        m_llTotalDurationHns = 0;
        m_dblCurrentPosition = 0.0;
        m_dblTotalDuration = 0.0;
        return;
    }

    m_llCurrentPositionHns = m_pPresenter->GetCurrentPositionHns();
    m_llTotalDurationHns = m_pPresenter->GetTotalDurationHns();

    static const double hnsToSeconds = 1.0 / 10000000.0;
    m_dblCurrentPosition = static_cast<double>(m_llCurrentPositionHns) * hnsToSeconds;
    m_dblTotalDuration = static_cast<double>(m_llTotalDurationHns) * hnsToSeconds;
}

// ============================================================================
// GetCurrentPositionHns
// ============================================================================
LONGLONG PreviewDataContext::GetCurrentPositionHns() const throw()
{
    EnterCriticalSection(&m_csLock);
    LONGLONG llPosition = m_pPresenter
        ? m_pPresenter->GetCurrentPositionHns()
        : m_llCurrentPositionHns;
    LeaveCriticalSection(&m_csLock);
    return llPosition;
}

// ============================================================================
// GetTotalDurationHns
// ============================================================================
LONGLONG PreviewDataContext::GetTotalDurationHns() const throw()
{
    EnterCriticalSection(&m_csLock);
    LONGLONG llDuration = m_pPresenter
        ? m_pPresenter->GetTotalDurationHns()
        : m_llTotalDurationHns;
    LeaveCriticalSection(&m_csLock);
    return llDuration;
}

// ============================================================================
// SetCurrentPositionHns
// ============================================================================
void PreviewDataContext::SetCurrentPositionHns(LONGLONG llPositionHns)
{
    if (llPositionHns < 0)
        llPositionHns = 0;

    bool fNotify = false;

    EnterCriticalSection(&m_csLock);
    m_llCurrentPositionHns = llPositionHns;
    if (m_pPresenter)
    {
        m_pPresenter->SeekTo(llPositionHns);
        fNotify = true;
    }

    static const double hnsToSeconds = 1.0 / 10000000.0;
    m_dblCurrentPosition = static_cast<double>(llPositionHns) * hnsToSeconds;
    LeaveCriticalSection(&m_csLock);

    if (fNotify)
        FirePropertyChanged(kPropCurrentPosition);
}

// ============================================================================
// Notification handlers
// ============================================================================
void PreviewDataContext::OnPlaybackStateChanged()
{
    bool fNotify = false;

    EnterCriticalSection(&m_csLock);
    if (m_pPresenter)
    {
        fNotify = true;
        RefreshPlaybackState();
    }
    LeaveCriticalSection(&m_csLock);

    if (!fNotify)
        return;

    FirePropertyChanged(kPropIsPlaying);
    FirePropertyChanged(kPropIsPaused);
    FirePropertyChanged(kPropIsStopped);
}

void PreviewDataContext::OnPositionChanged(LONGLONG llPositionHns)
{
    if (llPositionHns < 0)
        llPositionHns = 0;

    bool fNotify = false;

    EnterCriticalSection(&m_csLock);
    if (m_pPresenter)
    {
        fNotify = true;
        m_llCurrentPositionHns = llPositionHns;

        static const double hnsToSeconds = 1.0 / 10000000.0;
        m_dblCurrentPosition = static_cast<double>(llPositionHns) * hnsToSeconds;
    }
    LeaveCriticalSection(&m_csLock);

    if (!fNotify)
        return;

    FirePropertyChanged(kPropCurrentPosition);
}

void PreviewDataContext::OnVolumeChanged(double dblVolume)
{
    bool fNotify = false;

    EnterCriticalSection(&m_csLock);
    if (m_pPresenter)
    {
        fNotify = true;
        m_dblVolume = dblVolume;
    }
    LeaveCriticalSection(&m_csLock);

    if (!fNotify)
        return;

    FirePropertyChanged(kPropVolume);
}

// ============================================================================
// FirePropertyChanged
// ============================================================================
void PreviewDataContext::FirePropertyChanged(LPCWSTR pszPropertyName)
{
    if (!pszPropertyName)
        return;

    DISPID dispid = DISPID_UNKNOWN;
    for (int j = 0; j < g_cProperties; ++j)
    {
        if (_wcsicmp(g_propertyMap[j].pwszName, pszPropertyName) == 0)
        {
            dispid = g_propertyMap[j].dispid;
            break;
        }
    }

    if (dispid == DISPID_UNKNOWN)
        return;

    IConnectionPointContainer* pCPC = NULL;
    if (SUCCEEDED(QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC)))
    {
        IConnectionPoint* pCP = NULL;
        if (SUCCEEDED(pCPC->FindConnectionPoint(IID_IDispatch, &pCP)))
        {
            IEnumConnections* pEnum = NULL;
            if (SUCCEEDED(pCP->EnumConnections(&pEnum)))
            {
                CONNECTDATA cd;
                while (pEnum->Next(1, &cd, NULL) == S_OK)
                {
                    IDispatch* pSink = NULL;
                    if (SUCCEEDED(cd.pUnk->QueryInterface(IID_IDispatch, (void**)&pSink)))
                    {
                        DISPPARAMS dp = { NULL, NULL, 0, 0 };
                        pSink->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dp, NULL, NULL, NULL);
                        pSink->Release();
                    }
                    if (cd.pUnk) cd.pUnk->Release();
                }
                pEnum->Release();
            }
            pCP->Release();
        }
        pCPC->Release();
    }
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

    EnterCriticalSection(&m_csLock);

    HRESULT hr = S_OK;

    switch (dispIdMember)
    {
    case dispidCurrentPosition:
        RefreshPositionState();
        pVarResult->vt = VT_R8;
        pVarResult->dblVal = m_dblCurrentPosition;
        break;

    case dispidTotalDuration:
        RefreshPositionState();
        pVarResult->vt = VT_R8;
        pVarResult->dblVal = m_dblTotalDuration;
        break;

    case dispidIsPlaying:
        RefreshPlaybackState();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = m_fIsPlaying ? VARIANT_TRUE : VARIANT_FALSE;
        break;

    case dispidIsPaused:
        RefreshPlaybackState();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = m_fIsPaused ? VARIANT_TRUE : VARIANT_FALSE;
        break;

    case dispidIsStopped:
        RefreshPlaybackState();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = m_fIsStopped ? VARIANT_TRUE : VARIANT_FALSE;
        break;

    case dispidVolume:
        pVarResult->vt = VT_R8;
        pVarResult->dblVal = m_dblVolume;
        break;

    case dispidPlaybackSpeed:
        pVarResult->vt = VT_R8;
        pVarResult->dblVal = m_dblPlaybackSpeed;
        break;

    case dispidCurrentFrameUrl:
        pVarResult->vt = VT_BSTR;
        pVarResult->bstrVal = SysAllocString(m_bstrCurrentFrameUrl ? m_bstrCurrentFrameUrl : L"");
        if (!pVarResult->bstrVal)
            hr = E_OUTOFMEMORY;
        break;

    case dispidHasPreview:
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = m_fHasPreview ? VARIANT_TRUE : VARIANT_FALSE;
        break;

    default:
        hr = DISP_E_MEMBERNOTFOUND;
        break;
    }

    LeaveCriticalSection(&m_csLock);
    return hr;
}
