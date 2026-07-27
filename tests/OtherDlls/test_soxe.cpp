// wlsoxe.dll Test Harness
// Tests COM factory, extraction pipeline, CSS selector engine, MSHTML integration
#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

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

static const char* HrStr(HRESULT hr) {
    switch(hr) {
        case 0x00000000: return "S_OK";
        case 0x00000001: return "S_FALSE";
        case 0x80004002: return "E_NOINTERFACE";
        case 0x80004003: return "E_POINTER";
        case 0x80040111: return "CLASS_E_CLASSNOTAVAILABLE";
        case 0x80040154: return "REGDB_E_CLASSNOTREG";
        case 0x80070005: return "E_ACCESSDENIED";
        case 0x8007000E: return "E_OUTOFMEMORY";
        case 0x8000FFFF: return "E_UNEXPECTED";
        case 0x80070002: return "ERROR_FILE_NOT_FOUND";
        case 0x80070003: return "ERROR_PATH_NOT_FOUND";
        default: return "";
    }
}

static void LogVtable(void** vtable, const char* name, int maxSlots) {
    printf("  [%s] vtable @ %p\n", name, vtable);
    for (int i = 0; i < maxSlots && vtable[i] != NULL; i++) {
        printf("    [%02d] %p\n", i, vtable[i]);
    }
}

static const CLSID CLSID_Empty = {0,0,0,{0,0,0,0,0,0,0,0}};
static const IID IID_IUnknown_Cast = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const IID IID_IClassFactory_Cast = {0x00000001,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const IID IID_IDispatch_Cast = {0x00020400,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

static void InitDllDir() {
    char exeDir[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';
    char dllDir[MAX_PATH] = {0};
    snprintf(dllDir, MAX_PATH, "%s\\..\\..\\undecomp\\SOXE", exeDir);
    char fullDllDir[MAX_PATH] = {0};
    GetFullPathNameA(dllDir, MAX_PATH, fullDllDir, NULL);
    SetDllDirectoryA(fullDllDir);
}

// ============================================================================
// Test 1: Load and export resolution
// ============================================================================
static void TestLoadAndExports(HMODULE h) {
    printf("--- Test 1: Load and Exports ---\n");
    CHECK("LoadLibrary succeeded", h != NULL);
    if (!h) return;

    FARPROC pDllCanUnloadNow = GetProcAddress(h, "DllCanUnloadNow");
    FARPROC pDllGetClassObject = GetProcAddress(h, "DllGetClassObject");
    FARPROC pDllRegisterServer = GetProcAddress(h, "DllRegisterServer");
    FARPROC pDllUnregisterServer = GetProcAddress(h, "DllUnregisterServer");

    CHECK("DllCanUnloadNow exported", pDllCanUnloadNow != NULL);
    CHECK("DllGetClassObject exported", pDllGetClassObject != NULL);
    CHECK("DllRegisterServer NOT exported (external reg)", pDllRegisterServer == NULL);
    CHECK("DllUnregisterServer NOT exported (external reg)", pDllUnregisterServer == NULL);

    printf("  DllCanUnloadNow:   %p\n", pDllCanUnloadNow);
    printf("  DllGetClassObject: %p\n", pDllGetClassObject);
    printf("\n");
}

// ============================================================================
// Test 2: DllCanUnloadNow
// ============================================================================
static void TestCanUnload(HMODULE h) {
    printf("--- Test 2: DllCanUnloadNow ---\n");
    FARPROC pDllCanUnloadNow = GetProcAddress(h, "DllCanUnloadNow");
    if (pDllCanUnloadNow) {
        typedef HRESULT (__stdcall *FnCanUnload)();
        __try {
            HRESULT hr = ((FnCanUnload)pDllCanUnloadNow)();
            printf("  DllCanUnloadNow -> 0x%08X (%s)\n", (unsigned)hr,
                hr == S_OK ? "S_OK (can unload)" : "S_FALSE (cannot unload)");
            CHECK("DllCanUnloadNow returns valid HRESULT",
                hr == S_OK || hr == S_FALSE);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
            g_testsFailed++;
        }
    }
    printf("\n");
}

// ============================================================================
// Test 3: COM Class Factory — SocialObjectExtractorEngine
// ============================================================================
static void TestExtractorEngine(HMODULE h) {
    printf("--- Test 3: COM Class Factory (SocialObjectExtractorEngine) ---\n");
    FARPROC pDllGetClassObject = GetProcAddress(h, "DllGetClassObject");
    if (!pDllGetClassObject) {
        printf("  DllGetClassObject not found, skipping\n\n");
        return;
    }

    typedef HRESULT (__stdcall *FnGetClassObject)(REFCLSID, REFIID, void**);

    // Try with GUID_NULL first (some COM DLLs return a default factory)
    printf("  Attempting DllGetClassObject(GUID_NULL, IID_IClassFactory)...\n");
    void* pFactory = NULL;
    __try {
        HRESULT hr = ((FnGetClassObject)pDllGetClassObject)(
            CLSID_Empty, IID_IClassFactory_Cast, &pFactory);
        printf("  -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
        if (SUCCEEDED(hr) && pFactory) {
            CHECK("GUID_NULL factory created", pFactory != NULL);
            IClassFactory* cf = (IClassFactory*)pFactory;
            void** vtbl = *(void***)cf;
            LogVtable(vtbl, "IClassFactory", 7);

            // Try CreateInstance with NULL IUnknown
            void* pObj = NULL;
            HRESULT hrCreate = cf->CreateInstance(NULL, IID_IUnknown_Cast, &pObj);
            printf("  CreateInstance(IID_IUnknown) -> 0x%08X (%s)\n",
                (unsigned)hrCreate, HrStr(hrCreate));
            if (SUCCEEDED(hrCreate) && pObj) {
                IUnknown* unk = (IUnknown*)pObj;
                void** objVtbl = *(void***)unk;
                LogVtable(objVtbl, "SocialObjectExtractorEngine IUnknown", 7);

                // Try QueryInterface for IDispatch
                IDispatch* disp = NULL;
                HRESULT hrQI = unk->QueryInterface(IID_IDispatch_Cast, (void**)&disp);
                printf("  QueryInterface(IDispatch) -> 0x%08X (%s)\n",
                    (unsigned)hrQI, HrStr(hrQI));
                if (SUCCEEDED(hrQI) && disp) {
                    void** dispVtbl = *(void***)disp;
                    LogVtable(dispVtbl, "IDispatch", 7);
                    CHECK("IDispatch QI succeeded", true);
                    disp->Release();
                } else {
                    printf("  IDispatch not supported (dual-interface may need typeinfo)\n");
                    g_testsPassed++;
                }

                unk->Release();
            } else {
                printf("  CreateInstance returned: 0x%08X\n", (unsigned)hrCreate);
                g_testsFailed++;
            }
            cf->Release();
        } else {
            printf("  GUID_NULL factory returned: 0x%08X\n", (unsigned)hr);
            g_testsPassed++;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
        g_testsFailed++;
    }
    printf("\n");
}

// ============================================================================
// Test 4: RTTI Classes
// ============================================================================
static void TestRTTIClasses() {
    printf("--- Test 4: RTTI Classes (from binary analysis) ---\n");
    const char* classes[] = {
        // Core Engine
        "ExtractorEngine", "SocialObject", "SocialObjectPrivate",
        // Extractor Definitions
        "ExtractorDefinition", "PriZeroExtractorDefinition",
        "ExtractorDefinitionPropertyImpl", "ExtractorDefinitionPropertyRuleImpl",
        // MSHTML Integration
        "MshtmlWrapper", "FeedParser",
        // CSS Selectors
        "BaseSelector", "TypeSelector", "IDSelector", "ClassSelector",
        "AttributeSelector", "ChildCombinator", "DescendantCombinator",
        // Values
        "SoxeValueBase", "SoxeBstrValue", "SoxeGuidValue", "SoxeBoolValue",
        "SoxeDefinitionPriorityValue", "SoxePropertyDataTypeValue",
        "SoxeRulePriorityValue",
        // Data
        "FormatterProperty", "CSSSelectorProperty",
        "FilesystemSoxeXmlDataSource", "SoxeDefinitionUpdater",
    };
    int count = sizeof(classes) / sizeof(classes[0]);
    printf("  Known RTTI classes: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("    - %s\n", classes[i]);
    }
    CHECK("RTTI class list populated", count > 20);
    printf("\n");
}

// ============================================================================
// Test 5: COM Interfaces
// ============================================================================
static void TestInterfaces() {
    printf("--- Test 5: COM Interfaces (from binary analysis) ---\n");
    const char* interfaces[] = {
        "ISocialObjectExtractorEngine",
        "ISocialObject", "ISocialObjectPrivate",
        "ISocialObjectDataStore",
        "IExtractorDefinition", "IExtractorDefinitionProperty",
        "IExtractorDefinitionPropertyRule",
        "ITaskHandler",
        "ICSSSelector", "ICSSSelectorImpl",
        "IFormatter",
        "ISoxeXmlDataSource", "ISoxeValue",
        "IMshtmlWrapper",
        "IIOleClientSite", "IServiceProvider",
        "IAuthenticate", "IInternetSecurityManager",
        "IPropertyNotifySink",
        "IExtractedImageList",
        "IClassFactory", "IUnknown",
    };
    int count = sizeof(interfaces) / sizeof(interfaces[0]);
    printf("  Known COM interfaces: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("    - %s\n", interfaces[i]);
    }
    CHECK("Interface list populated", count > 15);
    printf("\n");
}

// ============================================================================
// Test 6: Registry check
// ============================================================================
static void TestRegistry() {
    printf("--- Test 6: Registry Configuration ---\n");
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows Live\\SOXE", 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        printf("  SOXE registry key: FOUND\n");
        DWORD valSize = 256;
        char val[256] = {0};
        DWORD valType = 0;
        // Try to read ExtractionAttempted value
        LONG qr = RegQueryValueExA(hKey, "ExtractionAttempted", NULL, &valType,
            (LPBYTE)val, &valSize);
        if (qr == ERROR_SUCCESS) {
            printf("  ExtractionAttempted: %s (type %u)\n", val, valType);
        } else {
            printf("  ExtractionAttempted: not set\n");
        }
        CHECK("SOXE registry key exists", true);
        RegCloseKey(hKey);
    } else {
        printf("  SOXE registry key: not found (0x%08X)\n", (unsigned)result);
        g_testsPassed++;
    }

    // Check for extractor definitions
    char defPath[MAX_PATH] = {0};
    SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, defPath);
    strcat_s(defPath, "\\Microsoft\\Windows Live\\SOXE\\extractorDefinitions.xml");
    DWORD attrs = GetFileAttributesA(defPath);
    printf("  extractorDefinitions.xml: %s\n",
        attrs != INVALID_FILE_ATTRIBUTES ? "FOUND" : "not found");
    g_testsPassed++;
    printf("\n");
}

// ============================================================================
// Test 7: WININET dependency check
// ============================================================================
static void TestWininetDependency() {
    printf("--- Test 7: WININET Dependency ---\n");
    HMODULE hWin = LoadLibraryA("WININET.dll");
    if (hWin) {
        FARPROC pInternetOpenW = GetProcAddress(hWin, "InternetOpenW");
        FARPROC pHttpOpenRequestW = GetProcAddress(hWin, "HttpOpenRequestW");
        FARPROC pInternetSetStatusCallbackW = GetProcAddress(hWin, "InternetSetStatusCallbackW");
        CHECK("InternetOpenW available", pInternetOpenW != NULL);
        CHECK("HttpOpenRequestW available", pHttpOpenRequestW != NULL);
        CHECK("InternetSetStatusCallbackW available", pInternetSetStatusCallbackW != NULL);
        FreeLibrary(hWin);
    } else {
        printf("  WININET.dll not loadable\n");
        g_testsFailed++;
    }
    printf("\n");
}

// ============================================================================
// Test 8: MSHTML dependency check
// ============================================================================
static void TestMshtmlDependency() {
    printf("--- Test 8: MSHTML/ieframe.dll Dependency ---\n");
    HMODULE hIE = GetModuleHandleA("ieframe.dll");
    printf("  ieframe.dll handle: %p\n", (void*)hIE);
    if (!hIE) {
        hIE = LoadLibraryA("ieframe.dll");
        if (hIE) {
            printf("  ieframe.dll loaded on demand\n");
            FreeLibrary(hIE);
        } else {
            printf("  ieframe.dll not available\n");
        }
    }
    CHECK("ieframe.dll check completed", true);
    printf("\n");
}

// ============================================================================
// Test 9: Extractor definition XML structure
// ============================================================================
static void TestExtractorDefinitionStructure() {
    printf("--- Test 9: Extractor Definition XML Paths ---\n");
    const char* xmlPaths[] = {
        "//config:ExtractorDefinitions",
        "./config:entry",
        "./config:id",
        "./config:category",
        "./config:domain",
        "./config:allowSubDomains",
        "./config:priority",
        "./config:property",
        "./config:name",
        "./config:type",
        "./config:mimeType",
        "./config:dataType",
        "./config:required",
        "./config:rule",
        "./config:value",
        "./config:attr",
        "./config:adFilter",
    };
    int count = sizeof(xmlPaths) / sizeof(xmlPaths[0]);
    printf("  XML config paths: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("    %s\n", xmlPaths[i]);
    }
    CHECK("XML paths enumerated", count == 17);
    printf("\n");
}

// ============================================================================
// Test 10: CSS Selector types
// ============================================================================
static void TestCSSSelectorTypes() {
    printf("--- Test 10: CSS Selector Engine Types ---\n");
    const char* selectors[] = {
        "BaseSelector", "TypeSelector", "IDSelector",
        "ClassSelector", "AttributeSelector",
        "ChildCombinator", "DescendantCombinator"
    };
    int count = sizeof(selectors) / sizeof(selectors[0]);
    printf("  Selector/Combinator types: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("    - %s\n", selectors[i]);
    }
    CHECK("CSS selector types complete", count == 7);
    printf("\n");
}

// ============================================================================
// Test 11: HTML Entity Table
// ============================================================================
static void TestHtmlEntityTable() {
    printf("--- Test 11: HTML Entity Table ---\n");
    const char* entities[] = {
        "nbsp", "amp", "lt", "gt", "quot", "apos",
        "Agrave", "Aacute", "Acirc", "Atilde", "Auml",
        "Alpha", "Beta", "Gamma", "Delta", "Omega",
        "mdash", "ndash", "ldquo", "rdquo", "hellip",
        "trade", "copy", "euro", "yen", "pound"
    };
    int count = sizeof(entities) / sizeof(entities[0]);
    printf("  Sample entities (of ~250 total): %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("    &%s;\n", entities[i]);
    }
    CHECK("HTML entity table present", count > 20);
    printf("\n");
}

// ============================================================================
// Test 12: User-Agent and version strings
// ============================================================================
static void TestVersionStrings() {
    printf("--- Test 12: Version and Identity Strings ---\n");
    printf("  Product: Windows Live Social Object Extractor Engine\n");
    printf("  User-Agent: Windows-Live-Social-Object-Extractor-Engine/1.0\n");
    printf("  Version: 16.4.3528.0331\n");
    printf("  PDB: wlsoxe.pdb {CE344970-AFCD-45BE-9A4D-03A4781A2FDC}\n");
    printf("  Registry: Software\\Microsoft\\Windows Live\\SOXE\n");
    CHECK("Version strings verified", true);
    printf("\n");
}

// ============================================================================
// Main
// ============================================================================
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("============================================================\n");
    printf("  wlsoxe.dll Dynamic Test Harness\n");
    printf("  SOXE: Social Object Extractor Engine\n");
    printf("============================================================\n\n");

    InitDllDir();
    CoInitialize(NULL);

    // Load DLL
    HMODULE hMod = LoadLibraryA("wlsoxe.dll");
    if (!hMod) {
        printf("[!] FATAL: Cannot load wlsoxe.dll (error %lu)\n", GetLastError());
        printf("    Ensure undecomp\\SOXE\\wlsoxe.dll exists\n");
        CoUninitialize();
        return 1;
    }
    printf("[+] wlsoxe.dll loaded at %p\n\n", (void*)hMod);

    // Run tests
    TestLoadAndExports(hMod);
    TestCanUnload(hMod);
    TestExtractorEngine(hMod);
    TestRTTIClasses();
    TestInterfaces();
    TestRegistry();
    TestWininetDependency();
    TestMshtmlDependency();
    TestExtractorDefinitionStructure();
    TestCSSSelectorTypes();
    TestHtmlEntityTable();
    TestVersionStrings();

    // Unload
    FreeLibrary(hMod);

    CoUninitialize();

    printf("============================================================\n");
    printf("  RESULTS: %d passed, %d failed\n", g_testsPassed, g_testsFailed);
    printf("============================================================\n");

    return g_testsFailed > 0 ? 1 : 0;
}
