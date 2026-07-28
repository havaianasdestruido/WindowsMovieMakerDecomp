/*
 * test_transcode.cpp
 *
 * Dedicated test harness for WLXTranscode.exe reverse engineering.
 * Launches the transcoder as a child process with all 13 CLI argument
 * combinations, checks exit codes, and probes DLL-loading behavior
 * (VERSIONINFO says WLXTranscoder.dll).
 *
 * Build:
 *   cl /EHsc /W4 test_transcode.cpp /link ole32.lib oleaut32.lib advapi32.lib shlwapi.lib version.lib
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objbase.h>
#include <shlwapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "version.lib")

static const wchar_t* WLXTRANSCODE_PATH =
    L"..\\..\\undecomp\\Photo Gallery\\WLXTranscode.exe";

static const DWORD PROCESS_TIMEOUT_MS = 10000;

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

static DWORD LaunchTranscode(const wchar_t* args, DWORD timeoutMs) {
    wchar_t cmdLine[4096];
    swprintf_s(cmdLine, L"\"%s\" %s", WLXTRANSCODE_PATH, args);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (!ok) {
        DWORD err = GetLastError();
        LOG_INDENT("CreateProcess failed: error %lu (0x%08X)", err, err);
        return (DWORD)-1;
    }

    LOG_INDENT("PID %lu launched", pi.dwProcessId);
    DWORD waitResult = WaitForSingleObject(pi.hProcess, timeoutMs);
    DWORD exitCode = 0;

    if (waitResult == WAIT_TIMEOUT) {
        LOG_INDENT("TIMEOUT (%lu ms) -- terminating", timeoutMs);
        TerminateProcess(pi.hProcess, 0xDEAD);
        GetExitCodeProcess(pi.hProcess, &exitCode);
    } else if (waitResult == WAIT_OBJECT_0) {
        GetExitCodeProcess(pi.hProcess, &exitCode);
        LOG_INDENT("exit code 0x%08X (%lu)", exitCode, exitCode);
    } else {
        LOG_INDENT("Wait failed: %lu", waitResult);
        exitCode = (DWORD)-1;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return exitCode;
}

static BOOL IsKnownErrorHResult(DWORD code) {
    return (code >= 0x80000000 && code <= 0xFFFFFFFF);
}

static const char* ExitCodeDescription(DWORD code) {
    if (code == 0) return "SUCCESS";
    if (code == (DWORD)-1) return "LAUNCH_FAILED";
    if (code == 0xDEAD) return "TIMEOUT";
    if (code == 0x80070002) return "ERROR_FILE_NOT_FOUND";
    if (code == 0x80070003) return "ERROR_PATH_NOT_FOUND";
    if (code == 0x80004005) return "E_FAIL";
    if (code == 0x80070057) return "E_INVALIDARG";
    if (code == 0x8007000E) return "E_OUTOFMEMORY";
    if (code == 0x80040201) return "MF_E_INVALID_FILE_FORMAT";
    if (code == 0xC00D36BD) return "MF_E_TOPO_CODEC_NOT_FOUND";
    if (code == 0xC00D6D60) return "MF_E_UNSUPPORTED_FORMAT";
    if (IsKnownErrorHResult(code)) return "HRESULT_ERROR";
    return "UNKNOWN";
}

// ============================================================================
// Test 1: No arguments
// ============================================================================
static void Test_NoArgs(void) {
    LOG("\n[T1] No arguments");
    DWORD exitCode = LaunchTranscode(L"", PROCESS_TIMEOUT_MS);
    LOG_INDENT("-> %s (0x%08X)", ExitCodeDescription(exitCode), exitCode);
    RecordResult("NoArgs", exitCode != (DWORD)-1 ? "PASS" : "FAIL",
                 ExitCodeDescription(exitCode), exitCode);
}

// ============================================================================
// Test 2: -q (quiet/query mode)
// ============================================================================
static void Test_Quiet(void) {
    LOG("\n[T2] -q (quiet/query)");
    DWORD exitCode = LaunchTranscode(L"-q", PROCESS_TIMEOUT_MS);
    LOG_INDENT("-> %s (0x%08X)", ExitCodeDescription(exitCode), exitCode);
    RecordResult("Quiet", "PASS", ExitCodeDescription(exitCode), exitCode);
}

// ============================================================================
// Test 3: -fsp only (input file)
// ============================================================================
static void Test_InputFile(void) {
    LOG("\n[T3] -fsp (input file)");
    DWORD exitCode = LaunchTranscode(L"-fsp C:\\nope.wmv", PROCESS_TIMEOUT_MS);
    LOG_INDENT("-> %s (0x%08X)", ExitCodeDescription(exitCode), exitCode);
    RecordResult("InputFile", "PASS", ExitCodeDescription(exitCode), exitCode);
}

// ============================================================================
// Test 4: -mw only (output file)
// ============================================================================
static void Test_OutputFile(void) {
    LOG("\n[T4] -mw (output file) only");
    DWORD exitCode = LaunchTranscode(L"-mw C:\\out.mp4", PROCESS_TIMEOUT_MS);
    LOG_INDENT("-> %s (0x%08X)", ExitCodeDescription(exitCode), exitCode);
    RecordResult("OutputFile", "PASS", ExitCodeDescription(exitCode), exitCode);
}

// ============================================================================
// Test 5: -mh (metadata header)
// ============================================================================
static void Test_MetaHeader(void) {
    LOG("\n[T5] -mh (metadata header)");
    DWORD exitCode = LaunchTranscode(L"-mh C:\\meta.xml", PROCESS_TIMEOUT_MS);
    LOG_INDENT("-> %s (0x%08X)", ExitCodeDescription(exitCode), exitCode);
    RecordResult("MetaHeader", "PASS", ExitCodeDescription(exitCode), exitCode);
}

// ============================================================================
// Test 6: -mc codec selection
// ============================================================================
static void Test_CodecSelect(void) {
    LOG("\n[T6] -mc codec selection");
    const char* codecs[] = {"H264","WMV3","WVC1","MP4V","WMV1","WMV2","MP43","MP4S"};
    for (int i = 0; i < 8; i++) {
        wchar_t args[256];
        swprintf_s(args, L"-fsp C:\\t.wmv -mw C:\\o.mp4 -mc %hs", codecs[i]);
        DWORD ec = LaunchTranscode(args, 5000);
        LOG_INDENT("  -mc %hs -> 0x%08X (%s)", codecs[i], ec, ExitCodeDescription(ec));
    }
    RecordResult("CodecSelect", "PASS", "8 codecs tested", 0);
}

// ============================================================================
// Test 7: -tf format selection
// ============================================================================
static void Test_FormatSelect(void) {
    LOG("\n[T7] -tf format selection");
    const char* fmts[] = {"mp4","wmv","wma","m4a","wtv","asf"};
    for (int i = 0; i < 6; i++) {
        wchar_t args[256];
        swprintf_s(args, L"-fsp C:\\t.wmv -mw C:\\o.%hs -mc H264 -tf %hs", fmts[i], fmts[i]);
        DWORD ec = LaunchTranscode(args, 5000);
        LOG_INDENT("  -tf %hs -> 0x%08X (%s)", fmts[i], ec, ExitCodeDescription(ec));
    }
    RecordResult("FormatSelect", "PASS", "6 formats tested", 0);
}

// ============================================================================
// Test 8: Full transcode profile
// ============================================================================
static void Test_FullProfile(void) {
    LOG("\n[T8] Full transcode profile (all 13 args)");
    /*
     * Complete set of 13 CLI arguments:
     * -mw  -mh  -mfps  -mvbr  -q  -mkfs  -fsp  -rd  -vsm  -mc  -tf  -pph  -ms
     */
    DWORD exitCode = LaunchTranscode(
        L"-fsp C:\\input.wmv -mw C:\\output.mp4 -mc H264 -mvbr 8000000 "
        L"-mfps 30 -mkfs 30 -tf mp4 -mh C:\\meta.xml -vsm 0 "
        L"-rd -pph -ms C:\\input.wmv -q",
        PROCESS_TIMEOUT_MS);
    LOG_INDENT("-> %s (0x%08X)", ExitCodeDescription(exitCode), exitCode);
    RecordResult("FullProfile", "PASS", ExitCodeDescription(exitCode), exitCode);
}

// ============================================================================
// Test 9: Bitrate / FPS / keyframe variants
// ============================================================================
static void Test_EncodeParams(void) {
    LOG("\n[T9] Encoding parameter variants");
    const wchar_t* sets[] = {
        L"-fsp C:\\t.wmv -mw C:\\o1.mp4 -mc H264 -mvbr 1000000 -mfps 24 -mkfs 24",
        L"-fsp C:\\t.wmv -mw C:\\o2.mp4 -mc H264 -mvbr 5000000 -mfps 30 -mkfs 30",
        L"-fsp C:\\t.wmv -mw C:\\o3.mp4 -mc H264 -mvbr 20000000 -mfps 60 -mkfs 60",
        L"-fsp C:\\t.wmv -mw C:\\o4.mp4 -mc WMV3 -mvbr 3000000 -mfps 25 -mkfs 48",
    };
    for (int i = 0; i < 4; i++) {
        DWORD ec = LaunchTranscode(sets[i], 5000);
        LOG_INDENT("  set %d: 0x%08X (%s)", i, ec, ExitCodeDescription(ec));
    }
    RecordResult("EncodeParams", "PASS", "4 param variants", 0);
}

// ============================================================================
// Test 10: -rd raw data flag
// ============================================================================
static void Test_RawData(void) {
    LOG("\n[T10] -rd (raw data flag)");
    DWORD exitCode = LaunchTranscode(
        L"-fsp C:\\t.wmv -mw C:\\o.mp4 -mc H264 -rd",
        PROCESS_TIMEOUT_MS);
    LOG_INDENT("-> %s (0x%08X)", ExitCodeDescription(exitCode), exitCode);
    RecordResult("RawData", "PASS", ExitCodeDescription(exitCode), exitCode);
}

// ============================================================================
// Test 11: -vsm video stream map
// ============================================================================
static void Test_VideoStreamMap(void) {
    LOG("\n[T11] -vsm (video stream map) variants");
    const wchar_t* vsmArgs[] = {
        L"-fsp C:\\t.wmv -mw C:\\o.mp4 -mc H264 -vsm 0",
        L"-fsp C:\\t.wmv -mw C:\\o.mp4 -mc H264 -vsm 1",
        L"-fsp C:\\t.wmv -mw C:\\o.mp4 -mc H264 -vsm -1",
    };
    for (int i = 0; i < 3; i++) {
        DWORD ec = LaunchTranscode(vsmArgs[i], 5000);
        LOG_INDENT("  -vsm variant %d: 0x%08X (%s)", i, ec, ExitCodeDescription(ec));
    }
    RecordResult("VideoStreamMap", "PASS", "3 vsm variants", 0);
}

// ============================================================================
// Test 12: -pph pre-process header flag
// ============================================================================
static void Test_PreProcessHeader(void) {
    LOG("\n[T12] -pph (pre-process header)");
    DWORD exitCode = LaunchTranscode(
        L"-fsp C:\\t.wmv -mw C:\\o.mp4 -mc H264 -pph",
        PROCESS_TIMEOUT_MS);
    LOG_INDENT("-> %s (0x%08X)", ExitCodeDescription(exitCode), exitCode);
    RecordResult("PreProcessHeader", "PASS", ExitCodeDescription(exitCode), exitCode);
}

// ============================================================================
// Test 13: -ms media source
// ============================================================================
static void Test_MediaSource(void) {
    LOG("\n[T13] -ms (media source)");
    DWORD exitCode = LaunchTranscode(
        L"-ms C:\\source.wmv -mw C:\\o.mp4 -mc H264",
        PROCESS_TIMEOUT_MS);
    LOG_INDENT("-> %s (0x%08X)", ExitCodeDescription(exitCode), exitCode);
    RecordResult("MediaSource", "PASS", ExitCodeDescription(exitCode), exitCode);
}

// ============================================================================
// Test 14: LoadLibrary / LoadLibraryEx as DLL
// ============================================================================
static void Test_DllLoading(void) {
    LOG("\n[T14] DLL loading probe (VERSIONINFO says WLXTranscoder.dll)");

    HMODULE hMod = LoadLibraryExW(WLXTRANSCODE_PATH, NULL,
                                   LOAD_LIBRARY_AS_DATAFILE);
    if (hMod) {
        LOG_INDENT("LoadLibraryEx(AS_DATAFILE) succeeded");
        FreeLibrary(hMod);
    } else {
        LOG_INDENT("LoadLibraryEx(AS_DATAFILE) failed: %lu", GetLastError());
    }

    hMod = LoadLibraryExW(WLXTRANSCODE_PATH, NULL,
                           LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if (hMod) {
        LOG_INDENT("LoadLibraryEx(AS_IMAGE_RESOURCE) succeeded");
        FreeLibrary(hMod);
    } else {
        LOG_INDENT("LoadLibraryEx(AS_IMAGE_RESOURCE) failed: %lu", GetLastError());
    }

    hMod = LoadLibraryExW(WLXTRANSCODE_PATH, NULL, 0);
    if (hMod) {
        LOG_INDENT("LoadLibraryEx(normal) succeeded -- DLL entry point runs!");

        FARPROC pDllGetClassObject = GetProcAddress(hMod, "DllGetClassObject");
        FARPROC pDllCanUnloadNow   = GetProcAddress(hMod, "DllCanUnloadNow");
        FARPROC pDllRegisterServer = GetProcAddress(hMod, "DllRegisterServer");

        LOG_INDENT("  DllGetClassObject:    %p", pDllGetClassObject);
        LOG_INDENT("  DllCanUnloadNow:      %p", pDllCanUnloadNow);
        LOG_INDENT("  DllRegisterServer:    %p", pDllRegisterServer);

        if (pDllGetClassObject)
            LOG_INDENT("  => Binary IS loadable as a COM DLL (DllGetClassObject found)");
        else
            LOG_INDENT("  => Loaded but no COM exports (EXE main() ran on attach)");

        RecordResult("DllLoading", "PASS",
                     pDllGetClassObject ? "COM DLL found" : "loaded, no COM exports", 0);
        FreeLibrary(hMod);
    } else {
        DWORD err = GetLastError();
        LOG_INDENT("LoadLibraryEx(normal) failed: %lu (0x%08X)", err, err);
        RecordResult("DllLoading", "FAIL", "not loadable as DLL", err);
    }

    DWORD verSize = GetFileVersionInfoSizeW(WLXTRANSCODE_PATH, NULL);
    if (verSize > 0) {
        BYTE* verData = (BYTE*)malloc(verSize);
        if (verData && GetFileVersionInfoW(WLXTRANSCODE_PATH, 0, verSize, verData)) {
            VS_FIXEDFILEINFO* fileInfo = NULL;
            UINT len = 0;
            if (VerQueryValueW(verData, L"\\", (void**)&fileInfo, &len) && fileInfo) {
                LOG_INDENT("FileVersion: %d.%d.%d.%d",
                    HIWORD(fileInfo->dwFileVersionMS),
                    LOWORD(fileInfo->dwFileVersionMS),
                    HIWORD(fileInfo->dwFileVersionLS),
                    LOWORD(fileInfo->dwFileVersionLS));
            }

            wchar_t* origName = NULL;
            UINT origLen = 0;
            if (VerQueryValueW(verData,
                    L"\\StringFileInfo\\040904b0\\OriginalFilename",
                    (void**)&origName, &origLen) && origName) {
                LOG_INDENT("OriginalFilename: %ls", origName);
            }
            wchar_t* intName = NULL;
            UINT intLen = 0;
            if (VerQueryValueW(verData,
                    L"\\StringFileInfo\\040904b0\\InternalName",
                    (void**)&intName, &intLen) && intName) {
                LOG_INDENT("InternalName: %ls", intName);
            }
        }
        free(verData);
    }
}

// ============================================================================
// Test 15: PE subsystem analysis
// ============================================================================
static void Test_PEAnalysis(void) {
    LOG("\n[T15] PE analysis");

    HANDLE hFile = CreateFileW(WLXTRANSCODE_PATH, GENERIC_READ,
                                FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_INDENT("Cannot open EXE");
        RecordResult("PEAnalysis", "FAIL", "cannot open file", 0);
        return;
    }

    BYTE buf[4096];
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buf, sizeof(buf), &bytesRead, NULL)) {
        LOG_INDENT("Read failed");
        CloseHandle(hFile);
        RecordResult("PEAnalysis", "FAIL", "read failed", 0);
        return;
    }
    CloseHandle(hFile);

    DWORD peOffset = *(DWORD*)(buf + 0x3C);
    if (peOffset + 24 >= bytesRead) {
        LOG_INDENT("Invalid PE header");
        RecordResult("PEAnalysis", "FAIL", "invalid PE", 0);
        return;
    }

    WORD machine = *(WORD*)(buf + peOffset + 4);
    WORD characteristics = *(WORD*)(buf + peOffset + 22);
    DWORD optOffset = peOffset + 24;
    WORD magic = *(WORD*)(buf + optOffset);
    WORD subsystem = *(WORD*)(buf + optOffset + 68);
    DWORD dllChar = *(DWORD*)(buf + optOffset + 70); // DLL characteristics

    LOG_INDENT("Machine:      0x%04X (%s)", machine,
               machine == 0x14C ? "x86" : "other");
    LOG_INDENT("Magic:        0x%04X (%s)", magic,
               magic == 0x10B ? "PE32" : magic == 0x20B ? "PE32+" : "unknown");
    LOG_INDENT("Subsystem:    %u (%s)", subsystem,
               subsystem == 2 ? "GUI" :
               subsystem == 3 ? "CUI" : "other");
    LOG_INDENT("DLL char:     0x%08X", dllChar);
    LOG_INDENT("  NX Compat:  %s", (dllChar & 0x0100) ? "yes" : "no");
    LOG_INDENT("  ASLR:       %s", (dllChar & 0x0040) ? "yes" : "no");
    LOG_INDENT("  AppCont:    %s", (dllChar & 0x1000) ? "yes" : "no");
    LOG_INDENT("  GDW:        %s", (dllChar & 0x2000) ? "yes" : "no");

    RecordResult("PEAnalysis", "PASS",
                 subsystem == 3 ? "CUI confirmed" : "subsys=" + subsystem, subsystem);
}

// ============================================================================
// Test 16: Stdin/stdout/stderr redirect probe
// ============================================================================
static void Test_StdioCapture(void) {
    LOG("\n[T16] Stdio capture with -q");

    wchar_t cmdLine[4096];
    swprintf_s(cmdLine, L"\"%s\" -q", WLXTRANSCODE_PATH);

    HANDLE hStdoutRd, hStdoutWr;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    CreatePipe(&hStdoutRd, &hStdoutWr, &sa, 0);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hStdoutWr;
    si.hStdError = hStdoutWr;
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessW(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    if (!ok) {
        LOG_INDENT("CreateProcess failed: %lu", GetLastError());
        CloseHandle(hStdoutRd);
        CloseHandle(hStdoutWr);
        RecordResult("StdioCapture", "FAIL", "createprocess failed", (DWORD)-1);
        return;
    }

    CloseHandle(hStdoutWr); // close write end in parent

    DWORD waitResult = WaitForSingleObject(pi.hProcess, PROCESS_TIMEOUT_MS);
    DWORD exitCode = 0;
    if (waitResult == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 0xDEAD);
        GetExitCodeProcess(pi.hProcess, &exitCode);
        LOG_INDENT("TIMEOUT exit=0x%08X", exitCode);
    } else {
        GetExitCodeProcess(pi.hProcess, &exitCode);
    }

    char outBuf[4096] = {};
    DWORD bytesRead = 0;
    if (ReadFile(hStdoutRd, outBuf, sizeof(outBuf) - 1, &bytesRead, NULL) && bytesRead > 0) {
        outBuf[bytesRead] = 0;
        LOG_INDENT("Captured %lu bytes of stdout:", bytesRead);
        LOG_INDENT("---[stdout]---");
        printf("%s", outBuf);
        if (outBuf[strlen(outBuf) - 1] != '\n') printf("\n");
        LOG_INDENT("---[end]---");
    } else {
        LOG_INDENT("No stdout captured (err=%lu)", GetLastError());
    }

    CloseHandle(hStdoutRd);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    RecordResult("StdioCapture", "PASS",
                 bytesRead > 0 ? "stdout captured" : "no output", exitCode);
}

// ============================================================================
// Test 17: Invalid arg fuzzing
// ============================================================================
static void Test_ArgFuzzing(void) {
    LOG("\n[T17] Argument fuzzing");
    const wchar_t* fuzzArgs[] = {
        L"-unknown",
        L"---",
        L"-fsp",
        L"-fsp \"\"",
        L"-fsp C:\\nope.wmv -mc BADCODEC",
        L"-fsp C:\\nope.wmv -tf unknown",
        L"-fsp C:\\nope.wmv -mvbr notanumber",
        L"-fsp C:\\nope.wmv -mfps notanumber",
        L"-mw",
        L"-mh",
        L"-rd -rd -rd",
        L"\"C:\\Program Files\\some path with spaces.wmv\"",
        L"",
        L"-fsp C:\\nope.wmv -mw C:\\out.mp4 -q -q -q",
    };
    for (int i = 0; i < 14; i++) {
        DWORD ec = LaunchTranscode(fuzzArgs[i], 3000);
        LOG_INDENT("  fuzz[%d]: \"%ls\" -> 0x%08X (%s)",
                   i, fuzzArgs[i], ec, ExitCodeDescription(ec));
    }
    RecordResult("ArgFuzzing", "PASS", "14 fuzz cases", 0);
}

// ============================================================================
// Test 18: Current working directory side-effect
// ============================================================================
static void Test_CwdLaunch(void) {
    LOG("\n[T18] Launch from undecomp dir");
    wchar_t cmdLine[4096];
    swprintf_s(cmdLine, L"\"%s\" -q", WLXTRANSCODE_PATH);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL,
                              L"..\\..\\undecomp\\Photo Gallery", &si, &pi);
    if (!ok) {
        LOG_INDENT("CreateProcess failed: %lu", GetLastError());
        RecordResult("CwdLaunch", "FAIL", "createprocess failed", (DWORD)-1);
        return;
    }

    DWORD waitResult = WaitForSingleObject(pi.hProcess, PROCESS_TIMEOUT_MS);
    DWORD exitCode = 0;
    if (waitResult == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 0xDEAD);
        GetExitCodeProcess(pi.hProcess, &exitCode);
    } else {
        GetExitCodeProcess(pi.hProcess, &exitCode);
    }
    LOG_INDENT("exit code 0x%08X (%s)", exitCode, ExitCodeDescription(exitCode));
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    RecordResult("CwdLaunch", "PASS", ExitCodeDescription(exitCode), exitCode);
}

// ============================================================================
// Main
// ============================================================================
int main(void) {
    LOG("=== WLXTranscode.exe Test Harness ===");
    LOG("Build: %s %s", __DATE__, __TIME__);
    LOG("Target: %ls", WLXTRANSCODE_PATH);
    LOG("");

    DWORD attrs = GetFileAttributesW(WLXTRANSCODE_PATH);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        LOG("[WARNING] WLXTranscode.exe not found at %ls", WLXTRANSCODE_PATH);
    } else {
        LOG("Found: %ls", WLXTRANSCODE_PATH);
    }

    Test_NoArgs();
    Test_Quiet();
    Test_InputFile();
    Test_OutputFile();
    Test_MetaHeader();
    Test_CodecSelect();
    Test_FormatSelect();
    Test_FullProfile();
    Test_EncodeParams();
    Test_RawData();
    Test_VideoStreamMap();
    Test_PreProcessHeader();
    Test_MediaSource();
    Test_DllLoading();
    Test_PEAnalysis();
    Test_StdioCapture();
    Test_ArgFuzzing();
    Test_CwdLaunch();

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
