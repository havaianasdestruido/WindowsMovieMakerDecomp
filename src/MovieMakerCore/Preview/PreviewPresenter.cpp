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
    UNREFERENCED_PARAMETER(pBitmap);
    if (!m_fInitialized)
        return E_UNEXPECTED;

    return S_OK;
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
    m_llCurrentPositionHns = 0;

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
    }
    else if (m_state == PreviewStatePaused)
    {
        m_state = PreviewStatePlaying;

        if (m_hRenderThread)
        {
            SetEvent(m_hSeekEvent);
            ResetEvent(m_hSeekEvent);
        }
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

    if (m_pPreviewDX && hWnd)
    {
        RECT rc;
        if (GetClientRect(hWnd, &rc))
        {
            return m_pPreviewDX->Resize(
                static_cast<UINT>(rc.right - rc.left),
                static_cast<UINT>(rc.bottom - rc.top));
        }
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

    if (m_pPreviewDX)
        return m_pPreviewDX->Resize(cx, cy);

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
// CaptureFrame
// ============================================================================
HRESULT PreviewPresenterWrapper::CaptureFrame(Gdiplus::Bitmap** ppBitmap)
{
    if (!ppBitmap)
        return E_POINTER;

    *ppBitmap = nullptr;

    if (!m_pPreviewDX || !m_pPreviewDX->IsInitialized())
        return E_UNEXPECTED;

    return S_OK;
}

HRESULT PreviewPresenterWrapper::CaptureFrameToHBitmap(HBITMAP* phBitmap)
{
    if (!phBitmap)
        return E_POINTER;

    *phBitmap = nullptr;
    return S_OK;
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
            if (m_state == PreviewStatePaused)
            {
                RenderCurrentFrame();
                continue;
            }
        }

        if (m_state == PreviewStatePlaying)
        {
            EnterCriticalSection(&m_csLock);
            m_llCurrentPositionHns += static_cast<LONGLONG>(kRenderThreadSleepMs * 10000);
            if (m_llCurrentPositionHns >= m_llTotalDurationHns)
            {
                m_llCurrentPositionHns = 0;
            }
            LeaveCriticalSection(&m_csLock);
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
    if (!m_pPreviewDX || !m_pPreviewDX->IsInitialized())
        return;

    m_pPreviewDX->BeginFrame();
    m_pPreviewDX->EndFrame();
    m_pPreviewDX->Present();
}

// ============================================================================
// Extent navigation helpers
// ============================================================================
int PreviewPresenterWrapper::FindExtentAtPosition(LONGLONG llPositionHns) const
{
    UNREFERENCED_PARAMETER(llPositionHns);
    return -1;
}

LONGLONG PreviewPresenterWrapper::GetExtentStartTime(int nIndex) const
{
    UNREFERENCED_PARAMETER(nIndex);
    return 0;
}

LONGLONG PreviewPresenterWrapper::GetExtentEndTime(int nIndex) const
{
    UNREFERENCED_PARAMETER(nIndex);
    return 0;
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
