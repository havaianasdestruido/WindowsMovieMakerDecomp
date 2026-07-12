#pragma once
// SensorImpl.h - TimeSensor, TimeTrigger, Switch implementations

#include "HMREngine.h"
#include "X3DNodes.h"
#include <functional>

namespace HMREngine
{
    class TimeSensorImpl : public ImplObject<TimeSensorImpl>
    {
    public:
        TimeSensorImpl();
        virtual ~TimeSensorImpl();

        HRESULT Initialize(TimeSensorNode* node);
        void Shutdown();

        void Update(double globalTime);

        double GetFraction() const { return m_fraction; }
        double GetElapsedTime() const { return m_elapsedTime; }
        bool IsActive() const { return m_isActive; }

        void SetOnFractionChanged(std::function<void(double)> cb) { m_onFractionChanged = cb; }
        void SetOnActiveChanged(std::function<void(bool)> cb) { m_onActiveChanged = cb; }

    private:
        TimeSensorNode* m_node = nullptr;
        double m_startTime = 0.0;
        double m_stopTime = 0.0;
        double m_cycleInterval = 1.0;
        bool m_loop = false;
        bool m_enabled = true;

        double m_fraction = 0.0;
        double m_elapsedTime = 0.0;
        bool m_isActive = false;
        bool m_wasActive = false;

        std::function<void(double)> m_onFractionChanged;
        std::function<void(bool)> m_onActiveChanged;
    };

    class TimeTriggerImpl : public ImplObject<TimeTriggerImpl>
    {
    public:
        HRESULT Initialize(TimeTriggerNode* node);
        void Shutdown();
        bool ShouldTrigger(double time) const;

    private:
        bool m_enabled = true;
        double m_triggerTime = 0.0;
        bool m_triggered = false;
    };

    class SwitchImpl : public ImplObject<SwitchImpl>
    {
    public:
        HRESULT Initialize(SwitchNode* node);
        void Shutdown();

        void SetChoice(int choice) { m_choice = choice; }
        int GetChoice() const { return m_choice; }
        void NextChoice(int maxChoices);
        void PreviousChoice();

        bool IsEnabled() const { return m_enabled; }

    private:
        int m_choice = 0;
        bool m_enabled = true;
    };

} // namespace HMREngine
