// OtherDlls Test Harness - wlxclip.dll + WLXDSPA.dll
// Loads both DLLs, probes all exports with SEH protection
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
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
// wlxclip.dll - 6 exports
// ============================================================
// Export typedefs
typedef HRESULT (__stdcall *DllCanUnloadNowFunc)();
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServerFunc)();
typedef HRESULT (__stdcall *DllUnregisterServerFunc)();
typedef void*   (__cdecl   *VMGGetClipCreateFunctionsFunc)();
typedef void    (__cdecl   *VMGShellThumbnailGeneratorWFunc)(const wchar_t*, const wchar_t*, void*);

static void TestWlxclip(HMODULE h) {
    printf("\n========================================\n");
    printf("  wlxclip.dll Export Probing\n");
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

    // DllGetClassObject
    {
        printf("\n[*] DllGetClassObject (ordinal 3, hint 1)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, NULL);
                printf("    => HRESULT=0x%08X (null args)\n", hr);
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

    // VMGGetClipCreateFunctions
    {
        printf("\n[*] VMGGetClipCreateFunctions (ordinal 1, hint 4)\n");
        VMGGetClipCreateFunctionsFunc fn = (VMGGetClipCreateFunctionsFunc)GetProcAddress(h, "VMGGetClipCreateFunctions");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                void* result = fn();
                printf("    => returns ptr=0x%p\n", result);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // VMGShellThumbnailGeneratorW
    {
        printf("\n[*] VMGShellThumbnailGeneratorW (ordinal 6, hint 5)\n");
        VMGShellThumbnailGeneratorWFunc fn = (VMGShellThumbnailGeneratorWFunc)GetProcAddress(h, "VMGShellThumbnailGeneratorW");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                fn(L"test.jpg", L"C:\\temp\\out.jpg", NULL);
                printf("    => returned (with test args)\n");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }
}

// ============================================================
// WLXDSPA.dll - 4 exports (all standard COM)
// ============================================================
static void TestWLXDSPA(HMODULE h) {
    printf("\n========================================\n");
    printf("  WLXDSPA.dll Export Probing\n");
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

    // DllGetClassObject
    {
        printf("\n[*] DllGetClassObject (ordinal 2, hint 1)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, NULL);
                printf("    => HRESULT=0x%08X (null args)\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
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
    printf("Targets: wlxclip.dll + WLXDSPA.dll\n\n");

    SetSearchPaths();

    // --- wlxclip.dll ---
    HMODULE hClip = SafeLoadDll(L"undecomp\\Photo Gallery\\wlxclip.dll");
    if (hClip) {
        TestWlxclip(hClip);
        printf("\n[*] Unloading wlxclip.dll\n");
        FreeLibrary(hClip);
    }

    // --- WLXDSPA.dll ---
    HMODULE hDspa = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXDSPA.dll");
    if (hDspa) {
        TestWLXDSPA(hDspa);
        printf("\n[*] Unloading WLXDSPA.dll\n");
        FreeLibrary(hDspa);
    }

    printf("\n=== Done ===\n");
    return 0;
}
