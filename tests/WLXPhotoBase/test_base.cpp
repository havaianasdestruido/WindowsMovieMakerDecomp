#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct VersionObj { unsigned int d[8]; };
struct TCpuCaps { unsigned int d[16]; };
struct ModuleVersion { unsigned int d[16]; };
struct ModuleAddresses { void* pBase; unsigned int cbSize; };
struct ReportMetrics { unsigned int d[256]; };
struct ReportsForSqm { unsigned int d[256]; };
struct ReportsForWer { unsigned int d[256]; };

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

#define CHECK(p, name) printf("  %-45s %s\n", name, p ? "OK" : "MISSING")

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    fprintf(stderr, "STDERR: entering main\n");
    SetDllDirectoryA("undecomp\\Photo Gallery");
    printf("=== WLXPhotoBase.dll Dynamic Test ===\n\n");

    init_thunks();
    printf("Thunks at %p %p %p %p\n\n", (void*)g_t0, (void*)g_t1, (void*)g_t4, (void*)g_t2);

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

    FARPROC pExCtor=fn[0], pExCopy=fn[1], pOomCtor=fn[2], pOomCopy=fn[3];
    FARPROC pVerCopyCtor=fn[4], pVerCtor=fn[5];
    FARPROC pExDtor=fn[6], pOomDtor=fn[7];
    FARPROC pExAssign=fn[8], pOomAssign=fn[9], pVerAssign=fn[10];
    FARPROC pExHr=fn[11], pVerLt=fn[12], pVerGt=fn[13];
    FARPROC pExVtable=fn[14], pOomVtable=fn[15];
    FARPROC pVerStr=fn[16], pAtlThrow=fn[17], pAtlThrowErr=fn[18];
    FARPROC pDecAssert=fn[19], pDel=fn[20], pDisAssert=fn[21];
    FARPROC pLeakTrack=fn[22], pEnAssert=fn[23], pGdiHr=fn[24];
    FARPROC pGetCb=fn[25], pStrComMgr=fn[26], pStrMgr=fn[27];
    FARPROC pModAddr=fn[28], pModVer=fn[29], pProcCaps=fn[30], pProcCount=fn[31];
    FARPROC pGetMetrics=fn[32], pGetSqm=fn[33], pGetWer=fn[34];
    FARPROC pIncAssert=fn[35], pExInit=fn[36], pVerInv=fn[37];
    FARPROC pIsOom=fn[38], pVerValid=fn[39];
    FARPROC pIsVista=fn[40], pIsWin7=fn[41], pIsWin8=fn[42];
    FARPROC pNew=fn[43], pNoAssert=fn[44], pRepError=fn[45];
    FARPROC pRepFault=fn[46], pVerSetFFI=fn[47], pVerSet4=fn[48], pVerSet2=fn[49];
    FARPROC pSetWer=fn[50], pExTid=fn[51];
    FARPROC pThrow=fn[52], pThrowErr=fn[53], pVerify=fn[54];
    FARPROC pStaticAssert=fn[55];

    printf("--- OS Detection ---\n");
    typedef bool (__stdcall *FnB)();
    if(pIsVista) printf("  IsVistaOrGreater: %s\n", ((FnB)pIsVista)()?"true":"false");
    if(pIsWin7) printf("  IsWin7OrGreater:  %s\n", ((FnB)pIsWin7)()?"true":"false");
    if(pIsWin8) printf("  IsWin8OrGreater:  %s\n", ((FnB)pIsWin8)()?"true":"false");

    printf("\n--- CPU ---\n");
    typedef int (__stdcall *FnI)();
    if(pProcCount) printf("  ProcessorCount: %d\n", ((FnI)pProcCount)());
    TCpuCaps caps = {0};
    if(pProcCaps) {
        typedef void (__stdcall *FnCaps)(TCpuCaps*);
        ((FnCaps)pProcCaps)(&caps);
        printf("  Caps[0..3]: %08X %08X %08X %08X\n", caps.d[0],caps.d[1],caps.d[2],caps.d[3]);
    }

    printf("\n--- Assert ---\n");
    if(pStaticAssert) printf("  s_nAssertsInhibited: %d\n", *(int*)pStaticAssert);
    if(pNoAssert) printf("  NoAssertCount: %d\n", ((FnI)pNoAssert)());
    if(pIncAssert) { ((FnI)pIncAssert)(); printf("  +IncrementNoAssertCount\n"); }
    if(pNoAssert) printf("  NoAssertCount: %d\n", ((FnI)pNoAssert)());
    if(pDecAssert) { ((FnI)pDecAssert)(); printf("  -DecrementNoAssertCount\n"); }
    if(pNoAssert) printf("  NoAssertCount: %d\n", ((FnI)pNoAssert)());

    printf("\n--- IsOutOfMemoryError ---\n");
    if(pIsOom) {
        typedef bool (__stdcall *FnIsOom)(long);
        FnIsOom fnOom = (FnIsOom)pIsOom;
        printf("  S_OK(0):          %s\n", fnOom(0)?"true":"false");
        printf("  E_OUTOFMEMORY:    %s\n", fnOom((long)0x8007000E)?"true":"false");
        printf("  E_FAIL:           %s\n", fnOom((long)0x80004005)?"true":"false");
        printf("  E_ACCESSDENIED:   %s\n", fnOom((long)0x80070005)?"true":"false");
    }

    printf("\n--- Version ---\n");
    {
        VersionObj v1 = {0}, v2 = {0}, v3 = {0};
        if(pVerCtor) { g_t0(&v1, pVerCtor); printf("  Default ctor done\n"); }
        if(pVerValid) printf("  IsValid(default): %s\n", g_t0(&v1, pVerValid)?"true":"false");
        if(pVerSet4) { g_t4(&v1, pVerSet4, 16, 4, 3528, 331); printf("  Set(16,4,3528,331) done\n"); }
        if(pVerValid) printf("  IsValid(after Set4): %s\n", g_t0(&v1, pVerValid)?"true":"false");
        if(pVerSet2) { g_t2(&v1, pVerSet2, 0x00100004, 0x0DD0014B); printf("  Set(2ul) done\n"); }
        if(pVerCtor) g_t0(&v2, pVerCtor);
        if(pVerSet4) g_t4(&v2, pVerSet4, 16, 4, 3528, 331);
        if(pVerLt) printf("  v1<v2: %s\n", g_t1(&v1, pVerLt, &v2)?"true":"false");
        if(pVerGt) printf("  v1>v2: %s\n", g_t1(&v1, pVerGt, &v2)?"true":"false");
        if(pVerCopyCtor) { g_t1(&v3, pVerCopyCtor, &v1); printf("  Copy ctor done\n"); }
        if(pVerValid) printf("  v3 IsValid(copy): %s\n", g_t0(&v3, pVerValid)?"true":"false");
        if(pVerInv) { g_t0(&v3, pVerInv); printf("  Invalidate done\n"); }
        if(pVerValid) printf("  v3 IsValid(inval): %s\n", g_t0(&v3, pVerValid)?"true":"false");
        if(pVerAssign) { g_t1(&v3, pVerAssign, &v1); printf("  operator= done\n"); }
        if(pVerValid) printf("  v3 IsValid(assign): %s\n", g_t0(&v3, pVerValid)?"true":"false");
    }

    printf("\n--- Memory ---\n");
    if(pNew && pDel && pVerify) {
        typedef void* (__stdcall *FnNew)(unsigned int,bool);
        typedef void (__stdcall *FnDel)(void*);
        typedef bool (__stdcall *FnVer)(const void*);
        void* p = ((FnNew)pNew)(64, false);
        printf("  New(64,false): %p\n", p);
        if (p) {
            printf("  Verify(valid): %s\n", ((FnVer)pVerify)(p)?"true":"false");
            ((FnDel)pDel)(p);
            printf("  Deleted\n");
        }
        printf("  Verify(NULL): %s\n", ((FnVer)pVerify)(NULL)?"true":"false");
        printf("  Verify(0xDEADBEEF): %s\n", ((FnVer)pVerify)((void*)0xDEADBEEF)?"true":"false");
    }

    printf("\n--- Vtables ---\n");
    if(pExVtable) {
        printf("  Exception vtable @ %p:\n", pExVtable);
        uintptr_t* vt = (uintptr_t*)pExVtable;
        for(int i=0;i<5;i++) printf("    [%d] = 0x%p\n", i, (void*)vt[i]);
    }
    if(pOomVtable) {
        printf("  OOM vtable @ %p:\n", pOomVtable);
        uintptr_t* vt = (uintptr_t*)pOomVtable;
        for(int i=0;i<5;i++) printf("    [%d] = 0x%p\n", i, (void*)vt[i]);
    }
    if(pExVtable && pOomVtable) {
        uintptr_t* ve = (uintptr_t*)pExVtable;
        uintptr_t* vo = (uintptr_t*)pOomVtable;
        printf("  Shared [0]:%s [1]:%s [2]:%s\n",
            ve[0]==vo[0]?"same":"diff", ve[1]==vo[1]?"same":"diff", ve[2]==vo[2]?"same":"diff");
    }

    printf("\n--- Module Version ---\n");
    {
        ModuleVersion mv = {0};
        ModuleAddresses ma = {0};
        HMODULE hSelf = GetModuleHandleA(NULL);
        typedef long (__stdcall *FnMV)(void*, ModuleVersion*);
        typedef long (__stdcall *FnMA)(void*, ModuleAddresses*);
        if(pModVer) printf("  GetModuleVersion: 0x%08X\n", (unsigned)((FnMV)pModVer)((void*)hSelf, &mv));
        printf("  MV: %08X %08X %08X %08X\n", mv.d[0],mv.d[1],mv.d[2],mv.d[3]);
        if(pModAddr) printf("  GetModuleAddresses: 0x%08X base=%p sz=0x%X\n",
            (unsigned)((FnMA)pModAddr)((void*)hSelf, &ma), ma.pBase, ma.cbSize);
    }

    printf("\n--- Telemetry ---\n");
    {
        ReportMetrics rm = {0};
        typedef void (__stdcall *FnRM)(ReportMetrics*);
        __try {
            if(pGetMetrics) { ((FnRM)pGetMetrics)(&rm); printf("  Metrics: %08X %08X %08X %08X\n",rm.d[0],rm.d[1],rm.d[2],rm.d[3]); }
        } __except(EXCEPTION_EXECUTE_HANDLER) { printf("  Metrics: EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }

        ReportsForSqm sq = {0};
        typedef void (__stdcall *FnSQ)(ReportsForSqm*);
        __try {
            if(pGetSqm) { ((FnSQ)pGetSqm)(&sq); printf("  SQM: %08X %08X %08X %08X\n",sq.d[0],sq.d[1],sq.d[2],sq.d[3]); }
        } __except(EXCEPTION_EXECUTE_HANDLER) { printf("  SQM: EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }

        ReportsForWer* pWerBuf = (ReportsForWer*)VirtualAlloc(NULL, sizeof(ReportsForWer)*2, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        if(pWerBuf) {
            memset(pWerBuf, 0, sizeof(ReportsForWer));
            __try {
                if(pGetWer) { ((void(__stdcall*)(ReportsForWer*))pGetWer)(pWerBuf); printf("  WER: %08X %08X %08X %08X\n",pWerBuf->d[0],pWerBuf->d[1],pWerBuf->d[2],pWerBuf->d[3]); }
            } __except(EXCEPTION_EXECUTE_HANDLER) { printf("  WER: EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }
            VirtualFree(pWerBuf, 0, MEM_RELEASE);
        }
    }

    printf("\n--- SetReportsForWer ---\n");
    {
        ReportsForWer* pW2 = (ReportsForWer*)VirtualAlloc(NULL, sizeof(ReportsForWer)*2, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        if(pW2) {
            memset(pW2, 0, sizeof(ReportsForWer));
            __try {
                if(pSetWer) { ((void(__stdcall*)(ReportsForWer*))pSetWer)(pW2); printf("  SetReportsForWer(NULL-fields): OK\n"); }
            } __except(EXCEPTION_EXECUTE_HANDLER) { printf("  SetReportsForWer: EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }
            VirtualFree(pW2, 0, MEM_RELEASE);
        }
    }

    printf("\n--- GdiplusStatusToHresult ---\n");
    if(pGdiHr) {
        typedef long (__stdcall *FnGI)(int);
        FnGI gi = (FnGI)pGdiHr;
        for(int s=0;s<=10;s++) {
            __try { printf("  Status %2d -> 0x%08X\n", s, (unsigned)gi(s)); }
            __except(EXCEPTION_EXECUTE_HANDLER) { printf("  Status %2d -> EXCEPTION\n", s); }
        }
    }

    printf("\n--- Base::Throw (SEH) ---\n");
    if(pThrow) {
        typedef void (__stdcall *FnThrow)(long);
        printf("  Throw(E_FAIL)...\n");
        __try { ((FnThrow)pThrow)((long)0x80004005); printf("  -> No exception!\n"); }
        __except(EXCEPTION_EXECUTE_HANDLER) { printf("  -> Caught SEH 0x%08X\n", (unsigned)GetExceptionCode()); }
    }
    if(pThrowErr) {
        printf("  ThrowLastError(ERROR_FILE_NOT_FOUND)...\n");
        SetLastError(ERROR_FILE_NOT_FOUND);
        __try { ((void(__stdcall*)())pThrowErr)(); printf("  -> No exception!\n"); }
        __except(EXCEPTION_EXECUTE_HANDLER) { printf("  -> Caught SEH 0x%08X\n", (unsigned)GetExceptionCode()); }
    }

    printf("\n--- Exception Lifecycle ---\n");
    {
        void* ex1 = malloc(64); memset(ex1,0,64);
        if(pExCtor) { g_t1(ex1, pExCtor, (void*)(unsigned)0x80004005); printf("  ctor(E_FAIL)\n"); }
        if(pExTid) printf("  ThreadID: %lu (self: %lu)\n", g_t0(ex1, pExTid), GetCurrentThreadId());
        if(pExHr) printf("  HR: 0x%08X\n", (unsigned)g_t0(ex1, pExHr));
        void* ex2 = malloc(64); memset(ex2,0,64);
        if(pExCopy) { g_t1(ex2, pExCopy, ex1); printf("  Copy ctor\n"); }
        if(pExHr) printf("  ex2 HR: 0x%08X\n", (unsigned)g_t0(ex2, pExHr));
        void* ex3 = malloc(64); memset(ex3,0,64);
        if(pExCtor) g_t1(ex3, pExCtor, (void*)0);
        if(pExHr) printf("  ex3 pre: 0x%08X\n", (unsigned)g_t0(ex3, pExHr));
        if(pExAssign) g_t1(ex3, pExAssign, ex1);
        if(pExHr) printf("  ex3 post: 0x%08X\n", (unsigned)g_t0(ex3, pExHr));
        if(pExDtor) { g_t0(ex1, pExDtor); g_t0(ex2, pExDtor); g_t0(ex3, pExDtor); }
        free(ex1); free(ex2); free(ex3);
        printf("  Cleaned up\n");
    }

    printf("\n--- OOM Lifecycle ---\n");
    {
        void* o1=malloc(64); memset(o1,0,64);
        if(pOomCtor) { g_t0(o1, pOomCtor); printf("  OOM ctor\n"); }
        void* o2=malloc(64); memset(o2,0,64);
        if(pOomCopy) { g_t1(o2, pOomCopy, o1); printf("  OOM copy\n"); }
        if(pOomDtor) { g_t0(o1, pOomDtor); g_t0(o2, pOomDtor); }
        free(o1); free(o2);
        printf("  Cleaned up\n");
    }

    printf("\n--- Remaining Function Pointers ---\n");
    CHECK(pRepFault, "ReportFault");
    CHECK(pRepError, "ReportError");
    CHECK(pLeakTrack, "EnableLeakTrackingAndSetSymbolPath");
    CHECK(pEnAssert, "EnableShipAsserts");
    CHECK(pDisAssert, "DisableShipAsserts");
    CHECK(pAtlThrow, "BaseAtlThrow");
    CHECK(pAtlThrowErr, "BaseAtlThrowLastError");
    CHECK(pStrMgr, "GetBaseStringManager");
    CHECK(pStrComMgr, "GetBaseStringComManager");
    CHECK(pExInit, "Exception::Init (private)");
    CHECK(pVerSetFFI, "Version::Set(VS_FIXEDFILEINFO)");
    CHECK(pVerStr, "Version::AsString");
    CHECK(pSetWer, "SetReportsForWer");

    printf("\n--- ReportFault (SEH) ---\n");
    if(pRepFault) {
        typedef void (__stdcall *FnRF)(long);
        __try { ((FnRF)pRepFault)((long)0x80004005); printf("  ReportFault(E_FAIL): completed\n"); }
        __except(EXCEPTION_EXECUTE_HANDLER) { printf("  ReportFault: EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }
    }

    printf("\n--- EnableShipAsserts / DisableShipAsserts ---\n");
    if(pEnAssert) {
        typedef void (__stdcall *FnEA)(void*, void*, void*);
        __try { ((FnEA)pEnAssert)(NULL, NULL, NULL); printf("  EnableShipAsserts(NULL,NULL,NULL): OK\n"); }
        __except(EXCEPTION_EXECUTE_HANDLER) { printf("  EnableShipAsserts: EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }
    }
    if(pDisAssert) {
        __try { ((void(__stdcall*)())pDisAssert)(); printf("  DisableShipAsserts: OK\n"); }
        __except(EXCEPTION_EXECUTE_HANDLER) { printf("  DisableShipAsserts: EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }
    }

    printf("\n--- EnableLeakTracking ---\n");
    if(pLeakTrack) {
        typedef void (__stdcall *FnLT)(bool);
        __try { ((FnLT)pLeakTrack)(false); printf("  EnableLeakTracking(false): OK\n"); }
        __except(EXCEPTION_EXECUTE_HANDLER) { printf("  EnableLeakTracking: EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }
    }

    printf("\n--- GetAssertCallback ---\n");
    if(pGetCb) {
        typedef void* (__stdcall *FnGC)();
        __try {
            void* cb = ((FnGC)pGetCb)();
            printf("  GetAssertCallback: %p\n", cb);
        } __except(EXCEPTION_EXECUTE_HANDLER) { printf("  GetAssertCallback: EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }
    }

    printf("\n--- BaseAtlThrow (SEH expected) ---\n");
    if(pAtlThrow) {
        typedef void (__stdcall *FnAT)(long);
        __try { ((FnAT)pAtlThrow)((long)0x80004005); printf("  -> No exception!\n"); }
        __except(EXCEPTION_EXECUTE_HANDLER) { printf("  -> Caught SEH 0x%08X (expected)\n", (unsigned)GetExceptionCode()); }
    }

    printf("\n--- Version::AsString (SEH) ---\n");
    if(pVerStr && pVerCtor) {
        VersionObj vs = {0};
        g_t0(&vs, pVerCtor);
        if(pVerSet4) g_t4(&vs, pVerSet4, 16, 4, 3528, 331);
        wchar_t buf[256] = {0};
        typedef void (__thiscall *FnAS)(void*, wchar_t*);
        __try { ((FnAS)pVerStr)(&vs, buf); printf("  AsString: %ls\n", buf); }
        __except(EXCEPTION_EXECUTE_HANDLER) { printf("  AsString: EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }
    }

    printf("\n--- Version::Set(VS_FIXEDFILEINFO) ---\n");
    if(pVerSetFFI && pVerCtor) {
        VS_FIXEDFILEINFO ffi = {0};
        ffi.dwSignature = 0xFEEF04BD;
        ffi.dwStrucVersion = 0x00010001;
        ffi.dwProductVersionMS = 0x00100004;
        ffi.dwProductVersionLS = 0x0DD0014B;
        ffi.dwFileVersionMS = 0x00100004;
        ffi.dwFileVersionLS = 0x0DD0014B;
        VersionObj vf = {0};
        g_t0(&vf, pVerCtor);
        typedef void (__thiscall *FnSFI)(void*, VS_FIXEDFILEINFO*);
        __try { ((FnSFI)pVerSetFFI)(&vf, &ffi); printf("  Set(FFI): done\n"); }
        __except(EXCEPTION_EXECUTE_HANDLER) { printf("  Set(FFI): EXCEPTION 0x%08X\n", (unsigned)GetExceptionCode()); }
    }

    printf("\n--- Exception vtable deeper walk ---\n");
    if(pExVtable) {
        uintptr_t* vt = (uintptr_t*)pExVtable;
        printf("  Exception vtable extent:\n");
        for(int i=0;i<10;i++) printf("    [%d] = 0x%p\n", i, (void*)vt[i]);
    }

    printf("\n--- GetModuleVersion (self-exe) ---\n");
    {
        HMODULE hSelf = GetModuleHandleA(NULL);
        ModuleVersion mv2 = {0};
        ModuleAddresses ma2 = {0};
        typedef long (__stdcall *FnMV)(HMODULE*, ModuleVersion*);
        typedef long (__stdcall *FnMA)(void*, ModuleAddresses*);
        if(pModVer) {
            __try {
                long hr = ((FnMV)pModVer)(&hSelf, &mv2);
                printf("  GetModuleVersion(self): 0x%08X\n", (unsigned)hr);
                printf("  MV: %08X %08X %08X %08X\n", mv2.d[0],mv2.d[1],mv2.d[2],mv2.d[3]);
            } __except(EXCEPTION_EXECUTE_HANDLER) { printf("  GetModuleVersion: EXCEPTION\n"); }
        }
        if(pModAddr) {
            memset(&ma2, 0, sizeof(ma2));
            __try {
                long hr2 = ((FnMA)pModAddr)((void*)hSelf, &ma2);
                printf("  GetModuleAddresses(self): 0x%08X base=%p sz=0x%X\n",
                    (unsigned)hr2, ma2.pBase, ma2.cbSize);
            } __except(EXCEPTION_EXECUTE_HANDLER) { printf("  GetModuleAddresses: EXCEPTION\n"); }
        }
    }

    printf("\n--- GdiplusStatusToHresult additional ---\n");
    if(pGdiHr) {
        typedef long (__stdcall *FnGI)(int);
        FnGI gi = (FnGI)pGdiHr;
        int special[] = {-1, 11, 12, 13, 14, 15, 20, 100};
        for(int i=0;i<8;i++) {
            __try { printf("  Status %3d -> 0x%08X\n", special[i], (unsigned)gi(special[i])); }
            __except(EXCEPTION_EXECUTE_HANDLER) { printf("  Status %3d -> EXCEPTION\n", special[i]); }
        }
    }

    printf("\n=== Test Complete ===\n");
    FreeLibrary(hMod);
    return 0;
}
