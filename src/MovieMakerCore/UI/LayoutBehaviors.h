/*
 * LayoutBehaviors.h
 *
 * Layout-related DirectUI behaviors: fullscreen, resizable panes,
 * workspace layout, and inline preview.
 *
 * RTTI classes:
 *   ?AVFullscreenLayoutBehavior@@
 *   ?AVFullscreenBackgroundWindow@@
 *   ?AVResizeablePaneBehavior@@
 *   ?AVProjectWorkspaceLayoutBehavior@@
 *   ?AVInlinePreviewLayoutBehavior@@
 *   ?AVInlinePreviewSliderBehavior@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_LAYOUT_BEHAVIORS_H
#define SUNDANCE_LAYOUT_BEHAVIORS_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

class SundanceAppMain;

namespace Sundance
{

// ============================================================================
// FullscreenLayoutBehavior
// ============================================================================
// Manages fullscreen mode for the preview pane. When activated, the
// preview element is resized to fill the entire screen with a black
// background and the timeline/command bar are hidden.
//
// RTTI: ?AVFullscreenLayoutBehavior@@
// ATL:  CComObjectNoLock<FullscreenLayoutBehavior>
//
class ATL_NO_VTABLE FullscreenLayoutBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<FullscreenLayoutBehavior>,
    public IDuiBehaviorImpl
{
public:
    FullscreenLayoutBehavior();
    virtual ~FullscreenLayoutBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_FULLSCREEN_LAYOUT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(FullscreenLayoutBehavior)

    BEGIN_COM_MAP(FullscreenLayoutBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    HRESULT EnterFullscreen(HWND hWndPreview);
    HRESULT ExitFullscreen();
    bool    IsFullscreen() const throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    HWND            m_hWndPreview;
    HWND            m_hWndBackground;
    bool            m_bFullscreen;
    RECT            m_rcOriginalPreview;
    DWORD           m_dwOriginalStyle;
    DWORD           m_dwOriginalExStyle;
};

// ============================================================================
// FullscreenBackgroundWindow
// ============================================================================
// Background HWND window for fullscreen mode. A simple black window
// that sits behind the preview element during fullscreen playback.
// Implemented as ATL CWindowImpl.
//
// RTTI: ?AVFullscreenBackgroundWindow@@
//
class FullscreenBackgroundWindow :
    public CWindowImpl<FullscreenBackgroundWindow, CWindow>
{
public:
    FullscreenBackgroundWindow();
    ~FullscreenBackgroundWindow();

    DECLARE_WND_CLASS(L"SundanceFullscreenBackground")

    BEGIN_MSG_MAP(FullscreenBackgroundWindow)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()

    HRESULT Create(HWND hWndParent);
    void    SetFullScreen();

    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

// ============================================================================
// ResizeablePaneBehavior
// ============================================================================
// Resizable split pane behavior. Manages a splitter bar between two
// panes (e.g., preview and timeline) that the user can drag to resize.
//
// RTTI: ?AVResizeablePaneBehavior@@
// ATL:  CComObjectNoLock<ResizeablePaneBehavior>
//
class ATL_NO_VTABLE ResizeablePaneBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<ResizeablePaneBehavior>,
    public IDuiBehaviorImpl
{
public:
    ResizeablePaneBehavior();
    virtual ~ResizeablePaneBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_RESIZEABLE_PANE_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(ResizeablePaneBehavior)

    BEGIN_COM_MAP(ResizeablePaneBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Splitter configuration --
    enum SplitDirection { SplitHorizontal, SplitVertical };
    void SetSplitDirection(SplitDirection direction) throw();
    void SetSplitRatio(float fRatio);
    void SetMinPaneSize(int nMinSize);
    void SetSplitterWidth(int nWidth);

private:
    IDuiElement*    m_pElement;
    SplitDirection  m_direction;
    float           m_fSplitRatio;
    int             m_nMinPaneSize;
    int             m_nSplitterWidth;
    bool            m_bDragging;
    int             m_nDragStart;
    float           m_fDragStartRatio;
};

// ============================================================================
// ProjectWorkspaceLayoutBehavior
// ============================================================================
// Workspace layout behavior that manages the overall arrangement of the
// Sundance application panes: media browser, preview pane, timeline,
// and detail/properties panel. Handles pane show/hide, docking, and
// layout transitions between view modes.
//
// RTTI: ?AVProjectWorkspaceLayoutBehavior@@
// ATL:  CComObjectNoLock<ProjectWorkspaceLayoutBehavior>
//
class ATL_NO_VTABLE ProjectWorkspaceLayoutBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<ProjectWorkspaceLayoutBehavior>,
    public IDuiBehaviorImpl
{
public:
    ProjectWorkspaceLayoutBehavior();
    virtual ~ProjectWorkspaceLayoutBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_PROJECT_WORKSPACE_LAYOUT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(ProjectWorkspaceLayoutBehavior)

    BEGIN_COM_MAP(ProjectWorkspaceLayoutBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Pane management --
    void ShowPane(LPCWSTR pszPaneName);
    void HidePane(LPCWSTR pszPaneName);
    void TogglePane(LPCWSTR pszPaneName);
    bool IsPaneVisible(LPCWSTR pszPaneName) const;

    // -- Layout presets --
    enum LayoutPreset
    {
        LayoutPresetDefault,        // Media + Preview + Timeline
        LayoutPresetPreviewLarge,   // Large preview, compact timeline
        LayoutPresetTimelineLarge,  // Large timeline, compact preview
        LayoutPresetEdit            // Editing mode layout
    };

    HRESULT ApplyLayoutPreset(LayoutPreset preset);
    LayoutPreset GetCurrentPreset() const throw();

private:
    IDuiElement*        m_pElement;
    SundanceAppMain*    m_pAppMain;
    LayoutPreset        m_currentPreset;

    struct PaneInfo
    {
        ATL::CString strName;
        bool        bVisible;
        RECT        rcBounds;
    };

    std::vector<PaneInfo> m_panes;

    void InitializePanes();
    void RecalculateLayout();
};

// ============================================================================
// InlinePreviewLayoutBehavior
// ============================================================================
// Inline preview layout behavior. Manages the small preview pane that
// appears above the timeline for quick preview without entering
// fullscreen mode.
//
// RTTI: ?AVInlinePreviewLayoutBehavior@@
// ATL:  CComObjectNoLock<InlinePreviewLayoutBehavior>
//
class ATL_NO_VTABLE InlinePreviewLayoutBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<InlinePreviewLayoutBehavior>,
    public IDuiBehaviorImpl
{
public:
    InlinePreviewLayoutBehavior();
    virtual ~InlinePreviewLayoutBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_INLINE_PREVIEW_LAYOUT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(InlinePreviewLayoutBehavior)

    BEGIN_COM_MAP(InlinePreviewLayoutBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Preview sizing --
    void SetPreviewSize(int nWidth, int nHeight);
    void GetPreviewSize(int* pnWidth, int* pnHeight) const;
    void SetPreviewPosition(POINT ptPosition);

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    int             m_nPreviewWidth;
    int             m_nPreviewHeight;
    POINT           m_ptPreviewPosition;
};

// ============================================================================
// InlinePreviewSliderBehavior
// ============================================================================
// Inline preview slider behavior. Manages a small playback position
// slider that appears below the inline preview pane, showing the
// current position and allowing scrub.
//
// RTTI: ?AVInlinePreviewSliderBehavior@@
// ATL:  CComObjectNoLock<InlinePreviewSliderBehavior>
//
class ATL_NO_VTABLE InlinePreviewSliderBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<InlinePreviewSliderBehavior>,
    public IDuiBehaviorImpl
{
public:
    InlinePreviewSliderBehavior();
    virtual ~InlinePreviewSliderBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_INLINE_PREVIEW_SLIDER_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(InlinePreviewSliderBehavior)

    BEGIN_COM_MAP(InlinePreviewSliderBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

    void SetPosition(double dPositionSeconds);
    void SetDuration(double dDurationSeconds);
    double GetPosition() const throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    double          m_dPosition;
    double          m_dDuration;
    bool            m_bDragging;
};

} // namespace Sundance

#endif // SUNDANCE_LAYOUT_BEHAVIORS_H
