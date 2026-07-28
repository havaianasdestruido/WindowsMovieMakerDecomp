#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

typedef HRESULT (__stdcall *FnDllCanUnloadNow)();
typedef void (__stdcall *FnRunAsStandAlone)(HINSTANCE, HINSTANCE, LPSTR, int);
typedef HRESULT (__stdcall *FnDisplayRepairPromptDialog)(HWND);

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    SetDllDirectoryA("undecomp\\Photo Gallery");

    printf("=== WLXPhotoLibraryMain.dll Test Harness ===\n\n");

    HMODULE hMod = LoadLibraryA("WLXPhotoLibraryMain.dll");
    if (!hMod) {
        printf("FAILED: LoadLibrary error %lu\n", GetLastError());
        return 1;
    }
    printf("DLL loaded at %p\n\n", (void*)hMod);

    FARPROC pDllCanUnloadNow        = GetProcAddress(hMod, "DllCanUnloadNow");
    FARPROC pRunAsStandAlone        = GetProcAddress(hMod, "RunAsStandAlone");
    FARPROC pDisplayRepairPromptDialog = GetProcAddress(hMod, "DisplayRepairPromptDialog");

    printf("=== Export Resolution ===\n");
    printf("  DllCanUnloadNow             %s\n", pDllCanUnloadNow ? "OK" : "MISSING");
    printf("  RunAsStandAlone             %s\n", pRunAsStandAlone ? "OK" : "MISSING");
    printf("  DisplayRepairPromptDialog   %s\n\n", pDisplayRepairPromptDialog ? "OK" : "MISSING");

    // 1. DllCanUnloadNow
    printf("--- DllCanUnloadNow ---\n");
    if (pDllCanUnloadNow) {
        __try {
            HRESULT hr = ((FnDllCanUnloadNow)pDllCanUnloadNow)();
            printf("  Returned 0x%08X (%s)\n", (unsigned)hr,
                hr == S_OK ? "S_OK (can unload)" :
                hr == S_FALSE ? "S_FALSE (cannot unload)" :
                "other");
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
        }
    }

    // 2. RunAsStandAlone — call with NULL/minimal params (SEH-guarded)
    printf("\n--- RunAsStandAlone ---\n");
    if (pRunAsStandAlone) {
        __try {
            ((FnRunAsStandAlone)pRunAsStandAlone)(NULL, NULL, NULL, 0);
            printf("  Returned (may hang or block)\n");
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
        }
    }

    // 3. DisplayRepairPromptDialog — call with NULL parent HWND
    printf("\n--- DisplayRepairPromptDialog ---\n");
    if (pDisplayRepairPromptDialog) {
        __try {
            HRESULT hr = ((FnDisplayRepairPromptDialog)pDisplayRepairPromptDialog)(NULL);
            printf("  Returned 0x%08X\n", (unsigned)hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("  EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode());
        }
    }

    printf("\n=== Test Complete ===\n");
    FreeLibrary(hMod);
    return 0;
}
