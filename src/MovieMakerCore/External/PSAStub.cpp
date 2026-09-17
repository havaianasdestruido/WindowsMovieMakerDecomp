#include "pch.h"

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
#include <wincrypt.h>

// ============================================================================
// DPAPI helpers: tokens are persisted to %LOCALAPPDATA%\...\auth.dat as a
// single blob encrypted with CryptProtectData (current-user scope). Plaintext
// credentials never touch the disk directly.
// ============================================================================

namespace
{
    // Magic + version guard for the persisted auth store file.
    const DWORD kAuthStoreMagic = 0x574C4D52; // 'WLMR'
    const DWORD kAuthStoreVersion = 1;

    struct AuthStoreHeader
    {
        DWORD dwMagic;
        DWORD dwVersion;
        DWORD dwDataSize;
    };

    // Serialize the token table to an opaque byte vector (UTF-16 lengths +
    // payload per entry, length-prefixed to avoid ambiguities).
    std::vector<BYTE> SerializeTokens(const std::vector<PSAAuthenticationStore::TokenEntry>& tokens)
    {
        std::vector<BYTE> blob;
        const DWORD dwCount = static_cast<DWORD>(tokens.size());
        const BYTE* pCount = reinterpret_cast<const BYTE*>(&dwCount);
        blob.insert(blob.end(), pCount, pCount + sizeof(dwCount));

        for (const auto& token : tokens)
        {
            const std::wstring fields[] = {
                token.strServiceName.GetString(),
                token.strTokenName.GetString(),
                token.strTokenValue.GetString()
            };
            for (const auto& field : fields)
            {
                const DWORD dwLen = static_cast<DWORD>(field.size());
                const BYTE* pLen = reinterpret_cast<const BYTE*>(&dwLen);
                blob.insert(blob.end(), pLen, pLen + sizeof(dwLen));
                const BYTE* pData = reinterpret_cast<const BYTE*>(field.c_str());
                blob.insert(blob.end(), pData, pData + dwLen * sizeof(wchar_t));
            }
        }
        return blob;
    }

    bool DeserializeTokens(const std::vector<BYTE>& blob,
        std::vector<PSAAuthenticationStore::TokenEntry>& tokens)
    {
        size_t pos = 0;
        if (blob.size() < sizeof(DWORD))
            return false;

        DWORD dwCount = 0;
        memcpy(&dwCount, blob.data(), sizeof(dwCount));
        pos += sizeof(dwCount);

        // Bounded token count: refuse absurd tables.
        if (dwCount > 4096)
            return false;

        tokens.clear();
        tokens.reserve(dwCount);

        for (DWORD i = 0; i < dwCount; ++i)
        {
            PSAAuthenticationStore::TokenEntry entry;
            std::wstring fields[3];
            for (size_t f = 0; f < 3; ++f)
            {
                if (blob.size() - pos < sizeof(DWORD))
                    return false;

                DWORD dwLen = 0;
                memcpy(&dwLen, blob.data() + pos, sizeof(dwLen));
                pos += sizeof(dwLen);

                // String length sanity guard against corrupted/hostile data.
                if (dwLen > 65536 || blob.size() - pos < static_cast<size_t>(dwLen) * sizeof(wchar_t))
                    return false;

                fields[f].assign(
                    reinterpret_cast<const wchar_t*>(blob.data() + pos), dwLen);
                pos += static_cast<size_t>(dwLen) * sizeof(wchar_t);
            }

            entry.strServiceName = fields[0].c_str();
            entry.strTokenName = fields[1].c_str();
            entry.strTokenValue = fields[2].c_str();
            tokens.push_back(entry);
        }
        return true;
    }
} // namespace

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

    const std::vector<BYTE> plain = SerializeTokens(m_tokens);
    if (plain.empty())
    {
        // Nothing to persist; remove any stale auth file.
        ::DeleteFileW(m_strStoragePath);
        return S_OK;
    }

    DATA_BLOB in = {};
    in.pbData = const_cast<BYTE*>(plain.data());
    in.cbData = static_cast<DWORD>(plain.size());

    DATA_BLOB out = {};
    if (!::CryptProtectData(&in, L"WindowsLiveAuthenticationStore", nullptr,
        nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out))
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    HRESULT hr = E_FAIL;

    HANDLE hFile = ::CreateFileW(m_strStoragePath, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    else
    {
        AuthStoreHeader header = {};
        header.dwMagic = kAuthStoreMagic;
        header.dwVersion = kAuthStoreVersion;
        header.dwDataSize = out.cbData;

        DWORD dwWritten = 0;
        BOOL bOk = ::WriteFile(hFile, &header, sizeof(header), &dwWritten, nullptr);
        if (bOk && dwWritten == sizeof(header))
        {
            bOk = ::WriteFile(hFile, out.pbData, out.cbData, &dwWritten, nullptr);
            if (bOk && dwWritten == out.cbData)
                hr = S_OK;
        }
        if (FAILED(hr))
            hr = HRESULT_FROM_WIN32(::GetLastError());

        ::CloseHandle(hFile);
    }

    ::LocalFree(out.pbData);
    return hr;
}

HRESULT PSAAuthenticationStore::Load()
{
    if (m_strStoragePath.IsEmpty())
        return E_UNEXPECTED;

    m_tokens.clear();

    HANDLE hFile = ::CreateFileW(m_strStoragePath, GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        // First run or no persisted store: treat as clean state.
        return S_OK;
    }

    HRESULT hr = E_FAIL;

    AuthStoreHeader header = {};
    DWORD dwRead = 0;
    if (::ReadFile(hFile, &header, sizeof(header), &dwRead, nullptr) &&
        dwRead == sizeof(header) &&
        header.dwMagic == kAuthStoreMagic &&
        header.dwVersion == kAuthStoreVersion &&
        header.dwDataSize > 0 &&
        header.dwDataSize <= 64 * 1024 * 1024)
    {
        std::vector<BYTE> encrypted(header.dwDataSize);
        if (::ReadFile(hFile, encrypted.data(), header.dwDataSize, &dwRead, nullptr) &&
            dwRead == header.dwDataSize)
        {
            DATA_BLOB in = {};
            in.pbData = encrypted.data();
            in.cbData = header.dwDataSize;

            DATA_BLOB out = {};
            if (::CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr,
                CRYPTPROTECT_UI_FORBIDDEN, &out))
            {
                std::vector<BYTE> plain(out.pbData, out.pbData + out.cbData);
                hr = DeserializeTokens(plain, m_tokens) ? S_OK : E_FAIL;
                ::LocalFree(out.pbData);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
            }
        }
    }

    ::CloseHandle(hFile);
    return hr;
}
