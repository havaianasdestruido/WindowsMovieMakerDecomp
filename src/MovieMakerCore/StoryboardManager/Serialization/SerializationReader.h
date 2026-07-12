/*
 * SerializationReader.h
 *
 * Binary and XML project file reader for .wlmp files. Supports the
 * Windows Live Movie Maker 2012 XML-based project format with forward-
 * compatible element handling for version migration (2011 -> 2012).
 *
 * RTTI: ?AVSerializationReader@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef SERIALIZATIONREADER_H
#define SERIALIZATIONREADER_H

#include "SerializationContext.h"

namespace StoryboardManager
{

class MovieProject;

// ============================================================================
// SerializationElementHandler
// ============================================================================
// Callback interface for processing individual XML elements during
// deserialization. Implementations handle specific element names (e.g.,
// "media", "timeline", "transition") and populate the project model.
//
class STORYBOARD_API SerializationElementHandler
{
public:
    SerializationElementHandler();
    virtual ~SerializationElementHandler();

    virtual HRESULT OnBeginElement(LPCWSTR pszName, SerializationContext& ctx) = 0;
    virtual HRESULT OnEndElement(LPCWSTR pszName, SerializationContext& ctx) = 0;
    virtual HRESULT OnAttribute(LPCWSTR pszName, LPCWSTR pszValue,
                                SerializationContext& ctx) = 0;
    virtual HRESULT OnText(LPCWSTR pszText, SerializationContext& ctx) = 0;

    LPCWSTR GetElementName() const;
    void SetElementName(LPCWSTR pszName);

private:
    ATL::CString m_strElementName;
};

// ============================================================================
// SerializationReader
// ============================================================================
// Reads .wlmp project files using XmlLite (IXmlReader). Parses the XML
// structure and dispatches to registered element handlers. Supports
// forward-compatible reading by skipping unrecognized elements.
//
class STORYBOARD_API SerializationReader
{
public:
    SerializationReader();
    ~SerializationReader();

    // -- File reading --
    HRESULT ReadFromFile(LPCWSTR pszFilePath, MovieProject* pProject);
    HRESULT ReadFromStream(IStream* pStream, MovieProject* pProject);
    HRESULT ReadFromMemory(const BYTE* pcbData, DWORD cbData, MovieProject* pProject);

    // -- Element handler registration --
    void RegisterHandler(LPCWSTR pszElementName, SerializationElementHandler* pHandler);
    void UnregisterHandler(LPCWSTR pszElementName);
    void UnregisterAllHandlers();

    // -- Version migration --
    bool IsVersionSupported(const SerializationVersion& ver) const throw();
    HRESULT MigrateVersion(SerializationContext& ctx);

    // -- Error handling --
    HRESULT GetLastHResult() const throw();
    ATL::CString GetErrorDetails() const;

    // -- Skip unknown elements (forward compatibility) --
    HRESULT SkipCurrentElement(IXmlReader* pReader);

    // -- Attribute reading helpers --
    static HRESULT ReadIntAttribute(IXmlReader* pReader, LPCWSTR pszName, int* pValue);
    static HRESULT ReadDwordAttribute(IXmlReader* pReader, LPCWSTR pszName, DWORD* pValue);
    static HRESULT ReadLONGLONGAttribute(IXmlReader* pReader, LPCWSTR pszName, LONGLONG* pValue);
    static HRESULT ReadDoubleAttribute(IXmlReader* pReader, LPCWSTR pszName, double* pValue);
    static HRESULT ReadStringAttribute(IXmlReader* pReader, LPCWSTR pszName,
                                       ATL::CString& strValue);
    static HRESULT ReadBoolAttribute(IXmlReader* pReader, LPCWSTR pszName, bool* pValue);

private:
    // -- Internal parsing loop --
    HRESULT ParseDocument(IXmlReader* pReader, MovieProject* pProject);
    HRESULT ParseElement(IXmlReader* pReader, MovieProject* pProject,
                         SerializationContext& ctx);
    HRESULT ParseElementAttributes(IXmlReader* pReader, SerializationElementHandler* pHandler,
                                   SerializationContext& ctx);

    // -- Forward-compatible element skipping --
    HRESULT SkipEndElement(IXmlReader* pReader, DWORD dwDepth);

    // -- Version detection --
    HRESULT DetectVersion(IXmlReader* pReader, SerializationContext& ctx);

    // -- Handler lookup --
    SerializationElementHandler* FindHandler(LPCWSTR pszElementName);

    // -- State --
    SerializationContext             m_ctx;
    ATL::CAtlArray<SerializationElementHandler*> m_arrHandlers;
    ATL::CString                    m_strLastError;
    HRESULT                         m_hrLast;
    DWORD                           m_dwMajorVersion;
    DWORD                           m_dwMinorVersion;

    SerializationReader(const SerializationReader&);
    SerializationReader& operator=(const SerializationReader&);
};

} // namespace StoryboardManager

#endif // SERIALIZATIONREADER_H
