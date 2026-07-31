#include "pch.h"
/*
 * MovieEffect.cpp
 *
 * Implementation of the movie effect classes for the StoryboardManager
 * namespace. Provides TextEffect, PanAndZoomShapeEffect, and
 * AudioDuckingProperties.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "MovieEffect.h"

namespace StoryboardManager
{

// ============================================================================
// MovieEffect implementation
// ============================================================================

MovieEffect::MovieEffect()
    : m_type(MovieEffectTypeNone)
    , m_dwEffectId(0)
    , m_llDurationHns(0)
    , m_llStartOffsetHns(0)
    , m_flIntensity(1.0f)
    , m_fEnabled(true)
{
}

MovieEffect::~MovieEffect()
{
}

MovieEffectType MovieEffect::GetType() const throw() { return m_type; }
void MovieEffect::SetType(MovieEffectType type) throw() { m_type = type; }

DWORD MovieEffect::GetEffectId() const throw() { return m_dwEffectId; }
void MovieEffect::SetEffectId(DWORD dwId) throw() { m_dwEffectId = dwId; }

ATL::CString MovieEffect::GetName() const { return m_strName; }
void MovieEffect::SetName(LPCWSTR pszName) { m_strName = pszName ? pszName : L""; }

LONGLONG MovieEffect::GetDurationHns() const throw() { return m_llDurationHns; }
void MovieEffect::SetDurationHns(LONGLONG llDuration) throw()
{
    if (llDuration < 0) llDuration = 0;
    m_llDurationHns = llDuration;
}

LONGLONG MovieEffect::GetStartOffsetHns() const throw() { return m_llStartOffsetHns; }
void MovieEffect::SetStartOffsetHns(LONGLONG llOffset) throw()
{
    if (llOffset < 0) llOffset = 0;
    m_llStartOffsetHns = llOffset;
}

float MovieEffect::GetIntensity() const throw() { return m_flIntensity; }
void MovieEffect::SetIntensity(float flIntensity) throw()
{
    if (flIntensity < 0.0f) flIntensity = 0.0f;
    if (flIntensity > 1.0f) flIntensity = 1.0f;
    m_flIntensity = flIntensity;
}

size_t MovieEffect::GetParameterCount() const throw()
{
    return m_arrParameters.GetCount();
}

ATL::CString MovieEffect::GetParameter(LPCWSTR pszKey) const
{
    if (!pszKey) return ATL::CString();
    for (size_t i = 0; i < m_arrParameters.GetCount(); ++i)
    {
        if (m_arrParameters.GetAt(i).strKey.CompareNoCase(pszKey) == 0)
            return m_arrParameters.GetAt(i).strValue;
    }
    return ATL::CString();
}

void MovieEffect::SetParameter(LPCWSTR pszKey, LPCWSTR pszValue)
{
    if (!pszKey) return;
    for (size_t i = 0; i < m_arrParameters.GetCount(); ++i)
    {
        if (m_arrParameters.GetAt(i).strKey.CompareNoCase(pszKey) == 0)
        {
            m_arrParameters.GetAt(i).strValue = pszValue ? pszValue : L"";
            return;
        }
    }
    EffectParam param;
    param.strKey = pszKey;
    param.strValue = pszValue ? pszValue : L"";
    m_arrParameters.Add(param);
}

void MovieEffect::RemoveParameter(LPCWSTR pszKey)
{
    if (!pszKey) return;
    for (size_t i = 0; i < m_arrParameters.GetCount(); ++i)
    {
        if (m_arrParameters.GetAt(i).strKey.CompareNoCase(pszKey) == 0)
        {
            m_arrParameters.RemoveAt(i);
            return;
        }
    }
}

bool MovieEffect::IsEnabled() const throw() { return m_fEnabled; }
void MovieEffect::SetEnabled(bool fEnabled) throw() { m_fEnabled = fEnabled; }

HRESULT MovieEffect::LoadFromXml(IXmlReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    m_arrParameters.RemoveAll();

    LPCWSTR pszValue = nullptr;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"id", &pszValue)) && pszValue)
        m_dwEffectId = _wtol(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"name", &pszValue)) && pszValue)
        m_strName = pszValue;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"duration", &pszValue)) && pszValue)
        m_llDurationHns = _wtoi64(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"offset", &pszValue)) && pszValue)
        m_llStartOffsetHns = _wtoi64(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"intensity", &pszValue)) && pszValue)
        SetIntensity((float)_wtof(pszValue));

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"enabled", &pszValue)) && pszValue)
        m_fEnabled = (_wtoi(pszValue) != 0);

    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_EndElement)
            break;

        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            pReader->GetLocalName(&pszLocalName, nullptr);
            if (pszLocalName && wcscmp(pszLocalName, L"param") == 0)
            {
                LPCWSTR pszKey = nullptr;
                LPCWSTR pszVal = nullptr;
                XmlReaderGetAttribute(pReader, L"key", &pszKey);
                XmlReaderGetAttribute(pReader, L"value", &pszVal);
                if (pszKey)
                    SetParameter(pszKey, pszVal ? pszVal : L"");
            }
        }
    }

    return S_OK;
}

HRESULT MovieEffect::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_POINTER;

    HRESULT hr;
    WCHAR szBuf[64];

    _itow_s(m_dwEffectId, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"id", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    if (!m_strName.IsEmpty())
    {
        hr = pWriter->WriteAttributeString(nullptr, L"name", nullptr, m_strName);
        if (FAILED(hr)) return hr;
    }

    _i64tow_s(m_llDurationHns, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"duration", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _i64tow_s(m_llStartOffsetHns, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"offset", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    char szFloat[32];
    sprintf_s(szFloat, "%.2f", m_flIntensity);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"intensity", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    hr = pWriter->WriteAttributeString(nullptr, L"enabled", nullptr, m_fEnabled ? L"1" : L"0");
    if (FAILED(hr)) return hr;

    for (size_t i = 0; i < m_arrParameters.GetCount(); ++i)
    {
        hr = pWriter->WriteStartElement(nullptr, L"param", nullptr);
        if (FAILED(hr)) return hr;

        hr = pWriter->WriteAttributeString(nullptr, L"key", nullptr, m_arrParameters.GetAt(i).strKey);
        if (FAILED(hr)) return hr;

        hr = pWriter->WriteAttributeString(nullptr, L"value", nullptr, m_arrParameters.GetAt(i).strValue);
        if (FAILED(hr)) return hr;

        hr = pWriter->WriteEndElement();
        if (FAILED(hr)) return hr;
    }

    return S_OK;
}

// ============================================================================
// TextEffect implementation
// ============================================================================

TextEffect::TextEffect()
    : m_flFontSize(36.0f)
    , m_dwFontColor(0xFFFFFFFF)
    , m_dwFontStyle(0)
    , m_flPositionX(0.5f)
    , m_flPositionY(0.5f)
    , m_alignment(TextEffectAlignCenter)
    , m_fBackground(false)
    , m_dwBackgroundColor(0x80000000)
    , m_dwAnimationType(0)
    , m_llAnimationDurationHns(5000000)
{
    SetType(MovieEffectTypeText);
}

TextEffect::~TextEffect()
{
}

ATL::CString TextEffect::GetText() const { return m_strText; }
void TextEffect::SetText(LPCWSTR pszText) { m_strText = pszText ? pszText : L""; }

ATL::CString TextEffect::GetFontFamily() const { return m_strFontFamily; }
void TextEffect::SetFontFamily(LPCWSTR pszFontFamily) { m_strFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI"; }

float TextEffect::GetFontSize() const throw() { return m_flFontSize; }
void TextEffect::SetFontSize(float flSize) throw()
{
    if (flSize < 1.0f) flSize = 1.0f;
    m_flFontSize = flSize;
}

DWORD TextEffect::GetFontColor() const throw() { return m_dwFontColor; }
void TextEffect::SetFontColor(DWORD dwColor) throw() { m_dwFontColor = dwColor; }

DWORD TextEffect::GetFontStyle() const throw() { return m_dwFontStyle; }
void TextEffect::SetFontStyle(DWORD dwStyle) throw() { m_dwFontStyle = dwStyle; }

float TextEffect::GetPositionX() const throw() { return m_flPositionX; }
float TextEffect::GetPositionY() const throw() { return m_flPositionY; }
void TextEffect::SetPosition(float x, float y) throw() { m_flPositionX = x; m_flPositionY = y; }

TextEffect::TextEffectAlignment TextEffect::GetAlignment() const throw() { return m_alignment; }
void TextEffect::SetAlignment(TextEffectAlignment align) throw() { m_alignment = align; }

bool TextEffect::HasBackground() const throw() { return m_fBackground; }
void TextEffect::SetBackground(bool fBackground) throw() { m_fBackground = fBackground; }

DWORD TextEffect::GetBackgroundColor() const throw() { return m_dwBackgroundColor; }
void TextEffect::SetBackgroundColor(DWORD dwColor) throw() { m_dwBackgroundColor = dwColor; }

DWORD TextEffect::GetAnimationType() const throw() { return m_dwAnimationType; }
void TextEffect::SetAnimationType(DWORD dwType) throw() { m_dwAnimationType = dwType; }

LONGLONG TextEffect::GetAnimationDurationHns() const throw() { return m_llAnimationDurationHns; }
void TextEffect::SetAnimationDurationHns(LONGLONG llDuration) throw() { m_llAnimationDurationHns = llDuration; }

HRESULT TextEffect::LoadFromXml(IXmlReader* pReader)
{
    HRESULT hr = MovieEffect::LoadFromXml(pReader);
    if (FAILED(hr)) return hr;

    LPCWSTR pszValue = nullptr;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"text", &pszValue)) && pszValue)
        m_strText = pszValue;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fontFamily", &pszValue)) && pszValue)
        m_strFontFamily = pszValue;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fontSize", &pszValue)) && pszValue)
        m_flFontSize = (float)_wtof(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fontColor", &pszValue)) && pszValue)
        m_dwFontColor = (DWORD)_wtol(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fontStyle", &pszValue)) && pszValue)
        m_dwFontStyle = (DWORD)_wtol(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"posX", &pszValue)) && pszValue)
        m_flPositionX = (float)_wtof(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"posY", &pszValue)) && pszValue)
        m_flPositionY = (float)_wtof(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"alignment", &pszValue)) && pszValue)
        m_alignment = static_cast<TextEffectAlignment>(_wtoi(pszValue));

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"background", &pszValue)) && pszValue)
        m_fBackground = (_wtoi(pszValue) != 0);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"bgColor", &pszValue)) && pszValue)
        m_dwBackgroundColor = (DWORD)_wtol(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"animType", &pszValue)) && pszValue)
        m_dwAnimationType = (DWORD)_wtol(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"animDuration", &pszValue)) && pszValue)
        m_llAnimationDurationHns = _wtoi64(pszValue);

    return S_OK;
}

HRESULT TextEffect::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_POINTER;

    HRESULT hr;
    WCHAR szBuf[64];

    hr = pWriter->WriteStartElement(nullptr, L"textEffect", nullptr);
    if (FAILED(hr)) return hr;

    hr = MovieEffect::SaveToXml(pWriter);
    if (FAILED(hr)) return hr;

    if (!m_strText.IsEmpty())
    {
        hr = pWriter->WriteAttributeString(nullptr, L"text", nullptr, m_strText);
        if (FAILED(hr)) return hr;
    }

    if (!m_strFontFamily.IsEmpty())
    {
        hr = pWriter->WriteAttributeString(nullptr, L"fontFamily", nullptr, m_strFontFamily);
        if (FAILED(hr)) return hr;
    }

    char szFloat[32];

    sprintf_s(szFloat, "%.1f", m_flFontSize);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"fontSize", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _itow_s(m_dwFontColor, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"fontColor", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _itow_s(m_dwFontStyle, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"fontStyle", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.4f", m_flPositionX);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"posX", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.4f", m_flPositionY);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"posY", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _itow_s(static_cast<int>(m_alignment), szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"alignment", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    hr = pWriter->WriteAttributeString(nullptr, L"background", nullptr, m_fBackground ? L"1" : L"0");
    if (FAILED(hr)) return hr;

    _itow_s(m_dwBackgroundColor, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"bgColor", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _itow_s(m_dwAnimationType, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"animType", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _i64tow_s(m_llAnimationDurationHns, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"animDuration", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    hr = pWriter->WriteEndElement(); // textEffect
    return hr;
}

// ============================================================================
// PanAndZoomShapeEffect implementation
// ============================================================================

PanAndZoomShapeEffect::PanAndZoomShapeEffect()
    : m_flStartLeft(0.0f)
    , m_flStartTop(0.0f)
    , m_flStartRight(1.0f)
    , m_flStartBottom(1.0f)
    , m_flEndLeft(0.1f)
    , m_flEndTop(0.1f)
    , m_flEndRight(0.9f)
    , m_flEndBottom(0.9f)
    , m_shape(PanZoomShapeRect)
    , m_easing(EasingLinear)
    , m_fRandomStartPosition(false)
    , m_llHoldStartHns(0)
    , m_llHoldEndHns(0)
{
    SetType(MovieEffectTypePanZoom);
}

PanAndZoomShapeEffect::~PanAndZoomShapeEffect()
{
}

float PanAndZoomShapeEffect::GetStartLeft() const throw() { return m_flStartLeft; }
float PanAndZoomShapeEffect::GetStartTop() const throw() { return m_flStartTop; }
float PanAndZoomShapeEffect::GetStartRight() const throw() { return m_flStartRight; }
float PanAndZoomShapeEffect::GetStartBottom() const throw() { return m_flStartBottom; }

void PanAndZoomShapeEffect::SetStartRect(float left, float top, float right, float bottom) throw()
{
    m_flStartLeft = left;
    m_flStartTop = top;
    m_flStartRight = right;
    m_flStartBottom = bottom;
}

float PanAndZoomShapeEffect::GetEndLeft() const throw() { return m_flEndLeft; }
float PanAndZoomShapeEffect::GetEndTop() const throw() { return m_flEndTop; }
float PanAndZoomShapeEffect::GetEndRight() const throw() { return m_flEndRight; }
float PanAndZoomShapeEffect::GetEndBottom() const throw() { return m_flEndBottom; }

void PanAndZoomShapeEffect::SetEndRect(float left, float top, float right, float bottom) throw()
{
    m_flEndLeft = left;
    m_flEndTop = top;
    m_flEndRight = right;
    m_flEndBottom = bottom;
}

PanZoomShape PanAndZoomShapeEffect::GetShape() const throw() { return m_shape; }
void PanAndZoomShapeEffect::SetShape(PanZoomShape shape) throw() { m_shape = shape; }

PanAndZoomShapeEffect::EasingType PanAndZoomShapeEffect::GetEasing() const throw() { return m_easing; }
void PanAndZoomShapeEffect::SetEasing(EasingType easing) throw() { m_easing = easing; }

bool PanAndZoomShapeEffect::IsRandomStartPosition() const throw() { return m_fRandomStartPosition; }
void PanAndZoomShapeEffect::SetRandomStartPosition(bool fRandom) throw() { m_fRandomStartPosition = fRandom; }

LONGLONG PanAndZoomShapeEffect::GetHoldStartHns() const throw() { return m_llHoldStartHns; }
void PanAndZoomShapeEffect::SetHoldStartHns(LONGLONG llHold) throw() { m_llHoldStartHns = llHold; }

LONGLONG PanAndZoomShapeEffect::GetHoldEndHns() const throw() { return m_llHoldEndHns; }
void PanAndZoomShapeEffect::SetHoldEndHns(LONGLONG llHold) throw() { m_llHoldEndHns = llHold; }

HRESULT PanAndZoomShapeEffect::LoadFromXml(IXmlReader* pReader)
{
    HRESULT hr = MovieEffect::LoadFromXml(pReader);
    if (FAILED(hr)) return hr;

    LPCWSTR pszValue = nullptr;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"startLeft", &pszValue)) && pszValue)
        m_flStartLeft = (float)_wtof(pszValue);
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"startTop", &pszValue)) && pszValue)
        m_flStartTop = (float)_wtof(pszValue);
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"startRight", &pszValue)) && pszValue)
        m_flStartRight = (float)_wtof(pszValue);
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"startBottom", &pszValue)) && pszValue)
        m_flStartBottom = (float)_wtof(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"endLeft", &pszValue)) && pszValue)
        m_flEndLeft = (float)_wtof(pszValue);
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"endTop", &pszValue)) && pszValue)
        m_flEndTop = (float)_wtof(pszValue);
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"endRight", &pszValue)) && pszValue)
        m_flEndRight = (float)_wtof(pszValue);
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"endBottom", &pszValue)) && pszValue)
        m_flEndBottom = (float)_wtof(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"shape", &pszValue)) && pszValue)
        m_shape = static_cast<PanZoomShape>(_wtoi(pszValue));
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"easing", &pszValue)) && pszValue)
        m_easing = static_cast<EasingType>(_wtoi(pszValue));
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"randomStart", &pszValue)) && pszValue)
        m_fRandomStartPosition = (_wtoi(pszValue) != 0);
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"holdStart", &pszValue)) && pszValue)
        m_llHoldStartHns = _wtoi64(pszValue);
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"holdEnd", &pszValue)) && pszValue)
        m_llHoldEndHns = _wtoi64(pszValue);

    return S_OK;
}

HRESULT PanAndZoomShapeEffect::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_POINTER;

    HRESULT hr;
    WCHAR szBuf[64];
    char szFloat[32];

    hr = pWriter->WriteStartElement(nullptr, L"panZoomEffect", nullptr);
    if (FAILED(hr)) return hr;

    hr = MovieEffect::SaveToXml(pWriter);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.4f", m_flStartLeft);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"startLeft", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.4f", m_flStartTop);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"startTop", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.4f", m_flStartRight);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"startRight", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.4f", m_flStartBottom);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"startBottom", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.4f", m_flEndLeft);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"endLeft", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.4f", m_flEndTop);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"endTop", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.4f", m_flEndRight);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"endRight", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.4f", m_flEndBottom);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"endBottom", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _itow_s(static_cast<int>(m_shape), szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"shape", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _itow_s(static_cast<int>(m_easing), szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"easing", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    hr = pWriter->WriteAttributeString(nullptr, L"randomStart", nullptr, m_fRandomStartPosition ? L"1" : L"0");
    if (FAILED(hr)) return hr;

    _i64tow_s(m_llHoldStartHns, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"holdStart", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _i64tow_s(m_llHoldEndHns, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"holdEnd", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    hr = pWriter->WriteEndElement(); // panZoomEffect
    return hr;
}

// ============================================================================
// AudioDuckingProperties implementation
// ============================================================================

AudioDuckingProperties::AudioDuckingProperties()
    : m_fEnabled(false)
    , m_flDuckLevel(0.5f)
    , m_dwFadeInMs(300)
    , m_dwFadeOutMs(500)
    , m_flThresholdDb(-30.0f)
    , m_dwTargetTrackIndex(1)
{
}

AudioDuckingProperties::~AudioDuckingProperties()
{
}

bool AudioDuckingProperties::IsEnabled() const throw() { return m_fEnabled; }
void AudioDuckingProperties::SetEnabled(bool fEnabled) throw() { m_fEnabled = fEnabled; }

float AudioDuckingProperties::GetDuckLevel() const throw() { return m_flDuckLevel; }
void AudioDuckingProperties::SetDuckLevel(float flLevel) throw()
{
    if (flLevel < 0.0f) flLevel = 0.0f;
    if (flLevel > 1.0f) flLevel = 1.0f;
    m_flDuckLevel = flLevel;
}

DWORD AudioDuckingProperties::GetFadeInMs() const throw() { return m_dwFadeInMs; }
void AudioDuckingProperties::SetFadeInMs(DWORD dwMs) throw() { m_dwFadeInMs = dwMs; }

DWORD AudioDuckingProperties::GetFadeOutMs() const throw() { return m_dwFadeOutMs; }
void AudioDuckingProperties::SetFadeOutMs(DWORD dwMs) throw() { m_dwFadeOutMs = dwMs; }

float AudioDuckingProperties::GetThresholdDb() const throw() { return m_flThresholdDb; }
void AudioDuckingProperties::SetThresholdDb(float flDb) throw() { m_flThresholdDb = flDb; }

DWORD AudioDuckingProperties::GetTargetTrackIndex() const throw() { return m_dwTargetTrackIndex; }
void AudioDuckingProperties::SetTargetTrackIndex(DWORD dwIndex) throw() { m_dwTargetTrackIndex = dwIndex; }

HRESULT AudioDuckingProperties::LoadFromXml(IXmlReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    LPCWSTR pszValue = nullptr;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"enabled", &pszValue)) && pszValue)
        m_fEnabled = (_wtoi(pszValue) != 0);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"duckLevel", &pszValue)) && pszValue)
        m_flDuckLevel = (float)_wtof(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fadeInMs", &pszValue)) && pszValue)
        m_dwFadeInMs = (DWORD)_wtol(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fadeOutMs", &pszValue)) && pszValue)
        m_dwFadeOutMs = (DWORD)_wtol(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"thresholdDb", &pszValue)) && pszValue)
        m_flThresholdDb = (float)_wtof(pszValue);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"targetTrack", &pszValue)) && pszValue)
        m_dwTargetTrackIndex = (DWORD)_wtol(pszValue);

    return S_OK;
}

HRESULT AudioDuckingProperties::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_POINTER;

    HRESULT hr;
    WCHAR szBuf[64];
    char szFloat[32];

    hr = pWriter->WriteStartElement(nullptr, L"audioDucking", nullptr);
    if (FAILED(hr)) return hr;

    hr = pWriter->WriteAttributeString(nullptr, L"enabled", nullptr, m_fEnabled ? L"1" : L"0");
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.2f", m_flDuckLevel);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"duckLevel", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _itow_s(m_dwFadeInMs, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"fadeInMs", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _itow_s(m_dwFadeOutMs, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"fadeOutMs", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    sprintf_s(szFloat, "%.1f", m_flThresholdDb);
    MultiByteToWideChar(CP_ACP, 0, szFloat, -1, szBuf, _countof(szBuf));
    hr = pWriter->WriteAttributeString(nullptr, L"thresholdDb", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _itow_s(m_dwTargetTrackIndex, szBuf, _countof(szBuf), 10);
    hr = pWriter->WriteAttributeString(nullptr, L"targetTrack", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    hr = pWriter->WriteEndElement(); // audioDucking
    return hr;
}

} // namespace StoryboardManager
