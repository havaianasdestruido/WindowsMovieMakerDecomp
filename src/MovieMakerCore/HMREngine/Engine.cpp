#include "pch.h"

// Engine.cpp - HMREngine::Engine implementation

#include "Engine.h"
#include "Scene.h"
#include "Browser.h"
#include "ExecutionContext.h"
#include "RenderLoop.h"
#include "ResourceCache.h"
#include "ErrHandler.h"
#include <dxgi.h>

namespace HMREngine
{
    Engine::Engine()
    {
        m_errorHandler = new ErrHandler();
        m_resourceCache = new ResourceCache();
        m_context = new ExecutionContext();
    }

    Engine::~Engine()
    {
        Shutdown();
        delete m_context;
        delete m_resourceCache;
        delete m_errorHandler;
    }

    HRESULT Engine::Initialize(const EngineDesc& desc)
    {
        m_hWnd = desc.hWnd;
        m_width = desc.width;
        m_height = desc.height;
        m_vsync = desc.vsync;

        HRESULT hr = InitializeDevice(desc);
        if (FAILED(hr)) return hr;

        hr = InitializeSwapChain(desc);
        if (FAILED(hr)) return hr;

        hr = InitializeRenderTargets();
        if (FAILED(hr)) return hr;

        hr = InitializeDepthBuffer();
        if (FAILED(hr)) return hr;

        hr = CreateDepthStencilState();
        if (FAILED(hr)) return hr;

        hr = CreateBlendState();
        if (FAILED(hr)) return hr;

        hr = CreateRasterizerState();
        if (FAILED(hr)) return hr;

        InitializeViewport();

        m_renderLoop = new RenderLoop(this);

        return S_OK;
    }

    HRESULT Engine::InitializeDevice(const EngineDesc& desc)
    {
        UINT createFlags = 0;
#ifdef _DEBUG
        createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = sizeof(featureLevels) / sizeof(featureLevels[0]);

        D3D_FEATURE_LEVEL achievedLevel;
        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createFlags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &m_device,
            &achievedLevel,
            &m_immediateContext
        );

        if (FAILED(hr))
        {
            hr = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_WARP,
                nullptr,
                createFlags,
                featureLevels,
                numFeatureLevels,
                D3D11_SDK_VERSION,
                &m_device,
                &achievedLevel,
                &m_immediateContext
            );
        }

        if (SUCCEEDED(hr))
        {
            m_featureLevel = achievedLevel;
        }

        return hr;
    }

    HRESULT Engine::InitializeSwapChain(const EngineDesc& desc)
    {
        CComPtr<IDXGIDevice> dxgiDevice;
        HRESULT hr = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
        if (FAILED(hr)) return hr;

        hr = dxgiDevice->GetAdapter(&m_adapter);
        if (FAILED(hr)) return hr;

        hr = m_adapter->GetParent(__uuidof(IDXGIFactory), (void**)&m_factory);
        if (FAILED(hr)) return hr;

        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferCount = 2;
        sd.BufferDesc.Width = m_width;
        sd.BufferDesc.Height = m_height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = m_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = desc.windowed ? TRUE : FALSE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        hr = m_factory->CreateSwapChain(m_device, &sd, &m_swapChain);
        if (FAILED(hr)) return hr;

        m_factory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
        return S_OK;
    }

    HRESULT Engine::InitializeRenderTargets()
    {
        CComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        if (FAILED(hr)) return hr;

        hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_backBufferRTV);
        return hr;
    }

    HRESULT Engine::InitializeDepthBuffer()
    {
        D3D11_TEXTURE2D_DESC depthDesc{};
        depthDesc.Width = m_width;
        depthDesc.Height = m_height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        HRESULT hr = m_device->CreateTexture2D(&depthDesc, nullptr, &m_depthStencilBuffer);
        if (FAILED(hr)) return hr;

        hr = m_device->CreateDepthStencilView(m_depthStencilBuffer, nullptr, &m_depthStencilView);
        return hr;
    }

    HRESULT Engine::CreateDepthStencilState()
    {
        D3D11_DEPTH_STENCIL_DESC dsd{};
        dsd.DepthEnable = TRUE;
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        return m_device->CreateDepthStencilState(&dsd, &m_depthStencilState);
    }

    HRESULT Engine::CreateBlendState()
    {
        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable = FALSE;
        bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
        bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        return m_device->CreateBlendState(&bd, &m_blendState);
    }

    HRESULT Engine::CreateRasterizerState()
    {
        D3D11_RASTERIZER_DESC rd{};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_BACK;
        rd.FrontCounterClockwise = FALSE;
        rd.DepthClipEnable = TRUE;
        return m_device->CreateRasterizerState(&rd, &m_rasterizerState);
    }

    void Engine::InitializeViewport()
    {
        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(m_width);
        vp.Height = static_cast<float>(m_height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        m_immediateContext->RSSetViewports(1, &vp);
    }

    void Engine::Shutdown()
    {
        if (m_renderLoop) { delete m_renderLoop; m_renderLoop = nullptr; }

        if (m_scene) { delete m_scene; m_scene = nullptr; }
        if (m_browser) { delete m_browser; m_browser = nullptr; }

        m_backBufferRTV.Release();
        m_depthStencilView.Release();
        m_depthStencilBuffer.Release();
        m_depthStencilState.Release();
        m_blendState.Release();
        m_rasterizerState.Release();
        m_swapChain.Release();
        m_immediateContext.Release();
        m_device.Release();
        m_factory.Release();
        m_adapter.Release();
    }

    HRESULT Engine::SetScene(Scene* scene)
    {
        m_scene = scene;
        return S_OK;
    }

    void Engine::SetBrowser(Browser* browser)
    {
        m_browser = browser;
    }

    HRESULT Engine::BeginFrame()
    {
        ResetStats();

        float clearColor[4] = { m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w };
        m_immediateContext->ClearRenderTargetView(m_backBufferRTV, clearColor);
        m_immediateContext->ClearDepthStencilView(m_depthStencilView,
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, m_clearDepth, m_clearStencil);

        m_immediateContext->OMSetRenderTargets(1, &m_backBufferRTV.p, m_depthStencilView);
        m_immediateContext->OMSetDepthStencilState(m_depthStencilState, 0);
        m_immediateContext->OMSetBlendState(m_blendState, nullptr, 0xFFFFFFFF);
        m_immediateContext->RSSetState(m_rasterizerState);

        return S_OK;
    }

    HRESULT Engine::EndFrame()
    {
        return S_OK;
    }

    HRESULT Engine::Present()
    {
        HRESULT hr = m_swapChain->Present(m_vsync ? 1 : 0, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            m_deviceLost = true;
            return HandleDeviceLost();
        }

        return hr;
    }

    HRESULT Engine::Resize(UINT width, UINT height)
    {
        if (width == 0 || height == 0 || (width == m_width && height == m_height))
            return S_OK;

        m_immediateContext->OMSetRenderTargets(0, nullptr, nullptr);
        m_backBufferRTV.Release();
        m_depthStencilView.Release();
        m_depthStencilBuffer.Release();

        HRESULT hr = m_swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr)) return hr;

        m_width = width;
        m_height = height;

        hr = InitializeRenderTargets();
        if (FAILED(hr)) return hr;

        hr = InitializeDepthBuffer();
        if (FAILED(hr)) return hr;

        InitializeViewport();
        return S_OK;
    }

    HRESULT Engine::HandleDeviceLost()
    {
        m_backBufferRTV.Release();
        m_depthStencilView.Release();
        m_depthStencilBuffer.Release();
        m_depthStencilState.Release();
        m_blendState.Release();
        m_rasterizerState.Release();
        m_swapChain.Release();
        m_immediateContext.Release();
        m_device.Release();
        m_factory.Release();
        m_adapter.Release();

        EngineDesc desc{};
        desc.hWnd = m_hWnd;
        desc.width = m_width;
        desc.height = m_height;
        desc.vsync = m_vsync;

        m_deviceLost = false;
        return Initialize(desc);
    }

    HRESULT Engine::CreateRenderTarget(UINT width, UINT height, ID3D11RenderTargetView** ppRTV)
    {
        D3D11_TEXTURE2D_DESC td{};
        td.Width = width;
        td.Height = height;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        CComPtr<ID3D11Texture2D> tex;
        HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &tex);
        if (FAILED(hr)) return hr;

        return m_device->CreateRenderTargetView(tex, nullptr, ppRTV);
    }

    HRESULT Engine::CreateOffscreenTarget(UINT width, UINT height,
        ID3D11Texture2D** ppTex, ID3D11RenderTargetView** ppRTV, ID3D11ShaderResourceView** ppSRV)
    {
        D3D11_TEXTURE2D_DESC td{};
        td.Width = width;
        td.Height = height;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        HRESULT hr = m_device->CreateTexture2D(&td, nullptr, ppTex);
        if (FAILED(hr)) return hr;

        hr = m_device->CreateRenderTargetView(*ppTex, nullptr, ppRTV);
        if (FAILED(hr)) return hr;

        hr = m_device->CreateShaderResourceView(*ppTex, nullptr, ppSRV);
        return hr;
    }

    void Engine::SetClearColor(const Rgba& color) { m_clearColor = color; }
    void Engine::SetClearDepth(float depth) { m_clearDepth = depth; }
    void Engine::SetClearStencil(UINT8 stencil) { m_clearStencil = stencil; }

    void Engine::EnableDepthTest(bool enable)
    {
        D3D11_DEPTH_STENCIL_DESC dsd{};
        dsd.DepthEnable = enable ? TRUE : FALSE;
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        m_depthStencilState.Release();
        m_device->CreateDepthStencilState(&dsd, &m_depthStencilState);
        m_immediateContext->OMSetDepthStencilState(m_depthStencilState, 0);
    }

    void Engine::EnableBlending(bool enable)
    {
        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable = enable ? TRUE : FALSE;
        bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        m_blendState.Release();
        m_device->CreateBlendState(&bd, &m_blendState);
        m_immediateContext->OMSetBlendState(m_blendState, nullptr, 0xFFFFFFFF);
    }

    void Engine::EnableBackfaceCulling(bool enable)
    {
        D3D11_RASTERIZER_DESC rd{};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = enable ? D3D11_CULL_BACK : D3D11_CULL_NONE;
        rd.FrontCounterClockwise = FALSE;
        rd.DepthClipEnable = TRUE;
        m_rasterizerState.Release();
        m_device->CreateRasterizerState(&rd, &m_rasterizerState);
        m_immediateContext->RSSetState(m_rasterizerState);
    }

    void Engine::ResetStats()
    {
        m_stats = RenderStats{};
    }

} // namespace HMREngine
