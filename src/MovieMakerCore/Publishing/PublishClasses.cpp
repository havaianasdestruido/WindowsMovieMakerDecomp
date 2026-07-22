#include "pch.h"

/*
 * PublishClasses.cpp
 *
 * Implementation of additional publishing RTTI classes for
 * Windows Live Movie Maker 2012.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "PublishClasses.h"

// ============================================================================
// PublishManagerConfig implementation
// ============================================================================

PublishManagerConfig::PublishManagerConfig()
    : m_dwMaxConcurrentJobs(2)
    , m_dwMaxRetries(3)
    , m_dwRetryDelayMs(5000)
    , m_dwUploadTimeoutSec(3600)
    , m_bShowSummaryDialog(true)
{
}

PublishManagerConfig::~PublishManagerConfig()
{
}

void PublishManagerConfig::SetMaxConcurrentJobs(DWORD dwMaxJobs)
{
    m_dwMaxConcurrentJobs = (dwMaxJobs < 1) ? 1 : dwMaxJobs;
}

DWORD PublishManagerConfig::GetMaxConcurrentJobs() const throw()
{
    return m_dwMaxConcurrentJobs;
}

void PublishManagerConfig::SetMaxRetries(DWORD dwMaxRetries)
{
    m_dwMaxRetries = dwMaxRetries;
}

DWORD PublishManagerConfig::GetMaxRetries() const throw()
{
    return m_dwMaxRetries;
}

void PublishManagerConfig::SetRetryDelayMs(DWORD dwDelayMs)
{
    m_dwRetryDelayMs = dwDelayMs;
}

DWORD PublishManagerConfig::GetRetryDelayMs() const throw()
{
    return m_dwRetryDelayMs;
}

void PublishManagerConfig::SetUploadTimeoutSec(DWORD dwTimeoutSec)
{
    m_dwUploadTimeoutSec = dwTimeoutSec;
}

DWORD PublishManagerConfig::GetUploadTimeoutSec() const throw()
{
    return m_dwUploadTimeoutSec;
}

void PublishManagerConfig::SetTempOutputDirectory(LPCWSTR pszPath)
{
    m_strTempOutputDir = pszPath ? pszPath : L"";
}

ATL::CString PublishManagerConfig::GetTempOutputDirectory() const
{
    return m_strTempOutputDir;
}

void PublishManagerConfig::SetShowSummaryDialog(bool bShow)
{
    m_bShowSummaryDialog = bShow;
}

bool PublishManagerConfig::GetShowSummaryDialog() const throw()
{
    return m_bShowSummaryDialog;
}

// ============================================================================
// PublishManagerState implementation
// ============================================================================

PublishManagerState::PublishManagerState()
    : m_state(PublishManagerStateIdle)
    , m_dwTotalJobs(0)
    , m_dwCompletedJobs(0)
    , m_dwFailedJobs(0)
    , m_hrLastErrorCode(S_OK)
{
    ZeroMemory(&m_ftSessionStart, sizeof(FILETIME));
    ZeroMemory(&m_ftSessionEnd, sizeof(FILETIME));
}

PublishManagerState::~PublishManagerState()
{
}

void PublishManagerState::SetState(PublishManagerStateValue state)
{
    m_state = state;
}

PublishManagerStateValue PublishManagerState::GetState() const throw()
{
    return m_state;
}

void PublishManagerState::SetTotalJobCount(DWORD dwTotal)
{
    m_dwTotalJobs = dwTotal;
}

DWORD PublishManagerState::GetTotalJobCount() const throw()
{
    return m_dwTotalJobs;
}

void PublishManagerState::SetCompletedJobCount(DWORD dwCompleted)
{
    m_dwCompletedJobs = dwCompleted;
}

DWORD PublishManagerState::GetCompletedJobCount() const throw()
{
    return m_dwCompletedJobs;
}

void PublishManagerState::SetFailedJobCount(DWORD dwFailed)
{
    m_dwFailedJobs = dwFailed;
}

DWORD PublishManagerState::GetFailedJobCount() const throw()
{
    return m_dwFailedJobs;
}

float PublishManagerState::GetOverallProgress() const throw()
{
    if (m_dwTotalJobs == 0)
        return 0.0f;
    return static_cast<float>(m_dwCompletedJobs + m_dwFailedJobs) /
           static_cast<float>(m_dwTotalJobs);
}

void PublishManagerState::SetLastErrorCode(HRESULT hr)
{
    m_hrLastErrorCode = hr;
}

HRESULT PublishManagerState::GetLastErrorCode() const throw()
{
    return m_hrLastErrorCode;
}

void PublishManagerState::SetLastErrorMessage(LPCWSTR pszMessage)
{
    m_strLastErrorMessage = pszMessage ? pszMessage : L"";
}

ATL::CString PublishManagerState::GetLastErrorMessage() const
{
    return m_strLastErrorMessage;
}

void PublishManagerState::SetSessionStartTime(const FILETIME& ft)
{
    m_ftSessionStart = ft;
}

FILETIME PublishManagerState::GetSessionStartTime() const throw()
{
    return m_ftSessionStart;
}

void PublishManagerState::SetSessionEndTime(const FILETIME& ft)
{
    m_ftSessionEnd = ft;
}

FILETIME PublishManagerState::GetSessionEndTime() const throw()
{
    return m_ftSessionEnd;
}

// ============================================================================
// PublishManager implementation
// ============================================================================

PublishManager::PublishManager()
    : m_pConfig(nullptr)
    , m_pState(nullptr)
    , m_dwNextJobId(0)
    , m_bInitialized(false)
{
}

PublishManager::~PublishManager()
{
    Shutdown();
}

HRESULT PublishManager::Initialize()
{
    if (m_bInitialized)
        return S_FALSE;

    m_pConfig = new PublishManagerConfig();
    m_pState = new PublishManagerState();
    m_bInitialized = true;

    return S_OK;
}

void PublishManager::Shutdown()
{
    if (!m_bInitialized)
        return;

    delete m_pConfig;
    m_pConfig = nullptr;

    delete m_pState;
    m_pState = nullptr;

    m_bInitialized = false;
}

void PublishManager::SetConfig(PublishManagerConfig* pConfig)
{
    if (m_pConfig)
        delete m_pConfig;
    m_pConfig = pConfig;
}

PublishManagerConfig* PublishManager::GetConfig() const throw()
{
    return m_pConfig;
}

HRESULT PublishManager::SubmitJob(PublishJob* pJob)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    if (!pJob)
        return E_POINTER;

    if (!m_pState)
        return E_UNEXPECTED;

    if (m_pState->GetState() == PublishManagerStateIdle)
        m_pState->SetState(PublishManagerStatePreparing);

    if (pJob->GetJobId() == 0)
    {
        m_dwNextJobId++;
        pJob->SetJobId(m_dwNextJobId);
    }

    m_pState->SetTotalJobCount(m_pState->GetTotalJobCount() + 1);

    HRESULT hr = m_jobQueue.Enqueue(pJob);
    if (FAILED(hr))
    {
        m_pState->SetState(PublishManagerStateError);
        m_pState->SetLastErrorCode(hr);
        m_pState->SetLastErrorMessage(L"Failed to enqueue job");
    }

    return hr;
}

HRESULT PublishManager::CancelJob(DWORD dwJobId)
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    HRESULT hr = m_jobQueue.CancelJob(dwJobId);
    if (SUCCEEDED(hr))
    {
        m_pState->SetFailedJobCount(m_pState->GetFailedJobCount() + 1);
        m_pState->SetLastErrorCode(E_ABORT);
        m_pState->SetLastErrorMessage(L"Job cancelled by user");
    }

    return hr;
}

HRESULT PublishManager::CancelAllJobs()
{
    if (!m_bInitialized)
        return E_UNEXPECTED;

    DWORD dwRemaining = m_jobQueue.GetCount();
    HRESULT hr = m_jobQueue.CancelAll();
    m_pState->SetFailedJobCount(m_pState->GetFailedJobCount() + dwRemaining);
    m_pState->SetLastErrorCode(E_ABORT);
    m_pState->SetLastErrorMessage(L"All jobs cancelled by user");

    return hr;
}

PublishManagerState* PublishManager::GetState() const throw()
{
    return m_pState;
}

PublishManagerStateValue PublishManager::GetCurrentState() const throw()
{
    return m_pState ? m_pState->GetState() : PublishManagerStateIdle;
}

float PublishManager::GetOverallProgress() const throw()
{
    return m_pState ? m_pState->GetOverallProgress() : 0.0f;
}

HRESULT PublishManager::GetSessionResult() const throw()
{
    if (!m_pState)
        return E_UNEXPECTED;

    return m_pState->GetLastErrorCode();
}

// ============================================================================
// PublishJobProgress implementation
// ============================================================================

PublishJobProgress::PublishJobProgress()
    : m_dwJobId(0)
    , m_flEncodeProgress(0.0f)
    , m_flUploadProgress(0.0f)
    , m_flOverallProgress(0.0f)
    , m_cbTotalBytes(0)
    , m_cbBytesUploaded(0)
{
    ZeroMemory(&m_ftStart, sizeof(FILETIME));
    ZeroMemory(&m_ftEnd, sizeof(FILETIME));
}

PublishJobProgress::~PublishJobProgress()
{
}

void PublishJobProgress::SetJobId(DWORD dwJobId)
{
    m_dwJobId = dwJobId;
}

DWORD PublishJobProgress::GetJobId() const throw()
{
    return m_dwJobId;
}

void PublishJobProgress::SetCurrentStage(LPCWSTR pszStage)
{
    m_strCurrentStage = pszStage ? pszStage : L"";
}

ATL::CString PublishJobProgress::GetCurrentStage() const
{
    return m_strCurrentStage;
}

void PublishJobProgress::SetEncodeProgress(float flProgress)
{
    m_flEncodeProgress = (flProgress < 0.0f) ? 0.0f : ((flProgress > 1.0f) ? 1.0f : flProgress);
}

float PublishJobProgress::GetEncodeProgress() const throw()
{
    return m_flEncodeProgress;
}

void PublishJobProgress::SetUploadProgress(float flProgress)
{
    m_flUploadProgress = (flProgress < 0.0f) ? 0.0f : ((flProgress > 1.0f) ? 1.0f : flProgress);
}

float PublishJobProgress::GetUploadProgress() const throw()
{
    return m_flUploadProgress;
}

void PublishJobProgress::SetOverallProgress(float flProgress)
{
    m_flOverallProgress = (flProgress < 0.0f) ? 0.0f : ((flProgress > 1.0f) ? 1.0f : flProgress);
}

float PublishJobProgress::GetOverallProgress() const throw()
{
    return m_flOverallProgress;
}

void PublishJobProgress::SetTotalBytesToUpload(ULONGLONG cbTotal)
{
    m_cbTotalBytes = cbTotal;
}

ULONGLONG PublishJobProgress::GetTotalBytesToUpload() const throw()
{
    return m_cbTotalBytes;
}

void PublishJobProgress::SetBytesUploaded(ULONGLONG cbUploaded)
{
    m_cbBytesUploaded = cbUploaded;
}

ULONGLONG PublishJobProgress::GetBytesUploaded() const throw()
{
    return m_cbBytesUploaded;
}

void PublishJobProgress::SetStartTime(const FILETIME& ft)
{
    m_ftStart = ft;
}

FILETIME PublishJobProgress::GetStartTime() const throw()
{
    return m_ftStart;
}

void PublishJobProgress::SetEndTime(const FILETIME& ft)
{
    m_ftEnd = ft;
}

FILETIME PublishJobProgress::GetEndTime() const throw()
{
    return m_ftEnd;
}

void PublishJobProgress::SetStatusText(LPCWSTR pszText)
{
    m_strStatusText = pszText ? pszText : L"";
}

ATL::CString PublishJobProgress::GetStatusText() const
{
    return m_strStatusText;
}

// ============================================================================
// PublishJobResult implementation
// ============================================================================

PublishJobResult::PublishJobResult()
    : m_dwJobId(0)
    , m_hrResult(S_OK)
    , m_bSuccess(false)
    , m_dwErrorCode(0)
    , m_dwRetryCount(0)
    , m_bWasRetried(false)
    , m_cbResultFileSize(0)
{
}

PublishJobResult::~PublishJobResult()
{
}

void PublishJobResult::SetJobId(DWORD dwJobId)
{
    m_dwJobId = dwJobId;
}

DWORD PublishJobResult::GetJobId() const throw()
{
    return m_dwJobId;
}

void PublishJobResult::SetResultHRESULT(HRESULT hr)
{
    m_hrResult = hr;
    m_bSuccess = SUCCEEDED(hr);
}

HRESULT PublishJobResult::GetResultHRESULT() const throw()
{
    return m_hrResult;
}

void PublishJobResult::SetSuccess(bool bSuccess)
{
    m_bSuccess = bSuccess;
}

bool PublishJobResult::IsSuccess() const throw()
{
    return m_bSuccess;
}

void PublishJobResult::SetResultUrl(LPCWSTR pszUrl)
{
    m_strResultUrl = pszUrl ? pszUrl : L"";
}

ATL::CString PublishJobResult::GetResultUrl() const
{
    return m_strResultUrl;
}

void PublishJobResult::SetErrorCode(DWORD dwCode)
{
    m_dwErrorCode = dwCode;
}

DWORD PublishJobResult::GetErrorCode() const throw()
{
    return m_dwErrorCode;
}

void PublishJobResult::SetErrorDescription(LPCWSTR pszDescription)
{
    m_strErrorDescription = pszDescription ? pszDescription : L"";
}

ATL::CString PublishJobResult::GetErrorDescription() const
{
    return m_strErrorDescription;
}

void PublishJobResult::SetRetryCount(DWORD dwCount)
{
    m_dwRetryCount = dwCount;
}

DWORD PublishJobResult::GetRetryCount() const throw()
{
    return m_dwRetryCount;
}

void PublishJobResult::SetWasRetried(bool bRetried)
{
    m_bWasRetried = bRetried;
}

bool PublishJobResult::WasRetried() const throw()
{
    return m_bWasRetried;
}

void PublishJobResult::SetResultServiceName(LPCWSTR pszName)
{
    m_strServiceName = pszName ? pszName : L"";
}

ATL::CString PublishJobResult::GetResultServiceName() const
{
    return m_strServiceName;
}

void PublishJobResult::SetResultFileSize(ULONGLONG cbSize)
{
    m_cbResultFileSize = cbSize;
}

ULONGLONG PublishJobResult::GetResultFileSize() const throw()
{
    return m_cbResultFileSize;
}

// ============================================================================
// PublishJobQueue implementation
// ============================================================================

PublishJobQueue::PublishJobQueue()
{
}

PublishJobQueue::~PublishJobQueue()
{
    Clear();
}

HRESULT PublishJobQueue::Enqueue(PublishJob* pJob)
{
    if (!pJob)
        return E_POINTER;

    m_jobs.push_back(pJob);
    return S_OK;
}

HRESULT PublishJobQueue::Dequeue(PublishJob** ppJob)
{
    if (!ppJob)
        return E_POINTER;

    if (m_jobs.empty())
    {
        *ppJob = nullptr;
        return S_FALSE;
    }

    *ppJob = m_jobs.front();
    m_jobs.pop_front();
    return S_OK;
}

HRESULT PublishJobQueue::Peek(PublishJob** ppJob) const
{
    if (!ppJob)
        return E_POINTER;

    if (m_jobs.empty())
    {
        *ppJob = nullptr;
        return S_FALSE;
    }

    *ppJob = m_jobs.front();
    return S_OK;
}

HRESULT PublishJobQueue::CancelJob(DWORD dwJobId)
{
    for (auto it = m_jobs.begin(); it != m_jobs.end(); ++it)
    {
        if ((*it)->GetJobId() == dwJobId)
        {
            (*it)->Cancel();
            delete *it;
            m_jobs.erase(it);
            return S_OK;
        }
    }
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT PublishJobQueue::CancelAll()
{
    while (!m_jobs.empty())
    {
        m_jobs.front()->Cancel();
        delete m_jobs.front();
        m_jobs.pop_front();
    }
    return S_OK;
}

DWORD PublishJobQueue::GetCount() const throw()
{
    return static_cast<DWORD>(m_jobs.size());
}

bool PublishJobQueue::IsEmpty() const throw()
{
    return m_jobs.empty();
}

DWORD PublishJobQueue::GetPendingJobIdAt(DWORD dwIndex) const
{
    if (dwIndex < m_jobs.size())
        return m_jobs[dwIndex]->GetJobId();
    return 0;
}

bool PublishJobQueue::HasJobForService(LPCWSTR pszServiceName) const
{
    if (!pszServiceName)
        return false;

    for (const auto& pJob : m_jobs)
    {
        if (pJob->GetServiceName().CompareNoCase(pszServiceName) == 0)
            return true;
    }
    return false;
}

void PublishJobQueue::Clear()
{
    while (!m_jobs.empty())
    {
        delete m_jobs.front();
        m_jobs.pop_front();
    }
}

// ============================================================================
// PublishBackgroundTask implementation
// ============================================================================

PublishBackgroundTask::PublishBackgroundTask()
    : m_dwTaskId(0)
    , m_bRunning(false)
    , m_bCancelled(false)
    , m_flProgress(0.0f)
    , m_hrResult(S_OK)
{
}

PublishBackgroundTask::~PublishBackgroundTask()
{
}

void PublishBackgroundTask::SetTaskId(DWORD dwTaskId)
{
    m_dwTaskId = dwTaskId;
}

DWORD PublishBackgroundTask::GetTaskId() const throw()
{
    return m_dwTaskId;
}

void PublishBackgroundTask::SetTaskName(LPCWSTR pszName)
{
    m_strTaskName = pszName ? pszName : L"";
}

ATL::CString PublishBackgroundTask::GetTaskName() const
{
    return m_strTaskName;
}

HRESULT PublishBackgroundTask::Execute()
{
    if (m_bRunning)
        return S_FALSE;

    m_bRunning = true;
    m_bCancelled = false;
    m_flProgress = 0.0f;
    m_hrResult = S_OK;
    m_strStatusText = L"Starting task...";

    if (m_bCancelled)
    {
        m_hrResult = E_ABORT;
        m_strStatusText = L"Task cancelled";
        m_bRunning = false;
        return E_ABORT;
    }

    m_flProgress = 0.25f;
    m_strStatusText = L"Task in progress...";

    if (m_bCancelled)
    {
        m_hrResult = E_ABORT;
        m_strStatusText = L"Task cancelled";
        m_bRunning = false;
        return E_ABORT;
    }

    m_flProgress = 0.5f;
    m_strStatusText = L"Task in progress...";

    if (m_bCancelled)
    {
        m_hrResult = E_ABORT;
        m_strStatusText = L"Task cancelled";
        m_bRunning = false;
        return E_ABORT;
    }

    m_flProgress = 0.75f;
    m_strStatusText = L"Finalizing task...";

    if (m_bCancelled)
    {
        m_hrResult = E_ABORT;
        m_strStatusText = L"Task cancelled";
        m_bRunning = false;
        return E_ABORT;
    }

    m_flProgress = 1.0f;
    m_strStatusText = L"Task completed";
    m_bRunning = false;
    return S_OK;
}

HRESULT PublishBackgroundTask::Cancel()
{
    m_bCancelled = true;
    return S_OK;
}

bool PublishBackgroundTask::IsRunning() const throw()
{
    return m_bRunning;
}

bool PublishBackgroundTask::IsCancelled() const throw()
{
    return m_bCancelled;
}

void PublishBackgroundTask::SetProgress(float flProgress)
{
    m_flProgress = (flProgress < 0.0f) ? 0.0f : ((flProgress > 1.0f) ? 1.0f : flProgress);
}

float PublishBackgroundTask::GetProgress() const throw()
{
    return m_flProgress;
}

void PublishBackgroundTask::SetStatusText(LPCWSTR pszText)
{
    m_strStatusText = pszText ? pszText : L"";
}

ATL::CString PublishBackgroundTask::GetStatusText() const
{
    return m_strStatusText;
}

HRESULT PublishBackgroundTask::GetResult() const throw()
{
    return m_hrResult;
}

// ============================================================================
// PublishItemMetadata implementation
// ============================================================================

PublishItemMetadata::PublishItemMetadata()
    : m_uVideoWidth(0)
    , m_uVideoHeight(0)
    , m_ullVideoDurationMs(0)
    , m_dwVideoBitrateKbps(0)
    , m_dwAudioBitrateKbps(0)
    , m_dwAudioSampleRate(0)
    , m_dwAudioChannels(0)
    , m_cbFileSize(0)
{
}

PublishItemMetadata::~PublishItemMetadata()
{
}

void PublishItemMetadata::SetVideoWidth(UINT uWidth)
{
    m_uVideoWidth = uWidth;
}

UINT PublishItemMetadata::GetVideoWidth() const throw()
{
    return m_uVideoWidth;
}

void PublishItemMetadata::SetVideoHeight(UINT uHeight)
{
    m_uVideoHeight = uHeight;
}

UINT PublishItemMetadata::GetVideoHeight() const throw()
{
    return m_uVideoHeight;
}

void PublishItemMetadata::SetVideoDurationMs(ULONGLONG ullDurationMs)
{
    m_ullVideoDurationMs = ullDurationMs;
}

ULONGLONG PublishItemMetadata::GetVideoDurationMs() const throw()
{
    return m_ullVideoDurationMs;
}

void PublishItemMetadata::SetVideoBitrateKbps(DWORD dwBitrate)
{
    m_dwVideoBitrateKbps = dwBitrate;
}

DWORD PublishItemMetadata::GetVideoBitrateKbps() const throw()
{
    return m_dwVideoBitrateKbps;
}

void PublishItemMetadata::SetAudioBitrateKbps(DWORD dwBitrate)
{
    m_dwAudioBitrateKbps = dwBitrate;
}

DWORD PublishItemMetadata::GetAudioBitrateKbps() const throw()
{
    return m_dwAudioBitrateKbps;
}

void PublishItemMetadata::SetAudioSampleRate(DWORD dwSampleRate)
{
    m_dwAudioSampleRate = dwSampleRate;
}

DWORD PublishItemMetadata::GetAudioSampleRate() const throw()
{
    return m_dwAudioSampleRate;
}

void PublishItemMetadata::SetAudioChannels(DWORD dwChannels)
{
    m_dwAudioChannels = dwChannels;
}

DWORD PublishItemMetadata::GetAudioChannels() const throw()
{
    return m_dwAudioChannels;
}

void PublishItemMetadata::SetVideoCodec(LPCWSTR pszCodec)
{
    m_strVideoCodec = pszCodec ? pszCodec : L"";
}

ATL::CString PublishItemMetadata::GetVideoCodec() const
{
    return m_strVideoCodec;
}

void PublishItemMetadata::SetAudioCodec(LPCWSTR pszCodec)
{
    m_strAudioCodec = pszCodec ? pszCodec : L"";
}

ATL::CString PublishItemMetadata::GetAudioCodec() const
{
    return m_strAudioCodec;
}

void PublishItemMetadata::SetContentType(LPCWSTR pszType)
{
    m_strContentType = pszType ? pszType : L"";
}

ATL::CString PublishItemMetadata::GetContentType() const
{
    return m_strContentType;
}

void PublishItemMetadata::SetContentRating(LPCWSTR pszRating)
{
    m_strContentRating = pszRating ? pszRating : L"";
}

ATL::CString PublishItemMetadata::GetContentRating() const
{
    return m_strContentRating;
}

void PublishItemMetadata::SetThumbnailPath(LPCWSTR pszPath)
{
    m_strThumbnailPath = pszPath ? pszPath : L"";
}

ATL::CString PublishItemMetadata::GetThumbnailPath() const
{
    return m_strThumbnailPath;
}

void PublishItemMetadata::SetMetadataValue(LPCWSTR pszKey, LPCWSTR pszValue)
{
    if (pszKey)
        m_metadataValues[pszKey] = pszValue ? pszValue : L"";
}

ATL::CString PublishItemMetadata::GetMetadataValue(LPCWSTR pszKey) const
{
    auto it = m_metadataValues.find(pszKey);
    if (it != m_metadataValues.end())
        return it->second;
    return L"";
}

void PublishItemMetadata::SetFileSizeBytes(ULONGLONG cbSize)
{
    m_cbFileSize = cbSize;
}

ULONGLONG PublishItemMetadata::GetFileSizeBytes() const throw()
{
    return m_cbFileSize;
}

// ============================================================================
// PublishServiceBase implementation
// ============================================================================

PublishServiceBase::PublishServiceBase()
    : m_bAuthenticated(false)
    , m_bUploading(false)
{
}

PublishServiceBase::~PublishServiceBase()
{
}

HRESULT PublishServiceBase::Authenticate(LPCWSTR pszToken, LPCWSTR pszSecret)
{
    if (!pszToken)
        return E_POINTER;

    UNREFERENCED_PARAMETER(pszSecret);

    m_bAuthenticated = true;
    return S_OK;
}

HRESULT PublishServiceBase::RefreshAuthentication()
{
    if (!m_bAuthenticated)
        return E_UNEXPECTED;

    return S_OK;
}

bool PublishServiceBase::IsAuthenticated() const throw()
{
    return m_bAuthenticated;
}

HRESULT PublishServiceBase::UploadFile(LPCWSTR pszFilePath, PublishJobProgress* pProgress)
{
    if (!pszFilePath)
        return E_POINTER;

    if (!m_bAuthenticated)
        return E_ACCESSDENIED;

    m_bUploading = true;

    if (pProgress)
    {
        pProgress->SetCurrentStage(L"Uploading");
        pProgress->SetUploadProgress(0.0f);
    }

    // Base class: report completion immediately.
    // Derived classes override with actual upload logic.

    if (pProgress)
    {
        pProgress->SetUploadProgress(1.0f);
        pProgress->SetOverallProgress(1.0f);
    }

    m_bUploading = false;
    return S_OK;
}

HRESULT PublishServiceBase::CancelUpload()
{
    if (!m_bUploading)
        return S_FALSE;

    m_bUploading = false;
    return S_OK;
}

bool PublishServiceBase::IsUploading() const throw()
{
    return m_bUploading;
}

HRESULT PublishServiceBase::GetMaxVideoDimensions(UINT* puWidth, UINT* puHeight) const
{
    if (!puWidth || !puHeight)
        return E_POINTER;
    *puWidth = 1920;
    *puHeight = 1080;
    return S_OK;
}

HRESULT PublishServiceBase::GetMaxFileSizeMB(DWORD* pdwSizeMB) const
{
    if (!pdwSizeMB)
        return E_POINTER;
    *pdwSizeMB = 2048;
    return S_OK;
}

HRESULT PublishServiceBase::GetMaxDurationSec(DWORD* pdwDurationSec) const
{
    if (!pdwDurationSec)
        return E_POINTER;
    *pdwDurationSec = 3600;
    return S_OK;
}

HRESULT PublishServiceBase::ValidateFile(LPCWSTR pszFilePath) const
{
    if (!pszFilePath)
        return E_POINTER;

    DWORD dwAttr = GetFileAttributesW(pszFilePath);
    if (dwAttr == INVALID_FILE_ATTRIBUTES)
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
        return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExW(pszFilePath, GetFileExInfoStandard, &fad))
    {
        ULONGLONG cbFileSize = (static_cast<ULONGLONG>(fad.nFileSizeHigh) << 32) | fad.nFileSizeLow;
        DWORD dwMaxSizeMB = 0;
        const_cast<PublishServiceBase*>(this)->GetMaxFileSizeMB(&dwMaxSizeMB);
        if (dwMaxSizeMB > 0 && cbFileSize > static_cast<ULONGLONG>(dwMaxSizeMB) * 1024 * 1024)
            return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
    }

    return S_OK;
}

// ============================================================================
// PublishServiceYouTube implementation
// ============================================================================

PublishServiceYouTube::PublishServiceYouTube()
{
    m_strServiceName = L"YouTube";
}

PublishServiceYouTube::~PublishServiceYouTube()
{
}

LPCWSTR PublishServiceYouTube::GetServiceName() const
{
    return m_strServiceName;
}

PublishServiceType PublishServiceYouTube::GetServiceType() const
{
    return PublishServiceTypeYouTube;
}

HRESULT PublishServiceYouTube::Authenticate(LPCWSTR pszToken, LPCWSTR pszSecret)
{
    if (!pszToken || !pszSecret)
        return E_POINTER;

    if (wcslen(pszToken) == 0 || wcslen(pszSecret) == 0)
        return E_INVALIDARG;

    m_bAuthenticated = true;
    return S_OK;
}

HRESULT PublishServiceYouTube::RefreshAuthentication()
{
    if (!m_bAuthenticated)
        return E_UNEXPECTED;

    return S_OK;
}

HRESULT PublishServiceYouTube::UploadFile(LPCWSTR pszFilePath, PublishJobProgress* pProgress)
{
    if (!pszFilePath)
        return E_POINTER;

    if (!m_bAuthenticated)
        return E_ACCESSDENIED;

    HRESULT hr = ValidateFile(pszFilePath);
    if (FAILED(hr))
        return hr;

    m_bUploading = true;

    if (pProgress)
    {
        pProgress->SetCurrentStage(L"Uploading to YouTube");
        pProgress->SetUploadProgress(0.0f);
    }

    WIN32_FILE_ATTRIBUTE_DATA fad;
    ULONGLONG cbFileSize = 0;
    if (GetFileAttributesExW(pszFilePath, GetFileExInfoStandard, &fad))
        cbFileSize = (static_cast<ULONGLONG>(fad.nFileSizeHigh) << 32) | fad.nFileSizeLow;

    HANDLE hFile = CreateFileW(pszFilePath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        m_bUploading = false;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const DWORD cbChunkSize = 1024 * 1024;
    std::vector<BYTE> buffer(cbChunkSize);
    ULONGLONG cbUploaded = 0;
    DWORD cbRead = 0;

    while (cbUploaded < cbFileSize)
    {
        if (!m_bUploading)
        {
            CloseHandle(hFile);
            return E_ABORT;
        }

        if (!ReadFile(hFile, buffer.data(), cbChunkSize, &cbRead, NULL) || cbRead == 0)
            break;

        cbUploaded += cbRead;

        if (pProgress && cbFileSize > 0)
        {
            float flProgress = static_cast<float>(cbUploaded) / static_cast<float>(cbFileSize);
            pProgress->SetUploadProgress(flProgress);
            pProgress->SetBytesUploaded(cbUploaded);
        }

        ::Sleep(10);
    }

    CloseHandle(hFile);

    if (cbUploaded >= cbFileSize)
    {
        if (pProgress)
        {
            pProgress->SetUploadProgress(1.0f);
            pProgress->SetOverallProgress(1.0f);
        }
    }

    m_bUploading = false;
    return S_OK;
}

HRESULT PublishServiceYouTube::CancelUpload()
{
    m_bUploading = false;
    return S_OK;
}

HRESULT PublishServiceYouTube::GetMaxVideoDimensions(UINT* puWidth, UINT* puHeight) const
{
    if (!puWidth || !puHeight)
        return E_POINTER;
    *puWidth = 3840;
    *puHeight = 2160;
    return S_OK;
}

HRESULT PublishServiceYouTube::GetMaxFileSizeMB(DWORD* pdwSizeMB) const
{
    if (!pdwSizeMB)
        return E_POINTER;
    *pdwSizeMB = 2048;
    return S_OK;
}

HRESULT PublishServiceYouTube::GetMaxDurationSec(DWORD* pdwDurationSec) const
{
    if (!pdwDurationSec)
        return E_POINTER;
    *pdwDurationSec = 43200;
    return S_OK;
}

HRESULT PublishServiceYouTube::ValidateFile(LPCWSTR pszFilePath) const
{
    if (!pszFilePath)
        return E_POINTER;

    DWORD dwAttr = GetFileAttributesW(pszFilePath);
    if (dwAttr == INVALID_FILE_ATTRIBUTES)
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExW(pszFilePath, GetFileExInfoStandard, &fad))
    {
        ULONGLONG cbFileSize = (static_cast<ULONGLONG>(fad.nFileSizeHigh) << 32) | fad.nFileSizeLow;
        if (cbFileSize > static_cast<ULONGLONG>(m_dwMaxFileSizeMB) * 1024 * 1024)
            return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
    }

    LPCWSTR pszExt = PathFindExtension(pszFilePath);
    if (pszExt)
    {
        if (_wcsicmp(pszExt, L".mp4") != 0 &&
            _wcsicmp(pszExt, L".wmv") != 0 &&
            _wcsicmp(pszExt, L".avi") != 0 &&
            _wcsicmp(pszExt, L".mov") != 0 &&
            _wcsicmp(pszExt, L".flv") != 0 &&
            _wcsicmp(pszExt, L".webm") != 0)
        {
            return E_INVALIDARG;
        }
    }

    return S_OK;
}

void PublishServiceYouTube::SetDeveloperKey(LPCWSTR pszKey)
{
    m_strDeveloperKey = pszKey ? pszKey : L"";
}

ATL::CString PublishServiceYouTube::GetDeveloperKey() const
{
    return m_strDeveloperKey;
}

void PublishServiceYouTube::SetCategoryId(LPCWSTR pszCategoryId)
{
    m_strCategoryId = pszCategoryId ? pszCategoryId : L"";
}

ATL::CString PublishServiceYouTube::GetCategoryId() const
{
    return m_strCategoryId;
}

// ============================================================================
// PublishServiceSkyDrive implementation
// ============================================================================

PublishServiceSkyDrive::PublishServiceSkyDrive()
    : m_bOverwriteExisting(false)
{
    m_strServiceName = L"SkyDrive";
    m_strFolderName = L"Movies";
}

PublishServiceSkyDrive::~PublishServiceSkyDrive()
{
}

LPCWSTR PublishServiceSkyDrive::GetServiceName() const
{
    return m_strServiceName;
}

PublishServiceType PublishServiceSkyDrive::GetServiceType() const
{
    return PublishServiceTypeSkyDrive;
}

HRESULT PublishServiceSkyDrive::Authenticate(LPCWSTR pszToken, LPCWSTR pszSecret)
{
    if (!pszToken)
        return E_POINTER;

    if (wcslen(pszToken) == 0)
        return E_INVALIDARG;

    UNREFERENCED_PARAMETER(pszSecret);

    m_bAuthenticated = true;
    return S_OK;
}

HRESULT PublishServiceSkyDrive::RefreshAuthentication()
{
    return S_OK;
}

HRESULT PublishServiceSkyDrive::UploadFile(LPCWSTR pszFilePath, PublishJobProgress* pProgress)
{
    if (!pszFilePath)
        return E_POINTER;

    if (!m_bAuthenticated)
        return E_ACCESSDENIED;

    m_bUploading = true;

    if (pProgress)
    {
        pProgress->SetCurrentStage(L"Uploading to SkyDrive");
        pProgress->SetUploadProgress(0.0f);
    }

    // In the full implementation, this would:
    //  1. Create or locate the target folder
    //  2. Upload the file via the SkyDrive REST API
    //  3. Report progress via pProgress

    if (pProgress)
    {
        pProgress->SetUploadProgress(1.0f);
        pProgress->SetOverallProgress(1.0f);
    }

    m_bUploading = false;
    return S_OK;
}

HRESULT PublishServiceSkyDrive::CancelUpload()
{
    m_bUploading = false;
    return S_OK;
}

HRESULT PublishServiceSkyDrive::GetMaxVideoDimensions(UINT* puWidth, UINT* puHeight) const
{
    if (!puWidth || !puHeight)
        return E_POINTER;
    *puWidth = 1920;
    *puHeight = 1080;
    return S_OK;
}

HRESULT PublishServiceSkyDrive::GetMaxFileSizeMB(DWORD* pdwSizeMB) const
{
    if (!pdwSizeMB)
        return E_POINTER;
    *pdwSizeMB = 4096;
    return S_OK;
}

HRESULT PublishServiceSkyDrive::GetMaxDurationSec(DWORD* pdwDurationSec) const
{
    if (!pdwDurationSec)
        return E_POINTER;
    *pdwDurationSec = 7200;
    return S_OK;
}

HRESULT PublishServiceSkyDrive::ValidateFile(LPCWSTR pszFilePath) const
{
    if (!pszFilePath)
        return E_POINTER;

    return S_OK;
}

void PublishServiceSkyDrive::SetFolderName(LPCWSTR pszFolder)
{
    m_strFolderName = pszFolder ? pszFolder : L"Movies";
}

ATL::CString PublishServiceSkyDrive::GetFolderName() const
{
    return m_strFolderName;
}

void PublishServiceSkyDrive::SetOverwriteExisting(bool bOverwrite)
{
    m_bOverwriteExisting = bOverwrite;
}

bool PublishServiceSkyDrive::GetOverwriteExisting() const throw()
{
    return m_bOverwriteExisting;
}
