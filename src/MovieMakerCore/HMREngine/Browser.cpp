// Browser.cpp - HMREngine::Browser implementation

#include "Browser.h"
#include "Engine.h"
#include "Scene.h"
#include "X3DReader.h"

namespace HMREngine
{
    Browser::Browser()
    {
    }

    Browser::~Browser()
    {
        Shutdown();
    }

    HRESULT Browser::Initialize(Engine* engine)
    {
        m_engine = engine;
        return S_OK;
    }

    void Browser::Shutdown()
    {
        DeselectAll();
        m_currentScene = nullptr;
        m_engine = nullptr;
    }

    HRESULT Browser::LoadScene(const std::string& filePath)
    {
        CComObject<Scene>* scene = nullptr;
        CComObject<Scene>::CreateInstance(&scene);
        scene->AddRef();

        HRESULT hr = scene->Initialize(m_engine);
        if (FAILED(hr)) { scene->Release(); return hr; }

        X3DReader reader;
        hr = reader.LoadFromFile(filePath, scene);

        if (FAILED(hr))
        {
            scene->Release();
            return hr;
        }

        if (m_currentScene)
        {
            m_currentScene->Shutdown();
            m_currentScene->Release();
        }

        m_currentScene = scene;
        m_engine->SetScene(m_currentScene);

        ZoomToFit();
        return S_OK;
    }

    HRESULT Browser::LoadSceneFromMemory(const char* data, size_t length)
    {
        CComObject<Scene>* scene = nullptr;
        CComObject<Scene>::CreateInstance(&scene);
        scene->AddRef();

        HRESULT hr = scene->Initialize(m_engine);
        if (FAILED(hr)) { scene->Release(); return hr; }

        X3DReader reader;
        hr = reader.LoadFromMemory(data, length, scene);

        if (FAILED(hr))
        {
            scene->Release();
            return hr;
        }

        if (m_currentScene)
        {
            m_currentScene->Shutdown();
            m_currentScene->Release();
        }

        m_currentScene = scene;
        m_engine->SetScene(m_currentScene);

        ZoomToFit();
        return S_OK;
    }

    HRESULT Browser::SaveScene(const std::string& filePath)
    {
        if (!m_currentScene) return E_POINTER;
        X3DReader writer;
        return writer.SaveToFile(filePath, m_currentScene);
    }

    void Browser::SetViewPosition(float x, float y, float z) { m_viewPosition = Vec3(x, y, z); }
    void Browser::SetViewTarget(float x, float y, float z) { m_viewTarget = Vec3(x, y, z); }
    void Browser::SetViewUp(float x, float y, float z) { m_viewUp = Vec3(x, y, z); }
    void Browser::SetFieldOfView(float fov) { m_fieldOfView = fov; }

    void Browser::ZoomToFit()
    {
        m_viewPosition = Vec3(0, 0, 5);
        m_viewTarget = Vec3(0, 0, 0);
        m_viewUp = Vec3(0, 1, 0);
    }

    void Browser::ResetView()
    {
        ZoomToFit();
        m_fieldOfView = 0.785398f;
    }

    void Browser::SetBackgroundGradient(const Rgb& top, const Rgb& bottom)
    {
        m_backgroundTop = top;
        m_backgroundBottom = bottom;
        m_transparentBackground = false;
    }

    void Browser::SetBackgroundSolid(const Rgb& color)
    {
        m_backgroundTop = color;
        m_backgroundBottom = color;
        m_transparentBackground = false;
    }

    void Browser::SetBackgroundTransparent()
    {
        m_transparentBackground = true;
    }

    void Browser::SetCoordinateSystem(const char* cs)
    {
        m_coordinateSystem = cs;
    }

    void Browser::SetUnits(const char* units)
    {
        if (strcmp(units, "METER") == 0) m_unitScale = 1.0;
        else if (strcmp(units, "CENTIMETER") == 0) m_unitScale = 0.01;
        else if (strcmp(units, "MILLIMETER") == 0) m_unitScale = 0.001;
        else if (strcmp(units, "INCH") == 0) m_unitScale = 0.0254;
        else if (strcmp(units, "FOOT") == 0) m_unitScale = 0.3048;
    }

    void Browser::OnMouseDown(int button, int x, int y)
    {
        m_dragging = true;
        m_lastMouseX = x;
        m_lastMouseY = y;
    }

    void Browser::OnMouseUp(int button, int x, int y)
    {
        m_dragging = false;
    }

    void Browser::OnMouseMove(int x, int y)
    {
        if (!m_dragging) return;

        int dx = x - m_lastMouseX;
        int dy = y - m_lastMouseY;

        Vec3 forward = (m_viewTarget - m_viewPosition).Normalized();
        Vec3 right = forward.Cross(m_viewUp).Normalized();
        Vec3 up = right.Cross(forward).Normalized();

        float sensitivity = 0.005f;

        Vec3 delta = right * (-dx * sensitivity) + up * (dy * sensitivity);
        m_viewTarget = m_viewTarget + delta;
        m_viewPosition = m_viewPosition + delta;

        m_lastMouseX = x;
        m_lastMouseY = y;
    }

    void Browser::OnMouseWheel(float delta)
    {
        Vec3 forward = (m_viewTarget - m_viewPosition).Normalized();
        float zoomSpeed = 0.1f;
        Vec3 offset = forward * (delta * zoomSpeed);
        m_viewPosition = m_viewPosition + offset;
    }

    void Browser::OnKeyDown(int key) {}
    void Browser::OnKeyUp(int key) {}

    void Browser::SelectNode(X3DChildNode* node)
    {
        m_selectedNode = node;
    }

    void Browser::DeselectAll()
    {
        m_selectedNode = nullptr;
    }

} // namespace HMREngine
