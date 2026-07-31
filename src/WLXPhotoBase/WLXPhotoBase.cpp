/*
 * WLXPhotoBase.cpp
 *
 * Implementation of all 56 exports from WLXPhotoBase.dll.
 * Provides base exception handling, GDI+ status mapping, OS version detection,
 * memory management, and CPU info - the shared foundation for all WLX components.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXPhotoBase.h"

#include <process.h>
#include <strsafe.h>
#include <psapi.h>
#include <tchar.h>

// Disable ATL exception interception - we use our own SEH model
#undef _ATL_CATCH_ALL

// Module handle for resource loading
static HINSTANCE g_hModule = NULL;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    (lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        break;

    case DLL_PROCESS_DETACH:
        g_hModule = NULL;
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

// ============================================================================
// Internal helpers
// ============================================================================
namespace
{

// Maps common Win32 errors to HRESULT facility codes
HRESULT Win32ErrorToHresult(DWORD dwError)
{
    // Map well-known Win32 errors to their HRESULT equivalents
    switch (dwError)
    {
    case ERROR_SUCCESS:
        return S_OK;

    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        return HRESULT_FROM_WIN32(dwError);

    case ERROR_ACCESS_DENIED:
        return E_ACCESSDENIED;

    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
        return E_OUTOFMEMORY;

    case ERROR_INVALID_PARAMETER:
        return E_INVALIDARG;

    case ERROR_NOT_SUPPORTED:
        return E_NOTIMPL;

    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:
        return HRESULT_FROM_WIN32(dwError);

    case ERROR_INSUFFICIENT_BUFFER:
        return HRESULT_FROM_WIN32(dwError);

    case ERROR_MORE_DATA:
        return HRESULT_FROM_WIN32(dwError);

    default:
        return HRESULT_FROM_WIN32(dwError);
    }
}

// Cached processor count
static int g_cProcessorCount = 0;

// OS version detection result cache
// Bits: bit 0 = Win7+, bit 1 = Win8+
static BYTE g_osVersionCache = 0;
static bool g_osVersionCached = false;

bool VerifyVersion(uint32_t major, uint32_t minor, uint32_t spMajor, uint32_t spMinor)
{
    OSVERSIONINFOEXW osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = major;
    osvi.dwMinorVersion = minor;
    osvi.wServicePackMajor = static_cast<WORD>(spMajor);
    osvi.wServicePackMinor = static_cast<WORD>(spMinor);

    DWORDLONG dwlConditionMask = 0;

    // Verify version >= major.minor
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMINOR, VER_GREATER_EQUAL);

    return ::VerifyVersionInfoW(&osvi,
                                VER_MAJORVERSION | VER_MINORVERSION |
                                VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
                                dwlConditionMask) != FALSE;
}

} // anonymous namespace

// ============================================================================
// Base::Exception implementation
// ============================================================================
namespace Base
{

Exception::Exception(HRESULT hr)
    : m_hr(hr)
{
}

Exception::Exception(const Exception& other)
    : m_hr(other.m_hr)
{
}

Exception::~Exception()
{
}

Exception::operator HRESULT() const
{
    return m_hr;
}

HRESULT Exception::GetHResult() const throw()
{
    return m_hr;
}

void Exception::SetHResult(HRESULT hr) throw()
{
    m_hr = hr;
}

void Exception::Throw() const
{
    Base::Throw(m_hr);
}

Exception& Exception::operator=(const Exception& other)
{
    if (this != &other)
    {
        m_hr = other.m_hr;
    }
    return *this;
}

void* Exception::operator new(size_t size)
{
    (size);
    // Exception objects are allocated from a fixed process heap
    // In the original, these are thrown via _CxxThrowException with
    // a custom allocator that ensures exception unwinding works correctly.
    void* p = ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    if (!p)
    {
        // If we can't allocate memory for the exception object,
        // fail fast - there's nothing we can do
        ::RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION,
                         EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
    return p;
}

void Exception::operator delete(void* p) throw()
{
    if (p)
        ::HeapFree(::GetProcessHeap(), 0, p);
}

// ============================================================================
// Base::Throw - throws a structured exception with HRESULT
// ============================================================================
WLXPHOTOBASE_API void Throw(HRESULT hr)
{
    // Directly raise SEH with HRESULT; no heap allocation, avoids leak.
    ::RaiseException(static_cast<DWORD>(hr), EXCEPTION_NONCONTINUABLE, 0, NULL);
}

// ============================================================================
// Base::ThrowLastError
// ============================================================================
WLXPHOTOBASE_API void ThrowLastError()
{
    DWORD dwError = ::GetLastError();
    HRESULT hr = Win32ErrorToHresult(dwError);
    Throw(hr);
}

// ============================================================================
// Base::GdiplusStatusToHresult
// ============================================================================
WLXPHOTOBASE_API HRESULT GdiplusStatusToHresult(Gdiplus::Status status)
{
    switch (status)
    {
    case Gdiplus::Ok:
        return S_OK;

    case Gdiplus::GenericError:
        return E_FAIL;

    case Gdiplus::InvalidParameter:
        return E_INVALIDARG;

    case Gdiplus::OutOfMemory:
        return E_OUTOFMEMORY;

    case Gdiplus::ObjectBusy:
        return HRESULT_FROM_WIN32(ERROR_BUSY);

    case Gdiplus::InsufficientBuffer:
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

    case Gdiplus::NotImplemented:
        return E_NOTIMPL;

    case Gdiplus::PropertyNotFound:
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

    case Gdiplus::PropertyNotSupported:
        return E_NOTIMPL;

    case Gdiplus::FontFamilyNotFound:
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

    case Gdiplus::FontStyleNotFound:
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

    case Gdiplus::NotTrueTypeFont:
        return E_UNEXPECTED;

    case Gdiplus::UnsupportedGdiplusVersion:
        return E_NOTIMPL;

    case Gdiplus::GdiplusNotInitialized:
        return E_UNEXPECTED;

    case Gdiplus::WrongState:
        return E_UNEXPECTED;

    case Gdiplus::Aborted:
        return E_ABORT;

    case Gdiplus::FileNotFound:
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    case Gdiplus::ValueOverflow:
        return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

    case Gdiplus::AccessDenied:
        return E_ACCESSDENIED;

    case Gdiplus::UnknownImageFormat:
        return E_UNEXPECTED;

    default:
        return E_FAIL;
    }
}

// ============================================================================
// Base::String::GetBaseStringManager
// ============================================================================
WLXPHOTOBASE_API ATL::IAtlStringMgr* String::GetBaseStringManager()
{
    static ATL::IAtlStringMgr* spMgr = nullptr;
    if (!spMgr)
    {
        spMgr = new ATL::CAtlStringMgr;
    }
    return spMgr;
}

// ============================================================================
// Base::OS version detection
// ============================================================================
WLXPHOTOBASE_API bool OS::IsWin7OrGreater()
{
    if (!g_osVersionCached)
    {
        g_osVersionCache = 0;
        if (VerifyVersion(6, 1, 0, 0))
            g_osVersionCache |= 0x01;
        if (VerifyVersion(6, 2, 0, 0))
            g_osVersionCache |= 0x02;
        g_osVersionCached = true;
    }
    return (g_osVersionCache & 0x01) != 0;
}

WLXPHOTOBASE_API bool OS::IsWin8OrGreater()
{
    if (!g_osVersionCached)
    {
        g_osVersionCache = 0;
        if (VerifyVersion(6, 1, 0, 0))
            g_osVersionCache |= 0x01;
        if (VerifyVersion(6, 2, 0, 0))
            g_osVersionCache |= 0x02;
        g_osVersionCached = true;
    }
    return (g_osVersionCache & 0x02) != 0;
}

// ============================================================================
// Base::Private memory management
// ============================================================================
WLXPHOTOBASE_API void* Private::New(size_t size, bool zeroInit)
{
    if (size == 0)
        size = 1; // HeapAlloc with size 0 is implementation-defined

    DWORD dwFlags = HEAP_GENERATE_EXCEPTIONS;
    if (zeroInit)
        dwFlags |= HEAP_ZERO_MEMORY;

    void* p = ::HeapAlloc(::GetProcessHeap(), dwFlags, size);
    return p;
}

WLXPHOTOBASE_API void Private::Delete(void* p)
{
    if (p)
    {
        ::HeapFree(::GetProcessHeap(), 0, p);
    }
}

// ============================================================================
// Base::CPU information
// ============================================================================
WLXPHOTOBASE_API int CPU::GetProcessorCount()
{
    if (g_cProcessorCount == 0)
    {
        SYSTEM_INFO si = { 0 };
        ::GetSystemInfo(&si);
        g_cProcessorCount = static_cast<int>(si.dwNumberOfProcessors);
    }
    return g_cProcessorCount;
}

} // namespace Base

// BaseAtlThrow is provided natively by ATL 14+ - no custom definition needed

// ============================================================================
// Base::File implementation
// ============================================================================
namespace Base
{

File::File() throw()
    : m_hFile(INVALID_HANDLE_VALUE)
{
}

File::~File() throw()
{
    Close();
}

bool File::Open(LPCWSTR pszPath, DWORD dwAccess, DWORD dwShareMode, DWORD dwCreation)
{
    if (!pszPath)
        return false;

    Close();

    m_hFile = ::CreateFileW(
        pszPath,
        dwAccess,
        dwShareMode,
        NULL,
        dwCreation,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        m_strPath = pszPath;
        return true;
    }

    return false;
}

void File::Close() throw()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
    m_strPath.Empty();
}

bool File::IsValid() const throw()
{
    return m_hFile != INVALID_HANDLE_VALUE;
}

ATL::CString File::GetFileName() const
{
    if (m_strPath.IsEmpty())
        return ATL::CString();

    LPCWSTR pszSlash = wcsrchr(m_strPath, L'\\');
    if (!pszSlash)
        pszSlash = wcsrchr(m_strPath, L'/');

    if (pszSlash)
        return ATL::CString(pszSlash + 1);

    return m_strPath;
}

ATL::CString File::GetPath() const
{
    return m_strPath;
}

ATL::CString File::GetFullPath() const
{
    if (m_strPath.IsEmpty())
        return ATL::CString();

    // Resolve to full path
    WCHAR szFull[MAX_PATH] = { 0 };
    DWORD dwLen = ::GetFullPathNameW(m_strPath, MAX_PATH, szFull, NULL);
    if (dwLen > 0 && dwLen < MAX_PATH)
        return ATL::CString(szFull);

    return m_strPath;
}

ULONGLONG File::GetFileSize() const
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return 0;

    ULARGE_INTEGER uli = { 0 };
    uli.LowPart = ::GetFileSize(m_hFile, &uli.HighPart);
    if (uli.LowPart == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR)
        return 0;

    return uli.QuadPart;
}

HANDLE File::GetHandle() const throw()
{
    return m_hFile;
}

DWORD File::Read(LPVOID pBuffer, DWORD cbToRead)
{
    if (m_hFile == INVALID_HANDLE_VALUE || !pBuffer)
        return 0;

    DWORD dwRead = 0;
    if (::ReadFile(m_hFile, pBuffer, cbToRead, &dwRead, NULL))
        return dwRead;

    return 0;
}

DWORD File::Write(LPCVOID pBuffer, DWORD cbToWrite)
{
    if (m_hFile == INVALID_HANDLE_VALUE || !pBuffer)
        return 0;

    DWORD dwWritten = 0;
    if (::WriteFile(m_hFile, pBuffer, cbToWrite, &dwWritten, NULL))
        return dwWritten;

    return 0;
}

ULONGLONG File::Seek(LONGLONG llOffset, DWORD dwMethod)
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return INVALID_SET_FILE_POINTER;

    LARGE_INTEGER liOffset = { 0 };
    liOffset.QuadPart = llOffset;

    LARGE_INTEGER liNewPos = { 0 };
    if (::SetFilePointerEx(m_hFile, liOffset, &liNewPos, dwMethod))
        return liNewPos.QuadPart;

    return INVALID_SET_FILE_POINTER;
}

// ============================================================================
// Base::TempFile implementation
// ============================================================================

TempFile::TempFile() throw()
{
}

TempFile::~TempFile() throw()
{
    Close();
}

bool TempFile::Create(LPCWSTR pszDir)
{
    WCHAR szTempPath[MAX_PATH] = { 0 };
    WCHAR szTempFile[MAX_PATH] = { 0 };

    // Get temp directory
    if (pszDir && pszDir[0])
    {
        if (::GetTempFileNameW(pszDir, L"WLX", 0, szTempFile) == 0)
            return false;
    }
    else
    {
        if (::GetTempPathW(MAX_PATH, szTempPath) == 0)
            return false;

        if (::GetTempFileNameW(szTempPath, L"WLX", 0, szTempFile) == 0)
            return false;
    }

    m_strPath = szTempFile;

    // Open the file for read/write
    if (!m_file.Open(szTempFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE, OPEN_EXISTING))
    {
        m_strPath.Empty();
        return false;
    }

    return true;
}

void TempFile::Close()
{
    m_file.Close();

    // Delete the temporary file
    if (!m_strPath.IsEmpty())
    {
        ::DeleteFileW(m_strPath);
        m_strPath.Empty();
    }
}

ATL::CString TempFile::GetPath() const
{
    return m_strPath;
}

const File& TempFile::GetFile() const throw()
{
    return m_file;
}

File& TempFile::GetFile() throw()
{
    return m_file;
}

bool TempFile::IsValid() const throw()
{
    return m_file.IsValid();
}

// ============================================================================
// Base::Thread implementation
// ============================================================================

Thread::Thread() throw()
    : m_hThread(NULL)
    , m_dwThreadId(0)
    , m_fRunning(false)
    , m_fStopRequested(false)
{
}

Thread::~Thread() throw()
{
    if (m_hThread)
    {
        if (m_fRunning)
        {
            // Request stop and wait briefly
            m_fStopRequested = true;
            ::WaitForSingleObject(m_hThread, 5000);
        }
        ::CloseHandle(m_hThread);
        m_hThread = NULL;
    }
}

HRESULT Thread::Start()
{
    if (m_hThread)
    {
        if (m_fRunning)
            return E_UNEXPECTED;
        ::CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    m_fStopRequested = false;
    m_fRunning = false;

    m_hThread = ::CreateThread(
        NULL,
        0,
        ThreadProcThunk,
        this,
        0,
        &m_dwThreadId
    );

    if (!m_hThread)
    {
        m_dwThreadId = 0;
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    return S_OK;
}

void Thread::Stop(DWORD dwTimeoutMs)
{
    if (!m_hThread || !m_fRunning)
        return;

    m_fStopRequested = true;

    if (::WaitForSingleObject(m_hThread, dwTimeoutMs) == WAIT_TIMEOUT)
    {
        // Thread did not exit in time - force termination
        ::TerminateThread(m_hThread, 1);
    }
}

HRESULT Thread::Wait(DWORD dwTimeoutMs)
{
    if (!m_hThread)
        return E_UNEXPECTED;

    DWORD dwResult = ::WaitForSingleObject(m_hThread, dwTimeoutMs);
    switch (dwResult)
    {
    case WAIT_OBJECT_0:
        return S_OK;
    case WAIT_TIMEOUT:
        return S_FALSE;
    default:
        return HRESULT_FROM_WIN32(::GetLastError());
    }
}

bool Thread::IsRunning() const throw()
{
    return m_fRunning;
}

HANDLE Thread::GetHandle() const throw()
{
    return m_hThread;
}

DWORD Thread::GetThreadId() const throw()
{
    return m_dwThreadId;
}

bool Thread::ShouldStop() const throw()
{
    return m_fStopRequested;
}

DWORD WINAPI Thread::ThreadProcThunk(LPVOID pParam)
{
    Thread* pThis = static_cast<Thread*>(pParam);
    pThis->m_fRunning = true;

    DWORD dwResult = 0;
    __try
    {
        dwResult = pThis->ThreadProc();
    }
    __finally
    {
        pThis->m_fRunning = false;
    }

    return dwResult;
}

// ============================================================================
// Base::FindFile implementation
// ============================================================================

FindFile::FindFile() throw()
    : m_hFind(INVALID_HANDLE_VALUE)
    , m_fFirst(false)
{
    ZeroMemory(&m_findData, sizeof(m_findData));
}

FindFile::~FindFile() throw()
{
    Close();
}

bool FindFile::FindFirstFile(LPCWSTR pszPattern)
{
    Close();

    if (!pszPattern || !pszPattern[0])
        return false;

    m_hFind = ::FindFirstFileW(pszPattern, &m_findData);
    if (m_hFind == INVALID_HANDLE_VALUE)
        return false;

    m_fFirst = true;

    // Extract directory from pattern
    LPCWSTR pszLastSlash = wcsrchr(pszPattern, L'\\');
    if (!pszLastSlash)
        pszLastSlash = wcsrchr(pszPattern, L'/');
    if (pszLastSlash)
    {
        int cchDir = static_cast<int>(pszLastSlash - pszPattern);
        LPTSTR pszDir = m_strDirectory.GetBuffer(cchDir + 1);
        CopyMemory(pszDir, pszPattern, cchDir * sizeof(WCHAR));
        pszDir[cchDir] = L'\0';
        m_strDirectory.ReleaseBuffer(cchDir);
    }

    return true;
}

bool FindFile::FindNextFile()
{
    if (m_hFind == INVALID_HANDLE_VALUE)
        return false;

    if (!::FindNextFileW(m_hFind, &m_findData))
        return false;

    m_fFirst = false;
    return true;
}

void FindFile::Close() throw()
{
    if (m_hFind != INVALID_HANDLE_VALUE)
    {
        ::FindClose(m_hFind);
        m_hFind = INVALID_HANDLE_VALUE;
    }
    m_strDirectory.Empty();
    ZeroMemory(&m_findData, sizeof(m_findData));
    m_fFirst = false;
}

ATL::CString FindFile::GetFileName() const
{
    return ATL::CString(m_findData.cFileName);
}

ATL::CString FindFile::GetFilePath() const
{
    if (m_strDirectory.IsEmpty())
        return ATL::CString(m_findData.cFileName);

    ATL::CString strPath = m_strDirectory;
    if (strPath[strPath.GetLength() - 1] != L'\\')
        strPath += L'\\';
    strPath += m_findData.cFileName;
    return strPath;
}

ATL::CString FindFile::GetDirectory() const
{
    return m_strDirectory;
}

bool FindFile::IsDirectory() const throw()
{
    return (m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool FindFile::IsFile() const throw()
{
    return !IsDirectory();
}

ULONGLONG FindFile::GetFileSize() const throw()
{
    ULARGE_INTEGER uli;
    uli.LowPart = m_findData.nFileSizeLow;
    uli.HighPart = m_findData.nFileSizeHigh;
    return uli.QuadPart;
}

const WIN32_FIND_DATAW& FindFile::GetFindData() const throw()
{
    return m_findData;
}

// ============================================================================
// Base::GdiException implementation
// ============================================================================

GdiException::GdiException(Gdiplus::Status status)
    : Exception(GdiplusStatusToHresult(status))
    , m_gdipStatus(status)
{
}

GdiException::~GdiException() throw()
{
}

Gdiplus::Status GdiException::GetGdiplusStatus() const throw()
{
    return m_gdipStatus;
}

void GdiException::Throw() const
{
    Base::Throw(GetHResult());
}

// ============================================================================
// Base::DataStructures::IntSet implementation
// ============================================================================

IntSet::IntSet()
{
}

IntSet::~IntSet() throw()
{
}

IntSet::IntSet(const IntSet& other)
{
    m_values.Copy(other.m_values);
}

IntSet& IntSet::operator=(const IntSet& other)
{
    if (this != &other)
    {
        m_values.RemoveAll();
        m_values.Copy(other.m_values);
    }
    return *this;
}

void IntSet::Add(int value)
{
    // Check for duplicates
    for (size_t i = 0; i < m_values.GetCount(); ++i)
    {
        if (m_values[i] == value)
            return; // Already present
    }
    m_values.Add(value);
}

void IntSet::Remove(int value)
{
    for (size_t i = 0; i < m_values.GetCount(); ++i)
    {
        if (m_values[i] == value)
        {
            m_values.RemoveAt(i);
            return;
        }
    }
}

bool IntSet::Contains(int value) const
{
    for (size_t i = 0; i < m_values.GetCount(); ++i)
    {
        if (m_values[i] == value)
            return true;
    }
    return false;
}

size_t IntSet::GetCount() const throw()
{
    return m_values.GetCount();
}

bool IntSet::IsEmpty() const throw()
{
    return m_values.IsEmpty();
}

void IntSet::Clear() throw()
{
    m_values.RemoveAll();
}

int IntSet::GetAt(size_t index) const
{
    ATLASSERT(index < m_values.GetCount());
    return m_values.GetAt(index);
}

} // namespace Base

// ============================================================================
// WLXPhotoBase_Init -- DLL initialization entry point called by MovieMaker.exe
// ============================================================================
extern "C" WLXPHOTOBASE_API void __stdcall WLXPhotoBase_Init(void)
{
}
