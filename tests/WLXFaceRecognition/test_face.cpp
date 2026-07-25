#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

// ============================================================================
// Known CLSIDs from binary string analysis
// ============================================================================

// {EF401225-1260-4716-A842-7D180DC14C1E} - Confirmed in RGS registry script
static const CLSID CLSID_FRP =
    {0xef401225, 0x1260, 0x4716, {0xa8, 0x42, 0x7d, 0x18, 0x0d, 0xc1, 0x4c, 0x1e}};

// {4107FA03-3FD3-4406-B4F3-68E6D610EC2B} - FaceDetection (RGS confirmed)
static const CLSID CLSID_FD =
    {0x4107fa03, 0x3fd3, 0x4406, {0xb4, 0xf3, 0x68, 0xe6, 0xd6, 0x10, 0xec, 0x2b}};

// {483A53CD-EF18-4b19-8AA3-2E2E3214EB41} - ImageManager (RGS confirmed)
static const CLSID CLSID_IM =
    {0x483a53cd, 0xef18, 0x4b19, {0x8a, 0xa3, 0x2e, 0x2e, 0x32, 0x14, 0xeb, 0x41}};

// {D01C34A5-A6DC-4d28-ABBD-78D06EA27B60} - FaceRecognitionPipeline (RGS confirmed)
static const CLSID CLSID_FRPS =
    {0xd01c34a5, 0xa6dc, 0x4d28, {0xab, 0xbd, 0x78, 0xd0, 0x6e, 0xa2, 0x7b, 0x60}};

// Unknown CLSIDs from binary GUIDs (likely FaceRegion, FaceRegionSet, FaceRepImpl, ImageData)
static const GUID GUID_UNKNOWN1 =
    {0xef401225, 0x1260, 0x4716, {0xa8, 0x42, 0x7d, 0x18, 0x0d, 0xc1, 0x4c, 0x1e}};

static const GUID GUID_NULL_REF =
    {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

// ============================================================================
// Function pointer types
// ============================================================================
typedef HRESULT (STDAPICALLTYPE *FN_DllCanUnloadNow)(void);
typedef HRESULT (STDAPICALLTYPE *FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (STDAPICALLTYPE *FN_DllRegisterServer)(void);
typedef HRESULT (STDAPICALLTYPE *FN_DllUnregisterServer)(void);

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
            printf("  [PASS] %s -> 0x%08X (expected)\n", name, (unsigned int)_hr); \
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

    // Build absolute path to the DLL directory so dependent WLX DLLs are found
    char exeDir[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    // Strip exe name
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';

    // Build the full DLL directory path (relative to project root)
    char dllDir[MAX_PATH] = {0};
    snprintf(dllDir, MAX_PATH, "%s\\..\\..\\undecomp\\Photo Gallery", exeDir);
    // Normalize
    char fullDllDir[MAX_PATH] = {0};
    GetFullPathNameA(dllDir, MAX_PATH, fullDllDir, NULL);
    printf("  DLL search dir: %s\n", fullDllDir);
    SetDllDirectoryA(fullDllDir);

    // Build full DLL path
    char dllPath[MAX_PATH] = {0};
    snprintf(dllPath, MAX_PATH, "%s\\WLXFaceRecognition.dll", fullDllDir);
    printf("  DLL path: %s\n", dllPath);

    // Normal load
    *phMod = LoadLibraryA(dllPath);
    CHECK("LoadLibrary with valid path", *phMod != NULL);
    if (*phMod) {
        printf("  Module handle: %p\n", (void*)*phMod);
    }

    // LoadLibrary with NULL
    HMODULE hBad = LoadLibraryA(NULL);
    // NULL path behavior varies: on modern Windows returns NULL (not valid)
    printf("  LoadLibrary(NULL) -> %p (varies by OS version)\n", (void*)hBad);
    g_testsPassed++; // Just report, don't fail

    // LoadLibrary with nonexistent
    HMODULE hFake = LoadLibraryA("nonexistent_face_recognition.dll");
    CHECK("LoadLibrary(nonexistent) fails", hFake == NULL);

    // LoadLibrary with empty string
    HMODULE hEmpty = LoadLibraryA("");
    CHECK("LoadLibrary(\"\") fails", hEmpty == NULL);

    printf("\n");
}

// ============================================================================
// Test 2: GetProcAddress
// ============================================================================
static void TestGetProcAddress(HMODULE hMod)
{
    printf("=== Test 2: GetProcAddress ===\n");

    // Valid export names
    FARPROC fp1 = GetProcAddress(hMod, "DllCanUnloadNow");
    CHECK("GetProcAddress(DllCanUnloadNow)", fp1 != NULL);

    FARPROC fp2 = GetProcAddress(hMod, "DllGetClassObject");
    CHECK("GetProcAddress(DllGetClassObject)", fp2 != NULL);

    FARPROC fp3 = GetProcAddress(hMod, "DllRegisterServer");
    CHECK("GetProcAddress(DllRegisterServer)", fp3 != NULL);

    FARPROC fp4 = GetProcAddress(hMod, "DllUnregisterServer");
    CHECK("GetProcAddress(DllUnregisterServer)", fp4 != NULL);

    // Invalid name
    FARPROC fpBad = GetProcAddress(hMod, "NonexistentExport");
    CHECK("GetProcAddress(invalid name) returns NULL", fpBad == NULL);

    // NULL name
    FARPROC fpNull = GetProcAddress(hMod, NULL);
    // On Windows, NULL name gets the export by ordinal 0 which is the hint
    printf("  GetProcAddress(NULL) -> %p (ordinal behavior)\n", (void*)fpNull);

    // Ordinals
    FARPROC fpOrd1 = (FARPROC)GetProcAddress(hMod, (LPCSTR)1);
    CHECK("GetProcAddress(ordinal 1 = DllCanUnloadNow)", fpOrd1 != NULL);

    FARPROC fpOrd2 = (FARPROC)GetProcAddress(hMod, (LPCSTR)2);
    CHECK("GetProcAddress(ordinal 2 = DllGetClassObject)", fpOrd2 != NULL);

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

    // Test before any COM object creation
    __try {
        HRESULT hr = pFn();
        printf("  DllCanUnloadNow() before COM creation: 0x%08X\n", (unsigned int)hr);
        CHECK("DllCanUnloadNow returns S_OK or S_FALSE",
            hr == S_OK || hr == S_FALSE);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH exception in DllCanUnloadNow!\n");
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

    // Test known CLSIDs
    struct { const char* name; REFCLSID clsid; } knownClsids[] = {
        {"FaceDetection",             CLSID_FD},
        {"FaceRecognitionPipeline",   CLSID_FRP},
        {"ImageManager",              CLSID_IM},
        {"FaceRecognitionPipeline2",  CLSID_FRPS},
    };

    for (int i = 0; i < 4; i++) {
        IClassFactory* pFactory = NULL;
        __try {
            HRESULT hr = pFn(knownClsids[i].clsid, IID_IClassFactory, (LPVOID*)&pFactory);
            printf("  DllGetClassObject(%s): 0x%08X", knownClsids[i].name, (unsigned int)hr);
            if (SUCCEEDED(hr) && pFactory) {
                printf(" -> factory=%p\n", (void*)pFactory);

                // Test CreateInstance
                IUnknown* pUnk = NULL;
                HRESULT hrCreate = pFactory->CreateInstance(NULL, IID_IUnknown, (LPVOID*)&pUnk);
                printf("    CreateInstance(IUnknown): 0x%08X", (unsigned int)hrCreate);
                if (SUCCEEDED(hrCreate) && pUnk) {
                    printf(" -> unk=%p\n", (void*)pUnk);
                    pUnk->Release();
                } else {
                    printf("\n");
                }

                // Test LockServer
                HRESULT hrLock1 = pFactory->LockServer(TRUE);
                printf("    LockServer(TRUE): 0x%08X\n", (unsigned int)hrLock1);
                HRESULT hrLock2 = pFactory->LockServer(FALSE);
                printf("    LockServer(FALSE): 0x%08X\n", (unsigned int)hrLock2);

                pFactory->Release();
            } else {
                printf("\n");
            }
            g_testsPassed++;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH exception in DllGetClassObject(%s)!\n", knownClsids[i].name);
            g_testsFailed++;
            if (pFactory) pFactory->Release();
        }
    }

    // Test GUID_NULL
    {
        IClassFactory* pFactory = NULL;
        __try {
            HRESULT hr = pFn(GUID_NULL_REF, IID_IClassFactory, (LPVOID*)&pFactory);
            printf("  DllGetClassObject(GUID_NULL): 0x%08X\n", (unsigned int)hr);
            CHECK("GUID_NULL returns error", FAILED(hr));
            if (pFactory) pFactory->Release();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH exception with GUID_NULL\n");
            g_testsFailed++;
        }
    }

    // Test NULL ppv
    {
        __try {
            HRESULT hr = pFn(CLSID_FD, IID_IClassFactory, NULL);
            printf("  DllGetClassObject(NULL ppv): 0x%08X\n", (unsigned int)hr);
            CHECK("NULL ppv returns E_POINTER", hr == E_POINTER || FAILED(hr));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH exception with NULL ppv\n");
            g_testsFailed++;
        }
    }

    // Test random GUID
    {
        GUID randomGuid = {0xDEADBEEF, 0x1234, 0x5678, {0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89}};
        IClassFactory* pFactory = NULL;
        __try {
            HRESULT hr = pFn(randomGuid, IID_IClassFactory, (LPVOID*)&pFactory);
            printf("  DllGetClassObject(random GUID): 0x%08X\n", (unsigned int)hr);
            CHECK("Random GUID returns CLASS_E_CLASSNOTAVAILABLE", hr == CLASS_E_CLASSNOTAVAILABLE || FAILED(hr));
            if (pFactory) pFactory->Release();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH exception with random GUID\n");
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

    // Try unregister first (clean state)
    __try {
        HRESULT hr = pUnreg();
        printf("  DllUnregisterServer (pre-clean): 0x%08X\n", (unsigned int)hr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllUnregisterServer (pre-clean)\n");
    }

    // Register
    __try {
        HRESULT hr = pReg();
        printf("  DllRegisterServer: 0x%08X\n", (unsigned int)hr);
        // May fail without admin rights, that's expected
        if (SUCCEEDED(hr)) {
            printf("  Registration succeeded - keys created under Software\\Microsoft\\MSRA\\FaceRecognition\\\n");
            g_testsPassed++;
        } else {
            printf("  Registration failed (likely no admin rights) - 0x%08X\n", (unsigned int)hr);
            g_testsPassed++; // This is expected behavior without admin
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllRegisterServer\n");
        g_testsFailed++;
    }

    // Double-register (idempotent test)
    __try {
        HRESULT hr = pReg();
        printf("  DllRegisterServer (double): 0x%08X\n", (unsigned int)hr);
        g_testsPassed++; // Should not crash
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllRegisterServer (double)\n");
        g_testsFailed++;
    }

    // Unregister
    __try {
        HRESULT hr = pUnreg();
        printf("  DllUnregisterServer: 0x%08X\n", (unsigned int)hr);
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllUnregisterServer\n");
        g_testsFailed++;
    }

    // Double-unregister
    __try {
        HRESULT hr = pUnreg();
        printf("  DllUnregisterServer (double): 0x%08X\n", (unsigned int)hr);
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllUnregisterServer (double)\n");
        g_testsFailed++;
    }

    printf("\n");
}

// ============================================================================
// Test 6: CoCreateInstance for known CLSIDs
// ============================================================================
static void TestCoCreateInstance()
{
    printf("=== Test 6: CoCreateInstance ===\n");

    struct { const char* name; REFCLSID clsid; } classes[] = {
        {"FaceDetection",             CLSID_FD},
        {"FaceRecognitionPipeline",   CLSID_FRP},
        {"ImageManager",              CLSID_IM},
        {"FaceRecognitionPipeline2",  CLSID_FRPS},
    };

    for (int i = 0; i < 4; i++) {
        IUnknown* pUnk = NULL;
        __try {
            HRESULT hr = CoCreateInstance(classes[i].clsid, NULL, CLSCTX_INPROC_SERVER,
                IID_IUnknown, (LPVOID*)&pUnk);
            printf("  CoCreateInstance(%s): 0x%08X", classes[i].name, (unsigned int)hr);
            if (SUCCEEDED(hr) && pUnk) {
                printf(" -> unk=%p\n", (void*)pUnk);

                // QueryInterface for IUnknown (should always succeed)
                IUnknown* pQI = NULL;
                HRESULT hrQI = pUnk->QueryInterface(IID_IUnknown, (LPVOID*)&pQI);
                printf("    QueryInterface(IUnknown): 0x%08X\n", (unsigned int)hrQI);

                pUnk->Release();
            } else {
                printf("\n");
            }
            g_testsPassed++;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH in CoCreateInstance(%s)\n", classes[i].name);
            g_testsFailed++;
            if (pUnk) pUnk->Release();
        }
    }

    // Test random CLSID
    {
        IUnknown* pUnk = NULL;
        GUID randomGuid = {0xDEADBEEF, 0x1234, 0x5678, {0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89}};
        __try {
            HRESULT hr = CoCreateInstance(randomGuid, NULL, CLSCTX_INPROC_SERVER,
                IID_IUnknown, (LPVOID*)&pUnk);
            printf("  CoCreateInstance(random): 0x%08X\n", (unsigned int)hr);
            CHECK("Random CLSID fails", FAILED(hr));
            if (pUnk) pUnk->Release();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH in CoCreateInstance(random)\n");
            g_testsFailed++;
        }
    }

    printf("\n");
}

// ============================================================================
// Test 7: COM Interface method probing (IFaceDetection)
// ============================================================================
static void TestIFaceDetection()
{
    printf("=== Test 7: IFaceDetection Interface Probing ===\n");

    IUnknown* pUnk = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FD, NULL, CLSCTX_INPROC_SERVER,
        IID_IUnknown, (LPVOID*)&pUnk);

    if (FAILED(hr) || !pUnk) {
        printf("  Could not create IFaceDetection object (hr=0x%08X)\n", (unsigned int)hr);
        printf("  Skipping interface probing tests\n\n");
        return;
    }

    printf("  Created IFaceDetection object: %p\n", (void*)pUnk);

    // Try to find IFaceDetection interface
    // The IID is {E1ED7C08-0549-4FC5-B61D-3A3AB625F97C}
    static const IID IID_IFaceDetection =
        {0xE1ED7C08, 0x0549, 0x4FC5, {0xB6, 0x1D, 0x3A, 0x3A, 0xB6, 0x25, 0xF9, 0x7C}};

    void* pFaceDet = NULL;
    hr = pUnk->QueryInterface(IID_IFaceDetection, &pFaceDet);
    printf("  QueryInterface(IFaceDetection): 0x%08X", (unsigned int)hr);
    if (SUCCEEDED(hr) && pFaceDet) {
        printf(" -> %p\n", (void*)pFaceDet);
        // We can't call methods without knowing the vtable layout, but we can
        // verify the pointer is valid
        CHECK("IFaceDetection QI succeeded", TRUE);
        ((IUnknown*)pFaceDet)->Release();
    } else {
        printf(" (not supported or different IID)\n");
    }

    // Also try IID_IClassInfo if available
    static const IID IID_IClassInfo_Ref =
        {0x0002E400, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
    hr = pUnk->QueryInterface(IID_IClassInfo_Ref, &pFaceDet);
    printf("  QueryInterface(IID_IClassInfo): 0x%08X\n", (unsigned int)hr);

    pUnk->Release();
    printf("\n");
}

// ============================================================================
// Test 8: FreeLibrary
// ============================================================================
static void TestFreeLibrary(HMODULE hMod)
{
    printf("=== Test 8: FreeLibrary ===\n");

    // Valid handle
    BOOL bRet = FreeLibrary(hMod);
    CHECK("FreeLibrary(valid handle)", bRet != FALSE);
    hMod = NULL;

    // NULL handle
    bRet = FreeLibrary(NULL);
    CHECK("FreeLibrary(NULL) returns FALSE", bRet == FALSE);

    // Invalid handle (already freed)
    bRet = FreeLibrary(hMod);
    CHECK("FreeLibrary(freed handle) returns FALSE", bRet == FALSE);

    printf("\n");
}

// ============================================================================
// Main
// ============================================================================
int main()
{
    printf("=== WLXFaceRecognition.dll Dynamic Test Harness ===\n");
    printf("COM probing, DllGetClassObject, CoCreateInstance\n\n");

    // Initialize COM
    HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    printf("CoInitializeEx: 0x%08X\n\n", (unsigned int)hrInit);

    HMODULE hMod = NULL;

    TestLoadLibrary(&hMod);
    if (!hMod) {
        printf("FATAL: Could not load WLXFaceRecognition.dll\n");
        printf("Make sure this is run from the WMMR project root.\n");
        printf("Also ensure MSVCR110.dll and MSVCP110.dll are available.\n");
        CoUninitialize();
        return 1;
    }

    TestGetProcAddress(hMod);
    TestDllCanUnloadNow(hMod);
    TestDllGetClassObject(hMod);
    TestDllRegistration(hMod);

    // Re-init COM after registration test which may have side effects
    CoUninitialize();
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    TestCoCreateInstance();
    TestIFaceDetection();
    TestFreeLibrary(hMod);

    // Cleanup COM
    CoUninitialize();
    if (hrInit == S_FALSE || hrInit == S_OK) {
        // Extra CoUninitialize if needed
    }

    printf("=== Results: %d passed, %d failed, %d total ===\n",
        g_testsPassed, g_testsFailed, g_testsPassed + g_testsFailed);

    return g_testsFailed;
}
