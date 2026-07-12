/*
 * SAXProfileBuilder.h
 *
 * SAXProfileBuilder - SAX-based XML parser for encode profile definitions.
 * Reads profile XML files from %LOCALAPPDATA%\Microsoft\WL\MovieMaker\Profiles\
 * and builds EncodeProfile instances from the parsed data.
 *
 * RTTI: ?AVSAXProfileBuilder@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef SAXPROFILEBUILDER_H
#define SAXPROFILEBUILDER_H

#include "HMRAVSource.h"
#include "EncodeProfile.h"
#include <xmllite.h>

namespace HMRAVSource
{

// ============================================================================
// SAXProfileBuilder
// ============================================================================
// Parses XML profile definition files using the XmlLite SAX parser.
// Builds EncodeProfile instances from the XML elements and attributes
// found in profile files. Supports both built-in and user-defined profiles.
//
class AVSOURCE_API SAXProfileBuilder :
    public IXmlReaderInput
{
public:
    SAXProfileBuilder();
    virtual ~SAXProfileBuilder();

    // Profile loading
    HRESULT LoadProfile(LPCWSTR pszFilePath, EncodeProfile* pProfile);
    HRESULT LoadProfilesFromDirectory(LPCWSTR pszDirectory, ATL::CAtlArray<EncodeProfile>& profiles);
    HRESULT LoadDefaultProfiles();

    // Profile saving
    HRESULT SaveProfile(LPCWSTR pszFilePath, const EncodeProfile& profile);

    // Direct XML parsing
    HRESULT ParseFromString(LPCWSTR pszXml, DWORD cchXml, EncodeProfile* pProfile);
    HRESULT ParseFromStream(IStream* pStream, EncodeProfile* pProfile);

    // Profile directory
    static ATL::CString GetProfileDirectory();
    static HRESULT EnsureProfileDirectory();

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

private:
    LONG m_cRef;

    // Current parsing state
    EncodeProfile*      m_pCurrentProfile;
    ATL::CString        m_strCurrentElement;
    ATL::CString        m_strCurrentValue;
    bool                m_fInVideoSection;
    bool                m_fInAudioSection;
    bool                m_fInConstraints;

    // XmlLite helpers
    HRESULT CreateXmlReader(IXmlReader** ppReader);
    HRESULT CreateXmlWriter(LPCWSTR pszFilePath, IXmlWriter** ppWriter);
    HRESULT ProcessStartElement(IXmlReader* pReader);
    HRESULT ProcessEndElement(IXmlReader* pReader);
    HRESULT ProcessAttributes(IXmlReader* pReader);
    HRESULT ProcessEndElement(const ATL::CString& strElement);

    // Element handlers
    HRESULT HandleProfileElement(IXmlReader* pReader);
    HRESULT HandleVideoElement(IXmlReader* pReader);
    HRESULT HandleAudioElement(IXmlReader* pReader);
    HRESULT HandleConstraintsElement(IXmlReader* pReader);

    // Attribute value parsing
    static UINT ParseUInt(const ATL::CString& strValue, UINT uDefault = 0);
    static DWORD ParseDWord(const ATL::CString& strValue, DWORD dwDefault = 0);
    static double ParseDouble(const ATL::CString& strValue, double dblDefault = 0.0);
    static bool ParseBool(const ATL::CString& strValue, bool fDefault = false);
    static VideoCodec ParseVideoCodec(const ATL::CString& strValue);
    static AudioCodec ParseAudioCodec(const ATL::CString& strValue);

    // XML writing helpers
    HRESULT WriteProfileXml(IXmlWriter* pWriter, const EncodeProfile& profile);
    HRESULT WriteVideoParams(IXmlWriter* pWriter, const EncodeVideoParams& params);
    HRESULT WriteAudioParams(IXmlWriter* pWriter, const EncodeAudioParams& params);

    // Profile element attribute handling
    HRESULT OnProfileAttribute(const ATL::CString& strName, const ATL::CString& strValue);
    HRESULT OnVideoAttribute(const ATL::CString& strName, const ATL::CString& strValue);
    HRESULT OnAudioAttribute(const ATL::CString& strName, const ATL::CString& strValue);
    HRESULT OnConstraintsAttribute(const ATL::CString& strName, const ATL::CString& strValue);
};

} // namespace HMRAVSource

#endif // SAXPROFILEBUILDER_H
