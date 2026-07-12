/*
 * TextProperty.h
 *
 * Text formatting property for title and credit overlays. Stores font,
 * color, size, and alignment properties used by the X3D title rendering
 * engine.
 *
 * RTTI: ?AVTextProperty@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef TEXTPROPERTY_H
#define TEXTPROPERTY_H

#include "../StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// TextAlignment
// ============================================================================
enum TextAlignment
{
    TextAlignmentLeft    = 0,
    TextAlignmentCenter  = 1,
    TextAlignmentRight   = 2
};

// ============================================================================
// TextProperty
// ============================================================================
// Complete set of text formatting properties for a title or credit overlay.
// Includes font family, size, color, style flags, background, alignment,
// and layout properties for the DirectUI text rendering pipeline.
//
class STORYBOARD_API TextProperty
{
public:
    TextProperty();
    ~TextProperty();

    // -- Copy semantics --
    TextProperty(const TextProperty& other);
    TextProperty& operator=(const TextProperty& other);

    // -- Font family --
    ATL::CString GetFontFamily() const;
    void SetFontFamily(LPCWSTR pszFontFamily);

    // -- Font size (points) --
    float GetFontSize() const throw();
    void SetFontSize(float flSize) throw();

    // -- Font color (ARGB) --
    DWORD GetFontColor() const throw();
    void SetFontColor(DWORD dwColor) throw();

    // -- Style flags --
    bool IsBold() const throw();
    void SetBold(bool fBold) throw();

    bool IsItalic() const throw();
    void SetItalic(bool fItalic) throw();

    bool IsUnderline() const throw();
    void SetUnderline(bool fUnderline) throw();

    bool IsStrikethrough() const throw();
    void SetStrikethrough(bool fStrikethrough) throw();

    // -- Background --
    DWORD GetBackgroundColor() const throw();
    void SetBackgroundColor(DWORD dwColor) throw();

    bool HasBackground() const throw();
    void SetHasBackground(bool fHas) throw();

    // -- Outline --
    DWORD GetOutlineColor() const throw();
    void SetOutlineColor(DWORD dwColor) throw();

    float GetOutlineWidth() const throw();
    void SetOutlineWidth(float flWidth) throw();

    bool HasOutline() const throw();
    void SetHasOutline(bool fHas) throw();

    // -- Shadow --
    DWORD GetShadowColor() const throw();
    void SetShadowColor(DWORD dwColor) throw();

    float GetShadowOffsetX() const throw();
    float GetShadowOffsetY() const throw();
    void SetShadowOffset(float flX, float flY) throw();

    bool HasShadow() const throw();
    void SetHasShadow(bool fHas) throw();

    // -- Alignment --
    TextAlignment GetAlignment() const throw();
    void SetAlignment(TextAlignment alignment) throw();

    // -- Word wrap --
    bool IsWordWrapEnabled() const throw();
    void SetWordWrapEnabled(bool fEnabled) throw();

    // -- Position (normalized 0.0 - 1.0) --
    double GetPositionX() const throw();
    double GetPositionY() const throw();
    void SetPosition(double dblX, double dblY) throw();

    // -- Max width / height (0 = auto) --
    double GetMaxWidth() const throw();
    void SetMaxWidth(double dblWidth) throw();

    double GetMaxHeight() const throw();
    void SetMaxHeight(double dblHeight) throw();

    // -- Line spacing (1.0 = normal) --
    double GetLineSpacing() const throw();
    void SetLineSpacing(double dblSpacing) throw();

    // -- Comparison --
    bool operator==(const TextProperty& other) const;
    bool operator!=(const TextProperty& other) const;

    // -- Merge (apply non-default properties from source) --
    void MergeFrom(const TextProperty& source);

    // -- Is default (all properties at default values) --
    bool IsDefault() const throw();

private:
    ATL::CString m_strFontFamily;
    float       m_flFontSize;
    DWORD       m_dwFontColor;
    bool        m_fBold;
    bool        m_fItalic;
    bool        m_fUnderline;
    bool        m_fStrikethrough;
    DWORD       m_dwBackgroundColor;
    bool        m_fHasBackground;
    DWORD       m_dwOutlineColor;
    float       m_flOutlineWidth;
    bool        m_fHasOutline;
    DWORD       m_dwShadowColor;
    float       m_flShadowOffsetX;
    float       m_flShadowOffsetY;
    bool        m_fHasShadow;
    TextAlignment m_alignment;
    bool        m_fWordWrapEnabled;
    double      m_dblPositionX;
    double      m_dblPositionY;
    double      m_dblMaxWidth;
    double      m_dblMaxHeight;
    double      m_dblLineSpacing;
};

// ============================================================================
// TextPropertySet
// ============================================================================
// Collection of TextProperty objects, indexed by title/credits slot.
// Manages the text properties for multiple overlay positions.
//
class STORYBOARD_API TextPropertySet
{
public:
    TextPropertySet();
    ~TextPropertySet();

    // -- Add/Remove --
    size_t AddProperty(const TextProperty& prop);
    void RemoveProperty(size_t nIndex);
    void RemoveAll();

    // -- Access --
    size_t GetCount() const throw();
    TextProperty& GetAt(size_t nIndex);
    const TextProperty& GetAt(size_t nIndex) const;

    // -- Find first with matching font --
    int FindByFontFamily(LPCWSTR pszFontFamily) const;

private:
    ATL::CAtlArray<TextProperty> m_arrProperties;
};

} // namespace StoryboardManager

#endif // TEXTPROPERTY_H
