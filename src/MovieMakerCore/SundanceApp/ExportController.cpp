#include "pch.h"
#include "ExportController.h"
#include "../Publishing/PublishClasses.h"

// ============================================================================
// ExportProgressHandler implementation
// ============================================================================

ExportProgressHandler::ExportProgressHandler()
    : m_pController(nullptr)
{
}

ExportProgressHandler::~ExportProgressHandler()
{
}

void ExportProgressHandler::SetController(ExportController* pController)
{
    m_pController = pController;
}

void ExportProgressHandler::OnProgressChanged(float flOverallProgress, LPCWSTR pszStatusText)
{
    if (m_pController)
        m_pController->m_flProgress = flOverallProgress;
}

void ExportProgressHandler::OnStageChanged(DWORD dwStage, LPCWSTR pszStageName)
{
    if (m_pController)
    {
        m_pController->m_dwCurrentStage = dwStage;
        m_pController->m_strCurrentStage = pszStageName ? pszStageName : L"";
    }
}

void ExportProgressHandler::OnCompleted(HRESULT hrResult, LPCWSTR pszMessage)
{
    if (m_pController)
    {
        m_pController->m_hrLastResult = hrResult;
        m_pController->m_strLastResultMessage = pszMessage ? pszMessage : L"";
        m_pController->m_flProgress = (SUCCEEDED(hrResult)) ? 1.0f : 0.0f;
    }
}

bool ExportProgressHandler::CheckCancelled()
{
    if (m_pController)
        return !m_pController->m_bExporting;
    return false;
}

// ============================================================================
// ExportController implementation
// ============================================================================

ExportController::ExportController()
    : m_dwQuality(80)
    , m_dwWidth(1920)
    , m_dwHeight(1080)
    , m_bExporting(false)
    , m_flProgress(0.0f)
    , m_pBackgroundJob(nullptr)
    , m_pCompletionCallback(nullptr)
    , m_pCallbackUserData(nullptr)
    , m_dwCurrentStage(0)
    , m_hrLastResult(S_OK)
{
    m_progressHandler.SetController(this);
}

ExportController::~ExportController()
{
    CancelExport();
    delete m_pBackgroundJob;
}

HRESULT ExportController::Export(HWND hWndOwner)
{
    if (m_bExporting)
        return E_UNEXPECTED;

    TCHAR szFile[MAX_PATH] = L"MyMovie.mp4";

    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = hWndOwner;
    ofn.lpstrFilter  = L"MP4 Video (*.mp4)\0*.mp4\0WMV Video (*.wmv)\0*.wmv\0\0";
    ofn.lpstrFile    = szFile;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrTitle   = L"Export Movie";
    ofn.Flags        = OFN_OVERWRITEPROMPT | OFN_EXPLORER;
    ofn.lpstrDefExt  = L"mp4";

    if (!::GetSaveFileName(&ofn))
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);

    LPCWSTR pszExt = PathFindExtension(szFile);
    if (pszExt && _wcsicmp(pszExt, L".wmv") == 0)
        m_strFormat = L"wmv";
    else
        m_strFormat = L"mp4";

    PublishJob* pJob = new PublishJob();
    pJob->SetJobId(1);
    pJob->SetJobName(L"Export");
    pJob->SetOutputPath(szFile);

    m_pBackgroundJob = new PublishBackgroundJob(pJob, &m_progressHandler);

    m_bExporting = true;
    m_flProgress = 0.0f;
    m_hrLastResult = S_OK;
    m_strLastResultMessage.Empty();

    m_pBackgroundJob->Start();

    return S_OK;
}

HRESULT ExportController::PollExport()
{
    if (!m_bExporting || !m_pBackgroundJob)
        return S_FALSE;

    if (!m_pBackgroundJob->IsRunning())
    {
        PublishJob* pJob = m_pBackgroundJob->GetJob();
        if (pJob)
        {
            m_hrLastResult = pJob->GetResult();
            m_strLastResultMessage = pJob->GetResultMessage();
            m_flProgress = pJob->GetOverallProgress();
        }

        m_bExporting = false;

        delete m_pBackgroundJob;
        m_pBackgroundJob = nullptr;

        if (m_pCompletionCallback)
            m_pCompletionCallback(m_hrLastResult, m_strLastResultMessage, m_pCallbackUserData);
    }
    else
    {
        PublishJob* pJob = m_pBackgroundJob->GetJob();
        if (pJob)
            m_flProgress = pJob->GetOverallProgress();
    }

    return S_OK;
}

void ExportController::SetCompletionCallback(ExportCompleteCallback pCallback, void* pUserData)
{
    m_pCompletionCallback = pCallback;
    m_pCallbackUserData = pUserData;
}

HRESULT ExportController::Publish(LPCWSTR pszOutputPath, DWORD dwProfileIndex)
{
    if (!pszOutputPath || !pszOutputPath[0])
        return E_INVALIDARG;

    if (m_bExporting)
        return E_UNEXPECTED;

    PublishJob* pJob = new PublishJob();
    if (!pJob)
        return E_OUTOFMEMORY;

    pJob->SetJobName(L"Publish");
    pJob->SetOutputPath(pszOutputPath);
    pJob->SetProfileIndex(dwProfileIndex);

    delete m_pBackgroundJob;
    m_pBackgroundJob = new PublishBackgroundJob(pJob, &m_progressHandler);
    if (!m_pBackgroundJob)
    {
        delete pJob;
        return E_OUTOFMEMORY;
    }

    m_bExporting = true;
    m_flProgress = 0.0f;
    m_hrLastResult = S_OK;
    m_strLastResultMessage.Empty();

    HRESULT hr = m_pBackgroundJob->Start();
    if (FAILED(hr))
    {
        m_bExporting = false;
    }

    return hr;
}

HRESULT ExportController::PublishToService(LPCWSTR pszServiceName)
{
    if (!pszServiceName || !pszServiceName[0])
        return E_INVALIDARG;

    if (m_bExporting)
        return E_UNEXPECTED;

    PublishJob* pJob = new PublishJob();
    if (!pJob)
        return E_OUTOFMEMORY;

    pJob->SetJobName(pszServiceName);
    pJob->SetServiceName(pszServiceName);

    delete m_pBackgroundJob;
    m_pBackgroundJob = new PublishBackgroundJob(pJob, &m_progressHandler);
    if (!m_pBackgroundJob)
    {
        delete pJob;
        return E_OUTOFMEMORY;
    }

    m_bExporting = true;
    m_flProgress = 0.0f;
    m_hrLastResult = S_OK;
    m_strLastResultMessage.Empty();

    HRESULT hr = m_pBackgroundJob->Start();
    if (FAILED(hr))
    {
        m_bExporting = false;
    }

    return hr;
}

HRESULT ExportController::SetExportSettings(LPCWSTR pszFormat, DWORD dwQuality, DWORD dwWidth, DWORD dwHeight)
{
    if (!pszFormat || !pszFormat[0])
        return E_INVALIDARG;
    if (dwQuality > 100)
        return E_INVALIDARG;
    if (dwWidth == 0 || dwHeight == 0)
        return E_INVALIDARG;

    m_strFormat = pszFormat;
    m_dwQuality = dwQuality;
    m_dwWidth = dwWidth;
    m_dwHeight = dwHeight;
    return S_OK;
}

HRESULT ExportController::StartExport()
{
    if (m_bExporting)
        return E_UNEXPECTED;

    PublishJob* pJob = new PublishJob();
    if (!pJob)
        return E_OUTOFMEMORY;

    pJob->SetJobName(L"Export");
    pJob->SetOutputPath(L"");

    delete m_pBackgroundJob;
    m_pBackgroundJob = new PublishBackgroundJob(pJob, &m_progressHandler);
    if (!m_pBackgroundJob)
    {
        delete pJob;
        return E_OUTOFMEMORY;
    }

    m_bExporting = true;
    m_flProgress = 0.0f;
    m_hrLastResult = S_OK;
    m_strLastResultMessage.Empty();

    HRESULT hr = m_pBackgroundJob->Start();
    if (FAILED(hr))
    {
        m_bExporting = false;
    }

    return hr;
}

HRESULT ExportController::CancelExport()
{
    if (!m_bExporting)
        return S_FALSE;

    if (m_pBackgroundJob)
        m_pBackgroundJob->Cancel();

    m_bExporting = false;
    m_flProgress = 0.0f;
    return S_OK;
}

float ExportController::GetExportProgress() const throw()
{
    return m_flProgress;
}

bool ExportController::IsExporting() const throw()
{
    return m_bExporting;
}
