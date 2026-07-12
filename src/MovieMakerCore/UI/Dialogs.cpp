/*
 * Dialogs.cpp
 *
 * Implementation of dialog behavior classes.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "Dialogs.h"
#include "../SundanceApp/SundanceAppMain.h"

namespace Sundance
{

// ============================================================================
// RenderSummaryDialog
// ============================================================================
RenderSummaryDialog::RenderSummaryDialog()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_result(DialogResultNone)
    , m_dDuration(0.0)
    , m_cbFileSize(0)
    , m_dwRenderTimeSeconds(0)
{
}

RenderSummaryDialog::~RenderSummaryDialog() { m_pElement = NULL; }

HRESULT RenderSummaryDialog::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    UpdateSummaryDisplay();
    return S_OK;
}

HRESULT RenderSummaryDialog::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT RenderSummaryDialog::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;
    if (uMsg == WM_COMMAND)
    {
        WORD wCmd = LOWORD(wParam);
        if (wCmd == IDOK) { m_result = DialogResultPublish; if (pbHandled) *pbHandled = TRUE; }
        else if (wCmd == IDCANCEL) { m_result = DialogResultCancel; if (pbHandled) *pbHandled = TRUE; }
        else if (wCmd == IDRETRY) { m_result = DialogResultBack; if (pbHandled) *pbHandled = TRUE; }
    }
    return S_OK;
}

void RenderSummaryDialog::SetOutputFilePath(LPCWSTR pszPath) { m_strOutputPath = pszPath ? pszPath : L""; }
void RenderSummaryDialog::SetOutputDuration(double dDurationSeconds) { m_dDuration = dDurationSeconds; }
void RenderSummaryDialog::SetEstimatedFileSize(ULONGLONG cbSize) { m_cbFileSize = cbSize; }
void RenderSummaryDialog::SetEstimatedRenderTime(DWORD dwTimeSeconds) { m_dwRenderTimeSeconds = dwTimeSeconds; }
void RenderSummaryDialog::SetOutputFormat(LPCWSTR pszFormat) { m_strFormat = pszFormat ? pszFormat : L""; }
RenderSummaryDialog::DialogResult RenderSummaryDialog::GetResult() const throw() { return m_result; }

void RenderSummaryDialog::UpdateSummaryDisplay()
{
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
}

// ============================================================================
// UserEncodeProfileDialog
// ============================================================================
UserEncodeProfileDialog::UserEncodeProfileDialog()
    : m_pElement(NULL), m_pAppMain(NULL), m_dwSelectedProfile(0) {}

UserEncodeProfileDialog::~UserEncodeProfileDialog() { m_pElement = NULL; }

HRESULT UserEncodeProfileDialog::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT UserEncodeProfileDialog::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT UserEncodeProfileDialog::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    return S_OK;
}

void UserEncodeProfileDialog::SetSelectedProfile(DWORD dwProfileId) { m_dwSelectedProfile = dwProfileId; }
DWORD UserEncodeProfileDialog::GetSelectedProfile() const throw() { return m_dwSelectedProfile; }

// ============================================================================
// UserEncodeProfileRecommended
// ============================================================================
UserEncodeProfileRecommended::UserEncodeProfileRecommended() : m_pElement(NULL) {}
UserEncodeProfileRecommended::~UserEncodeProfileRecommended() { m_pElement = NULL; }

HRESULT UserEncodeProfileRecommended::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    RefreshProfiles();
    return S_OK;
}

HRESULT UserEncodeProfileRecommended::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

void UserEncodeProfileRecommended::RefreshProfiles() { m_profiles.clear(); }

// ============================================================================
// UserEncodeProfileWLVS
// ============================================================================
UserEncodeProfileWLVS::UserEncodeProfileWLVS() : m_pElement(NULL) {}
UserEncodeProfileWLVS::~UserEncodeProfileWLVS() { m_pElement = NULL; }

HRESULT UserEncodeProfileWLVS::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    RefreshProfiles();
    return S_OK;
}

HRESULT UserEncodeProfileWLVS::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

void UserEncodeProfileWLVS::RefreshProfiles() {}

// ============================================================================
// UserEncodeProfileInfo
// ============================================================================
UserEncodeProfileInfo::UserEncodeProfileInfo() : m_pElement(NULL), m_dwProfileId(0) { ZeroMemory(&m_details, sizeof(m_details)); }
UserEncodeProfileInfo::~UserEncodeProfileInfo() { m_pElement = NULL; }

HRESULT UserEncodeProfileInfo::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT UserEncodeProfileInfo::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

void UserEncodeProfileInfo::SetProfileId(DWORD dwProfileId) { m_dwProfileId = dwProfileId; LoadProfileDetails(); }
void UserEncodeProfileInfo::UpdateDisplay() { if (m_pElement) m_pElement->InvalidateRect(NULL); }
void UserEncodeProfileInfo::LoadProfileDetails() { ZeroMemory(&m_details, sizeof(m_details)); }

// ============================================================================
// MultipleEffectDialog
// ============================================================================
MultipleEffectDialog::MultipleEffectDialog()
    : m_pElement(NULL), m_pAppMain(NULL), m_dwTargetItemId(0) {}

MultipleEffectDialog::~MultipleEffectDialog() { m_pElement = NULL; }

HRESULT MultipleEffectDialog::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT MultipleEffectDialog::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT MultipleEffectDialog::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    return S_OK;
}

void MultipleEffectDialog::SetTargetItemId(DWORD dwItemId) { m_dwTargetItemId = dwItemId; }
void MultipleEffectDialog::SetAlreadyAppliedEffects(const std::vector<DWORD>& effectIds) { m_appliedEffects = effectIds; }

HRESULT MultipleEffectDialog::GetSelectedEffects(std::vector<DWORD>& effectIds)
{
    effectIds = m_selectedEffects;
    return S_OK;
}

// ============================================================================
// SundanceApplicationOptionsDialog
// ============================================================================
SundanceApplicationOptionsDialog::SundanceApplicationOptionsDialog()
    : m_pElement(NULL), m_pAppMain(NULL), m_dwDefaultPhotoDuration(7000), m_dwAutoSaveInterval(300000) {}

SundanceApplicationOptionsDialog::~SundanceApplicationOptionsDialog() { m_pElement = NULL; }

HRESULT SundanceApplicationOptionsDialog::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundanceApplicationOptionsDialog::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT SundanceApplicationOptionsDialog::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    return S_OK;
}

void SundanceApplicationOptionsDialog::SetDefaultPhotoDuration(DWORD dwDurationMs) { m_dwDefaultPhotoDuration = dwDurationMs; }
DWORD SundanceApplicationOptionsDialog::GetDefaultPhotoDuration() const throw() { return m_dwDefaultPhotoDuration; }
void SundanceApplicationOptionsDialog::SetAutoSaveInterval(DWORD dwIntervalMs) { m_dwAutoSaveInterval = dwIntervalMs; }
DWORD SundanceApplicationOptionsDialog::GetAutoSaveInterval() const throw() { return m_dwAutoSaveInterval; }

// ============================================================================
// DontShowPromptDialog
// ============================================================================
DontShowPromptDialog::DontShowPromptDialog()
    : m_pElement(NULL), m_bDontShowChecked(false), m_result(PromptResultCancel) {}

DontShowPromptDialog::~DontShowPromptDialog() { m_pElement = NULL; }

HRESULT DontShowPromptDialog::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT DontShowPromptDialog::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

void DontShowPromptDialog::SetMessage(LPCWSTR pszMessage) { m_strMessage = pszMessage ? pszMessage : L""; }
void DontShowPromptDialog::SetPromptKey(LPCWSTR pszKey) { m_strPromptKey = pszKey ? pszKey : L""; }
bool DontShowPromptDialog::IsDontShowChecked() const throw() { return m_bDontShowChecked; }
DontShowPromptDialog::PromptResult DontShowPromptDialog::GetResult() const throw() { return m_result; }

// ============================================================================
// SundanceDontShowPromptDialog
// ============================================================================
SundanceDontShowPromptDialog::SundanceDontShowPromptDialog()
    : m_pElement(NULL), m_pAppMain(NULL), m_bDontShowChecked(false), m_result(PromptResultCancel) {}

SundanceDontShowPromptDialog::~SundanceDontShowPromptDialog() { m_pElement = NULL; }

HRESULT SundanceDontShowPromptDialog::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundanceDontShowPromptDialog::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

void SundanceDontShowPromptDialog::SetMessage(LPCWSTR pszMessage) { m_strMessage = pszMessage ? pszMessage : L""; }
void SundanceDontShowPromptDialog::SetPromptKey(LPCWSTR pszKey) { m_strPromptKey = pszKey ? pszKey : L""; }

bool SundanceDontShowPromptDialog::ShouldShow() const
{
    if (!m_pAppMain)
        return true;
    return m_pAppMain->ShouldShowPrompt(m_strPromptKey);
}

SundanceDontShowPromptDialog::PromptResult SundanceDontShowPromptDialog::GetResult() const throw() { return m_result; }

} // namespace Sundance
