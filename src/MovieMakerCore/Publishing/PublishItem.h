/*
 * PublishItem.h
 *
 * PublishItemProperties: Properties for a publish target (Facebook, YouTube, etc.).
 * PublishItemPropertyStore: Persistent property store for publish settings.
 *
 * RTTI: ?AVPublishItemProperties@@, ?AVPublishItemPropertyStore@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef PUBLISH_ITEM_H
#define PUBLISH_ITEM_H

#include "../pch.h"

// ============================================================================
// PublishServiceType enum
// ============================================================================
enum PublishServiceType
{
    PublishServiceTypeLocal        = 0,
    PublishServiceTypeYouTube      = 1,
    PublishServiceTypeFacebook     = 2,
    PublishServiceTypeSkyDrive     = 3,
    PublishServiceTypeVimeo        = 4,
    PublishServiceTypeCustom       = 5
};

// ============================================================================
// PublishItemProperties
// ============================================================================
// Properties for a specific publish target. Stores the service-specific
// settings required to publish a movie (authentication tokens, upload
// URLs, privacy settings, formatting options).
//
class PublishItemProperties
{
public:
    PublishItemProperties();
    ~PublishItemProperties();

    // -- Identity --
    void SetServiceType(PublishServiceType type);
    PublishServiceType GetServiceType() const throw();

    void SetDisplayName(LPCWSTR pszName);
    ATL::CString GetDisplayName() const;

    void SetServiceUrl(LPCWSTR pszUrl);
    ATL::CString GetServiceUrl() const;

    // -- Authentication --
    void SetAuthToken(LPCWSTR pszToken);
    ATL::CString GetAuthToken() const;

    void SetAuthSecret(LPCWSTR pszSecret);
    ATL::CString GetAuthSecret() const;

    // -- Upload settings --
    void SetTitle(LPCWSTR pszTitle);
    ATL::CString GetTitle() const;

    void SetDescription(LPCWSTR pszDescription);
    ATL::CString GetDescription() const;

    void SetTags(LPCWSTR pszTags);
    ATL::CString GetTags() const;

    void SetPrivacyPublic(bool bPublic);
    bool IsPrivacyPublic() const throw();

    void SetAllowComments(bool bAllow);
    bool IsCommentsAllowed() const throw();

    void SetAllowEmbedding(bool bAllow);
    bool IsEmbeddingAllowed() const throw();

    // -- Formatting --
    void SetMaxVideoWidth(UINT uWidth);
    UINT GetMaxVideoWidth() const throw();

    void SetMaxVideoHeight(UINT uHeight);
    UINT GetMaxVideoHeight() const throw();

    void SetMaxFileSizeMB(DWORD dwSizeMB);
    DWORD GetMaxFileSizeMB() const throw();

    void SetMaxDurationSec(DWORD dwDurationSec);
    DWORD GetMaxDurationSec() const throw();

    // -- Category --
    void SetCategory(LPCWSTR pszCategory);
    ATL::CString GetCategory() const;

    // -- Custom property --
    void SetCustomProperty(LPCWSTR pszKey, LPCWSTR pszValue);
    ATL::CString GetCustomProperty(LPCWSTR pszKey) const;

private:
    PublishServiceType m_serviceType;
    ATL::CString       m_strDisplayName;
    ATL::CString       m_strServiceUrl;
    ATL::CString       m_strAuthToken;
    ATL::CString       m_strAuthSecret;
    ATL::CString       m_strTitle;
    ATL::CString       m_strDescription;
    ATL::CString       m_strTags;
    bool               m_bPrivacyPublic;
    bool               m_bAllowComments;
    bool               m_bAllowEmbedding;
    UINT               m_uMaxVideoWidth;
    UINT               m_uMaxVideoHeight;
    DWORD              m_dwMaxFileSizeMB;
    DWORD              m_dwMaxDurationSec;
    ATL::CString       m_strCategory;

    std::map<ATL::CString, ATL::CString> m_customProperties;
};

// ============================================================================
// PublishItemPropertyStore
// ============================================================================
// Persistent property store for publish settings. Saves and loads
// publish target properties to/from the Windows registry or local
// config file. Provides a mapping of service types to stored settings.
//
class PublishItemPropertyStore
{
public:
    PublishItemPropertyStore();
    ~PublishItemPropertyStore();

    // -- Store management --
    HRESULT Save(PublishItemProperties* pProperties);
    HRESULT Load(PublishItemProperties* pOutProperties, PublishServiceType type);
    HRESULT Delete(PublishServiceType type);

    // -- Enumeration --
    size_t GetStoredServiceCount() const throw();
    PublishServiceType GetStoredServiceAt(size_t nIndex) const;

    // -- Persistence --
    HRESULT SaveToRegistry(LPCWSTR pszRegistryKey);
    HRESULT LoadFromRegistry(LPCWSTR pszRegistryKey);

    // -- Defaults --
    static PublishItemProperties* GetDefaultProperties(PublishServiceType type);

private:
    struct StoreEntry
    {
        PublishServiceType serviceType;
        PublishItemProperties properties;
    };

    std::vector<StoreEntry> m_entries;
};

#endif // PUBLISH_ITEM_H
