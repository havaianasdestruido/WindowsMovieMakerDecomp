#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>

typedef HRESULT (WINAPI *PFNDllCanUnloadNow)(void);
typedef HRESULT (WINAPI *PFNDllGetClassObject)(REFCLSID rclsid, REFIID riid, void** ppv);
typedef HRESULT (WINAPI *PFN_CreateSrcReaderURL)(const WCHAR*, void*, void**);
typedef HRESULT (WINAPI *PFN_CreateSrcReaderBS)(void*, void*, void**);
typedef HRESULT (WINAPI *PFN_CreateSrcReaderMS)(void*, void*, void**);
typedef HRESULT (WINAPI *PFN_CreateSinkWriterURL)(const WCHAR*, void*, void*, void**);
typedef HRESULT (WINAPI *PFN_CreateSinkWriterMS)(void*, void*, void**);

struct ExportEntry {
    const char* name;
    FARPROC pfn;
};

static void LogResult(const char* call, HRESULT hr) {
    printf("  %-56s -> 0x%08X", call, (unsigned)hr);
    if (FAILED(hr)) {
        char buf[64];
        switch ((unsigned)hr) {
            case 0x80070057: printf("  (E_INVALIDARG)"); break;
            case 0x80004003: printf("  (E_POINTER)"); break;
            case 0x8007000E: printf("  (E_OUTOFMEMORY)"); break;
            case 0x80004005: printf("  (E_FAIL)"); break;
            case 0x80004002: printf("  (E_NOINTERFACE)"); break;
            case 0x80040154: printf("  (CLASS_E_CLASSNOTAVAILABLE)"); break;
            default: printf("  (FAILED)"); break;
        }
    } else if (hr == S_OK) {
        printf("  (S_OK)");
    }
    printf("\n");
}

int main() {
    printf("=== WLMFReadWrite.dll Dynamic Analysis ===\n\n");

    HMODULE hDll = LoadLibraryW(L"undecomp\\Shared\\WLMFReadWrite.dll");
    if (!hDll) {
        hDll = LoadLibraryW(L"WLMFReadWrite.dll");
    }
    if (!hDll) {
        printf("FAIL: Cannot load WLMFReadWrite.dll (err=%u)\n", GetLastError());
        return 1;
    }
    printf("Module base: %p\n\n", hDll);

    PFNDllCanUnloadNow pfnCanUnload =
        (PFNDllCanUnloadNow)GetProcAddress(hDll, "DllCanUnloadNow");
    PFNDllGetClassObject pfnGetClass =
        (PFNDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject");
    PFN_CreateSrcReaderURL pfnSrcURL =
        (PFN_CreateSrcReaderURL)GetProcAddress(hDll, "MFCreateSourceReaderFromURL");
    PFN_CreateSrcReaderBS pfnSrcBS =
        (PFN_CreateSrcReaderBS)GetProcAddress(hDll, "MFCreateSourceReaderFromByteStream");
    PFN_CreateSrcReaderMS pfnSrcMS =
        (PFN_CreateSrcReaderMS)GetProcAddress(hDll, "MFCreateSourceReaderFromMediaSource");
    PFN_CreateSinkWriterURL pfnSnkURL =
        (PFN_CreateSinkWriterURL)GetProcAddress(hDll, "MFCreateSinkWriterFromURL");
    PFN_CreateSinkWriterMS pfnSnkMS =
        (PFN_CreateSinkWriterMS)GetProcAddress(hDll, "MFCreateSinkWriterFromMediaSink");

    ExportEntry exports[] = {
        {"DllCanUnloadNow",                     (FARPROC)pfnCanUnload},
        {"DllGetClassObject",                   (FARPROC)pfnGetClass},
        {"MFCreateSourceReaderFromURL",         (FARPROC)pfnSrcURL},
        {"MFCreateSourceReaderFromByteStream",  (FARPROC)pfnSrcBS},
        {"MFCreateSourceReaderFromMediaSource", (FARPROC)pfnSrcMS},
        {"MFCreateSinkWriterFromURL",           (FARPROC)pfnSnkURL},
        {"MFCreateSinkWriterFromMediaSink",     (FARPROC)pfnSnkMS},
    };

    printf("--- Export Resolution (7 functions) ---\n");
    int resolved = 0;
    for (int i = 0; i < 7; i++) {
        BOOL ok = exports[i].pfn != NULL;
        printf("  %-44s %s\n", exports[i].name, ok ? "[OK]" : "[MISSING]");
        if (ok) resolved++;
    }
    printf("Resolved: %d/7\n\n", resolved);

    printf("--- DllCanUnloadNow ---\n");
    if (pfnCanUnload) {
        __try {
            HRESULT hr = pfnCanUnload();
            LogResult("DllCanUnloadNow()", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            LogResult("DllCanUnloadNow() [exception]", (HRESULT)GetExceptionCode());
        }
    }
    printf("\n");

    printf("--- MFCreateSourceReaderFromURL (NULL params) ---\n");
    if (pfnSrcURL) {
        __try {
            void* out = NULL;
            HRESULT hr = pfnSrcURL(NULL, NULL, &out);
            LogResult("MFCreateSourceReaderFromURL(NULL, NULL, &out)", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            LogResult("MFCreateSourceReaderFromURL(NULL,NULL,&out) [exc]", (HRESULT)GetExceptionCode());
        }
        __try {
            HRESULT hr = pfnSrcURL(L"", NULL, NULL);
            LogResult("MFCreateSourceReaderFromURL(L\"\", NULL, NULL)", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            LogResult("MFCreateSourceReaderFromURL(L\"\",NULL,NULL) [exc]", (HRESULT)GetExceptionCode());
        }
    }
    printf("\n");

    printf("--- MFCreateSourceReaderFromByteStream (NULL params) ---\n");
    if (pfnSrcBS) {
        __try {
            void* out = NULL;
            HRESULT hr = pfnSrcBS(NULL, NULL, &out);
            LogResult("MFCreateSourceReaderFromByteStream(NULL, NULL, &out)", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            LogResult("MFCreateSourceReaderFromByteStream(NULL,NULL,&out) [exc]", (HRESULT)GetExceptionCode());
        }
    }
    printf("\n");

    printf("--- MFCreateSourceReaderFromMediaSource (NULL params) ---\n");
    if (pfnSrcMS) {
        __try {
            void* out = NULL;
            HRESULT hr = pfnSrcMS(NULL, NULL, &out);
            LogResult("MFCreateSourceReaderFromMediaSource(NULL, NULL, &out)", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            LogResult("MFCreateSourceReaderFromMediaSource(NULL,NULL,&out) [exc]", (HRESULT)GetExceptionCode());
        }
    }
    printf("\n");

    printf("--- MFCreateSinkWriterFromURL (NULL params) ---\n");
    if (pfnSnkURL) {
        __try {
            void* out = NULL;
            HRESULT hr = pfnSnkURL(NULL, NULL, NULL, &out);
            LogResult("MFCreateSinkWriterFromURL(NULL, NULL, NULL, &out)", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            LogResult("MFCreateSinkWriterFromURL(NULL,NULL,NULL,&out) [exc]", (HRESULT)GetExceptionCode());
        }
        __try {
            HRESULT hr = pfnSnkURL(L"", NULL, NULL, NULL);
            LogResult("MFCreateSinkWriterFromURL(L\"\", NULL, NULL, NULL)", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            LogResult("MFCreateSinkWriterFromURL(L\"\",NULL,NULL,NULL) [exc]", (HRESULT)GetExceptionCode());
        }
    }
    printf("\n");

    printf("--- MFCreateSinkWriterFromMediaSink (NULL params) ---\n");
    if (pfnSnkMS) {
        __try {
            void* out = NULL;
            HRESULT hr = pfnSnkMS(NULL, NULL, &out);
            LogResult("MFCreateSinkWriterFromMediaSink(NULL, NULL, &out)", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            LogResult("MFCreateSinkWriterFromMediaSink(NULL,NULL,&out) [exc]", (HRESULT)GetExceptionCode());
        }
    }
    printf("\n");

    printf("--- DllCanUnloadNow (after calls) ---\n");
    if (pfnCanUnload) {
        __try {
            HRESULT hr = pfnCanUnload();
            LogResult("DllCanUnloadNow() [2nd call]", hr);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            LogResult("DllCanUnloadNow() [2nd] [exc]", (HRESULT)GetExceptionCode());
        }
    }

    printf("\n=== Test Complete ===\n");
    return 0;
}
