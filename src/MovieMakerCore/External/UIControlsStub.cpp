/*
 * UIControlsStub.cpp
 *
 * Implementation of IsolationAwareTaskDialog.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "UIControlsStub.h"

// ============================================================================
// IsolationAwareTaskDialog implementation
// ============================================================================

IsolationAwareTaskDialog::IsolationAwareTaskDialog()
    : m_nDefaultButton(0)
    , m_dwCommonButtons(TDCBF_OK_BUTTON)
    , m_bVerificationChecked(false)
    , m_bExpandedByDefault(false)
    , m_nButtonResult(0)
{
}

IsolationAwareTaskDialog::~IsolationAwareTaskDialog()
{
}

void IsolationAwareTaskDialog::SetWindowTitle(LPCWSTR pszTitle)
{
    m_strWindowTitle = pszTitle ? pszTitle : L"";
}

void IsolationAwareTaskDialog::SetMainInstruction(LPCWSTR pszInstruction)
{
    m_strMainInstruction = pszInstruction ? pszInstruction : L"";
}

void IsolationAwareTaskDialog::SetContent(LPCWSTR pszContent)
{
    m_strContent = pszContent ? pszContent : L"";
}

void IsolationAwareTaskDialog::SetFooter(LPCWSTR pszFooter)
{
    m_strFooter = pszFooter ? pszFooter : L"";
}

void IsolationAwareTaskDialog::SetMainIcon(LPCWSTR pszIcon)
{
    m_strMainIcon = pszIcon ? pszIcon : L"";
}

void IsolationAwareTaskDialog::SetFooterIcon(LPCWSTR pszIcon)
{
    m_strFooterIcon = pszIcon ? pszIcon : L"";
}

void IsolationAwareTaskDialog::AddButton(int nButtonId, LPCWSTR pszButtonText)
{
    ButtonEntry entry;
    entry.nButtonId = nButtonId;
    entry.strText = pszButtonText ? pszButtonText : L"";
    m_customButtons.push_back(entry);
}

void IsolationAwareTaskDialog::SetDefaultButton(int nButtonId)
{
    m_nDefaultButton = nButtonId;
}

void IsolationAwareTaskDialog::SetCommonButtons(DWORD dwCommonButtons)
{
    m_dwCommonButtons = dwCommonButtons;
}

void IsolationAwareTaskDialog::SetVerificationText(LPCWSTR pszText)
{
    m_strVerificationText = pszText ? pszText : L"";
}

void IsolationAwareTaskDialog::SetVerificationChecked(bool bChecked)
{
    m_bVerificationChecked = bChecked;
}

bool IsolationAwareTaskDialog::IsVerificationChecked() const throw()
{
    return m_bVerificationChecked;
}

void IsolationAwareTaskDialog::SetExpandedInfo(LPCWSTR pszInfo)
{
    m_strExpandedInfo = pszInfo ? pszInfo : L"";
}

void IsolationAwareTaskDialog::SetExpandedByDefault(bool bExpand)
{
    m_bExpandedByDefault = bExpand;
}

INT_PTR IsolationAwareTaskDialog::Show(HWND hWndParent)
{
    // Try TaskDialogIndirect first (Vista+)
    HMODULE hComCtl32 = LoadLibraryW(L"comctl32.dll");
    if (hComCtl32)
    {
        typedef HRESULT(WINAPI* PFN_TaskDialogIndirect)(
            const TASKDIALOGCONFIG* pTaskConfig,
            int* pnButton, int* pnRadioButton, BOOL* pfVerificationFlagChecked);

        PFN_TaskDialogIndirect pfnTaskDialogIndirect =
            reinterpret_cast<PFN_TaskDialogIndirect>(
                GetProcAddress(hComCtl32, "TaskDialogIndirect"));

        if (pfnTaskDialogIndirect)
        {
            TASKDIALOGCONFIG config = {};
            config.cbSize = sizeof(config);
            config.hwndParent = hWndParent;
            config.dwFlags = m_bExpandedByDefault ? TDF_EXPANDED_BY_DEFAULT : 0;

            if (!m_strWindowTitle.IsEmpty())
                config.pszWindowTitle = m_strWindowTitle;

            if (!m_strMainInstruction.IsEmpty())
                config.pszMainInstruction = m_strMainInstruction;

            if (!m_strContent.IsEmpty())
                config.pszContent = m_strContent;

            if (!m_strFooter.IsEmpty())
                config.pszFooter = m_strFooter;

            config.dwCommonButtons = m_dwCommonButtons;
            config.nDefaultButton = m_nDefaultButton;

            if (!m_strVerificationText.IsEmpty())
            {
                config.pszVerificationText = m_strVerificationText;
                config.dwFlags |= TDF_VERIFICATION_FLAG_CHECKED;
            }

            if (!m_strExpandedInfo.IsEmpty())
                config.pszExpandedInformation = m_strExpandedInfo;

            // Configure custom buttons
            if (!m_customButtons.empty())
            {
                // In the full implementation, we would allocate button text
                // array and set pszButtons/cButtons.
            }

            BOOL fVerificationChecked = m_bVerificationChecked;
            int nButton = 0;

            HRESULT hr = pfnTaskDialogIndirect(&config, &nButton, nullptr, &fVerificationChecked);
            if (SUCCEEDED(hr))
            {
                m_bVerificationChecked = (fVerificationChecked != FALSE);
                m_nButtonResult = nButton;
                return nButton;
            }
        }

        FreeLibrary(hComCtl32);
    }

    // Fallback to MessageBox
    UINT uType = MB_OK;
    if (m_dwCommonButtons & TDCBF_YESNOBUTTON)
        uType = MB_YESNO;
    else if (m_dwCommonButtons & TDCBF_OKCANCELBUTTON)
        uType = MB_OKCANCEL;
    else if (m_dwCommonButtons & TDCBF_RETRYCANCELBUTTON)
        uType = MB_RETRYCANCEL;

    int nResult = MessageBoxW(hWndParent, m_strContent, m_strWindowTitle, uType);

    switch (nResult)
    {
    case IDYES: m_nButtonResult = IDYES; break;
    case IDNO:  m_nButtonResult = IDNO; break;
    case IDOK:  m_nButtonResult = IDOK; break;
    case IDCANCEL: m_nButtonResult = IDCANCEL; break;
    case IDRETRY: m_nButtonResult = IDRETRY; break;
    default:    m_nButtonResult = IDCANCEL; break;
    }

    return m_nButtonResult;
}

int IsolationAwareTaskDialog::GetButtonResult() const throw()
{
    return m_nButtonResult;
}

int IsolationAwareTaskDialog::ShowSimpleMessage(
    HWND hWndParent, LPCWSTR pszTitle, LPCWSTR pszMessage, UINT uType)
{
    return MessageBoxW(hWndParent, pszMessage, pszTitle, uType);
}
