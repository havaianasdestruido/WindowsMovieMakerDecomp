#include <windows.h>
#include <stdio.h>
int main() {
    printf("Start\n");
    SetDllDirectoryA("undecomp\\Photo Gallery");
    HMODULE h = LoadLibraryA("WLXPhotoBase.dll");
    printf("h=%p err=%lu\n", h, GetLastError());
    if (h) FreeLibrary(h);
    return 0;
}
