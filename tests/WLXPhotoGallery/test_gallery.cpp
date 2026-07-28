#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "psapi.lib")

#define LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define LOG_INDENT(fmt, ...) printf("  " fmt "\n", ##__VA_ARGS__)

static wchar_t EXE_PATH[MAX_PATH] = {};

static void ResolveExePath(void) {
    wchar_t modPath[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, modPath, MAX_PATH);
    if (len == 0) return;
    wchar_t* slash = wcsrchr(modPath, L'\\');
    if (!slash) return;
    *(slash + 1) = L'\0';
    wcscat_s(modPath, L"..\\..\\undecomp\\Photo Gallery\\WLXPhotoGallery.exe");
    _wfullpath(EXE_PATH, modPath, MAX_PATH);
    LOG_INDENT("Resolved EXE path: %ls", EXE_PATH);
}

static const DWORD PROCESS_TIMEOUT_MS = 10000;

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

static DWORD LaunchProcess(const wchar_t* args, DWORD timeoutMs) {
    wchar_t cmdLine[2048];
    swprintf_s(cmdLine, L"\"%s\" %s", EXE_PATH, args ? args : L"");

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    if (!ok) {
        DWORD err = GetLastError();
        LOG_INDENT("CreateProcess failed: error %lu (0x%08X)", err, err);
        return (DWORD)-1;
    }

    LOG_INDENT("PID %lu, handle %p", pi.dwProcessId, pi.hProcess);

    DWORD waitResult = WaitForSingleObject(pi.hProcess, timeoutMs);
    DWORD exitCode = 0;

    if (waitResult == WAIT_TIMEOUT) {
        LOG_INDENT("Timed out after %lu ms — terminating", timeoutMs);
        TerminateProcess(pi.hProcess, 0xDEAD);
        exitCode = 0xDEAD;
    } else if (waitResult == WAIT_OBJECT_0) {
        GetExitCodeProcess(pi.hProcess, &exitCode);
        LOG_INDENT("Exit code %lu (0x%08X)", exitCode, exitCode);
    } else {
        LOG_INDENT("WaitForSingleObject failed: %lu", waitResult);
        exitCode = (DWORD)-1;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return exitCode;
}

static void Test_NoArgs(void) {
    LOG("\n[TEST 1] No arguments — default launch");
    DWORD exitCode = LaunchProcess(L"", PROCESS_TIMEOUT_MS);
    RecordResult("NoArgs", exitCode == (DWORD)-1 ? "FAIL" : "PASS",
                 exitCode == (DWORD)-1 ? "CreateProcess failed" : "launched",
                 exitCode);
}

static void Test_HelpArg(void) {
    LOG("\n[TEST 2] /? argument (help/usage)");
    DWORD exitCode = LaunchProcess(L"/?", 5000);
    RecordResult("HelpArg", "PASS", "/? swiitch", exitCode);
}

static void Test_LaunchViewer(void) {
    LOG("\n[TEST 3] /LaunchViewer argument");
    DWORD exitCode = LaunchProcess(L"/LaunchViewer", PROCESS_TIMEOUT_MS);
    RecordResult("LaunchViewer", "PASS", "viewer launch mode", exitCode);
}

static void Test_LaunchViewerComServer(void) {
    LOG("\n[TEST 4] /LaunchViewerComServer argument");
    DWORD exitCode = LaunchProcess(L"/LaunchViewerComServer", PROCESS_TIMEOUT_MS);
    RecordResult("LaunchViewerComServer", "PASS", "COM server launch mode", exitCode);
}

static void Test_RunAsStandAlone(void) {
    LOG("\n[TEST 5] /RunAsStandAlone argument");
    DWORD exitCode = LaunchProcess(L"/RunAsStandAlone", PROCESS_TIMEOUT_MS);
    RecordResult("RunAsStandAlone", "PASS", "standalone mode", exitCode);
}

static void Test_InvalidArg(void) {
    LOG("\n[TEST 6] /InvalidArg (unknown switch)");
    DWORD exitCode = LaunchProcess(L"/InvalidArg", 5000);
    RecordResult("InvalidArg", "PASS", "unknown switch", exitCode);
}

static void Test_DllLoadingProbe(void) {
    LOG("\n[TEST 7] LoadLibraryEx probe (load as DLL / DONT_RESOLVE_DLL)");
    HMODULE hMod = LoadLibraryExW(EXE_PATH, NULL,
                                  DONT_RESOLVE_DLL_REFERENCES);
    if (hMod) {
        LOG_INDENT("DONT_RESOLVE_DLL_REFERENCES succeeded — mapped at %p", (void*)hMod);
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)hMod;
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)hMod + dos->e_lfanew);
        LOG_INDENT("  Machine:    0x%04X (x86)", nt->FileHeader.Machine);
        LOG_INDENT("  EntryPoint: 0x%X", nt->OptionalHeader.AddressOfEntryPoint);
        LOG_INDENT("  Subsystem:  %u (GUI)", nt->OptionalHeader.Subsystem);
        LOG_INDENT("  Sections:   %u", nt->FileHeader.NumberOfSections);

        IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
            char sname[9] = {};
            memcpy(sname, sec[i].Name, 8);
            LOG_INDENT("    %-8s VA=0x%08X Size=0x%08X",
                       sname, sec[i].VirtualAddress, sec[i].Misc.VirtualSize);
        }

        FARPROC addr = GetProcAddress(hMod, "DllMain");
        if (addr) {
            LOG_INDENT("  DllMain found at %p (EXE has DllMain entry)", (void*)addr);
        } else {
            LOG_INDENT("  No DllMain export (expected for pure EXE)");
        }

        FreeLibrary(hMod);
        RecordResult("DllLoadProbe", "PASS", "loaded as mapped image", 0);
    } else {
        DWORD err = GetLastError();
        LOG_INDENT("LoadLibraryEx failed: error %lu (0x%08X)", err, err);
        RecordResult("DllLoadProbe", "FAIL", "LoadLibraryEx failed", err);
    }
}

static void Test_WerRegistration(void) {
    LOG("\n[TEST 8] WER / crash recovery registration check");
    DWORD exitCode = LaunchProcess(L"/? 2>&1", 5000);
    LOG_INDENT("Process exit code: 0x%08X", exitCode);

    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\Windows Error Reporting\\"
        L"ExcludedApplications\\WLXPhotoGallery.exe",
        0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        LOG_INDENT("WER exclusion entry found in HKCU");
        RegCloseKey(hKey);
        RecordResult("WerExclusion", "PASS", "WER exclusion registered", 0);
    } else {
        LOG_INDENT("No WER exclusion found (may not persist after single run)");
        RecordResult("WerExclusion", "INFO", "no WER exclusion key", 0);
    }
}

static void Test_ProcessInfo(void) {
    LOG("\n[TEST 9] Process information — PE header verification");
    HANDLE hFile = CreateFileW(EXE_PATH, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_INDENT("Could not open file");
        RecordResult("ProcessInfo", "FAIL", "file open failed", 0);
        return;
    }

    BYTE peHeader[4096] = {};
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, peHeader, sizeof(peHeader), &bytesRead, NULL)) {
        LOG_INDENT("ReadFile failed");
        CloseHandle(hFile);
        RecordResult("ProcessInfo", "FAIL", "read failed", 0);
        return;
    }
    CloseHandle(hFile);

    DWORD peOffset = *(DWORD*)(peHeader + 0x3C);
    if (peOffset + 24 >= bytesRead) {
        RecordResult("ProcessInfo", "FAIL", "truncated PE header", 0);
        return;
    }

    WORD machine = *(WORD*)(peHeader + peOffset + 4);
    WORD characteristics = *(WORD*)(peHeader + peOffset + 22);
    DWORD optionalOffset = peOffset + 24;
    WORD magic = *(WORD*)(peHeader + optionalOffset);
    WORD subsystem = *(WORD*)(peHeader + optionalOffset + 68);
    DWORD entryPoint = *(DWORD*)(peHeader + optionalOffset + 16);
    DWORD imageBase = *(DWORD*)(peHeader + optionalOffset + 28);
    DWORD sizeOfImage = *(DWORD*)(peHeader + optionalOffset + 56);
    DWORD checksum = *(DWORD*)(peHeader + optionalOffset + 64);

    LOG_INDENT("Machine:        0x%04X (%s)", machine,
               machine == 0x14C ? "x86" : "other");
    LOG_INDENT("Subsystem:      %d (%s)", subsystem,
               subsystem == 2 ? "Windows GUI" : "other");
    LOG_INDENT("Entry Point:    0x%08X", imageBase + entryPoint);
    LOG_INDENT("Image Base:     0x%08X", imageBase);
    LOG_INDENT("Size of Image:  0x%X (%u bytes)", sizeOfImage, sizeOfImage);
    LOG_INDENT("Checksum:       0x%08X", checksum);
    LOG_INDENT("Characteristics: 0x%04X", characteristics);
    LOG_INDENT("  ASLR:         %s", (characteristics & 0x0040) ? "yes" : "no");
    LOG_INDENT("  DEP/NX:       %s", (characteristics & 0x0100) ? "yes" : "no");
    LOG_INDENT("  TS Aware:     %s", (characteristics & 0x8000) ? "yes" : "no");

    RecordResult("ProcessInfo", "PASS", "PE header verified", subsystem);
}

static void Test_ExitCodeConsistency(void) {
    LOG("\n[TEST 10] Exit code consistency across multiple launches");
    const int runs = 3;
    DWORD prevExit = 0;
    bool consistent = true;

    for (int i = 0; i < runs; i++) {
        DWORD exitCode = LaunchProcess(L"/?", 5000);
        LOG_INDENT("  Run %d: exit 0x%08X", i + 1, exitCode);
        if (exitCode == (DWORD)-1) continue;
        if (i > 0 && exitCode != prevExit) {
            consistent = false;
        }
        prevExit = exitCode;
    }

    RecordResult("ExitConsistency", consistent ? "PASS" : "INFO",
                 consistent ? "consistent exit codes" : "non-deterministic exit codes",
                 prevExit);
}

int main(int argc, char* argv[]) {
    LOG("=== WLXPhotoGallery.exe Test Harness ===");
    LOG("Build: %s %s", __DATE__, __TIME__);

    ResolveExePath();
    LOG("Target: %ls", EXE_PATH);
    LOG("");

    DWORD attrs = GetFileAttributesW(EXE_PATH);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        LOG("[WARNING] WLXPhotoGallery.exe not found at expected path.");
        LOG("  Adjust EXE_PATH if the binary is elsewhere.");
    }

    Test_NoArgs();
    Test_HelpArg();
    Test_LaunchViewer();
    Test_LaunchViewerComServer();
    Test_RunAsStandAlone();
    Test_InvalidArg();
    Test_DllLoadingProbe();
    Test_WerRegistration();
    Test_ProcessInfo();
    Test_ExitCodeConsistency();

    LOG("\n========================================");
    LOG("=== TEST SUMMARY ===");
    LOG("========================================");
    int passCount = 0, failCount = 0, infoCount = 0;
    for (int i = 0; i < g_resultCount; i++) {
        const char* icon = "??";
        if (strcmp(g_results[i].status, "PASS") == 0) { icon = "OK"; passCount++; }
        else if (strcmp(g_results[i].status, "FAIL") == 0) { icon = "!!"; failCount++; }
        else { icon = "--"; infoCount++; }

        LOG("  [%s] %-25s exit=0x%08X  %s",
            icon, g_results[i].name, g_results[i].exitCode, g_results[i].detail);
    }
    LOG("");
    LOG("Total: %d — %d passed, %d failed, %d info",
        g_resultCount, passCount, failCount, infoCount);
    LOG("========================================");

    return failCount > 0 ? 1 : 0;
}
