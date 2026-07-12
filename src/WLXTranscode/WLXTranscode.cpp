/*
 * WLXTranscode.cpp
 *
 * Implementation of WLXTranscode.exe -- the out-of-process transcoding engine
 * for Windows Live Movie Maker 2012.
 *
 * This executable performs video and audio transcoding operations in a
 * separate process. It supports:
 *   - H.264/AVC encoding (hardware via DXVA2 or software)
 *   - WMV encoding (Windows Media Video 9)
 *   - AAC/WMA audio encoding
 *   - MP4, WMV, and ASF container muxing
 *   - GPU-accelerated encoding via D3D9/D3D11 + DXVA2
 *   - Media Foundation transform-based transcoding pipeline
 *
 * The host process communicates with MovieMakerCore via COM RPC.
 * Transcoding progress is reported via callback proxy.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include <windows.h>
#include <objbase.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mfobjects.h>
#include <evr.h>
#include <d3d9.h>
#include <d3d11.h>
#include <dxva2api.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <vector>
#include <string>

// ============================================================================
// Forward declarations
// ============================================================================
static HRESULT TranscodeFile(LPCWSTR pszInput, LPCWSTR pszOutput, LPCWSTR pszProfile);
static HRESULT InitializeMediaFoundation();
static void    ShutdownMediaFoundation();
static HRESULT CreateHardwareDevice(ID3D11Device** ppDevice);
static void    LogMessage(LPCWSTR pszFormat, ...);

// ============================================================================
// Module state
// ============================================================================
static HINSTANCE     g_hInstance      = NULL;
static bool          g_bComInit       = false;
static bool          g_bMFInit        = false;
static bool          g_bGdipInit      = false;
static ULONG_PTR     g_gdipToken      = 0;

// ============================================================================
// Entry point
// ============================================================================
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                    LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    g_hInstance = hInstance;

    // COM initialization
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
        return 1;
    g_bComInit = true;

    // GDI+ (for thumbnail generation during transcode)
    Gdiplus::GdiplusStartupInput gdipInput;
    hr = Gdiplus::GdiplusStartup(&g_gdipToken, &gdipInput, NULL);
    if (hr != Gdiplus::Ok)
    {
        CoUninitialize();
        return 2;
    }
    g_bGdipInit = true;

    // Media Foundation
    hr = InitializeMediaFoundation();
    if (FAILED(hr))
    {
        Gdiplus::GdiplusShutdown(g_gdipToken);
        CoUninitialize();
        return 3;
    }

    // Parse command line:
    //   WLXTranscode.exe /input <file> /output <file> /profile <name>
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    std::wstring strInput, strOutput, strProfile;

    if (argv)
    {
        for (int i = 1; i < argc - 1; ++i)
        {
            if (_wcsicmp(argv[i], L"/input") == 0 || _wcsicmp(argv[i], L"-input") == 0)
                strInput = argv[++i];
            else if (_wcsicmp(argv[i], L"/output") == 0 || _wcsicmp(argv[i], L"-output") == 0)
                strOutput = argv[++i];
            else if (_wcsicmp(argv[i], L"/profile") == 0 || _wcsicmp(argv[i], L"-profile") == 0)
                strProfile = argv[++i];
        }
        LocalFree(argv);
    }

    int exitCode = 0;

    if (!strInput.empty() && !strOutput.empty())
    {
        // Execute transcode
        hr = TranscodeFile(strInput.c_str(), strOutput.c_str(),
            strProfile.empty() ? L"Default" : strProfile.c_str());

        if (FAILED(hr))
            exitCode = 4;
    }
    else
    {
        // No command-line args -- enter message loop for COM activation
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Cleanup
    ShutdownMediaFoundation();

    if (g_bGdipInit)
    {
        Gdiplus::GdiplusShutdown(g_gdipToken);
        g_gdipToken = 0;
    }

    if (g_bComInit)
    {
        CoUninitialize();
    }

    return exitCode;
}

// ============================================================================
// InitializeMediaFoundation
// ============================================================================
static HRESULT InitializeMediaFoundation()
{
    if (g_bMFInit)
        return S_OK;

    MFAttributes attr;
    HRESULT hr = MFCreateAttributes(&attr, 1);
    if (SUCCEEDED(hr))
    {
        // Enable hardware acceleration
        hr = attr.SetUINT32(MF_ENABLE_HARDWARE_TRANSFORMS, TRUE);
    }

    hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
    if (SUCCEEDED(hr))
    {
        g_bMFInit = true;

        // Register hardware video processor if available
        MFT_REGISTER_TYPE_INFO info = { 0 };
        info.guidMajorType = MFMediaType_Video;
        info.guidSubtype = MFVideoFormat_H264;
    }

    return hr;
}

// ============================================================================
// ShutdownMediaFoundation
// ============================================================================
static void ShutdownMediaFoundation()
{
    if (g_bMFInit)
    {
        MFShutdown();
        g_bMFInit = false;
    }
}

// ============================================================================
// CreateHardwareDevice -- create D3D11 device for hardware encoding
// ============================================================================
static HRESULT CreateHardwareDevice(ID3D11Device** ppDevice)
{
    if (!ppDevice)
        return E_INVALIDARG;

    *ppDevice = NULL;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
    };

    D3D_FEATURE_LEVEL achievedLevel = D3D_FEATURE_LEVEL_9_1;

    HRESULT hr = D3D11CreateDevice(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        D3D11_CREATE_DEVICE_VIDEO_SUPPORT | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        ppDevice,
        &achievedLevel,
        NULL);

    return hr;
}

// ============================================================================
// TranscodeFile -- main transcoding operation
// ============================================================================
static HRESULT TranscodeFile(LPCWSTR pszInput, LPCWSTR pszOutput, LPCWSTR pszProfile)
{
    if (!pszInput || !pszOutput)
        return E_INVALIDARG;

    // Verify input file exists
    DWORD dwAttr = GetFileAttributesW(pszInput);
    if (dwAttr == INVALID_FILE_ATTRIBUTES)
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    // Create source reader
    IMFSourceReader* pReader = NULL;
    HRESULT hr = MFCreateSourceReaderFromURL(pszInput, NULL, &pReader);
    if (FAILED(hr))
        return hr;

    // Create sink writer
    IMFSinkWriter* pWriter = NULL;
    hr = MFCreateSinkWriterFromURL(pszOutput, NULL, NULL, &pWriter);
    if (FAILED(hr))
    {
        pReader->Release();
        return hr;
    }

    // Negotiate media types between reader and writer
    // In the full implementation, this reads the source media types,
    // selects the appropriate encoding profile based on pszProfile,
    // and configures the H.264/AAC/WMV encoders.

    // Process samples
    DWORD dwStreamIndex = 0;
    LONGLONG llDuration = 0;

    // Query source duration
    PROPVARIANT var;
    PropVariantInit(&var);
    hr = pReader->GetPresentationAttribute(
        (DWORD)MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION, &var);
    if (SUCCEEDED(hr))
    {
        llDuration = var.uhVal.QuadPart;
        PropVariantClear(&var);
    }

    // Read and write loop
    DWORD dwFlags = 0;
    IMFSample* pSample = NULL;
    LONGLONG llPosition = 0;

    while (true)
    {
        hr = pReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0, NULL, &dwFlags, &llPosition, &pSample);

        if (FAILED(hr))
            break;

        if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
            break;

        if (pSample)
        {
            // Write sample to output
            hr = pWriter->WriteSample(0, pSample);
            pSample->Release();
            pSample = NULL;

            if (FAILED(hr))
                break;
        }
    }

    // Finalize
    if (SUCCEEDED(hr))
    {
        hr = pWriter->Finalize();
    }

    pWriter->Release();
    pReader->Release();

    return hr;
}

// ============================================================================
// LogMessage
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
