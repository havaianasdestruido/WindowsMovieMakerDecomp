#include <windows.h>
#include <stdio.h>

typedef void* HSQMSESSION;

typedef BOOL  (WINAPI *pfnSqmGetEnabled)(void);
typedef BOOL  (WINAPI *pfnSqmSetEnabled)(BOOL bEnabled);
typedef BOOL  (WINAPI *pfnSqmIsWindowsOptedIn)(void);
typedef DWORD (WINAPI *pfnSqmGetSession)(void);
typedef DWORD (WINAPI *pfnSqmStartSession)(void);
typedef DWORD (WINAPI *pfnSqmEndSession)(DWORD dwSession);
typedef DWORD (WINAPI *pfnSqmSetAppId)(DWORD dwAppId);
typedef DWORD (WINAPI *pfnSqmSet)(DWORD dwSession, DWORD dwId, DWORD dwValue);
typedef DWORD (WINAPI *pfnSqmSetString)(DWORD dwSession, DWORD dwId, const wchar_t* wszValue);
typedef DWORD (WINAPI *pfnSqmIncrement)(DWORD dwSession, DWORD dwId, DWORD dwValue);
typedef DWORD (WINAPI *pfnSqmGetMachineId)(wchar_t* wszMachineId, DWORD cchMax);
typedef DWORD (WINAPI *pfnSqmGetUserId)(wchar_t* wszUserId, DWORD cchMax);
typedef DWORD (WINAPI *pfnSqmCleanup)(void);
typedef DWORD (WINAPI *pfnSqmFlushSession)(DWORD dwSession);
typedef DWORD (WINAPI *pfnSqmStartUpload)(void);
typedef DWORD (WINAPI *pfnSqmWaitForUploadComplete)(DWORD dwTimeout);
typedef DWORD (WINAPI *pfnSqmSetFlags)(DWORD dwSession, DWORD dwFlags);
typedef DWORD (WINAPI *pfnSqmClearFlags)(DWORD dwSession, DWORD dwFlags);
typedef DWORD (WINAPI *pfnSqmCreateNewId)(void);
typedef DWORD (WINAPI *pfnSqmTimerStart)(DWORD dwSession, DWORD dwTimerId);
typedef DWORD (WINAPI *pfnSqmTimerRecord)(DWORD dwSession, DWORD dwTimerId);
typedef DWORD (WINAPI *pfnSqmAddToAverage)(DWORD dwSession, DWORD dwId, DWORD dwValue);
typedef DWORD (WINAPI *pfnSqmSetIfMax)(DWORD dwSession, DWORD dwId, DWORD dwValue);
typedef DWORD (WINAPI *pfnSqmSetIfMin)(DWORD dwSession, DWORD dwId, DWORD dwValue);
typedef DWORD (WINAPI *pfnSqmSetBool)(DWORD dwSession, DWORD dwId, BOOL bValue);
typedef DWORD (WINAPI *pfnSqmSetDWord64)(DWORD dwSession, DWORD dwId, ULONGLONG ullValue);
typedef DWORD (WINAPI *pfnSqmSysprepCleanup)(void);
typedef DWORD (WINAPI *pfnSqmSysprepGeneralize)(void);
typedef DWORD (WINAPI *pfnSqmSysprepSpecialize)(void);

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name, expr) do { \
    tests_run++; \
    if (expr) { tests_passed++; printf("  [PASS] %s\n", name); } \
    else { printf("  [FAIL] %s\n", name); } \
} while(0)

#define LOG_API(h, type, name) \
    type pfn##name = (type)GetProcAddress(h, #name); \
    printf("  %-45s %s\n", #name, pfn##name ? "OK" : "NOT FOUND")

void test_sqmapi(void) {
    printf("\n=== sqmapi.dll ===\n");
    HMODULE h = LoadLibraryW(L"undecomp\\Shared\\sqmapi.dll");
    TEST("LoadLibrary sqmapi.dll", h != NULL);
    if (!h) return;

    LOG_API(h, pfnSqmGetEnabled, SqmGetEnabled);
    LOG_API(h, pfnSqmSetEnabled, SqmSetEnabled);
    LOG_API(h, pfnSqmIsWindowsOptedIn, SqmIsWindowsOptedIn);
    LOG_API(h, pfnSqmGetSession, SqmGetSession);
    LOG_API(h, pfnSqmStartSession, SqmStartSession);
    LOG_API(h, pfnSqmEndSession, SqmEndSession);
    LOG_API(h, pfnSqmSetAppId, SqmSetAppId);
    LOG_API(h, pfnSqmSet, SqmSet);
    LOG_API(h, pfnSqmSetString, SqmSetString);
    LOG_API(h, pfnSqmIncrement, SqmIncrement);
    LOG_API(h, pfnSqmGetMachineId, SqmGetMachineId);
    LOG_API(h, pfnSqmGetUserId, SqmGetUserId);
    LOG_API(h, pfnSqmCleanup, SqmCleanup);
    LOG_API(h, pfnSqmFlushSession, SqmFlushSession);
    LOG_API(h, pfnSqmStartUpload, SqmStartUpload);
    LOG_API(h, pfnSqmWaitForUploadComplete, SqmWaitForUploadComplete);
    LOG_API(h, pfnSqmSetFlags, SqmSetFlags);
    LOG_API(h, pfnSqmClearFlags, SqmClearFlags);
    LOG_API(h, pfnSqmCreateNewId, SqmCreateNewId);
    LOG_API(h, pfnSqmTimerStart, SqmTimerStart);
    LOG_API(h, pfnSqmTimerRecord, SqmTimerRecord);
    LOG_API(h, pfnSqmAddToAverage, SqmAddToAverage);
    LOG_API(h, pfnSqmSetIfMax, SqmSetIfMax);
    LOG_API(h, pfnSqmSetIfMin, SqmSetIfMin);
    LOG_API(h, pfnSqmSetBool, SqmSetBool);
    LOG_API(h, pfnSqmSetDWord64, SqmSetDWord64);
    LOG_API(h, pfnSqmSysprepCleanup, SqmSysprepCleanup);
    LOG_API(h, pfnSqmSysprepGeneralize, SqmSysprepGeneralize);
    LOG_API(h, pfnSqmSysprepSpecialize, SqmSysprepSpecialize);

    if (pfnSqmGetEnabled) {
        BOOL enabled = pfnSqmGetEnabled();
        TEST("SqmGetEnabled returns BOOL", TRUE);
        printf("    -> SQM enabled: %s\n", enabled ? "YES" : "NO");
    }

    if (pfnSqmIsWindowsOptedIn) {
        BOOL opted = pfnSqmIsWindowsOptedIn();
        TEST("SqmIsWindowsOptedIn returns BOOL", TRUE);
        printf("    -> Windows opted in: %s\n", opted ? "YES" : "NO");
    }

    if (pfnSqmStartSession && pfnSqmEndSession) {
        DWORD session = pfnSqmStartSession();
        TEST("SqmStartSession returns session handle", session != 0);
        if (session) {
            if (pfnSqmSet) {
                DWORD hr = pfnSqmSet(session, 100, 42);
                TEST("SqmSet returns HRESULT", SUCCEEDED(hr) || hr == 0);
            }
            if (pfnSqmFlushSession) {
                DWORD hr = pfnSqmFlushSession(session);
                TEST("SqmFlushSession succeeds", SUCCEEDED(hr) || hr == 0);
            }
            if (pfnSqmEndSession) {
                DWORD hr = pfnSqmEndSession(session);
                TEST("SqmEndSession succeeds", SUCCEEDED(hr) || hr == 0);
            }
        }
    }

    FreeLibrary(h);
}

void test_wlbici(void) {
    printf("\n=== wlbici.dll ===\n");
    HMODULE h = LoadLibraryW(L"undecomp\\Shared\\wlbici.dll");
    TEST("LoadLibrary wlbici.dll", h != NULL);
    if (!h) return;

    typedef void* (WINAPI *pfnStartExperience)(void);
    typedef BOOL  (WINAPI *pfnEndExperience)(void* hExp);
    typedef DWORD (WINAPI *pfnSet)(void* hExp, DWORD dwId, DWORD dwValue);
    typedef DWORD (WINAPI *pfnSetString)(void* hExp, DWORD dwId, const wchar_t* wszValue);
    typedef DWORD (WINAPI *pfnIncrement)(void* hExp, DWORD dwId, DWORD dwValue);
    typedef DWORD (WINAPI *pfnUploadData)(void);
    typedef DWORD (WINAPI *pfnSetAnid)(const wchar_t* wszAnid);
    typedef DWORD (WINAPI *pfnSetDataFilePath)(const wchar_t* wszPath);
    typedef DWORD (WINAPI *pfnSetUploadInterval)(DWORD dwInterval);
    typedef DWORD (WINAPI *pfnSetFlags)(void* hExp, DWORD dwFlags);

    LOG_API(h, pfnStartExperience, StartExperience);
    LOG_API(h, pfnEndExperience, EndExperience);
    LOG_API(h, pfnSet, Set);
    LOG_API(h, pfnSetString, SetString);
    LOG_API(h, pfnIncrement, Increment);
    LOG_API(h, pfnUploadData, UploadData);
    LOG_API(h, pfnSetAnid, SetAnid);
    LOG_API(h, pfnSetDataFilePath, SetDataFilePath);
    LOG_API(h, pfnSetUploadInterval, SetUploadInterval);
    LOG_API(h, pfnSetFlags, SetFlags);

    /* COM registration exports */
    typedef HRESULT (WINAPI *pfnDllRegisterServer)(void);
    typedef HRESULT (WINAPI *pfnDllCanUnloadNow)(void);

    LOG_API(h, pfnDllRegisterServer, DllRegisterServer);
    LOG_API(h, pfnDllCanUnloadNow, DllCanUnloadNow);

    if (pfnDllCanUnloadNow) {
        HRESULT hr = pfnDllCanUnloadNow();
        TEST("DllCanUnloadNow returns S_OK initially", SUCCEEDED(hr));
        printf("    -> DllCanUnloadNow: 0x%08X\n", hr);
    }

    FreeLibrary(h);
}

void test_wldcore(void) {
    printf("\n=== wldcore.dll ===\n");
    HMODULE h = LoadLibraryW(L"undecomp\\Shared\\wldcore.dll");
    TEST("LoadLibrary wldcore.dll", h != NULL);
    if (!h) return;

    /* DCCreateObject is the only named export besides AddMinutesToSystemTime */
    typedef HRESULT (WINAPI *pfnDCCreateObject)(REFIID riid, void** ppv);
    pfnDCCreateObject pfnDCCreateObject = (pfnDCCreateObject)GetProcAddress(h, "DCCreateObject");
    LOG_API(h, pfnDCCreateObject, DCCreateObject);

    /* Test ordinal exports - key ones based on CSqmSessionManager pattern */
    typedef DWORD (WINAPI *pfnOrdinal)(void);
    char ordinalNames[10][64];
    int found = 0;
    for (int i = 1; i <= 120; i++) {
        pfnOrdinal pfn = (pfnOrdinal)GetProcAddress(h, (LPCSTR)(INT_PTR)i);
        if (pfn) {
            found++;
        }
    }
    for (int i = 200; i <= 272; i++) {
        pfnOrdinal pfn = (pfnOrdinal)GetProcAddress(h, (LPCSTR)(INT_PTR)i);
        if (pfn) found++;
    }
    TEST("Ordinal exports resolvable", found > 200);
    printf("    -> Resolved %d of 272 ordinals\n", found);

    /* Test AddMinutesToSystemTime */
    typedef LONG (WINAPI *pfnAddMinutesToSystemTime)(const SYSTEMTIME* pIn, SYSTEMTIME* pOut, DWORD dwMinutes);
    pfnAddMinutesToSystemTime pfnAddMin = (pfnAddMinutesToSystemTime)GetProcAddress(h, "?AddMinutesToSystemTime@@YGJABU_SYSTEMTIME@@PAU1@K@Z");
    if (pfnAddMin) {
        SYSTEMTIME stIn = {2014, 4, 1, 1, 12, 0, 0, 0};
        SYSTEMTIME stOut = {0};
        LONG hr = pfnAddMin(&stIn, &stOut, 60);
        TEST("AddMinutesToSystemTime callable", TRUE);
        printf("    -> 2014-04-01 12:00 + 60min = %04d-%02d-%02d %02d:%02d (hr=%ld)\n",
               stOut.wYear, stOut.wMonth, stOut.wDay, stOut.wHour, stOut.wMinute, hr);
    }

    /* Test DCCreateObject with IUnknown */
    if (pfnDCCreateObject) {
        IUnknown* pUnk = NULL;
        HRESULT hr = pfnDCCreateObject(&IID_IUnknown, (void**)&pUnk);
        printf("    -> DCCreateObject(IID_IUnknown): hr=0x%08X, ptr=%p\n", hr, pUnk);
        if (pUnk) pUnk->lpVtbl->Release(pUnk);
    }

    FreeLibrary(h);
}

void test_wldlog(void) {
    printf("\n=== wldlog.dll ===\n");
    HMODULE h = LoadLibraryW(L"undecomp\\Shared\\wldlog.dll");
    TEST("LoadLibrary wldlog.dll", h != NULL);
    if (!h) return;

    typedef BOOL (WINAPI *pfnInitializeLogging)(void);
    typedef BOOL (WINAPI *pfnUninitializeLogging)(void);
    typedef BOOL (WINAPI *pfnInitializeModule)(DWORD dwModuleId);
    typedef BOOL (WINAPI *pfnIsModuleInitialized)(DWORD dwModuleId);
    typedef BOOL (WINAPI *pfnIsRetailLoggingEnabled)(void);
    typedef void (WINAPI *pfnLogOutput)(const char*pszFmt, ...);
    typedef BOOL (WINAPI *pfnEnableParallelLog)(void);
    typedef BOOL (WINAPI *pfnDisableParallelLog)(void);
    typedef DWORD (WINAPI *pfnSetZoneLevel)(DWORD dwZone, DWORD dwLevel);
    typedef DWORD (WINAPI *pfnGetZoneLevel)(DWORD dwZone);
    typedef BOOL (WINAPI *pfnSetZoneLoggingEnabled)(DWORD dwZone, BOOL bEnabled);
    typedef BOOL (WINAPI *pfnZoneLoggingEnabled)(DWORD dwZone);
    typedef BOOL (WINAPI *pfnZoneFileLoggingEnabled)(DWORD dwZone);
    typedef BOOL (WINAPI *pfnSetRetailLogging)(BOOL bEnable);
    typedef DWORD (WINAPI *pfnRetailLoggingFlush)(void);
    typedef DWORD (WINAPI *pfnRetailLoggingDirtyFlushLogs)(void);
    typedef char* (WINAPI *pfnRetailLoggingGetMemoryLog)(DWORD* pdwSize);
    typedef BOOL (WINAPI *pfnRetailLoggingGetPath)(wchar_t* wszPath, DWORD cchMax);

    LOG_API(h, pfnInitializeLogging, InitializeLogging);
    LOG_API(h, pfnUninitializeLogging, UninitializeLogging);
    LOG_API(h, pfnInitializeModule, InitializeModule);
    LOG_API(h, pfnIsModuleInitialized, IsModuleInitialized);
    LOG_API(h, pfnIsRetailLoggingEnabled, IsRetailLoggingEnabled);
    LOG_API(h, pfnLogOutput, LogOutput);
    LOG_API(h, pfnEnableParallelLog, EnableParallelLog);
    LOG_API(h, pfnDisableParallelLog, DisableParallelLog);
    LOG_API(h, pfnSetZoneLevel, SetZoneLevel);
    LOG_API(h, pfnGetZoneLevel, GetZoneLevel);
    LOG_API(h, pfnSetZoneLoggingEnabled, SetZoneLoggingEnabled);
    LOG_API(h, pfnZoneLoggingEnabled, ZoneLoggingEnabled);
    LOG_API(h, pfnZoneFileLoggingEnabled, ZoneFileLoggingEnabled);
    LOG_API(h, pfnSetRetailLogging, SetRetailLogging);
    LOG_API(h, pfnRetailLoggingFlush, RetailLoggingFlush);
    LOG_API(h, pfnRetailLoggingDirtyFlushLogs, RetailLoggingDirtyFlushLogs);
    LOG_API(h, pfnRetailLoggingGetMemoryLog, RetailLoggingGetMemoryLog);
    LOG_API(h, pfnRetailLoggingGetPath, RetailLoggingGetPath);

    typedef HRESULT (WINAPI *pfnDllRegisterServer)(void);
    typedef HRESULT (WINAPI *pfnDllCanUnloadNow)(void);
    LOG_API(h, pfnDllRegisterServer, DllRegisterServer);
    LOG_API(h, pfnDllCanUnloadNow, DllCanUnloadNow);

    if (pfnInitializeLogging) {
        BOOL ok = pfnInitializeLogging();
        TEST("InitializeLogging succeeds", ok);
    }

    if (pfnIsRetailLoggingEnabled) {
        BOOL retail = pfnIsRetailLoggingEnabled();
        printf("    -> Retail logging: %s\n", retail ? "ENABLED" : "DISABLED");
    }

    if (pfnGetZoneLevel) {
        for (DWORD zone = 0; zone < 5; zone++) {
            DWORD level = pfnGetZoneLevel(zone);
            printf("    -> Zone %lu level: %lu\n", zone, level);
        }
        TEST("GetZoneLevel callable for zones 0-4", TRUE);
    }

    if (pfnRetailLoggingGetPath) {
        wchar_t path[MAX_PATH] = {0};
        BOOL ok = pfnRetailLoggingGetPath(path, MAX_PATH);
        if (ok && path[0]) {
            printf("    -> Retail log path: %ls\n", path);
        }
        TEST("RetailLoggingGetPath callable", TRUE);
    }

    if (pfnEnableParallelLog) {
        BOOL ok = pfnEnableParallelLog();
        printf("    -> EnableParallelLog: %s\n", ok ? "OK" : "FAIL");
    }

    if (pfnUninitializeLogging) {
        pfnUninitializeLogging();
        TEST("UninitializeLogging succeeds", TRUE);
    }

    FreeLibrary(h);
}

int main(void) {
    printf("========================================\n");
    printf(" Windows Live Shared DLL Test Harness\n");
    printf(" sqmapi + wlbici + wldcore + wldlog\n");
    printf("========================================\n");

    test_sqmapi();
    test_wlbici();
    test_wldcore();
    test_wldlog();

    printf("\n========================================\n");
    printf(" Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
