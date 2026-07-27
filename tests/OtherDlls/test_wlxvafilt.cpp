// OtherDlls Test Harness - WLXVAFilt.dll
// Loads the DirectShow filter DLL, probes all exports with SEH protection
// Verifies DllCanUnloadNow, DllGetClassObject, DllRegisterServer, DllUnregisterServer
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#include <unknwn.h>
#include <cstdio>
#include <cstdlib>

// ============================================================
// GUIDs
// ============================================================
static const CLSID CLSID_Empty = {0,0,0,{0,0,0,0,0,0,0,0}};
static const IID IID_IUnknown_Cast = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const IID IID_IClassFactory_Cast = {0x00000001,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

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
// Export typedefs (standard COM server - 4 exports)
// ============================================================
typedef HRESULT (__stdcall *DllCanUnloadNowFunc)();
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServerFunc)();
typedef HRESULT (__stdcall *DllUnregisterServerFunc)();

// ============================================================
// WLXVAFilt.dll - 4 exports (standard COM DirectShow filter server)
// 8 registered DirectShow filters:
//   Record Queue, WMT Switch Filter, WMT Virtual Renderer,
//   WMT Virtual Source, WMT DV Extract Filter, WM VIH2 Fix,
//   WMT Sample Info Filter, Capture File Writer
// ============================================================
static void TestWLXVAFilt(HMODULE h) {
    printf("\n========================================\n");
    printf("  WLXVAFilt.dll Export Probing\n");
    printf("  (Video Acquisition Filters)\n");
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
                if (hr == S_OK)
                    printf("    [OK] S_OK (no outstanding references)\n");
                else if (hr == S_FALSE)
                    printf("    [OK] S_FALSE (references exist)\n");
                else
                    printf("    [?] Unexpected result\n");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllCanUnloadNow (second call - should be idempotent)
    {
        printf("\n[*] DllCanUnloadNow (idempotency check)\n");
        DllCanUnloadNowFunc fn = (DllCanUnloadNowFunc)GetProcAddress(h, "DllCanUnloadNow");
        if (fn) {
            __try {
                HRESULT hr = fn();
                printf("    => HRESULT=0x%08X (should match first call)\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        }
    }

    // DllGetClassObject with empty CLSID + null ppv
    {
        printf("\n[*] DllGetClassObject (empty CLSID, null ppv)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, NULL);
                printf("    => HRESULT=0x%08X\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X (expected - null ppv)\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllGetClassObject with empty CLSID + valid ppv
    {
        printf("\n[*] DllGetClassObject (empty CLSID, valid ppv)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            LPVOID pUnk = NULL;
            __try {
                HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, &pUnk);
                printf("    => HRESULT=0x%08X\n", hr);
                if (SUCCEEDED(hr) && pUnk) {
                    printf("    [+] IUnknown obtained at 0x%p\n", pUnk);
                    ((IUnknown*)pUnk)->Release();
                } else {
                    printf("    [OK] No object for empty CLSID (expected)\n");
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                if (pUnk) ((IUnknown*)pUnk)->Release();
            }
        }
    }

    // DllGetClassObject with zeroed CLSID + valid ppv
    {
        printf("\n[*] DllGetClassObject (zeroed CLSID, valid ppv)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            CLSID zeroed = {0,0,0,{0,0,0,0,0,0,0,0}};
            LPVOID pUnk = NULL;
            __try {
                HRESULT hr = fn(zeroed, IID_IUnknown_Cast, &pUnk);
                printf("    => HRESULT=0x%08X\n", hr);
                if (pUnk) ((IUnknown*)pUnk)->Release();
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                if (pUnk) ((IUnknown*)pUnk)->Release();
            }
        }
    }

    // DllGetClassObject with Record Queue CLSID
    // CLSID not known from static analysis (in reloc table), try probing
    {
        printf("\n[*] DllGetClassObject (IClassFactory IID as CLSID probe)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            LPVOID pUnk = NULL;
            __try {
                HRESULT hr = fn(IID_IClassFactory_Cast, IID_IUnknown_Cast, &pUnk);
                printf("    => HRESULT=0x%08X\n", hr);
                if (SUCCEEDED(hr) && pUnk) {
                    printf("    [+] IUnknown obtained at 0x%p\n", pUnk);
                    ((IUnknown*)pUnk)->Release();
                } else {
                    printf("    [OK] No object (expected - random CLSID)\n");
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
            printf("    [skip] Would register 8 DirectShow filters via IFilterMapper2\n");
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
            printf("    [skip] Would unregister DirectShow filters\n");
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
    printf("Target: WLXVAFilt.dll (Video Acquisition Filters)\n\n");

    SetSearchPaths();

    // Initialize COM (needed for CLSIDFromString and IFilterMapper2)
    HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    printf("[*] CoInitializeEx: HRESULT=0x%08X\n\n", hrCoInit);

    // --- WLXVAFilt.dll ---
    HMODULE hVAFilt = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXVAFilt.dll");
    if (hVAFilt) {
        TestWLXVAFilt(hVAFilt);

        printf("\n[*] Module info:\n");
        wchar_t modPath[MAX_PATH];
        GetModuleFileNameW(hVAFilt, modPath, MAX_PATH);
        printf("    Path: %ls\n", modPath);

        DWORD verSize = GetFileVersionInfoSizeW(modPath, NULL);
        printf("    Version info size: %u bytes\n", verSize);

        printf("\n[*] Unloading WLXVAFilt.dll\n");
        FreeLibrary(hVAFilt);
    }

    if (SUCCEEDED(hrCoInit))
        CoUninitialize();

    printf("\n=== Done ===\n");
    return 0;
}
