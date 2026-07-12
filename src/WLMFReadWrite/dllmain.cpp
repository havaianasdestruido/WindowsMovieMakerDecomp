/*
 * dllmain.cpp
 *
 * DLL entry point for WLMFReadWrite.dll.
 * Initializes Media Foundation on attach, shuts down on detach.
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLMFReadWrite.h"
#include "WLXPhotoBase.h"

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

        CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
        g_bComInit = true;

        if (SUCCEEDED(MFStartup(MF_VERSION, MFSTARTUP_LITE)))
            g_bMFInit = true;

        break;
    }

    case DLL_PROCESS_DETACH:
    {
        if (g_bMFInit) { MFShutdown(); g_bMFInit = false; }
        if (g_bComInit) { CoUninitialize(); g_bComInit = false; }
        g_hModule = NULL;
        break;
    }

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
