/*
 * SliderUI.cpp
 *
 * Implementation of popup slider and track bar controls.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "SliderUI.h"

namespace Sundance
{

// ============================================================================
// PopUpSlider
// ============================================================================
PopUpSlider::PopUpSlider()
    : m_pElement(NULL)
    , m_hWndPopup(NULL)
    , m_hWndTrackBar(NULL)
    , m_nValue(50)
    , m_nMin(0)
    , m_nMax(100)
    , m_mode(SliderModeVolume)
    , m_pfnCallback(NULL)
    , m_pUserData(NULL)
{
}

PopUpSlider::~PopUpSlider()
{
    if (IsWindow())
        DestroyWindow();

    m_pElement = NULL;
}

HRESULT PopUpSlider::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT PopUpSlider::OnElementDetached(IDuiElement* /*pElement*/)
{
    HidePopup();
    m_pElement = NULL;
    return S_OK;
}

HRESULT PopUpSlider::ShowPopup(HWND hWndParent, POINT ptPosition)
{
    if (!hWndParent)
        return E_INVALIDARG;

    if (!IsWindow())
    {
        RECT rcPopup = { ptPosition.x, ptPosition.y, ptPosition.x + 30, ptPosition.y + 200 };
        Create(hWndParent, rcPopup, L"SundancePopUpSlider",
            WS_POPUP | WS_BORDER | WS_VISIBLE, WS_EX_TOPMOST | WS_EX_NOACTIVATE);
    }

    if (IsWindow())
    {
        SetWindowPos(HWND_TOPMOST, ptPosition.x, ptPosition.y,
            30, 200, SWP_SHOWWINDOW | SWP_NOACTIVATE);
        m_hWndPopup = m_hWnd;
    }

    return S_OK;
}

HRESULT PopUpSlider::HidePopup()
{
    if (IsWindow())
    {
        ShowWindow(SW_HIDE);
    }

    if (m_hWndTrackBar && ::IsWindow(m_hWndTrackBar))
    {
        ::DestroyWindow(m_hWndTrackBar);
        m_hWndTrackBar = NULL;
    }

    return S_OK;
}

bool PopUpSlider::IsPopupVisible() const throw()
{
    return IsWindow() && ::IsWindowVisible(m_hWnd);
}

void PopUpSlider::SetValue(int nValue)
{
    m_nValue = std::max(m_nMin, std::min(m_nMax, nValue));

    if (m_hWndTrackBar && ::IsWindow(m_hWndTrackBar))
        ::SendMessage(m_hWndTrackBar, TBM_SETPOS, TRUE, m_nValue);

    if (m_pfnCallback)
        m_pfnCallback(m_nValue, m_pUserData);
}

int PopUpSlider::GetValue() const throw() { return m_nValue; }

void PopUpSlider::SetRange(int nMin, int nMax)
{
    m_nMin = nMin;
    m_nMax = nMax;

    if (m_hWndTrackBar && ::IsWindow(m_hWndTrackBar))
        ::SendMessage(m_hWndTrackBar, TBM_SETRANGE, TRUE, MAKELPARAM(m_nMin, m_nMax));
}

void PopUpSlider::GetRange(int* pnMin, int* pnMax) const
{
    if (pnMin) *pnMin = m_nMin;
    if (pnMax) *pnMax = m_nMax;
}

void PopUpSlider::SetSliderMode(SliderMode mode) throw() { m_mode = mode; }
PopUpSlider::SliderMode PopUpSlider::GetSliderMode() const throw() { return m_mode; }

void PopUpSlider::SetCallback(PFN_SLIDER_CHANGED pfnCallback, void* pUserData)
{
    m_pfnCallback = pfnCallback;
    m_pUserData = pUserData;
}

void PopUpSlider::CreateTrackBar(HWND hWndParent)
{
    if (m_hWndTrackBar) return;

    m_hWndTrackBar = ::CreateWindowEx(
        0, TRACKBAR_CLASS, L"",
        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT | TBS_TOOLTIPS,
        0, 0, 30, 200,
        hWndParent, NULL, ModuleHelper::GetModuleInstance(), NULL);

    if (m_hWndTrackBar)
    {
        ::SendMessage(m_hWndTrackBar, TBM_SETRANGE, TRUE, MAKELPARAM(m_nMin, m_nMax));
        ::SendMessage(m_hWndTrackBar, TBM_SETPOS, TRUE, m_nValue);
    }
}

void PopUpSlider::LayoutControls()
{
    if (m_hWndTrackBar && ::IsWindow(m_hWndTrackBar))
    {
        RECT rcClient;
        GetClientRect(&rcClient);
        ::SetWindowPos(m_hWndTrackBar, NULL,
            0, 0, rcClient.right, rcClient.bottom,
            SWP_NOZORDER);
    }
}

LRESULT PopUpSlider::OnCreate(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;
    CreateTrackBar(m_hWnd);
    LayoutControls();
    return 0;
}

LRESULT PopUpSlider::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    m_hWndPopup = NULL;
    return 0;
}

LRESULT PopUpSlider::OnPaint(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);

    RECT rcClient;
    GetClientRect(&rcClient);
    FillRect(hdc, &rcClient, reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

    EndPaint(&ps);
    return 0;
}

LRESULT PopUpSlider::OnActivate(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    if (LOWORD(wParam) == WA_INACTIVE)
        HidePopup();
    return 0;
}

LRESULT PopUpSlider::OnKillFocus(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    HidePopup();
    return 0;
}

LRESULT PopUpSlider::OnKeyDown(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    if (wParam == VK_ESCAPE)
        HidePopup();
    return 0;
}

// ============================================================================
// PopUpSliderTrackBar
// ============================================================================
PopUpSliderTrackBar::PopUpSliderTrackBar()
    : m_nValue(50)
    , m_nMin(0)
    , m_nMax(100)
    , m_nTrackHeight(4)
    , m_nThumbWidth(16)
    , m_bDragging(false)
    , m_clrTrack(RGB(200, 200, 200))
    , m_clrFill(RGB(0, 120, 215))
    , m_clrThumb(RGB(255, 255, 255))
{
}

PopUpSliderTrackBar::~PopUpSliderTrackBar() {}

void PopUpSliderTrackBar::SetValue(int nValue)
{
    m_nValue = std::max(m_nMin, std::min(m_nMax, nValue));
    if (IsWindow())
        Invalidate(FALSE);
}

int PopUpSliderTrackBar::GetValue() const throw() { return m_nValue; }

void PopUpSliderTrackBar::SetRange(int nMin, int nMax)
{
    m_nMin = nMin;
    m_nMax = nMax;
    m_nValue = std::max(m_nMin, std::min(m_nMax, m_nValue));
}

void PopUpSliderTrackBar::SetTrackHeight(int nHeight) { m_nTrackHeight = nHeight; }
void PopUpSliderTrackBar::SetThumbWidth(int nWidth) { m_nThumbWidth = nWidth; }
void PopUpSliderTrackBar::SetTrackColor(COLORREF clrTrack) { m_clrTrack = clrTrack; }
void PopUpSliderTrackBar::SetFillColor(COLORREF clrFill) { m_clrFill = clrFill; }
void PopUpSliderTrackBar::SetThumbColor(COLORREF clrThumb) { m_clrThumb = clrThumb; }

int PopUpSliderTrackBar::ValueFromPoint(int x) const
{
    RECT rcClient;
    GetClientRect(&rcClient);

    int nRange = m_nMax - m_nMin;
    if (nRange <= 0) return m_nMin;

    int nClientWidth = rcClient.right - rcClient.left;
    if (nClientWidth <= 0) return m_nMin;

    int nValue = m_nMin + (x * nRange / nClientWidth);
    return std::max(m_nMin, std::min(m_nMax, nValue));
}

int PopUpSliderTrackBar::PointFromValue(int nValue) const
{
    RECT rcClient;
    GetClientRect(&rcClient);

    int nRange = m_nMax - m_nMin;
    if (nRange <= 0) return 0;

    int nClientWidth = rcClient.right - rcClient.left;
    return ((nValue - m_nMin) * nClientWidth) / nRange;
}

RECT PopUpSliderTrackBar::GetThumbRect() const
{
    RECT rcThumb;
    int x = PointFromValue(m_nValue);

    RECT rcClient;
    GetClientRect(&rcClient);

    int yCenter = (rcClient.top + rcClient.bottom) / 2;
    rcThumb.left = x - m_nThumbWidth / 2;
    rcThumb.right = x + m_nThumbWidth / 2;
    rcThumb.top = yCenter - m_nThumbWidth / 2;
    rcThumb.bottom = yCenter + m_nThumbWidth / 2;

    return rcThumb;
}

void PopUpSliderTrackBar::PaintTrack(HDC hdc, const RECT& rcClient)
{
    int yCenter = (rcClient.top + rcClient.bottom) / 2;
    RECT rcTrack = { rcClient.left, yCenter - m_nTrackHeight / 2,
                     rcClient.right, yCenter + m_nTrackHeight / 2 };

    HBRUSH hBrush = CreateSolidBrush(m_clrTrack);
    FillRect(hdc, &rcTrack, hBrush);
    DeleteObject(hBrush);
}

void PopUpSliderTrackBar::PaintFill(HDC hdc, const RECT& rcClient)
{
    int yCenter = (rcClient.top + rcClient.bottom) / 2;
    int xValue = PointFromValue(m_nValue);

    RECT rcFill = { rcClient.left, yCenter - m_nTrackHeight / 2,
                    xValue, yCenter + m_nTrackHeight / 2 };

    HBRUSH hBrush = CreateSolidBrush(m_clrFill);
    FillRect(hdc, &rcFill, hBrush);
    DeleteObject(hBrush);
}

void PopUpSliderTrackBar::PaintThumb(HDC hdc, const RECT& rcThumb)
{
    HBRUSH hBrush = CreateSolidBrush(m_clrThumb);
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
    HBRUSH hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, hBrush));

    RoundRect(hdc, rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom, 4, 4);

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
    DeleteObject(hBrush);
}

LRESULT PopUpSliderTrackBar::OnPaint(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);

    RECT rcClient;
    GetClientRect(&rcClient);

    FillRect(hdc, &rcClient, reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    PaintTrack(hdc, rcClient);
    PaintFill(hdc, rcClient);
    PaintThumb(hdc, GetThumbRect());

    EndPaint(&ps);
    return 0;
}

LRESULT PopUpSliderTrackBar::OnLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;

    int x = GET_X_LPARAM(lParam);
    m_nValue = ValueFromPoint(x);
    m_bDragging = TRUE;
    SetCapture();

    Invalidate(FALSE);
    return 0;
}

LRESULT PopUpSliderTrackBar::OnMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;

    if (m_bDragging && (wParam & MK_LBUTTON))
    {
        int x = GET_X_LPARAM(lParam);
        m_nValue = ValueFromPoint(x);
        Invalidate(FALSE);
    }

    return 0;
}

LRESULT PopUpSliderTrackBar::OnLButtonUp(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;

    if (m_bDragging)
    {
        m_bDragging = FALSE;
        ReleaseCapture();
    }

    return 0;
}

LRESULT PopUpSliderTrackBar::OnSize(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    Invalidate(FALSE);
    return 0;
}

LRESULT PopUpSliderTrackBar::OnSetCursor(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    SetCursor(LoadCursor(NULL, IDC_HAND));
    return TRUE;
}

} // namespace Sundance
