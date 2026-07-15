#include "pch.h"
/*
 * SerializationContext.cpp
 *
 * Implementation of the serialization context and error guard for the
 * StoryboardManager namespace.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "SerializationContext.h"

namespace StoryboardManager
{

// ============================================================================
// SerializationContext implementation
// ============================================================================

SerializationContext::SerializationContext()
    : m_mode(SerializationModeRead)
    , m_hr(S_OK)
    , m_dwElementDepth(0)
    , m_pUserData(nullptr)
{
}

SerializationContext::SerializationContext(SerializationMode mode)
    : m_mode(mode)
    , m_hr(S_OK)
    , m_dwElementDepth(0)
    , m_pUserData(nullptr)
{
}

SerializationContext::~SerializationContext()
{
}

SerializationMode SerializationContext::GetMode() const throw()
{
    return m_mode;
}

void SerializationContext::SetMode(SerializationMode mode) throw()
{
    m_mode = mode;
}

SerializationVersion SerializationContext::GetVersion() const throw()
{
    return m_version;
}

void SerializationContext::SetVersion(const SerializationVersion& ver) throw()
{
    m_version = ver;
}

void SerializationContext::SetVersion(DWORD dwMajor, DWORD dwMinor) throw()
{
    m_version.dwMajor = dwMajor;
    m_version.dwMinor = dwMinor;
}

HRESULT SerializationContext::GetHResult() const throw()
{
    return m_hr;
}

void SerializationContext::SetHResult(HRESULT hr) throw()
{
    m_hr = hr;
}

bool SerializationContext::HasError() const throw()
{
    return FAILED(m_hr);
}

DWORD SerializationContext::GetElementDepth() const throw()
{
    return m_dwElementDepth;
}

void SerializationContext::IncrementDepth() throw()
{
    ++m_dwElementDepth;
}

void SerializationContext::DecrementDepth() throw()
{
    if (m_dwElementDepth > 0)
        --m_dwElementDepth;
}

void* SerializationContext::GetUserData() const throw()
{
    return m_pUserData;
}

void SerializationContext::SetUserData(void* pData) throw()
{
    m_pUserData = pData;
}

void SerializationContext::Reset()
{
    m_hr = S_OK;
    m_dwElementDepth = 0;
    m_pUserData = nullptr;
}

SerializationContext SerializationContext::CreateForRead()
{
    return SerializationContext(SerializationModeRead);
}

SerializationContext SerializationContext::CreateForWrite()
{
    return SerializationContext(SerializationModeWrite);
}

// ============================================================================
// SerializationErrorGuard implementation
// ============================================================================

SerializationErrorGuard::SerializationErrorGuard(SerializationContext& ctx)
    : m_ctx(ctx)
    , m_hrOriginal(ctx.GetHResult())
{
}

SerializationErrorGuard::~SerializationErrorGuard()
{
}

HRESULT SerializationErrorGuard::GetResult() const throw()
{
    return m_ctx.GetHResult();
}

void SerializationErrorGuard::SetError(HRESULT hr)
{
    m_ctx.SetHResult(hr);
}

SerializationContext& SerializationErrorGuard::GetContext()
{
    return m_ctx;
}

} // namespace StoryboardManager
