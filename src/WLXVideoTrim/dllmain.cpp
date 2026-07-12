/*
 * dllmain.cpp
 *
 * DLL entry point for WLXVideoTrim.dll.
 * Initializes COM and sets up the DirectShow filter mapper.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXVideoTrim.h"
#include "WLXPhotoBase.h"

static HINSTANCE g_hModule    = NULL;
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

        // COM required for DirectShow filter graph operations
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr))
            g_bComInit = true;

        break;
    }

    case DLL_PROCESS_DETACH:
    {
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
