/*
 * SundanceAddInManager.h
 *
 * Add-in / plugin loader for MovieMakerCore.dll ("Sundance").
 *
 * SundanceAddInManager is the host-side counterpart to the contract in
 * SundanceAddInContract.h. It:
 *
 *   1. Scans a plugin directory for *.dll files.
 *   2. Loads each DLL with LoadLibraryW.
 *   3. Queries the add-in entry point (SundanceAddInCreateAddIn, with a
 *      fallback to the legacy SundanceAddInInitialize symbol).
 *   4. Negotiates the versioned contract (SundanceAddInContract).
 *   5. Instantiates the add-in (ISundanceAddIn) and calls OnInitialize.
 *   6. Tracks loaded add-ins and their modules so OnShutdown + FreeLibrary
 *      can be driven deterministically at application shutdown.
 *
 * The implementation is header-only (every member is inline) because the
 * file is not registered in CMakeLists.txt -- the host simply includes
 * this header from SundanceAppMain.cpp. Keeping it header-only preserves
 * the existing build graph untouched while still providing a real,
 * self-contained loader.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 * C++14, x86 Win32. No dependency on app-internal types; the loader talks
 * to the app only through ISundanceAddInHost.
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_ADDIN_MANAGER_H
#define SUNDANCE_ADDIN_MANAGER_H

#include "SundanceAddInContract.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <new>
#include <vector>
#include <cstddef>

// ============================================================================
// SundanceAddInManager
// ============================================================================
// Owns nothing but the list of loaded add-ins. The ISundanceAddInHost
// pointer passed to Initialize must remain valid until Shutdown() is
// called (the app heap-allocates and owns the host object).
// ============================================================================
class SundanceAddInManager
{
public:
    SundanceAddInManager() throw()
        : m_pHost(NULL)
    {
    }

    ~SundanceAddInManager() throw()
    {
        Shutdown();
    }

    // Binds the host. Must be called before any load operations.
    inline HRESULT Initialize(ISundanceAddInHost* pHost) throw()
    {
        m_pHost = pHost;
        return S_OK;
    }

    // Loads every add-in DLL found in pszDirectory. Returns S_OK if at
    // least one add-in loaded, S_FALSE if the directory contained no
    // loadable add-ins, or an error HRESULT on catastrophic failure.
    inline HRESULT LoadFromDirectory(LPCWSTR pszDirectory)
    {
        if (!pszDirectory || !pszDirectory[0])
            return E_INVALIDARG;
        if (!m_pHost)
            return E_UNEXPECTED;

        WCHAR szPattern[MAX_PATH] = { 0 };
        if (FAILED(::StringCchCopyW(szPattern, MAX_PATH, pszDirectory)))
            return E_INVALIDARG;
        if (!::PathAppendW(szPattern, L"*.dll"))
            return E_INVALIDARG;

        WIN32_FIND_DATAW fd = { 0 };
        HANDLE hFind = ::FindFirstFileW(szPattern, &fd);
        if (hFind == INVALID_HANDLE_VALUE)
            return S_OK; // No add-ins found — not an error

        DWORD cLoaded = 0;
        do
        {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            WCHAR szDllPath[MAX_PATH] = { 0 };
            if (FAILED(::StringCchCopyW(szDllPath, MAX_PATH, pszDirectory)))
                continue;
            if (!::PathAppendW(szDllPath, fd.cFileName))
                continue;

            if (SUCCEEDED(LoadSingleDll(szDllPath)))
                ++cLoaded;

        } while (::FindNextFileW(hFind, &fd));

        ::FindClose(hFind);

        return cLoaded ? S_OK : S_FALSE;
    }

    // Loads add-ins from the default per-user directory:
    //   %LOCALAPPDATA%\Microsoft\Windows Live\Movie Maker\AddIns
    // The directory is created on first run. Non-fatal failures return
    // S_OK so the app can start without add-ins.
    inline HRESULT LoadFromDefaultDirectory()
    {
        WCHAR szLocalAppData[MAX_PATH] = { 0 };
        HRESULT hr = ::SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szLocalAppData);
        if (FAILED(hr))
            return S_OK; // Non-fatal: app can run without add-ins

        WCHAR szAddInsDir[MAX_PATH] = { 0 };
        if (FAILED(::StringCchCopyW(szAddInsDir, MAX_PATH, szLocalAppData)))
            return E_INVALIDARG;
        if (!::PathAppendW(szAddInsDir, L"Microsoft\\Windows Live\\Movie Maker\\AddIns"))
            return E_INVALIDARG;

        ::CreateDirectoryW(szAddInsDir, NULL);

        return LoadFromDirectory(szAddInsDir);
    }

    // Unloads every add-in: ISundanceAddIn::OnShutdown first, then
    // FreeLibrary, in reverse load order. Safe to call multiple times.
    inline void Shutdown()
    {
        for (size_t i = m_entries.size(); i > 0; --i)
        {
            if (m_entries[i - 1].pAddIn)
                m_entries[i - 1].pAddIn->OnShutdown();
        }

        for (size_t i = m_entries.size(); i > 0; --i)
        {
            if (m_entries[i - 1].hModule)
                ::FreeLibrary(m_entries[i - 1].hModule);
        }

        m_entries.clear();
    }

    // -- Host -> add-in event dispatch -------------------------------------

    inline void NotifyProjectOpened(LPCWSTR pszProjectPath)
    {
        LPCWSTR pszPath = pszProjectPath ? pszProjectPath : L"";
        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            if (m_entries[i].pAddIn)
                m_entries[i].pAddIn->OnProjectOpened(pszPath);
        }
    }

    inline void NotifyProjectClosed()
    {
        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            if (m_entries[i].pAddIn)
                m_entries[i].pAddIn->OnProjectClosed();
        }
    }

    // Dispatches a command to every add-in. Returns S_OK if any add-in
    // handled it (OnCommand returned S_OK); S_FALSE if none did.
    inline HRESULT NotifyCommand(UINT nCommandId) throw()
    {
        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            if (m_entries[i].pAddIn)
            {
                HRESULT hr = m_entries[i].pAddIn->OnCommand(nCommandId);
                if (hr == S_OK)
                    return S_OK;
            }
        }
        return S_FALSE;
    }

    // -- Accessors ---------------------------------------------------------

    inline DWORD GetCount() const throw()
    {
        return static_cast<DWORD>(m_entries.size());
    }

    inline ISundanceAddIn* GetAddIn(DWORD dwIndex) const throw()
    {
        if (dwIndex >= m_entries.size())
            return NULL;
        return m_entries[dwIndex].pAddIn;
    }

private:
    // Loads a single DLL, queries its entry point, negotiates the
    // versioned contract and instantiates the add-in. On any failure the
    // DLL is unloaded and the error is returned (the directory scan
    // continues with the next file).
    inline HRESULT LoadSingleDll(LPCWSTR pszDllPath)
    {
        HMODULE hMod = ::LoadLibraryW(pszDllPath);
        if (!hMod)
            return HRESULT_FROM_WIN32(::GetLastError());

        // Primary entry point; fall back to the legacy symbol name for
        // add-ins built against pre-1.0 headers (same signature).
        typedef HRESULT (WINAPI *PFN_ADDIN_CREATE)(
            ISundanceAddInHost*,
            SundanceAddInContract*);

        PFN_ADDIN_CREATE pfnCreate = reinterpret_cast<PFN_ADDIN_CREATE>(
            ::GetProcAddress(hMod, "SundanceAddInCreateAddIn"));
        if (!pfnCreate)
        {
            pfnCreate = reinterpret_cast<PFN_ADDIN_CREATE>(
                ::GetProcAddress(hMod, "SundanceAddInInitialize"));
        }

        if (!pfnCreate)
        {
            ::FreeLibrary(hMod);
            return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        }

        // Versioned contract negotiation. The host pre-fills the struct
        // size; the add-in validates it and fills the version + instance.
        SundanceAddInContract contract = { 0 };
        contract.dwStructSize = sizeof(SundanceAddInContract);

        HRESULT hr = pfnCreate(m_pHost, &contract);
        if (FAILED(hr))
        {
            ::FreeLibrary(hMod);
            return hr;
        }

        // The add-in must have written a struct at least large enough to
        // contain pAddIn, declared a contract version the host supports,
        // and actually created an instance.
        const size_t cbMinContract =
            offsetof(SundanceAddInContract, pAddIn) + sizeof(contract.pAddIn);

        if (contract.dwStructSize < cbMinContract ||
            contract.dwMajorVersion > SUNDANCE_ADDIN_CONTRACT_VERSION_MAJOR ||
            contract.dwMinorVersion > SUNDANCE_ADDIN_CONTRACT_VERSION_MINOR ||
            contract.pAddIn == NULL)
        {
            ::FreeLibrary(hMod);
            return E_ABORT; // Contract mismatch — reject the add-in
        }

        hr = contract.pAddIn->OnInitialize(m_pHost);
        if (FAILED(hr))
        {
            ::FreeLibrary(hMod);
            return hr;
        }

        AddInEntry entry;
        entry.hModule = hMod;
        entry.pAddIn = contract.pAddIn;

        try
        {
            m_entries.push_back(entry);
        }
        catch (const std::bad_alloc&)
        {
            ::FreeLibrary(hMod);
            return E_OUTOFMEMORY;
        }

        return S_OK;
    }

    struct AddInEntry
    {
        HMODULE         hModule;
        ISundanceAddIn* pAddIn;
    };

    ISundanceAddInHost*    m_pHost;
    std::vector<AddInEntry> m_entries;
};

#endif // SUNDANCE_ADDIN_MANAGER_H
