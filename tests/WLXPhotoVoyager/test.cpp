#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#include <unknwn.h>
#include <strsafe.h>
#include <cstdio>
#include <cstdlib>

// IClassFactory IID
static const IID IID_IClassFactory = {0x00000001,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
static const IID IID_IUnknown     = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

// Voyager Theme CLSIDs (8 total)
static const CLSID CLSID_VoyagerThemeAlbum     = {0xC84CFE1B,0x89DC,0x40E7,{0x83,0xBF,0xCB,0x82,0x12,0x55,0xF9,0xEC}};
static const CLSID CLSID_VoyagerThemeCollage   = {0xAEE6C573,0xA192,0x4AF3,{0xB6,0x2B,0xA4,0xE6,0x84,0x85,0x33,0xD3}};
static const CLSID CLSID_VoyagerThemeFlip      = {0x653E52D8,0xD033,0x469A,{0x8B,0xB5,0x9C,0x1A,0x16,0x44,0x16,0xD5}};
static const CLSID CLSID_VoyagerThemeFrame     = {0xB4E10BE6,0xA2CE,0x4BEF,{0x9D,0x80,0x99,0x99,0x5C,0xB3,0xC1,0x62}};
static const CLSID CLSID_VoyagerThemeGlass     = {0x5515D2B5,0x6825,0x409E,{0xB3,0x77,0x54,0x47,0x08,0xC9,0xDD,0x06}};
static const CLSID CLSID_VoyagerThemeSnapshots = {0xD5561752,0xE5A7,0x46E7,{0xB7,0x68,0xD9,0x45,0xE1,0x44,0xCA,0x78}};
static const CLSID CLSID_VoyagerThemeTravel    = {0xCC4F1166,0xCE12,0x41F7,{0x85,0xE2,0xAE,0x47,0x44,0xD9,0x38,0x1B}};
static const CLSID CLSID_VoyagerTransform      = {0xE48325CB,0x1EFC,0x425E,{0x9C,0xD9,0x47,0xEF,0x51,0xBE,0xCD,0x55}};

struct ClsidEntry {
    const wchar_t* name;
    const CLSID*   clsid;
};

static const ClsidEntry g_clsids[] = {
    {L"VoyagerThemeAlbum",     &CLSID_VoyagerThemeAlbum},
    {L"VoyagerThemeCollage",   &CLSID_VoyagerThemeCollage},
    {L"VoyagerThemeFlip",      &CLSID_VoyagerThemeFlip},
    {L"VoyagerThemeFrame",     &CLSID_VoyagerThemeFrame},
    {L"VoyagerThemeGlass",     &CLSID_VoyagerThemeGlass},
    {L"VoyagerThemeSnapshots", &CLSID_VoyagerThemeSnapshots},
    {L"VoyagerThemeTravel",    &CLSID_VoyagerThemeTravel},
    {L"VoyagerTransform",      &CLSID_VoyagerTransform},
};
static const int g_nClsids = sizeof(g_clsids) / sizeof(g_clsids[0]);

typedef HRESULT (__stdcall *DllCanUnloadNowFunc)();
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServerFunc)();
typedef HRESULT (__stdcall *DllUnregisterServerFunc)();

static void PrintClsid(const CLSID& clsid) {
    printf("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        clsid.Data1, clsid.Data2, clsid.Data3,
        clsid.Data4[0], clsid.Data4[1], clsid.Data4[2], clsid.Data4[3],
        clsid.Data4[4], clsid.Data4[5], clsid.Data4[6], clsid.Data4[7]);
}

static wchar_t s_root[MAX_PATH];

static void GetRoot() {
    GetModuleFileNameW(NULL, s_root, MAX_PATH);
    for (int i = 0; i < 3; i++) { // strip test.exe -> WLXPhotoVoyager -> tests -> root
        wchar_t* p = wcsrchr(s_root, L'\\');
        if (p) *p = L'\0';
    }
}

int main() {
    GetRoot();
    printf("=== WLXPhotoVoyager.dll Dedicated Test Harness ===\n");
    printf("  8 CLSIDs via DllGetClassObject  |  4 COM exports\n\n");
    printf("[*] Root dir: %ls\n", s_root);

    // Add search paths for dependency resolution
    {
        wchar_t gallery[MAX_PATH], shared[MAX_PATH], mainDir[MAX_PATH];
        StringCchPrintfW(gallery, MAX_PATH, L"%ls\\undecomp\\Photo Gallery", s_root);
        StringCchPrintfW(shared,  MAX_PATH, L"%ls\\undecomp\\Shared",       s_root);
        GetFullPathNameW(L".", MAX_PATH, mainDir, NULL);
        AddDllDirectory(gallery);
        AddDllDirectory(shared);
        AddDllDirectory(mainDir);
        SetDllDirectoryW(gallery);
        printf("[*] Search paths set: %ls; %ls\n\n", gallery, shared);
    }

    // Load the DLL
    wchar_t dllPath[MAX_PATH];
    StringCchPrintfW(dllPath, MAX_PATH, L"%ls\\undecomp\\Photo Gallery\\WLXPhotoVoyager.dll", s_root);
    printf("[*] Loading: %ls\n", dllPath);
    HMODULE hMod = LoadLibraryExW(dllPath, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!hMod) {
        printf("[!] LoadLibraryExW failed (%u), retrying with SetDllDirectory\n", GetLastError());
        wchar_t gallery[MAX_PATH];
        GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, gallery, NULL);
        SetDllDirectoryW(gallery);
        hMod = LoadLibraryW(dllPath);
    }
    if (!hMod) {
        printf("[!] FATAL: Cannot load WLXPhotoVoyager.dll (err=%u)\n", GetLastError());
        return 1;
    }
    printf("[+] DLL loaded at 0x%p\n\n", hMod);

    // ================================================================
    // Phase 1 — Probe all 4 COM exports
    // ================================================================
    printf("========================================\n");
    printf("  Phase 1: Export Probing (4 exports)\n");
    printf("========================================\n");

    DllCanUnloadNowFunc     pfnCanUnload     = (DllCanUnloadNowFunc)    GetProcAddress(hMod, "DllCanUnloadNow");
    DllGetClassObjectFunc   pfnGetClassObject = (DllGetClassObjectFunc)  GetProcAddress(hMod, "DllGetClassObject");
    DllRegisterServerFunc   pfnRegister       = (DllRegisterServerFunc)  GetProcAddress(hMod, "DllRegisterServer");
    DllUnregisterServerFunc pfnUnregister     = (DllUnregisterServerFunc)GetProcAddress(hMod, "DllUnregisterServer");

    printf("  DllCanUnloadNow     = 0x%p\n",     pfnCanUnload);
    printf("  DllGetClassObject   = 0x%p\n",     pfnGetClassObject);
    printf("  DllRegisterServer   = 0x%p (skip)\n", pfnRegister);
    printf("  DllUnregisterServer = 0x%p (skip)\n", pfnUnregister);

    if (!pfnCanUnload || !pfnGetClassObject) {
        printf("[!] Missing critical exports\n");
        FreeLibrary(hMod);
        return 1;
    }

    // Call DllCanUnloadNow
    __try {
        HRESULT hr = pfnCanUnload();
        printf("  => DllCanUnloadNow() = 0x%08X\n", hr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  => DllCanUnloadNow() SEH 0x%08X\n", GetExceptionCode());
    }

    // ================================================================
    // Phase 2 — Probe all 8 CLSIDs via DllGetClassObject -> IClassFactory
    // ================================================================
    printf("\n========================================\n");
    printf("  Phase 2: DllGetClassObject (8 CLSIDs)\n");
    printf("========================================\n");

    for (int i = 0; i < g_nClsids; i++) {
        const ClsidEntry& entry = g_clsids[i];
        printf("\n[%d/%d] %ls\n  CLSID = ", i+1, g_nClsids, entry.name);
        PrintClsid(*entry.clsid);
        printf("\n");

        IClassFactory* pFactory = NULL;
        __try {
            HRESULT hr = pfnGetClassObject(*entry.clsid, IID_IClassFactory, (void**)&pFactory);
            printf("  DllGetClassObject => 0x%08X", hr);
            if (SUCCEEDED(hr) && pFactory) {
                printf(", pFactory=0x%p", pFactory);

                // Call CreateInstance from the factory
                IUnknown* pUnk = NULL;
                __try {
                    HRESULT hrCI = pFactory->CreateInstance(NULL, IID_IUnknown, (void**)&pUnk);
                    printf("\n    CreateInstance => 0x%08X", hrCI);
                    if (SUCCEEDED(hrCI) && pUnk) {
                        printf(", pUnk=0x%p", pUnk);
                        ULONG refs = pUnk->Release();
                        printf(", refs=%u", refs);
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("\n    CreateInstance SEH 0x%08X", GetExceptionCode());
                    if (pUnk) pUnk->Release();
                }
                pFactory->Release();
            }
            printf("\n");
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  DllGetClassObject SEH 0x%08X\n", GetExceptionCode());
            if (pFactory) pFactory->Release();
        }
    }

    // ================================================================
    // Phase 3 — DllCanUnloadNow after all class object activity
    // ================================================================
    printf("\n========================================\n");
    printf("  Phase 3: Final DllCanUnloadNow\n");
    printf("========================================\n");
    __try {
        HRESULT hr = pfnCanUnload();
        printf("  DllCanUnloadNow() = 0x%08X\n", hr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  DllCanUnloadNow() SEH 0x%08X\n", GetExceptionCode());
    }

    printf("\n[*] Unloading WLXPhotoVoyager.dll\n");
    FreeLibrary(hMod);

    printf("\n=== Test Harness Complete ===\n");
    return 0;
}
