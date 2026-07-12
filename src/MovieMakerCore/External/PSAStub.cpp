/*
 * PSAStub.cpp
 *
 * Implementation of PSAAuthenticationStore stub.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "PSAStub.h"
#include <shlobj.h>

// ============================================================================
// PSAAuthenticationStore implementation
// ============================================================================

PSAAuthenticationStore::PSAAuthenticationStore()
{
    // Default storage path: %LOCALAPPDATA%\Microsoft\WL\MovieMaker\auth.dat
    WCHAR szPath[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath)))
    {
        m_strStoragePath = szPath;
        m_strStoragePath += L"\\Microsoft\\WL\\MovieMaker\\auth.dat";
    }
}

PSAAuthenticationStore::~PSAAuthenticationStore()
{
}

HRESULT PSAAuthenticationStore::RegisterService(LPCWSTR pszServiceName)
{
    if (!pszServiceName)
        return E_POINTER;

    // Check if already registered
    for (size_t i = 0; i < m_tokens.size(); i++)
    {
        if (m_tokens[i].strServiceName.CompareNoCase(pszServiceName) == 0)
            return S_FALSE;
    }

    return S_OK;
}

HRESULT PSAAuthenticationStore::UnregisterService(LPCWSTR pszServiceName)
{
    if (!pszServiceName)
        return E_POINTER;

    auto it = m_tokens.begin();
    while (it != m_tokens.end())
    {
        if (it->strServiceName.CompareNoCase(pszServiceName) == 0)
            it = m_tokens.erase(it);
        else
            ++it;
    }

    return S_OK;
}

HRESULT PSAAuthenticationStore::StoreToken(LPCWSTR pszServiceName, LPCWSTR pszTokenName, LPCWSTR pszTokenValue)
{
    if (!pszServiceName || !pszTokenName || !pszTokenValue)
        return E_POINTER;

    // Check for existing token
    for (auto& token : m_tokens)
    {
        if (token.strServiceName.CompareNoCase(pszServiceName) == 0 &&
            token.strTokenName.CompareNoCase(pszTokenName) == 0)
        {
            token.strTokenValue = pszTokenValue;
            return S_OK;
        }
    }

    TokenEntry entry;
    entry.strServiceName = pszServiceName;
    entry.strTokenName = pszTokenName;
    entry.strTokenValue = pszTokenValue;
    m_tokens.push_back(entry);

    return S_OK;
}

HRESULT PSAAuthenticationStore::RetrieveToken(LPCWSTR pszServiceName, LPCWSTR pszTokenName, ATL::CString& strOutToken)
{
    if (!pszServiceName || !pszTokenName)
        return E_POINTER;

    strOutToken.Empty();

    for (const auto& token : m_tokens)
    {
        if (token.strServiceName.CompareNoCase(pszServiceName) == 0 &&
            token.strTokenName.CompareNoCase(pszTokenName) == 0)
        {
            strOutToken = token.strTokenValue;
            return S_OK;
        }
    }

    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT PSAAuthenticationStore::DeleteToken(LPCWSTR pszServiceName, LPCWSTR pszTokenName)
{
    if (!pszServiceName || !pszTokenName)
        return E_POINTER;

    for (auto it = m_tokens.begin(); it != m_tokens.end(); ++it)
    {
        if (it->strServiceName.CompareNoCase(pszServiceName) == 0 &&
            it->strTokenName.CompareNoCase(pszTokenName) == 0)
        {
            m_tokens.erase(it);
            return S_OK;
        }
    }

    return S_FALSE;
}

HRESULT PSAAuthenticationStore::ClearTokens(LPCWSTR pszServiceName)
{
    if (!pszServiceName)
        return E_POINTER;

    auto it = m_tokens.begin();
    while (it != m_tokens.end())
    {
        if (it->strServiceName.CompareNoCase(pszServiceName) == 0)
            it = m_tokens.erase(it);
        else
            ++it;
    }

    return S_OK;
}

bool PSAAuthenticationStore::IsAuthenticated(LPCWSTR pszServiceName) const
{
    if (!pszServiceName)
        return false;

    for (const auto& token : m_tokens)
    {
        if (token.strServiceName.CompareNoCase(pszServiceName) == 0 &&
            !token.strTokenValue.IsEmpty())
        {
            return true;
        }
    }

    return false;
}

HRESULT PSAAuthenticationStore::InvalidateAuthentication(LPCWSTR pszServiceName)
{
    return ClearTokens(pszServiceName);
}

size_t PSAAuthenticationStore::GetRegisteredServiceCount() const throw()
{
    // Count unique service names
    std::set<ATL::CString> uniqueServices;
    for (const auto& token : m_tokens)
        uniqueServices.insert(token.strServiceName);
    return uniqueServices.size();
}

ATL::CString PSAAuthenticationStore::GetRegisteredServiceAt(size_t nIndex) const
{
    std::set<ATL::CString> uniqueServices;
    for (const auto& token : m_tokens)
        uniqueServices.insert(token.strServiceName);

    if (nIndex < uniqueServices.size())
    {
        auto it = uniqueServices.begin();
        std::advance(it, nIndex);
        return *it;
    }

    return L"";
}

HRESULT PSAAuthenticationStore::Save()
{
    if (m_strStoragePath.IsEmpty())
        return E_UNEXPECTED;

    // In the full implementation, this would encrypt and persist tokens
    // to the storage file using DPAPI or Credential Manager.

    return S_OK;
}

HRESULT PSAAuthenticationStore::Load()
{
    if (m_strStoragePath.IsEmpty())
        return E_UNEXPECTED;

    m_tokens.clear();
    return S_OK;
}
