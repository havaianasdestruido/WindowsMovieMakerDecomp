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
    // Reference export is ??0Exception@Base@@IAE@J@Z (explicit ctor);
    // the recreation keeps the ctor non-explicit for consumer compatibility
    // and binds the reference name to this ctor in the .def file.
    Exception(HRESULT hr);
    Exception(const Exception& other);
    virtual ~Exception();

    // Accessors
    HRESULT GetHResult() const throw();
    void SetHResult(HRESULT hr) throw();

    // Thread that constructed the exception
    // (reference export ?ThreadID@Exception@Base@@QBEKXZ)
    DWORD ThreadID() const throw();

    // Reference export ?Init@Exception@Base@@AAEXJ@Z (private in the
    // original); public here so the .def alias resolves to the plain
    // public-member symbol (identical ABI).
    void Init(HRESULT hr);

    // Throws this exception (re-throws via SEH)
    void Throw() const;

    // Template assignment
    Exception& operator=(const Exception& other);

private:
    HRESULT m_hr;
    DWORD   m_threadId;

    // Placement new/delete for exception throwing
    void* operator new(size_t size);
    void  operator delete(void* p) throw();

    friend WLXPHOTOBASE_API void __stdcall Throw(HRESULT hr);
    friend WLXPHOTOBASE_API void __stdcall ThrowLastError();
};

// ============================================================================
// Base::OutOfMemoryException - thrown on allocation failure paths
// (reference exports ??0OutOfMemoryException@Base@@IAE@XZ / QAE@ABV01@@Z /
//  ??1...UAE@XZ / ??4...QAEAAV01@ABV01@@Z / vtable)
// ============================================================================
class WLXPHOTOBASE_API OutOfMemoryException : public Exception
{
public:
    // Reference export is ??0OutOfMemoryException@Base@@IAE@XZ (explicit);
    // bound via .def alias for link safety (see WLXPhotoBase.def).
    OutOfMemoryException();
    OutOfMemoryException(const OutOfMemoryException& other);
    virtual ~OutOfMemoryException();
    OutOfMemoryException& operator=(const OutOfMemoryException& other);
};

// ============================================================================
// Base::Version - version number (major.minor.build.revision)
// ============================================================================
class WLXPHOTOBASE_API Version
{
public:
    Version();
    Version(const Version& other);
    Version& operator=(const Version& other);

    bool operator<(const Version& other) const;
    bool operator>(const Version& other) const;

    void Set(const VS_FIXEDFILEINFO& fvi);
    void Set(unsigned short major, unsigned short minor,
             unsigned short build, unsigned short revision);
    void Set(unsigned long majorMinor, unsigned long buildRevision);

    bool IsValid() const;
    void Invalidate();
    void AsString(class String* str) const;

private:
    unsigned short m_major;
    unsigned short m_minor;
    unsigned short m_build;
    unsigned short m_revision;
    bool           m_valid;
};

// ============================================================================
// Base free functions
// ============================================================================
// Throws a Base::Exception with the given HRESULT via SEH
// (reference: ?Throw@Base@@YGXJ@Z - __stdcall)
WLXPHOTOBASE_API void __stdcall Throw(HRESULT hr);

// Throws a Base::Exception using the last Win32 error (GetLastError)
// (reference: ?ThrowLastError@Base@@YGXXZ)
WLXPHOTOBASE_API void __stdcall ThrowLastError();

// Maps a GDI+ status value to a corresponding HRESULT.
// Reference signature takes a plain unsigned int (H), not the enum:
// ?GdiplusStatusToHresult@Base@@YGJH@Z
WLXPHOTOBASE_API HRESULT __stdcall GdiplusStatusToHresult(unsigned int status);

// ============================================================================
// Base::String - ATL string manager holder
// Reference export: ?GetBaseStringManager@String@Base@@SGAAVCAtlStringMgr@ATL@@XZ
// (a CLASS with a static __stdcall member returning ATL::CAtlStringMgr&)
// ============================================================================
class WLXPHOTOBASE_API String
{
public:
    // Returns the global base string manager for ATL CStringT instances
    static ATL::CAtlStringMgr& GetBaseStringManager();
};

// Base::StringCom - COM string manager holder
// Reference export: ?GetBaseStringComManager@StringCom@Base@@SGAAVCAtlStringMgr@ATL@@XZ
class WLXPHOTOBASE_API StringCom
{
public:
    static ATL::CAtlStringMgr& GetBaseStringComManager();
};

// ============================================================================
// Base::OS - OS version detection (namespace-scope __stdcall functions)
// Reference: ?IsVistaOrGreater@OS@Base@@YG_NXZ / ?IsWin7OrGreater... / ?IsWin8OrGreater...
// ============================================================================
namespace OS
{
    WLXPHOTOBASE_API bool __stdcall IsVistaOrGreater();
    WLXPHOTOBASE_API bool __stdcall IsWin7OrGreater();
    WLXPHOTOBASE_API bool __stdcall IsWin8OrGreater();
}

// ============================================================================
// Base::CPU - processor information (namespace-scope __stdcall functions)
// Reference: ?GetProcessorCaps@CPU@Base@@YGXAATCPUCaps@12@@Z /
//            ?GetProcessorCount@CPU@Base@@YGHXZ
// ============================================================================
namespace CPU
{
    struct TCPUCaps
    {
        DWORD dwSSE;        // SSE feature flags
        DWORD dwSSE2;       // SSE2 feature flags
        DWORD dw3DNow;      // 3DNow feature flags
        DWORD dwVendor;     // CPU vendor signature
    };

    WLXPHOTOBASE_API void __stdcall GetProcessorCaps(TCPUCaps& caps);
    WLXPHOTOBASE_API int  __stdcall GetProcessorCount();
}

} // namespace Base

// ============================================================================
// BasePrivate - low-level memory management and error reporting
// (top-level namespace in the reference - NOT Base::Private:
//  ?New@BasePrivate@@YAPAXI_N@Z / ?Delete@BasePrivate@@YAXPAX@Z /
//  ?VerifyPtr@BasePrivate@@YA_NPBX@Z / ?ReportError@BasePrivate@@...)
// ============================================================================
namespace BasePrivate
{
    // Allocates a block from the process heap
    WLXPHOTOBASE_API void* __cdecl New(unsigned int size, bool zeroInit = false);

    // Frees a block allocated by BasePrivate::New
    WLXPHOTOBASE_API void __cdecl Delete(void* p);

    // Returns true if p is a pointer into a block allocated by New
    WLXPHOTOBASE_API bool __cdecl VerifyPtr(const void* p);

    // Logs a failed assertion/condition. Real implementation: formats the
    // condition and routes it through OutputDebugString + WER when available.
    WLXPHOTOBASE_API bool __cdecl ReportError(
        const char* file, unsigned short line, const wchar_t* func,
        const char* condition, char* message, const char* module,
        unsigned long hrError, unsigned short flags, const char* extra);

    // Assert-inhibition counter holder
    struct AssertInhibitor
    {
        static int s_nAssertsInhibited;
    };
}

// ============================================================================
// Base error reporting / assert / module-info infrastructure
// (reference exports - see analysis/WLXPhotoBase/analysis.md export table)
// ============================================================================
namespace Base
{
    struct ModuleAddresses
    {
        void*  lpBaseOfDll;
        DWORD  dwSize;
    };

    struct ModuleVersion
    {
        unsigned short major;
        unsigned short minor;
        unsigned short build;
        unsigned short revision;
    };

    struct ReportMetrics      { DWORD cMetrics; DWORD pad; };
    struct ReportsForSqm      { DWORD cReports; DWORD pad; };
    struct ReportsForWer      { DWORD cReports; DWORD pad; };

    // Fill in the base address and size of a loaded module
    WLXPHOTOBASE_API HRESULT __stdcall GetModuleAddresses(void* pModule, ModuleAddresses* pAddrs);

    // Read the PE version resource of a module
    WLXPHOTOBASE_API HRESULT __stdcall GetModuleVersion(HINSTANCE* pHInstance, ModuleVersion* pVersion);

    // Ship-assert control
    WLXPHOTOBASE_API void __stdcall DisableShipAsserts();
    WLXPHOTOBASE_API void __stdcall EnableShipAsserts(void* pOwner, void* pContext, ModuleVersion* pVersion);
    WLXPHOTOBASE_API void __stdcall IncrementNoAssertCount();
    WLXPHOTOBASE_API void __stdcall DecrementNoAssertCount();
    WLXPHOTOBASE_API int  __stdcall NoAssertCount();

    // Assert callback (function-pointer typedef matching
    // ?GetAssertCallback@Base@@YGAAP6G_NPBDH0@ZXZ)
    typedef unsigned int (WINAPI *PAssertCallback)(unsigned int, const char*,
                                                   unsigned short, unsigned int);
    WLXPHOTOBASE_API PAssertCallback __stdcall GetAssertCallback();

    // True if hr is an out-of-memory HRESULT (E_OUTOFMEMORY / 0x8007000E)
    WLXPHOTOBASE_API bool __stdcall IsOutOfMemoryError(HRESULT hr);

    // Fault/report plumbing (no-op unless a report sink is attached)
    WLXPHOTOBASE_API void __stdcall ReportFault(HRESULT hr);
    WLXPHOTOBASE_API void __stdcall GetReportMetrics(ReportMetrics* pMetrics);
    WLXPHOTOBASE_API void __stdcall GetReportsForSqm(ReportsForSqm* pReports);
    WLXPHOTOBASE_API void __stdcall GetReportsForWer(ReportsForWer* pReports);
    WLXPHOTOBASE_API void __stdcall SetReportsForWer(ReportsForWer* pReports);

    // CRT leak tracking toggle (calls _CrtSetDbgFlag when the CRT allows it)
    WLXPHOTOBASE_API void __stdcall EnableLeakTrackingAndSetSymbolPath(bool enable);
}

// ============================================================================
// ATL::BaseAtlThrow - ATL throw funnel into Base::Exception
// (reference exports ?BaseAtlThrow@ATL@@YGXJ@Z / ?BaseAtlThrowLastError@ATL@@YGXXZ)
// ============================================================================
namespace ATL
{
    WLXPHOTOBASE_API void __stdcall BaseAtlThrow(HRESULT hr);
    WLXPHOTOBASE_API void __stdcall BaseAtlThrowLastError();
}

// ============================================================================
// Include implementation details (BaseTypes) after the API declarations
// ============================================================================
#include "BaseTypes.h"

#pragma warning(pop)

#endif // WLXPHOTOBASE_H
