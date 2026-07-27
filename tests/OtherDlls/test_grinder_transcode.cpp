// OtherDlls Test Harness - WLXGrinderScheduler.dll + WLXImageTranscode.dll
// Loads both DLLs, probes all exports with SEH protection
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#include <unknwn.h>
#include <cstdio>
#include <cstdlib>

// ============================================================
// GUIDs (avoiding GUID_NULL / IID_IUnknown header issues)
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
// Export typedefs (standard COM server - 4 exports each)
// ============================================================
typedef HRESULT (__stdcall *DllCanUnloadNowFunc)();
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServerFunc)();
typedef HRESULT (__stdcall *DllUnregisterServerFunc)();

// ============================================================
// WLXGrinderScheduler.dll - 4 exports (standard COM)
// CLSID: {121244C5-B0AD-45fb-9D8E-B893B917D053}
// TypeLib: {77A16B7E-9DFE-410c-8863-264BF3E91103}
// ============================================================
static void TestGrinderScheduler(HMODULE h) {
    printf("\n========================================\n");
    printf("  WLXGrinderScheduler.dll Export Probing\n");
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
        printf("\n[*] DllGetClassObject (ordinal 2, hint 1)\n");
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

    // DllGetClassObject with Grinder CLSID + valid ppv
    {
        printf("\n[*] DllGetClassObject (GrinderScheduler CLSID)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            // {121244C5-B0AD-45fb-9D8E-B893B917D053}
            CLSID clsid_grinder = {0xB0AD44C5, 0x45fb, 0x454D, {0x9D, 0x8E, 0xB8, 0x93, 0xB9, 0x17, 0xD0, 0x53}};
            // Actually need the correct byte order for CLSIDFromString-style:
            // {121244C5-B0AD-45fb-9D8E-B893B917D053}
            CLSID clsid;
            CLSIDFromString(L"{121244C5-B0AD-45fb-9D8E-B893B917D053}", &clsid);
            LPVOID pUnk = NULL;
            __try {
                HRESULT hr = fn(clsid, IID_IUnknown_Cast, &pUnk);
                printf("    => HRESULT=0x%08X\n", hr);
                if (SUCCEEDED(hr) && pUnk) {
                    printf("    [+] IUnknown obtained at 0x%p\n", pUnk);
                    // Query for IClassFactory
                    IClassFactory* pCF = NULL;
                    HRESULT hrQI = ((IUnknown*)pUnk)->QueryInterface(IID_IClassFactory, (void**)&pCF);
                    printf("    QI(IClassFactory): HRESULT=0x%08X\n", hrQI);
                    if (SUCCEEDED(hrQI) && pCF) {
                        printf("    [+] IClassFactory obtained\n");
                        pCF->Release();
                    }
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
// WLXImageTranscode.dll - 4 exports (standard COM)
// CLSIDs: ImageTranscode, ImageLoader, WLXOutofProc
// ============================================================
static void TestImageTranscode(HMODULE h) {
    printf("\n========================================\n");
    printf("  WLXImageTranscode.dll Export Probing\n");
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
        printf("\n[*] DllGetClassObject (ordinal 2, hint 1)\n");
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

    // DllGetClassObject with ImageTranscode CLSID + IUnknown
    {
        printf("\n[*] DllGetClassObject (ImageTranscode CLSID)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            CLSID clsid;
            CLSIDFromString(L"{20575516-78AF-4404-B3C7-51D05F9945B5}", &clsid);
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

    // DllGetClassObject with ImageLoader CLSID + IUnknown
    {
        printf("\n[*] DllGetClassObject (ImageLoader CLSID)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            CLSID clsid;
            CLSIDFromString(L"{E253845C-A1A2-4ae4-B784-5E8140CD5B2C}", &clsid);
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

    // DllGetClassObject with WLXOutofProc CLSID + IUnknown
    {
        printf("\n[*] DllGetClassObject (WLXOutofProc CLSID)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            CLSID clsid;
            CLSIDFromString(L"{B8A2E14E-290D-4122-B092-1A7D86198CCE}", &clsid);
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
// Main
// ============================================================
int main(int argc, char** argv) {
    printf("=== OtherDlls Test Harness ===\n");
    printf("Targets: WLXGrinderScheduler.dll + WLXImageTranscode.dll\n\n");

    SetSearchPaths();

    // Initialize COM (needed for CLSIDFromString)
    HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    printf("[*] CoInitializeEx: HRESULT=0x%08X\n\n", hrCoInit);

    // --- WLXGrinderScheduler.dll ---
    HMODULE hGrinder = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXGrinderScheduler.dll");
    if (hGrinder) {
        TestGrinderScheduler(hGrinder);
        printf("\n[*] Unloading WLXGrinderScheduler.dll\n");
        FreeLibrary(hGrinder);
    }

    // --- WLXImageTranscode.dll ---
    HMODULE hTranscode = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXImageTranscode.dll");
    if (hTranscode) {
        TestImageTranscode(hTranscode);
        printf("\n[*] Unloading WLXImageTranscode.dll\n");
        FreeLibrary(hTranscode);
    }

    if (SUCCEEDED(hrCoInit))
        CoUninitialize();

    printf("\n=== Done ===\n");
    return 0;
}
