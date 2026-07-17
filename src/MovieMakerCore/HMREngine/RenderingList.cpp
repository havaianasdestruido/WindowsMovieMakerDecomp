#include "pch.h"

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
        if (!cmd.text || !engine) return;

        ID3D11Device* dev = engine->GetDevice();
        ID3D11DeviceContext* ctx = engine->GetImmediateContext();
        if (!dev || !ctx) return;

        TextNode* text = cmd.text;
        if (text->m_string.m_value.empty()) return;

        const std::string& str = text->m_string.m_value[0];
        if (str.empty()) return;

        float fontSize = 1.0f;
        if (text->m_fontStyle)
            fontSize = text->m_fontStyle->m_size;

        struct TextVertex { Vec2 pos; Vec2 uv; Vec4 color; };
        std::vector<TextVertex> verts;

        float cursorX = 0.0f;
        float charW = fontSize * 0.6f;
        float charH = fontSize;
        int numChars = static_cast<int>(str.size());

        verts.reserve(numChars * 6);
        for (int i = 0; i < numChars; i++)
        {
            float x0 = cursorX;
            float x1 = cursorX + charW;
            float y0 = 0.0f;
            float y1 = charH;

            float u0 = (float)(static_cast<unsigned char>(str[i]) % 16) / 16.0f;
            float u1 = u0 + 1.0f / 16.0f;
            float v0 = (float)(static_cast<unsigned char>(str[i]) / 16) / 16.0f;
            float v1 = v0 + 1.0f / 16.0f;

            Vec4 col(1.0f, 1.0f, 1.0f, 1.0f);
            verts.push_back({ Vec2(x0, y0), Vec2(u0, v0), col });
            verts.push_back({ Vec2(x1, y0), Vec2(u1, v0), col });
            verts.push_back({ Vec2(x0, y1), Vec2(u0, v1), col });
            verts.push_back({ Vec2(x1, y0), Vec2(u1, v0), col });
            verts.push_back({ Vec2(x1, y1), Vec2(u1, v1), col });
            verts.push_back({ Vec2(x0, y1), Vec2(u0, v1), col });

            cursorX += charW;
        }

        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = static_cast<UINT>(verts.size() * sizeof(TextVertex));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMemData = verts.data();

        CComPtr<ID3D11Buffer> vb;
        HRESULT hr = dev->CreateBuffer(&bd, &init, &vb);
        if (FAILED(hr)) return;

        UINT stride = sizeof(TextVertex);
        UINT offset = 0;
        ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ctx->Draw(static_cast<UINT>(verts.size()), 0);

        engine->m_stats.drawCalls++;
        engine->m_stats.vertices += static_cast<UINT>(verts.size());
        engine->m_stats.triangles += static_cast<UINT>(verts.size() / 3);

        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void RenderingList::ExecuteGrid(RenderCommand& cmd, Engine* engine)
    {
        if (!cmd.grid || !engine) return;

        ID3D11Device* dev = engine->GetDevice();
        ID3D11DeviceContext* ctx = engine->GetImmediateContext();
        if (!dev || !ctx) return;

        GridNode* grid = cmd.grid;

        float halfW = grid->m_width * 0.5f;
        float halfH = grid->m_height * 0.5f;
        int divs = grid->m_divisions > 0 ? grid->m_divisions : 1;

        struct GridVertex { Vec3 pos; };
        std::vector<GridVertex> verts;
        verts.reserve((divs + 1) * 4 + (grid->m_showOrigin ? 4 : 0));

        float stepX = grid->m_width / static_cast<float>(divs);
        float stepY = grid->m_height / static_cast<float>(divs);

        for (int i = 0; i <= divs; i++)
        {
            float x = -halfW + i * stepX;
            verts.push_back({ Vec3(x, 0.0f, -halfH) });
            verts.push_back({ Vec3(x, 0.0f, halfH) });
        }

        for (int i = 0; i <= divs; i++)
        {
            float z = -halfH + i * stepY;
            verts.push_back({ Vec3(-halfW, 0.0f, z) });
            verts.push_back({ Vec3(halfW, 0.0f, z) });
        }

        if (grid->m_showOrigin)
        {
            float originLen = (min)(halfW, halfH) * 0.2f;
            verts.push_back({ Vec3(-originLen, 0.0f, 0.0f) });
            verts.push_back({ Vec3(originLen, 0.0f, 0.0f) });
            verts.push_back({ Vec3(0.0f, 0.0f, -originLen) });
            verts.push_back({ Vec3(0.0f, 0.0f, originLen) });
        }

        if (verts.empty()) return;

        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = static_cast<UINT>(verts.size() * sizeof(GridVertex));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMemData = verts.data();

        CComPtr<ID3D11Buffer> vb;
        HRESULT hr = dev->CreateBuffer(&bd, &init, &vb);
        if (FAILED(hr)) return;

        UINT stride = sizeof(GridVertex);
        UINT offset = 0;
        ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

        ctx->Draw(static_cast<UINT>(verts.size()), 0);

        engine->m_stats.drawCalls++;
        engine->m_stats.vertices += static_cast<UINT>(verts.size());

        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

    void RenderingList::AddRenderable(X3DChildNode* renderable, int sortOrder)
    {
        if (!renderable) return;

        auto it = std::find_if(m_renderables.begin(), m_renderables.end(),
            [renderable](const std::pair<X3DChildNode*, int>& p) { return p.first == renderable; });

        if (it != m_renderables.end())
        {
            it->second = sortOrder;
        }
        else
        {
            m_renderables.emplace_back(renderable, sortOrder);
        }
    }

    void RenderingList::RemoveRenderable(X3DChildNode* renderable)
    {
        m_renderables.erase(
            std::remove_if(m_renderables.begin(), m_renderables.end(),
                [renderable](const std::pair<X3DChildNode*, int>& p) { return p.first == renderable; }),
            m_renderables.end());
    }

    void RenderingList::RenderAll(Engine* engine)
    {
        if (!engine) return;

        std::stable_sort(m_renderables.begin(), m_renderables.end(),
            [](const std::pair<X3DChildNode*, int>& a, const std::pair<X3DChildNode*, int>& b)
            { return a.second < b.second; });

        for (auto& [node, order] : m_renderables)
        {
            if (!node || !node->m_visible) continue;

            auto* shape = dynamic_cast<ShapeNode*>(node);
            if (shape)
            {
                Matrix4f worldMatrix = Matrix4f::Identity();
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
                RenderCommand cmd;
                cmd.type = RenderCommand::SHAPE;
                cmd.shape = shape;
                cmd.worldMatrix = worldMatrix;
                cmd.sortPhase = RenderCommand::PHASE_OPAQUE;
                cmd.sortDepth = 0.0f;

                if (shape->m_appearance)
                {
                    auto* mat = dynamic_cast<MaterialNode*>(shape->m_appearance->m_material);
                    if (mat && mat->m_transparency > 0.0f)
                        cmd.sortPhase = RenderCommand::PHASE_TRANSLUCENT;
                }

                ExecuteShape(cmd, engine);
            }

            auto* text = dynamic_cast<TextNode*>(node);
            if (text)
            {
                RenderCommand cmd;
                cmd.type = RenderCommand::TEXT;
                cmd.text = text;
                cmd.worldMatrix = Matrix4f::Identity();
                cmd.sortPhase = RenderCommand::PHASE_OVERLAY;
                ExecuteText(cmd, engine);
            }

            auto* grid = dynamic_cast<GridNode*>(node);
            if (grid)
            {
                RenderCommand cmd;
                cmd.type = RenderCommand::GRID;
                cmd.grid = grid;
                cmd.worldMatrix = Matrix4f::Identity();
                ExecuteGrid(cmd, engine);
            }
        }
    }

} // namespace HMREngine
