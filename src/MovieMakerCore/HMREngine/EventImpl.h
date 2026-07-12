#pragma once
// EventImpl.h - Event, TimeDependentNodes, SAIRoute

#include "HMREngine.h"
#include "X3DNodes.h"
#include <string>
#include <functional>
#include <vector>

namespace HMREngine
{
    class EventImpl
    {
    public:
        EventImpl() = default;
        virtual ~EventImpl() = default;

        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }

        void SetSourceNode(X3DNode* node) { m_sourceNode = node; }
        void SetDestNode(X3DNode* node) { m_destNode = node; }

        X3DNode* GetSourceNode() const { return m_sourceNode; }
        X3DNode* GetDestNode() const { return m_destNode; }

        void Fire();

    private:
        std::string m_name;
        X3DNode* m_sourceNode = nullptr;
        X3DNode* m_destNode = nullptr;
    };

    class TimeDependentImpl
    {
    public:
        virtual ~TimeDependentImpl() = default;

        HRESULT Initialize(double startTime, double stopTime, double pauseTime, double resumeTime);
        void Shutdown();

        bool IsRunning() const { return m_running; }
        double GetFraction() const { return m_fraction; }
        double GetElapsedTime() const { return m_elapsedTime; }

        void Start(double globalTime);
        void Stop(double globalTime);
        void Pause(double globalTime);
        void Resume(double globalTime);

        void Evaluate(double globalTime);

        void SetOnStarted(std::function<void()> cb) { m_onStarted = cb; }
        void SetOnStopped(std::function<void()> cb) { m_onStopped = cb; }
        void SetOnPaused(std::function<void()> cb) { m_onPaused = cb; }
        void SetOnResumed(std::function<void()> cb) { m_onResumed = cb; }
        void SetOnCycle(std::function<void(double)> cb) { m_onCycle = cb; }

    private:
        double m_startTime = 0.0;
        double m_stopTime = 0.0;
        double m_pauseTime = 0.0;
        double m_resumeTime = 0.0;

        bool m_enabled = true;
        bool m_running = false;
        bool m_paused = false;
        bool m_loop = false;

        double m_fraction = 0.0;
        double m_elapsedTime = 0.0;
        double m_cycleInterval = 1.0;
        double m_cycleStart = 0.0;

        std::function<void()> m_onStarted;
        std::function<void()> m_onStopped;
        std::function<void()> m_onPaused;
        std::function<void()> m_onResumed;
        std::function<void(double)> m_onCycle;
    };

    class SAIRouteImpl
    {
    public:
        SAIRouteImpl() = default;
        ~SAIRouteImpl() = default;

        HRESULT Initialize(X3DNode* source, const std::string& sourceField,
                           X3DNode* dest, const std::string& destField);

        void Propagate();
        void Enable(bool enable) { m_enabled = enable; }
        bool IsEnabled() const { return m_enabled; }

        X3DNode* GetSourceNode() const { return m_sourceNode; }
        X3DNode* GetDestNode() const { return m_destNode; }
        const std::string& GetSourceField() const { return m_sourceField; }
        const std::string& GetDestField() const { return m_destField; }

    private:
        X3DNode* m_sourceNode = nullptr;
        std::string m_sourceField;
        X3DNode* m_destNode = nullptr;
        std::string m_destField;
        bool m_enabled = true;
    };

} // namespace HMREngine
