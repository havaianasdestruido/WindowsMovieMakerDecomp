/*
 * dllmain.cpp
 *
 * DLL entry point for WLMFDS.dll (Media Foundation / DirectShow bridge).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLMFDS.h"
#include "WLXPhotoBase.h"

#include <avrt.h>

static HINSTANCE g_hModule    = NULL;
static bool      g_bComInit   = false;
static bool      g_bMFInit    = false;
static HANDLE    g_hMMCSS     = NULL;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);

        HRESULT hrCom = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hrCom))
            g_bComInit = true;

        if (SUCCEEDED(MFStartup(MF_VERSION, MFSTARTUP_LITE)))
            g_bMFInit = true;

        // AVRT for multimedia thread scheduling
        g_hMMCSS = AvSetMmThreadCharacteristicsW(L"MMCSS", NULL);

        break;
    }

    case DLL_PROCESS_DETACH:
    {
        if (g_hMMCSS) { AvRevertMmThreadCharacteristics(g_hMMCSS); g_hMMCSS = NULL; }
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
