/*
 * ProgressUI.cpp
 *
 * Implementation of progress indicator UI classes.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "ProgressUI.h"

namespace Sundance
{

// ============================================================================
// ProgressBase
// ============================================================================
ProgressBase::ProgressBase()
    : m_nLower(0)
    , m_nUpper(100)
    , m_nPos(0)
    , m_nStep(1)
    , m_bCancellable(false)
    , m_bCancelled(false)
    , m_bVisible(false)
{
}

ProgressBase::~ProgressBase() {}

void ProgressBase::SetRange(int nLower, int nUpper) { m_nLower = nLower; m_nUpper = nUpper; m_nPos = nLower; }
void ProgressBase::GetRange(int* pnLower, int* pnUpper) const { if (pnLower) *pnLower = m_nLower; if (pnUpper) *pnUpper = m_nUpper; }
void ProgressBase::SetPos(int nPos) { m_nPos = std::max(m_nLower, std::min(m_nUpper, nPos)); OnProgressChanged(); }
int ProgressBase::GetPos() const throw() { return m_nPos; }
void ProgressBase::SetStep(int nStep) { m_nStep = nStep; }

void ProgressBase::StepIt()
{
    m_nPos = std::min(m_nUpper, m_nPos + m_nStep);
    OnProgressChanged();
}

void ProgressBase::SetStatusText(LPCWSTR pszText) { m_strStatusText = pszText ? pszText : L""; }
LPCWSTR ProgressBase::GetStatusText() const throw() { return m_strStatusText; }

int ProgressBase::GetPercentage() const
{
    int nRange = m_nUpper - m_nLower;
    if (nRange <= 0) return 0;
    return ((m_nPos - m_nLower) * 100) / nRange;
}

void ProgressBase::SetCancellable(bool bCancellable) throw() { m_bCancellable = bCancellable; }
bool ProgressBase::IsCancellable() const throw() { return m_bCancellable; }
void ProgressBase::SetCancelled(bool bCancelled) throw() { m_bCancelled = bCancelled; }
bool ProgressBase::IsCancelled() const throw() { return m_bCancelled; }

void ProgressBase::Show() { m_bVisible = true; }
void ProgressBase::Hide() { m_bVisible = false; }
bool ProgressBase::IsVisible() const throw() { return m_bVisible; }

void ProgressBase::OnProgressChanged() {}

// ============================================================================
// ProgressStatusBar
// ============================================================================
ProgressStatusBar::ProgressStatusBar() : m_pElement(NULL) {}
ProgressStatusBar::~ProgressStatusBar() { m_pElement = NULL; }

HRESULT ProgressStatusBar::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ProgressStatusBar::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

void ProgressStatusBar::Show()
{
    ProgressBase::Show();
    if (m_pElement) m_pElement->InvalidateRect(NULL);
}

void ProgressStatusBar::Hide()
{
    ProgressBase::Hide();
    if (m_pElement) m_pElement->InvalidateRect(NULL);
}

// ============================================================================
// ProgressDialog
// ============================================================================
ProgressDialog::ProgressDialog()
    : m_pElement(NULL)
    , m_hWndDialog(NULL)
    , m_hWndParent(NULL)
    , m_pfnCancel(NULL)
    , m_pUserData(NULL)
{
}

ProgressDialog::~ProgressDialog()
{
    Close();
    m_pElement = NULL;
}

HRESULT ProgressDialog::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ProgressDialog::OnElementDetached(IDuiElement*) { Close(); m_pElement = NULL; return S_OK; }

void ProgressDialog::Show()
{
    ProgressBase::Show();
    UpdateUI();
}

void ProgressDialog::Hide()
{
    ProgressBase::Hide();
    Close();
}

HRESULT ProgressDialog::ShowModal(HWND hWndParent)
{
    m_hWndParent = hWndParent;
    m_bVisible = true;

    // In the full implementation, this creates a modeless dialog
    // with a progress bar, status text, and cancel button.
    // For now, we set the dialog as visible and update UI.
    UpdateUI();

    return S_OK;
}

HRESULT ProgressDialog::ShowModeless(HWND hWndParent)
{
    m_hWndParent = hWndParent;
    m_bVisible = true;
    UpdateUI();
    return S_OK;
}

void ProgressDialog::Close()
{
    if (m_hWndDialog && ::IsWindow(m_hWndDialog))
    {
        ::DestroyWindow(m_hWndDialog);
    }
    m_hWndDialog = NULL;
    m_bVisible = false;
}

void ProgressDialog::SetDetailText(LPCWSTR pszDetail) { m_strDetailText = pszDetail ? pszDetail : L""; }
LPCWSTR ProgressDialog::GetDetailText() const throw() { return m_strDetailText; }

void ProgressDialog::SetCancelCallback(PFN_CANCEL pfnCancel, void* pUserData)
{
    m_pfnCancel = pfnCancel;
    m_pUserData = pUserData;
}

void ProgressDialog::UpdateUI()
{
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
}

} // namespace Sundance
