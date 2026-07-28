#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("Start\n");
    
    long hr = 0x80004005;
    __try {
        printf("In __try\n");
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("Caught SEH 0x%08X\n", (unsigned)GetExceptionCode());
    }
    
    printf("Done\n");
    return 0;
}
