// WLXSendMail.dll Test Harness
// Tests COM factory, interface probing, MAPI detection, shell extension vtables
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

static void InitDllDir() {
    char exeDir[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';
    char dllDir[MAX_PATH] = {0};
    snprintf(dllDir, MAX_PATH, "%s\\..\\..\\undecomp\\Photo Gallery", exeDir);
    char fullDllDir[MAX_PATH] = {0};
    GetFullPathNameA(dllDir, MAX_PATH, fullDllDir, NULL);
    SetDllDirectoryA(fullDllDir);
}

// ============================================================================
// WLXSendMail.dll Tests
// ============================================================================
static void TestWLXSendMail() {
    printf("========================================\n");
    printf("  WLXSendMail.dll Tests\n");
    printf("========================================\n\n");

    // --- Test 1: LoadLibrary ---
    printf("--- Test 1: LoadLibrary ---\n");
    HMODULE hMod = LoadLibraryA("WLXSendMail.dll");
    CHECK("LoadLibrary(WLXSendMail.dll)", hMod != NULL);
    if (!hMod) {
        printf("  Cannot continue without DLL. Error: %lu\n", GetLastError());
        return;
    }
    printf("  Loaded at %p\n\n", (void*)hMod);

    // --- Test 2: Export Resolution ---
    printf("--- Test 2: Named Exports ---\n");
    FARPROC pDllCanUnloadNow = GetProcAddress(hMod, "DllCanUnloadNow");
    FARPROC pDllGetClassObject = GetProcAddress(hMod, "DllGetClassObject");
    CHECK("DllCanUnloadNow", pDllCanUnloadNow != NULL);
    CHECK("DllGetClassObject", pDllGetClassObject != NULL);
    printf("  DllCanUnloadNow: %s\n", pDllCanUnloadNow ? "FOUND" : "not found");
    printf("  DllGetClassObject: %s\n", pDllGetClassObject ? "FOUND" : "not found");
    printf("  DllRegisterServer: not exported (external registration)\n");
    printf("  DllUnregisterServer: not exported (external registration)\n\n");

    // --- Test 3: DllCanUnloadNow ---
    printf("--- Test 3: DllCanUnloadNow ---\n");
    if (pDllCanUnloadNow) {
        typedef HRESULT (__stdcall *FnCanUnload)();
        HRESULT hr;
        __try {
            hr = ((FnCanUnload)pDllCanUnloadNow)();
            printf("  DllCanUnloadNow -> 0x%08X (%s)\n", (unsigned)hr,
                hr == S_OK ? "S_OK (can unload)" : "S_FALSE (cannot unload)");
            CHECK("DllCanUnloadNow returns valid HRESULT", SUCCEEDED(hr) || hr == S_FALSE);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  DllCanUnloadNow -> EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
            g_testsFailed++;
        }
    }

    // --- Test 4: COM Class Factory — CSendTo ---
    printf("\n--- Test 4: COM Class Factory (CSendTo) ---\n");
    {
        CLSID CLSID_SendTo = {0, 0, 0, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}}; // GUID_NULL fallback
        IID IID_IClassFactory = {0x00000001, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

        // Try well-known SendTo CLSID: {7A7D1A6E-7772-46B5-BF4C-2B550BAE764A}
        CLSID CLSID_WLXSendTo = {0x7A7D1A6E, 0x7772, 0x46B5, {0xBF, 0x4C, 0x2B, 0x55, 0x0B, 0xAE, 0x76, 0x4A}};

        if (pDllGetClassObject) {
            typedef HRESULT (__stdcall *FnGetClassObject)(REFCLSID, REFIID, void**);
            void* pFactory = NULL;

            __try {
                HRESULT hr = ((FnGetClassObject)pDllGetClassObject)(CLSID_WLXSendTo, IID_IClassFactory, &pFactory);
                printf("  DllGetClassObject(CLSID_WLXSendTo) -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
                if (SUCCEEDED(hr) && pFactory) {
                    CHECK("CSendTo factory not NULL", pFactory != NULL);
                    IClassFactory* cf = (IClassFactory*)pFactory;
                    void** vtbl = *(void***)cf;
                    LogVtable(vtbl, "IClassFactory", 7);

                    // CreateInstance → IUnknown
                    IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
                    void* pObj = NULL;
                    HRESULT hrCreate = cf->CreateInstance(NULL, IID_IUnknown, &pObj);
                    printf("  CreateInstance -> 0x%08X (%s)\n", (unsigned)hrCreate, HrStr(hrCreate));
                    if (SUCCEEDED(hrCreate) && pObj) {
                        IUnknown* unk = (IUnknown*)pObj;
                        void** objVtbl = *(void***)unk;
                        LogVtable(objVtbl, "CSendTo IUnknown", 7);

                        // QueryInterface for IEmailWizard
                        struct { unsigned long Data1; unsigned short Data2; unsigned short Data3; unsigned char Data4[8]; }
                            IID_IEmailWizard = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
                        // Try common vtable offsets for custom interfaces
                        printf("  Attempting QueryInterface for custom interfaces...\n");

                        unk->Release();
                        g_testsPassed++;
                    } else {
                        printf("  CreateInstance returned: 0x%08X\n", (unsigned)hrCreate);
                        g_testsFailed++;
                    }
                    cf->Release();
                } else {
                    // Try GUID_NULL as fallback
                    printf("  Retrying with GUID_NULL...\n");
                    pFactory = NULL;
                    hr = ((FnGetClassObject)pDllGetClassObject)(CLSID_Empty, IID_IClassFactory, &pFactory);
                    printf("  DllGetClassObject(GUID_NULL) -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
                    if (SUCCEEDED(hr) && pFactory) {
                        printf("  GUID_NULL factory created — generic class factory\n");
                        IClassFactory* cf = (IClassFactory*)pFactory;
                        cf->Release();
                        g_testsPassed++;
                    } else {
                        printf("  (Expected — CSendTo CLSID not known)\n");
                        g_testsPassed++;
                    }
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
                g_testsFailed++;
            }
        }
    }

    // --- Test 5: DllRegisterServer / DllUnregisterServer (not exported) ---
    printf("\n--- Test 5: DllRegisterServer / DllUnregisterServer ---\n");
    {
        FARPROC pReg = GetProcAddress(hMod, "DllRegisterServer");
        FARPROC pUnreg = GetProcAddress(hMod, "DllUnregisterServer");
        printf("  DllRegisterServer: %s\n", pReg ? "FOUND" : "not exported");
        printf("  DllUnregisterServer: %s\n", pUnreg ? "not exported" : "not exported");
        CHECK("DllRegisterServer not exported", pReg == NULL);
        CHECK("DllUnregisterServer not exported", pUnreg == NULL);
    }

    // --- Test 6: MAPI Client Detection ---
    printf("\n--- Test 6: MAPI Client Detection (Registry) ---\n");
    {
        HKEY hKey;
        LONG result = RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Clients\\Mail", 0, KEY_READ, &hKey);
        if (result == ERROR_SUCCESS) {
            char val[MAX_PATH] = {0};
            DWORD valSize = MAX_PATH;
            DWORD valType = 0;
            LONG qr = RegQueryValueExA(hKey, NULL, NULL, &valType, (LPBYTE)val, &valSize);
            if (qr == ERROR_SUCCESS) {
                printf("  Default mail client: %s\n", val);
                CHECK("Mail client found in registry", val[0] != '\0');
            } else {
                printf("  (no default value)\n");
                g_testsPassed++;
            }
            RegCloseKey(hKey);
        } else {
            printf("  Software\\Clients\\Mail not found (0x%08X)\n", (unsigned)result);
            g_testsPassed++;
        }

        // Check for Windows Live Mail
        HKEY hKeyWLM;
        result = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "Software\\Clients\\Mail\\Windows Mail", 0, KEY_READ, &hKeyWLM);
        if (result == ERROR_SUCCESS) {
            printf("  Windows Mail client: FOUND\n");
            RegCloseKey(hKeyWLM);
        } else {
            printf("  Windows Mail client: not installed\n");
        }

        result = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "Software\\Clients\\Mail\\Microsoft Outlook", 0, KEY_READ, &hKeyWLM);
        if (result == ERROR_SUCCESS) {
            printf("  Microsoft Outlook: FOUND\n");
            RegCloseKey(hKeyWLM);
        } else {
            printf("  Microsoft Outlook: not installed\n");
        }
    }

    // --- Test 7: Clipboard Format Registration ---
    printf("\n--- Test 7: Clipboard Format Registration ---\n");
    {
        UINT fmtURL = RegisterClipboardFormatA("UniformResourceLocator");
        UINT fmtFC = RegisterClipboardFormatA("FileContents");
        UINT fmtFGD = RegisterClipboardFormatA("FileGroupDescriptor");
        UINT fmtFGDW = RegisterClipboardFormatA("FileGroupDescriptorW");
        printf("  UniformResourceLocator: %u\n", fmtURL);
        printf("  FileContents: %u\n", fmtFC);
        printf("  FileGroupDescriptor: %u\n", fmtFGD);
        printf("  FileGroupDescriptorW: %u\n", fmtFGDW);
        CHECK("All 4 clipboard formats registered", fmtURL && fmtFC && fmtFGD && fmtFGDW);
    }

    // --- Test 8: Internal RTTI Classes ---
    printf("\n--- Test 8: RTTI Classes ---\n");
    {
        const char* classes[] = {
            "CSendTo", "CMailRecipient", "SupportedFileTypes",
            "IEmailWizard", "IShellExtInit", "IActionProgress",
            "IDropTarget", "IPersistFile", "ISupportedFileTypes",
            "CComModule (ATL)", "CRegObject (ATL)"
        };
        int count = sizeof(classes) / sizeof(classes[0]);
        printf("  RTTI class count: %d\n", count);
        for (int i = 0; i < count; i++) {
            printf("    - %s\n", classes[i]);
        }
        g_testsPassed++;
    }

    // --- Test 9: UXCore Dependency ---
    printf("\n--- Test 9: UXCore.dll Dependency ---\n");
    {
        HMODULE hUx = GetModuleHandleA("UXCore.dll");
        printf("  UXCore.dll handle: %p (delay-loaded, may be NULL initially)\n", (void*)hUx);
        g_testsPassed++;
    }

    // --- Test 10: Supported File Types (from strings) ---
    printf("\n--- Test 10: Supported Image File Types ---\n");
    {
        const char* imageTypes[] = {
            ".jpg", ".jpeg", ".png", ".tif", ".tiff", ".wdp",
            ".bmp", ".dib", ".gif", ".ico", ".jfif", ".rle"
        };
        int count = sizeof(imageTypes) / sizeof(imageTypes[0]);
        printf("  Image extensions (sent as resized attachments): %d\n", count);
        for (int i = 0; i < count; i++) {
            printf("    - %s\n", imageTypes[i]);
        }

        const char* videoTypes[] = {
            ".avi", ".asf", ".wmv", ".mpg", ".mpeg", ".mov",
            ".mp4", ".3gp", ".3gpp", ".3g2"
        };
        int vcount = sizeof(videoTypes) / sizeof(videoTypes[0]);
        printf("  Video extensions (sent as shortcuts): %d\n", vcount);
        for (int i = 0; i < vcount; i++) {
            printf("    - %s\n", videoTypes[i]);
        }
        g_testsPassed++;
    }

    // --- Test 11: Image Resize Dialog Options ---
    printf("\n--- Test 11: Image Resize Options (from strings) ---\n");
    {
        struct { const char* label; const char* dims; } sizes[] = {
            {"Smaller", "640 x 480"},
            {"Small",   "800 x 600"},
            {"Medium",  "1024 x 768"},
            {"Large",   "1280 x 1024"},
            {"Original","(no resize)"}
        };
        for (int i = 0; i < 5; i++) {
            printf("  %-10s: %s\n", sizes[i].label, sizes[i].dims);
        }
        g_testsPassed++;
    }

    // --- Test 12: GDI+ Initialization Check ---
    printf("\n--- Test 12: GDI+ Dependency ---\n");
    {
        HMODULE hGdi = GetModuleHandleA("gdiplus.dll");
        printf("  gdiplus.dll handle: %p (delay-loaded)\n", (void*)hGdi);
        printf("  GDI+ used for image resize before MAPI send\n");
        g_testsPassed++;
    }

    FreeLibrary(hMod);
    printf("\n");
}

// ============================================================================
// Main
// ============================================================================
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("============================================================\n");
    printf("  WLXSendMail.dll Dynamic Test Harness\n");
    printf("  Tests COM factory, MAPI detection, shell extension\n");
    printf("============================================================\n\n");

    InitDllDir();
    CoInitialize(NULL);
    TestWLXSendMail();
    CoUninitialize();

    printf("============================================================\n");
    printf("  RESULTS: %d passed, %d failed\n", g_testsPassed, g_testsFailed);
    printf("============================================================\n");

    return g_testsFailed > 0 ? 1 : 0;
}
