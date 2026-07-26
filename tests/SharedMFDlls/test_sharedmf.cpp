// Shared MF DLLs Test Harness
// Tests both RSCMFT.dll (Rolling Shutter Correction) and MPG4DEMUX.dll (MPEG-4 Demuxer)
#include <windows.h>
#include <unknwn.h>
#include <cstdio>
#include <cstdlib>

// RSCMFT exports (standard COM DLL)
typedef HRESULT (__stdcall *DllCanUnloadNowProc)(void);
typedef HRESULT (__stdcall *DllGetClassObjectProc)(REFCLSID, REFIID, void**);
typedef HRESULT (__stdcall *DllRegisterServerProc)(void);
typedef HRESULT (__stdcall *DllUnregisterServerProc)(void);

static void TestRSCMFT(const wchar_t* dllDir) {
    printf("========================================\n");
    printf("  RSCMFT.dll - Rolling Shutter Correction MFT\n");
    printf("========================================\n\n");

    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Shared\\RSCMFT.dll", MAX_PATH, fullPath, NULL);
    printf("[*] DLL path: %ls\n", fullPath);

    HMODULE hMod = LoadLibraryExW(fullPath, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!hMod) {
        printf("[!] LoadLibraryEx failed: error %u\n", GetLastError());
        printf("[*] Trying SetDllDirectory fallback...\n");
        wchar_t sharedDir[MAX_PATH];
        GetFullPathNameW(L"undecomp\\Shared", MAX_PATH, sharedDir, NULL);
        SetDllDirectoryW(sharedDir);
        hMod = LoadLibraryW(fullPath);
        if (!hMod) {
            printf("[!] LoadLibrary also failed: error %u\n", GetLastError());
            return;
        }
    }
    printf("[+] DLL loaded at base: 0x%p\n\n", hMod);

    const char* exports[] = {
        "DllCanUnloadNow", "DllGetClassObject",
        "DllRegisterServer", "DllUnregisterServer"
    };

    printf("--- Export Table ---\n");
    for (int i = 0; i < 4; i++) {
        FARPROC p = GetProcAddress(hMod, exports[i]);
        printf("  %-30s -> 0x%p (RVA 0x%08X)\n", exports[i], p,
               p ? (DWORD)((BYTE*)p - (BYTE*)hMod) : 0);
    }

    printf("\n--- DllCanUnloadNow ---\n");
    {
        DllCanUnloadNowProc pFn = (DllCanUnloadNowProc)GetProcAddress(hMod, "DllCanUnloadNow");
        if (pFn) {
            HRESULT hr = E_FAIL;
            __try { hr = pFn(); } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  [!] Exception: 0x%08X\n", GetExceptionCode());
            }
            printf("  Result: 0x%08X (S_OK=%d)\n", hr, hr == 0);
        }
    }

    printf("\n--- DllRegisterServer ---\n");
    {
        DllRegisterServerProc pFn = (DllRegisterServerProc)GetProcAddress(hMod, "DllRegisterServer");
        if (pFn) {
            HRESULT hr = E_FAIL;
            __try { hr = pFn(); } __except(EXCEPTION_EXECUTE_HANDLER) {
                DWORD code = GetExceptionCode();
                printf("  [!] Exception: 0x%08X", code);
                if (code == 0xE06D7363) printf(" (C++ exception)");
                printf("\n");
            }
            printf("  Result: 0x%08X\n", hr);
        }
    }

    printf("\n--- DllUnregisterServer ---\n");
    {
        DllUnregisterServerProc pFn = (DllUnregisterServerProc)GetProcAddress(hMod, "DllUnregisterServer");
        if (pFn) {
            HRESULT hr = E_FAIL;
            __try { hr = pFn(); } __except(EXCEPTION_EXECUTE_HANDLER) {
                DWORD code = GetExceptionCode();
                printf("  [!] Exception: 0x%08X", code);
                if (code == 0xE06D7363) printf(" (C++ exception)");
                printf("\n");
            }
            printf("  Result: 0x%08X\n", hr);
        }
    }

    printf("\n--- DllGetClassObject (CLSID_RSCorrection) ---\n");
    {
        DllGetClassObjectProc pFn = (DllGetClassObjectProc)GetProcAddress(hMod, "DllGetClassObject");
        if (pFn) {
            // CLSID_RSCorrection: {B5E4A2E6-3B44-42B2-A5E4-3B3D3E8F5F1A} (from RTTI)
            CLSID clsid = {0x6BA2E4B5, 0x443B, 0xB242, {0xA5, 0xE4, 0x3B, 0x3D, 0x3E, 0x8F, 0x5F, 0x1A}};
            IID iid_IClassFactory = {0x00000001, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
            void* pFactory = NULL;
            HRESULT hr = E_FAIL;
            __try {
                hr = pFn(clsid, iid_IClassFactory, &pFactory);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                DWORD code = GetExceptionCode();
                printf("  [!] Exception: 0x%08X", code);
                if (code == 0xE06D7363) printf(" (C++ exception)");
                printf("\n");
            }
            printf("  Result: 0x%08X, Factory: 0x%p\n", hr, pFactory);
            if (pFactory) {
                IUnknown* pUnk = (IUnknown*)pFactory;
                printf("  [*] Calling Release() on factory...\n");
                ULONG refs = pUnk->Release();
                printf("  Release() returned: %lu\n", refs);
            }
        }
    }

    FreeLibrary(hMod);
    printf("\n[+] RSCMFT.dll unloaded\n\n");
}

static void TestMPG4DEMUX(const wchar_t* dllDir) {
    printf("========================================\n");
    printf("  MPG4DEMUX.dll - MPEG-4 Demuxer\n");
    printf("========================================\n\n");

    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Shared\\MPG4DEMUX.dll", MAX_PATH, fullPath, NULL);
    printf("[*] DLL path: %ls\n", fullPath);

    HMODULE hMod = LoadLibraryExW(fullPath, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!hMod) {
        printf("[!] LoadLibraryEx failed: error %u\n", GetLastError());
        printf("[*] Trying SetDllDirectory fallback...\n");
        wchar_t sharedDir[MAX_PATH];
        GetFullPathNameW(L"undecomp\\Shared", MAX_PATH, sharedDir, NULL);
        SetDllDirectoryW(sharedDir);
        hMod = LoadLibraryW(fullPath);
        if (!hMod) {
            printf("[!] LoadLibrary also failed: error %u\n", GetLastError());
            return;
        }
    }
    printf("[+] DLL loaded at base: 0x%p\n\n", hMod);

    const char* exports[] = {
        "DllCanUnloadNow", "DllGetClassObject",
        "DllRegisterServer", "DllUnregisterServer"
    };

    printf("--- Export Table ---\n");
    for (int i = 0; i < 4; i++) {
        FARPROC p = GetProcAddress(hMod, exports[i]);
        printf("  %-30s -> 0x%p (RVA 0x%08X)\n", exports[i], p,
               p ? (DWORD)((BYTE*)p - (BYTE*)hMod) : 0);
    }

    printf("\n--- DllCanUnloadNow ---\n");
    {
        DllCanUnloadNowProc pFn = (DllCanUnloadNowProc)GetProcAddress(hMod, "DllCanUnloadNow");
        if (pFn) {
            HRESULT hr = E_FAIL;
            __try { hr = pFn(); } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  [!] Exception: 0x%08X\n", GetExceptionCode());
            }
            printf("  Result: 0x%08X (S_OK=%d)\n", hr, hr == 0);
        }
    }

    printf("\n--- DllRegisterServer ---\n");
    {
        DllRegisterServerProc pFn = (DllRegisterServerProc)GetProcAddress(hMod, "DllRegisterServer");
        if (pFn) {
            HRESULT hr = E_FAIL;
            __try { hr = pFn(); } __except(EXCEPTION_EXECUTE_HANDLER) {
                DWORD code = GetExceptionCode();
                printf("  [!] Exception: 0x%08X", code);
                if (code == 0xE06D7363) printf(" (C++ exception)");
                printf("\n");
            }
            printf("  Result: 0x%08X\n", hr);
        }
    }

    printf("\n--- DllUnregisterServer ---\n");
    {
        DllUnregisterServerProc pFn = (DllUnregisterServerProc)GetProcAddress(hMod, "DllUnregisterServer");
        if (pFn) {
            HRESULT hr = E_FAIL;
            __try { hr = pFn(); } __except(EXCEPTION_EXECUTE_HANDLER) {
                DWORD code = GetExceptionCode();
                printf("  [!] Exception: 0x%08X", code);
                if (code == 0xE06D7363) printf(" (C++ exception)");
                printf("\n");
            }
            printf("  Result: 0x%08X\n", hr);
        }
    }

    printf("\n--- DllGetClassObject (CLSID_MPEG4Demultiplexer) ---\n");
    {
        DllGetClassObjectProc pFn = (DllGetClassObjectProc)GetProcAddress(hMod, "DllGetClassObject");
        if (pFn) {
            CLSID clsid = {0};
            IID iid_IClassFactory = {0x00000001, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
            void* pFactory = NULL;
            HRESULT hr = E_FAIL;
            __try {
                hr = pFn(clsid, iid_IClassFactory, &pFactory);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                DWORD code = GetExceptionCode();
                printf("  [!] Exception: 0x%08X", code);
                if (code == 0xE06D7363) printf(" (C++ exception)");
                printf("\n");
            }
            printf("  Result: 0x%08X (expected E_INVALIDARG with zero CLSID)\n", hr);
            if (pFactory) {
                IUnknown* pUnk = (IUnknown*)pFactory;
                printf("  [*] Unexpected factory returned, releasing...\n");
                ULONG refs = pUnk->Release();
                printf("  Release() returned: %lu\n", refs);
            }
        }
    }

    FreeLibrary(hMod);
    printf("\n[+] MPG4DEMUX.dll unloaded\n\n");
}

int main(int argc, char** argv) {
    printf("=== Shared MF DLLs Test Harness ===\n");
    printf("=== RSCMFT + MPG4DEMUX Dual Analysis ===\n\n");

    wchar_t mainDir[MAX_PATH];
    GetFullPathNameW(L".", MAX_PATH, mainDir, NULL);
    AddDllDirectory(mainDir);

    wchar_t sharedDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Shared", MAX_PATH, sharedDir, NULL);
    AddDllDirectory(sharedDir);

    __try {
        TestRSCMFT(sharedDir);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("[!] CRITICAL: Unhandled exception in RSCMFT tests: 0x%08X\n", GetExceptionCode());
    }

    __try {
        TestMPG4DEMUX(sharedDir);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("[!] CRITICAL: Unhandled exception in MPG4DEMUX tests: 0x%08X\n", GetExceptionCode());
    }

    printf("=== Test Complete ===\n");
    return 0;
}
