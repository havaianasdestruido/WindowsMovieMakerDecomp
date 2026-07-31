/*
 * SundanceAppDataContext.cpp
 *
 * Implementation of the COM data context for DirectUI data binding.
 * Bridges the SundanceAppMain model to DirectUI visual elements
 * through named property access via IDispatch.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "SundanceAppDataContext.h"
#include "SundanceAppMain.h"
#include "../StoryboardManager/MovieProject.h"

// ============================================================================
// Registry resource ID (for COM registration)
// ============================================================================
#ifndef IDR_SUNDANCE_APP_DATA_CONTEXT
#define IDR_SUNDANCE_APP_DATA_CONTEXT 2001
#endif

// ============================================================================
// Property name constants
// ============================================================================
namespace
{
    const WCHAR kPropProjectName[]      = L"ProjectName";
    const WCHAR kPropIsProjectOpen[]    = L"IsProjectOpen";
    const WCHAR kPropIsProjectDirty[]   = L"IsProjectDirty";
    const WCHAR kPropCanUndo[]          = L"CanUndo";
    const WCHAR kPropCanRedo[]          = L"CanRedo";
    const WCHAR kPropCanCut[]           = L"CanCut";
    const WCHAR kPropCanCopy[]          = L"CanCopy";
    const WCHAR kPropCanPaste[]         = L"CanPaste";
    const WCHAR kPropIsPlaying[]        = L"IsPlaying";
    const WCHAR kPropIsPublishing[]     = L"IsPublishing";
    const WCHAR kPropIsEncoding[]       = L"IsEncoding";
    const WCHAR kPropTimelinePosition[] = L"TimelinePosition";
    const WCHAR kPropTimelineDuration[] = L"TimelineDuration";
    const WCHAR kPropSelectedTrack[]    = L"SelectedTrack";
    const WCHAR kPropVersionString[]    = L"VersionString";

    // DISPID mapping (must be stable for DirectUI binding)
    enum PropertyDispId
    {
        dispidProjectName      = 1,
        dispidIsProjectOpen    = 2,
        dispidIsProjectDirty   = 3,
        dispidCanUndo          = 4,
        dispidCanRedo          = 5,
        dispidCanCut           = 6,
        dispidCanCopy          = 7,
        dispidCanPaste         = 8,
        dispidIsPlaying        = 9,
        dispidIsPublishing     = 10,
        dispidIsEncoding       = 11,
        dispidTimelinePosition = 12,
        dispidTimelineDuration = 13,
        dispidSelectedTrack    = 14,
        dispidVersionString    = 15,
    };

    struct PropertyMapping
    {
        LPCWSTR pwszName;
        DISPID  dispid;
    };

    const PropertyMapping g_propertyMap[] =
    {
        { kPropProjectName,      dispidProjectName },
        { kPropIsProjectOpen,    dispidIsProjectOpen },
        { kPropIsProjectDirty,   dispidIsProjectDirty },
        { kPropCanUndo,          dispidCanUndo },
        { kPropCanRedo,          dispidCanRedo },
        { kPropCanCut,           dispidCanCut },
        { kPropCanCopy,          dispidCanCopy },
        { kPropCanPaste,         dispidCanPaste },
        { kPropIsPlaying,        dispidIsPlaying },
        { kPropIsPublishing,     dispidIsPublishing },
        { kPropIsEncoding,       dispidIsEncoding },
        { kPropTimelinePosition, dispidTimelinePosition },
        { kPropTimelineDuration, dispidTimelineDuration },
        { kPropSelectedTrack,    dispidSelectedTrack },
        { kPropVersionString,    dispidVersionString },
    };

    const int g_cProperties = ARRAYSIZE(g_propertyMap);
}

// ============================================================================
// Connection-point enumerators
// ============================================================================
// Minimal, ref-counted enumerators used by the object's IConnectionPoint
// support. Both snapshot their data at creation time so later Advise/Unadvise
// calls cannot invalidate an in-progress enumeration.
namespace
{
    class CEnumConnectionPoints : public IEnumConnectionPoints
    {
    public:
        explicit CEnumConnectionPoints(IConnectionPoint* pCP) throw()
            : m_cRef(1), m_pCP(pCP), m_bDone(false)
        {
            m_pCP->AddRef();
        }

        virtual ~CEnumConnectionPoints()
        {
            m_pCP->Release();
        }

        STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override
        {
            if (!ppv)
                return E_POINTER;

            if (riid == IID_IUnknown || riid == IID_IEnumConnectionPoints)
            {
                *ppv = static_cast<IEnumConnectionPoints*>(this);
                AddRef();
                return S_OK;
            }

            *ppv = NULL;
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)() override
        {
            return static_cast<ULONG>(InterlockedIncrement(&m_cRef));
        }

        STDMETHOD_(ULONG, Release)() override
        {
            ULONG cRef = static_cast<ULONG>(InterlockedDecrement(&m_cRef));
            if (cRef == 0)
                delete this;
            return cRef;
        }

        STDMETHOD(Next)(ULONG celt, IConnectionPoint** ppCP, ULONG* pceltFetched) override
        {
            if (!ppCP)
                return E_POINTER;

            ULONG cFetched = 0;
            if (celt > 0 && !m_bDone)
            {
                m_bDone = true;
                *ppCP = m_pCP;
                m_pCP->AddRef();
                cFetched = 1;
            }

            if (pceltFetched)
                *pceltFetched = cFetched;

            return (celt == 0 || cFetched == celt) ? S_OK : S_FALSE;
        }

        STDMETHOD(Skip)(ULONG celt) override
        {
            if (celt == 0)
                return S_OK;

            if (!m_bDone)
            {
                m_bDone = true;
                return (celt == 1) ? S_OK : S_FALSE;
            }

            return S_FALSE;
        }

        STDMETHOD(Reset)() override
        {
            m_bDone = false;
            return S_OK;
        }

        STDMETHOD(Clone)(IEnumConnectionPoints** ppEnum) override
        {
            if (!ppEnum)
                return E_POINTER;

            CEnumConnectionPoints* pClone = new (std::nothrow) CEnumConnectionPoints(m_pCP);
            if (!pClone)
                return E_OUTOFMEMORY;

            pClone->m_bDone = m_bDone;
            *ppEnum = pClone;
            return S_OK;
        }

    private:
        LONG                m_cRef;
        IConnectionPoint*   m_pCP;
        bool                m_bDone;
    };

    class CEnumConnections : public IEnumConnections
    {
    public:
        CEnumConnections() throw()
            : m_cRef(1), m_cElements(0), m_nIndex(0), m_pData(NULL)
        {
        }

        virtual ~CEnumConnections()
        {
            for (ULONG i = 0; i < m_cElements; ++i)
            {
                if (m_pData[i].pUnk)
                    m_pData[i].pUnk->Release();
            }
            delete[] m_pData;
        }

        // Copies the given (non-owning) snapshot, AddRef'ing each sink.
        HRESULT InitCopy(const CONNECTDATA* pData, ULONG cElements) throw()
        {
            m_cElements = cElements;
            m_pData = new (std::nothrow) CONNECTDATA[m_cElements ? m_cElements : 1];
            if (!m_pData)
            {
                m_cElements = 0;
                return E_OUTOFMEMORY;
            }

            for (ULONG i = 0; i < m_cElements; ++i)
            {
                m_pData[i].dwCookie = pData[i].dwCookie;
                m_pData[i].pUnk = pData[i].pUnk;
                if (m_pData[i].pUnk)
                    m_pData[i].pUnk->AddRef();
            }

            return S_OK;
        }

        STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override
        {
            if (!ppv)
                return E_POINTER;

            if (riid == IID_IUnknown || riid == IID_IEnumConnections)
            {
                *ppv = static_cast<IEnumConnections*>(this);
                AddRef();
                return S_OK;
            }

            *ppv = NULL;
            return E_NOINTERFACE;
        }

        STDMETHOD_(ULONG, AddRef)() override
        {
            return static_cast<ULONG>(InterlockedIncrement(&m_cRef));
        }

        STDMETHOD_(ULONG, Release)() override
        {
            ULONG cRef = static_cast<ULONG>(InterlockedDecrement(&m_cRef));
            if (cRef == 0)
                delete this;
            return cRef;
        }

        STDMETHOD(Next)(ULONG celt, CONNECTDATA* pcd, ULONG* pceltFetched) override
        {
            if (!pcd)
                return E_POINTER;

            ULONG cFetched = 0;
            while (cFetched < celt && m_nIndex < m_cElements)
            {
                pcd[cFetched] = m_pData[m_nIndex];
                if (pcd[cFetched].pUnk)
                    pcd[cFetched].pUnk->AddRef();
                ++m_nIndex;
                ++cFetched;
            }

            if (pceltFetched)
                *pceltFetched = cFetched;

            return cFetched == celt ? S_OK : S_FALSE;
        }

        STDMETHOD(Skip)(ULONG celt) override
        {
            if (m_nIndex + celt > m_cElements)
            {
                m_nIndex = m_cElements;
                return S_FALSE;
            }

            m_nIndex += celt;
            return S_OK;
        }

        STDMETHOD(Reset)() override
        {
            m_nIndex = 0;
            return S_OK;
        }

        STDMETHOD(Clone)(IEnumConnections** ppEnum) override
        {
            if (!ppEnum)
                return E_POINTER;

            CEnumConnections* pClone = new (std::nothrow) CEnumConnections();
            if (!pClone)
                return E_OUTOFMEMORY;

            HRESULT hr = pClone->InitCopy(m_pData, m_cElements);
            if (FAILED(hr))
            {
                delete pClone;
                return hr;
            }

            pClone->m_nIndex = m_nIndex;
            *ppEnum = pClone;
            return S_OK;
        }

    private:
        LONG        m_cRef;
        ULONG       m_cElements;
        ULONG       m_nIndex;
        CONNECTDATA* m_pData;
    };
}

// ============================================================================
// Construction / destruction
// ============================================================================
SundanceAppDataContext::SundanceAppDataContext()
    : m_bProjectOpen(false)
    , m_bProjectDirty(false)
    , m_bCanUndo(false)
    , m_bCanRedo(false)
    , m_bIsPlaying(false)
    , m_bIsPublishing(false)
    , m_bIsEncoding(false)
    , m_dTimelinePosition(0.0)
    , m_dTimelineDuration(0.0)
    , m_lSelectedTrack(0)
{
}

SundanceAppDataContext::~SundanceAppDataContext()
{
}

// ============================================================================
// CreateSundanceAppDataContext
// ============================================================================
HRESULT CreateSundanceAppDataContext(SundanceAppDataContext** ppContext)
{
    if (!ppContext)
        return E_POINTER;

    *ppContext = NULL;

    CComObject<SundanceAppDataContext>* pObj = NULL;
    HRESULT hr = CComObject<SundanceAppDataContext>::CreateInstance(&pObj);
    if (FAILED(hr))
        return hr;

    pObj->AddRef();
    *ppContext = pObj;

    return S_OK;
}

// ============================================================================
// GetProperty
// ============================================================================
HRESULT SundanceAppDataContext::GetProperty(LPCWSTR pszName, VARIANT* pvarValue)
{
    if (!pszName || !pvarValue)
        return E_INVALIDARG;

    VariantInit(pvarValue);

    SundanceAppMain* pApp = GetSundanceAppMain();
    if (!pApp)
        return E_UNEXPECTED;

    if (wcscmp(pszName, kPropProjectName) == 0)
    {
        RefreshProjectProperties();
        pvarValue->vt = VT_BSTR;
        pvarValue->bstrVal = SysAllocString(m_strProjectName);
        if (!pvarValue->bstrVal)
            return E_OUTOFMEMORY;
        return S_OK;
    }

    if (wcscmp(pszName, kPropIsProjectOpen) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = pApp->IsProjectOpen() ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropIsProjectDirty) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = pApp->IsProjectDirty() ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropCanUndo) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = pApp->CanUndo() ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropCanRedo) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = pApp->CanRedo() ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropCanCut) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = pApp->CanCut() ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropCanCopy) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = pApp->CanCopy() ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropCanPaste) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = pApp->CanPaste() ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropIsPlaying) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = pApp->IsPlaying() ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropIsPublishing) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = pApp->IsPublishing() ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropIsEncoding) == 0)
    {
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = pApp->IsEncoding() ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    if (wcscmp(pszName, kPropTimelinePosition) == 0)
    {
        pvarValue->vt = VT_R8;
        pvarValue->dblVal = m_dTimelinePosition;
        return S_OK;
    }

    if (wcscmp(pszName, kPropTimelineDuration) == 0)
    {
        pvarValue->vt = VT_R8;
        pvarValue->dblVal = m_dTimelineDuration;
        return S_OK;
    }

    if (wcscmp(pszName, kPropSelectedTrack) == 0)
    {
        pvarValue->vt = VT_I4;
        pvarValue->lVal = m_lSelectedTrack;
        return S_OK;
    }

    if (wcscmp(pszName, kPropVersionString) == 0)
    {
        pvarValue->vt = VT_BSTR;
        pvarValue->bstrVal = SysAllocString(SUNDANCE_VERSION_STRING);
        if (!pvarValue->bstrVal)
            return E_OUTOFMEMORY;
        return S_OK;
    }

    return DISP_E_UNKNOWNNAME;
}

// ============================================================================
// SetProperty
// ============================================================================
HRESULT SundanceAppDataContext::SetProperty(LPCWSTR pszName, const VARIANT* varValue)
{
    if (!pszName || !varValue)
        return E_INVALIDARG;

    if (wcscmp(pszName, kPropTimelinePosition) == 0)
    {
        VARIANT varConverted;
        VariantInit(&varConverted);
        HRESULT hr = VariantChangeType(&varConverted, const_cast<VARIANT*>(varValue), 0, VT_R8);
        if (SUCCEEDED(hr))
        {
            m_dTimelinePosition = varConverted.dblVal;
            FirePropertyChanged(kPropTimelinePosition);
        }
        VariantClear(&varConverted);
        return hr;
    }

    if (wcscmp(pszName, kPropSelectedTrack) == 0)
    {
        VARIANT varConverted;
        VariantInit(&varConverted);
        HRESULT hr = VariantChangeType(&varConverted, const_cast<VARIANT*>(varValue), 0, VT_I4);
        if (SUCCEEDED(hr))
        {
            m_lSelectedTrack = varConverted.lVal;
            FirePropertyChanged(kPropSelectedTrack);
        }
        VariantClear(&varConverted);
        return hr;
    }

    return E_NOTIMPL;
}

// ============================================================================
// RefreshAllProperties
// ============================================================================
void SundanceAppDataContext::RefreshAllProperties()
{
    RefreshProjectProperties();
    RefreshCommandProperties();
    RefreshPlaybackProperties();

    FirePropertyChanged(kPropProjectName);
    FirePropertyChanged(kPropIsProjectOpen);
    FirePropertyChanged(kPropIsProjectDirty);
    FirePropertyChanged(kPropCanUndo);
    FirePropertyChanged(kPropCanRedo);
    FirePropertyChanged(kPropCanCut);
    FirePropertyChanged(kPropCanCopy);
    FirePropertyChanged(kPropCanPaste);
    FirePropertyChanged(kPropIsPlaying);
    FirePropertyChanged(kPropIsPublishing);
    FirePropertyChanged(kPropIsEncoding);
    FirePropertyChanged(kPropTimelinePosition);
    FirePropertyChanged(kPropTimelineDuration);
    FirePropertyChanged(kPropSelectedTrack);
}

// ============================================================================
// RefreshProjectProperties
// ============================================================================
void SundanceAppDataContext::RefreshProjectProperties()
{
    SundanceAppMain* pApp = GetSundanceAppMain();
    if (!pApp)
        return;

    m_bProjectOpen = pApp->IsProjectOpen();
    m_bProjectDirty = pApp->IsProjectDirty();

    // Total duration in seconds; hns = 100-nanosecond units
    const double dblHnsToSeconds = 1.0 / 10000000.0;

    StoryboardManagerNamespace::MovieProject* pProject = pApp->GetProject();
    if (pProject)
    {
        m_strProjectName = pProject->GetDisplayName();

        StoryboardManagerNamespace::ProjectTimeline* pTimeline =
            pProject->GetTimeline(StoryboardManagerNamespace::TimelineTrackTypeVideo);
        m_dTimelineDuration = pTimeline
            ? static_cast<double>(pTimeline->GetTotalDurationHns()) * dblHnsToSeconds
            : 0.0;
    }
    else
    {
        m_strProjectName.Empty();
        m_dTimelineDuration = 0.0;
    }
}

// ============================================================================
// RefreshCommandProperties
// ============================================================================
void SundanceAppDataContext::RefreshCommandProperties()
{
    SundanceAppMain* pApp = GetSundanceAppMain();
    if (!pApp)
        return;

    m_bCanUndo = pApp->CanUndo();
    m_bCanRedo = pApp->CanRedo();
}

// ============================================================================
// RefreshPlaybackProperties
// ============================================================================
void SundanceAppDataContext::RefreshPlaybackProperties()
{
    SundanceAppMain* pApp = GetSundanceAppMain();
    if (!pApp)
        return;

    m_bIsPlaying = pApp->IsPlaying();
    m_bIsPublishing = pApp->IsPublishing();
    m_bIsEncoding = pApp->IsEncoding();
}

// ============================================================================
// FirePropertyChanged
// ============================================================================
void SundanceAppDataContext::FirePropertyChanged(LPCWSTR pszPropertyName)
{
    if (!pszPropertyName)
        return;

    DISPID dispid = DISPID_UNKNOWN;
    for (int i = 0; i < _countof(g_propertyMap); ++i)
    {
        if (_wcsicmp(g_propertyMap[i].pwszName, pszPropertyName) == 0)
        {
            dispid = g_propertyMap[i].dispid;
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
// IConnectionPointContainer
// ============================================================================
HRESULT STDMETHODCALLTYPE SundanceAppDataContext::EnumConnectionPoints(
    IEnumConnectionPoints** ppEnum)
{
    if (!ppEnum)
        return E_POINTER;

    *ppEnum = NULL;

    CEnumConnectionPoints* pEnum = new (std::nothrow) CEnumConnectionPoints(
        static_cast<IConnectionPoint*>(this));
    if (!pEnum)
        return E_OUTOFMEMORY;

    *ppEnum = pEnum;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE SundanceAppDataContext::FindConnectionPoint(
    REFIID riid,
    IConnectionPoint** ppCP)
{
    if (!ppCP)
        return E_POINTER;

    *ppCP = NULL;

    if (!InlineIsEqualGUID(riid, IID_IDispatch))
        return E_NOINTERFACE;

    return QueryInterface(IID_IConnectionPoint, (void**)ppCP);
}

// ============================================================================
// IConnectionPoint
// ============================================================================
HRESULT STDMETHODCALLTYPE SundanceAppDataContext::GetConnectionInterface(IID* pIID)
{
    if (!pIID)
        return E_POINTER;

    *pIID = IID_IDispatch;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE SundanceAppDataContext::GetConnectionPointContainer(
    IConnectionPointContainer** ppCPC)
{
    if (!ppCPC)
        return E_POINTER;

    return QueryInterface(IID_IConnectionPointContainer, (void**)ppCPC);
}

HRESULT STDMETHODCALLTYPE SundanceAppDataContext::Advise(IUnknown* pUnkSink, DWORD* pdwCookie)
{
    if (!pUnkSink || !pdwCookie)
        return E_POINTER;

    *pdwCookie = 0;

    IDispatch* pSink = NULL;
    HRESULT hr = pUnkSink->QueryInterface(IID_IDispatch, (void**)&pSink);
    if (FAILED(hr))
        return CONNECT_E_CANNOTCONNECT;

    // Allocate a cookie that is unique among current connections
    DWORD dwCookie = 1;
    for (size_t i = 0; i < m_connectionSinks.size(); ++i)
    {
        if (m_connectionSinks[i].first >= dwCookie)
            dwCookie = m_connectionSinks[i].first + 1;
    }

    CComPtr<IDispatch> spSink;
    spSink.Attach(pSink); // takes ownership of the QI reference

    m_connectionSinks.push_back(std::make_pair(dwCookie, spSink));

    *pdwCookie = dwCookie;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE SundanceAppDataContext::Unadvise(DWORD dwCookie)
{
    for (size_t i = 0; i < m_connectionSinks.size(); ++i)
    {
        if (m_connectionSinks[i].first == dwCookie)
        {
            m_connectionSinks.erase(m_connectionSinks.begin() + i);
            return S_OK;
        }
    }

    return CONNECT_E_NOCONNECTION;
}

HRESULT STDMETHODCALLTYPE SundanceAppDataContext::EnumConnections(
    IEnumConnections** ppEnum)
{
    if (!ppEnum)
        return E_POINTER;

    *ppEnum = NULL;

    // Snapshot the current connections (borrowed references; InitCopy AddRef's)
    std::vector<CONNECTDATA> snapshot;
    snapshot.reserve(m_connectionSinks.size());
    for (size_t i = 0; i < m_connectionSinks.size(); ++i)
    {
        CONNECTDATA cd;
        cd.dwCookie = m_connectionSinks[i].first;
        cd.pUnk = m_connectionSinks[i].second; // borrowed
        snapshot.push_back(cd);
    }

    CEnumConnections* pEnum = new (std::nothrow) CEnumConnections();
    if (!pEnum)
        return E_OUTOFMEMORY;

    HRESULT hr = pEnum->InitCopy(snapshot.empty() ? NULL : &snapshot[0],
                                 static_cast<ULONG>(snapshot.size()));
    if (FAILED(hr))
    {
        delete pEnum;
        return hr;
    }

    *ppEnum = pEnum;
    return S_OK;
}

// ============================================================================
// Notification handlers
// ============================================================================
void SundanceAppDataContext::OnProjectStateChanged()
{
    RefreshProjectProperties();
    RefreshCommandProperties();
    RefreshPlaybackProperties();

    // Invalidate timeline-scoped state when no project is open so bound
    // properties do not retain stale values after a project close.
    if (!m_bProjectOpen)
    {
        m_dTimelinePosition = 0.0;
        m_dTimelineDuration = 0.0;
        m_lSelectedTrack = 0;
    }

    FirePropertyChanged(kPropProjectName);
    FirePropertyChanged(kPropIsProjectOpen);
    FirePropertyChanged(kPropIsProjectDirty);
    FirePropertyChanged(kPropCanUndo);
    FirePropertyChanged(kPropCanRedo);
    FirePropertyChanged(kPropCanCut);
    FirePropertyChanged(kPropCanCopy);
    FirePropertyChanged(kPropCanPaste);
    FirePropertyChanged(kPropIsPlaying);
    FirePropertyChanged(kPropIsPublishing);
    FirePropertyChanged(kPropIsEncoding);
    FirePropertyChanged(kPropTimelinePosition);
    FirePropertyChanged(kPropTimelineDuration);
    FirePropertyChanged(kPropSelectedTrack);
}

void SundanceAppDataContext::OnTimelinePositionChanged(double dPosition)
{
    m_dTimelinePosition = dPosition;
    FirePropertyChanged(kPropTimelinePosition);
}

void SundanceAppDataContext::OnSelectionChanged()
{
    RefreshCommandProperties();

    FirePropertyChanged(kPropCanUndo);
    FirePropertyChanged(kPropCanRedo);
    FirePropertyChanged(kPropCanCut);
    FirePropertyChanged(kPropCanCopy);
    FirePropertyChanged(kPropCanPaste);
}

// ============================================================================
// IDispatch::GetIDsOfNames
// ============================================================================
HRESULT STDMETHODCALLTYPE SundanceAppDataContext::GetIDsOfNames(
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
HRESULT STDMETHODCALLTYPE SundanceAppDataContext::GetTypeInfo(
    UINT /*iTInfo*/,
    LCID /*lcid*/,
    ITypeInfo** ppTInfo)
{
    if (!ppTInfo)
        return E_POINTER;

    *ppTInfo = NULL;
    return E_NOTIMPL;
}

// ============================================================================
// IDispatch::GetTypeInfoCount
// ============================================================================
HRESULT STDMETHODCALLTYPE SundanceAppDataContext::GetTypeInfoCount(UINT* pctinfo)
{
    if (!pctinfo)
        return E_POINTER;

    *pctinfo = 0;
    return S_OK;
}

// ============================================================================
// IDispatch::Invoke
// ============================================================================
HRESULT STDMETHODCALLTYPE SundanceAppDataContext::Invoke(
    DISPID dispIdMember,
    REFIID /*riid*/,
    LCID /*lcid*/,
    WORD /*wFlags*/,
    DISPPARAMS* /*pDispParams*/,
    VARIANT* pVarResult,
    EXCEPINFO* /*pExcepInfo*/,
    UINT* /*puArgErr*/)
{
    if (!pVarResult)
        return E_POINTER;

    VariantInit(pVarResult);

    switch (dispIdMember)
    {
    case dispidProjectName:
    {
        RefreshProjectProperties();
        pVarResult->vt = VT_BSTR;
        pVarResult->bstrVal = SysAllocString(m_strProjectName);
        return pVarResult->bstrVal ? S_OK : E_OUTOFMEMORY;
    }
    case dispidIsProjectOpen:
    {
        SundanceAppMain* pApp = GetSundanceAppMain();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = (pApp && pApp->IsProjectOpen()) ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    case dispidIsProjectDirty:
    {
        SundanceAppMain* pApp = GetSundanceAppMain();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = (pApp && pApp->IsProjectDirty()) ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    case dispidCanUndo:
    {
        SundanceAppMain* pApp = GetSundanceAppMain();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = (pApp && pApp->CanUndo()) ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    case dispidCanRedo:
    {
        SundanceAppMain* pApp = GetSundanceAppMain();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = (pApp && pApp->CanRedo()) ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    case dispidCanCut:
    {
        SundanceAppMain* pApp = GetSundanceAppMain();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = (pApp && pApp->CanCut()) ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    case dispidCanCopy:
    {
        SundanceAppMain* pApp = GetSundanceAppMain();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = (pApp && pApp->CanCopy()) ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    case dispidCanPaste:
    {
        SundanceAppMain* pApp = GetSundanceAppMain();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = (pApp && pApp->CanPaste()) ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    case dispidIsPlaying:
    {
        SundanceAppMain* pApp = GetSundanceAppMain();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = (pApp && pApp->IsPlaying()) ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    case dispidIsPublishing:
    {
        SundanceAppMain* pApp = GetSundanceAppMain();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = (pApp && pApp->IsPublishing()) ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    case dispidIsEncoding:
    {
        SundanceAppMain* pApp = GetSundanceAppMain();
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = (pApp && pApp->IsEncoding()) ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }
    case dispidTimelinePosition:
        pVarResult->vt = VT_R8;
        pVarResult->dblVal = m_dTimelinePosition;
        return S_OK;

    case dispidTimelineDuration:
        pVarResult->vt = VT_R8;
        pVarResult->dblVal = m_dTimelineDuration;
        return S_OK;

    case dispidSelectedTrack:
        pVarResult->vt = VT_I4;
        pVarResult->lVal = m_lSelectedTrack;
        return S_OK;

    case dispidVersionString:
        pVarResult->vt = VT_BSTR;
        pVarResult->bstrVal = SysAllocString(SUNDANCE_VERSION_STRING);
        return pVarResult->bstrVal ? S_OK : E_OUTOFMEMORY;

    default:
        return DISP_E_MEMBERNOTFOUND;
    }
}
