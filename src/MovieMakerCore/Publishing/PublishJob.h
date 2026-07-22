/*
 * PublishJob.h
 *
 * Publish job classes for encoding and uploading movies to online services.
 *
 * PublishJob: Individual publish job (encode + upload).
 * PublishBackgroundWorker: Background thread for publish operations.
 * PublishBackgroundJob: Background publish task.
 * PublishProgressCallBack: Progress callback interface.
 *
 * RTTI: ?AVPublishJob@@, ?AVPublishBackgroundWorker@@,
 *       ?AVPublishBackgroundJob@@, ?AVPublishProgressCallBack@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef PUBLISH_JOB_H
#define PUBLISH_JOB_H

#include "../pch.h"

// ============================================================================
// PublishJobStatus enum
// ============================================================================
enum PublishJobStatus
{
    PublishJobStatusPending      = 0,
    PublishJobStatusEncoding     = 1,
    PublishJobStatusUploading    = 2,
    PublishJobStatusFinalizing   = 3,
    PublishJobStatusCompleted    = 4,
    PublishJobStatusFailed       = 5,
    PublishJobStatusCancelled    = 6
};

// ============================================================================
// PublishProgressCallBack
// ============================================================================
// Callback interface for progress reporting during publish operations.
// Implemented by the UI layer to display progress bars and status text.
//
class PublishProgressCallBack
{
public:
    PublishProgressCallBack();
    virtual ~PublishProgressCallBack();

    virtual void OnProgressChanged(float flOverallProgress, LPCWSTR pszStatusText) = 0;
    virtual void OnStageChanged(DWORD dwStage, LPCWSTR pszStageName) = 0;
    virtual void OnCompleted(HRESULT hrResult, LPCWSTR pszMessage) = 0;
    virtual bool CheckCancelled() = 0;
};

// ============================================================================
// PublishJob
// ============================================================================
// Represents a single publish job (encode + upload to a service).
// Manages the state machine from pending through encoding, uploading,
// completion, and error handling. Supports cancellation and progress
// reporting via PublishProgressCallBack.
//
class PublishJob
{
public:
    PublishJob();
    ~PublishJob();

    // -- Identity --
    void SetJobId(DWORD dwJobId);
    DWORD GetJobId() const throw();

    void SetJobName(LPCWSTR pszName);
    ATL::CString GetJobName() const;

    // -- Configuration --
    void SetInputPath(LPCWSTR pszPath);
    ATL::CString GetInputPath() const;

    void SetOutputPath(LPCWSTR pszPath);
    ATL::CString GetOutputPath() const;

    void SetServiceName(LPCWSTR pszService);
    ATL::CString GetServiceName() const;

    void SetProfileIndex(DWORD dwProfileIndex);
    DWORD GetProfileIndex() const throw();

    // -- Encoding parameters --
    void SetWidth(DWORD dwWidth);
    DWORD GetWidth() const throw();

    void SetHeight(DWORD dwHeight);
    DWORD GetHeight() const throw();

    void SetQuality(DWORD dwQuality);
    DWORD GetQuality() const throw();

    // -- Execution --
    HRESULT Start(PublishProgressCallBack* pCallback);
    HRESULT Cancel();
    HRESULT Pause();
    HRESULT Resume();

    // -- Status --
    PublishJobStatus GetStatus() const throw();
    bool IsCompleted() const throw();
    bool IsFailed() const throw();
    bool IsCancelled() const throw();

    // -- Results --
    HRESULT GetResult() const throw();
    ATL::CString GetResultMessage() const;
    ATL::CString GetResultUrl() const;

    // -- Progress --
    float GetOverallProgress() const throw();
    float GetEncodeProgress() const throw();
    float GetUploadProgress() const throw();

private:
    HRESULT Encode();
    HRESULT Upload();

    DWORD              m_dwJobId;
    ATL::CString       m_strJobName;
    ATL::CString       m_strInputPath;
    ATL::CString       m_strOutputPath;
    ATL::CString       m_strServiceName;
    DWORD              m_dwProfileIndex;
    DWORD              m_dwWidth;
    DWORD              m_dwHeight;
    DWORD              m_dwQuality;
    PublishJobStatus   m_status;
    HRESULT            m_hrResult;
    ATL::CString       m_strResultMessage;
    ATL::CString       m_strResultUrl;
    float              m_flOverallProgress;
    float              m_flEncodeProgress;
    float              m_flUploadProgress;
    bool               m_bCancelled;
    bool               m_bPaused;

    PublishProgressCallBack* m_pCallback; // not owned
};

// ============================================================================
// PublishBackgroundJob
// ============================================================================
// Background publish job that executes a PublishJob on a worker thread.
// Manages the thread lifecycle and coordinates with the publish queue.
//
class PublishBackgroundJob
{
public:
    PublishBackgroundJob(PublishJob* pJob, PublishProgressCallBack* pCallback);
    ~PublishBackgroundJob();

    // -- Execution --
    HRESULT Start();
    HRESULT Cancel();
    bool IsRunning() const throw();

    // -- Access --
    PublishJob* GetJob() const throw();
    DWORD GetJobId() const throw();

    // -- Worker integration --
    void SetWorker(PublishBackgroundWorker* pWorker);

private:
    static unsigned int __stdcall ThreadProc(void* pParam);

    PublishJob*             m_pJob;       // owned
    PublishProgressCallBack* m_pCallback; // not owned
    PublishBackgroundWorker* m_pWorker;   // not owned
    std::thread*            m_pThread;
    bool                    m_bRunning;
};

// ============================================================================
// PublishBackgroundWorker
// ============================================================================
// Background thread pool for publish operations. Manages a queue of
// PublishBackgroundJob instances and processes them sequentially on
// background threads. Provides progress aggregation across all jobs.
//
class PublishBackgroundWorker
{
public:
    PublishBackgroundWorker();
    ~PublishBackgroundWorker();

    // -- Lifetime --
    HRESULT Initialize(DWORD dwMaxJobs = 2);
    void    Shutdown();

    // -- Queue management --
    HRESULT EnqueueJob(PublishJob* pJob);
    HRESULT CancelJob(DWORD dwJobId);
    HRESULT CancelAllJobs();

    // -- Status --
    DWORD GetPendingJobCount() const throw();
    DWORD GetActiveJobCount() const throw();
    DWORD GetCompletedJobCount() const throw();
    float GetOverallProgress() const throw();

    // -- Job lifecycle --
    void OnJobCompleted(PublishBackgroundJob* pJob);

    // -- Singleton --
    static PublishBackgroundWorker* GetInstance();

private:
    std::deque<PublishBackgroundJob*> m_pendingJobs;
    std::vector<PublishBackgroundJob*> m_activeJobs;
    std::vector<PublishBackgroundJob*> m_completedJobs;
    mutable CRITICAL_SECTION            m_csQueue;

    DWORD  m_dwMaxJobs;
    DWORD  m_dwCompletedJobCount;
    bool   m_bInitialized;

    static PublishBackgroundWorker* s_pInstance;
};

#endif // PUBLISH_JOB_H
