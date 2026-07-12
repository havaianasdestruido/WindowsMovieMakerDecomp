/*
 * BackgroundOrchestrator.cpp
 *
 * Implementation of BackgroundOrchestrator thread pool manager.
 * Manages a configurable pool of worker threads that process background
 * requests (transcode, media load, scene merge) with priority queuing.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "BackgroundOrchestrator.h"
#include <process.h>

// ============================================================================
// Global singleton instance
// ============================================================================
static BackgroundOrchestrator* g_pBackgroundOrchestrator = nullptr;

// ============================================================================
// BackgroundOrchestrator implementation
// ============================================================================

BackgroundOrchestrator::BackgroundOrchestrator()
    : m_bInitialized(false)
    , m_dwMaxThreads(4)
    , m_lThreadCount(0)
    , m_dwCompletedCount(0)
    , m_dwNextRequestId(1)
    , m_bShutdownRequested(false)
{
    InitializeCriticalSection(&m_csQueue);
    InitializeCriticalSection(&m_csActive);
    InitializeCriticalSection(&m_csIdGen);
    InitializeConditionVariable(&m_cvWorkAvailable);
}

BackgroundOrchestrator::~BackgroundOrchestrator()
{
    Shutdown();
    DeleteCriticalSection(&m_csQueue);
    DeleteCriticalSection(&m_csActive);
    DeleteCriticalSection(&m_csIdGen);
}

HRESULT BackgroundOrchestrator::Initialize(DWORD dwMaxThreads)
{
    if (m_bInitialized)
        return S_FALSE;

    m_dwMaxThreads = max(1, min(dwMaxThreads, 16));
    m_bShutdownRequested = false;

    // Create worker threads
    for (DWORD i = 0; i < m_dwMaxThreads; i++)
    {
        std::thread* pThread = new std::thread(&BackgroundOrchestrator::WorkerLoop, this);
        m_threads.push_back(pThread);
        InterlockedIncrement(&m_lThreadCount);
    }

    m_bInitialized = true;
    g_pBackgroundOrchestrator = this;

    return S_OK;
}

void BackgroundOrchestrator::Shutdown()
{
    if (!m_bInitialized)
        return;

    m_bShutdownRequested = true;

    // Wake all waiting threads
    WakeAllConditionVariable(&m_cvWorkAvailable);

    // Cancel all pending requests
    EnterCriticalSection(&m_csQueue);
    for (auto pReq : m_requestQueue)
        delete pReq;
    m_requestQueue.clear();
    LeaveCriticalSection(&m_csQueue);

    // Wait for all threads to exit
    for (auto pThread : m_threads)
    {
        if (pThread->joinable())
            pThread->join();
        delete pThread;
    }
    m_threads.clear();
    m_lThreadCount = 0;

    // Release active requests
    EnterCriticalSection(&m_csActive);
    for (auto& pair : m_activeRequests)
        delete pair.second;
    m_activeRequests.clear();
    LeaveCriticalSection(&m_csActive);

    m_bInitialized = false;
    g_pBackgroundOrchestrator = nullptr;
}

bool BackgroundOrchestrator::IsInitialized() const throw()
{
    return m_bInitialized;
}

HRESULT BackgroundOrchestrator::QueueRequest(BaseBackgroundRequest* pRequest)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    if (!pRequest)
        return E_POINTER;

    // Assign a unique request ID
    EnterCriticalSection(&m_csIdGen);
    pRequest->m_dwRequestId = m_dwNextRequestId++;
    LeaveCriticalSection(&m_csIdGen);

    pRequest->m_status = RequestStatusPending;

    // Insert into queue in priority order
    EnterCriticalSection(&m_csQueue);
    bool bInserted = false;
    for (auto it = m_requestQueue.begin(); it != m_requestQueue.end(); ++it)
    {
        if ((*it)->m_priority < pRequest->m_priority)
        {
            m_requestQueue.insert(it, pRequest);
            bInserted = true;
            break;
        }
    }
    if (!bInserted)
        m_requestQueue.push_back(pRequest);

    LeaveCriticalSection(&m_csQueue);

    // Wake a worker thread
    WakeConditionVariable(&m_cvWorkAvailable);

    return S_OK;
}

HRESULT BackgroundOrchestrator::CancelRequest(DWORD dwRequestId)
{
    EnterCriticalSection(&m_csQueue);

    for (auto it = m_requestQueue.begin(); it != m_requestQueue.end(); ++it)
    {
        if ((*it)->m_dwRequestId == dwRequestId)
        {
            (*it)->m_status = RequestStatusCancelled;
            (*it)->m_bCancelled = true;
            delete *it;
            m_requestQueue.erase(it);
            LeaveCriticalSection(&m_csQueue);
            return S_OK;
        }
    }

    LeaveCriticalSection(&m_csQueue);

    // Check active requests
    EnterCriticalSection(&m_csActive);
    auto it = m_activeRequests.find(dwRequestId);
    if (it != m_activeRequests.end())
    {
        it->second->m_bCancelled = true;
        it->second->m_status = RequestStatusCancelled;
        LeaveCriticalSection(&m_csActive);
        return S_OK;
    }
    LeaveCriticalSection(&m_csActive);

    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT BackgroundOrchestrator::CancelAllRequests()
{
    EnterCriticalSection(&m_csQueue);
    for (auto pReq : m_requestQueue)
    {
        pReq->m_bCancelled = true;
        pReq->m_status = RequestStatusCancelled;
        delete pReq;
    }
    m_requestQueue.clear();
    LeaveCriticalSection(&m_csQueue);

    EnterCriticalSection(&m_csActive);
    for (auto& pair : m_activeRequests)
    {
        pair.second->m_bCancelled = true;
        pair.second->m_status = RequestStatusCancelled;
    }
    LeaveCriticalSection(&m_csActive);

    return S_OK;
}

HRESULT BackgroundOrchestrator::CancelRequestsByType(RequestType type)
{
    EnterCriticalSection(&m_csQueue);
    std::deque<BaseBackgroundRequest*> keep;
    while (!m_requestQueue.empty())
    {
        BaseBackgroundRequest* pReq = m_requestQueue.front();
        m_requestQueue.pop_front();
        if (pReq->m_requestType == type)
        {
            pReq->m_bCancelled = true;
            pReq->m_status = RequestStatusCancelled;
            delete pReq;
        }
        else
        {
            keep.push_back(pReq);
        }
    }
    m_requestQueue.swap(keep);
    LeaveCriticalSection(&m_csQueue);

    return S_OK;
}

HRESULT BackgroundOrchestrator::SetRequestPriority(DWORD dwRequestId, RequestPriority priority)
{
    EnterCriticalSection(&m_csQueue);
    for (auto it = m_requestQueue.begin(); it != m_requestQueue.end(); ++it)
    {
        if ((*it)->m_dwRequestId == dwRequestId)
        {
            (*it)->m_priority = priority;
            // Re-sort queue (simple bubble)
            for (auto it2 = it; it2 != m_requestQueue.begin(); --it2)
            {
                auto prev = it2 - 1;
                if ((*prev)->m_priority < (*it2)->m_priority)
                    std::swap(*prev, *it2);
                else
                    break;
            }
            LeaveCriticalSection(&m_csQueue);
            return S_OK;
        }
    }
    LeaveCriticalSection(&m_csQueue);
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

RequestPriority BackgroundOrchestrator::GetRequestPriority(DWORD dwRequestId) const throw()
{
    EnterCriticalSection(&m_csQueue);
    for (auto pReq : m_requestQueue)
    {
        if (pReq->m_dwRequestId == dwRequestId)
        {
            RequestPriority pri = pReq->m_priority;
            LeaveCriticalSection(&m_csQueue);
            return pri;
        }
    }
    LeaveCriticalSection(&m_csQueue);
    return RequestPriorityDefault;
}

DWORD BackgroundOrchestrator::GetPendingCount() const throw()
{
    EnterCriticalSection(&m_csQueue);
    DWORD count = static_cast<DWORD>(m_requestQueue.size());
    LeaveCriticalSection(&m_csQueue);
    return count;
}

DWORD BackgroundOrchestrator::GetActiveCount() const throw()
{
    EnterCriticalSection(&m_csActive);
    DWORD count = static_cast<DWORD>(m_activeRequests.size());
    LeaveCriticalSection(&m_csActive);
    return count;
}

DWORD BackgroundOrchestrator::GetCompletedCount() const throw()
{
    return m_dwCompletedCount;
}

float BackgroundOrchestrator::GetOverallProgress() const throw()
{
    DWORD total = GetPendingCount() + GetActiveCount() + m_dwCompletedCount;
    if (total == 0)
        return 1.0f;
    return static_cast<float>(m_dwCompletedCount) / static_cast<float>(total);
}

size_t BackgroundOrchestrator::GetRequestCount() const throw()
{
    return GetPendingCount() + GetActiveCount();
}

BaseBackgroundRequest* BackgroundOrchestrator::GetRequest(DWORD dwRequestId) const
{
    EnterCriticalSection(&m_csQueue);
    for (auto pReq : m_requestQueue)
    {
        if (pReq->m_dwRequestId == dwRequestId)
        {
            LeaveCriticalSection(&m_csQueue);
            return pReq;
        }
    }
    LeaveCriticalSection(&m_csQueue);

    EnterCriticalSection(&m_csActive);
    auto it = m_activeRequests.find(dwRequestId);
    if (it != m_activeRequests.end())
    {
        LeaveCriticalSection(&m_csActive);
        return it->second;
    }
    LeaveCriticalSection(&m_csActive);

    return nullptr;
}

HRESULT BackgroundOrchestrator::SetMaxThreads(DWORD dwMaxThreads)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    m_dwMaxThreads = max(1, min(dwMaxThreads, 16));

    // If we have fewer threads than desired, spawn more
    while (static_cast<DWORD>(m_threads.size()) < m_dwMaxThreads)
    {
        std::thread* pThread = new std::thread(&BackgroundOrchestrator::WorkerLoop, this);
        m_threads.push_back(pThread);
        InterlockedIncrement(&m_lThreadCount);
    }

    return S_OK;
}

DWORD BackgroundOrchestrator::GetMaxThreads() const throw()
{
    return m_dwMaxThreads;
}

BackgroundOrchestrator* BackgroundOrchestrator::GetInstance()
{
    return g_pBackgroundOrchestrator;
}

// ============================================================================
// Internal thread procedure
// ============================================================================

unsigned int __stdcall BackgroundOrchestrator::WorkerThreadProc(void* pParam)
{
    BackgroundOrchestrator* pThis = static_cast<BackgroundOrchestrator*>(pParam);
    pThis->WorkerLoop();
    return 0;
}

void BackgroundOrchestrator::WorkerLoop()
{
    while (!m_bShutdownRequested)
    {
        BaseBackgroundRequest* pRequest = nullptr;

        EnterCriticalSection(&m_csQueue);
        while (m_requestQueue.empty() && !m_bShutdownRequested)
        {
            SleepConditionVariableCS(&m_cvWorkAvailable, &m_csQueue, INFINITE);
        }

        if (m_bShutdownRequested)
        {
            LeaveCriticalSection(&m_csQueue);
            break;
        }

        pRequest = DequeueNextRequest();
        LeaveCriticalSection(&m_csQueue);

        if (pRequest)
        {
            // Move to active set
            EnterCriticalSection(&m_csActive);
            m_activeRequests[pRequest->m_dwRequestId] = pRequest;
            LeaveCriticalSection(&m_csActive);

            pRequest->m_status = RequestStatusInProgress;

            // Execute the request
            HRESULT hr = pRequest->Execute();

            pRequest->m_status = SUCCEEDED(hr)
                ? RequestStatusCompleted
                : RequestStatusFailed;
            pRequest->m_hrResult = hr;

            InterlockedIncrement(&m_dwCompletedCount);

            // Remove from active set
            EnterCriticalSection(&m_csActive);
            m_activeRequests.erase(pRequest->m_dwRequestId);
            LeaveCriticalSection(&m_csActive);

            // Notify completion
            pRequest->OnCompleted(hr);

            // Auto-delete completed requests (unless persistent)
            if (!pRequest->m_bPersistent)
                delete pRequest;
        }
    }
}

BaseBackgroundRequest* BackgroundOrchestrator::DequeueNextRequest()
{
    if (m_requestQueue.empty())
        return nullptr;

    BaseBackgroundRequest* pRequest = m_requestQueue.front();
    m_requestQueue.pop_front();
    return pRequest;
}
