// AuthProvider.cpp - DRM authentication provider implementation

#include "pch.h"
#include "AuthProvider.h"

namespace HMRAVSource
{

// ============================================================================
// AuthCredentials
// ============================================================================

AuthCredentials::AuthCredentials()
    : m_type(AuthCredentialNone)
    , m_llExpiryTimeHns(0)
{
}

AuthCredentials::AuthCredentials(const AuthCredentials& other)
    : m_type(other.m_type)
    , m_strUserName(other.m_strUserName)
    , m_strPassword(other.m_strPassword)
    , m_strToken(other.m_strToken)
    , m_strDomain(other.m_strDomain)
    , m_strUrl(other.m_strUrl)
    , m_llExpiryTimeHns(other.m_llExpiryTimeHns)
    , m_attributes(other.m_attributes)
{
}

AuthCredentials& AuthCredentials::operator=(const AuthCredentials& other)
{
    if (this != &other)
    {
        m_type = other.m_type;
        m_strUserName = other.m_strUserName;
        m_strPassword = other.m_strPassword;
        m_strToken = other.m_strToken;
        m_strDomain = other.m_strDomain;
        m_strUrl = other.m_strUrl;
        m_llExpiryTimeHns = other.m_llExpiryTimeHns;
        m_attributes = other.m_attributes;
    }
    return *this;
}

AuthCredentials::~AuthCredentials()
{
}

void AuthCredentials::SetUserName(LPCWSTR pszUserName)
{
    m_strUserName = pszUserName ? pszUserName : L"";
}

ATL::CString AuthCredentials::GetUserName() const
{
    return m_strUserName;
}

void AuthCredentials::SetPassword(LPCWSTR pszPassword)
{
    m_strPassword = pszPassword ? pszPassword : L"";
}

ATL::CString AuthCredentials::GetPassword() const
{
    return m_strPassword;
}

void AuthCredentials::SetToken(LPCWSTR pszToken)
{
    m_strToken = pszToken ? pszToken : L"";
}

ATL::CString AuthCredentials::GetToken() const
{
    return m_strToken;
}

void AuthCredentials::SetDomain(LPCWSTR pszDomain)
{
    m_strDomain = pszDomain ? pszDomain : L"";
}

ATL::CString AuthCredentials::GetDomain() const
{
    return m_strDomain;
}

void AuthCredentials::SetUrl(LPCWSTR pszUrl)
{
    m_strUrl = pszUrl ? pszUrl : L"";
}

ATL::CString AuthCredentials::GetUrl() const
{
    return m_strUrl;
}

void AuthCredentials::SetCredentialType(AuthCredentialType type)
{
    m_type = type;
}

AuthCredentialType AuthCredentials::GetCredentialType() const throw()
{
    return m_type;
}

HRESULT AuthCredentials::SetAttribute(LPCWSTR pszName, LPCWSTR pszValue)
{
    if (!pszName)
        return E_POINTER;

    m_attributes[pszName] = pszValue ? pszValue : L"";
    return S_OK;
}

HRESULT AuthCredentials::GetAttribute(LPCWSTR pszName, ATL::CString* pstrValue) const
{
    if (!pszName || !pstrValue)
        return E_POINTER;

    auto it = m_attributes.find(pszName);
    if (it != m_attributes.end())
    {
        *pstrValue = it->second;
        return S_OK;
    }

    *pstrValue = CString();
    return E_FAIL;
}

bool AuthCredentials::HasAttribute(LPCWSTR pszName) const
{
    if (!pszName)
        return false;
    return m_attributes.find(pszName) != m_attributes.end();
}

bool AuthCredentials::IsEmpty() const throw()
{
    return m_strUserName.IsEmpty() &&
           m_strPassword.IsEmpty() &&
           m_strToken.IsEmpty();
}

HRESULT AuthCredentials::Validate() const
{
    if (m_type == AuthCredentialNone)
        return E_INVALIDARG;

    if (m_type == AuthCredentialUserName && m_strUserName.IsEmpty())
        return E_INVALIDARG;

    return S_OK;
}

void AuthCredentials::SetExpiryTime(LONGLONG llExpiryTimeHns)
{
    m_llExpiryTimeHns = llExpiryTimeHns;
}

LONGLONG AuthCredentials::GetExpiryTime() const throw()
{
    return m_llExpiryTimeHns;
}

bool AuthCredentials::IsExpired() const throw()
{
    if (m_llExpiryTimeHns == 0)
        return false;

    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    LONGLONG llNow = (static_cast<LONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;

    return llNow > m_llExpiryTimeHns;
}

void AuthCredentials::Clear()
{
    m_type = AuthCredentialNone;
    m_strUserName.Empty();
    m_strPassword.Empty();
    m_strToken.Empty();
    m_strDomain.Empty();
    m_strUrl.Empty();
    m_llExpiryTimeHns = 0;
    m_attributes.clear();
}

// ============================================================================
// AuthProvider
// ============================================================================

AuthProvider::AuthProvider()
    : m_type(AuthProviderBasic)
    , m_fInitialized(false)
    , m_fAuthenticated(false)
    , m_fPending(false)
{
}

AuthProvider::~AuthProvider()
{
    Shutdown();
}

HRESULT AuthProvider::Initialize()
{
    m_fInitialized = true;
    return S_OK;
}

HRESULT AuthProvider::Shutdown()
{
    m_fInitialized = false;
    m_fAuthenticated = false;
    m_fPending = false;
    m_credentialCache.clear();
    return S_OK;
}

// ============================================================================
// Authentication
// ============================================================================

HRESULT AuthProvider::Authenticate(LPCWSTR pszUrl, AuthCredentials* pCredentials)
{
    if (!pszUrl || !pCredentials)
        return E_POINTER;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    m_fPending = true;

    // Check credential cache first
    auto it = m_credentialCache.find(pszUrl);
    if (it != m_credentialCache.end())
    {
        if (!it->second.IsExpired())
        {
            *pCredentials = it->second;
            m_fAuthenticated = true;
            m_fPending = false;

            if (m_completeCb)
                m_completeCb(S_OK, *pCredentials);

            return S_OK;
        }
    }

    HRESULT hr = E_FAIL;

    switch (m_type)
    {
    case AuthProviderBasic:
        hr = AuthenticateBasic(pszUrl, pCredentials);
        break;
    case AuthProviderOAuth:
        hr = AuthenticateOAuth(pszUrl, pCredentials);
        break;
    case AuthProviderToken:
        hr = AuthenticateToken(pszUrl, pCredentials);
        break;
    default:
        hr = E_NOTIMPL;
        break;
    }

    m_fPending = false;

    if (SUCCEEDED(hr))
    {
        m_fAuthenticated = true;
        StoreCredentials(pszUrl, *pCredentials);
    }

    if (m_completeCb)
        m_completeCb(hr, *pCredentials);

    return hr;
}

HRESULT AuthProvider::AuthenticateAsync(LPCWSTR pszUrl)
{
    if (!pszUrl)
        return E_POINTER;

    m_fPending = true;

    // Would spawn a background thread for async authentication
    AuthCredentials credentials;
    HRESULT hr = Authenticate(pszUrl, &credentials);
    m_fPending = false;
    return hr;
}

HRESULT AuthProvider::CancelAuthentication()
{
    m_fPending = false;
    return S_OK;
}

// ============================================================================
// Credential management
// ============================================================================

HRESULT AuthProvider::StoreCredentials(LPCWSTR pszUrl, const AuthCredentials& credentials)
{
    if (!pszUrl)
        return E_POINTER;

    m_credentialCache[pszUrl] = credentials;
    return S_OK;
}

HRESULT AuthProvider::RetrieveCredentials(LPCWSTR pszUrl, AuthCredentials* pCredentials)
{
    if (!pszUrl || !pCredentials)
        return E_POINTER;

    auto it = m_credentialCache.find(pszUrl);
    if (it != m_credentialCache.end())
    {
        *pCredentials = it->second;
        return S_OK;
    }

    return E_FAIL;
}

HRESULT AuthProvider::RemoveCredentials(LPCWSTR pszUrl)
{
    if (!pszUrl)
        return E_POINTER;

    m_credentialCache.erase(pszUrl);
    return S_OK;
}

HRESULT AuthProvider::ClearAllCredentials()
{
    m_credentialCache.clear();
    return S_OK;
}

// ============================================================================
// Provider type
// ============================================================================

void AuthProvider::SetProviderType(AuthProviderType type)
{
    m_type = type;
}

AuthProviderType AuthProvider::GetProviderType() const throw()
{
    return m_type;
}

// ============================================================================
// DRM support
// ============================================================================

HRESULT AuthProvider::CheckDRMStatus(LPCWSTR /*pszUrl*/, bool* pfProtected, bool* pfLicensed)
{
    if (pfProtected) *pfProtected = false;
    if (pfLicensed) *pfLicensed = false;
    return S_OK;
}

HRESULT AuthProvider::AcquireLicense(LPCWSTR /*pszUrl*/)
{
    return E_NOTIMPL;
}

HRESULT AuthProvider::RequestPlayReadyLicense(LPCWSTR /*pszContentUrl*/)
{
    return E_NOTIMPL;
}

// ============================================================================
// Status
// ============================================================================

bool AuthProvider::IsAuthenticated() const throw()
{
    return m_fAuthenticated;
}

bool AuthProvider::IsAuthenticationPending() const throw()
{
    return m_fPending;
}

ATL::CString AuthProvider::GetLastError() const
{
    return m_strLastError;
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT AuthProvider::AuthenticateBasic(LPCWSTR /*pszUrl*/, AuthCredentials* pCredentials)
{
    if (m_promptCb)
    {
        m_promptCb(L"", pCredentials);
        return S_OK;
    }
    return E_NOTIMPL;
}

HRESULT AuthProvider::AuthenticateOAuth(LPCWSTR /*pszUrl*/, AuthCredentials* /*pCredentials*/)
{
    return E_NOTIMPL;
}

HRESULT AuthProvider::AuthenticateToken(LPCWSTR /*pszUrl*/, AuthCredentials* /*pCredentials*/)
{
    return E_NOTIMPL;
}

void AuthProvider::SetLastError(LPCWSTR pszError)
{
    m_strLastError = pszError ? pszError : L"";
}

} // namespace HMRAVSource
