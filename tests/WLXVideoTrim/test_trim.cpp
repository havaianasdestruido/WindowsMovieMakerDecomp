#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>

typedef HRESULT (WINAPI *CreateAVICopierDirect_t)(const wchar_t*, bool, long*);
typedef HRESULT (WINAPI *CreateVideoCopierFromMediaType_t)(const wchar_t*, void*, bool, long*);
typedef HRESULT (WINAPI *CreateVideoFormatContextTranscoder_t)(void*, const wchar_t*, bool, long*);
typedef HRESULT (WINAPI *CreateVideoPlayer_t)(const wchar_t*, HWND, int, bool, long*);
typedef HRESULT (WINAPI *CreateVideoWMVTranscoder_t)(void*, const wchar_t*, bool, long*);

static void LogResult(const char* func, HRESULT hr) {
    printf("[%-45s] HRESULT = 0x%08X", func, hr);
    if (hr == 0) printf(" (S_OK)\n");
    else if (hr == 1) printf(" (S_FALSE)\n");
    else if ((int)hr < 0) printf(" (FAILURE)\n");
    else printf("\n");
}

int main(void) {
    printf("=== WLXVideoTrim.dll Export Probing Harness ===\n\n");

    HMODULE hDll = LoadLibraryW(L"WLXVideoTrim.dll");
    if (!hDll) {
        printf("Failed to load WLXVideoTrim.dll (error %lu)\n", GetLastError());
        return 1;
    }
    printf("WLXVideoTrim.dll loaded at %p\n\n", hDll);

    printf("--- Export addresses ---\n");
    CreateAVICopierDirect_t pCreateAVICopierDirect =
        (CreateAVICopierDirect_t)GetProcAddress(hDll, "CreateAVICopierDirect");
    CreateVideoCopierFromMediaType_t pCreateVideoCopierFromMediaType =
        (CreateVideoCopierFromMediaType_t)GetProcAddress(hDll, "CreateVideoCopierFromMediaType");
    CreateVideoFormatContextTranscoder_t pCreateVideoFormatContextTranscoder =
        (CreateVideoFormatContextTranscoder_t)GetProcAddress(hDll, "CreateVideoFormatContextTranscoder");
    CreateVideoPlayer_t pCreateVideoPlayer =
        (CreateVideoPlayer_t)GetProcAddress(hDll, "CreateVideoPlayer");
    CreateVideoWMVTranscoder_t pCreateVideoWMVTranscoder =
        (CreateVideoWMVTranscoder_t)GetProcAddress(hDll, "CreateVideoWMVTranscoder");

    printf("  CreateAVICopierDirect:            %p\n", pCreateAVICopierDirect);
    printf("  CreateVideoCopierFromMediaType:   %p\n", pCreateVideoCopierFromMediaType);
    printf("  CreateVideoFormatContextTranscoder: %p\n", pCreateVideoFormatContextTranscoder);
    printf("  CreateVideoPlayer:                %p\n", pCreateVideoPlayer);
    printf("  CreateVideoWMVTranscoder:         %p\n", pCreateVideoWMVTranscoder);
    printf("\n");

    CoInitialize(NULL);

    printf("--- CreateAVICopierDirect ---\n");
    long handle = 0;
    if (pCreateAVICopierDirect) {
        LogResult("CreateAVICopierDirect(NULL, false, &h)", pCreateAVICopierDirect(NULL, false, &handle));
        printf("  handle = %ld\n", handle);

        handle = 0;
        LogResult("CreateAVICopierDirect(L\"C:\\\\test.avi\", false, &h)", pCreateAVICopierDirect(L"C:\\test.avi", false, &handle));
        printf("  handle = %ld\n", handle);

        handle = 0;
        LogResult("CreateAVICopierDirect(L\"C:\\\\test.avi\", true, &h)", pCreateAVICopierDirect(L"C:\\test.avi", true, &handle));
        printf("  handle = %ld\n", handle);
    } else {
        printf("  [not found]\n");
    }
    printf("\n");

    printf("--- CreateVideoCopierFromMediaType ---\n");
    if (pCreateVideoCopierFromMediaType) {
        handle = 0;
        LogResult("CreateVideoCopierFromMediaType(NULL, NULL, false, &h)",
            pCreateVideoCopierFromMediaType(NULL, NULL, false, &handle));
        printf("  handle = %ld\n", handle);

        handle = 0;
        LogResult("CreateVideoCopierFromMediaType(L\"C:\\\\test.wmv\", NULL, false, &h)",
            pCreateVideoCopierFromMediaType(L"C:\\test.wmv", NULL, false, &handle));
        printf("  handle = %ld\n", handle);
    } else {
        printf("  [not found]\n");
    }
    printf("\n");

    printf("--- CreateVideoFormatContextTranscoder ---\n");
    if (pCreateVideoFormatContextTranscoder) {
        handle = 0;
        LogResult("CreateVideoFormatContextTranscoder(NULL, NULL, false, &h)",
            pCreateVideoFormatContextTranscoder(NULL, NULL, false, &handle));
        printf("  handle = %ld\n", handle);

        handle = 0;
        LogResult("CreateVideoFormatContextTranscoder(NULL, L\"C:\\\\test.mp4\", false, &h)",
            pCreateVideoFormatContextTranscoder(NULL, L"C:\\test.mp4", false, &handle));
        printf("  handle = %ld\n", handle);
    } else {
        printf("  [not found]\n");
    }
    printf("\n");

    printf("--- CreateVideoPlayer ---\n");
    if (pCreateVideoPlayer) {
        handle = 0;
        LogResult("CreateVideoPlayer(NULL, NULL, 0, false, &h)",
            pCreateVideoPlayer(NULL, NULL, 0, false, &handle));
        printf("  handle = %ld\n", handle);

        handle = 0;
        LogResult("CreateVideoPlayer(L\"C:\\\\test.wmv\", NULL, 0, false, &h)",
            pCreateVideoPlayer(L"C:\\test.wmv", NULL, 0, false, &handle));
        printf("  handle = %ld\n", handle);

        handle = 0;
        LogResult("CreateVideoPlayer(L\"C:\\\\test.wmv\", NULL, 1, false, &h)",
            pCreateVideoPlayer(L"C:\\test.wmv", NULL, 1, false, &handle));
        printf("  handle = %ld\n", handle);
    } else {
        printf("  [not found]\n");
    }
    printf("\n");

    printf("--- CreateVideoWMVTranscoder ---\n");
    if (pCreateVideoWMVTranscoder) {
        handle = 0;
        LogResult("CreateVideoWMVTranscoder(NULL, NULL, false, &h)",
            pCreateVideoWMVTranscoder(NULL, NULL, false, &handle));
        printf("  handle = %ld\n", handle);
    } else {
        printf("  [not found]\n");
    }
    printf("\n");

    printf("--- DLL unload test ---\n");
    printf("Freeing DLL...");
    BOOL freed = FreeLibrary(hDll);
    printf(" result=%s\n", freed ? "OK" : "FAILED");

    CoUninitialize();
    printf("\n=== Done ===\n");
    return 0;
}
