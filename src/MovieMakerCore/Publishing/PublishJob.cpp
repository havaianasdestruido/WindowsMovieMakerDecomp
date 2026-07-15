#include "pch.h"

/*
 * PublishJob.cpp
 *
 * Implementation of PublishJob, PublishBackgroundJob, PublishBackgroundWorker,
 * and PublishProgressCallBack.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "PublishJob.h"
#include <process.h>

// ============================================================================
// PublishProgressCallBack implementation
// ============================================================================

PublishProgressCallBack::PublishProgressCallBack()
{
}

PublishProgressCallBack::~PublishProgressCallBack()
{
}

// ============================================================================
// PublishJob implementation
// ============================================================================

PublishJob::PublishJob()
    : m_dwJobId(0)
    , m_dwProfileIndex(0)
    , m_status(PublishJobStatusPending)
    , m_hrResult(S_OK)
    , m_flOverallProgress(0.0f)
    , m_flEncodeProgress(0.0f)
    , m_flUploadProgress(0.0f)
    , m_bCancelled(false)
    , m_pCallback(nullptr)
{
}

PublishJob::~PublishJob()
{
}

void PublishJob::SetJobId(DWORD dwJobId)
{
    m_dwJobId = dwJobId;
}

DWORD PublishJob::GetJobId() const throw()
{
    return m_dwJobId;
}

void PublishJob::SetJobName(LPCWSTR pszName)
{
    m_strJobName = pszName ? pszName : L"";
}

ATL::CString PublishJob::GetJobName() const
{
    return m_strJobName;
}

void PublishJob::SetInputPath(LPCWSTR pszPath)
{
    m_strInputPath = pszPath ? pszPath : L"";
}

ATL::CString PublishJob::GetInputPath() const
{
    return m_strInputPath;
}

void PublishJob::SetOutputPath(LPCWSTR pszPath)
{
    m_strOutputPath = pszPath ? pszPath : L"";
}

ATL::CString PublishJob::GetOutputPath() const
{
    return m_strOutputPath;
}

void PublishJob::SetServiceName(LPCWSTR pszService)
{
    m_strServiceName = pszService ? pszService : L"";
}

ATL::CString PublishJob::GetServiceName() const
{
    return m_strServiceName;
}

void PublishJob::SetProfileIndex(DWORD dwProfileIndex)
{
    m_dwProfileIndex = dwProfileIndex;
}

DWORD PublishJob::GetProfileIndex() const throw()
{
    return m_dwProfileIndex;
}

HRESULT PublishJob::Start(PublishProgressCallBack* pCallback)
{
    if (m_status != PublishJobStatusPending)
        return E_UNEXPECTED;

    m_pCallback = pCallback;
    m_bCancelled = false;
    m_status = PublishJobStatusEncoding;

    if (m_pCallback)
    {
        m_pCallback->OnStageChanged(1, L"Encoding");
        m_pCallback->OnProgressChanged(0.0f, L"Starting encode...");
    }

    // Encode the movie
    HRESULT hr = Encode();
    if (FAILED(hr))
    {
        m_status = PublishJobStatusFailed;
        m_hrResult = hr;
        m_strResultMessage = L"Encoding failed";

        if (m_pCallback)
            m_pCallback->OnCompleted(hr, L"Encoding failed");

        return hr;
    }

    if (m_bCancelled)
    {
        m_status = PublishJobStatusCancelled;
        return E_ABORT;
    }

    // Upload to service
    Upload();

    return S_OK;
}

HRESULT PublishJob::Cancel()
{
    m_bCancelled = true;
    m_status = PublishJobStatusCancelled;
    m_hrResult = E_ABORT;

    if (m_pCallback)
        m_pCallback->OnCompleted(E_ABORT, L"Publish cancelled");

    return S_OK;
}

HRESULT PublishJob::Pause()
{
    return E_NOTIMPL;
}

HRESULT PublishJob::Resume()
{
    return E_NOTIMPL;
}

PublishJobStatus PublishJob::GetStatus() const throw()
{
    return m_status;
}

bool PublishJob::IsCompleted() const throw()
{
    return m_status == PublishJobStatusCompleted;
}

bool PublishJob::IsFailed() const throw()
{
    return m_status == PublishJobStatusFailed;
}

bool PublishJob::IsCancelled() const throw()
{
    return m_status == PublishJobStatusCancelled;
}

HRESULT PublishJob::GetResult() const throw()
{
    return m_hrResult;
}

ATL::CString PublishJob::GetResultMessage() const
{
    return m_strResultMessage;
}

ATL::CString PublishJob::GetResultUrl() const
{
    return m_strResultUrl;
}

float PublishJob::GetOverallProgress() const throw()
{
    return m_flOverallProgress;
}

float PublishJob::GetEncodeProgress() const throw()
{
    return m_flEncodeProgress;
}

float PublishJob::GetUploadProgress() const throw()
{
    return m_flUploadProgress;
}

HRESULT PublishJob::Encode()
{
    // In the full implementation, this would:
    //  1. Set up a RenderTransport with output profile
    //  2. Execute the encode pipeline
    //  3. Report progress via m_pCallback
    //  4. Check m_bCancelled periodically

    m_flEncodeProgress = 1.0f;
    m_flOverallProgress = 0.5f;
    return S_OK;
}

HRESULT PublishJob::Upload()
{
    if (m_strServiceName.IsEmpty())
    {
        m_status = PublishJobStatusCompleted;
        m_flUploadProgress = 1.0f;
        m_flOverallProgress = 1.0f;
        m_strResultMessage = L"Published successfully";

        if (m_pCallback)
            m_pCallback->OnCompleted(S_OK, L"Published successfully");

        return S_OK;
    }

    m_status = PublishJobStatusUploading;

    // In the full implementation, this would:
    //  1. Get service provider (Facebook, YouTube, etc.)
    //  2. Create upload session using WLID or OAuth
    //  3. Upload the encoded file in chunks
    //  4. Report progress

    m_flUploadProgress = 1.0f;
    m_flOverallProgress = 1.0f;
    m_status = PublishJobStatusCompleted;
    m_strResultMessage = L"Published successfully";

    if (m_pCallback)
        m_pCallback->OnCompleted(S_OK, L"Published successfully");

    return S_OK;
}

// ============================================================================
// PublishBackgroundJob implementation
// ============================================================================
// Static instance pointer for singleton
PublishBackgroundWorker* PublishBackgroundWorker::s_pInstance = nullptr;

PublishBackgroundJob::PublishBackgroundJob(PublishJob* pJob, PublishProgressCallBack* pCallback)
    : m_pJob(pJob)
    , m_pCallback(pCallback)
    , m_pThread(nullptr)
    , m_bRunning(false)
{
}

PublishBackgroundJob::~PublishBackgroundJob()
{
    Cancel();
    delete m_pThread;
    delete m_pJob;
}

HRESULT PublishBackgroundJob::Start()
{
    if (m_bRunning)
        return S_FALSE;

    m_bRunning = true;
    m_pThread = new std::thread(ThreadProc, this);
    return S_OK;
}

HRESULT PublishBackgroundJob::Cancel()
{
    if (m_pJob)
        m_pJob->Cancel();
    return S_OK;
}

bool PublishBackgroundJob::IsRunning() const throw()
{
    return m_bRunning;
}

PublishJob* PublishBackgroundJob::GetJob() const throw()
{
    return m_pJob;
}

DWORD PublishBackgroundJob::GetJobId() const throw()
{
    return m_pJob ? m_pJob->GetJobId() : 0;
}

unsigned int __stdcall PublishBackgroundJob::ThreadProc(void* pParam)
{
    PublishBackgroundJob* pThis = static_cast<PublishBackgroundJob*>(pParam);

    if (pThis->m_pJob)
        pThis->m_pJob->Start(pThis->m_pCallback);

    pThis->m_bRunning = false;
    return 0;
}

// ============================================================================
// PublishBackgroundWorker implementation
// ============================================================================

PublishBackgroundWorker::PublishBackgroundWorker()
    : m_dwMaxJobs(2)
    , m_bInitialized(false)
{
    InitializeCriticalSection(&m_csQueue);
}

PublishBackgroundWorker::~PublishBackgroundWorker()
{
    Shutdown();
    DeleteCriticalSection(&m_csQueue);
}

HRESULT PublishBackgroundWorker::Initialize(DWORD dwMaxJobs)
{
    if (m_bInitialized)
        return S_FALSE;

    m_dwMaxJobs = (dwMaxJobs < 1) ? 1 : dwMaxJobs;
    m_bInitialized = true;
    s_pInstance = this;

    return S_OK;
}

void PublishBackgroundWorker::Shutdown()
{
    if (!m_bInitialized)
        return;

    CancelAllJobs();
    m_bInitialized = false;
    s_pInstance = nullptr;
}

HRESULT PublishBackgroundWorker::EnqueueJob(PublishJob* pJob)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    if (!pJob)
        return E_POINTER;

    // In the full implementation, this would assign a job ID and
    // enqueue the job for processing on the background thread pool.

    PublishBackgroundJob* pBgJob = new PublishBackgroundJob(pJob, nullptr);
    EnterCriticalSection(&m_csQueue);
    m_pendingJobs.push_back(pBgJob);
    LeaveCriticalSection(&m_csQueue);

    return S_OK;
}

HRESULT PublishBackgroundWorker::CancelJob(DWORD dwJobId)
{
    EnterCriticalSection(&m_csQueue);
    for (auto it = m_pendingJobs.begin(); it != m_pendingJobs.end(); ++it)
    {
        if ((*it)->GetJobId() == dwJobId)
        {
            (*it)->Cancel();
            delete *it;
            m_pendingJobs.erase(it);
            LeaveCriticalSection(&m_csQueue);
            return S_OK;
        }
    }
    for (auto it = m_activeJobs.begin(); it != m_activeJobs.end(); ++it)
    {
        if ((*it)->GetJobId() == dwJobId)
        {
            (*it)->Cancel();
            LeaveCriticalSection(&m_csQueue);
            return S_OK;
        }
    }
    LeaveCriticalSection(&m_csQueue);
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT PublishBackgroundWorker::CancelAllJobs()
{
    EnterCriticalSection(&m_csQueue);
    while (!m_pendingJobs.empty())
    {
        delete m_pendingJobs.front();
        m_pendingJobs.pop_front();
    }
    for (auto pJob : m_activeJobs)
        pJob->Cancel();
    m_activeJobs.clear();
    LeaveCriticalSection(&m_csQueue);
    return S_OK;
}

DWORD PublishBackgroundWorker::GetPendingJobCount() const throw()
{
    EnterCriticalSection(&m_csQueue);
    DWORD count = static_cast<DWORD>(m_pendingJobs.size());
    LeaveCriticalSection(&m_csQueue);
    return count;
}

DWORD PublishBackgroundWorker::GetActiveJobCount() const throw()
{
    EnterCriticalSection(&m_csQueue);
    DWORD count = static_cast<DWORD>(m_activeJobs.size());
    LeaveCriticalSection(&m_csQueue);
    return count;
}

DWORD PublishBackgroundWorker::GetCompletedJobCount() const throw()
{
    return 0;
}

float PublishBackgroundWorker::GetOverallProgress() const throw()
{
    return 0.0f;
}

PublishBackgroundWorker* PublishBackgroundWorker::GetInstance()
{
    return s_pInstance;
}
