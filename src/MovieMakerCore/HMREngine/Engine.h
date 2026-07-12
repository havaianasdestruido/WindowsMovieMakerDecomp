#pragma once
// Engine.h - HMREngine::Engine class

#include "HMREngine.h"
#include "X3DMath.h"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include <atlbase.h>
#include <atlcom.h>
#include <string>
#include <memory>
#include <functional>

namespace HMREngine
{
    class Scene;
    class Browser;
    class ExecutionContext;
    class RenderLoop;
    class ResourceCache;
    class ErrHandler;

    struct EngineDesc
    {
        HWND hWnd = nullptr;
        UINT width = 1920;
        UINT height = 1080;
        bool windowed = true;
        bool vsync = true;
        UINT adapterIndex = 0;
        D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_10_0;
        D3D_FEATURE_LEVEL* pFeatureLevels = nullptr;
        UINT numFeatureLevels = 0;
    };

    struct RenderStats
    {
        double frameTime = 0.0;
        double cpuTime = 0.0;
        double gpuTime = 0.0;
        UINT drawCalls = 0;
        UINT triangles = 0;
        UINT vertices = 0;
        UINT textureBinds = 0;
        UINT shaderSwitches = 0;
        UINT framesPerSecond = 0;
    };

    class Engine
    {
    public:
        Engine();
        virtual ~Engine();

        // Initialization / Shutdown
        HRESULT Initialize(const EngineDesc& desc);
        HRESULT InitializeDevice(const EngineDesc& desc);
        HRESULT InitializeSwapChain(const EngineDesc& desc);
        HRESULT InitializeRenderTargets();
        HRESULT InitializeDepthBuffer();
        HRESULT InitializeViewport();
        void Shutdown();

        // Scene management
        Scene* GetScene() const { return m_scene; }
        HRESULT SetScene(Scene* scene);

        // Browser
        Browser* GetBrowser() const { return m_browser; }
        void SetBrowser(Browser* browser);

        // Execution context
        ExecutionContext* GetExecutionContext() const { return m_context; }

        // Resource cache
        ResourceCache* GetResourceCache() const { return m_resourceCache; }

        // Render loop
        RenderLoop* GetRenderLoop() const { return m_renderLoop; }

        // Device access
        ID3D11Device* GetDevice() const { return m_device; }
        ID3D11DeviceContext* GetImmediateContext() const { return m_immediateContext; }
        IDXGISwapChain* GetSwapChain() const { return m_swapChain; }

        // Render targets
        ID3D11RenderTargetView* GetBackBufferRTV() const { return m_backBufferRTV; }
        ID3D11DepthStencilView* GetDepthStencilView() const { return m_depthStencilView; }
        ID3D11Texture2D* GetDepthStencilBuffer() const { return m_depthStencilBuffer; }

        // Frame management
        HRESULT BeginFrame();
        HRESULT EndFrame();
        HRESULT Present();
        HRESULT Resize(UINT width, UINT height);
        HRESULT ResetDevice();

        // Render targets
        HRESULT CreateRenderTarget(UINT width, UINT height, ID3D11RenderTargetView** ppRTV);
        HRESULT CreateOffscreenTarget(UINT width, UINT height,
            ID3D11Texture2D** ppTex, ID3D11RenderTargetView** ppRTV, ID3D11ShaderResourceView** ppSRV);

        // State
        void SetClearColor(const Rgba& color);
        void SetClearDepth(float depth);
        void SetClearStencil(UINT8 stencil);
        void EnableDepthTest(bool enable);
        void EnableBlending(bool enable);
        void EnableBackfaceCulling(bool enable);

        // Stats
        const RenderStats& GetStats() const { return m_stats; }
        void ResetStats();

        // Error handler
        ErrHandler* GetErrorHandler() const { return m_errorHandler; }

        // D3D feature
        D3D_FEATURE_LEVEL GetFeatureLevel() const { return m_featureLevel; }

        // Window
        HWND GetHWND() const { return m_hWnd; }
        UINT GetWidth() const { return m_width; }
        UINT GetHeight() const { return m_height; }

    protected:
        HWND m_hWnd = nullptr;
        UINT m_width = 0;
        UINT m_height = 0;

        CComPtr<ID3D11Device> m_device;
        CComPtr<ID3D11DeviceContext> m_immediateContext;
        CComPtr<IDXGISwapChain> m_swapChain;
        CComPtr<IDXGIAdapter> m_adapter;
        CComPtr<IDXGIFactory> m_factory;

        CComPtr<ID3D11RenderTargetView> m_backBufferRTV;
        CComPtr<ID3D11Texture2D> m_depthStencilBuffer;
        CComPtr<ID3D11DepthStencilView> m_depthStencilView;
        CComPtr<ID3D11DepthStencilState> m_depthStencilState;
        CComPtr<ID3D11BlendState> m_blendState;
        CComPtr<ID3D11RasterizerState> m_rasterizerState;

        D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_10_0;

        Scene* m_scene = nullptr;
        Browser* m_browser = nullptr;
        ExecutionContext* m_context = nullptr;
        RenderLoop* m_renderLoop = nullptr;
        ResourceCache* m_resourceCache = nullptr;
        ErrHandler* m_errorHandler = nullptr;

        RenderStats m_stats{};
        Rgba m_clearColor = Rgba(0.0f, 0.0f, 0.0f, 1.0f);
        float m_clearDepth = 1.0f;
        UINT8 m_clearStencil = 0;
        bool m_vsync = true;
        bool m_deviceLost = false;

        HRESULT HandleDeviceLost();
        HRESULT CreateDepthStencilState();
        HRESULT CreateBlendState();
        HRESULT CreateRasterizerState();
    };

} // namespace HMREngine
