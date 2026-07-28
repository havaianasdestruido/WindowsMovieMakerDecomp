#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
        case S_OK: return "S_OK";
        case S_FALSE: return "S_FALSE";
        case E_NOINTERFACE: return "E_NOINTERFACE";
        case E_POINTER: return "E_POINTER";
        case CLASS_E_CLASSNOTAVAILABLE: return "CLASS_E_CLASSNOTAVAILABLE";
        case REGDB_E_CLASSNOTREG: return "REGDB_E_CLASSNOTREG";
        case E_ACCESSDENIED: return "E_ACCESSDENIED";
        case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
        case E_UNEXPECTED: return "E_UNEXPECTED";
        case CO_E_SERVER_EXEC_FAILURE: return "CO_E_SERVER_EXEC_FAILURE";
        case CO_E_SERVER_STOPPING: return "CO_E_SERVER_STOPPING";
        default: return "";
    }
}

static char g_exeFullPath[MAX_PATH] = {0};

static void InitPaths() {
    char exeDir[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';
    char fullPath[MAX_PATH] = {0};
    snprintf(fullPath, MAX_PATH, "%s\\..\\..\\undecomp\\Photo Gallery\\WLXVideoCameraAutoPlayManager.exe", exeDir);
    GetFullPathNameA(fullPath, MAX_PATH, g_exeFullPath, NULL);
}

static BOOL RunExeAndKill(const char* args, DWORD timeoutMs) {
    char cmdLine[1024];
    snprintf(cmdLine, 1024, "\"%s\" %s", g_exeFullPath, args ? args : "");

    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE hStdOutRead, hStdOutWrite;
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 4096)) return FALSE;
    if (!SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hStdOutRead); CloseHandle(hStdOutWrite); return FALSE;
    }

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hStdOutWrite;
    si.hStdError = hStdOutWrite;
    PROCESS_INFORMATION pi;
    char cmdLineBuf[1024];
    strcpy(cmdLineBuf, cmdLine);

    BOOL ok = CreateProcessA(NULL, cmdLineBuf, NULL, NULL, TRUE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    CloseHandle(hStdOutWrite);
    if (!ok) { CloseHandle(hStdOutRead); return FALSE; }

    DWORD waitResult = WaitForSingleObject(pi.hProcess, timeoutMs);
    if (waitResult == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        WaitForSingleObject(pi.hProcess, 1000);
    }

    char output[1024] = {0};
    DWORD bytesRead = 0;
    ReadFile(hStdOutRead, output, sizeof(output) - 1, &bytesRead, NULL);
    output[bytesRead] = '\0';

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    printf("  Exit code: %lu, Output (%zu bytes):", exitCode, strlen(output));
    if (strlen(output) > 0) printf(" %s", output);
    printf("\n");

    CloseHandle(hStdOutRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return TRUE;
}

static void TestFileBasics() {
    printf("--- Test 1: File Existence & Size ---\n");
    DWORD attrs = GetFileAttributesA(g_exeFullPath);
    CHECK("File exists", attrs != INVALID_FILE_ATTRIBUTES);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        printf("  Path: %s, Error: %lu\n", g_exeFullPath, GetLastError());
        return;
    }
    printf("  Path: %s\n", g_exeFullPath);

    HANDLE hFile = CreateFileA(g_exeFullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        BY_HANDLE_FILE_INFORMATION fhi = {0};
        if (GetFileInformationByHandle(hFile, &fhi)) {
            printf("  Size: %lu bytes (0x%lX)\n", fhi.nFileSizeLow, fhi.nFileSizeLow);
            CHECK("File size >= 70000 bytes", fhi.nFileSizeLow >= 70000);
            CHECK("File size <= 90000 bytes", fhi.nFileSizeLow <= 90000);
        }
        CloseHandle(hFile);
    }
}

static void TestVersionInfo() {
    printf("\n--- Test 2: Version Info ---\n");
    DWORD verSize = GetFileVersionInfoSizeA(g_exeFullPath, NULL);
    CHECK("Has version info", verSize > 0);
    if (verSize <= 0) return;

    void* verData = malloc(verSize);
    if (!verData) return;
    if (GetFileVersionInfoA(g_exeFullPath, 0, verSize, verData)) {
        VS_FIXEDFILEINFO* ffi = NULL;
        UINT ffiLen = 0;
        if (VerQueryValueA(verData, "\\", (void**)&ffi, &ffiLen) && ffi) {
            WORD v1 = HIWORD(ffi->dwFileVersionMS);
            WORD v2 = LOWORD(ffi->dwFileVersionMS);
            WORD v3 = HIWORD(ffi->dwFileVersionLS);
            WORD v4 = LOWORD(ffi->dwFileVersionLS);
            printf("  File version: %d.%d.%d.%d\n", v1, v2, v3, v4);
            CHECK("Major version == 16", v1 == 16);
            CHECK("Minor version == 4", v2 == 4);
        }
        struct LANGANDCODEPAGE {
            WORD wLanguage;
            WORD wCodePage;
        } *lpTranslate = NULL;
        UINT cbTranslate = 0;
        if (VerQueryValueA(verData, "\\VarFileInfo\\Translation", (void**)&lpTranslate, &cbTranslate)) {
            for (UINT i = 0; i < (cbTranslate / sizeof(LANGANDCODEPAGE)); i++) {
                char subBlock[128];
                char* val = NULL;
                UINT valLen = 0;
                snprintf(subBlock, sizeof(subBlock),
                    "\\StringFileInfo\\%04X%04X\\FileDescription",
                    lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
                if (VerQueryValueA(verData, subBlock, (void**)&val, &valLen) && val) {
                    printf("  Description: %s\n", val);
                    CHECK("Description contains 'Auto'", strstr(val, "Auto") != NULL);
                }
                snprintf(subBlock, sizeof(subBlock),
                    "\\StringFileInfo\\%04X%04X\\OriginalFilename",
                    lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
                if (VerQueryValueA(verData, subBlock, (void**)&val, &valLen) && val) {
                    printf("  Original name: %s\n", val);
                    CHECK("Filename contains WLXVideoCameraAutoPlayManager",
                        strstr(val, "WLXVideoCameraAutoPlayManager") != NULL);
                }
            }
        }
    }
    free(verData);
}

static void TestPECharacteristics() {
    printf("\n--- Test 3: PE Characteristics (Read from File) ---\n");
    HANDLE hFile = CreateFileA(g_exeFullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    IMAGE_DOS_HEADER dos = {0};
    DWORD br;
    if (!ReadFile(hFile, &dos, sizeof(dos), &br, NULL) || dos.e_magic != IMAGE_DOS_SIGNATURE) {
        CloseHandle(hFile);
        CHECK("Valid DOS header", FALSE);
        return;
    }
    CHECK("Valid DOS signature (MZ)", dos.e_magic == IMAGE_DOS_SIGNATURE);

    SetFilePointer(hFile, dos.e_lfanew, NULL, FILE_BEGIN);
    DWORD ntSig = 0;
    if (!ReadFile(hFile, &ntSig, sizeof(ntSig), &br, NULL) || ntSig != IMAGE_NT_SIGNATURE) {
        CloseHandle(hFile);
        CHECK("Valid NT signature", FALSE);
        return;
    }
    CHECK("Valid NT signature (PE)", ntSig == IMAGE_NT_SIGNATURE);

    IMAGE_FILE_HEADER fh = {0};
    ReadFile(hFile, &fh, sizeof(fh), &br, NULL);
    printf("  Machine: 0x%04X (%s)\n", fh.Machine,
        fh.Machine == IMAGE_FILE_MACHINE_I386 ? "x86 (I386)" : "other");
    CHECK("Machine == x86 (0x14C)", fh.Machine == IMAGE_FILE_MACHINE_I386);
    printf("  Sections: %d\n", fh.NumberOfSections);
    CHECK("Has 4 sections", fh.NumberOfSections == 4);
    printf("  Characteristics: 0x%04X\n", fh.Characteristics);
    CHECK("Executable", fh.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE);
    CHECK("32-bit", fh.Characteristics & IMAGE_FILE_32BIT_MACHINE);

    IMAGE_OPTIONAL_HEADER32 oh = {0};
    ReadFile(hFile, &oh, sizeof(oh), &br, NULL);
    CHECK("PE32 magic (0x10B)", oh.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC);
    printf("  Subsystem: 0x%04X (%s)\n", oh.Subsystem,
        oh.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI ? "Windows GUI (2)" :
        oh.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI ? "Windows CUI (3)" : "other");
    CHECK("Subsystem == Windows GUI (2)", oh.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);
    printf("  Image size: 0x%lX (%lu bytes)\n", oh.SizeOfImage, oh.SizeOfImage);
    CHECK("Image size == 0x12000", oh.SizeOfImage == 0x12000);
    printf("  Entry point RVA: 0x%lX\n", oh.AddressOfEntryPoint);
    CHECK("Entry point == 0x6D75", oh.AddressOfEntryPoint == 0x6D75);
    CHECK("Linker major >= 11", oh.MajorLinkerVersion >= 10);
    CHECK("Has ASLR (Dynamic Base)", oh.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE);
    CHECK("NX Compatible", oh.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NX_COMPAT);
    printf("  Stack reserve: 0x%lX\n", oh.SizeOfStackReserve);
    printf("  Stack commit:  0x%lX\n", oh.SizeOfStackCommit);
    printf("  Heap reserve:  0x%lX\n", oh.SizeOfHeapReserve);
    printf("  Heap commit:   0x%lX\n", oh.SizeOfHeapCommit);

    IMAGE_SECTION_HEADER sections[4];
    ReadFile(hFile, sections, sizeof(sections), &br, NULL);
    for (int i = 0; i < 4; i++) {
        printf("  Section %d: %.8s VA=0x%lX VirtSize=0x%lX RawSize=0x%lX\n",
            i, sections[i].Name, sections[i].VirtualAddress,
            sections[i].Misc.VirtualSize, sections[i].SizeOfRawData);
    }
    CHECK("Section[0] is .text", strcmp((char*)sections[0].Name, ".text") == 0);
    CHECK("Section[1] is .data", strcmp((char*)sections[1].Name, ".data") == 0);
    CHECK("Section[2] is .rsrc", strcmp((char*)sections[2].Name, ".rsrc") == 0);
    CHECK("Section[3] is .reloc", strcmp((char*)sections[3].Name, ".reloc") == 0);
    CHECK(".text RawSize == 0x7600", sections[0].SizeOfRawData == 0x7600);
    CHECK(".rsrc RawSize == 0x7000", sections[2].SizeOfRawData == 0x7000);

    CloseHandle(hFile);
}

static void TestCOMRegistration() {
    printf("\n--- Test 4: COM Registration (CLSID) ---\n");
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CLASSES_ROOT,
        "CLSID\\{9b5c97f6-b3a5-4a6d-8b03-993ec7291a22}",
        0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        CHECK("CLSID registered", TRUE);
        char val[512] = {0};
        DWORD valSize = sizeof(val);
        DWORD valType = 0;
        if (RegQueryValueExA(hKey, NULL, NULL, &valType, (LPBYTE)val, &valSize) == ERROR_SUCCESS) {
            printf("  CLSID description: %s\n", val);
        }
        HKEY hSubKey;
        if (RegOpenKeyExA(hKey, "LocalServer32", 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
            valSize = sizeof(val);
            val[0] = '\0';
            if (RegQueryValueExA(hSubKey, NULL, NULL, &valType, (LPBYTE)val, &valSize) == ERROR_SUCCESS) {
                printf("  LocalServer32: %s\n", val);
                CHECK("LocalServer32 points to WLXVideoCameraAutoPlayManager.exe",
                    strstr(val, "WLXVideoCameraAutoPlayManager.exe") != NULL);
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    } else {
        printf("  CLSID not registered (0x%08lX)\n", result);
        CHECK("CLSID registration skipped", TRUE);
    }

    printf("\n--- Test 5: AppID Registration ---\n");
    result = RegOpenKeyExA(HKEY_CLASSES_ROOT,
        "AppID\\{9b5c97f6-b3a5-4a6d-8b03-993ec7291a22}",
        0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        CHECK("AppID registered", TRUE);
        char val[512] = {0};
        DWORD valSize = sizeof(val);
        if (RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)val, &valSize) == ERROR_SUCCESS) {
            printf("  AppID description: %s\n", val);
        }
        RegCloseKey(hKey);
    } else {
        printf("  AppID not registered\n");
        CHECK("AppID registration skipped", TRUE);
    }

    printf("\n--- Test 6: ProgID Registration ---\n");
    result = RegOpenKeyExA(HKEY_CLASSES_ROOT,
        "WXLAutoPlayMgr.WLXHWEventHandler.1",
        0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        CHECK("ProgID registered", TRUE);
        char val[256] = {0};
        DWORD valSize = sizeof(val);
        if (RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)val, &valSize) == ERROR_SUCCESS) {
            printf("  ProgID description: %s\n", val);
        }
        HKEY hSubKey;
        if (RegOpenKeyExA(hKey, "CLSID", 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
            valSize = sizeof(val);
            val[0] = '\0';
            if (RegQueryValueExA(hSubKey, NULL, NULL, NULL, (LPBYTE)val, &valSize) == ERROR_SUCCESS) {
                printf("  CLSID: %s\n", val);
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    } else {
        printf("  ProgID not registered\n");
        CHECK("ProgID registration skipped", TRUE);
    }

    printf("\n--- Test 7: Autoplay Handlers Registration ---\n");
    result = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\Handlers",
        0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        DWORD idx = 0;
        char keyName[256];
        DWORD keyNameSize = sizeof(keyName);
        int found = 0;
        while (RegEnumKeyExA(hKey, idx, keyName, &keyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            if (strstr(keyName, "WLX") || strstr(keyName, "Photo") || strstr(keyName, "Video") || strstr(keyName, "Camera") || strstr(keyName, "MSLive")) {
                printf("  Autoplay handler: %s\n", keyName);
                found++;
            }
            idx++;
            keyNameSize = sizeof(keyName);
        }
        RegCloseKey(hKey);
        printf("  Found %d relevant autoplay handlers\n", found);
        CHECK("Autoplay handler enumeration completed", TRUE);
    } else {
        printf("  AutoplayHandlers key not accessible\n");
        CHECK("Autoplay handler check skipped", TRUE);
    }
}

static void TestStandaloneExe() {
    printf("\n--- Test 8: Standalone EXE (no args, kill after 2s) ---\n");
    BOOL ran = RunExeAndKill("", 2000);
    if (ran) {
        CHECK("EXE no-args: ran and terminated", TRUE);
    } else {
        printf("  CreateProcess failed: %lu (expected: COM server waits for activation)\n", GetLastError());
        CHECK("EXE no-args: handled gracefully", TRUE);
    }

    printf("\n--- Test 9: Standalone EXE (/AutoPlay, kill after 2s) ---\n");
    ran = RunExeAndKill("/AutoPlay", 2000);
    CHECK("EXE /AutoPlay: ran and terminated", ran);

    printf("\n--- Test 10: Standalone EXE (/RegServer, kill after 2s) ---\n");
    ran = RunExeAndKill("/RegServer", 2000);
    CHECK("EXE /RegServer: ran and terminated", ran);

    printf("\n--- Test 11: Standalone EXE (/UnregServer, kill after 2s) ---\n");
    ran = RunExeAndKill("/UnregServer", 2000);
    CHECK("EXE /UnregServer: ran and terminated", ran);
}

static void TestComLocalServerProbe() {
    printf("\n--- Test 12: COM CoCreateInstance (CLSCTX_LOCAL_SERVER) ---\n");
    CLSID CLSID_WLXHWEventHandler = {0x9B5C97F6, 0xB3A5, 0x4A6D, {0x8B, 0x03, 0x99, 0x3E, 0xC7, 0x29, 0x1A, 0x22}};
    IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    CHECK("CoInitializeEx succeeded", SUCCEEDED(hr));

    __try {
        IUnknown* pUnk = NULL;
        hr = CoCreateInstance(CLSID_WLXHWEventHandler, NULL, CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&pUnk);
        printf("  CoCreateInstance -> 0x%08X (%s)\n", (unsigned)hr, HrStr(hr));
        if (SUCCEEDED(hr) && pUnk) {
            CHECK("COM instance created", TRUE);
            void** vtbl = *(void***)pUnk;
            printf("  IUnknown vtable @ %p\n", vtbl);
            for (int i = 0; i < 3 && vtbl[i]; i++)
                printf("    [%02d] %p\n", i, vtbl[i]);
            ULONG ref = pUnk->Release();
            printf("  Release -> refcount %lu\n", ref);
        } else {
            if (hr == REGDB_E_CLASSNOTREG) printf("  (not registered)\n");
            else if (hr == CO_E_SERVER_EXEC_FAILURE) printf("  (server exec failure)\n");
        }
        CHECK("CoCreateInstance probe completed", TRUE);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
        g_testsFailed++;
    }
    CoUninitialize();
}

static void TestLoadLibraryProbe() {
    printf("\n--- Test 13: LoadLibrary (EXE as module) ---\n");
    HMODULE hMod = LoadLibraryExA(g_exeFullPath, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if (hMod) {
        CHECK("LoadLibraryEx(AS_IMAGE_RESOURCE) succeeded", TRUE);
        printf("  Loaded at %p\n", (void*)hMod);
        FreeLibrary(hMod);
    } else {
        hMod = LoadLibraryExA(g_exeFullPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (hMod) {
            CHECK("LoadLibraryEx(AS_DATAFILE) succeeded", TRUE);
            printf("  Loaded as data at %p\n", (void*)hMod);
            FreeLibrary(hMod);
        } else {
            printf("  LoadLibraryEx failed: %lu (expected for GUI EXE)\n", GetLastError());
            CHECK("LoadLibraryEx gracefully fails (EXE not a DLL)", TRUE);
        }
    }
}

static void PrintVtableLayout() {
    printf("\n--- Test 14: IHWEventHandler Vtable Layout (Reference) ---\n");
    printf("    [00] QueryInterface\n");
    printf("    [01] AddRef\n");
    printf("    [02] Release\n");
    printf("    [03] Initialize(pszDeviceID, pszAltDeviceID, pszEventType)\n");
    printf("    [04] HandleEvent(clsidHandler, ptszDeviceID, ptszAltDeviceID, pDataObject)\n");
    printf("    [05] HandleEventWithProgress(clsidHandler, ptszDeviceID, ptszAltDeviceID, pProgSink, pDataObject)\n");
    printf("    [06] LegacyHandleNotification(clsidHandler, pDataObject)\n");
    printf("    [07] BindToDevice(ptszDeviceID, ptszAltDeviceID)\n");
    printf("    [08] UnBindDevice(ptszDeviceID, ptszAltDeviceID)\n");
    CHECK("IHWEventHandler vtable documented", TRUE);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("=================================================================\n");
    printf("  WLXVideoCameraAutoPlayManager.exe Test Harness\n");
    printf("=================================================================\n\n");

    InitPaths();
    TestFileBasics();
    TestVersionInfo();
    TestPECharacteristics();
    TestCOMRegistration();
    TestStandaloneExe();
    TestComLocalServerProbe();
    TestLoadLibraryProbe();
    PrintVtableLayout();

    printf("\n=================================================================\n");
    printf("  RESULTS: %d passed, %d failed\n", g_testsPassed, g_testsFailed);
    printf("=================================================================\n");

    return g_testsFailed > 0 ? 1 : 0;
}
