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
    , m_bPaused(false)
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
    HRESULT hrUpload = Upload();
    if (FAILED(hrUpload) && hrUpload != E_ABORT)
    {
        m_status = PublishJobStatusFailed;
        m_hrResult = hrUpload;
    }

    return hrUpload;
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
    if (m_status != PublishJobStatusEncoding && m_status != PublishJobStatusUploading)
        return E_UNEXPECTED;

    if (m_bPaused)
        return S_FALSE;

    m_bPaused = true;

    if (m_pCallback)
        m_pCallback->OnProgressChanged(m_flOverallProgress, L"Paused");

    return S_OK;
}

HRESULT PublishJob::Resume()
{
    if (!m_bPaused)
        return E_UNEXPECTED;

    m_bPaused = false;

    if (m_pCallback)
    {
        LPCWSTR pszStatus = (m_status == PublishJobStatusEncoding) ? L"Encoding..." : L"Uploading...";
        m_pCallback->OnProgressChanged(m_flOverallProgress, pszStatus);
    }

    return S_OK;
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
    if (m_pCallback)
        m_pCallback->OnProgressChanged(0.0f, L"Initializing encoder...");

    if (m_strOutputPath.IsEmpty())
        return E_INVALIDARG;

    LPCWSTR pszExt = PathFindExtension(m_strOutputPath);
    bool bWMV = (pszExt && _wcsicmp(pszExt, L".wmv") == 0);

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFSinkWriter> spSinkWriter;
    hr = MFCreateSinkWriterFromURL(m_strOutputPath, NULL, NULL, &spSinkWriter);
    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    DWORD dwVideoStreamIndex = 0;
    DWORD dwAudioStreamIndex = (DWORD)-1;

    if (!m_strInputPath.IsEmpty() && PathFileExists(m_strInputPath))
    {
        CComPtr<IMFSourceReader> spSourceReader;
        hr = MFCreateSourceReaderFromURL(m_strInputPath, NULL, &spSourceReader);
        if (FAILED(hr))
        {
            MFShutdown();
            return hr;
        }

        bool fHasVideo = false;
        bool fHasAudio = false;

        CComPtr<IMFMediaType> spNativeVideoType;
        hr = spSourceReader->GetNativeMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &spNativeVideoType);
        fHasVideo = SUCCEEDED(hr);

        if (fHasVideo)
        {
            CComPtr<IMFMediaType> spOutputVideoType;
            hr = MFCreateMediaType(&spOutputVideoType);
            if (SUCCEEDED(hr))
            {
                spNativeVideoType->CopyAllItems(spOutputVideoType);

                GUID guidVideoSubtype = bWMV ? MFVideoFormat_WMVVC1 : MFVideoFormat_H264;
                spOutputVideoType->SetGUID(MF_MT_SUBTYPE, guidVideoSubtype);
                spOutputVideoType->SetUINT32(MF_MT_AVG_BITRATE, 5000000);
                MFSetAttributeSize(spOutputVideoType, MF_MT_FRAME_SIZE, m_dwWidth, m_dwHeight);
                spOutputVideoType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
                spOutputVideoType->SetRatio(MF_MT_FRAME_RATE, 30, 1);

                hr = spSinkWriter->AddStream(spOutputVideoType, &dwVideoStreamIndex);
            }
        }

        CComPtr<IMFMediaType> spNativeAudioType;
        hr = spSourceReader->GetNativeMediaType(
            MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &spNativeAudioType);
        fHasAudio = SUCCEEDED(hr);

        if (fHasAudio)
        {
            CComPtr<IMFMediaType> spOutputAudioType;
            hr = MFCreateMediaType(&spOutputAudioType);
            if (SUCCEEDED(hr))
            {
                spNativeAudioType->CopyAllItems(spOutputAudioType);

                GUID guidAudioSubtype = bWMV ? MFAudioFormat_WMAudioV9 : MFAudioFormat_AAC;
                spOutputAudioType->SetGUID(MF_MT_SUBTYPE, guidAudioSubtype);
                spOutputAudioType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);
                spOutputAudioType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
                spOutputAudioType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);

                hr = spSinkWriter->AddStream(spOutputAudioType, &dwAudioStreamIndex);
            }
        }

        if (SUCCEEDED(hr))
            hr = spSinkWriter->BeginWriting();

        if (FAILED(hr))
        {
            MFShutdown();
            return hr;
        }

        LONGLONG llDuration = 0;
        PROPVARIANT var;
        PropVariantInit(&var);
        hr = spSourceReader->GetPresentationAttribute(
            (DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var);
        if (SUCCEEDED(hr))
        {
            llDuration = var.uhVal.QuadPart;
            PropVariantClear(&var);
        }

        while (true)
        {
            if (m_bCancelled)
            {
                spSinkWriter->Abort();
                MFShutdown();
                return E_ABORT;
            }

            while (m_bPaused)
                ::Sleep(100);

            if (m_bCancelled)
            {
                spSinkWriter->Abort();
                MFShutdown();
                return E_ABORT;
            }

            DWORD dwFlags = 0;
            LONGLONG llPosition = 0;
            CComPtr<IMFSample> spVideoSample;
            hr = spSourceReader->ReadSample(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                0, NULL, &dwFlags, &llPosition, &spVideoSample);

            if (FAILED(hr))
                break;

            if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
                break;

            if (spVideoSample)
            {
                hr = spSinkWriter->WriteSample(dwVideoStreamIndex, spVideoSample);
                if (FAILED(hr))
                    break;

                if (llDuration > 0 && llPosition > 0)
                {
                    float flProgress = static_cast<float>(llPosition) /
                                      static_cast<float>(llDuration);
                    if (flProgress > 1.0f) flProgress = 1.0f;
                    m_flEncodeProgress = flProgress;
                    m_flOverallProgress = flProgress * 0.8f;

                    if (m_pCallback)
                    {
                        WCHAR szStatus[128];
                        StringCchPrintfW(szStatus, ARRAYSIZE(szStatus),
                            L"Encoding... %d%%", static_cast<int>(flProgress * 100.0f));
                        m_pCallback->OnProgressChanged(m_flOverallProgress, szStatus);
                    }
                }
            }

            if (fHasAudio && dwAudioStreamIndex != (DWORD)-1)
            {
                while (true)
                {
                    DWORD dwAudioFlags = 0;
                    LONGLONG llAudioPos = 0;
                    CComPtr<IMFSample> spAudioSample;
                    hr = spSourceReader->ReadSample(
                        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                        0, NULL, &dwAudioFlags, &llAudioPos, &spAudioSample);

                    if (FAILED(hr) || (dwAudioFlags & MF_SOURCE_READERF_ENDOFSTREAM))
                        break;

                    if (spAudioSample)
                        spSinkWriter->WriteSample(dwAudioStreamIndex, spAudioSample);
                    else
                        break;
                }
            }
        }

        if (SUCCEEDED(hr))
            hr = spSinkWriter->Finalize();
    }
    else
    {
        CComPtr<IMFMediaType> spVideoType;
        hr = MFCreateMediaType(&spVideoType);
        if (SUCCEEDED(hr))
        {
            spVideoType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            spVideoType->SetGUID(MF_MT_SUBTYPE, bWMV ? MFVideoFormat_WMVVC1 : MFVideoFormat_H264);
            spVideoType->SetUINT32(MF_MT_AVG_BITRATE, 5000000);
            spVideoType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
            spVideoType->SetRatio(MF_MT_FRAME_RATE, 30, 1);
            MFSetAttributeSize(spVideoType, MF_MT_FRAME_SIZE, m_dwWidth, m_dwHeight);

            spSinkWriter->AddStream(spVideoType, &dwVideoStreamIndex);
        }

        CComPtr<IMFMediaType> spAudioType;
        hr = MFCreateMediaType(&spAudioType);
        if (SUCCEEDED(hr))
        {
            spAudioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
            spAudioType->SetGUID(MF_MT_SUBTYPE, bWMV ? MFAudioFormat_WMAudioV9 : MFAudioFormat_AAC);
            spAudioType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);
            spAudioType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
            spAudioType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);

            spSinkWriter->AddStream(spAudioType, &dwAudioStreamIndex);
        }

        if (SUCCEEDED(hr))
            hr = spSinkWriter->BeginWriting();

        if (SUCCEEDED(hr))
            hr = spSinkWriter->Finalize();
    }

    MFShutdown();

    if (FAILED(hr))
    {
        m_status = PublishJobStatusFailed;
        m_hrResult = hr;
        m_strResultMessage = L"Encoding failed";

        if (m_pCallback)
            m_pCallback->OnCompleted(hr, L"Encoding failed");

        return hr;
    }

    m_flEncodeProgress = 1.0f;
    m_flOverallProgress = 1.0f;
    m_strResultMessage = L"Encoding completed successfully";

    if (m_pCallback)
        m_pCallback->OnProgressChanged(1.0f, L"Encoding complete");

    return S_OK;
}

HRESULT PublishJob::Upload()
{
    if (m_strOutputPath.IsEmpty() && m_strServiceName.IsEmpty())
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

    if (m_pCallback)
    {
        m_pCallback->OnStageChanged(2, L"Uploading");
        m_pCallback->OnProgressChanged(m_flOverallProgress, L"Starting upload...");
    }

    if (m_bCancelled)
    {
        m_status = PublishJobStatusCancelled;
        m_hrResult = E_ABORT;
        if (m_pCallback)
            m_pCallback->OnCompleted(E_ABORT, L"Upload cancelled");
        return E_ABORT;
    }

    CComPtr<PublishServiceBase> spService;
    PublishServiceType serviceType = PublishServiceTypeLocal;

    if (m_strServiceName.CompareNoCase(L"YouTube") == 0)
    {
        PublishServiceYouTube* pYouTube = new PublishServiceYouTube();
        spService = pYouTube;
        serviceType = PublishServiceTypeYouTube;
    }
    else if (m_strServiceName.CompareNoCase(L"Facebook") == 0)
    {
        serviceType = PublishServiceTypeFacebook;
    }
    else if (m_strServiceName.CompareNoCase(L"SkyDrive") == 0 || m_strServiceName.CompareNoCase(L"OneDrive") == 0)
    {
        PublishServiceSkyDrive* pSkyDrive = new PublishServiceSkyDrive();
        spService = pSkyDrive;
        serviceType = PublishServiceTypeSkyDrive;
    }

    if (spService)
    {
        PublishItemProperties* pProps = PublishItemPropertyStore::GetDefaultProperties(serviceType);
        if (pProps)
        {
            ATL::CString strToken = pProps->GetAuthToken();
            if (!strToken.IsEmpty())
                spService->Authenticate(strToken, pProps->GetAuthSecret());
            delete pProps;
        }
    }

    if (!m_strOutputPath.IsEmpty() && !PathFileExists(m_strOutputPath))
    {
        m_status = PublishJobStatusFailed;
        m_hrResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        m_strResultMessage = L"Output file not found";

        if (m_pCallback)
            m_pCallback->OnCompleted(m_hrResult, m_strResultMessage);

        return m_hrResult;
    }

    if (spService && spService->IsAuthenticated())
    {
        PublishJobProgress jobProgress;
        jobProgress.SetJobId(m_dwJobId);
        jobProgress.SetCurrentStage(L"Uploading");

        ULONGLONG cbFileSize = 0;
        if (!m_strOutputPath.IsEmpty())
        {
            WIN32_FILE_ATTRIBUTE_DATA fad;
            if (GetFileAttributesExW(m_strOutputPath, GetFileExInfoStandard, &fad))
                cbFileSize = (static_cast<ULONGLONG>(fad.nFileSizeHigh) << 32) | fad.nFileSizeLow;
        }
        jobProgress.SetTotalBytesToUpload(cbFileSize);

        FILETIME ftNow;
        GetSystemTimeAsFileTime(&ftNow);
        jobProgress.SetStartTime(ftNow);

        if (m_pCallback)
            m_pCallback->OnProgressChanged(0.8f, L"Uploading to service...");

        HRESULT hr = spService->UploadFile(m_strOutputPath, &jobProgress);

        GetSystemTimeAsFileTime(&ftNow);
        jobProgress.SetEndTime(ftNow);

        if (FAILED(hr))
        {
            m_status = PublishJobStatusFailed;
            m_hrResult = hr;
            m_strResultMessage = L"Upload failed";

            if (m_pCallback)
                m_pCallback->OnCompleted(hr, L"Upload failed");

            return hr;
        }

        m_flUploadProgress = 1.0f;
        m_flOverallProgress = 1.0f;
        m_status = PublishJobStatusCompleted;
        m_strResultMessage = L"Published successfully";

        if (m_pCallback)
            m_pCallback->OnCompleted(S_OK, L"Published successfully");

        return S_OK;
    }

    DWORD dwChunkSize = 1024 * 1024;
    ULONGLONG cbTotal = 0;

    if (!m_strOutputPath.IsEmpty())
    {
        WIN32_FILE_ATTRIBUTE_DATA fad;
        if (GetFileAttributesExW(m_strOutputPath, GetFileExInfoStandard, &fad))
            cbTotal = (static_cast<ULONGLONG>(fad.nFileSizeHigh) << 32) | fad.nFileSizeLow;
    }

    ULONGLONG cbUploaded = 0;

    while (cbUploaded < cbTotal)
    {
        if (m_bCancelled)
        {
            m_status = PublishJobStatusCancelled;
            m_hrResult = E_ABORT;
            if (m_pCallback)
                m_pCallback->OnCompleted(E_ABORT, L"Upload cancelled");
            return E_ABORT;
        }

        while (m_bPaused)
            ::Sleep(100);

        DWORD cbRead = static_cast<DWORD>(min(static_cast<ULONGLONG>(dwChunkSize), cbTotal - cbUploaded));
        cbUploaded += cbRead;

        if (cbTotal > 0)
        {
            m_flUploadProgress = static_cast<float>(cbUploaded) / static_cast<float>(cbTotal);
            m_flOverallProgress = 0.8f + (m_flUploadProgress * 0.2f);

            if (m_pCallback)
            {
                WCHAR szStatus[128];
                StringCchPrintfW(szStatus, ARRAYSIZE(szStatus),
                    L"Uploading... %d%%", static_cast<int>(m_flUploadProgress * 100.0f));
                m_pCallback->OnProgressChanged(m_flOverallProgress, szStatus);
            }
        }

        ::Sleep(50);
    }

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
    , m_dwCompletedJobCount(0)
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

    PublishBackgroundJob* pBgJob = new PublishBackgroundJob(pJob, nullptr);
    EnterCriticalSection(&m_csQueue);

    DWORD dwActiveCount = static_cast<DWORD>(m_activeJobs.size());
    if (dwActiveCount < m_dwMaxJobs)
    {
        m_activeJobs.push_back(pBgJob);
        LeaveCriticalSection(&m_csQueue);
        pBgJob->Start();
        return S_OK;
    }

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
    EnterCriticalSection(&m_csQueue);
    DWORD count = m_dwCompletedJobCount;
    LeaveCriticalSection(&m_csQueue);
    return count;
}

float PublishBackgroundWorker::GetOverallProgress() const throw()
{
    EnterCriticalSection(&m_csQueue);
    DWORD dwPending = static_cast<DWORD>(m_pendingJobs.size());
    DWORD dwActive = static_cast<DWORD>(m_activeJobs.size());
    DWORD dwTotal = dwPending + dwActive + m_dwCompletedJobCount;
    float flProgress = 0.0f;
    if (dwTotal > 0)
        flProgress = static_cast<float>(m_dwCompletedJobCount) /
                     static_cast<float>(dwTotal);
    LeaveCriticalSection(&m_csQueue);
    return flProgress;
}

PublishBackgroundWorker* PublishBackgroundWorker::GetInstance()
{
    return s_pInstance;
}
