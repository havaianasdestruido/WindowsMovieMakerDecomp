#include <windows.h>
#include <stdio.h>
int main() {
    printf("Loading...\n");
    SetDllDirectoryA("undecomp\\Photo Gallery");
    HMODULE h = LoadLibraryA("WLXPhotoBase.dll");
    printf("h=%p err=%lu\n", h, GetLastError());
    if (h) {
        printf("DllMain succeeded\n");
        FARPROC p = GetProcAddress(h, "?GetProcessorCount@CPU@Base@@YGHXZ");
        printf("GetProcessorCount @ %p\n", p);
        if (p) {
            int n = ((int(__stdcall*)())p)();
            printf("ProcessorCount = %d\n", n);
        }
        FreeLibrary(h);
    }
    return 0;
}
