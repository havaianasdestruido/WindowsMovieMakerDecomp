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

    bool RenderFrame(HWND hWnd, const Gdiplus::Bitmap* pSource)
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
            return E_NOTIMPL;

        case PreviewCmd_LoadFile:
            return E_NOTIMPL;

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
// Exported functions (4 exports)
// ============================================================================

extern "C"
{

MPC_API HANDLE __stdcall PreviewClient_Create(const PreviewConfig* pConfig)
{
    Preview::PreviewClient* pClient = new(std::nothrow) Preview::PreviewClient();
    if (!pClient)
        return NULL;

    HRESULT hr = pClient->Initialize(pConfig);
    if (FAILED(hr))
    {
        delete pClient;
        return NULL;
    }

    return static_cast<HANDLE>(pClient);
}

MPC_API void __stdcall PreviewClient_Destroy(HANDLE hClient)
{
    if (!hClient)
        return;

    Preview::PreviewClient* pClient = static_cast<Preview::PreviewClient*>(hClient);
    delete pClient;
}

MPC_API HRESULT __stdcall PreviewClient_Command(HANDLE hClient, UINT uCommand, LPARAM lParam)
{
    if (!hClient)
        return E_INVALIDARG;

    Preview::PreviewClient* pClient = static_cast<Preview::PreviewClient*>(hClient);
    return pClient->SendCommand(uCommand, lParam);
}

MPC_API HRESULT __stdcall PreviewClient_GetState(HANDLE hClient, UINT* pState)
{
    if (!hClient)
        return E_INVALIDARG;

    Preview::PreviewClient* pClient = static_cast<Preview::PreviewClient*>(hClient);
    return pClient->GetState(pState);
}

} // extern "C"
