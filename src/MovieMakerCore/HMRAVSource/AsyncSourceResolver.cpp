// AsyncSourceResolver.cpp - Asynchronous source resolver implementation

#include "pch.h"
#include <mferror.h>
#include <wininet.h>
#include "AsyncSourceResolver.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

AsyncSourceResolver::AsyncSourceResolver()
    : m_state(AsyncResolverIdle)
    , m_hrAsyncResult(S_OK)
    , m_hResolveThread(nullptr)
    , m_hResolveEvent(nullptr)
{
}

AsyncSourceResolver::~AsyncSourceResolver()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT AsyncSourceResolver::Initialize()
{
    m_hResolveEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!m_hResolveEvent)
        return HRESULT_FROM_WIN32(GetLastError());

    return CreateSourceResolver();
}

HRESULT AsyncSourceResolver::Shutdown()
{
    CancelResolve();

    if (m_hResolveThread)
    {
        WaitForSingleObject(m_hResolveThread, 5000);
        CloseHandle(m_hResolveThread);
        m_hResolveThread = nullptr;
    }

    if (m_hResolveEvent)
    {
        CloseHandle(m_hResolveEvent);
        m_hResolveEvent = nullptr;
    }

    m_spResolver = nullptr;
    m_spResolvedSource = nullptr;
    m_state = AsyncResolverIdle;

    return S_OK;
}

// ============================================================================
// Async resolve
// ============================================================================

HRESULT AsyncSourceResolver::BeginResolve(LPCWSTR pszUrl)
{
    if (!pszUrl)
        return E_POINTER;

    if (m_state == AsyncResolverResolving)
        return E_UNEXPECTED;

    m_strOriginalUrl = pszUrl;
    m_strResolvedUrl.Empty();
    m_hrAsyncResult = S_OK;
    m_resolvedInfo = ResolvedSourceInfo();
    m_spResolvedSource = nullptr;

    DetectProtocol(pszUrl);
    m_resolvedInfo.strProtocol = m_strProtocol;
    m_resolvedInfo.fIsStreaming = IsStreamingProtocol(m_strProtocol.GetString());

    SetState(AsyncResolverResolving);

    if (m_hResolveEvent)
        ResetEvent(m_hResolveEvent);

    m_hResolveThread = CreateThread(
        nullptr,
        0,
        ResolveThreadProc,
        this,
        0,
        nullptr);

    if (!m_hResolveThread)
    {
        m_hrAsyncResult = HRESULT_FROM_WIN32(GetLastError());
        SetState(AsyncResolverError);
        return m_hrAsyncResult;
    }

    return S_OK;
}

HRESULT AsyncSourceResolver::EndResolve(ResolvedSourceInfo* pInfo)
{
    if (m_state == AsyncResolverResolving)
    {
        if (m_hResolveEvent)
            WaitForSingleObject(m_hResolveEvent, 30000);
    }

    if (pInfo)
        *pInfo = m_resolvedInfo;

    return m_hrAsyncResult;
}

HRESULT AsyncSourceResolver::CancelResolve()
{
    if (m_state != AsyncResolverResolving)
        return S_OK;

    if (m_hResolveThread)
    {
        TerminateThread(m_hResolveThread, 0);
        CloseHandle(m_hResolveThread);
        m_hResolveThread = nullptr;
    }

    SetState(AsyncResolverIdle);
    return S_OK;
}

// ============================================================================
// Sync resolve
// ============================================================================

HRESULT AsyncSourceResolver::Resolve(LPCWSTR pszUrl, ResolvedSourceInfo* pInfo)
{
    HRESULT hr = BeginResolve(pszUrl);
    if (FAILED(hr))
        return hr;

    return EndResolve(pInfo);
}

// ============================================================================
// State
// ============================================================================

AsyncSourceResolverState AsyncSourceResolver::GetState() const throw()
{
    return m_state;
}

bool AsyncSourceResolver::IsResolving() const throw()
{
    return m_state == AsyncResolverResolving;
}

// ============================================================================
// URL info
// ============================================================================

ATL::CString AsyncSourceResolver::GetOriginalUrl() const
{
    return m_strOriginalUrl;
}

ATL::CString AsyncSourceResolver::GetResolvedUrl() const
{
    return m_strResolvedUrl;
}

// ============================================================================
// Result access
// ============================================================================

HRESULT AsyncSourceResolver::GetAsyncResult() const throw()
{
    return m_hrAsyncResult;
}

IMFMediaSource* AsyncSourceResolver::GetResolvedSource()
{
    if (m_spResolvedSource)
    {
        IMFMediaSource* pSource = m_spResolvedSource;
        pSource->AddRef();
        return pSource;
    }
    return nullptr;
}

// ============================================================================
// Protocol support
// ============================================================================

bool AsyncSourceResolver::IsStreamingProtocol(LPCWSTR pszProtocol)
{
    if (!pszProtocol)
        return false;

    if (_wcsicmp(pszProtocol, L"http") == 0 ||
        _wcsicmp(pszProtocol, L"https") == 0 ||
        _wcsicmp(pszProtocol, L"rtsp") == 0 ||
        _wcsicmp(pszProtocol, L"rtmp") == 0 ||
        _wcsicmp(pszProtocol, L"mms") == 0)
    {
        return true;
    }

    return false;
}

bool AsyncSourceResolver::IsSupportedScheme(LPCWSTR pszUrl)
{
    if (!pszUrl)
        return false;

    ATL::CString strProtocol, strHost, strPath;
    return SUCCEEDED(ParseUrl(pszUrl, &strProtocol, &strHost, &strPath));
}

HRESULT AsyncSourceResolver::ParseUrl(
    LPCWSTR pszUrl,
    ATL::CString* pstrProtocol,
    ATL::CString* pstrHost,
    ATL::CString* pstrPath)
{
    if (!pszUrl || !pstrProtocol || !pstrHost || !pstrPath)
        return E_POINTER;

    URL_COMPONENTSW components = {};
    components.dwStructSize = sizeof(components);

    WCHAR szProtocol[64] = {};
    WCHAR szHost[256] = {};
    WCHAR szPath[1024] = {};
    components.lpszScheme = szProtocol;
    components.dwSchemeLength = ARRAYSIZE(szProtocol);
    components.lpszHostName = szHost;
    components.dwHostNameLength = ARRAYSIZE(szHost);
    components.lpszUrlPath = szPath;
    components.dwUrlPathLength = ARRAYSIZE(szPath);

    if (!InternetCrackUrlW(pszUrl, 0, 0, &components))
        return HRESULT_FROM_WIN32(GetLastError());

    *pstrProtocol = szProtocol;
    *pstrHost = szHost;
    *pstrPath = szPath;

    return S_OK;
}

// ============================================================================
// Private helpers
// ============================================================================

DWORD WINAPI AsyncSourceResolver::ResolveThreadProc(LPVOID lpParam)
{
    AsyncSourceResolver* pThis = static_cast<AsyncSourceResolver*>(lpParam);
    if (pThis)
        pThis->ResolveOnBackgroundThread();
    return 0;
}

HRESULT AsyncSourceResolver::ResolveOnBackgroundThread()
{
    HRESULT hr = S_OK;

    if (m_spResolver)
    {
        PROPVARIANT var;
        PropVariantInit(&var);
        var.vt = VT_LPWSTR;
        var.pwszVal = const_cast<LPWSTR>(m_strOriginalUrl.GetString());

        IUnknown* pSource = nullptr;
        MF_OBJECT_TYPE objectType = MF_OBJECT_UNKNOWN;

        hr = m_spResolver->CreateObjectFromURL(
            m_strOriginalUrl.GetString(),
            MF_RESOLUTION_MEDIASOURCE,
            nullptr,
            &objectType,
            &pSource);

        if (SUCCEEDED(hr) && pSource)
        {
            hr = pSource->QueryInterface(IID_PPV_ARGS(&m_spResolvedSource));
            pSource->Release();

            if (SUCCEEDED(hr))
            {
                m_strResolvedUrl = m_strOriginalUrl;
                m_resolvedInfo.strResolvedUrl = m_strResolvedUrl;
                m_resolvedInfo.strMimeType = L"unknown";

                m_hrAsyncResult = S_OK;
                SetState(AsyncResolverResolved);
            }
            else
            {
                m_hrAsyncResult = hr;
                SetState(AsyncResolverError);
            }
        }
        else
        {
            m_hrAsyncResult = hr;
            SetState(AsyncResolverError);
        }

        PropVariantClear(&var);
    }
    else
    {
        m_hrAsyncResult = E_UNEXPECTED;
        SetState(AsyncResolverError);
    }

    if (m_hResolveEvent)
        SetEvent(m_hResolveEvent);

    if (m_completeCb)
        m_completeCb(m_hrAsyncResult, m_resolvedInfo);

    return m_hrAsyncResult;
}

HRESULT AsyncSourceResolver::CreateSourceResolver()
{
    return MFCreateSourceResolver(&m_spResolver);
}

void AsyncSourceResolver::DetectProtocol(LPCWSTR pszUrl)
{
    ATL::CString strProtocol, strHost, strPath;
    if (SUCCEEDED(ParseUrl(pszUrl, &strProtocol, &strHost, &strPath)))
        m_strProtocol = strProtocol;
}

void AsyncSourceResolver::SetState(AsyncSourceResolverState state)
{
    m_state = state;
}

} // namespace HMRAVSource
