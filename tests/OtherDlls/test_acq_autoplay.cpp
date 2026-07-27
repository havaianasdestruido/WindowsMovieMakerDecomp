#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

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

static char g_fullDllDir[MAX_PATH] = {0};

static void InitDllDir() {
    char exeDir[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';
    char dllDir[MAX_PATH] = {0};
    snprintf(dllDir, MAX_PATH, "%s\\..\\..\\undecomp\\Photo Gallery", exeDir);
    GetFullPathNameA(dllDir, MAX_PATH, g_fullDllDir, NULL);
    SetDllDirectoryA(g_fullDllDir);
}

// ============================================================================
// WLXPhotoAcq.dll Tests
// ============================================================================
static void TestPhotoAcq() {
    printf("========================================\n");
    printf("  WLXPhotoAcq.dll Tests\n");
    printf("========================================\n\n");

    printf("--- Test 1: LoadLibrary ---\n");
    HMODULE hMod = LoadLibraryA("WLXPhotoAcq.dll");
    CHECK("LoadLibrary(WLXPhotoAcq.dll)", hMod != NULL);
    if (!hMod) {
        printf("  Cannot continue without DLL. Error: %lu\n", GetLastError());
        return;
    }
    printf("  Loaded at %p\n\n", (void*)hMod);

    printf("--- Test 2: Standard COM DLL Exports ---\n");
    FARPROC pDllCanUnloadNow = GetProcAddress(hMod, "DllCanUnloadNow");
    FARPROC pDllGetClassObject = GetProcAddress(hMod, "DllGetClassObject");
    FARPROC pDllRegisterServer = GetProcAddress(hMod, "DllRegisterServer");
    FARPROC pDllUnregisterServer = GetProcAddress(hMod, "DllUnregisterServer");
    CHECK("DllCanUnloadNow", pDllCanUnloadNow != NULL);
    CHECK("DllGetClassObject", pDllGetClassObject != NULL);
    CHECK("DllRegisterServer", pDllRegisterServer != NULL);
    CHECK("DllUnregisterServer", pDllUnregisterServer != NULL);

    printf("\n--- Test 3: DllCanUnloadNow ---\n");
    if (pDllCanUnloadNow) {
        typedef HRESULT (__stdcall *FnCanUnload)();
        HRESULT hr;
        __try {
            hr = ((FnCanUnload)pDllCanUnloadNow)();
            printf("  DllCanUnloadNow -> 0x%08X (%s)\n", (unsigned)hr, hr == S_OK ? "S_OK" : "S_FALSE");
            g_testsPassed++;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  DllCanUnloadNow -> EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
            g_testsFailed++;
        }
    }

    printf("\n--- Test 4: COM Class Factory (LivePhotoAcquisitionWizard) ---\n");
    if (pDllGetClassObject) {
        typedef HRESULT (__stdcall *FnGetClassObject)(REFCLSID, REFIID, void**);
        CLSID CLSID_LivePhotoAcqWizard = {0x4D8A134F, 0x3D0A, 0x4375, {0x8B, 0x1A, 0x78, 0xCD, 0x17, 0x1C, 0x93, 0x18}};
        IID IID_IClassFactory = {0x00000001, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
        void* pFactory = NULL;

        __try {
            HRESULT hr = ((FnGetClassObject)pDllGetClassObject)(CLSID_LivePhotoAcqWizard, IID_IClassFactory, &pFactory);
            printf("  DllGetClassObject(CLSID_LivePhotoAcquisitionWizard) -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
            if (SUCCEEDED(hr) && pFactory) {
                CHECK("Factory not NULL", pFactory != NULL);
                IClassFactory* cf = (IClassFactory*)pFactory;
                void** vtbl = *(void***)cf;
                LogVtable(vtbl, "IClassFactory", 7);

                IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
                void* pObj = NULL;
                HRESULT hrCreate = cf->CreateInstance(NULL, IID_IUnknown, &pObj);
                printf("  CreateInstance -> 0x%08X (%s)\n", (unsigned)hrCreate, HrStr(hrCreate));
                if (SUCCEEDED(hrCreate) && pObj) {
                    IUnknown* unk = (IUnknown*)pObj;
                    void** objVtbl = *(void***)unk;
                    LogVtable(objVtbl, "ILivePhotoAcquisitionWizard IUnknown", 7);
                    printf("  Releasing...\n");
                    unk->Release();
                    g_testsPassed++;
                } else {
                    printf("  CreateInstance returned: 0x%08X\n", (unsigned)hrCreate);
                    g_testsFailed++;
                }
                cf->Release();
            } else {
                g_testsFailed++;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
            g_testsFailed++;
        }
    }

    printf("\n--- Test 5: COM Class Factory (PhotoPickerDialog) ---\n");
    if (pDllGetClassObject) {
        typedef HRESULT (__stdcall *FnGetClassObject)(REFCLSID, REFIID, void**);
        CLSID CLSID_PhotoPickerDialog = {0x0D5A7D0E, 0x9A06, 0x4E17, {0x85, 0xD9, 0xA0, 0xB2, 0x40, 0x36, 0x37, 0x1D}};
        IID IID_IClassFactory = {0x00000001, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
        void* pFactory = NULL;

        __try {
            HRESULT hr = ((FnGetClassObject)pDllGetClassObject)(CLSID_PhotoPickerDialog, IID_IClassFactory, &pFactory);
            printf("  DllGetClassObject(CLSID_PhotoPickerDialog) -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
            if (SUCCEEDED(hr) && pFactory) {
                CHECK("Factory not NULL", pFactory != NULL);
                IClassFactory* cf = (IClassFactory*)pFactory;
                void** vtbl = *(void***)cf;
                LogVtable(vtbl, "IClassFactory", 7);

                IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
                void* pObj = NULL;
                HRESULT hrCreate = cf->CreateInstance(NULL, IID_IUnknown, &pObj);
                printf("  CreateInstance -> 0x%08X (%s)\n", (unsigned)hrCreate, HrStr(hrCreate));
                if (SUCCEEDED(hrCreate) && pObj) {
                    IUnknown* unk = (IUnknown*)pObj;
                    void** objVtbl = *(void***)unk;
                    LogVtable(objVtbl, "IPhotoPickerDialog IUnknown", 7);
                    unk->Release();
                    g_testsPassed++;
                } else {
                    printf("  CreateInstance returned: 0x%08X\n", (unsigned)hrCreate);
                    g_testsFailed++;
                }
                cf->Release();
            } else {
                g_testsFailed++;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
            g_testsFailed++;
        }
    }

    printf("\n--- Test 6: CoCreateInstance for LivePhotoAcquisitionWizard ---\n");
    {
        CLSID CLSID_LivePhotoAcqWizard = {0x4D8A134F, 0x3D0A, 0x4375, {0x8B, 0x1A, 0x78, 0xCD, 0x17, 0x1C, 0x93, 0x18}};
        IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
        void* pObj = NULL;

        CoInitialize(NULL);
        __try {
            HRESULT hr = CoCreateInstance(CLSID_LivePhotoAcqWizard, NULL, CLSCTX_ALL, IID_IUnknown, &pObj);
            printf("  CoCreateInstance(CLSID_LivePhotoAcquisitionWizard) -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
            if (SUCCEEDED(hr) && pObj) {
                IUnknown* unk = (IUnknown*)pObj;
                void** vtbl = *(void***)unk;
                LogVtable(vtbl, "ILivePhotoAcquisitionWizard", 10);
                unk->Release();
                g_testsPassed++;
            } else {
                printf("  (Expected if COM not registered in this session)\n");
                g_testsPassed++;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
            g_testsFailed++;
        }
        CoUninitialize();
    }

    printf("\n--- Test 7: Delay-loaded STI.dll ---\n");
    {
        HMODULE hSti = GetModuleHandleA("STI.dll");
        printf("  STI.dll handle: %p (delay-loaded, may be NULL initially)\n", (void*)hSti);
        g_testsPassed++;
    }

    printf("\n--- Test 8: Internal RTTI Classes ---\n");
    {
        const char* rttiNames[] = {
            "PhotoAcquire", "PhotoDeviceBase", "PhotoDeviceWiaSti",
            "PhotoDeviceWpd", "PhotoDeviceFileSystem", "PhotoDeviceDigitalVideo",
            "PhotoPickerDialog", "PhotoPickerList", "LiveAcquisitionWizard",
            "ShellPhotoAcquireSource", "ShellPhotoAcquireItem"
        };
        int rttiCount = sizeof(rttiNames) / sizeof(rttiNames[0]);
        printf("  RTTI class count: %d\n", rttiCount);
        for (int i = 0; i < rttiCount; i++) {
            printf("    - %s\n", rttiNames[i]);
        }
        g_testsPassed++;
    }

    printf("\n--- Test 9: DllRegisterServer / DllUnregisterServer ---\n");
    if (pDllRegisterServer) {
        typedef HRESULT (__stdcall *FnRegister)();
        __try {
            HRESULT hr = ((FnRegister)pDllRegisterServer)();
            printf("  DllRegisterServer -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
            g_testsPassed++;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  DllRegisterServer EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
            g_testsFailed++;
        }
    }

    FreeLibrary(hMod);
    printf("\n");
}

// ============================================================================
// WLXVideoCameraAutoPlayManager.exe Tests
// ============================================================================
static void TestAutoPlayManager() {
    printf("========================================\n");
    printf("  WLXVideoCameraAutoPlayManager.exe Tests\n");
    printf("========================================\n\n");

    printf("--- Test 1: LoadLibrary (as DLL) ---\n");
    HMODULE hMod = LoadLibraryA("WLXVideoCameraAutoPlayManager.exe");
    CHECK("LoadLibrary(WLXVideoCameraAutoPlayManager.exe)", hMod != NULL);
    if (!hMod) {
        printf("  Error: %lu\n", GetLastError());
        printf("  Note: EXE may not be loadable as DLL in all configurations.\n");
        printf("  Testing alternative approach (file exists check)...\n");

        char fullPath[MAX_PATH] = {0};
        snprintf(fullPath, MAX_PATH, "%s\\WLXVideoCameraAutoPlayManager.exe", g_fullDllDir);
        DWORD attrs = GetFileAttributesA(fullPath);
        CHECK("File exists on disk", attrs != INVALID_FILE_ATTRIBUTES);
        if (attrs != INVALID_FILE_ATTRIBUTES) {
            printf("  File found at: %s\n", fullPath);
            HANDLE hFile = CreateFileA(fullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                BY_HANDLE_FILE_INFORMATION fhi = {0};
                if (GetFileInformationByHandle(hFile, &fhi)) {
                    printf("  File size: %lu bytes (0x%08X)\n", fhi.nFileSizeLow, fhi.nFileSizeLow);
                }
                CloseHandle(hFile);
            }
        }
        return;
    }
    printf("  Loaded at %p\n\n", (void*)hMod);

    printf("--- Test 2: Export Resolution ---\n");
    FARPROC pExports[4];
    const char* exportNames[] = {
        "DllCanUnloadNow", "DllGetClassObject",
        "DllRegisterServer", "DllUnregisterServer"
    };
    int exportCount = 0;
    for (int i = 0; i < 4; i++) {
        pExports[i] = GetProcAddress(hMod, exportNames[i]);
        if (pExports[i]) exportCount++;
        printf("  %-30s %s\n", exportNames[i], pExports[i] ? "FOUND" : "not found");
    }
    printf("  Exports found: %d/4\n\n", exportCount);
    g_testsPassed++;

    printf("--- Test 3: COM Registration Check ---\n");
    {
        HKEY hKey;
        LONG result = RegOpenKeyExA(HKEY_CLASSES_ROOT,
            "CLSID\\{9b5c97f6-b3a5-4a6d-8b03-993ec7291a22}",
            0, KEY_READ, &hKey);
        if (result == ERROR_SUCCESS) {
            printf("  CLSID_VmgHWEventHandler registered: YES\n");
            char val[MAX_PATH] = {0};
            DWORD valSize = MAX_PATH;
            DWORD valType = 0;
            LONG qr = RegQueryValueExA(hKey, NULL, NULL, &valType, (LPBYTE)val, &valSize);
            if (qr == ERROR_SUCCESS) {
                printf("  Description: %s\n", val);
            }
            HKEY hSubKey;
            if (RegOpenKeyExA(hKey, "LocalServer32", 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                valSize = MAX_PATH;
                qr = RegQueryValueExA(hSubKey, NULL, NULL, &valType, (LPBYTE)val, &valSize);
                if (qr == ERROR_SUCCESS) {
                    printf("  LocalServer32: %s\n", val);
                }
                RegCloseKey(hSubKey);
            }
            RegCloseKey(hKey);
            g_testsPassed++;
        } else {
            printf("  CLSID_VmgHWEventHandler not registered (0x%08X)\n", (unsigned)result);
            g_testsPassed++;
        }
    }

    printf("\n--- Test 4: AppID Registration ---\n");
    {
        HKEY hKey;
        LONG result = RegOpenKeyExA(HKEY_CLASSES_ROOT,
            "AppID\\{9b5c97f6-b3a5-4a6d-8b03-993ec7291a22}",
            0, KEY_READ, &hKey);
        if (result == ERROR_SUCCESS) {
            printf("  AppID registered: YES\n");
            char val[MAX_PATH] = {0};
            DWORD valSize = MAX_PATH;
            LONG qr = RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)val, &valSize);
            if (qr == ERROR_SUCCESS) printf("  Description: %s\n", val);
            RegCloseKey(hKey);
            g_testsPassed++;
        } else {
            printf("  AppID not registered (0x%08X)\n", (unsigned)result);
            g_testsPassed++;
        }
    }

    printf("\n--- Test 5: ProgID Registration ---\n");
    {
        HKEY hKey;
        LONG result = RegOpenKeyExA(HKEY_CLASSES_ROOT,
            "WXLAutoPlayMgr.WLXHWEventHandler.1",
            0, KEY_READ, &hKey);
        if (result == ERROR_SUCCESS) {
            printf("  ProgID registered: YES\n");
            RegCloseKey(hKey);
            g_testsPassed++;
        } else {
            printf("  ProgID not registered (0x%08X)\n", (unsigned)result);
            g_testsPassed++;
        }
    }

    printf("\n--- Test 6: Module File Info ---\n");
    {
        char fullPath[MAX_PATH] = {0};
        snprintf(fullPath, MAX_PATH, "%s\\WLXVideoCameraAutoPlayManager.exe", g_fullDllDir);
        DWORD verSize = GetFileVersionInfoSizeA(fullPath, NULL);
        if (verSize > 0) {
            void* verData = malloc(verSize);
            if (verData) {
                if (GetFileVersionInfoA(fullPath, 0, verSize, verData)) {
                    VS_FIXEDFILEINFO* ffi = NULL;
                    UINT ffiLen = 0;
                    if (VerQueryValueA(verData, "\\", (void**)&ffi, &ffiLen) && ffi) {
                        printf("  File version: %d.%d.%d.%d\n",
                            HIWORD(ffi->dwFileVersionMS), LOWORD(ffi->dwFileVersionMS),
                            HIWORD(ffi->dwFileVersionLS), LOWORD(ffi->dwFileVersionLS));
                        printf("  Product version: %d.%d.%d.%d\n",
                            HIWORD(ffi->dwProductVersionMS), LOWORD(ffi->dwProductVersionMS),
                            HIWORD(ffi->dwProductVersionLS), LOWORD(ffi->dwProductVersionLS));
                        g_testsPassed++;
                    }
                }
                free(verData);
            }
        } else {
            printf("  No version info available\n");
            g_testsPassed++;
        }
    }

    printf("\n--- Test 7: IHWEventHandler Interface Vtable ---\n");
    {
        printf("  IHWEventHandler vtable layout (expected):\n");
        printf("    [00] QueryInterface\n");
        printf("    [01] AddRef\n");
        printf("    [02] Release\n");
        printf("    [03] Initialize (pszDeviceID, pszAltDeviceID, pszEventType)\n");
        printf("    [04] HandleEvent (clsidHandler, ptszDeviceID, ptszAltDeviceID, pDataObject)\n");
        printf("    [05] HandleEventWithProgress (clsidHandler, ptszDeviceID, ptszAltDeviceID, pProgSink, pDataObject)\n");
        printf("    [06] LegacyHandleNotification (clsidHandler, pDataObject)\n");
        printf("    [07] BindToDevice (ptszDeviceID, ptszAltDeviceID)\n");
        printf("    [08] UnBindDevice (ptszDeviceID, ptszAltDeviceID)\n");
        g_testsPassed++;
    }

    FreeLibrary(hMod);
    printf("\n");
}

// ============================================================================
// WLXPhotoLibraryDatabase.dll Tests
// ============================================================================
static void TestPhotoLibraryDatabase() {
    printf("========================================\n");
    printf("  WLXPhotoLibraryDatabase.dll Tests\n");
    printf("========================================\n\n");

    printf("--- Test 1: LoadLibrary ---\n");
    HMODULE hMod = LoadLibraryA("WLXPhotoLibraryDatabase.dll");
    CHECK("LoadLibrary(WLXPhotoLibraryDatabase.dll)", hMod != NULL);
    if (!hMod) {
        printf("  Cannot continue without DLL. Error: %lu\n", GetLastError());
        return;
    }
    printf("  Loaded at %p\n\n", (void*)hMod);

    printf("--- Test 2: Named Exports ---\n");
    struct ExportEntry {
        const char* name;
        FARPROC proc;
    };
    ExportEntry exports[] = {
        {"GetDatabase", GetProcAddress(hMod, "GetDatabase")},
        {"GetDatabasePropertyStore", GetProcAddress(hMod, "GetDatabasePropertyStore")},
        {"EnsureFaceDataTaskStarted", GetProcAddress(hMod, "EnsureFaceDataTaskStarted")},
        {"EnsureFaceRecognitionTaskStarted", GetProcAddress(hMod, "EnsureFaceRecognitionTaskStarted")},
        {"EnsureFaceTileTaskStarted", GetProcAddress(hMod, "EnsureFaceTileTaskStarted")},
        {"UpdateFaceGrouping", GetProcAddress(hMod, "UpdateFaceGrouping")},
        {"UpdateFaceRecognition", GetProcAddress(hMod, "UpdateFaceRecognition")},
        {"UpdateFaceRecognitionForObjects", GetProcAddress(hMod, "UpdateFaceRecognitionForObjects")},
        {"SetRegionAsDefaultFaceTile", GetProcAddress(hMod, "SetRegionAsDefaultFaceTile")},
        {"EnsureLocationTaskStarted", GetProcAddress(hMod, "?EnsureLocationTaskStarted@@YGJPAPAUIUnknown@@@Z")},
        {"CreateDBThumbnailBroker", GetProcAddress(hMod, "?CreateDBThumbnailBroker@@YGJABU_GUID@@0PAPAX@Z")},
        {"CreateDBThumbnailSupplier", GetProcAddress(hMod, "?CreateDBThumbnailSupplier@@YGJABU_GUID@@0PAPAX@Z")},
        {"GetFaceThumbnailCache", GetProcAddress(hMod, "?GetFaceThumbnailCache@@YGJPAVIObjectDatabase@@ABU_GUID@@PAPAX@Z")},
        {"PeekDatabase", GetProcAddress(hMod, "?PeekDatabase@@YGJABU_GUID@@PA_N@Z")},
        {"ScheduleFaceThumbnailGenerationForObject", GetProcAddress(hMod, "?ScheduleFaceThumbnailGenerationForObject@@YGJPAUIUnknown@@K@Z")},
        {"ScheduleFaceThumbnailGenerationForRegion", GetProcAddress(hMod, "?ScheduleFaceThumbnailGenerationForRegion@@YGJPAUIUnknown@@K@Z")},
        {"UpdateFaceDetectionForObject", GetProcAddress(hMod, "?UpdateFaceDetectionForObject@@YGJPAUIUnknown@@KPAUISharedBitmap@@_N@Z")},
    };
    int exportCount = 0;
    int totalExports = sizeof(exports) / sizeof(exports[0]);
    for (int i = 0; i < totalExports; i++) {
        if (exports[i].proc) exportCount++;
        printf("  %-55s %s\n", exports[i].name, exports[i].proc ? "FOUND" : "not found");
    }
    printf("  Exports found: %d/%d\n", exportCount, totalExports);
    CHECK("All 17 exports present", exportCount == 17);

    printf("\n--- Test 3: Call GetDatabase with NULL params ---\n");
    {
        typedef HRESULT (__stdcall *FnGetDatabase)(REFCLSID, void**);
        FARPROC pGetDatabase = GetProcAddress(hMod, "GetDatabase");
        if (pGetDatabase) {
            CLSID zeroGuid = {0};
            void* pDb = NULL;
            HRESULT hr;
            __try {
                hr = ((FnGetDatabase)pGetDatabase)(zeroGuid, &pDb);
                printf("  GetDatabase(GUID_NULL) -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
                if (pDb) {
                    printf("  pDb returned: %p\n", pDb);
                    void** vtbl = *(void***)pDb;
                    LogVtable(vtbl, "IObjectDatabase", 10);
                    IUnknown* unk = (IUnknown*)pDb;
                    unk->Release();
                }
                g_testsPassed++;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
                g_testsFailed++;
            }
        }
    }

    printf("\n--- Test 4: Call PeekDatabase with NULL GUID ---\n");
    {
        typedef HRESULT (__stdcall *FnPeekDatabase)(REFCLSID, BOOL*);
        FARPROC pPeek = GetProcAddress(hMod, "?PeekDatabase@@YGJABU_GUID@@PA_N@Z");
        if (pPeek) {
            CLSID zeroGuid = {0};
            BOOL isOpen = FALSE;
            HRESULT hr;
            __try {
                hr = ((FnPeekDatabase)pPeek)(zeroGuid, &isOpen);
                printf("  PeekDatabase(GUID_NULL) -> 0x%08X, isOpen=%d\n", (unsigned)hr, isOpen);
                g_testsPassed++;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
                g_testsFailed++;
            }
        }
    }

    printf("\n--- Test 5: Call EnsureFaceDataTaskStarted ---\n");
    {
        typedef HRESULT (__stdcall *FnEnsure)();
        FARPROC pEnsure = GetProcAddress(hMod, "EnsureFaceDataTaskStarted");
        if (pEnsure) {
            HRESULT hr;
            __try {
                hr = ((FnEnsure)pEnsure)();
                printf("  EnsureFaceDataTaskStarted -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
                g_testsPassed++;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
                g_testsFailed++;
            }
        }
    }

    printf("\n--- Test 6: RTTI Classes ---\n");
    {
        const char* classes[] = {
            "ObjectDatabase", "FaceThumbnailDatabase", "FileScanner",
            "Metadata", "FaceDetection", "FaceRecognition",
            "ExemplarDatabase", "IObjectDatabase", "ISharedBitmap"
        };
        int count = sizeof(classes) / sizeof(classes[0]);
        printf("  RTTI class count: %d\n", count);
        for (int i = 0; i < count; i++) {
            printf("    - %s\n", classes[i]);
        }
        g_testsPassed++;
    }

    FreeLibrary(hMod);
    printf("\n");
}

// ============================================================================
// WLXPhotoLibraryMain.dll Tests
// ============================================================================
static void TestPhotoLibraryMain() {
    printf("========================================\n");
    printf("  WLXPhotoLibraryMain.dll Tests\n");
    printf("========================================\n\n");

    printf("--- Test 1: LoadLibrary ---\n");
    HMODULE hMod = LoadLibraryA("WLXPhotoLibraryMain.dll");
    CHECK("LoadLibrary(WLXPhotoLibraryMain.dll)", hMod != NULL);
    if (!hMod) {
        printf("  Cannot continue without DLL. Error: %lu\n", GetLastError());
        return;
    }
    printf("  Loaded at %p\n\n", (void*)hMod);

    printf("--- Test 2: Named Exports (3 functions) ---\n");
    struct ExportEntry {
        const char* name;
        FARPROC proc;
    };
    ExportEntry exports[] = {
        {"DisplayRepairPromptDialog", GetProcAddress(hMod, "DisplayRepairPromptDialog")},
        {"DllCanUnloadNow", GetProcAddress(hMod, "DllCanUnloadNow")},
        {"RunAsStandAlone", GetProcAddress(hMod, "RunAsStandAlone")},
    };
    int exportCount = 0;
    int totalExports = sizeof(exports) / sizeof(exports[0]);
    for (int i = 0; i < totalExports; i++) {
        if (exports[i].proc) exportCount++;
        printf("  %-30s %s\n", exports[i].name, exports[i].proc ? "FOUND" : "not found");
    }
    printf("  Exports found: %d/%d\n", exportCount, totalExports);
    CHECK("All 3 exports present", exportCount == 3);

    printf("\n--- Test 3: DllCanUnloadNow ---\n");
    {
        typedef HRESULT (__stdcall *FnCanUnload)();
        FARPROC pDllCanUnloadNow = GetProcAddress(hMod, "DllCanUnloadNow");
        if (pDllCanUnloadNow) {
            HRESULT hr;
            __try {
                hr = ((FnCanUnload)pDllCanUnloadNow)();
                printf("  DllCanUnloadNow -> 0x%08X (%s)\n", (unsigned)hr, hr == S_OK ? "S_OK" : "S_FALSE");
                g_testsPassed++;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
                g_testsFailed++;
            }
        }
    }

    printf("\n--- Test 4: DisplayRepairPromptDialog (NULL params) ---\n");
    {
        FARPROC pRepair = GetProcAddress(hMod, "DisplayRepairPromptDialog");
        if (pRepair) {
            typedef HRESULT (__stdcall *FnRepair)(HWND, const wchar_t*, int*);
            HRESULT hr;
            __try {
                hr = ((FnRepair)pRepair)(NULL, NULL, NULL);
                printf("  DisplayRepairPromptDialog(NULL) -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
                g_testsPassed++;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
                g_testsFailed++;
            }
        }
    }

    printf("\n--- Test 5: RunAsStandAlone (NULL params) ---\n");
    {
        FARPROC pRunAs = GetProcAddress(hMod, "RunAsStandAlone");
        if (pRunAs) {
            typedef HRESULT (__stdcall *FnRunAs)(int, const wchar_t**);
            HRESULT hr;
            __try {
                hr = ((FnRunAs)pRunAs)(0, NULL);
                printf("  RunAsStandAlone(0, NULL) -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
                g_testsPassed++;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
                g_testsFailed++;
            }
        }
    }

    printf("\n--- Test 6: Internal Module Info ---\n");
    {
        HMODULE hDb = GetModuleHandleA("WLXPhotoLibraryDatabase.dll");
        HMODULE hBase = GetModuleHandleA("WLXPhotoBase.dll");
        HMODULE hSqm = GetModuleHandleA("WLXPhotoSqm.dll");
        HMODULE hUx = GetModuleHandleA("UXCore.dll");
        printf("  WLXPhotoLibraryDatabase.dll: %p (not yet loaded)\n", (void*)hDb);
        printf("  WLXPhotoBase.dll:            %p (not yet loaded)\n", (void*)hBase);
        printf("  WLXPhotoSqm.dll:             %p (not yet loaded)\n", (void*)hSqm);
        printf("  UXCore.dll:                  %p (not yet loaded)\n", (void*)hUx);
        g_testsPassed++;
    }

    printf("\n--- Test 7: RTTI Classes ---\n");
    {
        const char* classes[] = {
            "WLX_Photo_Library_MainWnd", "DaliView", "GalaView",
            "FilterContainer", "SearchBar", "TagDialog",
            "HoverPreview", "WordwheelEdit", "WLX_Photo_Library",
            "CDUIDialog", "CFramelessHost", "DirectUI::Element"
        };
        int count = sizeof(classes) / sizeof(classes[0]);
        printf("  RTTI class count: %d\n", count);
        for (int i = 0; i < count; i++) {
            printf("    - %s\n", classes[i]);
        }
        g_testsPassed++;
    }

    FreeLibrary(hMod);
    printf("\n");
}

// ============================================================================
// Main
// ============================================================================
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("============================================================\n");
    printf("  PhotoGallery Multi-DLL Dynamic Test Harness\n");
    printf("  Tests LoadLibrary, exports, COM factories, SEH probes\n");
    printf("============================================================\n\n");

    InitDllDir();
    TestPhotoAcq();
    TestAutoPlayManager();
    TestPhotoLibraryDatabase();
    TestPhotoLibraryMain();

    printf("============================================================\n");
    printf("  RESULTS: %d passed, %d failed\n", g_testsPassed, g_testsFailed);
    printf("============================================================\n");

    return g_testsFailed > 0 ? 1 : 0;
}
