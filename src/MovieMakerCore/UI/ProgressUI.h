/*
 * ProgressUI.h
 *
 * Progress indicator UI classes: base progress, status bar progress,
 * and modal progress dialog.
 *
 * RTTI classes:
 *   ?AVProgressBase@@
 *   ?AVProgressStatusBar@@
 *   ?AVProgressDialog@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_PROGRESS_UI_H
#define SUNDANCE_PROGRESS_UI_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

namespace Sundance
{

// ============================================================================
// ProgressBase
// ============================================================================
// Base class for all progress indicators. Provides common range/current
// value management, cancellation support, and percentage calculation.
//
// RTTI: ?AVProgressBase@@
//
class ProgressBase
{
public:
    ProgressBase();
    virtual ~ProgressBase();

    // -- Range --
    void    SetRange(int nLower, int nUpper);
    void    GetRange(int* pnLower, int* pnUpper) const;

    // -- Current value --
    void    SetPos(int nPos);
    int     GetPos() const throw();

    // -- Step --
    void    SetStep(int nStep);
    void    StepIt();

    // -- Text --
    void    SetStatusText(LPCWSTR pszText);
    LPCWSTR GetStatusText() const throw();

    // -- Percentage --
    int     GetPercentage() const;

    // -- Cancellation --
    void    SetCancellable(bool bCancellable) throw();
    bool    IsCancellable() const throw();
    void    SetCancelled(bool bCancelled) throw();
    bool    IsCancelled() const throw();

    // -- Visibility --
    virtual void Show();
    virtual void Hide();
    virtual bool IsVisible() const throw();

protected:
    int     m_nLower;
    int     m_nUpper;
    int     m_nPos;
    int     m_nStep;
    bool    m_bCancellable;
    bool    m_bCancelled;
    bool    m_bVisible;

    ATL::CString m_strStatusText;

    virtual void OnProgressChanged();
};

// ============================================================================
// ProgressStatusBar
// ============================================================================
// Progress indicator that renders in the application status bar.
// Uses DirectUI element binding to update a progress bar element
// and status text element in the status bar.
//
// RTTI: ?AVProgressStatusBar@@
// ATL:  CComObjectNoLock<ProgressStatusBar>
//
class ATL_NO_VTABLE ProgressStatusBar :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<ProgressStatusBar>,
    public IDuiBehaviorImpl,
    public ProgressBase
{
public:
    ProgressStatusBar();
    virtual ~ProgressStatusBar();

    DECLARE_REGISTRY_RESOURCEID(IDR_PROGRESS_STATUS_BAR)
    DECLARE_NOT_AGGREGATABLE(ProgressStatusBar)

    BEGIN_COM_MAP(ProgressStatusBar)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiBehavior --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    // -- ProgressBase overrides --
    void Show() override;
    void Hide() override;

private:
    IDuiElement* m_pElement;
};

// ============================================================================
// ProgressDialog
// ============================================================================
// Modal progress dialog for long-running operations (publish, render,
// import). Displays a progress bar, status text, cancel button, and
// optional detail text. Creates its own HWND dialog.
//
// RTTI: ?AVProgressDialog@@
// ATL:  CComObjectNoLock<ProgressDialog>
//
class ATL_NO_VTABLE ProgressDialog :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<ProgressDialog>,
    public IDuiBehaviorImpl,
    public ProgressBase
{
public:
    ProgressDialog();
    virtual ~ProgressDialog();

    DECLARE_REGISTRY_RESOURCEID(IDR_PROGRESS_DIALOG)
    DECLARE_NOT_AGGREGATABLE(ProgressDialog)

    BEGIN_COM_MAP(ProgressDialog)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiBehavior --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    // -- ProgressBase overrides --
    void Show() override;
    void Hide() override;

    // -- Dialog operations --
    HRESULT ShowModal(HWND hWndParent);
    HRESULT ShowModeless(HWND hWndParent);
    void    Close();

    // -- Detail text --
    void    SetDetailText(LPCWSTR pszDetail);
    LPCWSTR GetDetailText() const throw();

    // -- Cancel callback --
    typedef void (CALLBACK* PFN_CANCEL)(void* pUserData);
    void    SetCancelCallback(PFN_CANCEL pfnCancel, void* pUserData);

private:
    IDuiElement*    m_pElement;
    HWND            m_hWndDialog;
    HWND            m_hWndParent;
    ATL::CString    m_strDetailText;
    PFN_CANCEL      m_pfnCancel;
    void*           m_pUserData;

    void UpdateUI();
};

} // namespace Sundance

#endif // SUNDANCE_PROGRESS_UI_H
