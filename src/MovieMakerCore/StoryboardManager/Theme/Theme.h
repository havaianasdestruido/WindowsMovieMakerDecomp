/*
 * Theme.h
 *
 * Top-level theme container and management for the StoryboardManager
 * namespace. ThemeManager loads/manages themes from the local app data
 * directory. ThemeProject represents a complete project theme definition.
 * Theme is the top-level theme with intro/mid/outro sections.
 *
 * RTTI classes:
 *   ?AVThemeManager@@, ?AVThemeProject@@, ?AVTheme@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef THEME_H
#define THEME_H

#include "../StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// Theme constants
// ============================================================================
static const LPCWSTR kThemeFileExtension     = L".wlmt";
static const LPCWSTR kThemeRootElement       = L"theme";
static const LPCWSTR kThemeVersionAttr       = L"version";
static const DWORD   kThemeVersionMajor      = 1;
static const DWORD   kThemeVersionMinor      = 0;

static const DWORD   kThemeMaxNameLength     = 256;
static const DWORD   kThemeMaxDescLength     = 1024;

// ============================================================================
// Theme file location: %LOCALAPPDATA%\Microsoft\WL\MovieMaker\Themes\
// ============================================================================
static const LPCWSTR kThemeBasePath          = L"\\Microsoft\\WL\\MovieMaker\\Themes";

// ============================================================================
// ThemeRenderMode
// ============================================================================
enum ThemeRenderMode
{
    ThemeRenderModeNone         = 0,
    ThemeRenderModeFull         = 1,    // full rendering with all effects
    ThemeRenderModePreview      = 2,    // reduced quality for preview
    ThemeRenderModeThumbnail    = 3     // thumbnail generation only
};

// ============================================================================
// ThemeLoadState
// ============================================================================
enum ThemeLoadState
{
    ThemeLoadStateUnloaded      = 0,
    ThemeLoadStateLoading       = 1,
    ThemeLoadStateLoaded        = 2,
    ThemeLoadStateError         = 3
};

// ============================================================================
// Theme
// ============================================================================
// Top-level theme with intro/mid/outro sections. A Theme aggregates the
// structural elements (ThemeIntro, ThemeMid, ThemeOutro) and provides
// the rendering interface for applying theme effects to the timeline.
//
class STORYBOARD_API Theme
{
public:
    Theme();
    ~Theme();

    // Identity
    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    ATL::CString GetDisplayName() const;
    void SetDisplayName(LPCWSTR pszDisplayName);

    ATL::CString GetDescription() const;
    void SetDescription(LPCWSTR pszDescription);

    // Version
    DWORD GetVersionMajor() const throw();
    DWORD GetVersionMinor() const throw();
    void SetVersion(DWORD dwMajor, DWORD dwMinor) throw();

    // Thumbnail
    ATL::CString GetThumbnailPath() const;
    void SetThumbnailPath(LPCWSTR pszPath);

    // Load state
    ThemeLoadState GetLoadState() const throw();
    void SetLoadState(ThemeLoadState state) throw();

    // Sections (owned)
    class ThemeIntro* GetIntro();
    const class ThemeIntro* GetIntro() const;
    void SetIntro(class ThemeIntro* pIntro);

    class ThemeMid* GetMid();
    const class ThemeMid* GetMid() const;
    void SetMid(class ThemeMid* pMid);

    class ThemeOutro* GetOutro();
    const class ThemeOutro* GetOutro() const;
    void SetOutro(class ThemeOutro* pOutro);

    // Effect templates
    size_t GetEffectTemplateCount() const throw();
    class ThemeEffectTemplate* GetEffectTemplate(size_t nIndex);
    const class ThemeEffectTemplate* GetEffectTemplate(size_t nIndex) const;
    size_t AddEffectTemplate(class ThemeEffectTemplate* pTemplate);
    void RemoveEffectTemplate(size_t nIndex);
    void RemoveAllEffectTemplates();

    // Transition templates
    size_t GetTransitionTemplateCount() const throw();
    class ThemeTransition* GetTransitionTemplate(size_t nIndex);
    const class ThemeTransition* GetTransitionTemplate(size_t nIndex) const;
    size_t AddTransitionTemplate(class ThemeTransition* pTransition);
    void RemoveTransitionTemplate(size_t nIndex);
    void RemoveAllTransitionTemplates();

    // Rendering
    HRESULT BeginRender(ThemeRenderMode mode);
    HRESULT EndRender();
    ThemeRenderMode GetCurrentRenderMode() const throw();

    // Serialization
    HRESULT LoadFromFile(LPCWSTR pszFilePath);
    HRESULT SaveToFile(LPCWSTR pszFilePath);
    HRESULT LoadFromStream(IStream* pStream);
    HRESULT SaveToStream(IStream* pStream);
    HRESULT SaveToXml(IXmlWriter* pWriter);

    // Validation
    HRESULT Validate() const;

    // Built-in check
    bool IsBuiltIn() const throw();
    void SetBuiltIn(bool fBuiltIn) throw();

    // Category
    ATL::CString GetCategory() const;
    void SetCategory(LPCWSTR pszCategory);

    // Visual appearance
    DWORD GetPrimaryColor() const throw();
    void SetPrimaryColor(DWORD dwColor) throw();

    DWORD GetSecondaryColor() const throw();
    void SetSecondaryColor(DWORD dwColor) throw();

    DWORD GetAccentColor() const throw();
    void SetAccentColor(DWORD dwColor) throw();

    ATL::CString GetFontFamily() const;
    void SetFontFamily(LPCWSTR pszFontFamily);

    float GetFontSize() const throw();
    void SetFontSize(float flSize) throw();

    // Apply theme properties to a theme element
    HRESULT ApplyToElement(class ThemeComplexType* pElement);

    // Apply this theme's visual properties to a ThemeProject
    HRESULT Apply(class ThemeProject* pProject);

    // Clear this theme's influence from a ThemeProject (reset to defaults)
    HRESULT Remove(class ThemeProject* pProject);

private:
    ATL::CString        m_strName;
    ATL::CString        m_strDisplayName;
    ATL::CString        m_strDescription;
    ATL::CString        m_strThumbnailPath;
    ATL::CString        m_strCategory;
    ATL::CString        m_strFontFamily;
    DWORD               m_dwPrimaryColor;
    DWORD               m_dwSecondaryColor;
    DWORD               m_dwAccentColor;
    float               m_flFontSize;
    DWORD               m_dwVersionMajor;
    DWORD               m_dwVersionMinor;
    ThemeLoadState      m_loadState;
    ThemeRenderMode     m_renderMode;
    bool                m_fBuiltIn;

    class ThemeIntro*   m_pIntro;
    class ThemeMid*     m_pMid;
    class ThemeOutro*   m_pOutro;

    ATL::CAtlArray<class ThemeEffectTemplate*>    m_arrEffectTemplates;
    ATL::CAtlArray<class ThemeTransition*>         m_arrTransitionTemplates;
};

// ============================================================================
// ThemeProject
// ============================================================================
// Complete project theme definition. Combines a Theme with project-specific
// overrides and state. Owned by the project and provides the interface
// between the project timeline and the theme rendering system.
//
class STORYBOARD_API ThemeProject
{
public:
    ThemeProject();
    ~ThemeProject();

    // Theme association
    Theme* GetTheme();
    const Theme* GetTheme() const;
    void SetTheme(Theme* pTheme);

    // Project-specific overrides
    ATL::CString GetTitleText() const;
    void SetTitleText(LPCWSTR pszText);

    ATL::CString GetCreditsText() const;
    void SetCreditsText(LPCWSTR pszText);

    // Font overrides
    ATL::CString GetTitleFontFamily() const;
    void SetTitleFontFamily(LPCWSTR pszFontFamily);

    ATL::CString GetCreditsFontFamily() const;
    void SetCreditsFontFamily(LPCWSTR pszFontFamily);

    DWORD GetTitleFontColor() const throw();
    void SetTitleFontColor(DWORD dwColor) throw();

    DWORD GetCreditsFontColor() const throw();
    void SetCreditsFontColor(DWORD dwColor) throw();

    // Duration overrides (hundred-nanoseconds)
    LONGLONG GetTitleDurationHns() const throw();
    void SetTitleDurationHns(LONGLONG llDuration) throw();

    LONGLONG GetCreditsDurationHns() const throw();
    void SetCreditsDurationHns(LONGLONG llDuration) throw();

    // Transition duration override
    LONGLONG GetTransitionDurationHns() const throw();
    void SetTransitionDurationHns(LONGLONG llDuration) throw();

    // Background color
    DWORD GetBackgroundColor() const throw();
    void SetBackgroundColor(DWORD dwColor) throw();

    // Enable/disable sections
    bool IsIntroEnabled() const throw();
    void SetIntroEnabled(bool fEnabled) throw();

    bool IsOutroEnabled() const throw();
    void SetOutroEnabled(bool fEnabled) throw();

    // Text manager access
    class TextManager* GetTextManager();
    const class TextManager* GetTextManager() const;

    // Validation
    HRESULT Validate() const;

    // Reset to defaults (based on theme)
    HRESULT ResetToDefaults();

private:
    Theme*              m_pTheme;
    ATL::CString        m_strTitleText;
    ATL::CString        m_strCreditsText;
    ATL::CString        m_strTitleFontFamily;
    ATL::CString        m_strCreditsFontFamily;
    DWORD               m_dwTitleFontColor;
    DWORD               m_dwCreditsFontColor;
    LONGLONG            m_llTitleDurationHns;
    LONGLONG            m_llCreditsDurationHns;
    LONGLONG            m_llTransitionDurationHns;
    DWORD               m_dwBackgroundColor;
    bool                m_fIntroEnabled;
    bool                m_fOutroEnabled;

    class TextManager*  m_pTextManager;
};

// ============================================================================
// ThemeManager
// ============================================================================
// Loads and manages the collection of available themes. Scans the theme
// directory for installed themes and provides lookup/enumeration for the
// theme picker UI.
//
// Theme files are stored at:
//   %LOCALAPPDATA%\Microsoft\WL\MovieMaker\Themes\*.wlmt
//
class STORYBOARD_API ThemeManager
{
public:
    ThemeManager();
    ~ThemeManager();

    // Lifecycle
    HRESULT Initialize();
    HRESULT Shutdown();

    // Theme enumeration
    size_t GetThemeCount() const throw();
    Theme* GetThemeAt(size_t nIndex);
    const Theme* GetThemeAt(size_t nIndex) const;

    // Lookup
    Theme* FindTheme(LPCWSTR pszName);
    const Theme* FindTheme(LPCWSTR pszName) const;
    Theme* FindThemeByDisplayName(LPCWSTR pszDisplayName);
    int FindThemeIndex(LPCWSTR pszName) const;

    // Add/remove
    size_t AddTheme(Theme* pTheme);
    void RemoveTheme(LPCWSTR pszName);
    void RemoveAllThemes();

    // Default theme
    Theme* GetDefaultTheme();
    void SetDefaultTheme(LPCWSTR pszName);
    ATL::CString GetDefaultThemeName() const;

    // Theme directory
    ATL::CString GetThemeDirectory() const;
    HRESULT SetThemeDirectory(LPCWSTR pszDirectory);

    // Reload from disk
    HRESULT ReloadThemes();

    // Theme project creation
    ThemeProject* CreateThemeProject(Theme* pTheme);

    // Built-in themes
    HRESULT LoadBuiltInThemes();

private:
    HRESULT ScanThemeDirectory(LPCWSTR pszDirectory);
    HRESULT LoadThemeFile(LPCWSTR pszFilePath);
    Theme* ParseThemeXml(IXmlReader* pReader);

    ATL::CAtlArray<Theme*>  m_arrThemes;
    ATL::CString            m_strThemeDirectory;
    ATL::CString            m_strDefaultThemeName;
    DWORD                   m_dwNextThemeId;
    bool                    m_fInitialized;
};

} // namespace StoryboardManager

#endif // THEME_H
