#include "pch.h"

/*
 * PublishItem.cpp
 *
 * Implementation of PublishItemProperties and PublishItemPropertyStore.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "PublishItem.h"

// ============================================================================
// PublishItemProperties implementation
// ============================================================================

PublishItemProperties::PublishItemProperties()
    : m_serviceType(PublishServiceTypeLocal)
    , m_bPrivacyPublic(true)
    , m_bAllowComments(true)
    , m_bAllowEmbedding(true)
    , m_uMaxVideoWidth(1920)
    , m_uMaxVideoHeight(1080)
    , m_dwMaxFileSizeMB(2048)
    , m_dwMaxDurationSec(3600)
{
}

PublishItemProperties::~PublishItemProperties()
{
}

void PublishItemProperties::SetServiceType(PublishServiceType type)
{
    m_serviceType = type;
}

PublishServiceType PublishItemProperties::GetServiceType() const throw()
{
    return m_serviceType;
}

void PublishItemProperties::SetDisplayName(LPCWSTR pszName)
{
    m_strDisplayName = pszName ? pszName : L"";
}

ATL::CString PublishItemProperties::GetDisplayName() const
{
    return m_strDisplayName;
}

void PublishItemProperties::SetServiceUrl(LPCWSTR pszUrl)
{
    m_strServiceUrl = pszUrl ? pszUrl : L"";
}

ATL::CString PublishItemProperties::GetServiceUrl() const
{
    return m_strServiceUrl;
}

void PublishItemProperties::SetAuthToken(LPCWSTR pszToken)
{
    m_strAuthToken = pszToken ? pszToken : L"";
}

ATL::CString PublishItemProperties::GetAuthToken() const
{
    return m_strAuthToken;
}

void PublishItemProperties::SetAuthSecret(LPCWSTR pszSecret)
{
    m_strAuthSecret = pszSecret ? pszSecret : L"";
}

ATL::CString PublishItemProperties::GetAuthSecret() const
{
    return m_strAuthSecret;
}

void PublishItemProperties::SetTitle(LPCWSTR pszTitle)
{
    m_strTitle = pszTitle ? pszTitle : L"";
}

ATL::CString PublishItemProperties::GetTitle() const
{
    return m_strTitle;
}

void PublishItemProperties::SetDescription(LPCWSTR pszDescription)
{
    m_strDescription = pszDescription ? pszDescription : L"";
}

ATL::CString PublishItemProperties::GetDescription() const
{
    return m_strDescription;
}

void PublishItemProperties::SetTags(LPCWSTR pszTags)
{
    m_strTags = pszTags ? pszTags : L"";
}

ATL::CString PublishItemProperties::GetTags() const
{
    return m_strTags;
}

void PublishItemProperties::SetPrivacyPublic(bool bPublic)
{
    m_bPrivacyPublic = bPublic;
}

bool PublishItemProperties::IsPrivacyPublic() const throw()
{
    return m_bPrivacyPublic;
}

void PublishItemProperties::SetAllowComments(bool bAllow)
{
    m_bAllowComments = bAllow;
}

bool PublishItemProperties::IsCommentsAllowed() const throw()
{
    return m_bAllowComments;
}

void PublishItemProperties::SetAllowEmbedding(bool bAllow)
{
    m_bAllowEmbedding = bAllow;
}

bool PublishItemProperties::IsEmbeddingAllowed() const throw()
{
    return m_bAllowEmbedding;
}

void PublishItemProperties::SetMaxVideoWidth(UINT uWidth)
{
    m_uMaxVideoWidth = uWidth;
}

UINT PublishItemProperties::GetMaxVideoWidth() const throw()
{
    return m_uMaxVideoWidth;
}

void PublishItemProperties::SetMaxVideoHeight(UINT uHeight)
{
    m_uMaxVideoHeight = uHeight;
}

UINT PublishItemProperties::GetMaxVideoHeight() const throw()
{
    return m_uMaxVideoHeight;
}

void PublishItemProperties::SetMaxFileSizeMB(DWORD dwSizeMB)
{
    m_dwMaxFileSizeMB = dwSizeMB;
}

DWORD PublishItemProperties::GetMaxFileSizeMB() const throw()
{
    return m_dwMaxFileSizeMB;
}

void PublishItemProperties::SetMaxDurationSec(DWORD dwDurationSec)
{
    m_dwMaxDurationSec = dwDurationSec;
}

DWORD PublishItemProperties::GetMaxDurationSec() const throw()
{
    return m_dwMaxDurationSec;
}

void PublishItemProperties::SetCategory(LPCWSTR pszCategory)
{
    m_strCategory = pszCategory ? pszCategory : L"";
}

ATL::CString PublishItemProperties::GetCategory() const
{
    return m_strCategory;
}

void PublishItemProperties::SetCustomProperty(LPCWSTR pszKey, LPCWSTR pszValue)
{
    if (pszKey)
        m_customProperties[pszKey] = pszValue ? pszValue : L"";
}

ATL::CString PublishItemProperties::GetCustomProperty(LPCWSTR pszKey) const
{
    auto it = m_customProperties.find(pszKey);
    if (it != m_customProperties.end())
        return it->second;
    return L"";
}

// ============================================================================
// PublishItemPropertyStore implementation
// ============================================================================

PublishItemPropertyStore::PublishItemPropertyStore()
{
}

PublishItemPropertyStore::~PublishItemPropertyStore()
{
    m_entries.clear();
}

HRESULT PublishItemPropertyStore::Save(PublishItemProperties* pProperties)
{
    if (!pProperties)
        return E_POINTER;

    PublishServiceType type = pProperties->GetServiceType();

    // Update existing or add new
    for (auto& entry : m_entries)
    {
        if (entry.serviceType == type)
        {
            entry.properties = *pProperties;
            return S_OK;
        }
    }

    StoreEntry entry;
    entry.serviceType = type;
    entry.properties = *pProperties;
    m_entries.push_back(entry);
    return S_OK;
}

HRESULT PublishItemPropertyStore::Load(PublishItemProperties* pOutProperties, PublishServiceType type)
{
    if (!pOutProperties)
        return E_POINTER;

    for (const auto& entry : m_entries)
    {
        if (entry.serviceType == type)
        {
            *pOutProperties = entry.properties;
            return S_OK;
        }
    }
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT PublishItemPropertyStore::Delete(PublishServiceType type)
{
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        if (it->serviceType == type)
        {
            m_entries.erase(it);
            return S_OK;
        }
    }
    return S_FALSE;
}

size_t PublishItemPropertyStore::GetStoredServiceCount() const throw()
{
    return m_entries.size();
}

PublishServiceType PublishItemPropertyStore::GetStoredServiceAt(size_t nIndex) const
{
    if (nIndex < m_entries.size())
        return m_entries[nIndex].serviceType;
    return PublishServiceTypeLocal;
}

HRESULT PublishItemPropertyStore::SaveToRegistry(LPCWSTR pszRegistryKey)
{
    if (!pszRegistryKey)
        return E_POINTER;

    // In the full implementation, this would save all entries to
    // the Windows registry under HKCU\Software\Microsoft\WL\MovieMaker

    UNREFERENCED_PARAMETER(pszRegistryKey);
    return S_OK;
}

HRESULT PublishItemPropertyStore::LoadFromRegistry(LPCWSTR pszRegistryKey)
{
    if (!pszRegistryKey)
        return E_POINTER;

    m_entries.clear();
    return S_OK;
}

PublishItemProperties* PublishItemPropertyStore::GetDefaultProperties(PublishServiceType type)
{
    PublishItemProperties* pProps = new PublishItemProperties();
    pProps->SetServiceType(type);

    switch (type)
    {
    case PublishServiceTypeYouTube:
        pProps->SetDisplayName(L"YouTube");
        pProps->SetMaxVideoWidth(1920);
        pProps->SetMaxVideoHeight(1080);
        pProps->SetMaxFileSizeMB(2048);
        pProps->SetMaxDurationSec(3600);
        break;

    case PublishServiceTypeFacebook:
        pProps->SetDisplayName(L"Facebook");
        pProps->SetMaxVideoWidth(1280);
        pProps->SetMaxVideoHeight(720);
        pProps->SetMaxFileSizeMB(1024);
        pProps->SetMaxDurationSec(2400);
        break;

    case PublishServiceTypeSkyDrive:
        pProps->SetDisplayName(L"SkyDrive");
        pProps->SetMaxVideoWidth(1920);
        pProps->SetMaxVideoHeight(1080);
        pProps->SetMaxFileSizeMB(4096);
        pProps->SetMaxDurationSec(7200);
        break;

    default:
        pProps->SetDisplayName(L"Local file");
        break;
    }

    return pProps;
}
