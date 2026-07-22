/*
 * SerializationWriter.h
 *
 * Binary and XML project file writer for .wlmp files. Produces the
 * XmlLite-based XML format used by Windows Live Movie Maker 2012.
 *
 * RTTI: ?AVSerializationWriter@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef SERIALIZATIONWRITER_H
#define SERIALIZATIONWRITER_H

#include "SerializationContext.h"

namespace StoryboardManager
{

class MovieProject;

// ============================================================================
// SerializationWriter
// ============================================================================
// Writes .wlmp project files using XmlLite (IXmlWriter). Manages the
// document structure, indentation, and attribute encoding for the XML
// output. Provides typed attribute writing helpers.
//
class STORYBOARD_API SerializationWriter
{
public:
    SerializationWriter();
    ~SerializationWriter();

    // -- File writing --
    HRESULT WriteToFile(LPCWSTR pszFilePath, MovieProject* pProject);
    HRESULT WriteToStream(IStream* pStream, MovieProject* pProject);

    // -- Project save/load (WLMP format) --
    HRESULT SaveProject(LPCWSTR pszFilePath, MovieProject* pProject);

    // -- Element nesting --
    HRESULT BeginElement(LPCWSTR pszName);
    HRESULT EndElement();
    HRESULT BeginElementWithId(LPCWSTR pszName, DWORD dwId);

    // -- Attribute writing (typed) --
    HRESULT WriteAttribute(LPCWSTR pszName, LPCWSTR pszValue);
    HRESULT WriteAttribute(LPCWSTR pszName, int nValue);
    HRESULT WriteAttribute(LPCWSTR pszName, DWORD dwValue);
    HRESULT WriteAttribute(LPCWSTR pszName, LONGLONG llValue);
    HRESULT WriteAttribute(LPCWSTR pszName, double dblValue);
    HRESULT WriteAttribute(LPCWSTR pszName, bool fValue);

    // -- Text content --
    HRESULT WriteText(LPCWSTR pszText);

    // -- Legacy project writing --
    HRESULT WriteProject(LPCWSTR pszFilePath, MovieProject* pProject);
    HRESULT WriteMediaItems(IXmlWriter* pWriter, MovieProject* pProject);
    HRESULT WriteProperties(IXmlWriter* pWriter, MovieProject* pProject);

    // -- XML string helpers --
    static ATL::CString EscapeXmlString(LPCWSTR pszInput);

    // -- Document lifecycle --
    HRESULT BeginDocument();
    HRESULT EndDocument();
    HRESULT Flush();

    // -- Indentation --
    void SetIndentLevel(int nLevel);
    int  GetIndentLevel() const throw();

    // -- Error state --
    HRESULT GetLastHResult() const throw();

    // -- Context access --
    SerializationContext& GetContext();
    const SerializationContext& GetContext() const;

private:
    // -- Project serialization dispatch --
    HRESULT WriteProjectElement(MovieProject* pProject);
    HRESULT WriteMediaElement(MovieProject* pProject);
    HRESULT WriteTimelineElement(MovieProject* pProject);
    HRESULT WriteTransitionsElement(MovieProject* pProject);
    HRESULT WriteEffectsElement(MovieProject* pProject);
    HRESULT WriteTitlesElement(MovieProject* pProject);
    HRESULT WriteSettingsElement(MovieProject* pProject);

    // -- State --
    SerializationContext m_ctx;
    IXmlWriter*          m_pWriter;
    IStream*             m_pStream;
    int                  m_nIndentLevel;
    DWORD                m_dwElementDepth;
    HRESULT              m_hrLast;

    SerializationWriter(const SerializationWriter&);
    SerializationWriter& operator=(const SerializationWriter&);
};

} // namespace StoryboardManager

#endif // SERIALIZATIONWRITER_H
