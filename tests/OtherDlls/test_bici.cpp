// DmxBici.dll Test Harness - BICI telemetry wrapper
// Loads DLL, resolves all 19 exports with SEH protection
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdlib>

// ============================================================
// Helpers
// ============================================================
static void SetSearchPaths() {
    wchar_t galleryDir[MAX_PATH], mainDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
    GetFullPathNameW(L".", MAX_PATH, mainDir, NULL);
    AddDllDirectory(galleryDir);
    AddDllDirectory(mainDir);
    SetDllDirectoryW(galleryDir);
}

static HMODULE SafeLoadDll(const wchar_t* relPath) {
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(relPath, MAX_PATH, fullPath, NULL);
    printf("[*] Loading: %ls\n", fullPath);
    HMODULE h = LoadLibraryExW(fullPath, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!h) {
        printf("[!] LoadLibraryEx failed (err %u), retrying SetDllDirectory\n", GetLastError());
        wchar_t galleryDir[MAX_PATH];
        GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
        SetDllDirectoryW(galleryDir);
        h = LoadLibraryW(fullPath);
    }
    if (h)
        printf("[+] Loaded at base: 0x%p\n", h);
    else
        printf("[!] FAILED to load: error %u\n", GetLastError());
    return h;
}

// ============================================================
// Export typedefs - all __stdcall (__stdcall = VC++ __stdcall)
// BiciWrapper is a static class (all methods are static)
// ============================================================

// Ordinal 1: bool __stdcall AddStringToDataPoint(DWORD key, DWORD subKey, const wchar_t* str)
typedef BOOL (__stdcall *AddStringToDataPointFunc)(DWORD, DWORD, const wchar_t*);

// Ordinal 2: bool __stdcall AddToAverage(DWORD key, DWORD value)
typedef BOOL (__stdcall *AddToAverageFunc)(DWORD, DWORD);

// Ordinal 3: bool __stdcall AddToDataPoint(DWORD key, DWORD subKey, DWORD value)
typedef BOOL (__stdcall *AddToDataPointFunc)(DWORD, DWORD, DWORD);

// Ordinal 4: void __stdcall AddToStream(DWORD key, const void* tuple)
// Tuple is an internal type; passing NULL is safe
typedef void (__stdcall *AddToStreamFunc)(DWORD, const void*);

// Ordinal 5: long __stdcall EndExperience()
typedef LONG (__stdcall *EndExperienceFunc)();

// Ordinal 6: bool __stdcall Increment(DWORD key, DWORD value)
typedef BOOL (__stdcall *IncrementFunc)(DWORD, DWORD);

// Ordinal 7: bool __stdcall Set(DWORD key, DWORD value)
typedef BOOL (__stdcall *SetFunc)(DWORD, DWORD);

// Ordinal 8: long __stdcall SetAnid(const wchar_t* anid)
typedef LONG (__stdcall *SetAnidFunc)(const wchar_t*);

// Ordinal 9: bool __stdcall SetIfMax(DWORD key, DWORD value)
typedef BOOL (__stdcall *SetIfMaxFunc)(DWORD, DWORD);

// Ordinal 10: bool __stdcall SetIfMin(DWORD key, DWORD value)
typedef BOOL (__stdcall *SetIfMinFunc)(DWORD, DWORD);

// Ordinal 11: bool __stdcall SetString(DWORD key, const wchar_t* str)
typedef BOOL (__stdcall *SetStringFunc)(DWORD, const wchar_t*);

// Ordinal 12: long __stdcall StartExperience(BiciStartupId id)
// BiciStartupId is an enum; pass 0 as safe default
typedef LONG (__stdcall *StartExperienceByIdFunc)(DWORD);

// Ordinal 13: long __stdcall StartExperience() [overload, no params]
typedef LONG (__stdcall *StartExperienceFunc)();

// Ordinal 14: bool __stdcall TimerAccumulate(DWORD timerId)
typedef BOOL (__stdcall *TimerAccumulateFunc)(DWORD);

// Ordinal 15: bool __stdcall TimerRecord(DWORD timerId)
typedef BOOL (__stdcall *TimerRecordFunc)(DWORD);

// Ordinal 16: bool __stdcall TimerStart(DWORD timerId)
typedef BOOL (__stdcall *TimerStartFunc)(DWORD);

// Ordinal 17: bool __stdcall TransferExperienceToApp(wchar_t*** outPtr)
typedef BOOL (__stdcall *TransferExperienceToAppFunc)(wchar_t***);

// Ordinal 18: bool __stdcall TransferExperienceToAppId(DWORD appId)
typedef BOOL (__stdcall *TransferExperienceToAppIdFunc)(DWORD);

// Ordinal 19: bool __stdcall TransferExperienceToWeb(const wchar_t* url, wchar_t*** outPtr)
typedef BOOL (__stdcall *TransferExperienceToWebFunc)(const wchar_t*, wchar_t***);

// ============================================================
// Probing helpers
// ============================================================
static int g_passCount = 0;
static int g_failCount = 0;

static void ProbeBoolFunc(const char* name, const char* exportName, void* fn, BOOL result, LONG sehCode) {
    if (sehCode != 0) {
        printf("    [%s] SEH exception 0x%08X\n", name, sehCode);
        g_failCount++;
    } else {
        printf("    [%s] returned %s (0x%08X)\n", name, result ? "TRUE" : "FALSE", result);
        g_passCount++;
    }
}

static void ProbeLongFunc(const char* name, const char* exportName, void* fn, LONG result, LONG sehCode) {
    if (sehCode != 0) {
        printf("    [%s] SEH exception 0x%08X\n", name, sehCode);
        g_failCount++;
    } else {
        printf("    [%s] returned %d (0x%08X)\n", name, result, result);
        g_passCount++;
    }
}

#define PROBE_BOOL(h, funcType, funcName, exportName, ...) \
do { \
    funcType fn = (funcType)GetProcAddress(h, exportName); \
    if (fn) { printf("\n[*] %s => 0x%p\n", exportName, fn); \
        LONG _seh = 0; BOOL _result = FALSE; \
        __try { _result = fn(__VA_ARGS__); } \
        __except(EXCEPTION_EXECUTE_HANDLER) { _seh = GetExceptionCode(); } \
        ProbeBoolFunc(funcName, exportName, (void*)fn, _result, _seh); \
    } else { printf("\n[*] %s => NOT FOUND\n", exportName); } \
} while(0)

#define PROBE_LONG(h, funcType, funcName, exportName, ...) \
do { \
    funcType fn = (funcType)GetProcAddress(h, exportName); \
    if (fn) { printf("\n[*] %s => 0x%p\n", exportName, fn); \
        LONG _seh = 0; LONG _result = 0; \
        __try { _result = fn(__VA_ARGS__); } \
        __except(EXCEPTION_EXECUTE_HANDLER) { _seh = GetExceptionCode(); } \
        ProbeLongFunc(funcName, exportName, (void*)fn, _result, _seh); \
    } else { printf("\n[*] %s => NOT FOUND\n", exportName); } \
} while(0)

#define PROBE_VOID(h, funcType, funcName, exportName, ...) \
do { \
    funcType fn = (funcType)GetProcAddress(h, exportName); \
    if (fn) { printf("\n[*] %s => 0x%p\n", exportName, fn); \
        LONG _seh = 0; \
        __try { fn(__VA_ARGS__); } \
        __except(EXCEPTION_EXECUTE_HANDLER) { _seh = GetExceptionCode(); } \
        if (_seh != 0) { printf("    [%s] SEH exception 0x%08X\n", funcName, _seh); g_failCount++; } \
        else { printf("    [%s] completed (void)\n", funcName); g_passCount++; } \
    } else { printf("\n[*] %s => NOT FOUND\n", exportName); } \
} while(0)

// ============================================================
// Main
// ============================================================
int main() {
    SetSearchPaths();

    printf("========================================\n");
    printf("  DmxBici.dll - BICI Telemetry Probing\n");
    printf("========================================\n\n");

    HMODULE h = SafeLoadDll(L"undecomp\\Photo Gallery\\DmxBici.dll");
    if (!h) {
        printf("[FATAL] Cannot load DmxBici.dll\n");
        return 1;
    }

    // Export count verification
    printf("\n[*] Verifying export count...\n");
    DWORD exportCount = 0;
    {
        __try {
            IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)h;
            IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)h + dos->e_lfanew);
            IMAGE_DATA_DIRECTORY* expDir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
            if (expDir->Size > 0) {
                IMAGE_EXPORT_DIRECTORY* exports = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)h + expDir->VirtualAddress);
                exportCount = exports->NumberOfFunctions;
                printf("    Export count: %u\n", exportCount);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("    [!] Exception reading export table\n");
        }
    }

    // RTTI class check
    printf("\n[*] RTTI classes in .data:\n");
    printf("    .?AVtype_info@@ (type_info)\n");
    printf("    .?AVException@Base@@ (from WLXPhotoBase.dll)\n");

    // Probe all 19 exports
    printf("\n========================================\n");
    printf("  Export Probing (all SEH-protected)\n");
    printf("========================================\n");

    // Ordinal 1: AddStringToDataPoint(key=1, subKey=1, str=L"test")
    PROBE_BOOL(h, AddStringToDataPointFunc, "AddStringToDataPoint",
        "?AddStringToDataPoint@BiciWrapper@@YG_NKKPB_W@Z", 1, 1, L"test");

    // Ordinal 2: AddToAverage(key=1, value=100)
    PROBE_BOOL(h, AddToAverageFunc, "AddToAverage",
        "?AddToAverage@BiciWrapper@@YG_NKK@Z", 1, 100);

    // Ordinal 3: AddToDataPoint(key=1, subKey=1, value=42)
    PROBE_BOOL(h, AddToDataPointFunc, "AddToDataPoint",
        "?AddToDataPoint@BiciWrapper@@YG_NKKK@Z", 1, 1, 42);

    // Ordinal 4: AddToStream(key=1, tuple=NULL)
    PROBE_VOID(h, AddToStreamFunc, "AddToStream",
        "?AddToStream@BiciWrapper@@YGXKPBVTuple@1@@Z", 1, (const void*)NULL);

    // Ordinal 13: StartExperience() - no-arg overload
    PROBE_LONG(h, StartExperienceFunc, "StartExperience()",
        "?StartExperience@BiciWrapper@@YGJXZ");

    // Ordinal 12: StartExperience(BiciStartupId=0)
    PROBE_LONG(h, StartExperienceByIdFunc, "StartExperience(id=0)",
        "?StartExperience@BiciWrapper@@YGJW4BiciStartupId@1@@Z", 0);

    // Ordinal 5: EndExperience()
    PROBE_LONG(h, EndExperienceFunc, "EndExperience",
        "?EndExperience@BiciWrapper@@YGJXZ");

    // Ordinal 17: TransferExperienceToApp(outPtr=NULL)
    PROBE_BOOL(h, TransferExperienceToAppFunc, "TransferExperienceToApp",
        "?TransferExperienceToApp@BiciWrapper@@YG_NPAPA_W@Z", (wchar_t***)NULL);

    // Ordinal 18: TransferExperienceToAppId(appId=0)
    PROBE_BOOL(h, TransferExperienceToAppIdFunc, "TransferExperienceToAppId",
        "?TransferExperienceToAppId@BiciWrapper@@YG_NK@Z", (DWORD)0);

    // Ordinal 19: TransferExperienceToWeb(url=L"", outPtr=NULL)
    PROBE_BOOL(h, TransferExperienceToWebFunc, "TransferExperienceToWeb",
        "?TransferExperienceToWeb@BiciWrapper@@YG_NPB_WPAPA_W@Z", L"", (wchar_t***)NULL);

    // Ordinal 8: SetAnid(anid=L"test-anid")
    PROBE_LONG(h, SetAnidFunc, "SetAnid",
        "?SetAnid@BiciWrapper@@YGJPB_W@Z", L"test-anid");

    // Ordinal 7: Set(key=1, value=99)
    PROBE_BOOL(h, SetFunc, "Set",
        "?Set@BiciWrapper@@YG_NKK@Z", 1, 99);

    // Ordinal 11: SetString(key=1, str=L"hello")
    PROBE_BOOL(h, SetStringFunc, "SetString",
        "?SetString@BiciWrapper@@YG_NKPB_W@Z", 1, L"hello");

    // Ordinal 9: SetIfMax(key=1, value=200)
    PROBE_BOOL(h, SetIfMaxFunc, "SetIfMax",
        "?SetIfMax@BiciWrapper@@YG_NKK@Z", 1, 200);

    // Ordinal 10: SetIfMin(key=1, value=10)
    PROBE_BOOL(h, SetIfMinFunc, "SetIfMin",
        "?SetIfMin@BiciWrapper@@YG_NKK@Z", 1, 10);

    // Ordinal 6: Increment(key=1, value=1)
    PROBE_BOOL(h, IncrementFunc, "Increment",
        "?Increment@BiciWrapper@@YG_NKK@Z", 1, 1);

    // Ordinal 14: TimerAccumulate(timerId=1)
    PROBE_BOOL(h, TimerAccumulateFunc, "TimerAccumulate",
        "?TimerAccumulate@BiciWrapper@@YG_NK@Z", 1);

    // Ordinal 15: TimerRecord(timerId=1)
    PROBE_BOOL(h, TimerRecordFunc, "TimerRecord",
        "?TimerRecord@BiciWrapper@@YG_NK@Z", 1);

    // Ordinal 16: TimerStart(timerId=1)
    PROBE_BOOL(h, TimerStartFunc, "TimerStart",
        "?TimerStart@BiciWrapper@@YG_NK@Z", 1);

    // Summary
    printf("\n========================================\n");
    printf("  Summary: %d passed, %d failed\n", g_passCount, g_failCount);
    printf("  Exports in DLL: %u\n", exportCount);
    printf("========================================\n");

    // Enumerate all 19 exports by name via export table walk
    printf("\n[*] Full export enumeration:\n");
    {
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)h;
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)h + dos->e_lfanew);
        IMAGE_DATA_DIRECTORY* expDir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        if (expDir->Size > 0) {
            IMAGE_EXPORT_DIRECTORY* exports = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)h + expDir->VirtualAddress);
            DWORD* names = (DWORD*)((BYTE*)h + exports->AddressOfNames);
            WORD* ordinals = (WORD*)((BYTE*)h + exports->AddressOfNameOrdinals);
            DWORD* funcs = (DWORD*)((BYTE*)h + exports->AddressOfFunctions);
            for (DWORD i = 0; i < exports->NumberOfNames; i++) {
                const char* name = (const char*)((BYTE*)h + names[i]);
                WORD ord = ordinals[i];
                DWORD rva = funcs[ord];
                printf("    [%3u] Ordinal %2u  RVA=0x%05X  %s\n",
                    i, ord + exports->Base, rva, name);
            }
        }
    }

    FreeLibrary(h);
    return g_failCount;
}
