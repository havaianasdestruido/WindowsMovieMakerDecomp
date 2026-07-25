# WLXSlideshow.dll — Test Harness

## 1. Exports Under Test

| Ordinal | RVA | Name | Signature | Expected Return |
|---------|-----|------|-----------|-----------------|
| 1 | 0x0000B52B | `DllCanUnloadNow` | `STDAPI DllCanUnloadNow(void)` | `S_OK` (always) |
| 2 | 0x0000B51B | `DllGetClassObject` | `STDAPI DllGetClassObject(REFCLSID, REFIID, LPVOID*)` | `CLASS_E_CLASSNOTAVAILABLE` |
| 3 | 0x0000B54B | `DllRegisterServer` | `STDAPI DllRegisterServer(void)` | `S_OK` (stub) |
| 4 | 0x0000B53C | `DllUnregisterServer` | `STDAPI DllUnregisterServer(void)` | `S_OK` (stub) |

All four use `__stdcall` (STDAPI convention, returns `HRESULT`).

## 2. Test Harness main.cpp

```cpp
// ============================================================================
// WLXSlideshow Test Harness
// Exercises all 4 COM exports via LoadLibrary + GetProcAddress
// Build: 16.4.3528.0331_ship.client.main.w5m4
// ============================================================================

#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <excpt.h>

// ---------------------------------------------------------------------------
// Typedefs for exported functions
// ---------------------------------------------------------------------------
typedef HRESULT (STDAPICALLTYPE* FN_DllCanUnloadNow)(void);
typedef HRESULT (STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (STDAPICALLTYPE* FN_DllRegisterServer)(void);
typedef HRESULT (STDAPICALLTYPE* FN_DllUnregisterServer)(void);

// ---------------------------------------------------------------------------
// Known COM CLSID from the binary
// ---------------------------------------------------------------------------
// {84FBA192-4F8D-4a5d-94DE-083446ACC1D0}
static const GUID CLSID_WLXSlideshow =
    { 0x84FBA192, 0x4F8D, 0x4A5D, { 0x94, 0xDE, 0x08, 0x34, 0x46, 0xAC, 0xC1, 0xD0 } };

// IUnknown IID
static const GUID IID_IUnknown =
    { 0x00000000, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

// ---------------------------------------------------------------------------
// Test result tracking
// ---------------------------------------------------------------------------
static int  g_nPassed = 0;
static int  g_nFailed = 0;

static void TestResult(const char* szTest, HRESULT hrActual, HRESULT hrExpected)
{
    if (hrActual == hrExpected)
    {
        printf("  PASS: %s (0x%08lX)\n", szTest, hrActual);
        g_nPassed++;
    }
    else
    {
        printf("  FAIL: %s -- got 0x%08lX, expected 0x%08lX\n",
               szTest, hrActual, hrExpected);
        g_nFailed++;
    }
}

static void TestResultBool(const char* szTest, BOOL bActual, BOOL bExpected)
{
    if (bActual == bExpected)
    {
        printf("  PASS: %s (%d)\n", szTest, bActual);
        g_nPassed++;
    }
    else
    {
        printf("  FAIL: %s -- got %d, expected %d\n", szTest, bActual, bExpected);
        g_nFailed++;
    }
}

// ---------------------------------------------------------------------------
// SEH wrapper for calling exported functions
// ---------------------------------------------------------------------------
static HRESULT SafeCall_CanUnloadNow(FN_DllCanUnloadNow pfn)
{
    HRESULT hr = E_UNEXPECTED;
    __try
    {
        hr = pfn();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("  EXCEPTION: DllCanUnloadNow raised 0x%08lX\n",
               GetExceptionCode());
        hr = E_UNEXPECTED;
    }
    return hr;
}

static HRESULT SafeCall_GetClassObject(FN_DllGetClassObject pfn,
                                       REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr = E_UNEXPECTED;
    __try
    {
        hr = pfn(rclsid, riid, ppv);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("  EXCEPTION: DllGetClassObject raised 0x%08lX\n",
               GetExceptionCode());
        hr = E_UNEXPECTED;
    }
    return hr;
}

static HRESULT SafeCall_RegisterServer(FN_DllRegisterServer pfn)
{
    HRESULT hr = E_UNEXPECTED;
    __try
    {
        hr = pfn();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("  EXCEPTION: DllRegisterServer raised 0x%08lX\n",
               GetExceptionCode());
        hr = E_UNEXPECTED;
    }
    return hr;
}

static HRESULT SafeCall_UnregisterServer(FN_DllUnregisterServer pfn)
{
    HRESULT hr = E_UNEXPECTED;
    __try
    {
        hr = pfn();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("  EXCEPTION: DllUnregisterServer raised 0x%08lX\n",
               GetExceptionCode());
        hr = E_UNEXPECTED;
    }
    return hr;
}

// ---------------------------------------------------------------------------
// Test groups
// ---------------------------------------------------------------------------

static void Test_DllCanUnloadNow(HMODULE hMod)
{
    printf("\n=== DllCanUnloadNow ===\n");

    FN_DllCanUnloadNow pfn = (FN_DllCanUnloadNow)
        GetProcAddress(hMod, "DllCanUnloadNow");
    if (!pfn)
    {
        printf("  SKIP: GetProcAddress failed (0x%08lX)\n", GetLastError());
        g_nFailed++;
        return;
    }

    // TC-1: Normal call
    HRESULT hr = SafeCall_CanUnloadNow(pfn);
    TestResult("DllCanUnloadNow()", hr, S_OK);

    // TC-2: Verify reference count semantics (call repeatedly)
    for (int i = 0; i < 5; i++)
    {
        hr = SafeCall_CanUnloadNow(pfn);
        if (hr != S_OK)
        {
            printf("  FAIL: DllCanUnloadNow failed on iteration %d (0x%08lX)\n", i, hr);
            g_nFailed++;
            break;
        }
    }
    if (hr == S_OK)
    {
        printf("  PASS: DllCanUnloadNow repeated calls (5x)\n");
        g_nPassed++;
    }
}

static void Test_DllGetClassObject(HMODULE hMod)
{
    printf("\n=== DllGetClassObject ===\n");

    FN_DllGetClassObject pfn = (FN_DllGetClassObject)
        GetProcAddress(hMod, "DllGetClassObject");
    if (!pfn)
    {
        printf("  SKIP: GetProcAddress failed (0x%08lX)\n", GetLastError());
        g_nFailed++;
        return;
    }

    LPVOID pvObj = NULL;

    // TC-3: Known CLSID + IID_IUnknown
    pvObj = NULL;
    HRESULT hr = SafeCall_GetClassObject(pfn, CLSID_WLXSlideshow, IID_IUnknown, &pvObj);
    TestResult("DllGetClassObject(CLSID_WLXSlideshow, IID_IUnknown, &pv)",
               hr, CLASS_E_CLASSNOTAVAILABLE);
    if (pvObj != NULL)
        printf("  WARN: non-NULL pv returned despite failure\n");

    // TC-4: CLSID_NULL
    pvObj = NULL;
    hr = SafeCall_GetClassObject(pfn, GUID_NULL, IID_IUnknown, &pvObj);
    TestResult("DllGetClassObject(CLSID_NULL, IID_IUnknown, &pv)", hr, CLASS_E_CLASSNOTAVAILABLE);

    // TC-5: NULL ppv (boundary — should AV or return E_POINTER)
    printf("  INFO: DllGetClassObject(..., NULL) — expect AV or E_POINTER\n");
    __try
    {
        hr = pfn(CLSID_WLXSlideshow, IID_IUnknown, NULL);
        printf("  INFO: returned 0x%08lX (no AV)\n", hr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("  INFO: access violation caught (0x%08lX)\n", GetExceptionCode());
    }

    // TC-6: Invalid CLSID (all zeros)
    GUID guidInvalid = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
    pvObj = NULL;
    hr = SafeCall_GetClassObject(pfn, guidInvalid, IID_IUnknown, &pvObj);
    TestResult("DllGetClassObject(zeroed CLSID, IID_IUnknown)", hr, CLASS_E_CLASSNOTAVAILABLE);

    // TC-7: Invalid IID (all zeros)
    IID iidInvalid = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
    pvObj = NULL;
    hr = SafeCall_GetClassObject(pfn, CLSID_WLXSlideshow, iidInvalid, &pvObj);
    TestResult("DllGetClassObject(CLSID_WLXSlideshow, zeroed IID)", hr, CLASS_E_CLASSNOTAVAILABLE);
}

static void Test_DllRegisterServer(HMODULE hMod)
{
    printf("\n=== DllRegisterServer ===\n");

    FN_DllRegisterServer pfn = (FN_DllRegisterServer)
        GetProcAddress(hMod, "DllRegisterServer");
    if (!pfn)
    {
        printf("  SKIP: GetProcAddress failed (0x%08lX)\n", GetLastError());
        g_nFailed++;
        return;
    }

    // TC-8: Normal call
    HRESULT hr = SafeCall_RegisterServer(pfn);
    TestResult("DllRegisterServer()", hr, S_OK);

    // TC-9: Call twice (idempotency)
    hr = SafeCall_RegisterServer(pfn);
    TestResult("DllRegisterServer() [2nd call]", hr, S_OK);
}

static void Test_DllUnregisterServer(HMODULE hMod)
{
    printf("\n=== DllUnregisterServer ===\n");

    FN_DllUnregisterServer pfn = (FN_DllUnregisterServer)
        GetProcAddress(hMod, "DllUnregisterServer");
    if (!pfn)
    {
        printf("  SKIP: GetProcAddress failed (0x%08lX)\n", GetLastError());
        g_nFailed++;
        return;
    }

    // TC-10: Normal call
    HRESULT hr = SafeCall_UnregisterServer(pfn);
    TestResult("DllUnregisterServer()", hr, S_OK);

    // TC-11: Call twice (idempotency)
    hr = SafeCall_UnregisterServer(pfn);
    TestResult("DllUnregisterServer() [2nd call]", hr, S_OK);
}

// ---------------------------------------------------------------------------
// LoadLibrary + GetProcAddress integrity checks
// ---------------------------------------------------------------------------
static void Test_ModuleLoad(void)
{
    printf("=== Module Load ===\n");

    // TC-12: Load the DLL
    HMODULE hMod = LoadLibraryW(L"WLXSlideshow.dll");
    if (!hMod)
    {
        printf("  FAIL: LoadLibraryW(WLXSlideshow.dll) failed (0x%08lX)\n", GetLastError());
        g_nFailed++;
        return;
    }
    printf("  PASS: LoadLibraryW succeeded (handle 0x%p)\n", (void*)hMod);
    g_nPassed++;

    // TC-13: Verify all 4 exports exist
    const char* szExports[] = {
        "DllCanUnloadNow",
        "DllGetClassObject",
        "DllRegisterServer",
        "DllUnregisterServer"
    };
    int nFound = 0;
    for (int i = 0; i < 4; i++)
    {
        FARPROC p = GetProcAddress(hMod, szExports[i]);
        if (p)
        {
            printf("  FOUND: %s @ 0x%p\n", szExports[i], (void*)p);
            nFound++;
        }
        else
        {
            printf("  MISSING: %s (0x%08lX)\n", szExports[i], GetLastError());
        }
    }
    if (nFound == 4)
    {
        printf("  PASS: All 4 exports resolved\n");
        g_nPassed++;
    }
    else
    {
        printf("  FAIL: Only %d/4 exports resolved\n", nFound);
        g_nFailed++;
    }

    // Run all export tests
    Test_DllCanUnloadNow(hMod);
    Test_DllGetClassObject(hMod);
    Test_DllRegisterServer(hMod);
    Test_DllUnregisterServer(hMod);

    // TC-14: FreeLibrary
    BOOL bFree = FreeLibrary(hMod);
    TestResultBool("FreeLibrary", bFree, TRUE);
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int main()
{
    printf("WLXSlideshow.dll Test Harness\n");
    printf("=============================\n");
    printf("Build: 16.4.3528.0331\n\n");

    // Initialize COM (required for DllGetClassObject path in real implementation)
    HRESULT hrCo = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hrCo))
        printf("WARNING: CoInitializeEx failed (0x%08lX) — COM tests may be limited\n", hrCo);

    Test_ModuleLoad();

    if (SUCCEEDED(hrCo))
        CoUninitialize();

    printf("\n=============================\n");
    printf("Results: %d passed, %d failed\n", g_nPassed, g_nFailed);
    printf("=============================\n");

    return (g_nFailed > 0) ? 1 : 0;
}
```

## 3. CMakeLists.txt

```cmake
# WLXSlideshow test harness
# Tests all 4 COM exports via LoadLibrary + GetProcAddress

set(TARGET WLXSlideshowTest)

add_executable(${TARGET}
    main.cpp
)

target_compile_definitions(${TARGET} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(${TARGET} PRIVATE
    kernel32
    ole32
    oleaut32
)

set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/test"
)

# Deploy the DLL next to the test executable for LoadLibrary resolution
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/bin/WLXSlideshow.dll"
        "$<TARGET_FILE_DIR:${TARGET}>/WLXSlideshow.dll"
)
```

## 4. Expected Behaviors Documented

| Test Case | Export | Input | Expected Result | Rationale |
|-----------|--------|-------|-----------------|-----------|
| TC-1 | DllCanUnloadNow | () | S_OK | Always returns S_OK (no outstanding locks in stub) |
| TC-2 | DllCanUnloadNow | () x5 | S_OK | Idempotent; no reference counting side effects |
| TC-3 | DllGetClassObject | Known CLSID + IID_IUnknown | CLASS_E_CLASSNOTAVAILABLE | CLSID not registered via ATL; module not initialized |
| TC-4 | DllGetClassObject | CLSID_NULL + IID_IUnknown | CLASS_E_CLASSNOTAVAILABLE | No class object for null CLSID |
| TC-5 | DllGetClassObject | Known CLSID + IID + NULL ppv | E_POINTER or AV | Access violation possible; may crash without SEH guard |
| TC-6 | DllGetClassObject | Zeroed CLSID | CLASS_E_CLASSNOTAVAILABLE | All-zero CLSID never matches |
| TC-7 | DllGetClassObject | Known CLSID + zeroed IID | CLASS_E_CLASSNOTAVAILABLE | Invalid IID, class still not available |
| TC-8 | DllRegisterServer | () | S_OK | Stub implementation returns S_OK |
| TC-9 | DllRegisterServer | () x2 | S_OK | Idempotent call |
| TC-10 | DllUnregisterServer | () | S_OK | Stub implementation returns S_OK |
| TC-11 | DllUnregisterServer | () x2 | S_OK | Idempotent call |
| TC-12 | LoadLibrary | "WLXSlideshow.dll" | Valid HMODULE | DLL loads and DllMain succeeds |
| TC-13 | GetProcAddress | All 4 names | Non-NULL | All exports present in .def |
| TC-14 | FreeLibrary | HMODULE | TRUE | DLL unloads cleanly |

### Behavior Notes

1. **All four exports are stubs** in the reconstructed source. The original binary's implementations may have different behavior (e.g., DllRegisterServer may write registry keys under `Software\Microsoft\Windows Live\Photo Gallery\Slideshow`).

2. **DllGetClassObject does NOT create COM objects** in the current stub — it returns `CLASS_E_CLASSNOTAVAILABLE`. The original binary would return a class factory for CLSID_SimpleSlideshowDisplay, CLSID_SlideshowExtension, and CLSID_TimelineDisplay after ATL module initialization.

3. **Thread safety**: All four exports are safe to call from any thread. DllMain runs during process attach/detach only.

4. **Registry side effects**: DllRegisterServer / DllUnregisterServer in the original binary read/write:
   - `HKCU\Software\Microsoft\Windows Live\Photo Gallery\Slideshow`
   - `HKCU\Software\Microsoft\Windows Live\Photo Gallery\Slideshow\LastRunSettings`
   - COM CLSID registration under `HKCR\CLSID\{...}`

5. **GDI+ dependency**: DllMain initializes GDI+ via `GdiplusStartup`. The test harness should be built for the same platform (x86) and have `gdiplus.dll` available.

6. **CoInitialize required**: DllGetClassObject's COM path requires the calling thread to have called CoInitialize/CoInitializeEx. The harness calls it in main().

7. **SEH safety**: All export invocations are wrapped in `__try/__except` to catch access violations from invalid parameters (especially NULL ppv in DllGetClassObject).

8. **pTotalDuration output verification**: For the internal SlideshowEngine::Generate(), if pTotalDuration is NULL, the function still succeeds (S_OK) but does not write back the duration. This is per the source at `WLXSlideshow.cpp:202`.
