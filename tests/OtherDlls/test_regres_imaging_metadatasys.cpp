// OtherDlls Test Harness - RegRes.dll + Imaging.dll + MetadataSys.dll
// Loads all three DLLs, probes all exports with SEH protection
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <unknwn.h>
#include <cstdio>
#include <cstdlib>

// ============================================================
// GUIDs
// ============================================================
static const CLSID CLSID_Empty = {0,0,0,{0,0,0,0,0,0,0,0}};
static const IID IID_IUnknown_Cast = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

// ============================================================
// Helpers
// ============================================================
static void SetSearchPaths() {
    wchar_t galleryDir[MAX_PATH], sharedDir[MAX_PATH], mainDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
    GetFullPathNameW(L"undecomp\\Shared", MAX_PATH, sharedDir, NULL);
    GetFullPathNameW(L".", MAX_PATH, mainDir, NULL);
    AddDllDirectory(galleryDir);
    AddDllDirectory(sharedDir);
    AddDllDirectory(mainDir);
    SetDllDirectoryW(galleryDir);
}

static HMODULE SafeLoadDll(const wchar_t* relPath) {
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(relPath, MAX_PATH, fullPath, NULL);
    printf("[*] Loading: %ls\n", fullPath);
    HMODULE h = LoadLibraryExW(fullPath, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!h) {
        printf("[!] LoadLibraryEx failed (err %u), retrying SetDllDirectory\n", GetLastError());
        wchar_t galleryDir[MAX_PATH];
        GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
        SetDllDirectoryW(galleryDir);
        h = LoadLibraryW(fullPath);
    }
    if (h)
        printf("[+] Loaded at base: 0x%p\n", h);
    else
        printf("[!] FAILED to load: error %u\n", GetLastError());
    return h;
}

// ============================================================
// Export typedefs
// ============================================================
typedef HRESULT (__stdcall *DllCanUnloadNowFunc)();
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServerFunc)();
typedef HRESULT (__stdcall *DllUnregisterServerFunc)();
typedef HRESULT (__stdcall *WLXPSGetItemPropertyHandlerFunc)(IUnknown*, DWORD, REFIID, void**);

// ============================================================
// RegRes.dll - 0 exports (resource-only DLL)
// ============================================================
static void TestRegRes(HMODULE h) {
    printf("\n========================================\n");
    printf("  RegRes.dll Export Probing\n");
    printf("========================================\n");

    printf("  [info] Resource-only DLL — no code, no exports\n");

    // Verify no exports exist
    {
        printf("\n[*] DllCanUnloadNow\n");
        DllCanUnloadNowFunc fn = (DllCanUnloadNowFunc)GetProcAddress(h, "DllCanUnloadNow");
        if (fn)
            printf("    [!] unexpected: found DllCanUnloadNow\n");
        else
            printf("    [OK] not found (expected for resource-only DLL)\n");
    }

    {
        printf("\n[*] DllGetClassObject\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn)
            printf("    [!] unexpected: found DllGetClassObject\n");
        else
            printf("    [OK] not found (expected for resource-only DLL)\n");
    }

    // Try loading a resource string
    {
        printf("\n[*] Probing resource section\n");
        HRSRC hRes = FindResourceW(h, MAKEINTRESOURCEW(1), MAKEINTRESOURCEW(16));
        if (hRes) {
            printf("    [+] VS_VERSION_INFO resource found\n");
            HGLOBAL hGlob = LoadResource(h, hRes);
            if (hGlob) {
                VS_FIXEDFILEINFO* pInfo = (VS_FIXEDFILEINFO*)LockResource(hGlob);
                if (pInfo) {
                    printf("    [+] Version: %u.%u.%u.%u\n",
                        HIWORD(pInfo->dwProductVersionMS),
                        LOWORD(pInfo->dwProductVersionMS),
                        HIWORD(pInfo->dwProductVersionLS),
                        LOWORD(pInfo->dwProductVersionLS));
                }
                FreeResource(hGlob);
            }
        } else {
            printf("    [!] VS_VERSION_INFO not found (err %u)\n", GetLastError());
        }
    }
}

// ============================================================
// Imaging.dll - 4 exports (standard COM)
// CLSIDs: 23 COM classes including ImagingEngine, View, EffectGraph, etc.
// ============================================================
static void TestImaging(HMODULE h) {
    printf("\n========================================\n");
    printf("  Imaging.dll Export Probing\n");
    printf("========================================\n");

    // DllCanUnloadNow
    {
        printf("\n[*] DllCanUnloadNow (ordinal 1, hint 0)\n");
        DllCanUnloadNowFunc fn = (DllCanUnloadNowFunc)GetProcAddress(h, "DllCanUnloadNow");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn();
                printf("    => HRESULT=0x%08X\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllGetClassObject with empty CLSID
    {
        printf("\n[*] DllGetClassObject (ordinal 2, hint 1) — empty CLSID\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, NULL);
                printf("    => HRESULT=0x%08X (empty CLSID, null ppv)\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllGetClassObject with ImagingEngine CLSID
    {
        printf("\n[*] DllGetClassObject (ImagingEngine CLSID)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            CLSID clsid;
            CLSIDFromString(L"{8B7C4E00-A0F0-4E8A-B9F2-A8C5E0D5A5D3}", &clsid);
            LPVOID pUnk = NULL;
            __try {
                HRESULT hr = fn(clsid, IID_IUnknown_Cast, &pUnk);
                printf("    => HRESULT=0x%08X\n", hr);
                if (SUCCEEDED(hr) && pUnk) {
                    printf("    [+] IUnknown obtained at 0x%p\n", pUnk);
                    ((IUnknown*)pUnk)->Release();
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                if (pUnk) ((IUnknown*)pUnk)->Release();
            }
        }
    }

    // DllGetClassObject with View CLSID
    {
        printf("\n[*] DllGetClassObject (View CLSID)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            CLSID clsid;
            CLSIDFromString(L"{C9273E20-6764-4F93-B204-4555E0702A46}", &clsid);
            LPVOID pUnk = NULL;
            __try {
                HRESULT hr = fn(clsid, IID_IUnknown_Cast, &pUnk);
                printf("    => HRESULT=0x%08X\n", hr);
                if (SUCCEEDED(hr) && pUnk) {
                    printf("    [+] IUnknown obtained at 0x%p\n", pUnk);
                    ((IUnknown*)pUnk)->Release();
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                if (pUnk) ((IUnknown*)pUnk)->Release();
            }
        }
    }

    // DllGetClassObject with EffectGraph CLSID
    {
        printf("\n[*] DllGetClassObject (EffectGraph CLSID)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            CLSID clsid;
            CLSIDFromString(L"{7B13EA1C-CA6E-42B4-B074-66CE103A3971}", &clsid);
            LPVOID pUnk = NULL;
            __try {
                HRESULT hr = fn(clsid, IID_IUnknown_Cast, &pUnk);
                printf("    => HRESULT=0x%08X\n", hr);
                if (SUCCEEDED(hr) && pUnk) {
                    printf("    [+] IUnknown obtained at 0x%p\n", pUnk);
                    ((IUnknown*)pUnk)->Release();
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                if (pUnk) ((IUnknown*)pUnk)->Release();
            }
        }
    }

    // DllRegisterServer
    {
        printf("\n[*] DllRegisterServer (ordinal 3, hint 2)\n");
        DllRegisterServerFunc fn = (DllRegisterServerFunc)GetProcAddress(h, "DllRegisterServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            printf("    [skip] Would write to registry\n");
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllUnregisterServer
    {
        printf("\n[*] DllUnregisterServer (ordinal 4, hint 3)\n");
        DllUnregisterServerFunc fn = (DllUnregisterServerFunc)GetProcAddress(h, "DllUnregisterServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            printf("    [skip] Would write to registry\n");
        } else {
            printf("    [!] not found\n");
        }
    }
}

// ============================================================
// MetadataSys.dll - 5 exports (4 standard COM + 1 custom)
// Custom: WLXPSGetItemPropertyHandler
// ============================================================
static void TestMetadataSys(HMODULE h) {
    printf("\n========================================\n");
    printf("  MetadataSys.dll Export Probing\n");
    printf("========================================\n");

    // DllCanUnloadNow
    {
        printf("\n[*] DllCanUnloadNow (ordinal 2, hint 0)\n");
        DllCanUnloadNowFunc fn = (DllCanUnloadNowFunc)GetProcAddress(h, "DllCanUnloadNow");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn();
                printf("    => HRESULT=0x%08X\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllGetClassObject with empty CLSID
    {
        printf("\n[*] DllGetClassObject (ordinal 3, hint 1) — empty CLSID\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, NULL);
                printf("    => HRESULT=0x%08X (empty CLSID, null ppv)\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllRegisterServer
    {
        printf("\n[*] DllRegisterServer (ordinal 4, hint 2)\n");
        DllRegisterServerFunc fn = (DllRegisterServerFunc)GetProcAddress(h, "DllRegisterServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            printf("    [skip] Would write to registry\n");
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllUnregisterServer
    {
        printf("\n[*] DllUnregisterServer (ordinal 5, hint 3)\n");
        DllUnregisterServerFunc fn = (DllUnregisterServerFunc)GetProcAddress(h, "DllUnregisterServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            printf("    [skip] Would write to registry\n");
        } else {
            printf("    [!] not found\n");
        }
    }

    // WLXPSGetItemPropertyHandler (custom export, ordinal 1)
    {
        printf("\n[*] WLXPSGetItemPropertyHandler (ordinal 1, hint 4) — custom export\n");
        WLXPSGetItemPropertyHandlerFunc fn =
            (WLXPSGetItemPropertyHandlerFunc)GetProcAddress(h, "WLXPSGetItemPropertyHandler");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn(NULL, 0, IID_IUnknown_Cast, NULL);
                printf("    => HRESULT=0x%08X (null args)\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found by name, trying ordinal 1\n");
            fn = (WLXPSGetItemPropertyHandlerFunc)GetProcAddress(h, (LPCSTR)1);
            if (fn) {
                printf("    [+] found by ordinal 1, ptr=0x%p\n", fn);
                __try {
                    HRESULT hr = fn(NULL, 0, IID_IUnknown_Cast, NULL);
                    printf("    => HRESULT=0x%08X (null args)\n", hr);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                }
            } else {
                printf("    [!] not found by ordinal either\n");
            }
        }
    }
}

// ============================================================
// Main
// ============================================================
int main(int argc, char** argv) {
    printf("=== OtherDlls Test Harness ===\n");
    printf("Targets: RegRes.dll + Imaging.dll + MetadataSys.dll\n\n");

    SetSearchPaths();

    HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    printf("[*] CoInitializeEx: HRESULT=0x%08X\n\n", hrCoInit);

    // --- RegRes.dll (resource-only, no exports) ---
    HMODULE hRegRes = SafeLoadDll(L"undecomp\\Photo Gallery\\RegRes.dll");
    if (hRegRes) {
        TestRegRes(hRegRes);
        printf("\n[*] Unloading RegRes.dll\n");
        FreeLibrary(hRegRes);
    }

    // --- Imaging.dll (COM server, 4 exports, 23 CLSIDs) ---
    HMODULE hImaging = SafeLoadDll(L"undecomp\\Photo Gallery\\Imaging.dll");
    if (hImaging) {
        TestImaging(hImaging);
        printf("\n[*] Unloading Imaging.dll\n");
        FreeLibrary(hImaging);
    }

    // --- MetadataSys.dll (COM server + custom export) ---
    HMODULE hMetaSys = SafeLoadDll(L"undecomp\\Photo Gallery\\MetadataSys.dll");
    if (hMetaSys) {
        TestMetadataSys(hMetaSys);
        printf("\n[*] Unloading MetadataSys.dll\n");
        FreeLibrary(hMetaSys);
    }

    if (SUCCEEDED(hrCoInit))
        CoUninitialize();

    printf("\n=== Done ===\n");
    return 0;
}
