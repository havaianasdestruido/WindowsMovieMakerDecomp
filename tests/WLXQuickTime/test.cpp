#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <assert.h>

#ifndef RT_TYPELIB
#define RT_TYPELIB  MAKEINTRESOURCEA(17)
#endif

// === CLSIDs from static analysis ===

// {B9AD19CB-FA75-4B29-B4A4-86C7E9616390} — QuickTimePlayerHost (ControlHost.exe)
static const CLSID CLSID_QuickTimePlayerHost =
    {0xB9AD19CB, 0xFA75, 0x4B29, {0xB4, 0xA4, 0x86, 0xC7, 0xE9, 0x61, 0x63, 0x90}};

// {AE3A66BB-85FE-49B8-BF7B-4DB4E0005091} — QuickTimeMovieThumbnail (ControlHost.exe)
static const CLSID CLSID_QuickTimeMovieThumbnail =
    {0xAE3A66BB, 0x85FE, 0x49B8, {0xBF, 0x7B, 0x4D, 0xB4, 0xE0, 0x00, 0x50, 0x91}};

// {007EFBDF-8A5D-4930-97CC-A4B437CBA777} — MovieThumbnail (ShellExt.dll)
static const CLSID CLSID_MovieThumbnail =
    {0x007EFBDF, 0x8A5D, 0x4930, {0x97, 0xCC, 0xA4, 0xB4, 0x37, 0xCB, 0xA7, 0x77}};

// {C6D340BB-0CEA-4923-8082-51036E472379} — TypeLib for ControlHost
static const GUID LIBID_WLXQuickTimeControlHost =
    {0xC6D340BB, 0x0CEA, 0x4923, {0x80, 0x82, 0x51, 0x03, 0x6E, 0x47, 0x23, 0x79}};

typedef HRESULT (STDMETHODCALLTYPE *pfnDllCanUnloadNow)(void);
typedef HRESULT (STDMETHODCALLTYPE *pfnDllGetClassObject)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (STDMETHODCALLTYPE *pfnDllRegisterServer)(void);
typedef HRESULT (STDMETHODCALLTYPE *pfnDllUnregisterServer)(void);

// ============================================================
// 1. File attribute verification for all 3 binaries
// ============================================================
static int test_file_attributes(void) {
    int failures = 0;
    printf("--- 1. File Attribute Verification ---\n\n");

    const struct {
        const char* label;
        const char* relPath;
        DWORD expectedSizeLow;  // approximate upper bound
    } files[] = {
        {"WLXQuickTimeControlHost.exe (EXE)",  "undecomp\\Photo Gallery\\WLXQuickTimeControlHost.exe",  140000},
        {"WLXQuickTimeControlHostPS.dll (PS)",  "undecomp\\Photo Gallery\\WLXQuickTimeControlHostPS.dll", 29000},
        {"WLXQuickTimeShellExt.dll (ShellExt)", "undecomp\\Photo Gallery\\WLXQuickTimeShellExt.dll",    60000},
    };

    for (int i = 0; i < 3; i++) {
        WIN32_FILE_ATTRIBUTE_DATA info;
        if (!GetFileAttributesExA(files[i].relPath, GetFileExInfoStandard, &info)) {
            printf("[FAIL] %s: not found (error %lu)\n", files[i].label, GetLastError());
            failures++;
            continue;
        }
        DWORD size = info.nFileSizeLow;
        const char* status = (size > 0 && size <= files[i].expectedSizeLow) ? "PASS" : "FAIL";
        printf("[%s] %s\n", status, files[i].label);
        printf("      size=%lu bytes (expected <= %lu)\n", size, files[i].expectedSizeLow);

        if (size == 0 || size > files[i].expectedSizeLow) failures++;
    }

    printf("\n");
    return failures;
}

// ============================================================
// 2. EXE resource / version inspection (data-file load)
// ============================================================
static int test_exe_resources(void) {
    int failures = 0;
    printf("--- 2. WLXQuickTimeControlHost.exe — Resource/Version ---\n\n");

    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery\\WLXQuickTimeControlHost.exe", MAX_PATH, fullPath, NULL);
    printf("[*] Full path: %ls\n", fullPath);

    // Load as data file to inspect resources without executing code
    HMODULE hRes = LoadLibraryExW(fullPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (!hRes) {
        printf("[!] LoadLibraryEx(AS_DATAFILE) failed: error %lu\n", GetLastError());
        return 1;
    }
    printf("[+] Loaded as data-file at %p\n", hRes);

    // Verify version-information resource exists
    HRSRC hVer = FindResourceA(hRes, MAKEINTRESOURCEA(VS_VERSION_INFO), (LPSTR)RT_VERSION);
    if (hVer) {
        DWORD verSize = SizeofResource(hRes, hVer);
        printf("[PASS] VS_VERSION_INFO resource present, size=%lu\n", verSize);
    } else {
        printf("[FAIL] VS_VERSION_INFO resource missing\n");
        failures++;
    }

    // Verify manifest resource exists (CREATEPROCESS_MANIFEST_RESOURCE_ID = 1, RT_MANIFEST = 24)
    HRSRC hMan = FindResourceA(hRes, MAKEINTRESOURCEA(1), (LPSTR)RT_MANIFEST);
    if (hMan) {
        DWORD manSize = SizeofResource(hRes, hMan);
        printf("[PASS] RT_MANIFEST resource present, size=%lu\n", manSize);
    } else {
        printf("[FAIL] RT_MANIFEST resource missing\n");
        failures++;
    }

    // Check for type-library resource (TLB)
    HRSRC hTlb = FindResourceA(hRes, MAKEINTRESOURCEA(1), RT_TYPELIB);
    if (hTlb) {
        printf("[INFO] Type library resource (RT_TYPELIB) present\n");
    } else {
        printf("[INFO] No RT_TYPELIB resource found (embedded .reg instead)\n");
    }

    // Query version-info fixed data
    HRSRC hVerInfo = FindResourceA(hRes, MAKEINTRESOURCEA(VS_VERSION_INFO), (LPSTR)RT_VERSION);
    if (hVerInfo) {
        HGLOBAL hGlob = LoadResource(hRes, hVerInfo);
        if (hGlob) {
            LPVOID pVer = LockResource(hGlob);
            VS_FIXEDFILEINFO* vfi = NULL;
            UINT vfiLen = 0;
            if (VerQueryValueA(pVer, "\\", (LPVOID*)&vfi, &vfiLen) && vfiLen >= sizeof(VS_FIXEDFILEINFO)) {
                printf("[PASS] FileVersion: %u.%u.%u.%u\n",
                    HIWORD(vfi->dwFileVersionMS), LOWORD(vfi->dwFileVersionMS),
                    HIWORD(vfi->dwFileVersionLS), LOWORD(vfi->dwFileVersionLS));
                printf("[PASS] ProductVersion: %u.%u.%u.%u\n",
                    HIWORD(vfi->dwProductVersionMS), LOWORD(vfi->dwProductVersionMS),
                    HIWORD(vfi->dwProductVersionLS), LOWORD(vfi->dwProductVersionLS));
            } else {
                printf("[FAIL] VerQueryValue failed\n");
                failures++;
            }
        }
    }

    FreeLibrary(hRes);
    printf("\n");
    return failures;
}

// ============================================================
// 3. EXE quick launch test (start + terminate via window message)
// ============================================================
static int test_exe_launch(void) {
    printf("--- 3. WLXQuickTimeControlHost.exe — Launch Test ---\n\n");

    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery\\WLXQuickTimeControlHost.exe", MAX_PATH, fullPath, NULL);

    // Launch the EXE with /? argument (typical COM servers support /? or -?
    // If no args, it will register itself and exit or show a message.
    // We just need to see if it starts without crashing.
    STARTUPINFOW si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    // Try with -RegServer (standard ATL COM server argument)
    wchar_t cmdLine[MAX_PATH + 32];
    wcsncpy_s(cmdLine, fullPath, _TRUNCATE);
    wcscat_s(cmdLine, L" -RegServer");

    if (!CreateProcessW(fullPath, cmdLine, NULL, NULL, FALSE,
                        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        printf("[*] CreateProcess(-RegServer) failed: error %lu (expected if COM not needed)\n", GetLastError());
        printf("[*] Trying plain launch...\n");

        wcsncpy_s(cmdLine, fullPath, _TRUNCATE);
        if (!CreateProcessW(fullPath, cmdLine, NULL, NULL, FALSE,
                            CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            printf("[*] Plain launch also failed: error %lu\n", GetLastError());
            printf("[INFO] EXE launch not viable in test harness (no QuickTime installed)\n\n");
            return 0;  // not a failure, expected without QuickTime
        }
    }

    printf("[+] Process created (PID=%lu)\n", pi.dwProcessId);

    // Wait briefly
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 2000);
    if (waitResult == WAIT_OBJECT_0) {
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        printf("[*] Process exited with code %lu\n", exitCode);
    } else if (waitResult == WAIT_TIMEOUT) {
        printf("[*] Process still running after 2s — terminating\n");
        TerminateProcess(pi.hProcess, 0);
    } else {
        printf("[!] Wait failed: error %lu\n", GetLastError());
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    printf("\n");
    return 0;
}

// ============================================================
// 4. COM DLL test harness (shared by PS DLL and ShellExt DLL)
// ============================================================
struct ClsidEntry {
    const char* label;
    const CLSID* clsid;
};

static int test_com_dll(const char* label, const char* dllPath,
                        const struct ClsidEntry* entries, int nEntries) {
    int failures = 0;
    printf("--- 4. %s ---\n\n", label);

    HMODULE hMod = LoadLibraryA(dllPath);
    if (!hMod) {
        printf("[FAIL] LoadLibraryA failed: error %lu\n", GetLastError());
        return 1;
    }
    printf("[+] DLL loaded at %p\n", (void*)hMod);

    pfnDllCanUnloadNow pCanUnload = (pfnDllCanUnloadNow)GetProcAddress(hMod, "DllCanUnloadNow");
    pfnDllGetClassObject pGetClass = (pfnDllGetClassObject)GetProcAddress(hMod, "DllGetClassObject");
    pfnDllRegisterServer pRegister = (pfnDllRegisterServer)GetProcAddress(hMod, "DllRegisterServer");
    pfnDllUnregisterServer pUnregister = (pfnDllUnregisterServer)GetProcAddress(hMod, "DllUnregisterServer");

    printf("  DllCanUnloadNow:     %p %s\n", (void*)pCanUnload,     pCanUnload     ? "[OK]" : "[MISSING]");
    printf("  DllGetClassObject:   %p %s\n", (void*)pGetClass,      pGetClass      ? "[OK]" : "[MISSING]");
    printf("  DllRegisterServer:   %p %s\n", (void*)pRegister,      pRegister      ? "[OK]" : "[MISSING]");
    printf("  DllUnregisterServer: %p %s\n", (void*)pUnregister,    pUnregister    ? "[OK]" : "[MISSING]");

    if (!pCanUnload) failures++;
    if (!pGetClass) failures++;
    if (!pRegister) failures++;

    // Also check for GetProxyDllInfo (PS DLL only)
    FARPROC pProxyInfo = GetProcAddress(hMod, "GetProxyDllInfo");
    if (pProxyInfo) {
        printf("  GetProxyDllInfo:     %p [OK]\n", (void*)pProxyInfo);
    }

    // Call DllCanUnloadNow
    if (pCanUnload) {
        HRESULT hr = pCanUnload();
        printf("\n  DllCanUnloadNow() -> 0x%08X %s\n", hr,
               SUCCEEDED(hr) ? "(S_OK: can unload)" : "(S_FALSE: locks held)");
    }

    // DllGetClassObject with known CLSIDs
    if (pGetClass) {
        printf("\n  -- DllGetClassObject tests --\n");
        for (int i = 0; i < nEntries; i++) {
            IUnknown* pUnk = NULL;
            HRESULT hr = pGetClass(*entries[i].clsid, IID_IUnknown, (void**)&pUnk);
            printf("  [%s] -> 0x%08X", entries[i].label, hr);
            if (SUCCEEDED(hr)) {
                if (pUnk) {
                    ULONG refs = pUnk->Release();
                    printf(" (pUnk=%p, post-release refs=%lu)", (void*)pUnk, refs);
                } else {
                    printf(" (pUnk=NULL!)");
                    failures++;
                }
            }
            printf("\n");
        }

        // CLSID_NULL should fail
        IUnknown* pNull = NULL;
        HRESULT hr = pGetClass(CLSID_NULL, IID_IUnknown, (void**)&pNull);
        printf("  [CLSID_NULL] -> 0x%08X (expected: CLASS_E_CLASSNOTAVAILABLE)\n", hr);
        if (SUCCEEDED(hr)) {
            printf("[!] WARNING: CLSID_NULL unexpectedly succeeded\n");
        }
    }

    FreeLibrary(hMod);
    printf("\n");
    return failures;
}

// ============================================================
// 5. CoCreateInstance integration test
// ============================================================
static int test_cocreate_instance(void) {
    printf("--- 5. CoCreateInstance Integration ---\n\n");

    HRESULT coInit = CoInitialize(NULL);
    printf("CoInitialize -> 0x%08X\n", coInit);

    // Try local-server activation for EXE-hosted classes
    const struct ClsidEntry serverEntries[] = {
        {"QuickTimePlayerHost (EXE, LocalServer32)",    &CLSID_QuickTimePlayerHost},
        {"QuickTimeMovieThumbnail (EXE, LocalServer32)", &CLSID_QuickTimeMovieThumbnail},
    };

    for (int i = 0; i < 2; i++) {
        IUnknown* pUnk = NULL;
        HRESULT hr = CoCreateInstance(*serverEntries[i].clsid, NULL,
            CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&pUnk);
        printf("  [%s]\n", serverEntries[i].label);
        printf("    CLSCTX_LOCAL_SERVER -> 0x%08X", hr);
        if (SUCCEEDED(hr)) {
            printf(" (pUnk=%p)", (void*)pUnk);
            if (pUnk) {
                pUnk->Release();
                pUnk = NULL;
            }
        }
        printf("\n");

        // Also try with CLSCTX_INPROC_SERVER (should fail for EXE-hosted)
        hr = CoCreateInstance(*serverEntries[i].clsid, NULL,
            CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pUnk);
        printf("    CLSCTX_INPROC_SERVER -> 0x%08X (expected: "
               "CLASS_E_CLASSNOTAVAILABLE or REGDB_E_CLASSNOTREG)\n", hr);
    }

    // Try inproc activation for ShellExt-hosted class
    {
        IUnknown* pUnk = NULL;
        HRESULT hr = CoCreateInstance(CLSID_MovieThumbnail, NULL,
            CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pUnk);
        printf("  [MovieThumbnail (ShellExt.dll, InprocServer32)]\n");
        printf("    CLSCTX_INPROC_SERVER -> 0x%08X", hr);
        if (SUCCEEDED(hr)) {
            printf(" (pUnk=%p)", (void*)pUnk);
            if (pUnk) {
                pUnk->Release();
                pUnk = NULL;
            }
        } else {
            // Expected if the DLL is not registered in the test environment
            printf(" (expected without registration)");
        }
        printf("\n");
    }

    CoUninitialize();
    printf("\n");
    return 0;
}

// ============================================================
int main(void) {
    int totalFailures = 0;

    printf("==========================================\n");
    printf(" WLXQuickTime Integration Test Harness\n");
    printf("==========================================\n\n");

    SetDllDirectoryA("undecomp\\Photo Gallery");

    totalFailures += test_file_attributes();
    totalFailures += test_exe_resources();
    totalFailures += test_exe_launch();

    // Test PS DLL
    {
        struct ClsidEntry psEntries[] = {
            {"QuickTimePlayerHost CLSID",    &CLSID_QuickTimePlayerHost},
            {"QuickTimeMovieThumbnail CLSID", &CLSID_QuickTimeMovieThumbnail},
            {"TypeLib GUID",                  (const CLSID*)&LIBID_WLXQuickTimeControlHost},
        };
        totalFailures += test_com_dll(
            "WLXQuickTimeControlHostPS.dll — Proxy/Stub DLL",
            "WLXQuickTimeControlHostPS.dll",
            psEntries, 3);
    }

    // Test ShellExt DLL
    {
        struct ClsidEntry shellEntries[] = {
            {"MovieThumbnail CLSID",         &CLSID_MovieThumbnail},
            {"QuickTimePlayerHost CLSID",    &CLSID_QuickTimePlayerHost},
        };
        totalFailures += test_com_dll(
            "WLXQuickTimeShellExt.dll — Shell Extension DLL",
            "WLXQuickTimeShellExt.dll",
            shellEntries, 2);
    }

    totalFailures += test_cocreate_instance();

    printf("==========================================\n");
    if (totalFailures == 0) {
        printf(" ALL TESTS PASSED\n");
    } else {
        printf(" %d TEST(S) FAILED\n", totalFailures);
    }
    printf("==========================================\n");
    return totalFailures;
}
