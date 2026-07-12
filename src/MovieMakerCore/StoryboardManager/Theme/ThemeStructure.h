/*
 * ThemeStructure.h
 *
 * Theme structural elements for the StoryboardManager namespace.
 * Defines the hierarchical structure of a theme: ThemeComplexType,
 * sections (intro/mid/outro), titles, tracks, effects, transitions,
 * and the various element types that compose a theme.
 *
 * Note: ThemeEffect is defined in Templates.h and is reused here.
 *       ThemeFirstEffect/ThemeLastEffect wrap a ThemeEffect instance.
 *       ThemeTransition is defined here as it is theme-structure-specific.
 *
 * RTTI classes:
 *   ?AVThemeComplexType@@, ?AVThemeIntro@@, ?AVThemeMid@@, ?AVThemeOutro@@
 *   ?AVComplexIntro@@, ?AVSimpleIntro@@
 *   ?AVThemeTitle@@, ?AVThemeComplexTitle@@, ?AVThemeSimpleTitle@@
 *   ?AVThemeTrack@@, ?AVThemePrimaryTrack@@, ?AVThemeDependentTrack@@
 *   ?AVThemeSimpleElement@@, ?AVThemeInterior@@
 *   ?AVThemeTransition@@, ?AVThemeEffect@@, ?AVThemeEffectTemplate@@
 *   ?AVThemeFirstEffect@@, ?AVThemeLastEffect@@
 *   ?AVThemeFirstTransition@@, ?AVThemeLastTransition@@
 *   ?AVSimpleEffect@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef THEMESTRUCTURE_H
#define THEMESTRUCTURE_H

#include "../StoryboardManager.h"
#include "ThemeTemplates.h"
#include "../Templates.h"

namespace StoryboardManager
{

// ============================================================================
// Theme element type enumeration
// ============================================================================
enum ThemeElementType
{
    ThemeElementTypeUnknown         = 0,
    ThemeElementTypeIntro           = 1,
    ThemeElementTypeMid             = 2,
    ThemeElementTypeOutro           = 3,
    ThemeElementTypeTitle           = 4,
    ThemeElementTypeTrack           = 5,
    ThemeElementTypeTransition      = 6,
    ThemeElementTypeEffect          = 7,
    ThemeElementTypeSimpleElement   = 8,
    ThemeElementTypeInterior        = 9
};

// ============================================================================
// ThemeComplexType
// ============================================================================
// Base class for complex theme structural elements. Provides the common
// interface for elements that can contain child elements, have duration,
// and participate in the theme rendering pipeline.
//
class STORYBOARD_API ThemeComplexType
{
public:
    ThemeComplexType();
    virtual ~ThemeComplexType();

    // Identity
    ATL::CString GetId() const;
    void SetId(LPCWSTR pszId);

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    // Element type
    virtual ThemeElementType GetElementType() const throw();

    // Duration (hundred-nanoseconds)
    LONGLONG GetDurationHns() const throw();
    void SetDurationHns(LONGLONG llDuration) throw();

    // Delay before this element starts (hundred-nanoseconds)
    LONGLONG GetDelayHns() const throw();
    void SetDelayHns(LONGLONG llDelay) throw();

    // Children
    size_t GetChildCount() const throw();
    ThemeComplexType* GetChild(size_t nIndex);
    const ThemeComplexType* GetChild(size_t nIndex) const;
    size_t AddChild(ThemeComplexType* pChild);
    void RemoveChild(size_t nIndex);
    void RemoveAllChildren();

    // Parent
    ThemeComplexType* GetParent();
    const ThemeComplexType* GetParent() const;

    // X3D template reference
    BaseX3DTemplate* GetX3dTemplate();
    const BaseX3DTemplate* GetX3dTemplate() const;
    void SetX3dTemplate(BaseX3DTemplate* pTemplate);

    // Serialization
    virtual HRESULT LoadFromXml(IXmlReader* pReader);
    virtual HRESULT SaveToXml(IXmlWriter* pWriter);

protected:
    ATL::CString        m_strId;
    ATL::CString        m_strName;
    LONGLONG            m_llDurationHns;
    LONGLONG            m_llDelayHns;
    ThemeComplexType*   m_pParent;
    BaseX3DTemplate*    m_pX3dTemplate;

    ATL::CAtlArray<ThemeComplexType*> m_arrChildren;
};

// ============================================================================
// ThemeIntro
// ============================================================================
// Intro section of a theme. Defines the opening animation/title sequence
// that plays before the main content.
//
class STORYBOARD_API ThemeIntro : public ThemeComplexType
{
public:
    ThemeIntro();
    virtual ~ThemeIntro();

    virtual ThemeElementType GetElementType() const throw() override;

    // Title overlay
    ThemeTitle* GetTitle();
    const ThemeTitle* GetTitle() const;
    void SetTitle(ThemeTitle* pTitle);

    // Duration mode
    enum IntroDurationMode
    {
        IntroDurationModeFixed      = 0,
        IntroDurationModeAutoSize   = 1,
        IntroDurationModeFitToTitle = 2
    };

    IntroDurationMode GetDurationMode() const throw();
    void SetDurationMode(IntroDurationMode mode) throw();

    // Background
    DWORD GetBackgroundColor() const throw();
    void SetBackgroundColor(DWORD dwColor) throw();

    // Fade in duration
    LONGLONG GetFadeInDurationHns() const throw();
    void SetFadeInDurationHns(LONGLONG llDuration) throw();

    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    ThemeTitle*         m_pTitle;
    IntroDurationMode   m_durationMode;
    DWORD               m_dwBackgroundColor;
    LONGLONG            m_llFadeInDurationHns;
};

// ============================================================================
// ThemeMid
// ============================================================================
// Middle section of a theme. Defines the repeating content area where
// media items are displayed with effects and transitions.
//
class STORYBOARD_API ThemeMid : public ThemeComplexType
{
public:
    ThemeMid();
    virtual ~ThemeMid();

    virtual ThemeElementType GetElementType() const throw() override;

    // Track management
    size_t GetTrackCount() const throw();
    ThemeTrack* GetTrack(size_t nIndex);
    const ThemeTrack* GetTrack(size_t nIndex) const;
    size_t AddTrack(ThemeTrack* pTrack);
    void RemoveTrack(size_t nIndex);
    void RemoveAllTracks();

    // Default effect template
    ThemeEffectTemplate* GetDefaultEffectTemplate();
    const ThemeEffectTemplate* GetDefaultEffectTemplate() const;
    void SetDefaultEffectTemplate(ThemeEffectTemplate* pTemplate);

    // Default transition
    ThemeTransition* GetDefaultTransition();
    const ThemeTransition* GetDefaultTransition() const;
    void SetDefaultTransition(ThemeTransition* pTransition);

    // Loop flag (mid section repeats for all extents)
    bool IsLooping() const throw();
    void SetLooping(bool fLooping) throw();

    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    ATL::CAtlArray<ThemeTrack*>     m_arrTracks;
    ThemeEffectTemplate*            m_pDefaultEffectTemplate;
    ThemeTransition*                m_pDefaultTransition;
    bool                            m_fLooping;
};

// ============================================================================
// ThemeOutro
// ============================================================================
// Outro section of a theme. Defines the closing credits/animation sequence
// that plays after the main content.
//
class STORYBOARD_API ThemeOutro : public ThemeComplexType
{
public:
    ThemeOutro();
    virtual ~ThemeOutro();

    virtual ThemeElementType GetElementType() const throw() override;

    // Credits title
    ThemeTitle* GetCreditsTitle();
    const ThemeTitle* GetCreditsTitle() const;
    void SetCreditsTitle(ThemeTitle* pTitle);

    // Duration mode
    enum OutroDurationMode
    {
        OutroDurationModeFixed       = 0,
        OutroDurationModeAutoSize    = 1,
        OutroDurationModeScrollText  = 2
    };

    OutroDurationMode GetDurationMode() const throw();
    void SetDurationMode(OutroDurationMode mode) throw();

    // Background
    DWORD GetBackgroundColor() const throw();
    void SetBackgroundColor(DWORD dwColor) throw();

    // Fade out duration
    LONGLONG GetFadeOutDurationHns() const throw();
    void SetFadeOutDurationHns(LONGLONG llDuration) throw();

    // Scroll speed (pixels per second for scrolling credits)
    float GetScrollSpeed() const throw();
    void SetScrollSpeed(float flSpeed) throw();

    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    ThemeTitle*         m_pCreditsTitle;
    OutroDurationMode   m_durationMode;
    DWORD               m_dwBackgroundColor;
    LONGLONG            m_llFadeOutDurationHns;
    float               m_flScrollSpeed;
};

// ============================================================================
// ComplexIntro / SimpleIntro
// ============================================================================
// ComplexIntro: multi-element intro with layered animations and effects.
// SimpleIntro: single-element intro with basic title overlay.
//
class STORYBOARD_API ComplexIntro : public ThemeIntro
{
public:
    ComplexIntro();
    virtual ~ComplexIntro();

    // Layered elements
    size_t GetLayerCount() const throw();
    ThemeComplexType* GetLayer(size_t nIndex);
    const ThemeComplexType* GetLayer(size_t nIndex) const;
    size_t AddLayer(ThemeComplexType* pLayer);
    void RemoveLayer(size_t nIndex);

    // Entrance effects (ThemeEffect from Templates.h)
    size_t GetEntranceEffectCount() const throw();
    ThemeEffect* GetEntranceEffect(size_t nIndex);
    void AddEntranceEffect(ThemeEffect* pEffect);
    void RemoveAllEntranceEffects();

    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    ATL::CAtlArray<ThemeComplexType*> m_arrLayers;
    ATL::CAtlArray<ThemeEffect*>      m_arrEntranceEffects;
};

class STORYBOARD_API SimpleIntro : public ThemeIntro
{
public:
    SimpleIntro();
    virtual ~SimpleIntro();

    // Single overlay text
    ATL::CString GetOverlayText() const;
    void SetOverlayText(LPCWSTR pszText);

    // Overlay position
    float GetOverlayPositionX() const throw();
    float GetOverlayPositionY() const throw();
    void SetOverlayPosition(float x, float y) throw();

    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    ATL::CString    m_strOverlayText;
    float           m_flOverlayPositionX;
    float           m_flOverlayPositionY;
};

// ============================================================================
// ThemeTitle
// ============================================================================
// Defines a text title element within a theme. Handles font, color,
// position, and animation properties for rendered text.
//
class STORYBOARD_API ThemeTitle
{
public:
    ThemeTitle();
    ~ThemeTitle();

    // Text content
    ATL::CString GetText() const;
    void SetText(LPCWSTR pszText);

    // Font
    ATL::CString GetFontFamily() const;
    void SetFontFamily(LPCWSTR pszFontFamily);

    float GetFontSize() const throw();
    void SetFontSize(float flSize) throw();

    DWORD GetFontColor() const throw();
    void SetFontColor(DWORD dwColor) throw();

    DWORD GetFontStyle() const throw();  // bold, italic, etc.
    void SetFontStyle(DWORD dwStyle) throw();

    // Position (normalized 0.0-1.0)
    float GetPositionX() const throw();
    float GetPositionY() const throw();
    void SetPosition(float x, float y) throw();

    // Alignment
    enum TextAlignment
    {
        TextAlignmentLeft     = 0,
        TextAlignmentCenter   = 1,
        TextAlignmentRight    = 2
    };

    TextAlignment GetAlignment() const throw();
    void SetAlignment(TextAlignment align) throw();

    // Animation
    TitleAnimationType GetAnimation() const throw();
    void SetAnimation(TitleAnimationType type) throw();

    // Shadow
    bool HasShadow() const throw();
    void SetShadow(bool fShadow) throw();
    DWORD GetShadowColor() const throw();
    void SetShadowColor(DWORD dwColor) throw();

    // Outline
    bool HasOutline() const throw();
    void SetOutline(bool fOutline) throw();
    DWORD GetOutlineColor() const throw();
    void SetOutlineColor(DWORD dwColor) throw();
    float GetOutlineWidth() const throw();
    void SetOutlineWidth(float flWidth) throw();

    // Background
    bool HasBackground() const throw();
    void SetBackground(bool fBackground) throw();
    DWORD GetBackgroundColor() const throw();
    void SetBackgroundColor(DWORD dwColor) throw();
    float GetBackgroundOpacity() const throw();
    void SetBackgroundOpacity(float flOpacity) throw();

    // Max width for text wrapping (normalized, 0 = no wrap)
    float GetMaxWidth() const throw();
    void SetMaxWidth(float flMaxWidth) throw();

private:
    ATL::CString        m_strText;
    ATL::CString        m_strFontFamily;
    float               m_flFontSize;
    DWORD               m_dwFontColor;
    DWORD               m_dwFontStyle;
    float               m_flPositionX;
    float               m_flPositionY;
    TextAlignment       m_alignment;
    TitleAnimationType  m_animation;
    bool                m_fShadow;
    DWORD               m_dwShadowColor;
    bool                m_fOutline;
    DWORD               m_dwOutlineColor;
    float               m_flOutlineWidth;
    bool                m_fBackground;
    DWORD               m_dwBackgroundColor;
    float               m_flBackgroundOpacity;
    float               m_flMaxWidth;
};

// ============================================================================
// ThemeComplexTitle / ThemeSimpleTitle
// ============================================================================
// ThemeComplexTitle: multi-line title with per-line formatting.
// ThemeSimpleTitle: single-line title with uniform formatting.
//
class STORYBOARD_API ThemeComplexTitle : public ThemeTitle
{
public:
    ThemeComplexTitle();
    virtual ~ThemeComplexTitle();

    // Per-line overrides
    struct TitleLine
    {
        ATL::CString strText;
        ATL::CString strFontFamily;
        float        flFontSize;
        DWORD        dwFontColor;
        DWORD        dwFontStyle;
    };

    size_t GetLineCount() const throw();
    const TitleLine* GetLine(size_t nIndex) const;
    size_t AddLine(const TitleLine& line);
    void RemoveLine(size_t nIndex);
    void RemoveAllLines();

    void SetLineText(size_t nIndex, LPCWSTR pszText);
    void SetLineFont(size_t nIndex, LPCWSTR pszFontFamily, float flSize, DWORD dwColor);

private:
    ATL::CAtlArray<TitleLine> m_arrLines;
};

class STORYBOARD_API ThemeSimpleTitle : public ThemeTitle
{
public:
    ThemeSimpleTitle();
    virtual ~ThemeSimpleTitle();

    // Auto-fit text to screen
    bool IsAutoFitEnabled() const throw();
    void SetAutoFitEnabled(bool fEnabled) throw();

    // Minimum/maximum font size for auto-fit
    float GetMinFontSize() const throw();
    void SetMinFontSize(float flSize) throw();

    float GetMaxFontSize() const throw();
    void SetMaxFontSize(float flSize) throw();

private:
    bool    m_fAutoFitEnabled;
    float   m_flMinFontSize;
    float   m_flMaxFontSize;
};

// ============================================================================
// ThemeTrack
// ============================================================================
// A track within a theme's mid section. Defines a sequence of elements
// (effects/transitions) that are applied to the corresponding timeline track.
//
class STORYBOARD_API ThemeTrack
{
public:
    ThemeTrack();
    virtual ~ThemeTrack();

    // Identity
    ATL::CString GetId() const;
    void SetId(LPCWSTR pszId);

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    // Track type
    enum ThemeTrackType
    {
        ThemeTrackTypePrimary     = 0,
        ThemeTrackTypeDependent   = 1
    };

    virtual ThemeTrackType GetTrackType() const throw();

    // Elements in this track
    size_t GetElementCount() const throw();
    ThemeComplexType* GetElement(size_t nIndex);
    const ThemeComplexType* GetElement(size_t nIndex) const;
    size_t AddElement(ThemeComplexType* pElement);
    void RemoveElement(size_t nIndex);
    void RemoveAllElements();

    // Duration
    LONGLONG GetDurationHns() const throw();
    void SetDurationHns(LONGLONG llDuration) throw();

private:
    ATL::CString    m_strId;
    ATL::CString    m_strName;
    LONGLONG        m_llDurationHns;
    ATL::CAtlArray<ThemeComplexType*> m_arrElements;
};

// ============================================================================
// ThemePrimaryTrack / ThemeDependentTrack
// ============================================================================
// ThemePrimaryTrack: main track that drives the timeline.
// ThemeDependentTrack: secondary track that adapts to the primary track.
//
class STORYBOARD_API ThemePrimaryTrack : public ThemeTrack
{
public:
    ThemePrimaryTrack();
    virtual ~ThemePrimaryTrack();

    virtual ThemeTrackType GetTrackType() const throw() override;

    // Media binding (which timeline track this maps to)
    TimelineTrackType GetBoundTrackType() const throw();
    void SetBoundTrackType(TimelineTrackType type) throw();

    // Auto-fill flag (automatically fills empty extents)
    bool IsAutoFill() const throw();
    void SetAutoFill(bool fAutoFill) throw();

private:
    TimelineTrackType   m_boundTrackType;
    bool                m_fAutoFill;
};

class STORYBOARD_API ThemeDependentTrack : public ThemeTrack
{
public:
    ThemeDependentTrack();
    virtual ~ThemeDependentTrack();

    virtual ThemeTrackType GetTrackType() const throw() override;

    // Dependency: which primary track this depends on
    ATL::CString GetDependsOnTrackId() const;
    void SetDependsOnTrackId(LPCWSTR pszTrackId);

    // Sync mode: how this track syncs with the primary
    enum SyncMode
    {
        SyncModeSyncStart     = 0,    // starts at same time
        SyncModeSyncEnd       = 1,    // ends at same time
        SyncModeSyncCenter    = 2     // centered on primary
    };

    SyncMode GetSyncMode() const throw();
    void SetSyncMode(SyncMode mode) throw();

private:
    ATL::CString    m_strDependsOnTrackId;
    SyncMode        m_syncMode;
};

// ============================================================================
// ThemeSimpleElement
// ============================================================================
// A simple theme element that represents a single media slot with
// optional effect overlay. Used for straightforward theme layouts.
//
class STORYBOARD_API ThemeSimpleElement : public ThemeComplexType
{
public:
    ThemeSimpleElement();
    virtual ~ThemeSimpleElement();

    virtual ThemeElementType GetElementType() const throw() override;

    // Media socket
    ATL::CString GetMediaSocketId() const;
    void SetMediaSocketId(LPCWSTR pszId);

    // Effect overlay (ThemeEffect from Templates.h)
    ThemeEffect* GetEffectOverlay();
    const ThemeEffect* GetEffectOverlay() const;
    void SetEffectOverlay(ThemeEffect* pEffect);

    // Aspect ratio mode
    DWORD GetAspectRatioMode() const throw();
    void SetAspectRatioMode(DWORD dwMode) throw();

    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    ATL::CString    m_strMediaSocketId;
    ThemeEffect*    m_pEffectOverlay;
    DWORD           m_dwAspectRatioMode;
};

// ============================================================================
// ThemeInterior
// ============================================================================
// Interior element representing the body content area of a theme.
// Contains the primary media display area with optional decorations.
//
class STORYBOARD_API ThemeInterior : public ThemeComplexType
{
public:
    ThemeInterior();
    virtual ~ThemeInterior();

    virtual ThemeElementType GetElementType() const throw() override;

    // Background model/texture
    ATL::CString GetBackgroundModelPath() const;
    void SetBackgroundModelPath(LPCWSTR pszPath);

    // Decoration elements
    size_t GetDecorationCount() const throw();
    ThemeSimpleElement* GetDecoration(size_t nIndex);
    void AddDecoration(ThemeSimpleElement* pDecoration);
    void RemoveAllDecorations();

    // Media area bounds (normalized)
    float GetMediaBoundsLeft() const throw();
    float GetMediaBoundsTop() const throw();
    float GetMediaBoundsRight() const throw();
    float GetMediaBoundsBottom() const throw();
    void SetMediaBounds(float left, float top, float right, float bottom) throw();

    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    ATL::CString    m_strBackgroundModelPath;
    ATL::CAtlArray<ThemeSimpleElement*> m_arrDecorations;
    float           m_mediaBoundsLeft;
    float           m_mediaBoundsTop;
    float           m_mediaBoundsRight;
    float           m_mediaBoundsBottom;
};

// ============================================================================
// ThemeTransition
// ============================================================================
// Defines a transition between extents within a theme. Specifies the
// X3D transition clip, duration, and parameters.
//
class STORYBOARD_API ThemeTransition
{
public:
    ThemeTransition();
    ~ThemeTransition();

    // Identity
    ATL::CString GetId() const;
    void SetId(LPCWSTR pszId);

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    // Transition clip name (X3D clip)
    ATL::CString GetClipName() const;
    void SetClipName(LPCWSTR pszClipName);

    // Duration
    LONGLONG GetDurationHns() const throw();
    void SetDurationHns(LONGLONG llDuration) throw();

    // Parameters
    size_t GetParameterCount() const throw();
    TemplateProperty* GetParameter(size_t nIndex);
    TemplateProperty* FindParameter(LPCWSTR pszKey);
    size_t AddParameter(const TemplateProperty& param);
    void RemoveAllParameters();

    // X3D template
    ThemeX3DTemplate* GetX3dTemplate();
    void SetX3dTemplate(ThemeX3DTemplate* pTemplate);

    // Serialization
    HRESULT LoadFromXml(IXmlReader* pReader);
    HRESULT SaveToXml(IXmlWriter* pWriter);

private:
    ATL::CString                m_strId;
    ATL::CString                m_strName;
    ATL::CString                m_strClipName;
    LONGLONG                    m_llDurationHns;
    ATL::CAtlArray<TemplateProperty> m_arrParameters;
    ThemeX3DTemplate*           m_pX3dTemplate;
};

// ============================================================================
// ThemeEffectTemplate
// ============================================================================
// Template for effects within a theme. Defines the default effect
// configuration that is applied to extents unless overridden.
// Uses ThemeEffect (from Templates.h) for individual effect definitions.
//
class STORYBOARD_API ThemeEffectTemplate
{
public:
    ThemeEffectTemplate();
    ~ThemeEffectTemplate();

    // Identity
    ATL::CString GetId() const;
    void SetId(LPCWSTR pszId);

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    // Effect definitions (ThemeEffect from Templates.h)
    size_t GetEffectCount() const throw();
    ThemeEffect* GetEffect(size_t nIndex);
    const ThemeEffect* GetEffect(size_t nIndex) const;
    size_t AddEffect(ThemeEffect* pEffect);
    void RemoveEffect(size_t nIndex);
    void RemoveAllEffects();

    // Default intensity
    float GetDefaultIntensity() const throw();
    void SetDefaultIntensity(float flIntensity) throw();

    // Serialization
    HRESULT LoadFromXml(IXmlReader* pReader);
    HRESULT SaveToXml(IXmlWriter* pWriter);

private:
    ATL::CString    m_strId;
    ATL::CString    m_strName;
    ATL::CAtlArray<ThemeEffect*> m_arrEffects;
    float           m_flDefaultIntensity;
};

// ============================================================================
// ThemeFirstEffect / ThemeLastEffect
// ============================================================================
// Sentinel effects marking the first and last effect slots in a track.
// Used by the theme engine to handle edge cases at track boundaries.
// These wrap a ThemeEffect (from Templates.h) with additional metadata.
//
class STORYBOARD_API ThemeFirstEffect
{
public:
    ThemeFirstEffect();
    ~ThemeFirstEffect();

    // The wrapped effect
    ThemeEffect* GetEffect();
    const ThemeEffect* GetEffect() const;
    void SetEffect(ThemeEffect* pEffect);

    // Entry animation override
    ATL::CString GetEntryAnimation() const;
    void SetEntryAnimation(LPCWSTR pszAnimation);

    // Delay before first effect starts
    LONGLONG GetEntryDelayHns() const throw();
    void SetEntryDelayHns(LONGLONG llDelay) throw();

private:
    ThemeEffect*    m_pEffect;
    ATL::CString    m_strEntryAnimation;
    LONGLONG        m_llEntryDelayHns;
};

class STORYBOARD_API ThemeLastEffect
{
public:
    ThemeLastEffect();
    ~ThemeLastEffect();

    // The wrapped effect
    ThemeEffect* GetEffect();
    const ThemeEffect* GetEffect() const;
    void SetEffect(ThemeEffect* pEffect);

    // Exit animation override
    ATL::CString GetExitAnimation() const;
    void SetExitAnimation(LPCWSTR pszAnimation);

    // Hold duration after last effect
    LONGLONG GetExitHoldHns() const throw();
    void SetExitHoldHns(LONGLONG llHold) throw();

private:
    ThemeEffect*    m_pEffect;
    ATL::CString    m_strExitAnimation;
    LONGLONG        m_llExitHoldHns;
};

// ============================================================================
// ThemeFirstTransition / ThemeLastTransition
// ============================================================================
// Sentinel transitions marking the first and last transition slots.
// Handle the entry/exit transitions at the boundaries of the timeline.
//
class STORYBOARD_API ThemeFirstTransition
{
public:
    ThemeFirstTransition();
    ~ThemeFirstTransition();

    // The wrapped transition
    ThemeTransition* GetTransition();
    const ThemeTransition* GetTransition() const;
    void SetTransition(ThemeTransition* pTransition);

    // Fade-in from black
    bool IsFadeFromBlack() const throw();
    void SetFadeFromBlack(bool fFade) throw();

    // Fade duration
    LONGLONG GetFadeDurationHns() const throw();
    void SetFadeDurationHns(LONGLONG llDuration) throw();

private:
    ThemeTransition*    m_pTransition;
    bool                m_fFadeFromBlack;
    LONGLONG            m_llFadeDurationHns;
};

class STORYBOARD_API ThemeLastTransition
{
public:
    ThemeLastTransition();
    ~ThemeLastTransition();

    // The wrapped transition
    ThemeTransition* GetTransition();
    const ThemeTransition* GetTransition() const;
    void SetTransition(ThemeTransition* pTransition);

    // Fade-out to black
    bool IsFadeToBlack() const throw();
    void SetFadeToBlack(bool fFade) throw();

    // Fade duration
    LONGLONG GetFadeDurationHns() const throw();
    void SetFadeDurationHns(LONGLONG llDuration) throw();

private:
    ThemeTransition*    m_pTransition;
    bool                m_fFadeToBlack;
    LONGLONG            m_llFadeDurationHns;
};

// ============================================================================
// SimpleEffect
// ============================================================================
// Lightweight effect element used within ThemeSimpleElement. Defines
// a single visual effect without the full ThemeEffect overhead.
//
class STORYBOARD_API SimpleEffect
{
public:
    SimpleEffect();
    ~SimpleEffect();

    // Effect type
    DWORD GetEffectType() const throw();
    void SetEffectType(DWORD dwType) throw();

    // Duration
    LONGLONG GetDurationHns() const throw();
    void SetDurationHns(LONGLONG llDuration) throw();

    // Intensity
    float GetIntensity() const throw();
    void SetIntensity(float flIntensity) throw();

    // Parameter
    ATL::CString GetParameter(LPCWSTR pszKey) const;
    void SetParameter(LPCWSTR pszKey, LPCWSTR pszValue);

private:
    DWORD       m_dwEffectType;
    LONGLONG    m_llDurationHns;
    float       m_flIntensity;

    struct EffectParam
    {
        ATL::CString strKey;
        ATL::CString strValue;
    };
    ATL::CAtlArray<EffectParam> m_arrParams;
};

} // namespace StoryboardManager

#endif // THEMESTRUCTURE_H
