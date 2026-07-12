/*
 * Conductor.h
 *
 * Conductor: Project-level coordinator orchestrating background operations.
 * ThemeOperationLogger: Logs theme operations for diagnostics.
 * MonolithicThemeOperation: Atomic multi-step theme operation.
 * MTOListContainer: Container for ordered theme operations.
 *
 * RTTI: ?AVConductor@@, ?AVThemeOperationLogger@@,
 *       ?AVMonolithicThemeOperation@@, ?AVMTOListContainer@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef CONDUCTOR_H
#define CONDUCTOR_H

#include "../../pch.h"
#include "BackgroundOrchestrator.h"

// ============================================================================
// MonolithicThemeOperation
// ============================================================================
// Represents an atomic multi-step theme operation. Each operation consists
// of multiple steps that must be applied as a single transaction to maintain
// consistency in the theme system state.
//
class MonolithicThemeOperation
{
public:
    MonolithicThemeOperation();
    ~MonolithicThemeOperation();

    // -- Identity --
    void SetOperationId(DWORD dwId);
    DWORD GetOperationId() const throw();

    void SetOperationName(LPCWSTR pszName);
    ATL::CString GetOperationName() const;

    // -- Steps --
    void AddStep(LPCWSTR pszStepDescription);
    size_t GetStepCount() const throw();
    ATL::CString GetStepAt(size_t nIndex) const;

    // -- Execution state --
    void SetInProgress(bool bInProgress);
    bool IsInProgress() const throw();

    void SetCompleted(bool bCompleted);
    bool IsCompleted() const throw();

    void SetFailed(bool bFailed);
    bool IsFailed() const throw();

    void SetErrorDescription(LPCWSTR pszError);
    ATL::CString GetErrorDescription() const;

    // -- Timestamps --
    FILETIME GetStartTime() const throw();
    void SetStartTime(const FILETIME& ft);

    FILETIME GetEndTime() const throw();
    void SetEndTime(const FILETIME& ft);

private:
    DWORD                  m_dwOperationId;
    ATL::CString           m_strOperationName;
    std::vector<ATL::CString> m_vSteps;
    bool                   m_bInProgress;
    bool                   m_bCompleted;
    bool                   m_bFailed;
    ATL::CString           m_strErrorDescription;
    FILETIME               m_ftStartTime;
    FILETIME               m_ftEndTime;
};

// ============================================================================
// MTOListContainer
// ============================================================================
// Container for ordered MonolithicThemeOperation entries. Provides sequential
// access and management of theme operations in execution order.
//
class MTOListContainer
{
public:
    MTOListContainer();
    ~MTOListContainer();

    // -- Management --
    void AddOperation(MonolithicThemeOperation* pOperation);
    void RemoveOperation(DWORD dwOperationId);
    void RemoveAllOperations();
    size_t GetOperationCount() const throw();

    // -- Access --
    MonolithicThemeOperation* GetOperation(DWORD dwOperationId) const;
    MonolithicThemeOperation* GetOperationAt(size_t nIndex) const;

    // -- Query --
    bool HasPendingOperations() const throw();
    bool HasFailedOperations() const throw();

private:
    std::vector<MonolithicThemeOperation*> m_vOperations;
};

// ============================================================================
// ThemeOperationLogger
// ============================================================================
// Logs theme operations for diagnostics and telemetry. Maintains a time-
// ordered log of all MonolithicThemeOperations and provides query/filter
// capabilities for debugging cache invalidation and theme rendering issues.
//
class ThemeOperationLogger
{
public:
    ThemeOperationLogger();
    ~ThemeOperationLogger();

    // -- Logging --
    void LogOperation(MonolithicThemeOperation* pOperation);
    void ClearLog();
    size_t GetLogEntryCount() const throw();

    // -- Query --
    MonolithicThemeOperation* FindOperation(DWORD dwOperationId) const;
    MonolithicThemeOperation* FindOperationByName(LPCWSTR pszName) const;

    // -- Filter --
    void GetOperationsByStatus(bool bCompleted, std::vector<MonolithicThemeOperation*>& vResults) const;
    void GetOperationsSince(const FILETIME& ftSince, std::vector<MonolithicThemeOperation*>& vResults) const;

    // -- Persistence --
    HRESULT SaveToFile(LPCWSTR pszFilePath);
    HRESULT LoadFromFile(LPCWSTR pszFilePath);

private:
    std::vector<MonolithicThemeOperation*> m_vLogEntries;
    mutable CRITICAL_SECTION               m_csLog;
};

// ============================================================================
// Conductor
// ============================================================================
// Project-level coordinator that orchestrates all background operations.
// Manages the relationship between user actions, the BackgroundOrchestrator,
// theme operations, and scene merge contexts. Provides a single entry point
// for issuing background work from UI components.
//
class Conductor
{
public:
    Conductor();
    ~Conductor();

    // -- Lifetime --
    HRESULT Initialize();
    void    Shutdown();
    bool    IsInitialized() const throw();

    // -- Background request management --
    HRESULT QueueTranscode(DWORD dwExtentId, DWORD dwMediaId);
    HRESULT QueueMediaLoad(LPCWSTR pszFilePath);
    HRESULT QueueSceneMerge(SceneMergeContext* pContext);

    // -- Theme operations --
    HRESULT BeginThemeOperation(LPCWSTR pszName, DWORD* pdwOperationId);
    HRESULT CompleteThemeOperation(DWORD dwOperationId);
    HRESULT FailThemeOperation(DWORD dwOperationId, LPCWSTR pszError);

    // -- Autofit --
    HRESULT RunAutofit(DWORD dwPhotoCount, LONGLONG llTotalDurationHns);

    // -- Orchestrator access --
    BackgroundOrchestrator* GetOrchestrator() const throw();
    ThemeOperationLogger*   GetOperationLogger() const throw();

private:
    BackgroundOrchestrator* m_pOrchestrator;
    ThemeOperationLogger*   m_pOperationLogger;
    MTOListContainer*       m_pMTOContainer;
    bool                    m_bInitialized;
};

#endif // CONDUCTOR_H
