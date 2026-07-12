// LayerImpl.cpp - Layer/LayerSet/Viewport implementations

#include "LayerImpl.h"

namespace HMREngine
{
    // LayerImpl
    LayerImpl::LayerImpl() {}
    LayerImpl::~LayerImpl() { Shutdown(); }

    HRESULT LayerImpl::Initialize(LayerNode* node, ID3D11Device* dev)
    {
        if (!node) return E_POINTER;
        m_pickable = node->m_pickable;
        m_visible = node->m_visible;
        return S_OK;
    }

    void LayerImpl::Shutdown()
    {
        m_childLayers.clear();
    }

    void LayerImpl::AddChildLayer(LayerImpl* layer)
    {
        if (layer) m_childLayers.push_back(layer);
    }

    void LayerImpl::RemoveChildLayer(LayerImpl* layer)
    {
        auto it = std::find(m_childLayers.begin(), m_childLayers.end(), layer);
        if (it != m_childLayers.end()) m_childLayers.erase(it);
    }

    // LayerSetImpl
    LayerSetImpl::LayerSetImpl() {}
    LayerSetImpl::~LayerSetImpl() { Shutdown(); }

    HRESULT LayerSetImpl::Initialize(LayerSetNode* node, ID3D11Device* dev)
    {
        if (!node) return E_POINTER;

        if (node->m_activeLayer)
        {
            m_activeLayer = new LayerImpl();
            m_activeLayer->Initialize(node->m_activeLayer, dev);
        }

        return S_OK;
    }

    void LayerSetImpl::Shutdown()
    {
        m_activeLayer.Release();
    }

    // ViewportImpl
    ViewportImpl::ViewportImpl()
    {
        m_viewMatrix = Matrix4f::Identity();
        m_projMatrix = Matrix4f::Identity();
    }

    ViewportImpl::~ViewportImpl() { Shutdown(); }

    HRESULT ViewportImpl::Initialize(ViewportNode* node, ID3D11Device* dev)
    {
        if (!node) return E_POINTER;

        m_eyePosition = node->m_eyePosition;
        m_fov = node->m_fieldOfView;
        m_nearClip = node->m_nearClip;
        m_farClip = node->m_farClip;

        m_viewport.TopLeftX = node->m_viewport[0];
        m_viewport.TopLeftY = node->m_viewport[1];
        m_viewport.Width = node->m_viewport[2];
        m_viewport.Height = node->m_viewport[3];
        m_viewport.MinDepth = 0.0f;
        m_viewport.MaxDepth = 1.0f;

        m_viewMatrix = node->m_view;
        m_projMatrix = node->m_projection;

        return S_OK;
    }

    void ViewportImpl::Shutdown() {}

    void ViewportImpl::Apply(ID3D11DeviceContext* ctx)
    {
        if (ctx)
        {
            ctx->RSSetViewports(1, &m_viewport);
        }
    }

    void ViewportImpl::UpdateProjection(float aspectRatio)
    {
        m_projMatrix = Matrix4f::PerspectiveFovLH(m_fov, aspectRatio, m_nearClip, m_farClip);
    }

} // namespace HMREngine
