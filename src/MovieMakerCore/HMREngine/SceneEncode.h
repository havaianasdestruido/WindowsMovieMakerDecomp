#pragma once
// SceneEncode.h - HMREngine::SceneEncode

#include "HMREngine.h"
#include "X3DMath.h"
#include <atlbase.h>
#include <d3d11.h>
#include <string>
#include <functional>

namespace HMREngine
{
    class Engine;
    class Scene;
    class ExecutionContext;

    struct EncodeDesc
    {
        UINT width = 1920;
        UINT height = 1080;
        double fps = 30.0;
        double startTime = 0.0;
        double endTime = 10.0;
        double timeStep = 1.0 / 30.0;
        bool audioEnabled = false;
        std::string audioPath;
    };

    struct EncodeProgress
    {
        double currentTime = 0.0;
        double totalTime = 0.0;
        int currentFrame = 0;
        int totalFrames = 0;
        float percentComplete = 0.0f;
        bool complete = false;
        bool cancelled = false;
    };

    class SceneEncode
    {
    public:
        SceneEncode();
        ~SceneEncode();

        HRESULT Initialize(Engine* engine);
        void Shutdown();

        // Encode
        HRESULT BeginEncode(Scene* scene, const EncodeDesc& desc);
        HRESULT EncodeFrame();
        HRESULT EndEncode();
        void CancelEncode();

        // Progress
        const EncodeProgress& GetProgress() const { return m_progress; }

        // Output
        void SetOutputPath(const std::string& path) { m_outputPath = path; }
        std::string GetOutputPath() const { return m_outputPath; }

        // Callbacks
        using ProgressCallback = std::function<void(const EncodeProgress&)>;
        using FrameCallback = std::function<void(ID3D11Texture2D* frame, int frameNumber)>;
        void SetProgressCallback(ProgressCallback cb) { m_progressCb = cb; }
        void SetFrameCallback(FrameCallback cb) { m_frameCb = cb; }

        // Frame buffer access
        ID3D11Texture2D* GetCurrentFrameTexture() const { return m_frameTexture; }
        ID3D11ShaderResourceView* GetCurrentFrameSRV() const { return m_frameSRV; }

        bool IsEncoding() const { return m_encoding; }

    private:
        Engine* m_engine = nullptr;
        bool m_encoding = false;

        EncodeDesc m_desc;
        EncodeProgress m_progress;
        std::string m_outputPath;

        CComPtr<ID3D11Texture2D> m_frameTexture;
        CComPtr<ID3D11RenderTargetView> m_frameRTV;
        CComPtr<ID3D11ShaderResourceView> m_frameSRV;
        CComPtr<ID3D11Texture2D> m_frameDSTex;
        CComPtr<ID3D11DepthStencilView> m_frameDSV;

        ProgressCallback m_progressCb;
        FrameCallback m_frameCb;

        HRESULT CreateFrameBuffer();
        void ReleaseFrameBuffer();
        void UpdateProgress(double time);
    };

} // namespace HMREngine
