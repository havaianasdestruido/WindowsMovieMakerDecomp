/*
 * BackgroundOrchestrator.h
 *
 * Thread pool manager for background tasks (transcode, media load, scene merge).
 * Manages queue priority, cancellation, and progress reporting for all
 * asynchronous background operations within MovieMakerCore.
 *
 * RTTI: ?AVBackgroundOrchestrator@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef BACKGROUNDORCHESTRATOR_H
#define BACKGROUNDORCHESTRATOR_H

#include "../../pch.h"
#include "BackgroundRequests.h"

// ============================================================================
// BackgroundOrchestrator
// ============================================================================
// Central thread pool manager for all background processing in MovieMakerCore.
// Owns a configurable pool of worker threads and dispatches background
// requests (transcode, media load, scene merge) with configurable priority
// and cancellation semantics.
//
class BackgroundOrchestrator
{
public:
    BackgroundOrchestrator();
    ~BackgroundOrchestrator();

    // -- Lifetime --
    HRESULT Initialize(DWORD dwMaxThreads = 4);
    void    Shutdown();
    bool    IsInitialized() const throw();

    // -- Queue management --
    HRESULT QueueRequest(BaseBackgroundRequest* pRequest);
    HRESULT CancelRequest(DWORD dwRequestId);
    HRESULT CancelAllRequests();
    HRESULT CancelRequestsByType(RequestType type);

    // -- Priority control --
    HRESULT SetRequestPriority(DWORD dwRequestId, RequestPriority priority);
    RequestPriority GetRequestPriority(DWORD dwRequestId) const throw();

    // -- Progress reporting --
    DWORD GetPendingCount() const throw();
    DWORD GetActiveCount() const throw();
    DWORD GetCompletedCount() const throw();
    float GetOverallProgress() const throw();

    // -- Request enumeration --
    size_t GetRequestCount() const throw();
    BaseBackgroundRequest* GetRequest(DWORD dwRequestId) const;

    // -- Configuration --
    HRESULT SetMaxThreads(DWORD dwMaxThreads);
    DWORD   GetMaxThreads() const throw();

    // -- Singleton access --
    static BackgroundOrchestrator* GetInstance();

private:
    // Internal thread procedure
    static unsigned int __stdcall WorkerThreadProc(void* pParam);
    void WorkerLoop();

    // Queue operations (must hold lock)
    BaseBackgroundRequest* DequeueNextRequest();

    // State
    bool                    m_bInitialized;
    DWORD                   m_dwMaxThreads;

    // Thread pool
    std::vector<std::thread*> m_threads;
    volatile long           m_lThreadCount;

    // Request queue (protected by mutex)
    std::deque<BaseBackgroundRequest*> m_requestQueue;
    mutable CRITICAL_SECTION           m_csQueue;

    // Active requests
    std::map<DWORD, BaseBackgroundRequest*> m_activeRequests;
    mutable CRITICAL_SECTION               m_csActive;

    // Completion tracking
    volatile DWORD          m_dwCompletedCount;
    volatile DWORD          m_dwNextRequestId;

    // Shutdown signal
    bool                    m_bShutdownRequested;

    // Condition variable for worker wake-up
    CONDITION_VARIABLE      m_cvWorkAvailable;

    // Next request ID generation
    CRITICAL_SECTION        m_csIdGen;

    // Prevent copy
    BackgroundOrchestrator(const BackgroundOrchestrator&);
    BackgroundOrchestrator& operator=(const BackgroundOrchestrator&);
};

#endif // BACKGROUNDORCHESTRATOR_H
