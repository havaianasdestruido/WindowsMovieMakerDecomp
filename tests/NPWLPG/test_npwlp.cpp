#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdint>

typedef uint32_t NPError;
typedef void* NPP;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int16_t int16;
typedef uint16_t uint16;

typedef void* (*NPN_GetURLPtr)(void*);
typedef void* (*NP_PrintPtr)(void*);

#define NP_VERSION_MAJOR 0
#define NP_VERSION_MINOR 31

typedef struct _NPPluginFuncs {
    uint16 size;
    uint16 version;
    void* newp;
    void* destroy;
    void* setwindow;
    void* newstream;
    void* destroystream;
    void* streamasfile;
    void* writeready;
    void* write;
    void* writebuf;
    void* saveasfile;
    void* print;
    int16 event;
    void* urldata;
    void* javaClass;
    void* getvalue;
    void* setvalue;
    void* gotfocus;
    void* lostfocus;
    void* urlnotify;
    void* clearsitedata;
    void* getsiteswithdata;
} NPPluginFuncs;

typedef struct _NPNetscapeFuncs {
    uint16 size;
    uint16 version;
    void* geturl;
    void* posturl;
    void* requestread;
    void* newstream;
    void* write;
    void* destroystream;
    void* status;
    void* uagent;
    void* memalloc;
    void* memfree;
    void* memflush;
    void* reloadplugins;
    void* getJavaEnv;
    void* getJavaPeer;
    void* geturlnotify;
    void* posturlnotify;
    void* getvalue;
    void* setvalue;
    void* invalidaterect;
    void* invalidateregion;
    void* forceredraw;
    void* getstringidentifier;
    void* getstringidentifiers;
    void* getintidentifier;
    void* identifierisstring;
    void* utf8fromidentifier;
    void* intfromidentifier;
    void* createobject;
    void* retainobject;
    void* releaseobject;
    void* invoke;
    void* invokedefault;
    void* evaluate;
    void* getproperty;
    void* setproperty;
    void* removeproperty;
    void* hasproperty;
    void* hasmethod;
    void* releasevariantvalue;
    void* setexception;
    void* pushpopupsenabledstate;
    void* poppopupsenabledstate;
    void* enumerate;
    void* pluginthreadasynccall;
    void* construct;
    void* getvalueforurl;
    void* setvalueforurl;
    void* getauthenticationinfo;
    void* scheduletimer;
    void* unscheduletimer;
    void* popupcontextmenu;
    void* convertpoint;
} NPNetscapeFuncs;

typedef NPError (__stdcall *NP_InitializeFunc)(NPNetscapeFuncs*);
typedef NPError (__stdcall *NP_GetEntryPointsFunc)(NPPluginFuncs*);
typedef NPError (__stdcall *NP_ShutdownFunc)();

static const char* NPErrorToString(NPError err) {
    switch (err) {
        case 0: return "NPERR_NO_ERROR";
        case 1: return "NPERR_GENERIC_ERROR";
        case 2: return "NPERR_INVALID_FUNCS_ERROR";
        case 3: return "NPERR_INVALID_PLUGIN_ERROR";
        default: return "UNKNOWN";
    }
}

static void InitNetscapeFuncs(NPNetscapeFuncs* f, uint16 size) {
    memset(f, 0, size);
    f->size = size;
    f->version = 31;
}

static void InitPluginFuncs(NPPluginFuncs* f, uint16 size) {
    memset(f, 0, size);
    f->size = size;
    f->version = 31;
}

static void TestNP_GetEntryPoints(HMODULE h, int pass) {
    NP_GetEntryPointsFunc fn = (NP_GetEntryPointsFunc)GetProcAddress(h, "NP_GetEntryPoints");
    if (!fn) { printf("  NP_GetEntryPoints: ptr=NULL [NOT FOUND]\n"); return; }

    printf("  NP_GetEntryPoints: ptr=0x%p\n", fn);

    NPPluginFuncs funcs;
    uint16 size = 0;
    switch (pass) {
        case 0: size = sizeof(NPPluginFuncs); break;
        case 1: size = sizeof(NPPluginFuncs) - 4; break;
        case 2: size = sizeof(NPPluginFuncs) + 4; break;
        case 3: size = 0; break;
        default: return;
    }

    InitPluginFuncs(&funcs, size);
    printf("    [pass %d] size=%u => ", pass, size);

    __try {
        NPError err = fn(&funcs);
        printf("NPError=%d (%s)\n", err, NPErrorToString(err));
        if (err == 0 && funcs.newp)
            printf("      plugin func table populated (newp=0x%p)\n", funcs.newp);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("SEH exception 0x%08X\n", GetExceptionCode());
    }
}

static void TestNP_Initialize(HMODULE h, int pass) {
    NP_InitializeFunc fn = (NP_InitializeFunc)GetProcAddress(h, "NP_Initialize");
    if (!fn) { printf("  NP_Initialize: ptr=NULL [NOT FOUND]\n"); return; }

    printf("  NP_Initialize: ptr=0x%p\n", fn);

    NPNetscapeFuncs funcs;
    uint16 size = 0;
    switch (pass) {
        case 0: size = sizeof(NPNetscapeFuncs); break;
        case 1: size = sizeof(NPNetscapeFuncs) - 4; break;
        case 2: size = sizeof(NPNetscapeFuncs) + 4; break;
        case 3: size = 0; break;
        case 4: size = sizeof(NPNetscapeFuncs);
                InitNetscapeFuncs(&funcs, size);
                funcs.geturl = (void*)0xDEADBEEF;
                printf("    [pass 4] size=%u (corrupt func table) => ", size);
                __try {
                    NPError err = fn(&funcs);
                    printf("NPError=%d (%s)\n", err, NPErrorToString(err));
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    printf("SEH exception 0x%08X\n", GetExceptionCode());
                }
                return;
        default: return;
    }

    InitNetscapeFuncs(&funcs, size);
    printf("    [pass %d] size=%u => ", pass, size);

    __try {
        NPError err = fn(&funcs);
        printf("NPError=%d (%s)\n", err, NPErrorToString(err));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("SEH exception 0x%08X\n", GetExceptionCode());
    }
}

static void TestNP_Shutdown(HMODULE h) {
    NP_ShutdownFunc fn = (NP_ShutdownFunc)GetProcAddress(h, "NP_Shutdown");
    if (!fn) { printf("  NP_Shutdown: ptr=NULL [NOT FOUND]\n"); return; }

    printf("  NP_Shutdown: ptr=0x%p\n", fn);

    __try {
        NPError err = fn();
        printf("    => NPError=%d (%s)\n", err, NPErrorToString(err));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("    => SEH exception 0x%08X\n", GetExceptionCode());
    }
}

int main() {
    printf("=== NPWLPG.dll Dedicated Test Harness ===\n");
    printf("NPAPI Plugin for Windows Live Photo Gallery\n\n");

    wchar_t dllDir[MAX_PATH];
    GetFullPathNameW(L"undecomp\\Photo Gallery", MAX_PATH, dllDir, NULL);
    printf("[*] SetDllDirectoryW: %ls\n", dllDir);
    SetDllDirectoryW(dllDir);

    wchar_t dllPath[MAX_PATH];
    swprintf_s(dllPath, L"%ls\\NPWLPG.dll", dllDir);
    printf("[*] Loading: %ls\n", dllPath);
    HMODULE h = LoadLibraryW(dllPath);
    if (!h) {
        printf("[!] LoadLibrary failed: error %u\n", GetLastError());
        printf("\n=== FAILED ===\n");
        return 1;
    }
    printf("[+] Base address: 0x%p\n\n", h);

    printf("--- NP_GetEntryPoints ---\n");
    for (int i = 0; i < 4; i++)
        TestNP_GetEntryPoints(h, i);

    printf("\n--- NP_Initialize ---\n");
    for (int i = 0; i < 4; i++)
        TestNP_Initialize(h, i);

    printf("\n--- NP_Initialize (corrupt func table) ---\n");
    TestNP_Initialize(h, 4);

    printf("\n--- NP_Shutdown ---\n");
    TestNP_Shutdown(h);

    FreeLibrary(h);
    printf("\n=== Done ===\n");
    return 0;
}
