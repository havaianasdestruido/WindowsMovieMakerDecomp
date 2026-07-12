/*
 * LayoutBehaviors.cpp
 *
 * Implementation of layout-related DirectUI behaviors.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "LayoutBehaviors.h"
#include "../SundanceApp/SundanceAppMain.h"

namespace Sundance
{

// ============================================================================
// FullscreenLayoutBehavior
// ============================================================================
FullscreenLayoutBehavior::FullscreenLayoutBehavior()
    : m_pElement(NULL), m_pAppMain(NULL), m_hWndPreview(NULL)
    , m_hWndBackground(NULL), m_bFullscreen(false)
    , m_dwOriginalStyle(0), m_dwOriginalExStyle(0)
{
    SetRectEmpty(&m_rcOriginalPreview);
}

FullscreenLayoutBehavior::~FullscreenLayoutBehavior() { m_pElement = NULL; }

HRESULT FullscreenLayoutBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT FullscreenLayoutBehavior::OnElementDetached(IDuiElement*) { ExitFullscreen(); m_pElement = NULL; return S_OK; }

HRESULT FullscreenLayoutBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;
    if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE && m_bFullscreen)
    {
        ExitFullscreen();
        if (pbHandled) *pbHandled = TRUE;
    }
    return S_OK;
}

HRESULT FullscreenLayoutBehavior::EnterFullscreen(HWND hWndPreview)
{
    if (m_bFullscreen) return S_OK;
    if (!hWndPreview) return E_INVALIDARG;

    m_hWndPreview = hWndPreview;
    ::GetWindowRect(hWndPreview, &m_rcOriginalPreview);
    m_dwOriginalStyle = ::GetWindowLong(hWndPreview, GWL_STYLE);
    m_dwOriginalExStyle = ::GetWindowLong(hWndPreview, GWL_EXSTYLE);

    // Create fullscreen background
    m_hWndBackground = ::CreateWindowEx(0, L"SundanceFullscreenBackground", L"",
        WS_POPUP | WS_VISIBLE, 0, 0,
        ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, ModuleHelper::GetModuleInstance(), NULL);

    if (m_hWndBackground)
        ::ShowWindow(m_hWndBackground, SW_SHOW);

    // Resize preview to fill screen
    ::SetWindowLong(hWndPreview, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    ::SetWindowPos(hWndPreview, HWND_TOPMOST, 0, 0,
        ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN),
        SWP_FRAMECHANGED);

    m_bFullscreen = true;
    return S_OK;
}

HRESULT FullscreenLayoutBehavior::ExitFullscreen()
{
    if (!m_bFullscreen) return S_OK;

    if (m_hWndPreview && ::IsWindow(m_hWndPreview))
    {
        ::SetWindowLong(m_hWndPreview, GWL_STYLE, m_dwOriginalStyle);
        ::SetWindowLong(m_hWndPreview, GWL_EXSTYLE, m_dwOriginalExStyle);
        ::SetWindowPos(m_hWndPreview, NULL,
            m_rcOriginalPreview.left, m_rcOriginalPreview.top,
            m_rcOriginalPreview.right - m_rcOriginalPreview.left,
            m_rcOriginalPreview.bottom - m_rcOriginalPreview.top,
            SWP_FRAMECHANGED);
    }

    if (m_hWndBackground && ::IsWindow(m_hWndBackground))
    {
        ::DestroyWindow(m_hWndBackground);
    }
    m_hWndBackground = NULL;

    m_bFullscreen = false;
    return S_OK;
}

bool FullscreenLayoutBehavior::IsFullscreen() const throw() { return m_bFullscreen; }

// ============================================================================
// FullscreenBackgroundWindow
// ============================================================================
FullscreenBackgroundWindow::FullscreenBackgroundWindow() {}
FullscreenBackgroundWindow::~FullscreenBackgroundWindow() {}

HRESULT FullscreenBackgroundWindow::Create(HWND hWndParent)
{
    RECT rcScreen = { 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) };
    HWND hWnd = CWindowImpl<FullscreenBackgroundWindow, CWindow>::Create(
        hWndParent, rcScreen, NULL,
        WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW);
    return hWnd ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

void FullscreenBackgroundWindow::SetFullScreen()
{
    if (IsWindow())
    {
        ::SetWindowPos(m_hWnd, HWND_TOP, 0, 0,
            ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN),
            SWP_SHOWWINDOW);
    }
}

LRESULT FullscreenBackgroundWindow::OnPaint(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);
    RECT rcClient;
    GetClientRect(&rcClient);
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rcClient, hBrush);
    DeleteObject(hBrush);
    EndPaint(&ps);
    return 0;
}

LRESULT FullscreenBackgroundWindow::OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = TRUE;
    return 1;
}

// ============================================================================
// ResizeablePaneBehavior
// ============================================================================
ResizeablePaneBehavior::ResizeablePaneBehavior()
    : m_pElement(NULL), m_direction(SplitVertical)
    , m_fSplitRatio(0.5f), m_nMinPaneSize(100), m_nSplitterWidth(4)
    , m_bDragging(false), m_nDragStart(0), m_fDragStartRatio(0.5f)
{
}

ResizeablePaneBehavior::~ResizeablePaneBehavior() { m_pElement = NULL; }

HRESULT ResizeablePaneBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ResizeablePaneBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT ResizeablePaneBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        m_bDragging = true;
        m_nDragStart = (m_direction == SplitVertical) ? GET_X_LPARAM(lParam) : GET_Y_LPARAM(lParam);
        m_fDragStartRatio = m_fSplitRatio;
        if (pbHandled) *pbHandled = TRUE;
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (m_bDragging)
        {
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (m_bDragging)
        {
            m_bDragging = false;
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    }

    return S_OK;
}

void ResizeablePaneBehavior::SetSplitDirection(SplitDirection direction) throw() { m_direction = direction; }
void ResizeablePaneBehavior::SetSplitRatio(float fRatio) { m_fSplitRatio = max(0.1f, min(0.9f, fRatio)); }
void ResizeablePaneBehavior::SetMinPaneSize(int nMinSize) { m_nMinPaneSize = nMinSize; }
void ResizeablePaneBehavior::SetSplitterWidth(int nWidth) { m_nSplitterWidth = nWidth; }

// ============================================================================
// ProjectWorkspaceLayoutBehavior
// ============================================================================
ProjectWorkspaceLayoutBehavior::ProjectWorkspaceLayoutBehavior()
    : m_pElement(NULL), m_pAppMain(NULL), m_currentPreset(LayoutPresetDefault)
{
}

ProjectWorkspaceLayoutBehavior::~ProjectWorkspaceLayoutBehavior() { m_pElement = NULL; }

HRESULT ProjectWorkspaceLayoutBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    InitializePanes();
    return S_OK;
}

HRESULT ProjectWorkspaceLayoutBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT ProjectWorkspaceLayoutBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return S_OK;
}

void ProjectWorkspaceLayoutBehavior::ShowPane(LPCWSTR pszPaneName)
{
    if (!pszPaneName) return;
    for (size_t i = 0; i < m_panes.size(); ++i)
    {
        if (m_panes[i].strName.CompareNoCase(pszPaneName) == 0)
        {
            m_panes[i].bVisible = true;
            RecalculateLayout();
            return;
        }
    }
}

void ProjectWorkspaceLayoutBehavior::HidePane(LPCWSTR pszPaneName)
{
    if (!pszPaneName) return;
    for (size_t i = 0; i < m_panes.size(); ++i)
    {
        if (m_panes[i].strName.CompareNoCase(pszPaneName) == 0)
        {
            m_panes[i].bVisible = false;
            RecalculateLayout();
            return;
        }
    }
}

void ProjectWorkspaceLayoutBehavior::TogglePane(LPCWSTR pszPaneName)
{
    if (!pszPaneName) return;
    for (size_t i = 0; i < m_panes.size(); ++i)
    {
        if (m_panes[i].strName.CompareNoCase(pszPaneName) == 0)
        {
            m_panes[i].bVisible = !m_panes[i].bVisible;
            RecalculateLayout();
            return;
        }
    }
}

bool ProjectWorkspaceLayoutBehavior::IsPaneVisible(LPCWSTR pszPaneName) const
{
    if (!pszPaneName) return false;
    for (size_t i = 0; i < m_panes.size(); ++i)
    {
        if (m_panes[i].strName.CompareNoCase(pszPaneName) == 0)
            return m_panes[i].bVisible;
    }
    return false;
}

HRESULT ProjectWorkspaceLayoutBehavior::ApplyLayoutPreset(LayoutPreset preset)
{
    m_currentPreset = preset;
    RecalculateLayout();
    return S_OK;
}

ProjectWorkspaceLayoutBehavior::LayoutPreset ProjectWorkspaceLayoutBehavior::GetCurrentPreset() const throw() { return m_currentPreset; }

void ProjectWorkspaceLayoutBehavior::InitializePanes()
{
    m_panes.clear();
    PaneInfo panes[] = { { L"MediaBrowser", true }, { L"Preview", true }, { L"Timeline", true }, { L"Properties", false } };
    for (int i = 0; i < 4; ++i) m_panes.push_back(panes[i]);
}

void ProjectWorkspaceLayoutBehavior::RecalculateLayout()
{
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
}

// ============================================================================
// InlinePreviewLayoutBehavior
// ============================================================================
InlinePreviewLayoutBehavior::InlinePreviewLayoutBehavior()
    : m_pElement(NULL), m_pAppMain(NULL)
    , m_nPreviewWidth(320), m_nPreviewHeight(240)
{
    m_ptPreviewPosition.x = 0;
    m_ptPreviewPosition.y = 0;
}

InlinePreviewLayoutBehavior::~InlinePreviewLayoutBehavior() { m_pElement = NULL; }

HRESULT InlinePreviewLayoutBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT InlinePreviewLayoutBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT InlinePreviewLayoutBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return S_OK;
}

void InlinePreviewLayoutBehavior::SetPreviewSize(int nWidth, int nHeight) { m_nPreviewWidth = nWidth; m_nPreviewHeight = nHeight; }
void InlinePreviewLayoutBehavior::GetPreviewSize(int* pnWidth, int* pnHeight) const { if (pnWidth) *pnWidth = m_nPreviewWidth; if (pnHeight) *pnHeight = m_nPreviewHeight; }
void InlinePreviewLayoutBehavior::SetPreviewPosition(POINT ptPosition) { m_ptPreviewPosition = ptPosition; }

// ============================================================================
// InlinePreviewSliderBehavior
// ============================================================================
InlinePreviewSliderBehavior::InlinePreviewSliderBehavior()
    : m_pElement(NULL), m_pAppMain(NULL)
    , m_dPosition(0.0), m_dDuration(0.0), m_bDragging(false)
{
}

InlinePreviewSliderBehavior::~InlinePreviewSliderBehavior() { m_pElement = NULL; }

HRESULT InlinePreviewSliderBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT InlinePreviewSliderBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT InlinePreviewSliderBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;
    if (uMsg == WM_LBUTTONDOWN) { m_bDragging = true; if (pbHandled) *pbHandled = TRUE; }
    else if (uMsg == WM_LBUTTONUP) { m_bDragging = false; if (pbHandled) *pbHandled = TRUE; }
    return S_OK;
}

HRESULT InlinePreviewSliderBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    int nWidth = prcBounds->right - prcBounds->left;
    int nSliderY = prcBounds->top + (prcBounds->bottom - prcBounds->top) / 2;

    // Draw track
    HPEN hTrackPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hTrackPen));
    MoveToEx(hdc, prcBounds->left, nSliderY, NULL);
    LineTo(hdc, prcBounds->right, nSliderY);

    // Draw fill
    int nFillX = m_dDuration > 0 ? prcBounds->left + static_cast<int>((m_dPosition / m_dDuration) * nWidth) : prcBounds->left;
    HPEN hFillPen = CreatePen(PS_SOLID, 2, RGB(0, 120, 215));
    SelectObject(hdc, hFillPen);
    MoveToEx(hdc, prcBounds->left, nSliderY, NULL);
    LineTo(hdc, nFillX, nSliderY);

    // Draw thumb
    HBRUSH hThumbBrush = CreateSolidBrush(RGB(0, 120, 215));
    RECT rcThumb = { nFillX - 4, nSliderY - 4, nFillX + 4, nSliderY + 4 };
    FillRect(hdc, &rcThumb, hThumbBrush);
    DeleteObject(hThumbBrush);

    SelectObject(hdc, hOldPen);
    DeleteObject(hTrackPen);
    DeleteObject(hFillPen);

    return S_OK;
}

void InlinePreviewSliderBehavior::SetPosition(double dPositionSeconds) { m_dPosition = dPositionSeconds; }
void InlinePreviewSliderBehavior::SetDuration(double dDurationSeconds) { m_dDuration = dDurationSeconds; }
double InlinePreviewSliderBehavior::GetPosition() const throw() { return m_dPosition; }

} // namespace Sundance
