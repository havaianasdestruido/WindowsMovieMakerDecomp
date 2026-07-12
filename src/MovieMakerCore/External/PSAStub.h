/*
 * PSAStub.h
 *
 * Lightweight stub for Push-to-Service Authentication (PSA) classes.
 *
 * PSAAuthenticationStore: Authentication store for online service credentials.
 *
 * RTTI: ?AVPSAAuthenticationStore@@
 *
 * This class is normally provided by the Windows Live PSA (Push-to-Service
 * Authentication) library. This stub provides the RTTI signature and
 * minimal implementation needed for linking.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef PSA_STUB_H
#define PSA_STUB_H

#include "../pch.h"

// ============================================================================
// PSAAuthenticationStore
// ============================================================================
// Stores authentication tokens and credentials for online publish services
// (YouTube, Facebook, SkyDrive). Provides secure storage and retrieval of
// OAuth tokens, session secrets, and service credentials.
//
class PSAAuthenticationStore
{
public:
    PSAAuthenticationStore();
    ~PSAAuthenticationStore();

    // -- Service registration --
    HRESULT RegisterService(LPCWSTR pszServiceName);
    HRESULT UnregisterService(LPCWSTR pszServiceName);

    // -- Token storage --
    HRESULT StoreToken(LPCWSTR pszServiceName, LPCWSTR pszTokenName, LPCWSTR pszTokenValue);
    HRESULT RetrieveToken(LPCWSTR pszServiceName, LPCWSTR pszTokenName, ATL::CString& strOutToken);
    HRESULT DeleteToken(LPCWSTR pszServiceName, LPCWSTR pszTokenName);
    HRESULT ClearTokens(LPCWSTR pszServiceName);

    // -- Authentication state --
    bool IsAuthenticated(LPCWSTR pszServiceName) const;
    HRESULT InvalidateAuthentication(LPCWSTR pszServiceName);

    // -- List services --
    size_t GetRegisteredServiceCount() const throw();
    ATL::CString GetRegisteredServiceAt(size_t nIndex) const;

    // -- Persistence --
    HRESULT Save();
    HRESULT Load();

private:
    struct TokenEntry
    {
        ATL::CString strServiceName;
        ATL::CString strTokenName;
        ATL::CString strTokenValue;
    };

    std::vector<TokenEntry> m_tokens;
    ATL::CString            m_strStoragePath;
};

#endif // PSA_STUB_H
