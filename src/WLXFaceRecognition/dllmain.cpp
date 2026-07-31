/*
 * dllmain.cpp
 *
 * DLL entry point for WLXFaceRecognition.dll.
 * Loads the ML model data on attach, frees on detach.
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXFaceRecognition.h"
#include "WLXPhotoBase.h"

static HINSTANCE g_hModule    = NULL;
static bool      g_bComInit   = false;
static bool      g_bGdipInit  = false;
static ULONG_PTR g_gdipToken  = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);

        HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
        g_bComInit = SUCCEEDED(hrCoInit);

        Gdiplus::GdiplusStartupInput input = {};
        input.GdiplusVersion = 1;
        if (Gdiplus::GdiplusStartup(&g_gdipToken, &input, NULL) == Gdiplus::Ok)
            g_bGdipInit = true;

        break;
    }

    case DLL_PROCESS_DETACH:
    {
        if (g_bGdipInit)
        {
            Gdiplus::GdiplusShutdown(g_gdipToken);
            g_bGdipInit = false;
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
