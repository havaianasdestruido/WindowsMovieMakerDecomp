/*
 * UXBrush.cpp
 *
 * Implementation of the GDI+/Direct2D brush wrapper.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "UXBrush.h"

namespace Sundance
{

// ============================================================================
// UXBrush
// ============================================================================
UXBrush::UXBrush()
    : m_type(BrushTypeNone)
    , m_clrPrimary(0)
    , m_clrSecondary(0)
    , m_alpha(255)
    , m_hBitmap(NULL)
    , m_nHatchStyle(0)
{
    InitializeDefaults();
}

UXBrush::UXBrush(COLORREF clrSolid)
    : m_type(BrushTypeSolid)
    , m_clrPrimary(clrSolid)
    , m_clrSecondary(0)
    , m_alpha(255)
    , m_hBitmap(NULL)
    , m_nHatchStyle(0)
{
    InitializeDefaults();
}

UXBrush::UXBrush(BYTE r, BYTE g, BYTE b, BYTE a)
    : m_type(BrushTypeSolid)
    , m_clrPrimary(RGB(r, g, b))
    , m_clrSecondary(0)
    , m_alpha(a)
    , m_hBitmap(NULL)
    , m_nHatchStyle(0)
{
    InitializeDefaults();
}

UXBrush::~UXBrush()
{
    Release();
}

UXBrush::UXBrush(const UXBrush& other)
    : m_type(BrushTypeNone)
    , m_clrPrimary(0)
    , m_clrSecondary(0)
    , m_alpha(255)
    , m_hBitmap(NULL)
    , m_nHatchStyle(0)
{
    InitializeDefaults();
    CopyFrom(other);
}

UXBrush& UXBrush::operator=(const UXBrush& other)
{
    if (this != &other)
    {
        Release();
        CopyFrom(other);
    }
    return *this;
}

UXBrush UXBrush::CreateSolid(COLORREF clrSolid)
{
    UXBrush brush;
    brush.m_type = BrushTypeSolid;
    brush.m_clrPrimary = clrSolid;
    brush.m_alpha = 255;
    return brush;
}

UXBrush UXBrush::CreateSolid(BYTE r, BYTE g, BYTE b, BYTE a)
{
    UXBrush brush;
    brush.m_type = BrushTypeSolid;
    brush.m_clrPrimary = RGB(r, g, b);
    brush.m_alpha = a;
    return brush;
}

UXBrush UXBrush::CreateLinearGradient(POINT ptStart, POINT ptEnd,
    COLORREF clrStart, COLORREF clrEnd)
{
    UXBrush brush;
    brush.m_type = BrushTypeLinearGradient;
    brush.m_clrPrimary = clrStart;
    brush.m_clrSecondary = clrEnd;
    brush.m_ptGradientStart = ptStart;
    brush.m_ptGradientEnd = ptEnd;
    brush.m_alpha = 255;
    return brush;
}

UXBrush UXBrush::CreateRadialGradient(POINT ptCenter, int nRadius,
    COLORREF clrCenter, COLORREF clrEdge)
{
    UXBrush brush;
    brush.m_type = BrushTypeRadialGradient;
    brush.m_clrPrimary = clrCenter;
    brush.m_clrSecondary = clrEdge;
    brush.m_nGradientRadius = nRadius;
    brush.m_ptGradientStart = ptCenter;
    brush.m_alpha = 255;
    return brush;
}

UXBrush UXBrush::CreateFromBitmap(HBITMAP hBitmap)
{
    UXBrush brush;
    brush.m_type = BrushTypeBitmap;
    brush.m_hBitmap = hBitmap;
    brush.m_alpha = 255;
    return brush;
}

UXBrush UXBrush::CreateHatch(int nHatchStyle, COLORREF clrForeground, COLORREF clrBackground)
{
    UXBrush brush;
    brush.m_type = BrushTypeHatch;
    brush.m_clrPrimary = clrForeground;
    brush.m_clrSecondary = clrBackground;
    brush.m_nHatchStyle = nHatchStyle;
    brush.m_alpha = 255;
    return brush;
}

UXBrush::BrushType UXBrush::GetType() const throw() { return m_type; }
bool UXBrush::IsNone() const throw() { return m_type == BrushTypeNone; }
bool UXBrush::IsValid() const throw() { return m_type != BrushTypeNone; }

COLORREF UXBrush::GetColor() const throw() { return m_clrPrimary; }

void UXBrush::SetColor(COLORREF clrSolid)
{
    m_clrPrimary = clrSolid;
    m_type = BrushTypeSolid;
}

void UXBrush::SetOpacity(BYTE a) { m_alpha = a; }

void UXBrush::SetGradientStart(COLORREF clrStart) { m_clrPrimary = clrStart; }
void UXBrush::SetGradientEnd(COLORREF clrEnd) { m_clrSecondary = clrEnd; }

void UXBrush::SetGradientPoints(POINT ptStart, POINT ptEnd)
{
    m_ptGradientStart = ptStart;
    m_ptGradientEnd = ptEnd;
}

Gdiplus::Brush* UXBrush::CreateGdiPlusBrush() const
{
    switch (m_type)
    {
    case BrushTypeSolid:
        return CreateGdiPlusSolidBrush();

    case BrushTypeLinearGradient:
    {
        Gdiplus::Color clrStart(m_alpha, GetRValue(m_clrPrimary), GetGValue(m_clrPrimary), GetBValue(m_clrPrimary));
        Gdiplus::Color clrEnd(m_alpha, GetRValue(m_clrSecondary), GetGValue(m_clrSecondary), GetBValue(m_clrSecondary));
        return new Gdiplus::LinearGradientBrush(
            Gdiplus::Point(m_ptGradientStart.x, m_ptGradientStart.y),
            Gdiplus::Point(m_ptGradientEnd.x, m_ptGradientEnd.y),
            clrStart, clrEnd);
    }

    case BrushTypeRadialGradient:
    {
        Gdiplus::Color clrCenter(m_alpha, GetRValue(m_clrPrimary), GetGValue(m_clrPrimary), GetBValue(m_clrPrimary));
        Gdiplus::Color clrEdge(m_alpha, GetRValue(m_clrSecondary), GetGValue(m_clrSecondary), GetBValue(m_clrSecondary));
        Gdiplus::RadialGradientBrush* pBrush = new Gdiplus::RadialGradientBrush(
            Gdiplus::Point(m_ptGradientStart.x, m_ptGradientStart.y),
            static_cast<Gdiplus::REAL>(m_nGradientRadius),
            clrCenter, clrEdge);
        return pBrush;
    }

    case BrushTypeHatch:
    {
        Gdiplus::Color clrFg(m_alpha, GetRValue(m_clrPrimary), GetGValue(m_clrPrimary), GetBValue(m_clrPrimary));
        Gdiplus::Color clrBg(m_alpha, GetRValue(m_clrSecondary), GetGValue(m_clrSecondary), GetBValue(m_clrSecondary));
        return new Gdiplus::HatchBrush(
            static_cast<Gdiplus::HatchStyle>(m_nHatchStyle), clrFg, clrBg);
    }

    default:
        return NULL;
    }
}

Gdiplus::SolidBrush* UXBrush::CreateGdiPlusSolidBrush() const
{
    if (m_type != BrushTypeSolid)
        return NULL;

    Gdiplus::Color clr(m_alpha, GetRValue(m_clrPrimary), GetGValue(m_clrPrimary), GetBValue(m_clrPrimary));
    return new Gdiplus::SolidBrush(clr);
}

ID2D1Brush* UXBrush::CreateD2DBrush(ID2D1RenderTarget* pRenderTarget) const
{
    if (!pRenderTarget)
        return NULL;

    switch (m_type)
    {
    case BrushTypeSolid:
        return CreateD2DSolidColorBrush(pRenderTarget);

    case BrushTypeLinearGradient:
    {
        D2D1_GRADIENT_STOP stops[2];
        stops[0].color = D2D1::ColorF(GetRValue(m_clrPrimary) / 255.0f,
            GetGValue(m_clrPrimary) / 255.0f, GetBValue(m_clrPrimary) / 255.0f,
            m_alpha / 255.0f);
        stops[0].position = 0.0f;
        stops[1].color = D2D1::ColorF(GetRValue(m_clrSecondary) / 255.0f,
            GetGValue(m_clrSecondary) / 255.0f, GetBValue(m_clrSecondary) / 255.0f,
            m_alpha / 255.0f);
        stops[1].position = 1.0f;

        ID2D1LinearGradientBrush* pBrush = NULL;
        D2D1_POINT_2F startPoint = D2D1::Point2F(
            static_cast<float>(m_ptGradientStart.x),
            static_cast<float>(m_ptGradientStart.y));
        D2D1_POINT_2F endPoint = D2D1::Point2F(
            static_cast<float>(m_ptGradientEnd.x),
            static_cast<float>(m_ptGradientEnd.y));

        HRESULT hr = pRenderTarget->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(startPoint, endPoint),
            stops, 2, &pBrush);

        return SUCCEEDED(hr) ? pBrush : NULL;
    }

    default:
        return NULL;
    }
}

ID2D1SolidColorBrush* UXBrush::CreateD2DSolidColorBrush(ID2D1RenderTarget* pRenderTarget) const
{
    if (!pRenderTarget)
        return NULL;

    D2D1_COLOR_F color = D2D1::ColorF(
        GetRValue(m_clrPrimary) / 255.0f,
        GetGValue(m_clrPrimary) / 255.0f,
        GetBValue(m_clrPrimary) / 255.0f,
        m_alpha / 255.0f);

    ID2D1SolidColorBrush* pBrush = NULL;
    pRenderTarget->CreateSolidColorBrush(color, &pBrush);
    return pBrush;
}

HBRUSH UXBrush::CreateHBrush() const
{
    if (m_type == BrushTypeSolid)
    {
        return CreateSolidBrush(m_clrPrimary);
    }
    return NULL;
}

HPEN UXBrush::CreateHPen(int nWidth) const
{
    if (m_type == BrushTypeSolid)
    {
        return CreatePen(PS_SOLID, nWidth, m_clrPrimary);
    }
    return NULL;
}

COLORREF UXBrush::PreMultiplyAlpha(COLORREF clr, BYTE a)
{
    BYTE r = static_cast<BYTE>((GetRValue(clr) * a) / 255);
    BYTE g = static_cast<BYTE>((GetGValue(clr) * a) / 255);
    BYTE b = static_cast<BYTE>((GetBValue(clr) * a) / 255);
    return RGB(r, g, b);
}

void UXBrush::Release()
{
    if (m_hBitmap)
    {
        DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
    }
    m_type = BrushTypeNone;
}

void UXBrush::InitializeDefaults()
{
    m_ptGradientStart.x = 0;
    m_ptGradientStart.y = 0;
    m_ptGradientEnd.x = 0;
    m_ptGradientEnd.y = 0;
    m_nGradientRadius = 0;
}

void UXBrush::CopyFrom(const UXBrush& other)
{
    m_type = other.m_type;
    m_clrPrimary = other.m_clrPrimary;
    m_clrSecondary = other.m_clrSecondary;
    m_alpha = other.m_alpha;
    m_ptGradientStart = other.m_ptGradientStart;
    m_ptGradientEnd = other.m_ptGradientEnd;
    m_nGradientRadius = other.m_nGradientRadius;
    m_nHatchStyle = other.m_nHatchStyle;

    if (other.m_hBitmap)
    {
        m_hBitmap = static_cast<HBITMAP>(CopyImage(other.m_hBitmap, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR));
    }
    else
    {
        m_hBitmap = NULL;
    }
}

// ============================================================================
// UXBrushSet -- built-in brush name constants
// ============================================================================
const WCHAR UXBrushSet::kBrushTimelineBackground[]      = L"TimelineBackground";
const WCHAR UXBrushSet::kBrushTimelineItemDefault[]     = L"TimelineItemDefault";
const WCHAR UXBrushSet::kBrushTimelineItemSelected[]    = L"TimelineItemSelected";
const WCHAR UXBrushSet::kBrushTimelineItemHover[]       = L"TimelineItemHover";
const WCHAR UXBrushSet::kBrushTimelineTrackEven[]       = L"TimelineTrackEven";
const WCHAR UXBrushSet::kBrushTimelineTrackOdd[]        = L"TimelineTrackOdd";
const WCHAR UXBrushSet::kBrushPlaybackCursor[]          = L"PlaybackCursor";
const WCHAR UXBrushSet::kBrushTimeRuler[]               = L"TimeRuler";
const WCHAR UXBrushSet::kBrushTrimHandle[]              = L"TrimHandle";
const WCHAR UXBrushSet::kBrushSelectionMarquee[]        = L"SelectionMarquee";
const WCHAR UXBrushSet::kBrushCommandBarBackground[]    = L"CommandBarBackground";
const WCHAR UXBrushSet::kBrushCommandBarHover[]         = L"CommandBarHover";
const WCHAR UXBrushSet::kBrushCommandBarPressed[]       = L"CommandBarPressed";
const WCHAR UXBrushSet::kBrushFullscreenBackground[]    = L"FullscreenBackground";
const WCHAR UXBrushSet::kBrushProgressBar[]             = L"ProgressBar";
const WCHAR UXBrushSet::kBrushProgressBackground[]     = L"ProgressBackground";

UXBrushSet::UXBrushSet() {}
UXBrushSet::~UXBrushSet() {}

HRESULT UXBrushSet::LoadDefaultBrushes()
{
    m_brushes.clear();

    m_brushes[kBrushTimelineBackground]   = UXBrush::CreateSolid(RGB(40, 40, 40));
    m_brushes[kBrushTimelineItemDefault]  = UXBrush::CreateSolid(RGB(200, 220, 240));
    m_brushes[kBrushTimelineItemSelected] = UXBrush::CreateSolid(RGB(0, 120, 215));
    m_brushes[kBrushTimelineItemHover]    = UXBrush::CreateSolid(RGB(180, 210, 240));
    m_brushes[kBrushTimelineTrackEven]    = UXBrush::CreateSolid(RGB(240, 240, 240));
    m_brushes[kBrushTimelineTrackOdd]     = UXBrush::CreateSolid(RGB(255, 255, 255));
    m_brushes[kBrushPlaybackCursor]       = UXBrush::CreateSolid(RGB(255, 0, 0));
    m_brushes[kBrushTimeRuler]            = UXBrush::CreateSolid(RGB(220, 220, 220));
    m_brushes[kBrushTrimHandle]           = UXBrush::CreateSolid(RGB(0, 80, 160));
    m_brushes[kBrushSelectionMarquee]     = UXBrush::CreateSolid(RGB(0, 120, 215));
    m_brushes[kBrushCommandBarBackground] = UXBrush::CreateSolid(RGB(240, 240, 240));
    m_brushes[kBrushCommandBarHover]      = UXBrush::CreateSolid(RGB(220, 235, 250));
    m_brushes[kBrushCommandBarPressed]    = UXBrush::CreateSolid(RGB(200, 220, 240));
    m_brushes[kBrushFullscreenBackground] = UXBrush::CreateSolid(RGB(0, 0, 0));
    m_brushes[kBrushProgressBar]          = UXBrush::CreateSolid(RGB(0, 120, 215));
    m_brushes[kBrushProgressBackground]  = UXBrush::CreateSolid(RGB(220, 220, 220));

    return S_OK;
}

HRESULT UXBrushSet::LoadTheme(LPCWSTR pszThemeName)
{
    UNREFERENCED_PARAMETER(pszThemeName);
    return LoadDefaultBrushes();
}

UXBrush* UXBrushSet::GetBrush(LPCWSTR pszBrushName)
{
    if (!pszBrushName) return NULL;
    auto it = m_brushes.find(ATL::CString(pszBrushName));
    return (it != m_brushes.end()) ? &it->second : NULL;
}

const UXBrush* UXBrushSet::GetBrush(LPCWSTR pszBrushName) const
{
    if (!pszBrushName) return NULL;
    auto it = m_brushes.find(ATL::CString(pszBrushName));
    return (it != m_brushes.end()) ? &it->second : NULL;
}

} // namespace Sundance
