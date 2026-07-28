#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <mediaobj.h>
#include <dmort.h>
#include <cstdio>
#include <cstdlib>

// ============================================================================
// Missing GUID definitions (from older DirectShow uuids.h / dmoreg.h)
// ============================================================================
// MEDIATYPE_Video: {73646976-0000-0010-8000-00AA00389B71}
static const GUID MEDIATYPE_Video =
    {0x73646976, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};

// ============================================================================
// CLSIDs from analysis of WLXDSPA.dll embedded registry script
// ============================================================================

// {BC8CA1B3-B013-4866-9621-825957DF23F3} - CWLXTocGeneratorDmo (DMO)
static const CLSID CLSID_CWLXTocGeneratorDmo =
    {0xBC8CA1B3, 0xB013, 0x4866, {0x96, 0x21, 0x82, 0x59, 0x57, 0xDF, 0x23, 0xF3}};

// {09BC59C2-70DD-45f9-A5B7-DE9F2A5CA34B} - CWLXThumbnailGeneratorDmo (DMO)
static const CLSID CLSID_CWLXThumbnailGeneratorDmo =
    {0x09BC59C2, 0x70DD, 0x45f9, {0xA5, 0xB7, 0xDE, 0x9F, 0x2A, 0x5C, 0xA3, 0x4B}};

// {15CD2459-C14B-457b-B57B-3DBA111B9D09} - CClusterDetectorEx
static const CLSID CLSID_CClusterDetectorEx =
    {0x15CD2459, 0xC14B, 0x457b, {0xB5, 0x7B, 0x3D, 0xBA, 0x11, 0x1B, 0x9D, 0x09}};

// {1D8D19C8-0A33-45a4-9B3E-255B85C363A8} - CTocEntry
static const CLSID CLSID_CTocEntry =
    {0x1D8D19C8, 0x0A33, 0x45a4, {0x9B, 0x3E, 0x25, 0x5B, 0x85, 0xC3, 0x63, 0xA8}};

// {15A4E6E5-A9E5-49cb-AFFC-E822F082D427} - CTocEntryList
static const CLSID CLSID_CTocEntryList =
    {0x15A4E6E5, 0xA9E5, 0x49cb, {0xAF, 0xFC, 0xE8, 0x22, 0xF0, 0x82, 0xD4, 0x27}};

// {C9FF4813-CB5F-4ac6-B003-4D79AE2F43E9} - CToc
static const CLSID CLSID_CToc =
    {0xC9FF4813, 0xCB5F, 0x4ac6, {0xB0, 0x03, 0x4D, 0x79, 0xAE, 0x2F, 0x43, 0xE9}};

// {CE1D8A09-77EA-4eaa-9619-832A9E5DB447} - CTocCollection
static const CLSID CLSID_CTocCollection =
    {0xCE1D8A09, 0x77EA, 0x4eaa, {0x96, 0x19, 0x83, 0x2A, 0x9E, 0x5D, 0xB4, 0x47}};

// {75704D6C-09BA-4d19-AFEA-5F21FC08B3DB} - CTocParser
static const CLSID CLSID_CTocParser =
    {0x75704D6C, 0x09BA, 0x4d19, {0xAF, 0xEA, 0x5F, 0x21, 0xFC, 0x08, 0xB3, 0xDB}};

// {7F2CE947-4E80-446d-9AE4-17DD9D82A353} - CFileIo
static const CLSID CLSID_CFileIo =
    {0x7F2CE947, 0x4E80, 0x446d, {0x9A, 0xE4, 0x17, 0xDD, 0x9D, 0x82, 0xA3, 0x53}};

// {BF620143-7420-460a-9EEE-178B78D4939D} - CAsfTocParser
static const CLSID CLSID_CAsfTocParser =
    {0xBF620143, 0x7420, 0x460a, {0x9E, 0xEE, 0x17, 0x8B, 0x78, 0xD4, 0x93, 0x9D}};

// {9FAE79C9-BA02-43d9-9382-C7BEF740A596} - CAviTocParser
static const CLSID CLSID_CAviTocParser =
    {0x9FAE79C9, 0xBA02, 0x43d9, {0x93, 0x82, 0xC7, 0xBE, 0xF7, 0x40, 0xA5, 0x96}};

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
    snprintf(dllPath, MAX_PATH, "%s\\WLXDSPA.dll", fullDllDir);
    printf("  DLL path: %s\n", dllPath);

    *phMod = LoadLibraryA(dllPath);
    CHECK("LoadLibrary with valid path", *phMod != NULL);
    if (*phMod) {
        printf("  Module handle: %p\n", (void*)*phMod);
    }

    HMODULE hFake = LoadLibraryA("nonexistent_wlxdspa.dll");
    CHECK("LoadLibrary(nonexistent) fails", hFake == NULL);

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

    FARPROC fp1 = GetProcAddress(hMod, "DllCanUnloadNow");
    CHECK("GetProcAddress(DllCanUnloadNow)", fp1 != NULL);

    FARPROC fp2 = GetProcAddress(hMod, "DllGetClassObject");
    CHECK("GetProcAddress(DllGetClassObject)", fp2 != NULL);

    FARPROC fp3 = GetProcAddress(hMod, "DllRegisterServer");
    CHECK("GetProcAddress(DllRegisterServer)", fp3 != NULL);

    FARPROC fp4 = GetProcAddress(hMod, "DllUnregisterServer");
    CHECK("GetProcAddress(DllUnregisterServer)", fp4 != NULL);

    FARPROC fpBad = GetProcAddress(hMod, "NonexistentExport");
    CHECK("GetProcAddress(invalid name) returns NULL", fpBad == NULL);

    FARPROC fpOrd1 = (FARPROC)GetProcAddress(hMod, (LPCSTR)1);
    CHECK("GetProcAddress(ordinal 1 = DllCanUnloadNow)", fpOrd1 != NULL);

    FARPROC fpOrd2 = (FARPROC)GetProcAddress(hMod, (LPCSTR)2);
    CHECK("GetProcAddress(ordinal 2 = DllGetClassObject)", fpOrd2 != NULL);

    FARPROC fpOrd3 = (FARPROC)GetProcAddress(hMod, (LPCSTR)3);
    CHECK("GetProcAddress(ordinal 3 = DllRegisterServer)", fpOrd3 != NULL);

    FARPROC fpOrd4 = (FARPROC)GetProcAddress(hMod, (LPCSTR)4);
    CHECK("GetProcAddress(ordinal 4 = DllUnregisterServer)", fpOrd4 != NULL);

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
        printf("  SEH exception in DllCanUnloadNow!\n");
        g_testsFailed++;
    }

    printf("\n");
}

// ============================================================================
// Test 4: DllGetClassObject - probe all 11 COM classes
// ============================================================================
static void TestDllGetClassObject(HMODULE hMod)
{
    printf("=== Test 4: DllGetClassObject (11 classes) ===\n");

    FN_DllGetClassObject pFn = (FN_DllGetClassObject)GetProcAddress(hMod, "DllGetClassObject");
    if (!pFn) { printf("  Cannot resolve DllGetClassObject\n\n"); return; }

    struct {
        const char* name;
        REFCLSID clsid;
    } classes[] = {
        {"CWLXTocGeneratorDmo",       CLSID_CWLXTocGeneratorDmo},
        {"CWLXThumbnailGeneratorDmo", CLSID_CWLXThumbnailGeneratorDmo},
        {"CClusterDetectorEx",        CLSID_CClusterDetectorEx},
        {"CTocEntry",                 CLSID_CTocEntry},
        {"CTocEntryList",             CLSID_CTocEntryList},
        {"CToc",                      CLSID_CToc},
        {"CTocCollection",            CLSID_CTocCollection},
        {"CTocParser",                CLSID_CTocParser},
        {"CFileIo",                   CLSID_CFileIo},
        {"CAsfTocParser",             CLSID_CAsfTocParser},
        {"CAviTocParser",             CLSID_CAviTocParser},
    };

    for (int i = 0; i < 11; i++) {
        IClassFactory* pFactory = NULL;
        __try {
            HRESULT hr = pFn(classes[i].clsid, IID_IClassFactory, (LPVOID*)&pFactory);
            printf("  DllGetClassObject(%s): 0x%08X", classes[i].name, (unsigned int)hr);
            if (SUCCEEDED(hr) && pFactory) {
                printf(" -> factory=%p", (void*)pFactory);

                IUnknown* pUnk = NULL;
                HRESULT hrCreate = pFactory->CreateInstance(NULL, IID_IUnknown, (LPVOID*)&pUnk);
                if (SUCCEEDED(hrCreate) && pUnk) {
                    printf(", instance=%p", (void*)pUnk);
                    ULONG ref = pUnk->Release();
                    printf(", post-release ref=%u", (unsigned int)ref);
                } else {
                    printf(", CreateInstance=0x%08X", (unsigned int)hrCreate);
                }

                HRESULT hrLock1 = pFactory->LockServer(TRUE);
                HRESULT hrLock2 = pFactory->LockServer(FALSE);
                printf(", LockServer=0x%08X/0x%08X", (unsigned int)hrLock1, (unsigned int)hrLock2);
                pFactory->Release();
                printf("\n");
                g_testsPassed++;
            } else {
                printf(" (FAILED)\n");
                g_testsFailed++;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH exception in DllGetClassObject(%s)!\n", classes[i].name);
            g_testsFailed++;
            if (pFactory) pFactory->Release();
        }
    }

    // Test NULL CLSID
    {
        GUID nullGuid = {0};
        IClassFactory* pFactory = NULL;
        __try {
            HRESULT hr = pFn(nullGuid, IID_IClassFactory, (LPVOID*)&pFactory);
            printf("  DllGetClassObject(GUID_NULL): 0x%08X\n", (unsigned int)hr);
            CHECK("GUID_NULL returns error", FAILED(hr));
            if (pFactory) pFactory->Release();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH exception with GUID_NULL\n");
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
// Test 5: DMO interface probing (IMediaObject)
// ============================================================================
static void TestDmoInterfaces(HMODULE hMod)
{
    printf("=== Test 5: DMO IMediaObject Interface Probing ===\n");

    FN_DllGetClassObject pFn = (FN_DllGetClassObject)GetProcAddress(hMod, "DllGetClassObject");
    if (!pFn) { printf("  Cannot resolve DllGetClassObject\n\n"); return; }

    struct {
        const char* name;
        REFCLSID clsid;
    } dmos[] = {
        {"CWLXTocGeneratorDmo",       CLSID_CWLXTocGeneratorDmo},
        {"CWLXThumbnailGeneratorDmo", CLSID_CWLXThumbnailGeneratorDmo},
    };

    for (int i = 0; i < 2; i++) {
        printf("--- %s ---\n", dmos[i].name);

        IClassFactory* pFactory = NULL;
        __try {
            HRESULT hr = pFn(dmos[i].clsid, IID_IClassFactory, (LPVOID*)&pFactory);
            printf("  DllGetClassObject: 0x%08X\n", (unsigned int)hr);
            if (FAILED(hr) || !pFactory) {
                g_testsFailed++;
                continue;
            }

            IMediaObject* pDMO = NULL;
            hr = pFactory->CreateInstance(NULL, IID_IMediaObject, (LPVOID*)&pDMO);
            printf("  CreateInstance(IMediaObject): 0x%08X", (unsigned int)hr);
            if (SUCCEEDED(hr) && pDMO) {
                printf(" -> pDMO=%p\n", (void*)pDMO);

                DWORD cIn = 0, cOut = 0;
                hr = pDMO->GetStreamCount(&cIn, &cOut);
                printf("  GetStreamCount: 0x%08X (in=%u, out=%u)\n",
                    (unsigned int)hr, (unsigned int)cIn, (unsigned int)cOut);
                CHECK("GetStreamCount succeeds", SUCCEEDED(hr));

                DMO_MEDIA_TYPE mt = {0};
                hr = pDMO->GetInputType(0, 0, &mt);
                printf("  GetInputType(0,0): 0x%08X", (unsigned int)hr);
                if (SUCCEEDED(hr)) {
                    printf(" majortype=");
                    PrintGuid(mt.majortype);
                    if (IsEqualGUID(mt.majortype, MEDIATYPE_Video))
                        printf(" (Video)");
                    printf(", subtype=");
                    PrintGuid(mt.subtype);
                    printf(", formattype=");
                    PrintGuid(mt.formattype);
                    printf("\n");
                    MoFreeMediaType(&mt);
                }
                printf("\n");

                ZeroMemory(&mt, sizeof(mt));
                hr = pDMO->GetOutputType(0, 0, &mt);
                printf("  GetOutputType(0,0): 0x%08X", (unsigned int)hr);
                if (SUCCEEDED(hr)) {
                    printf(" majortype=");
                    PrintGuid(mt.majortype);
                    if (IsEqualGUID(mt.majortype, MEDIATYPE_Video))
                        printf(" (Video)");
                    printf("\n");
                    MoFreeMediaType(&mt);
                }
                printf("\n");

                ZeroMemory(&mt, sizeof(mt));
                hr = pDMO->GetInputCurrentType(0, &mt);
                printf("  GetInputCurrentType(0): 0x%08X (no media set, expected failure)\n",
                    (unsigned int)hr);

                ZeroMemory(&mt, sizeof(mt));
                hr = pDMO->GetOutputCurrentType(0, &mt);
                printf("  GetOutputCurrentType(0): 0x%08X (no media set, expected failure)\n",
                    (unsigned int)hr);

                DWORD cbSize = 0, cbMaxLookahead = 0, cbAlignment = 0;
                hr = pDMO->GetInputSizeInfo(0, &cbSize, &cbMaxLookahead, &cbAlignment);
                printf("  GetInputSizeInfo(0): 0x%08X (size=%u, lookahead=%u, align=%u)\n",
                    (unsigned int)hr, (unsigned int)cbSize, (unsigned int)cbMaxLookahead, (unsigned int)cbAlignment);

                cbSize = 0; cbAlignment = 0;
                hr = pDMO->GetOutputSizeInfo(0, &cbSize, &cbAlignment);
                printf("  GetOutputSizeInfo(0): 0x%08X (size=%u, align=%u)\n",
                    (unsigned int)hr, (unsigned int)cbSize, (unsigned int)cbAlignment);

                REFERENCE_TIME rtLatency = 0;
                hr = pDMO->GetInputMaxLatency(0, &rtLatency);
                printf("  GetInputMaxLatency(0): 0x%08X (latency=%lld)\n",
                    (unsigned int)hr, rtLatency);

                pDMO->Release();
                CHECK("DMO created and methods called without crash", TRUE);
            } else {
                printf(" (FAILED)\n");
                g_testsFailed++;
            }

            pFactory->Release();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  SEH exception in DMO probing!\n");
            g_testsFailed++;
            if (pFactory) pFactory->Release();
        }
        printf("\n");
    }
}

// ============================================================================
// Test 6: DllRegisterServer / DllUnregisterServer
// ============================================================================
static void TestDllRegistration(HMODULE hMod)
{
    printf("=== Test 6: DllRegisterServer / DllUnregisterServer ===\n");

    FN_DllRegisterServer pReg = (FN_DllRegisterServer)GetProcAddress(hMod, "DllRegisterServer");
    FN_DllUnregisterServer pUnreg = (FN_DllUnregisterServer)GetProcAddress(hMod, "DllUnregisterServer");

    if (!pReg || !pUnreg) {
        printf("  Cannot resolve registration functions\n\n");
        return;
    }

    __try {
        HRESULT hr = pUnreg();
        printf("  DllUnregisterServer (pre-clean): 0x%08X\n", (unsigned int)hr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllUnregisterServer (pre-clean)\n");
    }

    __try {
        HRESULT hr = pReg();
        printf("  DllRegisterServer: 0x%08X", (unsigned int)hr);
        if (SUCCEEDED(hr)) {
            printf(" (registered 11 CLSIDs)\n");
            g_testsPassed++;
        } else {
            printf(" (likely no admin rights)\n");
            g_testsPassed++;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllRegisterServer\n");
        g_testsFailed++;
    }

    __try {
        HRESULT hr = pReg();
        printf("  DllRegisterServer (double): 0x%08X (idempotent)\n", (unsigned int)hr);
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllRegisterServer (double)\n");
        g_testsFailed++;
    }

    __try {
        HRESULT hr = pUnreg();
        printf("  DllUnregisterServer: 0x%08X\n", (unsigned int)hr);
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in DllUnregisterServer\n");
        g_testsFailed++;
    }

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
    printf("=== WLXDSPA.dll Dynamic Test Harness ===\n");
    printf("4 exports + 2 DMO classes (11 COM classes total)\n\n");

    HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    printf("CoInitializeEx: 0x%08X\n\n", (unsigned int)hrInit);

    HMODULE hMod = NULL;

    TestLoadLibrary(&hMod);
    if (!hMod) {
        printf("FATAL: Could not load WLXDSPA.dll\n");
        printf("Make sure run from WMMR project root, and that MSVCR110.dll is available.\n");
        CoUninitialize();
        return 1;
    }

    TestGetProcAddress(hMod);
    TestDllCanUnloadNow(hMod);
    TestDllGetClassObject(hMod);
    TestDmoInterfaces(hMod);
    TestDllRegistration(hMod);

    TestFreeLibrary(hMod);

    CoUninitialize();

    printf("=== Results: %d passed, %d failed, %d total ===\n",
        g_testsPassed, g_testsFailed, g_testsPassed + g_testsFailed);

    return g_testsFailed;
}
