#include "pch.h"
// X3DSensorImpls.cpp - Sensor/time-dependent/sound bridge implementation

#include "X3DSensorImpls.h"

namespace HMREngine
{

// ============================================================================
// X3DSensorNodeImpl
// ============================================================================
X3DSensorNodeImpl::X3DSensorNodeImpl() = default;
X3DSensorNodeImpl::~X3DSensorNodeImpl() { Shutdown(); }

HRESULT X3DSensorNodeImpl::Initialize(X3DChildNode* node)
{
    return X3DChildNodeImpl::Initialize(node);
}

void X3DSensorNodeImpl::Shutdown()
{
    X3DChildNodeImpl::Shutdown();
}

void X3DSensorNodeImpl::SetEnabled(bool enabled)
{
    m_enabled = enabled;
    MarkDirty();
}

void X3DSensorNodeImpl::OnActiveChanged(bool active)
{
}

HRESULT X3DSensorNodeImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

// ============================================================================
// X3DTimeDependentObjectImpl
// ============================================================================
X3DTimeDependentObjectImpl::X3DTimeDependentObjectImpl() = default;
X3DTimeDependentObjectImpl::~X3DTimeDependentObjectImpl() { Shutdown(); }

HRESULT X3DTimeDependentObjectImpl::Initialize(X3DChildNode* node)
{
    if (node)
    {
        auto* td = dynamic_cast<TimeSensorNode*>(node);
        if (td)
        {
            m_startTime = td->m_startTime;
            m_stopTime = td->m_stopTime;
            m_pauseTime = td->m_pauseTime;
            m_resumeTime = td->m_resumeTime;
        }
    }
    return X3DChildNodeImpl::Initialize(node);
}

void X3DTimeDependentObjectImpl::Shutdown()
{
    X3DChildNodeImpl::Shutdown();
}

void X3DTimeDependentObjectImpl::Evaluate(double globalTime)
{
    if (!m_timeEnabled) return;

    if (!m_timeActive)
    {
        if (m_startTime > 0 && globalTime >= m_startTime)
        {
            m_timeActive = true;
            m_elapsed = 0.0;
        }
        return;
    }

    if (m_stopTime > 0 && globalTime >= m_stopTime)
    {
        m_timeActive = false;
        m_fraction = 1.0;
        return;
    }

    m_elapsed = globalTime - m_startTime;

    if (m_pauseTime > 0 && m_resumeTime <= m_pauseTime)
    {
        return;
    }

    m_fraction = 0.0;
    MarkDirty();
}

HRESULT X3DTimeDependentObjectImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

// ============================================================================
// X3DSoundNodeImpl
// ============================================================================
X3DSoundNodeImpl::X3DSoundNodeImpl() = default;
X3DSoundNodeImpl::~X3DSoundNodeImpl() { Shutdown(); }

HRESULT X3DSoundNodeImpl::Initialize(X3DChildNode* node)
{
    return X3DChildNodeImpl::Initialize(node);
}

void X3DSoundNodeImpl::Shutdown()
{
    X3DChildNodeImpl::Shutdown();
}

HRESULT X3DSoundNodeImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

// ============================================================================
// X3DSoundSourceObjectImpl
// ============================================================================
X3DSoundSourceObjectImpl::X3DSoundSourceObjectImpl() = default;
X3DSoundSourceObjectImpl::~X3DSoundSourceObjectImpl() { Shutdown(); }

HRESULT X3DSoundSourceObjectImpl::Initialize(X3DNode* node)
{
    return X3DNodeImpl::Initialize(node);
}

void X3DSoundSourceObjectImpl::Shutdown()
{
    X3DNodeImpl::Shutdown();
}

// ============================================================================
// X3DSoundSourceNodeImpl
// ============================================================================
X3DSoundSourceNodeImpl::X3DSoundSourceNodeImpl() = default;
X3DSoundSourceNodeImpl::~X3DSoundSourceNodeImpl() { Shutdown(); }

HRESULT X3DSoundSourceNodeImpl::Initialize(X3DChildNode* node)
{
    return X3DChildNodeImpl::Initialize(node);
}

void X3DSoundSourceNodeImpl::Shutdown()
{
    X3DChildNodeImpl::Shutdown();
}

HRESULT X3DSoundSourceNodeImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

} // namespace HMREngine
