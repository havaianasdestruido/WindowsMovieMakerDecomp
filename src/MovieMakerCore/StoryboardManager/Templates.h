/*
 * Templates.h
 *
 * Template management classes for the StoryboardManager namespace:
 *   - TemplateTable: registry of available AutoMovie templates
 *   - TemplateEntry: a single template definition
 *   - TimelineTemplateSource: provides template data for timeline generation
 *   - AutoMovieTheme: theme configuration for AutoMovie
 *   - ThemeEffect: individual effect within a theme
 *
 * RTTI classes from analysis:
 *   - TemplateTable
 *   - TimelineTemplateSource
 *
 * AutoMovie themes include: Contemporary, Fade, Fly in, Pan and zoom,
 * Magazine, Retro, Contemporary (photo), etc.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef TEMPLATES_H
#define TEMPLATES_H

#include "StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// Theme effect type enumeration
// ============================================================================
enum ThemeEffectType
{
    ThemeEffectTypeNone         = 0,
    ThemeEffectTypeTransition   = 1,  // transition between extents
    ThemeEffectTypePanZoom      = 2,  // Ken Burns pan/zoom effect
    ThemeEffectTypeFade         = 3,  // fade in/out
    ThemeEffectTypeBlur         = 4,  // blur overlay
    ThemeEffectTypeSepia        = 5,  // sepia tone
    ThemeEffectTypeGrayscale    = 6,  // black and white
    ThemeEffectTypeFilmGrain    = 7,  // film grain overlay
    ThemeEffectTypeVignette     = 8,  // vignette edge darkening
    ThemeEffectTypeLensFlare    = 9,  // lens flare overlay
    ThemeEffectTypeColorPop     = 10, // selective color
    ThemeEffectTypeBounce       = 11, // bounce entrance
    ThemeEffectTypeSpin         = 12, // spin entrance
    ThemeEffectTypeZoom         = 13, // zoom entrance
    ThemeEffectTypeFlyIn        = 14, // fly-in entrance
    ThemeEffectTypeShrink       = 15, // shrink exit
    ThemeEffectTypeWipeLeft     = 16, // wipe left transition
    ThemeEffectTypeWipeRight    = 17, // wipe right transition
    ThemeEffectTypeWipeUp       = 18, // wipe up transition
    ThemeEffectTypeWipeDown     = 19, // wipe down transition
    ThemeEffectTypeCrossfade    = 20, // crossfade transition
    ThemeEffectTypeDissolve     = 21, // dissolve transition
    ThemeEffectTypePush         = 22, // push transition
    ThemeEffectTypeSlide        = 23, // slide transition
    ThemeEffectTypeReveal       = 24  // reveal transition
};

// ============================================================================
// Title animation type
// ============================================================================
enum TitleAnimationType
{
    TitleAnimationNone          = 0,
    TitleAnimationFadeIn        = 1,
    TitleAnimationFlyInLeft     = 2,
    TitleAnimationFlyInRight    = 3,
    TitleAnimationFlyInTop      = 4,
    TitleAnimationFlyInBottom   = 5,
    TitleAnimationZoomIn        = 6,
    TitleAnimationZoomOut       = 7,
    TitleAnimationTypewriter    = 8,
    TitleAnimationScrollUp      = 9,
    TitleAnimationScrollDown    = 10,
    TitleAnimationBounce        = 11,
    TitleAnimationSpin          = 12
};

// ============================================================================
// ThemeEffect
// ============================================================================
// A single effect within a theme. Defines the effect type, duration, and
// any parameters needed for rendering.
//
class STORYBOARD_API ThemeEffect
{
public:
    ThemeEffect();
    ~ThemeEffect();
    ThemeEffect(const ThemeEffect& other);
    ThemeEffect& operator=(const ThemeEffect& other);

    // Effect type
    ThemeEffectType GetType() const throw();
    void SetType(ThemeEffectType type) throw();

    // Duration (hundred-nanoseconds)
    LONGLONG GetDurationHns() const throw();
    void SetDurationHns(LONGLONG llDuration) throw();

    // Effect parameters (key-value pairs)
    ATL::CString GetParameter(LPCWSTR pszKey) const;
    void SetParameter(LPCWSTR pszKey, LPCWSTR pszValue);

    // Intensity (0.0 - 1.0)
    double GetIntensity() const throw();
    void SetIntensity(double dblIntensity) throw();

    // Delay before effect starts (hundred-nanoseconds)
    LONGLONG GetDelayHns() const throw();
    void SetDelayHns(LONGLONG llDelay) throw();

    // X3D clip name for transitions
    ATL::CString GetX3dClipName() const;
    void SetX3dClipName(LPCWSTR pszClipName);

private:
    ThemeEffectType     m_type;
    LONGLONG            m_llDurationHns;
    LONGLONG            m_llDelayHns;
    double              m_dblIntensity;
    ATL::CString        m_strX3dClipName;

    // Simple parameter storage
    struct Parameter
    {
        ATL::CString strKey;
        ATL::CString strValue;
    };
    ATL::CAtlArray<Parameter> m_arrParameters;
};

// ============================================================================
// AutoMovieTheme
// ============================================================================
// Defines a complete theme for AutoMovie. Contains a set of effects,
// transitions, and title animations that are applied automatically when
// the user uses the "AutoMovie" feature.
//
class STORYBOARD_API AutoMovieTheme
{
public:
    AutoMovieTheme();
    ~AutoMovieTheme();

    // Theme identity
    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    ATL::CString GetDisplayName() const;
    void SetDisplayName(LPCWSTR pszDisplayName);

    ATL::CString GetDescription() const;
    void SetDescription(LPCWSTR pszDescription);

    // Thumbnail path for theme picker UI
    ATL::CString GetThumbnailPath() const;
    void SetThumbnailPath(LPCWSTR pszPath);

    // Effects in this theme
    size_t GetEffectCount() const throw();
    ThemeEffect* GetEffect(size_t nIndex);
    const ThemeEffect* GetEffect(size_t nIndex) const;
    size_t AddEffect(const ThemeEffect& effect);
    void RemoveEffect(size_t nIndex);
    void RemoveAllEffects();

    // Default transition type for this theme
    ThemeEffectType GetDefaultTransitionType() const throw();
    void SetDefaultTransitionType(ThemeEffectType type) throw();

    // Default transition duration (hundred-nanoseconds)
    LONGLONG GetDefaultTransitionDurationHns() const throw();
    void SetDefaultTransitionDurationHns(LONGLONG llDuration) throw();

    // Default title animation
    TitleAnimationType GetDefaultTitleAnimation() const throw();
    void SetDefaultTitleAnimation(TitleAnimationType type) throw();

    // Title font
    ATL::CString GetTitleFontFamily() const;
    void SetTitleFontFamily(LPCWSTR pszFontFamily);

    DWORD GetTitleFontColor() const throw();
    void SetTitleFontColor(DWORD dwColor) throw();

    // Title position (0.0-1.0 normalized coordinates)
    double GetTitlePositionX() const throw();
    double GetTitlePositionY() const throw();
    void SetTitlePosition(double dblX, double dblY) throw();

    // Whether this theme is built-in
    bool IsBuiltIn() const throw();
    void SetBuiltIn(bool fBuiltIn) throw();

    // Theme category (for grouping in UI)
    ATL::CString GetCategory() const;
    void SetCategory(LPCWSTR pszCategory);

private:
    ATL::CString    m_strName;
    ATL::CString    m_strDisplayName;
    ATL::CString    m_strDescription;
    ATL::CString    m_strThumbnailPath;
    ATL::CString    m_strCategory;
    ATL::CString    m_strTitleFontFamily;

    ATL::CAtlArray<ThemeEffect> m_arrEffects;

    ThemeEffectType     m_defaultTransitionType;
    LONGLONG            m_llDefaultTransitionDurationHns;
    TitleAnimationType  m_defaultTitleAnimation;
    DWORD               m_dwTitleFontColor;
    double              m_dblTitlePositionX;
    double              m_dblTitlePositionY;
    bool                m_fBuiltIn;
};

// ============================================================================
// TemplateEntry
// ============================================================================
// A single template definition within the template table. Combines a theme
// with additional metadata for the template picker UI.
//
class STORYBOARD_API TemplateEntry
{
public:
    TemplateEntry();
    ~TemplateEntry();

    // Template identity
    DWORD GetTemplateId() const throw();
    void SetTemplateId(DWORD dwId) throw();

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    ATL::CString GetDisplayName() const;
    void SetDisplayName(LPCWSTR pszDisplayName);

    ATL::CString GetDescription() const;
    void SetDescription(LPCWSTR pszDescription);

    // Associated theme
    AutoMovieTheme* GetTheme();
    const AutoMovieTheme* GetTheme() const;
    void SetTheme(AutoMovieTheme* pTheme);

    // Thumbnail
    ATL::CString GetThumbnailPath() const;
    void SetThumbnailPath(LPCWSTR pszPath);

    // Recommended media count (min/max)
    UINT GetMinMediaCount() const throw();
    UINT GetMaxMediaCount() const throw();
    void SetMediaCountRange(UINT uMin, UINT uMax) throw();

    // Supported media types (bitmask: 0x01=video, 0x02=photo, 0x04=audio)
    DWORD GetSupportedMediaTypes() const throw();
    void SetSupportedMediaTypes(DWORD dwMask) throw();

    // Sort order in UI
    int GetSortOrder() const throw();
    void SetSortOrder(int nOrder) throw();

private:
    DWORD               m_dwTemplateId;
    ATL::CString        m_strName;
    ATL::CString        m_strDisplayName;
    ATL::CString        m_strDescription;
    ATL::CString        m_strThumbnailPath;
    AutoMovieTheme*     m_pTheme;
    UINT                m_uMinMediaCount;
    UINT                m_uMaxMediaCount;
    DWORD               m_dwSupportedMediaTypes;
    int                 m_nSortOrder;
};

// ============================================================================
// TemplateTable
// ============================================================================
// Registry of all available templates. Provides lookup by ID or name,
// enumeration for the template picker UI, and management of built-in
// and custom templates.
//
// From RTTI: TemplateTable
//
class STORYBOARD_API TemplateTable
{
public:
    TemplateTable();
    ~TemplateTable();

    // Initialize with built-in templates
    HRESULT InitializeDefaults();

    // Template access
    size_t GetTemplateCount() const throw();
    TemplateEntry* GetTemplateAt(size_t nIndex);
    const TemplateEntry* GetTemplateAt(size_t nIndex) const;

    // Lookup
    TemplateEntry* FindTemplate(DWORD dwId);
    TemplateEntry* FindTemplateByName(LPCWSTR pszName);
    const TemplateEntry* FindTemplate(DWORD dwId) const;
    const TemplateEntry* FindTemplateByName(LPCWSTR pszName) const;

    // Add/remove
    size_t AddTemplate(TemplateEntry* pEntry);
    void RemoveTemplate(DWORD dwId);
    void RemoveAllTemplates();

    // Default template
    DWORD GetDefaultTemplateId() const throw();
    void SetDefaultTemplateId(DWORD dwId) throw();
    TemplateEntry* GetDefaultTemplate();

    // Theme access (shared ownership)
    size_t GetThemeCount() const throw();
    AutoMovieTheme* GetThemeAt(size_t nIndex);
    AutoMovieTheme* FindTheme(LPCWSTR pszName);
    void AddTheme(AutoMovieTheme* pTheme);
    void RemoveTheme(LPCWSTR pszName);

    // Recommendations based on media count
    TemplateEntry* RecommendTemplate(UINT uMediaCount, DWORD dwMediaType);

private:
    // Create built-in themes
    void CreateContemporaryTheme();
    void CreateFadeTheme();
    void CreateFlyInTheme();
    void CreatePanAndZoomTheme();
    void CreateMagazineTheme();
    void CreateRetroTheme();
    void CreateContemporaryPhotoTheme();

    ATL::CAtlArray<TemplateEntry*>  m_arrTemplates;
    ATL::CAtlArray<AutoMovieTheme*> m_arrThemes;
    DWORD                           m_dwDefaultTemplateId;
    DWORD                           m_dwNextTemplateId;
};

} // namespace StoryboardManager

#endif // TEMPLATES_H
