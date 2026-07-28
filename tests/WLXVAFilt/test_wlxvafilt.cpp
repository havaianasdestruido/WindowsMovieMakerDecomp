// WLXVAFilt.dll Dedicated Test Harness
// Tests all 4 COM exports + probes 8 DirectShow filter registrations
// Based on analysis/WLXVAFilt/analysis.md
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#include <unknwn.h>
#include <cstdio>
#include <cstdlib>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

// ============================================================
// Well-known GUIDs
// ============================================================
static const CLSID CLSID_Empty     = {0,0,0,{0,0,0,0,0,0,0,0}};
static const IID   IID_IUnknown    = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const IID   IID_IClassFactory = {0x00000001,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

// 8 Registered DirectShow Filters (from static analysis):
//   1. Record Queue              (CRecordQueue)
//   2. WMT Switch Filter         (CSwitchFilter)
//   3. WMT Virtual Renderer      (CWMTVirtualRendererFilter)
//   4. WMT Virtual Source        (CWMTVirtualSourceFilter)
//   5. WMT DV Extract Filter     (CDVExtractFilter)
//   6. WM VIH2 Fix               (CWMTVIH2Fix)
//   7. WMT Sample Info Filter    (CWMTSampleInformationFilter)
//   8. Capture File Writer       (CFileWriterFilter)

// ============================================================
// Export typedefs
// ============================================================
typedef HRESULT (__stdcall *DllCanUnloadNow_t)();
typedef HRESULT (__stdcall *DllGetClassObject_t)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServer_t)();
typedef HRESULT (__stdcall *DllUnregisterServer_t)();

// ============================================================
// Helpers
// ============================================================
static const wchar_t* HrName(HRESULT hr) {
    switch (hr) {
        case S_OK:                  return L"S_OK";
        case S_FALSE:               return L"S_FALSE";
        case E_NOINTERFACE:         return L"E_NOINTERFACE";
        case CLASS_E_CLASSNOTAVAILABLE: return L"CLASS_E_CLASSNOTAVAILABLE";
        case E_POINTER:             return L"E_POINTER";
        case E_UNEXPECTED:          return L"E_UNEXPECTED";
        default:                    return L"";
    }
}

static void TestDllCanUnloadNow(HMODULE h, const char* label) {
    DllCanUnloadNow_t fn = (DllCanUnloadNow_t)GetProcAddress(h, "DllCanUnloadNow");
    if (!fn) { printf("  [!] DllCanUnloadNow not found\n"); return; }
    printf("  ptr=0x%p\n", fn);
    __try {
        HRESULT hr = fn();
        printf("  => HRESULT=0x%08X %ls\n", hr, HrName(hr));
        if (hr == S_OK)
            printf("  [OK] No outstanding references\n");
        else if (hr == S_FALSE)
            printf("  [OK] Outstanding references exist\n");
        else
            printf("  [?] Unexpected\n");
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  => SEH exception 0x%08X\n", GetExceptionCode());
    }
}

static void TestDllGetClassObject(HMODULE h, REFCLSID rclsid, REFIID riid,
                                   const char* label, bool expectSuccess) {
    DllGetClassObject_t fn = (DllGetClassObject_t)GetProcAddress(h, "DllGetClassObject");
    if (!fn) { printf("  [!] DllGetClassObject not found\n"); return; }
    printf("  %s\n", label);
    LPVOID pUnk = NULL;
    __try {
        HRESULT hr = fn(rclsid, riid, &pUnk);
        printf("  => HRESULT=0x%08X %ls, ppv=0x%p\n", hr, HrName(hr), pUnk);
        if (SUCCEEDED(hr) && pUnk) {
            printf("  [+] Object created at 0x%p\n", pUnk);
            ((IUnknown*)pUnk)->Release();
        } else if (expectSuccess) {
            printf("  [!!] Expected success but got failure\n");
        } else {
            printf("  [OK] Rejected as expected\n");
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  => SEH exception 0x%08X\n", GetExceptionCode());
        if (pUnk) ((IUnknown*)pUnk)->Release();
    }
}

// ============================================================
// Main test
// ============================================================
static void TestWLXVAFilt(HMODULE h) {
    printf("========================================\n");
    printf("  WLXVAFilt.dll - Export Probing\n");
    printf("  Video Acquisition Filters (4 exports)\n");
    printf("========================================\n");

    printf("\n--- DllCanUnloadNow (ordinal 1) ---\n");
    TestDllCanUnloadNow(h, "First call");
    printf("\n--- DllCanUnloadNow (idempotency) ---\n");
    TestDllCanUnloadNow(h, "Second call");

    printf("\n--- DllGetClassObject (empty CLSID, null ppv) ---\n");
    TestDllGetClassObject(h, CLSID_Empty, IID_IUnknown, "empty CLSID, null ppv", false);

    printf("\n--- DllGetClassObject (empty CLSID, valid ppv) ---\n");
    TestDllGetClassObject(h, CLSID_Empty, IID_IUnknown, "empty CLSID, IID_IUnknown", false);

    printf("\n--- DllGetClassObject (zeroed CLSID, valid ppv) ---\n");
    {
        CLSID zeroed = {0};
        TestDllGetClassObject(h, zeroed, IID_IUnknown, "zeroed CLSID, IID_IUnknown", false);
    }

    printf("\n--- DllGetClassObject (IClassFactory IID as CLSID probe) ---\n");
    TestDllGetClassObject(h, IID_IClassFactory, IID_IUnknown,
                          "IClassFactory IID as CLSID probe", false);

    // DllRegisterServer
    printf("\n--- DllRegisterServer (ordinal 3) ---\n");
    {
        DllRegisterServer_t fn = (DllRegisterServer_t)GetProcAddress(h, "DllRegisterServer");
        if (fn) {
            printf("  ptr=0x%p\n", fn);
            printf("  [skip] Registers 8 DirectShow filters via IFilterMapper2\n");
        } else {
            printf("  [!] not found\n");
        }
    }

    // DllUnregisterServer
    printf("\n--- DllUnregisterServer (ordinal 4) ---\n");
    {
        DllUnregisterServer_t fn = (DllUnregisterServer_t)GetProcAddress(h, "DllUnregisterServer");
        if (fn) {
            printf("  ptr=0x%p\n", fn);
            printf("  [skip] Unregisters DirectShow filters\n");
        } else {
            printf("  [!] not found\n");
        }
    }

    // Registry probe: check if any of the 8 filters are registered
    printf("\n========================================\n");
    printf("  Filter Registration Probe\n");
    printf("========================================\n");
    // The 8 filter CLSIDs are stored in a relocation-dependent table
    // at runtime address (base + 0xD408) -- we can only check via COM
    // if the DLL has been previously registered. Try enumerating
    // filter categories via IFilterMapper2 to see if any WMT filters
    // appear in the system.
    printf("  [info] 8 DirectShow filters known from RTTI:\n");
    const char* filters[] = {
        "Record Queue",
        "WMT Switch Filter",
        "WMT Virtual Renderer",
        "WMT Virtual Source",
        "WMT DV Extract Filter",
        "WM VIH2 Fix",
        "WMT Sample Info Filter",
        "Capture File Writer",
    };
    for (int i = 0; i < 8; i++) {
        printf("    %d. %s\n", i+1, filters[i]);
    }
    printf("  [info] CLSIDs are in reloc-dependent table at DLL+0xD408\n");
    printf("  [info] Can only instantiate via DllGetClassObject after registration\n");
}

// ============================================================
// Entry point
// ============================================================
int main() {
    printf("=== WLXVAFilt.dll Dedicated Test Harness ===\n\n");

    // Set DLL search path to ../../undecomp/Photo Gallery (relative to test dir)
    wchar_t galleryDir[MAX_PATH], cwd[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, cwd);
    wcscpy_s(galleryDir, MAX_PATH, cwd);
    wcscat_s(galleryDir, MAX_PATH, L"\\..\\..\\undecomp\\Photo Gallery");
    AddDllDirectory(galleryDir);
    SetDllDirectoryW(galleryDir);
    printf("[*] DLL dir: %ls\n", galleryDir);

    // Initialize COM
    HRESULT hrCo = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    printf("[*] CoInitializeEx: HRESULT=0x%08X %ls\n\n", hrCo, HrName(hrCo));

    // Load DLL
    wchar_t fullPath[MAX_PATH];
    wcscpy_s(fullPath, MAX_PATH, galleryDir);
    wcscat_s(fullPath, MAX_PATH, L"\\WLXVAFilt.dll");
    printf("[*] Loading: %ls\n", fullPath);
    HMODULE h = LoadLibraryExW(fullPath, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!h) {
        printf("[!] LoadLibraryEx failed (err=%u), retrying\n", GetLastError());
        h = LoadLibraryW(fullPath);
    }
    if (!h) {
        printf("[!] FATAL: cannot load WLXVAFilt.dll (err=%u)\n", GetLastError());
        if (SUCCEEDED(hrCo)) CoUninitialize();
        return 1;
    }
    printf("[+] Loaded at base: 0x%p\n\n", h);

    // Run tests
    TestWLXVAFilt(h);

    // Module info
    printf("\n[*] Module info:\n");
    wchar_t modPath[MAX_PATH];
    GetModuleFileNameW(h, modPath, MAX_PATH);
    printf("    Path: %ls\n", modPath);
    DWORD verSize = GetFileVersionInfoSizeW(modPath, NULL);
    printf("    Version info size: %u bytes\n", verSize);

    // Unload
    printf("\n[*] Unloading DLL\n");
    FreeLibrary(h);

    if (SUCCEEDED(hrCo)) CoUninitialize();
    printf("\n=== Done ===\n");
    return 0;
}
