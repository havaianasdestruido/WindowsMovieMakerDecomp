#include <windows.h>
#include <stdio.h>

typedef int (__cdecl *NP_Initialize_t)(void);
typedef int (__cdecl *NP_Shutdown_t)(void);
typedef int (__cdecl *NP_GetEntryPoints_t)(void *, void *, void *);

static HMODULE hDll = NULL;

static const char *nperr_str(int c) {
    switch (c) {
        case 0:  return "NPERR_NO_ERROR";
        case 1:  return "NPERR_GENERIC_ERROR";
        case 2:  return "NPERR_INVALID_INSTANCE_ERROR";
        case 3:  return "NPERR_OUT_OF_MEMORY_ERROR";
        case 4:  return "NPERR_INVALID_PLUGIN_ERROR";
        default: return "UNKNOWN";
    }
}

static int test_load(void) {
    printf("[TEST] Loading NPWLPG.dll...\n");
    hDll = LoadLibraryA("NPWLPG.dll");
    if (!hDll) { printf("  FAIL: %lu\n", GetLastError()); return -1; }
    printf("  OK: 0x%p\n", hDll);
    return 0;
}

static int test_exports(void) {
    printf("[TEST] Resolving NPAPI exports...\n");
    if (!hDll) return -1;
    int ok = 0;
    const char *names[] = {"NP_Initialize","NP_Shutdown","NP_GetEntryPoints"};
    for (int i = 0; i < 3; i++) {
        FARPROC p = GetProcAddress(hDll, names[i]);
        printf("  %-20s %s (0x%p)\n", names[i], p?"OK":"MISSING", p);
        if (!p) ok = -1;
    }
    return ok;
}

static int test_pe(void) {
    printf("[TEST] PE validation...\n");
    if (!hDll) return -1;
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hDll;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) { printf("  FAIL: bad MZ\n"); return -1; }
    PIMAGE_NT_HEADERS32 nt = (PIMAGE_NT_HEADERS32)((PBYTE)dos + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) { printf("  FAIL: bad PE\n"); return -1; }
    printf("  Machine: 0x%X, Sections: %u, EP: 0x%08X\n",
           nt->FileHeader.Machine, nt->FileHeader.NumberOfSections,
           nt->OptionalHeader.AddressOfEntryPoint);
    printf("  DLL: %s, PE32: %s\n",
           (nt->FileHeader.Characteristics & IMAGE_FILE_DLL) ? "yes" : "no",
           nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ? "yes" : "no");
    return 0;
}

static int test_init(void) {
    printf("[TEST] NP_Initialize()...\n");
    if (!hDll) return -1;
    NP_Initialize_t fn = (NP_Initialize_t)GetProcAddress(hDll, "NP_Initialize");
    if (!fn) return -1;
    int r = fn();
    printf("  Result: %d (%s)\n", r, nperr_str(r));
    return 0;
}

static int test_shutdown(void) {
    printf("[TEST] NP_Shutdown()...\n");
    if (!hDll) return -1;
    NP_Shutdown_t fn = (NP_Shutdown_t)GetProcAddress(hDll, "NP_Shutdown");
    if (!fn) return -1;
    int r = fn();
    printf("  Result: %d\n", r);
    return 0;
}

int main(void) {
    printf("=== NPWLPG.dll NPAPI Plugin Test ===\n\n");
    int f = 0;
    f += (test_load() != 0);
    f += (test_exports() != 0);
    f += (test_pe() != 0);
    f += (test_init() != 0);
    test_shutdown();
    if (hDll) FreeLibrary(hDll);
    printf("\n=== %d failure(s) ===\n", f);
    return f;
}
