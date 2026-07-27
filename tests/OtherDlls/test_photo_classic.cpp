// WLXPhotoClassic.dll Test Harness
// Standard COM DLL with 4 exports (DllCanUnloadNow, DllGetClassObject, DllRegisterServer, DllUnregisterServer)
// Also dynamically imports GetTFXCreateFunctions from WLXPipeTran.dll and GetPipelineCreateFunctions from WLXPipeline.dll
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unknwn.h>
#include <cstdio>
#include <cstdlib>

// ============================================================
// GUIDs
// ============================================================
static const CLSID CLSID_Empty = {0,0,0,{0,0,0,0,0,0,0,0}};
static const IID IID_IUnknown_Cast = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

// WLXPhotoClassic CLSID candidates extracted from binary
// {773AFF18-2083-47C1-9EA9-A5DA346A0122}
// {854E43AC-E1FD-46f2-8DD3-EE4C7A1844B6}
// {B9087BDF-F0F8-4454-A7D1-F6242E1654F8}
// {F91A0A3F-3E4E-4273-88CC-6664834ACA6F}
// {71ED30A7-499A-4F61-84F8-10CDEC657FE0}

static const GUID CLSID_PhotoClassic_Candidate1 =
    {0x773AFF18,0x2083,0x47C1,{0x9E,0xA9,0xA5,0xDA,0x34,0x6A,0x01,0x22}};
static const GUID CLSID_PhotoClassic_Candidate2 =
    {0x854E43AC,0xE1FD,0x46f2,{0x8D,0xD3,0xEE,0x4C,0x7A,0x18,0x44,0xB6}};
static const GUID CLSID_PhotoClassic_Candidate3 =
    {0xB9087BDF,0xF0F8,0x4454,{0xA7,0xD1,0xF6,0x24,0x2E,0x16,0x54,0xF8}};
static const GUID CLSID_PhotoClassic_Candidate4 =
    {0xF91A0A3F,0x3E4E,0x4273,{0x88,0xCC,0x66,0x64,0x83,0x4A,0xCA,0x6F}};
static const GUID CLSID_PhotoClassic_Candidate5 =
    {0x71ED30A7,0x499A,0x4F61,{0x84,0xF8,0x10,0xCD,0xEC,0x65,0x7F,0xE0}};

// ============================================================
// Helpers
// ============================================================
static void SetSearchPaths() {
    wchar_t galleryDir[MAX_PATH], sharedDir[MAX_PATH], mainDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
    GetFullPathNameW(L"undecomp\\Shared", MAX_PATH, sharedDir, NULL);
    GetFullPathNameW(L".", MAX_PATH, mainDir, NULL);
    AddDllDirectory(galleryDir);
    AddDllDirectory(sharedDir);
    AddDllDirectory(mainDir);
    SetDllDirectoryW(galleryDir);
}

static HMODULE SafeLoadDll(const wchar_t* relPath) {
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(relPath, MAX_PATH, fullPath, NULL);
    printf("[*] Loading: %ls\n", fullPath);
    HMODULE h = LoadLibraryExW(fullPath, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!h) {
        printf("[!] LoadLibraryEx failed (err %u), retrying SetDllDirectory\n", GetLastError());
        wchar_t galleryDir[MAX_PATH];
        GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
        SetDllDirectoryW(galleryDir);
        h = LoadLibraryW(fullPath);
    }
    if (h)
        printf("[+] Loaded at base: 0x%p\n", h);
    else
        printf("[!] FAILED to load: error %u\n", GetLastError());
    return h;
}

// ============================================================
// Export typedefs (4 standard COM DLL exports)
// ============================================================
typedef HRESULT (__stdcall *DllCanUnloadNowFunc)();
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServerFunc)();
typedef HRESULT (__stdcall *DllUnregisterServerFunc)();

// ============================================================
// WLXPhotoClassic.dll Export Probing
// ============================================================
static void TestWLXPhotoClassic(HMODULE h) {
    printf("\n========================================\n");
    printf("  WLXPhotoClassic.dll Export Probing\n");
    printf("========================================\n");

    // DllCanUnloadNow (ordinal 1, hint 0)
    {
        printf("\n[*] DllCanUnloadNow (ordinal 1, hint 0)\n");
        DllCanUnloadNowFunc fn = (DllCanUnloadNowFunc)GetProcAddress(h, "DllCanUnloadNow");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn();
                printf("    => HRESULT=0x%08X\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllGetClassObject (ordinal 2, hint 1)
    {
        printf("\n[*] DllGetClassObject (ordinal 2, hint 1)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            printf("    ptr=0x%p\n", fn);

            // Test with NULL CLSID and NULL IID
            __try {
                HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, NULL);
                printf("    => HRESULT=0x%08X (null CLSID)\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }

            // Test each CLSID candidate
            const GUID* clsids[] = {
                &CLSID_PhotoClassic_Candidate1,
                &CLSID_PhotoClassic_Candidate2,
                &CLSID_PhotoClassic_Candidate3,
                &CLSID_PhotoClassic_Candidate4,
                &CLSID_PhotoClassic_Candidate5,
            };
            const char* names[] = {
                "Candidate1 {773AFF18}",
                "Candidate2 {854E43AC}",
                "Candidate3 {B9087BDF}",
                "Candidate4 {F91A0A3F}",
                "Candidate5 {71ED30A7}",
            };
            for (int i = 0; i < 5; i++) {
                printf("\n    [*] DllGetClassObject with %s\n", names[i]);
                void* pUnk = NULL;
                __try {
                    HRESULT hr = fn(*clsids[i], IID_IUnknown_Cast, &pUnk);
                    printf("        => HRESULT=0x%08X", hr);
                    if (SUCCEEDED(hr) && pUnk) {
                        printf(", IUnknown=0x%p", pUnk);
                        IUnknown* punk = (IUnknown*)pUnk;
                        punk->Release();
                        printf(" [RELEASED]");
                    }
                    printf("\n");
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("        => SEH exception 0x%08X\n", GetExceptionCode());
                }
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllRegisterServer (ordinal 3, hint 2)
    {
        printf("\n[*] DllRegisterServer (ordinal 3, hint 2)\n");
        DllRegisterServerFunc fn = (DllRegisterServerFunc)GetProcAddress(h, "DllRegisterServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            printf("    [skip] Would write to registry - not called\n");
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllUnregisterServer (ordinal 4, hint 3)
    {
        printf("\n[*] DllUnregisterServer (ordinal 4, hint 3)\n");
        DllUnregisterServerFunc fn = (DllUnregisterServerFunc)GetProcAddress(h, "DllUnregisterServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            printf("    [skip] Would write to registry - not called\n");
        } else {
            printf("    [!] not found\n");
        }
    }
}

// ============================================================
// Dynamic probing of WLXPhotoBase.dll imports
// ============================================================
static void ProbeWLXPhotoBaseImports() {
    printf("\n========================================\n");
    printf("  WLXPhotoBase.dll Import Probing\n");
    printf("========================================\n");

    HMODULE hBase = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXPhotoBase.dll");
    if (!hBase) {
        printf("[!] Could not load WLXPhotoBase.dll\n");
        return;
    }

    const char* baseExports[] = {
        "?Throw@Base@@YGXJ@Z",
        "?BaseAtlThrow@ATL@@YGXJ@Z",
        "??1Exception@Base@@UAE@XZ",
        "?New@BasePrivate@@YAPAXI_N@Z",
        "?GetBaseStringManager@String@Base@@SGAAVCAtlStringMgr@ATL@@XZ",
        "?Delete@BasePrivate@@YAXPAX@Z",
    };

    for (int i = 0; i < 6; i++) {
        printf("\n[*] %s\n", baseExports[i]);
        FARPROC fp = GetProcAddress(hBase, baseExports[i]);
        if (fp) {
            printf("    ptr=0x%p\n", fp);
        } else {
            printf("    [!] not found\n");
        }
    }

    FreeLibrary(hBase);
}

// ============================================================
// Dynamic probing of WLXPipeTran.dll and WLXPipeline.dll imports
// ============================================================
static void ProbePipelineImports() {
    printf("\n========================================\n");
    printf("  WLXPipeTran / WLXPipeline Probing\n");
    printf("========================================\n");

    // WLXPipeTran.dll
    {
        HMODULE hPipeTran = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXPipeTran.dll");
        if (hPipeTran) {
            printf("\n[*] GetTFXCreateFunctions from WLXPipeTran.dll\n");
            FARPROC fp = GetProcAddress(hPipeTran, "GetTFXCreateFunctions");
            if (fp) {
                printf("    ptr=0x%p\n", fp);
                __try {
                    void* result = ((void*(*)())fp)();
                    printf("    => returns ptr=0x%p\n", result);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                }
            } else {
                printf("    [!] not found\n");
            }
            FreeLibrary(hPipeTran);
        } else {
            printf("[!] Could not load WLXPipeTran.dll\n");
        }
    }

    // WLXPipeline.dll
    {
        HMODULE hPipeline = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXPipeline.dll");
        if (hPipeline) {
            printf("\n[*] GetPipelineCreateFunctions from WLXPipeline.dll\n");
            FARPROC fp = GetProcAddress(hPipeline, "GetPipelineCreateFunctions");
            if (fp) {
                printf("    ptr=0x%p\n", fp);
                __try {
                    void* result = ((void*(*)())fp)();
                    printf("    => returns ptr=0x%p\n", result);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                }
            } else {
                printf("    [!] not found\n");
            }
            FreeLibrary(hPipeline);
        } else {
            printf("[!] Could not load WLXPipeline.dll\n");
        }
    }
}

// ============================================================
// Main
// ============================================================
int main(int argc, char** argv) {
    printf("=== WLXPhotoClassic.dll Test Harness ===\n");
    printf("Target: Classic photo viewer COM DLL (slideshow themes)\n\n");

    SetSearchPaths();

    // Probe WLXPhotoBase.dll imports first (dependency)
    ProbeWLXPhotoBaseImports();

    // Probe pipeline DLLs (dynamic imports referenced in string data)
    ProbePipelineImports();

    // Load and test the main target
    HMODULE hClassic = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXPhotoClassic.dll");
    if (hClassic) {
        TestWLXPhotoClassic(hClassic);

        // Module info
        printf("\n========================================\n");
        printf("  Module Information\n");
        printf("========================================\n");
        wchar_t modPath[MAX_PATH];
        GetModuleFileNameW(hClassic, modPath, MAX_PATH);
        printf("[*] Full path: %ls\n", modPath);
        DWORD fileSize = GetFileSize(modPath, NULL);
        printf("[*] File size: %u bytes (0x%X)\n", fileSize, fileSize);

        printf("\n[*] Unloading WLXPhotoClassic.dll\n");
        FreeLibrary(hClassic);
    }

    printf("\n=== Done ===\n");
    return 0;
}
