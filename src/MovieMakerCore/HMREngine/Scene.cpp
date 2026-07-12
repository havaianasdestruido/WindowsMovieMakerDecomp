// Scene.cpp - HMREngine::Scene implementation

#include "Scene.h"
#include "Engine.h"

namespace HMREngine
{
    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
        Shutdown();
    }

    const char* Scene::GetNodeName() const
    {
        return "Scene";
    }

    const char* Scene::GetNodeTypeName() const
    {
        return "Scene";
    }

    HRESULT Scene::Initialize(Engine* engine)
    {
        m_engine = engine;

        m_width = engine->GetWidth();
        m_height = engine->GetHeight();

        return S_OK;
    }

    void Scene::Shutdown()
    {
        ClearRoutes();
        ClearNodes();
        m_rootLayerSet.Release();
        m_activeViewport = nullptr;
        m_engine = nullptr;
    }

    void Scene::RegisterNode(const std::string& id, X3DChildNode* node)
    {
        if (node)
        {
            m_nodeRegistry[id] = node;
            node->AddRef();
        }
    }

    X3DChildNode* Scene::FindNodeById(const std::string& id) const
    {
        auto it = m_nodeRegistry.find(id);
        return it != m_nodeRegistry.end() ? it->second : nullptr;
    }

    void Scene::ClearNodes()
    {
        for (auto& kv : m_nodeRegistry)
        {
            if (kv.second) kv.second->Release();
        }
        m_nodeRegistry.clear();
    }

    void Scene::Update(double deltaTime)
    {
        m_currentTime += deltaTime;

        if (m_rootLayerSet)
        {
            // Update time sensors
            Traverse([&](X3DChildNode* node, int depth)
            {
                auto* ts = dynamic_cast<TimeSensorNode*>(node);
                if (ts && ts->m_enabled)
                {
                    ts->Evaluate(m_currentTime);
                }
            });
        }

        PropagateRoutes();

        m_dirty = true;
    }

    void Scene::Traverse(std::function<void(X3DChildNode*, int)> visitor)
    {
        if (m_rootLayerSet)
        {
            TraverseNode(m_rootLayerSet, 0, visitor);
        }
    }

    void Scene::TraverseNode(X3DChildNode* node, int depth,
        std::function<void(X3DChildNode*, int)>& visitor)
    {
        if (!node) return;

        visitor(node, depth);

        for (size_t i = 0; i < node->GetNumChildren(); i++)
        {
            TraverseNode(node->GetChild(i), depth + 1, visitor);
        }
    }

    void Scene::AddRoute(SAIRoute* route)
    {
        m_routes.push_back(route);
    }

    void Scene::RemoveRoute(SAIRoute* route)
    {
        auto it = std::find(m_routes.begin(), m_routes.end(), route);
        if (it != m_routes.end())
        {
            m_routes.erase(it);
        }
    }

    void Scene::ClearRoutes()
    {
        for (auto* r : m_routes)
        {
            delete r;
        }
        m_routes.clear();
    }

    void Scene::PropagateRoutes()
    {
        for (auto* route : m_routes)
        {
            if (route->m_enabled)
            {
                route->Propagate();
            }
        }
    }

    void Scene::SetMetadata(const std::string& key, const std::string& value)
    {
        m_metadata[key] = value;
    }

    std::string Scene::GetMetadata(const std::string& key) const
    {
        auto it = m_metadata.find(key);
        return it != m_metadata.end() ? it->second : "";
    }

} // namespace HMREngine
