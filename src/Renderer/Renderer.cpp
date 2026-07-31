#include "Renderer.h"
#include <dxgi1_2.h>

template <class T>
inline void SafeRelease(T*& pVal) {
    if (pVal != nullptr) {
        pVal->Release();
        pVal = nullptr;
    }
}

namespace DirectUI {

GPURenderer::GPURenderer() {}

GPURenderer::~GPURenderer() {
    Cleanup();
}

void GPURenderer::Cleanup() {
    SafeRelease(m_pD2DTargetBitmap);
    SafeRelease(m_pD2DContext);
    SafeRelease(m_pD2DDevice);
    SafeRelease(m_pD2DFactory);
    SafeRelease(m_pRenderTargetView);
    SafeRelease(m_pSwapChain);
    SafeRelease(m_pContext);
    SafeRelease(m_pDevice);
}

HRESULT GPURenderer::Initialize(HWND hwnd) {
    Cleanup();
    if (!hwnd) return E_INVALIDARG;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_pDevice,
        &featureLevel,
        &m_pContext
    );

    if (FAILED(hr)) {
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &m_pDevice,
            &featureLevel,
            &m_pContext
        );
    }

    if (FAILED(hr)) return hr;

    IDXGIDevice* pDxgiDevice = nullptr;
    hr = m_pDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDxgiDevice));
    if (FAILED(hr)) return hr;

    IDXGIAdapter* pDxgiAdapter = nullptr;
    hr = pDxgiDevice->GetAdapter(&pDxgiAdapter);
    if (FAILED(hr)) {
        SafeRelease(pDxgiDevice);
        return hr;
    }

    IDXGIFactory2* pDxgiFactory = nullptr;
    hr = pDxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&pDxgiFactory));
    if (FAILED(hr)) {
        SafeRelease(pDxgiAdapter);
        SafeRelease(pDxgiDevice);
        return hr;
    }

    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    IDXGISwapChain1* pSwapChain1 = nullptr;
    hr = pDxgiFactory->CreateSwapChainForHwnd(
        m_pDevice,
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &pSwapChain1
    );

    if (SUCCEEDED(hr)) {
        m_pSwapChain = pSwapChain1;
    }

    SafeRelease(pDxgiFactory);
    SafeRelease(pDxgiAdapter);

    if (FAILED(hr)) {
        SafeRelease(pDxgiDevice);
        return hr;
    }

    D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, reinterpret_cast<void**>(&m_pD2DFactory));
    if (FAILED(hr)) {
        SafeRelease(pDxgiDevice);
        return hr;
    }

    hr = m_pD2DFactory->CreateDevice(pDxgiDevice, &m_pD2DDevice);
    SafeRelease(pDxgiDevice);

    if (FAILED(hr)) return hr;

    hr = m_pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2DContext);
    if (FAILED(hr)) return hr;

    return Resize(width, height);
}

HRESULT GPURenderer::Resize(int width, int height) {
    if (!m_pSwapChain || !m_pDevice || !m_pD2DContext) return E_UNEXPECTED;
    if (width < 1) width = 1;
    if (height < 1) height = 1;

    if (!m_pContext) return E_UNEXPECTED;
    m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
    SafeRelease(m_pRenderTargetView);
    SafeRelease(m_pD2DTargetBitmap);
    m_pD2DContext->SetTarget(nullptr);

    HRESULT hr = m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) return hr;

    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr)) return hr;

    hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
    if (FAILED(hr)) {
        SafeRelease(pBackBuffer);
        return hr;
    }

    IDXGISurface* pDxgiBackBuffer = nullptr;
    hr = pBackBuffer->QueryInterface(__uuidof(IDXGISurface), reinterpret_cast<void**>(&pDxgiBackBuffer));
    SafeRelease(pBackBuffer);
    if (FAILED(hr)) return hr;

    D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    hr = m_pD2DContext->CreateBitmapFromDxgiSurface(pDxgiBackBuffer, &bitmapProperties, &m_pD2DTargetBitmap);
    SafeRelease(pDxgiBackBuffer);
    if (FAILED(hr)) return hr;

    m_pD2DContext->SetTarget(m_pD2DTargetBitmap);
    return S_OK;
}

HRESULT GPURenderer::BeginDraw() {
    if (!m_pD2DContext) return E_UNEXPECTED;
    m_pD2DContext->BeginDraw();
    return S_OK;
}

HRESULT GPURenderer::EndDraw() {
    if (!m_pD2DContext) return E_UNEXPECTED;
    return m_pD2DContext->EndDraw();
}

HRESULT GPURenderer::Present() {
    if (!m_pSwapChain) return E_UNEXPECTED;
    return m_pSwapChain->Present(1, 0);
}

HRESULT GPURenderer::DrawVideoFrame(ID3D11Texture2D* pTexture, const RECT& destRect) {
    if (!m_pContext || !m_pRenderTargetView || !pTexture) return E_INVALIDARG;

    if (!m_pContext) return E_UNEXPECTED;
    m_pContext->OMSetRenderTargets(0, nullptr, nullptr);

    D3D11_TEXTURE2D_DESC desc;
    pTexture->GetDesc(&desc);

    ID3D11Resource* pBackBufferRes = nullptr;
    m_pRenderTargetView->GetResource(&pBackBufferRes);
    if (!pBackBufferRes) return E_FAIL;

    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = pBackBufferRes->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    SafeRelease(pBackBufferRes);
    if (FAILED(hr)) return hr;

    D3D11_BOX sourceBox;
    sourceBox.left = 0;
    sourceBox.top = 0;
    sourceBox.front = 0;
    sourceBox.right = desc.Width;
    sourceBox.bottom = desc.Height;
    sourceBox.back = 1;

    m_pContext->CopySubresourceRegion(
        pBackBuffer,
        0,
        destRect.left,
        destRect.top,
        0,
        pTexture,
        0,
        &sourceBox
    );

    SafeRelease(pBackBuffer);
    return S_OK;
}

} // namespace DirectUI
