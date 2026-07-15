/*
 * BaseTypes.h
 *
 * Core types and template classes used across the Windows Live Photo Gallery /
 * Movie Maker 2012 codebase. Provides reference counting, smart pointers,
 * containers, wrappers for Win32 types, and utility class templates.
 *
 * This header is included by WLXPhotoBase.h and consumed by all WLX components.
 * Built with MSVC 11.0 (VS2012), requires ATL headers at include site.
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef BASETYPES_H
#define BASETYPES_H

#ifndef STRICT
#define STRICT
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define WINVER        0x0602
#define _WIN32_WINNT  0x0602
#define _WIN32_IE     0x0800

#include <windows.h>
#include <objbase.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlsync.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <new>

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4505) // unreferenced local function has been removed
#pragma warning(disable: 4512) // assignment operator could not be generated
#pragma warning(disable: 4127) // conditional expression is constant

// ============================================================================
// Base namespace
// ============================================================================
namespace Base
{

// ============================================================================
// Forward declarations
// ============================================================================
class Exception;

// ============================================================================
// Base::RefCountBase<T> - single-threaded reference counting base class
// ============================================================================
// Intrusive reference counting base. Derive from this to get AddRef/Release
// semantics. Not thread-safe; use RefCountBaseMultiThreaded for MT scenarios.
//
template <typename T>
class RefCountBase
{
public:
    RefCountBase() : m_cRef(1) {}

    virtual ~RefCountBase() {}

    ULONG STDMETHODCALLTYPE AddRef() throw()
    {
        return InterlockedIncrement(&m_cRef);
    }

    ULONG STDMETHODCALLTYPE Release() throw()
    {
        ULONG cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete static_cast<T*>(this);
        }
        return cRef;
    }

    // QueryInterface support - returns S_OK for IUnknown only by default.
    // Derived classes should override to support additional interfaces.
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) throw()
    {
        if (!ppvObject)
            return E_POINTER;

        *ppvObject = NULL;

        if (IsEqualIID(riid, IID_IUnknown))
        {
            *ppvObject = static_cast<IUnknown*>(static_cast<RefCountBase<T>*>(this));
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG GetRefCount() const throw()
    {
        return m_cRef;
    }

protected:
    LONG m_cRef;
};

// ============================================================================
// Base::RefCountBaseMultiThreaded<T> - MT reference counting base class
// ============================================================================
// Thread-safe intrusive reference counting. Uses volatile long for m_cRef.
// Use this when the object may be shared across apartment/thread boundaries.
//
template <typename T>
class RefCountBaseMultiThreaded
{
public:
    RefCountBaseMultiThreaded() : m_cRef(1) {}

    virtual ~RefCountBaseMultiThreaded() {}

    ULONG STDMETHODCALLTYPE AddRef() throw()
    {
        return InterlockedIncrement(&m_cRef);
    }

    ULONG STDMETHODCALLTYPE Release() throw()
    {
        ULONG cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete static_cast<T*>(this);
        }
        return cRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) throw()
    {
        if (!ppvObject)
            return E_POINTER;

        *ppvObject = NULL;

        if (IsEqualIID(riid, IID_IUnknown))
        {
            *ppvObject = static_cast<IUnknown*>(static_cast<RefCountBaseMultiThreaded<T>*>(this));
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG GetRefCount() const throw()
    {
        return m_cRef;
    }

protected:
    volatile LONG m_cRef;
};

// ============================================================================
// Base::PtrRef<T> - intrusive smart pointer
// ============================================================================
// Non-owning intrusive reference counted smart pointer. Requires T to implement
// AddRef()/Release() (e.g., derive from RefCountBase or IUnknown).
//
template <typename T>
class PtrRef
{
public:
    PtrRef() throw()
        : m_p(NULL)
    {
    }

    PtrRef(T* p) throw()
        : m_p(p)
    {
        if (m_p)
            m_p->AddRef();
    }

    PtrRef(const PtrRef& other) throw()
        : m_p(other.m_p)
    {
        if (m_p)
            m_p->AddRef();
    }

    template <typename U>
    PtrRef(const PtrRef<U>& other) throw()
        : m_p(static_cast<T*>(other.Get()))
    {
        if (m_p)
            m_p->AddRef();
    }

    ~PtrRef() throw()
    {
        if (m_p)
        {
            m_p->Release();
            m_p = NULL;
        }
    }

    PtrRef& operator=(T* p) throw()
    {
        if (p != m_p)
        {
            T* pOld = m_p;
            m_p = p;
            if (m_p)
                m_p->AddRef();
            if (pOld)
                pOld->Release();
        }
        return *this;
    }

    PtrRef& operator=(const PtrRef& other) throw()
    {
        return operator=(other.m_p);
    }

    T* operator->() const throw()
    {
        ATLASSERT(m_p != NULL);
        return m_p;
    }

    T& operator*() const throw()
    {
        ATLASSERT(m_p != NULL);
        return *m_p;
    }

    T* Get() const throw()
    {
        return m_p;
    }

    T** GetAddressOf() throw()
    {
        Release();
        m_p = NULL;
        return &m_p;
    }

    T** GetReleaseAddress() throw()
    {
        return &m_p;
    }

    void Release() throw()
    {
        T* pOld = m_p;
        m_p = NULL;
        if (pOld)
            pOld->Release();
    }

    bool operator!() const throw()
    {
        return (m_p == NULL);
    }

    bool operator==(T* p) const throw()
    {
        return m_p == p;
    }

    bool operator!=(T* p) const throw()
    {
        return m_p != p;
    }

    bool operator==(const PtrRef& other) const throw()
    {
        return m_p == other.m_p;
    }

    bool operator!=(const PtrRef& other) const throw()
    {
        return m_p != other.m_p;
    }

    // Attach without AddRef (takes ownership of caller's ref)
    void Attach(T* p) throw()
    {
        if (m_p)
            m_p->Release();
        m_p = p;
    }

    // Detach without Release (caller takes ownership)
    T* Detach() throw()
    {
        T* p = m_p;
        m_p = NULL;
        return p;
    }

private:
    T* m_p;
};

// ============================================================================
// Base::Array<T> - thin wrapper around ATL CAtlArray
// ============================================================================
// Dynamically sized array backed by CAtlArray. Provides simple push/pop/iterate
// interface. Elements must be default-constructible.
//
template <typename T>
class Array
{
public:
    typedef ATL::CAtlArray<T> ContainerType;
    typedef void* POSITION;

    Array() throw()
    {
    }

    ~Array() throw()
    {
        m_arr.RemoveAll();
    }

    // Copy semantics
    Array(const Array& other)
    {
        m_arr.Copy(other.m_arr);
    }

    Array& operator=(const Array& other)
    {
        if (this != &other)
        {
            m_arr.Copy(other.m_arr);
        }
        return *this;
    }

    // Capacity
    size_t GetCount() const throw()
    {
        return m_arr.GetCount();
    }

    bool IsEmpty() const throw()
    {
        return m_arr.IsEmpty();
    }

    size_t GetCapacity() const throw()
    {
        return m_arr.GetCapacity();
    }

    void SetCapacity(size_t nNewCapacity)
    {
        m_arr.SetCapacity(nNewCapacity);
    }

    // Element access
    T& operator[](size_t nIndex)
    {
        ATLASSERT(nIndex < m_arr.GetCount());
        return m_arr[nIndex];
    }

    const T& operator[](size_t nIndex) const
    {
        ATLASSERT(nIndex < m_arr.GetCount());
        return m_arr[nIndex];
    }

    T& GetAt(size_t nIndex)
    {
        ATLASSERT(nIndex < m_arr.GetCount());
        return m_arr.GetAt(nIndex);
    }

    const T& GetAt(size_t nIndex) const
    {
        ATLASSERT(nIndex < m_arr.GetCount());
        return m_arr.GetAt(nIndex);
    }

    // Modification
    size_t Add(const T& element)
    {
        return m_arr.Add(element);
    }

    void InsertAt(size_t nIndex, const T& element)
    {
        m_arr.InsertAt(nIndex, element);
    }

    void RemoveAt(size_t nIndex)
    {
        m_arr.RemoveAt(nIndex);
    }

    void RemoveAll() throw()
    {
        m_arr.RemoveAll();
    }

    // Position-based iteration (ATL-style)
    POSITION GetStartPosition() const throw()
    {
        if (m_arr.IsEmpty())
            return NULL;
        return reinterpret_cast<POSITION>(1);
    }

    T& GetNext(POSITION& pos)
    {
        size_t nIndex = reinterpret_cast<size_t>(pos) - 1;
        ATLASSERT(nIndex < m_arr.GetCount());
        if (nIndex + 1 < m_arr.GetCount())
            pos = reinterpret_cast<POSITION>(nIndex + 2);
        else
            pos = NULL;
        return m_arr[nIndex];
    }

private:
    ContainerType m_arr;
};

// ============================================================================
// Base::NoHeap - base class to force stack allocation
// ============================================================================
// Inherit from this to prevent heap allocation of a class. Overloaded new/delete
// will assert/fail if someone tries to heap-allocate an object of this type.
//
class NoHeap
{
public:
    NoHeap() throw() {}

    void* operator new(size_t size)
    {
        // Objects of this type must not be heap-allocated.
        // Use stack allocation or placement new only.
        ATLASSERT(FALSE && "NoHeap: heap allocation prohibited");
        return ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    }

    void operator delete(void* p) throw()
    {
        if (p)
            ::HeapFree(::GetProcessHeap(), 0, p);
    }

    // Placement new/delete - always allowed
    void* operator new(size_t size, void* p) throw()
    {
        (size); // suppress unused parameter warning
        return p;
    }

    void operator delete(void* /*p*/, void* /*place*/) throw()
    {
    }

private:
    // Prohibit array new/delete
    void* operator new[](size_t size);
    void  operator delete[](void* p) throw();
};

// ============================================================================
// Base::File - Win32 file handle wrapper
// ============================================================================
// RAII wrapper for Win32 file handles. Provides path query, size, and validity
// checks. Supports both read and write modes.
//
class WLXPHOTOBASE_API File
{
public:
    File() throw();
    ~File() throw();

    // Opens a file. Returns true on success.
    bool Open(LPCWSTR pszPath, DWORD dwAccess = GENERIC_READ,
              DWORD dwShareMode = FILE_SHARE_READ,
              DWORD dwCreation = OPEN_EXISTING);

    // Closes the handle
    void Close() throw();

    // Returns true if the handle is valid
    bool IsValid() const throw();

    // Returns the file name portion of the path
    ATL::CString GetFileName() const;

    // Returns the full directory path
    ATL::CString GetPath() const;

    // Returns the full file path
    ATL::CString GetFullPath() const;

    // Returns the file size in bytes
    ULONGLONG GetFileSize() const;

    // Returns the underlying HANDLE (does not transfer ownership)
    HANDLE GetHandle() const throw();

    // Reads data into a buffer. Returns bytes read, 0 on failure.
    DWORD Read(LPVOID pBuffer, DWORD cbToRead);

    // Writes data from a buffer. Returns bytes written, 0 on failure.
    DWORD Write(LPCVOID pBuffer, DWORD cbToWrite);

    // Seeks to a position. Returns new position, INVALID_SET_FILE_POINTER on error.
    ULONGLONG Seek(LONGLONG llOffset, DWORD dwMethod = FILE_BEGIN);

private:
    HANDLE m_hFile;
    ATL::CString m_strPath;
};

// ============================================================================
// Base::TempFile - temporary file management
// ============================================================================
// Creates and manages a temporary file that is automatically deleted when closed.
// Backed by GetTempPath/GetTempFileName Win32 APIs.
//
class WLXPHOTOBASE_API TempFile
{
public:
    TempFile() throw();
    ~TempFile() throw();

    // Creates a temporary file in the given directory (or %TEMP% if NULL)
    bool Create(LPCWSTR pszDir = NULL);

    // Closes and optionally deletes the temporary file
    void Close();

    // Returns the path to the temporary file
    ATL::CString GetPath() const;

    // Returns the underlying File object
    const File& GetFile() const throw();
    File& GetFile() throw();

    bool IsValid() const throw();

private:
    File m_file;
    ATL::CString m_strPath;
};

// ============================================================================
// Base::Thread - simple thread wrapper
// ============================================================================
// Wraps a Win32 thread. Provides start/stop/wait semantics.
// Override the virtual ThreadProc to implement your thread body.
//
class WLXPHOTOBASE_API Thread
{
public:
    Thread() throw();
    virtual ~Thread() throw();

    // Starts the thread
    HRESULT Start();

    // Signals the thread to stop and waits for it (with timeout)
    void Stop(DWORD dwTimeoutMs = INFINITE);

    // Waits for the thread to finish
    HRESULT Wait(DWORD dwTimeoutMs = INFINITE);

    // Returns true if the thread is currently running
    bool IsRunning() const throw();

    // Returns the thread handle
    HANDLE GetHandle() const throw();

    // Returns the thread ID
    DWORD GetThreadId() const throw();

protected:
    // Override this to implement thread body. Return value becomes thread exit code.
    virtual DWORD STDMETHODCALLTYPE ThreadProc() = 0;

    // Override to signal the thread it should exit. Default sets a flag.
    virtual bool ShouldStop() const throw();

private:
    static DWORD WINAPI ThreadProcThunk(LPVOID pParam);

    HANDLE m_hThread;
    DWORD  m_dwThreadId;
    volatile bool m_fRunning;
    volatile bool m_fStopRequested;
};

// ============================================================================
// Base::FindFile - Win32 FindFirstFile/FindNextFile wrapper
// ============================================================================
// RAII wrapper for directory enumeration. Supports wildcard patterns.
//
class WLXPHOTOBASE_API FindFile
{
public:
    FindFile() throw();
    ~FindFile() throw();

    // Begins searching for files matching the pattern
    bool FindFirstFile(LPCWSTR pszPattern);

    // Advances to the next match
    bool FindNextFile();

    // Closes the search handle
    void Close() throw();

    // Returns the current file name
    ATL::CString GetFileName() const;

    // Returns the full path of the current match
    ATL::CString GetFilePath() const;

    // Returns the directory portion of the search pattern
    ATL::CString GetDirectory() const;

    // Returns true if the current match is a directory
    bool IsDirectory() const throw();

    // Returns true if the current match is a regular file (not directory)
    bool IsFile() const throw();

    // Returns the file size of the current match
    ULONGLONG GetFileSize() const throw();

    // Returns the underlying WIN32_FIND_DATA
    const WIN32_FIND_DATAW& GetFindData() const throw();

private:
    HANDLE          m_hFind;
    WIN32_FIND_DATAW m_findData;
    ATL::CString    m_strDirectory;
    bool            m_fFirst;
};

// ============================================================================
// Base::GdiException - GDI+ error wrapper
// ============================================================================
// Extends Base::Exception to carry a Gdiplus::Status code in addition to HRESULT.
// Automatically constructed from a Gdiplus::Status value.
//
class WLXPHOTOBASE_API GdiException : public Exception
{
public:
    explicit GdiException(Gdiplus::Status status);
    virtual ~GdiException() throw();

    // Returns the original GDI+ status code
    Gdiplus::Status GetGdiplusStatus() const throw();

    virtual void Throw() const;

private:
    Gdiplus::Status m_gdipStatus;
};

// ============================================================================
// Base::DataStructures::IntSet - simple integer set
// ============================================================================
// A set of integers backed by an ATL::CAtlArray used as a flat sorted list.
// Provides Contains/Add/Remove/GetCount operations.
//
class WLXPHOTOBASE_API IntSet
{
public:
    IntSet();
    ~IntSet() throw();

    // Copy semantics
    IntSet(const IntSet& other);
    IntSet& operator=(const IntSet& other);

    // Adds an integer to the set (no-op if already present)
    void Add(int value);

    // Removes an integer from the set (no-op if not present)
    void Remove(int value);

    // Returns true if the set contains the given integer
    bool Contains(int value) const;

    // Returns the number of elements in the set
    size_t GetCount() const throw();

    // Returns true if the set is empty
    bool IsEmpty() const throw();

    // Removes all elements
    void Clear() throw();

    // Returns an element by index (unsorted order)
    int GetAt(size_t index) const;

private:
    ATL::CAtlArray<int> m_values;
};

} // namespace Base

// ============================================================================
// BaseAtlThrow is provided natively by ATL 14+ - no custom definition needed
// ============================================================================

// ============================================================================
// ATL exception macros - map ATL throw macros to Base::Throw
// ============================================================================
// These override the default ATL exception macros so that all ATL-originated
// exceptions route through Base::Exception rather than CAtlException.
//
#ifndef _ATL_NO_EXCEPTIONS
    #define _ATLAtlThrowImpl(hr) Base::Throw(hr)
#endif

#pragma warning(pop)

#endif // BASETYPES_H
