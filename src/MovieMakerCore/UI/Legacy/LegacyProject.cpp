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

    // Detect version
    DWORD dwVersion = DetectProjectVersion(pszFilePath);
    if (dwVersion == 0)
        return E_FAIL;

    m_uVersionMajor = dwVersion;

    // In the full implementation, this would:
    //  1. Open the legacy XML file using IXmlReader
    //  2. Read the root <project> element
    //  3. Read <media> items, converting to current format
    //  4. Read <timeline> extents, converting as needed
    //  5. Read legacy <transitions>, <effects>, <titles>
    //  6. Apply property name mappings

    return S_OK;
}

HRESULT LegacyProjectSupport::ReadLegacyMediaItems(
    IXmlReader* pReader, StoryboardManager::MovieProject* pProject)
{
    if (!pReader || !pProject)
        return E_POINTER;

    UNREFERENCED_PARAMETER(pReader);
    UNREFERENCED_PARAMETER(pProject);
    return S_OK;
}

HRESULT LegacyProjectSupport::ReadLegacyTimeline(
    IXmlReader* pReader, StoryboardManager::MovieProject* pProject)
{
    if (!pReader || !pProject)
        return E_POINTER;

    UNREFERENCED_PARAMETER(pReader);
    UNREFERENCED_PARAMETER(pProject);
    return S_OK;
}

HRESULT LegacyProjectSupport::ReadLegacyExtent(
    IXmlReader* pReader, StoryboardManager::MovieExtent* pExtent)
{
    if (!pReader || !pExtent)
        return E_POINTER;

    UNREFERENCED_PARAMETER(pReader);
    UNREFERENCED_PARAMETER(pExtent);
    return S_OK;
}

HRESULT LegacyProjectSupport::ConvertLegacyMediaItem(
    const ATL::CString& strLegacyItem, StoryboardManager::ProjectMediaItem* pOutItem)
{
    if (!pOutItem)
        return E_POINTER;

    UNREFERENCED_PARAMETER(strLegacyItem);
    UNREFERENCED_PARAMETER(pOutItem);
    return S_OK;
}

HRESULT LegacyProjectSupport::ConvertLegacyExtent(
    const ATL::CString& strLegacyExtent, StoryboardManager::MovieExtent* pOutExtent)
{
    if (!pOutExtent)
        return E_POINTER;

    UNREFERENCED_PARAMETER(strLegacyExtent);
    UNREFERENCED_PARAMETER(pOutExtent);
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

    UNREFERENCED_PARAMETER(pszFilePath);
    UNREFERENCED_PARAMETER(pProject);
    return S_OK;
}

HRESULT LegacyProjectSupport::WriteLegacyMediaItems(
    IXmlWriter* pWriter, StoryboardManager::MovieProject* pProject)
{
    if (!pWriter || !pProject)
        return E_POINTER;

    UNREFERENCED_PARAMETER(pWriter);
    UNREFERENCED_PARAMETER(pProject);
    return S_OK;
}

HRESULT LegacyProjectSupport::WriteLegacyTimeline(
    IXmlWriter* pWriter, StoryboardManager::MovieProject* pProject)
{
    if (!pWriter || !pProject)
        return E_POINTER;

    UNREFERENCED_PARAMETER(pWriter);
    UNREFERENCED_PARAMETER(pProject);
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
