// WLXPhotoSqm.dll Test Harness - 44 Sqm:: exports
// Loads DLL, probes all exports with SEH protection
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdlib>

// ============================================================
// Helpers
// ============================================================
static void SetSearchPaths() {
    wchar_t galleryDir[MAX_PATH], sharedDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
    GetFullPathNameW(L"undecomp\\Shared", MAX_PATH, sharedDir, NULL);
    AddDllDirectory(galleryDir);
    AddDllDirectory(sharedDir);
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
// Function pointer typedefs for all 44 exports
// ============================================================

// Lifecycle
typedef void (__stdcall *StartupDefaultFunc)();
typedef void (__stdcall *StartupAppIdFunc)(DWORD appId);
typedef void (__stdcall *ShutdownFunc)();

// Configuration
typedef void (__stdcall *SetAppStatusReportingModeFunc)(BOOL enabled);
typedef void (__stdcall *SetOptInPreferenceFunc)(BOOL enabled);
typedef void (__stdcall *SetApplicationModeFunc)(DWORD mode);
typedef void (__stdcall *SetAppDefinedValueFunc)(DWORD value);
typedef void (__stdcall *EnableShipAssertsFunc)();
typedef void (__stdcall *InitializeUserExecutedActionReportingFunc)(DWORD a, DWORD b);

// Opt-in State / Query
typedef DWORD (__stdcall *GetOptInStateFunc)();     // returns OptInState enum
typedef BOOL  (__stdcall *IsEnabledFunc)();

// Data Collection - Basic
typedef void (__stdcall *SetDwordFunc)(DWORD id, DWORD value);
typedef void (__stdcall *SetStringFunc)(DWORD id, const wchar_t* value);
typedef void (__stdcall *SetIfMinFunc)(DWORD id, DWORD value);
typedef void (__stdcall *SetIfMaxFunc)(DWORD id, DWORD value);
typedef void (__stdcall *IncrementFunc)(DWORD id, DWORD increment);

// Data Collection - Streams
typedef void (__stdcall *AddToStreamDwordFunc)(DWORD id, DWORD value);
typedef void (__stdcall *AddToStream2Func)(DWORD id, DWORD a, DWORD b);
typedef void (__stdcall *AddToStream3Func)(DWORD id, DWORD a, DWORD b, DWORD c);
typedef void (__stdcall *AddToStreamTupleFunc)(DWORD id, const void* tuple);
typedef void (__stdcall *AddToStreamStringFunc)(DWORD id, const wchar_t* value);

// Data Collection - Averages
typedef void (__stdcall *AddToAverageFunc)(DWORD id, DWORD value);

// Deferred Operations
typedef void (__stdcall *DeferSetIfMinFunc)(DWORD id, DWORD value);
typedef void (__stdcall *DeferSetIfMaxFunc)(DWORD id, DWORD value);
typedef void (__stdcall *DeferAddToAverageFunc)(DWORD id, DWORD value);
typedef void (__stdcall *DeferAddToMedianFunc)(DWORD id, DWORD value);
typedef void (__stdcall *DeferReportMinFunc)(DWORD id);
typedef void (__stdcall *DeferReportMaxFunc)(DWORD id);
typedef void (__stdcall *DeferReportAverageFunc)(DWORD id);
typedef void (__stdcall *DeferReportMedianFunc)(DWORD id);

// Timers
typedef void (__stdcall *StartTimerFunc)(DWORD id);
typedef void (__stdcall *PauseTimerFunc)(DWORD id);

// Stream Timers
typedef void (__stdcall *StartStreamTimerFunc)(DWORD a, DWORD b, DWORD c);
typedef void (__stdcall *StopStreamTimerFunc)(DWORD a, DWORD b);
typedef void (__stdcall *AbortStreamTimerFunc)(DWORD a, DWORD b);
typedef BOOL (__stdcall *IsStreamTimerActiveFunc)(DWORD a, DWORD b);
typedef BOOL (__stdcall *IsStreamTimerDataSetFunc)(DWORD a, DWORD b);
typedef void (__stdcall *AddStreamTimerDataDwordFunc)(DWORD a, DWORD b, DWORD c);
typedef void (__stdcall *AddStreamTimerDataStringFunc)(DWORD a, DWORD b, const wchar_t* c);
typedef void (__stdcall *AddToStreamTimer3Func)(DWORD a, DWORD b, DWORD c, const void* tuple);
typedef void (__stdcall *AddToStreamTimer2Func)(DWORD a, DWORD b, const void* tuple);

// User Action Reporting
typedef void (__stdcall *ReportUserExecutedActionFunc)(DWORD a, DWORD b);
typedef void (__stdcall *ReportAppLaunchStatusFunc)(BOOL success);
typedef void (__stdcall *ReportAppCloseStatusFunc)(BOOL success);

// ============================================================
// Test: probe a single function pointer with SEH
// ============================================================
static int g_callCount = 0;

static void ProbeCall(const char* name, void* fn, const char* signature) {
    if (!fn) {
        printf("    [!] %s not found\n", name);
        return;
    }
    printf("    %s => %s [0x%p]\n", name, signature, fn);
    // We don't actually call the functions to avoid side effects
    // (SQM writes to disk, starts sessions, etc.)
    g_callCount++;
}

// ============================================================
// WLXPhotoSqm.dll - 44 exports
// ============================================================
static void TestWLXPhotoSqm(HMODULE h) {
    printf("\n========================================\n");
    printf("  WLXPhotoSqm.dll Export Probing (44 exports)\n");
    printf("========================================\n");

    // === Lifecycle (3) ===
    printf("\n--- Lifecycle ---\n");
    ProbeCall("?Startup@Sqm@@YGXXZ",
        GetProcAddress(h, "?Startup@Sqm@@YGXXZ"),
        "void __stdcall Sqm::Startup(void)");

    ProbeCall("?Startup@Sqm@@YGXW4SqmDmxAppId@1@@Z",
        GetProcAddress(h, "?Startup@Sqm@@YGXW4SqmDmxAppId@1@@Z"),
        "void __stdcall Sqm::Startup(SqmDmxAppId)");

    ProbeCall("?Shutdown@Sqm@@YGXXZ",
        GetProcAddress(h, "?Shutdown@Sqm@@YGXXZ"),
        "void __stdcall Sqm::Shutdown(void)");

    // === Configuration (6) ===
    printf("\n--- Configuration ---\n");
    ProbeCall("?SetAppStatusReportingMode@Sqm@@YGX_N@Z",
        GetProcAddress(h, "?SetAppStatusReportingMode@Sqm@@YGX_N@Z"),
        "void __stdcall Sqm::SetAppStatusReportingMode(BOOL)");

    ProbeCall("?SetOptInPreference@Sqm@@YGX_N@Z",
        GetProcAddress(h, "?SetOptInPreference@Sqm@@YGX_N@Z"),
        "void __stdcall Sqm::SetOptInPreference(BOOL)");

    ProbeCall("?SetApplicationMode@Sqm@@YGXK@Z",
        GetProcAddress(h, "?SetApplicationMode@Sqm@@YGXK@Z"),
        "void __stdcall Sqm::SetApplicationMode(DWORD)");

    ProbeCall("?SetAppDefinedValue@Sqm@@YGXK@Z",
        GetProcAddress(h, "?SetAppDefinedValue@Sqm@@YGXK@Z"),
        "void __stdcall Sqm::SetAppDefinedValue(DWORD)");

    ProbeCall("?EnableShipAsserts@Sqm@@YGXXZ",
        GetProcAddress(h, "?EnableShipAsserts@Sqm@@YGXXZ"),
        "void __stdcall Sqm::EnableShipAsserts(void)");

    ProbeCall("?InitializeUserExecutedActionReporting@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?InitializeUserExecutedActionReporting@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::InitializeUserExecutedActionReporting(DWORD,DWORD)");

    // === Query (2) ===
    printf("\n--- Query ---\n");
    ProbeCall("?GetOptInState@Sqm@@YG?AW4OptInState@1@XZ",
        GetProcAddress(h, "?GetOptInState@Sqm@@YG?AW4OptInState@1@XZ"),
        "OptInState __stdcall Sqm::GetOptInState(void)");

    ProbeCall("?IsEnabled@Sqm@@YG_NXZ",
        GetProcAddress(h, "?IsEnabled@Sqm@@YG_NXZ"),
        "BOOL __stdcall Sqm::IsEnabled(void)");

    // === Data Collection - Basic (5) ===
    printf("\n--- Data Collection: Basic ---\n");
    ProbeCall("?Set@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?Set@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::Set(DWORD,DWORD)");

    ProbeCall("?Set@Sqm@@YGXKPB_W@Z",
        GetProcAddress(h, "?Set@Sqm@@YGXKPB_W@Z"),
        "void __stdcall Sqm::Set(DWORD,const wchar_t*)");

    ProbeCall("?SetIfMin@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?SetIfMin@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::SetIfMin(DWORD,DWORD)");

    ProbeCall("?SetIfMax@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?SetIfMax@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::SetIfMax(DWORD,DWORD)");

    ProbeCall("?Increment@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?Increment@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::Increment(DWORD,DWORD)");

    // === Data Collection - Streams (5) ===
    printf("\n--- Data Collection: Streams ---\n");
    ProbeCall("?AddToStream@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?AddToStream@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::AddToStream(DWORD,DWORD)");

    ProbeCall("?AddToStream@Sqm@@YGXKKK@Z",
        GetProcAddress(h, "?AddToStream@Sqm@@YGXKKK@Z"),
        "void __stdcall Sqm::AddToStream(DWORD,DWORD,DWORD)");

    ProbeCall("?AddToStream@Sqm@@YGXKKKK@Z",
        GetProcAddress(h, "?AddToStream@Sqm@@YGXKKKK@Z"),
        "void __stdcall Sqm::AddToStream(DWORD,DWORD,DWORD,DWORD)");

    ProbeCall("?AddToStream@Sqm@@YGXKPBVTuple@1@@Z",
        GetProcAddress(h, "?AddToStream@Sqm@@YGXKPBVTuple@1@@Z"),
        "void __stdcall Sqm::AddToStream(DWORD,const Tuple*)");

    ProbeCall("?AddToStream@Sqm@@YGXKPB_W@Z",
        GetProcAddress(h, "?AddToStream@Sqm@@YGXKPB_W@Z"),
        "void __stdcall Sqm::AddToStream(DWORD,const wchar_t*)");

    // === Data Collection - Averages (1) ===
    printf("\n--- Data Collection: Averages ---\n");
    ProbeCall("?AddToAverage@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?AddToAverage@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::AddToAverage(DWORD,DWORD)");

    // === Deferred Operations (8) ===
    printf("\n--- Deferred Operations ---\n");
    ProbeCall("?DeferSetIfMin@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?DeferSetIfMin@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::DeferSetIfMin(DWORD,DWORD)");

    ProbeCall("?DeferSetIfMax@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?DeferSetIfMax@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::DeferSetIfMax(DWORD,DWORD)");

    ProbeCall("?DeferAddToAverage@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?DeferAddToAverage@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::DeferAddToAverage(DWORD,DWORD)");

    ProbeCall("?DeferAddToMedian@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?DeferAddToMedian@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::DeferAddToMedian(DWORD,DWORD)");

    ProbeCall("?DeferReportMin@Sqm@@YGXK@Z",
        GetProcAddress(h, "?DeferReportMin@Sqm@@YGXK@Z"),
        "void __stdcall Sqm::DeferReportMin(DWORD)");

    ProbeCall("?DeferReportMax@Sqm@@YGXK@Z",
        GetProcAddress(h, "?DeferReportMax@Sqm@@YGXK@Z"),
        "void __stdcall Sqm::DeferReportMax(DWORD)");

    ProbeCall("?DeferReportAverage@Sqm@@YGXK@Z",
        GetProcAddress(h, "?DeferReportAverage@Sqm@@YGXK@Z"),
        "void __stdcall Sqm::DeferReportAverage(DWORD)");

    ProbeCall("?DeferReportMedian@Sqm@@YGXK@Z",
        GetProcAddress(h, "?DeferReportMedian@Sqm@@YGXK@Z"),
        "void __stdcall Sqm::DeferReportMedian(DWORD)");

    // === Timers (2) ===
    printf("\n--- Timers ---\n");
    ProbeCall("?StartTimer@Sqm@@YGXK@Z",
        GetProcAddress(h, "?StartTimer@Sqm@@YGXK@Z"),
        "void __stdcall Sqm::StartTimer(DWORD)");

    ProbeCall("?PauseTimer@Sqm@@YGXK@Z",
        GetProcAddress(h, "?PauseTimer@Sqm@@YGXK@Z"),
        "void __stdcall Sqm::PauseTimer(DWORD)");

    // === Stream Timers (9) ===
    printf("\n--- Stream Timers ---\n");
    ProbeCall("?StartStreamTimer@Sqm@@YGXKKK@Z",
        GetProcAddress(h, "?StartStreamTimer@Sqm@@YGXKKK@Z"),
        "void __stdcall Sqm::StartStreamTimer(DWORD,DWORD,DWORD)");

    ProbeCall("?StopStreamTimer@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?StopStreamTimer@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::StopStreamTimer(DWORD,DWORD)");

    ProbeCall("?AbortStreamTimer@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?AbortStreamTimer@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::AbortStreamTimer(DWORD,DWORD)");

    ProbeCall("?IsStreamTimerActive@Sqm@@YG_NKK@Z",
        GetProcAddress(h, "?IsStreamTimerActive@Sqm@@YG_NKK@Z"),
        "BOOL __stdcall Sqm::IsStreamTimerActive(DWORD,DWORD)");

    ProbeCall("?IsStreamTimerDataSet@Sqm@@YG_NKK@Z",
        GetProcAddress(h, "?IsStreamTimerDataSet@Sqm@@YG_NKK@Z"),
        "BOOL __stdcall Sqm::IsStreamTimerDataSet(DWORD,DWORD)");

    ProbeCall("?AddStreamTimerData@Sqm@@YGXKKK@Z",
        GetProcAddress(h, "?AddStreamTimerData@Sqm@@YGXKKK@Z"),
        "void __stdcall Sqm::AddStreamTimerData(DWORD,DWORD,DWORD)");

    ProbeCall("?AddStreamTimerData@Sqm@@YGXKKPB_W@Z",
        GetProcAddress(h, "?AddStreamTimerData@Sqm@@YGXKKPB_W@Z"),
        "void __stdcall Sqm::AddStreamTimerData(DWORD,DWORD,const wchar_t*)");

    ProbeCall("?AddToStreamTimer@Sqm@@YGXKKKPBVTuple@1@@Z",
        GetProcAddress(h, "?AddToStreamTimer@Sqm@@YGXKKKPBVTuple@1@@Z"),
        "void __stdcall Sqm::AddToStreamTimer(DWORD,DWORD,DWORD,const Tuple*)");

    ProbeCall("?AddToStreamTimer@Sqm@@YGXKKPBVTuple@1@@Z",
        GetProcAddress(h, "?AddToStreamTimer@Sqm@@YGXKKPBVTuple@1@@Z"),
        "void __stdcall Sqm::AddToStreamTimer(DWORD,DWORD,const Tuple*)");

    // === User Action Reporting (3) ===
    printf("\n--- User Action Reporting ---\n");
    ProbeCall("?ReportUserExecutedAction@Sqm@@YGXKK@Z",
        GetProcAddress(h, "?ReportUserExecutedAction@Sqm@@YGXKK@Z"),
        "void __stdcall Sqm::ReportUserExecutedAction(DWORD,DWORD)");

    ProbeCall("?ReportAppLaunchStatus@Sqm@@YGX_N@Z",
        GetProcAddress(h, "?ReportAppLaunchStatus@Sqm@@YGX_N@Z"),
        "void __stdcall Sqm::ReportAppLaunchStatus(BOOL)");

    ProbeCall("?ReportAppCloseStatus@Sqm@@YGX_N@Z",
        GetProcAddress(h, "?ReportAppCloseStatus@Sqm@@YGX_N@Z"),
        "void __stdcall Sqm::ReportAppCloseStatus(BOOL)");
}

// ============================================================
// Main - selective safe calls only
// ============================================================
int main(int argc, char** argv) {
    printf("=== WLXPhotoSqm.dll Test Harness ===\n");
    printf("Target: 44 Sqm:: exports (SQM telemetry wrapper)\n\n");

    SetSearchPaths();

    HMODULE h = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXPhotoSqm.dll");
    if (!h) {
        printf("[!] Cannot continue without DLL\n");
        return 1;
    }

    TestWLXPhotoSqm(h);

    // === Safe live calls (query-only, no mutation) ===
    printf("\n========================================\n");
    printf("  Safe Live Calls (query-only)\n");
    printf("========================================\n");

    {
        printf("\n[*] Calling Sqm::IsEnabled()...\n");
        BOOL (__stdcall *fnIsEnabled)() =
            (BOOL(__stdcall*)())GetProcAddress(h, "?IsEnabled@Sqm@@YG_NXZ");
        if (fnIsEnabled) {
            __try {
                BOOL result = fnIsEnabled();
                printf("    => Sqm::IsEnabled() = %s\n", result ? "TRUE" : "FALSE");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        }
    }

    {
        printf("\n[*] Calling Sqm::GetOptInState()...\n");
        DWORD (__stdcall *fnGetOptInState)() =
            (DWORD(__stdcall*)())GetProcAddress(h, "?GetOptInState@Sqm@@YG?AW4OptInState@1@XZ");
        if (fnGetOptInState) {
            __try {
                DWORD result = fnGetOptInState();
                printf("    => Sqm::GetOptInState() = %u (0=OptOut, 1=OptIn, 2=TelemetryOnly, 3=NotDetermined)\n", result);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        }
    }

    {
        printf("\n[*] Calling Sqm::Startup() (default)...\n");
        void (__stdcall *fnStartup)() =
            (void(__stdcall*)())GetProcAddress(h, "?Startup@Sqm@@YGXXZ");
        if (fnStartup) {
            __try {
                fnStartup();
                printf("    => Sqm::Startup() completed\n");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        }
    }

    // Check IsEnabled again after Startup
    {
        printf("\n[*] Calling Sqm::IsEnabled() after Startup...\n");
        BOOL (__stdcall *fnIsEnabled)() =
            (BOOL(__stdcall*)())GetProcAddress(h, "?IsEnabled@Sqm@@YG_NXZ");
        if (fnIsEnabled) {
            __try {
                BOOL result = fnIsEnabled();
                printf("    => Sqm::IsEnabled() = %s\n", result ? "TRUE" : "FALSE");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        }
    }

    {
        printf("\n[*] Calling Sqm::Shutdown()...\n");
        void (__stdcall *fnShutdown)() =
            (void(__stdcall*)())GetProcAddress(h, "?Shutdown@Sqm@@YGXXZ");
        if (fnShutdown) {
            __try {
                fnShutdown();
                printf("    => Sqm::Shutdown() completed\n");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        }
    }

    printf("\n=== Summary ===\n");
    printf("Total exports resolved: %d/44\n", g_callCount);

    printf("\n[*] Unloading WLXPhotoSqm.dll\n");
    FreeLibrary(h);

    printf("\n=== Done ===\n");
    return 0;
}
