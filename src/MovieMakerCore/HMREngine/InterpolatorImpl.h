#pragma once
// InterpolatorImpl.h - InterpolatorNodes (keyframe animation)

#include "HMREngine.h"
#include "X3DNodes.h"
#include "X3DMath.h"
#include <vector>

namespace HMREngine
{
    class PositionInterpolatorImpl : public ImplObject<PositionInterpolatorImpl>
    {
    public:
        HRESULT Initialize(PositionInterpolatorNode* node);
        void Shutdown();

        void SetKeyFraction(float fraction);
        Vec3 GetCurrentValue() const { return m_currentValue; }
        bool IsEnabled() const { return m_enabled; }

    private:
        MFFloat m_key;
        MFVec3f m_value;
        bool m_enabled = true;
        Vec3 m_currentValue;
    };

    class OrientationInterpolatorImpl : public ImplObject<OrientationInterpolatorImpl>
    {
    public:
        HRESULT Initialize(OrientationInterpolatorNode* node);
        void Shutdown();

        void SetKeyFraction(float fraction);
        Rotation4f GetCurrentValue() const { return m_currentValue; }
        bool IsEnabled() const { return m_enabled; }

    private:
        MFFloat m_key;
        MFRotation m_value;
        bool m_enabled = true;
        Rotation4f m_currentValue;
    };

    class ScalarInterpolatorImpl : public ImplObject<ScalarInterpolatorImpl>
    {
    public:
        HRESULT Initialize(ScalarInterpolatorNode* node);
        void Shutdown();

        void SetKeyFraction(float fraction);
        float GetCurrentValue() const { return m_currentValue; }
        bool IsEnabled() const { return m_enabled; }

    private:
        MFFloat m_key;
        MFFloat m_value;
        bool m_enabled = true;
        float m_currentValue = 0.0f;
    };

    class ColorInterpolatorImpl : public ImplObject<ColorInterpolatorImpl>
    {
    public:
        HRESULT Initialize(ColorInterpolatorNode* node);
        void Shutdown();

        void SetKeyFraction(float fraction);
        Rgb GetCurrentValue() const { return m_currentValue; }
        bool IsEnabled() const { return m_enabled; }

    private:
        MFFloat m_key;
        MFColor m_value;
        bool m_enabled = true;
        Rgb m_currentValue;
    };

    class CoordinateInterpolatorImpl : public ImplObject<CoordinateInterpolatorImpl>
    {
    public:
        HRESULT Initialize(CoordinateInterpolatorNode* node);
        void Shutdown();

        void SetKeyFraction(float fraction);
        const std::vector<Vec3>& GetCurrentValues() const { return m_currentValues; }
        bool IsEnabled() const { return m_enabled; }

    private:
        MFFloat m_key;
        MFVec3f m_value;
        bool m_enabled = true;
        std::vector<Vec3> m_currentValues;
    };

} // namespace HMREngine
