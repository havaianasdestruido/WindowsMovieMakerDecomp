#include "pch.h"

/*
 * LegacyProject.cpp
 *
 * Implementation of LegacyProjectSupport for reading/writing older .wlmp
 * project file formats (Windows Live Movie Maker 2010/2011).
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "LegacyProject.h"
#include <shlwapi.h>

static HRESULT SkipCurrentElement(IXmlReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    DWORD dwDepth = 0;
    XmlNodeType nodeType;

    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
            ++dwDepth;
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
// LegacyProjectSupport implementation
// ============================================================================

LegacyProjectSupport::LegacyProjectSupport()
    : m_uVersionMajor(0)
    , m_uVersionMinor(0)
    , m_bIsLegacy(false)
{
    InitializePropertyMaps();
}

LegacyProjectSupport::~LegacyProjectSupport()
{
}

DWORD LegacyProjectSupport::GetRecognizedVersionMajor() const throw()
{
    return m_uVersionMajor;
}

DWORD LegacyProjectSupport::GetRecognizedVersionMinor() const throw()
{
    return m_uVersionMinor;
}

bool LegacyProjectSupport::IsVersion14() const
{
    return (m_uVersionMajor == 14);
}

bool LegacyProjectSupport::IsVersion15() const
{
    return (m_uVersionMajor == 15);
}

bool LegacyProjectSupport::IsLegacyProjectFormat(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return false;

    DWORD dwVersion = DetectProjectVersion(pszFilePath);
    return (dwVersion < 16);
}

bool LegacyProjectSupport::IsLegacyNamespace(IXmlReader* pReader)
{
    if (!pReader)
        return false;

    LPCWSTR pszNamespace = nullptr;
    if (SUCCEEDED(pReader->GetNamespaceUri(&pszNamespace, nullptr)))
    {
        if (pszNamespace)
        {
            if (wcscmp(pszNamespace, kLegacyNamespace2010) == 0 ||
                wcscmp(pszNamespace, kLegacyNamespace2011) == 0)
            {
                return true;
            }
        }
    }
    return false;
}

DWORD LegacyProjectSupport::DetectProjectVersion(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return 0;

    // Open the XML file and check the version attribute
    ATL::CComPtr<IXmlReader> spReader;
    ATL::CComPtr<IStream> spStream;

    if (FAILED(SHCreateStreamOnFileW(pszFilePath, STGM_READ, &spStream)))
        return 0;

    if (FAILED(CreateXmlReader(__uuidof(IXmlReader),
                                reinterpret_cast<void**>(&spReader), nullptr)))
    {
        return 0;
    }

    if (FAILED(spReader->SetInput(spStream)))
        return 0;

    // Read the root element and check attributes
    XmlNodeType nodeType;
    while (SUCCEEDED(spReader->Read(&nodeType)))
    {
        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            UINT cchName = 0;
            if (SUCCEEDED(spReader->GetLocalName(&pszLocalName, &cchName)))
            {
                if (pszLocalName && wcscmp(pszLocalName, L"project") == 0)
                {
                    // Read version attributes
                    LPCWSTR pszValue = nullptr;
                    UINT cchValue = 0;
                    DWORD dwVersionMajor = 0;
                    DWORD dwVersionMinor = 0;

                    // Move to first attribute
                    if (SUCCEEDED(spReader->MoveToFirstAttribute()))
                    {
                        do
                        {
                            LPCWSTR pszAttrName = nullptr;
                            UINT cchAttr = 0;
                            if (SUCCEEDED(spReader->GetLocalName(&pszAttrName, &cchAttr)))
                            {
                                if (pszAttrName && wcscmp(pszAttrName, L"versionMajor") == 0)
                                {
                                    if (SUCCEEDED(spReader->GetValue(&pszValue, &cchValue)))
                                    {
                                        dwVersionMajor = _wtoi(pszValue);
                                    }
                                }
                                else if (pszAttrName && wcscmp(pszAttrName, L"versionMinor") == 0)
                                {
                                    if (SUCCEEDED(spReader->GetValue(&pszValue, &cchValue)))
                                    {
                                        dwVersionMinor = _wtoi(pszValue);
                                    }
                                }
                            }
                        } while (SUCCEEDED(spReader->MoveToNextAttribute()));
                    }

                    if (dwVersionMajor >= 16)
                        return 16;
                    else if (dwVersionMajor == 15)
                        return 15;
                    else if (dwVersionMajor == 14)
                        return 14;
                    else
                        return dwVersionMajor;
                }
            }
        }
    }

    return 0;
}

HRESULT LegacyProjectSupport::ReadLegacyProject(
    LPCWSTR pszFilePath, StoryboardManager::MovieProject* pOutProject)
{
    if (!pszFilePath || !pOutProject)
        return E_POINTER;

    if (!::PathFileExistsW(pszFilePath))
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    DWORD dwVersion = DetectProjectVersion(pszFilePath);
    if (dwVersion == 0)
        return E_FAIL;

    m_uVersionMajor = dwVersion;
    m_bIsLegacy = true;
    m_mediaItems.clear();
    m_properties.clear();

    ATL::CComPtr<IStream> spStream;
    HRESULT hr = SHCreateStreamOnFileW(pszFilePath, STGM_READ, &spStream);
    if (FAILED(hr))
        return hr;

    ATL::CComPtr<IXmlReader> spReader;
    hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&spReader), nullptr);
    if (FAILED(hr))
        return hr;

    hr = spReader->SetInput(spStream);
    if (FAILED(hr))
        return hr;

    XmlNodeType nodeType;
    while (SUCCEEDED(spReader->Read(&nodeType)))
    {
        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            UINT cchName = 0;
            hr = spReader->GetLocalName(&pszLocalName, &cchName);
            if (FAILED(hr) || !pszLocalName)
                continue;

            if (wcscmp(pszLocalName, L"project") == 0)
            {
                // Already parsed version in DetectProjectVersion, read remaining attrs
                LPCWSTR pszAttrValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"name", &pszAttrValue)) && pszAttrValue)
                {
                    pOutProject->GetSettings().SetProjectName(pszAttrValue);
                }
            }
            else if (wcscmp(pszLocalName, L"media") == 0)
            {
                hr = ReadLegacyMediaItems(spReader, pOutProject);
                if (FAILED(hr))
                    return hr;
            }
            else if (wcscmp(pszLocalName, L"timeline") == 0)
            {
                hr = ReadLegacyTimeline(spReader, pOutProject);
                if (FAILED(hr))
                    return hr;
            }
            else if (wcscmp(pszLocalName, L"settings") == 0)
            {
                // Read project settings
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"outputWidth", &pszAttrValue)) && pszAttrValue)
                {
                    pOutProject->GetSettings().SetOutputDimensions(
                        static_cast<UINT>(_wtoi(pszAttrValue)),
                        pOutProject->GetSettings().GetOutputHeight());
                }
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"outputHeight", &pszAttrValue)) && pszAttrValue)
                {
                    pOutProject->GetSettings().SetOutputDimensions(
                        pOutProject->GetSettings().GetOutputWidth(),
                        static_cast<UINT>(_wtoi(pszAttrValue)));
                }
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"aspectRatio", &pszAttrValue)) && pszAttrValue)
                {
                    pOutProject->GetSettings().SetAspectRatio(_wtof(pszAttrValue));
                }
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"videoBitRate", &pszAttrValue)) && pszAttrValue)
                {
                    pOutProject->GetSettings().SetVideoBitRate(_wtol(pszAttrValue));
                }
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"audioBitRate", &pszAttrValue)) && pszAttrValue)
                {
                    pOutProject->GetSettings().SetAudioBitRate(_wtol(pszAttrValue));
                }
                if (SUCCEEDED(XmlReaderGetAttribute(spReader, L"frameRate", &pszAttrValue)) && pszAttrValue)
                {
                    pOutProject->GetSettings().SetFrameRate(_wtol(pszAttrValue));
                }
                SkipCurrentElement(spReader);
            }
            else
            {
                SkipCurrentElement(spReader);
            }
        }
    }

    // Convert legacy media items to current format
    for (size_t i = 0; i < m_mediaItems.size(); ++i)
    {
        StoryboardManager::ProjectMediaItem convertedItem;
        hr = ConvertLegacyMediaItem(m_mediaItems[i].strFilePath, &convertedItem);
        if (SUCCEEDED(hr))
        {
            convertedItem.SetMediaId(m_mediaItems[i].dwId);
            convertedItem.SetMediaType(m_mediaItems[i].dwMediaType);
            convertedItem.SetDurationHns(ConvertLegacyTimeToHns(static_cast<DWORD>(m_mediaItems[i].llDurationMs)));
            convertedItem.SetStartTimeHns(ConvertLegacyTimeToHns(static_cast<DWORD>(m_mediaItems[i].llStartTimeMs)));
            convertedItem.SetDimensions(m_mediaItems[i].uWidth, m_mediaItems[i].uHeight);
            convertedItem.SetFrameRate(m_mediaItems[i].dwFrameRate);
            pOutProject->AddMediaItem(convertedItem);
        }
    }

    return S_OK;
}

HRESULT LegacyProjectSupport::ReadLegacyMediaItems(
    IXmlReader* pReader, StoryboardManager::MovieProject* pProject)
{
    if (!pReader || !pProject)
        return E_POINTER;

    XmlNodeType nodeType;
    HRESULT hr;

    while (SUCCEEDED(pReader->Read(&nodeType)))
    {
        if (nodeType == XmlNodeTypeEndElement)
            break;

        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            UINT cchName = 0;
            hr = pReader->GetLocalName(&pszLocalName, &cchName);
            if (FAILED(hr) || !pszLocalName)
                continue;

            if (wcscmp(pszLocalName, L"mediaItem") == 0)
            {
                LegacyMediaItem item;

                LPCWSTR pszValue = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"id", &pszValue)) && pszValue)
                    item.dwId = _wtoi(pszValue);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"filePath", &pszValue)) && pszValue)
                    item.strFilePath = pszValue;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"mediaType", &pszValue)) && pszValue)
                    item.dwMediaType = _wtoi(pszValue);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"duration", &pszValue)) && pszValue)
                    item.llDurationMs = _wtoi64(pszValue);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"startTime", &pszValue)) && pszValue)
                    item.llStartTimeMs = _wtoi64(pszValue);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"width", &pszValue)) && pszValue)
                    item.uWidth = _wtoi(pszValue);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"height", &pszValue)) && pszValue)
                    item.uHeight = _wtoi(pszValue);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"frameRate", &pszValue)) && pszValue)
                    item.dwFrameRate = _wtoi(pszValue);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"displayName", &pszValue)) && pszValue)
                    item.strDisplayName = pszValue;

                if (!item.strFilePath.IsEmpty())
                {
                    ReadMediaItemAttributes(pReader, item);
                    m_mediaItems.push_back(item);
                }

                SkipCurrentElement(pReader);
            }
            else
            {
                SkipCurrentElement(pReader);
            }
        }
    }

    return S_OK;
}

HRESULT LegacyProjectSupport::ReadLegacyTimeline(
    IXmlReader* pReader, StoryboardManager::MovieProject* pProject)
{
    if (!pReader || !pProject)
        return E_POINTER;

    XmlNodeType nodeType;
    HRESULT hr;
    DWORD dwNextExtentId = 1;

    StoryboardManager::ProjectTimeline* pTimeline =
        pProject->GetTimeline(StoryboardManager::TimelineTrackTypeVideo);

    while (SUCCEEDED(pReader->Read(&nodeType)))
    {
        if (nodeType == XmlNodeTypeEndElement)
            break;

        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            UINT cchName = 0;
            hr = pReader->GetLocalName(&pszLocalName, &cchName);
            if (FAILED(hr) || !pszLocalName)
                continue;

            if (wcscmp(pszLocalName, L"extent") == 0)
            {
                LPCWSTR pszValue = nullptr;
                DWORD dwExtentId = dwNextExtentId++;
                DWORD dwMediaId = 0;
                LONGLONG llStartTime = 0;
                LONGLONG llEndTime = 0;

                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"extentId", &pszValue)) && pszValue)
                    dwExtentId = _wtoi(pszValue);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"mediaId", &pszValue)) && pszValue)
                    dwMediaId = _wtoi(pszValue);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"startTime", &pszValue)) && pszValue)
                    llStartTime = _wtoi64(pszValue);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"endTime", &pszValue)) && pszValue)
                    llEndTime = _wtoi64(pszValue);

                if (pTimeline)
                    pTimeline->AddExtent(dwExtentId);

                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"speed", &pszValue)) && pszValue)
                {
                }
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"volume", &pszValue)) && pszValue)
                {
                }
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"pan", &pszValue)) && pszValue)
                {
                }
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"reversed", &pszValue)) && pszValue)
                {
                }
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"muted", &pszValue)) && pszValue)
                {
                }
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fadeIn", &pszValue)) && pszValue)
                {
                }
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fadeOut", &pszValue)) && pszValue)
                {
                }

                SkipCurrentElement(pReader);
            }
            else
            {
                SkipCurrentElement(pReader);
            }
        }
    }

    if (pTimeline)
        pTimeline->SetTotalDurationHns(0);

    return S_OK;
}

HRESULT LegacyProjectSupport::ReadLegacyExtent(
    IXmlReader* pReader, StoryboardManager::MovieExtent* pExtent)
{
    if (!pReader || !pExtent)
        return E_POINTER;

    LPCWSTR pszValue = nullptr;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"extentId", &pszValue)) && pszValue)
        pExtent->SetExtentId(_wtoi(pszValue));
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"mediaId", &pszValue)) && pszValue)
        pExtent->SetMediaId(_wtoi(pszValue));
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"startTime", &pszValue)) && pszValue)
        pExtent->SetStartTimeHns(ConvertLegacyTimeToHns(static_cast<DWORD>(_wtoi64(pszValue))));
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"endTime", &pszValue)) && pszValue)
        pExtent->SetEndTimeHns(ConvertLegacyTimeToHns(static_cast<DWORD>(_wtoi64(pszValue))));
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"speed", &pszValue)) && pszValue)
        pExtent->SetSpeedFactor(_wtof(pszValue));
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"volume", &pszValue)) && pszValue)
        pExtent->SetVolume(_wtof(pszValue));
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"pan", &pszValue)) && pszValue)
        pExtent->SetPan(_wtof(pszValue));
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"reversed", &pszValue)) && pszValue)
        pExtent->SetReversed(_wtoi(pszValue) != 0);
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"muted", &pszValue)) && pszValue)
        pExtent->SetMuted(_wtoi(pszValue) != 0);
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fadeIn", &pszValue)) && pszValue)
        pExtent->SetFadeInDurationHns(ConvertLegacyTimeToHns(static_cast<DWORD>(_wtoi64(pszValue))));
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"fadeOut", &pszValue)) && pszValue)
        pExtent->SetFadeOutDurationHns(ConvertLegacyTimeToHns(static_cast<DWORD>(_wtoi64(pszValue))));

    return S_OK;
}

HRESULT LegacyProjectSupport::ConvertLegacyMediaItem(
    const ATL::CString& strLegacyItem, StoryboardManager::ProjectMediaItem* pOutItem)
{
    if (!pOutItem)
        return E_POINTER;

    pOutItem->SetSourcePath(strLegacyItem);

    if (strLegacyItem.IsEmpty())
        return E_INVALIDARG;

    return S_OK;
}

HRESULT LegacyProjectSupport::ConvertLegacyExtent(
    const ATL::CString& strLegacyExtent, StoryboardManager::MovieExtent* pOutExtent)
{
    if (!pOutExtent)
        return E_POINTER;

    if (strLegacyExtent.IsEmpty())
        return E_INVALIDARG;

    pOutExtent->SetExtentId(0);
    pOutExtent->SetMediaId(0);
    pOutExtent->SetStartTimeHns(0);
    pOutExtent->SetEndTimeHns(0);
    pOutExtent->SetSpeedFactor(1.0);
    pOutExtent->SetVolume(1.0);

    return S_OK;
}

LONGLONG LegacyProjectSupport::ConvertLegacyTimeToHns(DWORD dwLegacyTime)
{
    // In the legacy format (2009/2010), time was stored in milliseconds.
    // Current format uses hundred-nanosecond intervals (1 ms = 10000 HNS).
    return static_cast<LONGLONG>(dwLegacyTime) * 10000;
}

float LegacyProjectSupport::ConvertLegacyCoordToNormalized(DWORD dwLegacyCoord, DWORD dwContainerSize)
{
    if (dwContainerSize == 0)
        return 0.0f;
    return static_cast<float>(dwLegacyCoord) / static_cast<float>(dwContainerSize);
}

HRESULT LegacyProjectSupport::SaveAsLegacyFormat(
    LPCWSTR pszFilePath, StoryboardManager::MovieProject* pProject)
{
    if (!pszFilePath || !pProject)
        return E_POINTER;

    ATL::CComPtr<IStream> spStream;
    HRESULT hr = SHCreateStreamOnFileW(pszFilePath, STGM_WRITE | STGM_CREATE, &spStream);
    if (FAILED(hr))
        return hr;

    ATL::CComPtr<IXmlWriter> spWriter;
    hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&spWriter), nullptr);
    if (FAILED(hr))
        return hr;

    hr = spWriter->SetOutput(spStream);
    if (FAILED(hr))
        return hr;

    spWriter->SetProperty(XmlWriterProperty_MethodDecl, XmlWriterMethod_Xml);
    spWriter->WriteStartDocument(XmlStandalone_Omit);

    DWORD dwMajor = m_uVersionMajor ? m_uVersionMajor : 14;
    DWORD dwMinor = m_uVersionMinor ? m_uVersionMinor : 0;

    spWriter->WriteStartElement(nullptr, L"project", nullptr);
    spWriter->WriteAttributeString(nullptr, L"versionMajor", nullptr,
        CW2T(CStringW().Format(L"%u", dwMajor)));
    spWriter->WriteAttributeString(nullptr, L"versionMinor", nullptr,
        CW2T(CStringW().Format(L"%u", dwMinor)));

    CStringW strName;
    strName = pProject->GetProjectName();
    if (!strName.IsEmpty())
    {
        spWriter->WriteAttributeString(nullptr, L"name", nullptr, CW2T(strName));
    }

    WriteLegacyMediaItems(spWriter, pProject);

    WriteLegacyTimeline(spWriter, pProject);

    spWriter->WriteStartElement(nullptr, L"settings", nullptr);
    {
        CStringW strVal;
        strVal.Format(L"%u", pProject->GetSettings().GetOutputWidth());
        spWriter->WriteAttributeString(nullptr, L"outputWidth", nullptr, CW2T(strVal));
        strVal.Format(L"%u", pProject->GetSettings().GetOutputHeight());
        spWriter->WriteAttributeString(nullptr, L"outputHeight", nullptr, CW2T(strVal));
        strVal.Format(L"%.2f", pProject->GetSettings().GetAspectRatio());
        spWriter->WriteAttributeString(nullptr, L"aspectRatio", nullptr, CW2T(strVal));
        strVal.Format(L"%u", pProject->GetSettings().GetVideoBitRate());
        spWriter->WriteAttributeString(nullptr, L"videoBitRate", nullptr, CW2T(strVal));
        strVal.Format(L"%u", pProject->GetSettings().GetAudioBitRate());
        spWriter->WriteAttributeString(nullptr, L"audioBitRate", nullptr, CW2T(strVal));
        strVal.Format(L"%u", pProject->GetSettings().GetFrameRate());
        spWriter->WriteAttributeString(nullptr, L"frameRate", nullptr, CW2T(strVal));
    }
    spWriter->WriteEndElement();

    spWriter->WriteEndElement();
    spWriter->WriteEndDocument();
    spWriter->Flush();

    return S_OK;
}

HRESULT LegacyProjectSupport::WriteLegacyMediaItems(
    IXmlWriter* pWriter, StoryboardManager::MovieProject* pProject)
{
    if (!pWriter || !pProject)
        return E_POINTER;

    pWriter->WriteStartElement(nullptr, L"media", nullptr);

    for (size_t i = 0; i < pProject->GetMediaItemCount(); ++i)
    {
        const StoryboardManager::ProjectMediaItem* pItem = pProject->GetMediaItem(i);
        if (!pItem)
            continue;

        pWriter->WriteStartElement(nullptr, L"mediaItem", nullptr);

        CStringW strVal;
        strVal.Format(L"%u", pItem->GetMediaId());
        pWriter->WriteAttributeString(nullptr, L"id", nullptr, CW2T(strVal));

        pWriter->WriteAttributeString(nullptr, L"filePath", nullptr,
            CW2T(pItem->GetSourcePath()));

        strVal.Format(L"%u", pItem->GetMediaType());
        pWriter->WriteAttributeString(nullptr, L"mediaType", nullptr, CW2T(strVal));

        strVal.Format(L"%lld", pItem->GetDurationHns() / 10000);
        pWriter->WriteAttributeString(nullptr, L"duration", nullptr, CW2T(strVal));

        strVal.Format(L"%lld", pItem->GetStartTimeHns() / 10000);
        pWriter->WriteAttributeString(nullptr, L"startTime", nullptr, CW2T(strVal));

        strVal.Format(L"%u", pItem->GetWidth());
        pWriter->WriteAttributeString(nullptr, L"width", nullptr, CW2T(strVal));

        strVal.Format(L"%u", pItem->GetHeight());
        pWriter->WriteAttributeString(nullptr, L"height", nullptr, CW2T(strVal));

        strVal.Format(L"%u", pItem->GetFrameRate());
        pWriter->WriteAttributeString(nullptr, L"frameRate", nullptr, CW2T(strVal));

        pWriter->WriteEndElement();
    }

    pWriter->WriteEndElement();

    return S_OK;
}

HRESULT LegacyProjectSupport::WriteLegacyTimeline(
    IXmlWriter* pWriter, StoryboardManager::MovieProject* pProject)
{
    if (!pWriter || !pProject)
        return E_POINTER;

    pWriter->WriteStartElement(nullptr, L"timeline", nullptr);

    const StoryboardManager::ProjectTimeline* pTimeline =
        pProject->GetTimeline(StoryboardManager::TimelineTrackTypeVideo);

    if (pTimeline)
    {
        for (size_t i = 0; i < pTimeline->GetExtentCount(); ++i)
        {
            DWORD dwExtentId = pTimeline->GetExtentIdAt(i);

            pWriter->WriteStartElement(nullptr, L"extent", nullptr);

            CStringW strVal;
            strVal.Format(L"%u", dwExtentId);
            pWriter->WriteAttributeString(nullptr, L"extentId", nullptr, CW2T(strVal));

            pWriter->WriteEndElement();
        }
    }

    pWriter->WriteEndElement();

    return S_OK;
}

DWORD LegacyProjectSupport::GetLegacyDefaultDuration() const throw()
{
    // Default 5 seconds in HNS
    return 50000000;
}

SIZE LegacyProjectSupport::GetLegacyDefaultResolution() const
{
    SIZE sz = { 640, 480 };
    return sz;
}

DWORD LegacyProjectSupport::GetLegacyDefaultBitRate() const throw()
{
    return 2500; // 2.5 Mbps
}

void LegacyProjectSupport::InitializePropertyMaps()
{
    m_propertyMaps.clear();

    // Map of legacy property names to current property names
    LegacyPropertySetMap maps[] = {
        { L"StartTime",           L"StartTimeHns",        1 },
        { L"EndTime",             L"EndTimeHns",          1 },
        { L"CropRect",            L"CropCoordinates",     2 },
        { L"NormalizedRect",      L"LayoutRect",          2 },
        { L"TransitionDuration",  L"TransitionDurationHns", 1 },
        { L"EffectIntensity",     L"Intensity",           0 },
    };

    for (size_t i = 0; i < ARRAYSIZE(maps); i++)
        m_propertyMaps.push_back(maps[i]);
}

void LegacyProjectSupport::SetVersion(DWORD dwMajor, DWORD dwMinor)
{
    m_uVersionMajor = dwMajor;
    m_uVersionMinor = dwMinor;
}

DWORD LegacyProjectSupport::GetVersion() const throw()
{
    return (m_uVersionMajor << 16) | m_uVersionMinor;
}

size_t LegacyProjectSupport::GetMediaItemCount() const throw()
{
    return m_mediaItems.size();
}

const LegacyMediaItem* LegacyProjectSupport::GetMediaItem(size_t nIndex) const
{
    if (nIndex < m_mediaItems.size())
        return &m_mediaItems[nIndex];
    return nullptr;
}

size_t LegacyProjectSupport::AddMediaItem(const LegacyMediaItem& item)
{
    m_mediaItems.push_back(item);
    return m_mediaItems.size() - 1;
}

void LegacyProjectSupport::RemoveAllMediaItems()
{
    m_mediaItems.clear();
}

const std::map<ATL::CString, ATL::CString>& LegacyProjectSupport::GetProperties() const
{
    return m_properties;
}

void LegacyProjectSupport::SetProperty(LPCWSTR pszKey, LPCWSTR pszValue)
{
    if (pszKey)
        m_properties[pszKey] = pszValue ? pszValue : L"";
}

ATL::CString LegacyProjectSupport::GetProperty(LPCWSTR pszKey) const
{
    if (!pszKey)
        return L"";
    auto it = m_properties.find(pszKey);
    if (it != m_properties.end())
        return it->second;
    return L"";
}

HRESULT LegacyProjectSupport::ReadMediaItemAttributes(
    IXmlReader* pReader, LegacyMediaItem& item)
{
    if (!pReader)
        return E_POINTER;

    LPCWSTR pszValue = nullptr;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"displayName", &pszValue)) && pszValue)
        item.strDisplayName = pszValue;

    return S_OK;
}

HRESULT LegacyProjectSupport::ReadTimelineExtent(
    IXmlReader* pReader, StoryboardManager::MovieExtent* pExtent)
{
    if (!pReader || !pExtent)
        return E_POINTER;

    return ReadLegacyExtent(pReader, pExtent);
}

HRESULT LegacyProjectSupport::WriteMediaItemAttributes(
    IXmlWriter* pWriter, const LegacyMediaItem& item)
{
    if (!pWriter)
        return E_POINTER;

    CStringW strVal;
    strVal.Format(L"%u", item.dwId);
    pWriter->WriteAttributeString(nullptr, L"id", nullptr, CW2T(strVal));

    pWriter->WriteAttributeString(nullptr, L"filePath", nullptr, CW2T(item.strFilePath));

    strVal.Format(L"%u", item.dwMediaType);
    pWriter->WriteAttributeString(nullptr, L"mediaType", nullptr, CW2T(strVal));

    strVal.Format(L"%lld", item.llDurationMs);
    pWriter->WriteAttributeString(nullptr, L"duration", nullptr, CW2T(strVal));

    strVal.Format(L"%lld", item.llStartTimeMs);
    pWriter->WriteAttributeString(nullptr, L"startTime", nullptr, CW2T(strVal));

    strVal.Format(L"%u", item.uWidth);
    pWriter->WriteAttributeString(nullptr, L"width", nullptr, CW2T(strVal));

    strVal.Format(L"%u", item.uHeight);
    pWriter->WriteAttributeString(nullptr, L"height", nullptr, CW2T(strVal));

    strVal.Format(L"%u", item.dwFrameRate);
    pWriter->WriteAttributeString(nullptr, L"frameRate", nullptr, CW2T(strVal));

    if (!item.strDisplayName.IsEmpty())
    {
        pWriter->WriteAttributeString(nullptr, L"displayName", nullptr, CW2T(item.strDisplayName));
    }

    return S_OK;
}

ATL::CString LegacyProjectSupport::MapPropertyName(LPCWSTR pszLegacyName) const
{
    if (!pszLegacyName)
        return L"";

    for (const auto& map : m_propertyMaps)
    {
        if (map.legacyName.CompareNoCase(pszLegacyName) == 0)
            return map.currentName;
    }
    return pszLegacyName;
}
