/*
 * UIBehaviorClasses.cpp
 *
 * Stub implementations of additional UI behavior RTTI classes for the
 * Sundance DirectUI layer. Each class provides minimal constructor,
 * destructor, and IDuiBehavior overrides returning S_OK or E_NOTIMPL.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "UIBehaviorClasses.h"
#include "../SundanceApp/SundanceAppMain.h"

namespace Sundance
{

// ============================================================================
// TimelineBehaviorSelection
// ============================================================================
TimelineBehaviorSelection::TimelineBehaviorSelection() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorSelection::~TimelineBehaviorSelection() { m_pElement = NULL; }

HRESULT TimelineBehaviorSelection::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorSelection::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorSelection::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (m_pElement)
        {
            RECT rcBounds;
            m_pElement->GetBounds(&rcBounds);
            if (PtInRect(&rcBounds, pt))
            {
                if (pbHandled) *pbHandled = TRUE;
            }
        }
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (m_pElement)
        {
            RECT rcBounds;
            m_pElement->GetBounds(&rcBounds);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            HCURSOR hCursor = LoadCursor(NULL, PtInRect(&rcBounds, pt) ? IDC_HAND : IDC_ARROW);
            if (hCursor) ::SetCursor(hCursor);
        }
        break;
    }
    }

    return S_OK;
}

// ============================================================================
// TimelineBehaviorDragDrop
// ============================================================================
TimelineBehaviorDragDrop::TimelineBehaviorDragDrop() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorDragDrop::~TimelineBehaviorDragDrop() { m_pElement = NULL; }

HRESULT TimelineBehaviorDragDrop::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorDragDrop::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorDragDrop::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        m_bDragging = FALSE;
        m_ptDragStart = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (pbHandled) *pbHandled = TRUE;
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (m_bDragging)
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            int dx = pt.x - m_ptDragStart.x;
            int dy = pt.y - m_ptDragStart.y;
            if (abs(dx) > GetSystemMetrics(SM_CXDRAG) || abs(dy) > GetSystemMetrics(SM_CYDRAG))
            {
                POINT ptScreen = m_ptDragStart;
                ClientToScreen(NULL, &ptScreen);
                IDropTarget* pDropTarget = NULL;
                DragDetect(NULL, ptScreen);
            }
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (m_bDragging)
        {
            m_bDragging = FALSE;
            DragLeave(NULL);
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    }

    return S_OK;
}

// ============================================================================
// TimelineBehaviorResize
// ============================================================================
TimelineBehaviorResize::TimelineBehaviorResize() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorResize::~TimelineBehaviorResize() { m_pElement = NULL; }

HRESULT TimelineBehaviorResize::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorResize::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorResize::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_MOUSEMOVE:
    {
        if (m_pElement && m_bResizing)
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            LONG newWidth = pt.x - m_rcOriginal.left;
            if (newWidth < m_minWidth) newWidth = m_minWidth;
            m_rcOriginal.right = m_rcOriginal.left + newWidth;
            if (m_pTimelineElement) m_pTimelineElement->SetBounds(&m_rcOriginal);
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (m_pElement)
        {
            m_pElement->GetBounds(&m_rcOriginal);
            RECT rcRightEdge = m_rcOriginal;
            rcRightEdge.left = rcRightEdge.right - GetSystemMetrics(SM_CXEDGE) * 2;
            if (PtInRect(&rcRightEdge, pt))
            {
                m_bResizing = TRUE;
                SetCapture(m_hWnd);
                if (pbHandled) *pbHandled = TRUE;
            }
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (m_bResizing)
        {
            m_bResizing = FALSE;
            ReleaseCapture();
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    }

    return S_OK;
}

// ============================================================================
// TimelineBehaviorTrim
// ============================================================================
TimelineBehaviorTrim::TimelineBehaviorTrim() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorTrim::~TimelineBehaviorTrim() { m_pElement = NULL; }

HRESULT TimelineBehaviorTrim::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorTrim::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorTrim::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_MOUSEMOVE:
    {
        if (m_bTrimming)
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            LONG delta = pt.x - m_ptTrimStart.x;
            if (m_bTrimmingStart)
            {
                LONG newLeft = m_rcOriginal.left + delta;
                if (newLeft < m_rcOriginal.right - m_minTrimWidth)
                    m_rcTrimmed.left = newLeft;
            }
            else
            {
                LONG newRight = m_rcOriginal.right + delta;
                if (newRight > m_rcOriginal.left + m_minTrimWidth)
                    m_rcTrimmed.right = newRight;
            }
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (m_pElement)
        {
            m_pElement->GetBounds(&m_rcOriginal);
            m_rcTrimmed = m_rcOriginal;
            RECT rcLeftEdge = m_rcOriginal;
            rcLeftEdge.right = rcLeftEdge.left + GetSystemMetrics(SM_CXEDGE) * 2;
            RECT rcRightEdge = m_rcOriginal;
            rcRightEdge.left = rcRightEdge.right - GetSystemMetrics(SM_CXEDGE) * 2;
            if (PtInRect(&rcLeftEdge, pt))
            {
                m_bTrimming = TRUE;
                m_bTrimmingStart = TRUE;
                m_ptTrimStart = pt;
                SetCapture(m_hWnd);
                if (pbHandled) *pbHandled = TRUE;
            }
            else if (PtInRect(&rcRightEdge, pt))
            {
                m_bTrimming = TRUE;
                m_bTrimmingStart = FALSE;
                m_ptTrimStart = pt;
                SetCapture(m_hWnd);
                if (pbHandled) *pbHandled = TRUE;
            }
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (m_bTrimming)
        {
            m_bTrimming = FALSE;
            ReleaseCapture();
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    }

    return S_OK;
}

// ============================================================================
// TimelineBehaviorSplit
// ============================================================================
TimelineBehaviorSplit::TimelineBehaviorSplit() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorSplit::~TimelineBehaviorSplit() { m_pElement = NULL; }

HRESULT TimelineBehaviorSplit::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorSplit::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorSplit::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (m_pElement)
        {
            RECT rcBounds;
            m_pElement->GetBounds(&rcBounds);
            if (PtInRect(&rcBounds, pt))
            {
                m_ptSplitPoint = pt;
                if (m_pTimelineElement)
                {
                    m_pTimelineElement->SplitAtPosition(pt.x - rcBounds.left);
                }
                if (pbHandled) *pbHandled = TRUE;
            }
        }
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (m_pElement)
        {
            RECT rcBounds;
            m_pElement->GetBounds(&rcBounds);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            if (PtInRect(&rcBounds, pt))
            {
                HCURSOR hCursor = LoadCursor(NULL, IDC_CROSS);
                if (hCursor) ::SetCursor(hCursor);
            }
        }
        break;
    }
    }

    return S_OK;
}

// ============================================================================
// TimelineBehaviorReorder
// ============================================================================
TimelineBehaviorReorder::TimelineBehaviorReorder() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorReorder::~TimelineBehaviorReorder() { m_pElement = NULL; }

HRESULT TimelineBehaviorReorder::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorReorder::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorReorder::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        m_bReordering = FALSE;
        m_ptDragStart = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (m_pElement)
        {
            m_pElement->GetBounds(&m_rcOriginal);
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (!m_bReordering)
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            int dx = abs(pt.x - m_ptDragStart.x);
            int dy = abs(pt.y - m_ptDragStart.y);
            if (dx > GetSystemMetrics(SM_CXDRAG) || dy > GetSystemMetrics(SM_CYDRAG))
            {
                m_bReordering = TRUE;
                m_nStartIndex = m_nOriginalIndex;
            }
        }
        if (m_bReordering)
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            if (m_pTimelineElement)
            {
                m_pTimelineElement->ReorderToPosition(pt.x);
            }
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (m_bReordering)
        {
            m_bReordering = FALSE;
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    }

    return S_OK;
}

// ============================================================================
// TimelineBehaviorZoom
// ============================================================================
TimelineBehaviorZoom::TimelineBehaviorZoom() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorZoom::~TimelineBehaviorZoom() { m_pElement = NULL; }

HRESULT TimelineBehaviorZoom::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorZoom::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorZoom::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_MOUSEWHEEL:
    {
        short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (zDelta > 0)
        {
            if (m_pTimelineElement) m_pTimelineElement->ZoomIn();
        }
        else
        {
            if (m_pTimelineElement) m_pTimelineElement->ZoomOut();
        }
        if (pbHandled) *pbHandled = TRUE;
        break;
    }
    case WM_LBUTTONDOWN:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        m_bZooming = TRUE;
        m_ptZoomStart = pt;
        m_dStartZoomLevel = m_dCurrentZoomLevel;
        SetCapture(m_hWnd);
        if (pbHandled) *pbHandled = TRUE;
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (m_bZooming)
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            int dy = m_ptZoomStart.y - pt.y;
            double newZoom = m_dStartZoomLevel * (1.0 + dy * 0.005);
            if (newZoom < m_dMinZoom) newZoom = m_dMinZoom;
            if (newZoom > m_dMaxZoom) newZoom = m_dMaxZoom;
            m_dCurrentZoomLevel = newZoom;
            if (m_pTimelineElement) m_pTimelineElement->SetZoomLevel(m_dCurrentZoomLevel);
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (m_bZooming)
        {
            m_bZooming = FALSE;
            ReleaseCapture();
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    }

    return S_OK;
}

// ============================================================================
// TimelineBehaviorScroll
// ============================================================================
TimelineBehaviorScroll::TimelineBehaviorScroll() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorScroll::~TimelineBehaviorScroll() { m_pElement = NULL; }

HRESULT TimelineBehaviorScroll::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorScroll::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorScroll::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_MOUSEWHEEL:
    {
        short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int scrollAmount = -zDelta / WHEEL_DELTA * m_nScrollStep;
        if (m_pTimelineElement) m_pTimelineElement->Scroll(scrollAmount, 0);
        if (pbHandled) *pbHandled = TRUE;
        break;
    }
    case WM_LBUTTONDOWN:
    {
        m_bScrolling = TRUE;
        m_ptScrollStart = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        SetCapture(m_hWnd);
        if (pbHandled) *pbHandled = TRUE;
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (m_bScrolling)
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            int dx = pt.x - m_ptScrollStart.x;
            int dy = pt.y - m_ptScrollStart.y;
            if (m_pTimelineElement) m_pTimelineElement->Scroll(-dx, -dy);
            m_ptScrollStart = pt;
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (m_bScrolling)
        {
            m_bScrolling = FALSE;
            ReleaseCapture();
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    }

    return S_OK;
}

// ============================================================================
// TimelineBehaviorSnap
// ============================================================================
TimelineBehaviorSnap::TimelineBehaviorSnap() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorSnap::~TimelineBehaviorSnap() { m_pElement = NULL; }

HRESULT TimelineBehaviorSnap::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorSnap::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorSnap::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_MOUSEMOVE:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (m_pTimelineElement)
        {
            LONGLONG snapPosition = 0;
            BOOL bSnapped = m_pTimelineElement->FindSnapPoint(pt.x, &snapPosition);
            if (bSnapped)
            {
                m_bSnapping = TRUE;
                m_llSnapPosition = snapPosition;
                if (pbHandled) *pbHandled = TRUE;
            }
            else
            {
                m_bSnapping = FALSE;
            }
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (m_bSnapping && m_pTimelineElement)
        {
            m_pTimelineElement->ApplySnap(m_llSnapPosition);
            m_bSnapping = FALSE;
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    }

    return S_OK;
}

// ============================================================================
// RibbonCommandHandler
// ============================================================================
RibbonCommandHandler::RibbonCommandHandler() : m_pElement(NULL) {}
RibbonCommandHandler::~RibbonCommandHandler() { m_pElement = NULL; }

HRESULT RibbonCommandHandler::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonCommandHandler::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonEventHandler
// ============================================================================
RibbonEventHandler::RibbonEventHandler() : m_pElement(NULL) {}
RibbonEventHandler::~RibbonEventHandler() { m_pElement = NULL; }

HRESULT RibbonEventHandler::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonEventHandler::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonSiteBehavior
// ============================================================================
RibbonSiteBehavior::RibbonSiteBehavior() : m_pElement(NULL) {}
RibbonSiteBehavior::~RibbonSiteBehavior() { m_pElement = NULL; }

HRESULT RibbonSiteBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonSiteBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonContextualUIBehavior
// ============================================================================
RibbonContextualUIBehavior::RibbonContextualUIBehavior() : m_pElement(NULL) {}
RibbonContextualUIBehavior::~RibbonContextualUIBehavior() { m_pElement = NULL; }

HRESULT RibbonContextualUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonContextualUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonQuickAccessBehavior
// ============================================================================
RibbonQuickAccessBehavior::RibbonQuickAccessBehavior() : m_pElement(NULL) {}
RibbonQuickAccessBehavior::~RibbonQuickAccessBehavior() { m_pElement = NULL; }

HRESULT RibbonQuickAccessBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonQuickAccessBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonGalleryBehavior
// ============================================================================
RibbonGalleryBehavior::RibbonGalleryBehavior() : m_pElement(NULL) {}
RibbonGalleryBehavior::~RibbonGalleryBehavior() { m_pElement = NULL; }

HRESULT RibbonGalleryBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonGalleryBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonRecentItemsBehavior
// ============================================================================
RibbonRecentItemsBehavior::RibbonRecentItemsBehavior() : m_pElement(NULL) {}
RibbonRecentItemsBehavior::~RibbonRecentItemsBehavior() { m_pElement = NULL; }

HRESULT RibbonRecentItemsBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonRecentItemsBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SelectionRootImpl
// ============================================================================
SelectionRootImpl::SelectionRootImpl() : m_pElement(NULL), m_pAppMain(NULL) {}
SelectionRootImpl::~SelectionRootImpl() { m_pElement = NULL; }

HRESULT SelectionRootImpl::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SelectionRootImpl::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT SelectionRootImpl::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (m_pAppMain)
        {
            m_pAppMain->ClearSelection();
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            if (m_pAppMain)
            {
                m_pAppMain->ClearSelection();
                if (pbHandled) *pbHandled = TRUE;
            }
        }
        break;
    }
    }

    return S_OK;
}

// ============================================================================
// SelectionManager
// ============================================================================
SelectionManager::SelectionManager() : m_pElement(NULL), m_pAppMain(NULL) {}
SelectionManager::~SelectionManager() { m_pElement = NULL; }

HRESULT SelectionManager::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SelectionManager::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SelectionRange
// ============================================================================
SelectionRange::SelectionRange() : m_pElement(NULL) {}
SelectionRange::~SelectionRange() { m_pElement = NULL; }

HRESULT SelectionRange::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT SelectionRange::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT SelectionRange::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 149, 237));
    HBRUSH hBrush = CreateSolidBrush(RGB(100, 149, 237));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    Rectangle(hdc, prcBounds->left, prcBounds->top, prcBounds->right, prcBounds->bottom);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
    DeleteObject(hBrush);

    return S_OK;
}

// ============================================================================
// SelectionContext
// ============================================================================
SelectionContext::SelectionContext() : m_pElement(NULL) {}
SelectionContext::~SelectionContext() { m_pElement = NULL; }

HRESULT SelectionContext::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT SelectionContext::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SelectionUI
// ============================================================================
SelectionUI::SelectionUI() : m_pElement(NULL) {}
SelectionUI::~SelectionUI() { m_pElement = NULL; }

HRESULT SelectionUI::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT SelectionUI::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT SelectionUI::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    HPEN hPen = CreatePen(PS_DOT, 1, RGB(0, 120, 215));
    HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    Rectangle(hdc, prcBounds->left, prcBounds->top, prcBounds->right, prcBounds->bottom);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);

    return S_OK;
}

// ============================================================================
// EditorToolbar
// ============================================================================
EditorToolbar::EditorToolbar() : m_pElement(NULL), m_pAppMain(NULL) {}
EditorToolbar::~EditorToolbar() { m_pElement = NULL; }

HRESULT EditorToolbar::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT EditorToolbar::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// EditorCommandHandler
// ============================================================================
EditorCommandHandler::EditorCommandHandler() : m_pElement(NULL), m_pAppMain(NULL) {}
EditorCommandHandler::~EditorCommandHandler() { m_pElement = NULL; }

HRESULT EditorCommandHandler::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT EditorCommandHandler::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// EditorState
// ============================================================================
EditorState::EditorState() : m_pElement(NULL), m_pAppMain(NULL) {}
EditorState::~EditorState() { m_pElement = NULL; }

HRESULT EditorState::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT EditorState::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SundanceRibbonElementBehavior
// ============================================================================
SundanceRibbonElementBehavior::SundanceRibbonElementBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
SundanceRibbonElementBehavior::~SundanceRibbonElementBehavior() { m_pElement = NULL; }

HRESULT SundanceRibbonElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundanceRibbonElementBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SundanceStatusBarElementBehavior
// ============================================================================
SundanceStatusBarElementBehavior::SundanceStatusBarElementBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
SundanceStatusBarElementBehavior::~SundanceStatusBarElementBehavior() { m_pElement = NULL; }

HRESULT SundanceStatusBarElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundanceStatusBarElementBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SundanceTimelineElementBehavior
// ============================================================================
SundanceTimelineElementBehavior::SundanceTimelineElementBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
SundanceTimelineElementBehavior::~SundanceTimelineElementBehavior() { m_pElement = NULL; }

HRESULT SundanceTimelineElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundanceTimelineElementBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SundancePreviewElementBehavior
// ============================================================================
SundancePreviewElementBehavior::SundancePreviewElementBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
SundancePreviewElementBehavior::~SundancePreviewElementBehavior() { m_pElement = NULL; }

HRESULT SundancePreviewElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundancePreviewElementBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SundancePropertyElementBehavior
// ============================================================================
SundancePropertyElementBehavior::SundancePropertyElementBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
SundancePropertyElementBehavior::~SundancePropertyElementBehavior() { m_pElement = NULL; }

HRESULT SundancePropertyElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundancePropertyElementBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// WebcamUIBehavior
// ============================================================================
WebcamUIBehavior::WebcamUIBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
WebcamUIBehavior::~WebcamUIBehavior() { m_pElement = NULL; }

HRESULT WebcamUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT WebcamUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// WebcamPreviewBehavior
// ============================================================================
WebcamPreviewBehavior::WebcamPreviewBehavior() : m_pElement(NULL) {}
WebcamPreviewBehavior::~WebcamPreviewBehavior() { m_pElement = NULL; }

HRESULT WebcamPreviewBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT WebcamPreviewBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT WebcamPreviewBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    HBRUSH hBrush = CreateSolidBrush(RGB(32, 32, 32));
    FillRect(hdc, prcBounds, hBrush);
    DeleteObject(hBrush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(150, 150, 150));
    DrawText(hdc, L"Webcam Preview", -1, const_cast<RECT*>(prcBounds),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    return S_OK;
}

// ============================================================================
// WebcamCaptureBehavior
// ============================================================================
WebcamCaptureBehavior::WebcamCaptureBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
WebcamCaptureBehavior::~WebcamCaptureBehavior() { m_pElement = NULL; }

HRESULT WebcamCaptureBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT WebcamCaptureBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// NarrationUIBehavior
// ============================================================================
NarrationUIBehavior::NarrationUIBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
NarrationUIBehavior::~NarrationUIBehavior() { m_pElement = NULL; }

HRESULT NarrationUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT NarrationUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// NarrationRecordBehavior
// ============================================================================
NarrationRecordBehavior::NarrationRecordBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
NarrationRecordBehavior::~NarrationRecordBehavior() { m_pElement = NULL; }

HRESULT NarrationRecordBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT NarrationRecordBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// NarrationPlaybackBehavior
// ============================================================================
NarrationPlaybackBehavior::NarrationPlaybackBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
NarrationPlaybackBehavior::~NarrationPlaybackBehavior() { m_pElement = NULL; }

HRESULT NarrationPlaybackBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT NarrationPlaybackBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// UserEncodeProfileManager
// ============================================================================
UserEncodeProfileManager::UserEncodeProfileManager() : m_pElement(NULL), m_pAppMain(NULL) {}
UserEncodeProfileManager::~UserEncodeProfileManager() { m_pElement = NULL; }

HRESULT UserEncodeProfileManager::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT UserEncodeProfileManager::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// UserEncodeProfilePreset
// ============================================================================
UserEncodeProfilePreset::UserEncodeProfilePreset() : m_pElement(NULL) {}
UserEncodeProfilePreset::~UserEncodeProfilePreset() { m_pElement = NULL; }

HRESULT UserEncodeProfilePreset::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT UserEncodeProfilePreset::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// UserEncodeProfileCustom
// ============================================================================
UserEncodeProfileCustom::UserEncodeProfileCustom() : m_pElement(NULL) {}
UserEncodeProfileCustom::~UserEncodeProfileCustom() { m_pElement = NULL; }

HRESULT UserEncodeProfileCustom::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT UserEncodeProfileCustom::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// UserEncodeProfileRegistry
// ============================================================================
UserEncodeProfileRegistry::UserEncodeProfileRegistry() : m_pElement(NULL) {}
UserEncodeProfileRegistry::~UserEncodeProfileRegistry() { m_pElement = NULL; }

HRESULT UserEncodeProfileRegistry::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT UserEncodeProfileRegistry::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogBaseBehavior
// ============================================================================
PublishDialogBaseBehavior::PublishDialogBaseBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogBaseBehavior::~PublishDialogBaseBehavior() { m_pElement = NULL; }

HRESULT PublishDialogBaseBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogBaseBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogYouTubeBehavior
// ============================================================================
PublishDialogYouTubeBehavior::PublishDialogYouTubeBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogYouTubeBehavior::~PublishDialogYouTubeBehavior() { m_pElement = NULL; }

HRESULT PublishDialogYouTubeBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogYouTubeBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogFacebookBehavior
// ============================================================================
PublishDialogFacebookBehavior::PublishDialogFacebookBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogFacebookBehavior::~PublishDialogFacebookBehavior() { m_pElement = NULL; }

HRESULT PublishDialogFacebookBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogFacebookBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogSkyDriveBehavior
// ============================================================================
PublishDialogSkyDriveBehavior::PublishDialogSkyDriveBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogSkyDriveBehavior::~PublishDialogSkyDriveBehavior() { m_pElement = NULL; }

HRESULT PublishDialogSkyDriveBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogSkyDriveBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogEmailBehavior
// ============================================================================
PublishDialogEmailBehavior::PublishDialogEmailBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogEmailBehavior::~PublishDialogEmailBehavior() { m_pElement = NULL; }

HRESULT PublishDialogEmailBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogEmailBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogLocalFileBehavior
// ============================================================================
PublishDialogLocalFileBehavior::PublishDialogLocalFileBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogLocalFileBehavior::~PublishDialogLocalFileBehavior() { m_pElement = NULL; }

HRESULT PublishDialogLocalFileBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogLocalFileBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// ProgressBarUIBehavior
// ============================================================================
ProgressBarUIBehavior::ProgressBarUIBehavior() : m_pElement(NULL) {}
ProgressBarUIBehavior::~ProgressBarUIBehavior() { m_pElement = NULL; }

HRESULT ProgressBarUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ProgressBarUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT ProgressBarUIBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    int width = prcBounds->right - prcBounds->left;
    int height = prcBounds->bottom - prcBounds->top;

    HBRUSH hBgBrush = CreateSolidBrush(RGB(230, 230, 230));
    RECT rcBg = *prcBounds;
    FillRect(hdc, &rcBg, hBgBrush);
    DeleteObject(hBgBrush);

    HPEN hBorderPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
    Rectangle(hdc, prcBounds->left, prcBounds->top, prcBounds->right, prcBounds->bottom);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBorderPen);

    HBRUSH hBarBrush = CreateSolidBrush(RGB(0, 120, 215));
    RECT rcBar = *prcBounds;
    rcBar.right = rcBar.left + width / 2;
    InflateRect(&rcBar, -1, -1);
    FillRect(hdc, &rcBar, hBarBrush);
    DeleteObject(hBarBrush);

    return S_OK;
}

// ============================================================================
// ErrorDialogUIBehavior
// ============================================================================
ErrorDialogUIBehavior::ErrorDialogUIBehavior() : m_pElement(NULL) {}
ErrorDialogUIBehavior::~ErrorDialogUIBehavior() { m_pElement = NULL; }

HRESULT ErrorDialogUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ErrorDialogUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// WarningDialogUIBehavior
// ============================================================================
WarningDialogUIBehavior::WarningDialogUIBehavior() : m_pElement(NULL) {}
WarningDialogUIBehavior::~WarningDialogUIBehavior() { m_pElement = NULL; }

HRESULT WarningDialogUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT WarningDialogUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// InfoDialogUIBehavior
// ============================================================================
InfoDialogUIBehavior::InfoDialogUIBehavior() : m_pElement(NULL) {}
InfoDialogUIBehavior::~InfoDialogUIBehavior() { m_pElement = NULL; }

HRESULT InfoDialogUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT InfoDialogUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// ConfirmationDialogUIBehavior
// ============================================================================
ConfirmationDialogUIBehavior::ConfirmationDialogUIBehavior() : m_pElement(NULL) {}
ConfirmationDialogUIBehavior::~ConfirmationDialogUIBehavior() { m_pElement = NULL; }

HRESULT ConfirmationDialogUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ConfirmationDialogUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// FileBrowserUIBehavior
// ============================================================================
FileBrowserUIBehavior::FileBrowserUIBehavior() : m_pElement(NULL) {}
FileBrowserUIBehavior::~FileBrowserUIBehavior() { m_pElement = NULL; }

HRESULT FileBrowserUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT FileBrowserUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// FolderBrowserUIBehavior
// ============================================================================
FolderBrowserUIBehavior::FolderBrowserUIBehavior() : m_pElement(NULL) {}
FolderBrowserUIBehavior::~FolderBrowserUIBehavior() { m_pElement = NULL; }

HRESULT FolderBrowserUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT FolderBrowserUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// ColorPickerUIBehavior
// ============================================================================
ColorPickerUIBehavior::ColorPickerUIBehavior() : m_pElement(NULL) {}
ColorPickerUIBehavior::~ColorPickerUIBehavior() { m_pElement = NULL; }

HRESULT ColorPickerUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ColorPickerUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// FontPickerUIBehavior
// ============================================================================
FontPickerUIBehavior::FontPickerUIBehavior() : m_pElement(NULL) {}
FontPickerUIBehavior::~FontPickerUIBehavior() { m_pElement = NULL; }

HRESULT FontPickerUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT FontPickerUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// CropUIBehavior
// ============================================================================
CropUIBehavior::CropUIBehavior() : m_pElement(NULL) {}
CropUIBehavior::~CropUIBehavior() { m_pElement = NULL; }

HRESULT CropUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT CropUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT CropUIBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    int width = prcBounds->right - prcBounds->left;
    int height = prcBounds->bottom - prcBounds->top;

    int insetX = width / 6;
    int insetY = height / 6;

    HBRUSH hOverlayBrush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hOverlayBrush);

    RECT rcTop = *prcBounds;
    rcTop.bottom = rcTop.top + insetY;
    FillRect(hdc, &rcTop, hOverlayBrush);

    RECT rcBottom = *prcBounds;
    rcBottom.top = rcBottom.bottom - insetY;
    FillRect(hdc, &rcBottom, hOverlayBrush);

    RECT rcLeft = *prcBounds;
    rcLeft.top += insetY;
    rcLeft.bottom -= insetY;
    rcLeft.right = rcLeft.left + insetX;
    FillRect(hdc, &rcLeft, hOverlayBrush);

    RECT rcRight = *prcBounds;
    rcRight.top += insetY;
    rcRight.bottom -= insetY;
    rcRight.left = rcRight.right - insetX;
    FillRect(hdc, &rcRight, hOverlayBrush);

    SelectObject(hdc, hOldBrush);
    DeleteObject(hOverlayBrush);

    HPEN hGuidelinePen = CreatePen(PS_DOT, 1, RGB(255, 255, 255));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hGuidelinePen);
    SetBkColor(hdc, RGB(0, 0, 0));

    RECT rcCrop = *prcBounds;
    rcCrop.left += insetX;
    rcCrop.right -= insetX;
    rcCrop.top += insetY;
    rcCrop.bottom -= insetY;

    MoveToEx(hdc, rcCrop.left + (rcCrop.right - rcCrop.left) / 3, rcCrop.top, NULL);
    LineTo(hdc, rcCrop.left + (rcCrop.right - rcCrop.left) / 3, rcCrop.bottom);
    MoveToEx(hdc, rcCrop.left + 2 * (rcCrop.right - rcCrop.left) / 3, rcCrop.top, NULL);
    LineTo(hdc, rcCrop.left + 2 * (rcCrop.right - rcCrop.left) / 3, rcCrop.bottom);
    MoveToEx(hdc, rcCrop.left, rcCrop.top + (rcCrop.bottom - rcCrop.top) / 3, NULL);
    LineTo(hdc, rcCrop.right, rcCrop.top + (rcCrop.bottom - rcCrop.top) / 3);
    MoveToEx(hdc, rcCrop.left, rcCrop.top + 2 * (rcCrop.bottom - rcCrop.top) / 3, NULL);
    LineTo(hdc, rcCrop.right, rcCrop.top + 2 * (rcCrop.bottom - rcCrop.top) / 3);

    SelectObject(hdc, hOldPen);
    DeleteObject(hGuidelinePen);

    HPEN hBorderPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
    HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH hNullOld = (HBRUSH)SelectObject(hdc, hNullBrush);
    Rectangle(hdc, rcCrop.left, rcCrop.top, rcCrop.right, rcCrop.bottom);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hNullOld);
    DeleteObject(hBorderPen);

    return S_OK;
}

} // namespace Sundance
