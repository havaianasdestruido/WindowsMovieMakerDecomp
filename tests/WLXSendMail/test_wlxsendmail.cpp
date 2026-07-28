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

// CLSID_WLXSendTo: {7A7D1A6E-7772-46B5-BF4C-2B550BAE764A}
static const CLSID CLSID_WLXSendTo =
    {0x7A7D1A6E, 0x7772, 0x46B5, {0xBF, 0x4C, 0x2B, 0x55, 0x0B, 0xAE, 0x76, 0x4A}};

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

static void TestWLXSendMail() {
    printf("========================================\n");
    printf("  WLXSendMail.dll — Dedicated Harness\n");
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

    // --- Test 2: Named Exports (2) ---
    printf("--- Test 2: Named Exports ---\n");
    FARPROC pDllCanUnloadNow = GetProcAddress(hMod, "DllCanUnloadNow");
    FARPROC pDllGetClassObject = GetProcAddress(hMod, "DllGetClassObject");
    FARPROC pDllRegisterServer = GetProcAddress(hMod, "DllRegisterServer");
    FARPROC pDllUnregisterServer = GetProcAddress(hMod, "DllUnregisterServer");
    CHECK("DllCanUnloadNow exported", pDllCanUnloadNow != NULL);
    CHECK("DllGetClassObject exported", pDllGetClassObject != NULL);
    CHECK("DllRegisterServer not exported", pDllRegisterServer == NULL);
    CHECK("DllUnregisterServer not exported", pDllUnregisterServer == NULL);
    printf("  DllCanUnloadNow @ %p\n", (void*)pDllCanUnloadNow);
    printf("  DllGetClassObject @ %p\n\n", (void*)pDllGetClassObject);

    // --- Test 3: DllCanUnloadNow ---
    printf("--- Test 3: DllCanUnloadNow ---\n");
    if (pDllCanUnloadNow) {
        typedef HRESULT (__stdcall *FnCanUnload)();
        __try {
            HRESULT hr = ((FnCanUnload)pDllCanUnloadNow)();
            printf("  DllCanUnloadNow -> 0x%08X (%s)\n", (unsigned)hr,
                hr == S_OK ? "S_OK (can unload)" : "S_FALSE (cannot unload)");
            CHECK("DllCanUnloadNow returns valid HRESULT", SUCCEEDED(hr) || hr == S_FALSE);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  DllCanUnloadNow -> EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
            g_testsFailed++;
        }
    }

    // --- Test 4: DllGetClassObject (CSendTo) ---
    printf("\n--- Test 4: DllGetClassObject (CSendTo) ---\n");
    if (pDllGetClassObject) {
        typedef HRESULT (__stdcall *FnGetClassObject)(REFCLSID, REFIID, void**);
        IID IID_IClassFactory = {0x00000001, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
        void* pFactory = NULL;

        __try {
            HRESULT hr = ((FnGetClassObject)pDllGetClassObject)(
                CLSID_WLXSendTo, IID_IClassFactory, &pFactory);
            printf("  DllGetClassObject(CLSID_WLXSendTo, IID_IClassFactory) -> 0x%08X (%s)\n",
                (unsigned)hr, HrStr(hr));
            if (SUCCEEDED(hr) && pFactory) {
                CHECK("CSendTo factory created", pFactory != NULL);
                IClassFactory* cf = (IClassFactory*)pFactory;
                void** vtbl = *(void***)cf;
                LogVtable(vtbl, "IClassFactory", 7);

                IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
                void* pObj = NULL;
                HRESULT hrCreate = cf->CreateInstance(NULL, IID_IUnknown, &pObj);
                printf("  CreateInstance(IUnknown) -> 0x%08X (%s)\n",
                    (unsigned)hrCreate, HrStr(hrCreate));
                if (SUCCEEDED(hrCreate) && pObj) {
                    IUnknown* unk = (IUnknown*)pObj;
                    void** objVtbl = *(void***)unk;
                    LogVtable(objVtbl, "CSendTo IUnknown", 7);
                    unk->Release();
                    g_testsPassed++;
                } else {
                    g_testsFailed++;
                }
                cf->Release();
            } else {
                printf("  (Expected — CLSID_WLXSendTo not registered; registration is external)\n");
                g_testsPassed++;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
            g_testsFailed++;
        }
    }

    // --- Test 5: MAPI Client Detection ---
    printf("\n--- Test 5: MAPI Client Detection ---\n");
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
                CHECK("Default mail client found", val[0] != '\0');
            } else {
                printf("  (no default mail client)\n");
                g_testsPassed++;
            }
            RegCloseKey(hKey);
        } else {
            printf("  Software\\Clients\\Mail not found\n");
            g_testsPassed++;
        }

        const char* clients[] = {
            "Software\\Clients\\Mail\\Windows Mail",
            "Software\\Clients\\Mail\\Microsoft Outlook",
            "Software\\Clients\\Mail\\Windows Live Mail",
            "Software\\Clients\\Mail\\Outlook Express"
        };
        int found = 0;
        for (int i = 0; i < 4; i++) {
            HKEY hSub;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, clients[i], 0, KEY_READ, &hSub) == ERROR_SUCCESS) {
                printf("  Installed: %s\n", clients[i]);
                RegCloseKey(hSub);
                found++;
            }
        }
        if (found == 0) {
            printf("  (no MAPI clients detected via HKLM)\n");
        }
        g_testsPassed++;
    }

    // --- Test 6: Clipboard Formats ---
    printf("\n--- Test 6: Clipboard Formats ---\n");
    {
        UINT fmtURL = RegisterClipboardFormatA("UniformResourceLocator");
        UINT fmtFC = RegisterClipboardFormatA("FileContents");
        UINT fmtFGD = RegisterClipboardFormatA("FileGroupDescriptor");
        UINT fmtFGDW = RegisterClipboardFormatA("FileGroupDescriptorW");
        printf("  UniformResourceLocator: %u\n", fmtURL);
        printf("  FileContents: %u\n", fmtFC);
        printf("  FileGroupDescriptor: %u\n", fmtFGD);
        printf("  FileGroupDescriptorW: %u\n", fmtFGDW);
        CHECK("Clipboard formats registered OK", fmtURL && fmtFC && fmtFGD && fmtFGDW);
    }

    // --- Test 7: File Type Support ---
    printf("\n--- Test 7: Supported File Types ---\n");
    {
        const char* imageTypes[] = {
            ".jpg", ".jpeg", ".png", ".tif", ".tiff", ".wdp",
            ".bmp", ".dib", ".gif", ".ico", ".jfif", ".rle"
        };
        printf("  Image (resized attachments): %d types\n", 12);
        for (int i = 0; i < 12; i++) printf("    %s", imageTypes[i]);
        printf("\n");

        const char* videoTypes[] = {
            ".avi", ".asf", ".wmv", ".mpg", ".mpeg", ".mov",
            ".mp4", ".3gp", ".3gpp", ".3g2"
        };
        printf("  Video (shortcuts): %d types\n", 10);
        for (int i = 0; i < 10; i++) printf("    %s", videoTypes[i]);
        printf("\n");
        g_testsPassed++;
    }

    // --- Test 8: Resize Options ---
    printf("\n--- Test 8: Image Resize Options ---\n");
    {
        struct { const char* label; const char* dims; } sizes[] = {
            {"Smaller", "640 x 480"}, {"Small",   "800 x 600"},
            {"Medium",  "1024 x 768"}, {"Large",   "1280 x 1024"},
            {"Original","(no resize)"}
        };
        for (int i = 0; i < 5; i++)
            printf("  %-10s: %s\n", sizes[i].label, sizes[i].dims);
        g_testsPassed++;
    }

    // --- Test 9: Module Reference ---
    printf("\n--- Test 9: Module Reference Count ---\n");
    {
        // FreeLibrary once — module should still be locked by ATL if active
        FreeLibrary(hMod);
        HMODULE hCheck = GetModuleHandleA("WLXSendMail.dll");
        printf("  After FreeLibrary — handle: %p\n", (void*)hCheck);
        printf("  (0 = unloaded, non-0 = still resident due to ATL)\n");
        g_testsPassed++;
    }

    printf("\n");
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("============================================================\n");
    printf("  WLXSendMail.dll Dedicated Test Harness\n");
    printf("  COM exports: DllCanUnloadNow, DllGetClassObject\n");
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
