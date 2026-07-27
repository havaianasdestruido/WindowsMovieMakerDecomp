#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>

// Interface IID for IPreviewClientStatusCallback (from static analysis)
// {DBFFDF24-FBB1-42D1-719A-EC305FBF765F}
static const IID IID_IPreviewClientStatusCallback = 
    {0xDBFFDF24, 0xFBB1, 0x42D1, {0x71, 0x9A, 0xEC, 0x30, 0x5F, 0xBF, 0x76, 0x5F}};

typedef HRESULT (STDMETHODCALLTYPE *pfnDllCanUnloadNow)(void);
typedef HRESULT (STDMETHODCALLTYPE *pfnDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);

int main(void) {
    printf("=== MovieMakerPreviewClient.dll Test Harness ===\n\n");

    // Step 1: Load the DLL
    SetDllDirectoryA("undecomp\\Photo Gallery");
    HMODULE hMod = LoadLibraryA("MovieMakerPreviewClient.dll");
    if (!hMod) {
        printf("LoadLibrary failed: %lu\n", GetLastError());
        return 1;
    }
    printf("Loaded MovieMakerPreviewClient.dll at %p\n", (void*)hMod);

    // Step 2: Get DllCanUnloadNow
    pfnDllCanUnloadNow pCanUnload = (pfnDllCanUnloadNow)GetProcAddress(hMod, "DllCanUnloadNow");
    pfnDllGetClassObject pGetClass = (pfnDllGetClassObject)GetProcAddress(hMod, "DllGetClassObject");
    printf("DllCanUnloadNow:    %p\n", (void*)pCanUnload);
    printf("DllGetClassObject:  %p\n", (void*)pGetClass);
    printf("DllRegisterServer:  %p\n", (void*)GetProcAddress(hMod, "DllRegisterServer"));
    printf("DllUnregisterServer:%p\n\n", (void*)GetProcAddress(hMod, "DllUnregisterServer"));

    // Step 3: Call DllCanUnloadNow
    if (pCanUnload) {
        HRESULT hr = pCanUnload();
        printf("DllCanUnloadNow() -> 0x%08X\n", hr);
    }

    // Step 4: Try DllGetClassObject with empty CLSID
    if (pGetClass) {
        CLSID emptyClsid = CLSID_NULL;
        IUnknown *pUnk = NULL;
        HRESULT hr = pGetClass(emptyClsid, IID_IUnknown, (void**)&pUnk);
        printf("DllGetClassObject(CLSID_NULL, IID_IUnknown) -> 0x%08X\n", hr);
        if (SUCCEEDED(hr) && pUnk) {
            printf("  Got IUnknown* = %p\n", (void*)pUnk);
            pUnk->Release();
        }
    }

    // Step 5: Try DllGetClassObject with the proxy IID as CLSID
    if (pGetClass) {
        CLSID proxyClsid = IID_IPreviewClientStatusCallback;
        IUnknown *pUnk = NULL;
        HRESULT hr = pGetClass(proxyClsid, IID_IUnknown, (void**)&pUnk);
        printf("DllGetClassObject(proxy_IID_as_CLSID, IID_IUnknown) -> 0x%08X\n", hr);
        if (SUCCEEDED(hr) && pUnk) {
            printf("  Got IUnknown* = %p\n", (void*)pUnk);
            pUnk->Release();
        }
    }

    // Step 6: CoInitialize + CoCreateInstance experiments
    HRESULT coInit = CoInitialize(NULL);
    printf("\nCoInitialize -> 0x%08X\n", coInit);

    // Try CoCreateInstance with the interface IID as CLSID (expect CLASS_E_CLASSNOTAVAILABLE)
    {
        IUnknown *pUnk = NULL;
        HRESULT hr = CoCreateInstance(IID_IPreviewClientStatusCallback, NULL,
            CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pUnk);
        printf("CoCreateInstance(interface_IID_as_CLSID) -> 0x%08X\n", hr);
        if (SUCCEEDED(hr) && pUnk) {
            printf("  Got IUnknown* = %p\n", (void*)pUnk);
            pUnk->Release();
        }
    }

    // Step 7: Re-check DllCanUnloadNow after usage
    if (pCanUnload) {
        HRESULT hr = pCanUnload();
        printf("DllCanUnloadNow() after usage -> 0x%08X\n", hr);
    }

    CoUninitialize();
    FreeLibrary(hMod);
    printf("\nDone.\n");
    return 0;
}
