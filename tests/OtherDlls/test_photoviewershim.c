#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "advapi32.lib")

typedef HRESULT (__stdcall *DllCanUnloadNow_t)(void);
typedef HRESULT (__stdcall *DllGetClassObject_t)(REFCLSID, REFIID, LPVOID *);
typedef HRESULT (__stdcall *DllRegisterServer_t)(void);
typedef HRESULT (__stdcall *DllUnregisterServer_t)(void);

static HMODULE hDll = NULL;

static const char *guid_str(const GUID *g) {
    static char buf[48];
    sprintf(buf, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            g->Data1, g->Data2, g->Data3,
            g->Data4[0], g->Data4[1], g->Data4[2], g->Data4[3],
            g->Data4[4], g->Data4[5], g->Data4[6], g->Data4[7]);
    return buf;
}

static HRESULT test_can_unload(void) {
    printf("[TEST] DllCanUnloadNow()...\n");
    DllCanUnloadNow_t fn = (DllCanUnloadNow_t)GetProcAddress(hDll, "DllCanUnloadNow");
    if (!fn) { printf("  MISSING\n"); return E_FAIL; }
    HRESULT hr = fn();
    printf("  Result: 0x%08lX (%s)\n", hr, hr == S_OK ? "S_OK (can unload)" : "S_FALSE (busy)");
    return hr;
}

static int test_exports(void) {
    printf("[TEST] Resolving COM exports...\n");
    const char *names[] = {
        "DllCanUnloadNow", "DllGetClassObject",
        "DllRegisterServer", "DllUnregisterServer"
    };
    int ok = 0;
    for (int i = 0; i < 4; i++) {
        FARPROC p = GetProcAddress(hDll, names[i]);
        printf("  %-24s %s\n", names[i], p ? "OK" : "MISSING");
        if (!p) ok = -1;
    }
    return ok;
}

static int test_pe_info(void) {
    printf("[TEST] PE info...\n");
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hDll;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return -1;
    PIMAGE_NT_HEADERS32 nt = (PIMAGE_NT_HEADERS32)((PBYTE)dos + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return -1;
    printf("  Machine: 0x%X, EP: 0x%08X, ImageBase: 0x%08X\n",
           nt->FileHeader.Machine, nt->OptionalHeader.AddressOfEntryPoint,
           nt->OptionalHeader.ImageBase);
    return 0;
}

static int test_reg_check(void) {
    printf("[TEST] Checking CLSID registration...\n");
    /* CLSID for Live Shell Viewer Extension */
    const char *clsids[] = {
        "{00f346cb-35a4-465b-8b8f-65a29dbab1f6}",
        "{00f3712a-ca79-45b4-9e4d-d7891e7f8b9d}"
    };
    const char *descs[] = {
        "Live Shell Viewer Extension",
        "Live Shell Editor Extension"
    };
    for (int i = 0; i < 2; i++) {
        char key[128];
        sprintf(key, "CLSID\\%s", clsids[i]);
        HKEY hk;
        LONG r = RegOpenKeyExA(HKEY_CLASSES_ROOT, key, 0, KEY_READ, &hk);
        printf("  %s (%s): %s\n", descs[i], clsids[i],
               r == ERROR_SUCCESS ? "REGISTERED" : "NOT REGISTERED");
        if (r == ERROR_SUCCESS) RegCloseKey(hk);
    }
    return 0;
}

int main(void) {
    printf("=== PhotoViewerShim.dll COM Test ===\n\n");

    printf("[TEST] Loading PhotoViewerShim.dll...\n");
    hDll = LoadLibraryA("PhotoViewerShim.dll");
    if (!hDll) {
        printf("  FAIL: %lu\n", GetLastError());
        return 1;
    }
    printf("  OK: 0x%p\n\n", hDll);

    int f = 0;
    f += (test_pe_info() != 0);
    f += (test_exports() != 0);
    test_can_unload();
    test_reg_check();

    FreeLibrary(hDll);
    printf("\n=== %d failure(s) ===\n", f);
    return f;
}
