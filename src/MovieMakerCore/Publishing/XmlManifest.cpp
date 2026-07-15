#include "pch.h"

/*
 * XmlManifest.cpp
 *
 * Implementation of XmlManifestFactory, PublishSessionXmlManifestFactory,
 * and SharedBitmap.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "XmlManifest.h"

// ============================================================================
// SharedBitmap implementation
// ============================================================================

SharedBitmap::SharedBitmap()
    : m_pBitmap(nullptr)
    , m_pRefCount(nullptr)
{
}

SharedBitmap::SharedBitmap(Gdiplus::Bitmap* pBitmap)
    : m_pBitmap(pBitmap)
    , m_pRefCount(nullptr)
{
    if (m_pBitmap)
    {
        m_pRefCount = new long(1);
    }
}

SharedBitmap::SharedBitmap(const SharedBitmap& other)
    : m_pBitmap(other.m_pBitmap)
    , m_pRefCount(other.m_pRefCount)
{
    AddRef();
}

SharedBitmap& SharedBitmap::operator=(const SharedBitmap& other)
{
    if (this != &other)
    {
        Release();
        m_pBitmap = other.m_pBitmap;
        m_pRefCount = other.m_pRefCount;
        AddRef();
    }
    return *this;
}

SharedBitmap::~SharedBitmap()
{
    Release();
}

Gdiplus::Bitmap* SharedBitmap::GetBitmap() const throw()
{
    return m_pBitmap;
}

void SharedBitmap::SetBitmap(Gdiplus::Bitmap* pBitmap)
{
    Release();
    m_pBitmap = pBitmap;
    if (m_pBitmap)
    {
        m_pRefCount = new long(1);
    }
    else
    {
        m_pRefCount = nullptr;
    }
}

bool SharedBitmap::IsValid() const throw()
{
    return (m_pBitmap != nullptr);
}

UINT SharedBitmap::GetWidth() const throw()
{
    return m_pBitmap ? m_pBitmap->GetWidth() : 0;
}

UINT SharedBitmap::GetHeight() const throw()
{
    return m_pBitmap ? m_pBitmap->GetHeight() : 0;
}

void SharedBitmap::AddRef()
{
    if (m_pRefCount)
        InterlockedIncrement(m_pRefCount);
}

void SharedBitmap::Release()
{
    if (m_pRefCount)
    {
        long refCount = InterlockedDecrement(m_pRefCount);
        if (refCount <= 0)
        {
            delete m_pBitmap;
            delete m_pRefCount;
            m_pBitmap = nullptr;
            m_pRefCount = nullptr;
        }
    }
}

// ============================================================================
// XmlManifestFactory implementation
// ============================================================================

XmlManifestFactory::XmlManifestFactory()
    : m_bIndent(true)
{
    m_strEncoding = L"utf-8";
    m_strRootElementName = L"manifest";
}

XmlManifestFactory::~XmlManifestFactory()
{
}

HRESULT XmlManifestFactory::GenerateToFile(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return E_POINTER;

    ATL::CComPtr<IStream> spStream;
    HRESULT hr = SHCreateStreamOnFileW(pszFilePath, STGM_CREATE | STGM_WRITE, &spStream);
    if (FAILED(hr))
        return hr;

    return Generate(spStream);
}

void XmlManifestFactory::SetEncoding(LPCWSTR pszEncoding)
{
    m_strEncoding = pszEncoding ? pszEncoding : L"utf-8";
}

ATL::CString XmlManifestFactory::GetEncoding() const
{
    return m_strEncoding;
}

void XmlManifestFactory::SetIndent(bool bIndent)
{
    m_bIndent = bIndent;
}

bool XmlManifestFactory::IsIndentEnabled() const throw()
{
    return m_bIndent;
}

void XmlManifestFactory::SetRootElementName(LPCWSTR pszName)
{
    m_strRootElementName = pszName ? pszName : L"manifest";
}

ATL::CString XmlManifestFactory::GetRootElementName() const
{
    return m_strRootElementName;
}

void XmlManifestFactory::SetDefaultNamespace(LPCWSTR pszNamespace)
{
    m_strDefaultNamespace = pszNamespace ? pszNamespace : L"";
}

ATL::CString XmlManifestFactory::GetDefaultNamespace() const
{
    return m_strDefaultNamespace;
}

void XmlManifestFactory::AddAttribute(LPCWSTR pszName, LPCWSTR pszValue)
{
    if (pszName)
        m_attributes[pszName] = pszValue ? pszValue : L"";
}

void XmlManifestFactory::ClearAttributes()
{
    m_attributes.clear();
}

HRESULT XmlManifestFactory::InitializeWriter(IXmlWriter** ppWriter, IStream* pOutputStream)
{
    if (!ppWriter || !pOutputStream)
        return E_POINTER;

    HRESULT hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(ppWriter), nullptr);
    if (FAILED(hr))
        return hr;

    hr = (*ppWriter)->SetOutput(pOutputStream);
    if (FAILED(hr))
    {
        (*ppWriter)->Release();
        *ppWriter = nullptr;
        return hr;
    }

    // Write XML declaration
    hr = (*ppWriter)->WriteStartDocument(XmlStandalone_Omit);
    if (FAILED(hr))
        return hr;

    if (m_bIndent)
    {
        hr = (*ppWriter)->SetProperty(XmlWriterProperty_Indent, TRUE);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

// ============================================================================
// PublishSessionXmlManifestFactory implementation
// ============================================================================

PublishSessionXmlManifestFactory::PublishSessionXmlManifestFactory()
    : XmlManifestFactory()
    , m_dwTotalJobs(0)
    , m_dwSuccessfulJobs(0)
    , m_dwFailedJobs(0)
{
    m_strRootElementName = L"publishSession";
    ZeroMemory(&m_ftStartTime, sizeof(FILETIME));
    ZeroMemory(&m_ftEndTime, sizeof(FILETIME));
}

PublishSessionXmlManifestFactory::~PublishSessionXmlManifestFactory()
{
}

HRESULT PublishSessionXmlManifestFactory::Generate(IStream* pOutputStream)
{
    if (!pOutputStream)
        return E_POINTER;

    ATL::CComPtr<IXmlWriter> spWriter;
    HRESULT hr = InitializeWriter(&spWriter, pOutputStream);
    if (FAILED(hr))
        return hr;

    // Write root element with default namespace
    if (!m_strDefaultNamespace.IsEmpty())
        spWriter->WriteAttributeString(L"xmlns", m_strDefaultNamespace);

    // Write session attributes
    if (!m_strSessionId.IsEmpty())
        spWriter->WriteAttributeString(L"sessionId", m_strSessionId);

    SYSTEMTIME st;
    WCHAR szTime[64];

    // Start time
    FileTimeToSystemTime(&m_ftStartTime, &st);
    StringCchPrintfW(szTime, ARRAYSIZE(szTime),
                     L"%04u-%02u-%02uT%02u:%02u:%02uZ",
                     st.wYear, st.wMonth, st.wDay,
                     st.wHour, st.wMinute, st.wSecond);
    spWriter->WriteAttributeString(L"startTime", szTime);

    // End time
    FileTimeToSystemTime(&m_ftEndTime, &st);
    StringCchPrintfW(szTime, ARRAYSIZE(szTime),
                     L"%04u-%02u-%02uT%02u:%02u:%02uZ",
                     st.wYear, st.wMonth, st.wDay,
                     st.wHour, st.wMinute, st.wSecond);
    spWriter->WriteAttributeString(L"endTime", szTime);

    // Summary
    spWriter->WriteStartElement(L"summary");
    spWriter->WriteAttributeString(L"total", L"total");
    WCHAR szNum[32];
    StringCchPrintfW(szNum, ARRAYSIZE(szNum), L"%u", m_dwTotalJobs);
    spWriter->WriteAttributeString(L"total", szNum);
    StringCchPrintfW(szNum, ARRAYSIZE(szNum), L"%u", m_dwSuccessfulJobs);
    spWriter->WriteAttributeString(L"successful", szNum);
    StringCchPrintfW(szNum, ARRAYSIZE(szNum), L"%u", m_dwFailedJobs);
    spWriter->WriteAttributeString(L"failed", szNum);
    spWriter->WriteEndElement();

    // Job entries
    for (const auto& job : m_jobs)
    {
        spWriter->WriteStartElement(L"job");

        StringCchPrintfW(szNum, ARRAYSIZE(szNum), L"%u", job.dwJobId);
        spWriter->WriteAttributeString(L"id", szNum);
        spWriter->WriteAttributeString(L"name", job.strJobName);
        spWriter->WriteAttributeString(L"service", job.strServiceName);

        if (SUCCEEDED(job.hrResult))
            spWriter->WriteAttributeString(L"result", L"success");
        else
            spWriter->WriteAttributeString(L"result", L"failed");

        if (!job.strResultUrl.IsEmpty())
            spWriter->WriteAttributeString(L"url", job.strResultUrl);

        spWriter->WriteEndElement();
    }

    spWriter->WriteEndDocument();
    spWriter->Flush();

    return S_OK;
}

void PublishSessionXmlManifestFactory::SetSessionId(LPCWSTR pszSessionId)
{
    m_strSessionId = pszSessionId ? pszSessionId : L"";
}

ATL::CString PublishSessionXmlManifestFactory::GetSessionId() const
{
    return m_strSessionId;
}

void PublishSessionXmlManifestFactory::SetStartTime(const FILETIME& ftStart)
{
    m_ftStartTime = ftStart;
}

FILETIME PublishSessionXmlManifestFactory::GetStartTime() const throw()
{
    return m_ftStartTime;
}

void PublishSessionXmlManifestFactory::SetEndTime(const FILETIME& ftEnd)
{
    m_ftEndTime = ftEnd;
}

FILETIME PublishSessionXmlManifestFactory::GetEndTime() const throw()
{
    return m_ftEndTime;
}

void PublishSessionXmlManifestFactory::SetThumbnail(const SharedBitmap& thumbnail)
{
    m_thumbnail = thumbnail;
}

SharedBitmap PublishSessionXmlManifestFactory::GetThumbnail() const
{
    return m_thumbnail;
}

void PublishSessionXmlManifestFactory::AddJob(
    DWORD dwJobId, LPCWSTR pszJobName, LPCWSTR pszServiceName,
    HRESULT hrResult, LPCWSTR pszResultUrl)
{
    ManifestJobEntry entry;
    entry.dwJobId = dwJobId;
    entry.strJobName = pszJobName ? pszJobName : L"";
    entry.strServiceName = pszServiceName ? pszServiceName : L"";
    entry.hrResult = hrResult;
    entry.strResultUrl = pszResultUrl ? pszResultUrl : L"";
    m_jobs.push_back(entry);
}

size_t PublishSessionXmlManifestFactory::GetJobCount() const throw()
{
    return m_jobs.size();
}

void PublishSessionXmlManifestFactory::SetTotalJobs(DWORD dwTotal)
{
    m_dwTotalJobs = dwTotal;
}

void PublishSessionXmlManifestFactory::SetSuccessfulJobs(DWORD dwSuccessful)
{
    m_dwSuccessfulJobs = dwSuccessful;
}

void PublishSessionXmlManifestFactory::SetFailedJobs(DWORD dwFailed)
{
    m_dwFailedJobs = dwFailed;
}
