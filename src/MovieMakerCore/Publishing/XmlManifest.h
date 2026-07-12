/*
 * XmlManifest.h
 *
 * XmlManifestFactory: Base XML manifest factory.
 * PublishSessionXmlManifestFactory: Generate XML manifest for publish session.
 * SharedBitmap: Shared bitmap wrapper for publish thumbnails.
 *
 * RTTI: ?AVXmlManifestFactory@@, ?AVPublishSessionXmlManifestFactory@@,
 *       ?AVSharedBitmap@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef XML_MANIFEST_H
#define XML_MANIFEST_H

#include "../pch.h"

// ============================================================================
// SharedBitmap
// ============================================================================
// Wraps a Gdiplus::Bitmap with shared ownership semantics. Used to share
// thumbnail bitmaps between the publish UI and the manifest generator
// without duplicating memory. Uses reference counting for safe sharing.
//
class SharedBitmap
{
public:
    SharedBitmap();
    explicit SharedBitmap(Gdiplus::Bitmap* pBitmap);
    SharedBitmap(const SharedBitmap& other);
    SharedBitmap& operator=(const SharedBitmap& other);
    ~SharedBitmap();

    // -- Access --
    Gdiplus::Bitmap* GetBitmap() const throw();
    void SetBitmap(Gdiplus::Bitmap* pBitmap);

    // -- Validity --
    bool IsValid() const throw();

    // -- Dimensions --
    UINT GetWidth() const throw();
    UINT GetHeight() const throw();

private:
    void AddRef();
    void Release();

    Gdiplus::Bitmap* m_pBitmap;
    long*            m_pRefCount;
};

// ============================================================================
// XmlManifestFactory
// ============================================================================
// Base class for XML manifest factories. Generates XML documents describing
// publish jobs, media metadata, and session information using IXmlWriter.
// Used by the publish pipeline to create service-specific manifests.
//
class XmlManifestFactory
{
public:
    XmlManifestFactory();
    virtual ~XmlManifestFactory();

    // -- Manifest generation --
    virtual HRESULT Generate(IStream* pOutputStream) = 0;
    virtual HRESULT GenerateToFile(LPCWSTR pszFilePath);

    // -- Configuration --
    void SetEncoding(LPCWSTR pszEncoding);
    ATL::CString GetEncoding() const;

    void SetIndent(bool bIndent);
    bool IsIndentEnabled() const throw();

    // -- Root element --
    void SetRootElementName(LPCWSTR pszName);
    ATL::CString GetRootElementName() const;

    // -- Namespace --
    void SetDefaultNamespace(LPCWSTR pszNamespace);
    ATL::CString GetDefaultNamespace() const;

    // -- Attributes --
    void AddAttribute(LPCWSTR pszName, LPCWSTR pszValue);
    void ClearAttributes();

protected:
    HRESULT InitializeWriter(IXmlWriter** ppWriter, IStream* pOutputStream);

    ATL::CString m_strEncoding;
    bool         m_bIndent;
    ATL::CString m_strRootElementName;
    ATL::CString m_strDefaultNamespace;

    std::map<ATL::CString, ATL::CString> m_attributes;
};

// ============================================================================
// PublishSessionXmlManifestFactory
// ============================================================================
// Generates the XML manifest for a publish session. The manifest includes
// job identifiers, service endpoints, media metadata, upload progress, and
// result URLs. Used by the publish service to create session records.
//
class PublishSessionXmlManifestFactory : public XmlManifestFactory
{
public:
    PublishSessionXmlManifestFactory();
    ~PublishSessionXmlManifestFactory();

    // -- Manifest generation --
    HRESULT Generate(IStream* pOutputStream) override;

    // -- Session info --
    void SetSessionId(LPCWSTR pszSessionId);
    ATL::CString GetSessionId() const;

    void SetStartTime(const FILETIME& ftStart);
    FILETIME GetStartTime() const throw();

    void SetEndTime(const FILETIME& ftEnd);
    FILETIME GetEndTime() const throw();

    // -- Thumbnail --
    void SetThumbnail(const SharedBitmap& thumbnail);
    SharedBitmap GetThumbnail() const;

    // -- Jobs --
    void AddJob(DWORD dwJobId, LPCWSTR pszJobName, LPCWSTR pszServiceName,
                HRESULT hrResult, LPCWSTR pszResultUrl);
    size_t GetJobCount() const throw();

    // -- Summary --
    void SetTotalJobs(DWORD dwTotal);
    void SetSuccessfulJobs(DWORD dwSuccessful);
    void SetFailedJobs(DWORD dwFailed);

private:
    struct ManifestJobEntry
    {
        DWORD       dwJobId;
        ATL::CString strJobName;
        ATL::CString strServiceName;
        HRESULT     hrResult;
        ATL::CString strResultUrl;
    };

    ATL::CString               m_strSessionId;
    FILETIME                   m_ftStartTime;
    FILETIME                   m_ftEndTime;
    SharedBitmap               m_thumbnail;
    std::vector<ManifestJobEntry> m_jobs;
    DWORD                      m_dwTotalJobs;
    DWORD                      m_dwSuccessfulJobs;
    DWORD                      m_dwFailedJobs;
};

#endif // XML_MANIFEST_H
