/*
 * TimelineBehavior.cpp
 *
 * Implementation of timeline UI behavior classes for the Sundance DirectUI
 * layer. Implements the main timeline control, individual items, track items,
 * input handling, instruction overlays, and selection root.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "TimelineBehavior.h"
#include "../SundanceApp/SundanceAppMain.h"

namespace Sundance
{

// ============================================================================
// TimelineBaseBehavior
// ============================================================================
TimelineBaseBehavior::TimelineBaseBehavior()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_nScrollOffset(0)
    , m_nPixelsPerUnit(2)
    , m_nTimelineWidth(0)
    , m_nItemHeight(60)
{
}

TimelineBaseBehavior::~TimelineBaseBehavior()
{
    m_pElement = NULL;
    m_pAppMain = NULL;
}

HRESULT TimelineBaseBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement)
        return E_POINTER;

    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBaseBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    m_pElement = NULL;
    m_selectedItems.clear();
    return S_OK;
}

HRESULT TimelineBaseBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    if (pbHandled)
        *pbHandled = FALSE;

    return S_OK;
}

SundanceAppMain* TimelineBaseBehavior::GetAppMain() const throw()
{
    return m_pAppMain;
}

void TimelineBaseBehavior::SetAppMain(SundanceAppMain* pAppMain) throw()
{
    m_pAppMain = pAppMain;
}

bool TimelineBaseBehavior::IsItemSelected(DWORD dwItemId) const
{
    return m_selectedItems.find(dwItemId) != m_selectedItems.end();
}

void TimelineBaseBehavior::SelectItem(DWORD dwItemId, bool bSelect)
{
    if (bSelect)
    {
        m_selectedItems.insert(dwItemId);
    }
    else
    {
        m_selectedItems.erase(dwItemId);
    }
    OnSelectionChanged();
}

void TimelineBaseBehavior::ClearSelection()
{
    m_selectedItems.clear();
    OnSelectionChanged();
}

DWORD TimelineBaseBehavior::GetSelectedCount() const throw()
{
    return static_cast<DWORD>(m_selectedItems.size());
}

int TimelineBaseBehavior::PixelsToTimelineUnits(int xPixels) const
{
    if (m_nPixelsPerUnit <= 0)
        return 0;
    return (xPixels + m_nScrollOffset) / m_nPixelsPerUnit;
}

int TimelineBaseBehavior::TimelineUnitsToPixels(int dwUnits) const
{
    return (dwUnits * m_nPixelsPerUnit) - m_nScrollOffset;
}

void TimelineBaseBehavior::OnLayoutChanged()
{
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
}

void TimelineBaseBehavior::OnSelectionChanged()
{
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
}

// ============================================================================
// TimelineBehavior
// ============================================================================
TimelineBehavior::TimelineBehavior()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_dPlaybackPosition(0.0)
    , m_bPlaybackActive(false)
    , m_nZoomLevel(3)
    , m_nScrollX(0)
    , m_nScrollY(0)
    , m_nTimelineHeight(0)
    , m_nVisibleWidth(0)
{
}

TimelineBehavior::~TimelineBehavior()
{
    m_pElement = NULL;
    m_pAppMain = NULL;
}

HRESULT TimelineBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement)
        return E_POINTER;

    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    m_pElement = NULL;
    return S_OK;
}

HRESULT TimelineBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds)
        return E_POINTER;

    CalculateLayout();

    PaintTimeRuler(hdc, prcBounds);
    PaintTracks(hdc, prcBounds);
    PaintPlaybackCursor(hdc, prcBounds);

    return S_OK;
}

HRESULT TimelineBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled)
        *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_MOUSEWHEEL:
    {
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int nNewZoom = m_nZoomLevel + (zDelta > 0 ? 1 : -1);
        if (nNewZoom < kMinZoomLevel) nNewZoom = kMinZoomLevel;
        if (nNewZoom > kMaxZoomLevel) nNewZoom = kMaxZoomLevel;

        if (nNewZoom != m_nZoomLevel)
        {
            m_nZoomLevel = nNewZoom;
            CalculateLayout();
            if (m_pElement)
                m_pElement->InvalidateRect(NULL);
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }

    case WM_HSCROLL:
    {
        int nScrollCode = LOWORD(wParam);
        switch (nScrollCode)
        {
        case SB_LINELEFT:       m_nScrollX -= 50; break;
        case SB_LINERIGHT:      m_nScrollX += 50; break;
        case SB_PAGELEFT:       m_nScrollX -= m_nVisibleWidth; break;
        case SB_PAGERIGHT:      m_nScrollX += m_nVisibleWidth; break;
        case SB_THUMBTRACK:     m_nScrollX = HIWORD(wParam); break;
        }

        if (m_nScrollX < 0) m_nScrollX = 0;
        CalculateLayout();

        if (m_pElement)
            m_pElement->InvalidateRect(NULL);
        if (pbHandled) *pbHandled = TRUE;
        break;
    }
    }

    return S_OK;
}

HRESULT TimelineBehavior::OnBehaviorChanged()
{
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
    return S_OK;
}

HRESULT TimelineBehavior::SetPlaybackPosition(double dPositionSeconds)
{
    m_dPlaybackPosition = dPositionSeconds;

    if (m_pElement)
        m_pElement->InvalidateRect(NULL);

    return S_OK;
}

HRESULT TimelineBehavior::SetZoomLevel(int nZoomLevel)
{
    if (nZoomLevel < kMinZoomLevel || nZoomLevel > kMaxZoomLevel)
        return E_INVALIDARG;

    m_nZoomLevel = nZoomLevel;
    CalculateLayout();

    if (m_pElement)
        m_pElement->InvalidateRect(NULL);

    return S_OK;
}

HRESULT TimelineBehavior::EnsureItemVisible(DWORD dwItemId)
{
    UNREFERENCED_PARAMETER(dwItemId);
    return S_OK;
}

HRESULT TimelineBehavior::ScrollToPosition(int nPosition)
{
    m_nScrollX = nPosition;
    if (m_nScrollX < 0) m_nScrollX = 0;

    if (m_pElement)
        m_pElement->InvalidateRect(NULL);

    return S_OK;
}

int TimelineBehavior::GetTrackCount() const throw()
{
    return 4;
}

int TimelineBehavior::GetTrackHeight(int nTrackIndex) const
{
    UNREFERENCED_PARAMETER(nTrackIndex);
    return kDefaultTrackHeight;
}

bool TimelineBehavior::IsTrackVisible(int nTrackIndex) const
{
    return (nTrackIndex >= 0 && nTrackIndex < GetTrackCount());
}

void TimelineBehavior::CalculateLayout()
{
    int nPPU = GetPixelsPerUnit();
    if (m_pAppMain && m_pAppMain->GetProject())
    {
        // Query total project duration from the project model
        // to compute total timeline width
    }

    m_nTimelineHeight = GetTrackCount() * kDefaultTrackHeight;
}

int TimelineBehavior::GetPixelsPerUnit() const
{
    return kPixelsPerUnitAtZoom1 << (m_nZoomLevel - 1);
}

int TimelineBehavior::GetTotalTimelineWidth() const
{
    return GetPixelsPerUnit() * 10000;
}

void TimelineBehavior::PaintPlaybackCursor(HDC hdc, const RECT* prcBounds)
{
    int nCursorX = static_cast<int>(m_dPlaybackPosition * GetPixelsPerUnit()) - m_nScrollX;

    if (nCursorX >= prcBounds->left && nCursorX <= prcBounds->right)
    {
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));

        MoveToEx(hdc, nCursorX, prcBounds->top, NULL);
        LineTo(hdc, nCursorX, prcBounds->bottom);

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

void TimelineBehavior::PaintTimeRuler(HDC hdc, const RECT* prcBounds)
{
    RECT rcRuler = *prcBounds;
    rcRuler.bottom = rcRuler.top + 24;

    FillRect(hdc, &rcRuler, static_cast<HBRUSH>(GetStockObject(LTGRAY_BRUSH)));

    int nPPU = GetPixelsPerUnit();
    if (nPPU <= 0) return;

    int nStartUnit = m_nScrollX / nPPU;
    int nEndUnit = (m_nScrollX + (prcBounds->right - prcBounds->left)) / nPPU;

    for (int i = nStartUnit; i <= nEndUnit; ++i)
    {
        int x = (i * nPPU) - m_nScrollX;
        if (x >= prcBounds->left && x <= prcBounds->right)
        {
            int nHeight = (i % 10 == 0) ? 12 : (i % 5 == 0) ? 8 : 4;
            MoveToEx(hdc, x, rcRuler.bottom - nHeight, NULL);
            LineTo(hdc, x, rcRuler.bottom);
        }
    }
}

void TimelineBehavior::PaintTracks(HDC hdc, const RECT* prcBounds)
{
    int nTrackTop = prcBounds->top + 24;

    for (int i = 0; i < GetTrackCount(); ++i)
    {
        RECT rcTrack = *prcBounds;
        rcTrack.top = nTrackTop + (i * kDefaultTrackHeight);
        rcTrack.bottom = rcTrack.top + kDefaultTrackHeight;

        // Alternate track background colors
        HBRUSH hBrush = CreateSolidBrush(
            (i % 2 == 0) ? RGB(240, 240, 240) : RGB(255, 255, 255));
        FillRect(hdc, &rcTrack, hBrush);
        DeleteObject(hBrush);

        // Draw track separator
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
        MoveToEx(hdc, rcTrack.left, rcTrack.bottom, NULL);
        LineTo(hdc, rcTrack.right, rcTrack.bottom);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

// ============================================================================
// TimelineItemBehavior
// ============================================================================
TimelineItemBehavior::TimelineItemBehavior()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_dwItemId(0)
    , m_nTrackIndex(0)
    , m_dwTrimStartMs(0)
    , m_dwTrimEndMs(0)
    , m_dwTotalDurationMs(0)
    , m_bSelected(false)
    , m_bThumbnailsValid(false)
{
    ZeroMemory(&m_rcBounds, sizeof(m_rcBounds));
}

TimelineItemBehavior::~TimelineItemBehavior()
{
    ReleaseThumbnails();
    m_pElement = NULL;
}

HRESULT TimelineItemBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineItemBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    ReleaseThumbnails();
    m_pElement = NULL;
    return S_OK;
}

HRESULT TimelineItemBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    PaintItemBody(hdc, prcBounds);

    if (m_bSelected)
        PaintSelectionHighlight(hdc, prcBounds);

    PaintTrimHandles(hdc, prcBounds);

    return S_OK;
}

HRESULT TimelineItemBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    if (pbHandled)
        *pbHandled = FALSE;

    return S_OK;
}

void TimelineItemBehavior::SetItemId(DWORD dwItemId) throw() { m_dwItemId = dwItemId; }
DWORD TimelineItemBehavior::GetItemId() const throw() { return m_dwItemId; }
void TimelineItemBehavior::SetTrackIndex(int nIndex) throw() { m_nTrackIndex = nIndex; }
int  TimelineItemBehavior::GetTrackIndex() const throw() { return m_nTrackIndex; }

void TimelineItemBehavior::SetItemBounds(const RECT& rcBounds) { m_rcBounds = rcBounds; }
void TimelineItemBehavior::GetItemBounds(RECT* prcBounds) const
{
    if (prcBounds) *prcBounds = m_rcBounds;
}

void TimelineItemBehavior::SetSelected(bool bSelected) throw() { m_bSelected = bSelected; }
bool TimelineItemBehavior::IsSelected() const throw() { return m_bSelected; }
void TimelineItemBehavior::SetTrimStart(DWORD dwTrimStartMs) throw() { m_dwTrimStartMs = dwTrimStartMs; }
void TimelineItemBehavior::SetTrimEnd(DWORD dwTrimEndMs) throw() { m_dwTrimEndMs = dwTrimEndMs; }

void TimelineItemBehavior::InvalidateThumbnails()
{
    ReleaseThumbnails();
    m_bThumbnailsValid = false;
}

bool TimelineItemBehavior::HasThumbnails() const throw()
{
    return m_bThumbnailsValid;
}

void TimelineItemBehavior::PaintItemBody(HDC hdc, const RECT* prcBounds)
{
    HBRUSH hBrush = CreateSolidBrush(RGB(200, 220, 240));
    FillRect(hdc, prcBounds, hBrush);
    DeleteObject(hBrush);

    // Draw border
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(150, 170, 190));
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
    Rectangle(hdc, prcBounds->left, prcBounds->top, prcBounds->right, prcBounds->bottom);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void TimelineItemBehavior::PaintSelectionHighlight(HDC hdc, const RECT* prcBounds)
{
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 120, 215));
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 120, 215));
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
    HBRUSH hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, GetStockObject(NULL_BRUSH)));

    Rectangle(hdc, prcBounds->left, prcBounds->top, prcBounds->right, prcBounds->bottom);

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
    DeleteObject(hBrush);
}

void TimelineItemBehavior::PaintTrimHandles(HDC hdc, const RECT* prcBounds)
{
    const int kHandleWidth = 6;

    // Left trim handle
    RECT rcLeft = *prcBounds;
    rcLeft.right = rcLeft.left + kHandleWidth;

    HBRUSH hLeftBrush = CreateSolidBrush(RGB(0, 80, 160));
    FillRect(hdc, &rcLeft, hLeftBrush);
    DeleteObject(hLeftBrush);

    // Right trim handle
    RECT rcRight = *prcBounds;
    rcRight.left = rcRight.right - kHandleWidth;

    HBRUSH hRightBrush = CreateSolidBrush(RGB(0, 80, 160));
    FillRect(hdc, &rcRight, hRightBrush);
    DeleteObject(hRightBrush);
}

void TimelineItemBehavior::LoadThumbnails()
{
    // Thumbnail loading is deferred to the thumbnail cache
    m_bThumbnailsValid = true;
}

void TimelineItemBehavior::ReleaseThumbnails()
{
    for (size_t i = 0; i < m_Thumbnails.size(); ++i)
    {
        if (m_Thumbnails[i])
        {
            DeleteObject(m_Thumbnails[i]);
        }
    }
    m_Thumbnails.clear();
}

// ============================================================================
// TimelineSecondaryTrackItemBehavior
// ============================================================================
TimelineSecondaryTrackItemBehavior::TimelineSecondaryTrackItemBehavior()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_dwItemId(0)
    , m_bSelected(false)
{
    ZeroMemory(&m_rcBounds, sizeof(m_rcBounds));
}

TimelineSecondaryTrackItemBehavior::~TimelineSecondaryTrackItemBehavior()
{
    m_pElement = NULL;
}

HRESULT TimelineSecondaryTrackItemBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineSecondaryTrackItemBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    m_pElement = NULL;
    return S_OK;
}

HRESULT TimelineSecondaryTrackItemBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    HBRUSH hBrush = CreateSolidBrush(RGB(220, 200, 240));
    FillRect(hdc, prcBounds, hBrush);
    DeleteObject(hBrush);

    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(180, 160, 200));
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
    Rectangle(hdc, prcBounds->left, prcBounds->top, prcBounds->right, prcBounds->bottom);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    return S_OK;
}

HRESULT TimelineSecondaryTrackItemBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return S_OK;
}

void TimelineSecondaryTrackItemBehavior::SetItemId(DWORD dwItemId) throw() { m_dwItemId = dwItemId; }
DWORD TimelineSecondaryTrackItemBehavior::GetItemId() const throw() { return m_dwItemId; }
void TimelineSecondaryTrackItemBehavior::SetItemBounds(const RECT& rcBounds) { m_rcBounds = rcBounds; }
void TimelineSecondaryTrackItemBehavior::SetSelected(bool bSelected) throw() { m_bSelected = bSelected; }

// ============================================================================
// TimelineVisualTrackItemBehavior
// ============================================================================
TimelineVisualTrackItemBehavior::TimelineVisualTrackItemBehavior()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_dwItemId(0)
{
    ZeroMemory(&m_rcBounds, sizeof(m_rcBounds));
}

TimelineVisualTrackItemBehavior::~TimelineVisualTrackItemBehavior()
{
    m_pElement = NULL;
}

HRESULT TimelineVisualTrackItemBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineVisualTrackItemBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    m_pElement = NULL;
    return S_OK;
}

HRESULT TimelineVisualTrackItemBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    HBRUSH hBrush = CreateSolidBrush(RGB(255, 230, 180));
    FillRect(hdc, prcBounds, hBrush);
    DeleteObject(hBrush);

    return S_OK;
}

HRESULT TimelineVisualTrackItemBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return S_OK;
}

void TimelineVisualTrackItemBehavior::SetItemId(DWORD dwItemId) throw() { m_dwItemId = dwItemId; }
DWORD TimelineVisualTrackItemBehavior::GetItemId() const throw() { return m_dwItemId; }
void TimelineVisualTrackItemBehavior::SetEffectName(LPCWSTR pszEffectName) { m_strEffectName = pszEffectName ? pszEffectName : L""; }
void TimelineVisualTrackItemBehavior::SetItemBounds(const RECT& rcBounds) { m_rcBounds = rcBounds; }

// ============================================================================
// TimelineItemInputBehavior
// ============================================================================
TimelineItemInputBehavior::TimelineItemInputBehavior()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_bDragging(false)
    , m_bResizing(false)
    , m_resizeEdge(ResizeNone)
    , m_dwDragItemId(0)
    , m_nDragThreshold(5)
{
    ZeroMemory(&m_ptDragStart, sizeof(m_ptDragStart));
    ZeroMemory(&m_ptLastMouse, sizeof(m_ptLastMouse));
}

TimelineItemInputBehavior::~TimelineItemInputBehavior()
{
    m_pElement = NULL;
}

HRESULT TimelineItemInputBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineItemInputBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    m_pElement = NULL;
    return S_OK;
}

HRESULT TimelineItemInputBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
        OnMouseDown(static_cast<UINT>(wParam),
            { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
        if (pbHandled) *pbHandled = TRUE;
        break;

    case WM_MOUSEMOVE:
        OnMouseMove(static_cast<UINT>(wParam),
            { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
        if (m_bDragging || m_bResizing)
            if (pbHandled) *pbHandled = TRUE;
        break;

    case WM_LBUTTONUP:
        OnMouseUp(static_cast<UINT>(wParam),
            { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
        if (pbHandled) *pbHandled = TRUE;
        break;

    case WM_LBUTTONDBLCLK:
        OnDblClk(static_cast<UINT>(wParam),
            { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
        if (pbHandled) *pbHandled = TRUE;
        break;

    case WM_CONTEXTMENU:
        OnContextMenu({ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
        if (pbHandled) *pbHandled = TRUE;
        break;

    case WM_KEYDOWN:
        OnKeyDown(static_cast<UINT>(wParam),
            LOWORD(lParam), HIWORD(lParam));
        if (pbHandled) *pbHandled = TRUE;
        break;
    }

    return S_OK;
}

bool TimelineItemInputBehavior::IsDragging() const throw()
{
    return m_bDragging;
}

void TimelineItemInputBehavior::CancelDrag()
{
    m_bDragging = false;
    m_bResizing = false;
    m_resizeEdge = ResizeNone;
}

TimelineItemInputBehavior::ResizeEdge
TimelineItemInputBehavior::HitTestResizeEdge(POINT pt, const RECT* prcItem) const
{
    if (!prcItem) return ResizeNone;

    const int kResizeZone = 6;

    if (pt.y >= prcItem->top && pt.y <= prcItem->bottom)
    {
        if (abs(pt.x - prcItem->left) <= kResizeZone)
            return ResizeLeft;
        if (abs(pt.x - prcItem->right) <= kResizeZone)
            return ResizeRight;
    }

    return ResizeNone;
}

void TimelineItemInputBehavior::OnMouseDown(UINT nFlags, POINT pt)
{
    UNREFERENCED_PARAMETER(nFlags);

    m_ptDragStart = pt;
    m_ptLastMouse = pt;

    // Check for resize edge hit
    // In the real implementation, we'd get the item bounds from the element
    RECT rcItem = { 0, 0, 200, 60 };
    m_resizeEdge = HitTestResizeEdge(pt, &rcItem);

    if (m_resizeEdge != ResizeNone)
    {
        m_bResizing = true;
    }
    else
    {
        m_bDragging = true;
    }

    ::SetCapture(m_pElement ? NULL : NULL);
}

void TimelineItemInputBehavior::OnMouseMove(UINT nFlags, POINT pt)
{
    UNREFERENCED_PARAMETER(nFlags);

    m_ptLastMouse = pt;

    if (m_bDragging)
    {
        int dx = pt.x - m_ptDragStart.x;
        int dy = pt.y - m_ptDragStart.y;

        if (abs(dx) > m_nDragThreshold || abs(dy) > m_nDragThreshold)
        {
            // Drag threshold exceeded -- begin drag operation
        }
    }
    else if (m_bResizing)
    {
        int dx = pt.x - m_ptLastMouse.x;
        UNREFERENCED_PARAMETER(dx);
        // Resize the item based on the drag delta
    }
    else
    {
        // Update cursor based on hover position
        RECT rcItem = { 0, 0, 200, 60 };
        ResizeEdge edge = HitTestResizeEdge(pt, &rcItem);
        HCURSOR hCursor = LoadCursor(NULL,
            (edge != ResizeNone) ? IDC_SIZEWE : IDC_ARROW);
        if (hCursor)
            ::SetCursor(hCursor);
    }
}

void TimelineItemInputBehavior::OnMouseUp(UINT nFlags, POINT pt)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(pt);

    if (m_bDragging)
    {
        int dx = pt.x - m_ptDragStart.x;
        if (abs(dx) > m_nDragThreshold)
        {
            // Complete drag-drop operation
        }
    }
    else if (m_bResizing)
    {
        int dx = pt.x - m_ptDragStart.x;
        if (abs(dx) > m_nDragThreshold)
        {
            // Complete resize operation
        }
    }

    m_bDragging = false;
    m_bResizing = false;
    m_resizeEdge = ResizeNone;

    ::ReleaseCapture();
}

void TimelineItemInputBehavior::OnDblClk(UINT nFlags, POINT pt)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(pt);

    // Double-click opens the item for editing (text, trim, etc.)
}

void TimelineItemInputBehavior::OnContextMenu(POINT pt)
{
    UNREFERENCED_PARAMETER(pt);

    // Show context menu for the item
}

void TimelineItemInputBehavior::OnKeyDown(UINT nChar, UINT nRepCount, UINT nFlags)
{
    UNREFERENCED_PARAMETER(nRepCount);
    UNREFERENCED_PARAMETER(nFlags);

    switch (nChar)
    {
    case VK_DELETE:
        // Delete selected item
        break;
    case VK_LEFT:
        // Move selected item left
        break;
    case VK_RIGHT:
        // Move selected item right
        break;
    }
}

// ============================================================================
// TimelineInstructionsBehavior
// ============================================================================
TimelineInstructionsBehavior::TimelineInstructionsBehavior()
    : m_pElement(NULL)
    , m_bVisible(true)
    , m_hFont(NULL)
{
}

TimelineInstructionsBehavior::~TimelineInstructionsBehavior()
{
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }
    m_pElement = NULL;
}

HRESULT TimelineInstructionsBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;

    m_hFont = CreateFont(
        18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
        L"Segoe UI");

    return S_OK;
}

HRESULT TimelineInstructionsBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }
    m_pElement = NULL;
    return S_OK;
}

HRESULT TimelineInstructionsBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;
    if (!m_bVisible) return S_OK;

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(150, 150, 150));

    if (m_hFont)
        SelectObject(hdc, m_hFont);

    RECT rcText = *prcBounds;
    DrawText(hdc, m_strInstructionText, -1, &rcText,
        DT_CENTER | DT_VCENTER | DT_WORDBREAK);

    return S_OK;
}

void TimelineInstructionsBehavior::SetInstructionText(LPCWSTR pszText)
{
    m_strInstructionText = pszText ? pszText : L"";
}

void TimelineInstructionsBehavior::SetVisible(bool bVisible) throw()
{
    m_bVisible = bVisible;
}

bool TimelineInstructionsBehavior::IsVisible() const throw()
{
    return m_bVisible;
}

// ============================================================================
// TimelineSelectionRootBehavior
// ============================================================================
TimelineSelectionRootBehavior::TimelineSelectionRootBehavior()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_bMarqueeActive(false)
{
    ZeroMemory(&m_ptMarqueeStart, sizeof(m_ptMarqueeStart));
    ZeroMemory(&m_ptMarqueeEnd, sizeof(m_ptMarqueeEnd));
    ZeroMemory(&m_rcSelection, sizeof(m_rcSelection));
}

TimelineSelectionRootBehavior::~TimelineSelectionRootBehavior()
{
    m_pElement = NULL;
}

HRESULT TimelineSelectionRootBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineSelectionRootBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    m_pElement = NULL;
    return S_OK;
}

HRESULT TimelineSelectionRootBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        if (!(wParam & MK_CONTROL) && !(wParam & MK_SHIFT))
        {
            m_ptMarqueeStart.x = GET_X_LPARAM(lParam);
            m_ptMarqueeStart.y = GET_Y_LPARAM(lParam);
            m_ptMarqueeEnd = m_ptMarqueeStart;
            m_bMarqueeActive = true;
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }

    case WM_MOUSEMOVE:
    {
        if (m_bMarqueeActive && (wParam & MK_LBUTTON))
        {
            m_ptMarqueeEnd.x = GET_X_LPARAM(lParam);
            m_ptMarqueeEnd.y = GET_Y_LPARAM(lParam);
            UpdateSelectionRect();

            if (m_pElement)
                m_pElement->InvalidateRect(NULL);

            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }

    case WM_LBUTTONUP:
    {
        if (m_bMarqueeActive)
        {
            m_bMarqueeActive = false;
            SelectItemsInRect(&m_rcSelection);

            if (m_pElement)
                m_pElement->InvalidateRect(NULL);

            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    }

    return S_OK;
}

HRESULT TimelineSelectionRootBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    UNREFERENCED_PARAMETER(prcBounds);

    if (m_bMarqueeActive)
    {
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 120, 215));
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 120, 215));
        HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
        HBRUSH hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, hBrush));

        // Draw semi-transparent selection rectangle
        RECT rcDraw = m_rcSelection;
        // Use a light brush with alpha for selection overlay
        HBRUSH hLightBrush = CreateSolidBrush(RGB(0, 120, 215));
        FillRect(hdc, &rcDraw, hLightBrush);
        DeleteObject(hLightBrush);

        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
        DeleteObject(hBrush);
    }

    return S_OK;
}

bool TimelineSelectionRootBehavior::IsMarqueeActive() const throw()
{
    return m_bMarqueeActive;
}

void TimelineSelectionRootBehavior::GetSelectionRect(RECT* prcSelection) const
{
    if (prcSelection) *prcSelection = m_rcSelection;
}

void TimelineSelectionRootBehavior::UpdateSelectionRect()
{
    m_rcSelection.left   = std::min(m_ptMarqueeStart.x, m_ptMarqueeEnd.x);
    m_rcSelection.top    = std::min(m_ptMarqueeStart.y, m_ptMarqueeEnd.y);
    m_rcSelection.right  = std::max(m_ptMarqueeStart.x, m_ptMarqueeEnd.x);
    m_rcSelection.bottom = std::max(m_ptMarqueeStart.y, m_ptMarqueeEnd.y);
}

void TimelineSelectionRootBehavior::SelectItemsInRect(const RECT* /*prcSelection*/)
{
    // Query all timeline items and select those intersecting the rectangle
    if (m_pAppMain)
    {
        // Iterate timeline items and check intersection
    }
}

} // namespace Sundance
