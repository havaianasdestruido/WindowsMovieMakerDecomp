#pragma once
// ScenePreview.h - HMREngine::ScenePreview

#include "HMREngine.h"
#include "X3DMath.h"
#include <atlbase.h>
#include <d3d11.h>

namespace HMREngine
{
    class Engine;
    class Scene;
    class ExecutionContext;

    struct PreviewDesc
    {
        UINT width = 640;
        UINT height = 480;
        float fieldOfView = 0.785398f;
        bool showGrid = true;
        bool showAxes = true;
        bool showBoundingBox = false;
        Rgba backgroundColor = Rgba(0.2f, 0.2f, 0.3f, 1.0f);
    };

    class ScenePreview
    {
    public:
        ScenePreview();
        ~ScenePreview();

        HRESULT Initialize(Engine* engine, const PreviewDesc& desc);
        void Shutdown();

        // Render
        HRESULT Render(Scene* scene, ExecutionContext* ctx);
        HRESULT Resize(UINT width, UINT height);

        // View
        void SetCameraPosition(const Vec3& pos) { m_cameraPos = pos; }
        void SetCameraTarget(const Vec3& target) { m_cameraTarget = target; }
        void SetCameraUp(const Vec3& up) { m_cameraUp = up; }
        void SetFieldOfView(float fov) { m_fov = fov; }

        // Access rendered texture
        ID3D11ShaderResourceView* GetRenderedTexture() const { return m_renderedSRV; }
        ID3D11Texture2D* GetRenderedTexture2D() const { return m_renderedTexture; }

        // Options
        void SetShowGrid(bool show) { m_showGrid = show; }
        void SetShowAxes(bool show) { m_showAxes = show; }
        void SetShowBoundingBox(bool show) { m_showBoundingBox = show; }
        void SetBackgroundColor(const Rgba& c) { m_bgColor = c; }

    private:
        Engine* m_engine = nullptr;
        PreviewDesc m_desc;

        Vec3 m_cameraPos = Vec3(0, 2, 5);
        Vec3 m_cameraTarget = Vec3(0, 0, 0);
        Vec3 m_cameraUp = Vec3(0, 1, 0);
        float m_fov = 0.785398f;

        bool m_showGrid = true;
        bool m_showAxes = true;
        bool m_showBoundingBox = false;
        Rgba m_bgColor;

        CComPtr<ID3D11Texture2D> m_renderedTexture;
        CComPtr<ID3D11RenderTargetView> m_renderedRTV;
        CComPtr<ID3D11ShaderResourceView> m_renderedSRV;
        CComPtr<ID3D11DepthStencilView> m_previewDSV;

        Matrix4f m_viewMatrix;
        Matrix4f m_projMatrix;

        void UpdateMatrices();
        void RenderGrid(ExecutionContext* ctx);
        void RenderAxes(ExecutionContext* ctx);
    };

} // namespace HMREngine
