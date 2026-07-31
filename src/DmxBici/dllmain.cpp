/*
 * dllmain.cpp
 *
 * DLL entry point for DmxBici.dll.
 * Initializes COM for BICI telemetry operations.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include <windows.h>
#include <ole2.h>

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

        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (hr == S_OK)
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

STDAPI DllCanUnloadNow(void) { return S_OK; }
STDAPI DllGetClassObject(REFCLSID, REFIID, LPVOID*) { return CLASS_E_CLASSNOTAVAILABLE; }
STDAPI DllRegisterServer(void) { return S_OK; }
STDAPI DllUnregisterServer(void) { return S_OK; }
