/*
 * dllmain.cpp
 *
 * DLL entry point for WLXMovieLibrary.dll.
 * Initializes and shuts down Media Foundation on attach/detach.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXMovieLibrary.h"
#include <shlwapi.h>

static HINSTANCE g_hModule    = NULL;
static bool      g_bMFInit    = false;
static bool      g_bComInit   = false;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);

        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr))
            g_bComInit = true;

        hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
        if (SUCCEEDED(hr))
            g_bMFInit = true;

        break;
    }

    case DLL_PROCESS_DETACH:
    {
        if (g_bMFInit)
        {
            MFShutdown();
            g_bMFInit = false;
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
