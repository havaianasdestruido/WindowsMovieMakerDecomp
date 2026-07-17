#include "pch.h"
#include "ExportController.h"

ExportController::ExportController()
    : m_dwQuality(80)
    , m_dwWidth(1920)
    , m_dwHeight(1080)
    , m_bExporting(false)
    , m_flProgress(0.0f)
{
}

ExportController::~ExportController() {}

HRESULT ExportController::Publish(LPCWSTR pszOutputPath, DWORD dwProfileIndex)
{
    if (!pszOutputPath || !pszOutputPath[0])
        return E_INVALIDARG;

    m_bExporting = true;
    m_flProgress = 0.0f;
    m_bExporting = false;
    m_flProgress = 1.0f;
    return S_OK;
}

HRESULT ExportController::PublishToService(LPCWSTR pszServiceName)
{
    if (!pszServiceName || !pszServiceName[0])
        return E_INVALIDARG;

    return S_OK;
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

    m_bExporting = true;
    m_flProgress = 0.0f;
    return S_OK;
}

HRESULT ExportController::CancelExport()
{
    if (!m_bExporting)
        return S_FALSE;

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
