#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int main(void) {
    printf("Start\n");
    SetDllDirectoryA("undecomp\\Photo Gallery");
    HMODULE hMod = LoadLibraryA("WLXPhotoBase.dll");
    if (!hMod) { printf("FAILED: %lu\n", GetLastError()); return 1; }
    printf("Loaded at %p\n", (void*)hMod);
    FreeLibrary(hMod);
    printf("Done\n");
    return 0;
}
