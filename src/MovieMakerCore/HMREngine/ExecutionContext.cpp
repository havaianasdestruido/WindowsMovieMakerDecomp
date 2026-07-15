#include "pch.h"

// ExecutionContext.cpp - HMREngine::ExecutionContext implementation

#include "ExecutionContext.h"
#include "Engine.h"

namespace HMREngine
{
    ExecutionContext::ExecutionContext()
    {
        m_worldMatrix = Matrix4f::Identity();
        m_viewMatrix = Matrix4f::Identity();
        m_projectionMatrix = Matrix4f::Identity();
    }

    ExecutionContext::~ExecutionContext()
    {
    }

    Matrix4f ExecutionContext::GetWorldViewProjectionMatrix() const
    {
        return m_worldMatrix * m_viewMatrix * m_projectionMatrix;
    }

    void ExecutionContext::PushWorldMatrix()
    {
        m_worldMatrixStack.push(m_worldMatrix);
    }

    void ExecutionContext::PopWorldMatrix()
    {
        if (!m_worldMatrixStack.empty())
        {
            m_worldMatrix = m_worldMatrixStack.top();
            m_worldMatrixStack.pop();
        }
    }

    void ExecutionContext::MultiplyWorldMatrix(const Matrix4f& m)
    {
        m_worldMatrix = m_worldMatrix * m;
    }

    void ExecutionContext::SetWorldMatrixDirect(const Matrix4f& m)
    {
        m_worldMatrix = m;
    }

    void ExecutionContext::PushViewport(const ViewportState& vp)
    {
        ViewportState current{};
        current.x = 0; current.y = 0;
        current.width = static_cast<float>(m_engine->GetWidth());
        current.height = static_cast<float>(m_engine->GetHeight());
        current.viewMatrix = m_viewMatrix;
        current.projectionMatrix = m_projectionMatrix;
        m_viewportStack.push(current);

        m_viewMatrix = vp.viewMatrix;
        m_projectionMatrix = vp.projectionMatrix;

        if (m_engine)
        {
            D3D11_VIEWPORT d3dvp{};
            d3dvp.TopLeftX = vp.x;
            d3dvp.TopLeftY = vp.y;
            d3dvp.Width = vp.width;
            d3dvp.Height = vp.height;
            d3dvp.MinDepth = 0.0f;
            d3dvp.MaxDepth = 1.0f;
            m_engine->GetImmediateContext()->RSSetViewports(1, &d3dvp);
        }
    }

    void ExecutionContext::PopViewport()
    {
        if (!m_viewportStack.empty())
        {
            auto& vp = m_viewportStack.top();
            m_viewMatrix = vp.viewMatrix;
            m_projectionMatrix = vp.projectionMatrix;
            m_viewportStack.pop();

            if (m_engine)
            {
                D3D11_VIEWPORT d3dvp{};
                d3dvp.TopLeftX = vp.x;
                d3dvp.TopLeftY = vp.y;
                d3dvp.Width = vp.width;
                d3dvp.Height = vp.height;
                d3dvp.MinDepth = 0.0f;
                d3dvp.MaxDepth = 1.0f;
                m_engine->GetImmediateContext()->RSSetViewports(1, &d3dvp);
            }
        }
    }

    void ExecutionContext::PushRenderState(const RenderState& state)
    {
        m_renderStateStack.push(state);
    }

    void ExecutionContext::PopRenderState()
    {
        if (!m_renderStateStack.empty())
        {
            m_renderStateStack.pop();
        }
    }

    void ExecutionContext::ExecutePreRender()
    {
        if (m_preRenderCb) m_preRenderCb(this);
    }

    void ExecutionContext::ExecutePostRender()
    {
        if (m_postRenderCb) m_postRenderCb(this);
    }

    void ExecutionContext::SubmitCommand(const RenderCommand& cmd)
    {
        m_commands.push_back(cmd);
    }

    void ExecutionContext::ExecuteCommands()
    {
        if (!m_engine) return;

        ID3D11DeviceContext* ctx = m_engine->GetImmediateContext();
        if (!ctx) return;

        std::sort(m_commands.begin(), m_commands.end(),
            [](const RenderCommand& a, const RenderCommand& b) { return a.sortKey < b.sortKey; });

        for (auto& cmd : m_commands)
        {
            switch (cmd.type)
            {
            case RenderCommand::SET_TARGET:
            {
                auto* target = reinterpret_cast<ID3D11RenderTargetView*>(cmd.data);
                ctx->OMSetRenderTargets(1, &target, nullptr);
                break;
            }
            case RenderCommand::RESTORE_TARGET:
            {
                auto* rtv = m_engine->GetBackBufferRTV();
                auto* dsv = m_engine->GetDepthStencilView();
                ctx->OMSetRenderTargets(1, &rtv, dsv);
                break;
            }
            case RenderCommand::CLEAR:
            {
                float color[4] = { 0, 0, 0, 1 };
                auto* rtv = reinterpret_cast<ID3D11RenderTargetView*>(cmd.data);
                ctx->ClearRenderTargetView(rtv, color);
                break;
            }
            default:
                break;
            }
        }
    }

    void ExecutionContext::ClearCommands()
    {
        m_commands.clear();
    }

    void ExecutionContext::BindTexture(int slot, ID3D11ShaderResourceView* srv)
    {
        if (slot >= 0 && slot < 16)
        {
            m_textureBindings[slot] = srv;
            if (m_engine && m_engine->GetImmediateContext())
            {
                m_engine->GetImmediateContext()->PSSetShaderResources(slot, 1, &srv);
            }
        }
    }

    void ExecutionContext::ClearTextureBindings()
    {
        ID3D11ShaderResourceView* nullSRV[16] = {};
        if (m_engine && m_engine->GetImmediateContext())
        {
            m_engine->GetImmediateContext()->PSSetShaderResources(0, 16, nullSRV);
        }
        for (int i = 0; i < 16; i++) m_textureBindings[i].Release();
    }

} // namespace HMREngine
