/*
 * SundanceAddInContract.h
 *
 * Public add-in / plugin contract for MovieMakerCore.dll ("Sundance").
 *
 * This is the ONE header a third-party add-in DLL needs in order to be
 * discovered and hosted by Windows Live Movie Maker. It defines:
 *
 *   - The versioned contract descriptor      (SundanceAddInContract)
 *   - The plugin interface implemented by the add-in (ISundanceAddIn)
 *   - The host interface implemented by the app (ISundanceAddInHost)
 *   - The DLL entry point the loader queries (SundanceAddInCreateAddIn)
 *
 * Add-ins are loaded the same way the original binary loaded them: by
 * directory scan + LoadLibrary/GetProcAddress (see QUIRKS.md item 3).
 * They are intentionally NOT COM objects -- no CLSID, no registry entry.
 * A DLL in the add-in directory that exports the entry point is loaded;
 * everything else is ignored.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 * Written in C++14; safe for MSVC and for plain C-aware consumers of
 * the ABI (the interfaces are pure vtable + __stdcall).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_ADDIN_CONTRACT_H
#define SUNDANCE_ADDIN_CONTRACT_H

#include <windows.h>

// ============================================================================
// Export macro
//
// Add-in DLLs must define SUNDANCE_ADDIN_EXPORTS before including this
// header so the entry point is __declspec(dllexport). MovieMakerCore.dll
// does NOT link against add-in symbols directly -- it resolves them with
// GetProcAddress -- so on the host side the macro simply expands to
// nothing meaningful.
// ============================================================================
#ifdef SUNDANCE_ADDIN_EXPORTS
    #define SUNDANCE_ADDIN_API __declspec(dllexport)
#else
    #define SUNDANCE_ADDIN_API __declspec(dllimport)
#endif

// ============================================================================
// Versioned contract
//
// The loader and the add-in negotiate a contract version on every load.
// The host supports contracts with a major version <= SUNDANCE_ADDIN_MAJOR
// and a minor version <= SUNDANCE_ADDIN_MINOR. Bump MAJOR on ABI-breaking
// changes (vtable layout, struct layout, calling conventions). Bump MINOR
// when appending non-breaking members to SundanceAddInContract.
// ============================================================================
#define SUNDANCE_ADDIN_CONTRACT_VERSION_MAJOR  1
#define SUNDANCE_ADDIN_CONTRACT_VERSION_MINOR  0
#define SUNDANCE_ADDIN_CONTRACT_VERSION_STRING L"1.0"

// ============================================================================
// Host interface
//
// Implemented by MovieMakerCore.dll and passed to the add-in at
// initialization time. Gives add-ins access to the running application
// without leaking internal C++ types across the DLL boundary. All methods
// use __stdcall (WINAPI) so the ABI is stable regardless of compiler.
// ============================================================================
struct ISundanceAddInHost
{
    // Retrieves the main application window handle. May return NULL until
    // the main window has been created.
    virtual HRESULT WINAPI GetMainWindow(HWND* pHwnd) = 0;

    // Retrieves the host application version:
    //   pdwMajor / pdwMinor / pdwBuild / pdwRevision
    // (Sundance version 16.4.3528.331 -> 16 / 4 / 3528 / 331)
    virtual HRESULT WINAPI GetVersion(
        DWORD* pdwMajor,
        DWORD* pdwMinor,
        DWORD* pdwBuild,
        DWORD* pdwRevision) = 0;

    // Retrieves the per-user application data directory
    // (%LOCALAPPDATA%\Microsoft\Windows Live\Movie Maker). pszPath must
    // point to a buffer of at least cchPath wide characters. The path is
    // returned WITHOUT a trailing backslash.
    virtual HRESULT WINAPI GetDataDirectory(LPWSTR pszPath, DWORD cchPath) = 0;

    // Emits a diagnostic message. Add-ins may call this at any time; the
    // host decides how to route it (in the decompilation: OutputDebugString).
    virtual void WINAPI Log(LPCWSTR pszMessage) = 0;
};

// ============================================================================
// Add-in interface
//
// Implemented by the add-in DLL and instantiated by its entry point. The
// loader calls OnInitialize once after the contract is validated, and
// OnShutdown once during application shutdown before the DLL is unloaded.
// Project and command hooks are dispatched by the host at the appropriate
// times; add-ins that do not care about a given event can return S_FALSE
// (for OnCommand) or simply do nothing.
// ============================================================================
struct ISundanceAddIn
{
    // Called once after the add-in is loaded and its contract validated.
    // pHost is valid until OnShutdown. Return S_OK to stay loaded; any
    // failure causes the loader to unload the DLL immediately.
    virtual HRESULT WINAPI OnInitialize(ISundanceAddInHost* pHost) = 0;

    // Called once during application shutdown, before the add-in DLL is
    // freed. The add-in must release any resources it holds here.
    virtual void WINAPI OnShutdown() = 0;

    // Called after a project file (.wlmp) is opened.
    // pszProjectPath is the full path, or L"" for an untitled project.
    virtual void WINAPI OnProjectOpened(LPCWSTR pszProjectPath) = 0;

    // Called when the current project is closed (before it is destroyed).
    virtual void WINAPI OnProjectClosed() = 0;

    // Called when the host dispatches a command (e.g. a ribbon command
    // whose command id the host did not handle itself). Return S_OK to
    // indicate the command was handled; the host will not process it
    // further. Return S_FALSE or an error to let the host continue.
    virtual HRESULT WINAPI OnCommand(UINT nCommandId) = 0;
};

// ============================================================================
// Versioned contract descriptor
//
// A pointer to this struct is passed to the entry point. The HOST fills
// dwStructSize (sizeof the host's struct) before the call; the ADD-IN
// fills dwMajorVersion, dwMinorVersion and pAddIn. Both sides validate
// the size and version before trusting any member:
//
//   Host  -> plugin: dwStructSize == sizeof(SundanceAddInContract) tells
//            the plugin how large the host's struct is (it may be larger
//            if the plugin predates a struct extension).
//   Plugin -> host: dwMajorVersion/dwMinorVersion must be <= the host's
//            supported contract version, and pAddIn must be non-NULL.
// ============================================================================
struct SundanceAddInContract
{
    DWORD           dwStructSize;    // set by host: sizeof(SundanceAddInContract)
    DWORD           dwMajorVersion;  // set by add-in: SUNDANCE_ADDIN_CONTRACT_VERSION_MAJOR
    DWORD           dwMinorVersion;  // set by add-in: SUNDANCE_ADDIN_CONTRACT_VERSION_MINOR
    ISundanceAddIn* pAddIn;          // set by add-in: instantiated add-in instance
};

// ============================================================================
// Add-in DLL entry point
//
// The loader probes this export on every DLL in the add-in directory.
// The add-in must:
//   1. Validate the incoming contract (dwStructSize is at least large
//      enough to hold pAddIn).
//   2. Fill in its contract version and instantiate its ISundanceAddIn.
//   3. Return S_OK.
//
// SUNDANCE_ADDIN_API_EXPORT_ENTRY is defined as an extern "C" __stdcall
// export so the symbol is name-decorated as "_SundanceAddInCreateAddIn@8"
// on x86 and can be resolved with GetProcAddress("SundanceAddInCreateAddIn").
// ============================================================================
#ifdef __cplusplus
extern "C" {
#endif

SUNDANCE_ADDIN_API HRESULT WINAPI SundanceAddInCreateAddIn(
    ISundanceAddInHost*      pHost,
    SundanceAddInContract*   pContract);

// Legacy entry-point symbol. The original binary probed an export named
// SundanceAddInInitialize / SundanceAddInShutdown, but that ABI passed a
// concrete SundanceAppMain* pointer and was never a stable contract. The
// loader still probes "SundanceAddInInitialize" as a fallback symbol for
// the SAME signature as SundanceAddInCreateAddIn (host + versioned
// contract). Add-ins built against this header should use
// SundanceAddInCreateAddIn. A legacy "SundanceAddInShutdown" free export,
// if present, is NOT called -- shutdown is driven by ISundanceAddIn::
// OnShutdown.
SUNDANCE_ADDIN_API HRESULT WINAPI SundanceAddInInitialize(
    ISundanceAddInHost*      pHost,
    SundanceAddInContract*   pContract);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SUNDANCE_ADDIN_CONTRACT_H
