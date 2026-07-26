#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <cstdio>
#include <cstdint>

typedef HRESULT (WINAPI *DllCanUnloadNow_t)(void);
typedef HRESULT (WINAPI *DllGetClassObject_t)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (WINAPI *DllRegisterServer_t)(void);
typedef HRESULT (WINAPI *DllUnregisterServer_t)(void);

struct ExportEntry {
    const char* name;
    FARPROC addr;
};

static void ProbeExport(HMODULE hMod, const char* name, FARPROC addr) {
    printf("  %-30s @ 0x%p\n", name, addr);
    if (!addr) {
        printf("    [SKIP] NULL address\n");
        return;
    }
    if (strcmp(name, "DllCanUnloadNow") == 0) {
        auto fn = (DllCanUnloadNow_t)addr;
        __try {
            HRESULT hr = fn();
            printf("    -> DllCanUnloadNow() = 0x%08X\n", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("    -> DllCanUnloadNow() EXCEPTION 0x%08X\n", GetExceptionCode());
        }
    } else if (strcmp(name, "DllRegisterServer") == 0) {
        printf("    -> [SKIP] DllRegisterServer (requires admin)\n");
    } else if (strcmp(name, "DllUnregisterServer") == 0) {
        printf("    -> [SKIP] DllUnregisterServer (requires admin)\n");
    } else if (strcmp(name, "DllGetClassObject") == 0) {
        auto fn = (DllGetClassObject_t)addr;
        CLSID clsid_null = CLSID_NULL;
        IID iid_null = IID_IUnknown;
        IClassFactory* pCF = NULL;
        __try {
            HRESULT hr = fn(clsid_null, iid_null, (void**)&pCF);
            printf("    -> DllGetClassObject(CLSID_NULL) = 0x%08X\n", hr);
            if (pCF) pCF->Release();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("    -> DllGetClassObject() EXCEPTION 0x%08X\n", GetExceptionCode());
        }
    }
}

int main() {
    printf("=== WLMFDS.dll Dynamic Analysis Harness ===\n\n");

    HMODULE hMod = LoadLibraryW(L"undecomp\\Shared\\WLMFDS.dll");
    if (!hMod) {
        printf("Failed to load WLMFDS.dll: error %u\n", GetLastError());
        return 1;
    }
    printf("Module base: 0x%p\n\n", (void*)hMod);

    printf("--- Exported Functions (4) ---\n");
    ExportEntry exports[] = {
        {"DllCanUnloadNow",     GetProcAddress(hMod, "DllCanUnloadNow")},
        {"DllGetClassObject",   GetProcAddress(hMod, "DllGetClassObject")},
        {"DllRegisterServer",   GetProcAddress(hMod, "DllRegisterServer")},
        {"DllUnregisterServer", GetProcAddress(hMod, "DllUnregisterServer")},
    };
    for (auto& e : exports) {
        ProbeExport(hMod, e.name, e.addr);
    }

    printf("\n--- CLSID_DShowSourceResolver Probe ---\n");
    {
        CLSID clsid_dssr;
        HRESULT hr = CLSIDFromString(L"{C5D63D5A-16C8-4F16-A39C-0D7BF80B1672}", &clsid_dssr);
        if (SUCCEEDED(hr)) {
            auto fn_dgco = (DllGetClassObject_t)GetProcAddress(hMod, "DllGetClassObject");
            if (fn_dgco) {
                IClassFactory* pCF = NULL;
                hr = fn_dgco(clsid_dssr, IID_IClassFactory, (void**)&pCF);
                printf("  DllGetClassObject(CLSID_DShowSourceResolver) = 0x%08X\n", hr);
                if (SUCCEEDED(hr) && pCF) {
                    IUnknown* pUnk = NULL;
                    hr = pCF->CreateInstance(NULL, IID_IUnknown, (void**)&pUnk);
                    printf("  CreateInstance(IUnknown) = 0x%08X\n", hr);
                    if (pUnk) pUnk->Release();
                    pCF->Release();
                }
            }
        } else {
            printf("  CLSIDFromString failed: 0x%08X\n", hr);
        }
    }

    printf("\n--- MF Source Resolver Probe ---\n");
    {
        IMFSourceResolver* pResolver = NULL;
        HRESULT hr = MFCreateSourceResolver(&pResolver);
        if (SUCCEEDED(hr) && pResolver) {
            printf("  MFCreateSourceResolver OK, IMFSourceResolver* = 0x%p\n", pResolver);
            pResolver->Release();
        } else {
            printf("  MFCreateSourceResolver = 0x%08X\n", hr);
        }
    }

    printf("\n--- Probe Complete ---\n");
    printf("Key imported MF APIs:\n");
    printf("  MFCreateSample, MFCreateStreamDescriptor, MFCreatePresentationDescriptor\n");
    printf("  MFCreateMediaTypeFromRepresentation, MFCreateMemoryBuffer, MFCreateAlignedMemoryBuffer\n");
    printf("  MFCreateEventQueue, MFCreateAsyncResult, MFTEnum, MFPutWorkItem, MFPutWorkItemEx\n");
    printf("  MFInvokeCallback, MFLockPlatform, MFUnlockPlatform, MFHeapAlloc, MFHeapFree\n");
    printf("  MFCreateSourceResolver\n");
    printf("  MFCopyImage (EVR.dll)\n");
    printf("  AvSetMmThreadCharacteristicsW, AvRevertMmThreadCharacteristics (AVRT.dll)\n");
    printf("  timeSetEvent, timeGetTime, etc. (WINMM.dll)\n");

    FreeLibrary(hMod);
    return 0;
}
