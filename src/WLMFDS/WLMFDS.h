/*
 * WLMFDS.h
 *
 * Public header for WLMFDS.dll -- Media Foundation / DirectShow bridge.
 * Provides compatibility layer between DirectShow filter graphs and
 * Media Foundation pipelines for Windows Live Movie Maker 2012.
 *
 * Enables playback and transcoding of legacy DirectShow-encoded content
 * through the modern Media Foundation pipeline. Handles filter graph
 * serialization, DShow-to-MF source resolution, and EVR (Enhanced Video
 * Renderer) integration for hardware-accelerated preview.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLMFDS_H
#define WLMFDS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WINVER        0x0602
#define _WIN32_WINNT  0x0602

#include <windows.h>
#include <objbase.h>
#include <mfapi.h>
#include <mfidl.h>
#include <evr.h>

#ifdef WLMFDS_EXPORTS
    #define WLMFDS_API __declspec(dllexport)
#else
    #define WLMFDS_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace MFDS
{
    class BridgeManager;
    class DShowToMFConverter;
    class EVRPresenter;
}

// ============================================================================
// Bridge configuration
// ============================================================================
struct MFBridgeConfig
{
    HWND    hVideoWindow;
    UINT32  uVideoWidth;
    UINT32  uVideoHeight;
    BOOL    bUseEVR;

    MFBridgeConfig()
        : hVideoWindow(NULL)
        , uVideoWidth(640)
        , uVideoHeight(480)
        , bUseEVR(TRUE)
    {
    }
};

// ============================================================================
// Exported functions (4 exports)
// ============================================================================
extern "C"
{
    // Creates a DShow-to-MF bridge instance.
    WLMFDS_API HANDLE __stdcall MFDSBridge_Create(const MFBridgeConfig* pConfig);

    // Destroys the bridge instance.
    WLMFDS_API void __stdcall MFDSBridge_Destroy(HANDLE hBridge);

    // Converts a DirectShow filter graph moniker to an MF source reader.
    WLMFDS_API HRESULT __stdcall MFDSBridge_ConvertDShowSource(LPCWSTR pszDShowFilter,
        HANDLE hMFReader);

    // Sets up EVR (Enhanced Video Renderer) for the bridge output.
    WLMFDS_API HRESULT __stdcall MFDSBridge_SetupEVR(HANDLE hBridge, IBaseFilter* pEVRFilter);
}

#endif // WLMFDS_H
