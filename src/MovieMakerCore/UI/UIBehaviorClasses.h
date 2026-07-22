/*
 * UIBehaviorClasses.h
 *
 * Additional UI behavior RTTI classes for the Sundance DirectUI layer.
 * These classes fill in the remaining behavior families present in the
 * original Windows Live Movie Maker 2012 binary but not yet represented
 * in the source tree.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_UI_BEHAVIOR_CLASSES_H
#define SUNDANCE_UI_BEHAVIOR_CLASSES_H

#include "../pch.h"
#include "../MovieMakerCore.h"
#include "DuiInterfaces.h"

class SundanceAppMain;

namespace Sundance
{

// ============================================================================
// TimelineBehaviorFamily
// ============================================================================

// TimelineBehaviorSelection
class ATL_NO_VTABLE TimelineBehaviorSelection :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBehaviorSelection>,
    public IDuiBehaviorImpl
{
public:
    TimelineBehaviorSelection();
    virtual ~TimelineBehaviorSelection();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BEHAVIOR_SELECTION)
    DECLARE_NOT_AGGREGATABLE(TimelineBehaviorSelection)

    BEGIN_COM_MAP(TimelineBehaviorSelection)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// TimelineBehaviorDragDrop
class ATL_NO_VTABLE TimelineBehaviorDragDrop :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBehaviorDragDrop>,
    public IDuiBehaviorImpl
{
public:
    TimelineBehaviorDragDrop();
    virtual ~TimelineBehaviorDragDrop();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BEHAVIOR_DRAG_DROP)
    DECLARE_NOT_AGGREGATABLE(TimelineBehaviorDragDrop)

    BEGIN_COM_MAP(TimelineBehaviorDragDrop)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    BOOL            m_bDragging;
    POINT           m_ptDragStart;
};

// TimelineBehaviorResize
class ATL_NO_VTABLE TimelineBehaviorResize :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBehaviorResize>,
    public IDuiBehaviorImpl
{
public:
    TimelineBehaviorResize();
    virtual ~TimelineBehaviorResize();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BEHAVIOR_RESIZE)
    DECLARE_NOT_AGGREGATABLE(TimelineBehaviorResize)

    BEGIN_COM_MAP(TimelineBehaviorResize)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    BOOL            m_bResizing;
    RECT            m_rcOriginal;
    int             m_minWidth;
    IDuiElement*    m_pTimelineElement;
    HWND            m_hWnd;
};

// TimelineBehaviorTrim
class ATL_NO_VTABLE TimelineBehaviorTrim :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBehaviorTrim>,
    public IDuiBehaviorImpl
{
public:
    TimelineBehaviorTrim();
    virtual ~TimelineBehaviorTrim();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BEHAVIOR_TRIM)
    DECLARE_NOT_AGGREGATABLE(TimelineBehaviorTrim)

    BEGIN_COM_MAP(TimelineBehaviorTrim)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    BOOL            m_bTrimming;
    BOOL            m_bTrimmingStart;
    POINT           m_ptTrimStart;
    RECT            m_rcOriginal;
    RECT            m_rcTrimmed;
    int             m_minTrimWidth;
    HWND            m_hWnd;
};

// TimelineBehaviorSplit
class ATL_NO_VTABLE TimelineBehaviorSplit :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBehaviorSplit>,
    public IDuiBehaviorImpl
{
public:
    TimelineBehaviorSplit();
    virtual ~TimelineBehaviorSplit();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BEHAVIOR_SPLIT)
    DECLARE_NOT_AGGREGATABLE(TimelineBehaviorSplit)

    BEGIN_COM_MAP(TimelineBehaviorSplit)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    POINT           m_ptSplitPoint;
    IDuiElement*    m_pTimelineElement;
};

// TimelineBehaviorReorder
class ATL_NO_VTABLE TimelineBehaviorReorder :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBehaviorReorder>,
    public IDuiBehaviorImpl
{
public:
    TimelineBehaviorReorder();
    virtual ~TimelineBehaviorReorder();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BEHAVIOR_REORDER)
    DECLARE_NOT_AGGREGATABLE(TimelineBehaviorReorder)

    BEGIN_COM_MAP(TimelineBehaviorReorder)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    BOOL            m_bReordering;
    POINT           m_ptDragStart;
    RECT            m_rcOriginal;
    int             m_nStartIndex;
    int             m_nOriginalIndex;
    IDuiElement*    m_pTimelineElement;
};

// TimelineBehaviorZoom
class ATL_NO_VTABLE TimelineBehaviorZoom :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBehaviorZoom>,
    public IDuiBehaviorImpl
{
public:
    TimelineBehaviorZoom();
    virtual ~TimelineBehaviorZoom();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BEHAVIOR_ZOOM)
    DECLARE_NOT_AGGREGATABLE(TimelineBehaviorZoom)

    BEGIN_COM_MAP(TimelineBehaviorZoom)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    BOOL            m_bZooming;
    POINT           m_ptZoomStart;
    double          m_dStartZoomLevel;
    double          m_dCurrentZoomLevel;
    double          m_dMinZoom;
    double          m_dMaxZoom;
    HWND            m_hWnd;
    IDuiElement*    m_pTimelineElement;
};

// TimelineBehaviorScroll
class ATL_NO_VTABLE TimelineBehaviorScroll :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBehaviorScroll>,
    public IDuiBehaviorImpl
{
public:
    TimelineBehaviorScroll();
    virtual ~TimelineBehaviorScroll();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BEHAVIOR_SCROLL)
    DECLARE_NOT_AGGREGATABLE(TimelineBehaviorScroll)

    BEGIN_COM_MAP(TimelineBehaviorScroll)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    BOOL            m_bScrolling;
    POINT           m_ptScrollStart;
    int             m_nScrollStep;
    HWND            m_hWnd;
    IDuiElement*    m_pTimelineElement;
};

// TimelineBehaviorSnap
class ATL_NO_VTABLE TimelineBehaviorSnap :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineBehaviorSnap>,
    public IDuiBehaviorImpl
{
public:
    TimelineBehaviorSnap();
    virtual ~TimelineBehaviorSnap();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_BEHAVIOR_SNAP)
    DECLARE_NOT_AGGREGATABLE(TimelineBehaviorSnap)

    BEGIN_COM_MAP(TimelineBehaviorSnap)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    BOOL            m_bSnapping;
    LONGLONG        m_llSnapPosition;
    IDuiElement*    m_pTimelineElement;
};

// ============================================================================
// Ribbon Extended Behaviors
// ============================================================================

// RibbonCommandHandler
class ATL_NO_VTABLE RibbonCommandHandler :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<RibbonCommandHandler>,
    public IDuiBehaviorImpl
{
public:
    RibbonCommandHandler();
    virtual ~RibbonCommandHandler();

    DECLARE_REGISTRY_RESOURCEID(IDR_RIBBON_COMMAND_HANDLER)
    DECLARE_NOT_AGGREGATABLE(RibbonCommandHandler)

    BEGIN_COM_MAP(RibbonCommandHandler)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// RibbonEventHandler
class ATL_NO_VTABLE RibbonEventHandler :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<RibbonEventHandler>,
    public IDuiBehaviorImpl
{
public:
    RibbonEventHandler();
    virtual ~RibbonEventHandler();

    DECLARE_REGISTRY_RESOURCEID(IDR_RIBBON_EVENT_HANDLER)
    DECLARE_NOT_AGGREGATABLE(RibbonEventHandler)

    BEGIN_COM_MAP(RibbonEventHandler)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// RibbonSiteBehavior
class ATL_NO_VTABLE RibbonSiteBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<RibbonSiteBehavior>,
    public IDuiBehaviorImpl
{
public:
    RibbonSiteBehavior();
    virtual ~RibbonSiteBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_RIBBON_SITE)
    DECLARE_NOT_AGGREGATABLE(RibbonSiteBehavior)

    BEGIN_COM_MAP(RibbonSiteBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// RibbonContextualUIBehavior
class ATL_NO_VTABLE RibbonContextualUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<RibbonContextualUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    RibbonContextualUIBehavior();
    virtual ~RibbonContextualUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_RIBBON_CONTEXTUAL_UI)
    DECLARE_NOT_AGGREGATABLE(RibbonContextualUIBehavior)

    BEGIN_COM_MAP(RibbonContextualUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// RibbonQuickAccessBehavior
class ATL_NO_VTABLE RibbonQuickAccessBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<RibbonQuickAccessBehavior>,
    public IDuiBehaviorImpl
{
public:
    RibbonQuickAccessBehavior();
    virtual ~RibbonQuickAccessBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_RIBBON_QUICK_ACCESS)
    DECLARE_NOT_AGGREGATABLE(RibbonQuickAccessBehavior)

    BEGIN_COM_MAP(RibbonQuickAccessBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// RibbonGalleryBehavior
class ATL_NO_VTABLE RibbonGalleryBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<RibbonGalleryBehavior>,
    public IDuiBehaviorImpl
{
public:
    RibbonGalleryBehavior();
    virtual ~RibbonGalleryBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_RIBBON_GALLERY_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(RibbonGalleryBehavior)

    BEGIN_COM_MAP(RibbonGalleryBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// RibbonRecentItemsBehavior
class ATL_NO_VTABLE RibbonRecentItemsBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<RibbonRecentItemsBehavior>,
    public IDuiBehaviorImpl
{
public:
    RibbonRecentItemsBehavior();
    virtual ~RibbonRecentItemsBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_RIBBON_RECENT_ITEMS)
    DECLARE_NOT_AGGREGATABLE(RibbonRecentItemsBehavior)

    BEGIN_COM_MAP(RibbonRecentItemsBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// ============================================================================
// Selection / Editor Behaviors
// ============================================================================

// SelectionRootImpl
class ATL_NO_VTABLE SelectionRootImpl :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SelectionRootImpl>,
    public IDuiBehaviorImpl
{
public:
    SelectionRootImpl();
    virtual ~SelectionRootImpl();

    DECLARE_REGISTRY_RESOURCEID(IDR_SELECTION_ROOT_IMPL)
    DECLARE_NOT_AGGREGATABLE(SelectionRootImpl)

    BEGIN_COM_MAP(SelectionRootImpl)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// SelectionManager
class ATL_NO_VTABLE SelectionManager :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SelectionManager>,
    public IDuiBehaviorImpl
{
public:
    SelectionManager();
    virtual ~SelectionManager();

    DECLARE_REGISTRY_RESOURCEID(IDR_SELECTION_MANAGER)
    DECLARE_NOT_AGGREGATABLE(SelectionManager)

    BEGIN_COM_MAP(SelectionManager)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// SelectionRange
class ATL_NO_VTABLE SelectionRange :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SelectionRange>,
    public IDuiBehaviorImpl
{
public:
    SelectionRange();
    virtual ~SelectionRange();

    DECLARE_REGISTRY_RESOURCEID(IDR_SELECTION_RANGE)
    DECLARE_NOT_AGGREGATABLE(SelectionRange)

    BEGIN_COM_MAP(SelectionRange)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

private:
    IDuiElement* m_pElement;
};

// SelectionContext
class ATL_NO_VTABLE SelectionContext :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SelectionContext>,
    public IDuiBehaviorImpl
{
public:
    SelectionContext();
    virtual ~SelectionContext();

    DECLARE_REGISTRY_RESOURCEID(IDR_SELECTION_CONTEXT)
    DECLARE_NOT_AGGREGATABLE(SelectionContext)

    BEGIN_COM_MAP(SelectionContext)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// SelectionUI
class ATL_NO_VTABLE SelectionUI :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SelectionUI>,
    public IDuiBehaviorImpl
{
public:
    SelectionUI();
    virtual ~SelectionUI();

    DECLARE_REGISTRY_RESOURCEID(IDR_SELECTION_UI)
    DECLARE_NOT_AGGREGATABLE(SelectionUI)

    BEGIN_COM_MAP(SelectionUI)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

private:
    IDuiElement* m_pElement;
};

// EditorToolbar
class ATL_NO_VTABLE EditorToolbar :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<EditorToolbar>,
    public IDuiBehaviorImpl
{
public:
    EditorToolbar();
    virtual ~EditorToolbar();

    DECLARE_REGISTRY_RESOURCEID(IDR_EDITOR_TOOLBAR)
    DECLARE_NOT_AGGREGATABLE(EditorToolbar)

    BEGIN_COM_MAP(EditorToolbar)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// EditorCommandHandler
class ATL_NO_VTABLE EditorCommandHandler :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<EditorCommandHandler>,
    public IDuiBehaviorImpl
{
public:
    EditorCommandHandler();
    virtual ~EditorCommandHandler();

    DECLARE_REGISTRY_RESOURCEID(IDR_EDITOR_COMMAND_HANDLER)
    DECLARE_NOT_AGGREGATABLE(EditorCommandHandler)

    BEGIN_COM_MAP(EditorCommandHandler)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// EditorState
class ATL_NO_VTABLE EditorState :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<EditorState>,
    public IDuiBehaviorImpl
{
public:
    EditorState();
    virtual ~EditorState();

    DECLARE_REGISTRY_RESOURCEID(IDR_EDITOR_STATE)
    DECLARE_NOT_AGGREGATABLE(EditorState)

    BEGIN_COM_MAP(EditorState)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// ============================================================================
// Sundance Element Behaviors (extending SundanceMainElementBehavior)
// ============================================================================

// SundanceRibbonElementBehavior
class ATL_NO_VTABLE SundanceRibbonElementBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SundanceRibbonElementBehavior>,
    public IDuiBehaviorImpl
{
public:
    SundanceRibbonElementBehavior();
    virtual ~SundanceRibbonElementBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_SUNDANCE_RIBBON_ELEMENT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(SundanceRibbonElementBehavior)

    BEGIN_COM_MAP(SundanceRibbonElementBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// SundanceStatusBarElementBehavior
class ATL_NO_VTABLE SundanceStatusBarElementBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SundanceStatusBarElementBehavior>,
    public IDuiBehaviorImpl
{
public:
    SundanceStatusBarElementBehavior();
    virtual ~SundanceStatusBarElementBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_SUNDANCE_STATUS_BAR_ELEMENT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(SundanceStatusBarElementBehavior)

    BEGIN_COM_MAP(SundanceStatusBarElementBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// SundanceTimelineElementBehavior
class ATL_NO_VTABLE SundanceTimelineElementBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SundanceTimelineElementBehavior>,
    public IDuiBehaviorImpl
{
public:
    SundanceTimelineElementBehavior();
    virtual ~SundanceTimelineElementBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_SUNDANCE_TIMELINE_ELEMENT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(SundanceTimelineElementBehavior)

    BEGIN_COM_MAP(SundanceTimelineElementBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// SundancePreviewElementBehavior
class ATL_NO_VTABLE SundancePreviewElementBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SundancePreviewElementBehavior>,
    public IDuiBehaviorImpl
{
public:
    SundancePreviewElementBehavior();
    virtual ~SundancePreviewElementBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_SUNDANCE_PREVIEW_ELEMENT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(SundancePreviewElementBehavior)

    BEGIN_COM_MAP(SundancePreviewElementBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// SundancePropertyElementBehavior
class ATL_NO_VTABLE SundancePropertyElementBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SundancePropertyElementBehavior>,
    public IDuiBehaviorImpl
{
public:
    SundancePropertyElementBehavior();
    virtual ~SundancePropertyElementBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_SUNDANCE_PROPERTY_ELEMENT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(SundancePropertyElementBehavior)

    BEGIN_COM_MAP(SundancePropertyElementBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// ============================================================================
// Webcam / Narration Behaviors
// ============================================================================

// WebcamUIBehavior
class ATL_NO_VTABLE WebcamUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<WebcamUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    WebcamUIBehavior();
    virtual ~WebcamUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_WEBCAM_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(WebcamUIBehavior)

    BEGIN_COM_MAP(WebcamUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// WebcamPreviewBehavior
class ATL_NO_VTABLE WebcamPreviewBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<WebcamPreviewBehavior>,
    public IDuiBehaviorImpl
{
public:
    WebcamPreviewBehavior();
    virtual ~WebcamPreviewBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_WEBCAM_PREVIEW_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(WebcamPreviewBehavior)

    BEGIN_COM_MAP(WebcamPreviewBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

private:
    IDuiElement* m_pElement;
};

// WebcamCaptureBehavior
class ATL_NO_VTABLE WebcamCaptureBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<WebcamCaptureBehavior>,
    public IDuiBehaviorImpl
{
public:
    WebcamCaptureBehavior();
    virtual ~WebcamCaptureBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_WEBCAM_CAPTURE_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(WebcamCaptureBehavior)

    BEGIN_COM_MAP(WebcamCaptureBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// NarrationUIBehavior
class ATL_NO_VTABLE NarrationUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<NarrationUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    NarrationUIBehavior();
    virtual ~NarrationUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_NARRATION_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(NarrationUIBehavior)

    BEGIN_COM_MAP(NarrationUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// NarrationRecordBehavior
class ATL_NO_VTABLE NarrationRecordBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<NarrationRecordBehavior>,
    public IDuiBehaviorImpl
{
public:
    NarrationRecordBehavior();
    virtual ~NarrationRecordBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_NARRATION_RECORD_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(NarrationRecordBehavior)

    BEGIN_COM_MAP(NarrationRecordBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// NarrationPlaybackBehavior
class ATL_NO_VTABLE NarrationPlaybackBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<NarrationPlaybackBehavior>,
    public IDuiBehaviorImpl
{
public:
    NarrationPlaybackBehavior();
    virtual ~NarrationPlaybackBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_NARRATION_PLAYBACK_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(NarrationPlaybackBehavior)

    BEGIN_COM_MAP(NarrationPlaybackBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// ============================================================================
// Encode Profile Extended Behaviors
// ============================================================================

// UserEncodeProfileManager
class ATL_NO_VTABLE UserEncodeProfileManager :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<UserEncodeProfileManager>,
    public IDuiBehaviorImpl
{
public:
    UserEncodeProfileManager();
    virtual ~UserEncodeProfileManager();

    DECLARE_REGISTRY_RESOURCEID(IDR_USER_ENCODE_PROFILE_MANAGER)
    DECLARE_NOT_AGGREGATABLE(UserEncodeProfileManager)

    BEGIN_COM_MAP(UserEncodeProfileManager)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// UserEncodeProfilePreset
class ATL_NO_VTABLE UserEncodeProfilePreset :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<UserEncodeProfilePreset>,
    public IDuiBehaviorImpl
{
public:
    UserEncodeProfilePreset();
    virtual ~UserEncodeProfilePreset();

    DECLARE_REGISTRY_RESOURCEID(IDR_USER_ENCODE_PROFILE_PRESET)
    DECLARE_NOT_AGGREGATABLE(UserEncodeProfilePreset)

    BEGIN_COM_MAP(UserEncodeProfilePreset)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// UserEncodeProfileCustom
class ATL_NO_VTABLE UserEncodeProfileCustom :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<UserEncodeProfileCustom>,
    public IDuiBehaviorImpl
{
public:
    UserEncodeProfileCustom();
    virtual ~UserEncodeProfileCustom();

    DECLARE_REGISTRY_RESOURCEID(IDR_USER_ENCODE_PROFILE_CUSTOM)
    DECLARE_NOT_AGGREGATABLE(UserEncodeProfileCustom)

    BEGIN_COM_MAP(UserEncodeProfileCustom)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// UserEncodeProfileRegistry
class ATL_NO_VTABLE UserEncodeProfileRegistry :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<UserEncodeProfileRegistry>,
    public IDuiBehaviorImpl
{
public:
    UserEncodeProfileRegistry();
    virtual ~UserEncodeProfileRegistry();

    DECLARE_REGISTRY_RESOURCEID(IDR_USER_ENCODE_PROFILE_REGISTRY)
    DECLARE_NOT_AGGREGATABLE(UserEncodeProfileRegistry)

    BEGIN_COM_MAP(UserEncodeProfileRegistry)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// ============================================================================
// Publish Dialog Behaviors
// ============================================================================

// PublishDialogBaseBehavior
class ATL_NO_VTABLE PublishDialogBaseBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<PublishDialogBaseBehavior>,
    public IDuiBehaviorImpl
{
public:
    PublishDialogBaseBehavior();
    virtual ~PublishDialogBaseBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_PUBLISH_DIALOG_BASE)
    DECLARE_NOT_AGGREGATABLE(PublishDialogBaseBehavior)

    BEGIN_COM_MAP(PublishDialogBaseBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// PublishDialogYouTubeBehavior
class ATL_NO_VTABLE PublishDialogYouTubeBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<PublishDialogYouTubeBehavior>,
    public IDuiBehaviorImpl
{
public:
    PublishDialogYouTubeBehavior();
    virtual ~PublishDialogYouTubeBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_PUBLISH_DIALOG_YOUTUBE)
    DECLARE_NOT_AGGREGATABLE(PublishDialogYouTubeBehavior)

    BEGIN_COM_MAP(PublishDialogYouTubeBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// PublishDialogFacebookBehavior
class ATL_NO_VTABLE PublishDialogFacebookBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<PublishDialogFacebookBehavior>,
    public IDuiBehaviorImpl
{
public:
    PublishDialogFacebookBehavior();
    virtual ~PublishDialogFacebookBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_PUBLISH_DIALOG_FACEBOOK)
    DECLARE_NOT_AGGREGATABLE(PublishDialogFacebookBehavior)

    BEGIN_COM_MAP(PublishDialogFacebookBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// PublishDialogSkyDriveBehavior
class ATL_NO_VTABLE PublishDialogSkyDriveBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<PublishDialogSkyDriveBehavior>,
    public IDuiBehaviorImpl
{
public:
    PublishDialogSkyDriveBehavior();
    virtual ~PublishDialogSkyDriveBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_PUBLISH_DIALOG_SKYDRIVE)
    DECLARE_NOT_AGGREGATABLE(PublishDialogSkyDriveBehavior)

    BEGIN_COM_MAP(PublishDialogSkyDriveBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// PublishDialogEmailBehavior
class ATL_NO_VTABLE PublishDialogEmailBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<PublishDialogEmailBehavior>,
    public IDuiBehaviorImpl
{
public:
    PublishDialogEmailBehavior();
    virtual ~PublishDialogEmailBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_PUBLISH_DIALOG_EMAIL)
    DECLARE_NOT_AGGREGATABLE(PublishDialogEmailBehavior)

    BEGIN_COM_MAP(PublishDialogEmailBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// PublishDialogLocalFileBehavior
class ATL_NO_VTABLE PublishDialogLocalFileBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<PublishDialogLocalFileBehavior>,
    public IDuiBehaviorImpl
{
public:
    PublishDialogLocalFileBehavior();
    virtual ~PublishDialogLocalFileBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_PUBLISH_DIALOG_LOCAL_FILE)
    DECLARE_NOT_AGGREGATABLE(PublishDialogLocalFileBehavior)

    BEGIN_COM_MAP(PublishDialogLocalFileBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
};

// ============================================================================
// Miscellaneous UI Behaviors
// ============================================================================

// ProgressBarUIBehavior
class ATL_NO_VTABLE ProgressBarUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<ProgressBarUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    ProgressBarUIBehavior();
    virtual ~ProgressBarUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_PROGRESS_BAR_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(ProgressBarUIBehavior)

    BEGIN_COM_MAP(ProgressBarUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

private:
    IDuiElement* m_pElement;
};

// ErrorDialogUIBehavior
class ATL_NO_VTABLE ErrorDialogUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<ErrorDialogUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    ErrorDialogUIBehavior();
    virtual ~ErrorDialogUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_ERROR_DIALOG_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(ErrorDialogUIBehavior)

    BEGIN_COM_MAP(ErrorDialogUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// WarningDialogUIBehavior
class ATL_NO_VTABLE WarningDialogUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<WarningDialogUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    WarningDialogUIBehavior();
    virtual ~WarningDialogUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_WARNING_DIALOG_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(WarningDialogUIBehavior)

    BEGIN_COM_MAP(WarningDialogUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// InfoDialogUIBehavior
class ATL_NO_VTABLE InfoDialogUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<InfoDialogUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    InfoDialogUIBehavior();
    virtual ~InfoDialogUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_INFO_DIALOG_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(InfoDialogUIBehavior)

    BEGIN_COM_MAP(InfoDialogUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// ConfirmationDialogUIBehavior
class ATL_NO_VTABLE ConfirmationDialogUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<ConfirmationDialogUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    ConfirmationDialogUIBehavior();
    virtual ~ConfirmationDialogUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_CONFIRMATION_DIALOG_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(ConfirmationDialogUIBehavior)

    BEGIN_COM_MAP(ConfirmationDialogUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// FileBrowserUIBehavior
class ATL_NO_VTABLE FileBrowserUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<FileBrowserUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    FileBrowserUIBehavior();
    virtual ~FileBrowserUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_FILE_BROWSER_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(FileBrowserUIBehavior)

    BEGIN_COM_MAP(FileBrowserUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// FolderBrowserUIBehavior
class ATL_NO_VTABLE FolderBrowserUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<FolderBrowserUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    FolderBrowserUIBehavior();
    virtual ~FolderBrowserUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_FOLDER_BROWSER_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(FolderBrowserUIBehavior)

    BEGIN_COM_MAP(FolderBrowserUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// ColorPickerUIBehavior
class ATL_NO_VTABLE ColorPickerUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<ColorPickerUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    ColorPickerUIBehavior();
    virtual ~ColorPickerUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_COLOR_PICKER_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(ColorPickerUIBehavior)

    BEGIN_COM_MAP(ColorPickerUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// FontPickerUIBehavior
class ATL_NO_VTABLE FontPickerUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<FontPickerUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    FontPickerUIBehavior();
    virtual ~FontPickerUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_FONT_PICKER_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(FontPickerUIBehavior)

    BEGIN_COM_MAP(FontPickerUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// CropUIBehavior
class ATL_NO_VTABLE CropUIBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CropUIBehavior>,
    public IDuiBehaviorImpl
{
public:
    CropUIBehavior();
    virtual ~CropUIBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_CROP_UI_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(CropUIBehavior)

    BEGIN_COM_MAP(CropUIBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

private:
    IDuiElement* m_pElement;
};

} // namespace Sundance

#endif // SUNDANCE_UI_BEHAVIOR_CLASSES_H
