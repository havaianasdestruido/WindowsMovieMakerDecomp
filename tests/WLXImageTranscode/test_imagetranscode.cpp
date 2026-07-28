#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <unknwn.h>
#include <cstdio>

// ============================================================
// GUIDs
// ============================================================
static const IID IID_IUnknown_Cast = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

// WLXImageTranscode.dll COM CLSIDs
// {20575516-78AF-4404-B3C7-51D05F9945B5}
static const CLSID CLSID_ImageTranscode = {0x20575516,0x78AF,0x4404,{0xB3,0xC7,0x51,0xD0,0x5F,0x99,0x45,0xB5}};
// {E253845C-A1A2-4ae4-B784-5E8140CD5B2C}
static const CLSID CLSID_ImageLoader = {0xE253845C,0xA1A2,0x4ae4,{0xB7,0x84,0x5E,0x81,0x40,0xCD,0x5B,0x2C}};
// {B8A2E14E-290D-4122-B092-1A7D86198CCE}
static const CLSID CLSID_WLXOutofProc = {0xB8A2E14E,0x290D,0x4122,{0xB0,0x92,0x1A,0x7D,0x86,0x19,0x8C,0xCE}};
// {3FBB103C-F1B9-47dc-9EB3-A0C07F5F6AFA} — PSFactoryBuffer (proxy/stub)
static const CLSID CLSID_PSFactoryBuffer = {0x3FBB103C,0xF1B9,0x47dc,{0x9E,0xB3,0xA0,0xC0,0x7F,0x5F,0x6A,0xFA}};

// ============================================================
// Helpers
// ============================================================
static const char* GuidToString(REFGUID guid) {
    static char buf[64];
    sprintf_s(buf, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    return buf;
}

static void LogHr(const char* label, HRESULT hr) {
    printf("    %-35s => 0x%08X (%s)\n", label, hr,
        SUCCEEDED(hr) ? "S_OK" : hr == E_NOINTERFACE ? "E_NOINTERFACE" : hr == E_INVALIDARG ? "E_INVALIDARG" : hr == CLASS_E_CLASSNOTAVAILABLE ? "CLASS_E_CLASSNOTAVAILABLE" : "FAILED");
}

static HMODULE g_hDll = NULL;

static void SetSearchPaths() {
    wchar_t galleryDir[MAX_PATH], mainDir[MAX_PATH];
    GetFullPathNameW(L"..\\..\\undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
    GetFullPathNameW(L"..\\..", MAX_PATH, mainDir, NULL);
    AddDllDirectory(galleryDir);
    AddDllDirectory(mainDir);
    SetDllDirectoryW(galleryDir);
}

static bool LoadTargetDll() {
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(L"..\\..\\undecomp\\Photo Gallery\\WLXImageTranscode.dll", MAX_PATH, fullPath, NULL);
    printf("[*] Loading: %ls\n", fullPath);
    g_hDll = LoadLibraryExW(fullPath, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!g_hDll) {
        printf("[!] LoadLibraryEx failed (err %u), retrying with SetDllDirectory\n", GetLastError());
        wchar_t galleryDir[MAX_PATH];
        GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
        SetDllDirectoryW(galleryDir);
        g_hDll = LoadLibraryW(fullPath);
    }
    if (g_hDll) {
        printf("[+] Loaded at base: 0x%p\n", g_hDll);
        return true;
    }
    printf("[!] FAILED to load: error %u\n", GetLastError());
    return false;
}

// ============================================================
// Typedefs
// ============================================================
typedef HRESULT (__stdcall *DllCanUnloadNowFunc)();
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServerFunc)();
typedef HRESULT (__stdcall *DllUnregisterServerFunc)();

// ============================================================
// Test: Probe all 4 standard COM exports
// ============================================================
static void Test_StdExports() {
    printf("\n=== 1. Standard COM Exports ===\n");

    // DllCanUnloadNow
    {
        DllCanUnloadNowFunc fn = (DllCanUnloadNowFunc)GetProcAddress(g_hDll, "DllCanUnloadNow");
        printf("[*] DllCanUnloadNow: ptr=0x%p\n", fn);
        if (fn) {
            __try {
                HRESULT hr = fn();
                LogHr("DllCanUnloadNow", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        }
    }

    // DllGetClassObject
    {
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(g_hDll, "DllGetClassObject");
        printf("[*] DllGetClassObject: ptr=0x%p\n", fn);
        if (fn) {
            // Test with empty CLSID + null ppv (known crash point)
            __try {
                GUID empty = {};
                HRESULT hr = fn(empty, IID_IUnknown_Cast, NULL);
                LogHr("DllGetClassObject (empty CLSID, null ppv)", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X (empty CLSID crash)\n", GetExceptionCode());
            }
        }
    }

    // DllRegisterServer
    {
        DllRegisterServerFunc fn = (DllRegisterServerFunc)GetProcAddress(g_hDll, "DllRegisterServer");
        printf("[*] DllRegisterServer: ptr=0x%p\n", fn);
        if (fn) printf("    [skip] Would modify registry\n");
    }

    // DllUnregisterServer
    {
        DllUnregisterServerFunc fn = (DllUnregisterServerFunc)GetProcAddress(g_hDll, "DllUnregisterServer");
        printf("[*] DllUnregisterServer: ptr=0x%p\n", fn);
        if (fn) printf("    [skip] Would modify registry\n");
    }
}

// ============================================================
// Test: DllGetClassObject for all COM CLSIDs
// ============================================================
struct ClsidEntry {
    const CLSID* clsid;
    const char* name;
};

static void Test_ClassFactories() {
    printf("\n=== 2. COM Class Factory Probing ===\n");

    DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(g_hDll, "DllGetClassObject");
    if (!fn) {
        printf("[!] DllGetClassObject not found — skipping\n");
        return;
    }

    ClsidEntry entries[] = {
        { &CLSID_ImageTranscode, "ImageTranscode" },
        { &CLSID_ImageLoader,    "ImageLoader" },
        { &CLSID_WLXOutofProc,   "WLXOutofProc" },
        { &CLSID_PSFactoryBuffer,"PSFactoryBuffer (proxy/stub)" },
    };

    for (int i = 0; i < 4; i++) {
        printf("\n[*] CLSID: %s\n", entries[i].name);
        printf("    GUID: %s\n", GuidToString(*entries[i].clsid));

        IUnknown* pUnk = NULL;
        __try {
            HRESULT hr = fn(*entries[i].clsid, IID_IUnknown_Cast, (void**)&pUnk);
            LogHr("DllGetClassObject -> IUnknown", hr);

            if (SUCCEEDED(hr) && pUnk) {
                printf("    [+] IUnknown ptr: 0x%p\n", pUnk);

                // QueryInterface for IClassFactory
                IClassFactory* pCF = NULL;
                HRESULT hrQI = pUnk->QueryInterface(IID_IClassFactory, (void**)&pCF);
                LogHr("QI -> IClassFactory", hrQI);
                if (SUCCEEDED(hrQI) && pCF) {
                    printf("    [+] IClassFactory ptr: 0x%p\n", pCF);
                    pCF->Release();
                }

                ULONG refs = pUnk->Release();
                printf("    Release -> refcount: %lu\n", refs);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            if (pUnk) pUnk->Release();
        }
    }
}

// ============================================================
// Test: Instance creation via IClassFactory::CreateInstance
// ============================================================
static void Test_CreateInstance() {
    printf("\n=== 3. IClassFactory::CreateInstance ===\n");

    DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(g_hDll, "DllGetClassObject");
    if (!fn) return;

    ClsidEntry entries[] = {
        { &CLSID_ImageTranscode, "ImageTranscode" },
        { &CLSID_ImageLoader,    "ImageLoader" },
        { &CLSID_WLXOutofProc,   "WLXOutofProc" },
    };

    for (int i = 0; i < 3; i++) {
        printf("\n[*] %s\n", entries[i].name);

        IClassFactory* pCF = NULL;
        __try {
            HRESULT hr = fn(*entries[i].clsid, IID_IClassFactory, (void**)&pCF);
            LogHr("DllGetClassObject -> IClassFactory", hr);

            if (SUCCEEDED(hr) && pCF) {
                // CreateInstance (IUnknown)
                IUnknown* pObj = NULL;
                hr = pCF->CreateInstance(NULL, IID_IUnknown_Cast, (void**)&pObj);
                LogHr("CreateInstance(IID_IUnknown)", hr);
                if (SUCCEEDED(hr) && pObj) {
                    printf("    [+] Object ptr: 0x%p\n", pObj);
                    pObj->Release();
                }

                // CreateInstance with aggregator (NULL should still work)
                pObj = NULL;
                hr = pCF->CreateInstance(NULL, IID_IUnknown_Cast, (void**)&pObj);
                LogHr("CreateInstance(NULL, IUnknown) repeated", hr);
                if (pObj) pObj->Release();

                pCF->Release();
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            if (pCF) pCF->Release();
        }
    }
}

// ============================================================
// Test: Proxy/stub infrastructure (RPCRT4 NdrDll* functions)
// ============================================================
static void Test_ProxyStub() {
    printf("\n=== 4. Proxy/Stub Infrastructure ===\n");

    // Check for NdrDll* imports (exported from RPCRT4, used internally)
    // The DLL imports these from RPCRT4. Check that they resolve.
    HMODULE hRpcRt = GetModuleHandleW(L"RPCRT4.dll");
    if (hRpcRt) {
        printf("[*] RPCRT4.dll loaded at 0x%p\n", hRpcRt);

        struct { const char* name; const char* decorated; } ndrFuncs[] = {
            { "NdrDllGetClassObject",    "NdrDllGetClassObject" },
            { "NdrDllCanUnloadNow",      "NdrDllCanUnloadNow" },
            { "NdrDllRegisterProxy",     "NdrDllRegisterProxy" },
            { "NdrDllUnregisterProxy",   "NdrDllUnregisterProxy" },
            { "CStdStubBuffer_Connect",  "CStdStubBuffer_Connect" },
            { "CStdStubBuffer_Invoke",   "CStdStubBuffer_Invoke" },
            { "CStdStubBuffer_QueryInterface", "CStdStubBuffer_QueryInterface" },
            { "IUnknown_QueryInterface_Proxy",  "IUnknown_QueryInterface_Proxy" },
            { "IUnknown_AddRef_Proxy",          "IUnknown_AddRef_Proxy" },
            { "IUnknown_Release_Proxy",         "IUnknown_Release_Proxy" },
        };
        int n = sizeof(ndrFuncs) / sizeof(ndrFuncs[0]);

        for (int i = 0; i < n; i++) {
            FARPROC p = GetProcAddress(hRpcRt, ndrFuncs[i].decorated);
            printf("  %-35s ptr=0x%p %s\n", ndrFuncs[i].name, p, p ? "" : "(NOT FOUND)");
        }
    } else {
        printf("[!] RPCRT4.dll not loaded\n");
    }

    // Check .orpc section presence
    {
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)g_hDll;
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)g_hDll + dos->e_lfanew);
        IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
            char name[9] = {};
            strncpy_s(name, (char*)sec[i].Name, 8);
            if (strcmp(name, ".orpc") == 0) {
                printf("[*] .orpc section found: VA=0x%08X, Size=0x%X\n",
                    sec[i].VirtualAddress, sec[i].Misc.VirtualSize);
                break;
            }
        }
    }

    // Probe PSFactoryBuffer CLSID via NdrDllGetClassObject
    __try {
        // The DLL itself doesn't export NdrDll*, but the PSFactoryBuffer CLSID
        // should be creatable through the standard DllGetClassObject path.
        // Already tested above — just note it here.
        printf("[*] PSFactoryBuffer CLSID tested in Class Factory section\n");
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("    => SEH exception 0x%08X\n", GetExceptionCode());
    }
}

// ============================================================
// Test: ORPC proxy/stub via NdrDllGetClassObject (if accessible)
// ============================================================
static void Test_OprcProxy() {
    printf("\n=== 5. ORPC Proxy/Stub Direct Invocation ===\n");

    // The NdrDll* functions are imported from RPCRT4, not exported by our DLL.
    // However, a COM server that hosts proxy/stub typically does so via
    // NdrDllGetClassObject when called with the PSFactoryBuffer CLSID.
    // This is normally called by COM's registration-free activation or
    // via CoGetClassObject. Let's try through CoGetClassObject.

    printf("[*] Attempting CoGetClassObject for PSFactoryBuffer...\n");

    // The DLL is already loaded; we can try to route via COM
    // by registering the CLSID in the "InprocServer32" key temporarily.
    // But that modifies registry. Instead, we'll use the class factory
    // obtained earlier and verify it supports proxy/stub interfaces.

    // Check if our loaded DLL has the PSFactory CLSID registered
    IClassFactory* pCF = NULL;
    DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(g_hDll, "DllGetClassObject");
    if (!fn) return;

    __try {
        HRESULT hr = fn(CLSID_PSFactoryBuffer, IID_IClassFactory, (void**)&pCF);
        LogHr("DllGetClassObject(PSFactoryBuffer, IClassFactory)", hr);
        if (SUCCEEDED(hr) && pCF) {
            // Try to create an instance and QI for IRpcChannelBuffer or IRpcProxyBuffer
            IUnknown* pObj = NULL;
            hr = pCF->CreateInstance(NULL, IID_IUnknown_Cast, (void**)&pObj);
            LogHr("CreateInstance(IUnknown)", hr);
            if (SUCCEEDED(hr) && pObj) {
                printf("    [+] Proxy/stub object: 0x%p\n", pObj);
                // QI for IUnknown (should succeed)
                IUnknown* pUnk2 = NULL;
                hr = pObj->QueryInterface(IID_IUnknown, (void**)&pUnk2);
                LogHr("QI(IID_IUnknown)", hr);
                if (pUnk2) pUnk2->Release();
                pObj->Release();
            }

            // Check the TypeLib GUID
            // {17DC7884-443D-478e-ABD7-BC22856FC7F1}
            CLSID typelib = {0x17DC7884,0x443D,0x478e,{0xAB,0xD7,0xBC,0x22,0x85,0x6F,0xC7,0xF1}};
            printf("    TypeLib GUID: %s\n", GuidToString(typelib));

            pCF->Release();
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("    => SEH exception 0x%08X\n", GetExceptionCode());
        if (pCF) pCF->Release();
    }
}

// ============================================================
// Test: .orpc section dump
// ============================================================
static void Test_OrpcSection() {
    printf("\n=== 6. .orpc Section Analysis ===\n");

    BYTE* base = (BYTE*)g_hDll;
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);

    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        char name[9] = {};
        strncpy_s(name, (char*)sec[i].Name, 8);
        if (strcmp(name, ".orpc") == 0) {
            DWORD va = sec[i].VirtualAddress;
            DWORD size = sec[i].Misc.VirtualSize;
            DWORD rawOffset = sec[i].PointerToRawData;
            printf("  Section:  .orpc\n");
            printf("  VA:       0x%08X\n", va);
            printf("  Size:     0x%X (%u bytes)\n", size, size);
            printf("  Raw:      0x%08X\n", rawOffset);
            printf("  Flags:    0x%08X (code, execute, read)\n", sec[i].Characteristics);

            if (size > 0 && rawOffset > 0) {
                printf("\n  [Hex dump of first 128 bytes]:\n    ");
                BYTE* raw = base + rawOffset;
                for (DWORD j = 0; j < size && j < 128; j++) {
                    printf("%02X ", raw[j]);
                    if ((j + 1) % 16 == 0) printf("\n    ");
                }
                printf("\n");
            }
            return;
        }
    }
    printf("  [!] .orpc section not found\n");
}

// ============================================================
// Test: DllCanUnloadNow after class factory interactions
// ============================================================
static void Test_UnloadState() {
    printf("\n=== 7. DllCanUnloadNow (after factory interactions) ===\n");

    DllCanUnloadNowFunc fn = (DllCanUnloadNowFunc)GetProcAddress(g_hDll, "DllCanUnloadNow");
    if (fn) {
        __try {
            HRESULT hr = fn();
            LogHr("DllCanUnloadNow", hr);
            if (hr == S_OK)
                printf("    [+] No outstanding references — safe to unload\n");
            else if (hr == S_FALSE)
                printf("    [-] Outstanding references exist\n");
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("    => SEH exception 0x%08X\n", GetExceptionCode());
        }
    }
}

// ============================================================
// Main
// ============================================================
int main(int argc, char** argv) {
    printf("=== WLXImageTranscode.dll Dedicated Test Harness ===\n");
    printf("Build: %s %s\n\n", __DATE__, __TIME__);

    SetSearchPaths();

    HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    printf("[*] CoInitializeEx: 0x%08X\n\n", hrCoInit);

    if (!LoadTargetDll()) {
        printf("[!] Cannot load DLL — aborting\n");
        if (SUCCEEDED(hrCoInit)) CoUninitialize();
        return 1;
    }

    Test_StdExports();
    Test_ClassFactories();
    Test_CreateInstance();
    Test_ProxyStub();
    Test_OprcProxy();
    Test_OrpcSection();
    Test_UnloadState();

    printf("\n[*] Unloading WLXImageTranscode.dll\n");
    FreeLibrary(g_hDll);
    DllCanUnloadNowFunc fn = (DllCanUnloadNowFunc)GetProcAddress(g_hDll, "DllCanUnloadNow");
    if (fn) printf("[*] Post-unload DllCanUnloadNow call would be invalid — skipped\n");

    if (SUCCEEDED(hrCoInit)) CoUninitialize();

    printf("\n=== Done ===\n");
    return 0;
}
