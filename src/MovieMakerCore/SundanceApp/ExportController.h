#pragma once
#ifndef SUNDANCE_EXPORT_CONTROLLER_H
#define SUNDANCE_EXPORT_CONTROLLER_H

#include "../pch.h"
#include "../MovieMakerCore.h"
#include "../Publishing/PublishJob.h"

typedef void (*ExportCompleteCallback)(HRESULT hrResult, LPCWSTR pszMessage, void* pUserData);

class ExportController;

class ExportProgressHandler : public PublishProgressCallBack
{
public:
    ExportProgressHandler();
    virtual ~ExportProgressHandler();

    void SetController(ExportController* pController);

    virtual void OnProgressChanged(float flOverallProgress, LPCWSTR pszStatusText) override;
    virtual void OnStageChanged(DWORD dwStage, LPCWSTR pszStageName) override;
    virtual void OnCompleted(HRESULT hrResult, LPCWSTR pszMessage) override;
    virtual bool CheckCancelled() override;

private:
    ExportController* m_pController;
};

class ExportController
{
    friend class ExportProgressHandler;

public:
    ExportController();
    ~ExportController();

    HRESULT Export(HWND hWndOwner);
    HRESULT PollExport();

    void SetCompletionCallback(ExportCompleteCallback pCallback, void* pUserData);

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

    PublishBackgroundJob* m_pBackgroundJob;
    ExportProgressHandler m_progressHandler;

    ExportCompleteCallback m_pCompletionCallback;
    void*                 m_pCallbackUserData;

    ATL::CString m_strCurrentStage;
    DWORD        m_dwCurrentStage;

    ATL::CString m_strLastResultMessage;
    HRESULT      m_hrLastResult;
};

#endif
