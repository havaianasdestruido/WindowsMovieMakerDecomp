/*
 * DuiUtilStub.cpp
 *
 * Implementation of HcdpiParser and HcdpiResourceCache stubs.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "DuiUtilStub.h"

// ============================================================================
// HcdpiParser implementation
// ============================================================================

HcdpiParser::HcdpiParser()
{
}

HcdpiParser::~HcdpiParser()
{
}

UINT HcdpiParser::GetSystemDpi()
{
    HDC hdc = GetDC(nullptr);
    UINT dpi = static_cast<UINT>(GetDeviceCaps(hdc, LOGPIXELSY));
    ReleaseDC(nullptr, hdc);
    return dpi;
}

float HcdpiParser::GetDpiScaleFactor()
{
    return static_cast<float>(GetSystemDpi()) / 96.0f;
}

UINT HcdpiParser::GetDpiFromD2D(ID2D1RenderTarget* pRT)
{
    if (!pRT)
        return 96;

    FLOAT dpiX, dpiY;
    pRT->GetDpi(&dpiX, &dpiY);
    return static_cast<UINT>(dpiY);
}

HRESULT HcdpiParser::ParseResourceString(LPCWSTR pszInput, ATL::CString& strOutput)
{
    if (!pszInput)
        return E_POINTER;

    strOutput = pszInput;

    // In the full implementation, this would replace %dpi% tokens with
    // the current DPI scale factor, and handle other DPI-specific tokens.

    return S_OK;
}

HRESULT HcdpiParser::ParseDimension(LPCWSTR pszInput, float* pflOutput)
{
    if (!pszInput || !pflOutput)
        return E_POINTER;

    *pflOutput = static_cast<float>(_wtof(pszInput));

    // Apply DPI scaling
    *pflOutput *= GetDpiScaleFactor();

    return S_OK;
}

HRESULT HcdpiParser::ParseRect(LPCWSTR pszInput, RECT* pOutRect)
{
    if (!pszInput || !pOutRect)
        return E_POINTER;

    // Parse "left,top,right,bottom" format
    int vals[4] = { 0 };
    int parsed = swscanf_s(pszInput, L"%d,%d,%d,%d", &vals[0], &vals[1], &vals[2], &vals[3]);

    if (parsed != 4)
        return E_INVALIDARG;

    float scale = GetDpiScaleFactor();

    pOutRect->left = static_cast<LONG>(vals[0] * scale);
    pOutRect->top = static_cast<LONG>(vals[1] * scale);
    pOutRect->right = static_cast<LONG>(vals[2] * scale);
    pOutRect->bottom = static_cast<LONG>(vals[3] * scale);

    return S_OK;
}

HDC HcdpiParser::GetDesktopDC()
{
    return GetDC(nullptr);
}

UINT HcdpiParser::GetDeviceCapsDpi(HDC hdc, bool bHorizontal)
{
    if (!hdc)
        return 96;

    return static_cast<UINT>(GetDeviceCaps(hdc, bHorizontal ? LOGPIXELSX : LOGPIXELSY));
}

// ============================================================================
// HcdpiResourceCache implementation
// ============================================================================

HcdpiResourceCache::HcdpiResourceCache()
    : m_uCurrentDpi(96)
{
    m_uCurrentDpi = HcdpiParser::GetSystemDpi();
}

HcdpiResourceCache::~HcdpiResourceCache()
{
    ClearCache();
}

HRESULT HcdpiResourceCache::AddResource(LPCWSTR pszKey, IUnknown* pResource, UINT uDpi)
{
    if (!pszKey || !pResource)
        return E_POINTER;

    CacheEntry entry;
    entry.strKey = pszKey;
    entry.spResource = pResource;
    entry.uDpi = uDpi;

    // Remove existing entry with same key and DPI
    for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
    {
        if (it->strKey.CompareNoCase(pszKey) == 0 && it->uDpi == uDpi)
        {
            m_cache.erase(it);
            break;
        }
    }

    m_cache.push_back(entry);
    return S_OK;
}

HRESULT HcdpiResourceCache::GetResource(LPCWSTR pszKey, UINT uDpi, IUnknown** ppResource)
{
    if (!pszKey || !ppResource)
        return E_POINTER;

    *ppResource = nullptr;

    for (const auto& entry : m_cache)
    {
        if (entry.strKey.CompareNoCase(pszKey) == 0 && entry.uDpi == uDpi)
        {
            *ppResource = entry.spResource;
            if (*ppResource)
                (*ppResource)->AddRef();
            return S_OK;
        }
    }

    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT HcdpiResourceCache::RemoveResource(LPCWSTR pszKey)
{
    if (!pszKey)
        return E_POINTER;

    for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
    {
        if (it->strKey.CompareNoCase(pszKey) == 0)
        {
            m_cache.erase(it);
            return S_OK;
        }
    }

    return S_FALSE;
}

void HcdpiResourceCache::ClearCache()
{
    m_cache.clear();
}

HRESULT HcdpiResourceCache::OnDpiChanged(UINT uNewDpi)
{
    m_uCurrentDpi = uNewDpi;

    // In the full implementation, this would invalidate cached resources
    // that need to be re-created at the new DPI.

    return S_OK;
}

UINT HcdpiResourceCache::GetCurrentDpi() const throw()
{
    return m_uCurrentDpi;
}

size_t HcdpiResourceCache::GetCachedCount() const throw()
{
    return m_cache.size();
}
