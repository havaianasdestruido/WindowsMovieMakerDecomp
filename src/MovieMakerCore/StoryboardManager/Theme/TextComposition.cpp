#include "pch.h"
/*
 * TextComposition.cpp
 *
 * Implementation of the TextComposer and TextManager classes for the
 * StoryboardManager namespace. Handles text layout, measurement, line
 * breaking, auto-sizing, and animation keyframe generation for theme
 * titles and credits.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "TextComposition.h"
#include <dwrite.h>

namespace StoryboardManager
{

// ============================================================================
// TextComposer implementation
// ============================================================================

TextComposer::TextComposer()
    : m_flDefaultFontSize(36.0f)
    , m_dwDefaultFontColor(0xFFFFFFFF)
    , m_hrLastError(S_OK)
{
}

TextComposer::~TextComposer()
{
    Shutdown();
}

HRESULT TextComposer::Initialize(IDWriteFactory* pDWriteFactory)
{
    if (!pDWriteFactory)
        return E_INVALIDARG;

    m_spDWriteFactory = pDWriteFactory;
    return S_OK;
}

HRESULT TextComposer::Shutdown()
{
    m_spDWriteFactory.Release();
    return S_OK;
}

HRESULT TextComposer::ComposeTitle(ThemeTitle* pTitle,
                                   float flContainerWidth,
                                   float flContainerHeight,
                                   ComposedTextBlock* pOutput)
{
    if (!pTitle || !pOutput)
    {
        m_hrLastError = E_INVALIDARG;
        return m_hrLastError;
    }

    pOutput->arrLines.RemoveAll();
    pOutput->flBlockWidth = 0.0f;
    pOutput->flBlockHeight = 0.0f;
    pOutput->flPositionX = pTitle->GetPositionX() * flContainerWidth;
    pOutput->flPositionY = pTitle->GetPositionY() * flContainerHeight;
    pOutput->layoutMode = TextLayoutModeNormal;

    LPCWSTR pszText = pTitle->GetText();
    if (!pszText || wcslen(pszText) == 0)
        return S_OK;

    // Break text into lines
    float flMaxWidth = pTitle->GetMaxWidth() > 0.0f
        ? pTitle->GetMaxWidth() * flContainerWidth
        : flContainerWidth * 0.9f;

    ATL::CAtlArray<ATL::CString> arrTextLines;
    m_hrLastError = BreakLines(pszText,
                               pTitle->GetFontFamily(),
                               pTitle->GetFontSize(),
                               pTitle->GetFontStyle(),
                               flMaxWidth,
                               TextWrapModeWord,
                               arrTextLines);
    if (FAILED(m_hrLastError))
        return m_hrLastError;

    // Compose each line
    float flTotalHeight = 0.0f;
    float flMaxLineWidth = 0.0f;

    for (size_t i = 0; i < arrTextLines.GetCount(); ++i)
    {
        ComposedTextLine line;
        line.strText = arrTextLines.GetAt(i);
        line.strFontFamily = pTitle->GetFontFamily();
        line.flFontSize = pTitle->GetFontSize();
        line.dwFontStyle = pTitle->GetFontStyle();
        line.dwFontColor = pTitle->GetFontColor();
        line.flBaselineOffset = flTotalHeight;

        // Measure line dimensions
        float flWidth = 0.0f;
        float flHeight = 0.0f;
        MeasureText(line.strText, line.strFontFamily, line.flFontSize, line.dwFontStyle,
                    &flWidth, &flHeight);

        line.flWidth = flWidth;
        line.flHeight = flHeight;

        pOutput->arrLines.Add(line);

        flTotalHeight += flHeight * 1.2f; // 1.2 line spacing
        if (flWidth > flMaxLineWidth)
            flMaxLineWidth = flWidth;
    }

    pOutput->flBlockWidth = flMaxLineWidth;
    pOutput->flBlockHeight = flTotalHeight;

    return S_OK;
}

HRESULT TextComposer::ComposeCredits(ThemeTitle* pTitle,
                                     LPCWSTR pszCreditsText,
                                     float flContainerWidth,
                                     float flContainerHeight,
                                     ComposedTextBlock* pOutput)
{
    if (!pTitle || !pszCreditsText || !pOutput)
    {
        m_hrLastError = E_INVALIDARG;
        return m_hrLastError;
    }

    pOutput->arrLines.RemoveAll();
    pOutput->flPositionX = pTitle->GetPositionX() * flContainerWidth;
    pOutput->flPositionY = pTitle->GetPositionY() * flContainerHeight;
    pOutput->layoutMode = TextLayoutModeScroll;

    // Split credits text by newlines
    ATL::CString strCredits(pszCreditsText);
    int nStart = 0;

    while (nStart < strCredits.GetLength())
    {
        int nEnd = strCredits.Find(L'\n', nStart);
        if (nEnd < 0)
            nEnd = strCredits.GetLength();

        ATL::CString strLine = strCredits.Mid(nStart, nEnd - nStart);

        ComposedTextLine line;
        line.strText = strLine;
        line.strFontFamily = pTitle->GetFontFamily();
        line.flFontSize = pTitle->GetFontSize();
        line.dwFontStyle = pTitle->GetFontStyle();
        line.dwFontColor = pTitle->GetFontColor();

        float flWidth = 0.0f;
        float flHeight = 0.0f;
        MeasureText(line.strText, line.strFontFamily, line.flFontSize, line.dwFontStyle,
                    &flWidth, &flHeight);

        line.flWidth = flWidth;
        line.flHeight = flHeight;
        line.flBaselineOffset = 0.0f;

        pOutput->arrLines.Add(line);

        nStart = nEnd + 1;
    }

    // Compute block dimensions
    float flTotalHeight = 0.0f;
    float flMaxWidth = 0.0f;
    for (size_t i = 0; i < pOutput->arrLines.GetCount(); ++i)
    {
        flTotalHeight += pOutput->arrLines.GetAt(i).flHeight * 1.5f;
        if (pOutput->arrLines.GetAt(i).flWidth > flMaxWidth)
            flMaxWidth = pOutput->arrLines.GetAt(i).flWidth;
    }

    pOutput->flBlockWidth = flMaxWidth;
    pOutput->flBlockHeight = flTotalHeight;

    return S_OK;
}

HRESULT TextComposer::ComposeAutoSized(ThemeTitle* pTitle,
                                       float flContainerWidth,
                                       float flContainerHeight,
                                       float flMinFontSize,
                                       float flMaxFontSize,
                                       ComposedTextBlock* pOutput)
{
    if (!pTitle || !pOutput)
    {
        m_hrLastError = E_INVALIDARG;
        return m_hrLastError;
    }

    // Find the maximum font size that fits within the container
    float flBestSize = FindMaxFontSize(pTitle->GetText(),
                                       pTitle->GetFontFamily(),
                                       pTitle->GetFontStyle(),
                                       flContainerWidth,
                                       flContainerHeight,
                                       flMinFontSize,
                                       flMaxFontSize);

    // Temporarily override the font size
    float flOrigSize = pTitle->GetFontSize();
    pTitle->SetFontSize(flBestSize);

    HRESULT hr = ComposeTitle(pTitle, flContainerWidth, flContainerHeight, pOutput);

    // Restore original font size
    pTitle->SetFontSize(flOrigSize);

    pOutput->layoutMode = TextLayoutModeAutoSize;
    m_hrLastError = hr;
    return hr;
}

HRESULT TextComposer::MeasureText(LPCWSTR pszText,
                                  LPCWSTR pszFontFamily,
                                  float flFontSize,
                                  DWORD dwFontStyle,
                                  float* pflWidth,
                                  float* pflHeight)
{
    if (!pszText || !pflWidth || !pflHeight)
    {
        m_hrLastError = E_INVALIDARG;
        return m_hrLastError;
    }

    *pflWidth = 0.0f;
    *pflHeight = 0.0f;

    if (!m_spDWriteFactory)
    {
        // Fallback: approximate measurement
        size_t cchLen = wcslen(pszText);
        *pflWidth = static_cast<float>(cchLen) * flFontSize * 0.6f;
        *pflHeight = flFontSize * 1.2f;
        return S_OK;
    }

    CComPtr<IDWriteTextFormat> spFormat;
    HRESULT hr = m_spDWriteFactory->CreateTextFormat(
        pszFontFamily,
        nullptr,
        (dwFontStyle & 1) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        flFontSize,
        L"",
        &spFormat);

    if (FAILED(hr))
    {
        m_hrLastError = hr;
        return hr;
    }

    CComPtr<IDWriteTextLayout> spLayout;
    UINT32 cchLen = static_cast<UINT32>(wcslen(pszText));
    hr = m_spDWriteFactory->CreateTextLayout(
        pszText,
        cchLen,
        spFormat,
        0.0f,   // maxWidth (0 for unconstrained)
        0.0f,   // maxHeight
        &spLayout);

    if (FAILED(hr))
    {
        m_hrLastError = hr;
        return hr;
    }

    DWRITE_TEXT_METRICS metrics = { 0 };
    hr = spLayout->GetMetrics(&metrics);
    if (SUCCEEDED(hr))
    {
        *pflWidth = metrics.width;
        *pflHeight = metrics.height;
    }

    m_hrLastError = hr;
    return hr;
}

HRESULT TextComposer::BreakLines(LPCWSTR pszText,
                                 LPCWSTR pszFontFamily,
                                 float flFontSize,
                                 DWORD dwFontStyle,
                                 float flMaxWidth,
                                 TextWrapMode wrapMode,
                                 ATL::CAtlArray<ATL::CString>& arrLines)
{
    if (!pszText)
    {
        m_hrLastError = E_INVALIDARG;
        return m_hrLastError;
    }

    arrLines.RemoveAll();

    if (wrapMode == TextWrapModeNone)
    {
        arrLines.Add(ATL::CString(pszText));
        return S_OK;
    }

    if (!m_spDWriteFactory)
    {
        // Fallback: simple word wrap by character count approximation
        size_t cchText = wcslen(pszText);
        size_t cchMaxChars = static_cast<size_t>(flMaxWidth / (flFontSize * 0.6f));
        if (cchMaxChars == 0) cchMaxChars = 1;

        size_t nPos = 0;
        while (nPos < cchText)
        {
            size_t nLen = std::min(cchMaxChars, cchText - nPos);
            arrLines.Add(ATL::CString(pszText + nPos, static_cast<int>(nLen)));
            nPos += nLen;
        }
        return S_OK;
    }

    CComPtr<IDWriteTextFormat> spFormat;
    HRESULT hr = m_spDWriteFactory->CreateTextFormat(
        pszFontFamily,
        nullptr,
        (dwFontStyle & 1) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        flFontSize,
        L"",
        &spFormat);

    if (FAILED(hr))
    {
        m_hrLastError = hr;
        return hr;
    }

    // Create a constrained layout to force line breaking
    CComPtr<IDWriteTextLayout> spLayout;
    UINT32 cchLen = static_cast<UINT32>(wcslen(pszText));
    hr = m_spDWriteFactory->CreateTextLayout(
        pszText,
        cchLen,
        spFormat,
        flMaxWidth,
        10000.0f,
        &spLayout);

    if (FAILED(hr))
    {
        m_hrLastError = hr;
        return hr;
    }

    // Read line metrics
    UINT32 cLineCount = 0;
    spLayout->GetLineMetrics(nullptr, 0, &cLineCount);

    if (cLineCount == 0)
    {
        arrLines.Add(ATL::CString(pszText));
        return S_OK;
    }

    std::vector<DWRITE_LINE_METRICS> lineMetrics(cLineCount);
    hr = spLayout->GetLineMetrics(lineMetrics.data(), cLineCount, &cLineCount);
    if (FAILED(hr))
    {
        m_hrLastError = hr;
        return hr;
    }

    UINT32 nPos = 0;
    for (UINT32 i = 0; i < cLineCount; ++i)
    {
        UINT32 cchLine = lineMetrics[i].length;
        arrLines.Add(ATL::CString(pszText + nPos, static_cast<int>(cchLine)));
        nPos += cchLine;

        // Skip trailing whitespace/ newline
        while (nPos < cchLen && (pszText[nPos] == L'\r' || pszText[nPos] == L'\n' || pszText[nPos] == L' '))
            ++nPos;
    }

    return S_OK;
}

HRESULT TextComposer::GenerateFadeInKeyframes(const ComposedTextBlock& block,
                                              LONGLONG llDurationHns,
                                              ATL::CAtlArray<TemplateProperty>& keyframes)
{
    keyframes.RemoveAll();

    // Opacity 0 at start
    TemplateProperty kfStart(L"opacity", L"0");
    keyframes.Add(kfStart);

    // Opacity 1 at end
    TemplateProperty kfEnd(L"opacity", L"1");
    keyframes.Add(kfEnd);

    // Time markers
    TemplateProperty kfTimeStart(L"time_start", L"0");
    keyframes.Add(kfTimeStart);

    ATL::CString strTimeEnd;
    strTimeEnd.Format(L"%lld", llDurationHns);
    TemplateProperty kfTimeEnd(L"time_end", strTimeEnd);
    keyframes.Add(kfTimeEnd);

    return S_OK;
}

HRESULT TextComposer::GenerateFlyInKeyframes(const ComposedTextBlock& block,
                                             TitleAnimationType animType,
                                             LONGLONG llDurationHns,
                                             ATL::CAtlArray<TemplateProperty>& keyframes)
{
    keyframes.RemoveAll();

    LPCWSTR pszDirection = L"left";
    switch (animType)
    {
    case TitleAnimationFlyInLeft:  pszDirection = L"left"; break;
    case TitleAnimationFlyInRight: pszDirection = L"right"; break;
    case TitleAnimationFlyInTop:   pszDirection = L"top"; break;
    case TitleAnimationFlyInBottom:pszDirection = L"bottom"; break;
    default: break;
    }

    TemplateProperty kfDir(L"direction", pszDirection);
    keyframes.Add(kfDir);

    TemplateProperty kfStart(L"offset_x", L"-1.0");
    keyframes.Add(kfStart);

    TemplateProperty kfEnd(L"offset_x", L"0.0");
    keyframes.Add(kfEnd);

    TemplateProperty kfOpacityStart(L"opacity", L"0");
    keyframes.Add(kfOpacityStart);

    TemplateProperty kfOpacityEnd(L"opacity", L"1");
    keyframes.Add(kfOpacityEnd);

    ATL::CString strDuration;
    strDuration.Format(L"%lld", llDurationHns);
    TemplateProperty kfDuration(L"duration", strDuration);
    keyframes.Add(kfDuration);

    return S_OK;
}

HRESULT TextComposer::GenerateTypewriterKeyframes(const ComposedTextBlock& block,
                                                  LONGLONG llDurationHns,
                                                  ATL::CAtlArray<TemplateProperty>& keyframes)
{
    keyframes.RemoveAll();

    // Character-by-character reveal
    size_t cchTotal = 0;
    for (size_t i = 0; i < block.arrLines.GetCount(); ++i)
    {
        cchTotal += block.arrLines.GetAt(i).strText.GetLength();
    }

    TemplateProperty kfMode(L"mode", L"typewriter");
    keyframes.Add(kfMode);

    ATL::CString strChars;
    strChars.Format(L"%zu", cchTotal);
    TemplateProperty kfTotalChars(L"total_characters", strChars);
    keyframes.Add(kfTotalChars);

    ATL::CString strDuration;
    strDuration.Format(L"%lld", llDurationHns);
    TemplateProperty kfDuration(L"duration", strDuration);
    keyframes.Add(kfDuration);

    return S_OK;
}

void TextComposer::SetDefaultFontFamily(LPCWSTR pszFontFamily)
{
    m_strDefaultFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
}

void TextComposer::SetDefaultFontSize(float flSize)
{
    m_flDefaultFontSize = flSize;
}

void TextComposer::SetDefaultFontColor(DWORD dwColor)
{
    m_dwDefaultFontColor = dwColor;
}

HRESULT TextComposer::GetLastError() const throw()
{
    return m_hrLastError;
}

HRESULT TextComposer::CreateTextLayout(LPCWSTR pszText,
                                       LPCWSTR pszFontFamily,
                                       float flFontSize,
                                       DWORD dwFontStyle,
                                       float flMaxWidth,
                                       IDWriteTextLayout** ppLayout)
{
    if (!ppLayout)
        return E_INVALIDARG;

    *ppLayout = nullptr;

    if (!m_spDWriteFactory || !pszText || !pszFontFamily)
        return E_INVALIDARG;

    CComPtr<IDWriteTextFormat> spFormat;
    HRESULT hr = m_spDWriteFactory->CreateTextFormat(
        pszFontFamily,
        nullptr,
        (dwFontStyle & 1) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        flFontSize,
        L"",
        &spFormat);

    if (FAILED(hr))
        return hr;

    return m_spDWriteFactory->CreateTextLayout(
        pszText,
        static_cast<UINT32>(wcslen(pszText)),
        spFormat,
        flMaxWidth,
        10000.0f,
        ppLayout);
}

float TextComposer::FindMaxFontSize(LPCWSTR pszText,
                                    LPCWSTR pszFontFamily,
                                    DWORD dwFontStyle,
                                    float flContainerWidth,
                                    float flContainerHeight,
                                    float flMinSize,
                                    float flMaxSize)
{
    float flBest = flMinSize;
    float flLow = flMinSize;
    float flHigh = flMaxSize;

    // Binary search for the largest font size that fits
    for (int nIter = 0; nIter < 10; ++nIter)
    {
        float flMid = (flLow + flHigh) / 2.0f;

        float flWidth = 0.0f;
        float flHeight = 0.0f;
        MeasureText(pszText, pszFontFamily, flMid, dwFontStyle, &flWidth, &flHeight);

        if (flWidth <= flContainerWidth * 0.9f && flHeight <= flContainerHeight * 0.9f)
        {
            flBest = flMid;
            flLow = flMid + 0.5f;
        }
        else
        {
            flHigh = flMid - 0.5f;
        }

        if (flLow >= flHigh)
            break;
    }

    return flBest;
}

// ============================================================================
// TextManager implementation
// ============================================================================

TextManager::TextManager()
    : m_dwNextItemId(1)
    , m_pComposer(nullptr)
    , m_fInitialized(false)
{
}

TextManager::~TextManager()
{
    Shutdown();
}

HRESULT TextManager::Initialize()
{
    if (m_fInitialized)
        return S_FALSE;

    m_pComposer = new TextComposer();
    m_fInitialized = true;
    return S_OK;
}

HRESULT TextManager::Shutdown()
{
    if (!m_fInitialized)
        return S_FALSE;

    delete m_pComposer;
    m_pComposer = nullptr;

    m_arrItems.RemoveAll();
    m_fInitialized = false;
    return S_OK;
}

size_t TextManager::GetItemCount() const throw()
{
    return m_arrItems.GetCount();
}

const TextCollectionItem* TextManager::GetItem(size_t nIndex) const
{
    if (nIndex >= m_arrItems.GetCount())
        return nullptr;
    return &m_arrItems.GetAt(nIndex);
}

TextCollectionItem* TextManager::GetItem(size_t nIndex)
{
    if (nIndex >= m_arrItems.GetCount())
        return nullptr;
    return &m_arrItems.GetAt(nIndex);
}

const TextCollectionItem* TextManager::FindItem(DWORD dwItemId) const
{
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).dwItemId == dwItemId)
            return &m_arrItems.GetAt(i);
    }
    return nullptr;
}

TextCollectionItem* TextManager::FindItem(DWORD dwItemId)
{
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).dwItemId == dwItemId)
            return &m_arrItems.GetAt(i);
    }
    return nullptr;
}

const TextCollectionItem* TextManager::FindItemByText(LPCWSTR pszText) const
{
    if (!pszText)
        return nullptr;

    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).strText.Compare(pszText) == 0)
            return &m_arrItems.GetAt(i);
    }
    return nullptr;
}

DWORD TextManager::AddItem(const TextCollectionItem& item)
{
    TextCollectionItem newItem = item;
    newItem.dwItemId = GenerateNextItemId();
    m_arrItems.Add(newItem);
    return newItem.dwItemId;
}

void TextManager::RemoveItem(DWORD dwItemId)
{
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).dwItemId == dwItemId)
        {
            m_arrItems.RemoveAt(i);
            return;
        }
    }
}

void TextManager::RemoveAllItems()
{
    m_arrItems.RemoveAll();
}

HRESULT TextManager::UpdateItemText(DWORD dwItemId, LPCWSTR pszText)
{
    TextCollectionItem* pItem = FindItem(dwItemId);
    if (!pItem)
        return E_INVALIDARG;

    pItem->strText = pszText ? pszText : L"";
    return S_OK;
}

HRESULT TextManager::UpdateItemFont(DWORD dwItemId, LPCWSTR pszFontFamily, float flSize, DWORD dwColor)
{
    TextCollectionItem* pItem = FindItem(dwItemId);
    if (!pItem)
        return E_INVALIDARG;

    if (pszFontFamily)
        pItem->strFontFamily = pszFontFamily;
    pItem->flFontSize = flSize;
    pItem->dwFontColor = dwColor;
    return S_OK;
}

HRESULT TextManager::UpdateItemPosition(DWORD dwItemId, float flX, float flY)
{
    TextCollectionItem* pItem = FindItem(dwItemId);
    if (!pItem)
        return E_INVALIDARG;

    pItem->flPositionX = flX;
    pItem->flPositionY = flY;
    return S_OK;
}

HRESULT TextManager::SetItemVisible(DWORD dwItemId, bool fVisible)
{
    TextCollectionItem* pItem = FindItem(dwItemId);
    if (!pItem)
        return E_INVALIDARG;

    pItem->fVisible = fVisible;
    return S_OK;
}

HRESULT TextManager::SetTitleText(LPCWSTR pszText)
{
    // Find or create a title item
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).layoutMode == TextLayoutModeNormal &&
            m_arrItems.GetAt(i).fVisible)
        {
            m_arrItems.GetAt(i).strText = pszText ? pszText : L"";
            return S_OK;
        }
    }

    // Create new title item
    TextCollectionItem item = { 0 };
    item.dwItemId = GenerateNextItemId();
    item.strText = pszText ? pszText : L"";
    item.strFontFamily = L"Segoe UI";
    item.flFontSize = 36.0f;
    item.dwFontColor = 0xFFFFFFFF;
    item.flPositionX = 0.5f;
    item.flPositionY = 0.5f;
    item.layoutMode = TextLayoutModeNormal;
    item.fVisible = true;
    m_arrItems.Add(item);
    return S_OK;
}

ATL::CString TextManager::GetTitleText() const
{
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).layoutMode == TextLayoutModeNormal &&
            m_arrItems.GetAt(i).fVisible)
        {
            return m_arrItems.GetAt(i).strText;
        }
    }
    return ATL::CString();
}

HRESULT TextManager::SetCreditsText(LPCWSTR pszText)
{
    // Find or create a credits item
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).layoutMode == TextLayoutModeScroll)
        {
            m_arrItems.GetAt(i).strText = pszText ? pszText : L"";
            return S_OK;
        }
    }

    // Create new credits item
    TextCollectionItem item = { 0 };
    item.dwItemId = GenerateNextItemId();
    item.strText = pszText ? pszText : L"";
    item.strFontFamily = L"Segoe UI";
    item.flFontSize = 24.0f;
    item.dwFontColor = 0xFFFFFFFF;
    item.flPositionX = 0.5f;
    item.flPositionY = 0.5f;
    item.layoutMode = TextLayoutModeScroll;
    item.fVisible = true;
    m_arrItems.Add(item);
    return S_OK;
}

ATL::CString TextManager::GetCreditsText() const
{
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).layoutMode == TextLayoutModeScroll)
        {
            return m_arrItems.GetAt(i).strText;
        }
    }
    return ATL::CString();
}

TextComposer* TextManager::GetComposer()
{
    return m_pComposer;
}

const TextComposer* TextManager::GetComposer() const
{
    return m_pComposer;
}

HRESULT TextManager::SetDWriteFactory(IDWriteFactory* pFactory)
{
    if (m_pComposer)
        return m_pComposer->Initialize(pFactory);
    return E_FAIL;
}

HRESULT TextManager::LoadFromXml(IXmlReader* pReader)
{
    if (!pReader)
        return E_INVALIDARG;

    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pwszName = nullptr;
            pReader->GetLocalName(&pwszName, nullptr);

            if (pwszName && wcscmp(pwszName, L"textItem") == 0)
            {
                TextCollectionItem item = { 0 };
                item.fVisible = true;
                item.layoutMode = TextLayoutModeNormal;
                item.flFontSize = 36.0f;
                item.dwFontColor = 0xFFFFFFFF;

                LPCWSTR pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"text", &pwszVal)) && pwszVal)
                    item.strText = pwszVal;

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fontFamily", &pwszVal)) && pwszVal)
                    item.strFontFamily = pwszVal;

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fontSize", &pwszVal)) && pwszVal)
                    item.flFontSize = static_cast<float>(_wtof(pwszVal));

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fontColor", &pwszVal)) && pwszVal)
                    item.dwFontColor = static_cast<DWORD>(wcstoul(pwszVal, nullptr, 16));

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fontStyle", &pwszVal)) && pwszVal)
                    item.dwFontStyle = static_cast<DWORD>(_wtoi(pwszVal));

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"posX", &pwszVal)) && pwszVal)
                    item.flPositionX = static_cast<float>(_wtof(pwszVal));

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"posY", &pwszVal)) && pwszVal)
                    item.flPositionY = static_cast<float>(_wtof(pwszVal));

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"layoutMode", &pwszVal)) && pwszVal)
                    item.layoutMode = static_cast<TextLayoutMode>(_wtoi(pwszVal));

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"visible", &pwszVal)) && pwszVal)
                    item.fVisible = (wcscmp(pwszVal, L"true") == 0 || wcscmp(pwszVal, L"1") == 0);

                AddItem(item);
            }
        }
        else if (nodeType == XmlNodeType_EndElement)
        {
            break;
        }
    }

    return S_OK;
}

HRESULT TextManager::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_INVALIDARG;

    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        const TextCollectionItem& item = m_arrItems.GetAt(i);

        pWriter->WriteStartElement(nullptr, L"textItem", nullptr);

        if (!item.strText.IsEmpty())
            pWriter->WriteAttributeString(nullptr, L"text", nullptr, item.strText);

        if (!item.strFontFamily.IsEmpty())
            pWriter->WriteAttributeString(nullptr, L"fontFamily", nullptr, item.strFontFamily);

        WCHAR szValue[64] = { 0 };

        if (item.flFontSize != 36.0f)
        {
            swprintf_s(szValue, L"%g", item.flFontSize);
            pWriter->WriteAttributeString(nullptr, L"fontSize", nullptr, szValue);
        }

        if (item.dwFontColor != 0xFFFFFFFF)
        {
            swprintf_s(szValue, L"0x%08X", item.dwFontColor);
            pWriter->WriteAttributeString(nullptr, L"fontColor", nullptr, szValue);
        }

        if (item.dwFontStyle != 0)
        {
            swprintf_s(szValue, L"%u", item.dwFontStyle);
            pWriter->WriteAttributeString(nullptr, L"fontStyle", nullptr, szValue);
        }

        swprintf_s(szValue, L"%g", item.flPositionX);
        pWriter->WriteAttributeString(nullptr, L"posX", nullptr, szValue);

        swprintf_s(szValue, L"%g", item.flPositionY);
        pWriter->WriteAttributeString(nullptr, L"posY", nullptr, szValue);

        swprintf_s(szValue, L"%d", static_cast<int>(item.layoutMode));
        pWriter->WriteAttributeString(nullptr, L"layoutMode", nullptr, szValue);

        if (!item.fVisible)
            pWriter->WriteAttributeString(nullptr, L"visible", nullptr, L"false");

        pWriter->WriteEndElement();
    }

    return S_OK;
}

DWORD TextManager::GenerateNextItemId()
{
    return m_dwNextItemId++;
}

} // namespace StoryboardManager
