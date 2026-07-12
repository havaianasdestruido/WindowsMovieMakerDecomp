#pragma once
// LayerImpl.h - Layer, LayerSet, Viewport

#include "HMREngine.h"
#include "X3DNodes.h"
#include "X3DMath.h"
#include <d3d11.h>
#include <atlbase.h>
#include <vector>

namespace HMREngine
{
    class LayerImpl : public ImplObject<LayerImpl>
    {
    public:
        LayerImpl();
        virtual ~LayerImpl();

        HRESULT Initialize(LayerNode* node, ID3D11Device* dev);
        void Shutdown();

        bool IsPickable() const { return m_pickable; }
        bool IsVisible() const { return m_visible; }

        void AddChildLayer(LayerImpl* layer);
        void RemoveChildLayer(LayerImpl* layer);
        size_t GetNumChildLayers() const { return m_childLayers.size(); }

    private:
        bool m_pickable = true;
        bool m_visible = true;
        std::vector<LayerImpl*> m_childLayers;
    };

    class LayerSetImpl : public ImplObject<LayerSetImpl>
    {
    public:
        LayerSetImpl();
        virtual ~LayerSetImpl();

        HRESULT Initialize(LayerSetNode* node, ID3D11Device* dev);
        void Shutdown();

        LayerImpl* GetActiveLayer() const { return m_activeLayer; }
        void SetActiveLayer(LayerImpl* layer) { m_activeLayer = layer; }

    private:
        SAIPtr<LayerImpl> m_activeLayer;
    };

    class ViewportImpl : public ImplObject<ViewportImpl>
    {
    public:
        ViewportImpl();
        virtual ~ViewportImpl();

        HRESULT Initialize(ViewportNode* node, ID3D11Device* dev);
        void Shutdown();

        void Apply(ID3D11DeviceContext* ctx);

        Matrix4f GetViewMatrix() const { return m_viewMatrix; }
        Matrix4f GetProjectionMatrix() const { return m_projMatrix; }
        D3D11_VIEWPORT GetViewport() const { return m_viewport; }

        void SetViewMatrix(const Matrix4f& m) { m_viewMatrix = m; }
        void SetProjectionMatrix(const Matrix4f& m) { m_projMatrix = m; }
        void SetFieldOfView(float fov) { m_fov = fov; }
        void SetClipPlanes(float nearClip, float farClip) { m_nearClip = nearClip; m_farClip = farClip; }

        void UpdateProjection(float aspectRatio);

    private:
        Matrix4f m_viewMatrix;
        Matrix4f m_projMatrix;
        D3D11_VIEWPORT m_viewport{};

        Vec3 m_eyePosition;
        Vec3 m_target;
        Vec3 m_up = Vec3(0, 1, 0);
        float m_fov = 0.785398f;
        float m_nearClip = 0.1f;
        float m_farClip = 1000.0f;
    };

} // namespace HMREngine
