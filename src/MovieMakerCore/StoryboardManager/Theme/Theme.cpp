#include "pch.h"
/*
 * Theme.cpp
 *
 * Implementation of the Theme, ThemeProject, and ThemeManager classes
 * for the StoryboardManager namespace. Handles theme loading, management,
 * and project-specific theme configuration.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "Theme.h"
#include "ThemeStructure.h"
#include "TextComposition.h"

namespace StoryboardManager
{

// ============================================================================
// Theme implementation
// ============================================================================

Theme::Theme()
    : m_dwVersionMajor(kThemeVersionMajor)
    , m_dwVersionMinor(kThemeVersionMinor)
    , m_loadState(ThemeLoadStateUnloaded)
    , m_renderMode(ThemeRenderModeNone)
    , m_fBuiltIn(false)
    , m_dwPrimaryColor(0xFF2E74B5)   // Movie Maker blue
    , m_dwSecondaryColor(0xFF4472C4)  // lighter blue
    , m_dwAccentColor(0xFFED7D31)     // orange accent
    , m_flFontSize(36.0f)
    , m_pIntro(nullptr)
    , m_pMid(nullptr)
    , m_pOutro(nullptr)
{
}

Theme::~Theme()
{
    delete m_pIntro;
    delete m_pMid;
    delete m_pOutro;

    for (size_t i = 0; i < m_arrEffectTemplates.GetCount(); ++i)
    {
        delete m_arrEffectTemplates.GetAt(i);
    }
    m_arrEffectTemplates.RemoveAll();

    for (size_t i = 0; i < m_arrTransitionTemplates.GetCount(); ++i)
    {
        delete m_arrTransitionTemplates.GetAt(i);
    }
    m_arrTransitionTemplates.RemoveAll();
}

ATL::CString Theme::GetName() const
{
    return m_strName;
}

void Theme::SetName(LPCWSTR pszName)
{
    m_strName = pszName ? pszName : L"";
}

ATL::CString Theme::GetDisplayName() const
{
    return m_strDisplayName;
}

void Theme::SetDisplayName(LPCWSTR pszDisplayName)
{
    m_strDisplayName = pszDisplayName ? pszDisplayName : L"";
}

ATL::CString Theme::GetDescription() const
{
    return m_strDescription;
}

void Theme::SetDescription(LPCWSTR pszDescription)
{
    m_strDescription = pszDescription ? pszDescription : L"";
}

DWORD Theme::GetVersionMajor() const throw()
{
    return m_dwVersionMajor;
}

DWORD Theme::GetVersionMinor() const throw()
{
    return m_dwVersionMinor;
}

void Theme::SetVersion(DWORD dwMajor, DWORD dwMinor) throw()
{
    m_dwVersionMajor = dwMajor;
    m_dwVersionMinor = dwMinor;
}

ATL::CString Theme::GetThumbnailPath() const
{
    return m_strThumbnailPath;
}

void Theme::SetThumbnailPath(LPCWSTR pszPath)
{
    m_strThumbnailPath = pszPath ? pszPath : L"";
}

ThemeLoadState Theme::GetLoadState() const throw()
{
    return m_loadState;
}

void Theme::SetLoadState(ThemeLoadState state) throw()
{
    m_loadState = state;
}

ThemeIntro* Theme::GetIntro()
{
    return m_pIntro;
}

const ThemeIntro* Theme::GetIntro() const
{
    return m_pIntro;
}

void Theme::SetIntro(ThemeIntro* pIntro)
{
    if (m_pIntro && m_pIntro != pIntro)
        delete m_pIntro;
    m_pIntro = pIntro;
}

ThemeMid* Theme::GetMid()
{
    return m_pMid;
}

const ThemeMid* Theme::GetMid() const
{
    return m_pMid;
}

void Theme::SetMid(ThemeMid* pMid)
{
    if (m_pMid && m_pMid != pMid)
        delete m_pMid;
    m_pMid = pMid;
}

ThemeOutro* Theme::GetOutro()
{
    return m_pOutro;
}

const ThemeOutro* Theme::GetOutro() const
{
    return m_pOutro;
}

void Theme::SetOutro(ThemeOutro* pOutro)
{
    if (m_pOutro && m_pOutro != pOutro)
        delete m_pOutro;
    m_pOutro = pOutro;
}

size_t Theme::GetEffectTemplateCount() const throw()
{
    return m_arrEffectTemplates.GetCount();
}

ThemeEffectTemplate* Theme::GetEffectTemplate(size_t nIndex)
{
    if (nIndex >= m_arrEffectTemplates.GetCount())
        return nullptr;
    return m_arrEffectTemplates.GetAt(nIndex);
}

const ThemeEffectTemplate* Theme::GetEffectTemplate(size_t nIndex) const
{
    if (nIndex >= m_arrEffectTemplates.GetCount())
        return nullptr;
    return m_arrEffectTemplates.GetAt(nIndex);
}

size_t Theme::AddEffectTemplate(ThemeEffectTemplate* pTemplate)
{
    ATLASSERT(pTemplate != nullptr);
    return m_arrEffectTemplates.Add(pTemplate);
}

void Theme::RemoveEffectTemplate(size_t nIndex)
{
    if (nIndex < m_arrEffectTemplates.GetCount())
    {
        delete m_arrEffectTemplates.GetAt(nIndex);
        m_arrEffectTemplates.RemoveAt(nIndex);
    }
}

void Theme::RemoveAllEffectTemplates()
{
    for (size_t i = 0; i < m_arrEffectTemplates.GetCount(); ++i)
    {
        delete m_arrEffectTemplates.GetAt(i);
    }
    m_arrEffectTemplates.RemoveAll();
}

size_t Theme::GetTransitionTemplateCount() const throw()
{
    return m_arrTransitionTemplates.GetCount();
}

ThemeTransition* Theme::GetTransitionTemplate(size_t nIndex)
{
    if (nIndex >= m_arrTransitionTemplates.GetCount())
        return nullptr;
    return m_arrTransitionTemplates.GetAt(nIndex);
}

const ThemeTransition* Theme::GetTransitionTemplate(size_t nIndex) const
{
    if (nIndex >= m_arrTransitionTemplates.GetCount())
        return nullptr;
    return m_arrTransitionTemplates.GetAt(nIndex);
}

size_t Theme::AddTransitionTemplate(ThemeTransition* pTransition)
{
    ATLASSERT(pTransition != nullptr);
    return m_arrTransitionTemplates.Add(pTransition);
}

void Theme::RemoveTransitionTemplate(size_t nIndex)
{
    if (nIndex < m_arrTransitionTemplates.GetCount())
    {
        delete m_arrTransitionTemplates.GetAt(nIndex);
        m_arrTransitionTemplates.RemoveAt(nIndex);
    }
}

void Theme::RemoveAllTransitionTemplates()
{
    for (size_t i = 0; i < m_arrTransitionTemplates.GetCount(); ++i)
    {
        delete m_arrTransitionTemplates.GetAt(i);
    }
    m_arrTransitionTemplates.RemoveAll();
}

HRESULT Theme::BeginRender(ThemeRenderMode mode)
{
    m_renderMode = mode;
    return S_OK;
}

HRESULT Theme::EndRender()
{
    m_renderMode = ThemeRenderModeNone;
    return S_OK;
}

ThemeRenderMode Theme::GetCurrentRenderMode() const throw()
{
    return m_renderMode;
}

HRESULT Theme::LoadFromFile(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return E_INVALIDARG;

    m_loadState = ThemeLoadStateLoading;

    CComPtr<IStream> spStream;
    HRESULT hr = SHCreateStreamOnFile(pszFilePath, STGM_READ, &spStream);
    if (FAILED(hr))
    {
        m_loadState = ThemeLoadStateError;
        return hr;
    }

    hr = LoadFromStream(spStream);
    if (FAILED(hr))
    {
        m_loadState = ThemeLoadStateError;
        return hr;
    }

    m_loadState = ThemeLoadStateLoaded;
    return S_OK;
}

HRESULT Theme::SaveToFile(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return E_INVALIDARG;

    CComPtr<IStream> spStream;
    HRESULT hr = SHCreateStreamOnFile(pszFilePath, STGM_CREATE | STGM_WRITE, &spStream);
    if (FAILED(hr))
        return hr;

    return SaveToStream(spStream);
}

HRESULT Theme::LoadFromStream(IStream* pStream)
{
    if (!pStream)
        return E_INVALIDARG;

    CComPtr<IXmlReader> spReader;
    HRESULT hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&spReader), nullptr);
    if (FAILED(hr))
        return hr;

    spReader->SetInput(pStream);

    XmlNodeType nodeType;
    while (spReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pwszLocalName = nullptr;
            spReader->GetLocalName(&pwszLocalName, nullptr);
            if (pwszLocalName && wcscmp(pwszLocalName, kThemeRootElement) == 0)
            {
                LPCWSTR pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"name", &pwszValue)) && pwszValue)
                {
                    SetName(pwszValue);
                }
                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"displayName", &pwszValue)) && pwszValue)
                {
                    SetDisplayName(pwszValue);
                }

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"primaryColor", &pwszValue)) && pwszValue)
                {
                    SetPrimaryColor(static_cast<DWORD>(wcstoul(pwszValue, nullptr, 16)));
                }
                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"secondaryColor", &pwszValue)) && pwszValue)
                {
                    SetSecondaryColor(static_cast<DWORD>(wcstoul(pwszValue, nullptr, 16)));
                }
                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"accentColor", &pwszValue)) && pwszValue)
                {
                    SetAccentColor(static_cast<DWORD>(wcstoul(pwszValue, nullptr, 16)));
                }
                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"fontFamily", &pwszValue)) && pwszValue)
                {
                    SetFontFamily(pwszValue);
                }
                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"fontSize", &pwszValue)) && pwszValue)
                {
                    SetFontSize(static_cast<float>(_wtof(pwszValue)));
                }
                break;
            }
        }
    }

    m_loadState = ThemeLoadStateLoaded;
    return S_OK;
}

HRESULT Theme::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_INVALIDARG;

    pWriter->WriteStartElement(nullptr, L"theme", nullptr);

    pWriter->WriteAttributeString(nullptr, L"name", nullptr, m_strName);

    if (!m_strDisplayName.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"displayName", nullptr, m_strDisplayName);

    if (!m_strDescription.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"description", nullptr, m_strDescription);

    if (!m_strCategory.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"category", nullptr, m_strCategory);

    if (!m_strFontFamily.IsEmpty() && m_strFontFamily.Compare(L"Segoe UI") != 0)
        pWriter->WriteAttributeString(nullptr, L"fontFamily", nullptr, m_strFontFamily);

    WCHAR szValue[64] = { 0 };

    if (m_flFontSize != 36.0f)
    {
        swprintf_s(szValue, L"%g", m_flFontSize);
        pWriter->WriteAttributeString(nullptr, L"fontSize", nullptr, szValue);
    }

    if (m_dwPrimaryColor != 0xFF2E74B5)
    {
        swprintf_s(szValue, L"0x%08X", m_dwPrimaryColor);
        pWriter->WriteAttributeString(nullptr, L"primaryColor", nullptr, szValue);
    }

    if (m_dwSecondaryColor != 0xFF4472C4)
    {
        swprintf_s(szValue, L"0x%08X", m_dwSecondaryColor);
        pWriter->WriteAttributeString(nullptr, L"secondaryColor", nullptr, szValue);
    }

    if (m_dwAccentColor != 0xFFED7D31)
    {
        swprintf_s(szValue, L"0x%08X", m_dwAccentColor);
        pWriter->WriteAttributeString(nullptr, L"accentColor", nullptr, szValue);
    }

    swprintf_s(szValue, L"%u.%u", m_dwVersionMajor, m_dwVersionMinor);
    pWriter->WriteAttributeString(nullptr, L"version", nullptr, szValue);

    if (m_fBuiltIn)
        pWriter->WriteAttributeString(nullptr, L"builtIn", nullptr, L"true");

    // Write intro section
    if (m_pIntro)
    {
        pWriter->WriteStartElement(nullptr, L"intro", nullptr);
        m_pIntro->SaveToXml(pWriter);
        pWriter->WriteEndElement();
    }

    // Write mid section
    if (m_pMid)
    {
        pWriter->WriteStartElement(nullptr, L"mid", nullptr);
        m_pMid->SaveToXml(pWriter);
        pWriter->WriteEndElement();
    }

    // Write outro section
    if (m_pOutro)
    {
        pWriter->WriteStartElement(nullptr, L"outro", nullptr);
        m_pOutro->SaveToXml(pWriter);
        pWriter->WriteEndElement();
    }

    // Write effect templates
    for (size_t i = 0; i < m_arrEffectTemplates.GetCount(); ++i)
    {
        ThemeEffectTemplate* pEffTpl = m_arrEffectTemplates.GetAt(i);
        if (pEffTpl)
            pEffTpl->SaveToXml(pWriter);
    }

    // Write transition templates
    for (size_t i = 0; i < m_arrTransitionTemplates.GetCount(); ++i)
    {
        ThemeTransition* pTrans = m_arrTransitionTemplates.GetAt(i);
        if (pTrans)
            pTrans->SaveToXml(pWriter);
    }

    pWriter->WriteEndElement();
    return S_OK;
}

HRESULT Theme::SaveToStream(IStream* pStream)
{
    if (!pStream)
        return E_INVALIDARG;

    CComPtr<IXmlWriter> spWriter;
    HRESULT hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&spWriter), nullptr);
    if (FAILED(hr))
        return hr;

    spWriter->SetOutput(pStream);
    spWriter->SetProperty(XmlWriterProperty_ProcessNamespaces, FALSE);
    spWriter->WriteStartDocument(XmlStandalone_Omit);

    spWriter->WriteStartElement(nullptr, kThemeRootElement, nullptr);
    spWriter->WriteAttributeString(nullptr, L"name", nullptr, m_strName);
    if (!m_strDisplayName.IsEmpty())
        spWriter->WriteAttributeString(nullptr, L"displayName", nullptr, m_strDisplayName);

    // Visual appearance attributes
    WCHAR szValue[64] = { 0 };
    if (m_dwPrimaryColor != 0xFF2E74B5)
    {
        swprintf_s(szValue, L"0x%08X", m_dwPrimaryColor);
        spWriter->WriteAttributeString(nullptr, L"primaryColor", nullptr, szValue);
    }
    if (m_dwSecondaryColor != 0xFF4472C4)
    {
        swprintf_s(szValue, L"0x%08X", m_dwSecondaryColor);
        spWriter->WriteAttributeString(nullptr, L"secondaryColor", nullptr, szValue);
    }
    if (m_dwAccentColor != 0xFFED7D31)
    {
        swprintf_s(szValue, L"0x%08X", m_dwAccentColor);
        spWriter->WriteAttributeString(nullptr, L"accentColor", nullptr, szValue);
    }
    if (!m_strFontFamily.IsEmpty() && m_strFontFamily.Compare(L"Segoe UI") != 0)
    {
        spWriter->WriteAttributeString(nullptr, L"fontFamily", nullptr, m_strFontFamily);
    }
    if (m_flFontSize != 36.0f)
    {
        swprintf_s(szValue, L"%g", m_flFontSize);
        spWriter->WriteAttributeString(nullptr, L"fontSize", nullptr, szValue);
    }

    spWriter->WriteEndElement();
    spWriter->WriteEndDocument();
    spWriter->Flush();

    return S_OK;
}

HRESULT Theme::Validate() const
{
    if (m_strName.IsEmpty())
        return E_INVALIDARG;

    if (m_loadState == ThemeLoadStateError)
        return E_FAIL;

    return S_OK;
}

bool Theme::IsBuiltIn() const throw()
{
    return m_fBuiltIn;
}

void Theme::SetBuiltIn(bool fBuiltIn) throw()
{
    m_fBuiltIn = fBuiltIn;
}

ATL::CString Theme::GetCategory() const
{
    return m_strCategory;
}

void Theme::SetCategory(LPCWSTR pszCategory)
{
    m_strCategory = pszCategory ? pszCategory : L"";
}

DWORD Theme::GetPrimaryColor() const throw()
{
    return m_dwPrimaryColor;
}

void Theme::SetPrimaryColor(DWORD dwColor) throw()
{
    m_dwPrimaryColor = dwColor;
}

DWORD Theme::GetSecondaryColor() const throw()
{
    return m_dwSecondaryColor;
}

void Theme::SetSecondaryColor(DWORD dwColor) throw()
{
    m_dwSecondaryColor = dwColor;
}

DWORD Theme::GetAccentColor() const throw()
{
    return m_dwAccentColor;
}

void Theme::SetAccentColor(DWORD dwColor) throw()
{
    m_dwAccentColor = dwColor;
}

ATL::CString Theme::GetFontFamily() const
{
    return m_strFontFamily;
}

void Theme::SetFontFamily(LPCWSTR pszFontFamily)
{
    m_strFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
}

float Theme::GetFontSize() const throw()
{
    return m_flFontSize;
}

void Theme::SetFontSize(float flSize) throw()
{
    m_flFontSize = flSize;
}

HRESULT Theme::ApplyToElement(ThemeComplexType* pElement)
{
    if (!pElement)
        return E_INVALIDARG;

    BaseX3DTemplate* pX3d = pElement->GetX3dTemplate();
    if (pX3d)
    {
        TemplateProperty* pProp = pX3d->FindProperty(L"primaryColor");
        if (pProp)
        {
            WCHAR szValue[64] = { 0 };
            swprintf_s(szValue, L"0x%08X", m_dwPrimaryColor);
            pProp->SetValue(szValue);
        }

        pProp = pX3d->FindProperty(L"secondaryColor");
        if (pProp)
        {
            WCHAR szValue[64] = { 0 };
            swprintf_s(szValue, L"0x%08X", m_dwSecondaryColor);
            pProp->SetValue(szValue);
        }

        pProp = pX3d->FindProperty(L"accentColor");
        if (pProp)
        {
            WCHAR szValue[64] = { 0 };
            swprintf_s(szValue, L"0x%08X", m_dwAccentColor);
            pProp->SetValue(szValue);
        }

        pProp = pX3d->FindProperty(L"fontFamily");
        if (pProp)
        {
            pProp->SetValue(m_strFontFamily);
        }
    }

    for (size_t i = 0; i < pElement->GetChildCount(); ++i)
    {
        ThemeComplexType* pChild = pElement->GetChild(i);
        if (pChild)
            ApplyToElement(pChild);
    }

    return S_OK;
}

// ============================================================================
// ThemeProject implementation
// ============================================================================

ThemeProject::ThemeProject()
    : m_pTheme(nullptr)
    , m_dwTitleFontColor(0xFFFFFFFF)
    , m_dwCreditsFontColor(0xFFFFFFFF)
    , m_llTitleDurationHns(50000000)       // 5 seconds
    , m_llCreditsDurationHns(100000000)    // 10 seconds
    , m_llTransitionDurationHns(10000000)  // 1 second
    , m_dwBackgroundColor(0xFF000000)       // black
    , m_fIntroEnabled(true)
    , m_fOutroEnabled(true)
    , m_pTextManager(nullptr)
{
    m_pTextManager = new TextManager();
}

ThemeProject::~ThemeProject()
{
    delete m_pTextManager;
}

Theme* ThemeProject::GetTheme()
{
    return m_pTheme;
}

const Theme* ThemeProject::GetTheme() const
{
    return m_pTheme;
}

void ThemeProject::SetTheme(Theme* pTheme)
{
    m_pTheme = pTheme;
}

ATL::CString ThemeProject::GetTitleText() const
{
    return m_strTitleText;
}

void ThemeProject::SetTitleText(LPCWSTR pszText)
{
    m_strTitleText = pszText ? pszText : L"";
    if (m_pTextManager)
        m_pTextManager->SetTitleText(m_strTitleText);
}

ATL::CString ThemeProject::GetCreditsText() const
{
    return m_strCreditsText;
}

void ThemeProject::SetCreditsText(LPCWSTR pszText)
{
    m_strCreditsText = pszText ? pszText : L"";
    if (m_pTextManager)
        m_pTextManager->SetCreditsText(m_strCreditsText);
}

ATL::CString ThemeProject::GetTitleFontFamily() const
{
    return m_strTitleFontFamily;
}

void ThemeProject::SetTitleFontFamily(LPCWSTR pszFontFamily)
{
    m_strTitleFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
}

ATL::CString ThemeProject::GetCreditsFontFamily() const
{
    return m_strCreditsFontFamily;
}

void ThemeProject::SetCreditsFontFamily(LPCWSTR pszFontFamily)
{
    m_strCreditsFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
}

DWORD ThemeProject::GetTitleFontColor() const throw()
{
    return m_dwTitleFontColor;
}

void ThemeProject::SetTitleFontColor(DWORD dwColor) throw()
{
    m_dwTitleFontColor = dwColor;
}

DWORD ThemeProject::GetCreditsFontColor() const throw()
{
    return m_dwCreditsFontColor;
}

void ThemeProject::SetCreditsFontColor(DWORD dwColor) throw()
{
    m_dwCreditsFontColor = dwColor;
}

LONGLONG ThemeProject::GetTitleDurationHns() const throw()
{
    return m_llTitleDurationHns;
}

void ThemeProject::SetTitleDurationHns(LONGLONG llDuration) throw()
{
    m_llTitleDurationHns = llDuration;
}

LONGLONG ThemeProject::GetCreditsDurationHns() const throw()
{
    return m_llCreditsDurationHns;
}

void ThemeProject::SetCreditsDurationHns(LONGLONG llDuration) throw()
{
    m_llCreditsDurationHns = llDuration;
}

LONGLONG ThemeProject::GetTransitionDurationHns() const throw()
{
    return m_llTransitionDurationHns;
}

void ThemeProject::SetTransitionDurationHns(LONGLONG llDuration) throw()
{
    m_llTransitionDurationHns = llDuration;
}

DWORD ThemeProject::GetBackgroundColor() const throw()
{
    return m_dwBackgroundColor;
}

void ThemeProject::SetBackgroundColor(DWORD dwColor) throw()
{
    m_dwBackgroundColor = dwColor;
}

bool ThemeProject::IsIntroEnabled() const throw()
{
    return m_fIntroEnabled;
}

void ThemeProject::SetIntroEnabled(bool fEnabled) throw()
{
    m_fIntroEnabled = fEnabled;
}

bool ThemeProject::IsOutroEnabled() const throw()
{
    return m_fOutroEnabled;
}

void ThemeProject::SetOutroEnabled(bool fEnabled) throw()
{
    m_fOutroEnabled = fEnabled;
}

TextManager* ThemeProject::GetTextManager()
{
    return m_pTextManager;
}

const TextManager* ThemeProject::GetTextManager() const
{
    return m_pTextManager;
}

HRESULT ThemeProject::Validate() const
{
    if (!m_pTheme)
        return E_INVALIDARG;

    return S_OK;
}

HRESULT ThemeProject::ResetToDefaults()
{
    if (!m_pTheme)
        return E_INVALIDARG;

    m_strTitleText.Empty();
    m_strCreditsText.Empty();
    m_strTitleFontFamily = L"Segoe UI";
    m_strCreditsFontFamily = L"Segoe UI";
    m_dwTitleFontColor = 0xFFFFFFFF;
    m_dwCreditsFontColor = 0xFFFFFFFF;
    m_llTitleDurationHns = 50000000;
    m_llCreditsDurationHns = 100000000;
    m_llTransitionDurationHns = 10000000;
    m_dwBackgroundColor = 0xFF000000;
    m_fIntroEnabled = true;
    m_fOutroEnabled = true;

    return S_OK;
}

// ============================================================================
// ThemeManager implementation
// ============================================================================

ThemeManager::ThemeManager()
    : m_dwNextThemeId(1)
    , m_fInitialized(false)
{
}

ThemeManager::~ThemeManager()
{
    Shutdown();
}

HRESULT ThemeManager::Initialize()
{
    if (m_fInitialized)
        return S_FALSE;

    // Build the theme directory path from %LOCALAPPDATA%
    WCHAR szLocalAppData[MAX_PATH] = { 0 };
    HRESULT hr = SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, szLocalAppData);
    if (SUCCEEDED(hr))
    {
        m_strThemeDirectory = szLocalAppData;
        m_strThemeDirectory += kThemeBasePath;
    }
    else
    {
        m_strThemeDirectory = L".\\Themes";
    }

    m_fInitialized = true;
    return S_OK;
}

HRESULT ThemeManager::Shutdown()
{
    if (!m_fInitialized)
        return S_FALSE;

    RemoveAllThemes();
    m_fInitialized = false;
    return S_OK;
}

size_t ThemeManager::GetThemeCount() const throw()
{
    return m_arrThemes.GetCount();
}

Theme* ThemeManager::GetThemeAt(size_t nIndex)
{
    if (nIndex >= m_arrThemes.GetCount())
        return nullptr;
    return m_arrThemes.GetAt(nIndex);
}

const Theme* ThemeManager::GetThemeAt(size_t nIndex) const
{
    if (nIndex >= m_arrThemes.GetCount())
        return nullptr;
    return m_arrThemes.GetAt(nIndex);
}

Theme* ThemeManager::FindTheme(LPCWSTR pszName)
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

const Theme* ThemeManager::FindTheme(LPCWSTR pszName) const
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

Theme* ThemeManager::FindThemeByDisplayName(LPCWSTR pszDisplayName)
{
    if (!pszDisplayName)
        return nullptr;

    for (size_t i = 0; i < m_arrThemes.GetCount(); ++i)
    {
        if (m_arrThemes.GetAt(i)->GetDisplayName().Compare(pszDisplayName) == 0)
            return m_arrThemes.GetAt(i);
    }
    return nullptr;
}

int ThemeManager::FindThemeIndex(LPCWSTR pszName) const
{
    if (!pszName)
        return -1;

    for (size_t i = 0; i < m_arrThemes.GetCount(); ++i)
    {
        if (m_arrThemes.GetAt(i)->GetName().CompareNoCase(pszName) == 0)
            return static_cast<int>(i);
    }
    return -1;
}

size_t ThemeManager::AddTheme(Theme* pTheme)
{
    ATLASSERT(pTheme != nullptr);
    return m_arrThemes.Add(pTheme);
}

void ThemeManager::RemoveTheme(LPCWSTR pszName)
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

void ThemeManager::RemoveAllThemes()
{
    for (size_t i = 0; i < m_arrThemes.GetCount(); ++i)
    {
        delete m_arrThemes.GetAt(i);
    }
    m_arrThemes.RemoveAll();
}

Theme* ThemeManager::GetDefaultTheme()
{
    return FindTheme(m_strDefaultThemeName);
}

void ThemeManager::SetDefaultTheme(LPCWSTR pszName)
{
    m_strDefaultThemeName = pszName ? pszName : L"";
}

ATL::CString ThemeManager::GetDefaultThemeName() const
{
    return m_strDefaultThemeName;
}

ATL::CString ThemeManager::GetThemeDirectory() const
{
    return m_strThemeDirectory;
}

HRESULT ThemeManager::SetThemeDirectory(LPCWSTR pszDirectory)
{
    if (!pszDirectory)
        return E_INVALIDARG;

    m_strThemeDirectory = pszDirectory;
    return S_OK;
}

HRESULT ThemeManager::ReloadThemes()
{
    RemoveAllThemes();
    return ScanThemeDirectory(m_strThemeDirectory);
}

ThemeProject* ThemeManager::CreateThemeProject(Theme* pTheme)
{
    if (!pTheme)
        return nullptr;

    ThemeProject* pProject = new ThemeProject();
    pProject->SetTheme(pTheme);
    pProject->ResetToDefaults();
    return pProject;
}

HRESULT ThemeManager::LoadBuiltInThemes()
{
    // Create a default "Contemporary" theme
    Theme* pContemporary = new Theme();
    pContemporary->SetName(L"Contemporary");
    pContemporary->SetDisplayName(L"Contemporary");
    pContemporary->SetDescription(L"A modern, clean look with smooth crossfade transitions");
    pContemporary->SetCategory(L"AutoMovie");
    pContemporary->SetBuiltIn(true);
    pContemporary->SetPrimaryColor(0xFF2E74B5);
    pContemporary->SetSecondaryColor(0xFF4472C4);
    pContemporary->SetAccentColor(0xFFED7D31);
    pContemporary->SetFontFamily(L"Segoe UI");
    pContemporary->SetFontSize(36.0f);

    ThemeIntro* pIntro = new ThemeIntro();
    ThemeTitle* pTitle = new ThemeTitle();
    pTitle->SetText(L"");
    pTitle->SetFontFamily(L"Segoe UI");
    pTitle->SetFontSize(48.0f);
    pTitle->SetFontColor(0xFFFFFFFF);
    pTitle->SetPosition(0.5f, 0.5f);
    pTitle->SetAnimation(TitleAnimationFadeIn);
    pTitle->SetAlignment(ThemeTitle::TextAlignmentCenter);
    pIntro->SetTitle(pTitle);
    pIntro->SetDurationMode(ThemeIntro::IntroDurationModeFitToTitle);
    pIntro->SetBackgroundColor(0xFF000000);
    pIntro->SetFadeInDurationHns(5000000);
    pContemporary->SetIntro(pIntro);

    ThemeMid* pMid = new ThemeMid();
    ThemePrimaryTrack* pTrack = new ThemePrimaryTrack();
    pTrack->SetName(L"Primary");
    pTrack->SetBoundTrackType(TimelineTrackTypeVideo);
    pTrack->SetAutoFill(true);
    pMid->AddTrack(pTrack);

    ThemeEffectTemplate* pEffTpl = new ThemeEffectTemplate();
    pEffTpl->SetName(L"Default Effect");
    pEffTpl->SetDefaultIntensity(1.0f);
    ThemeEffect* pFade = new ThemeEffect();
    pFade->SetType(ThemeEffectTypeFade);
    pFade->SetDurationHns(5000000);
    pEffTpl->AddEffect(pFade);
    pMid->SetDefaultEffectTemplate(pEffTpl);

    ThemeTransition* pTrans = new ThemeTransition();
    pTrans->SetName(L"Crossfade");
    pTrans->SetClipName(L"crossfade");
    pTrans->SetDurationHns(10000000);
    pMid->SetDefaultTransition(pTrans);
    pMid->SetLooping(true);
    pContemporary->SetMid(pMid);

    ThemeOutro* pOutro = new ThemeOutro();
    ThemeTitle* pCreditsTitle = new ThemeTitle();
    pCreditsTitle->SetFontFamily(L"Segoe UI");
    pCreditsTitle->SetFontSize(24.0f);
    pCreditsTitle->SetFontColor(0xFFFFFFFF);
    pCreditsTitle->SetPosition(0.5f, 0.5f);
    pCreditsTitle->SetAnimation(TitleAnimationScrollUp);
    pOutro->SetCreditsTitle(pCreditsTitle);
    pOutro->SetDurationMode(ThemeOutro::OutroDurationModeScrollText);
    pOutro->SetBackgroundColor(0xFF000000);
    pOutro->SetFadeOutDurationHns(5000000);
    pOutro->SetScrollSpeed(50.0f);
    pContemporary->SetOutro(pOutro);

    pContemporary->SetLoadState(ThemeLoadStateLoaded);
    AddTheme(pContemporary);
    SetDefaultTheme(L"Contemporary");

    return S_OK;
}

HRESULT ThemeManager::ScanThemeDirectory(LPCWSTR pszDirectory)
{
    if (!pszDirectory)
        return E_INVALIDARG;

    WIN32_FIND_DATA findData = { 0 };
    ATL::CString strSearch = pszDirectory;
    strSearch += L"\\*";
    strSearch += kThemeFileExtension;

    HANDLE hFind = FindFirstFile(strSearch, &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return S_FALSE; // no themes found

    do
    {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        ATL::CString strFilePath = pszDirectory;
        strFilePath += L"\\";
        strFilePath += findData.cFileName;

        LoadThemeFile(strFilePath);

    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
    return S_OK;
}

HRESULT ThemeManager::LoadThemeFile(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return E_INVALIDARG;

    Theme* pTheme = new Theme();
    HRESULT hr = pTheme->LoadFromFile(pszFilePath);
    if (FAILED(hr))
    {
        delete pTheme;
        return hr;
    }

    pTheme->SetLoadState(ThemeLoadStateLoaded);
    AddTheme(pTheme);
    return S_OK;
}

Theme* ThemeManager::ParseThemeXml(IXmlReader* pReader)
{
    UNREFERENCED_PARAMETER(pReader);
    // Placeholder for full XML parsing of theme files
    return nullptr;
}

} // namespace StoryboardManager
