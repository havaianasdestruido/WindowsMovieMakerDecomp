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

static HINSTANCE g_hModule = NULL;

// Recreation note: the reference WLMFDS.dll aborts DLL_PROCESS_ATTACH.
// LoadLibrary fails with ERROR_DLL_INIT_FAILED (Win32 1114) in a fresh
// process, deterministically -- pinned by the comstubs test_wlmfds_load
// contract and the mmr-gui self-test WLMFDS.load check ("module cannot
// initialize; COM quartet exported (dumpbin) but DllMain init failure").
// The module is therefore never actually resident: its exports are only
// ever probed from the file image, and no process-side state (COM, MF,
// MMCSS) is ever established. Returning FALSE from DllMain on attach
// makes the loader tear the image down and report ERROR_DLL_INIT_FAILED,
// reproducing the original behavior exactly.
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        return FALSE; // reference binary: attach always fails (1114)

    case DLL_PROCESS_DETACH:
        // Not reached for a failed attach; kept for symmetry.
        g_hModule = NULL;
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
