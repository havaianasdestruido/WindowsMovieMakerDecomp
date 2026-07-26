#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <exception>

static const wchar_t* DLL_PATHS[] = {
    L"undecomp\\Photo Gallery\\WLXMediaPublishSubscribe.dll",
    L"WLXMediaPublishSubscribe.dll",
    L"C:\\Users\\mcmco\\Desktop\\WMMR\\undecomp\\Photo Gallery\\WLXMediaPublishSubscribe.dll",
};

static FILE* g_log = nullptr;

static void log(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buf[2048];
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
    va_end(ap);
    printf("%s\n", buf);
    if (g_log) { fprintf(g_log, "%s\n", buf); fflush(g_log); }
}

static const char* hr_name(HRESULT hr) {
    switch ((unsigned)hr) {
    case 0x00000000: return "S_OK";
    case 0x00000001: return "S_FALSE";
    case 0x80004001: return "E_NOTIMPL";
    case 0x80004002: return "E_NOINTERFACE";
    case 0x80004003: return "E_POINTER";
    case 0x80004005: return "E_FAIL";
    case 0x80070005: return "E_ACCESSDENIED";
    case 0x8007000E: return "E_OUTOFMEMORY";
    case 0x80070057: return "E_INVALIDARG";
    default: return "OTHER";
    }
}

#define TEST_EXPORT(name, type, call) \
    do { \
        auto fn = reinterpret_cast<type>(GetProcAddress(hMod, name)); \
        if (!fn) { log("  [%s] NOT FOUND", name); break; } \
        log("  [%s] found at %p -- calling...", name, (void*)fn); \
        __try { call; } \
        __except(EXCEPTION_EXECUTE_HANDLER) { \
            log("  [%s] EXCEPTION 0x%08X", name, GetExceptionCode()); \
        } \
    } while(0)

extern "C" {
    typedef HRESULT (__stdcall *pfnDllCanUnloadNow)(void);
    typedef HRESULT (__stdcall *pfnDllGetClassObject)(REFCLSID, REFIID, void**);
    typedef HRESULT (__stdcall *pfnDllRegisterServer)(void);
    typedef HRESULT (__stdcall *pfnDllUnregisterServer)(void);
}

int main() {
    log("=== WLXMediaPublishSubscribe Export Test ===");

    HMODULE hMod = NULL;
    for (int i = 0; i < 3; i++) {
        log("Trying: %S", DLL_PATHS[i]);
        hMod = LoadLibraryW(DLL_PATHS[i]);
        if (hMod) {
            log("  SUCCESS from path index %d", i);
            break;
        }
        log("  Error: %u", GetLastError());
    }
    if (!hMod) {
        log("FAILED to load DLL from all paths");
        return 1;
    }
    log("DLL loaded at %p", (void*)hMod);

    // --- COM infrastructure exports ---
    log("\n--- COM Infrastructure Exports ---");

    TEST_EXPORT("DllCanUnloadNow",
        pfnDllCanUnloadNow,
        { HRESULT r = fn(); log("    => %s (0x%08X)", hr_name(r), r); });

    TEST_EXPORT("DllRegisterServer",
        pfnDllRegisterServer,
        { HRESULT r = fn(); log("    => %s (0x%08X)", hr_name(r), r); });

    TEST_EXPORT("DllUnregisterServer",
        pfnDllUnregisterServer,
        { HRESULT r = fn(); log("    => %s (0x%08X)", hr_name(r), r); });

    // DllGetClassObject with null CLSID
    {
        auto fn = reinterpret_cast<pfnDllGetClassObject>(
            GetProcAddress(hMod, "DllGetClassObject"));
        if (fn) {
            log("  [DllGetClassObject] found -- calling with NULL...");
            __try {
                void* pUnk = nullptr;
                IID iid_unknown = __uuidof(IUnknown);
                CLSID clsid_zero = {};
                HRESULT r = fn(clsid_zero, iid_unknown, &pUnk);
                log("    => %s (0x%08X)", hr_name(r), r);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // --- MediaPublishSubscribeHelper exports ---
    log("\n--- MediaPublishSubscribeHelper Exports ---");

    // Ordinal 1: ??0MediaPublishSubscribeHelper@@QAE@XZ (ctor)
    {
        typedef void* (__thiscall *pfnCtor)(void* thisptr);
        auto fn = reinterpret_cast<pfnCtor>(
            GetProcAddress(hMod, "??0MediaPublishSubscribeHelper@@QAE@XZ"));
        if (fn) {
            log("  [MediaPublishSubscribeHelper::ctor] calling...");
            char obj[256] = {};
            __try {
                void* result = fn(obj);
                log("    => returned %p (this=%p)", result, result);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // Ordinal 11: Initialize(const GUID&)
    {
        typedef void (__thiscall *pfnInit)(void* thisptr, const GUID&);
        auto fn = reinterpret_cast<pfnInit>(
            GetProcAddress(hMod, "?Initialize@MediaPublishSubscribeHelper@@QAEXABU_GUID@@@Z"));
        if (fn) {
            log("  [MediaPublishSubscribeHelper::Initialize(GUID&)] calling with zero GUID...");
            char obj[256] = {};
            GUID zero = {};
            __try {
                // First construct
                auto ctor = reinterpret_cast<void* (__thiscall *)(void*)>(
                    GetProcAddress(hMod, "??0MediaPublishSubscribeHelper@@QAE@XZ"));
                if (ctor) ctor(obj);
                fn(obj, zero);
                log("    => completed (void)");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // Ordinal 9: GetLiveSignInProvider (static)
    {
        typedef HRESULT (__stdcall *pfnGetLive)(void** ppProvider);
        auto fn = reinterpret_cast<pfnGetLive>(
            GetProcAddress(hMod, "?GetLiveSignInProvider@MediaPublishSubscribeHelper@@SGJPAPAUILiveSignInProvider@@@Z"));
        if (fn) {
            log("  [GetLiveSignInProvider] calling...");
            void* pProv = nullptr;
            __try {
                HRESULT r = fn(&pProv);
                log("    => %s (0x%08X) pProvider=%p", hr_name(r), r, pProv);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // Ordinal 10: GetMediaPublishSubscribeProviderManager
    {
        typedef void (__thiscall *pfnGetPM)(void* thisptr, void** ppPM);
        auto fn = reinterpret_cast<pfnGetPM>(
            GetProcAddress(hMod, "?GetMediaPublishSubscribeProviderManager@MediaPublishSubscribeHelper@@QAEXPAPAUIMediaPublishSubscribeProviderManager@@@Z"));
        if (fn) {
            log("  [GetMediaPublishSubscribeProviderManager] calling...");
            char obj[256] = {};
            void* pPM = nullptr;
            __try {
                auto ctor = reinterpret_cast<void* (__thiscall *)(void*)>(
                    GetProcAddress(hMod, "??0MediaPublishSubscribeHelper@@QAE@XZ"));
                if (ctor) ctor(obj);
                fn(obj, &pPM);
                log("    => pManager=%p", pPM);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // Ordinal 14/15: LaunchAuthBrowser (static, 4-param and 5-param)
    {
        typedef void (__stdcall *pfnLaunch4)(const wchar_t*, int, BOOL, void*);
        auto fn4 = reinterpret_cast<pfnLaunch4>(
            GetProcAddress(hMod, "?LaunchAuthBrowser@MediaPublishSubscribeHelper@@SGXPB_WW4SiteIDType@1@_NPAUILiveSignInProvider@@@Z"));
        if (fn4) {
            log("  [LaunchAuthBrowser 4-param] found (not calling -- would open browser)");
        }

        typedef void (__stdcall *pfnLaunch5)(const wchar_t*, int, BOOL, BOOL, void*);
        auto fn5 = reinterpret_cast<pfnLaunch5>(
            GetProcAddress(hMod, "?LaunchAuthBrowser@MediaPublishSubscribeHelper@@SGXPB_WW4SiteIDType@1@_N2PAUILiveSignInProvider@@@Z"));
        if (fn5) {
            log("  [LaunchAuthBrowser 5-param] found (not calling -- would open browser)");
        }
    }

    // Ordinal 17: PublishItems (core method)
    {
        typedef void (__thiscall *pfnPub)(void* thisptr, void* pSched, void* pSink,
            unsigned long cItems, unsigned long* pcPublished, const void* pCookie,
            void* pItemSet, unsigned int flags1, unsigned int flags2);
        auto fn = reinterpret_cast<pfnPub>(
            GetProcAddress(hMod, "?PublishItems@MediaPublishSubscribeHelper@@QAEXPAUIGrinderTaskScheduler@@PAUIGrinderJobEventSink@@KPAKPBU__MIDL___MIDL_itf_mediapublishsubscribe_0000_0000_0008@@PAUIMediaPublishSubscribeItemSet@@II@Z"));
        if (fn) {
            log("  [PublishItems] found at %p (not calling -- requires live objects)", (void*)fn);
        }
    }

    // Ordinal 5: CreateContainerAndPublishItems
    {
        typedef void (__thiscall *pfnCC)(void* thisptr, void* pSched, void* pSink,
            unsigned long cItems, unsigned long* pcPublished, const void* pCookie,
            void* pItemSet);
        auto fn = reinterpret_cast<pfnCC>(
            GetProcAddress(hMod, "?CreateContainerAndPublishItems@MediaPublishSubscribeHelper@@QAEXPAUIGrinderTaskScheduler@@PAUIGrinderJobEventSink@@KPAKPBU__MIDL___MIDL_itf_mediapublishsubscribe_0000_0005_0002@@PAUIMediaPublishSubscribeItemSet@@@Z"));
        if (fn) {
            log("  [CreateContainerAndPublishItems] found at %p (not calling)", (void*)fn);
        }
    }

    // Ordinal 16: PluginPublishItems
    {
        typedef void (__thiscall *pfnPP)(void* thisptr, void* pSched, void* pSink,
            unsigned long cItems, unsigned long* pcPublished, const wchar_t* ws1,
            const wchar_t* ws2, void* pItemSet, void* pUnk, void* pXML);
        auto fn = reinterpret_cast<pfnPP>(
            GetProcAddress(hMod, "?PluginPublishItems@MediaPublishSubscribeHelper@@QAEXPAUIGrinderTaskScheduler@@PAUIGrinderJobEventSink@@KPAKPB_W3PAUIMediaPublishSubscribeItemSet@@PAUIUnknown@@PAUIXMLDOMDocument@@@Z"));
        if (fn) {
            log("  [PluginPublishItems] found at %p (not calling)", (void*)fn);
        }
    }

    // --- VideoTranscoder exports ---
    log("\n--- VideoTranscoder Exports ---");

    // Ordinal 2: ctor
    {
        typedef void* (__thiscall *pfnCtor)(void* thisptr);
        auto fn = reinterpret_cast<pfnCtor>(
            GetProcAddress(hMod, "??0VideoTranscoder@@QAE@XZ"));
        if (fn) {
            log("  [VideoTranscoder::ctor] calling...");
            char obj[512] = {};
            __try {
                void* r = fn(obj);
                log("    => this=%p", r);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // Ordinal 12: Initialize(input, output, flags...)
    {
        typedef HRESULT (__thiscall *pfnInit)(void* thisptr,
            const wchar_t* input, const wchar_t* output,
            unsigned long f1, unsigned long f2, unsigned long f3, unsigned long f4);
        auto fn = reinterpret_cast<pfnInit>(
            GetProcAddress(hMod, "?Initialize@VideoTranscoder@@QAEJPB_W0KKKK@Z"));
        if (fn) {
            log("  [VideoTranscoder::Initialize] calling with NULL paths...");
            char obj[512] = {};
            __try {
                auto ctor = reinterpret_cast<void* (__thiscall *)(void*)>(
                    GetProcAddress(hMod, "??0VideoTranscoder@@QAE@XZ"));
                if (ctor) ctor(obj);
                HRESULT r = fn(obj, L"nonexistent.avi", L"output.wmv", 0, 0, 0, 0);
                log("    => %s (0x%08X)", hr_name(r), r);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // Ordinal 6: GetInputFileAverageBandwidth
    {
        typedef HRESULT (__thiscall *pfnBW)(void* thisptr, unsigned long* pBW);
        auto fn = reinterpret_cast<pfnBW>(
            GetProcAddress(hMod, "?GetInputFileAverageBandwidth@VideoTranscoder@@QAEJPAK@Z"));
        if (fn) {
            log("  [VideoTranscoder::GetInputFileAverageBandwidth] calling...");
            char obj[512] = {};
            unsigned long bw = 0;
            __try {
                auto ctor = reinterpret_cast<void* (__thiscall *)(void*)>(
                    GetProcAddress(hMod, "??0VideoTranscoder@@QAE@XZ"));
                if (ctor) ctor(obj);
                HRESULT r = fn(obj, &bw);
                log("    => %s (0x%08X) bandwidth=%u", hr_name(r), r, bw);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // Ordinal 7: GetInputFileDisplaySize
    {
        typedef HRESULT (__thiscall *pfnSz)(void* thisptr, SIZE* pSize);
        auto fn = reinterpret_cast<pfnSz>(
            GetProcAddress(hMod, "?GetInputFileDisplaySize@VideoTranscoder@@QAEJPAUtagSIZE@@@Z"));
        if (fn) {
            log("  [VideoTranscoder::GetInputFileDisplaySize] calling...");
            char obj[512] = {};
            SIZE sz = {};
            __try {
                auto ctor = reinterpret_cast<void* (__thiscall *)(void*)>(
                    GetProcAddress(hMod, "??0VideoTranscoder@@QAE@XZ"));
                if (ctor) ctor(obj);
                HRESULT r = fn(obj, &sz);
                log("    => %s (0x%08X) size=%ldx%ld", hr_name(r), r, sz.cx, sz.cy);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // Ordinal 8: GetInputFileDuration
    {
        typedef HRESULT (__thiscall *pfnDur)(void* thisptr, unsigned long* pDur);
        auto fn = reinterpret_cast<pfnDur>(
            GetProcAddress(hMod, "?GetInputFileDuration@VideoTranscoder@@QAEJPAK@Z"));
        if (fn) {
            log("  [VideoTranscoder::GetInputFileDuration] calling...");
            char obj[512] = {};
            unsigned long dur = 0;
            __try {
                auto ctor = reinterpret_cast<void* (__thiscall *)(void*)>(
                    GetProcAddress(hMod, "??0VideoTranscoder@@QAE@XZ"));
                if (ctor) ctor(obj);
                HRESULT r = fn(obj, &dur);
                log("    => %s (0x%08X) duration=%u ms", hr_name(r), r, dur);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // Ordinal 18: Transcode
    {
        typedef HRESULT (__thiscall *pfnTrans)(void* thisptr, void* pProgressCB);
        auto fn = reinterpret_cast<pfnTrans>(
            GetProcAddress(hMod, "?Transcode@VideoTranscoder@@QAEJPAUIVideoTranscodeProgressCB@@@Z"));
        if (fn) {
            log("  [VideoTranscoder::Transcode] found at %p (not calling -- needs init)", (void*)fn);
        }
    }

    // --- Utility ---
    log("\n--- Utility Exports ---");

    // Ordinal 13: IsHResultDRMProtected (static)
    {
        typedef BOOL (__stdcall *pfnDRM)(HRESULT hr);
        auto fn = reinterpret_cast<pfnDRM>(
            GetProcAddress(hMod, "?IsHResultDRMProtected@@YG_NJ@Z"));
        if (fn) {
            log("  [IsHResultDRMProtected] calling with E_FAIL...");
            __try {
                BOOL r = fn(E_FAIL);
                log("    => %s", r ? "TRUE (DRM)" : "FALSE (not DRM)");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
            log("  [IsHResultDRMProtected] calling with S_OK...");
            __try {
                BOOL r = fn(S_OK);
                log("    => %s", r ? "TRUE (DRM)" : "FALSE (not DRM)");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
            log("  [IsHResultDRMProtected] calling with 0x800430C1 (DRM HRESULT)...");
            __try {
                BOOL r = fn((HRESULT)0x800430C1);
                log("    => %s", r ? "TRUE (DRM)" : "FALSE (not DRM)");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                log("    EXCEPTION 0x%08X", GetExceptionCode());
            }
        }
    }

    // --- Summary ---
    log("\n--- Export Resolution Summary ---");
    const char* export_names[] = {
        "??0MediaPublishSubscribeHelper@@QAE@XZ",
        "??0VideoTranscoder@@QAE@XZ",
        "??1MediaPublishSubscribeHelper@@UAE@XZ",
        "??1VideoTranscoder@@UAE@XZ",
        "?CreateContainerAndPublishItems@MediaPublishSubscribeHelper@@QAEXPAUIGrinderTaskScheduler@@PAUIGrinderJobEventSink@@KPAKPBU__MIDL___MIDL_itf_mediapublishsubscribe_0000_0005_0002@@PAUIMediaPublishSubscribeItemSet@@@Z",
        "?GetInputFileAverageBandwidth@VideoTranscoder@@QAEJPAK@Z",
        "?GetInputFileDisplaySize@VideoTranscoder@@QAEJPAUtagSIZE@@@Z",
        "?GetInputFileDuration@VideoTranscoder@@QAEJPAK@Z",
        "?GetLiveSignInProvider@MediaPublishSubscribeHelper@@SGJPAPAUILiveSignInProvider@@@Z",
        "?GetMediaPublishSubscribeProviderManager@MediaPublishSubscribeHelper@@QAEXPAPAUIMediaPublishSubscribeProviderManager@@@Z",
        "?Initialize@MediaPublishSubscribeHelper@@QAEXABU_GUID@@@Z",
        "?Initialize@VideoTranscoder@@QAEJPB_W0KKKK@Z",
        "?IsHResultDRMProtected@@YG_NJ@Z",
        "?LaunchAuthBrowser@MediaPublishSubscribeHelper@@SGXPB_WW4SiteIDType@1@_N2PAUILiveSignInProvider@@@Z",
        "?LaunchAuthBrowser@MediaPublishSubscribeHelper@@SGXPB_WW4SiteIDType@1@_NPAUILiveSignInProvider@@@Z",
        "?PluginPublishItems@MediaPublishSubscribeHelper@@QAEXPAUIGrinderTaskScheduler@@PAUIGrinderJobEventSink@@KPAKPB_W3PAUIMediaPublishSubscribeItemSet@@PAUIUnknown@@PAUIXMLDOMDocument@@@Z",
        "?PublishItems@MediaPublishSubscribeHelper@@QAEXPAUIGrinderTaskScheduler@@PAUIGrinderJobEventSink@@KPAKPBU__MIDL___MIDL_itf_mediapublishsubscribe_0000_0000_0008@@PAUIMediaPublishSubscribeItemSet@@II@Z",
        "?Transcode@VideoTranscoder@@QAEJPAUIVideoTranscodeProgressCB@@@Z",
        "DllCanUnloadNow",
        "DllGetClassObject",
        "DllRegisterServer",
        "DllUnregisterServer",
    };
    int resolved = 0;
    for (int i = 0; i < 22; i++) {
        void* p = (void*)GetProcAddress(hMod, export_names[i]);
        if (p) resolved++;
    }
    log("Resolved: %d / 22 exports", resolved);

    FreeLibrary(hMod);
    log("\n=== Test Complete ===");
    return 0;
}
