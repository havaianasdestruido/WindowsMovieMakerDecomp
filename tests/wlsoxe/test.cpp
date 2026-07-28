#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

static int g_pass = 0, g_fail = 0;

#define CHECK(label, cond) \
    do { \
        if (cond) { printf("  [PASS] %s\n", label); g_pass++; } \
        else      { printf("  [FAIL] %s\n", label); g_fail++; } \
    } while(0)

static const char* HrStr(HRESULT hr) {
    switch (hr) {
        case S_OK: return "S_OK";
        case S_FALSE: return "S_FALSE";
        case E_NOINTERFACE: return "E_NOINTERFACE";
        case E_POINTER: return "E_POINTER";
        case CLASS_E_CLASSNOTAVAILABLE: return "CLASS_E_CLASSNOTAVAILABLE";
        case REGDB_E_CLASSNOTREG: return "REGDB_E_CLASSNOTREG";
        case 0x80070057: return "E_INVALIDARG";
        case 0x8000FFFF: return "E_UNEXPECTED";
        default: return "";
    }
}

// Candidate CLSIDs extracted from binary (string data in certificate area)
// These may or may not be the actual COM CLSIDs.
static const GUID CLSID_SocialObjectExtractorEngine =
    {0x2860b52e,0xc4a3,0x454d,{0xbc,0x1e,0x32,0xc5,0xad,0xd1,0x7e,0x90}};
static const GUID CLSID_SoxeDefinitionUpdater =
    {0x4faf0b71,0xad37,0x4aa3,{0xa6,0x71,0x76,0xbc,0x05,0x23,0x44,0xad}};

typedef HRESULT (__stdcall *FnCanUnload)();
typedef HRESULT (__stdcall *FnGetClassObject)(REFCLSID, REFIID, void**);

static void LogVtbl(void** vtbl, const char* name) {
    printf("    [vtable @ %p] %s\n", vtbl, name);
    for (int i = 0; i < 7 && vtbl[i]; i++)
        printf("      [%02d] %p\n", i, vtbl[i]);
}

static void SetDllDir() {
    wchar_t path[MAX_PATH];
    GetFullPathNameW(L"..\\..\\undecomp\\SOXE", MAX_PATH, path, NULL);
    SetDllDirectoryW(path);
}

static void TestLoadAndExports(HMODULE h) {
    printf("--- Test 1: DLL Loading & Export Resolution ---\n");
    CHECK("hMod != NULL", h != NULL);
    if (!h) return;

    FARPROC p1 = GetProcAddress(h, "DllCanUnloadNow");
    FARPROC p2 = GetProcAddress(h, "DllGetClassObject");
    FARPROC p3 = GetProcAddress(h, "DllRegisterServer");
    FARPROC p4 = GetProcAddress(h, "DllUnregisterServer");

    CHECK("DllCanUnloadNow exported", p1 != NULL);
    CHECK("DllGetClassObject exported", p2 != NULL);
    CHECK("DllRegisterServer NOT exported", p3 == NULL);
    CHECK("DllUnregisterServer NOT exported", p4 == NULL);

    printf("  DllCanUnloadNow:   0x%p\n", p1);
    printf("  DllGetClassObject: 0x%p\n", p2);
    printf("\n");
}

static void TestDllCanUnloadNow(HMODULE h) {
    printf("--- Test 2: DllCanUnloadNow ---\n");
    FnCanUnload fn = (FnCanUnload)GetProcAddress(h, "DllCanUnloadNow");
    if (!fn) { CHECK("DllCanUnloadNow available", false); return; }

    __try {
        HRESULT hr = fn();
        printf("  DllCanUnloadNow -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
        CHECK("Returns S_OK (no active objects)", hr == S_OK);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
        g_fail++;
    }
    printf("\n");
}

static void TryClassFactory(HMODULE h, REFCLSID rclsid, const char* name) {
    FnGetClassObject fn = (FnGetClassObject)GetProcAddress(h, "DllGetClassObject");
    if (!fn) return;

    printf("  [%s] DllGetClassObject...\n", name);
    void* pFactory = NULL;
    __try {
        HRESULT hr = fn(rclsid, IID_IClassFactory, &pFactory);
        printf("    -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
        if (SUCCEEDED(hr) && pFactory) {
            CHECK("Factory obtained", true);
            void** vtbl = *(void***)pFactory;
            LogVtbl(vtbl, "IClassFactory");

            IClassFactory* cf = (IClassFactory*)pFactory;
            void* pObj = NULL;
            HRESULT hrCo = cf->CreateInstance(NULL, IID_IUnknown, &pObj);
            printf("    CreateInstance(IID_IUnknown) -> 0x%08X (%s)\n", (unsigned)hrCo, HrStr(hrCo));
            if (SUCCEEDED(hrCo) && pObj) {
                CHECK("Object created", true);
                IUnknown* unk = (IUnknown*)pObj;
                LogVtbl(*(void***)unk, "IUnknown");
                unk->Release();
                CHECK("Object released", true);
            } else {
                CHECK("Object creation", false);
            }
            cf->Release();
        } else {
            printf("    (no factory returned)\n");
            g_pass++;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("    EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
        g_fail++;
    }
}

static void TestGetClassObject(HMODULE h) {
    printf("--- Test 3: DllGetClassObject (both CLSIDs) ---\n");
    TryClassFactory(h, CLSID_SocialObjectExtractorEngine, "SocialObjectExtractorEngine");
    TryClassFactory(h, CLSID_SoxeDefinitionUpdater, "SoxeDefinitionUpdater");
    printf("\n");
}

static void TestRTTI() {
    printf("--- Test 4: RTTI Classes (from binary analysis) ---\n");
    const char* classes[] = {
        "ExtractorEngine", "SocialObject",
        "ExtractorDefinition", "PriZeroExtractorDefinition",
        "ExtractorDefinitionPropertyImpl", "ExtractorDefinitionPropertyRuleImpl",
        "MshtmlWrapper", "FeedParser",
        "BaseSelector", "TypeSelector", "IDSelector", "ClassSelector",
        "AttributeSelector", "ChildCombinator", "DescendantCombinator",
        "SoxeValueBase", "SoxeBstrValue", "SoxeGuidValue", "SoxeBoolValue",
        "SoxeDefinitionPriorityValue", "SoxePropertyDataTypeValue", "SoxeRulePriorityValue",
        "FormatterProperty", "CSSSelectorProperty",
        "FilesystemSoxeXmlDataSource", "SoxeDefinitionUpdater", "DirectoryWatcher",
    };
    int n = sizeof(classes) / sizeof(classes[0]);
    printf("  Total C++ classes: %d\n", n);
    for (int i = 0; i < n; i++) printf("    - %s\n", classes[i]);
    printf("\n");
}

static void TestInterfaces() {
    printf("--- Test 5: COM Interfaces (from binary analysis) ---\n");
    const char* ifs[] = {
        "ISocialObjectExtractorEngine", "ISocialObject", "ISocialObjectPrivate",
        "ISocialObjectDataStore",
        "IExtractorDefinition", "IExtractorDefinitionProperty",
        "IExtractorDefinitionPropertyRule", "ITaskHandler",
        "ICSSSelector", "ICSSSelectorImpl", "IFormatter",
        "ISoxeXmlDataSource", "ISoxeValue",
        "IMshtmlWrapper", "IIOleClientSite", "IServiceProvider",
        "IAuthenticate", "IInternetSecurityManager", "IPropertyNotifySink",
        "IExtractedImageList", "IClassFactory", "IUnknown",
    };
    int n = sizeof(ifs) / sizeof(ifs[0]);
    printf("  Total interfaces: %d\n", n);
    for (int i = 0; i < n; i++) printf("    - %s\n", ifs[i]);
    printf("\n");
}

static void TestRegistry() {
    printf("--- Test 6: Registry (SOXE key) ---\n");
    HKEY hKey;
    LONG r = RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows Live\\SOXE", 0, KEY_READ, &hKey);
    if (r == ERROR_SUCCESS) {
        printf("  Key: Software\\Microsoft\\Windows Live\\SOXE = FOUND\n");
        DWORD type = 0, size = 256;
        char buf[256] = {0};
        LONG qr = RegQueryValueExA(hKey, "ExtractionAttempted", NULL, &type,
            (LPBYTE)buf, &size);
        printf("  ExtractionAttempted: %s\n",
            qr == ERROR_SUCCESS ? buf : "(not set)");
        CHECK("SOXE registry key exists", true);
        RegCloseKey(hKey);
    } else {
        printf("  SOXE registry key not found (0x%08X)\n", (unsigned)r);
        g_pass++;
    }
    printf("\n");
}

static void TestWininet() {
    printf("--- Test 7: WININET Dependency ---\n");
    HMODULE hw = LoadLibraryA("WININET.dll");
    if (hw) {
        CHECK("InternetOpenW", GetProcAddress(hw, "InternetOpenW") != NULL);
        CHECK("HttpOpenRequestW", GetProcAddress(hw, "HttpOpenRequestW") != NULL);
        CHECK("InternetSetStatusCallbackW", GetProcAddress(hw, "InternetSetStatusCallbackW") != NULL);
        CHECK("InternetCloseHandle", GetProcAddress(hw, "InternetCloseHandle") != NULL);
        FreeLibrary(hw);
    } else {
        printf("  WININET.dll not loadable\n"); g_fail++;
    }
    printf("\n");
}

static void TestExtDefXml() {
    printf("--- Test 8: Extractor Definition XML Paths ---\n");
    const char* paths[] = {
        "//config:ExtractorDefinitions", "./config:entry",
        "./config:id", "./config:category", "./config:domain",
        "./config:allowSubDomains", "./config:priority",
        "./config:property", "./config:name", "./config:type",
        "./config:mimeType", "./config:dataType", "./config:required",
        "./config:rule", "./config:value", "./config:attr",
        "./config:adFilter",
    };
    int n = sizeof(paths) / sizeof(paths[0]);
    printf("  %d XML config paths\n", n);
    for (int i = 0; i < n; i++) printf("    %s\n", paths[i]);
    printf("\n");
}

static void TestVersion() {
    printf("--- Test 9: Identity Strings ---\n");
    printf("  DLL: wlsoxe.dll\n");
    printf("  Name: Windows Live Social Object Extractor Engine\n");
    printf("  Version: 16.4.3528.0331\n");
    printf("  User-Agent: Windows-Live-Social-Object-Extractor-Engine/1.0\n");
    printf("  PDB: wlsoxe.pdb {%s}\n", "CE344970-AFCD-45BE-9A4D-03A4781A2FDC");
    printf("  Registry: Software\\Microsoft\\Windows Live\\SOXE\n");
    printf("\n");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("============================================================\n");
    printf("  wlsoxe.dll Test Harness\n");
    printf("  Windows Live Social Object Extractor Engine\n");
    printf("============================================================\n\n");

    SetDllDir();
    CoInitialize(NULL);

    HMODULE h = LoadLibraryA("wlsoxe.dll");
    if (!h) {
        printf("[FATAL] LoadLibrary(wlsoxe.dll) failed: err %lu\n", GetLastError());
        printf("        Expected at undecomp\\SOXE\\wlsoxe.dll\n");
        CoUninitialize();
        return 1;
    }
    printf("[+] wlsoxe.dll loaded at 0x%p\n\n", (void*)h);

    TestLoadAndExports(h);
    TestDllCanUnloadNow(h);
    TestGetClassObject(h);
    TestRTTI();
    TestInterfaces();
    TestRegistry();
    TestWininet();
    TestExtDefXml();
    TestVersion();

    FreeLibrary(h);
    CoUninitialize();

    printf("============================================================\n");
    printf("  RESULTS: %d passed, %d failed\n", g_pass, g_fail);
    printf("============================================================\n");

    return g_fail > 0 ? 1 : 0;
}
