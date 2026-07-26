#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#include <stdio.h>
#include <stdint.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

typedef BOOL (WINAPI *FnDllCanUnloadNow)(void);
typedef HRESULT (WINAPI *FnDllGetClassObject)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (WINAPI *FnDllRegisterServer)(void);
typedef HRESULT (WINAPI *FnDllUnregisterServer)(void);

static const CLSID CLSID_ActualEntry0 =
    {0x557B4CD8, 0xC1EA, 0x4A46, {0x84, 0xEE, 0xBA, 0x1A, 0xF9, 0xAA, 0x67, 0xD4}};
static const CLSID CLSID_ActualEntry1 =
    {0x0B1A232A, 0x4A09, 0x4A43, {0xA7, 0xB3, 0xE3, 0x67, 0xD1, 0xC3, 0xB4, 0xB7}};
static const CLSID CLSID_RGS_CinematicFullScreen =
    {0xB1CACF91, 0x6F51, 0x4533, {0xBB, 0x28, 0xB2, 0x2D, 0x4E, 0x8A, 0x9C, 0x65}};
static const CLSID CLSID_RGS_CinematicTransform =
    {0x5409AB48, 0xD8D3, 0x40e6, {0xA1, 0xEB, 0x23, 0x48, 0x9D, 0xC4, 0x22, 0xDE}};
static const CLSID CLSID_KenBurnsEffect =
    {0xB1A6684D, 0x1C5B, 0x4976, {0x84, 0x3D, 0x53, 0xF1, 0xE8, 0x40, 0xE4, 0x9C}};
static const CLSID CLSID_KenBurnsEffectConfig =
    {0x0831F9E2, 0x19E0, 0x4837, {0xB0, 0x26, 0x28, 0xE1, 0xE6, 0xE0, 0x87, 0xF4}};

static const IID IID_IUnknown1 =
    {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
static const IID IID_IClassFactory =
    {0x00000001, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
static const IID IID_IServiceProvider =
    {0x6D5140C1, 0x7436, 0x11CE, {0x80, 0x34, 0x00, 0x00, 0x69, 0x1B, 0x77, 0x05}};
static const IID IID_ISlideshowTheme =
    {0x5E843E41, 0x3BA1, 0x4B0D, {0xA5, 0x1E, 0x6D, 0x22, 0xF5, 0x0E, 0x02, 0xE9}};
static const IID IID_Entry0_IUnknown =
    {0x70E8E77F, 0x8721, 0x46B6, {0xB7, 0x46, 0x33, 0x5E, 0xBF, 0x85, 0x77, 0x04}};
static const IID IID_Entry1_IUnknown =
    {0xC2DB9835, 0x1146, 0x4B10, {0xA7, 0xBB, 0x73, 0x61, 0x91, 0x25, 0xB0, 0x26}};
static const IID IID_Entry1_Second =
    {0x6D5140C1, 0x7436, 0x11CE, {0x80, 0x34, 0x00, 0xAA, 0x00, 0x60, 0x09, 0xFA}};

static const char* HrStr(HRESULT hr) {
    switch(hr) {
        case 0x00000000: return "S_OK";
        case 0x80040111: return "CLASS_E_CLASSNOTAVAILABLE";
        case 0x80040154: return "REGDB_E_CLASSNOTREG";
        case 0x800401F3: return "CO_E_CLASSSTRING";
        case 0x8000FFFF: return "CO_E_NOT_INITIALIZED";
        case 0x80070005: return "E_ACCESSDENIED";
        case 0x80004002: return "E_NOINTERFACE";
        case 0x80004003: return "E_POINTER";
        case 0x8007000E: return "E_OUTOFMEMORY";
        case 0x00000001: return "S_FALSE";
        default: return "";
    }
}

static void LogVtable(void** vtable, const char* name, int maxSlots) {
    printf("  [%s] vtable @ %p\n", name, vtable);
    for (int i = 0; i < maxSlots; i++) {
        printf("    [%02d] %p", i, vtable[i]);
        if (vtable[i] == NULL) printf(" (NULL)");
        printf("\n");
    }
}

static void TryClsid(REFCLSID clsid, const char* name, FnDllGetClassObject pGetClassObj) {
    printf("\n--- %s ---\n", name);

    char clsidStr[64];
    StringFromGUID2(clsid, (LPOLESTR)clsidStr, 64);
    printf("  CLSID: %hs\n", clsidStr);

    IClassFactory* pFactory = NULL;
    HRESULT hr = pGetClassObj(clsid, IID_IClassFactory, (void**)&pFactory);
    printf("  DllGetClassObject(IClassFactory): 0x%08X %s\n", hr, HrStr(hr));

    if (SUCCEEDED(hr) && pFactory) {
        void** pFactoryVtable = *(void***)pFactory;
        LogVtable(pFactoryVtable, "IClassFactory", 7);

        IUnknown* pObj = NULL;
        hr = pFactory->CreateInstance(NULL, IID_IUnknown1, (void**)&pObj);
        printf("  CreateInstance(IUnknown): 0x%08X %s obj=%p\n", hr, HrStr(hr), pObj);
        if (SUCCEEDED(hr) && pObj) {
            void** pObjVtable = *(void***)pObj;
            LogVtable(pObjVtable, "Object vtable", 20);

            printf("\n  --- Specific IID probing ---\n");
            struct { const IID* iid; const char* name; } probes[] = {
                {&IID_IUnknown1, "IUnknown"},
                {&IID_IServiceProvider, "IServiceProvider {6D5140C1-...}"},
                {&IID_ISlideshowTheme, "ISlideshowTheme {5E843E41-...}"},
                {&IID_Entry0_IUnknown, "Entry0_IUnknown {70E8E77F-...}"},
                {&IID_Entry1_IUnknown, "Entry1_IUnknown {C2DB9835-...}"},
                {&IID_Entry1_Second, "Entry1_Second {6D5140C1-...}"},
                {NULL, NULL}
            };
            for (int k = 0; probes[k].name; k++) {
                IUnknown* pQ = NULL;
                hr = pObj->QueryInterface(*probes[k].iid, (void**)&pQ);
                printf("    QI(%s): 0x%08X %s", probes[k].name, hr, HrStr(hr));
                if (SUCCEEDED(hr) && pQ) {
                    printf(" ptr=%p same=%d", pQ, pQ == (IUnknown*)pObj);
                    if (pQ != (IUnknown*)pObj) {
                        void** qvtable = *(void***)pQ;
                        printf("\n      Sub-vtable @ %p:", qvtable);
                        for (int s = 0; s < 15; s++) {
                            if (qvtable[s] == NULL) break;
                            printf("\n        [%02d] %p", s, qvtable[s]);
                        }
                    }
                    pQ->Release();
                }
                printf("\n");
            }
            pObj->Release();
        }
        pFactory->Release();
    }
}

int main() {
    printf("=== WLXPhotoCinematic.dll Dynamic Analysis (v3) ===\n\n");

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    WCHAR exeDir[MAX_PATH];
    GetModuleFileNameW(NULL, exeDir, MAX_PATH);
    WCHAR* lastSlash = wcsrchr(exeDir, L'\\');
    if (lastSlash) *(lastSlash + 1) = L'\0';
    SetDllDirectoryW(exeDir);
    printf("Working dir: %ls\n", exeDir);

    HMODULE hDll = LoadLibraryW(L"WLXPhotoCinematic.dll");
    if (!hDll) {
        printf("Failed to load DLL. Error: %lu\n", GetLastError());
        CoUninitialize();
        return 1;
    }
    printf("DLL loaded at %p\n", hDll);

    FnDllGetClassObject pGetClassObj = (FnDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject");
    FnDllCanUnloadNow pCanUnload = (FnDllCanUnloadNow)GetProcAddress(hDll, "DllCanUnloadNow");
    printf("DllGetClassObject @ %p\n", pGetClassObj);
    printf("DllCanUnloadNow @ %p\n\n", pCanUnload);

    printf("=== Object map CLSIDs (from disassembly) ===\n");
    TryClsid(CLSID_ActualEntry0, "Entry0 CinematicFullScreen {557B4CD8-C1EA-4A46-84EE-BA1AF9AA67D4}", pGetClassObj);
    TryClsid(CLSID_ActualEntry1, "Entry1 PanZoomTransform {0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}", pGetClassObj);

    printf("\n=== RGS script CLSIDs (from resources) ===\n");
    TryClsid(CLSID_RGS_CinematicFullScreen, "RGS_FullScreen {B1CACF91-...}", pGetClassObj);
    TryClsid(CLSID_RGS_CinematicTransform, "RGS_Transform {5409AB48-...}", pGetClassObj);

    printf("\n=== RGS script CLSIDs (KenBurns, likely from other DLLs) ===\n");
    TryClsid(CLSID_KenBurnsEffect, "KenBurnsEffect {B1A6684D-...}", pGetClassObj);
    TryClsid(CLSID_KenBurnsEffectConfig, "KenBurnsEffectConfig {0831F9E2-...}", pGetClassObj);

    printf("\n=== DllCanUnloadNow ===\n");
    BOOL canUnload = pCanUnload();
    printf("  Result: %d (TRUE=can unload, FALSE=still in use)\n", canUnload);

    CoUninitialize();
    FreeLibrary(hDll);
    return 0;
}
