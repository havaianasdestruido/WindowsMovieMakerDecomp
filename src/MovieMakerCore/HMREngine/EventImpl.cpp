// EventImpl.cpp - Event/TimeDependent/Route implementations

#include "EventImpl.h"
#include <cmath>

namespace HMREngine
{
    // EventImpl
    void EventImpl::Fire()
    {
        if (m_sourceNode && m_destNode)
        {
            auto* sourceField = m_sourceNode->GetField(m_name);
            auto* destField = m_destNode->GetField(m_name);
            if (sourceField && destField)
            {
                destField->CopyFrom(*sourceField);
            }
        }
    }

    // TimeDependentImpl
    HRESULT TimeDependentImpl::Initialize(double startTime, double stopTime, double pauseTime, double resumeTime)
    {
        m_startTime = startTime;
        m_stopTime = stopTime;
        m_pauseTime = pauseTime;
        m_resumeTime = resumeTime;
        return S_OK;
    }

    void TimeDependentImpl::Shutdown()
    {
        m_running = false;
        m_paused = false;
    }

    void TimeDependentImpl::Start(double globalTime)
    {
        if (!m_enabled) return;
        m_running = true;
        m_paused = false;
        m_cycleStart = globalTime;
        m_elapsedTime = 0.0;

        if (m_onStarted) m_onStarted();
    }

    void TimeDependentImpl::Stop(double globalTime)
    {
        m_running = false;
        m_fraction = 1.0;
        if (m_onStopped) m_onStopped();
    }

    void TimeDependentImpl::Pause(double globalTime)
    {
        if (!m_running || m_paused) return;
        m_paused = true;
        m_pauseTime = globalTime;
        if (m_onPaused) m_onPaused();
    }

    void TimeDependentImpl::Resume(double globalTime)
    {
        if (!m_paused) return;
        m_paused = false;
        double pauseDuration = globalTime - m_pauseTime;
        m_cycleStart += pauseDuration;
        m_startTime += pauseDuration;
        if (m_stopTime > 0) m_stopTime += pauseDuration;
        if (m_onResumed) m_onResumed();
    }

    void TimeDependentImpl::Evaluate(double globalTime)
    {
        if (!m_running || m_paused) return;

        m_elapsedTime = globalTime - m_startTime;

        if (m_stopTime > m_startTime && globalTime >= m_stopTime)
        {
            Stop(globalTime);
            return;
        }

        if (m_cycleInterval > 0.0)
        {
            double cycleTime = fmod(globalTime - m_cycleStart, m_cycleInterval);
            m_fraction = cycleTime / m_cycleInterval;

            if (m_onCycle) m_onCycle(m_fraction);
        }
    }

    // SAIRouteImpl
    HRESULT SAIRouteImpl::Initialize(X3DNode* source, const std::string& sourceField,
                                     X3DNode* dest, const std::string& destField)
    {
        if (!source || !dest) return E_POINTER;
        m_sourceNode = source;
        m_sourceField = sourceField;
        m_destNode = dest;
        m_destField = destField;
        return S_OK;
    }

    void SAIRouteImpl::Propagate()
    {
        if (!m_sourceNode || !m_destNode || !m_enabled) return;

        auto* srcField = m_sourceNode->GetField(m_sourceField);
        auto* dstField = m_destNode->GetField(m_destField);

        if (srcField && dstField)
        {
            dstField->CopyFrom(*srcField);
        }
    }

} // namespace HMREngine
