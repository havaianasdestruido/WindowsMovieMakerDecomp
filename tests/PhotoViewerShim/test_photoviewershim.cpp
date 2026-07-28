#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unknwn.h>
#include <cstdio>
#include <cstdlib>

typedef HRESULT (__stdcall *DllCanUnloadNow_t)();
typedef HRESULT (__stdcall *DllGetClassObject_t)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServer_t)();
typedef HRESULT (__stdcall *DllUnregisterServer_t)();

static const CLSID CLSID_PhotoViewerShim =
    {0x00f346cb,0x35a4,0x465b,{0x8b,0x8f,0x65,0xa2,0x9d,0xba,0xb1,0xf6}};
static const CLSID CLSID_PhotoEditorShim =
    {0x00f3712a,0xca79,0x45b4,{0x9e,0x4d,0xd7,0x89,0x1e,0x7f,0x8b,0x9d}};
static const IID IID_IUnknown_Cast =
    {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

static void TestExport(HMODULE h, const char* name, int ordinal) {
    FARPROC fp = GetProcAddress(h, name);
    printf("  %s (ord %d): ptr=0x%p", name, ordinal, fp);
    if (!fp) {
        printf(" [NOT FOUND]\n");
        return;
    }

    if (strcmp(name, "DllCanUnloadNow") == 0) {
        __try {
            HRESULT hr = ((DllCanUnloadNow_t)fp)();
            printf(" => HRESULT=0x%08X", hr);
            if (hr == S_OK) printf(" (S_OK)");
            else if (hr == S_FALSE) printf(" (S_FALSE)");
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf(" => SEH=0x%08X", GetExceptionCode());
        }
    } else if (strcmp(name, "DllGetClassObject") == 0) {
        printf("\n");

        void* pObj = NULL;
        printf("    [null CLSID, null ppv] ");
        __try {
            HRESULT hr = ((DllGetClassObject_t)fp)(CLSID_NULL, IID_IUnknown_Cast, NULL);
            printf("HRESULT=0x%08X", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("SEH=0x%08X", GetExceptionCode());
        }
        printf("\n");

        pObj = NULL;
        printf("    [null CLSID, valid ppv] ");
        __try {
            HRESULT hr = ((DllGetClassObject_t)fp)(CLSID_NULL, IID_IUnknown_Cast, (void**)&pObj);
            printf("HRESULT=0x%08X", hr);
            if (SUCCEEDED(hr) && pObj) {
                printf(", pUnk=0x%p", pObj);
                ((IUnknown*)pObj)->Release();
                printf(" [RELEASED]");
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("SEH=0x%08X", GetExceptionCode());
        }
        printf("\n");

        pObj = NULL;
        printf("    [CLSID_PhotoViewerShim] ");
        __try {
            HRESULT hr = ((DllGetClassObject_t)fp)(CLSID_PhotoViewerShim, IID_IUnknown_Cast, (void**)&pObj);
            printf("HRESULT=0x%08X", hr);
            if (SUCCEEDED(hr) && pObj) {
                printf(", pUnk=0x%p", pObj);
                ((IUnknown*)pObj)->Release();
                printf(" [RELEASED]");
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("SEH=0x%08X", GetExceptionCode());
        }
        printf("\n");

        pObj = NULL;
        printf("    [CLSID_PhotoEditorShim] ");
        __try {
            HRESULT hr = ((DllGetClassObject_t)fp)(CLSID_PhotoEditorShim, IID_IUnknown_Cast, (void**)&pObj);
            printf("HRESULT=0x%08X", hr);
            if (SUCCEEDED(hr) && pObj) {
                printf(", pUnk=0x%p", pObj);
                ((IUnknown*)pObj)->Release();
                printf(" [RELEASED]");
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("SEH=0x%08X", GetExceptionCode());
        }
        printf("\n");
        return;
    } else {
        printf(" [skip - would modify system]");
    }
    printf("\n");
}

int main() {
    printf("=== PhotoViewerShim.dll Dedicated Test Harness ===\n");
    printf("OLE Drop Target Shim for Windows Photo Gallery\n\n");

    wchar_t dllDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, dllDir, NULL);
    printf("[*] SetDllDirectoryW: %ls\n", dllDir);
    SetDllDirectoryW(dllDir);

    wchar_t dllPath[MAX_PATH];
    swprintf_s(dllPath, L"%ls\\PhotoViewerShim.dll", dllDir);
    printf("[*] Loading: %ls\n", dllPath);
    HMODULE h = LoadLibraryW(dllPath);
    if (!h) {
        printf("[!] LoadLibrary failed: error %u\n", GetLastError());
        printf("\n=== FAILED ===\n");
        return 1;
    }
    printf("[+] Base address: 0x%p\n\n", h);

    printf("--- Export Calls ---\n");
    TestExport(h, "DllCanUnloadNow", 1);
    TestExport(h, "DllGetClassObject", 2);
    TestExport(h, "DllRegisterServer", 3);
    TestExport(h, "DllUnregisterServer", 4);

    FreeLibrary(h);
    printf("\n=== Done ===\n");
    return 0;
}
