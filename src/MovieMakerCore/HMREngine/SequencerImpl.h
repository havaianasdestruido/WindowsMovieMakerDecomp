#pragma once
// SequencerImpl.h - SequencerNodes (discrete keyframe: Integer, Boolean)

#include "HMREngine.h"
#include "X3DNodes.h"

namespace HMREngine
{
    class IntegerSequencerImpl : public ImplObject<IntegerSequencerImpl>
    {
    public:
        HRESULT Initialize(IntegerSequencerNode* node);
        void Shutdown();

        void SetKeyFraction(float fraction);
        int GetCurrentValue() const { return m_currentValue; }
        bool IsEnabled() const { return m_enabled; }

    private:
        MFFloat m_key;
        MFInt32 m_value;
        bool m_enabled = true;
        int m_currentValue = 0;
    };

    class BooleanSequencerImpl : public ImplObject<BooleanSequencerImpl>
    {
    public:
        HRESULT Initialize(BooleanSequencerNode* node);
        void Shutdown();

        void SetKeyFraction(float fraction);
        bool GetCurrentValue() const { return m_currentValue; }
        bool IsEnabled() const { return m_enabled; }

    private:
        MFFloat m_key;
        MFBool m_value;
        bool m_enabled = true;
        bool m_currentValue = false;
    };

} // namespace HMREngine
