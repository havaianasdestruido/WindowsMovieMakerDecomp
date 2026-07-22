#include "pch.h"

/*
 * PublishDialogs.cpp
 *
 * Implementation of PublishSummaryDialog.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "PublishDialogs.h"

// ============================================================================
// PublishSummaryDialog implementation
// ============================================================================

PublishSummaryDialog::PublishSummaryDialog()
{
}

PublishSummaryDialog::~PublishSummaryDialog()
{
}

void PublishSummaryDialog::SetSummaryResult(const PublishSummaryResult& result)
{
    m_summaryResult = result;
}

const PublishSummaryResult& PublishSummaryDialog::GetSummaryResult() const throw()
{
    return m_summaryResult;
}

void PublishSummaryDialog::AddJobResult(DWORD dwJobId, HRESULT hrResult, LPCWSTR pszTitle, LPCWSTR pszUrl)
{
    JobResultEntry entry;
    entry.dwJobId = dwJobId;
    entry.hrResult = hrResult;
    entry.strTitle = pszTitle ? pszTitle : L"";
    entry.strUrl = pszUrl ? pszUrl : L"";
    m_jobResults.push_back(entry);
}

size_t PublishSummaryDialog::GetJobResultCount() const throw()
{
    return m_jobResults.size();
}

INT_PTR PublishSummaryDialog::ShowModal(HWND hWndParent)
{
    TASKDIALOGCONFIG config = {};
    config.cbSize = sizeof(config);
    config.hwndParent = hWndParent;
    config.dwCommonButtons = TDCBF_OK_BUTTON;
    config.pszWindowTitle = L"Publish Summary";
    config.pszMainInstruction = L"Your movie has been published";

    DWORD dwSuccess = 0;
    DWORD dwFailed = 0;
    DWORD dwCancelled = 0;
    for (const auto& result : m_jobResults)
    {
        if (SUCCEEDED(result.hrResult))
            dwSuccess++;
        else if (result.hrResult == E_ABORT)
            dwCancelled++;
        else
            dwFailed++;
    }

    m_summaryResult.dwTotalJobs = static_cast<DWORD>(m_jobResults.size());
    m_summaryResult.dwSuccessfulJobs = dwSuccess;
    m_summaryResult.dwFailedJobs = dwFailed;
    m_summaryResult.dwCancelledJobs = dwCancelled;

    ATL::CString strContent;
    strContent.Format(L"Successful: %u  Failed: %u  Cancelled: %u\nTotal uploaded: %s",
                      dwSuccess, dwFailed, dwCancelled,
                      FormatFileSize(m_summaryResult.llTotalBytesUploaded));

    if (!m_jobResults.empty())
    {
        strContent += L"\n\n";
        for (const auto& result : m_jobResults)
        {
            ATL::CString strEntry;
            if (SUCCEEDED(result.hrResult))
            {
                if (!result.strUrl.IsEmpty())
                    strEntry.Format(L"[OK] %s - %s\n", result.strTitle.GetString(), result.strUrl.GetString());
                else
                    strEntry.Format(L"[OK] %s\n", result.strTitle.GetString());
            }
            else if (result.hrResult == E_ABORT)
            {
                strEntry.Format(L"[Cancelled] %s\n", result.strTitle.GetString());
            }
            else
            {
                strEntry.Format(L"[Failed] %s (0x%08X)\n", result.strTitle.GetString(), result.hrResult);
            }
            strContent += strEntry;
        }
    }

    config.pszContent = strContent;

    int nButton = 0;
    TaskDialogIndirect(&config, &nButton, nullptr, nullptr);

    return nButton;
}

DWORD PublishSummaryDialog::GetSuccessfulCount() const throw()
{
    return m_summaryResult.dwSuccessfulJobs;
}

DWORD PublishSummaryDialog::GetFailedCount() const throw()
{
    return m_summaryResult.dwFailedJobs;
}

DWORD PublishSummaryDialog::GetCancelledCount() const throw()
{
    return m_summaryResult.dwCancelledJobs;
}

ATL::CString PublishSummaryDialog::FormatFileSize(LONGLONG llBytes)
{
    const LONGLONG KB = 1024;
    const LONGLONG MB = KB * 1024;
    const LONGLONG GB = MB * 1024;

    ATL::CString strResult;

    if (llBytes >= GB)
        strResult.Format(L"%.2f GB", static_cast<double>(llBytes) / GB);
    else if (llBytes >= MB)
        strResult.Format(L"%.2f MB", static_cast<double>(llBytes) / MB);
    else if (llBytes >= KB)
        strResult.Format(L"%.2f KB", static_cast<double>(llBytes) / KB);
    else
        strResult.Format(L"%I64u bytes", llBytes);

    return strResult;
}

ATL::CString PublishSummaryDialog::FormatDuration(LONGLONG llMs)
{
    LONGLONG seconds = llMs / 1000;
    LONGLONG minutes = seconds / 60;
    LONGLONG hours = minutes / 60;

    seconds %= 60;
    minutes %= 60;

    ATL::CString strResult;
    if (hours > 0)
        strResult.Format(L"%I64u:%02I64u:%02I64u", hours, minutes, seconds);
    else
        strResult.Format(L"%I64u:%02I64u", minutes, seconds);

    return strResult;
}
