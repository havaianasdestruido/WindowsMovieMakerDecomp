#include <windows.h>
#include <stdio.h>

// Just test that our inline asm helper compiles and works at all
static long simple_asm_test(long a, long b) {
    long r;
    __asm {
        mov eax, a
        add eax, b
        mov r, eax
    }
    return r;
}

int main() {
    printf("Start\n");
    long r = simple_asm_test(3, 4);
    printf("asm test: %ld\n", r);
    
    printf("Loading DLL...\n");
    SetDllDirectoryA("undecomp\\Photo Gallery");
    HMODULE h = LoadLibraryA("WLXPhotoBase.dll");
    printf("h=%p err=%lu\n", h, GetLastError());
    
    if (h) {
        printf("Testing GetProcAddress...\n");
        FARPROC fp = GetProcAddress(h, "?GetProcessorCount@CPU@Base@@YGHXZ");
        printf("GetProcessorCount @ %p\n", fp);
        FreeLibrary(h);
    }
    printf("Done\n");
    return 0;
}
