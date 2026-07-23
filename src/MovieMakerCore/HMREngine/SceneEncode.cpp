#include "pch.h"

// SceneEncode.cpp - HMREngine::SceneEncode implementation

#include "SceneEncode.h"
#include "Engine.h"
#include "Scene.h"
#include "ExecutionContext.h"
#include "RenderingList.h"

namespace HMREngine
{
    SceneEncode::SceneEncode()
    {
    }

    SceneEncode::~SceneEncode()
    {
        Shutdown();
    }

    HRESULT SceneEncode::Initialize(Engine* engine)
    {
        m_engine = engine;
        return S_OK;
    }

    void SceneEncode::Shutdown()
    {
        CancelEncode();
        ReleaseFrameBuffer();
        m_engine = nullptr;
    }

    HRESULT SceneEncode::CreateFrameBuffer()
    {
        if (!m_engine) return E_POINTER;
        ID3D11Device* dev = m_engine->GetDevice();
        if (!dev) return E_POINTER;

        D3D11_TEXTURE2D_DESC td{};
        td.Width = m_desc.width;
        td.Height = m_desc.height;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        HRESULT hr = dev->CreateTexture2D(&td, nullptr, &m_frameTexture);
        if (FAILED(hr)) return hr;

        hr = dev->CreateRenderTargetView(m_frameTexture, nullptr, &m_frameRTV);
        if (FAILED(hr)) return hr;

        hr = dev->CreateShaderResourceView(m_frameTexture, nullptr, &m_frameSRV);
        if (FAILED(hr)) return hr;

        D3D11_TEXTURE2D_DESC dsd{};
        dsd.Width = m_desc.width;
        dsd.Height = m_desc.height;
        dsd.MipLevels = 1;
        dsd.ArraySize = 1;
        dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsd.SampleDesc.Count = 1;
        dsd.Usage = D3D11_USAGE_DEFAULT;
        dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        CComPtr<ID3D11Texture2D> dsTex;
        hr = dev->CreateTexture2D(&dsd, nullptr, &m_frameDSTex);
        if (FAILED(hr)) return hr;

        hr = dev->CreateDepthStencilView(m_frameDSTex, nullptr, &m_frameDSV);
        return hr;
    }

    void SceneEncode::ReleaseFrameBuffer()
    {
        m_frameDSV.Release();
        m_frameDSTex.Release();
        m_frameSRV.Release();
        m_frameRTV.Release();
        m_frameTexture.Release();
    }

    HRESULT SceneEncode::BeginEncode(Scene* scene, const EncodeDesc& desc)
    {
        m_desc = desc;
        m_encoding = true;

        HRESULT hr = CreateFrameBuffer();
        if (FAILED(hr)) return hr;

        m_progress.totalTime = desc.endTime - desc.startTime;
        m_progress.totalFrames = static_cast<int>(m_progress.totalTime / desc.timeStep) + 1;
        m_progress.currentFrame = 0;
        m_progress.percentComplete = 0.0f;
        m_progress.complete = false;
        m_progress.cancelled = false;

        return S_OK;
    }

    HRESULT SceneEncode::EncodeFrame()
    {
        if (!m_encoding || !m_engine) return E_FAIL;

        ID3D11DeviceContext* ctx = m_engine->GetImmediateContext();
        if (!ctx) return E_FAIL;

        // Save old render targets
        CComPtr<ID3D11RenderTargetView> oldRTV;
        CComPtr<ID3D11DepthStencilView> oldDSV;
        ctx->OMGetRenderTargets(1, &oldRTV.p, &oldDSV.p);

        // Bind encode target
        ctx->OMSetRenderTargets(1, &m_frameRTV.p, m_frameDSV);

        float clearColor[4] = { 0, 0, 0, 1 };
        ctx->ClearRenderTargetView(m_frameRTV, clearColor);
        ctx->ClearDepthStencilView(m_frameDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(m_desc.width);
        vp.Height = static_cast<float>(m_desc.height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        ctx->RSSetViewports(1, &vp);

        Scene* scene = m_engine->GetScene();
        if (scene)
        {
            double time = m_desc.startTime + m_progress.currentFrame * m_desc.timeStep;
            scene->SetCurrentTime(time);

            RenderingList renderList;
            renderList.Build(scene);
            renderList.Execute(m_engine);
        }

        // Restore old targets
        ctx->OMSetRenderTargets(1, &oldRTV.p, oldDSV);

        if (m_frameCb) m_frameCb(m_frameTexture, m_progress.currentFrame);

        m_progress.currentFrame++;
        UpdateProgress(m_desc.startTime + m_progress.currentFrame * m_desc.timeStep);

        if (m_progress.currentFrame >= m_progress.totalFrames)
        {
            m_progress.complete = true;
            m_encoding = false;
        }

        return S_OK;
    }

    HRESULT SceneEncode::EndEncode()
    {
        m_encoding = false;
        m_progress.complete = true;
        ReleaseFrameBuffer();
        return S_OK;
    }

    void SceneEncode::CancelEncode()
    {
        m_encoding = false;
        m_progress.cancelled = true;
        ReleaseFrameBuffer();
    }

    void SceneEncode::UpdateProgress(double time)
    {
        m_progress.currentTime = time;
        m_progress.percentComplete = static_cast<float>(
            m_progress.currentFrame) / static_cast<float>(m_progress.totalFrames) * 100.0f;

        if (m_progressCb) m_progressCb(m_progress);
    }

} // namespace HMREngine
