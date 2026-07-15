#include "pch.h"

// RenderLoop.cpp - HMREngine::RenderLoop implementation

#include "RenderLoop.h"
#include "Engine.h"
#include "Scene.h"
#include "ExecutionContext.h"
#include "RenderingList.h"

namespace HMREngine
{
    RenderLoop::RenderLoop(Engine* engine)
        : m_engine(engine)
    {
        QueryPerformanceFrequency(&m_perfFrequency);
        QueryPerformanceCounter(&m_lastPerfCounter);
        m_lastTime = GetTime();
    }

    RenderLoop::~RenderLoop()
    {
        Stop();
    }

    double RenderLoop::GetTime() const
    {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return static_cast<double>(counter.QuadPart) / m_perfFrequency.QuadPart;
    }

    void RenderLoop::UpdateTiming()
    {
        double now = GetTime();
        m_deltaTime = now - m_lastTime;
        m_lastTime = now;

        // FPS calculation
        m_frameAccumulator += m_deltaTime;
        m_frameCount++;

        if (m_frameAccumulator >= 1.0)
        {
            m_fps = m_frameCount / m_frameAccumulator;
            m_frameCount = 0;
            m_frameAccumulator = 0.0;
        }

        // Frame rate limiting
        double frameInterval = 1.0 / m_targetFPS;
        if (m_deltaTime < frameInterval)
        {
            double sleepTime = frameInterval - m_deltaTime;
            if (sleepTime > 0.001)
            {
                LARGE_INTEGER start, end;
                QueryPerformanceCounter(&start);
                double target = start.QuadPart + static_cast<LONGLONG>(sleepTime * m_perfFrequency.QuadPart);
                do
                {
                    QueryPerformanceCounter(&end);
                } while (end.QuadPart < target);
            }
        }
    }

    void RenderLoop::Start()
    {
        m_running = true;
    }

    void RenderLoop::Stop()
    {
        m_running = false;
    }

    HRESULT RenderLoop::RenderFrame()
    {
        if (m_paused) return S_OK;
        if (!m_engine) return E_POINTER;

        UpdateTiming();

        ExecutionContext* ctx = m_engine->GetExecutionContext();
        if (ctx)
        {
            ctx->SetCurrentTime(GetTime());
            ctx->SetDeltaTime(m_deltaTime);
            ctx->SetFrameNumber(ctx->GetFrameNumber() + 1);
        }

        if (m_preFrameCb) m_preFrameCb(GetTime(), m_deltaTime);

        Scene* scene = m_engine->GetScene();
        if (scene)
        {
            scene->Update(m_deltaTime);
        }

        HRESULT hr = m_engine->BeginFrame();
        if (FAILED(hr)) return hr;

        if (ctx) ctx->ExecutePreRender();

        if (scene)
        {
            RenderingList renderList;
            renderList.Build(scene);
            renderList.Execute(m_engine);
        }

        if (ctx) ctx->ExecutePostRender();

        m_engine->EndFrame();
        hr = m_engine->Present();

        if (m_postFrameCb) m_postFrameCb(GetTime(), m_deltaTime);

        return hr;
    }

    HRESULT RenderLoop::RenderSceneToTexture()
    {
        if (!m_engine) return E_POINTER;

        ExecutionContext* ctx = m_engine->GetExecutionContext();
        if (ctx)
        {
            ctx->SetCurrentTime(GetTime());
            ctx->SetDeltaTime(0.0);
        }

        Scene* scene = m_engine->GetScene();
        if (scene) scene->Update(0.0);

        return S_OK;
    }

} // namespace HMREngine
