#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>

static const CLSID CLSID_Empty = {0,0,0,{0,0,0,0,0,0,0,0}};
static const IID IID_IUnknown_Cast = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

typedef HRESULT (STDAPICALLTYPE *FnDllCanUnloadNow)();
typedef HRESULT (STDAPICALLTYPE *FnDllGetClassObject)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (STDAPICALLTYPE *FnDllRegisterServer)();
typedef HRESULT (STDAPICALLTYPE *FnDllUnregisterServer)();

typedef struct {
    unsigned int cbSize;
    void* CreateClip;
    void* CreateClipContainer;
    void* CreatePixCommands;
    void* CreateSourceManager;
    void* CreateMediaSourceFile;
    void* reserved[4];
} VMGClipCreateFunctions;

typedef HRESULT (STDAPICALLTYPE *FnVMGGetClipCreateFunctions)(VMGClipCreateFunctions*);
typedef void (STDAPICALLTYPE *FnVMGShellThumbnailGeneratorW)(const wchar_t*, const wchar_t*, void*);

static void print_hr(const char* label, HRESULT hr) {
    printf("  %-40s => 0x%08X%s\n", label, hr,
        hr == S_OK ? " (S_OK)" :
        hr == S_FALSE ? " (S_FALSE)" :
        hr == E_FAIL ? " (E_FAIL)" :
        hr == E_NOTIMPL ? " (E_NOTIMPL)" :
        hr == E_OUTOFMEMORY ? " (E_OUTOFMEMORY)" :
        hr == E_INVALIDARG ? " (E_INVALIDARG)" :
        hr == E_POINTER ? " (E_POINTER)" :
        hr == E_ACCESSDENIED ? " (E_ACCESSDENIED)" : "");
}

static void test_can_unload(FnDllCanUnloadNow fn) {
    printf("\n--- DllCanUnloadNow ---\n");
    if (!fn) { printf("  MISSING\n"); return; }
    printf("  ptr=0x%p\n", fn);
    __try {
        HRESULT hr = fn();
        print_hr("DllCanUnloadNow()", hr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  => SEH 0x%08X\n", GetExceptionCode());
    }
}

static void test_get_class_object(FnDllGetClassObject fn) {
    printf("\n--- DllGetClassObject ---\n");
    if (!fn) { printf("  MISSING\n"); return; }
    printf("  ptr=0x%p\n", fn);

    __try {
        HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, NULL);
        print_hr("null CLSID, null ppv", hr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  null args => SEH 0x%08X\n", GetExceptionCode());
    }

    IUnknown* pUnk = NULL;
    __try {
        HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, (void**)&pUnk);
        print_hr("null CLSID, valid ppv", hr);
        if (pUnk) { pUnk->Release(); pUnk = NULL; }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  null CLSID+ppv => SEH 0x%08X\n", GetExceptionCode());
    }

    IUnknown* pUnk2 = NULL;
    __try {
        CLSID testClsid = {0xFFFFFFFF,0xFFFF,0xFFFF,{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}};
        HRESULT hr = fn(testClsid, IID_IUnknown_Cast, (void**)&pUnk2);
        print_hr("garbage CLSID", hr);
        if (pUnk2) { pUnk2->Release(); pUnk2 = NULL; }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  garbage CLSID => SEH 0x%08X\n", GetExceptionCode());
    }
}

static void test_register_server(FnDllRegisterServer fn) {
    printf("\n--- DllRegisterServer ---\n");
    if (!fn) { printf("  MISSING\n"); return; }
    printf("  ptr=0x%p\n", fn);
    printf("  SKIP (would write registry)\n");
}

static void test_unregister_server(FnDllUnregisterServer fn) {
    printf("\n--- DllUnregisterServer ---\n");
    if (!fn) { printf("  MISSING\n"); return; }
    printf("  ptr=0x%p\n", fn);
    printf("  SKIP (would remove registry)\n");
}

static void test_get_clip_create_functions(FnVMGGetClipCreateFunctions fn) {
    printf("\n--- VMGGetClipCreateFunctions ---\n");
    if (!fn) { printf("  MISSING\n"); return; }
    printf("  ptr=0x%p\n", fn);

    __try {
        VMGClipCreateFunctions funcs;
        ZeroMemory(&funcs, sizeof(funcs));
        funcs.cbSize = sizeof(funcs);
        HRESULT hr = fn(&funcs);
        print_hr("valid struct (zeroed)", hr);
        int nValid = (funcs.CreateClip ? 1 : 0)
                   + (funcs.CreateClipContainer ? 1 : 0)
                   + (funcs.CreatePixCommands ? 1 : 0)
                   + (funcs.CreateSourceManager ? 1 : 0)
                   + (funcs.CreateMediaSourceFile ? 1 : 0);
        printf("  function pointers populated: %d/5\n", nValid);
        if (funcs.CreateClip) printf("    CreateClip       = 0x%p\n", funcs.CreateClip);
        if (funcs.CreateClipContainer) printf("    CreateClipContainer = 0x%p\n", funcs.CreateClipContainer);
        if (funcs.CreatePixCommands) printf("    CreatePixCommands  = 0x%p\n", funcs.CreatePixCommands);
        if (funcs.CreateSourceManager) printf("    CreateSourceManager = 0x%p\n", funcs.CreateSourceManager);
        if (funcs.CreateMediaSourceFile) printf("    CreateMediaSourceFile=0x%p\n", funcs.CreateMediaSourceFile);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  => SEH 0x%08X\n", GetExceptionCode());
    }

    __try {
        HRESULT hr = fn(NULL);
        print_hr("NULL pointer", hr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  NULL => SEH 0x%08X\n", GetExceptionCode());
    }
}

static void test_thumbnail_generator(FnVMGShellThumbnailGeneratorW fn) {
    printf("\n--- VMGShellThumbnailGeneratorW ---\n");
    if (!fn) { printf("  MISSING\n"); return; }
    printf("  ptr=0x%p\n", fn);

    __try {
        fn(L"test_input.jpg", L"test_output.jpg", NULL);
        printf("  => returned (test args, no crash)\n");
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  test args => SEH 0x%08X\n", GetExceptionCode());
    }

    __try {
        fn(L"", L"", NULL);
        printf("  => returned (empty paths, no crash)\n");
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  empty paths => SEH 0x%08X\n", GetExceptionCode());
    }

    __try {
        fn(NULL, NULL, NULL);
        printf("  => returned (NULL paths, no crash)\n");
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  NULL paths => SEH 0x%08X\n", GetExceptionCode());
    }
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    SetDllDirectoryA("undecomp\\Photo Gallery");
    printf("=== wlxclip.dll Dedicated Test Harness ===\n\n");

    HMODULE hMod = LoadLibraryA("WLXCLIP.DLL");
    if (!hMod) {
        printf("FAILED to load wlxclip.dll: error %lu\n", GetLastError());
        return 1;
    }
    printf("DLL loaded at 0x%p\n\n", (void*)hMod);

    FARPROC fpDllCanUnloadNow         = GetProcAddress(hMod, "DllCanUnloadNow");
    FARPROC fpDllGetClassObject       = GetProcAddress(hMod, "DllGetClassObject");
    FARPROC fpDllRegisterServer       = GetProcAddress(hMod, "DllRegisterServer");
    FARPROC fpDllUnregisterServer     = GetProcAddress(hMod, "DllUnregisterServer");
    FARPROC fpVMGGetClipCreateFunctions = GetProcAddress(hMod, "VMGGetClipCreateFunctions");
    FARPROC fpVMGShellThumbnailGeneratorW = GetProcAddress(hMod, "VMGShellThumbnailGeneratorW");

    printf("=== Export Resolution ===\n");
    printf("  %-40s %s\n", "DllCanUnloadNow",             fpDllCanUnloadNow ? "OK" : "MISSING");
    printf("  %-40s %s\n", "DllGetClassObject",           fpDllGetClassObject ? "OK" : "MISSING");
    printf("  %-40s %s\n", "DllRegisterServer",           fpDllRegisterServer ? "OK" : "MISSING");
    printf("  %-40s %s\n", "DllUnregisterServer",         fpDllUnregisterServer ? "OK" : "MISSING");
    printf("  %-40s %s\n", "VMGGetClipCreateFunctions",   fpVMGGetClipCreateFunctions ? "OK" : "MISSING");
    printf("  %-40s %s\n", "VMGShellThumbnailGeneratorW", fpVMGShellThumbnailGeneratorW ? "OK" : "MISSING");

    test_can_unload((FnDllCanUnloadNow)fpDllCanUnloadNow);
    test_get_class_object((FnDllGetClassObject)fpDllGetClassObject);
    test_register_server((FnDllRegisterServer)fpDllRegisterServer);
    test_unregister_server((FnDllUnregisterServer)fpDllUnregisterServer);
    test_get_clip_create_functions((FnVMGGetClipCreateFunctions)fpVMGGetClipCreateFunctions);
    test_thumbnail_generator((FnVMGShellThumbnailGeneratorW)fpVMGShellThumbnailGeneratorW);

    printf("\n--- COM Initialized DllGetClassObject ---\n");
    HRESULT hrCo = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hrCo)) {
        IUnknown* pUnkCom = NULL;
        __try {
            HRESULT hr = ((FnDllGetClassObject)fpDllGetClassObject)(CLSID_Empty, IID_IUnknown_Cast, (void**)&pUnkCom);
            print_hr("after CoInit, null CLSID", hr);
            if (pUnkCom) { pUnkCom->Release(); pUnkCom = NULL; }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  after CoInit => SEH 0x%08X\n", GetExceptionCode());
        }
        CoUninitialize();
    } else {
        printf("  CoInitializeEx failed: 0x%08X\n", hrCo);
    }

    printf("\n=== All Tests Complete ===\n");
    FreeLibrary(hMod);
    return 0;
}
