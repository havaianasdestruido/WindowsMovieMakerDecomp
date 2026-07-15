/*
 * AuthProvider.h
 *
 * AuthProvider - DRM authentication provider for protected content.
 * Handles credential management and authentication for protected media
 * sources that require license acquisition or user authentication.
 *
 * AuthCredentials - Authentication credential container.
 *
 * RTTI: ?AVAuthProvider@@, ?AVAuthCredentials@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AUTHPROVIDER_H
#define AUTHPROVIDER_H

#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// AuthCredentialType
// ============================================================================
enum AuthCredentialType
{
    AuthCredentialNone          = 0,
    AuthCredentialUserName      = 1,
    AuthCredentialPassword      = 2,
    AuthCredentialToken         = 3,
    AuthCredentialCertificate   = 4,
    AuthCredentialOAuth         = 5
};

// ============================================================================
// AuthProviderType
// ============================================================================
enum AuthProviderType
{
    AuthProviderNone        = 0,
    AuthProviderBasic       = 1,    // Username/password
    AuthProviderOAuth       = 2,    // OAuth 2.0
    AuthProviderCertificate = 3,    // Certificate-based
    AuthProviderToken       = 4     // Token-based
};

// ============================================================================
// AuthCredentials
// ============================================================================
// Container for authentication credentials. Holds username, password,
// tokens, or certificate data needed to authenticate with a protected
// media source.
//
class AVSOURCE_API AuthCredentials
{
public:
    AuthCredentials();
    AuthCredentials(const AuthCredentials& other);
    AuthCredentials& operator=(const AuthCredentials& other);
    ~AuthCredentials();

    // Credential data
    void SetUserName(LPCWSTR pszUserName);
    ATL::CString GetUserName() const;

    void SetPassword(LPCWSTR pszPassword);
    ATL::CString GetPassword() const;

    void SetToken(LPCWSTR pszToken);
    ATL::CString GetToken() const;

    void SetDomain(LPCWSTR pszDomain);
    ATL::CString GetDomain() const;

    void SetUrl(LPCWSTR pszUrl);
    ATL::CString GetUrl() const;

    // Type
    void SetCredentialType(AuthCredentialType type);
    AuthCredentialType GetCredentialType() const throw();

    // Custom attributes
    HRESULT SetAttribute(LPCWSTR pszName, LPCWSTR pszValue);
    HRESULT GetAttribute(LPCWSTR pszName, ATL::CString* pstrValue) const;
    bool HasAttribute(LPCWSTR pszName) const;

    // Validation
    bool IsEmpty() const throw();
    HRESULT Validate() const;

    // Expiry
    void SetExpiryTime(LONGLONG llExpiryTimeHns);
    LONGLONG GetExpiryTime() const throw();
    bool IsExpired() const throw();

    // Clear
    void Clear();

private:
    AuthCredentialType  m_type;
    ATL::CString        m_strUserName;
    ATL::CString        m_strPassword;
    ATL::CString        m_strToken;
    ATL::CString        m_strDomain;
    ATL::CString        m_strUrl;
    LONGLONG            m_llExpiryTimeHns;

    std::map<ATL::CString, ATL::CString> m_attributes;
};

// ============================================================================
// AuthProvider
// ============================================================================
// DRM authentication provider. Manages credential storage and authentication
// for protected media content. Supports multiple authentication methods
// and credential caching for seamless media access.
//
class AVSOURCE_API AuthProvider
{
public:
    AuthProvider();
    virtual ~AuthProvider();

    // Lifecycle
    HRESULT Initialize();
    HRESULT Shutdown();

    // Authentication
    HRESULT Authenticate(LPCWSTR pszUrl, AuthCredentials* pCredentials);
    HRESULT AuthenticateAsync(LPCWSTR pszUrl);
    HRESULT CancelAuthentication();

    // Credential management
    HRESULT StoreCredentials(LPCWSTR pszUrl, const AuthCredentials& credentials);
    HRESULT RetrieveCredentials(LPCWSTR pszUrl, AuthCredentials* pCredentials);
    HRESULT RemoveCredentials(LPCWSTR pszUrl);
    HRESULT ClearAllCredentials();

    // Provider type
    void SetProviderType(AuthProviderType type);
    AuthProviderType GetProviderType() const throw();

    // DRM support
    HRESULT CheckDRMStatus(LPCWSTR pszUrl, bool* pfProtected, bool* pfLicensed);
    HRESULT AcquireLicense(LPCWSTR pszUrl);
    HRESULT RequestPlayReadyLicense(LPCWSTR pszContentUrl);

    // Status
    bool IsAuthenticated() const throw();
    bool IsAuthenticationPending() const throw();
    ATL::CString GetLastError() const;

    // Callbacks
    using AuthCompleteCallback = std::function<void(HRESULT, const AuthCredentials&)>;
    using AuthPromptCallback = std::function<void(LPCWSTR, AuthCredentials*)>;
    void SetAuthCompleteCallback(AuthCompleteCallback cb) { m_completeCb = cb; }
    void SetAuthPromptCallback(AuthPromptCallback cb) { m_promptCb = cb; }

private:
    AuthProviderType        m_type;
    bool                    m_fInitialized;
    bool                    m_fAuthenticated;
    bool                    m_fPending;
    ATL::CString            m_strLastError;

    std::map<ATL::CString, AuthCredentials> m_credentialCache;

    AuthCompleteCallback    m_completeCb;
    AuthPromptCallback      m_promptCb;

    HRESULT AuthenticateBasic(LPCWSTR pszUrl, AuthCredentials* pCredentials);
    HRESULT AuthenticateOAuth(LPCWSTR pszUrl, AuthCredentials* pCredentials);
    HRESULT AuthenticateToken(LPCWSTR pszUrl, AuthCredentials* pCredentials);
    void SetLastError(LPCWSTR pszError);
};

} // namespace HMRAVSource

#endif // AUTHPROVIDER_H
