#include "pch.h"
/*
 * PreviewPresenter.cpp
 *
 * Implementation of the PreviewPresenterWrapper COM object, the PreviewDX
 * abstract rendering surface, and the DefaultPreviewDX D3D11 implementation.
 *
 * The preview presenter manages the real-time rendering loop, extent
 * navigation, seeking, and frame capture for the preview panel.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "PreviewPresenter.h"
#include "PreviewDataContext.h"
#include "../StoryboardManager/MovieProject.h"
#include "../StoryboardManager/MovieExtent.h"
#include "../StoryboardManager/Extents.h"

// ============================================================================
// Constants
// ============================================================================
namespace
{
    const DWORD kRenderThreadSleepMs = 16;  // ~60 fps
    const DWORD kDefaultPreviewWidth  = 640;
    const DWORD kDefaultPreviewHeight = 480;
}

// ============================================================================
// PreviewDX abstract class implementation
// ============================================================================

PreviewDX::PreviewDX()
    : m_fInitialized(false)
    , m_hWnd(nullptr)
    , m_cx(0)
    , m_cy(0)
{
}

PreviewDX::~PreviewDX()
{
}

bool PreviewDX::IsInitialized() const throw()
{
    return m_fInitialized;
}

// ============================================================================
// DefaultPreviewDX implementation
// ============================================================================

DefaultPreviewDX::DefaultPreviewDX()
    : m_pDevice(nullptr)
    , m_pContext(nullptr)
    , m_pSwapChain(nullptr)
    , m_pRenderTarget(nullptr)
{
    ZeroMemory(&m_swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
}

DefaultPreviewDX::~DefaultPreviewDX()
{
    Uninitialize();
}

HRESULT DefaultPreviewDX::Initialize(HWND hWnd, UINT cx, UINT cy)
{
    if (m_fInitialized)
        return E_UNEXPECTED;

    if (!hWnd || cx == 0 || cy == 0)
        return E_INVALIDARG;

    m_hWnd = hWnd;
    m_cx = cx;
    m_cy = cy;

    HRESULT hr = CreateDeviceAndSwapChain();
    if (FAILED(hr))
        return hr;

    hr = CreateRenderTarget();
    if (FAILED(hr))
    {
        ReleaseDeviceAndSwapChain();
        return hr;
    }

    m_fInitialized = true;
    return S_OK;
}

HRESULT DefaultPreviewDX::Uninitialize()
{
    if (!m_fInitialized)
        return S_FALSE;

    ReleaseDeviceAndSwapChain();
    m_fInitialized = false;
    return S_OK;
}

HRESULT DefaultPreviewDX::Resize(UINT cx, UINT cy)
{
    if (!m_fInitialized)
        return E_UNEXPECTED;

    if (cx == 0 || cy == 0)
        return E_INVALIDARG;

    if (m_pContext)
        m_pContext->OMSetRenderTargets(0, nullptr, nullptr);

    if (m_pRenderTarget)
    {
        m_pRenderTarget->Release();
        m_pRenderTarget = nullptr;
    }

    HRESULT hr = m_pSwapChain->ResizeBuffers(1, cx, cy, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
    if (FAILED(hr))
        return hr;

    m_cx = cx;
    m_cy = cy;

    hr = CreateRenderTarget();
    if (FAILED(hr))
        return hr;

    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.Width = static_cast<float>(cx);
    vp.Height = static_cast<float>(cy);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    m_pContext->RSSetViewports(1, &vp);

    return S_OK;
}

HRESULT DefaultPreviewDX::BeginFrame()
{
    if (!m_fInitialized)
        return E_UNEXPECTED;

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    if (m_pContext && m_pRenderTarget)
    {
        m_pContext->ClearRenderTargetView(m_pRenderTarget, clearColor);
    }
    return S_OK;
}

HRESULT DefaultPreviewDX::EndFrame()
{
    return S_OK;
}

HRESULT DefaultPreviewDX::Present()
{
    if (!m_fInitialized || !m_pSwapChain)
        return E_UNEXPECTED;

    return m_pSwapChain->Present(1, 0);
}

HRESULT DefaultPreviewDX::DrawBitmap(Gdiplus::Bitmap* pBitmap)
{
    if (!m_fInitialized)
        return E_UNEXPECTED;
    if (!pBitmap || !m_pDevice || !m_pContext || !m_pSwapChain)
        return E_INVALIDARG;

    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    D3D11_TEXTURE2D_DESC bufferDesc;
    pBackBuffer->GetDesc(&bufferDesc);

    UINT bmpW = pBitmap->GetWidth();
    UINT bmpH = pBitmap->GetHeight();

    Gdiplus::BitmapData bitmapData;
    Gdiplus::Rect rect(0, 0, static_cast<INT>(bmpW), static_cast<INT>(bmpH));

    if (pBitmap->LockBits(&rect, Gdiplus::ImageLockModeRead,
            PixelFormat32bppARGB, &bitmapData) != Gdiplus::Ok)
    {
        pBackBuffer->Release();
        return E_FAIL;
    }

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = bmpW;
    texDesc.Height = bmpH;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_STAGING;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Texture2D* pStaging = nullptr;
    hr = m_pDevice->CreateTexture2D(&texDesc, nullptr, &pStaging);
    if (SUCCEEDED(hr))
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        hr = m_pContext->Map(pStaging, 0, D3D11_MAP_WRITE, 0, &mapped);
        if (SUCCEEDED(hr))
        {
            const BYTE* pSrc = static_cast<const BYTE*>(bitmapData.Scan0);
            BYTE* pDst = static_cast<BYTE*>(mapped.pData);
            UINT srcRowBytes = bmpW * 4;
            UINT copyBytes = (std::min)(srcRowBytes, mapped.RowPitch);

            for (UINT row = 0; row < bmpH; ++row)
            {
                memcpy(pDst + row * mapped.RowPitch,
                       pSrc + row * bitmapData.Stride,
                       copyBytes);
            }

            m_pContext->Unmap(pStaging, 0);

            if (bmpW == bufferDesc.Width && bmpH == bufferDesc.Height)
            {
                m_pContext->CopyResource(pBackBuffer, pStaging);
            }
            else
            {
                D3D11_BOX srcBox = {};
                srcBox.left = 0;
                srcBox.top = 0;
                srcBox.front = 0;
                srcBox.right = (std::min)(bmpW, bufferDesc.Width);
                srcBox.bottom = (std::min)(bmpH, bufferDesc.Height);
                srcBox.back = 1;
                m_pContext->CopySubresourceRegion(pBackBuffer, 0, 0, 0, 0,
                    pStaging, 0, &srcBox);
            }
        }
        pStaging->Release();
    }

    pBitmap->UnlockBits(&bitmapData);
    pBackBuffer->Release();
    return hr;
}

HRESULT DefaultPreviewDX::Clear()
{
    return BeginFrame();
}

ID3D11Device* DefaultPreviewDX::GetD3DDevice() const throw()
{
    return m_pDevice;
}

ID3D11DeviceContext* DefaultPreviewDX::GetD3DDeviceContext() const throw()
{
    return m_pContext;
}

HRESULT DefaultPreviewDX::CreateDeviceAndSwapChain()
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = m_cx;
    sd.BufferDesc.Height = m_cy;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createFlags = 0;
#ifdef _DEBUG
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createFlags,
        featureLevels,
        1,
        D3D11_SDK_VERSION,
        &sd,
        &m_pSwapChain,
        &m_pDevice,
        &featureLevel,
        &m_pContext);

    if (FAILED(hr))
    {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            createFlags,
            featureLevels,
            1,
            D3D11_SDK_VERSION,
            &sd,
            &m_pSwapChain,
            &m_pDevice,
            &featureLevel,
            &m_pContext);
    }

    m_swapChainDesc = sd;
    return hr;
}

HRESULT DefaultPreviewDX::CreateRenderTarget()
{
    if (!m_pSwapChain || !m_pDevice)
        return E_UNEXPECTED;

    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTarget);
    pBackBuffer->Release();

    if (SUCCEEDED(hr) && m_pContext && m_pRenderTarget)
    {
        m_pContext->OMSetRenderTargets(1, &m_pRenderTarget, nullptr);

        D3D11_VIEWPORT vp;
        ZeroMemory(&vp, sizeof(vp));
        vp.Width = static_cast<float>(m_cx);
        vp.Height = static_cast<float>(m_cy);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_pContext->RSSetViewports(1, &vp);
    }

    return hr;
}

void DefaultPreviewDX::ReleaseDeviceAndSwapChain()
{
    if (m_pRenderTarget)
    {
        m_pRenderTarget->Release();
        m_pRenderTarget = nullptr;
    }
    if (m_pContext)
    {
        m_pContext->Release();
        m_pContext = nullptr;
    }
    if (m_pSwapChain)
    {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }
    if (m_pDevice)
    {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }
}

// ============================================================================
// PreviewPresenterWrapper implementation
// ============================================================================

PreviewPresenterWrapper::PreviewPresenterWrapper()
    : m_state(PreviewStateStopped)
    , m_hWndPreview(nullptr)
    , m_fPreviewActive(false)
    , m_llCurrentPositionHns(0)
    , m_llTotalDurationHns(0)
    , m_dblVolume(1.0)
    , m_dblPlaybackSpeed(1.0)
    , m_pCurrentFrame(nullptr)
    , m_hdcBackBuffer(nullptr)
    , m_hbmpBackBuffer(nullptr)
    , m_hbmpOld(nullptr)
    , m_cxBackBuffer(0)
    , m_cyBackBuffer(0)
    , m_uTimerId(0)
    , m_hRenderThread(nullptr)
    , m_hStopEvent(nullptr)
    , m_hSeekEvent(nullptr)
    , m_pProject(nullptr)
    , m_pPreviewDX(nullptr)
{
    InitializeCriticalSection(&m_csLock);
}

PreviewPresenterWrapper::~PreviewPresenterWrapper()
{
    StopPreview();
    StopTimer();
    DestroyBackBuffer();

    if (m_pCurrentFrame)
    {
        delete m_pCurrentFrame;
        m_pCurrentFrame = nullptr;
    }

    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
        m_hStopEvent = nullptr;
    }
    if (m_hSeekEvent)
    {
        CloseHandle(m_hSeekEvent);
        m_hSeekEvent = nullptr;
    }

    DeleteCriticalSection(&m_csLock);
}

// ============================================================================
// StartPreview
// ============================================================================
HRESULT PreviewPresenterWrapper::StartPreview()
{
    if (m_fPreviewActive)
        return S_FALSE;

    if (!m_hWndPreview || !m_pProject)
        return E_UNEXPECTED;

    if (!m_hStopEvent)
    {
        m_hStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        if (!m_hStopEvent)
            return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!m_hSeekEvent)
    {
        m_hSeekEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        if (!m_hSeekEvent)
            return HRESULT_FROM_WIN32(GetLastError());
    }

    ResetEvent(m_hStopEvent);
    ResetEvent(m_hSeekEvent);

    m_fPreviewActive = true;
    m_state = PreviewStatePlaying;

    m_hRenderThread = CreateThread(nullptr, 0, RenderThreadProc, this, 0, nullptr);
    if (!m_hRenderThread)
    {
        m_fPreviewActive = false;
        m_state = PreviewStateStopped;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

// ============================================================================
// StopPreview
// ============================================================================
HRESULT PreviewPresenterWrapper::StopPreview()
{
    if (!m_fPreviewActive)
        return S_FALSE;

    StopTimer();

    if (m_hStopEvent)
        SetEvent(m_hStopEvent);

    if (m_hRenderThread)
    {
        WaitForSingleObject(m_hRenderThread, 5000);
        CloseHandle(m_hRenderThread);
        m_hRenderThread = nullptr;
    }

    m_fPreviewActive = false;
    m_state = PreviewStateStopped;

    EnterCriticalSection(&m_csLock);
    m_llCurrentPositionHns = 0;
    LeaveCriticalSection(&m_csLock);

    InvalidatePreview();
    return S_OK;
}

// ============================================================================
// PausePreview
// ============================================================================
HRESULT PreviewPresenterWrapper::PausePreview()
{
    if (!m_fPreviewActive)
        return E_UNEXPECTED;

    if (m_state == PreviewStatePlaying)
    {
        m_state = PreviewStatePaused;
        StopTimer();
    }
    else if (m_state == PreviewStatePaused)
    {
        m_state = PreviewStatePlaying;
        StartTimer();
    }

    return S_OK;
}

// ============================================================================
// IsPreviewActive
// ============================================================================
bool PreviewPresenterWrapper::IsPreviewActive() const throw()
{
    return m_fPreviewActive;
}

// ============================================================================
// SeekTo
// ============================================================================
HRESULT PreviewPresenterWrapper::SeekTo(LONGLONG llPositionHns)
{
    if (llPositionHns < 0)
        return E_INVALIDARG;

    EnterCriticalSection(&m_csLock);
    m_llCurrentPositionHns = llPositionHns;
    LeaveCriticalSection(&m_csLock);

    if (m_hSeekEvent)
        SetEvent(m_hSeekEvent);

    return S_OK;
}

HRESULT PreviewPresenterWrapper::SeekToNormalized(double dblPosition)
{
    if (dblPosition < 0.0 || dblPosition > 1.0)
        return E_INVALIDARG;

    LONGLONG llTarget = static_cast<LONGLONG>(dblPosition * m_llTotalDurationHns);
    return SeekTo(llTarget);
}

LONGLONG PreviewPresenterWrapper::GetCurrentPositionHns() const throw()
{
    return m_llCurrentPositionHns;
}

LONGLONG PreviewPresenterWrapper::GetTotalDurationHns() const throw()
{
    return m_llTotalDurationHns;
}

double PreviewPresenterWrapper::GetNormalizedPosition() const throw()
{
    if (m_llTotalDurationHns <= 0)
        return 0.0;
    return static_cast<double>(m_llCurrentPositionHns) /
           static_cast<double>(m_llTotalDurationHns);
}

// ============================================================================
// Window management
// ============================================================================
HRESULT PreviewPresenterWrapper::SetPreviewWindow(HWND hWnd)
{
    m_hWndPreview = hWnd;

    if (hWnd)
    {
        RECT rc;
        if (GetClientRect(hWnd, &rc))
        {
            UINT cx = static_cast<UINT>(rc.right - rc.left);
            UINT cy = static_cast<UINT>(rc.bottom - rc.top);

            CreateBackBuffer(cx, cy);

            if (m_pPreviewDX)
            {
                HRESULT hr = m_pPreviewDX->Resize(cx, cy);
                if (FAILED(hr))
                    return hr;
            }
        }
    }
    else
    {
        DestroyBackBuffer();
    }

    return S_OK;
}

HWND PreviewPresenterWrapper::GetPreviewWindow() const throw()
{
    return m_hWndPreview;
}

HRESULT PreviewPresenterWrapper::ResizePreview(UINT cx, UINT cy)
{
    if (cx == 0 || cy == 0)
        return E_INVALIDARG;

    CreateBackBuffer(cx, cy);

    if (m_pPreviewDX)
    {
        HRESULT hr = m_pPreviewDX->Resize(cx, cy);
        if (FAILED(hr))
            return hr;
    }

    InvalidatePreview();
    return S_OK;
}

// ============================================================================
// Project binding
// ============================================================================
HRESULT PreviewPresenterWrapper::SetProject(StoryboardManager::MovieProject* pProject)
{
    m_pProject = pProject;

    if (m_pProject)
    {
        m_llTotalDurationHns = 0;

        for (int t = 0; t < 6; ++t)
        {
            const StoryboardManager::ProjectTimeline* pTimeline =
                m_pProject->GetTimeline(static_cast<StoryboardManager::TimelineTrackType>(t));
            if (pTimeline)
            {
                m_llTotalDurationHns += pTimeline->GetTotalDurationHns();
            }
        }
    }
    else
    {
        m_llTotalDurationHns = 0;
    }

    return S_OK;
}

StoryboardManager::MovieProject* PreviewPresenterWrapper::GetProject() const throw()
{
    return m_pProject;
}

// ============================================================================
// Rendering surface
// ============================================================================
HRESULT PreviewPresenterWrapper::SetPreviewDX(PreviewDX* pPreviewDX)
{
    m_pPreviewDX = pPreviewDX;
    return S_OK;
}

PreviewDX* PreviewPresenterWrapper::GetPreviewDX() const throw()
{
    return m_pPreviewDX;
}

// ============================================================================
// SetDuration
// ============================================================================
HRESULT PreviewPresenterWrapper::SetDuration(LONGLONG llDurationHns)
{
    if (llDurationHns < 0)
        return E_INVALIDARG;

    EnterCriticalSection(&m_csLock);
    m_llTotalDurationHns = llDurationHns;
    LeaveCriticalSection(&m_csLock);

    return S_OK;
}

// ============================================================================
// CaptureFrame
// ============================================================================
HRESULT PreviewPresenterWrapper::CaptureFrame(Gdiplus::Bitmap** ppBitmap)
{
    if (!ppBitmap)
        return E_POINTER;

    *ppBitmap = nullptr;

    if (!m_pCurrentFrame)
        return E_UNEXPECTED;

    EnterCriticalSection(&m_csLock);
    *ppBitmap = m_pCurrentFrame->Clone(0, 0,
        m_pCurrentFrame->GetWidth(), m_pCurrentFrame->GetHeight(),
        m_pCurrentFrame->GetPixelFormat());
    LeaveCriticalSection(&m_csLock);

    return (*ppBitmap) ? S_OK : E_OUTOFMEMORY;
}

HRESULT PreviewPresenterWrapper::CaptureFrameToHBitmap(HBITMAP* phBitmap)
{
    if (!phBitmap)
        return E_POINTER;

    *phBitmap = nullptr;

    if (!m_pCurrentFrame)
        return E_UNEXPECTED;

    EnterCriticalSection(&m_csLock);
    m_pCurrentFrame->GetHBITMAP(Gdiplus::Color(0, 0, 0), phBitmap);
    LeaveCriticalSection(&m_csLock);

    return (*phBitmap) ? S_OK : E_FAIL;
}

// ============================================================================
// Volume / Speed
// ============================================================================
HRESULT PreviewPresenterWrapper::SetVolume(double dblVolume)
{
    if (dblVolume < 0.0 || dblVolume > 1.0)
        return E_INVALIDARG;

    m_dblVolume = dblVolume;
    return S_OK;
}

double PreviewPresenterWrapper::GetVolume() const throw()
{
    return m_dblVolume;
}

HRESULT PreviewPresenterWrapper::SetPlaybackSpeed(double dblSpeed)
{
    if (dblSpeed <= 0.0 || dblSpeed > 8.0)
        return E_INVALIDARG;

    m_dblPlaybackSpeed = dblSpeed;
    return S_OK;
}

double PreviewPresenterWrapper::GetPlaybackSpeed() const throw()
{
    return m_dblPlaybackSpeed;
}

// ============================================================================
// GetState
// ============================================================================
PreviewPresenterWrapper::PreviewState PreviewPresenterWrapper::GetState() const throw()
{
    return m_state;
}

// ============================================================================
// RenderThreadProc (static)
// ============================================================================
DWORD WINAPI PreviewPresenterWrapper::RenderThreadProc(LPVOID pParam)
{
    PreviewPresenterWrapper* pThis = static_cast<PreviewPresenterWrapper*>(pParam);
    if (pThis)
        pThis->DoRenderLoop();
    return 0;
}

// ============================================================================
// DoRenderLoop
// ============================================================================
void PreviewPresenterWrapper::DoRenderLoop()
{
    HANDLE handles[] = { m_hStopEvent, m_hSeekEvent };
    DWORD cHandles = m_hSeekEvent ? 2 : 1;

    while (m_fPreviewActive)
    {
        DWORD dwWait = WaitForMultipleObjects(cHandles, handles, FALSE, kRenderThreadSleepMs);

        if (dwWait == WAIT_OBJECT_0)
            break;

        if (dwWait == WAIT_OBJECT_0 + 1)
        {
            ResetEvent(m_hSeekEvent);
            RenderCurrentFrame();
            continue;
        }

        if (m_state == PreviewStatePlaying || m_state == PreviewStatePaused)
        {
            RenderCurrentFrame();
        }
    }
}

// ============================================================================
// RenderCurrentFrame
// ============================================================================
void PreviewPresenterWrapper::RenderCurrentFrame()
{
    if (!m_hWndPreview || !IsWindow(m_hWndPreview))
        return;

    EnterCriticalSection(&m_csLock);

    if (m_pPreviewDX && m_pPreviewDX->IsInitialized())
    {
        m_pPreviewDX->BeginFrame();
        if (m_pCurrentFrame)
        {
            m_pPreviewDX->DrawBitmap(m_pCurrentFrame);
        }
        m_pPreviewDX->EndFrame();
        m_pPreviewDX->Present();
    }

    LeaveCriticalSection(&m_csLock);

    InvalidatePreview();
}

// ============================================================================
// CreateBackBuffer
// ============================================================================
void PreviewPresenterWrapper::CreateBackBuffer(UINT cx, UINT cy)
{
    if (m_hdcBackBuffer && m_cxBackBuffer == cx && m_cyBackBuffer == cy)
        return;

    DestroyBackBuffer();

    if (cx == 0 || cy == 0)
        return;

    HDC hdcScreen = GetDC(m_hWndPreview);
    if (!hdcScreen)
        return;

    m_hdcBackBuffer = CreateCompatibleDC(hdcScreen);
    if (m_hdcBackBuffer)
    {
        m_hbmpBackBuffer = CreateCompatibleBitmap(hdcScreen, cx, cy);
        if (m_hbmpBackBuffer)
        {
            m_hbmpOld = static_cast<HBITMAP>(SelectObject(m_hdcBackBuffer, m_hbmpBackBuffer));
            m_cxBackBuffer = cx;
            m_cyBackBuffer = cy;

            RECT rc = { 0, 0, static_cast<LONG>(cx), static_cast<LONG>(cy) };
            FillRect(m_hdcBackBuffer, &rc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        }
        else
        {
            DeleteDC(m_hdcBackBuffer);
            m_hdcBackBuffer = nullptr;
        }
    }

    ReleaseDC(m_hWndPreview, hdcScreen);
}

// ============================================================================
// DestroyBackBuffer
// ============================================================================
void PreviewPresenterWrapper::DestroyBackBuffer()
{
    if (m_hdcBackBuffer)
    {
        if (m_hbmpOld)
        {
            SelectObject(m_hdcBackBuffer, m_hbmpOld);
            m_hbmpOld = nullptr;
        }
        if (m_hbmpBackBuffer)
        {
            DeleteObject(m_hbmpBackBuffer);
            m_hbmpBackBuffer = nullptr;
        }
        DeleteDC(m_hdcBackBuffer);
        m_hdcBackBuffer = nullptr;
    }
    m_cxBackBuffer = 0;
    m_cyBackBuffer = 0;
}

// ============================================================================
// PaintFrame - WM_PAINT handler with double-buffering
// ============================================================================
void PreviewPresenterWrapper::PaintFrame(HDC hdc, const RECT& rcPaint)
{
    UNREFERENCED_PARAMETER(rcPaint);

    if (!m_hdcBackBuffer)
        return;

    EnterCriticalSection(&m_csLock);

    if (m_pCurrentFrame)
    {
        Gdiplus::Graphics g(m_hdcBackBuffer);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

        UINT imgW = m_pCurrentFrame->GetWidth();
        UINT imgH = m_pCurrentFrame->GetHeight();

        if (imgW > 0 && imgH > 0)
        {
            float scaleX = static_cast<float>(m_cxBackBuffer) / static_cast<float>(imgW);
            float scaleY = static_cast<float>(m_cyBackBuffer) / static_cast<float>(imgH);
            float scale = (std::min)(scaleX, scaleY);

            float drawW = static_cast<float>(imgW) * scale;
            float drawH = static_cast<float>(imgH) * scale;
            float offsetX = (static_cast<float>(m_cxBackBuffer) - drawW) / 2.0f;
            float offsetY = (static_cast<float>(m_cyBackBuffer) - drawH) / 2.0f;

            g.DrawImage(m_pCurrentFrame,
                Gdiplus::RectF(offsetX, offsetY, drawW, drawH),
                0, 0, static_cast<float>(imgW), static_cast<float>(imgH),
                Gdiplus::UnitPixel);
        }
    }

    LeaveCriticalSection(&m_csLock);
}

// ============================================================================
// PresentFrame - Blit back buffer to window
// ============================================================================
void PreviewPresenterWrapper::PresentFrame()
{
    if (!m_hWndPreview || !m_hdcBackBuffer)
        return;

    HDC hdcWindow = GetDC(m_hWndPreview);
    if (hdcWindow)
    {
        BitBlt(hdcWindow, 0, 0, m_cxBackBuffer, m_cyBackBuffer,
               m_hdcBackBuffer, 0, 0, SRCCOPY);
        ReleaseDC(m_hWndPreview, hdcWindow);
    }
}

// ============================================================================
// InvalidatePreview
// ============================================================================
void PreviewPresenterWrapper::InvalidatePreview()
{
    if (m_hWndPreview && IsWindow(m_hWndPreview))
    {
        InvalidateRect(m_hWndPreview, nullptr, FALSE);
    }
}

// ============================================================================
// OnPaint
// ============================================================================
void PreviewPresenterWrapper::OnPaint()
{
    if (!m_hWndPreview || !IsWindow(m_hWndPreview))
        return;

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWndPreview, &ps);
    if (hdc)
    {
        PaintFrame(hdc, ps.rcPaint);
        PresentFrame();
        EndPaint(m_hWndPreview, &ps);
    }
}

// ============================================================================
// Playback timer
// ============================================================================
void PreviewPresenterWrapper::StartTimer()
{
    if (m_uTimerId != 0)
        return;

    DWORD intervalMs = CalculateFrameIntervalMs();
    if (intervalMs == 0)
        intervalMs = 33;

    m_uTimerId = timeSetEvent(intervalMs, 1, OnPlaybackTimer,
                              reinterpret_cast<DWORD_PTR>(this),
                              TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
}

void PreviewPresenterWrapper::StopTimer()
{
    if (m_uTimerId != 0)
    {
        timeKillEvent(m_uTimerId);
        m_uTimerId = 0;
    }
}

void CALLBACK PreviewPresenterWrapper::OnPlaybackTimer(
    UINT /*uTimerID*/, UINT /*uMsg*/, DWORD_PTR dwUser,
    DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/)
{
    PreviewPresenterWrapper* pThis = reinterpret_cast<PreviewPresenterWrapper*>(dwUser);
    if (!pThis)
        return;

    if (pThis->m_state != PreviewStatePlaying)
        return;

    EnterCriticalSection(&pThis->m_csLock);
    pThis->m_llCurrentPositionHns += static_cast<LONGLONG>(
        pThis->CalculateFrameIntervalMs() * 10000 * pThis->m_dblPlaybackSpeed);

    if (pThis->m_llTotalDurationHns > 0 &&
        pThis->m_llCurrentPositionHns >= pThis->m_llTotalDurationHns)
    {
        pThis->m_llCurrentPositionHns = 0;
    }
    LeaveCriticalSection(&pThis->m_csLock);

    pThis->RenderCurrentFrame();
}

DWORD PreviewPresenterWrapper::CalculateFrameIntervalMs() const
{
    double fps = 30.0;
    if (m_pProject)
    {
        DWORD dwFrameRate = m_pProject->GetSettings().GetFrameRate();
        if (dwFrameRate > 0)
            fps = static_cast<double>(dwFrameRate) / 100.0;
    }

    if (fps <= 0.0)
        fps = 30.0;

    double intervalMs = 1000.0 / fps;
    return static_cast<DWORD>(intervalMs + 0.5);
}

// ============================================================================
// Extent navigation helpers
// ============================================================================
int PreviewPresenterWrapper::FindExtentAtPosition(LONGLONG llPositionHns) const
{
    if (!m_pProject || llPositionHns < 0)
        return -1;

    const StoryboardManager::ProjectTimeline* pTimeline =
        m_pProject->GetTimeline(StoryboardManager::TimelineTrackTypeVideo);
    if (!pTimeline)
        return -1;

    size_t cExtents = pTimeline->GetExtentCount();
    if (cExtents == 0)
        return -1;

    LONGLONG llTotalDuration = pTimeline->GetTotalDurationHns();
    if (llTotalDuration <= 0)
        return -1;

    double dblExtentDuration = static_cast<double>(llTotalDuration) / static_cast<double>(cExtents);
    int nIndex = static_cast<int>(llPositionHns / dblExtentDuration);

    if (nIndex < 0)
        nIndex = 0;
    if (nIndex >= static_cast<int>(cExtents))
        nIndex = static_cast<int>(cExtents) - 1;

    return nIndex;
}

LONGLONG PreviewPresenterWrapper::GetExtentStartTime(int nIndex) const
{
    if (!m_pProject || nIndex < 0)
        return 0;

    const StoryboardManager::ProjectTimeline* pTimeline =
        m_pProject->GetTimeline(StoryboardManager::TimelineTrackTypeVideo);
    if (!pTimeline)
        return 0;

    size_t cExtents = pTimeline->GetExtentCount();
    if (cExtents == 0 || nIndex >= static_cast<int>(cExtents))
        return 0;

    LONGLONG llTotalDuration = pTimeline->GetTotalDurationHns();
    double dblExtentDuration = static_cast<double>(llTotalDuration) / static_cast<double>(cExtents);

    return static_cast<LONGLONG>(nIndex * dblExtentDuration);
}

LONGLONG PreviewPresenterWrapper::GetExtentEndTime(int nIndex) const
{
    if (!m_pProject || nIndex < 0)
        return 0;

    const StoryboardManager::ProjectTimeline* pTimeline =
        m_pProject->GetTimeline(StoryboardManager::TimelineTrackTypeVideo);
    if (!pTimeline)
        return 0;

    size_t cExtents = pTimeline->GetExtentCount();
    if (cExtents == 0 || nIndex >= static_cast<int>(cExtents))
        return 0;

    LONGLONG llTotalDuration = pTimeline->GetTotalDurationHns();
    double dblExtentDuration = static_cast<double>(llTotalDuration) / static_cast<double>(cExtents);

    LONGLONG llEndTime = static_cast<LONGLONG>((nIndex + 1) * dblExtentDuration);
    if (nIndex + 1 >= static_cast<int>(cExtents))
        llEndTime = llTotalDuration;

    return llEndTime;
}

// ============================================================================
// Factory functions
// ============================================================================

STORYBOARD_API HRESULT CreatePreviewPresenter(PreviewPresenterWrapper** ppPresenter)
{
    if (!ppPresenter)
        return E_POINTER;

    *ppPresenter = nullptr;

    CComObject<PreviewPresenterWrapper>* pObj = nullptr;
    HRESULT hr = CComObject<PreviewPresenterWrapper>::CreateInstance(&pObj);
    if (FAILED(hr))
        return hr;

    pObj->AddRef();
    *ppPresenter = pObj;

    return S_OK;
}

STORYBOARD_API HRESULT CreateDefaultPreviewDX(DefaultPreviewDX** ppPreviewDX)
{
    if (!ppPreviewDX)
        return E_POINTER;

    *ppPreviewDX = new (std::nothrow) DefaultPreviewDX();
    if (!*ppPreviewDX)
        return E_OUTOFMEMORY;

    return S_OK;
}
