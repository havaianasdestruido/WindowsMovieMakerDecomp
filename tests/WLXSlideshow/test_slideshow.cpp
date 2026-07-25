/*
 * test_slideshow.cpp
 *
 * Test harness for WLXSlideshow.dll reverse engineering analysis.
 * Loads the original binary and probes COM object creation for all known CLSIDs.
 *
 * Build:
 *   cl /EHsc /W4 test_slideshow.cpp /link ole32.lib oleaut32.lib advapi32.lib
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objbase.h>
#include <psapi.h>
#include <cstdio>
#include <cstdlib>

// ============================================================================
// Known COM CLSIDs from binary analysis
// ============================================================================
// {84FBA192-4F8D-4A5D-94DE-083446ACC1D0} - COM GUID (WLXSlideshow) - found in UTF-16 strings
static const CLSID CLSID_WLXSlideshow_Main =
    { 0x84FBA192, 0x4F8D, 0x4A5D, { 0x94, 0xDE, 0x08, 0x34, 0x46, 0xAC, 0xC1, 0xD0 } };

// {8095E7A5-286D-4881-B3A5-9AC37A18B836} - SimpleSlideshowDisplay (from analysis.md RTTI)
static const CLSID CLSID_SimpleSlideshowDisplay =
    { 0x8095E7A5, 0x286D, 0x4881, { 0xB3, 0xA5, 0x9A, 0xC3, 0x7A, 0x18, 0xB8, 0x36 } };

// {04A3F24B-E082-467B-9BE2-4A7C6E56C848} - SlideshowExtension (from analysis.md RTTI)
static const CLSID CLSID_SlideshowExtension =
    { 0x04A3F24B, 0xE082, 0x467B, { 0x9B, 0xE2, 0x4A, 0x7C, 0x6E, 0x56, 0xC8, 0x48 } };

// {07C8B3C4-E53D-4C55-8212-8E77A346E29A} - TimelineDisplay (from analysis.md RTTI)
static const CLSID CLSID_TimelineDisplay =
    { 0x07C8B3C4, 0xE53D, 0x4C55, { 0x82, 0x12, 0x8E, 0x77, 0xA3, 0x46, 0xE2, 0x9A } };

// ============================================================================
// Typedefs for DLL exports
// ============================================================================
typedef HRESULT (WINAPI *pfnDllCanUnloadNow)(void);
typedef HRESULT (WINAPI *pfnDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
typedef HRESULT (WINAPI *pfnDllRegisterServer)(void);
typedef HRESULT (WINAPI *pfnDllUnregisterServer)(void);

// ============================================================================
// Helper macros
// ============================================================================
#define LOG(fmt, ...) printf("  " fmt "\n", ##__VA_ARGS__)
#define LOG_HR(hr, label) printf("  %-40s HRESULT=0x%08X\n", label, (unsigned)(hr))

// ============================================================================
// Test structure
// ============================================================================
struct TestResult {
    const char* name;
    HRESULT hr;
    bool passed;
};

static TestResult g_results[64];
static int g_resultCount = 0;

static void RecordResult(const char* name, HRESULT hr, bool passed) {
    if (g_resultCount < 64) {
        g_results[g_resultCount].name = name;
        g_results[g_resultCount].hr = hr;
        g_results[g_resultCount].passed = passed;
        g_resultCount++;
    }
}

// ============================================================================
// Test: DllCanUnloadNow
// ============================================================================
static void Test_DllCanUnloadNow(pfnDllCanUnloadNow fn) {
    printf("\n[TEST] DllCanUnloadNow()\n");
    HRESULT hr = fn();
    LOG("Return value: 0x%08X (%s)", (unsigned)hr,
        hr == S_OK ? "S_OK (can unload)" :
        hr == S_FALSE ? "S_FALSE (cannot unload)" : "Other");
    LOG_HR(hr, "DllCanUnloadNow");
    RecordResult("DllCanUnloadNow", hr, SUCCEEDED(hr));
}

// ============================================================================
// Test: DllGetClassObject with various CLSIDs
// ============================================================================
static void Test_DllGetClassObject(pfnDllGetClassObject fn, REFCLSID rclsid, const char* name) {
    printf("\n[TEST] DllGetClassObject(%s)\n", name);
    printf("  CLSID: ");
    LPOLESTR clsidStr = NULL;
    if (StringFromCLSID(rclsid, &clsidStr) == S_OK) {
        printf("%ls\n", clsidStr);
        CoTaskMemFree(clsidStr);
    } else {
        printf("(conversion failed)\n");
    }

    IClassFactory* pFactory = NULL;
    HRESULT hr = fn(rclsid, IID_IClassFactory, (LPVOID*)&pFactory);
    LOG_HR(hr, "DllGetClassObject");

    if (SUCCEEDED(hr) && pFactory) {
        // Log vtable address
        void** vtable = *(void***)pFactory;
        LOG("  vtable address: %p", (void*)vtable);

        // Dump first 8 vtable entries (IUnknown + IClassFactory)
        LOG("  vtable layout (first 8 entries):");
        for (int i = 0; i < 8; i++) {
            LOG("    [%d] %p", i, vtable[i]);
        }

        // Try to create instance
        printf("\n  [IClassFactory::CreateInstance]\n");
        IUnknown* pObj = NULL;
        HRESULT hrCreate = pFactory->CreateInstance(NULL, IID_IUnknown, (LPVOID*)&pObj);
        LOG_HR(hrCreate, "CreateInstance(IID_IUnknown)");

        if (SUCCEEDED(hrCreate) && pObj) {
            void** objVtable = *(void***)pObj;
            LOG("  Object vtable address: %p", (void*)objVtable);
            LOG("  Object vtable layout (first 16 entries):");
            for (int i = 0; i < 16; i++) {
                LOG("    [%d] %p", i, objVtable[i]);
            }

            // Query for IClassFactory (second QI test)
            IClassFactory* pQI = NULL;
            HRESULT hrQI = pObj->QueryInterface(IID_IClassFactory, (LPVOID*)&pQI);
            LOG_HR(hrQI, "QI(IID_IClassFactory)");
            if (SUCCEEDED(hrQI) && pQI) {
                pQI->Release();
            }

            pObj->Release();
        } else if (FAILED(hrCreate)) {
            LOG("  CreateInstance failed as expected (DLL may need dependencies)");
        }

        pFactory->Release();
    }

    char label[128];
    snprintf(label, sizeof(label), "DllGetClassObject(%s)", name);
    RecordResult(label, hr, SUCCEEDED(hr));
}

// ============================================================================
// Test: CoCreateInstance
// ============================================================================
static void Test_CoCreateInstance(REFCLSID rclsid, const char* name) {
    printf("\n[TEST] CoCreateInstance(%s)\n", name);
    IUnknown* pObj = NULL;
    HRESULT hr = CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER,
        IID_IUnknown, (LPVOID*)&pObj);
    LOG_HR(hr, "CoCreateInstance");

    if (SUCCEEDED(hr) && pObj) {
        void** vtable = *(void***)pObj;
        LOG("  vtable address: %p", (void*)vtable);
        pObj->Release();
    }

    char label[128];
    snprintf(label, sizeof(label), "CoCreateInstance(%s)", name);
    RecordResult(label, hr, true); // Record even if it fails - that's informative
}

// ============================================================================
// Test: DLL export function addresses
// ============================================================================
static void Test_ExportAddresses(HMODULE hDll) {
    printf("\n[TEST] DLL Export Addresses\n");
    const char* exportNames[] = {
        "DllCanUnloadNow",
        "DllGetClassObject",
        "DllRegisterServer",
        "DllUnregisterServer"
    };

    for (const char* name : exportNames) {
        FARPROC addr = GetProcAddress(hDll, name);
        if (addr) {
            LOG("  %-25s -> %p (RVA=0x%X)", name, (void*)addr,
                (unsigned)((ULONG_PTR)addr - (ULONG_PTR)hDll));
        } else {
            LOG("  %-25s -> NOT FOUND", name);
        }
    }
}

// ============================================================================
// Test: Examine DLL module info
// ============================================================================
static void Test_ModuleInfo(HMODULE hDll) {
    printf("\n[TEST] Module Info\n");
    MODULEINFO modInfo;
    if (GetModuleInformation(GetCurrentProcess(), hDll, &modInfo, sizeof(modInfo))) {
        LOG("  Base address:    %p", modInfo.lpBaseOfDll);
        LOG("  Size of image:   0x%X bytes", (unsigned)modInfo.SizeOfImage);
        LOG("  Entry point:     %p", modInfo.EntryPoint);
    }

    // Get module file name
    WCHAR szPath[MAX_PATH];
    DWORD len = GetModuleFileNameW(hDll, szPath, MAX_PATH);
    if (len > 0) {
        char pathA[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, szPath, -1, pathA, MAX_PATH, NULL, NULL);
        LOG("  Path: %s", pathA);
    }
}

// ============================================================================
// Test: Registry check
// ============================================================================
static void Test_RegistryCheck() {
    printf("\n[TEST] Registry CLSID Entries\n");
    const char* clsids[] = {
        "{84FBA192-4F8D-4A5D-94DE-083446ACC1D0}",
        "{8095E7A5-286D-4881-B3A5-9AC37A18B836}",
        "{04A3F24B-E082-467B-9BE2-4A7C6E56C848}",
        "{07C8B3C4-E53D-4C55-8212-8E77A346E29A}"
    };
    const char* names[] = {
        "WLXSlideshow_Main",
        "SimpleSlideshowDisplay",
        "SlideshowExtension",
        "TimelineDisplay"
    };

    for (int i = 0; i < 4; i++) {
        char keyPath[256];
        snprintf(keyPath, sizeof(keyPath), "CLSID\\%s\\InprocServer32", clsids[i]);

        HKEY hKey;
        LONG result = RegOpenKeyExA(HKEY_CLASSES_ROOT, keyPath, 0, KEY_READ, &hKey);
        if (result == ERROR_SUCCESS) {
            char value[MAX_PATH];
            DWORD valueSize = sizeof(value);
            DWORD type = 0;
            result = RegQueryValueExA(hKey, NULL, NULL, &type, (LPBYTE)value, &valueSize);
            if (result == ERROR_SUCCESS) {
                LOG("  %-30s -> %s", names[i], value);
            } else {
                LOG("  %-30s -> (no default value)", names[i]);
            }

            // Check threading model
            char threadModel[64];
            DWORD tmSize = sizeof(threadModel);
            result = RegQueryValueExA(hKey, "ThreadingModel", NULL, &type, (LPBYTE)threadModel, &tmSize);
            if (result == ERROR_SUCCESS) {
                LOG("    ThreadingModel: %s", threadModel);
            }

            RegCloseKey(hKey);
        } else {
            LOG("  %-30s -> NOT REGISTERED", names[i]);
        }
    }
}

// ============================================================================
// Main
// ============================================================================
int main(int argc, char* argv[]) {
    printf("========================================\n");
    printf("WLXSlideshow.dll COM Test Harness\n");
    printf("Reverse Engineering Analysis Tool\n");
    printf("========================================\n");

    const char* dllPath = argv[1];
    if (!dllPath) {
        // Default path - the original binary
        dllPath = "..\\..\\undecomp\\Photo Gallery\\WLXSlideshow.dll";
    }

    printf("\nLoading DLL: %s\n", dllPath);

    // Load the DLL
    HMODULE hDll = LoadLibraryA(dllPath);
    if (!hDll) {
        printf("FATAL: Failed to load DLL. Error=%lu\n", GetLastError());
        return 1;
    }
    printf("DLL loaded at: %p\n", (void*)hDll);

    Test_ModuleInfo(hDll);
    Test_ExportAddresses(hDll);

    // Get export function pointers
    pfnDllCanUnloadNow pCanUnload = (pfnDllCanUnloadNow)GetProcAddress(hDll, "DllCanUnloadNow");
    pfnDllGetClassObject pGetClass = (pfnDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject");
    pfnDllRegisterServer pRegister = (pfnDllRegisterServer)GetProcAddress(hDll, "DllRegisterServer");
    pfnDllUnregisterServer pUnregister = (pfnDllUnregisterServer)GetProcAddress(hDll, "DllUnregisterServer");

    if (!pCanUnload || !pGetClass) {
        printf("FATAL: Could not resolve DllCanUnloadNow or DllGetClassObject\n");
        FreeLibrary(hDll);
        return 1;
    }

    // Initialize COM
    printf("\n[SETUP] CoInitialize(NULL)\n");
    HRESULT hrInit = CoInitialize(NULL);
    LOG_HR(hrInit, "CoInitialize");

    // Run tests
    Test_DllCanUnloadNow(pCanUnload);

    // Try DllGetClassObject with all known CLSIDs
    struct { REFCLSID clsid; const char* name; } tests[] = {
        { CLSID_WLXSlideshow_Main,   "WLXSlideshow_Main" },
        { CLSID_SimpleSlideshowDisplay, "SimpleSlideshowDisplay" },
        { CLSID_SlideshowExtension,  "SlideshowExtension" },
        { CLSID_TimelineDisplay,     "TimelineDisplay" },
    };

    for (const auto& t : tests) {
        Test_DllGetClassObject(pGetClass, t.clsid, t.name);
    }

    // Try CoCreateInstance with all CLSIDs
    for (const auto& t : tests) {
        Test_CoCreateInstance(t.clsid, t.name);
    }

    // Try with a random/invalid CLSID to test error handling
    printf("\n[TEST] DllGetClassObject with invalid CLSID\n");
    {
        CLSID invalidClsid;
        CLSIDFromString(L"{00000000-0000-0000-0000-000000000000}", &invalidClsid);
        IClassFactory* pFactory = NULL;
        HRESULT hr = pGetClass(invalidClsid, IID_IClassFactory, (LPVOID*)&pFactory);
        LOG_HR(hr, "DllGetClassObject(invalid)");
        RecordResult("DllGetClassObject(invalid)", hr, hr == CLASS_E_CLASSNOTAVAILABLE || FAILED(hr));
    }

    // Check if DllRegisterServer / DllUnregisterServer resolve
    printf("\n[TEST] DLL Registration Exports\n");
    if (pRegister) {
        LOG("  DllRegisterServer: resolved at %p", (void*)pRegister);
    } else {
        LOG("  DllRegisterServer: NOT FOUND");
    }
    if (pUnregister) {
        LOG("  DllUnregisterServer: resolved at %p", (void*)pUnregister);
    } else {
        LOG("  DllUnregisterServer: NOT FOUND");
    }

    // Check registry
    Test_RegistryCheck();

    // Print summary
    printf("\n========================================\n");
    printf("RESULTS SUMMARY\n");
    printf("========================================\n");
    for (int i = 0; i < g_resultCount; i++) {
        printf("  %-45s %s (0x%08X)\n",
            g_results[i].name,
            g_results[i].passed ? "PASS" : "FAIL",
            (unsigned)g_results[i].hr);
    }

    // Cleanup
    CoUninitialize();
    FreeLibrary(hDll);

    printf("\nDone.\n");
    return 0;
}
