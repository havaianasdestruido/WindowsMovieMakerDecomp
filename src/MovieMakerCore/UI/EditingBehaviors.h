/*
 * EditingBehaviors.h
 *
 * Editing-related DirectUI behaviors: trim, text editing, rich text,
 * caret management, z-ordering, options, help, and capture.
 *
 * RTTI classes:
 *   ?AVTrimBehavior@@
 *   ?AVTextBoxBehavior@@
 *   ?AVRichEditControlBehavior@@
 *   ?AVCaretBehavior@@
 *   ?AVCaretScrollBehavior@@
 *   ?AVAboveHomerBehavior@@
 *   ?AVOptionsDialogBehavior@@
 *   ?AVHelpBehavior@@
 *   ?AVCaptureUIBehavior@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_EDITING_BEHAVIORS_H
#define SUNDANCE_EDITING_BEHAVIORS_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

class SundanceAppMain;

namespace Sundance
{

// ============================================================================
// TrimBehavior
// ============================================================================
// Video trim UI behavior. Manages the trim handles displayed on timeline
// items, allowing the user to drag left/right handles to adjust the
// in-point and out-point of a clip. Shows a live preview of the trim
// boundary during drag.
//
// RTTI: ?AVTrimBehavior@@
// ATL:  CComObjectNoLock<TrimBehavior>
//
class ATL_NO_VTABLE TrimBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TrimBehavior>,
    public IDuiBehaviorImpl
{
public:
    TrimBehavior();
    virtual ~TrimBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_TRIM_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(TrimBehavior)

    BEGIN_COM_MAP(TrimBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

    // -- Trim state --
    void SetTrimRange(DWORD dwTrimStartMs, DWORD dwTrimEndMs);
    void GetTrimRange(DWORD* pdwTrimStartMs, DWORD* pdwTrimEndMs) const;
    void SetOriginalDuration(DWORD dwDurationMs);

    // -- Trim handle --
    enum TrimHandle { TrimHandleNone, TrimHandleLeft, TrimHandleRight };
    TrimHandle GetActiveHandle() const throw();

    // -- Callback --
    typedef void (CALLBACK* PFN_TRIM_CHANGED)(DWORD dwNewStartMs, DWORD dwNewEndMs, void* pUserData);
    void SetTrimCallback(PFN_TRIM_CHANGED pfnCallback, void* pUserData);

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;

    DWORD   m_dwTrimStartMs;
    DWORD   m_dwTrimEndMs;
    DWORD   m_dwOriginalDurationMs;
    bool    m_bDragging;
    TrimHandle m_activeHandle;
    int     m_nDragStartX;

    PFN_TRIM_CHANGED m_pfnTrimChanged;
    void*   m_pUserData;

    static const int kTrimHandleWidth = 6;
    static const int kTrimMinDurationMs = 1000;

    void PaintTrimOverlay(HDC hdc, const RECT* prcBounds);
    TrimHandle HitTestTrimHandle(POINT pt, const RECT* prcBounds) const;
};

// ============================================================================
// TextBoxBehavior
// ============================================================================
// Text editing behavior for timeline text overlays (titles, credits).
// Manages an in-place text editing overlay that appears when the user
// double-clicks a title or credits element on the timeline.
//
// RTTI: ?AVTextBoxBehavior@@
// ATL:  CComObjectNoLock<TextBoxBehavior>
//
class ATL_NO_VTABLE TextBoxBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TextBoxBehavior>,
    public IDuiBehaviorImpl
{
public:
    TextBoxBehavior();
    virtual ~TextBoxBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_TEXT_BOX_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(TextBoxBehavior)

    BEGIN_COM_MAP(TextBoxBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Text state --
    void SetText(LPCWSTR pszText);
    LPCWSTR GetText() const throw();
    void SetFont(LPCWSTR pszFontFamily, int nFontSize, bool bBold, bool bItalic);
    void SetTextColor(COLORREF clrText);
    void SetEditing(bool bEditing) throw();
    bool IsEditing() const throw();

private:
    IDuiElement*    m_pElement;
    ATL::CString    m_strText;
    ATL::CString    m_strFontFamily;
    int             m_nFontSize;
    bool            m_bBold;
    bool            m_bItalic;
    COLORREF        m_clrText;
    bool            m_bEditing;
    HWND            m_hWndEdit;
    HFONT           m_hFont;

    void CreateEditWindow(HWND hWndParent, RECT rcBounds);
    void DestroyEditWindow();
    void ApplyFont();
};

// ============================================================================
// RichEditControlBehavior
// ============================================================================
// Rich text editing behavior for formatted text on the timeline.
// Provides bold, italic, underline, font size, and color controls
// for text overlay editing.
//
// RTTI: ?AVRichEditControlBehavior@@
// ATL:  CComObjectNoLock<RichEditControlBehavior>
//
class ATL_NO_VTABLE RichEditControlBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<RichEditControlBehavior>,
    public IDuiBehaviorImpl
{
public:
    RichEditControlBehavior();
    virtual ~RichEditControlBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_RICH_EDIT_CONTROL_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(RichEditControlBehavior)

    BEGIN_COM_MAP(RichEditControlBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    // -- Rich text operations --
    void SetRichText(LPCWSTR pszRichText);
    HRESULT GetRichText(ATL::CString& strRichText);
    void SetSelectionBold(bool bBold);
    void SetSelectionItalic(bool bItalic);
    void SetSelectionUnderline(bool bUnderline);
    void SetSelectionFontSize(int nSize);
    void SetSelectionFontFamily(LPCWSTR pszFamily);
    void SetSelectionTextColor(COLORREF clrText);

private:
    IDuiElement*    m_pElement;
    HWND            m_hWndRichEdit;
    ATL::CString    m_strRichText;

    void CreateRichEditControl(HWND hWndParent);
    void DestroyRichEditControl();
};

// ============================================================================
// CaretBehavior
// ============================================================================
// Caret positioning behavior for text editing. Manages the blinking caret
// in text editing controls and positions it correctly relative to the
// text content and scroll offset.
//
// RTTI: ?AVCaretBehavior@@
// ATL:  CComObjectNoLock<CaretBehavior>
//
class ATL_NO_VTABLE CaretBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CaretBehavior>,
    public IDuiBehaviorImpl
{
public:
    CaretBehavior();
    virtual ~CaretBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_CARET_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(CaretBehavior)

    BEGIN_COM_MAP(CaretBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    void SetCaretPosition(POINT ptPosition);
    void SetCaretHeight(int nHeight);
    void ShowCaret(bool bShow);
    void SetCaretBlinkRate(DWORD dwBlinkRateMs);

private:
    IDuiElement* m_pElement;
    POINT        m_ptCaret;
    int          m_nCaretHeight;
    bool         m_bCaretVisible;
    DWORD        m_dwBlinkRate;
    UINT         m_nTimerId;
};

// ============================================================================
// CaretScrollBehavior
// ============================================================================
// Auto-scroll behavior that scrolls the text editing viewport to keep
// the caret visible during keyboard input or cursor movement.
//
// RTTI: ?AVCaretScrollBehavior@@
// ATL:  CComObjectNoLock<CaretScrollBehavior>
//
class ATL_NO_VTABLE CaretScrollBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CaretScrollBehavior>,
    public IDuiBehaviorImpl
{
public:
    CaretScrollBehavior();
    virtual ~CaretScrollBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_CARET_SCROLL_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(CaretScrollBehavior)

    BEGIN_COM_MAP(CaretScrollBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    void SetViewportRect(const RECT& rcViewport);
    void ScrollToCaret(POINT ptCaret, int nCaretHeight);
    void SetScrollMargin(int nMargin);

private:
    IDuiElement* m_pElement;
    RECT         m_rcViewport;
    int          m_nScrollMargin;
};

// ============================================================================
// AboveHomerBehavior
// ============================================================================
// Z-ordering behavior that places elements above the Homer rendering
// layer. Homer is the base video preview layer; elements with this
// behavior are rendered on top of the preview but below the main UI.
//
// RTTI: ?AVAboveHomerBehavior@@
// ATL:  CComObjectNoLock<AboveHomerBehavior>
//
class ATL_NO_VTABLE AboveHomerBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<AboveHomerBehavior>,
    public IDuiBehaviorImpl
{
public:
    AboveHomerBehavior();
    virtual ~AboveHomerBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_ABOVE_HOMER_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(AboveHomerBehavior)

    BEGIN_COM_MAP(AboveHomerBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement* m_pElement;
};

// ============================================================================
// OptionsDialogBehavior
// ============================================================================
// Behavior for the application options dialog. Manages the options
// page navigation and settings persistence.
//
// RTTI: ?AVOptionsDialogBehavior@@
// ATL:  CComObjectNoLock<OptionsDialogBehavior>
//
class ATL_NO_VTABLE OptionsDialogBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<OptionsDialogBehavior>,
    public IDuiBehaviorImpl
{
public:
    OptionsDialogBehavior();
    virtual ~OptionsDialogBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_OPTIONS_DIALOG_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(OptionsDialogBehavior)

    BEGIN_COM_MAP(OptionsDialogBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    void ShowOptionsPage(LPCWSTR pszPageName);
    void SaveOptions();
    void CancelOptions();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    ATL::CString    m_strCurrentPage;
};

// ============================================================================
// HelpBehavior
// ============================================================================
// Help system behavior. Launches the appropriate help topic in the
// Windows Live Help system or displays inline help popups.
//
// RTTI: ?AVHelpBehavior@@
// ATL:  CComObjectNoLock<HelpBehavior>
//
class ATL_NO_VTABLE HelpBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<HelpBehavior>,
    public IDuiBehaviorImpl
{
public:
    HelpBehavior();
    virtual ~HelpBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_HELP_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(HelpBehavior)

    BEGIN_COM_MAP(HelpBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    void ShowHelpTopic(LPCWSTR pszTopicId);
    void ShowInlineHelp(LPCWSTR pszHelpText, POINT ptPosition);

private:
    IDuiElement* m_pElement;
};

// ============================================================================
// CaptureUIBehavior
// ============================================================================
// Webcam capture UI behavior. Manages the video capture preview window,
// capture button, and device selection for webcam recording directly
// into the timeline.
//
// RTTI: ?AVCaptureUIBehavior@@
// ATL:  CComObjectNoLock<CaptureUIBehavior>
//
class ATL_NO_VTABLE CaptureUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CaptureUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    CaptureUIBehavior();
    virtual ~CaptureUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_CAPTURE_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(CaptureUIBehavior)

    BEGIN_COM_MAP(CaptureUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Capture operations --
    HRESULT StartPreview();
    HRESULT StopPreview();
    HRESULT StartCapture();
    HRESULT StopCapture();
    bool    IsCapturing() const throw();
    bool    IsPreviewing() const throw();

    // -- Device selection --
    HRESULT EnumCaptureDevices(std::vector<ATL::CString>& deviceNames);
    HRESULT SelectCaptureDevice(LPCWSTR pszDeviceName);

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    bool            m_bPreviewing;
    bool            m_bCapturing;
    ATL::CString    m_strSelectedDevice;
    HWND            m_hWndPreview;
};

} // namespace Sundance

#endif // SUNDANCE_EDITING_BEHAVIORS_H
