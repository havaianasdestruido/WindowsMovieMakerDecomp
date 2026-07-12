#pragma once
// X3DSensorImpls.h - Sensor, time-dependent, sound node bridges

#include "X3DNodeImplBase.h"
#include "../X3DNodes.h"

namespace HMREngine
{
    // --- X3DSensorNodeImpl: Sensor bridge ---
    class X3DSensorNodeImpl : public X3DChildNodeImpl, public IsActiveBase
    {
    public:
        X3DSensorNodeImpl();
        virtual ~X3DSensorNodeImpl();

        HRESULT Initialize(X3DChildNode* node);
        void Shutdown() override;

        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled);

    protected:
        bool m_enabled = true;

        HRESULT SetupFields() override;
        void OnActiveChanged(bool active) override;
    };

    // --- X3DTimeDependentObjectImpl: Time-dependent bridge ---
    class X3DTimeDependentObjectImpl : public X3DChildNodeImpl
    {
    public:
        X3DTimeDependentObjectImpl();
        virtual ~X3DTimeDependentObjectImpl();

        HRESULT Initialize(X3DChildNode* node);
        void Shutdown() override;

        double GetStartTime() const { return m_startTime; }
        double GetStopTime() const { return m_stopTime; }
        double GetPauseTime() const { return m_pauseTime; }
        double GetResumeTime() const { return m_resumeTime; }

        void SetStartTime(double t) { m_startTime = t; MarkDirty(); }
        void SetStopTime(double t) { m_stopTime = t; MarkDirty(); }
        void SetPauseTime(double t) { m_pauseTime = t; MarkDirty(); }
        void SetResumeTime(double t) { m_resumeTime = t; MarkDirty(); }

        bool IsTimeEnabled() const { return m_timeEnabled; }
        void SetTimeEnabled(bool enabled) { m_timeEnabled = enabled; }

        double GetElapsed() const { return m_elapsed; }
        double GetFraction() const { return m_fraction; }

        virtual void Evaluate(double globalTime);

    protected:
        double m_startTime = 0.0;
        double m_stopTime = 0.0;
        double m_pauseTime = 0.0;
        double m_resumeTime = 0.0;
        bool m_timeEnabled = true;
        double m_elapsed = 0.0;
        double m_fraction = 0.0;
        bool m_timeActive = false;

        HRESULT SetupFields() override;
    };

    // --- X3DSoundNodeImpl: Sound bridge ---
    class X3DSoundNodeImpl : public X3DChildNodeImpl
    {
    public:
        X3DSoundNodeImpl();
        virtual ~X3DSoundNodeImpl();

        HRESULT Initialize(X3DChildNode* node);
        void Shutdown() override;

        float GetIntensity() const { return m_intensity; }
        void SetIntensity(float i) { m_intensity = i; MarkDirty(); }

        float GetPriority() const { return m_priority; }
        void SetPriority(float p) { m_priority = p; MarkDirty(); }

        bool IsSpatialize() const { return m_spatialize; }
        void SetSpatialize(bool s) { m_spatialize = s; MarkDirty(); }

    protected:
        float m_intensity = 1.0f;
        float m_priority = 0.0f;
        bool m_spatialize = false;

        HRESULT SetupFields() override;
    };

    // --- X3DSoundSourceObjectImpl: Sound source object bridge ---
    class X3DSoundSourceObjectImpl : public X3DNodeImpl
    {
    public:
        X3DSoundSourceObjectImpl();
        virtual ~X3DSoundSourceObjectImpl();

        HRESULT Initialize(X3DNode* node);
        void Shutdown() override;

        double GetStartTime() const { return m_startTime; }
        double GetPauseTime() const { return m_pauseTime; }
        double GetResumeTime() const { return m_resumeTime; }
        bool IsLoop() const { return m_loop; }
        double GetSpeed() const { return m_speed; }
        double GetDuration() const { return m_duration; }
        bool IsPause() const { return m_paused; }

        void SetLoop(bool loop) { m_loop = loop; }
        void SetSpeed(double speed) { m_speed = speed; }
        void SetPaused(bool paused) { m_paused = paused; }

    protected:
        double m_startTime = 0.0;
        double m_pauseTime = 0.0;
        double m_resumeTime = 0.0;
        bool m_loop = false;
        double m_speed = 1.0;
        double m_duration = 0.0;
        bool m_paused = false;
    };

    // --- X3DSoundSourceNodeImpl: Sound source node bridge ---
    class X3DSoundSourceNodeImpl : public X3DChildNodeImpl
    {
    public:
        X3DSoundSourceNodeImpl();
        virtual ~X3DSoundSourceNodeImpl();

        HRESULT Initialize(X3DChildNode* node);
        void Shutdown() override;

        const std::string& GetDescription() const { return m_description; }
        void SetDescription(const std::string& desc) { m_description = desc; }

        bool IsActive() const { return m_active; }
        void SetActive(bool active) { m_active = active; }

    protected:
        std::string m_description;
        bool m_active = false;

        HRESULT SetupFields() override;
    };

} // namespace HMREngine
