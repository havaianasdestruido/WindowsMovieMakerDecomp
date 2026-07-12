// ScenePreview.cpp - HMREngine::ScenePreview implementation

#include "ScenePreview.h"
#include "Engine.h"
#include "Scene.h"
#include "ExecutionContext.h"
#include "RenderingList.h"

namespace HMREngine
{
    ScenePreview::ScenePreview()
    {
    }

    ScenePreview::~ScenePreview()
    {
        Shutdown();
    }

    HRESULT ScenePreview::Initialize(Engine* engine, const PreviewDesc& desc)
    {
        m_engine = engine;
        m_desc = desc;
        m_fov = desc.fieldOfView;
        m_showGrid = desc.showGrid;
        m_showAxes = desc.showAxes;
        m_showBoundingBox = desc.showBoundingBox;
        m_bgColor = desc.backgroundColor;

        ID3D11Device* dev = engine->GetDevice();
        if (!dev) return E_POINTER;

        D3D11_TEXTURE2D_DESC td{};
        td.Width = desc.width;
        td.Height = desc.height;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        HRESULT hr = dev->CreateTexture2D(&td, nullptr, &m_renderedTexture);
        if (FAILED(hr)) return hr;

        hr = dev->CreateRenderTargetView(m_renderedTexture, nullptr, &m_renderedRTV);
        if (FAILED(hr)) return hr;

        hr = dev->CreateShaderResourceView(m_renderedTexture, nullptr, &m_renderedSRV);
        if (FAILED(hr)) return hr;

        D3D11_TEXTURE2D_DESC dsd{};
        dsd.Width = desc.width;
        dsd.Height = desc.height;
        dsd.MipLevels = 1;
        dsd.ArraySize = 1;
        dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsd.SampleDesc.Count = 1;
        dsd.Usage = D3D11_USAGE_DEFAULT;
        dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        CComPtr<ID3D11Texture2D> dsTex;
        hr = dev->CreateTexture2D(&dsd, nullptr, &dsTex);
        if (FAILED(hr)) return hr;

        hr = dev->CreateDepthStencilView(dsTex, nullptr, &m_previewDSV);
        if (FAILED(hr)) return hr;

        UpdateMatrices();
        return S_OK;
    }

    void ScenePreview::Shutdown()
    {
        m_renderedSRV.Release();
        m_renderedRTV.Release();
        m_renderedTexture.Release();
        m_previewDSV.Release();
        m_engine = nullptr;
    }

    HRESULT ScenePreview::Render(Scene* scene, ExecutionContext* ctx)
    {
        if (!m_engine || !m_renderedRTV) return E_POINTER;

        ID3D11DeviceContext* d3dctx = m_engine->GetImmediateContext();

        UpdateMatrices();

        // Save old render targets
        CComPtr<ID3D11RenderTargetView> oldRTV;
        CComPtr<ID3D11DepthStencilView> oldDSV;
        d3dctx->OMGetRenderTargets(1, &oldRTV.p, &oldDSV.p);

        // Bind preview target
        d3dctx->OMSetRenderTargets(1, &m_renderedRTV.p, m_previewDSV);

        // Clear
        float clearColor[4] = { m_bgColor.x, m_bgColor.y, m_bgColor.z, m_bgColor.w };
        d3dctx->ClearRenderTargetView(m_renderedRTV, clearColor);
        d3dctx->ClearDepthStencilView(m_previewDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Set viewport
        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(m_desc.width);
        vp.Height = static_cast<float>(m_desc.height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        d3dctx->RSSetViewports(1, &vp);

        // Set matrices
        if (ctx)
        {
            ctx->SetViewMatrix(m_viewMatrix);
            ctx->SetProjectionMatrix(m_projMatrix);
        }

        if (m_showGrid) RenderGrid(ctx);
        if (m_showAxes) RenderAxes(ctx);

        // Render scene
        if (scene)
        {
            RenderingList renderList;
            renderList.Build(scene);
            renderList.Execute(m_engine);
        }

        // Restore old targets
        d3dctx->OMSetRenderTargets(1, &oldRTV.p, oldDSV);

        return S_OK;
    }

    HRESULT ScenePreview::Resize(UINT width, UINT height)
    {
        m_desc.width = width;
        m_desc.height = height;

        m_renderedSRV.Release();
        m_renderedRTV.Release();
        m_renderedTexture.Release();
        m_previewDSV.Release();

        ID3D11Device* dev = m_engine->GetDevice();
        D3D11_TEXTURE2D_DESC td{};
        td.Width = width;
        td.Height = height;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        HRESULT hr = dev->CreateTexture2D(&td, nullptr, &m_renderedTexture);
        if (FAILED(hr)) return hr;
        hr = dev->CreateRenderTargetView(m_renderedTexture, nullptr, &m_renderedRTV);
        if (FAILED(hr)) return hr;
        hr = dev->CreateShaderResourceView(m_renderedTexture, nullptr, &m_renderedSRV);
        if (FAILED(hr)) return hr;

        D3D11_TEXTURE2D_DESC dsd{};
        dsd.Width = width; dsd.Height = height;
        dsd.MipLevels = 1; dsd.ArraySize = 1;
        dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsd.SampleDesc.Count = 1;
        dsd.Usage = D3D11_USAGE_DEFAULT;
        dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        CComPtr<ID3D11Texture2D> dsTex;
        hr = dev->CreateTexture2D(&dsd, nullptr, &dsTex);
        if (FAILED(hr)) return hr;
        hr = dev->CreateDepthStencilView(dsTex, nullptr, &m_previewDSV);

        UpdateMatrices();
        return hr;
    }

    void ScenePreview::UpdateMatrices()
    {
        m_viewMatrix = Matrix4f::LookAtLH(m_cameraPos, m_cameraTarget, m_cameraUp);
        float aspect = static_cast<float>(m_desc.width) / static_cast<float>(m_desc.height);
        m_projMatrix = Matrix4f::PerspectiveFovLH(m_fov, aspect, 0.1f, 1000.0f);
    }

    void ScenePreview::RenderGrid(ExecutionContext* ctx)
    {
        // Grid rendering uses the GridShader with a fullscreen quad
    }

    void ScenePreview::RenderAxes(ExecutionContext* ctx)
    {
        // Axes rendering (RGB lines for X/Y/Z)
    }

} // namespace HMREngine
