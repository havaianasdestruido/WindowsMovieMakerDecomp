/*
 * dllmain.cpp
 *
 * DLL entry point and global initialization/shutdown for MovieMakerCore.dll.
 * Manages the lifetime of all major subsystems:
 *   - COM (CoInitializeEx)
 *   - ATL CComModule
 *   - GDI+
 *   - Media Foundation (MFStartup)
 *   - UXCore (UXCoreInitProcess / UXCoreInitThread)
 *   - DirectUI
 *   - Direct3D 11 (D3D11CreateDevice)
 *   - Direct2D
 *   - DirectWrite
 *   - WIC (ImagingFactory)
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "ComFactory.h"

// ============================================================================
// UXCore forward declarations (delay-loaded or private API)
// ============================================================================
// UXCore is a Windows Live shared component for UI rendering.
// These functions are resolved at runtime from uxcore.dll.
extern "C"
{
    typedef HRESULT(STDAPICALLTYPE* PFN_UXCoreInitProcess)(void);
    typedef HRESULT(STDAPICALLTYPE* PFN_UXCoreInitThread)(void);
    typedef HRESULT(STDAPICALLTYPE* PFN_UXCoreUninitThread)(void);
    typedef HRESULT(STDAPICALLTYPE* PFN_UXCoreUninitProcess)(void);
}

// ============================================================================
// DirectUI forward declarations
// ============================================================================
// DirectUI is a lightweight windowless UI framework used by Windows Live apps.
extern "C"
{
    typedef HRESULT(STDAPICALLTYPE* PFN_DirectUIInit)(void);
    typedef HRESULT(STDAPICALLTYPE* PFN_DirectUIUninit)(void);
}

// ============================================================================
// Module-level state
// ============================================================================
static HINSTANCE g_hInstance        = NULL;
static HMODULE   g_hUxCore          = NULL;
static HMODULE   g_hDirectUI        = NULL;
static bool      g_bComInitialized  = false;
static bool      g_bMfInitialized   = false;
static bool      g_bGdiPlusInit     = false;
static bool      g_bUxCoreInit      = false;
static bool      g_bDirectUIInit    = false;

// GDI+ token
static Gdiplus::GdiplusStartupInput g_gdipStartupInput;
static ULONG_PTR                    g_gdipToken = 0;

// ATL module (global CComModule used by Sundance framework)
CAppModule _Module;

// D3D11 device (shared across render contexts)
static ID3D11Device*           g_pD3D11Device        = NULL;
static ID3D11DeviceContext*    g_pD3D11Context       = NULL;

// WIC factory
static IWICImagingFactory*     g_pWICFactory         = NULL;

// Direct2D factory
static ID2D1Factory*           g_pD2DFactory         = NULL;

// DirectWrite factory
static IDWriteFactory*         g_pDWriteFactory      = NULL;

// ============================================================================
// Forward declarations
// ============================================================================
static BOOL InitializeSubsystems(HINSTANCE hInstance);
static void ShutdownSubsystems(void);

// ============================================================================
// DllMain
// ============================================================================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_hInstance = hModule;

        // Prevent per-thread DllMain calls for performance
        DisableThreadLibraryCalls(hModule);

        // Perform basic initialization here.
        // Full subsystem init is deferred to first call or
        // explicit initialization from MovieMakerMain.
        break;
    }

    case DLL_PROCESS_DETACH:
    {
        // Final cleanup -- ensure all subsystems are torn down.
        // In normal flow, ShutdownSubsystems() is called before
        // the DLL is unloaded, but we act as a safety net here.
        ShutdownSubsystems();
        g_hInstance = NULL;
        break;
    }

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

// ============================================================================
// Internal: Initialize all subsystems
// ============================================================================
static BOOL InitializeSubsystems(HINSTANCE hInstance)
{
    HRESULT hr = S_OK;

    // ------------------------------------------------------------------
    // 1. COM initialization (multithreaded apartment)
    // ------------------------------------------------------------------
    if (!g_bComInitialized)
    {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
        if (FAILED(hr))
        {
            hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            if (FAILED(hr))
                return FALSE;
        }
        g_bComInitialized = true;
    }

    // ------------------------------------------------------------------
    // 2. ATL module initialization (with COM object map)
    // ------------------------------------------------------------------
    {
        _Module.Init(MovieCoreObjectMap, hInstance);

        // Register class factories so objects can be created via CoCreateInstance
        HRESULT hrClassReg = _Module.RegisterClassObjects(
            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
            REGCLS_MULTIPLEUSE);
        // Non-fatal: registration failure logged but does not abort init
        UNREFERENCED_PARAMETER(hrClassReg);
    }

    // ------------------------------------------------------------------
    // 3. GDI+ initialization
    // ------------------------------------------------------------------
    if (!g_bGdiPlusInit)
    {
        Gdiplus::Status status = Gdiplus::GdiplusStartup(
            &g_gdipToken,
            &g_gdipStartupInput,
            NULL);

        if (status != Gdiplus::Ok)
            return FALSE;

        g_bGdiPlusInit = true;
    }

    // ------------------------------------------------------------------
    // 4. Media Foundation initialization
    // ------------------------------------------------------------------
    if (!g_bMfInitialized)
    {
        hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
        if (FAILED(hr))
            return FALSE;

        g_bMfInitialized = true;
    }

    // ------------------------------------------------------------------
    // 5. UXCore -- skipped (not present on this system)
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    // 6. DirectUI -- skipped (not present on this system)
    // ------------------------------------------------------------------

    // ------------------------------------------------------------------
    // 7. Direct3D 11 device creation
    // ------------------------------------------------------------------
    if (!g_pD3D11Device)
    {
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1,
        };

        D3D_FEATURE_LEVEL achievedLevel = D3D_FEATURE_LEVEL_9_1;

        hr = D3D11CreateDevice(
            NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                D3D11_CREATE_DEVICE_SINGLETHREADED,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &g_pD3D11Device,
            &achievedLevel,
            &g_pD3D11Context);

        if (FAILED(hr))
        {
            hr = D3D11CreateDevice(
                NULL,
                D3D_DRIVER_TYPE_WARP,
                NULL,
                D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                    D3D11_CREATE_DEVICE_SINGLETHREADED,
                featureLevels,
                ARRAYSIZE(featureLevels),
                D3D11_SDK_VERSION,
                &g_pD3D11Device,
                &achievedLevel,
                &g_pD3D11Context);
        }
    }

    // ------------------------------------------------------------------
    // 8. Direct2D factory
    // ------------------------------------------------------------------
    if (!g_pD2DFactory)
    {
        hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            __uuidof(ID2D1Factory),
            reinterpret_cast<void**>(&g_pD2DFactory));
    }

    // ------------------------------------------------------------------
    // 9. DirectWrite factory
    // ------------------------------------------------------------------
    if (!g_pDWriteFactory)
    {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&g_pDWriteFactory));
    }

    // ------------------------------------------------------------------
    // 10. WIC (Windows Imaging Component) factory
    // ------------------------------------------------------------------
    if (!g_pWICFactory)
    {
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&g_pWICFactory));
    }

    return TRUE;
}

// ============================================================================
// Internal: Shutdown all subsystems in reverse order
// ============================================================================
static void ShutdownSubsystems(void)
{
    // ------------------------------------------------------------------
    // 10. Release WIC factory
    // ------------------------------------------------------------------
    if (g_pWICFactory)
    {
        g_pWICFactory->Release();
        g_pWICFactory = NULL;
    }

    // ------------------------------------------------------------------
    // 9. Release DirectWrite factory
    // ------------------------------------------------------------------
    if (g_pDWriteFactory)
    {
        g_pDWriteFactory->Release();
        g_pDWriteFactory = NULL;
    }

    // ------------------------------------------------------------------
    // 8. Release Direct2D factory
    // ------------------------------------------------------------------
    if (g_pD2DFactory)
    {
        g_pD2DFactory->Release();
        g_pD2DFactory = NULL;
    }

    // ------------------------------------------------------------------
    // 7. Release Direct3D 11 context and device
    // ------------------------------------------------------------------
    if (g_pD3D11Context)
    {
        g_pD3D11Context->Release();
        g_pD3D11Context = NULL;
    }

    if (g_pD3D11Device)
    {
        g_pD3D11Device->Release();
        g_pD3D11Device = NULL;
    }

    // ------------------------------------------------------------------
    // 6. DirectUI uninit
    // ------------------------------------------------------------------
    if (g_bDirectUIInit && g_hDirectUI)
    {
        PFN_DirectUIUninit pfnUninit =
            reinterpret_cast<PFN_DirectUIUninit>(
                GetProcAddress(g_hDirectUI, "DirectUIUninit"));
        if (pfnUninit)
            pfnUninit();

        FreeLibrary(g_hDirectUI);
        g_hDirectUI = NULL;
        g_bDirectUIInit = false;
    }

    // ------------------------------------------------------------------
    // 5. UXCore uninit
    // ------------------------------------------------------------------
    if (g_bUxCoreInit && g_hUxCore)
    {
        PFN_UXCoreUninitThread pfnUninitThread =
            reinterpret_cast<PFN_UXCoreUninitThread>(
                GetProcAddress(g_hUxCore, "UXCoreUninitThread"));
        if (pfnUninitThread)
            pfnUninitThread();

        PFN_UXCoreUninitProcess pfnUninit =
            reinterpret_cast<PFN_UXCoreUninitProcess>(
                GetProcAddress(g_hUxCore, "UXCoreUninitProcess"));
        if (pfnUninit)
            pfnUninit();

        FreeLibrary(g_hUxCore);
        g_hUxCore = NULL;
        g_bUxCoreInit = false;
    }

    // ------------------------------------------------------------------
    // 4. Media Foundation shutdown
    // ------------------------------------------------------------------
    if (g_bMfInitialized)
    {
        MFShutdown();
        g_bMfInitialized = false;
    }

    // ------------------------------------------------------------------
    // 3. GDI+ shutdown
    // ------------------------------------------------------------------
    if (g_bGdiPlusInit)
    {
        Gdiplus::GdiplusShutdown(g_gdipToken);
        g_gdipToken = 0;
        g_bGdiPlusInit = false;
    }

    // ------------------------------------------------------------------
    // 2. ATL module term (revoke class factories first)
    // ------------------------------------------------------------------
    _Module.RevokeClassObjects();
    _Module.Term();

    // ------------------------------------------------------------------
    // 1. COM uninit
    // ------------------------------------------------------------------
    if (g_bComInitialized)
    {
        CoUninitialize();
        g_bComInitialized = false;
    }
}

// ============================================================================
// Public accessor: Get module instance handle
// ============================================================================
HINSTANCE MovieCore_GetInstance(void)
{
    return g_hInstance;
}

// ============================================================================
// Public accessor: Get D3D11 device (for render components)
// ============================================================================
ID3D11Device* MovieCore_GetD3D11Device(void)
{
    return g_pD3D11Device;
}

ID3D11DeviceContext* MovieCore_GetD3D11Context(void)
{
    return g_pD3D11Context;
}

// ============================================================================
// Public accessor: Get WIC factory
// ============================================================================
IWICImagingFactory* MovieCore_GetWICFactory(void)
{
    return g_pWICFactory;
}

// ============================================================================
// Public accessor: Get D2D factory
// ============================================================================
ID2D1Factory* MovieCore_GetD2DFactory(void)
{
    return g_pD2DFactory;
}

// ============================================================================
// Public accessor: Get DWrite factory
// ============================================================================
IDWriteFactory* MovieCore_GetDWriteFactory(void)
{
    return g_pDWriteFactory;
}

// ============================================================================
// Public accessor: Get ATL module
// ============================================================================
ATL::CComModule& MovieCore_GetModule(void)
{
    return _Module;
}

// ============================================================================
// Expose InitializeSubsystems for MovieMakerMain
// ============================================================================
extern "C" BOOL MovieCore_Initialize(HINSTANCE hInstance)
{
    return InitializeSubsystems(hInstance);
}

extern "C" void MovieCore_Shutdown(void)
{
    ShutdownSubsystems();
}
