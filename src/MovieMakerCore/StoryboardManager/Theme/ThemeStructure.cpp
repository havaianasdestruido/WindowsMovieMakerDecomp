#include "pch.h"
/*
 * ThemeStructure.cpp
 *
 * Implementation of the theme structural element classes for the
 * StoryboardManager namespace. Provides the hierarchical theme structure
 * including sections, titles, tracks, effects, and transitions.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "ThemeStructure.h"

namespace StoryboardManager
{

// ============================================================================
// ThemeComplexType implementation
// ============================================================================

ThemeComplexType::ThemeComplexType()
    : m_llDurationHns(0)
    , m_llDelayHns(0)
    , m_pParent(nullptr)
    , m_pX3dTemplate(nullptr)
{
}

ThemeComplexType::~ThemeComplexType()
{
    for (size_t i = 0; i < m_arrChildren.GetCount(); ++i)
    {
        delete m_arrChildren.GetAt(i);
    }
    m_arrChildren.RemoveAll();
}

ATL::CString ThemeComplexType::GetId() const { return m_strId; }
void ThemeComplexType::SetId(LPCWSTR pszId) { m_strId = pszId ? pszId : L""; }

ATL::CString ThemeComplexType::GetName() const { return m_strName; }
void ThemeComplexType::SetName(LPCWSTR pszName) { m_strName = pszName ? pszName : L""; }

ThemeElementType ThemeComplexType::GetElementType() const throw()
{
    return ThemeElementTypeUnknown;
}

LONGLONG ThemeComplexType::GetDurationHns() const throw() { return m_llDurationHns; }
void ThemeComplexType::SetDurationHns(LONGLONG llDuration) throw() { m_llDurationHns = llDuration; }

LONGLONG ThemeComplexType::GetDelayHns() const throw() { return m_llDelayHns; }
void ThemeComplexType::SetDelayHns(LONGLONG llDelay) throw() { m_llDelayHns = llDelay; }

size_t ThemeComplexType::GetChildCount() const throw()
{
    return m_arrChildren.GetCount();
}

ThemeComplexType* ThemeComplexType::GetChild(size_t nIndex)
{
    if (nIndex >= m_arrChildren.GetCount())
        return nullptr;
    return m_arrChildren.GetAt(nIndex);
}

const ThemeComplexType* ThemeComplexType::GetChild(size_t nIndex) const
{
    if (nIndex >= m_arrChildren.GetCount())
        return nullptr;
    return m_arrChildren.GetAt(nIndex);
}

size_t ThemeComplexType::AddChild(ThemeComplexType* pChild)
{
    ATLASSERT(pChild != nullptr);
    pChild->m_pParent = this;
    return m_arrChildren.Add(pChild);
}

void ThemeComplexType::RemoveChild(size_t nIndex)
{
    if (nIndex < m_arrChildren.GetCount())
    {
        ThemeComplexType* pChild = m_arrChildren.GetAt(nIndex);
        pChild->m_pParent = nullptr;
        delete pChild;
        m_arrChildren.RemoveAt(nIndex);
    }
}

void ThemeComplexType::RemoveAllChildren()
{
    for (size_t i = 0; i < m_arrChildren.GetCount(); ++i)
    {
        ThemeComplexType* pChild = m_arrChildren.GetAt(i);
        pChild->m_pParent = nullptr;
        delete pChild;
    }
    m_arrChildren.RemoveAll();
}

ThemeComplexType* ThemeComplexType::GetParent() { return m_pParent; }
const ThemeComplexType* ThemeComplexType::GetParent() const { return m_pParent; }

BaseX3DTemplate* ThemeComplexType::GetX3dTemplate() { return m_pX3dTemplate; }
const BaseX3DTemplate* ThemeComplexType::GetX3dTemplate() const { return m_pX3dTemplate; }
void ThemeComplexType::SetX3dTemplate(BaseX3DTemplate* pTemplate) { m_pX3dTemplate = pTemplate; }

HRESULT ThemeComplexType::LoadFromXml(IXmlReader* pReader)
{
    UNREFERENCED_PARAMETER(pReader);
    return E_NOTIMPL;
}

HRESULT ThemeComplexType::SaveToXml(IXmlWriter* pWriter)
{
    UNREFERENCED_PARAMETER(pWriter);
    return E_NOTIMPL;
}

// ============================================================================
// ThemeIntro implementation
// ============================================================================

ThemeIntro::ThemeIntro()
    : m_pTitle(nullptr)
    , m_durationMode(IntroDurationModeFixed)
    , m_dwBackgroundColor(0xFF000000)
    , m_llFadeInDurationHns(5000000)
{
}

ThemeIntro::~ThemeIntro() { delete m_pTitle; }

ThemeElementType ThemeIntro::GetElementType() const throw() { return ThemeElementTypeIntro; }

ThemeTitle* ThemeIntro::GetTitle() { return m_pTitle; }
const ThemeTitle* ThemeIntro::GetTitle() const { return m_pTitle; }

void ThemeIntro::SetTitle(ThemeTitle* pTitle)
{
    if (m_pTitle && m_pTitle != pTitle) delete m_pTitle;
    m_pTitle = pTitle;
}

ThemeIntro::IntroDurationMode ThemeIntro::GetDurationMode() const throw() { return m_durationMode; }
void ThemeIntro::SetDurationMode(IntroDurationMode mode) throw() { m_durationMode = mode; }

DWORD ThemeIntro::GetBackgroundColor() const throw() { return m_dwBackgroundColor; }
void ThemeIntro::SetBackgroundColor(DWORD dwColor) throw() { m_dwBackgroundColor = dwColor; }

LONGLONG ThemeIntro::GetFadeInDurationHns() const throw() { return m_llFadeInDurationHns; }
void ThemeIntro::SetFadeInDurationHns(LONGLONG llDuration) throw() { m_llFadeInDurationHns = llDuration; }

HRESULT ThemeIntro::LoadFromXml(IXmlReader* pReader) { return ThemeComplexType::LoadFromXml(pReader); }
HRESULT ThemeIntro::SaveToXml(IXmlWriter* pWriter) { return ThemeComplexType::SaveToXml(pWriter); }

// ============================================================================
// ThemeMid implementation
// ============================================================================

ThemeMid::ThemeMid()
    : m_pDefaultEffectTemplate(nullptr)
    , m_pDefaultTransition(nullptr)
    , m_fLooping(true)
{
}

ThemeMid::~ThemeMid()
{
    for (size_t i = 0; i < m_arrTracks.GetCount(); ++i)
        delete m_arrTracks.GetAt(i);
    m_arrTracks.RemoveAll();
}

ThemeElementType ThemeMid::GetElementType() const throw() { return ThemeElementTypeMid; }

size_t ThemeMid::GetTrackCount() const throw() { return m_arrTracks.GetCount(); }

ThemeTrack* ThemeMid::GetTrack(size_t nIndex)
{
    if (nIndex >= m_arrTracks.GetCount()) return nullptr;
    return m_arrTracks.GetAt(nIndex);
}

const ThemeTrack* ThemeMid::GetTrack(size_t nIndex) const
{
    if (nIndex >= m_arrTracks.GetCount()) return nullptr;
    return m_arrTracks.GetAt(nIndex);
}

size_t ThemeMid::AddTrack(ThemeTrack* pTrack) { ATLASSERT(pTrack); return m_arrTracks.Add(pTrack); }

void ThemeMid::RemoveTrack(size_t nIndex)
{
    if (nIndex < m_arrTracks.GetCount()) { delete m_arrTracks.GetAt(nIndex); m_arrTracks.RemoveAt(nIndex); }
}

void ThemeMid::RemoveAllTracks()
{
    for (size_t i = 0; i < m_arrTracks.GetCount(); ++i) delete m_arrTracks.GetAt(i);
    m_arrTracks.RemoveAll();
}

ThemeEffectTemplate* ThemeMid::GetDefaultEffectTemplate() { return m_pDefaultEffectTemplate; }
const ThemeEffectTemplate* ThemeMid::GetDefaultEffectTemplate() const { return m_pDefaultEffectTemplate; }
void ThemeMid::SetDefaultEffectTemplate(ThemeEffectTemplate* pTemplate) { m_pDefaultEffectTemplate = pTemplate; }

ThemeTransition* ThemeMid::GetDefaultTransition() { return m_pDefaultTransition; }
const ThemeTransition* ThemeMid::GetDefaultTransition() const { return m_pDefaultTransition; }
void ThemeMid::SetDefaultTransition(ThemeTransition* pTransition) { m_pDefaultTransition = pTransition; }

bool ThemeMid::IsLooping() const throw() { return m_fLooping; }
void ThemeMid::SetLooping(bool fLooping) throw() { m_fLooping = fLooping; }

HRESULT ThemeMid::LoadFromXml(IXmlReader* pReader) { return ThemeComplexType::LoadFromXml(pReader); }
HRESULT ThemeMid::SaveToXml(IXmlWriter* pWriter) { return ThemeComplexType::SaveToXml(pWriter); }

// ============================================================================
// ThemeOutro implementation
// ============================================================================

ThemeOutro::ThemeOutro()
    : m_pCreditsTitle(nullptr)
    , m_durationMode(OutroDurationModeFixed)
    , m_dwBackgroundColor(0xFF000000)
    , m_llFadeOutDurationHns(5000000)
    , m_flScrollSpeed(50.0f)
{
}

ThemeOutro::~ThemeOutro() { delete m_pCreditsTitle; }

ThemeElementType ThemeOutro::GetElementType() const throw() { return ThemeElementTypeOutro; }

ThemeTitle* ThemeOutro::GetCreditsTitle() { return m_pCreditsTitle; }
const ThemeTitle* ThemeOutro::GetCreditsTitle() const { return m_pCreditsTitle; }

void ThemeOutro::SetCreditsTitle(ThemeTitle* pTitle)
{
    if (m_pCreditsTitle && m_pCreditsTitle != pTitle) delete m_pCreditsTitle;
    m_pCreditsTitle = pTitle;
}

ThemeOutro::OutroDurationMode ThemeOutro::GetDurationMode() const throw() { return m_durationMode; }
void ThemeOutro::SetDurationMode(OutroDurationMode mode) throw() { m_durationMode = mode; }

DWORD ThemeOutro::GetBackgroundColor() const throw() { return m_dwBackgroundColor; }
void ThemeOutro::SetBackgroundColor(DWORD dwColor) throw() { m_dwBackgroundColor = dwColor; }

LONGLONG ThemeOutro::GetFadeOutDurationHns() const throw() { return m_llFadeOutDurationHns; }
void ThemeOutro::SetFadeOutDurationHns(LONGLONG llDuration) throw() { m_llFadeOutDurationHns = llDuration; }

float ThemeOutro::GetScrollSpeed() const throw() { return m_flScrollSpeed; }
void ThemeOutro::SetScrollSpeed(float flSpeed) throw() { m_flScrollSpeed = flSpeed; }

HRESULT ThemeOutro::LoadFromXml(IXmlReader* pReader) { return ThemeComplexType::LoadFromXml(pReader); }
HRESULT ThemeOutro::SaveToXml(IXmlWriter* pWriter) { return ThemeComplexType::SaveToXml(pWriter); }

// ============================================================================
// ComplexIntro implementation
// ============================================================================

ComplexIntro::ComplexIntro() {}

ComplexIntro::~ComplexIntro()
{
    for (size_t i = 0; i < m_arrLayers.GetCount(); ++i) delete m_arrLayers.GetAt(i);
    m_arrLayers.RemoveAll();
    for (size_t i = 0; i < m_arrEntranceEffects.GetCount(); ++i) delete m_arrEntranceEffects.GetAt(i);
    m_arrEntranceEffects.RemoveAll();
}

size_t ComplexIntro::GetLayerCount() const throw() { return m_arrLayers.GetCount(); }

ThemeComplexType* ComplexIntro::GetLayer(size_t nIndex)
{
    if (nIndex >= m_arrLayers.GetCount()) return nullptr;
    return m_arrLayers.GetAt(nIndex);
}

const ThemeComplexType* ComplexIntro::GetLayer(size_t nIndex) const
{
    if (nIndex >= m_arrLayers.GetCount()) return nullptr;
    return m_arrLayers.GetAt(nIndex);
}

size_t ComplexIntro::AddLayer(ThemeComplexType* pLayer) { ATLASSERT(pLayer); return m_arrLayers.Add(pLayer); }

void ComplexIntro::RemoveLayer(size_t nIndex)
{
    if (nIndex < m_arrLayers.GetCount()) { delete m_arrLayers.GetAt(nIndex); m_arrLayers.RemoveAt(nIndex); }
}

size_t ComplexIntro::GetEntranceEffectCount() const throw() { return m_arrEntranceEffects.GetCount(); }

ThemeEffect* ComplexIntro::GetEntranceEffect(size_t nIndex)
{
    if (nIndex >= m_arrEntranceEffects.GetCount()) return nullptr;
    return m_arrEntranceEffects.GetAt(nIndex);
}

void ComplexIntro::AddEntranceEffect(ThemeEffect* pEffect) { ATLASSERT(pEffect); m_arrEntranceEffects.Add(pEffect); }

void ComplexIntro::RemoveAllEntranceEffects()
{
    for (size_t i = 0; i < m_arrEntranceEffects.GetCount(); ++i) delete m_arrEntranceEffects.GetAt(i);
    m_arrEntranceEffects.RemoveAll();
}

HRESULT ComplexIntro::LoadFromXml(IXmlReader* pReader) { return ThemeIntro::LoadFromXml(pReader); }
HRESULT ComplexIntro::SaveToXml(IXmlWriter* pWriter) { return ThemeIntro::SaveToXml(pWriter); }

// ============================================================================
// SimpleIntro implementation
// ============================================================================

SimpleIntro::SimpleIntro()
    : m_flOverlayPositionX(0.5f)
    , m_flOverlayPositionY(0.5f)
{
}

SimpleIntro::~SimpleIntro() {}

ATL::CString SimpleIntro::GetOverlayText() const { return m_strOverlayText; }
void SimpleIntro::SetOverlayText(LPCWSTR pszText) { m_strOverlayText = pszText ? pszText : L""; }

float SimpleIntro::GetOverlayPositionX() const throw() { return m_flOverlayPositionX; }
float SimpleIntro::GetOverlayPositionY() const throw() { return m_flOverlayPositionY; }
void SimpleIntro::SetOverlayPosition(float x, float y) throw() { m_flOverlayPositionX = x; m_flOverlayPositionY = y; }

HRESULT SimpleIntro::LoadFromXml(IXmlReader* pReader) { return ThemeIntro::LoadFromXml(pReader); }
HRESULT SimpleIntro::SaveToXml(IXmlWriter* pWriter) { return ThemeIntro::SaveToXml(pWriter); }

// ============================================================================
// ThemeTitle implementation
// ============================================================================

ThemeTitle::ThemeTitle()
    : m_flFontSize(36.0f)
    , m_dwFontColor(0xFFFFFFFF)
    , m_dwFontStyle(0)
    , m_flPositionX(0.5f)
    , m_flPositionY(0.5f)
    , m_alignment(TextAlignmentCenter)
    , m_animation(TitleAnimationFadeIn)
    , m_fShadow(false)
    , m_dwShadowColor(0x80000000)
    , m_fOutline(false)
    , m_dwOutlineColor(0xFF000000)
    , m_flOutlineWidth(1.0f)
    , m_fBackground(false)
    , m_dwBackgroundColor(0x80000000)
    , m_flBackgroundOpacity(0.5f)
    , m_flMaxWidth(0.0f)
{
}

ThemeTitle::~ThemeTitle() {}

ATL::CString ThemeTitle::GetText() const { return m_strText; }
void ThemeTitle::SetText(LPCWSTR pszText) { m_strText = pszText ? pszText : L""; }

ATL::CString ThemeTitle::GetFontFamily() const { return m_strFontFamily; }
void ThemeTitle::SetFontFamily(LPCWSTR pszFontFamily) { m_strFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI"; }

float ThemeTitle::GetFontSize() const throw() { return m_flFontSize; }
void ThemeTitle::SetFontSize(float flSize) throw() { m_flFontSize = flSize; }

DWORD ThemeTitle::GetFontColor() const throw() { return m_dwFontColor; }
void ThemeTitle::SetFontColor(DWORD dwColor) throw() { m_dwFontColor = dwColor; }

DWORD ThemeTitle::GetFontStyle() const throw() { return m_dwFontStyle; }
void ThemeTitle::SetFontStyle(DWORD dwStyle) throw() { m_dwFontStyle = dwStyle; }

float ThemeTitle::GetPositionX() const throw() { return m_flPositionX; }
float ThemeTitle::GetPositionY() const throw() { return m_flPositionY; }
void ThemeTitle::SetPosition(float x, float y) throw() { m_flPositionX = x; m_flPositionY = y; }

ThemeTitle::TextAlignment ThemeTitle::GetAlignment() const throw() { return m_alignment; }
void ThemeTitle::SetAlignment(TextAlignment align) throw() { m_alignment = align; }

TitleAnimationType ThemeTitle::GetAnimation() const throw() { return m_animation; }
void ThemeTitle::SetAnimation(TitleAnimationType type) throw() { m_animation = type; }

bool ThemeTitle::HasShadow() const throw() { return m_fShadow; }
void ThemeTitle::SetShadow(bool fShadow) throw() { m_fShadow = fShadow; }
DWORD ThemeTitle::GetShadowColor() const throw() { return m_dwShadowColor; }
void ThemeTitle::SetShadowColor(DWORD dwColor) throw() { m_dwShadowColor = dwColor; }

bool ThemeTitle::HasOutline() const throw() { return m_fOutline; }
void ThemeTitle::SetOutline(bool fOutline) throw() { m_fOutline = fOutline; }
DWORD ThemeTitle::GetOutlineColor() const throw() { return m_dwOutlineColor; }
void ThemeTitle::SetOutlineColor(DWORD dwColor) throw() { m_dwOutlineColor = dwColor; }
float ThemeTitle::GetOutlineWidth() const throw() { return m_flOutlineWidth; }
void ThemeTitle::SetOutlineWidth(float flWidth) throw() { m_flOutlineWidth = flWidth; }

bool ThemeTitle::HasBackground() const throw() { return m_fBackground; }
void ThemeTitle::SetBackground(bool fBackground) throw() { m_fBackground = fBackground; }
DWORD ThemeTitle::GetBackgroundColor() const throw() { return m_dwBackgroundColor; }
void ThemeTitle::SetBackgroundColor(DWORD dwColor) throw() { m_dwBackgroundColor = dwColor; }
float ThemeTitle::GetBackgroundOpacity() const throw() { return m_flBackgroundOpacity; }
void ThemeTitle::SetBackgroundOpacity(float flOpacity) throw() { m_flBackgroundOpacity = flOpacity; }

float ThemeTitle::GetMaxWidth() const throw() { return m_flMaxWidth; }
void ThemeTitle::SetMaxWidth(float flMaxWidth) throw() { m_flMaxWidth = flMaxWidth; }

// ============================================================================
// ThemeComplexTitle implementation
// ============================================================================

ThemeComplexTitle::ThemeComplexTitle() {}
ThemeComplexTitle::~ThemeComplexTitle() {}

size_t ThemeComplexTitle::GetLineCount() const throw() { return m_arrLines.GetCount(); }

const ThemeComplexTitle::TitleLine* ThemeComplexTitle::GetLine(size_t nIndex) const
{
    if (nIndex >= m_arrLines.GetCount()) return nullptr;
    return &m_arrLines.GetAt(nIndex);
}

size_t ThemeComplexTitle::AddLine(const TitleLine& line) { return m_arrLines.Add(line); }

void ThemeComplexTitle::RemoveLine(size_t nIndex)
{
    if (nIndex < m_arrLines.GetCount()) m_arrLines.RemoveAt(nIndex);
}

void ThemeComplexTitle::RemoveAllLines() { m_arrLines.RemoveAll(); }

void ThemeComplexTitle::SetLineText(size_t nIndex, LPCWSTR pszText)
{
    if (nIndex < m_arrLines.GetCount())
        m_arrLines.GetAt(nIndex).strText = pszText ? pszText : L"";
}

void ThemeComplexTitle::SetLineFont(size_t nIndex, LPCWSTR pszFontFamily, float flSize, DWORD dwColor)
{
    if (nIndex < m_arrLines.GetCount())
    {
        TitleLine& line = m_arrLines.GetAt(nIndex);
        line.strFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
        line.flFontSize = flSize;
        line.dwFontColor = dwColor;
    }
}

// ============================================================================
// ThemeSimpleTitle implementation
// ============================================================================

ThemeSimpleTitle::ThemeSimpleTitle()
    : m_fAutoFitEnabled(false)
    , m_flMinFontSize(12.0f)
    , m_flMaxFontSize(72.0f)
{
}

ThemeSimpleTitle::~ThemeSimpleTitle() {}

bool ThemeSimpleTitle::IsAutoFitEnabled() const throw() { return m_fAutoFitEnabled; }
void ThemeSimpleTitle::SetAutoFitEnabled(bool fEnabled) throw() { m_fAutoFitEnabled = fEnabled; }

float ThemeSimpleTitle::GetMinFontSize() const throw() { return m_flMinFontSize; }
void ThemeSimpleTitle::SetMinFontSize(float flSize) throw() { m_flMinFontSize = flSize; }

float ThemeSimpleTitle::GetMaxFontSize() const throw() { return m_flMaxFontSize; }
void ThemeSimpleTitle::SetMaxFontSize(float flSize) throw() { m_flMaxFontSize = flSize; }

// ============================================================================
// ThemeTrack implementation
// ============================================================================

ThemeTrack::ThemeTrack() : m_llDurationHns(0) {}

ThemeTrack::~ThemeTrack()
{
    for (size_t i = 0; i < m_arrElements.GetCount(); ++i) delete m_arrElements.GetAt(i);
    m_arrElements.RemoveAll();
}

ATL::CString ThemeTrack::GetId() const { return m_strId; }
void ThemeTrack::SetId(LPCWSTR pszId) { m_strId = pszId ? pszId : L""; }

ATL::CString ThemeTrack::GetName() const { return m_strName; }
void ThemeTrack::SetName(LPCWSTR pszName) { m_strName = pszName ? pszName : L""; }

ThemeTrack::ThemeTrackType ThemeTrack::GetTrackType() const throw() { return ThemeTrackTypePrimary; }

size_t ThemeTrack::GetElementCount() const throw() { return m_arrElements.GetCount(); }

ThemeComplexType* ThemeTrack::GetElement(size_t nIndex)
{
    if (nIndex >= m_arrElements.GetCount()) return nullptr;
    return m_arrElements.GetAt(nIndex);
}

const ThemeComplexType* ThemeTrack::GetElement(size_t nIndex) const
{
    if (nIndex >= m_arrElements.GetCount()) return nullptr;
    return m_arrElements.GetAt(nIndex);
}

size_t ThemeTrack::AddElement(ThemeComplexType* pElement) { ATLASSERT(pElement); return m_arrElements.Add(pElement); }

void ThemeTrack::RemoveElement(size_t nIndex)
{
    if (nIndex < m_arrElements.GetCount()) { delete m_arrElements.GetAt(nIndex); m_arrElements.RemoveAt(nIndex); }
}

void ThemeTrack::RemoveAllElements()
{
    for (size_t i = 0; i < m_arrElements.GetCount(); ++i) delete m_arrElements.GetAt(i);
    m_arrElements.RemoveAll();
}

LONGLONG ThemeTrack::GetDurationHns() const throw() { return m_llDurationHns; }
void ThemeTrack::SetDurationHns(LONGLONG llDuration) throw() { m_llDurationHns = llDuration; }

// ============================================================================
// ThemePrimaryTrack implementation
// ============================================================================

ThemePrimaryTrack::ThemePrimaryTrack()
    : m_boundTrackType(TimelineTrackTypeVideo)
    , m_fAutoFill(true)
{
}

ThemePrimaryTrack::~ThemePrimaryTrack() {}

ThemeTrack::ThemeTrackType ThemePrimaryTrack::GetTrackType() const throw() { return ThemeTrackTypePrimary; }

TimelineTrackType ThemePrimaryTrack::GetBoundTrackType() const throw() { return m_boundTrackType; }
void ThemePrimaryTrack::SetBoundTrackType(TimelineTrackType type) throw() { m_boundTrackType = type; }

bool ThemePrimaryTrack::IsAutoFill() const throw() { return m_fAutoFill; }
void ThemePrimaryTrack::SetAutoFill(bool fAutoFill) throw() { m_fAutoFill = fAutoFill; }

// ============================================================================
// ThemeDependentTrack implementation
// ============================================================================

ThemeDependentTrack::ThemeDependentTrack() : m_syncMode(SyncModeSyncStart) {}
ThemeDependentTrack::~ThemeDependentTrack() {}

ThemeTrack::ThemeTrackType ThemeDependentTrack::GetTrackType() const throw() { return ThemeTrackTypeDependent; }

ATL::CString ThemeDependentTrack::GetDependsOnTrackId() const { return m_strDependsOnTrackId; }
void ThemeDependentTrack::SetDependsOnTrackId(LPCWSTR pszTrackId) { m_strDependsOnTrackId = pszTrackId ? pszTrackId : L""; }

ThemeDependentTrack::SyncMode ThemeDependentTrack::GetSyncMode() const throw() { return m_syncMode; }
void ThemeDependentTrack::SetSyncMode(SyncMode mode) throw() { m_syncMode = mode; }

// ============================================================================
// ThemeSimpleElement implementation
// ============================================================================

ThemeSimpleElement::ThemeSimpleElement()
    : m_pEffectOverlay(nullptr)
    , m_dwAspectRatioMode(0)
{
}

ThemeSimpleElement::~ThemeSimpleElement() { delete m_pEffectOverlay; }

ThemeElementType ThemeSimpleElement::GetElementType() const throw() { return ThemeElementTypeSimpleElement; }

ATL::CString ThemeSimpleElement::GetMediaSocketId() const { return m_strMediaSocketId; }
void ThemeSimpleElement::SetMediaSocketId(LPCWSTR pszId) { m_strMediaSocketId = pszId ? pszId : L""; }

ThemeEffect* ThemeSimpleElement::GetEffectOverlay() { return m_pEffectOverlay; }
const ThemeEffect* ThemeSimpleElement::GetEffectOverlay() const { return m_pEffectOverlay; }

void ThemeSimpleElement::SetEffectOverlay(ThemeEffect* pEffect)
{
    if (m_pEffectOverlay && m_pEffectOverlay != pEffect) delete m_pEffectOverlay;
    m_pEffectOverlay = pEffect;
}

DWORD ThemeSimpleElement::GetAspectRatioMode() const throw() { return m_dwAspectRatioMode; }
void ThemeSimpleElement::SetAspectRatioMode(DWORD dwMode) throw() { m_dwAspectRatioMode = dwMode; }

HRESULT ThemeSimpleElement::LoadFromXml(IXmlReader* pReader) { return ThemeComplexType::LoadFromXml(pReader); }
HRESULT ThemeSimpleElement::SaveToXml(IXmlWriter* pWriter) { return ThemeComplexType::SaveToXml(pWriter); }

// ============================================================================
// ThemeInterior implementation
// ============================================================================

ThemeInterior::ThemeInterior()
    : m_mediaBoundsLeft(0.0f)
    , m_mediaBoundsTop(0.0f)
    , m_mediaBoundsRight(1.0f)
    , m_mediaBoundsBottom(1.0f)
{
}

ThemeInterior::~ThemeInterior()
{
    for (size_t i = 0; i < m_arrDecorations.GetCount(); ++i) delete m_arrDecorations.GetAt(i);
    m_arrDecorations.RemoveAll();
}

ThemeElementType ThemeInterior::GetElementType() const throw() { return ThemeElementTypeInterior; }

ATL::CString ThemeInterior::GetBackgroundModelPath() const { return m_strBackgroundModelPath; }
void ThemeInterior::SetBackgroundModelPath(LPCWSTR pszPath) { m_strBackgroundModelPath = pszPath ? pszPath : L""; }

size_t ThemeInterior::GetDecorationCount() const throw() { return m_arrDecorations.GetCount(); }

ThemeSimpleElement* ThemeInterior::GetDecoration(size_t nIndex)
{
    if (nIndex >= m_arrDecorations.GetCount()) return nullptr;
    return m_arrDecorations.GetAt(nIndex);
}

void ThemeInterior::AddDecoration(ThemeSimpleElement* pDecoration) { ATLASSERT(pDecoration); m_arrDecorations.Add(pDecoration); }

void ThemeInterior::RemoveAllDecorations()
{
    for (size_t i = 0; i < m_arrDecorations.GetCount(); ++i) delete m_arrDecorations.GetAt(i);
    m_arrDecorations.RemoveAll();
}

float ThemeInterior::GetMediaBoundsLeft() const throw() { return m_mediaBoundsLeft; }
float ThemeInterior::GetMediaBoundsTop() const throw() { return m_mediaBoundsTop; }
float ThemeInterior::GetMediaBoundsRight() const throw() { return m_mediaBoundsRight; }
float ThemeInterior::GetMediaBoundsBottom() const throw() { return m_mediaBoundsBottom; }

void ThemeInterior::SetMediaBounds(float left, float top, float right, float bottom) throw()
{
    m_mediaBoundsLeft = left; m_mediaBoundsTop = top;
    m_mediaBoundsRight = right; m_mediaBoundsBottom = bottom;
}

HRESULT ThemeInterior::LoadFromXml(IXmlReader* pReader) { return ThemeComplexType::LoadFromXml(pReader); }
HRESULT ThemeInterior::SaveToXml(IXmlWriter* pWriter) { return ThemeComplexType::SaveToXml(pWriter); }

// ============================================================================
// ThemeTransition implementation
// ============================================================================

ThemeTransition::ThemeTransition()
    : m_llDurationHns(10000000)
    , m_pX3dTemplate(nullptr)
{
}

ThemeTransition::~ThemeTransition() {}

ATL::CString ThemeTransition::GetId() const { return m_strId; }
void ThemeTransition::SetId(LPCWSTR pszId) { m_strId = pszId ? pszId : L""; }

ATL::CString ThemeTransition::GetName() const { return m_strName; }
void ThemeTransition::SetName(LPCWSTR pszName) { m_strName = pszName ? pszName : L""; }

ATL::CString ThemeTransition::GetClipName() const { return m_strClipName; }
void ThemeTransition::SetClipName(LPCWSTR pszClipName) { m_strClipName = pszClipName ? pszClipName : L""; }

LONGLONG ThemeTransition::GetDurationHns() const throw() { return m_llDurationHns; }
void ThemeTransition::SetDurationHns(LONGLONG llDuration) throw() { m_llDurationHns = llDuration; }

size_t ThemeTransition::GetParameterCount() const throw() { return m_arrParameters.GetCount(); }

TemplateProperty* ThemeTransition::GetParameter(size_t nIndex)
{
    if (nIndex >= m_arrParameters.GetCount()) return nullptr;
    return &m_arrParameters.GetAt(nIndex);
}

TemplateProperty* ThemeTransition::FindParameter(LPCWSTR pszKey)
{
    if (!pszKey) return nullptr;
    for (size_t i = 0; i < m_arrParameters.GetCount(); ++i)
    {
        if (m_arrParameters.GetAt(i).GetKey().CompareNoCase(pszKey) == 0)
            return &m_arrParameters.GetAt(i);
    }
    return nullptr;
}

size_t ThemeTransition::AddParameter(const TemplateProperty& param) { return m_arrParameters.Add(param); }
void ThemeTransition::RemoveAllParameters() { m_arrParameters.RemoveAll(); }

ThemeX3DTemplate* ThemeTransition::GetX3dTemplate() { return m_pX3dTemplate; }
void ThemeTransition::SetX3dTemplate(ThemeX3DTemplate* pTemplate) { m_pX3dTemplate = pTemplate; }

HRESULT ThemeTransition::LoadFromXml(IXmlReader* pReader) { UNREFERENCED_PARAMETER(pReader); return E_NOTIMPL; }
HRESULT ThemeTransition::SaveToXml(IXmlWriter* pWriter) { UNREFERENCED_PARAMETER(pWriter); return E_NOTIMPL; }

// ============================================================================
// ThemeEffectTemplate implementation
// ============================================================================

ThemeEffectTemplate::ThemeEffectTemplate() : m_flDefaultIntensity(1.0f) {}

ThemeEffectTemplate::~ThemeEffectTemplate()
{
    for (size_t i = 0; i < m_arrEffects.GetCount(); ++i) delete m_arrEffects.GetAt(i);
    m_arrEffects.RemoveAll();
}

ATL::CString ThemeEffectTemplate::GetId() const { return m_strId; }
void ThemeEffectTemplate::SetId(LPCWSTR pszId) { m_strId = pszId ? pszId : L""; }

ATL::CString ThemeEffectTemplate::GetName() const { return m_strName; }
void ThemeEffectTemplate::SetName(LPCWSTR pszName) { m_strName = pszName ? pszName : L""; }

size_t ThemeEffectTemplate::GetEffectCount() const throw() { return m_arrEffects.GetCount(); }

ThemeEffect* ThemeEffectTemplate::GetEffect(size_t nIndex)
{
    if (nIndex >= m_arrEffects.GetCount()) return nullptr;
    return m_arrEffects.GetAt(nIndex);
}

const ThemeEffect* ThemeEffectTemplate::GetEffect(size_t nIndex) const
{
    if (nIndex >= m_arrEffects.GetCount()) return nullptr;
    return m_arrEffects.GetAt(nIndex);
}

size_t ThemeEffectTemplate::AddEffect(ThemeEffect* pEffect) { ATLASSERT(pEffect); return m_arrEffects.Add(pEffect); }

void ThemeEffectTemplate::RemoveEffect(size_t nIndex)
{
    if (nIndex < m_arrEffects.GetCount()) { delete m_arrEffects.GetAt(nIndex); m_arrEffects.RemoveAt(nIndex); }
}

void ThemeEffectTemplate::RemoveAllEffects()
{
    for (size_t i = 0; i < m_arrEffects.GetCount(); ++i) delete m_arrEffects.GetAt(i);
    m_arrEffects.RemoveAll();
}

float ThemeEffectTemplate::GetDefaultIntensity() const throw() { return m_flDefaultIntensity; }
void ThemeEffectTemplate::SetDefaultIntensity(float flIntensity) throw() { m_flDefaultIntensity = flIntensity; }

HRESULT ThemeEffectTemplate::LoadFromXml(IXmlReader* pReader) { UNREFERENCED_PARAMETER(pReader); return E_NOTIMPL; }
HRESULT ThemeEffectTemplate::SaveToXml(IXmlWriter* pWriter) { UNREFERENCED_PARAMETER(pWriter); return E_NOTIMPL; }

// ============================================================================
// ThemeFirstEffect / ThemeLastEffect implementation
// ============================================================================

ThemeFirstEffect::ThemeFirstEffect()
    : m_pEffect(nullptr)
    , m_llEntryDelayHns(0)
{
}

ThemeFirstEffect::~ThemeFirstEffect() {}

ThemeEffect* ThemeFirstEffect::GetEffect() { return m_pEffect; }
const ThemeEffect* ThemeFirstEffect::GetEffect() const { return m_pEffect; }
void ThemeFirstEffect::SetEffect(ThemeEffect* pEffect) { m_pEffect = pEffect; }

ATL::CString ThemeFirstEffect::GetEntryAnimation() const { return m_strEntryAnimation; }
void ThemeFirstEffect::SetEntryAnimation(LPCWSTR pszAnimation) { m_strEntryAnimation = pszAnimation ? pszAnimation : L""; }

LONGLONG ThemeFirstEffect::GetEntryDelayHns() const throw() { return m_llEntryDelayHns; }
void ThemeFirstEffect::SetEntryDelayHns(LONGLONG llDelay) throw() { m_llEntryDelayHns = llDelay; }

ThemeLastEffect::ThemeLastEffect()
    : m_pEffect(nullptr)
    , m_llExitHoldHns(0)
{
}

ThemeLastEffect::~ThemeLastEffect() {}

ThemeEffect* ThemeLastEffect::GetEffect() { return m_pEffect; }
const ThemeEffect* ThemeLastEffect::GetEffect() const { return m_pEffect; }
void ThemeLastEffect::SetEffect(ThemeEffect* pEffect) { m_pEffect = pEffect; }

ATL::CString ThemeLastEffect::GetExitAnimation() const { return m_strExitAnimation; }
void ThemeLastEffect::SetExitAnimation(LPCWSTR pszAnimation) { m_strExitAnimation = pszAnimation ? pszAnimation : L""; }

LONGLONG ThemeLastEffect::GetExitHoldHns() const throw() { return m_llExitHoldHns; }
void ThemeLastEffect::SetExitHoldHns(LONGLONG llHold) throw() { m_llExitHoldHns = llHold; }

// ============================================================================
// ThemeFirstTransition / ThemeLastTransition implementation
// ============================================================================

ThemeFirstTransition::ThemeFirstTransition()
    : m_pTransition(nullptr)
    , m_fFadeFromBlack(true)
    , m_llFadeDurationHns(10000000)
{
}

ThemeFirstTransition::~ThemeFirstTransition() {}

ThemeTransition* ThemeFirstTransition::GetTransition() { return m_pTransition; }
const ThemeTransition* ThemeFirstTransition::GetTransition() const { return m_pTransition; }
void ThemeFirstTransition::SetTransition(ThemeTransition* pTransition) { m_pTransition = pTransition; }

bool ThemeFirstTransition::IsFadeFromBlack() const throw() { return m_fFadeFromBlack; }
void ThemeFirstTransition::SetFadeFromBlack(bool fFade) throw() { m_fFadeFromBlack = fFade; }

LONGLONG ThemeFirstTransition::GetFadeDurationHns() const throw() { return m_llFadeDurationHns; }
void ThemeFirstTransition::SetFadeDurationHns(LONGLONG llDuration) throw() { m_llFadeDurationHns = llDuration; }

ThemeLastTransition::ThemeLastTransition()
    : m_pTransition(nullptr)
    , m_fFadeToBlack(true)
    , m_llFadeDurationHns(10000000)
{
}

ThemeLastTransition::~ThemeLastTransition() {}

ThemeTransition* ThemeLastTransition::GetTransition() { return m_pTransition; }
const ThemeTransition* ThemeLastTransition::GetTransition() const { return m_pTransition; }
void ThemeLastTransition::SetTransition(ThemeTransition* pTransition) { m_pTransition = pTransition; }

bool ThemeLastTransition::IsFadeToBlack() const throw() { return m_fFadeToBlack; }
void ThemeLastTransition::SetFadeToBlack(bool fFade) throw() { m_fFadeToBlack = fFade; }

LONGLONG ThemeLastTransition::GetFadeDurationHns() const throw() { return m_llFadeDurationHns; }
void ThemeLastTransition::SetFadeDurationHns(LONGLONG llDuration) throw() { m_llFadeDurationHns = llDuration; }

// ============================================================================
// SimpleEffect implementation
// ============================================================================

SimpleEffect::SimpleEffect()
    : m_dwEffectType(0)
    , m_llDurationHns(10000000)
    , m_flIntensity(1.0f)
{
}

SimpleEffect::~SimpleEffect() {}

DWORD SimpleEffect::GetEffectType() const throw() { return m_dwEffectType; }
void SimpleEffect::SetEffectType(DWORD dwType) throw() { m_dwEffectType = dwType; }

LONGLONG SimpleEffect::GetDurationHns() const throw() { return m_llDurationHns; }
void SimpleEffect::SetDurationHns(LONGLONG llDuration) throw() { m_llDurationHns = llDuration; }

float SimpleEffect::GetIntensity() const throw() { return m_flIntensity; }
void SimpleEffect::SetIntensity(float flIntensity) throw() { m_flIntensity = flIntensity; }

ATL::CString SimpleEffect::GetParameter(LPCWSTR pszKey) const
{
    if (!pszKey) return ATL::CString();
    for (size_t i = 0; i < m_arrParams.GetCount(); ++i)
    {
        if (m_arrParams.GetAt(i).strKey.CompareNoCase(pszKey) == 0)
            return m_arrParams.GetAt(i).strValue;
    }
    return ATL::CString();
}

void SimpleEffect::SetParameter(LPCWSTR pszKey, LPCWSTR pszValue)
{
    if (!pszKey) return;
    for (size_t i = 0; i < m_arrParams.GetCount(); ++i)
    {
        if (m_arrParams.GetAt(i).strKey.CompareNoCase(pszKey) == 0)
        {
            m_arrParams.GetAt(i).strValue = pszValue ? pszValue : L"";
            return;
        }
    }
    EffectParam param;
    param.strKey = pszKey;
    param.strValue = pszValue ? pszValue : L"";
    m_arrParams.Add(param);
}

} // namespace StoryboardManager
