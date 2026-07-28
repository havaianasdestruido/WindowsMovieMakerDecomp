#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct VersionObj { unsigned int d[8]; };

typedef void* (__cdecl *FnT0)(void* self, FARPROC fn);
typedef void* (__cdecl *FnT1)(void* self, FARPROC fn, void* p1);
typedef void  (__cdecl *FnT4)(void* self, FARPROC fn, unsigned short a, unsigned short b, unsigned short c, unsigned short d);
typedef void  (__cdecl *FnT2)(void* self, FARPROC fn, unsigned long a, unsigned long b);

static FnT0 g_t0 = NULL;
static FnT1 g_t1 = NULL;
static FnT4 g_t4 = NULL;
static FnT2 g_t2 = NULL;
static LPVOID g_thunks = NULL;
static int g_thunkPos = 0;

static void* emit_thunk(const unsigned char* bytes, int len) {
    if (!g_thunks) g_thunks = VirtualAlloc(NULL, 4096, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    void* p = (char*)g_thunks + g_thunkPos;
    memcpy(p, bytes, len);
    g_thunkPos = (g_thunkPos + len + 15) & ~15;
    return p;
}

static void init_thunks() {
    static const unsigned char T0[] = {
        0x8B,0x4C,0x24,0x04,
        0x8B,0x44,0x24,0x08,
        0xFF,0xD0,
        0xC3
    };
    static const unsigned char T1[] = {
        0x8B,0x4C,0x24,0x04,
        0xFF,0x74,0x24,0x0C,
        0x8B,0x44,0x24,0x0C,
        0xFF,0xD0,
        0xC3
    };
    static const unsigned char T4[] = {
        0x8B,0x4C,0x24,0x04,
        0xFF,0x74,0x24,0x18,
        0xFF,0x74,0x24,0x18,
        0xFF,0x74,0x24,0x18,
        0xFF,0x74,0x24,0x18,
        0x8B,0x44,0x24,0x18,
        0xFF,0xD0,
        0xC3
    };
    static const unsigned char T2[] = {
        0x8B,0x4C,0x24,0x04,
        0xFF,0x74,0x24,0x10,
        0xFF,0x74,0x24,0x10,
        0x8B,0x44,0x24,0x10,
        0xFF,0xD0,
        0xC3
    };
    g_t0 = (FnT0)emit_thunk(T0, sizeof(T0));
    g_t1 = (FnT1)emit_thunk(T1, sizeof(T1));
    g_t4 = (FnT4)emit_thunk(T4, sizeof(T4));
    g_t2 = (FnT2)emit_thunk(T2, sizeof(T2));
}

int main(void) {
    printf("Start\n");
    init_thunks();
    printf("Thunks: %p %p %p %p\n", (void*)g_t0, (void*)g_t1, (void*)g_t4, (void*)g_t2);

    SetDllDirectoryA("undecomp\\Photo Gallery");
    HMODULE hMod = LoadLibraryA("WLXPhotoBase.dll");
    if (!hMod) { printf("FAILED: %lu\n", GetLastError()); return 1; }
    printf("Loaded at %p\n\n", (void*)hMod);

    FARPROC fn[56];
    const char* names[] = {
        "??0Exception@Base@@IAE@J@Z", "??0Exception@Base@@QAE@ABV01@@Z",
        "??0OutOfMemoryException@Base@@IAE@XZ", "??0OutOfMemoryException@Base@@QAE@ABV01@@Z",
        "??0Version@Base@@QAE@ABV01@@Z", "??0Version@Base@@QAE@XZ",
        "??1Exception@Base@@UAE@XZ", "??1OutOfMemoryException@Base@@UAE@XZ",
        "??4Exception@Base@@QAEAAV01@ABV01@@Z", "??4OutOfMemoryException@Base@@QAEAAV01@ABV01@@Z",
        "??4Version@Base@@QAEAAV01@ABV01@@Z", "??BException@Base@@QBEJXZ",
        "??MVersion@Base@@QBE_NABV01@@Z", "??OVersion@Base@@QBE_NABV01@@Z",
        "??_7Exception@Base@@6B@", "??_7OutOfMemoryException@Base@@6B@",
        "?AsString@Version@Base@@QBEXPAVString@2@@Z",
        "?BaseAtlThrow@ATL@@YGXJ@Z", "?BaseAtlThrowLastError@ATL@@YGXXZ",
        "?DecrementNoAssertCount@Base@@YGXXZ", "?Delete@BasePrivate@@YAXPAX@Z",
        "?DisableShipAsserts@Base@@YGXXZ", "?EnableLeakTrackingAndSetSymbolPath@Base@@YGX_N@Z",
        "?EnableShipAsserts@Base@@YGXPAX0PAUModuleVersion@1@@Z",
        "?GdiplusStatusToHresult@Base@@YGJH@Z",
        "?GetAssertCallback@Base@@YGAAP6G_NPBDH0@ZXZ",
        "?GetBaseStringComManager@StringCom@Base@@SGAAVCAtlStringMgr@ATL@@XZ",
        "?GetBaseStringManager@String@Base@@SGAAVCAtlStringMgr@ATL@@XZ",
        "?GetModuleAddresses@Base@@YGJPAXPAUModuleAddresses@1@@Z",
        "?GetModuleVersion@Base@@YGJPAUHINSTANCE__@@PAUModuleVersion@1@@Z",
        "?GetProcessorCaps@CPU@Base@@YGXAATCPUCaps@12@@Z",
        "?GetProcessorCount@CPU@Base@@YGHXZ",
        "?GetReportMetrics@Base@@YGXPAUReportMetrics@1@@Z",
        "?GetReportsForSqm@Base@@YGXPAUReportsForSqm@1@@Z",
        "?GetReportsForWer@Base@@YGXPAUReportsForWer@1@@Z",
        "?IncrementNoAssertCount@Base@@YGXXZ",
        "?Init@Exception@Base@@AAEXJ@Z", "?Invalidate@Version@Base@@QAEXXZ",
        "?IsOutOfMemoryError@Base@@YG_NJ@Z", "?IsValid@Version@Base@@QBE_NXZ",
        "?IsVistaOrGreater@OS@Base@@YG_NXZ", "?IsWin7OrGreater@OS@Base@@YG_NXZ",
        "?IsWin8OrGreater@OS@Base@@YG_NXZ", "?New@BasePrivate@@YAPAXI_N@Z",
        "?NoAssertCount@Base@@YGHXZ",
        "?ReportError@BasePrivate@@YA_NPBDHPB_W0PAD0KH0@Z",
        "?ReportFault@Base@@YGXJ@Z",
        "?Set@Version@Base@@QAEXABUtagVS_FIXEDFILEINFO@@@Z",
        "?Set@Version@Base@@QAEXGGGG@Z", "?Set@Version@Base@@QAEXKK@Z",
        "?SetReportsForWer@Base@@YGXPAUReportsForWer@1@@Z",
        "?ThreadID@Exception@Base@@QBEKXZ",
        "?Throw@Base@@YGXJ@Z", "?ThrowLastError@Base@@YGXXZ",
        "?VerifyPtr@BasePrivate@@YA_NPBX@Z",
        "?s_nAssertsInhibited@AssertInhibitor@BasePrivate@@2HA"
    };

    int ok = 0;
    printf("=== Export Resolution (56) ===\n");
    for (int i = 0; i < 56; i++) {
        fn[i] = GetProcAddress(hMod, names[i]);
        if (fn[i]) ok++;
        else printf("  MISSING [%d]: %s\n", i, names[i]);
    }
    printf("Resolved: %d/56\n\n", ok);

    FARPROC pVerCtor = fn[5];
    FARPROC pVerValid = fn[39];
    FARPROC pVerSet4 = fn[48];
    FARPROC pVerInv = fn[37];

    printf("--- Version ---\n");
    VersionObj v1 = {0};
    if(pVerCtor) { g_t0(&v1, pVerCtor); printf("  ctor done\n"); }
    if(pVerValid) printf("  IsValid(default): %s\n", g_t0(&v1, pVerValid)?"true":"false");
    if(pVerSet4) { g_t4(&v1, pVerSet4, 16, 4, 3528, 331); printf("  Set done\n"); }
    if(pVerValid) printf("  IsValid(after Set): %s\n", g_t0(&v1, pVerValid)?"true":"false");
    if(pVerInv) { g_t0(&v1, pVerInv); printf("  Invalidate done\n"); }
    if(pVerValid) printf("  IsValid(inval): %s\n", g_t0(&v1, pVerValid)?"true":"false");

    FreeLibrary(hMod);
    printf("\nDone\n");
    return 0;
}
