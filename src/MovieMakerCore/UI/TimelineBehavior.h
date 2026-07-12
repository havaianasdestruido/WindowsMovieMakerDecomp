/*
 * TimelineBehavior.h
 *
 * Timeline UI behavior classes for the Sundance DirectUI layer.
 * These behaviors manage the main timeline control, individual timeline items,
 * track items, input handling, instruction overlays, and selection root.
 *
 * RTTI classes:
 *   ?AVTimelineBehavior@@
 *   ?AVTimelineItemBehavior@@
 *   ?AVTimelineSecondaryTrackItemBehavior@@
 *   ?AVTimelineVisualTrackItemBehavior@@
 *   ?AVTimelineItemInputBehavior@@
 *   ?AVTimelineInstructionsBehavior@@
 *   ?AVTimelineSelectionRootBehavior@@
 *   ?AVTimelineBaseBehavior@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef TIMELINE_BEHAVIOR_H
#define TIMELINE_BEHAVIOR_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

class SundanceAppMain;

namespace Sundance
{

// ============================================================================
// TimelineBaseBehavior
// ============================================================================
// Base class for all timeline-related DirectUI behaviors. Provides common
// utilities for item selection, hit testing, coordinate conversion, and
// access to the SundanceAppMain application object.
//
// RTTI: ?AVTimelineBaseBehavior@@
// ATL:  CComObjectNoLock<TimelineBaseBehavior>
//
class ATL_NO_VTABLE TimelineBaseBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBaseBehavior>,
    public IDuiBehaviorImpl
{
public:
    TimelineBaseBehavior();
    virtual ~TimelineBaseBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BASE_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(TimelineBaseBehavior)

    BEGIN_COM_MAP(TimelineBaseBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiBehavior overrides --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Timeline-specific --
    SundanceAppMain* GetAppMain() const throw();
    void SetAppMain(SundanceAppMain* pAppMain) throw();

    // Selection helpers
    bool IsItemSelected(DWORD dwItemId) const;
    void SelectItem(DWORD dwItemId, bool bSelect);
    void ClearSelection();
    DWORD GetSelectedCount() const throw();

    // Coordinate conversion
    int PixelsToTimelineUnits(int xPixels) const;
    int TimelineUnitsToPixels(int dwUnits) const;

protected:
    // Cached element reference
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;

    // Selection state
    std::set<DWORD> m_selectedItems;

    // Timeline geometry (cached from last layout pass)
    int m_nScrollOffset;
    int m_nPixelsPerUnit;
    int m_nTimelineWidth;
    int m_nItemHeight;

    // Helpers
    virtual void OnLayoutChanged();
    virtual void OnSelectionChanged();
};

// ============================================================================
// TimelineBehavior -- main timeline control behavior
// ============================================================================
// The primary behavior attached to the root timeline element. Manages the
// overall timeline view including track arrangement, scroll position,
// playback cursor position, and zoom level.
//
// RTTI: ?AVTimelineBehavior@@
// ATL:  CComObjectNoLock<TimelineBehavior>
//
class ATL_NO_VTABLE TimelineBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBehavior>,
    public IDuiBehaviorImpl
{
public:
    TimelineBehavior();
    virtual ~TimelineBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(TimelineBehavior)

    BEGIN_COM_MAP(TimelineBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiBehavior --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;
    STDMETHOD(OnBehaviorChanged)() override;

    // -- Timeline operations --
    HRESULT SetPlaybackPosition(double dPositionSeconds);
    HRESULT SetZoomLevel(int nZoomLevel);
    HRESULT EnsureItemVisible(DWORD dwItemId);
    HRESULT ScrollToPosition(int nPosition);

    // -- Track queries --
    int  GetTrackCount() const throw();
    int  GetTrackHeight(int nTrackIndex) const;
    bool IsTrackVisible(int nTrackIndex) const;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;

    // Playback state
    double  m_dPlaybackPosition;
    bool    m_bPlaybackActive;

    // View state
    int     m_nZoomLevel;       // 1-10, higher = more zoom
    int     m_nScrollX;
    int     m_nScrollY;
    int     m_nTimelineHeight;
    int     m_nVisibleWidth;

    // Track layout
    static const int kDefaultTrackHeight = 80;
    static const int kMinZoomLevel = 1;
    static const int kMaxZoomLevel = 10;
    static const int kPixelsPerUnitAtZoom1 = 2;

    void CalculateLayout();
    int  GetPixelsPerUnit() const;
    int  GetTotalTimelineWidth() const;
    void PaintPlaybackCursor(HDC hdc, const RECT* prcBounds);
    void PaintTimeRuler(HDC hdc, const RECT* prcBounds);
    void PaintTracks(HDC hdc, const RECT* prcBounds);
};

// ============================================================================
// TimelineItemBehavior -- individual item on timeline
// ============================================================================
// Behavior for a single timeline item (video clip, photo, audio segment).
// Handles rendering the item's thumbnail strip, duration indicator,
// and selection highlight.
//
// RTTI: ?AVTimelineItemBehavior@@
// ATL:  CComObjectNoLock<TimelineItemBehavior>
//
class ATL_NO_VTABLE TimelineItemBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineItemBehavior>,
    public IDuiBehaviorImpl
{
public:
    TimelineItemBehavior();
    virtual ~TimelineItemBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_ITEM_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(TimelineItemBehavior)

    BEGIN_COM_MAP(TimelineItemBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiBehavior --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Item data --
    void SetItemId(DWORD dwItemId) throw();
    DWORD GetItemId() const throw();
    void SetTrackIndex(int nIndex) throw();
    int  GetTrackIndex() const throw();
    void SetItemBounds(const RECT& rcBounds);
    void GetItemBounds(RECT* prcBounds) const;

    // -- Item state --
    void SetSelected(bool bSelected) throw();
    bool IsSelected() const throw();
    void SetTrimStart(DWORD dwTrimStartMs) throw();
    void SetTrimEnd(DWORD dwTrimEndMs) throw();

    // -- Thumbnails --
    void InvalidateThumbnails();
    bool HasThumbnails() const throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;

    // Item identity
    DWORD   m_dwItemId;
    int     m_nTrackIndex;
    RECT    m_rcBounds;

    // Trim state
    DWORD   m_dwTrimStartMs;
    DWORD   m_dwTrimEndMs;
    DWORD   m_dwTotalDurationMs;

    // Selection
    bool    m_bSelected;

    // Thumbnails
    bool    m_bThumbnailsValid;
    std::vector<HBITMAP> m Thumbnails;

    void PaintItemBody(HDC hdc, const RECT* prcBounds);
    void PaintSelectionHighlight(HDC hdc, const RECT* prcBounds);
    void PaintTrimHandles(HDC hdc, const RECT* prcBounds);
    void LoadThumbnails();
    void ReleaseThumbnails();
};

// ============================================================================
// TimelineSecondaryTrackItemBehavior
// ============================================================================
// Behavior for items on secondary tracks (title overlays, credits, narration).
// Similar to TimelineItemBehavior but with different visual presentation.
//
// RTTI: ?AVTimelineSecondaryTrackItemBehavior@@
// ATL:  CComObjectNoLock<TimelineSecondaryTrackItemBehavior>
//
class ATL_NO_VTABLE TimelineSecondaryTrackItemBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineSecondaryTrackItemBehavior>,
    public IDuiBehaviorImpl
{
public:
    TimelineSecondaryTrackItemBehavior();
    virtual ~TimelineSecondaryTrackItemBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_SECONDARY_TRACK_ITEM_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(TimelineSecondaryTrackItemBehavior)

    BEGIN_COM_MAP(TimelineSecondaryTrackItemBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    void SetItemId(DWORD dwItemId) throw();
    DWORD GetItemId() const throw();
    void SetItemBounds(const RECT& rcBounds);
    void SetSelected(bool bSelected) throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    DWORD           m_dwItemId;
    RECT            m_rcBounds;
    bool            m_bSelected;
};

// ============================================================================
// TimelineVisualTrackItemBehavior
// ============================================================================
// Behavior for visual track items (transitions, effects). These are rendered
// as small indicators between or on top of primary track items.
//
// RTTI: ?AVTimelineVisualTrackItemBehavior@@
// ATL:  CComObjectNoLock<TimelineVisualTrackItemBehavior>
//
class ATL_NO_VTABLE TimelineVisualTrackItemBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineVisualTrackItemBehavior>,
    public IDuiBehaviorImpl
{
public:
    TimelineVisualTrackItemBehavior();
    virtual ~TimelineVisualTrackItemBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_VISUAL_TRACK_ITEM_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(TimelineVisualTrackItemBehavior)

    BEGIN_COM_MAP(TimelineVisualTrackItemBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    void SetItemId(DWORD dwItemId) throw();
    DWORD GetItemId() const throw();
    void SetEffectName(LPCWSTR pszEffectName);
    void SetItemBounds(const RECT& rcBounds);

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    DWORD           m_dwItemId;
    ATL::CString    m_strEffectName;
    RECT            m_rcBounds;
};

// ============================================================================
// TimelineItemInputBehavior
// ============================================================================
// Handles mouse and keyboard input for timeline items: drag-and-drop,
// right-click context menus, double-click editing, resize handles,
// and keyboard navigation.
//
// RTTI: ?AVTimelineItemInputBehavior@@
// ATL:  CComObjectNoLock<TimelineItemInputBehavior>
//
class ATL_NO_VTABLE TimelineItemInputBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineItemInputBehavior>,
    public IDuiBehaviorImpl
{
public:
    TimelineItemInputBehavior();
    virtual ~TimelineItemInputBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_ITEM_INPUT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(TimelineItemInputBehavior)

    BEGIN_COM_MAP(TimelineItemInputBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // Drag-drop state
    bool IsDragging() const throw();
    void CancelDrag();

    // Resize state
    enum ResizeEdge { ResizeNone = 0, ResizeLeft, ResizeRight };
    ResizeEdge GetResizeEdge() const throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;

    // Input state
    bool        m_bDragging;
    bool        m_bResizing;
    ResizeEdge  m_resizeEdge;
    POINT       m_ptDragStart;
    POINT       m_ptLastMouse;
    DWORD       m_dwDragItemId;
    int         m_nDragThreshold;

    // Helpers
    ResizeEdge HitTestResizeEdge(POINT pt, const RECT* prcItem) const;
    void OnMouseDown(UINT nFlags, POINT pt);
    void OnMouseMove(UINT nFlags, POINT pt);
    void OnMouseUp(UINT nFlags, POINT pt);
    void OnDblClk(UINT nFlags, POINT pt);
    void OnContextMenu(POINT pt);
    void OnKeyDown(UINT nChar, UINT nRepCount, UINT nFlags);
};

// ============================================================================
// TimelineInstructionsBehavior
// ============================================================================
// Renders instruction/tooltip overlays on the timeline, such as
// "Drag media here to add to your movie" placeholder text when the
// timeline is empty.
//
// RTTI: ?AVTimelineInstructionsBehavior@@
// ATL:  CComObjectNoLock<TimelineInstructionsBehavior>
//
class ATL_NO_VTABLE TimelineInstructionsBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineInstructionsBehavior>,
    public IDuiBehaviorImpl
{
public:
    TimelineInstructionsBehavior();
    virtual ~TimelineInstructionsBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_INSTRUCTIONS_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(TimelineInstructionsBehavior)

    BEGIN_COM_MAP(TimelineInstructionsBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

    // Instruction state
    void SetInstructionText(LPCWSTR pszText);
    void SetVisible(bool bVisible) throw();
    bool IsVisible() const throw();

private:
    IDuiElement*    m_pElement;
    ATL::CString    m_strInstructionText;
    bool            m_bVisible;
    HFONT           m_hFont;
};

// ============================================================================
// TimelineSelectionRootBehavior
// ============================================================================
// Manages rubber-band selection (marquee) on the timeline. Attached to
// the root timeline element and coordinates selection rectangles across
// multiple track items.
//
// RTTI: ?AVTimelineSelectionRootBehavior@@
// ATL:  CComObjectNoLock<TimelineSelectionRootBehavior>
//
class ATL_NO_VTABLE TimelineSelectionRootBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineSelectionRootBehavior>,
    public IDuiBehaviorImpl
{
public:
    TimelineSelectionRootBehavior();
    virtual ~TimelineSelectionRootBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_SELECTION_ROOT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(TimelineSelectionRootBehavior)

    BEGIN_COM_MAP(TimelineSelectionRootBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

    // Selection rectangle
    bool IsMarqueeActive() const throw();
    void GetSelectionRect(RECT* prcSelection) const;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;

    // Marquee selection state
    bool    m_bMarqueeActive;
    POINT   m_ptMarqueeStart;
    POINT   m_ptMarqueeEnd;
    RECT    m_rcSelection;

    void UpdateSelectionRect();
    void SelectItemsInRect(const RECT* prcSelection);
};

} // namespace Sundance

#endif // TIMELINE_BEHAVIOR_H
