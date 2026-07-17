#pragma once
#ifndef SUNDANCE_EXPORT_CONTROLLER_H
#define SUNDANCE_EXPORT_CONTROLLER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class ExportController
{
public:
    ExportController();
    ~ExportController();

    HRESULT Publish(LPCWSTR pszOutputPath, DWORD dwProfileIndex);
    HRESULT PublishToService(LPCWSTR pszServiceName);

    HRESULT SetExportSettings(LPCWSTR pszFormat, DWORD dwQuality, DWORD dwWidth, DWORD dwHeight);
    HRESULT StartExport();
    HRESULT CancelExport();
    float GetExportProgress() const throw();
    bool IsExporting() const throw();

private:
    ExportController(const ExportController&);
    ExportController& operator=(const ExportController&);

    ATL::CString m_strFormat;
    DWORD        m_dwQuality;
    DWORD        m_dwWidth;
    DWORD        m_dwHeight;
    bool         m_bExporting;
    float        m_flProgress;
};

#endif
