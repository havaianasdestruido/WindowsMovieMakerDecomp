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
    {
        m_loadState = ThemeLoadStateError;
        return hr;
    }

    hr = spReader->SetInput(pStream);
    if (FAILED(hr))
    {
        m_loadState = ThemeLoadStateError;
        return hr;
    }

    XmlNodeType nodeType;
    HRESULT hrRead = S_OK;
    while ((hrRead = spReader->Read(&nodeType)) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pwszLocalName = nullptr;
            spReader->GetLocalName(&pwszLocalName, nullptr);
            if (!pwszLocalName)
                continue;

            if (wcscmp(pwszLocalName, kThemeRootElement) == 0)
            {
                LPCWSTR pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"name", &pwszValue)) && pwszValue)
                    SetName(pwszValue);

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"displayName", &pwszValue)) && pwszValue)
                    SetDisplayName(pwszValue);

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"description", &pwszValue)) && pwszValue)
                    SetDescription(pwszValue);

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"category", &pwszValue)) && pwszValue)
                    SetCategory(pwszValue);

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"primaryColor", &pwszValue)) && pwszValue)
                    SetPrimaryColor(static_cast<DWORD>(wcstoul(pwszValue, nullptr, 16)));

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"secondaryColor", &pwszValue)) && pwszValue)
                    SetSecondaryColor(static_cast<DWORD>(wcstoul(pwszValue, nullptr, 16)));

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"accentColor", &pwszValue)) && pwszValue)
                    SetAccentColor(static_cast<DWORD>(wcstoul(pwszValue, nullptr, 16)));

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"fontFamily", &pwszValue)) && pwszValue)
                    SetFontFamily(pwszValue);

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"fontSize", &pwszValue)) && pwszValue)
                    SetFontSize(static_cast<float>(_wtof(pwszValue)));

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"version", &pwszValue)) && pwszValue)
                {
                    DWORD dwMajor = 1, dwMinor = 0;
                    if (swscanf_s(pwszValue, L"%u.%u", &dwMajor, &dwMinor) == 2)
                        SetVersion(dwMajor, dwMinor);
                }

                pwszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"builtIn", &pwszValue)) && pwszValue)
                    SetBuiltIn(wcscmp(pwszValue, L"true") == 0);

                continue;
            }

            if (wcscmp(pwszLocalName, L"intro") == 0)
            {
                ThemeIntro* pIntro = new ThemeIntro();
                hr = pIntro->LoadFromXml(spReader);
                if (SUCCEEDED(hr))
                    SetIntro(pIntro);
                else
                    delete pIntro;
            }
            else if (wcscmp(pwszLocalName, L"mid") == 0)
            {
                ThemeMid* pMid = new ThemeMid();
                hr = pMid->LoadFromXml(spReader);
                if (SUCCEEDED(hr))
                    SetMid(pMid);
                else
                    delete pMid;
            }
            else if (wcscmp(pwszLocalName, L"outro") == 0)
            {
                ThemeOutro* pOutro = new ThemeOutro();
                hr = pOutro->LoadFromXml(spReader);
                if (SUCCEEDED(hr))
                    SetOutro(pOutro);
                else
                    delete pOutro;
            }
            else if (wcscmp(pwszLocalName, L"effectTemplate") == 0)
            {
                ThemeEffectTemplate* pEffTpl = new ThemeEffectTemplate();
                hr = pEffTpl->LoadFromXml(spReader);
                if (SUCCEEDED(hr))
                    AddEffectTemplate(pEffTpl);
                else
                    delete pEffTpl;
            }
            else if (wcscmp(pwszLocalName, L"transition") == 0)
            {
                ThemeTransition* pTrans = new ThemeTransition();
                hr = pTrans->LoadFromXml(spReader);
                if (SUCCEEDED(hr))
                    AddTransitionTemplate(pTrans);
                else
                    delete pTrans;
            }
        }
    }

    if (FAILED(hrRead))
    {
        m_loadState = ThemeLoadStateError;
        return hrRead;
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
    if (!m_strDescription.IsEmpty())
        spWriter->WriteAttributeString(nullptr, L"description", nullptr, m_strDescription);
    if (!m_strCategory.IsEmpty())
        spWriter->WriteAttributeString(nullptr, L"category", nullptr, m_strCategory);

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

    swprintf_s(szValue, L"%u.%u", m_dwVersionMajor, m_dwVersionMinor);
    spWriter->WriteAttributeString(nullptr, L"version", nullptr, szValue);

    if (m_fBuiltIn)
        spWriter->WriteAttributeString(nullptr, L"builtIn", nullptr, L"true");

    // Write intro section
    if (m_pIntro)
    {
        spWriter->WriteStartElement(nullptr, L"intro", nullptr);
        m_pIntro->SaveToXml(spWriter);
        spWriter->WriteEndElement();
    }

    // Write mid section
    if (m_pMid)
    {
        spWriter->WriteStartElement(nullptr, L"mid", nullptr);
        m_pMid->SaveToXml(spWriter);
        spWriter->WriteEndElement();
    }

    // Write outro section
    if (m_pOutro)
    {
        spWriter->WriteStartElement(nullptr, L"outro", nullptr);
        m_pOutro->SaveToXml(spWriter);
        spWriter->WriteEndElement();
    }

    // Write effect templates
    for (size_t i = 0; i < m_arrEffectTemplates.GetCount(); ++i)
    {
        ThemeEffectTemplate* pEffTpl = m_arrEffectTemplates.GetAt(i);
        if (pEffTpl)
            pEffTpl->SaveToXml(spWriter);
    }

    // Write transition templates
    for (size_t i = 0; i < m_arrTransitionTemplates.GetCount(); ++i)
    {
        ThemeTransition* pTrans = m_arrTransitionTemplates.GetAt(i);
        if (pTrans)
            pTrans->SaveToXml(spWriter);
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

HRESULT Theme::Apply(ThemeProject* pProject)
{
    if (!pProject)
        return E_INVALIDARG;

    pProject->SetTheme(this);

    // Apply font families from theme to project overrides
    if (!m_strFontFamily.IsEmpty())
    {
        pProject->SetTitleFontFamily(m_strFontFamily);
        pProject->SetCreditsFontFamily(m_strFontFamily);
    }

    // Apply colors from theme to project overrides
    pProject->SetTitleFontColor(m_dwPrimaryColor);
    pProject->SetCreditsFontColor(m_dwSecondaryColor);
    pProject->SetBackgroundColor(m_dwPrimaryColor);

    // Apply intro/outro enabled state based on theme sections
    pProject->SetIntroEnabled(m_pIntro != nullptr);
    pProject->SetOutroEnabled(m_pOutro != nullptr);

    // Apply durations from theme sections
    if (m_pIntro)
    {
        LONGLONG llIntroDuration = m_pIntro->GetDurationHns();
        if (llIntroDuration > 0)
            pProject->SetTitleDurationHns(llIntroDuration);
    }

    if (m_pOutro)
    {
        LONGLONG llOutroDuration = m_pOutro->GetDurationHns();
        if (llOutroDuration > 0)
            pProject->SetCreditsDurationHns(llOutroDuration);
    }

    // Apply transition duration from mid section default transition
    if (m_pMid)
    {
        ThemeTransition* pTrans = m_pMid->GetDefaultTransition();
        if (pTrans)
        {
            LONGLONG llTransDuration = pTrans->GetDurationHns();
            if (llTransDuration > 0)
                pProject->SetTransitionDurationHns(llTransDuration);
        }
    }

    return S_OK;
}

HRESULT Theme::Remove(ThemeProject* pProject)
{
    if (!pProject)
        return E_INVALIDARG;

    // Disconnect the theme association
    pProject->SetTheme(nullptr);

    // Reset all theme-overridden properties to defaults
    pProject->SetTitleText(L"");
    pProject->SetCreditsText(L"");
    pProject->SetTitleFontFamily(L"Segoe UI");
    pProject->SetCreditsFontFamily(L"Segoe UI");
    pProject->SetTitleFontColor(0xFFFFFFFF);
    pProject->SetCreditsFontColor(0xFFFFFFFF);
    pProject->SetTitleDurationHns(50000000);       // 5 seconds
    pProject->SetCreditsDurationHns(100000000);     // 10 seconds
    pProject->SetTransitionDurationHns(10000000);   // 1 second
    pProject->SetBackgroundColor(0xFF000000);        // black
    pProject->SetIntroEnabled(true);
    pProject->SetOutroEnabled(true);

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
    // =====================================================================
    // Contemporary theme
    // =====================================================================
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

    // =====================================================================
    // Fade theme
    // =====================================================================
    Theme* pFadeTheme = new Theme();
    pFadeTheme->SetName(L"Fade");
    pFadeTheme->SetDisplayName(L"Fade");
    pFadeTheme->SetDescription(L"Classic fade transitions between every clip");
    pFadeTheme->SetCategory(L"AutoMovie");
    pFadeTheme->SetBuiltIn(true);
    pFadeTheme->SetPrimaryColor(0xFF5B9BD5);
    pFadeTheme->SetSecondaryColor(0xFF9DC3E6);
    pFadeTheme->SetAccentColor(0xFFED7D31);
    pFadeTheme->SetFontFamily(L"Segoe UI");
    pFadeTheme->SetFontSize(36.0f);

    ThemeIntro* pFadeIntro = new ThemeIntro();
    ThemeTitle* pFadeTitle = new ThemeTitle();
    pFadeTitle->SetFontFamily(L"Segoe UI");
    pFadeTitle->SetFontSize(48.0f);
    pFadeTitle->SetFontColor(0xFFFFFFFF);
    pFadeTitle->SetPosition(0.5f, 0.5f);
    pFadeTitle->SetAnimation(TitleAnimationFadeIn);
    pFadeTitle->SetAlignment(ThemeTitle::TextAlignmentCenter);
    pFadeIntro->SetTitle(pFadeTitle);
    pFadeIntro->SetDurationMode(ThemeIntro::IntroDurationModeFitToTitle);
    pFadeIntro->SetBackgroundColor(0xFF000000);
    pFadeIntro->SetFadeInDurationHns(5000000);
    pFadeTheme->SetIntro(pFadeIntro);

    ThemeMid* pFadeMid = new ThemeMid();
    ThemePrimaryTrack* pFadeTrack = new ThemePrimaryTrack();
    pFadeTrack->SetName(L"Primary");
    pFadeTrack->SetBoundTrackType(TimelineTrackTypeVideo);
    pFadeTrack->SetAutoFill(true);
    pFadeMid->AddTrack(pFadeTrack);

    ThemeEffectTemplate* pFadeEffTpl = new ThemeEffectTemplate();
    pFadeEffTpl->SetName(L"Fade Effect");
    pFadeEffTpl->SetDefaultIntensity(1.0f);
    ThemeEffect* pFadeEffect = new ThemeEffect();
    pFadeEffect->SetType(ThemeEffectTypeFade);
    pFadeEffect->SetDurationHns(15000000);
    pFadeEffTpl->AddEffect(pFadeEffect);
    pFadeMid->SetDefaultEffectTemplate(pFadeEffTpl);

    ThemeTransition* pFadeTrans = new ThemeTransition();
    pFadeTrans->SetName(L"Fade");
    pFadeTrans->SetClipName(L"fade");
    pFadeTrans->SetDurationHns(15000000);
    pFadeMid->SetDefaultTransition(pFadeTrans);
    pFadeMid->SetLooping(true);
    pFadeTheme->SetMid(pFadeMid);

    ThemeOutro* pFadeOutro = new ThemeOutro();
    ThemeTitle* pFadeCredits = new ThemeTitle();
    pFadeCredits->SetFontFamily(L"Segoe UI");
    pFadeCredits->SetFontSize(24.0f);
    pFadeCredits->SetFontColor(0xFFFFFFFF);
    pFadeCredits->SetPosition(0.5f, 0.5f);
    pFadeCredits->SetAnimation(TitleAnimationFadeIn);
    pFadeOutro->SetCreditsTitle(pFadeCredits);
    pFadeOutro->SetDurationMode(ThemeOutro::OutroDurationModeScrollText);
    pFadeOutro->SetBackgroundColor(0xFF000000);
    pFadeOutro->SetFadeOutDurationHns(15000000);
    pFadeOutro->SetScrollSpeed(50.0f);
    pFadeTheme->SetOutro(pFadeOutro);

    pFadeTheme->SetLoadState(ThemeLoadStateLoaded);
    AddTheme(pFadeTheme);

    // =====================================================================
    // Magazine theme
    // =====================================================================
    Theme* pMagazine = new Theme();
    pMagazine->SetName(L"Magazine");
    pMagazine->SetDisplayName(L"Magazine");
    pMagazine->SetDescription(L"Bold magazine-style layout with dramatic transitions");
    pMagazine->SetCategory(L"AutoMovie");
    pMagazine->SetBuiltIn(true);
    pMagazine->SetPrimaryColor(0xFF1B1B1B);
    pMagazine->SetSecondaryColor(0xFF333333);
    pMagazine->SetAccentColor(0xFFED7D31);
    pMagazine->SetFontFamily(L"Segoe UI Light");
    pMagazine->SetFontSize(36.0f);

    ThemeIntro* pMagIntro = new ThemeIntro();
    ThemeTitle* pMagTitle = new ThemeTitle();
    pMagTitle->SetFontFamily(L"Segoe UI Light");
    pMagTitle->SetFontSize(56.0f);
    pMagTitle->SetFontColor(0xFFFFFFFF);
    pMagTitle->SetPosition(0.5f, 0.8f);
    pMagTitle->SetAnimation(TitleAnimationZoomIn);
    pMagTitle->SetAlignment(ThemeTitle::TextAlignmentCenter);
    pMagIntro->SetTitle(pMagTitle);
    pMagIntro->SetDurationMode(ThemeIntro::IntroDurationModeFitToTitle);
    pMagIntro->SetBackgroundColor(0xFF000000);
    pMagIntro->SetFadeInDurationHns(5000000);
    pMagazine->SetIntro(pMagIntro);

    ThemeMid* pMagMid = new ThemeMid();
    ThemePrimaryTrack* pMagTrack = new ThemePrimaryTrack();
    pMagTrack->SetName(L"Primary");
    pMagTrack->SetBoundTrackType(TimelineTrackTypeVideo);
    pMagTrack->SetAutoFill(true);
    pMagMid->AddTrack(pMagTrack);

    ThemeEffectTemplate* pMagEffTpl = new ThemeEffectTemplate();
    pMagEffTpl->SetName(L"Vignette Effect");
    pMagEffTpl->SetDefaultIntensity(0.3f);
    ThemeEffect* pMagVignette = new ThemeEffect();
    pMagVignette->SetType(ThemeEffectTypeVignette);
    pMagVignette->SetIntensity(0.3);
    pMagEffTpl->AddEffect(pMagVignette);
    pMagMid->SetDefaultEffectTemplate(pMagEffTpl);

    ThemeTransition* pMagTrans = new ThemeTransition();
    pMagTrans->SetName(L"Push");
    pMagTrans->SetClipName(L"push");
    pMagTrans->SetDurationHns(7000000);
    pMagMid->SetDefaultTransition(pMagTrans);
    pMagMid->SetLooping(true);
    pMagazine->SetMid(pMagMid);

    ThemeOutro* pMagOutro = new ThemeOutro();
    ThemeTitle* pMagCredits = new ThemeTitle();
    pMagCredits->SetFontFamily(L"Segoe UI Light");
    pMagCredits->SetFontSize(24.0f);
    pMagCredits->SetFontColor(0xFFFFFFFF);
    pMagCredits->SetPosition(0.5f, 0.5f);
    pMagCredits->SetAnimation(TitleAnimationZoomIn);
    pMagOutro->SetCreditsTitle(pMagCredits);
    pMagOutro->SetDurationMode(ThemeOutro::OutroDurationModeScrollText);
    pMagOutro->SetBackgroundColor(0xFF000000);
    pMagOutro->SetFadeOutDurationHns(5000000);
    pMagOutro->SetScrollSpeed(60.0f);
    pMagazine->SetOutro(pMagOutro);

    pMagazine->SetLoadState(ThemeLoadStateLoaded);
    AddTheme(pMagazine);

    // =====================================================================
    // Retro theme
    // =====================================================================
    Theme* pRetro = new Theme();
    pRetro->SetName(L"Retro");
    pRetro->SetDisplayName(L"Retro");
    pRetro->SetDescription(L"Vintage look with sepia tones and film grain");
    pRetro->SetCategory(L"AutoMovie");
    pRetro->SetBuiltIn(true);
    pRetro->SetPrimaryColor(0xFF8B7355);
    pRetro->SetSecondaryColor(0xFFD2B48C);
    pRetro->SetAccentColor(0xFFFFF0D0);
    pRetro->SetFontFamily(L"Georgia");
    pRetro->SetFontSize(36.0f);

    ThemeIntro* pRetroIntro = new ThemeIntro();
    ThemeTitle* pRetroTitle = new ThemeTitle();
    pRetroTitle->SetFontFamily(L"Georgia");
    pRetroTitle->SetFontSize(48.0f);
    pRetroTitle->SetFontColor(0xFFFFF0D0);
    pRetroTitle->SetPosition(0.5f, 0.5f);
    pRetroTitle->SetAnimation(TitleAnimationTypewriter);
    pRetroTitle->SetAlignment(ThemeTitle::TextAlignmentCenter);
    pRetroIntro->SetTitle(pRetroTitle);
    pRetroIntro->SetDurationMode(ThemeIntro::IntroDurationModeFitToTitle);
    pRetroIntro->SetBackgroundColor(0xFF1A1008);
    pRetroIntro->SetFadeInDurationHns(12000000);
    pRetro->SetIntro(pRetroIntro);

    ThemeMid* pRetroMid = new ThemeMid();
    ThemePrimaryTrack* pRetroTrack = new ThemePrimaryTrack();
    pRetroTrack->SetName(L"Primary");
    pRetroTrack->SetBoundTrackType(TimelineTrackTypeVideo);
    pRetroTrack->SetAutoFill(true);
    pRetroMid->AddTrack(pRetroTrack);

    ThemeEffectTemplate* pRetroEffTpl = new ThemeEffectTemplate();
    pRetroEffTpl->SetName(L"Retro Effects");
    pRetroEffTpl->SetDefaultIntensity(1.0f);
    ThemeEffect* pRetroSepia = new ThemeEffect();
    pRetroSepia->SetType(ThemeEffectTypeSepia);
    pRetroSepia->SetIntensity(0.7);
    pRetroEffTpl->AddEffect(pRetroSepia);
    ThemeEffect* pRetroGrain = new ThemeEffect();
    pRetroGrain->SetType(ThemeEffectTypeFilmGrain);
    pRetroGrain->SetIntensity(0.4);
    pRetroEffTpl->AddEffect(pRetroGrain);
    ThemeEffect* pRetroVignette = new ThemeEffect();
    pRetroVignette->SetType(ThemeEffectTypeVignette);
    pRetroVignette->SetIntensity(0.5);
    pRetroEffTpl->AddEffect(pRetroVignette);
    pRetroMid->SetDefaultEffectTemplate(pRetroEffTpl);

    ThemeTransition* pRetroTrans = new ThemeTransition();
    pRetroTrans->SetName(L"Crossfade");
    pRetroTrans->SetClipName(L"crossfade");
    pRetroTrans->SetDurationHns(12000000);
    pRetroMid->SetDefaultTransition(pRetroTrans);
    pRetroMid->SetLooping(true);
    pRetro->SetMid(pRetroMid);

    ThemeOutro* pRetroOutro = new ThemeOutro();
    ThemeTitle* pRetroCredits = new ThemeTitle();
    pRetroCredits->SetFontFamily(L"Georgia");
    pRetroCredits->SetFontSize(24.0f);
    pRetroCredits->SetFontColor(0xFFFFF0D0);
    pRetroCredits->SetPosition(0.5f, 0.5f);
    pRetroCredits->SetAnimation(TitleAnimationTypewriter);
    pRetroOutro->SetCreditsTitle(pRetroCredits);
    pRetroOutro->SetDurationMode(ThemeOutro::OutroDurationModeScrollText);
    pRetroOutro->SetBackgroundColor(0xFF1A1008);
    pRetroOutro->SetFadeOutDurationHns(12000000);
    pRetroOutro->SetScrollSpeed(40.0f);
    pRetro->SetOutro(pRetroOutro);

    pRetro->SetLoadState(ThemeLoadStateLoaded);
    AddTheme(pRetro);

    // =====================================================================
    // Pan and zoom theme
    // =====================================================================
    Theme* pPanZoom = new Theme();
    pPanZoom->SetName(L"Pan and zoom");
    pPanZoom->SetDisplayName(L"Pan and zoom");
    pPanZoom->SetDescription(L"Ken Burns style pan and zoom effects on photos");
    pPanZoom->SetCategory(L"AutoMovie");
    pPanZoom->SetBuiltIn(true);
    pPanZoom->SetPrimaryColor(0xFF3A7D44);
    pPanZoom->SetSecondaryColor(0xFF6AAF6A);
    pPanZoom->SetAccentColor(0xFFED7D31);
    pPanZoom->SetFontFamily(L"Segoe UI");
    pPanZoom->SetFontSize(36.0f);

    ThemeIntro* pPZIntro = new ThemeIntro();
    ThemeTitle* pPZTitle = new ThemeTitle();
    pPZTitle->SetFontFamily(L"Segoe UI");
    pPZTitle->SetFontSize(48.0f);
    pPZTitle->SetFontColor(0xFFFFFFFF);
    pPZTitle->SetPosition(0.5f, 0.5f);
    pPZTitle->SetAnimation(TitleAnimationFadeIn);
    pPZTitle->SetAlignment(ThemeTitle::TextAlignmentCenter);
    pPZIntro->SetTitle(pPZTitle);
    pPZIntro->SetDurationMode(ThemeIntro::IntroDurationModeFitToTitle);
    pPZIntro->SetBackgroundColor(0xFF000000);
    pPZIntro->SetFadeInDurationHns(5000000);
    pPanZoom->SetIntro(pPZIntro);

    ThemeMid* pPZMid = new ThemeMid();
    ThemePrimaryTrack* pPZTrack = new ThemePrimaryTrack();
    pPZTrack->SetName(L"Primary");
    pPZTrack->SetBoundTrackType(TimelineTrackTypeVideo);
    pPZTrack->SetAutoFill(true);
    pPZMid->AddTrack(pPZTrack);

    ThemeEffectTemplate* pPZEffTpl = new ThemeEffectTemplate();
    pPZEffTpl->SetName(L"PanZoom Effect");
    pPZEffTpl->SetDefaultIntensity(1.0f);
    ThemeEffect* pPZPanZoom = new ThemeEffect();
    pPZPanZoom->SetType(ThemeEffectTypePanZoom);
    pPZPanZoom->SetDurationHns(60000000);
    pPZPanZoom->SetParameter(L"Style", L"KenBurns");
    pPZEffTpl->AddEffect(pPZPanZoom);
    ThemeEffect* pPZDissolve = new ThemeEffect();
    pPZDissolve->SetType(ThemeEffectTypeDissolve);
    pPZDissolve->SetDurationHns(10000000);
    pPZEffTpl->AddEffect(pPZDissolve);
    pPZMid->SetDefaultEffectTemplate(pPZEffTpl);

    ThemeTransition* pPZTrans = new ThemeTransition();
    pPZTrans->SetName(L"Dissolve");
    pPZTrans->SetClipName(L"dissolve");
    pPZTrans->SetDurationHns(10000000);
    pPZMid->SetDefaultTransition(pPZTrans);
    pPZMid->SetLooping(true);
    pPanZoom->SetMid(pPZMid);

    ThemeOutro* pPZOutro = new ThemeOutro();
    ThemeTitle* pPZCredits = new ThemeTitle();
    pPZCredits->SetFontFamily(L"Segoe UI");
    pPZCredits->SetFontSize(24.0f);
    pPZCredits->SetFontColor(0xFFFFFFFF);
    pPZCredits->SetPosition(0.5f, 0.5f);
    pPZCredits->SetAnimation(TitleAnimationFadeIn);
    pPZOutro->SetCreditsTitle(pPZCredits);
    pPZOutro->SetDurationMode(ThemeOutro::OutroDurationModeScrollText);
    pPZOutro->SetBackgroundColor(0xFF000000);
    pPZOutro->SetFadeOutDurationHns(10000000);
    pPZOutro->SetScrollSpeed(50.0f);
    pPanZoom->SetOutro(pPZOutro);

    pPanZoom->SetLoadState(ThemeLoadStateLoaded);
    AddTheme(pPanZoom);

    // =====================================================================
    // Contemporary (photo) theme
    // =====================================================================
    Theme* pContPhoto = new Theme();
    pContPhoto->SetName(L"Contemporary (photo)");
    pContPhoto->SetDisplayName(L"Contemporary (photo)");
    pContPhoto->SetDescription(L"Photo-optimized Contemporary theme with pan/zoom and clean transitions");
    pContPhoto->SetCategory(L"AutoMovie");
    pContPhoto->SetBuiltIn(true);
    pContPhoto->SetPrimaryColor(0xFF2E74B5);
    pContPhoto->SetSecondaryColor(0xFF4472C4);
    pContPhoto->SetAccentColor(0xFFED7D31);
    pContPhoto->SetFontFamily(L"Segoe UI Semilight");
    pContPhoto->SetFontSize(36.0f);

    ThemeIntro* pCpIntro = new ThemeIntro();
    ThemeTitle* pCpTitle = new ThemeTitle();
    pCpTitle->SetFontFamily(L"Segoe UI Semilight");
    pCpTitle->SetFontSize(48.0f);
    pCpTitle->SetFontColor(0xFFFFFFFF);
    pCpTitle->SetPosition(0.5f, 0.5f);
    pCpTitle->SetAnimation(TitleAnimationFadeIn);
    pCpTitle->SetAlignment(ThemeTitle::TextAlignmentCenter);
    pCpIntro->SetTitle(pCpTitle);
    pCpIntro->SetDurationMode(ThemeIntro::IntroDurationModeFitToTitle);
    pCpIntro->SetBackgroundColor(0xFF000000);
    pCpIntro->SetFadeInDurationHns(5000000);
    pContPhoto->SetIntro(pCpIntro);

    ThemeMid* pCpMid = new ThemeMid();
    ThemePrimaryTrack* pCpTrack = new ThemePrimaryTrack();
    pCpTrack->SetName(L"Primary");
    pCpTrack->SetBoundTrackType(TimelineTrackTypeVideo);
    pCpTrack->SetAutoFill(true);
    pCpMid->AddTrack(pCpTrack);

    ThemeEffectTemplate* pCpEffTpl = new ThemeEffectTemplate();
    pCpEffTpl->SetName(L"Photo Effect");
    pCpEffTpl->SetDefaultIntensity(1.0f);
    ThemeEffect* pCpPanZoom = new ThemeEffect();
    pCpPanZoom->SetType(ThemeEffectTypePanZoom);
    pCpPanZoom->SetDurationHns(50000000);
    pCpPanZoom->SetParameter(L"Style", L"Auto");
    pCpEffTpl->AddEffect(pCpPanZoom);
    ThemeEffect* pCpCrossfade = new ThemeEffect();
    pCpCrossfade->SetType(ThemeEffectTypeCrossfade);
    pCpCrossfade->SetDurationHns(8000000);
    pCpEffTpl->AddEffect(pCpCrossfade);
    pCpMid->SetDefaultEffectTemplate(pCpEffTpl);

    ThemeTransition* pCpTrans = new ThemeTransition();
    pCpTrans->SetName(L"Crossfade");
    pCpTrans->SetClipName(L"crossfade");
    pCpTrans->SetDurationHns(8000000);
    pCpMid->SetDefaultTransition(pCpTrans);
    pCpMid->SetLooping(true);
    pContPhoto->SetMid(pCpMid);

    ThemeOutro* pCpOutro = new ThemeOutro();
    ThemeTitle* pCpCredits = new ThemeTitle();
    pCpCredits->SetFontFamily(L"Segoe UI Semilight");
    pCpCredits->SetFontSize(24.0f);
    pCpCredits->SetFontColor(0xFFFFFFFF);
    pCpCredits->SetPosition(0.5f, 0.5f);
    pCpCredits->SetAnimation(TitleAnimationFadeIn);
    pCpOutro->SetCreditsTitle(pCpCredits);
    pCpOutro->SetDurationMode(ThemeOutro::OutroDurationModeScrollText);
    pCpOutro->SetBackgroundColor(0xFF000000);
    pCpOutro->SetFadeOutDurationHns(5000000);
    pCpOutro->SetScrollSpeed(50.0f);
    pContPhoto->SetOutro(pCpOutro);

    pContPhoto->SetLoadState(ThemeLoadStateLoaded);
    AddTheme(pContPhoto);

    // =====================================================================
    // Fly in theme
    // =====================================================================
    Theme* pFlyIn = new Theme();
    pFlyIn->SetName(L"Fly in");
    pFlyIn->SetDisplayName(L"Fly in");
    pFlyIn->SetDescription(L"Dynamic fly-in entrance animations for photos");
    pFlyIn->SetCategory(L"AutoMovie");
    pFlyIn->SetBuiltIn(true);
    pFlyIn->SetPrimaryColor(0xFF7B2D8E);
    pFlyIn->SetSecondaryColor(0xFFB070C0);
    pFlyIn->SetAccentColor(0xFFED7D31);
    pFlyIn->SetFontFamily(L"Segoe UI Semibold");
    pFlyIn->SetFontSize(36.0f);

    ThemeIntro* pFiIntro = new ThemeIntro();
    ThemeTitle* pFiTitle = new ThemeTitle();
    pFiTitle->SetFontFamily(L"Segoe UI Semibold");
    pFiTitle->SetFontSize(48.0f);
    pFiTitle->SetFontColor(0xFFFFFFFF);
    pFiTitle->SetPosition(0.5f, 0.5f);
    pFiTitle->SetAnimation(TitleAnimationFlyInLeft);
    pFiTitle->SetAlignment(ThemeTitle::TextAlignmentCenter);
    pFiIntro->SetTitle(pFiTitle);
    pFiIntro->SetDurationMode(ThemeIntro::IntroDurationModeFitToTitle);
    pFiIntro->SetBackgroundColor(0xFF000000);
    pFiIntro->SetFadeInDurationHns(5000000);
    pFlyIn->SetIntro(pFiIntro);

    ThemeMid* pFiMid = new ThemeMid();
    ThemePrimaryTrack* pFiTrack = new ThemePrimaryTrack();
    pFiTrack->SetName(L"Primary");
    pFiTrack->SetBoundTrackType(TimelineTrackTypeVideo);
    pFiTrack->SetAutoFill(true);
    pFiMid->AddTrack(pFiTrack);

    ThemeEffectTemplate* pFiEffTpl = new ThemeEffectTemplate();
    pFiEffTpl->SetName(L"Fly In Effect");
    pFiEffTpl->SetDefaultIntensity(1.0f);
    ThemeEffect* pFiFlyIn = new ThemeEffect();
    pFiFlyIn->SetType(ThemeEffectTypeFlyIn);
    pFiFlyIn->SetDurationHns(8000000);
    pFiEffTpl->AddEffect(pFiFlyIn);
    ThemeEffect* pFiPanZoom = new ThemeEffect();
    pFiPanZoom->SetType(ThemeEffectTypePanZoom);
    pFiPanZoom->SetDurationHns(50000000);
    pFiPanZoom->SetParameter(L"Direction", L"Right");
    pFiEffTpl->AddEffect(pFiPanZoom);
    pFiMid->SetDefaultEffectTemplate(pFiEffTpl);

    ThemeTransition* pFiTrans = new ThemeTransition();
    pFiTrans->SetName(L"Fly In Transition");
    pFiTrans->SetClipName(L"flyin");
    pFiTrans->SetDurationHns(8000000);
    pFiMid->SetDefaultTransition(pFiTrans);
    pFiMid->SetLooping(true);
    pFlyIn->SetMid(pFiMid);

    ThemeOutro* pFiOutro = new ThemeOutro();
    ThemeTitle* pFiCredits = new ThemeTitle();
    pFiCredits->SetFontFamily(L"Segoe UI Semibold");
    pFiCredits->SetFontSize(24.0f);
    pFiCredits->SetFontColor(0xFFFFFFFF);
    pFiCredits->SetPosition(0.5f, 0.5f);
    pFiCredits->SetAnimation(TitleAnimationFlyInLeft);
    pFiOutro->SetCreditsTitle(pFiCredits);
    pFiOutro->SetDurationMode(ThemeOutro::OutroDurationModeScrollText);
    pFiOutro->SetBackgroundColor(0xFF000000);
    pFiOutro->SetFadeOutDurationHns(5000000);
    pFiOutro->SetScrollSpeed(55.0f);
    pFlyIn->SetOutro(pFiOutro);

    pFlyIn->SetLoadState(ThemeLoadStateLoaded);
    AddTheme(pFlyIn);

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
    if (!pReader)
        return nullptr;

    Theme* pTheme = new Theme();

    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pwszName = nullptr;
            pReader->GetLocalName(&pwszName, nullptr);

            if (!pwszName)
                continue;

            if (wcscmp(pwszName, L"theme") == 0)
            {
                LPCWSTR pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"name", &pwszVal)) && pwszVal)
                    pTheme->SetName(pwszVal);

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"displayName", &pwszVal)) && pwszVal)
                    pTheme->SetDisplayName(pwszVal);

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"description", &pwszVal)) && pwszVal)
                    pTheme->SetDescription(pwszVal);

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"category", &pwszVal)) && pwszVal)
                    pTheme->SetCategory(pwszVal);

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fontFamily", &pwszVal)) && pwszVal)
                    pTheme->SetFontFamily(pwszVal);

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fontSize", &pwszVal)) && pwszVal)
                    pTheme->SetFontSize(static_cast<float>(_wtof(pwszVal)));

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"primaryColor", &pwszVal)) && pwszVal)
                    pTheme->SetPrimaryColor(static_cast<DWORD>(wcstoul(pwszVal, nullptr, 16)));

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"secondaryColor", &pwszVal)) && pwszVal)
                    pTheme->SetSecondaryColor(static_cast<DWORD>(wcstoul(pwszVal, nullptr, 16)));

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"accentColor", &pwszVal)) && pwszVal)
                    pTheme->SetAccentColor(static_cast<DWORD>(wcstoul(pwszVal, nullptr, 16)));

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"version", &pwszVal)) && pwszVal)
                {
                    DWORD dwMajor = 1, dwMinor = 0;
                    if (swscanf_s(pwszVal, L"%u.%u", &dwMajor, &dwMinor) == 2)
                        pTheme->SetVersion(dwMajor, dwMinor);
                }

                pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"builtIn", &pwszVal)) && pwszVal)
                    pTheme->SetBuiltIn(wcscmp(pwszVal, L"true") == 0);
            }
            else if (wcscmp(pwszName, L"intro") == 0)
            {
                ThemeIntro* pIntro = new ThemeIntro();
                pIntro->LoadFromXml(pReader);
                pTheme->SetIntro(pIntro);
            }
            else if (wcscmp(pwszName, L"mid") == 0)
            {
                ThemeMid* pMid = new ThemeMid();
                pMid->LoadFromXml(pReader);
                pTheme->SetMid(pMid);
            }
            else if (wcscmp(pwszName, L"outro") == 0)
            {
                ThemeOutro* pOutro = new ThemeOutro();
                pOutro->LoadFromXml(pReader);
                pTheme->SetOutro(pOutro);
            }
        }
        else if (nodeType == XmlNodeType_EndElement)
        {
            break;
        }
    }

    pTheme->SetLoadState(ThemeLoadStateLoaded);
    return pTheme;
}

} // namespace StoryboardManager
