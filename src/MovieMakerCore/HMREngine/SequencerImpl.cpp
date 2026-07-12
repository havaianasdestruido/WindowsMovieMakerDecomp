// SequencerImpl.cpp - Sequencer implementations

#include "SequencerImpl.h"

namespace HMREngine
{
    // IntegerSequencerImpl
    HRESULT IntegerSequencerImpl::Initialize(IntegerSequencerNode* node)
    {
        if (!node) return E_POINTER;
        m_key = node->m_key;
        m_value = node->m_value;
        m_enabled = node->m_enabled;
        return S_OK;
    }

    void IntegerSequencerImpl::Shutdown() {}

    void IntegerSequencerImpl::SetKeyFraction(float fraction)
    {
        if (!m_enabled || m_key.m_value.empty() || m_value.m_value.empty()) return;

        if (fraction <= m_key.m_value.front())
        {
            m_currentValue = m_value.m_value.front();
            return;
        }

        for (size_t i = 0; i < m_key.m_value.size(); i++)
        {
            if (fraction < m_key.m_value[i] && i > 0)
            {
                m_currentValue = m_value.m_value[i - 1];
                return;
            }
        }

        m_currentValue = m_value.m_value.back();
    }

    // BooleanSequencerImpl
    HRESULT BooleanSequencerImpl::Initialize(BooleanSequencerNode* node)
    {
        if (!node) return E_POINTER;
        m_key = node->m_key;
        m_value = node->m_value;
        m_enabled = node->m_enabled;
        return S_OK;
    }

    void BooleanSequencerImpl::Shutdown() {}

    void BooleanSequencerImpl::SetKeyFraction(float fraction)
    {
        if (!m_enabled || m_key.m_value.empty() || m_value.m_value.empty()) return;

        if (fraction <= m_key.m_value.front())
        {
            m_currentValue = m_value.m_value.front();
            return;
        }

        for (size_t i = 0; i < m_key.m_value.size(); i++)
        {
            if (fraction < m_key.m_value[i] && i > 0)
            {
                m_currentValue = m_value.m_value[i - 1];
                return;
            }
        }

        m_currentValue = m_value.m_value.back();
    }

} // namespace HMREngine
