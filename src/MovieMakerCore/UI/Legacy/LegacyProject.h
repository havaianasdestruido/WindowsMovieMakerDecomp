/*
 * LegacyProject.h
 *
 * LegacyProjectSupport: Reading/writing older .wlmp format versions (2010/2011).
 *
 * RTTI: ?AVLegacyProjectSupport@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 * Provides backward compatibility with .wlmp project files from:
 *   - Windows Live Movie Maker 2010 (version 14.x)
 *   - Windows Live Movie Maker 2011 (version 15.x)
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef LEGACY_PROJECT_H
#define LEGACY_PROJECT_H

#include "../../pch.h"
#include "../../StoryboardManager/MovieProject.h"

// Legacy project version constants
static const DWORD kLegacyProjectVersion2010Major = 14;
static const DWORD kLegacyProjectVersion2010Minor = 0;
static const DWORD kLegacyProjectVersion2011Major = 15;
static const DWORD kLegacyProjectVersion2011Minor = 4;

// Legacy namespace used in 2010/2011 wlmp XML
static const LPCWSTR kLegacyNamespace2010 = L"http://schemas.microsoft.com/wl/moviemaker/2010";
static const LPCWSTR kLegacyNamespace2011 = L"http://schemas.microsoft.com/wl/moviemaker/2011";

// ============================================================================
// LegacyPropertySetMap
// ============================================================================
// Mapping of legacy property names to current StoryboardManager equivalents.
// Used during conversion of older project files.
//
struct LegacyPropertySetMap
{
    ATL::CString legacyName;
    ATL::CString currentName;
    DWORD        conversionType; // 0 = direct copy, 1 = rename, 2 = convert
};

// ============================================================================
// LegacyProjectSupport
// ============================================================================
// Provides reading and writing of older .wlmp format versions. This is
// essential for backward compatibility when a user opens a project created
// by an earlier version of Windows Live Movie Maker.
//
// The class handles:
//
//   1. Detection of project file version
//   2. Reading older XML schemas and converting items
//   3. Mapping of deprecated property names
//   4. Unit conversion (e.g., 1000-unit normalized space to current)
//
class LegacyProjectSupport
{
public:
    LegacyProjectSupport();
    ~LegacyProjectSupport();

    // -- Version detection --
    DWORD GetRecognizedVersionMajor() const throw();
    DWORD GetRecognizedVersionMinor() const throw();

    bool IsVersion14() const;
    bool IsVersion15() const;
    static bool IsLegacyProjectFormat(LPCWSTR pszFilePath);
    static bool IsLegacyNamespace(IXmlReader* pReader);
    static DWORD DetectProjectVersion(LPCWSTR pszFilePath);

    // -- Reading legacy projects --
    HRESULT ReadLegacyProject(LPCWSTR pszFilePath, StoryboardManager::MovieProject* pOutProject);
    HRESULT ReadLegacyMediaItems(IXmlReader* pReader, StoryboardManager::MovieProject* pProject);
    HRESULT ReadLegacyTimeline(IXmlReader* pReader, StoryboardManager::MovieProject* pProject);
    HRESULT ReadLegacyExtent(IXmlReader* pReader, StoryboardManager::MovieExtent* pExtent);

    // -- Conversion helpers --
    HRESULT ConvertLegacyMediaItem(const ATL::CString& strLegacyItem, StoryboardManager::ProjectMediaItem* pOutItem);
    HRESULT ConvertLegacyExtent(const ATL::CString& strLegacyExtent, StoryboardManager::MovieExtent* pOutExtent);
    LONGLONG ConvertLegacyTimeToHns(DWORD dwLegacyTime);
    float ConvertLegacyCoordToNormalized(DWORD dwLegacyCoord, DWORD dwContainerSize);

    // -- Writing legacy format --
    HRESULT SaveAsLegacyFormat(LPCWSTR pszFilePath, StoryboardManager::MovieProject* pProject);
    HRESULT WriteLegacyMediaItems(IXmlWriter* pWriter, StoryboardManager::MovieProject* pProject);
    HRESULT WriteLegacyTimeline(IXmlWriter* pWriter, StoryboardManager::MovieProject* pProject);

    // -- Defaults for missing legacy values --
    DWORD GetLegacyDefaultDuration() const throw();
    SIZE GetLegacyDefaultResolution() const;
    DWORD GetLegacyDefaultBitRate() const throw();

private:
    void InitializePropertyMaps();

    UINT  m_uVersionMajor;
    UINT  m_uVersionMinor;
    std::vector<LegacyPropertySetMap> m_propertyMaps;
    bool  m_bIsLegacy;
};

#endif // LEGACY_PROJECT_H
