#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdlib>

// Relative to tests/DmxBici/
static const wchar_t* DLL_RELPATH = L"..\\..\\undecomp\\Photo Gallery\\DmxBici.dll";

static void SetSearchPaths() {
    wchar_t galleryDir[MAX_PATH], mainDir[MAX_PATH];
    GetFullPathNameW(L"..\\..\\undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
    GetFullPathNameW(L"..\\..", MAX_PATH, mainDir, NULL);
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
        GetFullPathNameW(L"..\\..\\undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
        SetDllDirectoryW(galleryDir);
        h = LoadLibraryW(fullPath);
    }
    if (h)
        printf("[+] Loaded at base: 0x%p\n", h);
    else
        printf("[!] FAILED to load: error %u\n", GetLastError());
    return h;
}

// Export typedefs
typedef BOOL  (__stdcall *AddStringToDataPointFunc)(DWORD, DWORD, const wchar_t*);
typedef BOOL  (__stdcall *AddToAverageFunc)(DWORD, DWORD);
typedef BOOL  (__stdcall *AddToDataPointFunc)(DWORD, DWORD, DWORD);
typedef void  (__stdcall *AddToStreamFunc)(DWORD, const void*);
typedef LONG  (__stdcall *EndExperienceFunc)();
typedef BOOL  (__stdcall *IncrementFunc)(DWORD, DWORD);
typedef BOOL  (__stdcall *SetFunc)(DWORD, DWORD);
typedef LONG  (__stdcall *SetAnidFunc)(const wchar_t*);
typedef BOOL  (__stdcall *SetIfMaxFunc)(DWORD, DWORD);
typedef BOOL  (__stdcall *SetIfMinFunc)(DWORD, DWORD);
typedef BOOL  (__stdcall *SetStringFunc)(DWORD, const wchar_t*);
typedef LONG  (__stdcall *StartExperienceByIdFunc)(DWORD);
typedef LONG  (__stdcall *StartExperienceFunc)();
typedef BOOL  (__stdcall *TimerAccumulateFunc)(DWORD);
typedef BOOL  (__stdcall *TimerRecordFunc)(DWORD);
typedef BOOL  (__stdcall *TimerStartFunc)(DWORD);
typedef BOOL  (__stdcall *TransferExperienceToAppFunc)(wchar_t***);
typedef BOOL  (__stdcall *TransferExperienceToAppIdFunc)(DWORD);
typedef BOOL  (__stdcall *TransferExperienceToWebFunc)(const wchar_t*, wchar_t***);

static int g_pass = 0, g_fail = 0;

static void ProbeBool(const char* label, void* fn, BOOL result, LONG seh) {
    if (seh) { printf("    [%s] SEH 0x%08X\n", label, seh); g_fail++; }
    else     { printf("    [%s] %s (0x%08X)\n", label, result ? "TRUE" : "FALSE", result); g_pass++; }
}

static void ProbeLong(const char* label, void* fn, LONG result, LONG seh) {
    if (seh) { printf("    [%s] SEH 0x%08X\n", label, seh); g_fail++; }
    else     { printf("    [%s] %ld (0x%08X)\n", label, result, result); g_pass++; }
}

static void ProbeVoid(const char* label, void* fn, LONG seh) {
    if (seh) { printf("    [%s] SEH 0x%08X\n", label, seh); g_fail++; }
    else     { printf("    [%s] completed (void)\n", label); g_pass++; }
}

#define PROBE_BOOL(h, ft, label, exp, ...) \
do { ft fn = (ft)GetProcAddress(h, exp); if (fn) { \
    printf("\n[*] %s => 0x%p\n", exp, fn); \
    LONG _seh = 0; BOOL _r = FALSE; \
    __try { _r = fn(__VA_ARGS__); } __except(EXCEPTION_EXECUTE_HANDLER) { _seh = GetExceptionCode(); } \
    ProbeBool(label, (void*)fn, _r, _seh); } } while(0)

#define PROBE_LONG(h, ft, label, exp, ...) \
do { ft fn = (ft)GetProcAddress(h, exp); if (fn) { \
    printf("\n[*] %s => 0x%p\n", exp, fn); \
    LONG _seh = 0; LONG _r = 0; \
    __try { _r = fn(__VA_ARGS__); } __except(EXCEPTION_EXECUTE_HANDLER) { _seh = GetExceptionCode(); } \
    ProbeLong(label, (void*)fn, _r, _seh); } } while(0)

#define PROBE_VOID(h, ft, label, exp, ...) \
do { ft fn = (ft)GetProcAddress(h, exp); if (fn) { \
    printf("\n[*] %s => 0x%p\n", exp, fn); \
    LONG _seh = 0; \
    __try { fn(__VA_ARGS__); } __except(EXCEPTION_EXECUTE_HANDLER) { _seh = GetExceptionCode(); } \
    ProbeVoid(label, (void*)fn, _seh); } } while(0)

int main() {
    SetSearchPaths();

    printf("========================================\n");
    printf("  DmxBici.dll - 19 BICI Telemetry Exports\n");
    printf("========================================\n\n");

    HMODULE h = SafeLoadDll(DLL_RELPATH);
    if (!h) {
        printf("[FATAL] Cannot load DmxBici.dll\n");
        return 1;
    }

    // RTTI class check
    printf("\n[*] RTTI classes in .data:\n");
    printf("    .?AVtype_info@@ (type_info)\n");
    printf("    .?AVException@Base@@ (from WLXPhotoBase.dll)\n");

    printf("\n========================================\n");
    printf("  Probing all 19 exports (SEH-protected)\n");
    printf("========================================\n");

    // 1  AddStringToDataPoint(key=1, subKey=1, str=L"test")
    PROBE_BOOL(h, AddStringToDataPointFunc, "AddStringToDataPoint",
        "?AddStringToDataPoint@BiciWrapper@@YG_NKKPB_W@Z", 1, 1, L"test");

    // 2  AddToAverage(key=1, value=100)
    PROBE_BOOL(h, AddToAverageFunc, "AddToAverage",
        "?AddToAverage@BiciWrapper@@YG_NKK@Z", 1, 100);

    // 3  AddToDataPoint(key=1, subKey=1, value=42)
    PROBE_BOOL(h, AddToDataPointFunc, "AddToDataPoint",
        "?AddToDataPoint@BiciWrapper@@YG_NKKK@Z", 1, 1, 42);

    // 4  AddToStream(key=1, tuple=NULL)
    PROBE_VOID(h, AddToStreamFunc, "AddToStream",
        "?AddToStream@BiciWrapper@@YGXKPBVTuple@1@@Z", 1, (const void*)NULL);

    // 5  EndExperience()
    PROBE_LONG(h, EndExperienceFunc, "EndExperience",
        "?EndExperience@BiciWrapper@@YGJXZ");

    // 6  Increment(key=1, value=1)
    PROBE_BOOL(h, IncrementFunc, "Increment",
        "?Increment@BiciWrapper@@YG_NKK@Z", 1, 1);

    // 7  Set(key=1, value=99)
    PROBE_BOOL(h, SetFunc, "Set",
        "?Set@BiciWrapper@@YG_NKK@Z", 1, 99);

    // 8  SetAnid(anid=L"test-anid")
    PROBE_LONG(h, SetAnidFunc, "SetAnid",
        "?SetAnid@BiciWrapper@@YGJPB_W@Z", L"test-anid");

    // 9  SetIfMax(key=1, value=200)
    PROBE_BOOL(h, SetIfMaxFunc, "SetIfMax",
        "?SetIfMax@BiciWrapper@@YG_NKK@Z", 1, 200);

    // 10 SetIfMin(key=1, value=10)
    PROBE_BOOL(h, SetIfMinFunc, "SetIfMin",
        "?SetIfMin@BiciWrapper@@YG_NKK@Z", 1, 10);

    // 11 SetString(key=1, str=L"hello")
    PROBE_BOOL(h, SetStringFunc, "SetString",
        "?SetString@BiciWrapper@@YG_NKPB_W@Z", 1, L"hello");

    // 12 StartExperience(BiciStartupId=0)
    PROBE_LONG(h, StartExperienceByIdFunc, "StartExperience(id=0)",
        "?StartExperience@BiciWrapper@@YGJW4BiciStartupId@1@@Z", 0);

    // 13 StartExperience() - no-arg overload
    PROBE_LONG(h, StartExperienceFunc, "StartExperience()",
        "?StartExperience@BiciWrapper@@YGJXZ");

    // 14 TimerAccumulate(timerId=1)
    PROBE_BOOL(h, TimerAccumulateFunc, "TimerAccumulate",
        "?TimerAccumulate@BiciWrapper@@YG_NK@Z", 1);

    // 15 TimerRecord(timerId=1)
    PROBE_BOOL(h, TimerRecordFunc, "TimerRecord",
        "?TimerRecord@BiciWrapper@@YG_NK@Z", 1);

    // 16 TimerStart(timerId=1)
    PROBE_BOOL(h, TimerStartFunc, "TimerStart",
        "?TimerStart@BiciWrapper@@YG_NK@Z", 1);

    // 17 TransferExperienceToApp(outPtr=NULL)
    PROBE_BOOL(h, TransferExperienceToAppFunc, "TransferExperienceToApp",
        "?TransferExperienceToApp@BiciWrapper@@YG_NPAPA_W@Z", (wchar_t***)NULL);

    // 18 TransferExperienceToAppId(appId=0)
    PROBE_BOOL(h, TransferExperienceToAppIdFunc, "TransferExperienceToAppId",
        "?TransferExperienceToAppId@BiciWrapper@@YG_NK@Z", (DWORD)0);

    // 19 TransferExperienceToWeb(url=L"", outPtr=NULL)
    PROBE_BOOL(h, TransferExperienceToWebFunc, "TransferExperienceToWeb",
        "?TransferExperienceToWeb@BiciWrapper@@YG_NPB_WPAPA_W@Z", L"", (wchar_t***)NULL);

    printf("\n========================================\n");
    printf("  Summary: %d passed, %d failed\n", g_pass, g_fail);
    printf("========================================\n");

    // Enumerate all 19 exports by name
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
                printf("    [%3u] Ordinal %2u  RVA=0x%05X  %s\n", i, ord + exports->Base, rva, name);
            }
        }
    }

    FreeLibrary(h);
    return g_fail;
}
