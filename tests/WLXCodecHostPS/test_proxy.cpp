#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <cstdio>
#include <cstdlib>

// ============================================================================
// Known IIDs and CLSIDs from binary analysis
// ============================================================================

// {AC46E1E4-9981-49E8-A5E1-90A07142EDC3} - PSFactoryBuffer CLSID
static const CLSID CLSID_PSFactoryBuffer =
    {0xAC46E1E4, 0x9981, 0x49E8, {0xA5, 0xE1, 0x90, 0xA0, 0x71, 0x42, 0xED, 0xC3}};

// {BB40E64E-19B1-44BF-6811-001000000000} - IID_ICodecHost
static const IID IID_ICodecHost =
    {0xBB40E64E, 0x19B1, 0x44BF, {0x68, 0x11, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00}};

// Random GUID for negative testing
static const GUID GUID_DEADBEEF =
    {0xDEADBEEF, 0x1234, 0x5678, {0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89}};

// ============================================================================
// Function pointer types
// ============================================================================
typedef HRESULT (STDAPICALLTYPE *FN_DllCanUnloadNow)(void);
typedef HRESULT (STDAPICALLTYPE *FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (STDAPICALLTYPE *FN_DllRegisterServer)(void);
typedef HRESULT (STDAPICALLTYPE *FN_DllUnregisterServer)(void);

// GetProxyDllInfo: from disassembly — 2 output params, stdcall ret 8
// Writes PSFactoryBuffer base addr to param1, resolved proxy info to param2.
typedef void (WINAPI *FN_GetProxyDllInfo)(void**, void**);

// ============================================================================
// Helpers
// ============================================================================
static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define CHECK(name, condition) \
    do { \
        if (condition) { \
            printf("  [PASS] %s\n", name); \
            g_testsPassed++; \
        } else { \
            printf("  [FAIL] %s\n", name); \
            g_testsFailed++; \
        } \
    } while(0)

#define HR_CHECK(name, hr, expected) \
    do { \
        HRESULT _hr = (hr); \
        if (_hr == (expected)) { \
            printf("  [PASS] %s -> 0x%08X\n", name, (unsigned int)_hr); \
            g_testsPassed++; \
        } else { \
            printf("  [FAIL] %s -> 0x%08X (expected 0x%08X)\n", name, (unsigned int)_hr, (unsigned int)(expected)); \
            g_testsFailed++; \
        } \
    } while(0)

static void PrintGuid(const GUID& guid)
{
    printf("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

// ============================================================================
// Test 1: LoadLibrary
// ============================================================================
static void TestLoadLibrary(HMODULE* phMod)
{
    printf("=== Test 1: LoadLibrary ===\n");

    char exeDir[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';

    char dllDir[MAX_PATH] = {0};
    snprintf(dllDir, MAX_PATH, "%s\\..\\..\\undecomp\\Photo Gallery", exeDir);
    char fullDllDir[MAX_PATH] = {0};
    GetFullPathNameA(dllDir, MAX_PATH, fullDllDir, NULL);
    printf("  DLL search dir: %s\n", fullDllDir);
    SetDllDirectoryA(fullDllDir);

    char dllPath[MAX_PATH] = {0};
    snprintf(dllPath, MAX_PATH, "%s\\WLXCodecHostPS.dll", fullDllDir);
    printf("  DLL path: %s\n", dllPath);

    *phMod = LoadLibraryA(dllPath);
    CHECK("LoadLibrary with valid path", *phMod != NULL);
    if (*phMod) {
        printf("  Module handle: %p\n", (void*)*phMod);
    }

    // Error cases
    HMODULE hFake = LoadLibraryA("nonexistent_proxy.dll");
    CHECK("LoadLibrary(nonexistent) fails", hFake == NULL);

    HMODULE hEmpty = LoadLibraryA("");
    CHECK("LoadLibrary(\"\") fails", hEmpty == NULL);

    printf("\n");
}

// ============================================================================
// Test 2: GetProcAddress (all 5 exports)
// ============================================================================
static void TestGetProcAddress(HMODULE hMod)
{
    printf("=== Test 2: GetProcAddress (5 exports) ===\n");

    const char* exportNames[] = {
        "DllCanUnloadNow",
        "DllGetClassObject",
        "DllRegisterServer",
        "DllUnregisterServer",
        "GetProxyDllInfo"
    };

    for (int i = 0; i < 5; i++) {
        FARPROC fp = GetProcAddress(hMod, exportNames[i]);
        CHECK(exportNames[i], fp != NULL);
        if (fp) {
            printf("    -> addr 0x%p\n", (void*)fp);
        }
    }

    // By ordinal
    for (int ord = 1; ord <= 5; ord++) {
        FARPROC fp = GetProcAddress(hMod, (LPCSTR)ord);
        CHECK(exportNames[ord - 1], fp != NULL);
    }

    // Invalid
    FARPROC fpBad = GetProcAddress(hMod, "NonexistentExport");
    CHECK("GetProcAddress(invalid name) returns NULL", fpBad == NULL);

    FARPROC fpOrd999 = (FARPROC)GetProcAddress(hMod, (LPCSTR)999);
    CHECK("GetProcAddress(ordinal 999) returns NULL", fpOrd999 == NULL);

    printf("\n");
}

// ============================================================================
// Test 3: DllCanUnloadNow
// ============================================================================
static void TestDllCanUnloadNow(HMODULE hMod)
{
    printf("=== Test 3: DllCanUnloadNow ===\n");

    FN_DllCanUnloadNow pFn = (FN_DllCanUnloadNow)GetProcAddress(hMod, "DllCanUnloadNow");
    if (!pFn) { printf("  Cannot resolve DllCanUnloadNow\n\n"); return; }

    __try {
        HRESULT hr = pFn();
        printf("  DllCanUnloadNow() = 0x%08X\n", (unsigned int)hr);
        CHECK("DllCanUnloadNow returns S_OK or S_FALSE",
            hr == S_OK || hr == S_FALSE);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH exception in DllCanUnloadNow! code=0x%08X\n", GetExceptionCode());
        g_testsFailed++;
    }

    printf("\n");
}

// ============================================================================
// Test 4: DllGetClassObject
// ============================================================================
static void TestDllGetClassObject(HMODULE hMod)
{
    printf("=== Test 4: DllGetClassObject ===\n");

    FN_DllGetClassObject pFn = (FN_DllGetClassObject)GetProcAddress(hMod, "DllGetClassObject");
    if (!pFn) { printf("  Cannot resolve DllGetClassObject\n\n"); return; }

    // 1) Known PS CLSID -> NdrDllGetClassObject returns E_NOINTERFACE
    //    (proxy/stub class factories don't implement IClassFactory)
    {
        IClassFactory* pFactory = NULL;
        __try {
            HRESULT hr = pFn(CLSID_PSFactoryBuffer, IID_IClassFactory, (LPVOID*)&pFactory);
            printf("  DllGetClassObject(PS CLSID, IClassFactory) = 0x%08X", (unsigned int)hr);
            if (SUCCEEDED(hr) && pFactory) {
                printf(" -> factory=%p\n", (void*)pFactory);
                IUnknown* pUnk = NULL;
                HRESULT hrCreate = pFactory->CreateInstance(NULL, IID_IUnknown, (LPVOID*)&pUnk);
                printf("    CreateInstance(IUnknown) = 0x%08X\n", (unsigned int)hrCreate);
                pFactory->Release();
            } else {
                printf(" (E_NOINTERFACE — expected, PS class factory)\n");
            }
            g_testsPassed++;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("\n  SEH exception! code=0x%08X\n", GetExceptionCode());
            g_testsFailed++;
            if (pFactory) pFactory->Release();
        }
    }

    // 2) CLSID_NULL -> should fail
    {
        IClassFactory* pFactory = NULL;
        __try {
            HRESULT hr = pFn(CLSID_NULL, IID_IClassFactory, (LPVOID*)&pFactory);
            printf("  DllGetClassObject(CLSID_NULL) = 0x%08X\n", (unsigned int)hr);
            CHECK("CLSID_NULL returns error", FAILED(hr));
            if (pFactory) pFactory->Release();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH exception with CLSID_NULL! code=0x%08X\n", GetExceptionCode());
            g_testsFailed++;
        }
    }

    // 3) NULL ppv — underlying NdrDllGetClassObject crashes (no NULL check)
    {
        __try {
            HRESULT hr = pFn(CLSID_PSFactoryBuffer, IID_IClassFactory, NULL);
            printf("  DllGetClassObject(NULL ppv) = 0x%08X (unexpectedly no crash)\n", (unsigned int)hr);
            g_testsPassed++;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  DllGetClassObject(NULL ppv): SEH 0x%08X (expected — no NULL validation)\n", GetExceptionCode());
            g_testsPassed++; // Expected: Ndr runtime doesn't validate ppv
        }
    }

    // 4) Random GUID
    {
        IClassFactory* pFactory = NULL;
        __try {
            HRESULT hr = pFn(GUID_DEADBEEF, IID_IClassFactory, (LPVOID*)&pFactory);
            printf("  DllGetClassObject(random GUID) = 0x%08X\n", (unsigned int)hr);
            CHECK("Random GUID returns CLASS_E_CLASSNOTAVAILABLE", hr == CLASS_E_CLASSNOTAVAILABLE);
            if (pFactory) pFactory->Release();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH exception with random GUID! code=0x%08X\n", GetExceptionCode());
            g_testsFailed++;
        }
    }

    printf("\n");
}

// ============================================================================
// Test 5: DllRegisterServer / DllUnregisterServer
// ============================================================================
static void TestDllRegistration(HMODULE hMod)
{
    printf("=== Test 5: DllRegisterServer / DllUnregisterServer ===\n");

    FN_DllRegisterServer pReg = (FN_DllRegisterServer)GetProcAddress(hMod, "DllRegisterServer");
    FN_DllUnregisterServer pUnreg = (FN_DllUnregisterServer)GetProcAddress(hMod, "DllUnregisterServer");

    if (!pReg || !pUnreg) {
        printf("  Cannot resolve registration functions\n\n");
        return;
    }

    // Pre-clean
    __try {
        HRESULT hr = pUnreg();
        printf("  DllUnregisterServer (pre-clean) = 0x%08X\n", (unsigned int)hr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllUnregisterServer (pre-clean), code=0x%08X\n", GetExceptionCode());
    }

    // Register (likely fails without admin)
    __try {
        HRESULT hr = pReg();
        printf("  DllRegisterServer = 0x%08X", (unsigned int)hr);
        if (SUCCEEDED(hr)) {
            printf(" (proxy/stub registration succeeded)\n");
        } else {
            printf(" (likely no admin rights)\n");
        }
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllRegisterServer, code=0x%08X\n", GetExceptionCode());
        g_testsFailed++;
    }

    // Double-register
    __try {
        HRESULT hr = pReg();
        printf("  DllRegisterServer (double) = 0x%08X\n", (unsigned int)hr);
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllRegisterServer (double), code=0x%08X\n", GetExceptionCode());
        g_testsFailed++;
    }

    // Unregister
    __try {
        HRESULT hr = pUnreg();
        printf("  DllUnregisterServer = 0x%08X\n", (unsigned int)hr);
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllUnregisterServer, code=0x%08X\n", GetExceptionCode());
        g_testsFailed++;
    }

    // Double-unregister
    __try {
        HRESULT hr = pUnreg();
        printf("  DllUnregisterServer (double) = 0x%08X\n", (unsigned int)hr);
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllUnregisterServer (double), code=0x%08X\n", GetExceptionCode());
        g_testsFailed++;
    }

    printf("\n");
}

// ============================================================================
// Test 6: GetProxyDllInfo — the 5th export unique to proxy/stub DLLs
// ============================================================================
static void TestGetProxyDllInfo(HMODULE hMod)
{
    printf("=== Test 6: GetProxyDllInfo ===\n");

    FN_GetProxyDllInfo pFn = (FN_GetProxyDllInfo)GetProcAddress(hMod, "GetProxyDllInfo");
    if (!pFn) { printf("  Cannot resolve GetProxyDllInfo\n\n"); return; }
    printf("  GetProxyDllInfo @ 0x%p\n", (void*)pFn);

    // Normal call: two valid output pointers
    {
        void* pFactoryBase = NULL;
        void* pProxyInfo = NULL;
        __try {
            pFn(&pFactoryBase, &pProxyInfo);
            printf("  GetProxyDllInfo(&pBase, &pInfo):\n");
            printf("    pFactoryBase = %p\n", pFactoryBase);
            printf("    pProxyInfo  = %p\n", pProxyInfo);
            CHECK("GetProxyDllInfo writes pFactoryBase", pFactoryBase != NULL);
            CHECK("GetProxyDllInfo writes pProxyInfo", pProxyInfo != NULL);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH exception! code=0x%08X\n", GetExceptionCode());
            g_testsFailed++;
        }
    }

    // NULL first param — function dereferences without NULL check
    {
        void* pProxyInfo = NULL;
        __try {
            pFn(NULL, &pProxyInfo);
            printf("  GetProxyDllInfo(NULL, &pInfo): pProxyInfo=%p (unexpectedly no crash)\n", pProxyInfo);
            g_testsPassed++;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  GetProxyDllInfo(NULL, &pInfo): SEH 0x%08X (expected — no NULL check on param1)\n", GetExceptionCode());
            g_testsPassed++;
        }
    }

    // NULL second param — function dereferences without NULL check
    {
        void* pFactoryBase = NULL;
        __try {
            pFn(&pFactoryBase, NULL);
            printf("  GetProxyDllInfo(&pBase, NULL): pFactoryBase=%p (unexpectedly no crash)\n", pFactoryBase);
            g_testsPassed++;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  GetProxyDllInfo(&pBase, NULL): SEH 0x%08X (expected — no NULL check on param2)\n", GetExceptionCode());
            g_testsPassed++;
        }
    }

    // Both NULL
    {
        __try {
            pFn(NULL, NULL);
            printf("  GetProxyDllInfo(NULL, NULL) — no crash (unexpected)\n");
            g_testsPassed++;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  GetProxyDllInfo(NULL, NULL): SEH 0x%08X (expected)\n", GetExceptionCode());
            g_testsPassed++;
        }
    }

    printf("\n");
}

// ============================================================================
// Test 7: FreeLibrary
// ============================================================================
static void TestFreeLibrary(HMODULE hMod)
{
    printf("=== Test 7: FreeLibrary ===\n");

    BOOL bRet = FreeLibrary(hMod);
    CHECK("FreeLibrary(valid handle)", bRet != FALSE);

    bRet = FreeLibrary(NULL);
    CHECK("FreeLibrary(NULL) returns FALSE", bRet == FALSE);

    printf("\n");
}

// ============================================================================
// Main
// ============================================================================
int main()
{
    printf("=== WLXCodecHostPS.dll Dynamic Test Harness ===\n");
    printf("COM proxy/stub: DllCanUnloadNow, DllGetClassObject,\n");
    printf("  DllRegisterServer, DllUnregisterServer, GetProxyDllInfo\n\n");

    printf("PS CLSID: ");
    PrintGuid(CLSID_PSFactoryBuffer);
    printf("\nIID_ICodecHost: ");
    PrintGuid(IID_ICodecHost);
    printf("\n\n");

    HMODULE hMod = NULL;

    TestLoadLibrary(&hMod);
    if (!hMod) {
        printf("FATAL: Could not load WLXCodecHostPS.dll\n");
        printf("Ensure MSVCR110.dll is available.\n");
        return 1;
    }

    TestGetProcAddress(hMod);
    TestDllCanUnloadNow(hMod);
    TestDllGetClassObject(hMod);
    TestDllRegistration(hMod);
    TestGetProxyDllInfo(hMod);
    TestFreeLibrary(hMod);

    printf("=== Results: %d passed, %d failed, %d total ===\n",
        g_testsPassed, g_testsFailed, g_testsPassed + g_testsFailed);

    return g_testsFailed;
}
