#pragma once
// EngineDX.h - D3D11 device management, feature level detection, device lost recovery

#include "DXResources.h"

namespace HMREngine
{
    namespace DX
    {
        // --- EngineDX: Core D3D11 device management ---
        class EngineDX
        {
        public:
            EngineDX();
            virtual ~EngineDX();

            HRESULT Initialize(const EngineDesc& desc);
            HRESULT InitializeDevice(const EngineDesc& desc);
            HRESULT InitializeSwapChain(const EngineDesc& desc);
            HRESULT InitializeRenderTargets();
            HRESULT InitializeDepthBuffer();
            HRESULT InitializeViewport();
            void Shutdown();

            // Device access
            ID3D11Device* GetDevice() const { return m_device; }
            ID3D11DeviceContext* GetImmediateContext() const { return m_immediateContext; }
            IDXGISwapChain* GetSwapChain() const { return m_swapChain; }
            IDXGIFactory* GetFactory() const { return m_factory; }

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

            // Render target creation
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
            void SetViewProjection(const Matrix4f& view, const Matrix4f& proj);

            // Features
            D3D_FEATURE_LEVEL GetFeatureLevel() const { return m_featureLevel; }
            const DeviceFeatures& GetFeatures() const { return m_features; }
            bool IsDeviceLost() const { return m_deviceLost; }

            // Window
            HWND GetHWND() const { return m_hWnd; }
            UINT GetWidth() const { return m_width; }
            UINT GetHeight() const { return m_height; }
            float GetAspectRatio() const { return m_height > 0 ? (float)m_width / (float)m_height : 1.0f; }

            // Error handler
            void SetErrorHandler(ErrHandler* handler) { m_errorHandler = handler; }

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
            DeviceFeatures m_features{};

            ErrHandler* m_errorHandler = nullptr;
            Rgba m_clearColor = Rgba(0.0f, 0.0f, 0.0f, 1.0f);
            float m_clearDepth = 1.0f;
            UINT8 m_clearStencil = 0;
            bool m_vsync = true;
            bool m_deviceLost = false;
            bool m_allowTearing = false;

            virtual HRESULT HandleDeviceLost();
            HRESULT CreateDepthStencilState();
            HRESULT CreateBlendState();
            HRESULT CreateRasterizerState();
            HRESULT QueryDeviceFeatures();
        };

        // --- PreviewDX: Engine context for real-time preview ---
        class PreviewDX : public EngineDX
        {
        public:
            PreviewDX();
            virtual ~PreviewDX();

            HRESULT Initialize(const EngineDesc& desc);
            void Shutdown();

            HRESULT RenderFrame(double time);
            HRESULT SetPreviewSize(UINT width, UINT height);
            void SetPreviewCallback(std::function<void(ID3D11Texture2D*)> cb) { m_previewCallback = cb; }

            ID3D11Texture2D* GetPreviewTexture() const { return m_previewTexture; }
            ID3D11ShaderResourceView* GetPreviewSRV() const { return m_previewSRV; }

        protected:
            CComPtr<ID3D11Texture2D> m_previewTexture;
            CComPtr<ID3D11ShaderResourceView> m_previewSRV;
            CComPtr<ID3D11RenderTargetView> m_previewRTV;
            std::function<void(ID3D11Texture2D*)> m_previewCallback;

            HRESULT CreatePreviewResources(UINT width, UINT height);
            void ReleasePreviewResources();
        };

        // --- DefaultPreviewDX: Default preview engine ---
        class DefaultPreviewDX : public PreviewDX
        {
        public:
            DefaultPreviewDX();
            virtual ~DefaultPreviewDX();

            HRESULT Initialize(const EngineDesc& desc);
            void Shutdown();

            void SetBackground(const Rgba& color) { m_backgroundColor = color; }
            const Rgba& GetBackground() const { return m_backgroundColor; }

        protected:
            Rgba m_backgroundColor = Rgba(0.15f, 0.15f, 0.15f, 1.0f);

            void RenderBackground();
        };

        // --- EncodeDX: Engine context for encoding/rendering to file ---
        class EncodeDX : public EngineDX
        {
        public:
            EncodeDX();
            virtual ~EncodeDX();

            HRESULT Initialize(const EngineDesc& desc);
            void Shutdown();

            HRESULT BeginEncode(UINT width, UINT height, UINT fps);
            HRESULT EndEncode();
            HRESULT RenderFrame(double time);

            HRESULT ReadBackBuffer(ID3D11Texture2D** ppTexture);
            HRESULT MapBuffer(void** ppData, UINT* pRowPitch);
            void UnmapBuffer();

            void SetOutputSize(UINT width, UINT height) { m_outputWidth = width; m_outputHeight = height; }
            UINT GetOutputWidth() const { return m_outputWidth; }
            UINT GetOutputHeight() const { return m_outputHeight; }

        protected:
            CComPtr<ID3D11Texture2D> m_stagingTexture;
            CComPtr<ID3D11Texture2D> m_renderTarget;
            CComPtr<ID3D11RenderTargetView> m_renderTargetRTV;
            CComPtr<ID3D11ShaderResourceView> m_renderTargetSRV;
            UINT m_outputWidth = 1920;
            UINT m_outputHeight = 1080;
            UINT m_encodeFps = 30;
            bool m_encoding = false;

            HRESULT CreateEncodeResources();
            void ReleaseEncodeResources();
            HRESULT CreateStagingTexture(UINT width, UINT height);
        };

        // --- DeviceClientDXImpl: Base for device-dependent resources ---
        class DeviceClientDXImpl
        {
        public:
            DeviceClientDXImpl();
            virtual ~DeviceClientDXImpl();

            virtual HRESULT InitializeDevice(ID3D11Device* dev, ID3D11DeviceContext* ctx);
            virtual void ReleaseDevice();

            virtual HRESULT OnDeviceLost();
            virtual HRESULT OnDeviceRestored(ID3D11Device* dev, ID3D11DeviceContext* ctx);

            ID3D11Device* GetDevice() const { return m_device; }
            ID3D11DeviceContext* GetContext() const { return m_context; }
            bool HasDevice() const { return m_device != nullptr; }

        protected:
            ID3D11Device* m_device = nullptr;
            ID3D11DeviceContext* m_context = nullptr;
            EngineDX* m_engine = nullptr;
        };

        // --- SnapShotDX: Single-frame capture ---
        class SnapShotDX : public DeviceClientDXImpl
        {
        public:
            SnapShotDX();
            virtual ~SnapShotDX();

            HRESULT Capture(ID3D11Texture2D* pSource, const std::wstring& filePath);
            HRESULT CaptureFromRTV(ID3D11RenderTargetView* pRTV, const std::wstring& filePath);
            HRESULT CaptureScreen(ID3D11Texture2D** ppOut);

        protected:
            CComPtr<ID3D11Texture2D> m_stagingTexture;
            HRESULT CreateStagingTexture(UINT width, UINT height, DXGI_FORMAT format);
        };

    } // namespace DX
} // namespace HMREngine
