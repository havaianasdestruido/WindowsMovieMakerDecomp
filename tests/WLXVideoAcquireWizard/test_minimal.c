#include <windows.h>
#include <stdio.h>

int main(void) {
    printf("Starting...\n");
    char cmdline[1024];
    _snprintf(cmdline, sizeof(cmdline), "\"%s\"", "C:\\Users\\mcmco\\Desktop\\WMMR\\undecomp\\Photo Gallery\\WLXVideoAcquireWizard.exe");
    printf("Cmdline: %s\n", cmdline);
    fflush(stdout);
    
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL ok = CreateProcessA(NULL, cmdline, NULL, NULL, FALSE,
                             CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    printf("CreateProcess: %d (err=%lu)\n", ok, GetLastError());
    if (ok) {
        printf("hProcess=%p, hThread=%p, pid=%lu\n", pi.hProcess, pi.hThread, pi.dwProcessId);
        DWORD ec = STILL_ACTIVE;
        GetExitCodeProcess(pi.hProcess, &ec);
        printf("ExitCode: %lu (STILL_ACTIVE=%lu)\n", ec, STILL_ACTIVE);
        TerminateProcess(pi.hProcess, 1);
        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    printf("Done.\n");
    return 0;
}
