#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward-declare types we reference from mangled signatures
struct IObjectDatabase;
struct ISharedBitmap;

// Known decorated export signatures (from mangled names)
typedef HRESULT (__stdcall *FN_CreateDBThumbnailBroker)(const GUID&, const GUID&, void**);
typedef HRESULT (__stdcall *FN_CreateDBThumbnailSupplier)(const GUID&, const GUID&, void**);
typedef HRESULT (__stdcall *FN_EnsureLocationTaskStarted)(IUnknown**);
typedef HRESULT (__stdcall *FN_GetFaceThumbnailCache)(IObjectDatabase*, const GUID&, void**);
typedef HRESULT (__stdcall *FN_PeekDatabase)(const GUID&, bool*);
typedef HRESULT (__stdcall *FN_ScheduleThumbGenForObject)(IUnknown*, unsigned long);
typedef HRESULT (__stdcall *FN_ScheduleThumbGenForRegion)(IUnknown*, unsigned long);
typedef HRESULT (__stdcall *FN_UpdateFaceDetectForObject)(IUnknown*, unsigned long, ISharedBitmap*, bool);

// GetDatabase – guessed signature: (const GUID&, void**)
typedef HRESULT (__stdcall *FN_GetDatabase)(const GUID&, void**);

static int g_pass = 0, g_fail = 0;

#define TEST_RESOLVE(name, addr) \
    do { \
        if ((addr)) { printf("  [OK] %-55s %p\n", name, (void*)(addr)); g_pass++; } \
        else        { printf("  [--] %-55s NOT FOUND\n", name); g_fail++; } \
    } while(0)

#define TEST_CALL(name, expr) \
    do { \
        __try { HRESULT _hr = (expr); \
            printf("  [OK] %-55s 0x%08X\n", name, (unsigned)_hr); g_pass++; } \
        __except(1) { \
            printf("  [EX] %-55s EXCEPTION 0x%08X\n", name, (unsigned)GetExceptionCode()); g_fail++; } \
    } while(0)

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== WLXPhotoLibraryDatabase.dll - 17-Export Test Harness ===\n\n");

    SetDllDirectoryA("undecomp\\Photo Gallery");
    HMODULE hMod = LoadLibraryA("WLXPhotoLibraryDatabase.dll");
    if (!hMod) { printf("FATAL: LoadLibrary error %lu\n", GetLastError()); return 1; }
    printf("Module @ %p\n\n", (void*)hMod);

    static const GUID GUID_NULL = {0};
    bool bOpen = false;

    // ==================================================================
    // 1. Resolve all 17 exports
    // ==================================================================
    printf("--- 1. Resolve all 17 exports ---\n");

    const char* names[17] = {
        "GetDatabase",                                                          //  1 (ord)
        "GetDatabasePropertyStore",                                             //  2
        "EnsureFaceDataTaskStarted",                                            //  3
        "EnsureFaceRecognitionTaskStarted",                                     //  4
        "UpdateFaceRecognitionForObjects",                                      //  5
        "UpdateFaceRecognition",                                                //  6
        "EnsureFaceTileTaskStarted",                                            //  7
        "SetRegionAsDefaultFaceTile",                                           //  8
        "UpdateFaceGrouping",                                                   //  9
        "?CreateDBThumbnailBroker@@YGJABU_GUID@@0PAPAX@Z",                     // 10
        "?CreateDBThumbnailSupplier@@YGJABU_GUID@@0PAPAX@Z",                   // 11
        "?EnsureLocationTaskStarted@@YGJPAPAUIUnknown@@@Z",                    // 12
        "?GetFaceThumbnailCache@@YGJPAVIObjectDatabase@@ABU_GUID@@PAPAX@Z",    // 13
        "?PeekDatabase@@YGJABU_GUID@@PA_N@Z",                                  // 14
        "?ScheduleFaceThumbnailGenerationForObject@@YGJPAUIUnknown@@K@Z",      // 15
        "?ScheduleFaceThumbnailGenerationForRegion@@YGJPAUIUnknown@@K@Z",      // 16
        "?UpdateFaceDetectionForObject@@YGJPAUIUnknown@@KPAUISharedBitmap@@_N@Z", // 17
    };

    FARPROC fp[17];
    for (int i = 0; i < 17; i++) {
        fp[i] = GetProcAddress(hMod, names[i]);
        TEST_RESOLVE(names[i], fp[i]);
    }
    printf("\n");

    // ==================================================================
    // 2. Decorated exports – known signatures from name mangling
    // ==================================================================
    printf("--- 2. Decorated exports (exact known signatures) ---\n");

    if (fp[ 9]) { FN_CreateDBThumbnailBroker fn = (FN_CreateDBThumbnailBroker)fp[ 9];
        void* p = NULL;
        TEST_CALL("CreateDBThumbnailBroker(GUID_NULL, GUID_NULL, &p)", fn(GUID_NULL, GUID_NULL, &p));
    }
    if (fp[10]) { FN_CreateDBThumbnailSupplier fn = (FN_CreateDBThumbnailSupplier)fp[10];
        void* p = NULL;
        TEST_CALL("CreateDBThumbnailSupplier(GUID_NULL, GUID_NULL, &p)", fn(GUID_NULL, GUID_NULL, &p));
    }
    if (fp[11]) { FN_EnsureLocationTaskStarted fn = (FN_EnsureLocationTaskStarted)fp[11];
        IUnknown* pUnk = NULL;
        TEST_CALL("EnsureLocationTaskStarted(&pUnk=null)", fn(&pUnk));
        TEST_CALL("EnsureLocationTaskStarted(NULL)", fn(NULL));
    }
    if (fp[12]) { FN_GetFaceThumbnailCache fn = (FN_GetFaceThumbnailCache)fp[12];
        void* p = NULL;
        TEST_CALL("GetFaceThumbnailCache(NULL, GUID_NULL, &p)", fn(NULL, GUID_NULL, &p));
    }
    if (fp[13]) { FN_PeekDatabase fn = (FN_PeekDatabase)fp[13];
        TEST_CALL("PeekDatabase(GUID_NULL, &bOpen)", fn(GUID_NULL, &bOpen));
        printf("         bOpen = %s\n", bOpen ? "true" : "false");
        TEST_CALL("PeekDatabase(GUID_NULL, NULL)", fn(GUID_NULL, NULL));
    }
    if (fp[14]) { FN_ScheduleThumbGenForObject fn = (FN_ScheduleThumbGenForObject)fp[14];
        TEST_CALL("ScheduleFaceThumbGenForObject(NULL, 0)", fn(NULL, 0));
        TEST_CALL("ScheduleFaceThumbGenForObject(NULL, 0xFFFFFFFF)", fn(NULL, 0xFFFFFFFF));
    }
    if (fp[15]) { FN_ScheduleThumbGenForRegion fn = (FN_ScheduleThumbGenForRegion)fp[15];
        TEST_CALL("ScheduleFaceThumbGenForRegion(NULL, 0)", fn(NULL, 0));
    }
    if (fp[16]) { FN_UpdateFaceDetectForObject fn = (FN_UpdateFaceDetectForObject)fp[16];
        TEST_CALL("UpdateFaceDetectForObject(NULL, 0, NULL, false)", fn(NULL, 0, NULL, false));
        TEST_CALL("UpdateFaceDetectForObject(NULL, 0, NULL, true)", fn(NULL, 0, NULL, true));
    }
    printf("\n");

    // ==================================================================
    // 3. Undecorated exports – only call where signature is confidently
    //    guessed (GetDatabase).  Others resolved only — calling a
    //    __stdcall function with a wrong param count corrupts the stack
    //    and cannot be safely SEH-recovered.
    // ==================================================================
    printf("--- 3. Undecorated exports (probed / skipped) ---\n");

    // GetDatabase – at 0x1CD55, just 88 bytes before PeekDatabase (0x1CDAD)
    // signature: almost certainly (const GUID&, void**)
    if (fp[0]) { FN_GetDatabase fn = (FN_GetDatabase)fp[0];
        void* pDB = NULL;
        TEST_CALL("GetDatabase(GUID_NULL, &pDB)", fn(GUID_NULL, &pDB));
    }

    // Remaining 8 exports — signatures unknown, skip invocation to
    // avoid __stdcall stack corruption.
    printf("  [--] GetDatabasePropertyStore     resolved only (unknown signature)\n");
    printf("  [--] EnsureFaceDataTaskStarted    resolved only (unknown signature)\n");
    printf("  [--] EnsureFaceRecognitionTaskStarted resolved only (unknown sig)\n");
    printf("  [--] UpdateFaceRecognitionForObjects resolved only (unknown sig)\n");
    printf("  [--] UpdateFaceRecognition        resolved only (unknown signature)\n");
    printf("  [--] EnsureFaceTileTaskStarted    resolved only (unknown signature)\n");
    printf("  [--] SetRegionAsDefaultFaceTile   resolved only (unknown signature)\n");
    printf("  [--] UpdateFaceGrouping           resolved only (unknown signature)\n");
    printf("\n");

    // ==================================================================
    // 4. Edge cases
    // ==================================================================
    printf("--- 4. Edge cases ---\n");

    FARPROC fpBad = GetProcAddress(NULL, "GetDatabase");
    printf("  [--] GetProcAddress(NULL-mod, \"GetDatabase\") -> %p\n", (void*)fpBad);

    FARPROC fpBogus = GetProcAddress(hMod, "NoSuchExport");
    printf("  [--] GetProcAddress(bogus name) -> %p (expect 0)\n", (void*)fpBogus);

    FARPROC fpOrd1 = GetProcAddress(hMod, (LPCSTR)1);
    printf("  [--] GetProcAddress(ordinal=1)   -> %p\n", (void*)fpOrd1);

    FARPROC fpOrd17 = GetProcAddress(hMod, (LPCSTR)17);
    printf("  [--] GetProcAddress(ordinal=17)  -> %p\n", (void*)fpOrd17);

    FARPROC fpOrd999 = GetProcAddress(hMod, (LPCSTR)999);
    printf("  [--] GetProcAddress(ordinal=999) -> %p (expect 0)\n", (void*)fpOrd999);

    BOOL bFree = FreeLibrary(hMod);
    printf("  [--] FreeLibrary -> %s\n", bFree ? "TRUE" : "FALSE");

    printf("\n");
    printf("=== Results: %d pass, %d fail, %d total ===\n",
           g_pass, g_fail, g_pass + g_fail);
    return g_fail;
}
