/*
 * WLXCodecHost.cpp
 *
 * Implementation of WLXCodecHost.exe -- the out-of-process codec host for
 * Windows Live Movie Maker 2012.
 *
 * This executable runs as a separate process to host third-party and
 * system codec decoders. By running codecs out-of-process, a codec crash
 * does not take down the main Movie Maker application.
 *
 * Communication with the host process uses COM marshaling (RPC) over
 * named pipes. The main process sends decoding requests and receives
 * decoded frames via shared memory sections.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include <windows.h>
#include <objbase.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <vector>
#include <string>

// ============================================================================
// Forward declarations
// ============================================================================
static HRESULT RunMessageLoop();
static HRESULT ProcessDecodeRequest(LPCWSTR pszFilePath);
static void LogMessage(LPCWSTR pszFormat, ...);

// ============================================================================
// Module state
// ============================================================================
static HINSTANCE g_hInstance    = NULL;
static bool      g_bComInit     = false;
static bool      g_bGdipInit    = false;
static ULONG_PTR g_gdipToken    = 0;
static HANDLE    g_hExitEvent   = NULL;

// ============================================================================
// Main entry point
// ============================================================================
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                    LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    g_hInstance = hInstance;

    // Initialize COM (MTA for codec hosting)
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
        return 1;
    g_bComInit = true;

    // Initialize GDI+ for image decoding
    Gdiplus::GdiplusStartupInput gdipInput;
    hr = Gdiplus::GdiplusStartup(&g_gdipToken, &gdipInput, NULL);
    if (hr != Gdiplus::Ok)
    {
        CoUninitialize();
        return 2;
    }
    g_bGdipInit = true;

    // Create exit event
    g_hExitEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    // Check command line for decode request
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc >= 3 && argv)
    {
        // WLXCodecHost.exe /decode <filepath>
        if (_wcsicmp(argv[1], L"/decode") == 0 || _wcsicmp(argv[1], L"-decode") == 0)
        {
            hr = ProcessDecodeRequest(argv[2]);
        }
        else
        {
            // Unknown command -- enter message loop for COM activation
            hr = RunMessageLoop();
        }

        LocalFree(argv);
    }
    else
    {
        // No command-line args -- enter message loop for COM activation
        hr = RunMessageLoop();
    }

    // Cleanup
    if (g_hExitEvent)
    {
        CloseHandle(g_hExitEvent);
        g_hExitEvent = NULL;
    }

    if (g_bGdipInit)
    {
        Gdiplus::GdiplusShutdown(g_gdipToken);
        g_gdipToken = 0;
        g_bGdipInit = false;
    }

    if (g_bComInit)
    {
        CoUninitialize();
        g_bComInit = false;
    }

    return SUCCEEDED(hr) ? 0 : 3;
}

// ============================================================================
// RunMessageLoop -- COM-based message pump for out-of-process activation
// ============================================================================
static HRESULT RunMessageLoop()
{
    // In the full implementation, this registers a class factory
    // and pumps messages waiting for COM activation requests from
    // the parent Movie Maker process. Each decode request is
    // marshaled via RPC.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_QUIT)
            break;
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // Check for exit signal
        if (WaitForSingleObject(g_hExitEvent, 0) == WAIT_OBJECT_0)
            break;
    }

    return S_OK;
}

// ============================================================================
// ProcessDecodeRequest -- decode a single file
// ============================================================================
static HRESULT ProcessDecodeRequest(LPCWSTR pszFilePath)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    // Verify the file exists
    DWORD dwAttr = GetFileAttributesW(pszFilePath);
    if (dwAttr == INVALID_FILE_ATTRIBUTES)
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    // Load and decode the image using GDI+
    Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(pszFilePath);
    if (!pBitmap)
        return E_OUTOFMEMORY;

    Gdiplus::Status status = pBitmap->GetLastStatus();
    if (status != Gdiplus::Ok)
    {
        delete pBitmap;
        return E_FAIL;
    }

    // In the full implementation, the decoded bitmap would be serialized
    // and sent back to the parent process via shared memory or RPC.
    // For this skeleton, we verify the decode succeeded.

    UINT uWidth = pBitmap->GetWidth();
    UINT uHeight = pBitmap->GetHeight();

    delete pBitmap;

    UNREFERENCED_PARAMETER(uWidth);
    UNREFERENCED_PARAMETER(uHeight);

    return S_OK;
}

// ============================================================================
// LogMessage -- debug output
// ============================================================================
static void LogMessage(LPCWSTR pszFormat, ...)
{
    WCHAR szBuffer[1024] = { 0 };
    va_list args;
    va_start(args, pszFormat);
    StringCchVPrintfW(szBuffer, ARRAYSIZE(szBuffer), pszFormat, args);
    va_end(args);

    OutputDebugStringW(szBuffer);
    OutputDebugStringW(L"\n");
}
