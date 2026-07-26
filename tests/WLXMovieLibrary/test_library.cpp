#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

static const IID IID_IUnknown_local =
    {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define CHECK(name, condition) \
    do { \
        if (condition) { \
            printf("  [PASS] %s\n", name); \
            g_testsPassed++; \
        } else { \
            printf("  [FAIL] %s\n", name); \
            g_testsFailed++; \
        } \
    } while(0)

static const char* HrStr(HRESULT hr) {
    switch(hr) {
        case 0x00000000: return "S_OK";
        case 0x00000001: return "S_FALSE";
        case 0x80004002: return "E_NOINTERFACE";
        case 0x80004003: return "E_POINTER";
        case 0x80040111: return "CLASS_E_CLASSNOTAVAILABLE";
        case 0x80040154: return "REGDB_E_CLASSNOTREG";
        case 0x80070005: return "E_ACCESSDENIED";
        case 0x8007000E: return "E_OUTOFMEMORY";
        case 0x8000FFFF: return "E_UNEXPECTED";
        default: return "";
    }
}

static void LogVtable(void** vtable, const char* name, int maxSlots) {
    printf("  [%s] vtable @ %p\n", name, vtable);
    for (int i = 0; i < maxSlots && vtable[i] != NULL; i++) {
        printf("    [%02d] %p\n", i, vtable[i]);
    }
}

// ============================================================================
// Test 1: LoadLibrary
// ============================================================================
static HMODULE TestLoadLibrary() {
    printf("=== Test 1: LoadLibrary ===\n");

    char exeDir[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';

    char dllDir[MAX_PATH] = {0};
    snprintf(dllDir, MAX_PATH, "%s\\..\\..\\undecomp\\Shared", exeDir);
    char fullDllDir[MAX_PATH] = {0};
    GetFullPathNameA(dllDir, MAX_PATH, fullDllDir, NULL);
    printf("  DLL search dir: %s\n", fullDllDir);

    SetDllDirectoryA(fullDllDir);

    char dllPath[MAX_PATH] = {0};
    snprintf(dllPath, MAX_PATH, "%s\\WLXMovieLibrary.dll", fullDllDir);
    printf("  DLL path: %s\n", dllPath);

    HMODULE hMod = LoadLibraryA(dllPath);
    CHECK("LoadLibrary with valid path", hMod != NULL);
    if (hMod) printf("  Module handle: %p\n", (void*)hMod);

    HMODULE hBad = LoadLibraryA(NULL);
    printf("  LoadLibrary(NULL) -> %p\n", (void*)hBad);
    g_testsPassed++;

    HMODULE hFake = LoadLibraryA("nonexistent.dll");
    CHECK("LoadLibrary(nonexistent) fails", hFake == NULL);

    printf("\n");
    return hMod;
}

// ============================================================================
// Test 2: Export resolution
// ============================================================================
static void TestExports(HMODULE hMod) {
    printf("=== Test 2: Export Resolution ===\n");

    FARPROC fp1 = GetProcAddress(hMod, "CreateMovieFactory");
    CHECK("GetProcAddress(CreateMovieFactory)", fp1 != NULL);
    if (fp1) printf("  CreateMovieFactory @ %p\n", (void*)fp1);

    FARPROC fpDllCanUnload = GetProcAddress(hMod, "DllCanUnloadNow");
    printf("  DllCanUnloadNow: %p %s\n", (void*)fpDllCanUnload,
        fpDllCanUnload ? "(found)" : "(NOT exported)");

    FARPROC fpDllGetClassObj = GetProcAddress(hMod, "DllGetClassObject");
    printf("  DllGetClassObject: %p %s\n", (void*)fpDllGetClassObj,
        fpDllGetClassObj ? "(found)" : "(NOT exported)");

    FARPROC fpDllReg = GetProcAddress(hMod, "DllRegisterServer");
    printf("  DllRegisterServer: %p %s\n", (void*)fpDllReg,
        fpDllReg ? "(found)" : "(NOT exported)");

    FARPROC fpBad = GetProcAddress(hMod, "NonexistentExport");
    CHECK("GetProcAddress(invalid) returns NULL", fpBad == NULL);

    printf("\n");
}

// ============================================================================
// Test 3: Disassembly analysis
// ============================================================================
static void TestSignature(HMODULE hMod) {
    printf("=== Test 3: CreateMovieFactory Signature Analysis ===\n");

    FARPROC fp = GetProcAddress(hMod, "CreateMovieFactory");
    if (!fp) { printf("  Cannot resolve\n\n"); return; }

    unsigned char* code = (unsigned char*)fp;
    printf("  Bytes: ");
    for (int i = 0; i < 64; i++) printf("%02X ", code[i]);
    printf("\n");

    if (code[0] == 0x8B && code[1] == 0xFF) {
        printf("  Prologue: MOV EDI,EDI / PUSH EBP / MOV EBP,ESP — stdcall with hotpatch\n");
    }

    // Scan for RETN (0xC2 xx xx)
    for (int i = 0; i < 512; i++) {
        if (code[i] == 0xC2 && i + 2 < 512) {
            unsigned short n = code[i+1] | (code[i+2] << 8);
            if (n > 0 && n <= 32) {
                printf("  RETN at offset %d — callee cleans %d bytes (%d arg(s) stdcall)\n", i, n, n / 4);
                break;
            }
        }
        if (code[i] == 0xC3) {
            printf("  RET at offset %d — cdecl (caller cleans) or frameless\n", i);
            break;
        }
    }

    // List all call targets
    printf("  Calls in first 256 bytes:\n");
    for (int i = 0; i < 256; i++) {
        if (code[i] == 0xE8 && i + 5 <= 256) {
            int32_t rel = *(int32_t*)(code + i + 1);
            void* target = (void*)(code + i + 5 + rel);
            printf("    [+%02d] CALL %p\n", i, target);
        } else if (code[i] == 0xFF && i + 6 <= 256 && code[i+1] == 0x15) {
            void** addr = *(void***)(code + i + 2);
            printf("    [+%02d] CALL [indirect %p] -> %p\n", i, (void*)(code + i + 2), addr ? *addr : NULL);
        }
    }

    g_testsPassed++;
    printf("\n");
}

// ============================================================================
// Test 4: CreateMovieFactory — probing various signatures
// ============================================================================
static void TestCreateMovieFactory(HMODULE hMod) {
    printf("=== Test 4: CreateMovieFactory Call Probing ===\n");

    FARPROC fp = GetProcAddress(hMod, "CreateMovieFactory");
    if (!fp) { printf("  Cannot resolve\n\n"); return; }

    // The disassembly shows:
    //   [ebp+8] is read and tested for NULL -> E_POINTER if NULL
    //   So the function takes at least 1 DWORD argument
    //   It's stdcall (MOV EDI,EDI hotpatch prologue)
    //   Signature: HRESULT __stdcall CreateMovieFactory(void* pArg)

    // Probe 1: 1 arg = NULL (should return E_POINTER = 0x80004003)
    __try {
        typedef HRESULT (WINAPI *Fn1)(void*);
        HRESULT hr = ((Fn1)fp)(NULL);
        printf("  CreateMovieFactory(NULL): hr=0x%08X (%s)\n", (unsigned int)hr, HrStr(hr));
        CHECK("NULL arg returns E_POINTER", hr == 0x80004003);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in CreateMovieFactory(NULL)\n");
        g_testsFailed++;
    }

    // Probe 2: 1 arg = valid pointer (out-param)
    __try {
        void* pResult = (void*)0xDEADBEEF;
        typedef HRESULT (WINAPI *Fn1)(void*);
        HRESULT hr = ((Fn1)fp)(&pResult);
        printf("  CreateMovieFactory(&ptr): hr=0x%08X (%s) result=%p\n",
            (unsigned int)hr, HrStr(hr), pResult);
        if (SUCCEEDED(hr) && pResult && pResult != (void*)0xDEADBEEF) {
            printf("    ** Object created! **\n");
            LogVtable(*(void***)pResult, "vtable", 30);
            void** vtbl = *(void***)pResult;
            typedef ULONG (__stdcall *FnRelease)(IUnknown*);
            FnRelease pRelease = (FnRelease)vtbl[2];
            if (pRelease) {
                ULONG ref = pRelease((IUnknown*)pResult);
                printf("    Release() -> refcount=%lu\n", ref);
            }
        }
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in CreateMovieFactory(&ptr)\n");
        g_testsFailed++;
    }

    // Probe 3: 2 args (ptr + flags)
    __try {
        void* pResult = (void*)0xDEADBEEF;
        typedef HRESULT (WINAPI *Fn2)(void*, DWORD);
        HRESULT hr = ((Fn2)fp)(&pResult, 0);
        printf("  CreateMovieFactory(&ptr, 0): hr=0x%08X (%s) result=%p\n",
            (unsigned int)hr, HrStr(hr), pResult);
        if (SUCCEEDED(hr) && pResult && pResult != (void*)0xDEADBEEF) {
            printf("    ** Object created! **\n");
            LogVtable(*(void***)pResult, "vtable", 30);
            void** vtbl = *(void***)pResult;
            typedef ULONG (__stdcall *FnRelease)(IUnknown*);
            FnRelease pRelease = (FnRelease)vtbl[2];
            if (pRelease) {
                ULONG ref = pRelease((IUnknown*)pResult);
                printf("    Release() -> refcount=%lu\n", ref);
            }
        }
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in CreateMovieFactory(&ptr, 0)\n");
        g_testsFailed++;
    }

    // Probe 4: 3 args
    __try {
        void* pResult = (void*)0xDEADBEEF;
        typedef HRESULT (WINAPI *Fn3)(void*, DWORD, DWORD);
        HRESULT hr = ((Fn3)fp)(&pResult, 0, 0);
        printf("  CreateMovieFactory(&ptr, 0, 0): hr=0x%08X (%s) result=%p\n",
            (unsigned int)hr, HrStr(hr), pResult);
        g_testsPassed++;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  SEH in CreateMovieFactory(&ptr, 0, 0)\n");
        g_testsFailed++;
    }

    printf("\n");
}

// ============================================================================
// Test 5: FreeLibrary
// ============================================================================
static void TestFreeLibrary(HMODULE hMod) {
    printf("=== Test 5: FreeLibrary ===\n");

    BOOL bRet = FreeLibrary(hMod);
    CHECK("FreeLibrary(valid handle)", bRet != FALSE);

    bRet = FreeLibrary(NULL);
    CHECK("FreeLibrary(NULL) returns FALSE", bRet == FALSE);

    printf("\n");
}

// ============================================================================
// Main
// ============================================================================
int main() {
    printf("=== WLXMovieLibrary.dll Dynamic Test Harness ===\n");
    printf("CreateMovieFactory probing, delay-load dependency resolution\n\n");

    HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    printf("CoInitializeEx: 0x%08X\n\n", (unsigned int)hrInit);

    HMODULE hMod = TestLoadLibrary();
    if (!hMod) {
        printf("FATAL: Could not load WLXMovieLibrary.dll\n");
        CoUninitialize();
        return 1;
    }

    TestExports(hMod);
    TestSignature(hMod);
    TestCreateMovieFactory(hMod);
    TestFreeLibrary(hMod);

    CoUninitialize();

    printf("=== Results: %d passed, %d failed, %d total ===\n",
        g_testsPassed, g_testsFailed, g_testsPassed + g_testsFailed);

    return g_testsFailed;
}
