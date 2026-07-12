/*
 * TextComposition.h
 *
 * Text composition and layout for theme titles. TextComposer composes
 * title and credits text with theme formatting. TextManager manages
 * text collections for themes.
 *
 * RTTI classes:
 *   ?AVTextComposer@@, ?AVTextManager@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef TEXTCOMPOSITION_H
#define TEXTCOMPOSITION_H

#include "../StoryboardManager.h"
#include "ThemeStructure.h"

namespace StoryboardManager
{

// ============================================================================
// Text alignment enums
// ============================================================================
enum TextLayoutMode
{
    TextLayoutModeNormal       = 0,    // standard text layout
    TextLayoutModeAutoSize     = 1,    // auto-size to fit container
    TextLayoutModeScroll       = 2,    // scrolling credits text
    TextLayoutModeTypewriter   = 3     // typewriter reveal effect
};

enum TextWrapMode
{
    TextWrapModeNone           = 0,    // no wrapping
    TextWrapModeWord           = 1,    // word wrap
    TextWrapModeCharacter      = 2     // character wrap
};

// ============================================================================
// ComposedTextLine
// ============================================================================
// A single composed text line with layout metrics.
//
struct ComposedTextLine
{
    ATL::CString strText;
    float        flWidth;
    float        flHeight;
    float        flBaselineOffset;
    DWORD        dwFontColor;
    ATL::CString strFontFamily;
    float        flFontSize;
    DWORD        dwFontStyle;
};

// ============================================================================
// ComposedTextBlock
// ============================================================================
// A block of composed text lines forming a paragraph or title.
//
struct ComposedTextBlock
{
    ATL::CAtlArray<ComposedTextLine> arrLines;
    float        flBlockWidth;
    float        flBlockHeight;
    float        flPositionX;
    float        flPositionY;
    TextLayoutMode layoutMode;
};

// ============================================================================
// TextComposer
// ============================================================================
// Composes title and credits text using theme formatting rules. Handles
// font measurement, line breaking, auto-sizing, and text animation
// keyframe generation.
//
class STORYBOARD_API TextComposer
{
public:
    TextComposer();
    ~TextComposer();

    // Initialize with a DirectWrite factory for text measurement
    HRESULT Initialize(IDWriteFactory* pDWriteFactory);
    HRESULT Shutdown();

    // Compose a title text block
    HRESULT ComposeTitle(ThemeTitle* pTitle,
                         float flContainerWidth,
                         float flContainerHeight,
                         ComposedTextBlock* pOutput);

    // Compose credits text (multi-line scrolling)
    HRESULT ComposeCredits(ThemeTitle* pTitle,
                           LPCWSTR pszCreditsText,
                           float flContainerWidth,
                           float flContainerHeight,
                           ComposedTextBlock* pOutput);

    // Compose with auto-sizing
    HRESULT ComposeAutoSized(ThemeTitle* pTitle,
                             float flContainerWidth,
                             float flContainerHeight,
                             float flMinFontSize,
                             float flMaxFontSize,
                             ComposedTextBlock* pOutput);

    // Measure text dimensions
    HRESULT MeasureText(LPCWSTR pszText,
                        LPCWSTR pszFontFamily,
                        float flFontSize,
                        DWORD dwFontStyle,
                        float* pflWidth,
                        float* pflHeight);

    // Line breaking
    HRESULT BreakLines(LPCWSTR pszText,
                       LPCWSTR pszFontFamily,
                       float flFontSize,
                       DWORD dwFontStyle,
                       float flMaxWidth,
                       TextWrapMode wrapMode,
                       ATL::CAtlArray<ATL::CString>& arrLines);

    // Animation keyframe generation
    HRESULT GenerateFadeInKeyframes(const ComposedTextBlock& block,
                                    LONGLONG llDurationHns,
                                    ATL::CAtlArray<TemplateProperty>& keyframes);

    HRESULT GenerateFlyInKeyframes(const ComposedTextBlock& block,
                                   TitleAnimationType animType,
                                   LONGLONG llDurationHns,
                                   ATL::CAtlArray<TemplateProperty>& keyframes);

    HRESULT GenerateTypewriterKeyframes(const ComposedTextBlock& block,
                                        LONGLONG llDurationHns,
                                        ATL::CAtlArray<TemplateProperty>& keyframes);

    // Configuration
    void SetDefaultFontFamily(LPCWSTR pszFontFamily);
    void SetDefaultFontSize(float flSize);
    void SetDefaultFontColor(DWORD dwColor);

    // Error info
    HRESULT GetLastError() const throw();

private:
    HRESULT CreateTextLayout(LPCWSTR pszText,
                             LPCWSTR pszFontFamily,
                             float flFontSize,
                             DWORD dwFontStyle,
                             float flMaxWidth,
                             IDWriteTextLayout** ppLayout);

    float FindMaxFontSize(LPCWSTR pszText,
                          LPCWSTR pszFontFamily,
                          DWORD dwFontStyle,
                          float flContainerWidth,
                          float flContainerHeight,
                          float flMinSize,
                          float flMaxSize);

    CComPtr<IDWriteFactory>     m_spDWriteFactory;
    ATL::CString                m_strDefaultFontFamily;
    float                       m_flDefaultFontSize;
    DWORD                       m_dwDefaultFontColor;
    HRESULT                     m_hrLastError;
};

// ============================================================================
// TextCollectionItem
// ============================================================================
// A single text item in the text manager's collection.
//
struct TextCollectionItem
{
    DWORD           dwItemId;
    ATL::CString   strText;
    ATL::CString   strFontFamily;
    float          flFontSize;
    DWORD          dwFontColor;
    DWORD          dwFontStyle;
    float          flPositionX;
    float          flPositionY;
    TextLayoutMode layoutMode;
    bool           fVisible;
};

// ============================================================================
// TextManager
// ============================================================================
// Manages the collection of text items for a theme. Provides lookup,
// enumeration, and editing of text elements that appear in titles,
// credits, and overlays.
//
class STORYBOARD_API TextManager
{
public:
    TextManager();
    ~TextManager();

    // Lifecycle
    HRESULT Initialize();
    HRESULT Shutdown();

    // Text item access
    size_t GetItemCount() const throw();
    const TextCollectionItem* GetItem(size_t nIndex) const;
    TextCollectionItem* GetItem(size_t nIndex);

    // Lookup
    const TextCollectionItem* FindItem(DWORD dwItemId) const;
    TextCollectionItem* FindItem(DWORD dwItemId);
    const TextCollectionItem* FindItemByText(LPCWSTR pszText) const;

    // Add/remove
    DWORD AddItem(const TextCollectionItem& item);
    void RemoveItem(DWORD dwItemId);
    void RemoveAllItems();

    // Update
    HRESULT UpdateItemText(DWORD dwItemId, LPCWSTR pszText);
    HRESULT UpdateItemFont(DWORD dwItemId, LPCWSTR pszFontFamily, float flSize, DWORD dwColor);
    HRESULT UpdateItemPosition(DWORD dwItemId, float flX, float flY);
    HRESULT SetItemVisible(DWORD dwItemId, bool fVisible);

    // Title text
    HRESULT SetTitleText(LPCWSTR pszText);
    ATL::CString GetTitleText() const;

    // Credits text
    HRESULT SetCreditsText(LPCWSTR pszText);
    ATL::CString GetCreditsText() const;

    // Text composer access
    TextComposer* GetComposer();
    const TextComposer* GetComposer() const;

    // DWrite factory for text measurement
    HRESULT SetDWriteFactory(IDWriteFactory* pFactory);

    // Serialization
    HRESULT LoadFromXml(IXmlReader* pReader);
    HRESULT SaveToXml(IXmlWriter* pWriter);

private:
    DWORD GenerateNextItemId();

    ATL::CAtlArray<TextCollectionItem> m_arrItems;
    DWORD                              m_dwNextItemId;
    TextComposer*                      m_pComposer;
    CComPtr<IDWriteFactory>            m_spDWriteFactory;
    bool                               m_fInitialized;
};

} // namespace StoryboardManager

#endif // TEXTCOMPOSITION_H
