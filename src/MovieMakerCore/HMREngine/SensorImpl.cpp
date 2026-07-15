#include "pch.h"

// SensorImpl.cpp - TimeSensor, TimeTrigger, Switch implementations

#include "SensorImpl.h"

namespace HMREngine
{
    // TimeSensorImpl
    TimeSensorImpl::TimeSensorImpl() {}
    TimeSensorImpl::~TimeSensorImpl() { Shutdown(); }

    HRESULT TimeSensorImpl::Initialize(TimeSensorNode* node)
    {
        if (!node) return E_POINTER;
        m_node = node;
        m_startTime = node->m_startTime;
        m_stopTime = node->m_stopTime;
        m_cycleInterval = node->m_cycleInterval;
        m_loop = node->m_loop;
        m_enabled = node->m_enabled;
        return S_OK;
    }

    void TimeSensorImpl::Shutdown()
    {
        m_node = nullptr;
    }

    void TimeSensorImpl::Update(double globalTime)
    {
        if (!m_enabled) return;

        m_wasActive = m_isActive;

        if (!m_isActive && globalTime >= m_startTime &&
            (m_stopTime <= m_startTime || globalTime < m_stopTime))
        {
            m_isActive = true;
            m_elapsedTime = 0.0;
        }
        else if (m_isActive && m_stopTime > m_startTime && globalTime >= m_stopTime)
        {
            m_isActive = false;
            m_fraction = 1.0;
            if (m_onActiveChanged) m_onActiveChanged(false);
            return;
        }

        if (!m_isActive) return;

        m_elapsedTime = globalTime - m_startTime;

        if (m_cycleInterval > 0.0)
        {
            double cycleTime = fmod(m_elapsedTime, m_cycleInterval);
            m_fraction = cycleTime / m_cycleInterval;
        }

        if (m_onFractionChanged) m_onFractionChanged(m_fraction);

        if (m_wasActive != m_isActive && m_onActiveChanged)
        {
            m_onActiveChanged(m_isActive);
        }
    }

    // TimeTriggerImpl
    HRESULT TimeTriggerImpl::Initialize(TimeTriggerNode* node)
    {
        if (!node) return E_POINTER;
        m_enabled = node->m_enabled;
        m_triggerTime = node->m_triggerTime;
        return S_OK;
    }

    void TimeTriggerImpl::Shutdown() {}

    bool TimeTriggerImpl::ShouldTrigger(double time) const
    {
        if (!m_enabled || m_triggered) return false;
        if (time >= m_triggerTime)
        {
            const_cast<TimeTriggerImpl*>(this)->m_triggered = true;
            return true;
        }
        return false;
    }

    // SwitchImpl
    HRESULT SwitchImpl::Initialize(SwitchNode* node)
    {
        if (!node) return E_POINTER;
        m_choice = node->m_choice;
        m_enabled = node->m_enabled;
        return S_OK;
    }

    void SwitchImpl::Shutdown() {}

    void SwitchImpl::NextChoice(int maxChoices)
    {
        m_choice = (m_choice + 1) % maxChoices;
    }

    void SwitchImpl::PreviousChoice()
    {
        m_choice = m_choice > 0 ? m_choice - 1 : 0;
    }

} // namespace HMREngine
