/*
 * AutoSaveManager.cpp
 *
 * Implementation of the automatic project save manager for Sundance.
 * Provides periodic background saves and crash recovery detection.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "AutoSaveManager.h"
#include "SundanceAppMain.h"

// ============================================================================
// Construction / destruction
// ============================================================================
AutoSaveManager::AutoSaveManager()
    : m_bEnabled(false)
    , m_bSaving(false)
    , m_dwIntervalMs(kDefaultAutoSaveIntervalMs)
    , m_dwLastSaveTick(0)
    , m_pAppMain(NULL)
    , m_hTimerThread(NULL)
    , m_hStopEvent(NULL)
{
    InitializeCriticalSection(&m_csLock);
}

AutoSaveManager::~AutoSaveManager()
{
    Shutdown();
    DeleteCriticalSection(&m_csLock);
}

// ============================================================================
// Initialize
// ============================================================================
HRESULT AutoSaveManager::Initialize(SundanceAppMain* pAppMain)
{
    if (!pAppMain)
        return E_INVALIDARG;

    m_pAppMain = pAppMain;

    // Compute recovery directory path
    WCHAR szLocalAppData[MAX_PATH] = { 0 };
    HRESULT hr = SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szLocalAppData);
    if (FAILED(hr))
        return hr;

    m_strRecoveryDir.Format(L"%s\\Microsoft\\Windows Live\\Movie Maker\\Recovery", szLocalAppData);
    EnsureRecoveryDirectory();

    // Create stop event for thread shutdown
    m_hStopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!m_hStopEvent)
        return HRESULT_FROM_WIN32(::GetLastError());

    ComputeRecoveryFilePath();

    // Enable auto-save by default
    m_bEnabled = true;

    return S_OK;
}

// ============================================================================
// Shutdown
// ============================================================================
void AutoSaveManager::Shutdown()
{
    // Signal stop and wait for thread
    if (m_hStopEvent)
    {
        ::SetEvent(m_hStopEvent);
    }

    if (m_hTimerThread)
    {
        ::WaitForSingleObject(m_hTimerThread, 5000);
        ::CloseHandle(m_hTimerThread);
        m_hTimerThread = NULL;
    }

    if (m_hStopEvent)
    {
        ::CloseHandle(m_hStopEvent);
        m_hStopEvent = NULL;
    }

    m_pAppMain = NULL;
    m_bEnabled = false;
}

// ============================================================================
// OnProjectOpened
// ============================================================================
void AutoSaveManager::OnProjectOpened()
{
    // Check for recovery file from a previous crash
    // (HasRecoveryFile / RecoverProject are called externally if needed)

    // Start the auto-save background thread if enabled
    if (m_bEnabled && !m_hTimerThread)
    {
        m_hTimerThread = ::CreateThread(
            NULL, 0, AutoSaveThreadProc, this, 0, NULL);
    }

    m_dwLastSaveTick = ::GetTickCount();
}

// ============================================================================
// OnProjectClosed
// ============================================================================
void AutoSaveManager::OnProjectClosed()
{
    // Stop the background thread
    if (m_hStopEvent)
        ::SetEvent(m_hStopEvent);

    if (m_hTimerThread)
    {
        ::WaitForSingleObject(m_hTimerThread, 5000);
        ::CloseHandle(m_hTimerThread);
        m_hTimerThread = NULL;
    }

    // Clean up recovery file
    DeleteRecoveryFile();

    // Reset stop event for next use
    if (m_hStopEvent)
        ::ResetEvent(m_hStopEvent);
}

// ============================================================================
// OnProjectSaved
// ============================================================================
void AutoSaveManager::OnProjectSaved()
{
    // A real save clears the need for auto-save recovery
    DeleteRecoveryFile();
    m_dwLastSaveTick = ::GetTickCount();
}

// ============================================================================
// EnableAutoSave / IsAutoSaveEnabled
// ============================================================================
HRESULT AutoSaveManager::EnableAutoSave(bool bEnable)
{
    m_bEnabled = bEnable;
    return S_OK;
}

bool AutoSaveManager::IsAutoSaveEnabled() const throw()
{
    return m_bEnabled;
}

// ============================================================================
// SetAutoSaveInterval / GetAutoSaveInterval
// ============================================================================
HRESULT AutoSaveManager::SetAutoSaveInterval(DWORD dwIntervalMs)
{
    if (dwIntervalMs < kMinAutoSaveIntervalMs)
        return E_INVALIDARG;

    m_dwIntervalMs = dwIntervalMs;
    return S_OK;
}

DWORD AutoSaveManager::GetAutoSaveInterval() const throw()
{
    return m_dwIntervalMs;
}

// ============================================================================
// SaveNow
// ============================================================================
HRESULT AutoSaveManager::SaveNow()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    EnterCriticalSection(&m_csLock);

    m_bSaving = true;
    HRESULT hr = E_FAIL;

    // Save to the recovery file path
    StoryboardManagerNamespace::MovieProject* pProject = m_pAppMain->GetProject();
    if (pProject && !m_strRecoveryFile.IsEmpty())
    {
        hr = pProject->SaveAs(m_strRecoveryFile);
    }

    m_bSaving = false;
    m_dwLastSaveTick = ::GetTickCount();

    LeaveCriticalSection(&m_csLock);

    return hr;
}

// ============================================================================
// HasRecoveryFile
// ============================================================================
bool AutoSaveManager::HasRecoveryFile() const
{
    if (m_strRecoveryFile.IsEmpty())
        return false;

    DWORD dwAttr = ::GetFileAttributesW(m_strRecoveryFile);
    return (dwAttr != INVALID_FILE_ATTRIBUTES &&
            !(dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}

// ============================================================================
// RecoverProject
// ============================================================================
HRESULT AutoSaveManager::RecoverProject(LPCWSTR pszRecoveryPath)
{
    if (!pszRecoveryPath || !pszRecoveryPath[0])
        return E_INVALIDARG;

    if (!m_pAppMain)
        return E_UNEXPECTED;

    return m_pAppMain->OpenProject(pszRecoveryPath);
}

// ============================================================================
// DeleteRecoveryFile
// ============================================================================
HRESULT AutoSaveManager::DeleteRecoveryFile()
{
    if (m_strRecoveryFile.IsEmpty())
        return S_FALSE;

    if (::DeleteFileW(m_strRecoveryFile))
        return S_OK;

    DWORD dwErr = ::GetLastError();
    if (dwErr == ERROR_FILE_NOT_FOUND)
        return S_FALSE;

    return HRESULT_FROM_WIN32(dwErr);
}

// ============================================================================
// GetRecoveryFilePath
// ============================================================================
LPCWSTR AutoSaveManager::GetRecoveryFilePath() const throw()
{
    return m_strRecoveryFile;
}

// ============================================================================
// IsSaving / GetLastSaveTickCount
// ============================================================================
bool AutoSaveManager::IsSaving() const throw()
{
    return m_bSaving;
}

DWORD AutoSaveManager::GetLastSaveTickCount() const throw()
{
    return m_dwLastSaveTick;
}

// ============================================================================
// EnsureRecoveryDirectory
// ============================================================================
void AutoSaveManager::EnsureRecoveryDirectory()
{
    if (m_strRecoveryDir.IsEmpty())
        return;

    ::CreateDirectoryW(m_strRecoveryDir, NULL);
    // Ignore failure - directory may already exist
}

// ============================================================================
// ComputeRecoveryFilePath
// ============================================================================
void AutoSaveManager::ComputeRecoveryFilePath()
{
    if (m_strRecoveryDir.IsEmpty())
        return;

    // Generate a unique recovery file name based on timestamp
    SYSTEMTIME st = { 0 };
    ::GetLocalTime(&st);

    m_strRecoveryFile.Format(
        L"%s\\recover_%04d%02d%02d_%02d%02d%02d.wlmp",
        m_strRecoveryDir.GetString(),
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
}

// ============================================================================
// CleanupRecoveryDir
// ============================================================================
void AutoSaveManager::CleanupRecoveryDir()
{
    if (m_strRecoveryDir.IsEmpty())
        return;

    ATL::CString strPattern = m_strRecoveryDir + L"\\recover_*.wlmp";

    WIN32_FIND_DATAW findData = { 0 };
    HANDLE hFind = ::FindFirstFileW(strPattern, &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            ATL::CString strFile = m_strRecoveryDir + L"\\" + findData.cFileName;
            ::DeleteFileW(strFile);
        }
    } while (::FindNextFileW(hFind, &findData));

    ::FindClose(hFind);
}

// ============================================================================
// AutoSaveThreadProc (static)
// ============================================================================
DWORD WINAPI AutoSaveManager::AutoSaveThreadProc(LPVOID pParam)
{
    AutoSaveManager* pThis = static_cast<AutoSaveManager*>(pParam);
    pThis->AutoSaveLoop();
    return 0;
}

// ============================================================================
// AutoSaveLoop
// ============================================================================
void AutoSaveManager::AutoSaveLoop()
{
    while (true)
    {
        DWORD dwWait = ::WaitForSingleObject(m_hStopEvent, m_dwIntervalMs);

        if (dwWait == WAIT_OBJECT_0)
        {
            // Stop event signaled
            break;
        }

        if (dwWait == WAIT_TIMEOUT && m_bEnabled && !m_bSaving)
        {
            SaveNow();
        }
    }
}
