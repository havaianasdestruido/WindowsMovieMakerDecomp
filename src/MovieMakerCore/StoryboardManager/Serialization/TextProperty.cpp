/*
 * TextProperty.cpp
 *
 * Implementation of text formatting properties for title and credit
 * overlays in the StoryboardManager serialization system.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "TextProperty.h"

namespace StoryboardManager
{

// ============================================================================
// TextProperty implementation
// ============================================================================

TextProperty::TextProperty()
    : m_flFontSize(24.0f)
    , m_dwFontColor(0xFFFFFFFF)
    , m_fBold(false)
    , m_fItalic(false)
    , m_fUnderline(false)
    , m_fStrikethrough(false)
    , m_dwBackgroundColor(0x00000000)
    , m_fHasBackground(false)
    , m_dwOutlineColor(0xFF000000)
    , m_flOutlineWidth(0.0f)
    , m_fHasOutline(false)
    , m_dwShadowColor(0x80000000)
    , m_flShadowOffsetX(1.0f)
    , m_flShadowOffsetY(1.0f)
    , m_fHasShadow(false)
    , m_alignment(TextAlignmentCenter)
    , m_fWordWrapEnabled(true)
    , m_dblPositionX(0.5)
    , m_dblPositionY(0.5)
    , m_dblMaxWidth(0.0)
    , m_dblMaxHeight(0.0)
    , m_dblLineSpacing(1.2)
{
}

TextProperty::~TextProperty()
{
}

TextProperty::TextProperty(const TextProperty& other)
    : m_strFontFamily(other.m_strFontFamily)
    , m_flFontSize(other.m_flFontSize)
    , m_dwFontColor(other.m_dwFontColor)
    , m_fBold(other.m_fBold)
    , m_fItalic(other.m_fItalic)
    , m_fUnderline(other.m_fUnderline)
    , m_fStrikethrough(other.m_fStrikethrough)
    , m_dwBackgroundColor(other.m_dwBackgroundColor)
    , m_fHasBackground(other.m_fHasBackground)
    , m_dwOutlineColor(other.m_dwOutlineColor)
    , m_flOutlineWidth(other.m_flOutlineWidth)
    , m_fHasOutline(other.m_fHasOutline)
    , m_dwShadowColor(other.m_dwShadowColor)
    , m_flShadowOffsetX(other.m_flShadowOffsetX)
    , m_flShadowOffsetY(other.m_flShadowOffsetY)
    , m_fHasShadow(other.m_fHasShadow)
    , m_alignment(other.m_alignment)
    , m_fWordWrapEnabled(other.m_fWordWrapEnabled)
    , m_dblPositionX(other.m_dblPositionX)
    , m_dblPositionY(other.m_dblPositionY)
    , m_dblMaxWidth(other.m_dblMaxWidth)
    , m_dblMaxHeight(other.m_dblMaxHeight)
    , m_dblLineSpacing(other.m_dblLineSpacing)
{
}

TextProperty& TextProperty::operator=(const TextProperty& other)
{
    if (this != &other)
    {
        m_strFontFamily = other.m_strFontFamily;
        m_flFontSize = other.m_flFontSize;
        m_dwFontColor = other.m_dwFontColor;
        m_fBold = other.m_fBold;
        m_fItalic = other.m_fItalic;
        m_fUnderline = other.m_fUnderline;
        m_fStrikethrough = other.m_fStrikethrough;
        m_dwBackgroundColor = other.m_dwBackgroundColor;
        m_fHasBackground = other.m_fHasBackground;
        m_dwOutlineColor = other.m_dwOutlineColor;
        m_flOutlineWidth = other.m_flOutlineWidth;
        m_fHasOutline = other.m_fHasOutline;
        m_dwShadowColor = other.m_dwShadowColor;
        m_flShadowOffsetX = other.m_flShadowOffsetX;
        m_flShadowOffsetY = other.m_flShadowOffsetY;
        m_fHasShadow = other.m_fHasShadow;
        m_alignment = other.m_alignment;
        m_fWordWrapEnabled = other.m_fWordWrapEnabled;
        m_dblPositionX = other.m_dblPositionX;
        m_dblPositionY = other.m_dblPositionY;
        m_dblMaxWidth = other.m_dblMaxWidth;
        m_dblMaxHeight = other.m_dblMaxHeight;
        m_dblLineSpacing = other.m_dblLineSpacing;
    }
    return *this;
}

// -- Font family --
ATL::CString TextProperty::GetFontFamily() const
{
    return m_strFontFamily;
}

void TextProperty::SetFontFamily(LPCWSTR pszFontFamily)
{
    m_strFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
}

// -- Font size --
float TextProperty::GetFontSize() const throw()
{
    return m_flFontSize;
}

void TextProperty::SetFontSize(float flSize) throw()
{
    m_flFontSize = flSize;
}

// -- Font color --
DWORD TextProperty::GetFontColor() const throw()
{
    return m_dwFontColor;
}

void TextProperty::SetFontColor(DWORD dwColor) throw()
{
    m_dwFontColor = dwColor;
}

// -- Style flags --
bool TextProperty::IsBold() const throw()
{
    return m_fBold;
}

void TextProperty::SetBold(bool fBold) throw()
{
    m_fBold = fBold;
}

bool TextProperty::IsItalic() const throw()
{
    return m_fItalic;
}

void TextProperty::SetItalic(bool fItalic) throw()
{
    m_fItalic = fItalic;
}

bool TextProperty::IsUnderline() const throw()
{
    return m_fUnderline;
}

void TextProperty::SetUnderline(bool fUnderline) throw()
{
    m_fUnderline = fUnderline;
}

bool TextProperty::IsStrikethrough() const throw()
{
    return m_fStrikethrough;
}

void TextProperty::SetStrikethrough(bool fStrikethrough) throw()
{
    m_fStrikethrough = fStrikethrough;
}

// -- Background --
DWORD TextProperty::GetBackgroundColor() const throw()
{
    return m_dwBackgroundColor;
}

void TextProperty::SetBackgroundColor(DWORD dwColor) throw()
{
    m_dwBackgroundColor = dwColor;
}

bool TextProperty::HasBackground() const throw()
{
    return m_fHasBackground;
}

void TextProperty::SetHasBackground(bool fHas) throw()
{
    m_fHasBackground = fHas;
}

// -- Outline --
DWORD TextProperty::GetOutlineColor() const throw()
{
    return m_dwOutlineColor;
}

void TextProperty::SetOutlineColor(DWORD dwColor) throw()
{
    m_dwOutlineColor = dwColor;
}

float TextProperty::GetOutlineWidth() const throw()
{
    return m_flOutlineWidth;
}

void TextProperty::SetOutlineWidth(float flWidth) throw()
{
    m_flOutlineWidth = flWidth;
}

bool TextProperty::HasOutline() const throw()
{
    return m_fHasOutline;
}

void TextProperty::SetHasOutline(bool fHas) throw()
{
    m_fHasOutline = fHas;
}

// -- Shadow --
DWORD TextProperty::GetShadowColor() const throw()
{
    return m_dwShadowColor;
}

void TextProperty::SetShadowColor(DWORD dwColor) throw()
{
    m_dwShadowColor = dwColor;
}

float TextProperty::GetShadowOffsetX() const throw()
{
    return m_flShadowOffsetX;
}

float TextProperty::GetShadowOffsetY() const throw()
{
    return m_flShadowOffsetY;
}

void TextProperty::SetShadowOffset(float flX, float flY) throw()
{
    m_flShadowOffsetX = flX;
    m_flShadowOffsetY = flY;
}

bool TextProperty::HasShadow() const throw()
{
    return m_fHasShadow;
}

void TextProperty::SetHasShadow(bool fHas) throw()
{
    m_fHasShadow = fHas;
}

// -- Alignment --
TextAlignment TextProperty::GetAlignment() const throw()
{
    return m_alignment;
}

void TextProperty::SetAlignment(TextAlignment alignment) throw()
{
    m_alignment = alignment;
}

// -- Word wrap --
bool TextProperty::IsWordWrapEnabled() const throw()
{
    return m_fWordWrapEnabled;
}

void TextProperty::SetWordWrapEnabled(bool fEnabled) throw()
{
    m_fWordWrapEnabled = fEnabled;
}

// -- Position --
double TextProperty::GetPositionX() const throw()
{
    return m_dblPositionX;
}

double TextProperty::GetPositionY() const throw()
{
    return m_dblPositionY;
}

void TextProperty::SetPosition(double dblX, double dblY) throw()
{
    m_dblPositionX = dblX;
    m_dblPositionY = dblY;
}

// -- Max width / height --
double TextProperty::GetMaxWidth() const throw()
{
    return m_dblMaxWidth;
}

void TextProperty::SetMaxWidth(double dblWidth) throw()
{
    m_dblMaxWidth = dblWidth;
}

double TextProperty::GetMaxHeight() const throw()
{
    return m_dblMaxHeight;
}

void TextProperty::SetMaxHeight(double dblHeight) throw()
{
    m_dblMaxHeight = dblHeight;
}

// -- Line spacing --
double TextProperty::GetLineSpacing() const throw()
{
    return m_dblLineSpacing;
}

void TextProperty::SetLineSpacing(double dblSpacing) throw()
{
    m_dblLineSpacing = dblSpacing;
}

// -- Comparison --
bool TextProperty::operator==(const TextProperty& other) const
{
    return (m_strFontFamily.CompareNoCase(other.m_strFontFamily) == 0) &&
           (m_flFontSize == other.m_flFontSize) &&
           (m_dwFontColor == other.m_dwFontColor) &&
           (m_fBold == other.m_fBold) &&
           (m_fItalic == other.m_fItalic) &&
           (m_fUnderline == other.m_fUnderline) &&
           (m_fStrikethrough == other.m_fStrikethrough) &&
           (m_dwBackgroundColor == other.m_dwBackgroundColor) &&
           (m_fHasBackground == other.m_fHasBackground) &&
           (m_alignment == other.m_alignment) &&
           (m_fWordWrapEnabled == other.m_fWordWrapEnabled);
}

bool TextProperty::operator!=(const TextProperty& other) const
{
    return !(*this == other);
}

// -- Merge --
void TextProperty::MergeFrom(const TextProperty& source)
{
    if (!source.m_strFontFamily.IsEmpty())
        m_strFontFamily = source.m_strFontFamily;

    if (source.m_flFontSize != 0.0f)
        m_flFontSize = source.m_flFontSize;

    if (source.m_dwFontColor != 0)
        m_dwFontColor = source.m_dwFontColor;

    if (source.m_fBold)
        m_fBold = true;

    if (source.m_fItalic)
        m_fItalic = true;

    if (source.m_fUnderline)
        m_fUnderline = true;

    if (source.m_fStrikethrough)
        m_fStrikethrough = true;

    if (source.m_fHasBackground)
    {
        m_fHasBackground = true;
        m_dwBackgroundColor = source.m_dwBackgroundColor;
    }

    if (source.m_fHasOutline)
    {
        m_fHasOutline = true;
        m_dwOutlineColor = source.m_dwOutlineColor;
        m_flOutlineWidth = source.m_flOutlineWidth;
    }

    if (source.m_fHasShadow)
    {
        m_fHasShadow = true;
        m_dwShadowColor = source.m_dwShadowColor;
        m_flShadowOffsetX = source.m_flShadowOffsetX;
        m_flShadowOffsetY = source.m_flShadowOffsetY;
    }

    m_alignment = source.m_alignment;
    m_fWordWrapEnabled = source.m_fWordWrapEnabled;
    m_dblLineSpacing = source.m_dblLineSpacing;
}

// -- IsDefault --
bool TextProperty::IsDefault() const throw()
{
    TextProperty def;
    return (*this == def);
}

// ============================================================================
// TextPropertySet implementation
// ============================================================================

TextPropertySet::TextPropertySet()
{
}

TextPropertySet::~TextPropertySet()
{
}

size_t TextPropertySet::AddProperty(const TextProperty& prop)
{
    return m_arrProperties.Add(prop);
}

void TextPropertySet::RemoveProperty(size_t nIndex)
{
    if (nIndex < m_arrProperties.GetCount())
    {
        m_arrProperties.RemoveAt(nIndex);
    }
}

void TextPropertySet::RemoveAll()
{
    m_arrProperties.RemoveAll();
}

size_t TextPropertySet::GetCount() const throw()
{
    return m_arrProperties.GetCount();
}

TextProperty& TextPropertySet::GetAt(size_t nIndex)
{
    return m_arrProperties.GetAt(nIndex);
}

const TextProperty& TextPropertySet::GetAt(size_t nIndex) const
{
    return m_arrProperties.GetAt(nIndex);
}

int TextPropertySet::FindByFontFamily(LPCWSTR pszFontFamily) const
{
    if (!pszFontFamily)
        return -1;

    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        if (m_arrProperties.GetAt(i).GetFontFamily().CompareNoCase(pszFontFamily) == 0)
            return static_cast<int>(i);
    }

    return -1;
}

} // namespace StoryboardManager
