#pragma once
// RenderLoop.h - HMREngine::RenderLoop

#include "HMREngine.h"
#include <atomic>
#include <functional>

namespace HMREngine
{
    class Engine;

    class RenderLoop
    {
    public:
        RenderLoop(Engine* engine);
        ~RenderLoop();

        // Frame control
        void Start();
        void Stop();
        bool IsRunning() const { return m_running; }

        // Single frame render
        HRESULT RenderFrame();
        HRESULT RenderSceneToTexture();

        // Timing
        double GetTime() const;
        double GetDeltaTime() const { return m_deltaTime; }
        double GetFPS() const { return m_fps; }

        // Callbacks
        using PreFrameCallback = std::function<void(double time, double deltaTime)>;
        using PostFrameCallback = std::function<void(double time, double deltaTime)>;
        using FrameCallback = std::function<void(double time, double deltaTime)>;
        void SetPreFrameCallback(PreFrameCallback cb) { m_preFrameCb = cb; }
        void SetPostFrameCallback(PostFrameCallback cb) { m_postFrameCb = cb; }
        void SetFrameCallback(FrameCallback cb) { m_frameCb = cb; }

        // Target frame rate
        void SetTargetFPS(double fps) { m_targetFPS = fps; }
        double GetTargetFPS() const { return m_targetFPS; }

        // Pause
        void SetPaused(bool p) { m_paused = p; }
        bool IsPaused() const { return m_paused; }

    private:
        Engine* m_engine = nullptr;
        std::atomic<bool> m_running{ false };
        std::atomic<bool> m_paused{ false };

        double m_lastTime = 0.0;
        double m_deltaTime = 0.0;
        double m_fps = 0.0;
        double m_targetFPS = 60.0;
        double m_frameAccumulator = 0.0;
        int m_frameCount = 0;
        double m_fpsTimer = 0.0;

        LARGE_INTEGER m_perfFrequency{};
        LARGE_INTEGER m_lastPerfCounter{};

        PreFrameCallback m_preFrameCb;
        PostFrameCallback m_postFrameCb;
        FrameCallback m_frameCb;

        void UpdateTiming();
    };

} // namespace HMREngine
