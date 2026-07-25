// MovieMakerCore.dll Test Harness
// Loads the DLL and calls the single export: MovieMakerMain
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdlib>

typedef int (__cdecl *MovieMakerMainProc)(int argc, wchar_t** argv);

int main(int argc, char** argv) {
    printf("=== MovieMakerCore.dll Test Harness ===\n\n");

    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery\\MovieMakerCore.dll", MAX_PATH, fullPath, NULL);
    printf("[*] Full DLL path: %ls\n", fullPath);

    // Add dependency directories to search path
    wchar_t galleryDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, galleryDir, NULL);
    AddDllDirectory(galleryDir);

    wchar_t sharedDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Shared", MAX_PATH, sharedDir, NULL);
    AddDllDirectory(sharedDir);

    wchar_t mainDir[MAX_PATH];
    GetFullPathNameW(L".", MAX_PATH, mainDir, NULL);
    AddDllDirectory(mainDir);

    printf("[*] Loading DLL\n");
    HMODULE hMod = LoadLibraryExW(fullPath, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!hMod) {
        printf("[!] LoadLibraryEx failed: error %u\n", GetLastError());
        printf("[*] Falling back to SetDllDirectory...\n");
        SetDllDirectoryW(galleryDir);
        hMod = LoadLibraryW(fullPath);
        if (!hMod) {
            printf("[!] LoadLibrary also failed: error %u\n", GetLastError());
            return 1;
        }
    }
    printf("[+] DLL loaded at base: 0x%p\n", hMod);

    MovieMakerMainProc pMain = (MovieMakerMainProc)GetProcAddress(hMod, "MovieMakerMain");
    if (!pMain) {
        printf("[!] GetProcAddress failed: error %u\n", GetLastError());
        FreeLibrary(hMod);
        return 1;
    }
    printf("[+] MovieMakerMain at: 0x%p (RVA 0x%08X)\n", pMain, (DWORD)((BYTE*)pMain - (BYTE*)hMod));

    wchar_t* testArgv[] = { L"MovieMakerCore.dll" };
    printf("[*] Calling MovieMakerMain(1, testArgv)...\n");

    int result = -1;
    __try {
        result = pMain(1, testArgv);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD code = GetExceptionCode();
        printf("[!] Exception caught: 0x%08X\n", code);
        if (code == 0xE06D7363) {
            printf("[!] C++ exception (mscpp) thrown\n");
        } else if (code == EXCEPTION_ACCESS_VIOLATION) {
            printf("[!] Access violation\n");
        } else if (code == EXCEPTION_STACK_OVERFLOW) {
            printf("[!] Stack overflow\n");
        } else {
            printf("[!] SEH exception code: 0x%08X\n", code);
        }
    }

    printf("\n[*] MovieMakerMain returned: %d (0x%08X)\n", result, result);
    printf("[*] Last error: %u\n", GetLastError());

    FreeLibrary(hMod);
    printf("[+] DLL unloaded\n");
    return 0;
}
