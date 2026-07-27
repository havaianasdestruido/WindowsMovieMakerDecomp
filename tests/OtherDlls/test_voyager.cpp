// OtherDlls Test Harness - WLXPhotoVoyager.dll
// Loads the DLL, probes all 4 exports with SEH protection, attempts CoCreateInstance for each theme CLSID
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#include <unknwn.h>
#include <cstdio>
#include <cstdlib>

// ============================================================
// GUIDs
// ============================================================
static const CLSID CLSID_Empty = {0,0,0,{0,0,0,0,0,0,0,0}};
static const IID IID_IUnknown_Cast = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

// Voyager Theme CLSIDs
static const CLSID CLSID_VoyagerThemeAlbum     = {0xC84CFE1B,0x89DC,0x40E7,{0x83,0xBF,0xCB,0x82,0x12,0x55,0xF9,0xEC}};
static const CLSID CLSID_VoyagerThemeCollage   = {0xAEE6C573,0xA192,0x4AF3,{0xB6,0x2B,0xA4,0xE6,0x84,0x85,0x33,0xD3}};
static const CLSID CLSID_VoyagerThemeFlip      = {0x653E52D8,0xD033,0x469A,{0x8B,0xB5,0x9C,0x1A,0x16,0x44,0x16,0xD5}};
static const CLSID CLSID_VoyagerThemeFrame     = {0xB4E10BE6,0xA2CE,0x4BEF,{0x9D,0x80,0x99,0x99,0x5C,0xB3,0xC1,0x62}};
static const CLSID CLSID_VoyagerThemeGlass     = {0x5515D2B5,0x6825,0x409E,{0xB3,0x77,0x54,0x47,0x08,0xC9,0xDD,0x06}};
static const CLSID CLSID_VoyagerThemeSnapshots = {0xD5561752,0xE5A7,0x46E7,{0xB7,0x68,0xD9,0x45,0xE1,0x44,0xCA,0x78}};
static const CLSID CLSID_VoyagerThemeTravel    = {0xCC4F1166,0xCE12,0x41F7,{0x85,0xE2,0xAE,0x47,0x44,0xD9,0x38,0x1B}};
static const CLSID CLSID_VoyagerTransform      = {0xE48325CB,0x1EFC,0x425E,{0x9C,0xD9,0x47,0xEF,0x51,0xBE,0xCD,0x55}};

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
// Export typedefs
// ============================================================
typedef HRESULT (__stdcall *DllCanUnloadNowFunc)();
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServerFunc)();
typedef HRESULT (__stdcall *DllUnregisterServerFunc)();

static void TestExports(HMODULE h) {
    printf("\n========================================\n");
    printf("  WLXPhotoVoyager.dll Export Probing\n");
    printf("========================================\n");

    // DllCanUnloadNow
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

    // DllGetClassObject
    {
        printf("\n[*] DllGetClassObject (ordinal 2, hint 1)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, NULL);
                printf("    => HRESULT=0x%08X (null CLSID args)\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllRegisterServer
    {
        printf("\n[*] DllRegisterServer (ordinal 3, hint 2)\n");
        DllRegisterServerFunc fn = (DllRegisterServerFunc)GetProcAddress(h, "DllRegisterServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            printf("    [skip] Would write to registry\n");
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllUnregisterServer
    {
        printf("\n[*] DllUnregisterServer (ordinal 4, hint 3)\n");
        DllUnregisterServerFunc fn = (DllUnregisterServerFunc)GetProcAddress(h, "DllUnregisterServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            printf("    [skip] Would write to registry\n");
        } else {
            printf("    [!] not found\n");
        }
    }
}

// ============================================================
// COM Probing - CoCreateInstance for each theme CLSID
// ============================================================
struct ThemeProbe {
    const wchar_t* name;
    const CLSID* clsid;
};

static void TestComCreate() {
    printf("\n========================================\n");
    printf("  COM CoCreateInstance Probing\n");
    printf("========================================\n");

    const ThemeProbe themes[] = {
        {L"VoyagerThemeAlbum",     &CLSID_VoyagerThemeAlbum},
        {L"VoyagerThemeCollage",   &CLSID_VoyagerThemeCollage},
        {L"VoyagerThemeFlip",      &CLSID_VoyagerThemeFlip},
        {L"VoyagerThemeFrame",     &CLSID_VoyagerThemeFrame},
        {L"VoyagerThemeGlass",     &CLSID_VoyagerThemeGlass},
        {L"VoyagerThemeSnapshots", &CLSID_VoyagerThemeSnapshots},
        {L"VoyagerThemeTravel",    &CLSID_VoyagerThemeTravel},
        {L"VoyagerTransform",      &CLSID_VoyagerTransform},
    };

    for (int i = 0; i < sizeof(themes)/sizeof(themes[0]); i++) {
        printf("\n[*] CoCreateInstance(%ls)\n", themes[i].name);
        printf("    CLSID = {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
            themes[i].clsid->Data1, themes[i].clsid->Data2, themes[i].clsid->Data3,
            themes[i].clsid->Data4[0], themes[i].clsid->Data4[1],
            themes[i].clsid->Data4[2], themes[i].clsid->Data4[3],
            themes[i].clsid->Data4[4], themes[i].clsid->Data4[5],
            themes[i].clsid->Data4[6], themes[i].clsid->Data4[7]);

        IUnknown* punk = NULL;
        __try {
            HRESULT hr = CoCreateInstance(*themes[i].clsid, NULL,
                CLSCTX_INPROC_SERVER, IID_IUnknown_Cast, (void**)&punk);
            printf("    => HRESULT=0x%08X", hr);
            if (SUCCEEDED(hr) && punk) {
                printf(", IUnknown=0x%p", punk);
                // QueryInterface to ISupportErrorInfo or check ref count
                ULONG refs = punk->Release();
                printf(", refs after Release=%u\n", refs);
            } else {
                printf("\n");
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            if (punk) punk->Release();
        }
    }
}

// ============================================================
// Main
// ============================================================
int main(int argc, char** argv) {
    printf("=== WLXPhotoVoyager Test Harness ===\n");
    printf("Target: WLXPhotoVoyager.dll (COM slideshow theme server)\n\n");

    SetSearchPaths();

    // Initialize COM (Apartment threaded, as registered)
    HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    printf("[*] CoInitializeEx => 0x%08X\n\n", hrInit);

    // Load the DLL
    HMODULE hVoyager = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXPhotoVoyager.dll");
    if (hVoyager) {
        TestExports(hVoyager);
        TestComCreate();

        printf("\n[*] Unloading WLXPhotoVoyager.dll\n");
        FreeLibrary(hVoyager);
    }

    CoUninitialize();

    printf("\n=== Done ===\n");
    return 0;
}
