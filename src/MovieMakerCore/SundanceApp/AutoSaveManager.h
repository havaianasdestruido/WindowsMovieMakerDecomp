/*
 * AutoSaveManager.h
 *
 * Automatic project save manager for the Sundance application framework.
 * Periodically saves the current project to a recovery file and provides
 * crash recovery by detecting incomplete save artifacts at startup.
 *
 * RTTI: ?AVAutoSaveManager@@
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_AUTO_SAVE_MANAGER_H
#define SUNDANCE_AUTO_SAVE_MANAGER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class SundanceAppMain;

// ============================================================================
// AutoSaveManager
// ============================================================================
// Manages periodic auto-saving of the current project. Uses a Win32 timer
// or thread to periodically trigger saves to a well-known recovery
// directory (%LOCALAPPDATA%\Microsoft\Windows Live\Movie Maker\Recovery).
//
// On startup, checks for recovery files left behind by an incomplete
// auto-save (indicating a crash) and offers to restore them.
//
class AutoSaveManager
{
public:
    AutoSaveManager();
    ~AutoSaveManager();

    // -- Lifecycle --
    HRESULT Initialize(SundanceAppMain* pAppMain);
    void    Shutdown();

    // -- Project lifecycle hooks --
    void    OnProjectOpened();
    void    OnProjectClosed();
    void    OnProjectSaved();

    // -- Auto-save control --
    HRESULT EnableAutoSave(bool bEnable);
    bool    IsAutoSaveEnabled() const throw();
    HRESULT SetAutoSaveInterval(DWORD dwIntervalMs);
    DWORD   GetAutoSaveInterval() const throw();

    // -- Manual trigger --
    HRESULT SaveNow();

    // -- Crash recovery --
    bool    HasRecoveryFile() const;
    HRESULT RecoverProject(LPCWSTR pszRecoveryPath);
    HRESULT DeleteRecoveryFile();
    LPCWSTR GetRecoveryFilePath() const throw();

    // -- State --
    bool    IsSaving() const throw();
    DWORD   GetLastSaveTickCount() const throw();

private:
    // Configuration
    static const DWORD kDefaultAutoSaveIntervalMs = 300000; // 5 minutes
    static const DWORD kMinAutoSaveIntervalMs     = 30000;  // 30 seconds minimum

    // State
    bool                m_bEnabled;
    bool                m_bSaving;
    DWORD               m_dwIntervalMs;
    DWORD               m_dwLastSaveTick;
    SundanceAppMain*    m_pAppMain;
    ATL::CString        m_strRecoveryDir;
    ATL::CString        m_strRecoveryFile;
    HANDLE              m_hTimerThread;
    HANDLE              m_hStopEvent;
    CRITICAL_SECTION    m_csLock;

    // Internal
    void    EnsureRecoveryDirectory();
    void    ComputeRecoveryFilePath();
    void    CleanupRecoveryDir();
    static DWORD WINAPI AutoSaveThreadProc(LPVOID pParam);
    void    AutoSaveLoop();

    // Noncopyable
    AutoSaveManager(const AutoSaveManager&);
    AutoSaveManager& operator=(const AutoSaveManager&);
};

#endif // SUNDANCE_AUTO_SAVE_MANAGER_H
