/*
 * PublishDialogs.h
 *
 * PublishSummaryDialog: Post-publish summary dialog (success/failure counts).
 *
 * RTTI: ?AVPublishSummaryDialog@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef PUBLISH_DIALOGS_H
#define PUBLISH_DIALOGS_H

#include "../pch.h"

// ============================================================================
// PublishSummaryResult
// ============================================================================
// Contains the summary data for a completed publish session.
//
struct PublishSummaryResult
{
    DWORD  dwTotalJobs;
    DWORD  dwSuccessfulJobs;
    DWORD  dwFailedJobs;
    DWORD  dwCancelledJobs;
    LONGLONG llTotalBytesUploaded;
    LONGLONG llTotalTimeMs;

    PublishSummaryResult()
        : dwTotalJobs(0)
        , dwSuccessfulJobs(0)
        , dwFailedJobs(0)
        , dwCancelledJobs(0)
        , llTotalBytesUploaded(0)
        , llTotalTimeMs(0)
    {
    }
};

// ============================================================================
// PublishSummaryDialog
// ============================================================================
// Modal dialog displayed after a publish session completes. Shows a summary
// of successful and failed publish jobs with links to published content.
// Built using the task dialog API for consistent Windows 8 appearance.
//
class PublishSummaryDialog
{
public:
    PublishSummaryDialog();
    ~PublishSummaryDialog();

    // -- Configuration --
    void SetSummaryResult(const PublishSummaryResult& result);
    const PublishSummaryResult& GetSummaryResult() const throw();

    void AddJobResult(DWORD dwJobId, HRESULT hrResult, LPCWSTR pszTitle, LPCWSTR pszUrl);
    size_t GetJobResultCount() const throw();

    // -- Display --
    INT_PTR ShowModal(HWND hWndParent);

    // -- Results --
    DWORD GetSuccessfulCount() const throw();
    DWORD GetFailedCount() const throw();
    DWORD GetCancelledCount() const throw();

    // -- Helpers --
    static ATL::CString FormatFileSize(LONGLONG llBytes);
    static ATL::CString FormatDuration(LONGLONG llMs);

private:
    struct JobResultEntry
    {
        DWORD       dwJobId;
        HRESULT     hrResult;
        ATL::CString strTitle;
        ATL::CString strUrl;
    };

    PublishSummaryResult           m_summaryResult;
    std::vector<JobResultEntry>    m_jobResults;
};

#endif // PUBLISH_DIALOGS_H
