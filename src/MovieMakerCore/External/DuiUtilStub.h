/*
 * DuiUtilStub.h
 *
 * Lightweight stubs for DirectUI utility classes referenced by MovieMakerCore.
 *
 * HcdpiParser: DPI-aware resource parsing helper.
 * HcdpiResourceCache: DPI-aware resource caching.
 *
 * RTTI: ?AVHcdpiParser@@, ?AVHcdpiResourceCache@@
 *
 * These classes are normally provided by DuiUtil.dll. These stubs provide
 * the RTTI signatures and minimal implementations needed for linking.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef DUIUTIL_STUB_H
#define DUIUTIL_STUB_H

#include "../pch.h"

// ============================================================================
// HcdpiParser (Forward declaration stub)
// ============================================================================
// DPI-aware resource parser. Parses DUI resource files with DPI-specific
// scaling values. In the full implementation, this would:
//   - Read DPI scale factors from manifest or registry
//   - Parse resource strings with %dpi% substitution tokens
//   - Provide scaled dimension values for layout
//
class HcdpiParser
{
public:
    HcdpiParser();
    ~HcdpiParser();

    // -- DPI information --
    static UINT GetSystemDpi();
    static float GetDpiScaleFactor();
    static UINT GetDpiFromD2D(ID2D1RenderTarget* pRT);

    // -- Resource parsing --
    HRESULT ParseResourceString(LPCWSTR pszInput, ATL::CString& strOutput);
    HRESULT ParseDimension(LPCWSTR pszInput, float* pflOutput);
    HRESULT ParseRect(LPCWSTR pszInput, RECT* pOutRect);

    // -- Raw PPI/DPI --
    static HDC GetDesktopDC();
    static UINT GetDeviceCapsDpi(HDC hdc, bool bHorizontal);
};

// ============================================================================
// HcdpiResourceCache (Forward declaration stub)
// ============================================================================
// DPI-aware resource cache. Caches parsed resources indexed by DPI value
// so that UI elements are re-created at the correct scale when DPI changes
// (e.g., when the window is moved between monitors with different DPIs).
//
class HcdpiResourceCache
{
public:
    HcdpiResourceCache();
    ~HcdpiResourceCache();

    // -- Cache management --
    HRESULT AddResource(LPCWSTR pszKey, IUnknown* pResource, UINT uDpi);
    HRESULT GetResource(LPCWSTR pszKey, UINT uDpi, IUnknown** ppResource);
    HRESULT RemoveResource(LPCWSTR pszKey);
    void    ClearCache();

    // -- DPI change handling --
    HRESULT OnDpiChanged(UINT uNewDpi);
    UINT    GetCurrentDpi() const throw();

    // -- Size --
    size_t GetCachedCount() const throw();

private:
    struct CacheEntry
    {
        ATL::CString  strKey;
        ATL::CComPtr<IUnknown> spResource;
        UINT          uDpi;
    };

    std::vector<CacheEntry> m_cache;
    UINT                    m_uCurrentDpi;
};

#endif // DUIUTIL_STUB_H
