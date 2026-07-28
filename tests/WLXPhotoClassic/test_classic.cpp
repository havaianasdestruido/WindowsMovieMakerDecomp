#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unknwn.h>
#include <cstdio>
#include <cstdlib>

typedef HRESULT (__stdcall *DllCanUnloadNow_t)();
typedef HRESULT (__stdcall *DllGetClassObject_t)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServer_t)();
typedef HRESULT (__stdcall *DllUnregisterServer_t)();

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
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf(" => SEH=0x%08X", GetExceptionCode());
        }
    } else if (strcmp(name, "DllGetClassObject") == 0) {
        CLSID empty = {0,0,0,{0,0,0,0,0,0,0,0}};
        IID iid = {0,0,0,{0,0,0,0,0,0,0,0}};
        void* pObj = NULL;
        __try {
            HRESULT hr = ((DllGetClassObject_t)fp)(empty, iid, &pObj);
            printf(" => HRESULT=0x%08X, pObj=0x%p", hr, pObj);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf(" => SEH=0x%08X", GetExceptionCode());
        }
    } else {
        printf(" [skip - would modify system]");
    }
    printf("\n");
}

int main() {
    printf("=== WLXPhotoClassic.dll Dedicated Test Harness ===\n\n");

    wchar_t dllDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, dllDir, NULL);
    printf("[*] SetDllDirectoryW: %ls\n", dllDir);
    SetDllDirectoryW(dllDir);

    wchar_t dllPath[MAX_PATH];
    swprintf_s(dllPath, L"%ls\\WLXPhotoClassic.dll", dllDir);
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
