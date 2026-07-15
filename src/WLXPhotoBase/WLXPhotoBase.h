/*
 * WLXPhotoBase.h
 *
 * Shared foundation library header for Windows Live Photo Gallery / Movie Maker 2012.
 * Provides base exception handling, memory management, OS detection, GDI+ integration,
 * smart pointers, containers, and common utility classes used across all WLX components.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 * Depends on ATL/WTL, GDI+, and Windows API.
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef WLXPHOTOBASE_H
#define WLXPHOTOBASE_H

#ifndef STRICT
#define STRICT
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WINVER        0x0602
#define _WIN32_WINNT  0x0602
#define _WIN32_IE     0x0800

#include <windows.h>
#include <objbase.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <werapi.h>

#pragma warning(push)
#pragma warning(disable: 4505) // unreferenced local function has been removed
#pragma warning(disable: 4100) // unreferenced formal parameter

// Use ATL::IAtlStringMgr directly - no separate forward declaration needed
// when ATL headers are available
#include <atlbase.h>
#include <atlstr.h>

// ============================================================================
// WLXPHOTOBASE_EXPORTS - macro for dllexport/dllimport
// ============================================================================
#ifdef WLXPHOTOBASE_EXPORTS
    #define WLXPHOTOBASE_API __declspec(dllexport)
#else
    #define WLXPHOTOBASE_API __declspec(dllimport)
#endif

// ============================================================================
// Base namespace - core foundation types and utilities
// ============================================================================
namespace Base
{

// ============================================================================
// Forward declarations
// ============================================================================
class File;
class TempFile;
class Thread;
class FindFile;
class GdiException;
class NoHeap;

namespace DataStructs
{
    class IntSet;
}

// ============================================================================
// Base::Exception
// ============================================================================
// Exception class wrapping HRESULT error codes. This is the fundamental error
// handling mechanism for all WLX components. Throwing an Exception dispatches
// via structured exception handling (SEH) so callers can catch at any scope.
//
class WLXPHOTOBASE_API Exception
{
public:
    Exception(HRESULT hr);
    Exception(const Exception& other);
    virtual ~Exception();

    // Implicit conversion to HRESULT for quick error propagation
    operator HRESULT() const;

    // Accessors
    HRESULT GetHResult() const throw();
    void SetHResult(HRESULT hr) throw();

    // Throws this exception (re-throws via SEH)
    void Throw() const;

    // Template assignment
    Exception& operator=(const Exception& other);

private:
    HRESULT m_hr;

    // Placement new/delete for exception throwing
    void* operator new(size_t size);
    void  operator delete(void* p) throw();

    friend WLXPHOTOBASE_API void Throw(HRESULT hr);
    friend WLXPHOTOBASE_API void ThrowLastError();
};

// ============================================================================
// Base free functions
// ============================================================================
// Throws a Base::Exception with the given HRESULT via SEH
WLXPHOTOBASE_API void Throw(HRESULT hr);

// Throws a Base::Exception using the last Win32 error (GetLastError)
WLXPHOTOBASE_API void ThrowLastError();

// Maps a Gdiplus::Status value to a corresponding HRESULT
WLXPHOTOBASE_API HRESULT GdiplusStatusToHresult(Gdiplus::Status status);

// ============================================================================
// Base::String - thin wrapper for ATL CString
// ============================================================================
namespace String
{
    // Returns the global base string manager for ATL CStringT instances
    WLXPHOTOBASE_API ATL::IAtlStringMgr* GetBaseStringManager();
}

// ============================================================================
// Base::OS - OS version detection
// ============================================================================
namespace OS
{
    // Returns true if the current OS is Windows 7 or greater
    WLXPHOTOBASE_API bool IsWin7OrGreater();

    // Returns true if the current OS is Windows 8 or greater
    WLXPHOTOBASE_API bool IsWin8OrGreater();
}

// ============================================================================
// Base::Private - low-level memory management
// ============================================================================
namespace Private
{
    // Allocates a block from the process heap
    // size: number of bytes to allocate
    // zeroInit: if true, memory is zero-initialized
    WLXPHOTOBASE_API void* New(size_t size, bool zeroInit = false);

    // Frees a block allocated by Base::Private::New
    WLXPHOTOBASE_API void Delete(void* p);
}

// ============================================================================
// Base::CPU - processor information
// ============================================================================
namespace CPU
{
    // Returns the number of logical processors on the system
    WLXPHOTOBASE_API int GetProcessorCount();
}

} // namespace Base

// BaseAtlThrow is provided by ATL 14+ natively - no custom definition needed

// ============================================================================
// Include implementation details (BaseTypes) after the API declarations
// ============================================================================
#include "BaseTypes.h"

#pragma warning(pop)

#endif // WLXPHOTOBASE_H
