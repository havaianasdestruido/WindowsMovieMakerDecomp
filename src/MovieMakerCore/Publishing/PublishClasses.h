/*
 * PublishClasses.h
 *
 * Additional publishing RTTI classes for Windows Live Movie Maker 2012.
 * Provides the PublishManager hierarchy, PublishJob tracking classes,
 * PublishBackgroundTask, PublishItemMetadata, and PublishService
 * hierarchy used by the original WLMM publishing pipeline.
 *
 * RTTI: ?AVPublishManager@@, ?AVPublishManagerConfig@@,
 *       ?AVPublishManagerState@@, ?AVPublishJobProgress@@,
 *       ?AVPublishJobResult@@, ?AVPublishJobQueue@@,
 *       ?AVPublishBackgroundTask@@, ?AVPublishItemMetadata@@,
 *       ?AVPublishServiceBase@@, ?AVPublishServiceYouTube@@,
 *       ?AVPublishServiceSkyDrive@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef PUBLISH_CLASSES_H
#define PUBLISH_CLASSES_H

#include "../pch.h"
#include "PublishItem.h"
#include "PublishJob.h"

// ============================================================================
// PublishManagerState enum
// ============================================================================
enum PublishManagerStateValue
{
    PublishManagerStateIdle       = 0,
    PublishManagerStatePreparing  = 1,
    PublishManagerStatePublishing = 2,
    PublishManagerStateCompleting = 3,
    PublishManagerStateError      = 4
};

// ============================================================================
// PublishManagerConfig
// ============================================================================
// Configuration for the publish manager. Stores global publishing
// preferences such as maximum concurrent jobs, retry counts, and
// timeout values.
//
class PublishManagerConfig
{
public:
    PublishManagerConfig();
    virtual ~PublishManagerConfig();

    // -- Concurrency --
    void SetMaxConcurrentJobs(DWORD dwMaxJobs);
    DWORD GetMaxConcurrentJobs() const throw();

    // -- Retry --
    void SetMaxRetries(DWORD dwMaxRetries);
    DWORD GetMaxRetries() const throw();

    void SetRetryDelayMs(DWORD dwDelayMs);
    DWORD GetRetryDelayMs() const throw();

    // -- Timeout --
    void SetUploadTimeoutSec(DWORD dwTimeoutSec);
    DWORD GetUploadTimeoutSec() const throw();

    // -- Output directory --
    void SetTempOutputDirectory(LPCWSTR pszPath);
    ATL::CString GetTempOutputDirectory() const;

    // -- Notifications --
    void SetShowSummaryDialog(bool bShow);
    bool GetShowSummaryDialog() const throw();

private:
    DWORD      m_dwMaxConcurrentJobs;
    DWORD      m_dwMaxRetries;
    DWORD      m_dwRetryDelayMs;
    DWORD      m_dwUploadTimeoutSec;
    ATL::CString m_strTempOutputDir;
    bool       m_bShowSummaryDialog;
};

// ============================================================================
// PublishManagerState
// ============================================================================
// Tracks the state of the publish manager including currently queued
// and completed jobs, aggregate progress, and error information.
//
class PublishManagerState
{
public:
    PublishManagerState();
    virtual ~PublishManagerState();

    // -- State --
    void SetState(PublishManagerStateValue state);
    PublishManagerStateValue GetState() const throw();

    // -- Job tracking --
    void SetTotalJobCount(DWORD dwTotal);
    DWORD GetTotalJobCount() const throw();

    void SetCompletedJobCount(DWORD dwCompleted);
    DWORD GetCompletedJobCount() const throw();

    void SetFailedJobCount(DWORD dwFailed);
    DWORD GetFailedJobCount() const throw();

    // -- Progress --
    float GetOverallProgress() const throw();

    // -- Error --
    void SetLastErrorCode(HRESULT hr);
    HRESULT GetLastErrorCode() const throw();

    void SetLastErrorMessage(LPCWSTR pszMessage);
    ATL::CString GetLastErrorMessage() const;

    // -- Session timing --
    void SetSessionStartTime(const FILETIME& ft);
    FILETIME GetSessionStartTime() const throw();

    void SetSessionEndTime(const FILETIME& ft);
    FILETIME GetSessionEndTime() const throw();

private:
    PublishManagerStateValue m_state;
    DWORD    m_dwTotalJobs;
    DWORD    m_dwCompletedJobs;
    DWORD    m_dwFailedJobs;
    HRESULT  m_hrLastErrorCode;
    ATL::CString m_strLastErrorMessage;
    FILETIME m_ftSessionStart;
    FILETIME m_ftSessionEnd;
};

// ============================================================================
// PublishManager
// ============================================================================
// Top-level manager for the publishing workflow. Coordinates the publish
// background worker, service providers, and job queue. Manages session
// lifecycle from initialization through job submission to completion.
//
class PublishManager
{
public:
    PublishManager();
    virtual ~PublishManager();

    // -- Lifetime --
    HRESULT Initialize();
    void Shutdown();

    // -- Configuration --
    void SetConfig(PublishManagerConfig* pConfig);
    PublishManagerConfig* GetConfig() const throw();

    // -- Job submission --
    HRESULT SubmitJob(PublishJob* pJob);
    HRESULT CancelJob(DWORD dwJobId);
    HRESULT CancelAllJobs();

    // -- State --
    PublishManagerState* GetState() const throw();
    PublishManagerStateValue GetCurrentState() const throw();

    // -- Progress --
    float GetOverallProgress() const throw();

    // -- Results --
    HRESULT GetSessionResult() const throw();

private:
    PublishManagerConfig*  m_pConfig;
    PublishManagerState*   m_pState;
    bool                   m_bInitialized;
};

// ============================================================================
// PublishJobProgress
// ============================================================================
// Tracks detailed progress information for a single publish job,
// including per-stage progress, byte counts, and timing.
//
class PublishJobProgress
{
public:
    PublishJobProgress();
    virtual ~PublishJobProgress();

    // -- Job identity --
    void SetJobId(DWORD dwJobId);
    DWORD GetJobId() const throw();

    // -- Stage --
    void SetCurrentStage(LPCWSTR pszStage);
    ATL::CString GetCurrentStage() const;

    // -- Per-stage progress (0.0 - 1.0) --
    void SetEncodeProgress(float flProgress);
    float GetEncodeProgress() const throw();

    void SetUploadProgress(float flProgress);
    float GetUploadProgress() const throw();

    void SetOverallProgress(float flProgress);
    float GetOverallProgress() const throw();

    // -- Byte tracking --
    void SetTotalBytesToUpload(ULONGLONG cbTotal);
    ULONGLONG GetTotalBytesToUpload() const throw();

    void SetBytesUploaded(ULONGLONG cbUploaded);
    ULONGLONG GetBytesUploaded() const throw();

    // -- Timing --
    void SetStartTime(const FILETIME& ft);
    FILETIME GetStartTime() const throw();

    void SetEndTime(const FILETIME& ft);
    FILETIME GetEndTime() const throw();

    // -- Status text --
    void SetStatusText(LPCWSTR pszText);
    ATL::CString GetStatusText() const;

private:
    DWORD        m_dwJobId;
    ATL::CString m_strCurrentStage;
    float        m_flEncodeProgress;
    float        m_flUploadProgress;
    float        m_flOverallProgress;
    ULONGLONG    m_cbTotalBytes;
    ULONGLONG    m_cbBytesUploaded;
    FILETIME     m_ftStart;
    FILETIME     m_ftEnd;
    ATL::CString m_strStatusText;
};

// ============================================================================
// PublishJobResult
// ============================================================================
// Result information for a completed or failed publish job. Contains
// the outcome code, result URL (for successful publishes), error
// description, and result metadata.
//
class PublishJobResult
{
public:
    PublishJobResult();
    virtual ~PublishJobResult();

    // -- Job identity --
    void SetJobId(DWORD dwJobId);
    DWORD GetJobId() const throw();

    // -- Outcome --
    void SetResultHRESULT(HRESULT hr);
    HRESULT GetResultHRESULT() const throw();

    void SetSuccess(bool bSuccess);
    bool IsSuccess() const throw();

    // -- Result URL --
    void SetResultUrl(LPCWSTR pszUrl);
    ATL::CString GetResultUrl() const;

    // -- Error info --
    void SetErrorCode(DWORD dwCode);
    DWORD GetErrorCode() const throw();

    void SetErrorDescription(LPCWSTR pszDescription);
    ATL::CString GetErrorDescription() const;

    // -- Retry info --
    void SetRetryCount(DWORD dwCount);
    DWORD GetRetryCount() const throw();

    void SetWasRetried(bool bRetried);
    bool WasRetried() const throw();

    // -- Result metadata --
    void SetResultServiceName(LPCWSTR pszName);
    ATL::CString GetResultServiceName() const;

    void SetResultFileSize(ULONGLONG cbSize);
    ULONGLONG GetResultFileSize() const throw();

private:
    DWORD        m_dwJobId;
    HRESULT      m_hrResult;
    bool         m_bSuccess;
    ATL::CString m_strResultUrl;
    DWORD        m_dwErrorCode;
    ATL::CString m_strErrorDescription;
    DWORD        m_dwRetryCount;
    bool         m_bWasRetried;
    ATL::CString m_strServiceName;
    ULONGLONG    m_cbResultFileSize;
};

// ============================================================================
// PublishJobQueue
// ============================================================================
// FIFO queue for managing publish jobs. Supports priority ordering,
// job deduplication, and queue persistence for crash recovery.
//
class PublishJobQueue
{
public:
    PublishJobQueue();
    virtual ~PublishJobQueue();

    // -- Queue management --
    HRESULT Enqueue(PublishJob* pJob);
    HRESULT Dequeue(PublishJob** ppJob);
    HRESULT Peek(PublishJob** ppJob) const;

    // -- Cancellation --
    HRESULT CancelJob(DWORD dwJobId);
    HRESULT CancelAll();

    // -- Status --
    DWORD GetCount() const throw();
    bool  IsEmpty() const throw();

    // -- Peek at pending jobs --
    DWORD GetPendingJobIdAt(DWORD dwIndex) const;

    // -- Deduplication --
    bool HasJobForService(LPCWSTR pszServiceName) const;

    // -- Clear --
    void Clear();

private:
    std::deque<PublishJob*> m_jobs;
};

// ============================================================================
// PublishBackgroundTask
// ============================================================================
// Represents a single background task executed by the publish worker.
// Encapsulates work items that run on background threads with progress
// reporting and cancellation support.
//
class PublishBackgroundTask
{
public:
    PublishBackgroundTask();
    virtual ~PublishBackgroundTask();

    // -- Task identity --
    void SetTaskId(DWORD dwTaskId);
    DWORD GetTaskId() const throw();

    void SetTaskName(LPCWSTR pszName);
    ATL::CString GetTaskName() const;

    // -- Execution --
    HRESULT Execute();
    HRESULT Cancel();
    bool IsRunning() const throw();
    bool IsCancelled() const throw();

    // -- Progress --
    void SetProgress(float flProgress);
    float GetProgress() const throw();

    void SetStatusText(LPCWSTR pszText);
    ATL::CString GetStatusText() const;

    // -- Result --
    HRESULT GetResult() const throw();

private:
    DWORD        m_dwTaskId;
    ATL::CString m_strTaskName;
    bool         m_bRunning;
    bool         m_bCancelled;
    float        m_flProgress;
    ATL::CString m_strStatusText;
    HRESULT      m_hrResult;
};

// ============================================================================
// PublishItemMetadata
// ============================================================================
// Extended metadata for a publish item. Stores media-specific metadata
// such as video dimensions, duration, codec information, and content
// ratings that are passed to the publishing service.
//
class PublishItemMetadata
{
public:
    PublishItemMetadata();
    virtual ~PublishItemMetadata();

    // -- Video properties --
    void SetVideoWidth(UINT uWidth);
    UINT GetVideoWidth() const throw();

    void SetVideoHeight(UINT uHeight);
    UINT GetVideoHeight() const throw();

    void SetVideoDurationMs(ULONGLONG ullDurationMs);
    ULONGLONG GetVideoDurationMs() const throw();

    void SetVideoBitrateKbps(DWORD dwBitrate);
    DWORD GetVideoBitrateKbps() const throw();

    // -- Audio properties --
    void SetAudioBitrateKbps(DWORD dwBitrate);
    DWORD GetAudioBitrateKbps() const throw();

    void SetAudioSampleRate(DWORD dwSampleRate);
    DWORD GetAudioSampleRate() const throw();

    void SetAudioChannels(DWORD dwChannels);
    DWORD GetAudioChannels() const throw();

    // -- Codec --
    void SetVideoCodec(LPCWSTR pszCodec);
    ATL::CString GetVideoCodec() const;

    void SetAudioCodec(LPCWSTR pszCodec);
    ATL::CString GetAudioCodec() const;

    // -- Content --
    void SetContentType(LPCWSTR pszType);
    ATL::CString GetContentType() const;

    void SetContentRating(LPCWSTR pszRating);
    ATL::CString GetContentRating() const;

    void SetThumbnailPath(LPCWSTR pszPath);
    ATL::CString GetThumbnailPath() const;

    // -- Custom metadata --
    void SetMetadataValue(LPCWSTR pszKey, LPCWSTR pszValue);
    ATL::CString GetMetadataValue(LPCWSTR pszKey) const;

    // -- Aggregate --
    void SetFileSizeBytes(ULONGLONG cbSize);
    ULONGLONG GetFileSizeBytes() const throw();

private:
    UINT         m_uVideoWidth;
    UINT         m_uVideoHeight;
    ULONGLONG    m_ullVideoDurationMs;
    DWORD        m_dwVideoBitrateKbps;
    DWORD        m_dwAudioBitrateKbps;
    DWORD        m_dwAudioSampleRate;
    DWORD        m_dwAudioChannels;
    ATL::CString m_strVideoCodec;
    ATL::CString m_strAudioCodec;
    ATL::CString m_strContentType;
    ATL::CString m_strContentRating;
    ATL::CString m_strThumbnailPath;
    ULONGLONG    m_cbFileSize;

    std::map<ATL::CString, ATL::CString> m_metadataValues;
};

// ============================================================================
// PublishServiceBase
// ============================================================================
// Abstract base class for publish service implementations. Defines
// the interface used by PublishManager to interact with specific
// publishing targets (YouTube, SkyDrive, etc.).
//
class PublishServiceBase
{
public:
    PublishServiceBase();
    virtual ~PublishServiceBase();

    // -- Service identity --
    virtual LPCWSTR GetServiceName() const = 0;
    virtual PublishServiceType GetServiceType() const = 0;

    // -- Authentication --
    virtual HRESULT Authenticate(LPCWSTR pszToken, LPCWSTR pszSecret);
    virtual HRESULT RefreshAuthentication();
    virtual bool IsAuthenticated() const throw();

    // -- Upload --
    virtual HRESULT UploadFile(LPCWSTR pszFilePath, PublishJobProgress* pProgress);
    virtual HRESULT CancelUpload();
    virtual bool IsUploading() const throw();

    // -- Capabilities --
    virtual HRESULT GetMaxVideoDimensions(UINT* puWidth, UINT* puHeight) const;
    virtual HRESULT GetMaxFileSizeMB(DWORD* pdwSizeMB) const;
    virtual HRESULT GetMaxDurationSec(DWORD* pdwDurationSec) const;

    // -- Validation --
    virtual HRESULT ValidateFile(LPCWSTR pszFilePath) const;

protected:
    bool     m_bAuthenticated;
    bool     m_bUploading;
    ATL::CString m_strServiceName;
};

// ============================================================================
// PublishServiceYouTube
// ============================================================================
// YouTube-specific publishing service implementation. Handles YouTube
// API authentication, video upload with resumable upload protocol,
// and metadata submission.
//
class PublishServiceYouTube : public PublishServiceBase
{
public:
    PublishServiceYouTube();
    ~PublishServiceYouTube();

    // -- PublishServiceBase --
    LPCWSTR GetServiceName() const override;
    PublishServiceType GetServiceType() const override;

    HRESULT Authenticate(LPCWSTR pszToken, LPCWSTR pszSecret) override;
    HRESULT RefreshAuthentication() override;

    HRESULT UploadFile(LPCWSTR pszFilePath, PublishJobProgress* pProgress) override;
    HRESULT CancelUpload() override;

    HRESULT GetMaxVideoDimensions(UINT* puWidth, UINT* puHeight) const override;
    HRESULT GetMaxFileSizeMB(DWORD* pdwSizeMB) const override;
    HRESULT GetMaxDurationSec(DWORD* pdwDurationSec) const override;

    HRESULT ValidateFile(LPCWSTR pszFilePath) const override;

    // -- YouTube-specific --
    void SetDeveloperKey(LPCWSTR pszKey);
    ATL::CString GetDeveloperKey() const;

    void SetCategoryId(LPCWSTR pszCategoryId);
    ATL::CString GetCategoryId() const;

private:
    ATL::CString m_strDeveloperKey;
    ATL::CString m_strCategoryId;
};

// ============================================================================
// PublishServiceSkyDrive
// ============================================================================
// SkyDrive (OneDrive) publishing service implementation. Handles
// Windows Live ID authentication and file upload to the user's
// SkyDrive storage.
//
class PublishServiceSkyDrive : public PublishServiceBase
{
public:
    PublishServiceSkyDrive();
    ~PublishServiceSkyDrive();

    // -- PublishServiceBase --
    LPCWSTR GetServiceName() const override;
    PublishServiceType GetServiceType() const override;

    HRESULT Authenticate(LPCWSTR pszToken, LPCWSTR pszSecret) override;
    HRESULT RefreshAuthentication() override;

    HRESULT UploadFile(LPCWSTR pszFilePath, PublishJobProgress* pProgress) override;
    HRESULT CancelUpload() override;

    HRESULT GetMaxVideoDimensions(UINT* puWidth, UINT* puHeight) const override;
    HRESULT GetMaxFileSizeMB(DWORD* pdwSizeMB) const override;
    HRESULT GetMaxDurationSec(DWORD* pdwDurationSec) const override;

    HRESULT ValidateFile(LPCWSTR pszFilePath) const override;

    // -- SkyDrive-specific --
    void SetFolderName(LPCWSTR pszFolder);
    ATL::CString GetFolderName() const;

    void SetOverwriteExisting(bool bOverwrite);
    bool GetOverwriteExisting() const throw();

private:
    ATL::CString m_strFolderName;
    bool         m_bOverwriteExisting;
};

#endif // PUBLISH_CLASSES_H
