/*
 * SerializationContext.h
 *
 * Serialization context and state management for the StoryboardManager
 * namespace. Maintains the current serialization mode, version info,
 * and error state during project file read/write operations.
 *
 * RTTI: ?AVSerializationContext@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef SERIALIZATIONCONTEXT_H
#define SERIALIZATIONCONTEXT_H

#include "../StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// SerializationMode
// ============================================================================
enum SerializationMode
{
    SerializationModeRead       = 0,
    SerializationModeWrite      = 1,
    SerializationModeMemory     = 2
};

// ============================================================================
// SerializationVersion
// ============================================================================
// Tracks the .wlmp format version being read or written. Used for
// forward-compatible migration between 2011 and 2012 formats.
//
struct SerializationVersion
{
    DWORD dwMajor;
    DWORD dwMinor;

    SerializationVersion()
        : dwMajor(1)
        , dwMinor(0)
    {
    }

    SerializationVersion(DWORD dwMaj, DWORD dwMin)
        : dwMajor(dwMaj)
        , dwMinor(dwMin)
    {
    }

    bool operator>=(const SerializationVersion& other) const
    {
        return (dwMajor > other.dwMajor) ||
               (dwMajor == other.dwMajor && dwMinor >= other.dwMinor);
    }

    bool operator<(const SerializationVersion& other) const
    {
        return !(*this >= other);
    }
};

// ============================================================================
// SerializationContext
// ============================================================================
// Manages the state during a serialization operation. Carries the current
// mode (read/write), format version, error code, and user-data pointer
// used by the serialization element callbacks.
//
class STORYBOARD_API SerializationContext
{
public:
    SerializationContext();
    SerializationContext(SerializationMode mode);
    ~SerializationContext();

    // -- Mode --
    SerializationMode GetMode() const throw();
    void SetMode(SerializationMode mode) throw();

    // -- Version --
    SerializationVersion GetVersion() const throw();
    void SetVersion(const SerializationVersion& ver) throw();
    void SetVersion(DWORD dwMajor, DWORD dwMinor) throw();

    // -- Error state --
    HRESULT GetHResult() const throw();
    void SetHResult(HRESULT hr) throw();
    bool HasError() const throw();

    // -- Element depth tracking --
    DWORD GetElementDepth() const throw();
    void IncrementDepth() throw();
    void DecrementDepth() throw();

    // -- User data pointer (for element callback context) --
    void* GetUserData() const throw();
    void SetUserData(void* pData) throw();

    // -- Reset context for reuse --
    void Reset();

    // -- Static helpers --
    static SerializationContext CreateForRead();
    static SerializationContext CreateForWrite();

private:
    SerializationMode    m_mode;
    SerializationVersion m_version;
    HRESULT              m_hr;
    DWORD                m_dwElementDepth;
    void*                m_pUserData;
};

// ============================================================================
// SerializationErrorGuard
// ============================================================================
// RAII guard that captures HRESULT at scope exit. Used to propagate errors
// through serialization callback chains without explicit checking at every
// call site.
//
class STORYBOARD_API SerializationErrorGuard
{
public:
    explicit SerializationErrorGuard(SerializationContext& ctx);
    ~SerializationErrorGuard();

    HRESULT GetResult() const throw();
    void SetError(HRESULT hr);

    SerializationContext& GetContext();

private:
    SerializationContext& m_ctx;
    HRESULT              m_hrOriginal;
};

} // namespace StoryboardManager

#endif // SERIALIZATIONCONTEXT_H
