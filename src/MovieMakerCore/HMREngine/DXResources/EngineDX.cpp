#include "pch.h"
// EngineDX.cpp - D3D11 device management implementation

#include "EngineDX.h"
#include "../Engine.h"
#include "../ErrHandler.h"

namespace HMREngine
{
namespace DX
{

// ============================================================================
// Helper: D3D11 feature level table (descending order)
// ============================================================================
static const D3D_FEATURE_LEVEL s_featureLevels[] = {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
};

// ============================================================================
// EnumerateAdapters
// ============================================================================
HRESULT EnumerateAdapters(std::vector<AdapterInfo>& adapters)
{
    adapters.clear();

    CComPtr<IDXGIFactory> factory;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(hr)) return hr;

    for (UINT i = 0; ; ++i)
    {
        CComPtr<IDXGIAdapter> adapter;
        hr = factory->EnumAdapters(i, &adapter);
        if (hr == DXGI_ERROR_NOT_FOUND) break;
        if (FAILED(hr)) break;

        DXGI_ADAPTER_DESC desc;
        hr = adapter->GetDesc(&desc);
        if (FAILED(hr)) continue;

        AdapterInfo info;
        info.description = desc.Description;
        info.vendorId = desc.VendorId;
        info.deviceId = desc.DeviceId;
        info.dedicatedVideoMemory = desc.DedicatedVideoMemory;
        info.dedicatedSystemMemory = desc.DedicatedSystemMemory;
        info.sharedSystemMemory = desc.SharedSystemMemory;

        adapters.push_back(info);
    }

    return S_OK;
}

// ============================================================================
// GetDeviceFeatures
// ============================================================================
HRESULT GetDeviceFeatures(ID3D11Device* dev, DeviceFeatures& features)
{
    if (!dev) return E_POINTER;

    features.instancing = false;

    HRESULT hr = S_OK;

    D3D11_FEATURE_DATA_D3D9_OPTIONS d9opts = {};
    hr = dev->CheckFeatureSupport(D3D11_FEATURE_D3D9_OPTIONS, &d9opts, sizeof(d9opts));

    D3D11_FEATURE_DATA_THREADING threading = {};
    hr = dev->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threading, sizeof(threading));

    D3D11_FEATURE_DATA_FORMAT_SUPPORT fmtSupport = {};
    fmtSupport.InFormat = DXGI_FORMAT_BC1_UNORM;
    hr = dev->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &fmtSupport, sizeof(fmtSupport));
    features.bc1Compression = SUCCEEDED(hr) && (fmtSupport.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) != 0;

    fmtSupport.InFormat = DXGI_FORMAT_BC3_UNORM;
    hr = dev->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &fmtSupport, sizeof(fmtSupport));
    features.bc3Compression = SUCCEEDED(hr) && (fmtSupport.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) != 0;

    fmtSupport.InFormat = DXGI_FORMAT_BC5_UNORM;
    hr = dev->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &fmtSupport, sizeof(fmtSupport));
    features.bc5Compression = SUCCEEDED(hr) && (fmtSupport.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) != 0;

    D3D11_FEATURE_DATA_DOUBLES dbl = {};
    hr = dev->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &dbl, sizeof(dbl));
    features.doublePrecision = SUCCEEDED(hr) && dbl.DoublePrecisionFloatShaderOps != 0;

    features.featureLevel = dev->GetFeatureLevel();
    features.computeShader = (features.featureLevel >= D3D_FEATURE_LEVEL_11_0);
    features.tessellation = (features.featureLevel >= D3D_FEATURE_LEVEL_11_0);
    features.maxRenderTargets = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

    switch (features.featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0: features.maxTextureSize = 16384; break;
    case D3D_FEATURE_LEVEL_10_1:
    case D3D_FEATURE_LEVEL_10_0: features.maxTextureSize = 8192; break;
    default: features.maxTextureSize = 4096; break;
    }

    return S_OK;
}

// ============================================================================
// EngineDX
// ============================================================================
EngineDX::EngineDX() = default;

EngineDX::~EngineDX()
{
    Shutdown();
}

HRESULT EngineDX::Initialize(const EngineDesc& desc)
{
    HRESULT hr = InitializeDevice(desc);
    if (FAILED(hr)) return hr;

    hr = InitializeSwapChain(desc);
    if (FAILED(hr)) return hr;

    hr = InitializeRenderTargets();
    if (FAILED(hr)) return hr;

    hr = InitializeDepthBuffer();
    if (FAILED(hr)) return hr;

    InitializeViewport();

    hr = CreateDepthStencilState();
    if (FAILED(hr)) return hr;

    hr = CreateBlendState();
    if (FAILED(hr)) return hr;

    hr = CreateRasterizerState();
    if (FAILED(hr)) return hr;

    hr = QueryDeviceFeatures();
    return hr;
}

HRESULT EngineDX::InitializeDevice(const EngineDesc& desc)
{
    m_hWnd = desc.hWnd;
    m_width = desc.width;
    m_height = desc.height;
    m_vsync = desc.vsync;

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifndef NDEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL requestedLevels[4];
    UINT numLevels = desc.numFeatureLevels;
    if (numLevels == 0 || !desc.pFeatureLevels)
    {
        numLevels = _countof(s_featureLevels);
        memcpy(requestedLevels, s_featureLevels, sizeof(s_featureLevels));
    }
    else
    {
        memcpy(requestedLevels, desc.pFeatureLevels, numLevels * sizeof(D3D_FEATURE_LEVEL));
    }

    HRESULT hr = D3D11CreateDevice(
        desc.adapterIndex == 0 ? nullptr : nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        requestedLevels,
        numLevels,
        D3D11_SDK_VERSION,
        &m_device,
        &m_featureLevel,
        &m_immediateContext
    );

    if (FAILED(hr))
    {
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            flags,
            requestedLevels,
            numLevels,
            D3D11_SDK_VERSION,
            &m_device,
            &m_featureLevel,
            &m_immediateContext
        );
    }

    return hr;
}

HRESULT EngineDX::InitializeSwapChain(const EngineDesc& desc)
{
    CComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) return hr;

    hr = dxgiDevice->GetAdapter(&m_adapter);
    if (FAILED(hr)) return hr;

    hr = m_adapter->GetParent(__uuidof(IDXGIFactory), (void**)&m_factory);
    if (FAILED(hr)) return hr;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Width = m_width;
    scd.BufferDesc.Height = m_height;
    scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = m_hWnd;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.Windowed = desc.windowed;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    hr = m_factory->CreateSwapChain(m_device, &scd, &m_swapChain);
    return hr;
}

HRESULT EngineDX::InitializeRenderTargets()
{
    if (!m_swapChain) return E_FAIL;

    CComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_backBufferRTV);
    return hr;
}

HRESULT EngineDX::InitializeDepthBuffer()
{
    D3D11_TEXTURE2D_DESC dsd = {};
    dsd.Width = m_width;
    dsd.Height = m_height;
    dsd.MipLevels = 1;
    dsd.ArraySize = 1;
    dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsd.SampleDesc.Count = 1;
    dsd.SampleDesc.Quality = 0;
    dsd.Usage = D3D11_USAGE_DEFAULT;
    dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT hr = m_device->CreateTexture2D(&dsd, nullptr, &m_depthStencilBuffer);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateDepthStencilView(m_depthStencilBuffer, nullptr, &m_depthStencilView);
    return hr;
}

void EngineDX::InitializeViewport()
{
    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)m_width;
    vp.Height = (FLOAT)m_height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_immediateContext->RSSetViewports(1, &vp);
}

void EngineDX::Shutdown()
{
    if (m_immediateContext) m_immediateContext->ClearState();

    m_depthStencilView.Release();
    m_depthStencilBuffer.Release();
    m_backBufferRTV.Release();
    m_depthStencilState.Release();
    m_blendState.Release();
    m_rasterizerState.Release();
    m_swapChain.Release();
    m_immediateContext.Release();
    m_device.Release();
    m_factory.Release();
    m_adapter.Release();

    m_deviceLost = false;
}

HRESULT EngineDX::CreateDepthStencilState()
{
    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = TRUE;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    return m_device->CreateDepthStencilState(&dsd, &m_depthStencilState);
}

HRESULT EngineDX::CreateBlendState()
{
    D3D11_BLEND_DESC bd = {};
    bd.RenderTarget[0].BlendEnable = FALSE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    return m_device->CreateBlendState(&bd, &m_blendState);
}

HRESULT EngineDX::CreateRasterizerState()
{
    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = FALSE;
    rd.DepthClipEnable = TRUE;
    rd.ScissorEnable = FALSE;
    rd.MultisampleEnable = FALSE;
    rd.AntialiasedLineEnable = FALSE;
    return m_device->CreateRasterizerState(&rd, &m_rasterizerState);
}

HRESULT EngineDX::QueryDeviceFeatures()
{
    return GetDeviceFeatures(m_device, m_features);
}

HRESULT EngineDX::BeginFrame()
{
    if (!m_backBufferRTV) return E_FAIL;

    const float clearCol[4] = { m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w };
    m_immediateContext->ClearRenderTargetView(m_backBufferRTV, clearCol);

    if (m_depthStencilView)
        m_immediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, m_clearDepth, m_clearStencil);

    m_immediateContext->OMSetRenderTargets(1, &m_backBufferRTV.p, m_depthStencilView);
    m_immediateContext->OMSetDepthStencilState(m_depthStencilState, 0);
    m_immediateContext->OMSetBlendState(m_blendState, nullptr, 0xFFFFFFFF);
    m_immediateContext->RSSetState(m_rasterizerState);

    return S_OK;
}

HRESULT EngineDX::EndFrame()
{
    return S_OK;
}

HRESULT EngineDX::Present()
{
    if (!m_swapChain) return E_FAIL;

    HRESULT hr = m_swapChain->Present(m_vsync ? 1 : 0, 0);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        m_deviceLost = true;
        HandleDeviceLost();
    }

    return hr;
}

HRESULT EngineDX::Resize(UINT width, UINT height)
{
    if (width == 0 || height == 0) return E_INVALIDARG;
    if (width == m_width && height == m_height) return S_OK;

    m_width = width;
    m_height = height;

    if (m_immediateContext) m_immediateContext->OMSetRenderTargets(0, nullptr, nullptr);

    m_backBufferRTV.Release();
    m_depthStencilView.Release();
    m_depthStencilBuffer.Release();

    HRESULT hr = m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
    if (FAILED(hr)) return hr;

    hr = InitializeRenderTargets();
    if (FAILED(hr)) return hr;

    hr = InitializeDepthBuffer();
    if (FAILED(hr)) return hr;

    InitializeViewport();

    return S_OK;
}

HRESULT EngineDX::ResetDevice()
{
    if (!m_deviceLost) return S_FALSE;

    Shutdown();
    EngineDesc desc;
    desc.hWnd = m_hWnd;
    desc.width = m_width;
    desc.height = m_height;
    desc.windowed = true;
    desc.vsync = m_vsync;

    return Initialize(desc);
}

HRESULT EngineDX::HandleDeviceLost()
{
    if (m_errorHandler)
    {
    }
    return S_OK;
}

HRESULT EngineDX::CreateRenderTarget(UINT width, UINT height, ID3D11RenderTargetView** ppRTV)
{
    if (!ppRTV) return E_POINTER;

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    CComPtr<ID3D11Texture2D> tex;
    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &tex);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateRenderTargetView(tex, nullptr, ppRTV);
    return hr;
}

HRESULT EngineDX::CreateOffscreenTarget(UINT width, UINT height,
    ID3D11Texture2D** ppTex, ID3D11RenderTargetView** ppRTV, ID3D11ShaderResourceView** ppSRV)
{
    if (!ppTex || !ppRTV || !ppSRV) return E_POINTER;

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, ppTex);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateRenderTargetView(*ppTex, nullptr, ppRTV);
    if (FAILED(hr)) { (*ppTex)->Release(); return hr; }

    hr = m_device->CreateShaderResourceView(*ppTex, nullptr, ppSRV);
    if (FAILED(hr))
    {
        (*ppRTV)->Release();
        (*ppTex)->Release();
        return hr;
    }

    return S_OK;
}

void EngineDX::SetClearColor(const Rgba& color) { m_clearColor = color; }
void EngineDX::SetClearDepth(float depth) { m_clearDepth = depth; }
void EngineDX::SetClearStencil(UINT8 stencil) { m_clearStencil = stencil; }

void EngineDX::EnableDepthTest(bool enable)
{
    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = enable ? TRUE : FALSE;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    m_depthStencilState.Release();
    m_device->CreateDepthStencilState(&dsd, &m_depthStencilState);
}

void EngineDX::EnableBlending(bool enable)
{
    D3D11_BLEND_DESC bd = {};
    bd.RenderTarget[0].BlendEnable = enable ? TRUE : FALSE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    m_blendState.Release();
    m_device->CreateBlendState(&bd, &m_blendState);
}

void EngineDX::EnableBackfaceCulling(bool enable)
{
    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = enable ? D3D11_CULL_BACK : D3D11_CULL_NONE;
    rd.FrontCounterClockwise = FALSE;
    rd.DepthClipEnable = TRUE;
    m_rasterizerState.Release();
    m_device->CreateRasterizerState(&rd, &m_rasterizerState);
}

void EngineDX::SetViewProjection(const Matrix4f& view, const Matrix4f& proj)
{
}

// ============================================================================
// PreviewDX
// ============================================================================
PreviewDX::PreviewDX() = default;
PreviewDX::~PreviewDX() { Shutdown(); }

HRESULT PreviewDX::Initialize(const EngineDesc& desc)
{
    HRESULT hr = EngineDX::Initialize(desc);
    if (FAILED(hr)) return hr;

    return CreatePreviewResources(desc.width, desc.height);
}

void PreviewDX::Shutdown()
{
    ReleasePreviewResources();
    EngineDX::Shutdown();
}

HRESULT PreviewDX::CreatePreviewResources(UINT width, UINT height)
{
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &m_previewTexture);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateShaderResourceView(m_previewTexture, nullptr, &m_previewSRV);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateRenderTargetView(m_previewTexture, nullptr, &m_previewRTV);
    return hr;
}

void PreviewDX::ReleasePreviewResources()
{
    m_previewRTV.Release();
    m_previewSRV.Release();
    m_previewTexture.Release();
}

HRESULT PreviewDX::RenderFrame(double time)
{
    if (!m_previewRTV) return E_FAIL;

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_immediateContext->ClearRenderTargetView(m_previewRTV, clearColor);

    m_immediateContext->OMSetRenderTargets(1, &m_previewRTV.p, nullptr);

    if (m_previewCallback && m_previewTexture)
        m_previewCallback(m_previewTexture);

    return S_OK;
}

HRESULT PreviewDX::SetPreviewSize(UINT width, UINT height)
{
    if (width == m_width && height == m_height) return S_OK;

    m_width = width;
    m_height = height;

    ReleasePreviewResources();
    return CreatePreviewResources(width, height);
}

// ============================================================================
// DefaultPreviewDX
// ============================================================================
DefaultPreviewDX::DefaultPreviewDX() = default;
DefaultPreviewDX::~DefaultPreviewDX() { Shutdown(); }

HRESULT DefaultPreviewDX::Initialize(const EngineDesc& desc)
{
    return PreviewDX::Initialize(desc);
}

void DefaultPreviewDX::Shutdown()
{
    PreviewDX::Shutdown();
}

void DefaultPreviewDX::RenderBackground()
{
    if (!m_previewRTV) return;
    const float clearCol[4] = { m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, m_backgroundColor.w };
    m_immediateContext->ClearRenderTargetView(m_previewRTV, clearCol);
}

// ============================================================================
// EncodeDX
// ============================================================================
EncodeDX::EncodeDX() = default;
EncodeDX::~EncodeDX() { Shutdown(); }

HRESULT EncodeDX::Initialize(const EngineDesc& desc)
{
    m_outputWidth = desc.width;
    m_outputHeight = desc.height;
    return EngineDX::Initialize(desc);
}

void EncodeDX::Shutdown()
{
    ReleaseEncodeResources();
    EngineDX::Shutdown();
}

HRESULT EncodeDX::BeginEncode(UINT width, UINT height, UINT fps)
{
    m_outputWidth = width;
    m_outputHeight = height;
    m_encodeFps = fps;
    m_encoding = true;

    HRESULT hr = CreateEncodeResources();
    if (FAILED(hr)) { m_encoding = false; return hr; }

    return S_OK;
}

HRESULT EncodeDX::EndEncode()
{
    m_encoding = false;
    ReleaseEncodeResources();
    return S_OK;
}

HRESULT EncodeDX::CreateEncodeResources()
{
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = m_outputWidth;
    td.Height = m_outputHeight;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &m_renderTarget);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateRenderTargetView(m_renderTarget, nullptr, &m_renderTargetRTV);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateShaderResourceView(m_renderTarget, nullptr, &m_renderTargetSRV);
    if (FAILED(hr)) return hr;

    hr = CreateStagingTexture(m_outputWidth, m_outputHeight);
    return hr;
}

void EncodeDX::ReleaseEncodeResources()
{
    m_stagingTexture.Release();
    m_renderTargetSRV.Release();
    m_renderTargetRTV.Release();
    m_renderTarget.Release();
}

HRESULT EncodeDX::CreateStagingTexture(UINT width, UINT height)
{
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_STAGING;
    td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    return m_device->CreateTexture2D(&td, nullptr, &m_stagingTexture);
}

HRESULT EncodeDX::RenderFrame(double time)
{
    if (!m_encoding || !m_renderTargetRTV) return E_FAIL;

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_immediateContext->ClearRenderTargetView(m_renderTargetRTV, clearColor);
    m_immediateContext->OMSetRenderTargets(1, &m_renderTargetRTV.p, nullptr);

    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)m_outputWidth;
    vp.Height = (FLOAT)m_outputHeight;
    vp.MaxDepth = 1.0f;
    m_immediateContext->RSSetViewports(1, &vp);

    return S_OK;
}

HRESULT EncodeDX::ReadBackBuffer(ID3D11Texture2D** ppTexture)
{
    if (!m_renderTarget || !ppTexture) return E_POINTER;
    m_immediateContext->CopyResource(m_stagingTexture, m_renderTarget);
    *ppTexture = m_stagingTexture;
    (*ppTexture)->AddRef();
    return S_OK;
}

HRESULT EncodeDX::MapBuffer(void** ppData, UINT* pRowPitch)
{
    if (!m_stagingTexture || !ppData || !pRowPitch) return E_POINTER;

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = m_immediateContext->Map(m_stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) return hr;

    *ppData = mapped.pData;
    *pRowPitch = mapped.RowPitch;
    return S_OK;
}

void EncodeDX::UnmapBuffer()
{
    if (m_stagingTexture)
    m_immediateContext->Unmap(m_stagingTexture, 0);
}

// ============================================================================
// DeviceClientDXImpl
// ============================================================================
DeviceClientDXImpl::DeviceClientDXImpl() = default;

DeviceClientDXImpl::~DeviceClientDXImpl()
{
    ReleaseDevice();
}

HRESULT DeviceClientDXImpl::InitializeDevice(ID3D11Device* dev, ID3D11DeviceContext* ctx)
{
    m_device = dev;
    m_context = ctx;
    return S_OK;
}

void DeviceClientDXImpl::ReleaseDevice()
{
    m_device = nullptr;
    m_context = nullptr;
}

HRESULT DeviceClientDXImpl::OnDeviceLost()
{
    ReleaseDevice();
    return S_OK;
}

HRESULT DeviceClientDXImpl::OnDeviceRestored(ID3D11Device* dev, ID3D11DeviceContext* ctx)
{
    return InitializeDevice(dev, ctx);
}

// ============================================================================
// SnapShotDX
// ============================================================================
SnapShotDX::SnapShotDX() = default;
SnapShotDX::~SnapShotDX() = default;

HRESULT SnapShotDX::CreateStagingTexture(UINT width, UINT height, DXGI_FORMAT format)
{
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = format;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_STAGING;
    td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    return m_device->CreateTexture2D(&td, nullptr, &m_stagingTexture);
}

HRESULT SnapShotDX::Capture(ID3D11Texture2D* pSource, const std::wstring& filePath)
{
    if (!pSource || !m_device) return E_POINTER;

    D3D11_TEXTURE2D_DESC srcDesc;
    pSource->GetDesc(&srcDesc);

    HRESULT hr = CreateStagingTexture(srcDesc.Width, srcDesc.Height, srcDesc.Format);
    if (FAILED(hr)) return hr;

    m_context->CopyResource(m_stagingTexture, pSource);

    CComPtr<IWICImagingFactory> wicFactory;
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory), (void**)&wicFactory);
    if (FAILED(hr)) return hr;

    CComPtr<IWICBitmapEncoder> encoder;
    hr = wicFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
    if (FAILED(hr)) return hr;

    CComPtr<IStream> fileStream;
    hr = SHCreateStreamOnFileEx(filePath.c_str(), STGM_WRITE | STGM_CREATE, FALSE, FALSE, nullptr, &fileStream);
    if (FAILED(hr)) return hr;

    hr = encoder->Initialize(fileStream, WICBitmapEncoderNoCache);
    if (FAILED(hr)) return hr;

    CComPtr<IWICBitmapFrameEncode> frame;
    CComPtr<IPropertyBag2> props;
    hr = encoder->CreateNewFrame(&frame, &props);
    if (FAILED(hr)) return hr;

    hr = frame->Initialize(props);
    if (FAILED(hr)) return hr;

    frame->SetSize(srcDesc.Width, srcDesc.Height);

    WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
    frame->SetPixelFormat(&fmt);

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = m_context->Map(m_stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) return hr;

    hr = frame->WritePixels(srcDesc.Height, mapped.RowPitch,
        srcDesc.Height * mapped.RowPitch, (BYTE*)mapped.pData);

    m_context->Unmap(m_stagingTexture, 0);

    if (SUCCEEDED(hr)) hr = frame->Commit();
    if (SUCCEEDED(hr)) hr = encoder->Commit();

    m_stagingTexture.Release();
    return hr;
}

HRESULT SnapShotDX::CaptureFromRTV(ID3D11RenderTargetView* pRTV, const std::wstring& filePath)
{
    if (!pRTV || !m_device) return E_POINTER;

    CComPtr<ID3D11Resource> res;
    pRTV->GetResource(&res);

    CComPtr<ID3D11Texture2D> tex;
    HRESULT hr = res->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex);
    if (FAILED(hr)) return hr;

    return Capture(tex, filePath);
}

HRESULT SnapShotDX::CaptureScreen(ID3D11Texture2D** ppOut)
{
    if (!ppOut) return E_POINTER;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = 1;
    desc.Height = 1;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, ppOut);
    return hr;
}

} // namespace DX
} // namespace HMREngine
