#pragma once
// Browser.h - HMREngine::Browser (CComObject)

#include "HMREngine.h"
#include <atlbase.h>
#include <atlcom.h>
#include <string>
#include <vector>

namespace HMREngine
{
    class Engine;
    class Scene;

    class ATL_NO_VTABLE Browser :
        public CComObjectRootEx<CComSingleThreadModel>,
        public CComCoClass<Browser>
    {
    public:
        Browser();
        virtual ~Browser();

        DECLARE_REGISTRY_RESOURCEID(IDR_BROWSER)
        DECLARE_NOT_AGGREGATABLE(Browser)

        BEGIN_COM_MAP(Browser)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        // Lifecycle
        HRESULT Initialize(Engine* engine);
        void Shutdown();

        // Scene loading
        HRESULT LoadScene(const std::string& filePath);
        HRESULT LoadSceneFromMemory(const char* data, size_t length);
        HRESULT SaveScene(const std::string& filePath);

        // Current scene
        Scene* GetCurrentScene() const { return m_currentScene; }

        // Navigation
        void SetViewPosition(float x, float y, float z);
        void SetViewTarget(float x, float y, float z);
        void SetViewUp(float x, float y, float z);
        void SetFieldOfView(float fov);
        void ZoomToFit();
        void ResetView();

        // View state
        Vec3 GetViewPosition() const { return m_viewPosition; }
        Vec3 GetViewTarget() const { return m_viewTarget; }
        Vec3 GetViewUp() const { return m_viewUp; }
        float GetFieldOfView() const { return m_fieldOfView; }

        // Render mode
        enum RenderMode { RENDER_SOLID, RENDER_WIREFRAME, RENDER_POINTS, RENDER_BOUNDING_BOX };
        void SetRenderMode(RenderMode mode) { m_renderMode = mode; }
        RenderMode GetRenderMode() const { return m_renderMode; }

        // Background
        void SetBackgroundGradient(const Rgb& top, const Rgb& bottom);
        void SetBackgroundSolid(const Rgb& color);
        void SetBackgroundTransparent();

        // Coordinate system
        void SetCoordinateSystem(const char* cs);
        const char* GetCoordinateSystem() const { return m_coordinateSystem.c_str(); }

        // Units
        void SetUnits(const char* units);
        double GetUnitScale() const { return m_unitScale; }

        // Interaction
        void OnMouseDown(int button, int x, int y);
        void OnMouseUp(int button, int x, int y);
        void OnMouseMove(int x, int y);
        void OnMouseWheel(float delta);
        void OnKeyDown(int key);
        void OnKeyUp(int key);

        // Selection
        void SelectNode(X3DChildNode* node);
        void DeselectAll();
        X3DChildNode* GetSelectedNode() const { return m_selectedNode; }

        // Headlight
        void EnableHeadlight(bool enable) { m_headlightEnabled = enable; }
        bool IsHeadlightEnabled() const { return m_headlightEnabled; }

    private:
        Engine* m_engine = nullptr;
        Scene* m_currentScene = nullptr;

        Vec3 m_viewPosition = Vec3(0, 0, 5);
        Vec3 m_viewTarget = Vec3(0, 0, 0);
        Vec3 m_viewUp = Vec3(0, 1, 0);
        float m_fieldOfView = 0.785398f;

        RenderMode m_renderMode = RENDER_SOLID;
        std::string m_coordinateSystem = "Y_UP";
        double m_unitScale = 1.0;

        bool m_headlightEnabled = true;
        bool m_dragging = false;
        int m_lastMouseX = 0;
        int m_lastMouseY = 0;

        X3DChildNode* m_selectedNode = nullptr;

        Vec3 m_backgroundTop = Vec3(0.2f, 0.2f, 0.3f);
        Vec3 m_backgroundBottom = Vec3(0.1f, 0.1f, 0.15f);
        bool m_transparentBackground = false;
    };

    OBJECT_ENTRY_AUTO(__uuidof(Browser), Browser)

} // namespace HMREngine
