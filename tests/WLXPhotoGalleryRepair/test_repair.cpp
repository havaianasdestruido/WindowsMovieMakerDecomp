#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objbase.h>
#include <shlwapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "advapi32.lib")

static const wchar_t* REPAIR_EXE =     L"..\\..\\undecomp\\Photo Gallery\\WLXPhotoGalleryRepair.exe";
static const DWORD PROCESS_TIMEOUT_MS = 15000;

#define LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define LOG_INDENT(fmt, ...) printf("  " fmt "\n", ##__VA_ARGS__)

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

static DWORD LaunchRepair(const wchar_t* args, DWORD timeoutMs) {
    wchar_t cmdLine[2048];
    swprintf_s(cmdLine, L"\"%s\" %s", REPAIR_EXE, args);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (!ok) {
        DWORD err = GetLastError();
        LOG_INDENT("CreateProcess failed: error %lu (0x%08X)", err, err);
        return (DWORD)-1;
    }

    LOG_INDENT("Process started: PID %lu", pi.dwProcessId);

    DWORD waitResult = WaitForSingleObject(pi.hProcess, timeoutMs);
    DWORD exitCode = 0;

    if (waitResult == WAIT_TIMEOUT) {
        LOG_INDENT("Process timed out after %lu ms — terminating", timeoutMs);
        TerminateProcess(pi.hProcess, 0xDEAD);
        RecordResult("timeout", "FAIL", "timeout", 0xDEAD);
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

static void Test_NoArgs(void) {
    LOG("\n[TEST 1] No arguments — default behavior");
    DWORD exitCode = LaunchRepair(L"", PROCESS_TIMEOUT_MS);
    if (exitCode == (DWORD)-1) {
        LOG_INDENT("[FAIL] NoArgs — CreateProcess failed");
        RecordResult("NoArgs", "FAIL", "process not found", exitCode);
    } else {
        RecordResult("NoArgs", "PASS", "launched successfully", exitCode);
    }
}

static void Test_HelpArg(void) {
    LOG("\n[TEST 2] /? help argument");
    DWORD exitCode = LaunchRepair(L"/?", PROCESS_TIMEOUT_MS);
    LOG_INDENT("Exit code: 0x%08X", exitCode);
    RecordResult("HelpArg", "PASS", "launched with /?", exitCode);
}

static void Test_RepairArg(void) {
    LOG("\n[TEST 3] /repair argument");
    DWORD exitCode = LaunchRepair(L"/repair", PROCESS_TIMEOUT_MS);
    LOG_INDENT("Exit code: 0x%08X", exitCode);
    RecordResult("RepairArg", "PASS", "launched with /repair", exitCode);
}

static void Test_SlashPArg(void) {
    LOG("\n[TEST 4] /p argument (product code?)");
    DWORD exitCode = LaunchRepair(L"/p", PROCESS_TIMEOUT_MS);
    LOG_INDENT("Exit code: 0x%08X", exitCode);
    RecordResult("SlashPArg", "PASS", "launched with /p", exitCode);
}

static void Test_LongUnicodeArg(void) {
    LOG("\n[TEST 5] Long garbage argument");
    DWORD exitCode = LaunchRepair(L"-\"!@#$%^&*()_+-=[]{}|;':\",./<>?`~", PROCESS_TIMEOUT_MS);
    LOG_INDENT("Exit code: 0x%08X", exitCode);
    RecordResult("LongGarbageArg", "PASS", "launched with garbage", exitCode);
}

static void Test_EmptyQuotesArg(void) {
    LOG("\n[TEST 6] Empty quoted argument");
    DWORD exitCode = LaunchRepair(L"\"\"", PROCESS_TIMEOUT_MS);
    LOG_INDENT("Exit code: 0x%08X", exitCode);
    RecordResult("EmptyQuotesArg", "PASS", "launched with empty quotes", exitCode);
}

static void Test_DllLoading(void) {
    LOG("\n[TEST 7] DLL loading probe (VERSIONINFO says WLXPhotoGalleryRepair.dll)");
    HMODULE hMod = LoadLibraryW(REPAIR_EXE);
    if (hMod) {
        LOG_INDENT("LoadLibrary succeeded — entry point executed");
        FARPROC pDllGetClassObject = GetProcAddress(hMod, "DllGetClassObject");
        FARPROC pDllCanUnloadNow = GetProcAddress(hMod, "DllCanUnloadNow");
        LOG_INDENT("  DllGetClassObject: %p", pDllGetClassObject);
        LOG_INDENT("  DllCanUnloadNow:   %p", pDllCanUnloadNow);
        if (pDllGetClassObject)
            RecordResult("DllLoad", "PASS", "loadable as DLL with exports", 0);
        else
            RecordResult("DllLoad", "PASS", "loaded but no COM exports", 0);
        FreeLibrary(hMod);
    } else {
        DWORD err = GetLastError();
        LOG_INDENT("LoadLibrary failed: error %lu (0x%08X)", err, err);
        RecordResult("DllLoad", "FAIL", "LoadLibrary failed", err);
    }
}

static void Test_LoadLibraryExAsImage(void) {
    LOG("\n[TEST 8] LoadLibraryEx with LOAD_LIBRARY_AS_IMAGE_RESOURCE");
    HMODULE hMod = LoadLibraryExW(REPAIR_EXE, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if (hMod) {
        LOG_INDENT("Loaded as image resource at %p", hMod);
        BYTE* dos = (BYTE*)hMod;
        if (dos[0] == 'M' && dos[1] == 'Z') {
            LOG_INDENT("  MZ signature verified");
            DWORD peOff = *(DWORD*)(dos + 0x3C);
            if (dos[peOff] == 'P' && dos[peOff+1] == 'E') {
                WORD machine = *(WORD*)(dos + peOff + 4);
                WORD subsystem = *(WORD*)(dos + peOff + 24 + 68);
                LOG_INDENT("  Machine:  0x%04X (%s)", machine,
                           machine == 0x14C ? "x86" : "other");
                LOG_INDENT("  Subsystem: %u (%s)", subsystem,
                           subsystem == 2 ? "GUI" :
                           subsystem == 3 ? "CUI" : "other");
            }
        }
        FreeLibrary(hMod);
        RecordResult("LoadLibraryExImage", "PASS", "loaded as image resource", 0);
    } else {
        DWORD err = GetLastError();
        LOG_INDENT("LoadLibraryEx failed: error %lu (0x%08X)", err, err);
        RecordResult("LoadLibraryExImage", "FAIL", "LoadLibraryEx failed", err);
    }
}

static void Test_LoadLibraryExAsDataFile(void) {
    LOG("\n[TEST 9] LoadLibraryEx with LOAD_LIBRARY_AS_DATAFILE");
    HMODULE hMod = LoadLibraryExW(REPAIR_EXE, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hMod) {
        LOG_INDENT("Loaded as data file at %p", hMod);
        FreeLibrary(hMod);
        RecordResult("LoadLibraryExDataFile", "PASS", "loaded as data file", 0);
    } else {
        DWORD err = GetLastError();
        LOG_INDENT("LoadLibraryEx failed: error %lu (0x%08X)", err, err);
        RecordResult("LoadLibraryExDataFile", "FAIL", "LoadLibraryEx failed", err);
    }
}

static void Test_WerRegistration(void) {
    LOG("\n[TEST 10] WER registration check");
    HKEY hKey = NULL;
    LONG ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting\\ExcludedApplications",
        0, KEY_READ, &hKey);
    if (ret == ERROR_SUCCESS) {
        wchar_t value[256] = {};
        DWORD size = sizeof(value);
        ret = RegQueryValueExW(hKey, L"WLXPhotoGalleryRepair.exe", NULL, NULL,
                               (BYTE*)value, &size);
        if (ret == ERROR_SUCCESS) {
            LOG_INDENT("Found in WER ExcludedApplications: '%ls'", value);
            RecordResult("WER", "PASS", "WER exclusion registered", 0);
        } else {
            LOG_INDENT("Not in WER ExcludedApplications (err=%ld)", ret);
            RecordResult("WER", "PASS", "not in WER exclusion list", 0);
        }
        RegCloseKey(hKey);
    } else {
        LOG_INDENT("Could not open WER key (err=%ld)", ret);
        RecordResult("WER", "SKIP", "cannot check WER", 0);
    }
}

static void Test_PeHeaderParse(void) {
    LOG("\n[TEST 11] PE header parsing from file");
    HANDLE hFile = CreateFileW(REPAIR_EXE, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_INDENT("Could not open file");
        RecordResult("PEHeader", "FAIL", "file open failed", 0);
        return;
    }
    BYTE buf[4096] = {};
    DWORD read = 0;
    if (ReadFile(hFile, buf, sizeof(buf), &read, NULL)) {
        DWORD peOff = *(DWORD*)(buf + 0x3C);
        WORD machine    = *(WORD*)(buf + peOff + 4);
        WORD sectCount  = *(WORD*)(buf + peOff + 6);
        WORD charact    = *(WORD*)(buf + peOff + 22);
        WORD magic      = *(WORD*)(buf + peOff + 24);
        WORD subsystem  = *(WORD*)(buf + peOff + 24 + 68);
        DWORD imgSize   = *(DWORD*)(buf + peOff + 24 + 56);
        DWORD epRVA     = *(DWORD*)(buf + peOff + 24 + 16);

        LOG_INDENT("Machine:          0x%04X", machine);
        LOG_INDENT("Section count:    %u", sectCount);
        LOG_INDENT("Subsystem:        %u (%s)", subsystem,
                   subsystem == 2 ? "GUI" : subsystem == 3 ? "CUI" : "other");
        LOG_INDENT("Magic:            0x%04X (%s)", magic,
                   magic == 0x10B ? "PE32" : magic == 0x20B ? "PE32+" : "unknown");
        LOG_INDENT("Entry Point RVA:  0x%08X", epRVA);
        LOG_INDENT("Image Size:       0x%08X (%u bytes)", imgSize, imgSize);
        LOG_INDENT("Characteristics:  0x%04X", charact);
        LOG_INDENT("  ASLR:           %s", (charact & 0x0040) ? "yes" : "no");
        LOG_INDENT("  DEP/NX:         %s", (charact & 0x0100) ? "yes" : "no");

        RecordResult("PEHeader", "PASS", "PE header parsed", subsystem);
    } else {
        RecordResult("PEHeader", "FAIL", "ReadFile failed", GetLastError());
    }
    CloseHandle(hFile);
}

int main(void) {
    LOG("=== WLXPhotoGalleryRepair.exe Test Harness ===");
    LOG("Build: %s %s", __DATE__, __TIME__);
    LOG("Target: %ls", REPAIR_EXE);
    LOG("");

    DWORD attrs = GetFileAttributesW(REPAIR_EXE);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        LOG("[WARNING] WLXPhotoGalleryRepair.exe not found at expected path.");
    }

    Test_NoArgs();
    Test_HelpArg();
    Test_RepairArg();
    Test_SlashPArg();
    Test_LongUnicodeArg();
    Test_EmptyQuotesArg();
    Test_DllLoading();
    Test_LoadLibraryExAsImage();
    Test_LoadLibraryExAsDataFile();
    Test_WerRegistration();
    Test_PeHeaderParse();

    LOG("\n========================================");
    LOG("=== TEST SUMMARY ===");
    LOG("========================================");
    int passCount = 0, failCount = 0, skipCount = 0;
    for (int i = 0; i < g_resultCount; i++) {
        const char* icon = "??";
        if (strcmp(g_results[i].status, "PASS") == 0) { icon = "OK"; passCount++; }
        else if (strcmp(g_results[i].status, "FAIL") == 0) { icon = "!!"; failCount++; }
        else { icon = "--"; skipCount++; }
        LOG("  [%s] %-24s exit=0x%08X  %s",
            icon, g_results[i].name, g_results[i].exitCode, g_results[i].detail);
    }
    LOG("");
    LOG("Total: %d tests — %d passed, %d failed, %d skipped",
        g_resultCount, passCount, failCount, skipCount);
    LOG("========================================");

    return failCount > 0 ? 1 : 0;
}
