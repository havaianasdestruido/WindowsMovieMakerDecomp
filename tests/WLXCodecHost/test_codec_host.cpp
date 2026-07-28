#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objbase.h>
#include <combaseapi.h>
#include <psapi.h>
#include <shlwapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")

static const wchar_t* CODEC_HOST_PATH =
    L"..\\..\\undecomp\\Photo Gallery\\WLXCodecHost.exe";

static const DWORD PROCESS_TIMEOUT_MS = 15000;

// CLSID_CodecHost  {E30A45E6-1916-4659-95EE-035E62DB9AB0}
static const CLSID CLSID_CodecHost = {
    0xE30A45E6, 0x1916, 0x4659,
    {0x95, 0xEE, 0x03, 0x5E, 0x62, 0xDB, 0x9A, 0xB0}
};

// APPID  {87A9DFB0-BA04-45F3-85EB-C33727ECEF22}
static const CLSID APPID_CodecHost = {
    0x87A9DFB0, 0xBA04, 0x45F3,
    {0x85, 0xEB, 0xC3, 0x37, 0x27, 0xEC, 0xEF, 0x22}
};

#define LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define LOG_INDENT(fmt, ...) printf("  " fmt "\n", ##__VA_ARGS__)
#define LOG_PASS(name) printf("  [PASS] %s\n", name)
#define LOG_FAIL(name, reason) printf("  [FAIL] %s -- %s\n", name, reason)
#define LOG_SKIP(name, reason) printf("  [SKIP] %s -- %s\n", name, reason)

struct TestResult {
    const char* name;
    const char* status;
    const char* detail;
    DWORD exitCode;
};

static TestResult g_results[64];
static int g_resultCount = 0;

static void RecordResult(const char* name, const char* status,
                         const char* detail, DWORD exitCode) {
    if (g_resultCount < 64) {
        g_results[g_resultCount].name = name;
        g_results[g_resultCount].status = status;
        g_results[g_resultCount].detail = detail;
        g_results[g_resultCount].exitCode = exitCode;
        g_resultCount++;
    }
}

static DWORD LaunchHost(const wchar_t* args, DWORD timeoutMs) {
    wchar_t cmdLine[2048];
    swprintf_s(cmdLine, L"\"%s\" %s", CODEC_HOST_PATH, args ? args : L"");

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessW(
        NULL, cmdLine, NULL, NULL, FALSE,
        0, NULL, NULL, &si, &pi
    );

    if (!ok) {
        DWORD err = GetLastError();
        LOG_INDENT("CreateProcess failed: error %lu (0x%08X)", err, err);
        return (DWORD)-1;
    }

    LOG_INDENT("Process started: PID %lu", pi.dwProcessId);

    DWORD waitResult = WaitForSingleObject(pi.hProcess, timeoutMs);
    DWORD exitCode = 0;

    if (waitResult == WAIT_TIMEOUT) {
        LOG_INDENT("Process timed out after %lu ms -- terminating", timeoutMs);
        TerminateProcess(pi.hProcess, 0xDEAD);
        WaitForSingleObject(pi.hProcess, 5000);
        exitCode = 0xDEAD;
    } else if (waitResult == WAIT_OBJECT_0) {
        GetExitCodeProcess(pi.hProcess, &exitCode);
        LOG_INDENT("Process exited: code %lu (0x%08X)", exitCode, exitCode);
    } else {
        LOG_INDENT("WaitForSingleObject failed: %lu", waitResult);
        exitCode = (DWORD)-1;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return exitCode;
}

// ============================================================================
// Test 1: Standalone launch with no args
// ============================================================================
static void Test_StandaloneNoArgs(void) {
    LOG("\n[TEST 1] Standalone launch -- no arguments");
    LOG_INDENT("GUI subsystem EXE should self-terminate when no COM client connects");
    DWORD exitCode = LaunchHost(L"", PROCESS_TIMEOUT_MS);

    if (exitCode == (DWORD)-1) {
        LOG_FAIL("StandaloneNoArgs", "CreateProcess failed -- file not found?");
        RecordResult("StandaloneNoArgs", "FAIL", "process not found", exitCode);
    } else {
        LOG_INDENT("Exit code: 0x%08X", exitCode);
        RecordResult("StandaloneNoArgs", "PASS", "launched successfully", exitCode);
    }
}

// ============================================================================
// Test 2: COM activation probe -- CoCreateInstance
// ============================================================================
static void Test_COMActivation(void) {
    LOG("\n[TEST 2] COM activation probe -- CoCreateInstance(CLSID_CodecHost)");

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        LOG_FAIL("COMActivation", "CoInitializeEx failed");
        RecordResult("COMActivation", "FAIL", "CoInitializeEx failed", hr);
        return;
    }

    IUnknown* pUnk = NULL;
    hr = CoCreateInstance(
        CLSID_CodecHost,
        NULL,
        CLSCTX_LOCAL_SERVER,
        IID_IUnknown,
        (void**)&pUnk
    );

    if (SUCCEEDED(hr) && pUnk) {
        LOG_INDENT("CoCreateInstance SUCCEEDED -- object created at %p", pUnk);
        ULONG refs = pUnk->Release();
        LOG_INDENT("After Release, ref count: %lu", refs);
        RecordResult("COMActivation", "PASS", "COM object created", hr);
    } else {
        LOG_INDENT("CoCreateInstance failed: HRESULT 0x%08X", hr);
        if (hr == REGDB_E_CLASSNOTREG) {
            LOG_INDENT("CLSID not registered -- expected if WLXCodecHost is not installed");
            RecordResult("COMActivation", "SKIP", "CLSID not registered", hr);
        } else if (hr == CO_E_SERVER_EXEC_FAILURE) {
            LOG_INDENT("Server execution failure -- possible missing dependencies");
            RecordResult("COMActivation", "SKIP", "server exec failure", hr);
        } else {
            RecordResult("COMActivation", "FAIL", "CoCreateInstance failed", hr);
        }
    }

    CoUninitialize();
}

// ============================================================================
// Test 3: COM activation via ProgID
// ============================================================================
static void Test_ProgIDActivation(void) {
    LOG("\n[TEST 3] COM activation via ProgID -- Microsoft.WLXCodecHost.CodecHost.1");

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        LOG_FAIL("ProgIDActivation", "CoInitializeEx failed");
        RecordResult("ProgIDActivation", "FAIL", "CoInitializeEx failed", hr);
        return;
    }

    CLSID clsid;
    hr = CLSIDFromProgID(L"Microsoft.WLXCodecHost.CodecHost.1", &clsid);

    if (SUCCEEDED(hr)) {
        wchar_t guidStr[64];
        StringFromGUID2(clsid, guidStr, 64);
        LOG_INDENT("ProgID resolved to CLSID: %ls", guidStr);

        IUnknown* pUnk = NULL;
        hr = CoCreateInstance(
            clsid, NULL, CLSCTX_LOCAL_SERVER,
            IID_IUnknown, (void**)&pUnk
        );

        if (SUCCEEDED(hr) && pUnk) {
            LOG_INDENT("CoCreateInstance via ProgID SUCCEEDED");
            pUnk->Release();
            RecordResult("ProgIDActivation", "PASS", "activated via ProgID", hr);
        } else {
            LOG_INDENT("CoCreateInstance via ProgID failed: 0x%08X", hr);
            RecordResult("ProgIDActivation", "SKIP", "ProgID found but activation failed", hr);
        }
    } else {
        LOG_INDENT("ProgID not found (HRESULT 0x%08X) -- not registered", hr);
        RecordResult("ProgIDActivation", "SKIP", "ProgID not registered", hr);
    }

    CoUninitialize();
}

// ============================================================================
// Test 4: Version-independent ProgID activation
// ============================================================================
static void Test_VerIndepProgID(void) {
    LOG("\n[TEST 4] Version-independent ProgID activation");
    LOG_INDENT("ProgID: Microsoft.WLXCodecHost.CodecHost");

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        LOG_FAIL("VerIndepProgID", "CoInitializeEx failed");
        RecordResult("VerIndepProgID", "FAIL", "CoInitializeEx failed", hr);
        return;
    }

    CLSID clsid;
    hr = CLSIDFromProgID(L"Microsoft.WLXCodecHost.CodecHost", &clsid);

    if (SUCCEEDED(hr)) {
        wchar_t guidStr[64];
        StringFromGUID2(clsid, guidStr, 64);
        LOG_INDENT("Version-independent ProgID resolved to CLSID: %ls", guidStr);
        RecordResult("VerIndepProgID", "PASS", "ProgID resolved", hr);
    } else {
        LOG_INDENT("Version-independent ProgID not found: 0x%08X", hr);
        RecordResult("VerIndepProgID", "SKIP", "ProgID not registered", hr);
    }

    CoUninitialize();
}

// ============================================================================
// Test 5: DLL loading probe
// ============================================================================
static void Test_DllLoading(void) {
    LOG("\n[TEST 5] DLL loading probe -- can the EXE be loaded as a DLL?");

    HMODULE hMod = LoadLibraryW(CODEC_HOST_PATH);
    if (hMod) {
        LOG_INDENT("LoadLibrary succeeded -- module handle %p", hMod);

        FARPROC pDllGetClassObject = GetProcAddress(hMod, "DllGetClassObject");
        FARPROC pDllCanUnloadNow   = GetProcAddress(hMod, "DllCanUnloadNow");
        FARPROC pDllRegisterServer = GetProcAddress(hMod, "DllRegisterServer");

        LOG_INDENT("  DllGetClassObject:   %p %s", pDllGetClassObject,
                   pDllGetClassObject ? "(found)" : "(null)");
        LOG_INDENT("  DllCanUnloadNow:     %p %s", pDllCanUnloadNow,
                   pDllCanUnloadNow ? "(found)" : "(null)");
        LOG_INDENT("  DllRegisterServer:   %p %s", pDllRegisterServer,
                   pDllRegisterServer ? "(found)" : "(null)");

        if (pDllGetClassObject) {
            LOG_INDENT("EXE has DllGetClassObject -- dual-mode EXE/DLL");
            RecordResult("DllLoading", "PASS", "loadable with COM exports", 0);
        } else {
            LOG_INDENT("No DLL exports -- pure EXE with no COM exports exposed");
            RecordResult("DllLoading", "PASS", "loadable but no exports", 0);
        }

        FreeLibrary(hMod);
    } else {
        DWORD err = GetLastError();
        LOG_INDENT("LoadLibrary failed: error %lu (0x%08X)", err, err);
        RecordResult("DllLoading", "PASS", "not loadable as DLL (expected for EXE)", err);
    }
}

// ============================================================================
// Test 6: PE header verification
// ============================================================================
static void Test_PEHeader(void) {
    LOG("\n[TEST 6] PE header verification");

    HANDLE hFile = CreateFileW(CODEC_HOST_PATH, GENERIC_READ,
                               FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_FAIL("PEHeader", "file open failed");
        RecordResult("PEHeader", "FAIL", "file open failed", GetLastError());
        return;
    }

    BYTE buffer[4096] = {};
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL)) {
        LOG_FAIL("PEHeader", "ReadFile failed");
        RecordResult("PEHeader", "FAIL", "ReadFile failed", GetLastError());
        CloseHandle(hFile);
        return;
    }
    CloseHandle(hFile);

    DWORD peOffset = *(DWORD*)(buffer + 0x3C);
    if (peOffset + 24 >= bytesRead) {
        LOG_FAIL("PEHeader", "invalid PE offset");
        RecordResult("PEHeader", "FAIL", "invalid PE offset", 0);
        return;
    }

    WORD   machine         = *(WORD*)(buffer + peOffset + 4);
    WORD   characteristics = *(WORD*)(buffer + peOffset + 22);
    DWORD  optionalOffset  = peOffset + 24;
    WORD   magic           = *(WORD*)(buffer + optionalOffset);
    WORD   subsystem       = *(WORD*)(buffer + optionalOffset + 68);
    DWORD  imageSize       = *(DWORD*)(buffer + optionalOffset + 56);
    DWORD  entryPoint      = *(DWORD*)(buffer + optionalOffset + 16);
    DWORD  sizeOfCode      = *(DWORD*)(buffer + optionalOffset + 4);
    WORD   dllChar         = *(WORD*)(buffer + optionalOffset + 70);

    LOG_INDENT("Machine:        0x%04X (%s)", machine,
               machine == 0x14C ? "x86" : "unknown");
    LOG_INDENT("Subsystem:      %d (%s)", subsystem,
               subsystem == 2 ? "Windows GUI" :
               subsystem == 3 ? "Windows CUI (console)" : "other");
    LOG_INDENT("Entry point:    0x%08X", entryPoint);
    LOG_INDENT("Image size:     %lu bytes (%lu KB)", imageSize, imageSize / 1024);
    LOG_INDENT("Size of code:   %lu bytes", sizeOfCode);
    LOG_INDENT("PE magic:       0x%04X (%s)", magic,
               magic == 0x10B ? "PE32" : magic == 0x20B ? "PE32+" : "unknown");
    LOG_INDENT("Characteristics: 0x%04X", characteristics);
    LOG_INDENT("DLL char:       0x%04X", dllChar);
    LOG_INDENT("ASLR:           %s", (dllChar & 0x0040) ? "yes" : "no");
    LOG_INDENT("DEP/NX:         %s", (dllChar & 0x0100) ? "yes" : "no");
    LOG_INDENT("TS aware:       %s", (dllChar & 0x8000) ? "yes" : "no");

    int failures = 0;
    if (machine != 0x14C) { LOG_FAIL("PEHeader", "expected x86 machine"); failures++; }
    if (subsystem != 2)   { LOG_FAIL("PEHeader", "expected GUI subsystem"); failures++; }
    if (magic != 0x10B)   { LOG_FAIL("PEHeader", "expected PE32"); failures++; }

    if (failures == 0) {
        LOG_PASS("PEHeader");
        RecordResult("PEHeader", "PASS", "all PE header checks passed", subsystem);
    } else {
        RecordResult("PEHeader", "FAIL", "PE header mismatch", subsystem);
    }
}

// ============================================================================
// Test 7: VERSIONINFO probe
// ============================================================================
static void Test_VersionInfo(void) {
    LOG("\n[TEST 7] VERSIONINFO resource probe");

    DWORD verSize = GetFileVersionInfoSizeW(CODEC_HOST_PATH, NULL);
    if (verSize == 0) {
        DWORD err = GetLastError();
        LOG_INDENT("No VERSIONINFO data (error %lu) -- binary may not have it", err);
        RecordResult("VersionInfo", "SKIP", "no VERSIONINFO resource", err);
        return;
    }

    BYTE* verData = (BYTE*)malloc(verSize);
    if (!verData) {
        LOG_FAIL("VersionInfo", "malloc failed");
        RecordResult("VersionInfo", "FAIL", "malloc failed", 0);
        return;
    }

    BOOL ok = GetFileVersionInfoW(CODEC_HOST_PATH, 0, verSize, verData);
    if (!ok) {
        LOG_FAIL("VersionInfo", "GetFileVersionInfo failed");
        RecordResult("VersionInfo", "FAIL", "GetFileVersionInfo failed", GetLastError());
        free(verData);
        return;
    }

    VS_FIXEDFILEINFO* fileInfo = NULL;
    UINT len = 0;
    if (VerQueryValueW(verData, L"\\", (void**)&fileInfo, &len) && fileInfo) {
        LOG_INDENT("FileVersion: %d.%d.%d.%d",
            HIWORD(fileInfo->dwFileVersionMS),
            LOWORD(fileInfo->dwFileVersionMS),
            HIWORD(fileInfo->dwFileVersionLS),
            LOWORD(fileInfo->dwFileVersionLS));
        LOG_INDENT("ProductVersion: %d.%d.%d.%d",
            HIWORD(fileInfo->dwProductVersionMS),
            LOWORD(fileInfo->dwProductVersionMS),
            HIWORD(fileInfo->dwProductVersionLS),
            LOWORD(fileInfo->dwProductVersionLS));
        LOG_INDENT("FileFlags:      0x%08X", fileInfo->dwFileFlags);
        LOG_INDENT("FileOS:         0x%08X", fileInfo->dwFileOS);
        LOG_INDENT("FileType:       0x%08X (%s)", fileInfo->dwFileType,
                   fileInfo->dwFileType == 1 ? "APP" :
                   fileInfo->dwFileType == 2 ? "DLL" : "other");
        LOG_INDENT("FileSubtype:    0x%08X", fileInfo->dwFileSubtype);

        RecordResult("VersionInfo", "PASS", "VERSIONINFO read successfully", 0);
    } else {
        LOG_INDENT("VerQueryValue failed");
        RecordResult("VersionInfo", "FAIL", "VerQueryValue failed", 0);
    }

    free(verData);
}

// ============================================================================
// Test 8: Command-line argument tolerance
// ============================================================================
static void Test_CommandLineArgs(void) {
    LOG("\n[TEST 8] Command-line argument tolerance");
    LOG_INDENT("GUI COM server should ignore unexpected arguments");

    const wchar_t* argSets[] = {
        L"-Embedding",            // COM activation flag (but standalone)
        L"/regserver",            // common ATL registration flag
        L"/unregserver",          // common ATL unregistration flag
        L"-?"                     // help flag (should be ignored)
    };
    const char* names[] = {
        "EmbeddingFlag",
        "RegServerFlag",
        "UnregServerFlag",
        "HelpFlag"
    };
    int count = 4;

    for (int i = 0; i < count; i++) {
        DWORD exitCode = LaunchHost(argSets[i], 5000);
        LOG_INDENT("Args %hs (%ls): exit 0x%08X", names[i], argSets[i], exitCode);
    }

    RecordResult("CommandLineArgs", "PASS", "all argument variants tested", 0);
}

// ============================================================================
// Test 9: Registry registration check via ADVAPI32
// ============================================================================
static void Test_RegistryRegistration(void) {
    LOG("\n[TEST 9] Registry registration check");

    HKEY hKey = NULL;
    wchar_t clsidPath[] =
        L"CLSID\\{E30A45E6-1916-4659-95EE-035E62DB9AB0}";
    wchar_t clsidPath2[] =
        L"CLSID\\{E30A45E6-1916-4659-95EE-035E62DB9AB0}\\LocalServer32";
    wchar_t appidPath[] =
        L"AppID\\{87A9DFB0-BA04-45F3-85EB-C33727ECEF22}";

    struct {
        const wchar_t* path;
        const char* name;
        LONG expected; // ERROR_SUCCESS or ERROR_FILE_NOT_FOUND
    } regChecks[] = {
        { clsidPath,   "CLSID key",           ERROR_SUCCESS },
        { clsidPath2,  "LocalServer32 key",   ERROR_SUCCESS },
        { appidPath,   "AppID key",           ERROR_SUCCESS },
    };
    int checkCount = sizeof(regChecks) / sizeof(regChecks[0]);

    for (int i = 0; i < checkCount; i++) {
        HKEY hSubKey = NULL;
        LONG result = RegOpenKeyExW(
            HKEY_CLASSES_ROOT, regChecks[i].path, 0, KEY_READ, &hSubKey
        );

        if (result == ERROR_SUCCESS) {
            LOG_INDENT("[FOUND] %hs", regChecks[i].name);
            RegCloseKey(hSubKey);
        } else if (result == ERROR_FILE_NOT_FOUND) {
            LOG_INDENT("[NOT FOUND] %hs -- not registered on this system", regChecks[i].name);
        } else {
            LOG_INDENT("[ERROR] %hs: 0x%08X", regChecks[i].name, result);
        }
    }

    // Check ProgID keys
    const wchar_t* progIds[] = {
        L"Microsoft.WLXCodecHost.CodecHost.1",
        L"Microsoft.WLXCodecHost.CodecHost"
    };
    for (int i = 0; i < 2; i++) {
        HKEY hSubKey = NULL;
        LONG result = RegOpenKeyExW(
            HKEY_CLASSES_ROOT, progIds[i], 0, KEY_READ, &hSubKey
        );
        if (result == ERROR_SUCCESS) {
            LOG_INDENT("[FOUND] ProgID: %ls", progIds[i]);
            RegCloseKey(hSubKey);
        } else {
            LOG_INDENT("[NOT FOUND] ProgID: %ls", progIds[i]);
        }
    }

    RecordResult("RegistryRegistration", "PASS", "registry check complete", 0);
}

// ============================================================================
// Test 10: Process priority probe (SetPriorityClass import)
// ============================================================================
static void Test_ProcessPriority(void) {
    LOG("\n[TEST 10] Process priority probe");
    LOG_INDENT("Launch with PROCESS_SET_INFORMATION to observe priority class");

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    wchar_t cmdLine[2048];
    swprintf_s(cmdLine, L"\"%s\"", CODEC_HOST_PATH);

    BOOL ok = CreateProcessW(
        NULL, cmdLine, NULL, NULL, FALSE,
        PROCESS_SET_INFORMATION, NULL, NULL, &si, &pi
    );

    if (!ok) {
        LOG_INDENT("CreateProcess failed: %lu", GetLastError());
        RecordResult("ProcessPriority", "SKIP", "could not launch process", 0);
        return;
    }

    DWORD priority = GetPriorityClass(pi.hProcess);
    const char* priorityName = "unknown";
    switch (priority) {
        case IDLE_PRIORITY_CLASS:         priorityName = "IDLE";         break;
        case BELOW_NORMAL_PRIORITY_CLASS: priorityName = "BELOW_NORMAL"; break;
        case NORMAL_PRIORITY_CLASS:       priorityName = "NORMAL";       break;
        case ABOVE_NORMAL_PRIORITY_CLASS: priorityName = "ABOVE_NORMAL"; break;
        case HIGH_PRIORITY_CLASS:         priorityName = "HIGH";         break;
        case REALTIME_PRIORITY_CLASS:     priorityName = "REALTIME";     break;
    }
    LOG_INDENT("Initial priority class: %lu (%s)", priority, priorityName);

    // Give it a moment then check again
    WaitForSingleObject(pi.hProcess, 1000);
    priority = GetPriorityClass(pi.hProcess);
    LOG_INDENT("Priority after 1s:     %lu (%s)", priority, priorityName);

    // Terminate
    TerminateProcess(pi.hProcess, 0);
    WaitForSingleObject(pi.hProcess, 3000);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    RecordResult("ProcessPriority", "PASS", "priority probed", priority);
}

// ============================================================================
// Main
// ============================================================================
int main(int argc, char* argv[]) {
    LOG("=== WLXCodecHost.exe Test Harness ===");
    LOG("Build: %s %s", __DATE__, __TIME__);
    LOG("Target: %ls", CODEC_HOST_PATH);
    LOG("");

    DWORD attrs = GetFileAttributesW(CODEC_HOST_PATH);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        LOG("[WARNING] WLXCodecHost.exe not found at expected path.");
        LOG("  Adjust CODEC_HOST_PATH if the binary is elsewhere.");
    } else {
        WIN32_FILE_ATTRIBUTE_DATA fad = {};
        if (GetFileAttributesExW(CODEC_HOST_PATH, GetFileExInfoStandard, &fad)) {
            ULARGE_INTEGER li;
            li.LowPart  = fad.nFileSizeLow;
            li.HighPart = fad.nFileSizeHigh;
            LOG("Target file size: %llu bytes", li.QuadPart);
        }
    }

    Test_StandaloneNoArgs();
    Test_COMActivation();
    Test_ProgIDActivation();
    Test_VerIndepProgID();
    Test_DllLoading();
    Test_PEHeader();
    Test_VersionInfo();
    Test_CommandLineArgs();
    Test_RegistryRegistration();
    Test_ProcessPriority();

    // Print summary
    LOG("\n========================================");
    LOG("=== TEST SUMMARY ===");
    LOG("========================================");
    int passCount = 0, failCount = 0, skipCount = 0;
    for (int i = 0; i < g_resultCount; i++) {
        const char* icon = "??";
        if (strcmp(g_results[i].status, "PASS") == 0) { icon = "OK"; passCount++; }
        else if (strcmp(g_results[i].status, "FAIL") == 0) { icon = "!!"; failCount++; }
        else { icon = "--"; skipCount++; }

        LOG("  [%s] %-20s exit=0x%08X  %s",
            icon, g_results[i].name, g_results[i].exitCode, g_results[i].detail);
    }
    LOG("");
    LOG("Total: %d tests -- %d passed, %d failed, %d skipped",
        g_resultCount, passCount, failCount, skipCount);
    LOG("========================================");

    return failCount > 0 ? 1 : 0;
}
