#include "pch.h"

/*
 * LegacyText.cpp
 *
 * Implementation of LegacyTextExtent, LegacyExtent, LegacyParagraph,
 * and LegacyTransform.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "LegacyText.h"

// ============================================================================
// LegacyTransform implementation
// ============================================================================

LegacyTransform::LegacyTransform()
    : m_matrix(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f)
{
}

LegacyTransform::~LegacyTransform()
{
}

void LegacyTransform::SetOffsetX(float flX)
{
    m_matrix.SetOffsetX(flX);
}

float LegacyTransform::GetOffsetX() const throw()
{
    return m_matrix.GetOffsetX();
}

void LegacyTransform::SetOffsetY(float flY)
{
    m_matrix.SetOffsetY(flY);
}

float LegacyTransform::GetOffsetY() const throw()
{
    return m_matrix.GetOffsetY();
}

void LegacyTransform::SetRotation(float flDegrees)
{
    Gdiplus::Matrix rot;
    rot.Rotate(flDegrees);
    m_matrix.Multiply(&rot, Gdiplus::MatrixOrderAppend);
}

float LegacyTransform::GetRotation() const throw()
{
    Gdiplus::REAL elements[6];
    m_matrix.GetElements(elements);
    return (float)(atan2(elements[1], elements[0]) * 180.0 / 3.14159);
}

void LegacyTransform::SetScale(float flScaleX, float flScaleY)
{
    m_matrix.SetElements(
        flScaleX, m_matrix.GetOffsetY(),
        m_matrix.GetOffsetX(), flScaleY,
        m_matrix.GetOffsetX(), m_matrix.GetOffsetY());
}

float LegacyTransform::GetScaleX() const throw()
{
    Gdiplus::REAL elements[6];
    m_matrix.GetElements(elements);
    return elements[0];
}

float LegacyTransform::GetScaleY() const throw()
{
    Gdiplus::REAL elements[6];
    m_matrix.GetElements(elements);
    return elements[3];
}

POINT LegacyTransform::TransformPoint(const POINT& ptSrc) const
{
    Gdiplus::PointF pt(static_cast<Gdiplus::REAL>(ptSrc.x), static_cast<Gdiplus::REAL>(ptSrc.y));
    m_matrix.Transform(&pt);
    POINT ptDst;
    ptDst.x = static_cast<LONG>(pt.X);
    ptDst.y = static_cast<LONG>(pt.Y);
    return ptDst;
}

RECT LegacyTransform::TransformRect(const RECT& rcSrc) const
{
    Gdiplus::RectF rc(static_cast<Gdiplus::REAL>(rcSrc.left), static_cast<Gdiplus::REAL>(rcSrc.top),
                      static_cast<Gdiplus::REAL>(rcSrc.right - rcSrc.left),
                      static_cast<Gdiplus::REAL>(rcSrc.bottom - rcSrc.top));

    Gdiplus::PointF pts[4] = {
        Gdiplus::PointF(rc.GetLeft(), rc.GetTop()),
        Gdiplus::PointF(rc.GetRight(), rc.GetTop()),
        Gdiplus::PointF(rc.GetRight(), rc.GetBottom()),
        Gdiplus::PointF(rc.GetLeft(), rc.GetBottom())
    };

    m_matrix.Transform(pts, 4);

    RECT rcDst;
    rcDst.left = static_cast<LONG>(std::min(std::min(pts[0].X, pts[1].X), std::min(pts[2].X, pts[3].X)));
    rcDst.top = static_cast<LONG>(std::min(std::min(pts[0].Y, pts[1].Y), std::min(pts[2].Y, pts[3].Y)));
    rcDst.right = static_cast<LONG>(std::max(std::max(pts[0].X, pts[1].X), std::max(pts[2].X, pts[3].X)));
    rcDst.bottom = static_cast<LONG>(std::max(std::max(pts[0].Y, pts[1].Y), std::max(pts[2].Y, pts[3].Y)));
    return rcDst;
}

POINT LegacyTransform::InverseTransformPoint(const POINT& ptSrc) const
{
    Gdiplus::Matrix inv;
    m_matrix.Invert(&inv);

    Gdiplus::PointF pt(static_cast<Gdiplus::REAL>(ptSrc.x), static_cast<Gdiplus::REAL>(ptSrc.y));
    inv.Transform(&pt);

    POINT ptDst;
    ptDst.x = static_cast<LONG>(pt.X);
    ptDst.y = static_cast<LONG>(pt.Y);
    return ptDst;
}

const Gdiplus::Matrix& LegacyTransform::GetMatrix() const throw()
{
    return m_matrix;
}

void LegacyTransform::SetMatrix(const Gdiplus::Matrix& matrix)
{
    m_matrix = matrix;
}

// ============================================================================
// LegacyExtent implementation
// ============================================================================

LegacyExtent::LegacyExtent()
    : m_flX(0.0f)
    , m_flY(0.0f)
    , m_flZ(0.0f)
    , m_flWidth(0.0f)
    , m_flHeight(0.0f)
    , m_flDepth(0.0f)
{
}

LegacyExtent::~LegacyExtent()
{
}

void LegacyExtent::SetWidth(float flWidth)
{
    m_flWidth = flWidth;
}

float LegacyExtent::GetWidth() const throw()
{
    return m_flWidth;
}

void LegacyExtent::SetHeight(float flHeight)
{
    m_flHeight = flHeight;
}

float LegacyExtent::GetHeight() const throw()
{
    return m_flHeight;
}

void LegacyExtent::SetDepth(float flDepth)
{
    m_flDepth = flDepth;
}

float LegacyExtent::GetDepth() const throw()
{
    return m_flDepth;
}

void LegacyExtent::SetX(float flX)
{
    m_flX = flX;
}

float LegacyExtent::GetX() const throw()
{
    return m_flX;
}

void LegacyExtent::SetY(float flY)
{
    m_flY = flY;
}

float LegacyExtent::GetY() const throw()
{
    return m_flY;
}

void LegacyExtent::SetZ(float flZ)
{
    m_flZ = flZ;
}

float LegacyExtent::GetZ() const throw()
{
    return m_flZ;
}

RECT LegacyExtent::GetBounds() const
{
    RECT rc;
    rc.left = static_cast<LONG>(m_flX);
    rc.top = static_cast<LONG>(m_flY);
    rc.right = static_cast<LONG>(m_flX + m_flWidth);
    rc.bottom = static_cast<LONG>(m_flY + m_flHeight);
    return rc;
}

void LegacyExtent::SetBounds(const RECT& rc)
{
    m_flX = static_cast<float>(rc.left);
    m_flY = static_cast<float>(rc.top);
    m_flWidth = static_cast<float>(rc.right - rc.left);
    m_flHeight = static_cast<float>(rc.bottom - rc.top);
}

void LegacyExtent::Normalize(UINT uContainerWidth, UINT uContainerHeight)
{
    if (uContainerWidth > 0)
    {
        m_flX /= uContainerWidth;
        m_flWidth /= uContainerWidth;
    }
    if (uContainerHeight > 0)
    {
        m_flY /= uContainerHeight;
        m_flHeight /= uContainerHeight;
    }
}

void LegacyExtent::Denormalize(UINT uContainerWidth, UINT uContainerHeight)
{
    m_flX *= uContainerWidth;
    m_flWidth *= uContainerWidth;
    m_flY *= uContainerHeight;
    m_flHeight *= uContainerHeight;
}

LegacyExtent LegacyExtent::FromGdiplusRect(const Gdiplus::RectF& rc)
{
    LegacyExtent extent;
    extent.m_flX = rc.X;
    extent.m_flY = rc.Y;
    extent.m_flWidth = rc.Width;
    extent.m_flHeight = rc.Height;
    return extent;
}

Gdiplus::RectF LegacyExtent::ToGdiplusRect() const
{
    return Gdiplus::RectF(m_flX, m_flY, m_flWidth, m_flHeight);
}

bool LegacyExtent::operator==(const LegacyExtent& other) const
{
    return (m_flX == other.m_flX &&
            m_flY == other.m_flY &&
            m_flWidth == other.m_flWidth &&
            m_flHeight == other.m_flHeight);
}

bool LegacyExtent::operator!=(const LegacyExtent& other) const
{
    return !(*this == other);
}

// ============================================================================
// LegacyParagraph implementation
// ============================================================================

LegacyParagraph::LegacyParagraph()
    : m_flFontSize(18.0f)
    , m_fontStyle(Gdiplus::FontStyleRegular)
    , m_crForeColor(RGB(255, 255, 255))
    , m_crBackColor(RGB(0, 0, 0))
    , m_hAlign(HAlignCenter)
    , m_vAlign(VAlignMiddle)
    , m_bWordWrap(true)
{
    SetRect(&m_rcLayout, 0, 0, 0, 0);
}

LegacyParagraph::~LegacyParagraph()
{
}

void LegacyParagraph::SetText(LPCWSTR pszText)
{
    m_strText = pszText ? pszText : L"";
}

ATL::CString LegacyParagraph::GetText() const
{
    return m_strText;
}

void LegacyParagraph::SetFontFamily(LPCWSTR pszFontFamily)
{
    m_strFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
}

ATL::CString LegacyParagraph::GetFontFamily() const
{
    return m_strFontFamily;
}

void LegacyParagraph::SetFontSize(float flSize)
{
    m_flFontSize = std::max(1.0f, flSize);
}

float LegacyParagraph::GetFontSize() const throw()
{
    return m_flFontSize;
}

void LegacyParagraph::SetFontStyle(Gdiplus::FontStyle style)
{
    m_fontStyle = style;
}

Gdiplus::FontStyle LegacyParagraph::GetFontStyle() const throw()
{
    return m_fontStyle;
}

void LegacyParagraph::SetForeColor(COLORREF crColor)
{
    m_crForeColor = crColor;
}

COLORREF LegacyParagraph::GetForeColor() const throw()
{
    return m_crForeColor;
}

void LegacyParagraph::SetBackColor(COLORREF crColor)
{
    m_crBackColor = crColor;
}

COLORREF LegacyParagraph::GetBackColor() const throw()
{
    return m_crBackColor;
}

void LegacyParagraph::SetHorizontalAlignment(HAlign align)
{
    m_hAlign = align;
}

LegacyParagraph::HAlign LegacyParagraph::GetHorizontalAlignment() const throw()
{
    return m_hAlign;
}

void LegacyParagraph::SetVerticalAlignment(VAlign align)
{
    m_vAlign = align;
}

LegacyParagraph::VAlign LegacyParagraph::GetVerticalAlignment() const throw()
{
    return m_vAlign;
}

void LegacyParagraph::SetLayoutRect(const RECT& rcLayout)
{
    m_rcLayout = rcLayout;
}

RECT LegacyParagraph::GetLayoutRect() const
{
    return m_rcLayout;
}

void LegacyParagraph::SetWordWrap(bool bWrap)
{
    m_bWordWrap = bWrap;
}

bool LegacyParagraph::IsWordWrap() const throw()
{
    return m_bWordWrap;
}

SIZE LegacyParagraph::Measure(Gdiplus::Graphics* pGraphics) const
{
    SIZE sz = { 0, 0 };
    if (!pGraphics || m_strText.IsEmpty())
        return sz;

    Gdiplus::Font font(
        m_strFontFamily,
        m_flFontSize,
        m_fontStyle,
        Gdiplus::UnitPixel);

    Gdiplus::RectF rcLayout(
        static_cast<Gdiplus::REAL>(m_rcLayout.left),
        static_cast<Gdiplus::REAL>(m_rcLayout.top),
        m_bWordWrap ? static_cast<Gdiplus::REAL>(m_rcLayout.right - m_rcLayout.left) : 10000.0f,
        static_cast<Gdiplus::REAL>(m_rcLayout.bottom - m_rcLayout.top));

    Gdiplus::RectF rcBounds;
    Gdiplus::StringFormat format;
    if (m_bWordWrap)
        format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
    format.SetAlignment(Gdiplus::StringAlignmentNear);

    pGraphics->MeasureString(
        m_strText,
        m_strText.GetLength(),
        &font,
        rcLayout,
        &format,
        &rcBounds);

    sz.cx = static_cast<LONG>(rcBounds.Width + 0.5f);
    sz.cy = static_cast<LONG>(rcBounds.Height + 0.5f);
    return sz;
}

float LegacyParagraph::MeasureHeight(Gdiplus::Graphics* pGraphics, float flMaxWidth) const
{
    if (!pGraphics || m_strText.IsEmpty())
        return 0.0f;

    Gdiplus::Font font(m_strFontFamily, m_flFontSize, m_fontStyle, Gdiplus::UnitPixel);

    Gdiplus::RectF rcLayout(0, 0, flMaxWidth, 10000.0f);
    Gdiplus::RectF rcBounds;
    Gdiplus::StringFormat format;
    if (m_bWordWrap)
        format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

    pGraphics->MeasureString(
        m_strText,
        m_strText.GetLength(),
        &font,
        rcLayout,
        &format,
        &rcBounds);

    return rcBounds.Height;
}

void LegacyParagraph::Draw(Gdiplus::Graphics* pGraphics, const RECT& rcBounds)
{
    if (!pGraphics || m_strText.IsEmpty())
        return;

    Gdiplus::Font font(m_strFontFamily, m_flFontSize, m_fontStyle, Gdiplus::UnitPixel);

    // Fill background
    Gdiplus::SolidBrush backBrush(Gdiplus::Color(
        GetRValue(m_crBackColor), GetGValue(m_crBackColor), GetBValue(m_crBackColor)));
    pGraphics->FillRectangle(&backBrush, rcBounds.left, rcBounds.top,
                              rcBounds.right - rcBounds.left,
                              rcBounds.bottom - rcBounds.top);

    // Compute text position based on alignment
    SIZE sz = Measure(pGraphics);
    int x = rcBounds.left;
    int y = rcBounds.top;

    switch (m_hAlign)
    {
    case HAlignCenter:
        x = rcBounds.left + ((rcBounds.right - rcBounds.left) - sz.cx) / 2;
        break;
    case HAlignRight:
        x = rcBounds.right - sz.cx;
        break;
    default:
        break;
    }

    switch (m_vAlign)
    {
    case VAlignMiddle:
        y = rcBounds.top + ((rcBounds.bottom - rcBounds.top) - sz.cy) / 2;
        break;
    case VAlignBottom:
        y = rcBounds.bottom - sz.cy;
        break;
    default:
        break;
    }

    Gdiplus::SolidBrush foreBrush(Gdiplus::Color(
        GetRValue(m_crForeColor), GetGValue(m_crForeColor), GetBValue(m_crForeColor)));

    pGraphics->DrawString(
        m_strText,
        m_strText.GetLength(),
        &font,
        Gdiplus::PointF(static_cast<Gdiplus::REAL>(x), static_cast<Gdiplus::REAL>(y)),
        &foreBrush);
}

// ============================================================================
// LegacyTextExtent implementation
// ============================================================================

LegacyTextExtent::LegacyTextExtent()
    : m_flFontSize(18.0f)
    , m_bBold(false)
    , m_bItalic(false)
    , m_bUnderline(false)
    , m_bStrikethrough(false)
    , m_crTextColor(RGB(255, 255, 255))
    , m_flMaxWidth(0.0f)
    , m_flMaxHeight(0.0f)
{
}

LegacyTextExtent::~LegacyTextExtent()
{
}

void LegacyTextExtent::SetText(LPCWSTR pszText)
{
    m_strText = pszText ? pszText : L"";
}

ATL::CString LegacyTextExtent::GetText() const
{
    return m_strText;
}

void LegacyTextExtent::SetFontName(LPCWSTR pszFontName)
{
    m_strFontName = pszFontName ? pszFontName : L"Segoe UI";
}

ATL::CString LegacyTextExtent::GetFontName() const
{
    return m_strFontName;
}

void LegacyTextExtent::SetFontSize(float flSize)
{
    m_flFontSize = std::max(1.0f, flSize);
}

float LegacyTextExtent::GetFontSize() const throw()
{
    return m_flFontSize;
}

void LegacyTextExtent::SetBold(bool bBold)
{
    m_bBold = bBold;
}

bool LegacyTextExtent::IsBold() const throw()
{
    return m_bBold;
}

void LegacyTextExtent::SetItalic(bool bItalic)
{
    m_bItalic = bItalic;
}

bool LegacyTextExtent::IsItalic() const throw()
{
    return m_bItalic;
}

void LegacyTextExtent::SetUnderline(bool bUnderline)
{
    m_bUnderline = bUnderline;
}

bool LegacyTextExtent::IsUnderline() const throw()
{
    return m_bUnderline;
}

void LegacyTextExtent::SetStrikethrough(bool bStrikethrough)
{
    m_bStrikethrough = bStrikethrough;
}

bool LegacyTextExtent::IsStrikethrough() const throw()
{
    return m_bStrikethrough;
}

void LegacyTextExtent::SetTextColor(COLORREF crColor)
{
    m_crTextColor = crColor;
}

COLORREF LegacyTextExtent::GetTextColor() const throw()
{
    return m_crTextColor;
}

void LegacyTextExtent::SetMaxWidth(float flMaxWidth)
{
    m_flMaxWidth = flMaxWidth;
}

float LegacyTextExtent::GetMaxWidth() const throw()
{
    return m_flMaxWidth;
}

void LegacyTextExtent::SetMaxHeight(float flMaxHeight)
{
    m_flMaxHeight = flMaxHeight;
}

float LegacyTextExtent::GetMaxHeight() const throw()
{
    return m_flMaxHeight;
}

SIZE LegacyTextExtent::Measure(const Gdiplus::Graphics* pGraphics) const
{
    SIZE sz = { 0, 0 };
    if (!pGraphics || m_strText.IsEmpty())
        return sz;

    Gdiplus::FontStyle style = Gdiplus::FontStyleRegular;
    if (m_bBold && m_bItalic)
        style = Gdiplus::FontStyleBoldItalic;
    else if (m_bBold)
        style = Gdiplus::FontStyleBold;
    else if (m_bItalic)
        style = Gdiplus::FontStyleItalic;
    if (m_bUnderline)
        style = static_cast<Gdiplus::FontStyle>(style | Gdiplus::FontStyleUnderline);
    if (m_bStrikethrough)
        style = static_cast<Gdiplus::FontStyle>(style | Gdiplus::FontStyleStrikeout);

    Gdiplus::Font font(m_strFontName, m_flFontSize, style, Gdiplus::UnitPixel);

    float flMaxW = (m_flMaxWidth > 0.0f) ? m_flMaxWidth : 10000.0f;
    float flMaxH = (m_flMaxHeight > 0.0f) ? m_flMaxHeight : 10000.0f;

    Gdiplus::RectF rcLayout(0.0f, 0.0f, flMaxW, flMaxH);
    Gdiplus::RectF rcBounds;

    const_cast<Gdiplus::Graphics*>(pGraphics)->MeasureString(
        m_strText, m_strText.GetLength(), &font, rcLayout, &rcBounds);

    sz.cx = static_cast<LONG>(rcBounds.Width + 0.5f);
    sz.cy = static_cast<LONG>(rcBounds.Height + 0.5f);
    return sz;
}

int LegacyTextExtent::GetLineCount() const throw()
{
    // Simple line count by counting newline characters
    if (m_strText.IsEmpty())
        return 0;

    int count = 1;
    for (int i = 0; i < m_strText.GetLength(); i++)
    {
        if (m_strText[i] == L'\n')
            count++;
    }
    return count;
}

int LegacyTextExtent::HitTestPoint(const POINT& pt) const
{
    UNREFERENCED_PARAMETER(pt);
    // In the full implementation, this would use GDI+ character range
    // hit-testing to determine which character index corresponds to
    // the given point.
    return -1;
}

int LegacyTextExtent::HitTestTextPosition(DWORD dwCharIndex) const
{
    UNREFERENCED_PARAMETER(dwCharIndex);
    // In the full implementation, this would compute the X,Y position
    // of the given character index within the text layout.
    return -1;
}
