/*
 * MovieMakerPreviewClient.cpp
 *
 * Implementation of MovieMakerPreviewClient.dll -- the preview window
 * communication component embedded within Windows Live Movie Maker 2012.
 *
 * Manages the preview pane lifecycle, transport controls, frame rendering,
 * and snapshot capture. Uses GDI+ for software rendering and supports
 * hardware-accelerated paths via shared D3D surfaces.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "MovieMakerPreviewClient.h"
#include "WLXPhotoBase.h"

#include <comdef.h>
#include <vector>
#include <string>
#include <memory>
#include <new>

// ============================================================================
// Internal state structures
// ============================================================================
namespace Preview
{

struct PreviewState
{
    PreviewTransportState   eTransport;
    LONGLONG                llPosition;     // current position in 100ns units
    LONGLONG                llDuration;     // total duration in 100ns units
    LONGLONG                llSeekTarget;   // pending seek target
    double                  dVolume;        // 0.0 to 1.0
    bool                    bFileLoaded;
    bool                    bHasAudio;
    bool                    bHasVideo;
    std::wstring            strFilePath;

    PreviewState()
        : eTransport(PreviewTransportStopped)
        , llPosition(0)
        , llDuration(0)
        , llSeekTarget(0)
        , dVolume(1.0)
        , bFileLoaded(false)
        , bHasAudio(false)
        , bHasVideo(false)
    {
    }
};

class PreviewWindow
{
public:
    PreviewWindow()
        : m_hWnd(NULL)
        , m_hParentWnd(NULL)
        , m_bRegistered(false)
    {
    }

    ~PreviewWindow()
    {
        Destroy();
    }

    bool Create(HWND hParent, const RECT& rcPreview)
    {
        m_hParentWnd = hParent;

        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc = PreviewWndProc;
        wc.hInstance = ::GetModuleHandleW(NULL);
        wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
        wc.lpszClassName = L"WMMPreviewClient";

        if (!m_bRegistered)
        {
            if (::RegisterClassExW(&wc) == 0)
            {
                if (::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
                    return false;
            }
            m_bRegistered = true;
        }

        DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS;

        m_hWnd = ::CreateWindowExW(
            0,
            L"WMMPreviewClient",
            L"",
            dwStyle,
            rcPreview.left, rcPreview.top,
            rcPreview.right - rcPreview.left,
            rcPreview.bottom - rcPreview.top,
            hParent,
            NULL,
            ::GetModuleHandleW(NULL),
            this);

        return m_hWnd != NULL;
    }

    void Destroy()
    {
        if (m_hWnd)
        {
            ::DestroyWindow(m_hWnd);
            m_hWnd = NULL;
        }
    }

    HWND GetHWnd() const { return m_hWnd; }

    void Resize(const RECT& rcNew)
    {
        if (m_hWnd)
        {
            ::MoveWindow(m_hWnd,
                rcNew.left, rcNew.top,
                rcNew.right - rcNew.left,
                rcNew.bottom - rcNew.top,
                TRUE);
        }
    }

    void Invalidate()
    {
        if (m_hWnd)
            ::InvalidateRect(m_hWnd, NULL, FALSE);
    }

private:
    static LRESULT CALLBACK PreviewWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        PreviewWindow* pThis = NULL;

        if (uMsg == WM_CREATE)
        {
            CREATESTRUCTW* pCS = reinterpret_cast<CREATESTRUCTW*>(lParam);
            pThis = static_cast<PreviewWindow*>(pCS->lpCreateParams);
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        }
        else
        {
            pThis = reinterpret_cast<PreviewWindow*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            switch (uMsg)
            {
            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = ::BeginPaint(hWnd, &ps);
                // Black background for preview area
                RECT rc;
                ::GetClientRect(hWnd, &rc);
                ::FillRect(hdc, &rc, static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)));
                ::EndPaint(hWnd, &ps);
                return 0;
            }

            case WM_ERASEBKGND:
                return 1; // Suppress flicker

            case WM_SIZE:
                pThis->Invalidate();
                return 0;
            }
        }

        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    HWND    m_hWnd;
    HWND    m_hParentWnd;
    bool    m_bRegistered;
};

class PreviewRenderer
{
public:
    PreviewRenderer()
        : m_pGdiplusBitmap(NULL)
    {
    }

    ~PreviewRenderer()
    {
        ClearFrame();
    }

    void ClearFrame()
    {
        if (m_pGdiplusBitmap)
        {
            delete m_pGdiplusBitmap;
            m_pGdiplusBitmap = NULL;
        }
    }

    bool RenderFrame(HWND hWnd, Gdiplus::Bitmap* pSource)
    {
        if (!hWnd || !pSource)
            return false;

        HDC hdc = ::GetDC(hWnd);
        if (!hdc)
            return false;

        RECT rc;
        ::GetClientRect(hWnd, &rc);

        Gdiplus::Graphics graphics(hdc);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

        Gdiplus::Status status = graphics.DrawImage(
            pSource,
            0, 0,
            rc.right - rc.left,
            rc.bottom - rc.top);

        ::ReleaseDC(hWnd, hdc);

        return (status == Gdiplus::Ok);
    }

    bool Capture(HWND hWnd)
    {
        if (!hWnd)
            return false;

        ClearFrame();

        RECT rc;
        if (!::GetClientRect(hWnd, &rc))
            return false;

        const int cx = rc.right - rc.left;
        const int cy = rc.bottom - rc.top;
        if (cx <= 0 || cy <= 0)
            return false;

        HDC hdcWindow = ::GetDC(hWnd);
        if (!hdcWindow)
            return false;

        HDC hdcMem = ::CreateCompatibleDC(hdcWindow);
        HBITMAP hbm = ::CreateCompatibleBitmap(hdcWindow, cx, cy);
        if (!hbm)
        {
            ::DeleteDC(hdcMem);
            ::ReleaseDC(hWnd, hdcWindow);
            return false;
        }

        HGDIOBJ hOld = ::SelectObject(hdcMem, hbm);
        ::BitBlt(hdcMem, 0, 0, cx, cy, hdcWindow, 0, 0, SRCCOPY);
        ::SelectObject(hdcMem, hOld);

        m_pGdiplusBitmap = Gdiplus::Bitmap::FromHBITMAP(hbm, NULL);
        if (m_pGdiplusBitmap && m_pGdiplusBitmap->GetLastStatus() != Gdiplus::Ok)
        {
            delete m_pGdiplusBitmap;
            m_pGdiplusBitmap = NULL;
        }

        ::DeleteObject(hbm);
        ::DeleteDC(hdcMem);
        ::ReleaseDC(hWnd, hdcWindow);

        return (m_pGdiplusBitmap != NULL);
    }

private:
    Gdiplus::Bitmap* m_pGdiplusBitmap;
};

} // namespace Preview

// ============================================================================
// PreviewClient -- main client object
// ============================================================================
namespace Preview
{

class PreviewClient
{
public:
    PreviewClient()
        : m_bInitialized(false)
    {
    }

    ~PreviewClient()
    {
        Shutdown();
    }

    HRESULT Initialize(const PreviewConfig* pConfig)
    {
        if (m_bInitialized)
            return E_UNEXPECTED;

        if (!pConfig)
            return E_INVALIDARG;

        m_config = *pConfig;

        // Initialize GDI+ for this instance
        Gdiplus::GdiplusStartupInput gdipInput;
        Gdiplus::Status status = Gdiplus::GdiplusStartup(
            &m_gdipToken, &gdipInput, NULL);
        if (status != Gdiplus::Ok)
            return E_FAIL;

        // Create the preview window
        m_window.reset(new PreviewWindow());
        if (!m_window->Create(pConfig->hParentWnd, pConfig->rcPreview))
        {
            Gdiplus::GdiplusShutdown(m_gdipToken);
            return E_FAIL;
        }

        m_renderer.reset(new PreviewRenderer());

        m_state.reset(new PreviewState());
        m_bInitialized = true;

        return S_OK;
    }

    void Shutdown()
    {
        if (!m_bInitialized)
            return;

        m_renderer.reset();
        m_window.reset();
        m_state.reset();

        Gdiplus::GdiplusShutdown(m_gdipToken);
        m_gdipToken = 0;

        m_bInitialized = false;
    }

    HRESULT SendCommand(UINT uCommand, LPARAM lParam)
    {
        if (!m_bInitialized)
            return E_UNEXPECTED;

        switch (uCommand)
        {
        case PreviewCmd_Play:
            m_state->eTransport = PreviewTransportPlaying;
            return S_OK;

        case PreviewCmd_Pause:
            m_state->eTransport = PreviewTransportPaused;
            return S_OK;

        case PreviewCmd_Stop:
            m_state->eTransport = PreviewTransportStopped;
            m_state->llPosition = 0;
            if (m_window)
                m_window->Invalidate();
            return S_OK;

        case PreviewCmd_Seek:
            m_state->llSeekTarget = static_cast<LONGLONG>(lParam);
            m_state->llPosition = m_state->llSeekTarget;
            return S_OK;

        case PreviewCmd_RenderFrame:
            if (m_window)
                m_window->Invalidate();
            return S_OK;

        case PreviewCmd_Resize:
        {
            const RECT* pRect = reinterpret_cast<const RECT*>(lParam);
            if (pRect && m_window)
                m_window->Resize(*pRect);
            return S_OK;
        }

        case PreviewCmd_Snapshot:
            if (!m_window || !m_renderer)
                return E_UNEXPECTED;
            if (m_renderer->Capture(m_window->GetHWnd()))
                return S_OK;
            return E_FAIL;

        case PreviewCmd_LoadFile:
        {
            const wchar_t* pszPath = reinterpret_cast<const wchar_t*>(lParam);
            if (!pszPath)
                return E_INVALIDARG;
            m_state->strFilePath = pszPath;
            m_state->bFileLoaded = true;
            m_state->eTransport = PreviewTransportStopped;
            m_state->llPosition = 0;
            m_state->llSeekTarget = 0;
            return S_OK;
        }

        case PreviewCmd_CloseFile:
            m_state->bFileLoaded = false;
            m_state->strFilePath.clear();
            return S_OK;

        case PreviewCmd_SetVolume:
            m_state->dVolume = static_cast<double>(lParam) / 10000.0;
            return S_OK;

        case PreviewCmd_GetPosition:
            return S_OK;

        case PreviewCmd_GetDuration:
            return S_OK;

        default:
            return E_INVALIDARG;
        }
    }

    HRESULT GetState(UINT* pState) const
    {
        if (!m_bInitialized)
            return E_UNEXPECTED;

        if (!pState)
            return E_INVALIDARG;

        *pState = static_cast<UINT>(m_state->eTransport);
        return S_OK;
    }

private:
    bool                                        m_bInitialized;
    PreviewConfig                               m_config;
    ULONG_PTR                                   m_gdipToken;
    std::unique_ptr<PreviewWindow>              m_window;
    std::unique_ptr<PreviewRenderer>            m_renderer;
    std::unique_ptr<PreviewState>               m_state;
};

} // namespace Preview

// ============================================================================
// Module state
// ============================================================================
static HINSTANCE g_hModule = NULL;
static volatile LONG g_cObjects = 0;
static volatile LONG g_cLocks = 0;

// CLSID of the preview client component.
// {1BCE3B7B-DB2B-4013-AA4E-610640375A7B}
static const CLSID CLSID_MovieMakerPreviewClient =
{
    0x1BCE3B7B, 0xDB2B, 0x4013,
    { 0xAA, 0x4E, 0x61, 0x06, 0x40, 0x37, 0x5A, 0x7B }
};

// ============================================================================
// Class factory -- creates instances of the preview client COM object
// ============================================================================
class PreviewClientObject : public IUnknown
{
public:
    PreviewClientObject()
        : m_cRef(1)
    {
        ::InterlockedIncrement(&g_cObjects);
    }

    ~PreviewClientObject()
    {
        ::InterlockedDecrement(&g_cObjects);
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject)
    {
        if (!ppvObject)
            return E_POINTER;
        if (riid == IID_IUnknown)
        {
            *ppvObject = static_cast<IUnknown*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return static_cast<ULONG>(::InterlockedIncrement(&m_cRef));
    }

    STDMETHODIMP_(ULONG) Release()
    {
        ULONG cRef = static_cast<ULONG>(::InterlockedDecrement(&m_cRef));
        if (cRef == 0)
            delete this;
        return cRef;
    }

private:
    volatile LONG m_cRef;
};

class PreviewClientFactory : public IClassFactory
{
public:
    PreviewClientFactory()
        : m_cRef(1)
    {
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject)
    {
        if (!ppvObject)
            return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IClassFactory)
        {
            *ppvObject = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return static_cast<ULONG>(::InterlockedIncrement(&m_cRef));
    }

    STDMETHODIMP_(ULONG) Release()
    {
        ULONG cRef = static_cast<ULONG>(::InterlockedDecrement(&m_cRef));
        if (cRef == 0)
            delete this;
        return cRef;
    }

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
    {
        if (pUnkOuter != NULL)
            return CLASS_E_NOAGGREGATION;
        if (!ppvObject)
            return E_POINTER;
        *ppvObject = NULL;

        PreviewClientObject* pObject = new (std::nothrow) PreviewClientObject();
        if (!pObject)
            return E_OUTOFMEMORY;

        HRESULT hr = pObject->QueryInterface(riid, ppvObject);
        pObject->Release();
        return hr;
    }

    STDMETHODIMP LockServer(BOOL fLock)
    {
        if (fLock)
            ::InterlockedIncrement(&g_cLocks);
        else
            ::InterlockedDecrement(&g_cLocks);
        return S_OK;
    }

private:
    volatile LONG m_cRef;
};

// ============================================================================
// DllMain
// ============================================================================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        break;

    case DLL_PROCESS_DETACH:
        g_hModule = NULL;
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

// ============================================================================
// Exported functions -- standard COM DLL entry points
// ============================================================================

extern "C"
{

STDAPI DllCanUnloadNow()
{
    return (g_cObjects == 0 && g_cLocks == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (!ppv)
        return E_POINTER;
    *ppv = NULL;

    if (rclsid != CLSID_MovieMakerPreviewClient)
        return CLASS_E_CLASSNOTAVAILABLE;

    PreviewClientFactory* pFactory = new (std::nothrow) PreviewClientFactory();
    if (!pFactory)
        return E_OUTOFMEMORY;

    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();
    return hr;
}

STDAPI DllRegisterServer()
{
    wchar_t szModule[MAX_PATH];
    DWORD cch = ::GetModuleFileNameW(g_hModule, szModule, MAX_PATH);
    if (cch == 0 || cch >= MAX_PATH)
        return HRESULT_FROM_WIN32(::GetLastError());

    wchar_t szClsid[64];
    ::StringFromGUID2(CLSID_MovieMakerPreviewClient, szClsid, 64);

    wchar_t szKey[160];
    wsprintfW(szKey, L"CLSID\\%s", szClsid);

    HKEY hKeyClsid = NULL;
    LONG lResult = ::RegCreateKeyExW(HKEY_CLASSES_ROOT, szKey, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyClsid, NULL);
    if (lResult != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lResult);

    static const wchar_t szDesc[] = L"Movie Maker Preview Client";
    lResult = ::RegSetValueExW(hKeyClsid, NULL, 0, REG_SZ,
        reinterpret_cast<const BYTE*>(szDesc), (DWORD)sizeof(szDesc));

    HKEY hKeyInproc = NULL;
    if (lResult == ERROR_SUCCESS)
    {
        lResult = ::RegCreateKeyExW(hKeyClsid, L"InprocServer32", 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyInproc, NULL);
    }
    if (lResult == ERROR_SUCCESS)
    {
        lResult = ::RegSetValueExW(hKeyInproc, NULL, 0, REG_SZ,
            reinterpret_cast<const BYTE*>(szModule),
            (DWORD)((wcslen(szModule) + 1) * sizeof(wchar_t)));
    }
    if (lResult == ERROR_SUCCESS)
    {
        static const wchar_t szModel[] = L"Apartment";
        lResult = ::RegSetValueExW(hKeyInproc, L"ThreadingModel", 0, REG_SZ,
            reinterpret_cast<const BYTE*>(szModel), (DWORD)sizeof(szModel));
    }

    if (hKeyInproc) ::RegCloseKey(hKeyInproc);
    if (hKeyClsid)  ::RegCloseKey(hKeyClsid);

    if (lResult != ERROR_SUCCESS)
    {
        DllUnregisterServer();
        return HRESULT_FROM_WIN32(lResult);
    }
    return S_OK;
}

STDAPI DllUnregisterServer()
{
    wchar_t szClsid[64];
    ::StringFromGUID2(CLSID_MovieMakerPreviewClient, szClsid, 64);

    wchar_t szKey[160];
    wsprintfW(szKey, L"CLSID\\%s\\InprocServer32", szClsid);
    LONG lResult = ::RegDeleteKeyW(HKEY_CLASSES_ROOT, szKey);
    if (lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND)
        return HRESULT_FROM_WIN32(lResult);

    wsprintfW(szKey, L"CLSID\\%s", szClsid);
    lResult = ::RegDeleteKeyW(HKEY_CLASSES_ROOT, szKey);
    if (lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND)
        return HRESULT_FROM_WIN32(lResult);

    return S_OK;
}

} // extern "C"
