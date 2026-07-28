// WLXGrinderScheduler.dll Dedicated Test Harness
// Probes all 4 standard COM exports with SEH protection
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <unknwn.h>
#include <cstdio>

// ============================================================
// GUIDs
// ============================================================
static const CLSID CLSID_Empty = {0,0,0,{0,0,0,0,0,0,0,0}};
static const IID IID_IUnknown_Cast = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

// ============================================================
// Export typedefs
// ============================================================
typedef HRESULT (__stdcall *DllCanUnloadNowFunc)();
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServerFunc)();
typedef HRESULT (__stdcall *DllUnregisterServerFunc)();

// ============================================================
// Safe loader with search-path setup
// ============================================================
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
// Test all 4 exports
// ============================================================
static void TestAllExports(HMODULE h) {
    printf("\n========================================\n");
    printf("  WLXGrinderScheduler.dll Export Probe\n");
    printf("========================================\n");

    // 1) DllCanUnloadNow
    {
        printf("\n--- [1/4] DllCanUnloadNow ---\n");
        DllCanUnloadNowFunc fn = (DllCanUnloadNowFunc)GetProcAddress(h, "DllCanUnloadNow");
        if (!fn) { printf("[FAIL] GetProcAddress failed\n"); return; }
        printf("    ptr=0x%p\n", fn);
        __try {
            HRESULT hr = fn();
            printf("    HRESULT=0x%08X (%s)\n", hr,
                hr == S_OK ? "S_OK" : hr == S_FALSE ? "S_FALSE" : "OTHER");
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("    SEH exception 0x%08X\n", GetExceptionCode());
        }
    }

    // 2) DllGetClassObject (empty CLSID — expect failure)
    {
        printf("\n--- [2/4] DllGetClassObject (empty CLSID) ---\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (!fn) { printf("[FAIL] GetProcAddress failed\n"); return; }
        printf("    ptr=0x%p\n", fn);
        __try {
            HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, NULL);
            printf("    HRESULT=0x%08X (empty CLSID, null ppv)\n", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("    SEH exception 0x%08X\n", GetExceptionCode());
        }
    }

    // 3) DllGetClassObject (GrinderScheduler CLSID — expect S_OK + IClassFactory)
    {
        printf("\n--- [3/4] DllGetClassObject (GrinderScheduler CLSID) ---\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (!fn) { printf("[FAIL] GetProcAddress failed\n"); return; }
        printf("    ptr=0x%p\n", fn);
        CLSID clsid;
        CLSIDFromString(L"{121244C5-B0AD-45fb-9D8E-B893B917D053}", &clsid);
        LPVOID pUnk = NULL;
        __try {
            HRESULT hr = fn(clsid, IID_IUnknown_Cast, &pUnk);
            printf("    HRESULT=0x%08X\n", hr);
            if (SUCCEEDED(hr) && pUnk) {
                printf("    [+] IUnknown at 0x%p\n", pUnk);
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
            printf("    SEH exception 0x%08X\n", GetExceptionCode());
            if (pUnk) ((IUnknown*)pUnk)->Release();
        }
    }

    // 4) DllRegisterServer + DllUnregisterServer (detect only — skip modification)
    {
        printf("\n--- [4/4] Registry exports (detect only) ---\n");
        DllRegisterServerFunc fnReg = (DllRegisterServerFunc)GetProcAddress(h, "DllRegisterServer");
        DllUnregisterServerFunc fnUnreg = (DllUnregisterServerFunc)GetProcAddress(h, "DllUnregisterServer");
        printf("    DllRegisterServer   ptr=0x%p [skip — would modify registry]\n", fnReg);
        printf("    DllUnregisterServer ptr=0x%p [skip — would modify registry]\n", fnUnreg);
        if (!fnReg || !fnUnreg) printf("    [!] One or both registry exports missing\n");
        else printf("    [+] Both registry exports present\n");
    }
}

// ============================================================
// Main
// ============================================================
int main() {
    printf("=== WLXGrinderScheduler.dll Test Harness ===\n");

    wchar_t galleryDir[MAX_PATH], sharedDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
    GetFullPathNameW(L"undecomp\\Shared", MAX_PATH, sharedDir, NULL);
    AddDllDirectory(galleryDir);
    AddDllDirectory(sharedDir);
    SetDllDirectoryW(galleryDir);

    HRESULT hrCo = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    printf("[*] CoInitializeEx: HRESULT=0x%08X\n\n", hrCo);

    HMODULE hDll = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXGrinderScheduler.dll");
    if (hDll) {
        TestAllExports(hDll);
        printf("\n[*] Unloading WLXGrinderScheduler.dll\n");
        FreeLibrary(hDll);
    }

    if (SUCCEEDED(hrCo))
        CoUninitialize();

    printf("\n=== Test Complete ===\n");
    return 0;
}
