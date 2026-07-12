/*
 * Templates.cpp
 *
 * Implementation of the template management system for the StoryboardManager
 * namespace. Provides the TemplateTable registry, AutoMovie theme definitions,
 * and built-in template initialization.
 *
 * Built-in themes:
 *   - Contemporary: modern look with crossfade transitions
 *   - Fade: classic fade transitions between extents
 *   - Fly in: dynamic fly-in entrance animations
 *   - Pan and zoom: Ken Burns effect on photos
 *   - Magazine: magazine-style layout with bold titles
 *   - Retro: vintage look with sepia and film grain
 *   - Contemporary (photo): photo-optimized version of Contemporary
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "Templates.h"

namespace StoryboardManager
{

// ============================================================================
// ThemeEffect implementation
// ============================================================================

ThemeEffect::ThemeEffect()
    : m_type(ThemeEffectTypeNone)
    , m_llDurationHns(10000000) // 1 second default
    , m_llDelayHns(0)
    , m_dblIntensity(1.0)
{
}

ThemeEffect::~ThemeEffect()
{
}

ThemeEffectType ThemeEffect::GetType() const throw()
{
    return m_type;
}

void ThemeEffect::SetType(ThemeEffectType type) throw()
{
    m_type = type;
}

LONGLONG ThemeEffect::GetDurationHns() const throw()
{
    return m_llDurationHns;
}

void ThemeEffect::SetDurationHns(LONGLONG llDuration) throw()
{
    m_llDurationHns = llDuration;
}

ATL::CString ThemeEffect::GetParameter(LPCWSTR pszKey) const
{
    if (!pszKey)
        return ATL::CString();

    for (size_t i = 0; i < m_arrParameters.GetCount(); ++i)
    {
        if (m_arrParameters.GetAt(i).strKey.CompareNoCase(pszKey) == 0)
            return m_arrParameters.GetAt(i).strValue;
    }
    return ATL::CString();
}

void ThemeEffect::SetParameter(LPCWSTR pszKey, LPCWSTR pszValue)
{
    if (!pszKey)
        return;

    // Update existing or add new
    for (size_t i = 0; i < m_arrParameters.GetCount(); ++i)
    {
        if (m_arrParameters.GetAt(i).strKey.CompareNoCase(pszKey) == 0)
        {
            m_arrParameters.GetAt(i).strValue = pszValue ? pszValue : L"";
            return;
        }
    }

    Parameter param;
    param.strKey = pszKey;
    param.strValue = pszValue ? pszValue : L"";
    m_arrParameters.Add(param);
}

double ThemeEffect::GetIntensity() const throw()
{
    return m_dblIntensity;
}

void ThemeEffect::SetIntensity(double dblIntensity) throw()
{
    if (dblIntensity < 0.0) dblIntensity = 0.0;
    if (dblIntensity > 1.0) dblIntensity = 1.0;
    m_dblIntensity = dblIntensity;
}

LONGLONG ThemeEffect::GetDelayHns() const throw()
{
    return m_llDelayHns;
}

void ThemeEffect::SetDelayHns(LONGLONG llDelay) throw()
{
    m_llDelayHns = llDelay;
}

ATL::CString ThemeEffect::GetX3dClipName() const
{
    return m_strX3dClipName;
}

void ThemeEffect::SetX3dClipName(LPCWSTR pszClipName)
{
    m_strX3dClipName = pszClipName ? pszClipName : L"";
}

// ============================================================================
// AutoMovieTheme implementation
// ============================================================================

AutoMovieTheme::AutoMovieTheme()
    : m_defaultTransitionType(ThemeEffectTypeCrossfade)
    , m_llDefaultTransitionDurationHns(10000000) // 1 second
    , m_defaultTitleAnimation(TitleAnimationFadeIn)
    , m_dwTitleFontColor(0xFFFFFFFF) // white
    , m_dblTitlePositionX(0.5) // center
    , m_dblTitlePositionY(0.5)
    , m_fBuiltIn(false)
{
}

AutoMovieTheme::~AutoMovieTheme()
{
}

ATL::CString AutoMovieTheme::GetName() const
{
    return m_strName;
}

void AutoMovieTheme::SetName(LPCWSTR pszName)
{
    m_strName = pszName ? pszName : L"";
}

ATL::CString AutoMovieTheme::GetDisplayName() const
{
    return m_strDisplayName;
}

void AutoMovieTheme::SetDisplayName(LPCWSTR pszDisplayName)
{
    m_strDisplayName = pszDisplayName ? pszDisplayName : L"";
}

ATL::CString AutoMovieTheme::GetDescription() const
{
    return m_strDescription;
}

void AutoMovieTheme::SetDescription(LPCWSTR pszDescription)
{
    m_strDescription = pszDescription ? pszDescription : L"";
}

ATL::CString AutoMovieTheme::GetThumbnailPath() const
{
    return m_strThumbnailPath;
}

void AutoMovieTheme::SetThumbnailPath(LPCWSTR pszPath)
{
    m_strThumbnailPath = pszPath ? pszPath : L"";
}

size_t AutoMovieTheme::GetEffectCount() const throw()
{
    return m_arrEffects.GetCount();
}

ThemeEffect* AutoMovieTheme::GetEffect(size_t nIndex)
{
    if (nIndex >= m_arrEffects.GetCount())
        return nullptr;
    return &m_arrEffects.GetAt(nIndex);
}

const ThemeEffect* AutoMovieTheme::GetEffect(size_t nIndex) const
{
    if (nIndex >= m_arrEffects.GetCount())
        return nullptr;
    return &m_arrEffects.GetAt(nIndex);
}

size_t AutoMovieTheme::AddEffect(const ThemeEffect& effect)
{
    return m_arrEffects.Add(effect);
}

void AutoMovieTheme::RemoveEffect(size_t nIndex)
{
    if (nIndex < m_arrEffects.GetCount())
    {
        m_arrEffects.RemoveAt(nIndex);
    }
}

void AutoMovieTheme::RemoveAllEffects()
{
    m_arrEffects.RemoveAll();
}

ThemeEffectType AutoMovieTheme::GetDefaultTransitionType() const throw()
{
    return m_defaultTransitionType;
}

void AutoMovieTheme::SetDefaultTransitionType(ThemeEffectType type) throw()
{
    m_defaultTransitionType = type;
}

LONGLONG AutoMovieTheme::GetDefaultTransitionDurationHns() const throw()
{
    return m_llDefaultTransitionDurationHns;
}

void AutoMovieTheme::SetDefaultTransitionDurationHns(LONGLONG llDuration) throw()
{
    m_llDefaultTransitionDurationHns = llDuration;
}

TitleAnimationType AutoMovieTheme::GetDefaultTitleAnimation() const throw()
{
    return m_defaultTitleAnimation;
}

void AutoMovieTheme::SetDefaultTitleAnimation(TitleAnimationType type) throw()
{
    m_defaultTitleAnimation = type;
}

ATL::CString AutoMovieTheme::GetTitleFontFamily() const
{
    return m_strTitleFontFamily;
}

void AutoMovieTheme::SetTitleFontFamily(LPCWSTR pszFontFamily)
{
    m_strTitleFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
}

DWORD AutoMovieTheme::GetTitleFontColor() const throw()
{
    return m_dwTitleFontColor;
}

void AutoMovieTheme::SetTitleFontColor(DWORD dwColor) throw()
{
    m_dwTitleFontColor = dwColor;
}

double AutoMovieTheme::GetTitlePositionX() const throw()
{
    return m_dblTitlePositionX;
}

double AutoMovieTheme::GetTitlePositionY() const throw()
{
    return m_dblTitlePositionY;
}

void AutoMovieTheme::SetTitlePosition(double dblX, double dblY) throw()
{
    m_dblTitlePositionX = dblX;
    m_dblTitlePositionY = dblY;
}

bool AutoMovieTheme::IsBuiltIn() const throw()
{
    return m_fBuiltIn;
}

void AutoMovieTheme::SetBuiltIn(bool fBuiltIn) throw()
{
    m_fBuiltIn = fBuiltIn;
}

ATL::CString AutoMovieTheme::GetCategory() const
{
    return m_strCategory;
}

void AutoMovieTheme::SetCategory(LPCWSTR pszCategory)
{
    m_strCategory = pszCategory ? pszCategory : L"";
}

// ============================================================================
// TemplateEntry implementation
// ============================================================================

TemplateEntry::TemplateEntry()
    : m_dwTemplateId(0)
    , m_pTheme(nullptr)
    , m_uMinMediaCount(1)
    , m_uMaxMediaCount(100)
    , m_dwSupportedMediaTypes(0x07) // video | photo | audio
    , m_nSortOrder(0)
{
}

TemplateEntry::~TemplateEntry()
{
}

DWORD TemplateEntry::GetTemplateId() const throw()
{
    return m_dwTemplateId;
}

void TemplateEntry::SetTemplateId(DWORD dwId) throw()
{
    m_dwTemplateId = dwId;
}

ATL::CString TemplateEntry::GetName() const
{
    return m_strName;
}

void TemplateEntry::SetName(LPCWSTR pszName)
{
    m_strName = pszName ? pszName : L"";
}

ATL::CString TemplateEntry::GetDisplayName() const
{
    return m_strDisplayName;
}

void TemplateEntry::SetDisplayName(LPCWSTR pszDisplayName)
{
    m_strDisplayName = pszDisplayName ? pszDisplayName : L"";
}

ATL::CString TemplateEntry::GetDescription() const
{
    return m_strDescription;
}

void TemplateEntry::SetDescription(LPCWSTR pszDescription)
{
    m_strDescription = pszDescription ? pszDescription : L"";
}

AutoMovieTheme* TemplateEntry::GetTheme()
{
    return m_pTheme;
}

const AutoMovieTheme* TemplateEntry::GetTheme() const
{
    return m_pTheme;
}

void TemplateEntry::SetTheme(AutoMovieTheme* pTheme)
{
    m_pTheme = pTheme;
}

ATL::CString TemplateEntry::GetThumbnailPath() const
{
    return m_strThumbnailPath;
}

void TemplateEntry::SetThumbnailPath(LPCWSTR pszPath)
{
    m_strThumbnailPath = pszPath ? pszPath : L"";
}

UINT TemplateEntry::GetMinMediaCount() const throw()
{
    return m_uMinMediaCount;
}

UINT TemplateEntry::GetMaxMediaCount() const throw()
{
    return m_uMaxMediaCount;
}

void TemplateEntry::SetMediaCountRange(UINT uMin, UINT uMax) throw()
{
    m_uMinMediaCount = uMin;
    m_uMaxMediaCount = uMax;
}

DWORD TemplateEntry::GetSupportedMediaTypes() const throw()
{
    return m_dwSupportedMediaTypes;
}

void TemplateEntry::SetSupportedMediaTypes(DWORD dwMask) throw()
{
    m_dwSupportedMediaTypes = dwMask;
}

int TemplateEntry::GetSortOrder() const throw()
{
    return m_nSortOrder;
}

void TemplateEntry::SetSortOrder(int nOrder) throw()
{
    m_nSortOrder = nOrder;
}

// ============================================================================
// TemplateTable implementation
// ============================================================================

TemplateTable::TemplateTable()
    : m_dwDefaultTemplateId(0)
    , m_dwNextTemplateId(1)
{
}

TemplateTable::~TemplateTable()
{
    RemoveAllTemplates();

    for (size_t i = 0; i < m_arrThemes.GetCount(); ++i)
    {
        delete m_arrThemes.GetAt(i);
    }
    m_arrThemes.RemoveAll();
}

// ============================================================================
// TemplateTable::InitializeDefaults
// ============================================================================
HRESULT TemplateTable::InitializeDefaults()
{
    CreateContemporaryTheme();
    CreateFadeTheme();
    CreateFlyInTheme();
    CreatePanAndZoomTheme();
    CreateMagazineTheme();
    CreateRetroTheme();
    CreateContemporaryPhotoTheme();

    // Set Contemporary as default
    if (m_arrTemplates.GetCount() > 0)
    {
        m_dwDefaultTemplateId = m_arrTemplates.GetAt(0)->GetTemplateId();
    }

    return S_OK;
}

// ============================================================================
// Template access
// ============================================================================

size_t TemplateTable::GetTemplateCount() const throw()
{
    return m_arrTemplates.GetCount();
}

TemplateEntry* TemplateTable::GetTemplateAt(size_t nIndex)
{
    if (nIndex >= m_arrTemplates.GetCount())
        return nullptr;
    return m_arrTemplates.GetAt(nIndex);
}

const TemplateEntry* TemplateTable::GetTemplateAt(size_t nIndex) const
{
    if (nIndex >= m_arrTemplates.GetCount())
        return nullptr;
    return m_arrTemplates.GetAt(nIndex);
}

TemplateEntry* TemplateTable::FindTemplate(DWORD dwId)
{
    for (size_t i = 0; i < m_arrTemplates.GetCount(); ++i)
    {
        if (m_arrTemplates.GetAt(i)->GetTemplateId() == dwId)
            return m_arrTemplates.GetAt(i);
    }
    return nullptr;
}

TemplateEntry* TemplateTable::FindTemplateByName(LPCWSTR pszName)
{
    if (!pszName)
        return nullptr;

    for (size_t i = 0; i < m_arrTemplates.GetCount(); ++i)
    {
        if (m_arrTemplates.GetAt(i)->GetName().CompareNoCase(pszName) == 0)
            return m_arrTemplates.GetAt(i);
    }
    return nullptr;
}

const TemplateEntry* TemplateTable::FindTemplate(DWORD dwId) const
{
    for (size_t i = 0; i < m_arrTemplates.GetCount(); ++i)
    {
        if (m_arrTemplates.GetAt(i)->GetTemplateId() == dwId)
            return m_arrTemplates.GetAt(i);
    }
    return nullptr;
}

const TemplateEntry* TemplateTable::FindTemplateByName(LPCWSTR pszName) const
{
    if (!pszName)
        return nullptr;

    for (size_t i = 0; i < m_arrTemplates.GetCount(); ++i)
    {
        if (m_arrTemplates.GetAt(i)->GetName().CompareNoCase(pszName) == 0)
            return m_arrTemplates.GetAt(i);
    }
    return nullptr;
}

size_t TemplateTable::AddTemplate(TemplateEntry* pEntry)
{
    ATLASSERT(pEntry != nullptr);
    if (pEntry->GetTemplateId() == 0)
    {
        pEntry->SetTemplateId(m_dwNextTemplateId++);
    }
    return m_arrTemplates.Add(pEntry);
}

void TemplateTable::RemoveTemplate(DWORD dwId)
{
    for (size_t i = 0; i < m_arrTemplates.GetCount(); ++i)
    {
        if (m_arrTemplates.GetAt(i)->GetTemplateId() == dwId)
        {
            delete m_arrTemplates.GetAt(i);
            m_arrTemplates.RemoveAt(i);
            return;
        }
    }
}

void TemplateTable::RemoveAllTemplates()
{
    for (size_t i = 0; i < m_arrTemplates.GetCount(); ++i)
    {
        delete m_arrTemplates.GetAt(i);
    }
    m_arrTemplates.RemoveAll();
    m_dwDefaultTemplateId = 0;
}

DWORD TemplateTable::GetDefaultTemplateId() const throw()
{
    return m_dwDefaultTemplateId;
}

void TemplateTable::SetDefaultTemplateId(DWORD dwId) throw()
{
    m_dwDefaultTemplateId = dwId;
}

TemplateEntry* TemplateTable::GetDefaultTemplate()
{
    return FindTemplate(m_dwDefaultTemplateId);
}

// ============================================================================
// Theme management
// ============================================================================

size_t TemplateTable::GetThemeCount() const throw()
{
    return m_arrThemes.GetCount();
}

AutoMovieTheme* TemplateTable::GetThemeAt(size_t nIndex)
{
    if (nIndex >= m_arrThemes.GetCount())
        return nullptr;
    return m_arrThemes.GetAt(nIndex);
}

AutoMovieTheme* TemplateTable::FindTheme(LPCWSTR pszName)
{
    if (!pszName)
        return nullptr;

    for (size_t i = 0; i < m_arrThemes.GetCount(); ++i)
    {
        if (m_arrThemes.GetAt(i)->GetName().CompareNoCase(pszName) == 0)
            return m_arrThemes.GetAt(i);
    }
    return nullptr;
}

void TemplateTable::AddTheme(AutoMovieTheme* pTheme)
{
    ATLASSERT(pTheme != nullptr);
    m_arrThemes.Add(pTheme);
}

void TemplateTable::RemoveTheme(LPCWSTR pszName)
{
    if (!pszName)
        return;

    for (size_t i = 0; i < m_arrThemes.GetCount(); ++i)
    {
        if (m_arrThemes.GetAt(i)->GetName().CompareNoCase(pszName) == 0)
        {
            delete m_arrThemes.GetAt(i);
            m_arrThemes.RemoveAt(i);
            return;
        }
    }
}

// ============================================================================
// Template recommendation
// ============================================================================

TemplateEntry* TemplateTable::RecommendTemplate(UINT uMediaCount, DWORD dwMediaType)
{
    // Find the best template for the given media count and type
    TemplateEntry* pBest = nullptr;
    int nBestScore = -1;

    for (size_t i = 0; i < m_arrTemplates.GetCount(); ++i)
    {
        TemplateEntry* pEntry = m_arrTemplates.GetAt(i);

        // Check media type support
        if ((pEntry->GetSupportedMediaTypes() & dwMediaType) == 0)
            continue;

        // Check media count range
        if (uMediaCount < pEntry->GetMinMediaCount() ||
            uMediaCount > pEntry->GetMaxMediaCount())
        {
            continue;
        }

        // Simple scoring: prefer templates where media count is in the middle of range
        UINT uRange = pEntry->GetMaxMediaCount() - pEntry->GetMinMediaCount();
        UINT uMid = pEntry->GetMinMediaCount() + uRange / 2;
        int nScore = static_cast<int>(uRange) - abs(static_cast<int>(uMediaCount) - static_cast<int>(uMid));

        if (nScore > nBestScore)
        {
            nBestScore = nScore;
            pBest = pEntry;
        }
    }

    // Fall back to default if no match
    if (!pBest)
    {
        pBest = GetDefaultTemplate();
    }

    return pBest;
}

// ============================================================================
// Built-in theme creation helpers
// ============================================================================

void TemplateTable::CreateContemporaryTheme()
{
    AutoMovieTheme* pTheme = new AutoMovieTheme();
    pTheme->SetName(L"Contemporary");
    pTheme->SetDisplayName(L"Contemporary");
    pTheme->SetDescription(L"A modern, clean look with smooth crossfade transitions");
    pTheme->SetCategory(L"AutoMovie");
    pTheme->SetBuiltIn(true);
    pTheme->SetDefaultTransitionType(ThemeEffectTypeCrossfade);
    pTheme->SetDefaultTransitionDurationHns(10000000); // 1 second
    pTheme->SetDefaultTitleAnimation(TitleAnimationFadeIn);
    pTheme->SetTitleFontFamily(L"Segoe UI");
    pTheme->SetTitleFontColor(0xFFFFFFFF);

    ThemeEffect fadeEffect;
    fadeEffect.SetType(ThemeEffectTypeFade);
    fadeEffect.SetDurationHns(5000000); // 0.5 seconds
    pTheme->AddEffect(fadeEffect);

    m_arrThemes.Add(pTheme);

    TemplateEntry* pEntry = new TemplateEntry();
    pEntry->SetTemplateId(m_dwNextTemplateId++);
    pEntry->SetName(L"Contemporary");
    pEntry->SetDisplayName(L"Contemporary");
    pEntry->SetDescription(L"A modern, clean look with smooth crossfade transitions");
    pEntry->SetTheme(pTheme);
    pEntry->SetMediaCountRange(1, 100);
    pEntry->SetSupportedMediaTypes(0x07);
    pEntry->SetSortOrder(0);

    m_arrTemplates.Add(pEntry);
}

void TemplateTable::CreateFadeTheme()
{
    AutoMovieTheme* pTheme = new AutoMovieTheme();
    pTheme->SetName(L"Fade");
    pTheme->SetDisplayName(L"Fade");
    pTheme->SetDescription(L"Classic fade transitions between every clip");
    pTheme->SetCategory(L"AutoMovie");
    pTheme->SetBuiltIn(true);
    pTheme->SetDefaultTransitionType(ThemeEffectTypeFade);
    pTheme->SetDefaultTransitionDurationHns(15000000); // 1.5 seconds
    pTheme->SetDefaultTitleAnimation(TitleAnimationFadeIn);
    pTheme->SetTitleFontFamily(L"Segoe UI");
    pTheme->SetTitleFontColor(0xFFFFFFFF);

    ThemeEffect fadeEffect;
    fadeEffect.SetType(ThemeEffectTypeFade);
    fadeEffect.SetDurationHns(15000000);
    pTheme->AddEffect(fadeEffect);

    m_arrThemes.Add(pTheme);

    TemplateEntry* pEntry = new TemplateEntry();
    pEntry->SetTemplateId(m_dwNextTemplateId++);
    pEntry->SetName(L"Fade");
    pEntry->SetDisplayName(L"Fade");
    pEntry->SetDescription(L"Classic fade transitions between every clip");
    pEntry->SetTheme(pTheme);
    pEntry->SetMediaCountRange(2, 100);
    pEntry->SetSupportedMediaTypes(0x07);
    pEntry->SetSortOrder(1);

    m_arrTemplates.Add(pEntry);
}

void TemplateTable::CreateFlyInTheme()
{
    AutoMovieTheme* pTheme = new AutoMovieTheme();
    pTheme->SetName(L"Fly in");
    pTheme->SetDisplayName(L"Fly in");
    pTheme->SetDescription(L"Dynamic fly-in entrance animations for photos");
    pTheme->SetCategory(L"AutoMovie");
    pTheme->SetBuiltIn(true);
    pTheme->SetDefaultTransitionType(ThemeEffectTypeFlyIn);
    pTheme->SetDefaultTransitionDurationHns(8000000); // 0.8 seconds
    pTheme->SetDefaultTitleAnimation(TitleAnimationFlyInLeft);
    pTheme->SetTitleFontFamily(L"Segoe UI Semibold");
    pTheme->SetTitleFontColor(0xFFFFFFFF);

    ThemeEffect flyIn;
    flyIn.SetType(ThemeEffectTypeFlyIn);
    flyIn.SetDurationHns(8000000);
    pTheme->AddEffect(flyIn);

    ThemeEffect panZoom;
    panZoom.SetType(ThemeEffectTypePanZoom);
    panZoom.SetDurationHns(50000000); // 5 seconds per photo
    panZoom.SetParameter(L"Direction", L"Right");
    pTheme->AddEffect(panZoom);

    m_arrThemes.Add(pTheme);

    TemplateEntry* pEntry = new TemplateEntry();
    pEntry->SetTemplateId(m_dwNextTemplateId++);
    pEntry->SetName(L"Fly in");
    pEntry->SetDisplayName(L"Fly in");
    pEntry->SetDescription(L"Dynamic fly-in entrance animations for photos");
    pEntry->SetTheme(pTheme);
    pEntry->SetMediaCountRange(3, 50);
    pEntry->SetSupportedMediaTypes(0x02); // photos only
    pEntry->SetSortOrder(2);

    m_arrTemplates.Add(pEntry);
}

void TemplateTable::CreatePanAndZoomTheme()
{
    AutoMovieTheme* pTheme = new AutoMovieTheme();
    pTheme->SetName(L"Pan and zoom");
    pTheme->SetDisplayName(L"Pan and zoom");
    pTheme->SetDescription(L"Ken Burns style pan and zoom effects on photos");
    pTheme->SetCategory(L"AutoMovie");
    pTheme->SetBuiltIn(true);
    pTheme->SetDefaultTransitionType(ThemeEffectTypeDissolve);
    pTheme->SetDefaultTransitionDurationHns(10000000);
    pTheme->SetDefaultTitleAnimation(TitleAnimationFadeIn);
    pTheme->SetTitleFontFamily(L"Segoe UI");
    pTheme->SetTitleFontColor(0xFFFFFFFF);

    ThemeEffect panZoom;
    panZoom.SetType(ThemeEffectTypePanZoom);
    panZoom.SetDurationHns(60000000); // 6 seconds per photo
    panZoom.SetParameter(L"Style", L"KenBurns");
    pTheme->AddEffect(panZoom);

    ThemeEffect dissolve;
    dissolve.SetType(ThemeEffectTypeDissolve);
    dissolve.SetDurationHns(10000000);
    pTheme->AddEffect(dissolve);

    m_arrThemes.Add(pTheme);

    TemplateEntry* pEntry = new TemplateEntry();
    pEntry->SetTemplateId(m_dwNextTemplateId++);
    pEntry->SetName(L"Pan and zoom");
    pEntry->SetDisplayName(L"Pan and zoom");
    pEntry->SetDescription(L"Ken Burns style pan and zoom effects on photos");
    pEntry->SetTheme(pTheme);
    pEntry->SetMediaCountRange(2, 50);
    pEntry->SetSupportedMediaTypes(0x02); // photos
    pEntry->SetSortOrder(3);

    m_arrTemplates.Add(pEntry);
}

void TemplateTable::CreateMagazineTheme()
{
    AutoMovieTheme* pTheme = new AutoMovieTheme();
    pTheme->SetName(L"Magazine");
    pTheme->SetDisplayName(L"Magazine");
    pTheme->SetDescription(L"Bold magazine-style layout with dramatic transitions");
    pTheme->SetCategory(L"AutoMovie");
    pTheme->SetBuiltIn(true);
    pTheme->SetDefaultTransitionType(ThemeEffectTypePush);
    pTheme->SetDefaultTransitionDurationHns(7000000);
    pTheme->SetDefaultTitleAnimation(TitleAnimationZoomIn);
    pTheme->SetTitleFontFamily(L"Segoe UI Light");
    pTheme->SetTitleFontColor(0xFFFFFFFF);
    pTheme->SetTitlePosition(0.5, 0.8);

    ThemeEffect push;
    push.SetType(ThemeEffectTypePush);
    push.SetDurationHns(7000000);
    pTheme->AddEffect(push);

    ThemeEffect vignette;
    vignette.SetType(ThemeEffectTypeVignette);
    vignette.SetIntensity(0.3);
    pTheme->AddEffect(vignette);

    m_arrThemes.Add(pTheme);

    TemplateEntry* pEntry = new TemplateEntry();
    pEntry->SetTemplateId(m_dwNextTemplateId++);
    pEntry->SetName(L"Magazine");
    pEntry->SetDisplayName(L"Magazine");
    pEntry->SetDescription(L"Bold magazine-style layout with dramatic transitions");
    pEntry->SetTheme(pTheme);
    pEntry->SetMediaCountRange(5, 100);
    pEntry->SetSupportedMediaTypes(0x03); // video | photo
    pEntry->SetSortOrder(4);

    m_arrTemplates.Add(pEntry);
}

void TemplateTable::CreateRetroTheme()
{
    AutoMovieTheme* pTheme = new AutoMovieTheme();
    pTheme->SetName(L"Retro");
    pTheme->SetDisplayName(L"Retro");
    pTheme->SetDescription(L"Vintage look with sepia tones and film grain");
    pTheme->SetCategory(L"AutoMovie");
    pTheme->SetBuiltIn(true);
    pTheme->SetDefaultTransitionType(ThemeEffectTypeCrossfade);
    pTheme->SetDefaultTransitionDurationHns(12000000);
    pTheme->SetDefaultTitleAnimation(TitleAnimationTypewriter);
    pTheme->SetTitleFontFamily(L"Georgia");
    pTheme->SetTitleFontColor(0xFFFFF0D0); // warm cream

    ThemeEffect sepia;
    sepia.SetType(ThemeEffectTypeSepia);
    sepia.SetIntensity(0.7);
    pTheme->AddEffect(sepia);

    ThemeEffect grain;
    grain.SetType(ThemeEffectTypeFilmGrain);
    grain.SetIntensity(0.4);
    pTheme->AddEffect(grain);

    ThemeEffect vignette;
    vignette.SetType(ThemeEffectTypeVignette);
    vignette.SetIntensity(0.5);
    pTheme->AddEffect(vignette);

    m_arrThemes.Add(pTheme);

    TemplateEntry* pEntry = new TemplateEntry();
    pEntry->SetTemplateId(m_dwNextTemplateId++);
    pEntry->SetName(L"Retro");
    pEntry->SetDisplayName(L"Retro");
    pEntry->SetDescription(L"Vintage look with sepia tones and film grain");
    pEntry->SetTheme(pTheme);
    pEntry->SetMediaCountRange(3, 50);
    pEntry->SetSupportedMediaTypes(0x03);
    pEntry->SetSortOrder(5);

    m_arrTemplates.Add(pEntry);
}

void TemplateTable::CreateContemporaryPhotoTheme()
{
    AutoMovieTheme* pTheme = new AutoMovieTheme();
    pTheme->SetName(L"Contemporary (photo)");
    pTheme->SetDisplayName(L"Contemporary (photo)");
    pTheme->SetDescription(L"Photo-optimized Contemporary theme with pan/zoom and clean transitions");
    pTheme->SetCategory(L"AutoMovie");
    pTheme->SetBuiltIn(true);
    pTheme->SetDefaultTransitionType(ThemeEffectTypeCrossfade);
    pTheme->SetDefaultTransitionDurationHns(8000000);
    pTheme->SetDefaultTitleAnimation(TitleAnimationFadeIn);
    pTheme->SetTitleFontFamily(L"Segoe UI Semilight");
    pTheme->SetTitleFontColor(0xFFFFFFFF);

    ThemeEffect panZoom;
    panZoom.SetType(ThemeEffectTypePanZoom);
    panZoom.SetDurationHns(50000000);
    panZoom.SetParameter(L"Style", L"Auto");
    pTheme->AddEffect(panZoom);

    ThemeEffect crossfade;
    crossfade.SetType(ThemeEffectTypeCrossfade);
    crossfade.SetDurationHns(8000000);
    pTheme->AddEffect(crossfade);

    m_arrThemes.Add(pTheme);

    TemplateEntry* pEntry = new TemplateEntry();
    pEntry->SetTemplateId(m_dwNextTemplateId++);
    pEntry->SetName(L"Contemporary (photo)");
    pEntry->SetDisplayName(L"Contemporary (photo)");
    pEntry->SetDescription(L"Photo-optimized Contemporary theme with pan/zoom and clean transitions");
    pEntry->SetTheme(pTheme);
    pEntry->SetMediaCountRange(2, 100);
    pEntry->SetSupportedMediaTypes(0x02); // photos
    pEntry->SetSortOrder(6);

    m_arrTemplates.Add(pEntry);
}

} // namespace StoryboardManager
