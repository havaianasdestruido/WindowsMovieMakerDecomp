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
void MovieEffect::SetDurationHns(LONGLONG llDuration) throw() { m_llDurationHns = llDuration; }

LONGLONG MovieEffect::GetStartOffsetHns() const throw() { return m_llStartOffsetHns; }
void MovieEffect::SetStartOffsetHns(LONGLONG llOffset) throw() { m_llStartOffsetHns = llOffset; }

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
    UNREFERENCED_PARAMETER(pReader);
    return E_NOTIMPL;
}

HRESULT MovieEffect::SaveToXml(IXmlWriter* pWriter)
{
    UNREFERENCED_PARAMETER(pWriter);
    return E_NOTIMPL;
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
void TextEffect::SetFontSize(float flSize) throw() { m_flFontSize = flSize; }

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

HRESULT TextEffect::LoadFromXml(IXmlReader* pReader) { return MovieEffect::LoadFromXml(pReader); }
HRESULT TextEffect::SaveToXml(IXmlWriter* pWriter) { return MovieEffect::SaveToXml(pWriter); }

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

HRESULT PanAndZoomShapeEffect::LoadFromXml(IXmlReader* pReader) { return MovieEffect::LoadFromXml(pReader); }
HRESULT PanAndZoomShapeEffect::SaveToXml(IXmlWriter* pWriter) { return MovieEffect::SaveToXml(pWriter); }

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
    UNREFERENCED_PARAMETER(pReader);
    return E_NOTIMPL;
}

HRESULT AudioDuckingProperties::SaveToXml(IXmlWriter* pWriter)
{
    UNREFERENCED_PARAMETER(pWriter);
    return E_NOTIMPL;
}

} // namespace StoryboardManager
