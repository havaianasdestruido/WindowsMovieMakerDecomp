/*
 * UXBrush.h
 *
 * GDI+/Direct2D brush wrapper for the Sundance UI layer. Provides a
 * unified brush abstraction that can render using either GDI+ (Gdiplus::Brush)
 * or Direct2D (ID2D1Brush) depending on the available rendering context.
 * Used by behavior classes that need to paint colored or textured fills.
 *
 * RTTI: ?AVUXBrush@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_UXBRUSH_H
#define SUNDANCE_UXBRUSH_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"

namespace Sundance
{

class UXBrush
{
public:
    // -- Brush types --
    enum BrushType
    {
        BrushTypeNone = 0,
        BrushTypeSolid,
        BrushTypeLinearGradient,
        BrushTypeRadialGradient,
        BrushTypeBitmap,
        BrushTypeHatch
    };

    UXBrush();
    UXBrush(COLORREF clrSolid);
    UXBrush(BYTE r, BYTE g, BYTE b, BYTE a = 255);
    ~UXBrush();

    // -- Copy --
    UXBrush(const UXBrush& other);
    UXBrush& operator=(const UXBrush& other);

    // -- Factory methods --
    static UXBrush CreateSolid(COLORREF clrSolid);
    static UXBrush CreateSolid(BYTE r, BYTE g, BYTE b, BYTE a = 255);
    static UXBrush CreateLinearGradient(POINT ptStart, POINT ptEnd,
                COLORREF clrStart, COLORREF clrEnd);
    static UXBrush CreateRadialGradient(POINT ptCenter, int nRadius,
                COLORREF clrCenter, COLORREF clrEdge);
    static UXBrush CreateFromBitmap(HBITMAP hBitmap);
    static UXBrush CreateHatch(int nHatchStyle, COLORREF clrForeground, COLORREF clrBackground);

    // -- Type queries --
    BrushType GetType() const throw();
    bool IsNone() const throw();
    bool IsValid() const throw();

    // -- Color accessors --
    COLORREF GetColor() const throw();
    void     SetColor(COLORREF clrSolid);
    void     SetOpacity(BYTE a);

    // -- Gradient configuration --
    void SetGradientStart(COLORREF clrStart);
    void SetGradientEnd(COLORREF clrEnd);
    void SetGradientPoints(POINT ptStart, POINT ptEnd);

    // -- GDI+ rendering --
    Gdiplus::Brush* CreateGdiPlusBrush() const;
    Gdiplus::SolidBrush* CreateGdiPlusSolidBrush() const;

    // -- Direct2D rendering --
    ID2D1Brush* CreateD2DBrush(ID2D1RenderTarget* pRenderTarget) const;
    ID2D1SolidColorBrush* CreateD2DSolidColorBrush(ID2D1RenderTarget* pRenderTarget) const;

    // -- HDC rendering --
    HBRUSH CreateHBrush() const;
    HPEN   CreateHPen(int nWidth = 1) const;

    // -- Pre-multiply alpha --
    static COLORREF PreMultiplyAlpha(COLORREF clr, BYTE a);

    // -- Cleanup --
    void Release();

private:
    BrushType   m_type;
    COLORREF    m_clrPrimary;
    COLORREF    m_clrSecondary;
    BYTE        m_alpha;
    POINT       m_ptGradientStart;
    POINT       m_ptGradientEnd;
    int         m_nGradientRadius;
    HBITMAP     m_hBitmap;
    int         m_nHatchStyle;

    void InitializeDefaults();
    void CopyFrom(const UXBrush& other);
};

// ============================================================================
// UXBrushSet -- collection of named brushes for theme support
// ============================================================================
class UXBrushSet
{
public:
    UXBrushSet();
    ~UXBrushSet();

    // -- Theme brushes --
    HRESULT LoadDefaultBrushes();
    HRESULT LoadTheme(LPCWSTR pszThemeName);

    UXBrush* GetBrush(LPCWSTR pszBrushName);
    const UXBrush* GetBrush(LPCWSTR pszBrushName) const;

    // -- Built-in brush names --
    static const WCHAR kBrushTimelineBackground[];
    static const WCHAR kBrushTimelineItemDefault[];
    static const WCHAR kBrushTimelineItemSelected[];
    static const WCHAR kBrushTimelineItemHover[];
    static const WCHAR kBrushTimelineTrackEven[];
    static const WCHAR kBrushTimelineTrackOdd[];
    static const WCHAR kBrushPlaybackCursor[];
    static const WCHAR kBrushTimeRuler[];
    static const WCHAR kBrushTrimHandle[];
    static const WCHAR kBrushSelectionMarquee[];
    static const WCHAR kBrushCommandBarBackground[];
    static const WCHAR kBrushCommandBarHover[];
    static const WCHAR kBrushCommandBarPressed[];
    static const WCHAR kBrushFullscreenBackground[];
    static const WCHAR kBrushProgressBar[];
    static const WCHAR kBrushProgressBackground[];

private:
    std::map<ATL::CString, UXBrush> m_brushes;
};

} // namespace Sundance

#endif // SUNDANCE_UXBRUSH_H
