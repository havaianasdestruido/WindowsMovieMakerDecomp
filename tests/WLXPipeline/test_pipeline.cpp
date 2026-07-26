#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objbase.h>
#include <psapi.h>
#include <cstdio>
#include <cstdlib>

// ============================================================================
// Typedefs
// ============================================================================

// Reversed from disassembly: 3-param __stdcall
// HRESULT __stdcall GetPipelineCreateFunctions(DWORD dwVersion, DWORD dwStructSize, void* pOutBuffer)
typedef HRESULT (WINAPI *pfnGetPipelineCreateFunctions)(DWORD dwVersion, DWORD dwStructSize, void* pOutBuffer);
typedef HRESULT (WINAPI *pfnDllRegisterServer)(void);
typedef void*    (WINAPI *pfnVMGGetClipCreateFunctions)(void);

#define NUM_PIPELINE_CREATE_FUNCS 7
#define PIPELINE_CREATE_STRUCT_SIZE 0x1C  // 7 * sizeof(DWORD)

// ============================================================================
// Helpers
// ============================================================================

#define LOG(fmt, ...) printf("  " fmt "\n", ##__VA_ARGS__)
#define LOG_HR(hr, label) printf("  %-50s HRESULT=0x%08X (%s)\n", label, (unsigned)(hr), \
    (hr) == S_OK ? "S_OK" : (hr) == E_POINTER ? "E_POINTER" : \
    (hr) == E_FAIL ? "E_FAIL" : "OTHER")

static void LogVtable(void* pObj, int entries) {
    if (!pObj) return;
    void** vtable = *(void***)pObj;
    LOG("  vtable @ %p", (void*)vtable);
    for (int i = 0; i < entries; i++) {
        MEMORY_BASIC_INFORMATION mbi;
        VOID* addr = vtable[i];
        const char* module = "?";
        if (VirtualQuery(addr, &mbi, sizeof(mbi)) && mbi.AllocationBase) {
            char modPath[MAX_PATH];
            if (GetModuleFileNameA((HMODULE)mbi.AllocationBase, modPath, MAX_PATH)) {
                char* slash = strrchr(modPath, '\\');
                module = slash ? slash + 1 : modPath;
            }
        }
        LOG("    [%2d] %p  (%s)", i, addr, module);
    }
}

static void DumpModuleInfo(HMODULE hDll) {
    printf("\n[MODULE INFO]\n");
    MODULEINFO modInfo;
    if (GetModuleInformation(GetCurrentProcess(), hDll, &modInfo, sizeof(modInfo))) {
        LOG("  Base:    %p", modInfo.lpBaseOfDll);
        LOG("  Size:    0x%X (%u KB)", (unsigned)modInfo.SizeOfImage, (unsigned)(modInfo.SizeOfImage / 1024));
        LOG("  Entry:   %p", modInfo.EntryPoint);
    }
    WCHAR szPath[MAX_PATH];
    DWORD len = GetModuleFileNameW(hDll, szPath, MAX_PATH);
    if (len > 0) {
        char pathA[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, szPath, -1, pathA, MAX_PATH, NULL, NULL);
        LOG("  Path:    %s", pathA);
    }
}

static void DumpExports(HMODULE hDll) {
    printf("\n[EXPORTS]\n");
    const char* names[] = {
        "GetPipelineCreateFunctions",
        "DllRegisterServer"
    };
    for (const char* name : names) {
        FARPROC addr = GetProcAddress(hDll, name);
        if (addr) {
            LOG("  %-35s -> %p (RVA=0x%X)", name, (void*)addr,
                (unsigned)((ULONG_PTR)addr - (ULONG_PTR)hDll));
        } else {
            LOG("  %-35s -> NOT FOUND", name);
        }
    }
}

// ============================================================================
// Test: GetPipelineCreateFunctions (disassembly-confirmed signature)
// ============================================================================

static void Test_GetPipelineCreateFunctions(pfnGetPipelineCreateFunctions fn) {
    printf("\n[TEST] GetPipelineCreateFunctions (3-param __stdcall)\n");
    printf("  Signature: HRESULT fn(DWORD dwVersion, DWORD dwStructSize, void* pOutBuffer)\n");
    printf("  dwVersion must be 1, dwStructSize must be 0x1C (28)\n");
    printf("  Output: 7 DWORD function pointers (pipeline stage creators)\n\n");

    // Validate E_POINTER when third param is NULL
    {
        printf("  --- Validation tests ---\n");
        HRESULT hr = fn(1, PIPELINE_CREATE_STRUCT_SIZE, NULL);
        LOG_HR(hr, "fn(1, 0x1C, NULL) -> should be E_POINTER");
    }
    {
        HRESULT hr = fn(0, PIPELINE_CREATE_STRUCT_SIZE, (void*)1);
        LOG_HR(hr, "fn(0, 0x1C, ptr) -> should be E_FAIL (version != 1)");
    }
    {
        HRESULT hr = fn(1, 0x10, (void*)1);
        LOG_HR(hr, "fn(1, 0x10, ptr) -> should be E_FAIL (size != 0x1C)");
    }

    // Actual call with valid params
    printf("\n  --- Actual factory retrieval ---\n");
    DWORD createFuncs[NUM_PIPELINE_CREATE_FUNCS];
    memset(createFuncs, 0, sizeof(createFuncs));

    __try {
        HRESULT hr = fn(1, PIPELINE_CREATE_STRUCT_SIZE, createFuncs);
        LOG_HR(hr, "fn(1, 0x1C, &createFuncs[7])");

        if (SUCCEEDED(hr)) {
            printf("\n  7 pipeline creation functions retrieved:\n");
            for (int i = 0; i < NUM_PIPELINE_CREATE_FUNCS; i++) {
                void* addr = (void*)createFuncs[i];
                MEMORY_BASIC_INFORMATION mbi;
                const char* module = "?";
                DWORD rva = createFuncs[i] - (DWORD)(ULONG_PTR)GetModuleHandle(NULL);

                // Check if it's within our DLL
                HMODULE hDll = GetModuleHandleA("WLXPipeline.dll");
                if (hDll && createFuncs[i] >= (DWORD)(ULONG_PTR)hDll &&
                    createFuncs[i] < (DWORD)(ULONG_PTR)hDll + 0xB2000) {
                    rva = createFuncs[i] - (DWORD)(ULONG_PTR)hDll;
                    module = "WLXPipeline.dll";
                }

                LOG("  [%d] func_ptr=%p  RVA=0x%X  module=%s", i, addr, rva, module);

                if (addr && addr != (void*)0xCCCCCCCC) {
                    printf("      Probing func[%d] signature...\n", i);
                    __try {
                        void* p1 = NULL;
                        DWORD dwOut = 0;
                        HRESULT hrProbe = ((HRESULT(WINAPI*)(void*, DWORD*))addr)(NULL, NULL);
                        LOG("      fn(NULL, NULL) = 0x%08X", (unsigned)hrProbe);

                        hrProbe = ((HRESULT(WINAPI*)(void*, DWORD*))addr)(&p1, &dwOut);
                        LOG("      fn(&p1, &dwOut) = 0x%08X (p1=%p, dwOut=%u)",
                            (unsigned)hrProbe, p1, (unsigned)dwOut);

                        if (SUCCEEDED(hrProbe) && p1) {
                            LogVtable(p1, 8);
                        }
                    } __except(EXCEPTION_EXECUTE_HANDLER) {
                        printf("      EXCEPTION 0x%08X\n", GetExceptionCode());
                    }
                }
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  EXCEPTION 0x%08X in main call\n", GetExceptionCode());
    }
}

// ============================================================================
// Test: DllRegisterServer (probe only, don't execute)
// ============================================================================

static void Test_DllRegisterServer(pfnDllRegisterServer fn) {
    printf("\n[TEST] DllRegisterServer (skipped - would modify registry)\n");
}

// ============================================================================
// Test: Sibling Pipeline.dll
// ============================================================================

static void Test_SiblingPipelineDll() {
    printf("\n[TEST] Sibling Pipeline.dll\n");
    HMODULE hPipeline = LoadLibraryW(L"Pipeline.dll");
    if (!hPipeline) {
        LOG("  Pipeline.dll not found (error %lu) - expected", GetLastError());
    } else {
        LOG("  Pipeline.dll loaded at %p", (void*)hPipeline);
        FARPROC pCreate = GetProcAddress(hPipeline, "GetPipelineCreateFunctions");
        FARPROC pVMG = GetProcAddress(hPipeline, "VMGGetClipCreateFunctions");
        LOG("  GetPipelineCreateFunctions: %p", (void*)pCreate);
        LOG("  VMGGetClipCreateFunctions:  %p", (void*)pVMG);

        if (pVMG) {
            printf("\n  Calling VMGGetClipCreateFunctions()...\n");
            __try {
                void* result = ((pfnVMGGetClipCreateFunctions)pVMG)();
                LOG("  result = %p", result);
                if (result) {
                    LogVtable(result, 12);
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  EXCEPTION 0x%08X\n", GetExceptionCode());
            }
        }
        FreeLibrary(hPipeline);
    }
}

// ============================================================================
// Test: Dynamic DLLs
// ============================================================================

static void Test_DynamicDxva2() {
    printf("\n[TEST] Dynamic dxva2.dll load\n");
    HMODULE hDxva2 = LoadLibraryW(L"dxva2.dll");
    if (!hDxva2) {
        LOG("  dxva2.dll not available (error %lu)", GetLastError());
    } else {
        LOG("  dxva2.dll loaded at %p", (void*)hDxva2);
        FARPROC pCreateVideoService = GetProcAddress(hDxva2, "DXVA2CreateVideoService");
        LOG("  DXVA2CreateVideoService: %p", (void*)pCreateVideoService);
        FreeLibrary(hDxva2);
    }
}

static void Test_DynamicWindowsCodecs() {
    printf("\n[TEST] Dynamic WindowsCodecs.dll (WIC)\n");
    HMODULE hWic = LoadLibraryW(L"WindowsCodecs.dll");
    if (!hWic) {
        LOG("  WindowsCodecs.dll not available (error %lu)", GetLastError());
    } else {
        LOG("  WindowsCodecs.dll loaded at %p", (void*)hWic);
        FARPROC pConvert = GetProcAddress(hWic, "WICConvertBitmapSource");
        LOG("  WICConvertBitmapSource: %p", (void*)pConvert);
        FreeLibrary(hWic);
    }
}

// ============================================================================
// Test: Registry
// ============================================================================

static void Test_RegistryCheck() {
    printf("\n[TEST] Registry CLSID Check\n");
    const char* clsid = "{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}";
    char keyPath[256];
    snprintf(keyPath, sizeof(keyPath), "CLSID\\%s\\InprocServer32", clsid);

    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CLASSES_ROOT, keyPath, 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        char value[MAX_PATH];
        DWORD valueSize = sizeof(value);
        DWORD type = 0;
        result = RegQueryValueExA(hKey, NULL, NULL, &type, (LPBYTE)value, &valueSize);
        if (result == ERROR_SUCCESS) {
            LOG("  CLSID_VideoTrim InprocServer32: %s", value);
        }
        char threadModel[64];
        DWORD tmSize = sizeof(threadModel);
        result = RegQueryValueExA(hKey, "ThreadingModel", NULL, &type, (LPBYTE)threadModel, &tmSize);
        if (result == ERROR_SUCCESS) {
            LOG("  ThreadingModel: %s", threadModel);
        }
        RegCloseKey(hKey);
    } else {
        LOG("  CLSID_VideoTrim NOT registered (error %ld)", result);
    }

    const char* gpuKey = "Software\\Microsoft\\GPUPipeline";
    result = RegOpenKeyExA(HKEY_CURRENT_USER, gpuKey, 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        LOG("  GPUPipeline key exists");
        RegCloseKey(hKey);
    } else {
        LOG("  GPUPipeline key not present");
    }

    const char* wlxAbove = "Software\\Microsoft\\Windows Live\\Photo Gallery\\WLXGPUPipeline";
    result = RegOpenKeyExA(HKEY_CURRENT_USER, wlxAbove, 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        LOG("  WLXGPUPipeline key exists");
        RegCloseKey(hKey);
    } else {
        LOG("  WLXGPUPipeline key not present");
    }
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char* argv[]) {
    printf("=========================================================\n");
    printf("  WLXPipeline.dll - Reverse Engineering Test Harness\n");
    printf("  D3D9 Proxy Layer + Pipeline Factory Probing\n");
    printf("=========================================================\n");

    const char* dllPath = (argc > 1) ? argv[1] : "..\\..\\undecomp\\Photo Gallery\\WLXPipeline.dll";
    printf("\nLoading: %s\n", dllPath);

    HMODULE hDll = LoadLibraryA(dllPath);
    if (!hDll) {
        printf("FATAL: Failed to load DLL (error %lu)\n", GetLastError());
        return 1;
    }
    printf("Loaded at: %p\n\n", (void*)hDll);

    DumpModuleInfo(hDll);
    DumpExports(hDll);

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    pfnGetPipelineCreateFunctions pGetPipeline =
        (pfnGetPipelineCreateFunctions)GetProcAddress(hDll, "GetPipelineCreateFunctions");
    pfnDllRegisterServer pRegister =
        (pfnDllRegisterServer)GetProcAddress(hDll, "DllRegisterServer");

    if (!pGetPipeline) {
        printf("\nFATAL: GetPipelineCreateFunctions not found\n");
        FreeLibrary(hDll);
        return 1;
    }

    Test_DllRegisterServer(pRegister);
    Test_GetPipelineCreateFunctions(pGetPipeline);
    Test_SiblingPipelineDll();
    Test_DynamicDxva2();
    Test_DynamicWindowsCodecs();
    Test_RegistryCheck();

    CoUninitialize();
    FreeLibrary(hDll);

    printf("\n=========================================================\n");
    printf("  Done.\n");
    printf("=========================================================\n");
    return 0;
}
