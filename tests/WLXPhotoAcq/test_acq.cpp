#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>

#define CHECK(p, name) printf("  %-50s %s\n", name, (p) ? "OK" : "MISSING")
#define HR(name, hr)  printf("  %-50s 0x%08X\n", name, (unsigned)(hr))

// {4E13844B-6012-40cb-8E3C-0F7EB9AEF6DD}
static const CLSID CLSID_PhotoAcquire =
    {0x4E13844B, 0x6012, 0x40cb, {0x8E, 0x3C, 0x0F, 0x7E, 0xB9, 0xAE, 0xF6, 0xDD}};
// {94B1CBEA-F33F-4508-8AAA-55FD8113EFCB}
static const CLSID CLSID_PhotoAcquireOptionsDialog =
    {0x94B1CBEA, 0xF33F, 0x4508, {0x8A, 0xAA, 0x55, 0xFD, 0x81, 0x13, 0xEF, 0xCB}};
// {0D5A7D0E-9A06-4e17-85D9-A0B24036371D}
static const CLSID CLSID_PhotoPickerDialog =
    {0x0D5A7D0E, 0x9A06, 0x4e17, {0x85, 0xD9, 0xA0, 0xB2, 0x40, 0x36, 0x37, 0x1D}};
// {E84D0D46-3D57-4039-9EFE-310AF1CAF92A}
static const CLSID CLSID_PhotoAcquireDeviceSelectionDialog =
    {0xE84D0D46, 0x3D57, 0x4039, {0x9E, 0xFE, 0x31, 0x0A, 0xF1, 0xCA, 0xF9, 0x2A}};
// {4D8A134F-3D0A-4375-8B1A-78CD171C9318}
static const CLSID CLSID_LivePhotoAcquisitionWizard =
    {0x4D8A134F, 0x3D0A, 0x4375, {0x8B, 0x1A, 0x78, 0xCD, 0x17, 0x1C, 0x93, 0x18}};

typedef HRESULT (STDMETHODCALLTYPE *DllCanUnloadNow_t)(void);
typedef HRESULT (STDMETHODCALLTYPE *DllGetClassObject_t)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (STDMETHODCALLTYPE *DllRegisterServer_t)(void);
typedef HRESULT (STDMETHODCALLTYPE *DllUnregisterServer_t)(void);

static int g_nSkipped = 0;
#define TRY(x) do { __try { x; } __except(EXCEPTION_EXECUTE_HANDLER) { printf("  -> EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); } } while(0)

static void try_dllgetclassobject(DllGetClassObject_t fn, const char* label, REFCLSID rclsid) {
    IUnknown* pUnk = NULL;
    __try {
        HRESULT hr = fn(rclsid, IID_IUnknown, (void**)&pUnk);
        printf("  %-50s 0x%08X", label, (unsigned)hr);
        if (SUCCEEDED(hr) && pUnk) {
            printf(" pUnk=%p", (void*)pUnk);
            pUnk->Release();
        }
        printf("\n");
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  %-50s EXCEPTION 0x%08X\n", label, (unsigned)GetExceptionCode());
    }
}

static void try_reg(DllRegisterServer_t fn, const char* label) {
    __try {
        HRESULT hr = fn();
        printf("  %-50s 0x%08X\n", label, (unsigned)hr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  %-50s EXCEPTION 0x%08X\n", label, (unsigned)GetExceptionCode());
    }
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== WLXPhotoAcq.dll Test Harness ===\n\n");

    SetDllDirectoryA("undecomp\\Photo Gallery");

    HMODULE hMod = LoadLibraryA("WLXPhotoAcq.dll");
    if (!hMod) {
        printf("LoadLibrary failed: %lu\n", GetLastError());
        return 1;
    }
    printf("Loaded at %p\n\n", (void*)hMod);

    DllCanUnloadNow_t pCanUnload = (DllCanUnloadNow_t)GetProcAddress(hMod, "DllCanUnloadNow");
    DllGetClassObject_t pGetClass = (DllGetClassObject_t)GetProcAddress(hMod, "DllGetClassObject");
    DllRegisterServer_t pRegSvr = (DllRegisterServer_t)GetProcAddress(hMod, "DllRegisterServer");
    DllUnregisterServer_t pUnregSvr = (DllUnregisterServer_t)GetProcAddress(hMod, "DllUnregisterServer");

    printf("=== Exports (4) ===\n");
    CHECK(pCanUnload, "DllCanUnloadNow");
    CHECK(pGetClass,  "DllGetClassObject");
    CHECK(pRegSvr,    "DllRegisterServer");
    CHECK(pUnregSvr,  "DllUnregisterServer");
    printf("\n");

    printf("=== DllCanUnloadNow ===\n");
    if (pCanUnload) {
        HRESULT hr = pCanUnload();
        HR("DllCanUnloadNow()", hr);
    }
    printf("\n");

    printf("=== DllGetClassObject ===\n");
    if (pGetClass) {
        try_dllgetclassobject(pGetClass, "CLSID_PhotoAcquire",             CLSID_PhotoAcquire);
        try_dllgetclassobject(pGetClass, "CLSID_PhotoAcquireOptionsDialog", CLSID_PhotoAcquireOptionsDialog);
        try_dllgetclassobject(pGetClass, "CLSID_PhotoPickerDialog",         CLSID_PhotoPickerDialog);
        try_dllgetclassobject(pGetClass, "CLSID_DeviceSelectionDialog",     CLSID_PhotoAcquireDeviceSelectionDialog);
        try_dllgetclassobject(pGetClass, "CLSID_LiveAcquisitionWizard",     CLSID_LivePhotoAcquisitionWizard);
        try_dllgetclassobject(pGetClass, "CLSID_NULL",                      CLSID_NULL);
    }
    printf("\n");

    printf("=== DllRegisterServer / DllUnregisterServer ===\n");
    if (pRegSvr)   try_reg(pRegSvr,   "DllRegisterServer");
    if (pUnregSvr) try_reg(pUnregSvr, "DllUnregisterServer");
    printf("\n");

    printf("=== CoCreateInstance (via COM) ===\n");
    HRESULT coInit = CoInitialize(NULL);
    HR("CoInitialize", coInit);
    if (SUCCEEDED(coInit)) {
        struct { const char* name; REFCLSID clsid; } coclasses[] = {
            { "CLSID_PhotoAcquire",             CLSID_PhotoAcquire },
            { "CLSID_PhotoAcquireOptionsDialog", CLSID_PhotoAcquireOptionsDialog },
            { "CLSID_PhotoPickerDialog",         CLSID_PhotoPickerDialog },
            { "CLSID_DeviceSelectionDialog",     CLSID_PhotoAcquireDeviceSelectionDialog },
            { "CLSID_LiveAcquisitionWizard",     CLSID_LivePhotoAcquisitionWizard },
        };
        for (int i = 0; i < 5; i++) {
            IUnknown* pUnk = NULL;
            __try {
                HRESULT hr = CoCreateInstance(coclasses[i].clsid, NULL,
                    CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pUnk);
                printf("  %-50s 0x%08X", coclasses[i].name, (unsigned)hr);
                if (SUCCEEDED(hr) && pUnk) {
                    printf(" pUnk=%p", (void*)pUnk);
                    pUnk->Release();
                }
                printf("\n");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  %-50s EXCEPTION 0x%08X\n", coclasses[i].name, (unsigned)GetExceptionCode());
            }
        }
        CoUninitialize();
    }
    printf("\n");

    printf("=== Re-check DllCanUnloadNow ===\n");
    if (pCanUnload) {
        HRESULT hr = pCanUnload();
        HR("DllCanUnloadNow()", hr);
    }
    printf("\n");

    printf("=== Known Import DLLs ===\n");
    const char* deps[] = {
        "WLXPhotoBase.dll", "WLXPhotoSqm.dll", "DmxBici.dll", "UXCore.dll",
        "MetadataSys.dll", "STI.dll",
    };
    for (int i = 0; i < 6; i++) {
        HMODULE hDep = LoadLibraryA(deps[i]);
        if (hDep) {
            printf("  %-30s %p\n", deps[i], (void*)hDep);
            FreeLibrary(hDep);
        } else {
            printf("  %-30s FAILED %lu\n", deps[i], GetLastError());
        }
    }
    printf("\n");

    printf("=== STI (WIA) Delay-Load Probe ===\n");
    {
        HMODULE hSti = GetModuleHandleA("STI.dll");
        printf("  STI.dll loaded: %s\n", hSti ? "yes" : "no (delay)");
        if (!hSti) {
            hSti = LoadLibraryA("STI.dll");
            if (hSti) {
                FARPROC pSti = GetProcAddress(hSti, "StiCreateInstanceW");
                printf("  StiCreateInstanceW: %p\n", (void*)pSti);
                FreeLibrary(hSti);
            } else {
                printf("  LoadLibrary(STI.dll) failed: %lu\n", GetLastError());
            }
        }
    }
    printf("\n");

    printf("=== PHOTOACQ.DLL (real name) ===\n");
    {
        wchar_t path[MAX_PATH];
        DWORD len = GetModuleFileNameW(hMod, path, MAX_PATH);
        printf("  ModuleFileName: %ls\n", path);
        printf("  Real DLL name:  PhotoAcq.dll (exported as)\n");
    }
    printf("\n");

    printf("=== Version Resource ===\n");
    {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(hMod, path, MAX_PATH);
        DWORD handle = 0;
        DWORD sz = GetFileVersionInfoSizeW(path, &handle);
        printf("  Version info size: %lu\n", sz);
        if (sz) {
            void* buf = malloc(sz);
            if (buf && GetFileVersionInfoW(path, 0, sz, buf)) {
                VS_FIXEDFILEINFO* ffi = NULL;
                UINT ffiLen = 0;
                if (VerQueryValueW(buf, L"\\", (void**)&ffi, &ffiLen) && ffi) {
                    printf("  Product:  %u.%u.%u.%u\n",
                        HIWORD(ffi->dwProductVersionMS), LOWORD(ffi->dwProductVersionMS),
                        HIWORD(ffi->dwProductVersionLS), LOWORD(ffi->dwProductVersionLS));
                    printf("  File:     %u.%u.%u.%u\n",
                        HIWORD(ffi->dwFileVersionMS), LOWORD(ffi->dwFileVersionMS),
                        HIWORD(ffi->dwFileVersionLS), LOWORD(ffi->dwFileVersionLS));
                }
            }
            free(buf);
        }
    }
    printf("\n");

    printf("=== Test Complete ===\n");
    FreeLibrary(hMod);
    return 0;
}
