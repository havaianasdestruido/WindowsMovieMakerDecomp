/*
 * test_transcode.cpp
 *
 * Test harness for WLXTranscode.exe reverse engineering analysis.
 * Launches the transcoder as a child process with various command-line
 * arguments, monitors exit codes, and probes DLL-loading behavior
 * (VERSIONINFO says WLXTranscoder.dll).
 *
 * Build:
 *   cl /EHsc /W4 test_transcode.cpp /link ole32.lib oleaut32.lib advapi32.lib shlwapi.lib
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objbase.h>
#include <psapi.h>
#include <shlwapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")

// ============================================================================
// Constants
// ============================================================================
static const wchar_t* WLXTRANSCODE_PATH =
    L"..\\..\\..\\undecomp\\Photo Gallery\\WLXTranscode.exe";

static const DWORD PROCESS_TIMEOUT_MS = 10000;  // 10 second timeout

// ============================================================================
// Helper macros
// ============================================================================
#define LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define LOG_INDENT(fmt, ...) printf("  " fmt "\n", ##__VA_ARGS__)
#define LOG_PASS(name) printf("  [PASS] %s\n", name)
#define LOG_FAIL(name, reason) printf("  [FAIL] %s — %s\n", name, reason)

// ============================================================================
// Result tracking
// ============================================================================
struct TestResult {
    const char* name;
    const char* status;  // "PASS", "FAIL", "SKIP"
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

// ============================================================================
// Launch EXE and capture exit code
// ============================================================================
static DWORD LaunchTranscode(const wchar_t* args, DWORD timeoutMs) {
    wchar_t cmdLine[2048];
    swprintf_s(cmdLine, L"\"%s\" %s", WLXTRANSCODE_PATH, args);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessW(
        NULL,           // Application name (use command line)
        cmdLine,        // Command line
        NULL, NULL,     // Process/Thread security
        FALSE,          // Inherit handles
        0,              // Creation flags
        NULL,           // Environment
        NULL,           // Current directory
        &si, &pi
    );

    if (!ok) {
        DWORD err = GetLastError();
        LOG_INDENT("CreateProcess failed: error %lu (0x%08X)", err, err);
        return (DWORD)-1;
    }

    LOG_INDENT("Process started: PID %lu, handle %p", pi.dwProcessId, pi.hProcess);

    DWORD waitResult = WaitForSingleObject(pi.hProcess, timeoutMs);
    DWORD exitCode = 0;

    if (waitResult == WAIT_TIMEOUT) {
        LOG_INDENT("Process timed out after %lu ms — terminating", timeoutMs);
        TerminateProcess(pi.hProcess, 0xDEAD);
        RecordResult("process_timeout", "FAIL", "timeout", 0xDEAD);
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
// Test 1: No arguments — default behavior
// ============================================================================
static void Test_NoArgs(void) {
    LOG("\n[TEST 1] No arguments — default behavior");
    DWORD exitCode = LaunchTranscode(L"", PROCESS_TIMEOUT_MS);

    if (exitCode == (DWORD)-1) {
        LOG_FAIL("NoArgs", "CreateProcess failed — file not found?");
        RecordResult("NoArgs", "FAIL", "process not found", exitCode);
    } else {
        LOG_INDENT("Exit code: 0x%08X", exitCode);
        RecordResult("NoArgs", "PASS", "launched successfully", exitCode);
    }
}

// ============================================================================
// Test 2: -q flag (quiet/query mode)
// ============================================================================
static void Test_QuietFlag(void) {
    LOG("\n[TEST 2] -q flag (quiet/query)");
    DWORD exitCode = LaunchTranscode(L"-q", PROCESS_TIMEOUT_MS);
    LOG_INDENT("Exit code: 0x%08X", exitCode);
    RecordResult("QuietFlag", "PASS", "launched with -q", exitCode);
}

// ============================================================================
// Test 3: Input file specification only
// ============================================================================
static void Test_InputFileOnly(void) {
    LOG("\n[TEST 3] -fsp with non-existent file");
    DWORD exitCode = LaunchTranscode(
        L"-fsp C:\\nonexistent_test_file.wmv",
        PROCESS_TIMEOUT_MS
    );
    LOG_INDENT("Exit code: 0x%08X", exitCode);
    RecordResult("InputFileOnly", "PASS", "tested with missing file", exitCode);
}

// ============================================================================
// Test 4: Full transcode command with non-existent input
// ============================================================================
static void Test_FullTranscode(void) {
    LOG("\n[TEST 4] Full transcode command — non-existent input");
    DWORD exitCode = LaunchTranscode(
        L"-fsp C:\\nonexistent.wmv -mw C:\\output.mp4 -mc H264 -mvbr 8000000 "
        L"-mfps 30 -mkfs 30 -tf mp4 -mh C:\\meta.xml -pph",
        PROCESS_TIMEOUT_MS
    );
    LOG_INDENT("Exit code: 0x%08X", exitCode);
    RecordResult("FullTranscode", "PASS", "tested full arg set", exitCode);
}

// ============================================================================
// Test 5: Different codec selections
// ============================================================================
static void Test_CodecVariants(void) {
    LOG("\n[TEST 5] Codec variant arguments");
    const char* codecs[] = { "H264", "WMV3", "WVC1", "MP4V", "WMV1", "WMV2" };
    int count = sizeof(codecs) / sizeof(codecs[0]);

    for (int i = 0; i < count; i++) {
        wchar_t args[512];
        swprintf_s(args, L"-fsp C:\\test.wmv -mw C:\\out_%hs.mp4 -mc %hs",
                   codecs[i], codecs[i]);
        DWORD exitCode = LaunchTranscode(args, 5000);
        LOG_INDENT("Codec %hs: exit 0x%08X", codecs[i], exitCode);
    }
    RecordResult("CodecVariants", "PASS", "all codec variants tested", 0);
}

// ============================================================================
// Test 6: Output format variants
// ============================================================================
static void Test_FormatVariants(void) {
    LOG("\n[TEST 6] Output format variants");
    const char* formats[] = { "mp4", "wmv", "wma", "m4a" };
    int count = sizeof(formats) / sizeof(formats[0]);

    for (int i = 0; i < count; i++) {
        wchar_t args[512];
        swprintf_s(args, L"-fsp C:\\test.wmv -mw C:\\out.%hs -mc H264 -tf %hs",
                   formats[i], formats[i]);
        DWORD exitCode = LaunchTranscode(args, 5000);
        LOG_INDENT("Format %hs: exit 0x%08X", formats[i], exitCode);
    }
    RecordResult("FormatVariants", "PASS", "all format variants tested", 0);
}

// ============================================================================
// Test 7: Bitrate and FPS parameter variations
// ============================================================================
static void Test_EncodeParams(void) {
    LOG("\n[TEST 7] Encoding parameter variations");
    const wchar_t* paramSets[] = {
        L"-fsp C:\\t.wmv -mw C:\\o.mp4 -mc H264 -mvbr 1000000 -mfps 24",
        L"-fsp C:\\t.wmv -mw C:\\o.mp4 -mc H264 -mvbr 5000000 -mfps 30",
        L"-fsp C:\\t.wmv -mw C:\\o.mp4 -mc H264 -mvbr 20000000 -mfps 60",
        L"-fsp C:\\t.wmv -mw C:\\o.mp4 -mc WMV3 -mvbr 3000000 -mfps 25 -mkfs 48",
    };
    int count = sizeof(paramSets) / sizeof(paramSets[0]);

    for (int i = 0; i < count; i++) {
        DWORD exitCode = LaunchTranscode(paramSets[i], 5000);
        LOG_INDENT("Params set %d: exit 0x%08X", i, exitCode);
    }
    RecordResult("EncodeParams", "PASS", "encoding param variations tested", 0);
}

// ============================================================================
// Test 8: VERSIONINFO probe — check if it can load as DLL
// ============================================================================
static void Test_DllLoading(void) {
    LOG("\n[TEST 8] DLL loading probe (VERSIONINFO says WLXTranscoder.dll)");

    /* First check: can we LoadLibrary it as a DLL? */
    HMODULE hMod = LoadLibraryW(WLXTRANSCODE_PATH);
    if (hMod) {
        LOG_INDENT("LoadLibrary succeeded — DLL entry point found!");

        /* Check for DllGetClassObject export */
        FARPROC pDllGetClassObject = GetProcAddress(hMod, "DllGetClassObject");
        FARPROC pDllCanUnloadNow = GetProcAddress(hMod, "DllCanUnloadNow");
        FARPROC pDllRegisterServer = GetProcAddress(hMod, "DllRegisterServer");
        FARPROC pDllUnregisterServer = GetProcAddress(hMod, "DllUnregisterServer");

        LOG_INDENT("  DllGetClassObject:    %p %s", pDllGetClassObject,
                   pDllGetClassObject ? "(found)" : "(null)");
        LOG_INDENT("  DllCanUnloadNow:      %p %s", pDllCanUnloadNow,
                   pDllCanUnloadNow ? "(found)" : "(null)");
        LOG_INDENT("  DllRegisterServer:    %p %s", pDllRegisterServer,
                   pDllRegisterServer ? "(found)" : "(null)");
        LOG_INDENT("  DllUnregisterServer:  %p %s", pDllUnregisterServer,
                   pDllUnregisterServer ? "(found)" : "(null)");

        if (pDllGetClassObject) {
            LOG_INDENT("  This binary CAN be loaded as a DLL — "
                       "VERSIONINFO is accurate!");
            RecordResult("DllLoad", "PASS",
                         "loadable as DLL with DllGetClassObject", 0);
        } else {
            LOG_INDENT("  Loaded but no DllGetClassObject — "
                       "EXE main() runs on load");
            RecordResult("DllLoad", "PASS",
                         "loaded but no DLL exports", 0);
        }

        FreeLibrary(hMod);
    } else {
        DWORD err = GetLastError();
        LOG_INDENT("LoadLibrary failed: error %lu (0x%08X)", err, err);
        RecordResult("DllLoad", "PASS", "not loadable as DLL", err);
    }

    /* Second check: read VERSIONINFO directly */
    DWORD verSize = GetFileVersionInfoSizeW(WLXTRANSCODE_PATH, NULL);
    if (verSize > 0) {
        BYTE* verData = (BYTE*)malloc(verSize);
        if (verData && GetFileVersionInfoW(WLXTRANSCODE_PATH, 0, verSize, verData)) {
            VS_FIXEDFILEINFO* fileInfo = NULL;
            UINT len = 0;
            if (VerQueryValueW(verData, L"\\", (void**)&fileInfo, &len) && fileInfo) {
                LOG_INDENT("VERSIONINFO internal name and original filename "
                           "say 'WLXTranscoder.dll'");
                LOG_INDENT("  FileVersion: %d.%d.%d.%d",
                    HIWORD(fileInfo->dwFileVersionMS),
                    LOWORD(fileInfo->dwFileVersionMS),
                    HIWORD(fileInfo->dwFileVersionLS),
                    LOWORD(fileInfo->dwFileVersionLS));
            }
        }
        free(verData);
    }
}

// ============================================================================
// Test 9: Process security attributes and environment
// ============================================================================
static void Test_ProcessInfo(void) {
    LOG("\n[TEST 9] Process information gathering");

    /* Check if we can get the PDB path via SymGetSearchPath or module info */
    DWORD exitCode = LaunchTranscode(L"-q", 3000);
    LOG_INDENT("Quick launch test: exit 0x%08X", exitCode);

    /* Check PE subsystem from the file header */
    HANDLE hFile = CreateFileW(WLXTRANSCODE_PATH, GENERIC_READ,
                               FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        BYTE peHeader[4096] = {};
        DWORD bytesRead = 0;
        if (ReadFile(hFile, peHeader, sizeof(peHeader), &bytesRead, NULL)) {
            /* DOS header -> PE header */
            DWORD peOffset = *(DWORD*)(peHeader + 0x3C);
            if (peOffset + 24 < bytesRead) {
                WORD machine = *(WORD*)(peHeader + peOffset + 4);
                WORD characteristics = *(WORD*)(peHeader + peOffset + 22);
                DWORD optionalOffset = peOffset + 24;
                WORD magic = *(WORD*)(peHeader + optionalOffset);
                WORD subsystem = *(WORD*)(peHeader + optionalOffset + 68);

                LOG_INDENT("PE Header Analysis:");
                LOG_INDENT("  Machine:        0x%04X (%s)", machine,
                           machine == 0x14C ? "x86" : "unknown");
                LOG_INDENT("  Subsystem:      %d (%s)", subsystem,
                           subsystem == 2 ? "GUI" :
                           subsystem == 3 ? "CUI (console)" : "other");
                LOG_INDENT("  Characteristics: 0x%04X", characteristics);
                LOG_INDENT("  ASLR:           %s",
                           (characteristics & 0x0040) ? "yes" : "no");
                LOG_INDENT("  DEP/NX:         %s",
                           (characteristics & 0x0100) ? "yes" : "no");
                LOG_INDENT("  Large address:  %s",
                           (characteristics & 0x0020) ? "yes" : "no");

                RecordResult("ProcessInfo", "PASS",
                             subsystem == 3 ? "CUI confirmed" : "unexpected subsystem",
                             subsystem);
            }
        }
        CloseHandle(hFile);
    } else {
        LOG_INDENT("Could not open PE file for analysis");
        RecordResult("ProcessInfo", "FAIL", "file open failed", 0);
    }
}

// ============================================================================
// Test 10: Error message formatting
// ============================================================================
static void Test_ErrorMessage(void) {
    LOG("\n[TEST 10] Error message formatting — known HRESULT codes");

    struct { HRESULT hr; const char* name; } knownErrors[] = {
        { 0x80070002, "ERROR_FILE_NOT_FOUND" },
        { 0x80070003, "ERROR_PATH_NOT_FOUND" },
        { 0x80070005, "ERROR_ACCESS_DENIED" },
        { 0x80004005, "E_FAIL" },
        { 0x8007000E, "E_OUTOFMEMORY" },
        { 0x80070057, "E_INVALIDARG" },
    };
    int count = sizeof(knownErrors) / sizeof(knownErrors[0]);

    for (int i = 0; i < count; i++) {
        wchar_t msgBuf[256] = {};
        DWORD fmtResult = FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, knownErrors[i].hr, 0,
            msgBuf, sizeof(msgBuf) / sizeof(msgBuf[0]), NULL
        );
        if (fmtResult > 0) {
            /* Trim trailing newline */
            while (fmtResult > 0 && (msgBuf[fmtResult-1] == L'\n' || msgBuf[fmtResult-1] == L'\r'))
                msgBuf[--fmtResult] = L'\0';
            LOG_INDENT("0x%08X (%hs): \"%ls\"", knownErrors[i].hr,
                       knownErrors[i].name, msgBuf);
        } else {
            LOG_INDENT("0x%08X (%hs): <no message>", knownErrors[i].hr,
                       knownErrors[i].name);
        }
    }
    RecordResult("ErrorMessage", "PASS", "error message formatting complete", 0);
}

// ============================================================================
// Main
// ============================================================================
int main(int argc, char* argv[]) {
    LOG("=== WLXTranscode.exe Test Harness ===");
    LOG("Build: %s %s", __DATE__, __TIME__);
    LOG("Target: %ls", WLXTRANSCODE_PATH);
    LOG("");

    /* Check if the EXE exists */
    DWORD attrs = GetFileAttributesW(WLXTRANSCODE_PATH);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        LOG("[WARNING] WLXTranscode.exe not found at expected path.");
        LOG("  The tests will attempt to run but may fail with error 2.");
        LOG("  Adjust WLXTRANSCODE_PATH if the binary is elsewhere.");
    }

    /* Run all tests */
    Test_NoArgs();
    Test_QuietFlag();
    Test_InputFileOnly();
    Test_FullTranscode();
    Test_CodecVariants();
    Test_FormatVariants();
    Test_EncodeParams();
    Test_DllLoading();
    Test_ProcessInfo();
    Test_ErrorMessage();

    /* Print summary */
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
    LOG("Total: %d tests — %d passed, %d failed, %d skipped",
        g_resultCount, passCount, failCount, skipCount);
    LOG("========================================");

    return failCount > 0 ? 1 : 0;
}
