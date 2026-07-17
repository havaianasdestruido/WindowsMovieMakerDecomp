#pragma once
// Scene.h - HMREngine::Scene (CComObject)

#include "HMREngine.h"
#include "X3DNodes.h"
#include <atlbase.h>
#include <atlcom.h>
#include <string>
#include <vector>
#include <map>

namespace HMREngine
{
    class Engine;
    class LayerSetNode;
    class ViewportNode;
    class LayerNode;

    class ATL_NO_VTABLE Scene :
        public CComObjectRootEx<CComSingleThreadModel>,
        public CComCoClass<Scene>,
        public IX3DNode
    {
    public:
        Scene();
        virtual ~Scene();

        DECLARE_REGISTRY_RESOURCEID(IDR_SCENE)
        DECLARE_NOT_AGGREGATABLE(Scene)

        BEGIN_COM_MAP(Scene)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        // IX3DNode
        const char* GetNodeName() const override;
        const char* GetNodeTypeName() const override;

        // Scene management
        HRESULT Initialize(Engine* engine);
        void Shutdown();

        // Root node access
        SAIPtr<LayerSetNode> GetRootLayerSet() const { return m_rootLayerSet; }
        void SetRootLayerSet(LayerSetNode* ls) { m_rootLayerSet.Attach(ls); }

        // Active viewport
        ViewportNode* GetActiveViewport() const { return m_activeViewport; }
        void SetActiveViewport(ViewportNode* vp) { m_activeViewport = vp; }

        // Node registry
        void RegisterNode(const std::string& id, X3DChildNode* node);
        X3DChildNode* FindNodeById(const std::string& id) const;
        void ClearNodes();

        // Time
        double GetCurrentTime() const { return m_currentTime; }
        void SetCurrentTime(double t) { m_currentTime = t; }
        double GetStartTime() const { return m_startTime; }
        double GetEndTime() const { return m_endTime; }
        void SetTimeRange(double start, double end) { m_startTime = start; m_endTime = end; }

        // Background
        void SetBackgroundColor(const Rgba& color) { m_backgroundColor = color; }
        const Rgba& GetBackgroundColor() const { return m_backgroundColor; }

        // Dimensions
        void SetDimensions(UINT width, UINT height) { m_width = width; m_height = height; }
        UINT GetWidth() const { return m_width; }
        UINT GetHeight() const { return m_height; }

        // Scene graph traversal
        void Update(double deltaTime);
        void Traverse(std::function<void(X3DChildNode*, int)> visitor);

        // Route management
        void AddRoute(SAIRoute* route);
        void RemoveRoute(SAIRoute* route);
        void ClearRoutes();
        void PropagateRoutes();

        // Metadata
        void SetMetadata(const std::string& key, const std::string& value);
        std::string GetMetadata(const std::string& key) const;

        // Light management
        void AddLight(X3DChildNode* light);
        void RemoveLight(X3DChildNode* light);
        const std::vector<X3DChildNode*>& GetLights() const { return m_lights; }
        size_t GetLightCount() const { return m_lights.size(); }

        // Camera management
        void SetCamera(ViewportNode* camera);
        ViewportNode* GetCamera() const { return m_camera; }

        // Render (stub: rendering is done via RenderingList)
        HRESULT Render();

        // Dirty flag
        bool IsDirty() const { return m_dirty; }
        void SetDirty(bool d) { m_dirty = d; }

    private:
        Engine* m_engine = nullptr;

        SAIPtr<LayerSetNode> m_rootLayerSet;
        ViewportNode* m_activeViewport = nullptr;

        std::vector<X3DChildNode*> m_lights;
        ViewportNode* m_camera = nullptr;

        std::map<std::string, X3DChildNode*> m_nodeRegistry;
        std::vector<SAIRoute*> m_routes;
        std::map<std::string, std::string> m_metadata;

        double m_currentTime = 0.0;
        double m_startTime = 0.0;
        double m_endTime = 300.0;

        Rgba m_backgroundColor = Rgba(0.0f, 0.0f, 0.0f, 1.0f);
        UINT m_width = 1920;
        UINT m_height = 1080;

        bool m_dirty = false;

        void TraverseNode(X3DChildNode* node, int depth, std::function<void(X3DChildNode*, int)>& visitor);
    };

    OBJECT_ENTRY_AUTO(__uuidof(Scene), Scene)

} // namespace HMREngine
