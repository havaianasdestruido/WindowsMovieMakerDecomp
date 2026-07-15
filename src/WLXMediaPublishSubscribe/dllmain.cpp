/*
 * dllmain.cpp
 *
 * DLL entry point for WLXMediaPublishSubscribe.dll.
 * Initializes COM, WinINet, and the plugin registration system.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXMediaPublishSubscribe.h"
#include "WLXPhotoBase.h"

#include <wininet.h>

static HINSTANCE g_hModule    = NULL;
static bool      g_bComInit   = false;
static bool      g_bWinInetInit = false;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);

        // COM required for RPC-based service communication
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr))
            g_bComInit = true;

        g_bWinInetInit = true;

        break;
    }

    case DLL_PROCESS_DETACH:
    {
        if (g_bWinInetInit)
        {
            InternetCloseHandle(NULL);  // Cleanup WinInet state
            g_bWinInetInit = false;
        }

        if (g_bComInit)
        {
            CoUninitialize();
            g_bComInit = false;
        }

        g_hModule = NULL;
        break;
    }

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
