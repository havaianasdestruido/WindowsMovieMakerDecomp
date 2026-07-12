#pragma once
// ExecutionContext.h - HMREngine::ExecutionContext

#include "HMREngine.h"
#include "X3DMath.h"
#include <d3d11.h>
#include <atlbase.h>
#include <vector>
#include <stack>
#include <functional>

namespace HMREngine
{
    class Engine;

    struct ViewportState
    {
        float x, y, width, height;
        Matrix4f viewMatrix;
        Matrix4f projectionMatrix;
        Vec3 eyePosition;
    };

    struct RenderState
    {
        CComPtr<ID3D11RenderTargetView> renderTarget;
        CComPtr<ID3D11DepthStencilView> depthStencil;
        D3D11_VIEWPORT viewport{};
        Rgba clearColor;
        float clearDepth = 1.0f;
        bool depthTestEnabled = true;
        bool blendingEnabled = false;
    };

    class ExecutionContext
    {
    public:
        ExecutionContext();
        ~ExecutionContext();

        // Current state
        void SetEngine(Engine* engine) { m_engine = engine; }
        Engine* GetEngine() const { return m_engine; }

        // Matrices
        void SetWorldMatrix(const Matrix4f& m) { m_worldMatrix = m; }
        void SetViewMatrix(const Matrix4f& m) { m_viewMatrix = m; }
        void SetProjectionMatrix(const Matrix4f& m) { m_projectionMatrix = m; }
        const Matrix4f& GetWorldMatrix() const { return m_worldMatrix; }
        const Matrix4f& GetViewMatrix() const { return m_viewMatrix; }
        const Matrix4f& GetProjectionMatrix() const { return m_projectionMatrix; }
        Matrix4f GetWorldViewProjectionMatrix() const;

        // Transform stack
        void PushWorldMatrix();
        void PopWorldMatrix();
        void MultiplyWorldMatrix(const Matrix4f& m);
        void SetWorldMatrixDirect(const Matrix4f& m);

        // Viewport stack
        void PushViewport(const ViewportState& vp);
        void PopViewport();

        // Render state stack
        void PushRenderState(const RenderState& state);
        void PopRenderState();

        // Current time
        void SetCurrentTime(double t) { m_currentTime = t; }
        double GetCurrentTime() const { return m_currentTime; }

        // Delta time
        void SetDeltaTime(double dt) { m_deltaTime = dt; }
        double GetDeltaTime() const { return m_deltaTime; }

        // Frame number
        void SetFrameNumber(UINT64 f) { m_frameNumber = f; }
        UINT64 GetFrameNumber() const { return m_frameNumber; }

        // Active shader
        void SetActiveEffect(ID3DX11Effect* effect) { m_activeEffect = effect; }
        ID3DX11Effect* GetActiveEffect() const { return m_activeEffect; }

        // Render pass callback
        void SetPreRenderCallback(std::function<void(ExecutionContext*)> cb) { m_preRenderCb = cb; }
        void SetPostRenderCallback(std::function<void(ExecutionContext*)> cb) { m_postRenderCb = cb; }

        void ExecutePreRender();
        void ExecutePostRender();

        // Render queue
        struct RenderCommand
        {
            enum Type { DRAW_MESH, DRAW_TEXT, DRAW_GRID, DRAW_EFFECT, CLEAR, SET_TARGET, RESTORE_TARGET };
            Type type;
            void* data;
            Matrix4f worldMatrix;
            int sortKey = 0;
        };

        void SubmitCommand(const RenderCommand& cmd);
        void ExecuteCommands();
        void ClearCommands();

        // Texture binds
        void BindTexture(int slot, ID3D11ShaderResourceView* srv);
        void ClearTextureBindings();

    private:
        Engine* m_engine = nullptr;

        Matrix4f m_worldMatrix;
        Matrix4f m_viewMatrix;
        Matrix4f m_projectionMatrix;

        std::stack<Matrix4f> m_worldMatrixStack;
        std::stack<ViewportState> m_viewportStack;
        std::stack<RenderState> m_renderStateStack;

        double m_currentTime = 0.0;
        double m_deltaTime = 0.0;
        UINT64 m_frameNumber = 0;

        ID3DX11Effect* m_activeEffect = nullptr;

        std::function<void(ExecutionContext*)> m_preRenderCb;
        std::function<void(ExecutionContext*)> m_postRenderCb;

        std::vector<RenderCommand> m_commands;

        CComPtr<ID3D11ShaderResourceView> m_textureBindings[16]{};
    };

} // namespace HMREngine
