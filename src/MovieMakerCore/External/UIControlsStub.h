/*
 * UIControlsStub.h
 *
 * Lightweight stubs for UI controls referenced by MovieMakerCore.
 *
 * IsolationAwareTaskDialog: Task dialog wrapper (isolation-aware).
 *
 * RTTI: ?AVIsolationAwareTaskDialog@@
 *
 * This class wraps the Windows Task Dialog API (TaskDialogIndirect) with
 * isolation awareness for compatibility across Windows versions.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef UICONTROLS_STUB_H
#define UICONTROLS_STUB_H

#include "../pch.h"

// ============================================================================
// IsolationAwareTaskDialog
// ============================================================================
// Wrapper around the Windows TaskDialogIndirect API that provides
// isolation awareness. Falls back to MessageBox on systems where
// TaskDialogIndirect is unavailable (pre-Vista).
//
class IsolationAwareTaskDialog
{
public:
    IsolationAwareTaskDialog();
    ~IsolationAwareTaskDialog();

    // -- Configuration --
    void SetWindowTitle(LPCWSTR pszTitle);
    void SetMainInstruction(LPCWSTR pszInstruction);
    void SetContent(LPCWSTR pszContent);
    void SetFooter(LPCWSTR pszFooter);

    void SetMainIcon(LPCWSTR pszIcon);
    void SetFooterIcon(LPCWSTR pszIcon);

    // -- Buttons --
    void AddButton(int nButtonId, LPCWSTR pszButtonText);
    void SetDefaultButton(int nButtonId);
    void SetCommonButtons(DWORD dwCommonButtons);

    // -- Verification --
    void SetVerificationText(LPCWSTR pszText);
    void SetVerificationChecked(bool bChecked);
    bool IsVerificationChecked() const throw();

    // -- Expanded info --
    void SetExpandedInfo(LPCWSTR pszInfo);
    void SetExpandedByDefault(bool bExpand);

    // -- Display --
    INT_PTR Show(HWND hWndParent);

    // -- Result --
    int GetButtonResult() const throw();

    // -- Helpers --
    static int ShowSimpleMessage(HWND hWndParent, LPCWSTR pszTitle,
                                  LPCWSTR pszMessage, UINT uType);

private:
    ATL::CString m_strWindowTitle;
    ATL::CString m_strMainInstruction;
    ATL::CString m_strContent;
    ATL::CString m_strFooter;
    ATL::CString m_strMainIcon;
    ATL::CString m_strFooterIcon;

    struct ButtonEntry
    {
        int nButtonId;
        ATL::CString strText;
    };

    std::vector<ButtonEntry> m_customButtons;
    int     m_nDefaultButton;
    DWORD   m_dwCommonButtons;
    ATL::CString m_strVerificationText;
    bool    m_bVerificationChecked;
    ATL::CString m_strExpandedInfo;
    bool    m_bExpandedByDefault;

    int     m_nButtonResult;
};

#endif // UICONTROLS_STUB_H
