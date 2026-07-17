#include "pch.h"

// RenderLoop.cpp - HMREngine::RenderLoop implementation

#include "RenderLoop.h"
#include "Engine.h"
#include "Scene.h"
#include "ExecutionContext.h"
#include "RenderingList.h"

namespace HMREngine
{
    RenderLoop::RenderLoop(Engine* engine)
        : m_engine(engine)
    {
        QueryPerformanceFrequency(&m_perfFrequency);
        QueryPerformanceCounter(&m_lastPerfCounter);
        m_lastTime = GetTime();
    }

    RenderLoop::~RenderLoop()
    {
        Stop();
    }

    double RenderLoop::GetTime() const
    {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return static_cast<double>(counter.QuadPart) / m_perfFrequency.QuadPart;
    }

    void RenderLoop::UpdateTiming()
    {
        double now = GetTime();
        m_deltaTime = now - m_lastTime;
        m_lastTime = now;

        // FPS calculation
        m_frameAccumulator += m_deltaTime;
        m_frameCount++;

        if (m_frameAccumulator >= 1.0)
        {
            m_fps = m_frameCount / m_frameAccumulator;
            m_frameCount = 0;
            m_frameAccumulator = 0.0;
        }

        // Frame rate limiting
        double frameInterval = 1.0 / m_targetFPS;
        if (m_deltaTime < frameInterval)
        {
            double sleepTime = frameInterval - m_deltaTime;
            if (sleepTime > 0.001)
            {
                LARGE_INTEGER start, end;
                QueryPerformanceCounter(&start);
                double target = start.QuadPart + static_cast<LONGLONG>(sleepTime * m_perfFrequency.QuadPart);
                do
                {
                    QueryPerformanceCounter(&end);
                } while (end.QuadPart < target);
            }
        }
    }

    void RenderLoop::Start()
    {
        m_running = true;
    }

    void RenderLoop::Stop()
    {
        m_running = false;
    }

    HRESULT RenderLoop::RenderFrame()
    {
        if (m_paused) return S_OK;
        if (!m_engine) return E_POINTER;

        UpdateTiming();

        ExecutionContext* ctx = m_engine->GetExecutionContext();
        if (ctx)
        {
            ctx->SetCurrentTime(GetTime());
            ctx->SetDeltaTime(m_deltaTime);
            ctx->SetFrameNumber(ctx->GetFrameNumber() + 1);
        }

        if (m_preFrameCb) m_preFrameCb(GetTime(), m_deltaTime);

        Scene* scene = m_engine->GetScene();
        if (scene)
        {
            scene->Update(m_deltaTime);
        }

        HRESULT hr = m_engine->BeginFrame();
        if (FAILED(hr)) return hr;

        if (ctx) ctx->ExecutePreRender();

        if (scene)
        {
            RenderingList renderList;
            renderList.Build(scene);
            renderList.Execute(m_engine);
        }

        if (ctx) ctx->ExecutePostRender();

        m_engine->EndFrame();
        hr = m_engine->Present();

        if (m_frameCb) m_frameCb(GetTime(), m_deltaTime);
        if (m_postFrameCb) m_postFrameCb(GetTime(), m_deltaTime);

        return hr;
    }

    HRESULT RenderLoop::RenderSceneToTexture()
    {
        if (!m_engine) return E_POINTER;

        ExecutionContext* ctx = m_engine->GetExecutionContext();
        if (ctx)
        {
            ctx->SetCurrentTime(GetTime());
            ctx->SetDeltaTime(0.0);
        }

        Scene* scene = m_engine->GetScene();
        if (!scene) return E_FAIL;

        scene->Update(0.0);

        ID3D11Device* dev = m_engine->GetDevice();
        ID3D11DeviceContext* d3dctx = m_engine->GetImmediateContext();
        if (!dev || !d3dctx) return E_FAIL;

        UINT w = m_engine->GetWidth();
        UINT h = m_engine->GetHeight();

        D3D11_TEXTURE2D_DESC td{};
        td.Width = w;
        td.Height = h;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        CComPtr<ID3D11Texture2D> offTex;
        HRESULT hr = dev->CreateTexture2D(&td, nullptr, &offTex);
        if (FAILED(hr)) return hr;

        CComPtr<ID3D11RenderTargetView> offRTV;
        hr = dev->CreateRenderTargetView(offTex, nullptr, &offRTV);
        if (FAILED(hr)) return hr;

        CComPtr<ID3D11ShaderResourceView> offSRV;
        hr = dev->CreateShaderResourceView(offTex, nullptr, &offSRV);
        if (FAILED(hr)) return hr;

        D3D11_TEXTURE2D_DESC dsd{};
        dsd.Width = w;
        dsd.Height = h;
        dsd.MipLevels = 1;
        dsd.ArraySize = 1;
        dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsd.SampleDesc.Count = 1;
        dsd.Usage = D3D11_USAGE_DEFAULT;
        dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        CComPtr<ID3D11Texture2D> offDS;
        hr = dev->CreateTexture2D(&dsd, nullptr, &offDS);
        if (FAILED(hr)) return hr;

        CComPtr<ID3D11DepthStencilView> offDSV;
        hr = dev->CreateDepthStencilView(offDS, nullptr, &offDSV);
        if (FAILED(hr)) return hr;

        CComPtr<ID3D11RenderTargetView> oldRTV;
        CComPtr<ID3D11DepthStencilView> oldDSV;
        d3dctx->OMGetRenderTargets(1, &oldRTV.p, &oldDSV.p);

        d3dctx->OMSetRenderTargets(1, &offRTV.p, offDSV);

        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        d3dctx->ClearRenderTargetView(offRTV, clearColor);
        d3dctx->ClearDepthStencilView(offDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(w);
        vp.Height = static_cast<float>(h);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        d3dctx->RSSetViewports(1, &vp);

        RenderingList renderList;
        renderList.Build(scene);
        renderList.Execute(m_engine);

        d3dctx->OMSetRenderTargets(1, &oldRTV.p, oldDSV);

        return S_OK;
    }

} // namespace HMREngine
