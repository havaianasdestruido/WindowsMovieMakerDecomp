/*
 * LegacyText.h
 *
 * Legacy text measurement using GDI+ instead of DirectWrite.
 * LegacyExtent: Legacy extent/dimension computation.
 * LegacyParagraph: Legacy paragraph layout.
 * LegacyTransform: Legacy coordinate transform.
 *
 * RTTI: ?AVLegacyTextExtent@@, ?AVLegacyExtent@@,
 *       ?AVLegacyParagraph@@, ?AVLegacyTransform@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 * Used for compatibility with .wlmp project files created by
 * Windows Live Movie Maker 2010/2011 (version 14/15).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef LEGACY_TEXT_H
#define LEGACY_TEXT_H

#include "../../pch.h"

// ============================================================================
// LegacyTransform
// ============================================================================
// Legacy coordinate transform for positioning elements in the pre-DirectUI
// coordinate system. Uses GDI+ transforms with custom scaling and rotation.
//
class LegacyTransform
{
public:
    LegacyTransform();
    ~LegacyTransform();

    // GDI+ Matrix is non-copyable, so copying is done element-wise.
    LegacyTransform(const LegacyTransform& other);
    LegacyTransform& operator=(const LegacyTransform& other);

    // -- Translation --
    void SetOffsetX(float flX);
    float GetOffsetX() const throw();

    void SetOffsetY(float flY);
    float GetOffsetY() const throw();

    // -- Rotation --
    void SetRotation(float flDegrees);
    float GetRotation() const throw();

    // -- Scale --
    void SetScale(float flScaleX, float flScaleY);
    float GetScaleX() const throw();
    float GetScaleY() const throw();

    // -- Transform operations --
    POINT TransformPoint(const POINT& ptSrc) const;
    RECT  TransformRect(const RECT& rcSrc) const;
    POINT InverseTransformPoint(const POINT& ptSrc) const;

    // -- Matrix access --
    const Gdiplus::Matrix& GetMatrix() const throw();
    void SetMatrix(const Gdiplus::Matrix& matrix);

private:
    Gdiplus::Matrix m_matrix;
};

// ============================================================================
// LegacyExtent
// ============================================================================
// Legacy extent/dimension computation for pre-2012 project files.
// Used to compute element bounds in the older coordinate system where
// coordinates were based on 1000-unit normalized space rather than
// the Direct2D pixel-based layout used in version 16.
//
class LegacyExtent
{
public:
    LegacyExtent();
    ~LegacyExtent();

    // -- Dimensions --
    void SetWidth(float flWidth);
    float GetWidth() const throw();

    void SetHeight(float flHeight);
    float GetHeight() const throw();

    void SetDepth(float flDepth);
    float GetDepth() const throw();

    // -- Position --
    void SetX(float flX);
    float GetX() const throw();

    void SetY(float flY);
    float GetY() const throw();

    void SetZ(float flZ);
    float GetZ() const throw();

    // -- Bounds --
    RECT GetBounds() const;
    void SetBounds(const RECT& rc);

    // -- Normalization --
    void Normalize(UINT uContainerWidth, UINT uContainerHeight);
    void Denormalize(UINT uContainerWidth, UINT uContainerHeight);

    // -- Conversion --
    static LegacyExtent FromGdiplusRect(const Gdiplus::RectF& rc);
    Gdiplus::RectF ToGdiplusRect() const;

    // -- Operators --
    bool operator==(const LegacyExtent& other) const;
    bool operator!=(const LegacyExtent& other) const;

private:
    float m_flX;
    float m_flY;
    float m_flZ;
    float m_flWidth;
    float m_flHeight;
    float m_flDepth;
};

// ============================================================================
// LegacyParagraph
// ============================================================================
// Legacy paragraph layout using GDI+ text measurement instead of DirectWrite.
// Used for rendering titles and credits on systems without DirectWrite support
// or when processing older project files.
//
class LegacyParagraph
{
public:
    LegacyParagraph();
    ~LegacyParagraph();

    // -- Text --
    void SetText(LPCWSTR pszText);
    ATL::CString GetText() const;

    void SetFontFamily(LPCWSTR pszFontFamily);
    ATL::CString GetFontFamily() const;

    void SetFontSize(float flSize);
    float GetFontSize() const throw();

    void SetFontStyle(Gdiplus::FontStyle style);
    Gdiplus::FontStyle GetFontStyle() const throw();

    void SetForeColor(COLORREF crColor);
    COLORREF GetForeColor() const throw();

    void SetBackColor(COLORREF crColor);
    COLORREF GetBackColor() const throw();

    // -- Alignment --
    enum HAlign { HAlignLeft, HAlignCenter, HAlignRight };
    enum VAlign { VAlignTop, VAlignMiddle, VAlignBottom };

    void SetHorizontalAlignment(HAlign align);
    HAlign GetHorizontalAlignment() const throw();

    void SetVerticalAlignment(VAlign align);
    VAlign GetVerticalAlignment() const throw();

    // -- Layout --
    void SetLayoutRect(const RECT& rcLayout);
    RECT GetLayoutRect() const;

    void SetWordWrap(bool bWrap);
    bool IsWordWrap() const throw();

    // -- Measurement (GDI+) --
    SIZE Measure(Gdiplus::Graphics* pGraphics) const;
    float MeasureHeight(Gdiplus::Graphics* pGraphics, float flMaxWidth) const;

    // -- Rendering (GDI+) --
    void Draw(Gdiplus::Graphics* pGraphics, const RECT& rcBounds);

private:
    ATL::CString         m_strText;
    ATL::CString         m_strFontFamily;
    float                m_flFontSize;
    Gdiplus::FontStyle   m_fontStyle;
    COLORREF             m_crForeColor;
    COLORREF             m_crBackColor;
    HAlign               m_hAlign;
    VAlign               m_vAlign;
    RECT                 m_rcLayout;
    bool                 m_bWordWrap;
};

// ============================================================================
// LegacyTextExtent
// ============================================================================
// Legacy text extent measurement using GDI+ as a fallback when DirectWrite
// is unavailable. Provides text layout, wrapping, and hit-testing compatible
// with the Windows Live Movie Maker 2011 text engine.
//
class LegacyTextExtent
{
public:
    LegacyTextExtent();
    ~LegacyTextExtent();

    // -- Text content --
    void SetText(LPCWSTR pszText);
    ATL::CString GetText() const;

    // -- Font --
    void SetFontName(LPCWSTR pszFontName);
    ATL::CString GetFontName() const;

    void SetFontSize(float flSize);
    float GetFontSize() const throw();

    void SetBold(bool bBold);
    bool IsBold() const throw();

    void SetItalic(bool bItalic);
    bool IsItalic() const throw();

    void SetUnderline(bool bUnderline);
    bool IsUnderline() const throw();

    void SetStrikethrough(bool bStrikethrough);
    bool IsStrikethrough() const throw();

    void SetTextColor(COLORREF crColor);
    COLORREF GetTextColor() const throw();

    // -- Layout --
    void SetMaxWidth(float flMaxWidth);
    float GetMaxWidth() const throw();

    void SetMaxHeight(float flMaxHeight);
    float GetMaxHeight() const throw();

    // -- Measurement (GDI+) --
    SIZE Measure(const Gdiplus::Graphics* pGraphics) const;
    int  GetLineCount() const throw();

    // -- Hit testing --
    int  HitTestPoint(const POINT& pt) const;
    int  HitTestTextPosition(DWORD dwCharIndex) const;

private:
    ATL::CString m_strText;
    ATL::CString m_strFontName;
    float        m_flFontSize;
    bool         m_bBold;
    bool         m_bItalic;
    bool         m_bUnderline;
    bool         m_bStrikethrough;
    COLORREF     m_crTextColor;
    float        m_flMaxWidth;
    float        m_flMaxHeight;
};

#endif // LEGACY_TEXT_H
