#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>

// {E43EF6CD-A37A-4A9B-9E6F-83F89B8E6324}
static const GUID CLSID_AlbumDownloadLauncher = {
    0xe43ef6cd, 0xa37a, 0x4a9b, { 0x9e, 0x6f, 0x83, 0xf8, 0x9b, 0x8e, 0x63, 0x24 }
};

// {00000001-0000-0000-C000-000000000046} IID_IClassFactory
static const GUID IID_IClassFactory_ = {
    0x00000001, 0x0000, 0x0000, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
};

typedef HRESULT (STDAPICALLTYPE *FnDllCanUnloadNow)(void);
typedef HRESULT (STDAPICALLTYPE *FnDllGetClassObject)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (STDAPICALLTYPE *FnDllRegisterServer)(void);
typedef HRESULT (STDAPICALLTYPE *FnDllUnregisterServer)(void);

static const char* hr_name(HRESULT hr)
{
    switch (hr) {
    case S_OK:                 return "S_OK";
    case S_FALSE:              return "S_FALSE";
    case E_UNEXPECTED:         return "E_UNEXPECTED";
    case CLASS_E_CLASSNOTAVAILABLE: return "CLASS_E_CLASSNOTAVAILABLE";
    case REGDB_E_CLASSNOTREG:  return "REGDB_E_CLASSNOTREG";
    default:                   return "OTHER";
    }
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== AlbumDownloadProtocolHandler.dll Test Harness ===\n\n");

    // DLL search path for WLXPhotoBase.dll
    SetDllDirectoryW(L"C:\\Users\\mcmco\\Desktop\\WMMR\\undecomp\\Photo Gallery");

    HMODULE hDll = LoadLibraryW(L"C:\\Users\\mcmco\\Desktop\\WMMR\\undecomp\\Photo Gallery\\AlbumDownloadProtocolHandler.dll");
    if (!hDll) {
        printf("[FAIL] LoadLibrary: error %lu\n", GetLastError());
        return 1;
    }
    printf("[PASS] LoadLibrary: DLL loaded at %p\n\n", (void*)hDll);

    // Resolve all 4 exports
    FnDllCanUnloadNow      pCanUnload      = (FnDllCanUnloadNow)     GetProcAddress(hDll, "DllCanUnloadNow");
    FnDllGetClassObject    pGetClassObject  = (FnDllGetClassObject)   GetProcAddress(hDll, "DllGetClassObject");
    FnDllRegisterServer    pRegisterServer  = (FnDllRegisterServer)   GetProcAddress(hDll, "DllRegisterServer");
    FnDllUnregisterServer  pUnregisterServer= (FnDllUnregisterServer) GetProcAddress(hDll, "DllUnregisterServer");

    printf("Export Resolution:\n");
    printf("  DllCanUnloadNow      %s\n", pCanUnload       ? "OK" : "MISSING");
    printf("  DllGetClassObject    %s\n", pGetClassObject   ? "OK" : "MISSING");
    printf("  DllRegisterServer    %s\n", pRegisterServer   ? "OK" : "MISSING");
    printf("  DllUnregisterServer  %s\n", pUnregisterServer ? "OK" : "MISSING");

    if (!pCanUnload || !pGetClassObject || !pRegisterServer || !pUnregisterServer) {
        printf("\n[FAIL] One or more exports missing\n");
        FreeLibrary(hDll);
        return 1;
    }
    printf("\n");

    // ---- Test 1: DllCanUnloadNow ----
    printf("--- Test 1: DllCanUnloadNow ---\n");
    HRESULT hr = pCanUnload();
    printf("  Result: 0x%08X (%s)\n", (unsigned)hr, hr_name(hr));
    printf("  Expect: S_OK (0x00000000) since ATL module has zero locks\n\n");

    // ---- Test 2: DllGetClassObject ----
    printf("--- Test 2: DllGetClassObject(CLSID_AlbumDownloadLauncher, IID_IClassFactory) ---\n");
    IClassFactory* pFactory = NULL;
    hr = pGetClassObject(CLSID_AlbumDownloadLauncher, IID_IClassFactory_, (LPVOID*)&pFactory);
    printf("  Result: 0x%08X (%s)\n", (unsigned)hr, hr_name(hr));
    printf("  Note: E_UNEXPECTED is expected because ATL module's object map is not\n");
    printf("        populated outside the Photo Gallery runtime context (see analysis.md)\n");
    if (SUCCEEDED(hr) && pFactory) {
        printf("  Factory: %p\n", (void*)pFactory);
        // Try creating an instance
        IUnknown* pUnk = NULL;
        hr = pFactory->CreateInstance(NULL, IID_IUnknown, (void**)&pUnk);
        printf("  CreateInstance(IUnknown): 0x%08X\n", (unsigned)hr);
        if (SUCCEEDED(hr) && pUnk) {
            printf("  Instance: %p\n", (void*)pUnk);
            pUnk->Release();
        }
        pFactory->Release();
    }
    printf("\n");

    // ---- Test 3: DllRegisterServer ----
    printf("--- Test 3: DllRegisterServer ---\n");
    __try {
        hr = pRegisterServer();
        printf("  Result: 0x%08X (%s)\n", (unsigned)hr, hr_name(hr));
        if (SUCCEEDED(hr)) {
            printf("  DLL registered under CLSID {E43EF6CD-A37A-4A9B-9E6F-83F89B8E6324}\n");
            printf("  and PROTOCOLS\\Handler\\wlalbumdownload\n");
        } else {
            printf("  Note: Registration may require elevation\n");
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        printf("  EXCEPTION 0x%08X during DllRegisterServer\n", (unsigned)GetExceptionCode());
    }
    printf("\n");

    // ---- Test 4: DllUnregisterServer ----
    printf("--- Test 4: DllUnregisterServer ---\n");
    __try {
        hr = pUnregisterServer();
        printf("  Result: 0x%08X (%s)\n", (unsigned)hr, hr_name(hr));
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        printf("  EXCEPTION 0x%08X during DllUnregisterServer\n", (unsigned)GetExceptionCode());
    }
    printf("\n");

    // ---- Summary ----
    printf("=== Summary ===\n");
    printf("  4/4 COM exports resolved successfully\n");
    printf("  DllCanUnloadNow     : called\n");
    printf("  DllGetClassObject   : called\n");
    printf("  DllRegisterServer   : called\n");
    printf("  DllUnregisterServer : called\n\n");

    FreeLibrary(hDll);
    printf("Test harness complete.\n");
    return 0;
}
