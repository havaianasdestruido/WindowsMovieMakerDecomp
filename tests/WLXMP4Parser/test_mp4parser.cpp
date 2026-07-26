#include <windows.h>
#include <cstdio>
#include <cstdint>

typedef HRESULT (WINAPI *AddMP4SourceFilterFn)(IUnknown*, LPCWSTR, IUnknown**);
typedef HRESULT (WINAPI *BuildMP4FilterGraphFn)(IUnknown*, LPCWSTR, IUnknown**, IUnknown**);
typedef HRESULT (WINAPI *BuildMP4PlayBackFn)(IUnknown*, LPCWSTR, IUnknown**, IUnknown**, IUnknown**);
typedef BOOL    (WINAPI *IsMP4FilePlayableFn)(LPCWSTR);

#define SEH_TRY __try
#define SEH_EXCEPT __except(EXCEPTION_EXECUTE_HANDLER)

void TestExport(const char* name, FARPROC proc, const char* dllPath) {
    printf("  [%s] ", name);
    if (!proc) {
        printf("NOT FOUND\n");
        return;
    }
    printf("addr=0x%p ", proc);

    if (strcmp(name, "DllMain") == 0 || strcmp(name, "DllRegisterServer") == 0 ||
        strcmp(name, "DllUnregisterServer") == 0 || strcmp(name, "DllCanUnloadNow") == 0 ||
        strcmp(name, "DllGetClassObject") == 0) {
        printf("skip (COM entry)\n");
        return;
    }

    if (strcmp(name, "IsMP4FilePlayable") == 0) {
        auto fn = (IsMP4FilePlayableFn)proc;
        SEH_TRY {
            BOOL result = fn(L"C:\\nonexistent.mp4");
            printf("result=%d (expected 0 for bad path)\n", result);
        } SEH_EXCEPT {
            printf("SEH exception code=0x%08X\n", GetExceptionCode());
        }
        return;
    }

    if (strcmp(name, "AddMP4SourceFilter") == 0 || strcmp(name, "BuildMP4FilterGraph") == 0 ||
        strcmp(name, "BuildMP4PlayBack") == 0) {
        printf("requires COM infrastructure, skipping call\n");
        return;
    }

    printf("unknown export, skipping\n");
}

int main() {
    printf("=== WLXMP4Parser.dll Dynamic Analysis ===\n\n");

    const char* dllPath = "undecomp\\Photo Gallery\\WLXMP4Parser.dll";
    printf("Loading: %s\n", dllPath);

    HMODULE hMod = LoadLibraryA(dllPath);
    if (!hMod) {
        printf("Failed to load DLL. Error: %u\n", GetLastError());
        return 1;
    }
    printf("Module base: 0x%p\n\n", hMod);

    struct ExportEntry {
        const char* name;
        FARPROC addr;
    };

    ExportEntry exports[] = {
        {"AddMP4SourceFilter", GetProcAddress(hMod, "AddMP4SourceFilter")},
        {"BuildMP4FilterGraph", GetProcAddress(hMod, "BuildMP4FilterGraph")},
        {"BuildMP4PlayBack", GetProcAddress(hMod, "BuildMP4PlayBack")},
        {"DllCanUnloadNow", GetProcAddress(hMod, "DllCanUnloadNow")},
        {"DllGetClassObject", GetProcAddress(hMod, "DllGetClassObject")},
        {"DllMain", GetProcAddress(hMod, "DllMain")},
        {"DllRegisterServer", GetProcAddress(hMod, "DllRegisterServer")},
        {"DllUnregisterServer", GetProcAddress(hMod, "DllUnregisterServer")},
        {"IsMP4FilePlayable", GetProcAddress(hMod, "IsMP4FilePlayable")},
    };

    int count = sizeof(exports) / sizeof(exports[0]);
    printf("--- Export Table (%d entries) ---\n", count);
    for (int i = 0; i < count; i++) {
        TestExport(exports[i].name, exports[i].addr, dllPath);
    }

    printf("\n--- DLL Exports Summary ---\n");
    printf("Module: WLXMP4Parser.dll\n");
    printf("Exports: %d\n", count);
    printf("Base: 0x%p\n", hMod);

    SEH_TRY {
        DWORD ver = GetVersion();
        printf("OS Version: %u.%u\n", ver & 0xFF, (ver >> 8) & 0xFF);
    } SEH_EXCEPT {
        printf("OS Version: unavailable\n");
    }

    printf("\n--- RTTI Classes (from static analysis) ---\n");
    printf("Total RTTI classes identified: 57+\n");
    printf("AVCQT* atom classes: 35+\n");
    printf("Filter/Pin/Graph classes: 15+\n");
    printf("Metadata classes: 7+\n");

    printf("\n=== Analysis Complete ===\n");

    FreeLibrary(hMod);
    return 0;
}
