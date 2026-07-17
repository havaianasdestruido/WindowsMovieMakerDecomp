#pragma once
// RenderingList.h - HMREngine::RenderingList

#include "HMREngine.h"
#include "X3DNodes.h"
#include "X3DMath.h"
#include <vector>
#include <algorithm>

namespace HMREngine
{
    class Engine;
    class Scene;

    struct RenderCommand
    {
        enum Type { SHAPE, TEXT, GRID, EFFECT_OVERLAY, CUSTOM };
        enum SortPhase { PHASE_OPAQUE = 0, PHASE_TRANSLUCENT = 1000, PHASE_OVERLAY = 2000 };

        Type type;
        int sortPhase = PHASE_OPAQUE;
        float sortDepth = 0.0f;

        ShapeNode* shape = nullptr;
        Matrix4f worldMatrix;

        // For text
        TextNode* text = nullptr;

        // For grid
        GridNode* grid = nullptr;

        // Custom draw
        std::function<void(Engine*)> customDraw;
    };

    class RenderingList
    {
    public:
        RenderingList();
        ~RenderingList();

        void Build(Scene* scene);
        void Execute(Engine* engine);

        void Clear();
        size_t GetCommandCount() const { return m_commands.size(); }

        void AddShapeCommand(ShapeNode* shape, const Matrix4f& worldMatrix, float depth);
        void AddTextCommand(TextNode* text, const Matrix4f& worldMatrix, float depth);
        void AddGridCommand(GridNode* grid, const Matrix4f& worldMatrix, float depth);
        void AddCustomCommand(std::function<void(Engine*)> draw, int phase, float depth);

        // Renderable management
        void AddRenderable(X3DChildNode* renderable, int sortOrder);
        void RemoveRenderable(X3DChildNode* renderable);
        void RenderAll(Engine* engine);

    private:
        std::vector<RenderCommand> m_commands;
        std::vector<std::pair<X3DChildNode*, int>> m_renderables;

        void Sort();
        void ExecuteShape(RenderCommand& cmd, Engine* engine);
        void ExecuteText(RenderCommand& cmd, Engine* engine);
        void ExecuteGrid(RenderCommand& cmd, Engine* engine);
    };

} // namespace HMREngine
