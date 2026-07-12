#pragma once
// RenderLoopDX.h - D3D11 render loop, frame buffer, swap chain, back buffer

#include "DXResources.h"

namespace HMREngine
{
    namespace DX
    {
        // --- RenderLoopDX: D3D11 render loop implementation ---
        class RenderLoopDX
        {
        public:
            RenderLoopDX(EngineDX* engine);
            virtual ~RenderLoopDX();

            void Start();
            void Stop();
            bool IsRunning() const { return m_running; }

            HRESULT RenderFrame();
            HRESULT RenderSceneToTexture();

            double GetTime() const;
            double GetDeltaTime() const { return m_deltaTime; }
            double GetFPS() const { return m_fps; }

            void SetTargetFPS(double fps) { m_targetFPS = fps; }
            double GetTargetFPS() const { return m_targetFPS; }

            void SetPaused(bool p) { m_paused = p; }
            bool IsPaused() const { return m_paused; }

            using PreFrameCallback = std::function<void(double, double)>;
            using PostFrameCallback = std::function<void(double, double)>;
            void SetPreFrameCallback(PreFrameCallback cb) { m_preFrameCb = cb; }
            void SetPostFrameCallback(PostFrameCallback cb) { m_postFrameCb = cb; }

        protected:
            EngineDX* m_engine = nullptr;
            std::atomic<bool> m_running{ false };
            std::atomic<bool> m_paused{ false };

            double m_lastTime = 0.0;
            double m_deltaTime = 0.0;
            double m_fps = 0.0;
            double m_targetFPS = 60.0;
            double m_frameAccumulator = 0.0;
            int m_frameCount = 0;
            double m_fpsTimer = 0.0;

            LARGE_INTEGER m_perfFrequency{};
            LARGE_INTEGER m_lastPerfCounter{};

            PreFrameCallback m_preFrameCb;
            PostFrameCallback m_postFrameCb;

            void UpdateTiming();
        };

        // --- FrameBufferImplDX: Frame buffer management ---
        class FrameBufferImplDX : public DeviceClientDXImpl
        {
        public:
            FrameBufferImplDX();
            virtual ~FrameBufferImplDX();

            HRESULT Create(UINT width, UINT height,
                DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM,
                bool depthStencil = false);
            void Release();

            HRESULT Resize(UINT width, UINT height);

            void Clear(const Rgba& clearColor = Rgba(0, 0, 0, 1));
            void ClearDepth(float depth = 1.0f, UINT8 stencil = 0);

            HRESULT Bind(ID3D11DeviceContext* ctx);
            void Unbind(ID3D11DeviceContext* ctx);

            HRESULT GetRTV(ID3D11RenderTargetView** ppRTV) const;
            HRESULT GetDSV(ID3D11DepthStencilView** ppDSV) const;
            HRESULT GetSRV(ID3D11ShaderResourceView** ppSRV) const;
            HRESULT GetTexture(ID3D11Texture2D** ppTex) const;

            UINT GetWidth() const { return m_width; }
            UINT GetHeight() const { return m_height; }
            DXGI_FORMAT GetFormat() const { return m_format; }

        protected:
            CComPtr<ID3D11Texture2D> m_texture;
            CComPtr<ID3D11RenderTargetView> m_rtv;
            CComPtr<ID3D11ShaderResourceView> m_srv;
            CComPtr<ID3D11Texture2D> m_depthStencil;
            CComPtr<ID3D11DepthStencilView> m_dsv;
            UINT m_width = 0;
            UINT m_height = 0;
            DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
            bool m_hasDepthStencil = false;
        };

        // --- SharedSwapChainDX: Shared swap chain for multi-window ---
        class SharedSwapChainDX
        {
        public:
            SharedSwapChainDX();
            ~SharedSwapChainDX();

            HRESULT Initialize(ID3D11Device* dev, IDXGIFactory* factory,
                HWND hWnd, UINT width, UINT height,
                DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM);
            void Release();

            HRESULT Resize(UINT width, UINT height);
            HRESULT Present(UINT syncInterval = 1);

            HRESULT GetBackBuffer(ID3D11Texture2D** ppTex) const;
            HRESULT GetBackBufferRTV(ID3D11RenderTargetView** ppRTV) const;

            IDXGISwapChain* GetSwapChain() const { return m_swapChain; }
            UINT GetWidth() const { return m_width; }
            UINT GetHeight() const { return m_height; }

            void SetVSync(bool vsync) { m_vsync = vsync; }

        protected:
            CComPtr<IDXGISwapChain> m_swapChain;
            CComPtr<ID3D11Texture2D> m_backBuffer;
            CComPtr<ID3D11RenderTargetView> m_rtv;
            ID3D11Device* m_device = nullptr;
            UINT m_width = 0;
            UINT m_height = 0;
            DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
            bool m_vsync = true;

            HRESULT CreateRTV();
            void ReleaseRTV();
        };

        // --- BackBufferDX: Back buffer wrapper ---
        class BackBufferDX
        {
        public:
            BackBufferDX();
            ~BackBufferDX();

            HRESULT Initialize(IDXGISwapChain* swapChain, ID3D11Device* dev);
            void Release();

            HRESULT GetTexture(ID3D11Texture2D** ppTex) const;
            HRESULT GetRTV(ID3D11RenderTargetView** ppRTV) const;
            HRESULT GetSRV(ID3D11ShaderResourceView** ppSRV) const;

            UINT GetWidth() const { return m_width; }
            UINT GetHeight() const { return m_height; }

        protected:
            CComPtr<ID3D11Texture2D> m_texture;
            CComPtr<ID3D11RenderTargetView> m_rtv;
            CComPtr<ID3D11ShaderResourceView> m_srv;
            UINT m_width = 0;
            UINT m_height = 0;
        };

    } // namespace DX
} // namespace HMREngine
