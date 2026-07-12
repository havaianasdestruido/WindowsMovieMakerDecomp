// InterpolatorImpl.cpp - Interpolator implementations

#include "InterpolatorImpl.h"

namespace HMREngine
{
    template<typename T>
    static T InterpolateKeys(const std::vector<float>& keys, const std::vector<T>& values, float fraction)
    {
        if (keys.empty() || values.empty()) return T{};
        if (fraction <= keys.front()) return values.front();
        if (fraction >= keys.back()) return values.back();

        for (size_t i = 0; i < keys.size() - 1; i++)
        {
            if (fraction >= keys[i] && fraction <= keys[i + 1])
            {
                float t = (keys[i + 1] - keys[i]) > 0 ?
                    (fraction - keys[i]) / (keys[i + 1] - keys[i]) : 0.0f;
                return values[i] + (values[i + 1] - values[i]) * t;
            }
        }
        return values.back();
    }

    static Rotation4f SlerpKeys(const std::vector<float>& keys, const std::vector<Rotation4f>& values, float fraction)
    {
        if (keys.empty() || values.empty()) return Rotation4f();
        if (fraction <= keys.front()) return values.front();
        if (fraction >= keys.back()) return values.back();

        for (size_t i = 0; i < keys.size() - 1; i++)
        {
            if (fraction >= keys[i] && fraction <= keys[i + 1])
            {
                float t = (keys[i + 1] - keys[i]) > 0 ?
                    (fraction - keys[i]) / (keys[i + 1] - keys[i]) : 0.0f;
                XMVECTOR q1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&values[i]));
                XMVECTOR q2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&values[i + 1]));
                Rotation4f r;
                XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&r), XMQuaternionSlerp(q1, q2, t));
                return r;
            }
        }
        return values.back();
    }

    // PositionInterpolatorImpl
    HRESULT PositionInterpolatorImpl::Initialize(PositionInterpolatorNode* node)
    {
        if (!node) return E_POINTER;
        m_key = node->m_key;
        m_value = node->m_value;
        m_enabled = node->m_enabled;
        return S_OK;
    }
    void PositionInterpolatorImpl::Shutdown() {}
    void PositionInterpolatorImpl::SetKeyFraction(float fraction)
    {
        if (!m_enabled) return;
        m_currentValue = InterpolateKeys(m_key.m_value, m_value.m_value, fraction);
    }

    // OrientationInterpolatorImpl
    HRESULT OrientationInterpolatorImpl::Initialize(OrientationInterpolatorNode* node)
    {
        if (!node) return E_POINTER;
        m_key = node->m_key;
        m_value = node->m_value;
        m_enabled = node->m_enabled;
        return S_OK;
    }
    void OrientationInterpolatorImpl::Shutdown() {}
    void OrientationInterpolatorImpl::SetKeyFraction(float fraction)
    {
        if (!m_enabled) return;
        m_currentValue = SlerpKeys(m_key.m_value, m_value.m_value, fraction);
    }

    // ScalarInterpolatorImpl
    HRESULT ScalarInterpolatorImpl::Initialize(ScalarInterpolatorNode* node)
    {
        if (!node) return E_POINTER;
        m_key = node->m_key;
        m_value = node->m_value;
        m_enabled = node->m_enabled;
        return S_OK;
    }
    void ScalarInterpolatorImpl::Shutdown() {}
    void ScalarInterpolatorImpl::SetKeyFraction(float fraction)
    {
        if (!m_enabled) return;
        m_currentValue = InterpolateKeys(m_key.m_value, m_value.m_value, fraction);
    }

    // ColorInterpolatorImpl
    HRESULT ColorInterpolatorImpl::Initialize(ColorInterpolatorNode* node)
    {
        if (!node) return E_POINTER;
        m_key = node->m_key;
        m_value = node->m_value;
        m_enabled = node->m_enabled;
        return S_OK;
    }
    void ColorInterpolatorImpl::Shutdown() {}
    void ColorInterpolatorImpl::SetKeyFraction(float fraction)
    {
        if (!m_enabled) return;
        m_currentValue = InterpolateKeys(m_key.m_value, m_value.m_value, fraction);
    }

    // CoordinateInterpolatorImpl
    HRESULT CoordinateInterpolatorImpl::Initialize(CoordinateInterpolatorNode* node)
    {
        if (!node) return E_POINTER;
        m_key = node->m_key;
        m_value = node->m_value;
        m_enabled = node->m_enabled;
        return S_OK;
    }
    void CoordinateInterpolatorImpl::Shutdown() {}
    void CoordinateInterpolatorImpl::SetKeyFraction(float fraction)
    {
        if (!m_enabled) return;
        if (m_key.m_value.empty() || m_value.m_value.empty()) return;
        // For coordinate interpolators, we interpolate the entire array
        m_currentValues = m_value.m_value;
    }

} // namespace HMREngine
