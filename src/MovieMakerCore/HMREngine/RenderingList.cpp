// RenderingList.cpp - HMREngine::RenderingList implementation

#include "RenderingList.h"
#include "Engine.h"
#include "Scene.h"
#include "ExecutionContext.h"

namespace HMREngine
{
    RenderingList::RenderingList()
    {
    }

    RenderingList::~RenderingList()
    {
    }

    void RenderingList::Clear()
    {
        m_commands.clear();
    }

    void RenderingList::Build(Scene* scene)
    {
        if (!scene) return;

        Clear();

        scene->Traverse([&](X3DChildNode* node, int depth)
        {
            auto* shape = dynamic_cast<ShapeNode*>(node);
            if (shape)
            {
                Matrix4f worldMatrix = Matrix4f::Identity();

                // Walk up parents to build world matrix
                X3DChildNode* parent = shape->m_parent;
                while (parent)
                {
                    auto* transform = dynamic_cast<TransformNode*>(parent);
                    if (transform)
                    {
                        transform->UpdateMatrix();
                        worldMatrix = transform->m_matrix * worldMatrix;
                    }
                    parent = parent->m_parent;
                }

                AddShapeCommand(shape, worldMatrix, (float)depth);
                return;
            }

            auto* text = dynamic_cast<TextNode*>(node);
            if (text)
            {
                Matrix4f worldMatrix = Matrix4f::Identity();
                AddTextCommand(text, worldMatrix, (float)depth);
                return;
            }

            auto* grid = dynamic_cast<GridNode*>(node);
            if (grid)
            {
                Matrix4f worldMatrix = Matrix4f::Identity();
                AddGridCommand(grid, worldMatrix, (float)depth);
            }
        });

        Sort();
    }

    void RenderingList::Sort()
    {
        std::stable_sort(m_commands.begin(), m_commands.end(),
            [](const RenderCommand& a, const RenderCommand& b)
            {
                if (a.sortPhase != b.sortPhase) return a.sortPhase < b.sortPhase;
                return a.sortDepth > b.sortDepth; // back-to-front for translucent
            });
    }

    void RenderingList::Execute(Engine* engine)
    {
        if (!engine) return;

        ID3D11DeviceContext* ctx = engine->GetImmediateContext();
        if (!ctx) return;

        ExecutionContext* execCtx = engine->GetExecutionContext();

        for (auto& cmd : m_commands)
        {
            switch (cmd.type)
            {
            case RenderCommand::SHAPE:
                if (execCtx) execCtx->SetWorldMatrix(cmd.worldMatrix);
                ExecuteShape(cmd, engine);
                break;

            case RenderCommand::TEXT:
                if (execCtx) execCtx->SetWorldMatrix(cmd.worldMatrix);
                ExecuteText(cmd, engine);
                break;

            case RenderCommand::GRID:
                if (execCtx) execCtx->SetWorldMatrix(cmd.worldMatrix);
                ExecuteGrid(cmd, engine);
                break;

            case RenderCommand::CUSTOM:
                if (cmd.customDraw) cmd.customDraw(engine);
                break;

            default:
                break;
            }
        }
    }

    void RenderingList::ExecuteShape(RenderCommand& cmd, Engine* engine)
    {
        if (!cmd.shape || !engine) return;

        ShapeNode* shape = cmd.shape;

        if (!shape->m_geometry) return;

        ID3D11DeviceContext* ctx = engine->GetImmediateContext();

        // Get or build buffers
        ID3D11Buffer* vb = nullptr;
        ID3D11Buffer* ib = nullptr;

        shape->m_geometry->BuildVertexBuffer(engine->GetDevice(), &vb);
        shape->m_geometry->BuildIndexBuffer(engine->GetDevice(), &ib);

        UINT stride = sizeof(Vec3) * 2; // position + normal
        UINT offset = 0;

        ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

        if (ib)
        {
            ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
            ctx->DrawIndexed(shape->m_geometry->GetIndexCount(), 0, 0);
        }
        else
        {
            ctx->Draw(shape->m_geometry->GetVertexCount(), 0);
        }

        engine->m_stats.drawCalls++;
        engine->m_stats.vertices += shape->m_geometry->GetVertexCount();
        engine->m_stats.triangles += shape->m_geometry->GetIndexCount() / 3;
    }

    void RenderingList::ExecuteText(RenderCommand& cmd, Engine* engine)
    {
        // Text rendering via TextShader
    }

    void RenderingList::ExecuteGrid(RenderCommand& cmd, Engine* engine)
    {
        // Grid rendering via GridShader
    }

    void RenderingList::AddShapeCommand(ShapeNode* shape, const Matrix4f& worldMatrix, float depth)
    {
        RenderCommand cmd;
        cmd.type = RenderCommand::SHAPE;
        cmd.shape = shape;
        cmd.worldMatrix = worldMatrix;
        cmd.sortDepth = depth;

        // Determine if translucent
        if (shape->m_appearance)
        {
            auto* mat = dynamic_cast<MaterialNode*>(shape->m_appearance->m_material);
            if (mat && mat->m_transparency > 0.0f)
                cmd.sortPhase = RenderCommand::PHASE_TRANSLUCENT;
        }

        m_commands.push_back(std::move(cmd));
    }

    void RenderingList::AddTextCommand(TextNode* text, const Matrix4f& worldMatrix, float depth)
    {
        RenderCommand cmd;
        cmd.type = RenderCommand::TEXT;
        cmd.text = text;
        cmd.worldMatrix = worldMatrix;
        cmd.sortDepth = depth;
        cmd.sortPhase = RenderCommand::PHASE_OVERLAY;
        m_commands.push_back(std::move(cmd));
    }

    void RenderingList::AddGridCommand(GridNode* grid, const Matrix4f& worldMatrix, float depth)
    {
        RenderCommand cmd;
        cmd.type = RenderCommand::GRID;
        cmd.grid = grid;
        cmd.worldMatrix = worldMatrix;
        cmd.sortDepth = depth;
        m_commands.push_back(std::move(cmd));
    }

    void RenderingList::AddCustomCommand(std::function<void(Engine*)> draw, int phase, float depth)
    {
        RenderCommand cmd;
        cmd.type = RenderCommand::CUSTOM;
        cmd.customDraw = std::move(draw);
        cmd.sortPhase = phase;
        cmd.sortDepth = depth;
        m_commands.push_back(std::move(cmd));
    }

} // namespace HMREngine
