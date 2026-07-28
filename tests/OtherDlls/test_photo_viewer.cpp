// WLXPhotoViewer.dll Test Harness
// Main photo viewer (codename "Easel") — 6 exports, 308 RTTI classes
// Exports: DllCanUnloadNow, DllGetClassObject, DllRegisterServer,
//          DllUnregisterServer, LaunchViewer, LaunchViewerComServer
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unknwn.h>
#include <cstdio>
#include <cstdlib>

// ============================================================
// GUIDs extracted from binary
// ============================================================
static const CLSID CLSID_Empty = {0,0,0,{0,0,0,0,0,0,0,0}};
static const IID IID_IUnknown_Cast =
    {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

// CLSID_PhotoViewer — the primary COM class
// {BDA3D714-8482-4F75-B801-6F6EB607BFE5}
static const GUID CLSID_PhotoViewer =
    {0xBDA3D714,0x8482,0x4F75,{0xB8,0x01,0x6F,0x6E,0xB6,0x07,0xBF,0xE5}};

// {DC619D43-32E3-4981-9D9A-DBE91A8B2061}
static const GUID CLSID_SecondCandidate =
    {0xDC619D43,0x32E3,0x4981,{0x9D,0x9A,0xDB,0xE9,0x1A,0x8B,0x20,0x61}};

// {6BF52A52-394A-11d3-B153-00C04F79FAA6} — WMP-related
static const GUID CLSID_WMPCandidate =
    {0x6BF52A52,0x394A,0x11d3,{0xB1,0x53,0x00,0xC0,0x4F,0x79,0xFA,0xA6}};

// ============================================================
// Helpers
// ============================================================
static void SetSearchPaths() {
    wchar_t repoRoot[MAX_PATH];
    GetFullPathNameW(L"..\\..", MAX_PATH, repoRoot, NULL);
    wchar_t galleryDir[MAX_PATH], sharedDir[MAX_PATH];
    wcscpy_s(galleryDir, repoRoot); wcscat_s(galleryDir, L"\\undecomp\\Photo Gallery");
    wcscpy_s(sharedDir, repoRoot); wcscat_s(sharedDir, L"\\undecomp\\Shared");
    AddDllDirectory(galleryDir);
    AddDllDirectory(sharedDir);
    AddDllDirectory(repoRoot);
    SetDllDirectoryW(galleryDir);
}

static HMODULE SafeLoadDll(const wchar_t* relPath) {
    wchar_t repoRoot[MAX_PATH];
    GetFullPathNameW(L"..\\..", MAX_PATH, repoRoot, NULL);
    wchar_t galleryDir[MAX_PATH];
    wcscpy_s(galleryDir, repoRoot); wcscat_s(galleryDir, L"\\undecomp\\Photo Gallery");
    wchar_t fullPath[MAX_PATH];
    wcscpy_s(fullPath, repoRoot); wcscat_s(fullPath, L"\\"); wcscat_s(fullPath, relPath);
    printf("[*] Loading: %ls\n", fullPath);
    HMODULE h = LoadLibraryExW(fullPath, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!h) {
        printf("[!] LoadLibraryEx failed (err %u), retrying SetDllDirectory\n", GetLastError());
        SetDllDirectoryW(galleryDir);
        h = LoadLibraryW(fullPath);
    }
    if (h)
        printf("[+] Loaded at base: 0x%p\n", h);
    else
        printf("[!] FAILED to load: error %u\n", GetLastError());
    return h;
}

// ============================================================
// Export typedefs (6 exports)
// ============================================================
typedef HRESULT (__stdcall *DllCanUnloadNowFunc)();
typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT (__stdcall *DllRegisterServerFunc)();
typedef HRESULT (__stdcall *DllUnregisterServerFunc)();
typedef void (__stdcall *LaunchViewerFunc)(int, const wchar_t**);
typedef void (__stdcall *LaunchViewerComServerFunc)(int, const wchar_t**);

// ============================================================
// Standard COM exports
// ============================================================
static void TestComExports(HMODULE h) {
    printf("\n========================================\n");
    printf("  WLXPhotoViewer.dll Standard COM Exports\n");
    printf("========================================\n");

    // DllCanUnloadNow (ordinal 1, hint 0)
    {
        printf("\n[*] DllCanUnloadNow (ordinal 1, hint 0)\n");
        DllCanUnloadNowFunc fn = (DllCanUnloadNowFunc)GetProcAddress(h, "DllCanUnloadNow");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            __try {
                HRESULT hr = fn();
                printf("    => HRESULT=0x%08X\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllGetClassObject (ordinal 2, hint 1)
    {
        printf("\n[*] DllGetClassObject (ordinal 2, hint 1)\n");
        DllGetClassObjectFunc fn = (DllGetClassObjectFunc)GetProcAddress(h, "DllGetClassObject");
        if (fn) {
            printf("    ptr=0x%p\n", fn);

            // Test with empty CLSID
            __try {
                HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, NULL);
                printf("    => HRESULT=0x%08X (null CLSID, null ppv)\n", hr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }

            // Test with empty CLSID + valid ppv
            {
                void* pUnk = NULL;
                __try {
                    HRESULT hr = fn(CLSID_Empty, IID_IUnknown_Cast, &pUnk);
                    printf("    => HRESULT=0x%08X (null CLSID, valid ppv)\n", hr);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                }
            }

            // Test CLSID_PhotoViewer
            {
                printf("\n    [*] DllGetClassObject with CLSID_PhotoViewer {BDA3D714-8482-4F75-B801-6F6EB607BFE5}\n");
                void* pUnk = NULL;
                __try {
                    HRESULT hr = fn(CLSID_PhotoViewer, IID_IUnknown_Cast, &pUnk);
                    printf("        => HRESULT=0x%08X", hr);
                    if (SUCCEEDED(hr) && pUnk) {
                        printf(", IUnknown=0x%p", pUnk);
                        IUnknown* punk = (IUnknown*)pUnk;
                        punk->Release();
                        printf(" [RELEASED]");
                    }
                    printf("\n");
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("        => SEH exception 0x%08X\n", GetExceptionCode());
                }
            }

            // Test CLSID_SecondCandidate
            {
                printf("\n    [*] DllGetClassObject with CLSID_SecondCandidate {DC619D43-32E3-4981-9D9A-DBE91A8B2061}\n");
                void* pUnk = NULL;
                __try {
                    HRESULT hr = fn(CLSID_SecondCandidate, IID_IUnknown_Cast, &pUnk);
                    printf("        => HRESULT=0x%08X", hr);
                    if (SUCCEEDED(hr) && pUnk) {
                        printf(", IUnknown=0x%p", pUnk);
                        IUnknown* punk = (IUnknown*)pUnk;
                        punk->Release();
                        printf(" [RELEASED]");
                    }
                    printf("\n");
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("        => SEH exception 0x%08X\n", GetExceptionCode());
                }
            }

            // Test CLSID_WMPCandidate
            {
                printf("\n    [*] DllGetClassObject with CLSID_WMPCandidate {6BF52A52-394A-11d3-B153-00C04F79FAA6}\n");
                void* pUnk = NULL;
                __try {
                    HRESULT hr = fn(CLSID_WMPCandidate, IID_IUnknown_Cast, &pUnk);
                    printf("        => HRESULT=0x%08X", hr);
                    if (SUCCEEDED(hr) && pUnk) {
                        printf(", IUnknown=0x%p", pUnk);
                        IUnknown* punk = (IUnknown*)pUnk;
                        punk->Release();
                        printf(" [RELEASED]");
                    }
                    printf("\n");
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("        => SEH exception 0x%08X\n", GetExceptionCode());
                }
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllRegisterServer (ordinal 3, hint 2)
    {
        printf("\n[*] DllRegisterServer (ordinal 3, hint 2)\n");
        DllRegisterServerFunc fn = (DllRegisterServerFunc)GetProcAddress(h, "DllRegisterServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            printf("    [skip] Would write to registry - not called\n");
        } else {
            printf("    [!] not found\n");
        }
    }

    // DllUnregisterServer (ordinal 4, hint 3)
    {
        printf("\n[*] DllUnregisterServer (ordinal 4, hint 3)\n");
        DllUnregisterServerFunc fn = (DllUnregisterServerFunc)GetProcAddress(h, "DllUnregisterServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);
            printf("    [skip] Would write to registry - not called\n");
        } else {
            printf("    [!] not found\n");
        }
    }
}

// ============================================================
// Non-COM exports: LaunchViewer, LaunchViewerComServer
// ============================================================
static void TestLaunchExports(HMODULE h) {
    printf("\n========================================\n");
    printf("  WLXPhotoViewer.dll Launch Exports\n");
    printf("========================================\n");

    // LaunchViewer (ordinal 5, hint 4)
    {
        printf("\n[*] LaunchViewer (ordinal 5, hint 4)\n");
        LaunchViewerComServerFunc fn = (LaunchViewerComServerFunc)GetProcAddress(h, "LaunchViewer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);

            // Probe with NULL args
            printf("    [*] Probe with NULL/0...\n");
            __try {
                fn(0, NULL);
                printf("    => returned (argc=0, argv=NULL)\n");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }

            // Probe with argc=1, empty string
            {
                const wchar_t* fakeArg = L"";
                printf("    [*] Probe with argc=1, empty string...\n");
                __try {
                    fn(1, &fakeArg);
                    printf("    => returned (argc=1, argv=L\"\")\n");
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                }
            }
        } else {
            printf("    [!] not found\n");
        }
    }

    // LaunchViewerComServer (ordinal 6, hint 5)
    {
        printf("\n[*] LaunchViewerComServer (ordinal 6, hint 5)\n");
        LaunchViewerComServerFunc fn = (LaunchViewerComServerFunc)GetProcAddress(h, "LaunchViewerComServer");
        if (fn) {
            printf("    ptr=0x%p\n", fn);

            // Probe with NULL args
            printf("    [*] Probe with NULL/0...\n");
            __try {
                fn(0, NULL);
                printf("    => returned (argc=0, argv=NULL)\n");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                printf("    => SEH exception 0x%08X\n", GetExceptionCode());
            }

            // Probe with argc=1, empty string
            {
                const wchar_t* fakeArg = L"";
                printf("    [*] Probe with argc=1, empty string...\n");
                __try {
                    fn(1, &fakeArg);
                    printf("    => returned (argc=1, argv=L\"\")\n");
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("    => SEH exception 0x%08X\n", GetExceptionCode());
                }
            }
        } else {
            printf("    [!] not found\n");
        }
    }
}

// ============================================================
// Main
// ============================================================
int main(int argc, char** argv) {
    printf("=== WLXPhotoViewer.dll Test Harness ===\n");
    printf("Target: Main photo viewer (codename Easel)\n");
    printf("6 exports, 308 RTTI classes, PE32 x86\n\n");

    SetSearchPaths();

    HMODULE hViewer = SafeLoadDll(L"undecomp\\Photo Gallery\\WLXPhotoViewer.dll");
    if (hViewer) {
        TestComExports(hViewer);
        TestLaunchExports(hViewer);

        printf("\n========================================\n");
        printf("  Module Information\n");
        printf("========================================\n");
        wchar_t modPath[MAX_PATH];
        GetModuleFileNameW(hViewer, modPath, MAX_PATH);
        printf("[*] Full path: %ls\n", modPath);
        DWORD fileSize = GetFileSize(modPath, NULL);
        printf("[*] File size: %u bytes (0x%X)\n", fileSize, fileSize);

        printf("\n[*] Unloading WLXPhotoViewer.dll\n");
        FreeLibrary(hViewer);
    }

    printf("\n=== Done ===\n");
    return 0;
}
