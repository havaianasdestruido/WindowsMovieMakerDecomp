/*
 * EditingBehaviors.cpp
 *
 * Implementation of editing-related DirectUI behaviors.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "EditingBehaviors.h"
#include "../SundanceApp/SundanceAppMain.h"

namespace Sundance
{

// ============================================================================
// TrimBehavior
// ============================================================================
TrimBehavior::TrimBehavior()
    : m_pElement(NULL), m_pAppMain(NULL)
    , m_dwTrimStartMs(0), m_dwTrimEndMs(0), m_dwOriginalDurationMs(0)
    , m_bDragging(false), m_activeHandle(TrimHandleNone), m_nDragStartX(0)
    , m_pfnTrimChanged(NULL), m_pUserData(NULL)
{
}

TrimBehavior::~TrimBehavior() { m_pElement = NULL; }

HRESULT TrimBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TrimBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TrimBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        RECT rcBounds;
        if (m_pElement) m_pElement->GetBounds(&rcBounds);
        else SetRectEmpty(&rcBounds);

        m_activeHandle = HitTestTrimHandle(pt, &rcBounds);
        if (m_activeHandle != TrimHandleNone)
        {
            m_bDragging = true;
            m_nDragStartX = pt.x;
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (m_bDragging)
        {
            if (pbHandled) *pbHandled = TRUE;
        }
        else
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            RECT rcBounds;
            if (m_pElement) m_pElement->GetBounds(&rcBounds);
            else SetRectEmpty(&rcBounds);

            TrimHandle hoverHandle = HitTestTrimHandle(pt, &rcBounds);
            HCURSOR hCursor = LoadCursor(NULL, hoverHandle != TrimHandleNone ? IDC_SIZEWE : IDC_ARROW);
            if (hCursor) ::SetCursor(hCursor);
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (m_bDragging)
        {
            m_bDragging = false;
            m_activeHandle = TrimHandleNone;
            if (pbHandled) *pbHandled = TRUE;
        }
        break;
    }
    }

    return S_OK;
}

HRESULT TrimBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;
    PaintTrimOverlay(hdc, prcBounds);
    return S_OK;
}

void TrimBehavior::SetTrimRange(DWORD dwTrimStartMs, DWORD dwTrimEndMs) { m_dwTrimStartMs = dwTrimStartMs; m_dwTrimEndMs = dwTrimEndMs; }
void TrimBehavior::GetTrimRange(DWORD* pdwTrimStartMs, DWORD* pdwTrimEndMs) const { if (pdwTrimStartMs) *pdwTrimStartMs = m_dwTrimStartMs; if (pdwTrimEndMs) *pdwTrimEndMs = m_dwTrimEndMs; }
void TrimBehavior::SetOriginalDuration(DWORD dwDurationMs) { m_dwOriginalDurationMs = dwDurationMs; }
TrimBehavior::TrimHandle TrimBehavior::GetActiveHandle() const throw() { return m_activeHandle; }

void TrimBehavior::SetTrimCallback(PFN_TRIM_CHANGED pfnCallback, void* pUserData)
{
    m_pfnTrimChanged = pfnCallback;
    m_pUserData = pUserData;
}

void TrimBehavior::PaintTrimOverlay(HDC hdc, const RECT* prcBounds)
{
    if (m_dwOriginalDurationMs == 0) return;

    int nWidth = prcBounds->right - prcBounds->left;
    int nTrimStartX = prcBounds->left + (m_dwTrimStartMs * nWidth / m_dwOriginalDurationMs);
    int nTrimEndX = prcBounds->left + (m_dwTrimEndMs * nWidth / m_dwOriginalDurationMs);

    // Dim trimmed regions
    HBRUSH hDimBrush = CreateSolidBrush(RGB(0, 0, 0));
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 128, 0 };
    RECT rcDimLeft = { prcBounds->left, prcBounds->top, nTrimStartX, prcBounds->bottom };
    RECT rcDimRight = { nTrimEndX, prcBounds->top, prcBounds->right, prcBounds->bottom };
    FillRect(hdc, &rcDimLeft, hDimBrush);
    FillRect(hdc, &rcDimRight, hDimBrush);
    DeleteObject(hDimBrush);

    // Draw trim handles
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
    MoveToEx(hdc, nTrimStartX, prcBounds->top, NULL);
    LineTo(hdc, nTrimStartX, prcBounds->bottom);
    MoveToEx(hdc, nTrimEndX, prcBounds->top, NULL);
    LineTo(hdc, nTrimEndX, prcBounds->bottom);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

TrimBehavior::TrimHandle TrimBehavior::HitTestTrimHandle(POINT pt, const RECT* prcBounds) const
{
    if (m_dwOriginalDurationMs == 0 || !prcBounds) return TrimHandleNone;

    int nWidth = prcBounds->right - prcBounds->left;
    int nTrimStartX = prcBounds->left + (m_dwTrimStartMs * nWidth / m_dwOriginalDurationMs);
    int nTrimEndX = prcBounds->left + (m_dwTrimEndMs * nWidth / m_dwOriginalDurationMs);

    if (abs(pt.x - nTrimStartX) <= kTrimHandleWidth) return TrimHandleLeft;
    if (abs(pt.x - nTrimEndX) <= kTrimHandleWidth) return TrimHandleRight;
    return TrimHandleNone;
}

// ============================================================================
// TextBoxBehavior
// ============================================================================
TextBoxBehavior::TextBoxBehavior()
    : m_pElement(NULL), m_nFontSize(12), m_bBold(false), m_bItalic(false)
    , m_clrText(RGB(255, 255, 255)), m_bEditing(false), m_hWndEdit(NULL), m_hFont(NULL)
{
}

TextBoxBehavior::~TextBoxBehavior()
{
    DestroyEditWindow();
    if (m_hFont) { DeleteObject(m_hFont); m_hFont = NULL; }
    m_pElement = NULL;
}

HRESULT TextBoxBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    ApplyFont();
    return S_OK;
}

HRESULT TextBoxBehavior::OnElementDetached(IDuiElement*) { DestroyEditWindow(); m_pElement = NULL; return S_OK; }

HRESULT TextBoxBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;
    if (uMsg == WM_LBUTTONDBLCLK && !m_bEditing)
    {
        SetEditing(true);
        if (pbHandled) *pbHandled = TRUE;
    }
    return S_OK;
}

void TextBoxBehavior::SetText(LPCWSTR pszText) { m_strText = pszText ? pszText : L""; }
LPCWSTR TextBoxBehavior::GetText() const throw() { return m_strText; }

void TextBoxBehavior::SetFont(LPCWSTR pszFontFamily, int nFontSize, bool bBold, bool bItalic)
{
    m_strFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
    m_nFontSize = nFontSize;
    m_bBold = bBold;
    m_bItalic = bItalic;
    ApplyFont();
}

void TextBoxBehavior::SetTextColor(COLORREF clrText) { m_clrText = clrText; }
void TextBoxBehavior::SetEditing(bool bEditing) throw() { m_bEditing = bEditing; }
bool TextBoxBehavior::IsEditing() const throw() { return m_bEditing; }

void TextBoxBehavior::CreateEditWindow(HWND hWndParent, RECT rcBounds)
{
    if (m_hWndEdit) return;
    m_hWndEdit = ::CreateWindowEx(0, L"EDIT", m_strText,
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL,
        rcBounds.left, rcBounds.top, rcBounds.right - rcBounds.left, rcBounds.bottom - rcBounds.top,
        hWndParent, NULL, ModuleHelper::GetModuleInstance(), NULL);
    if (m_hWndEdit && m_hFont) ::SendMessage(m_hWndEdit, WM_SETFONT, (WPARAM)m_hFont, TRUE);
}

void TextBoxBehavior::DestroyEditWindow()
{
    if (m_hWndEdit && ::IsWindow(m_hWndEdit))
    {
        int cch = ::GetWindowTextLength(m_hWndEdit) + 1;
        std::vector<WCHAR> buf(cch);
        ::GetWindowText(m_hWndEdit, buf.data(), cch);
        m_strText = buf.data();
        ::DestroyWindow(m_hWndEdit);
    }
    m_hWndEdit = NULL;
}

void TextBoxBehavior::ApplyFont()
{
    if (m_hFont) DeleteObject(m_hFont);
    m_hFont = CreateFont(m_nFontSize * 10, 0, 0, 0, m_bBold ? FW_BOLD : FW_NORMAL,
        m_bItalic ? TRUE : FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, m_strFontFamily.IsEmpty() ? L"Segoe UI" : (LPCWSTR)m_strFontFamily);
}

// ============================================================================
// RichEditControlBehavior
// ============================================================================
RichEditControlBehavior::RichEditControlBehavior() : m_pElement(NULL), m_hWndRichEdit(NULL) {}
RichEditControlBehavior::~RichEditControlBehavior() { DestroyRichEditControl(); m_pElement = NULL; }

HRESULT RichEditControlBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RichEditControlBehavior::OnElementDetached(IDuiElement*) { DestroyRichEditControl(); m_pElement = NULL; return S_OK; }

void RichEditControlBehavior::SetRichText(LPCWSTR pszRichText) { m_strRichText = pszRichText ? pszRichText : L""; }
HRESULT RichEditControlBehavior::GetRichText(ATL::CString& strRichText) { strRichText = m_strRichText; return S_OK; }
void RichEditControlBehavior::SetSelectionBold(bool bBold)
{
    if (!m_hWndRichEdit) return;
    CHARFORMAT2W cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_BOLD;
    cf.dwEffects = bBold ? CFE_BOLD : 0;
    SendMessage(m_hWndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void RichEditControlBehavior::SetSelectionItalic(bool bItalic)
{
    if (!m_hWndRichEdit) return;
    CHARFORMAT2W cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_ITALIC;
    cf.dwEffects = bItalic ? CFE_ITALIC : 0;
    SendMessage(m_hWndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void RichEditControlBehavior::SetSelectionUnderline(bool bUnderline)
{
    if (!m_hWndRichEdit) return;
    CHARFORMAT2W cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_UNDERLINE;
    cf.dwEffects = bUnderline ? CFE_UNDERLINE : 0;
    SendMessage(m_hWndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void RichEditControlBehavior::SetSelectionFontSize(int nSizePt10)
{
    if (!m_hWndRichEdit) return;
    CHARFORMAT2W cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_SIZE;
    cf.yHeight = nSizePt10 * 20;
    SendMessage(m_hWndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void RichEditControlBehavior::SetSelectionFontFamily(LPCWSTR pszFontName)
{
    if (!m_hWndRichEdit || !pszFontName) return;
    CHARFORMAT2W cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_FACE;
    StringCchCopyW(cf.szFaceName, _countof(cf.szFaceName), pszFontName);
    SendMessage(m_hWndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void RichEditControlBehavior::SetSelectionTextColor(COLORREF clrText)
{
    if (!m_hWndRichEdit) return;
    CHARFORMAT2W cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = clrText;
    SendMessage(m_hWndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void RichEditControlBehavior::CreateRichEditControl(HWND hWndParent)
{
    if (m_hWndRichEdit) return;
    if (!hWndParent) return;

    RECT rcClient;
    GetClientRect(hWndParent, &rcClient);

    m_hWndRichEdit = CreateWindowExW(
        WS_EX_LEFT | WS_EX_CLIENTEDGE,
        RICHEDIT_CLASS,
        L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
        0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
        hWndParent,
        NULL,
        GetModuleHandle(NULL),
        NULL);

    if (m_hWndRichEdit)
    {
        SendMessage(m_hWndRichEdit, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
        if (!m_strRichText.IsEmpty())
            SetWindowTextW(m_hWndRichEdit, m_strRichText);
    }
}
void RichEditControlBehavior::DestroyRichEditControl() { m_hWndRichEdit = NULL; }

// ============================================================================
// CaretBehavior
// ============================================================================
CaretBehavior::CaretBehavior()
    : m_pElement(NULL), m_nCaretHeight(16), m_bCaretVisible(false)
    , m_dwBlinkRate(500), m_nTimerId(0)
{
    m_ptCaret.x = 0; m_ptCaret.y = 0;
}

CaretBehavior::~CaretBehavior() { m_pElement = NULL; }

HRESULT CaretBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT CaretBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

void CaretBehavior::SetCaretPosition(POINT ptPosition) { m_ptCaret = ptPosition; }
void CaretBehavior::SetCaretHeight(int nHeight) { m_nCaretHeight = nHeight; }

void CaretBehavior::ShowCaret(bool bShow)
{
    m_bCaretVisible = bShow;
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
}

void CaretBehavior::SetCaretBlinkRate(DWORD dwBlinkRateMs) { m_dwBlinkRate = dwBlinkRateMs; }

// ============================================================================
// CaretScrollBehavior
// ============================================================================
CaretScrollBehavior::CaretScrollBehavior() : m_pElement(NULL), m_nScrollMargin(10)
{
    SetRectEmpty(&m_rcViewport);
}

CaretScrollBehavior::~CaretScrollBehavior() { m_pElement = NULL; }

HRESULT CaretScrollBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT CaretScrollBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

void CaretScrollBehavior::SetViewportRect(const RECT& rcViewport) { m_rcViewport = rcViewport; }

void CaretScrollBehavior::ScrollToCaret(POINT ptCaret, int nCaretHeight)
{
    if (ptCaret.x < m_rcViewport.left + m_nScrollMargin ||
        ptCaret.x > m_rcViewport.right - m_nScrollMargin ||
        ptCaret.y + nCaretHeight < m_rcViewport.top + m_nScrollMargin ||
        ptCaret.y > m_rcViewport.bottom - m_nScrollMargin)
    {
        if (m_pElement)
            m_pElement->InvalidateRect(NULL);
    }
}

void CaretScrollBehavior::SetScrollMargin(int nMargin) { m_nScrollMargin = nMargin; }

// ============================================================================
// AboveHomerBehavior
// ============================================================================
AboveHomerBehavior::AboveHomerBehavior() : m_pElement(NULL) {}
AboveHomerBehavior::~AboveHomerBehavior() { m_pElement = NULL; }

HRESULT AboveHomerBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT AboveHomerBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT AboveHomerBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return S_OK;
}

// ============================================================================
// OptionsDialogBehavior
// ============================================================================
OptionsDialogBehavior::OptionsDialogBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
OptionsDialogBehavior::~OptionsDialogBehavior() { m_pElement = NULL; }

HRESULT OptionsDialogBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT OptionsDialogBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

void OptionsDialogBehavior::ShowOptionsPage(LPCWSTR pszPageName) { m_strCurrentPage = pszPageName ? pszPageName : L""; }
void OptionsDialogBehavior::SaveOptions()
{
    if (m_pAppMain)
    {
        m_pAppMain->SaveUserPreferences();
    }
}
void OptionsDialogBehavior::CancelOptions() { m_strCurrentPage.Empty(); }

// ============================================================================
// HelpBehavior
// ============================================================================
HelpBehavior::HelpBehavior() : m_pElement(NULL) {}
HelpBehavior::~HelpBehavior() { m_pElement = NULL; }

HRESULT HelpBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT HelpBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

void HelpBehavior::ShowHelpTopic(LPCWSTR pszTopicId)
{
    if (!pszTopicId || !*pszTopicId) return;

    WCHAR szHelpPath[MAX_PATH];
    if (GetModuleFileNameW(NULL, szHelpPath, MAX_PATH))
    {
        WCHAR* pSlash = wcsrchr(szHelpPath, L'\\');
        if (pSlash) { pSlash++; *pSlash = L'\0'; }
        StringCchCatW(szHelpPath, MAX_PATH, L"Help\\");
        StringCchCatW(szHelpPath, MAX_PATH, pszTopicId);
        StringCchCatW(szHelpPath, MAX_PATH, L".html");

        SHELLEXECUTEINFOW sei = { sizeof(sei) };
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb = L"open";
        sei.lpFile = L"hh.exe";
        WCHAR szParams[MAX_PATH + 10];
        StringCchPrintfW(szParams, _countof(szParams), L"%s", szHelpPath);
        sei.lpParameters = szParams;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteExW(&sei);
    }
}

void HelpBehavior::ShowInlineHelp(LPCWSTR pszHelpText, POINT ptPosition)
{
    if (!pszHelpText || !*pszHelpText) return;
    if (!m_pElement) return;

    RECT rcElement;
    m_pElement->GetBounds(&rcElement);

    HWND hWndToolTip = CreateWindowExW(
        WS_EX_TOPMOST,
        TOOLTIPS_CLASS,
        NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
        ptPosition.x, ptPosition.y,
        0, 0,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL);

    if (hWndToolTip)
    {
        TOOLINFOW ti = { sizeof(ti) };
        ti.uFlags = TTF_SUBCLASS;
        ti.hwnd = NULL;
        ti.lpszText = (LPWSTR)pszHelpText;
        ti.rect = rcElement;
        SendMessage(hWndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
        SendMessage(hWndToolTip, TTM_POPUP, 0, 0);
    }
}

// ============================================================================
// CaptureUIBehavior
// ============================================================================
CaptureUIBehavior::CaptureUIBehavior()
    : m_pElement(NULL), m_pAppMain(NULL), m_bPreviewing(false), m_bCapturing(false), m_hWndPreview(NULL)
{
}

CaptureUIBehavior::~CaptureUIBehavior() { m_pElement = NULL; }

HRESULT CaptureUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT CaptureUIBehavior::OnElementDetached(IDuiElement*)
{
    StopPreview();
    m_pElement = NULL;
    return S_OK;
}

HRESULT CaptureUIBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_PAINT:
    {
        if (m_hWndPreview && m_bPreviewing)
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hWndPreview, &ps);
            if (hdc)
            {
                RECT rcClient;
                GetClientRect(m_hWndPreview, &rcClient);
                FillRect(hdc, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
                EndPaint(m_hWndPreview, &ps);
                if (pbHandled) *pbHandled = TRUE;
            }
        }
        break;
    }
    case WM_CLOSE:
    {
        StopPreview();
        if (pbHandled) *pbHandled = TRUE;
        break;
    }
    }

    return S_OK;
}

HRESULT CaptureUIBehavior::StartPreview() { m_bPreviewing = true; return S_OK; }
HRESULT CaptureUIBehavior::StopPreview() { m_bPreviewing = false; return S_OK; }
HRESULT CaptureUIBehavior::StartCapture() { m_bCapturing = true; return S_OK; }
HRESULT CaptureUIBehavior::StopCapture() { m_bCapturing = false; return S_OK; }
bool CaptureUIBehavior::IsCapturing() const throw() { return m_bCapturing; }
bool CaptureUIBehavior::IsPreviewing() const throw() { return m_bPreviewing; }
HRESULT CaptureUIBehavior::EnumCaptureDevices(std::vector<ATL::CString>& deviceNames) { deviceNames.clear(); return S_OK; }
HRESULT CaptureUIBehavior::SelectCaptureDevice(LPCWSTR pszDeviceName) { m_strSelectedDevice = pszDeviceName ? pszDeviceName : L""; return S_OK; }

} // namespace Sundance
