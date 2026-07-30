#include "pch.h"
/*
 * SerializationReader.cpp
 *
 * Implementation of the .wlmp project file reader using XmlLite.
 * Handles forward-compatible XML parsing with element handler dispatch
 * and version migration support.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "SerializationReader.h"
#include "../MovieProject.h"
#include <strsafe.h>

namespace StoryboardManager
{

// ============================================================================
// SerializationElementHandler implementation
// ============================================================================

SerializationElementHandler::SerializationElementHandler()
{
}

SerializationElementHandler::~SerializationElementHandler()
{
}

ATL::CString SerializationElementHandler::GetElementName() const
{
    return m_strElementName;
}

void SerializationElementHandler::SetElementName(LPCWSTR pszName)
{
    m_strElementName = pszName ? pszName : L"";
}

// ============================================================================
// SerializationReader construction / destruction
// ============================================================================

SerializationReader::SerializationReader()
    : m_hrLast(S_OK)
    , m_dwMajorVersion(1)
    , m_dwMinorVersion(0)
{
}

SerializationReader::~SerializationReader()
{
    UnregisterAllHandlers();
}

// ============================================================================
// ReadFromFile
// ============================================================================

HRESULT SerializationReader::ReadFromFile(LPCWSTR pszFilePath, MovieProject* pProject)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    if (!pProject)
        return E_POINTER;

    if (!::PathFileExistsW(pszFilePath))
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    IStream* pStream = nullptr;
    HRESULT hr = SHCreateStreamOnFileW(pszFilePath, STGM_READ, &pStream);
    if (FAILED(hr))
        return hr;

    hr = ReadFromStream(pStream, pProject);
    pStream->Release();

    return hr;
}

// ============================================================================
// ReadFromStream
// ============================================================================

HRESULT SerializationReader::ReadFromStream(IStream* pStream, MovieProject* pProject)
{
    if (!pStream)
        return E_POINTER;

    if (!pProject)
        return E_POINTER;

    IXmlReader* pReader = nullptr;
    HRESULT hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), nullptr);
    if (FAILED(hr))
        return hr;

    hr = pReader->SetInput(pStream);
    if (SUCCEEDED(hr))
    {
        hr = ParseDocument(pReader, pProject);
    }

    pReader->Release();
    m_hrLast = hr;
    return hr;
}

// ============================================================================
// ReadFromMemory
// ============================================================================

HRESULT SerializationReader::ReadFromMemory(const BYTE* pcbData, DWORD cbData, MovieProject* pProject)
{
    if (!pcbData || cbData == 0)
        return E_INVALIDARG;

    if (!pProject)
        return E_POINTER;

    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, cbData);
    if (!hGlobal)
        return E_OUTOFMEMORY;

    void* pGlobalData = GlobalLock(hGlobal);
    if (!pGlobalData)
    {
        GlobalFree(hGlobal);
        return E_OUTOFMEMORY;
    }

    CopyMemory(pGlobalData, pcbData, cbData);
    GlobalUnlock(hGlobal);

    IStream* pStream = nullptr;
    HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
    if (FAILED(hr))
    {
        GlobalFree(hGlobal);
        return hr;
    }

    hr = ReadFromStream(pStream, pProject);
    pStream->Release();

    return hr;
}

// ============================================================================
// Handler registration
// ============================================================================

void SerializationReader::RegisterHandler(LPCWSTR pszElementName,
                                          SerializationElementHandler* pHandler)
{
    if (!pszElementName || !pHandler)
        return;

    pHandler->SetElementName(pszElementName);
    m_arrHandlers.Add(pHandler);
}

void SerializationReader::UnregisterHandler(LPCWSTR pszElementName)
{
    if (!pszElementName)
        return;

    for (size_t i = 0; i < m_arrHandlers.GetCount(); ++i)
    {
        SerializationElementHandler* pHandler = m_arrHandlers.GetAt(i);
        if (pHandler && pHandler->GetElementName().CompareNoCase(pszElementName) == 0)
        {
            delete pHandler;
            m_arrHandlers.RemoveAt(i);
            return;
        }
    }
}

void SerializationReader::UnregisterAllHandlers()
{
    for (size_t i = 0; i < m_arrHandlers.GetCount(); ++i)
    {
        delete m_arrHandlers.GetAt(i);
    }
    m_arrHandlers.RemoveAll();
}

// ============================================================================
// Version support
// ============================================================================

bool SerializationReader::IsVersionSupported(const SerializationVersion& ver) const throw()
{
    static const SerializationVersion kMinVersion(1, 0);
    static const SerializationVersion kMaxVersion(2, 0);
    // Accept only [1.0, 2.0) inclusive lower bound, exclusive upper bound
    return (ver >= kMinVersion) && (ver < kMaxVersion);
}


HRESULT SerializationReader::MigrateVersion(SerializationContext& ctx)
{
    SerializationVersion ver = ctx.GetVersion();

    if (ver.dwMajor < 1)
    {
        ctx.SetHResult(E_FAIL);
        return E_FAIL;
    }

    if (ver.dwMajor == 1 && ver.dwMinor == 0)
    {
        ctx.SetVersion(1, 1);
    }

    return S_OK;
}

// ============================================================================
// Error handling
// ============================================================================

HRESULT SerializationReader::GetLastHResult() const throw()
{
    return m_hrLast;
}

ATL::CString SerializationReader::GetErrorDetails() const
{
    return m_strLastError;
}

// ============================================================================
// SkipCurrentElement
// ============================================================================

HRESULT SerializationReader::SkipCurrentElement(IXmlReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    DWORD dwDepth = 0;
    XmlNodeType nodeType;

    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            ++dwDepth;
        }
        else if (nodeType == XmlNodeTypeEndElement)
        {
            if (dwDepth == 0)
                return S_OK;
            --dwDepth;
        }
    }

    return S_OK;
}

// ============================================================================
// Attribute reading helpers (static)
// ============================================================================

ATL::CString SerializationReader::ParseXmlString(LPCWSTR pszInput)
{
    if (!pszInput)
        return ATL::CString();

    return ATL::CString(pszInput);
}

HRESULT SerializationReader::ReadIntAttribute(IXmlReader* pReader, LPCWSTR pszName,
                                              int* pValue)
{
    if (!pReader || !pszName || !pValue)
        return E_INVALIDARG;

    LPCWSTR pszValue = nullptr;
    HRESULT hr = XmlReaderGetAttribute(pReader, pszName, &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        *pValue = _wtoi(pszValue);
    }
    return hr;
}

HRESULT SerializationReader::ReadDwordAttribute(IXmlReader* pReader, LPCWSTR pszName,
                                                DWORD* pValue)
{
    if (!pReader || !pszName || !pValue)
        return E_INVALIDARG;

    LPCWSTR pszValue = nullptr;
    HRESULT hr = XmlReaderGetAttribute(pReader, pszName, &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        *pValue = _wtol(pszValue);
    }
    return hr;
}

HRESULT SerializationReader::ReadLONGLONGAttribute(IXmlReader* pReader, LPCWSTR pszName,
                                                   LONGLONG* pValue)
{
    if (!pReader || !pszName || !pValue)
        return E_INVALIDARG;

    LPCWSTR pszValue = nullptr;
    HRESULT hr = XmlReaderGetAttribute(pReader, pszName, &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        *pValue = _wtoi64(pszValue);
    }
    return hr;
}

HRESULT SerializationReader::ReadDoubleAttribute(IXmlReader* pReader, LPCWSTR pszName,
                                                 double* pValue)
{
    if (!pReader || !pszName || !pValue)
        return E_INVALIDARG;

    LPCWSTR pszValue = nullptr;
    HRESULT hr = XmlReaderGetAttribute(pReader, pszName, &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        *pValue = _wtof(pszValue);
    }
    return hr;
}

HRESULT SerializationReader::ReadStringAttribute(IXmlReader* pReader, LPCWSTR pszName,
                                                 ATL::CString& strValue)
{
    if (!pReader || !pszName)
        return E_INVALIDARG;

    LPCWSTR pszValue = nullptr;
    HRESULT hr = XmlReaderGetAttribute(pReader, pszName, &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        strValue = pszValue;
    }
    else
    {
        strValue.Empty();
    }
    return hr;
}

HRESULT SerializationReader::ReadBoolAttribute(IXmlReader* pReader, LPCWSTR pszName,
                                               bool* pValue)
{
    if (!pReader || !pszName || !pValue)
        return E_INVALIDARG;

    LPCWSTR pszValue = nullptr;
    HRESULT hr = XmlReaderGetAttribute(pReader, pszName, &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        *pValue = (_wtoi(pszValue) != 0);
    }
    return hr;
}

// ============================================================================
// Internal parsing
// ============================================================================

HRESULT SerializationReader::ParseDocument(IXmlReader* pReader, MovieProject* pProject)
{
    if (!pReader || !pProject)
        return E_POINTER;

    SerializationContext ctx = SerializationContext::CreateForRead();
    XmlNodeType nodeType;

    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            HRESULT hr = ParseElement(pReader, pProject, ctx);
            if (FAILED(hr))
                return hr;
        }
    }

    return ctx.GetHResult();
}

HRESULT SerializationReader::ParseElement(IXmlReader* pReader, MovieProject* pProject,
                                          SerializationContext& ctx)
{
    if (!pReader || !pProject)
        return E_POINTER;

    LPCWSTR pszLocalName = nullptr;
    HRESULT hr = pReader->GetLocalName(&pszLocalName, nullptr);
    if (FAILED(hr) || !pszLocalName)
        return S_OK;

    ctx.IncrementDepth();

    if (ctx.GetElementDepth() == 1 && wcscmp(pszLocalName, L"project") == 0)
    {
        hr = DetectVersion(pReader, ctx);
        if (FAILED(hr))
            return hr;

        if (!IsVersionSupported(ctx.GetVersion()))
        {
            hr = MigrateVersion(ctx);
            if (FAILED(hr))
                return hr;
        }

        ParseElementAttributes(pReader, nullptr, ctx);
    }

    SerializationElementHandler* pHandler = FindHandler(pszLocalName);
    if (pHandler)
    {
        ParseElementAttributes(pReader, pHandler, ctx);
        hr = pHandler->OnBeginElement(pszLocalName, ctx);
    }
    else if (ctx.GetElementDepth() > 1)
    {
        SkipCurrentElement(pReader);
        ctx.DecrementDepth();
        return S_OK;
    }

    if (SUCCEEDED(hr))
    {
        XmlNodeType nodeType;
        while (pReader->Read(&nodeType) == S_OK)
        {
            if (nodeType == XmlNodeTypeEndElement)
            {
                ctx.DecrementDepth();

                if (pHandler)
                {
                    hr = pHandler->OnEndElement(pszLocalName, ctx);
                }
                return hr;
            }

            if (nodeType == XmlNodeType_Element)
            {
                hr = ParseElement(pReader, pProject, ctx);
                if (FAILED(hr))
                    return hr;
            }

            if (nodeType == XmlNodeType_Text && pHandler)
            {
                LPCWSTR pszText = nullptr;
                pReader->GetValue(&pszText, nullptr);
                if (pszText)
                {
                    hr = pHandler->OnText(pszText, ctx);
                }
            }
        }
    }

    ctx.DecrementDepth();
    return hr;
}

HRESULT SerializationReader::ParseElementAttributes(IXmlReader* pReader,
                                                     SerializationElementHandler* pHandler,
                                                     SerializationContext& ctx)
{
    UNREFERENCED_PARAMETER(ctx);

    if (!pReader)
        return E_POINTER;

    HRESULT hr = S_OK;
    const WCHAR* pwszAttrName = nullptr;
    const WCHAR* pwszAttrValue = nullptr;

    while (pReader->MoveToNextAttribute() == S_OK)
    {
        hr = pReader->GetLocalName(&pwszAttrName, nullptr);
        if (FAILED(hr) || !pwszAttrName)
            continue;

        hr = pReader->GetValue(&pwszAttrValue, nullptr);
        if (FAILED(hr) || !pwszAttrValue)
            continue;

        if (pHandler)
        {
            hr = pHandler->OnAttribute(pwszAttrName, pwszAttrValue, ctx);
            if (FAILED(hr))
                break;
        }
    }

    pReader->MoveToElement();
    return hr;
}

HRESULT SerializationReader::SkipEndElement(IXmlReader* pReader, DWORD dwDepth)
{
    if (!pReader)
        return E_POINTER;

    DWORD dwCurrent = 0;
    XmlNodeType nodeType;

    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            ++dwCurrent;
        }
        else if (nodeType == XmlNodeTypeEndElement)
        {
            if (dwCurrent == 0)
                return S_OK;
            --dwCurrent;
            if (dwCurrent < dwDepth)
                return S_OK;
        }
    }

    return S_OK;
}

HRESULT SerializationReader::DetectVersion(IXmlReader* pReader, SerializationContext& ctx)
{
    LPCWSTR pszValue = nullptr;

    HRESULT hr = XmlReaderGetAttribute(pReader, L"versionMajor", &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        m_dwMajorVersion = _wtol(pszValue);
    }

    hr = XmlReaderGetAttribute(pReader, L"versionMinor", &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        m_dwMinorVersion = _wtol(pszValue);
    }

    ctx.SetVersion(m_dwMajorVersion, m_dwMinorVersion);

    return S_OK;
}

SerializationElementHandler* SerializationReader::FindHandler(LPCWSTR pszElementName)
{
    if (!pszElementName)
        return nullptr;

    for (size_t i = 0; i < m_arrHandlers.GetCount(); ++i)
    {
        SerializationElementHandler* pHandler = m_arrHandlers.GetAt(i);
        if (pHandler && pHandler->GetElementName().CompareNoCase(pszElementName) == 0)
        {
            return pHandler;
        }
    }

    return nullptr;
}

// ============================================================================
// Legacy project reading
// ============================================================================

HRESULT SerializationReader::LoadProject(LPCWSTR pszFilePath, MovieProject* pProject)
{
    return ReadFromFile(pszFilePath, pProject);
}

HRESULT SerializationReader::ReadProject(LPCWSTR pszFilePath, MovieProject* pProject)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    if (!pProject)
        return E_POINTER;

    if (!::PathFileExistsW(pszFilePath))
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    IStream* pStream = nullptr;
    HRESULT hr = SHCreateStreamOnFileW(pszFilePath, STGM_READ, &pStream);
    if (FAILED(hr))
        return hr;

    IXmlReader* pReader = nullptr;
    hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), nullptr);
    if (FAILED(hr))
    {
        pStream->Release();
        return hr;
    }

    hr = pReader->SetInput(pStream);
    if (SUCCEEDED(hr))
    {
        XmlNodeType nodeType;
        while (pReader->Read(&nodeType) == S_OK)
        {
            if (nodeType == XmlNodeType_Element)
            {
                LPCWSTR pszLocalName = nullptr;
                hr = pReader->GetLocalName(&pszLocalName, nullptr);
                if (FAILED(hr) || !pszLocalName)
                    continue;

                if (wcscmp(pszLocalName, L"project") == 0)
                {
                    LPCWSTR pszValue = nullptr;

                    hr = XmlReaderGetAttribute(pReader, L"name", &pszValue);
                    if (SUCCEEDED(hr) && pszValue)
                    {
                        pProject->GetSettings().SetProjectName(pszValue);
                    }
                }
                else if (wcscmp(pszLocalName, L"media") == 0)
                {
                    hr = ReadMediaItems(pReader, pProject);
                }
                else if (wcscmp(pszLocalName, L"settings") == 0)
                {
                    hr = ReadProperties(pReader, pProject);
                }

                if (FAILED(hr))
                    break;
            }
        }
    }

    pReader->Release();
    pStream->Release();
    m_hrLast = hr;
    return hr;
}

HRESULT SerializationReader::ReadMediaItems(IXmlReader* pReader, MovieProject* pProject)
{
    if (!pReader || !pProject)
        return E_POINTER;

    HRESULT hr = S_OK;
    XmlNodeType nodeType;

    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_EndElement)
            break;

        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            hr = pReader->GetLocalName(&pszLocalName, nullptr);
            if (FAILED(hr) || !pszLocalName)
                continue;

            if (wcscmp(pszLocalName, L"mediaItem") == 0)
            {
                ProjectMediaItem item;

                LPCWSTR pszValue = nullptr;

                hr = XmlReaderGetAttribute(pReader, L"id", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                    item.SetMediaId(_wtol(pszValue));

                hr = XmlReaderGetAttribute(pReader, L"path", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                    item.SetSourcePath(pszValue);

                hr = XmlReaderGetAttribute(pReader, L"type", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                    item.SetMediaType(_wtol(pszValue));

                hr = XmlReaderGetAttribute(pReader, L"duration", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                    item.SetDurationHns(_wtoi64(pszValue));

                hr = XmlReaderGetAttribute(pReader, L"width", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                {
                    UINT cx = _wtol(pszValue);
                    hr = XmlReaderGetAttribute(pReader, L"height", &pszValue);
                    if (SUCCEEDED(hr) && pszValue)
                        item.SetDimensions(cx, _wtol(pszValue));
                }

                hr = XmlReaderGetAttribute(pReader, L"frameRate", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                    item.SetFrameRate(_wtol(pszValue));

                pProject->AddMediaItem(item);
            }
        }
    }

    return S_OK;
}

HRESULT SerializationReader::ReadProperties(IXmlReader* pReader, MovieProject* pProject)
{
    if (!pReader || !pProject)
        return E_POINTER;

    HRESULT hr = S_OK;
    MovieProjectSettings& settings = pProject->GetSettings();

    LPCWSTR pszValue = nullptr;

    hr = XmlReaderGetAttribute(pReader, L"outputWidth", &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        UINT cx = _wtol(pszValue);
        hr = XmlReaderGetAttribute(pReader, L"outputHeight", &pszValue);
        if (SUCCEEDED(hr) && pszValue)
            settings.SetOutputDimensions(cx, _wtol(pszValue));
    }

    hr = XmlReaderGetAttribute(pReader, L"aspectRatio", &pszValue);
    if (SUCCEEDED(hr) && pszValue)
        settings.SetAspectRatio(_wtof(pszValue));

    hr = XmlReaderGetAttribute(pReader, L"audioBitRate", &pszValue);
    if (SUCCEEDED(hr) && pszValue)
        settings.SetAudioBitRate(_wtol(pszValue));

    hr = XmlReaderGetAttribute(pReader, L"videoBitRate", &pszValue);
    if (SUCCEEDED(hr) && pszValue)
        settings.SetVideoBitRate(_wtol(pszValue));

    hr = XmlReaderGetAttribute(pReader, L"frameRate", &pszValue);
    if (SUCCEEDED(hr) && pszValue)
        settings.SetFrameRate(_wtol(pszValue));

    return S_OK;
}

} // namespace StoryboardManager
