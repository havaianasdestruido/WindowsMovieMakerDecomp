#include "pch.h"
/*
 * ThemeTemplates.cpp
 *
 * Implementation of the X3D template system for themes. Provides base
 * template classes, X3D scene templates, socket/placeholder handling,
 * and effect/extent template handler implementations.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "ThemeTemplates.h"

namespace StoryboardManager
{

// ============================================================================
// TemplatePlaceholder implementation
// ============================================================================

TemplatePlaceholder::TemplatePlaceholder()
    : m_type(TemplateSocketTypeNone)
    , m_posX(0.0f)
    , m_posY(0.0f)
    , m_posZ(0.0f)
    , m_rotX(0.0f)
    , m_rotY(0.0f)
    , m_rotZ(0.0f)
    , m_scaleX(1.0f)
    , m_scaleY(1.0f)
    , m_scaleZ(1.0f)
    , m_boundsLeft(0.0f)
    , m_boundsTop(0.0f)
    , m_boundsRight(1.0f)
    , m_boundsBottom(1.0f)
    , m_llDurationHns(0)
    , m_fRequired(false)
{
}

TemplatePlaceholder::~TemplatePlaceholder()
{
}

ATL::CString TemplatePlaceholder::GetId() const
{
    return m_strId;
}

void TemplatePlaceholder::SetId(LPCWSTR pszId)
{
    m_strId = pszId ? pszId : L"";
}

ATL::CString TemplatePlaceholder::GetName() const
{
    return m_strName;
}

void TemplatePlaceholder::SetName(LPCWSTR pszName)
{
    m_strName = pszName ? pszName : L"";
}

TemplateSocketType TemplatePlaceholder::GetType() const throw()
{
    return m_type;
}

void TemplatePlaceholder::SetType(TemplateSocketType type) throw()
{
    m_type = type;
}

float TemplatePlaceholder::GetPositionX() const throw() { return m_posX; }
float TemplatePlaceholder::GetPositionY() const throw() { return m_posY; }
float TemplatePlaceholder::GetPositionZ() const throw() { return m_posZ; }

void TemplatePlaceholder::SetPosition(float x, float y, float z) throw()
{
    m_posX = x;
    m_posY = y;
    m_posZ = z;
}

float TemplatePlaceholder::GetRotationX() const throw() { return m_rotX; }
float TemplatePlaceholder::GetRotationY() const throw() { return m_rotY; }
float TemplatePlaceholder::GetRotationZ() const throw() { return m_rotZ; }

void TemplatePlaceholder::SetRotation(float x, float y, float z) throw()
{
    m_rotX = x;
    m_rotY = y;
    m_rotZ = z;
}

float TemplatePlaceholder::GetScaleX() const throw() { return m_scaleX; }
float TemplatePlaceholder::GetScaleY() const throw() { return m_scaleY; }
float TemplatePlaceholder::GetScaleZ() const throw() { return m_scaleZ; }

void TemplatePlaceholder::SetScale(float x, float y, float z) throw()
{
    m_scaleX = x;
    m_scaleY = y;
    m_scaleZ = z;
}

float TemplatePlaceholder::GetBoundsLeft() const throw() { return m_boundsLeft; }
float TemplatePlaceholder::GetBoundsTop() const throw() { return m_boundsTop; }
float TemplatePlaceholder::GetBoundsRight() const throw() { return m_boundsRight; }
float TemplatePlaceholder::GetBoundsBottom() const throw() { return m_boundsBottom; }

void TemplatePlaceholder::SetBounds(float left, float top, float right, float bottom) throw()
{
    m_boundsLeft = left;
    m_boundsTop = top;
    m_boundsRight = right;
    m_boundsBottom = bottom;
}

LONGLONG TemplatePlaceholder::GetDurationHns() const throw()
{
    return m_llDurationHns;
}

void TemplatePlaceholder::SetDurationHns(LONGLONG llDuration) throw()
{
    m_llDurationHns = llDuration;
}

ATL::CString TemplatePlaceholder::GetDefaultValue() const
{
    return m_strDefaultValue;
}

void TemplatePlaceholder::SetDefaultValue(LPCWSTR pszValue)
{
    m_strDefaultValue = pszValue ? pszValue : L"";
}

bool TemplatePlaceholder::IsRequired() const throw()
{
    return m_fRequired;
}

void TemplatePlaceholder::SetRequired(bool fRequired) throw()
{
    m_fRequired = fRequired;
}

// ============================================================================
// TemplateProperty implementation
// ============================================================================

TemplateProperty::TemplateProperty()
    : m_fInherited(false)
{
}

TemplateProperty::TemplateProperty(LPCWSTR pszKey, LPCWSTR pszValue)
    : m_fInherited(false)
{
    m_strKey = pszKey ? pszKey : L"";
    m_strValue = pszValue ? pszValue : L"";
}

TemplateProperty::~TemplateProperty()
{
}

ATL::CString TemplateProperty::GetKey() const
{
    return m_strKey;
}

void TemplateProperty::SetKey(LPCWSTR pszKey)
{
    m_strKey = pszKey ? pszKey : L"";
}

ATL::CString TemplateProperty::GetValue() const
{
    return m_strValue;
}

void TemplateProperty::SetValue(LPCWSTR pszValue)
{
    m_strValue = pszValue ? pszValue : L"";
}

int TemplateProperty::GetIntValue() const
{
    return _wtoi(m_strValue);
}

void TemplateProperty::SetIntValue(int nValue)
{
    m_strValue.Format(L"%d", nValue);
}

float TemplateProperty::GetFloatValue() const
{
    return static_cast<float>(_wtof(m_strValue));
}

void TemplateProperty::SetFloatValue(float flValue)
{
    m_strValue.Format(L"%g", flValue);
}

bool TemplateProperty::GetBoolValue() const
{
    return m_strValue.CompareNoCase(L"true") == 0 ||
           m_strValue.CompareNoCase(L"1") == 0 ||
           m_strValue.CompareNoCase(L"yes") == 0;
}

void TemplateProperty::SetBoolValue(bool fValue)
{
    m_strValue = fValue ? L"true" : L"false";
}

bool TemplateProperty::IsInherited() const throw()
{
    return m_fInherited;
}

void TemplateProperty::SetInherited(bool fInherited) throw()
{
    m_fInherited = fInherited;
}

// ============================================================================
// TemplateSocket implementation
// ============================================================================

TemplateSocket::TemplateSocket()
    : m_type(TemplateSocketTypeNone)
    , m_dwAllowedTypes(0xFFFFFFFF)
    , m_dwIndex(0)
    , m_fConnected(false)
{
}

TemplateSocket::~TemplateSocket()
{
}

TemplateSocket::TemplateSocket(const TemplateSocket& other)
    : m_strId(other.m_strId)
    , m_strName(other.m_strName)
    , m_type(other.m_type)
    , m_strPlaceholderId(other.m_strPlaceholderId)
    , m_dwAllowedTypes(other.m_dwAllowedTypes)
    , m_dwIndex(other.m_dwIndex)
    , m_fConnected(other.m_fConnected)
{
    m_arrProperties.Copy(other.m_arrProperties);
}

TemplateSocket& TemplateSocket::operator=(const TemplateSocket& other)
{
    if (this != &other)
    {
        m_strId = other.m_strId;
        m_strName = other.m_strName;
        m_type = other.m_type;
        m_strPlaceholderId = other.m_strPlaceholderId;
        m_dwAllowedTypes = other.m_dwAllowedTypes;
        m_dwIndex = other.m_dwIndex;
        m_fConnected = other.m_fConnected;
        m_arrProperties.Copy(other.m_arrProperties);
    }
    return *this;
}

ATL::CString TemplateSocket::GetId() const
{
    return m_strId;
}

void TemplateSocket::SetId(LPCWSTR pszId)
{
    m_strId = pszId ? pszId : L"";
}

ATL::CString TemplateSocket::GetName() const
{
    return m_strName;
}

void TemplateSocket::SetName(LPCWSTR pszName)
{
    m_strName = pszName ? pszName : L"";
}

TemplateSocketType TemplateSocket::GetType() const throw()
{
    return m_type;
}

void TemplateSocket::SetType(TemplateSocketType type) throw()
{
    m_type = type;
}

ATL::CString TemplateSocket::GetPlaceholderId() const
{
    return m_strPlaceholderId;
}

void TemplateSocket::SetPlaceholderId(LPCWSTR pszPlaceholderId)
{
    m_strPlaceholderId = pszPlaceholderId ? pszPlaceholderId : L"";
}

bool TemplateSocket::AllowsVideo() const throw()
{
    return (m_dwAllowedTypes & (1 << TemplateSocketTypeVideo)) != 0;
}

bool TemplateSocket::AllowsAudio() const throw()
{
    return (m_dwAllowedTypes & (1 << TemplateSocketTypeAudio)) != 0;
}

bool TemplateSocket::AllowsImage() const throw()
{
    return (m_dwAllowedTypes & (1 << TemplateSocketTypeImage)) != 0;
}

bool TemplateSocket::AllowsText() const throw()
{
    return (m_dwAllowedTypes & (1 << TemplateSocketTypeText)) != 0;
}

void TemplateSocket::SetAllowedTypes(DWORD dwMask) throw()
{
    m_dwAllowedTypes = dwMask;
}

DWORD TemplateSocket::GetAllowedTypes() const throw()
{
    return m_dwAllowedTypes;
}

DWORD TemplateSocket::GetIndex() const throw()
{
    return m_dwIndex;
}

void TemplateSocket::SetIndex(DWORD dwIndex) throw()
{
    m_dwIndex = dwIndex;
}

bool TemplateSocket::IsConnected() const throw()
{
    return m_fConnected;
}

void TemplateSocket::SetConnected(bool fConnected) throw()
{
    m_fConnected = fConnected;
}

size_t TemplateSocket::GetPropertyCount() const throw()
{
    return m_arrProperties.GetCount();
}

TemplateProperty* TemplateSocket::GetProperty(size_t nIndex)
{
    if (nIndex >= m_arrProperties.GetCount())
        return nullptr;
    return &m_arrProperties.GetAt(nIndex);
}

const TemplateProperty* TemplateSocket::GetProperty(size_t nIndex) const
{
    if (nIndex >= m_arrProperties.GetCount())
        return nullptr;
    return &m_arrProperties.GetAt(nIndex);
}

TemplateProperty* TemplateSocket::FindProperty(LPCWSTR pszKey)
{
    if (!pszKey)
        return nullptr;

    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        if (m_arrProperties.GetAt(i).GetKey().CompareNoCase(pszKey) == 0)
            return &m_arrProperties.GetAt(i);
    }
    return nullptr;
}

size_t TemplateSocket::AddProperty(const TemplateProperty& prop)
{
    return m_arrProperties.Add(prop);
}

void TemplateSocket::RemoveProperty(size_t nIndex)
{
    if (nIndex < m_arrProperties.GetCount())
        m_arrProperties.RemoveAt(nIndex);
}

void TemplateSocket::RemoveAllProperties()
{
    m_arrProperties.RemoveAll();
}

// ============================================================================
// TemplateSocketsParser implementation
// ============================================================================

TemplateSocketsParser::TemplateSocketsParser()
    : m_hrLastError(S_OK)
{
}

TemplateSocketsParser::~TemplateSocketsParser()
{
}

HRESULT TemplateSocketsParser::Parse(IXmlReader* pReader,
                                     ATL::CAtlArray<TemplateSocket>& sockets,
                                     ATL::CAtlArray<TemplatePlaceholder>& placeholders)
{
    if (!pReader)
    {
        m_hrLastError = E_INVALIDARG;
        return m_hrLastError;
    }

    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pwszName = nullptr;
            pReader->GetLocalName(&pwszName, nullptr);

            if (pwszName && wcscmp(pwszName, L"socket") == 0)
            {
                TemplateSocket socket;
                m_hrLastError = ParseSocket(pReader, socket);
                if (SUCCEEDED(m_hrLastError))
                    sockets.Add(socket);
            }
            else if (pwszName && wcscmp(pwszName, L"placeholder") == 0)
            {
                TemplatePlaceholder placeholder;
                m_hrLastError = ParsePlaceholder(pReader, placeholder);
                if (SUCCEEDED(m_hrLastError))
                    placeholders.Add(placeholder);
            }
        }
        else if (nodeType == XmlNodeType_EndElement)
        {
            break;
        }
    }

    return m_hrLastError;
}

HRESULT TemplateSocketsParser::ParseSocket(IXmlReader* pReader, TemplateSocket& socket)
{
    HRESULT hr = ParseSocketAttributes(pReader, socket);
    if (FAILED(hr))
        return hr;

    // Parse child properties
    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pwszName = nullptr;
            pReader->GetLocalName(&pwszName, nullptr);
            if (pwszName && wcscmp(pwszName, L"properties") == 0)
            {
                hr = ParseSocketProperties(pReader, socket);
                if (FAILED(hr))
                    return hr;
            }
        }
        else if (nodeType == XmlNodeType_EndElement)
        {
            break;
        }
    }

    return S_OK;
}

HRESULT TemplateSocketsParser::ParsePlaceholder(IXmlReader* pReader, TemplatePlaceholder& placeholder)
{
    return ParsePlaceholderAttributes(pReader, placeholder);
}

HRESULT TemplateSocketsParser::GetLastError() const throw()
{
    return m_hrLastError;
}

ATL::CString TemplateSocketsParser::GetLastErrorMessage() const
{
    return m_strLastError;
}

HRESULT TemplateSocketsParser::ParseSocketAttributes(IXmlReader* pReader, TemplateSocket& socket)
{
    LPCWSTR pwszVal = nullptr;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"id", &pwszVal)) && pwszVal)
        socket.SetId(pwszVal);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"name", &pwszVal)) && pwszVal)
        socket.SetName(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"type", &pwszVal)) && pwszVal)
    {
        if (wcscmp(pwszVal, L"video") == 0)
            socket.SetType(TemplateSocketTypeVideo);
        else if (wcscmp(pwszVal, L"audio") == 0)
            socket.SetType(TemplateSocketTypeAudio);
        else if (wcscmp(pwszVal, L"image") == 0)
            socket.SetType(TemplateSocketTypeImage);
        else if (wcscmp(pwszVal, L"text") == 0)
            socket.SetType(TemplateSocketTypeText);
    }

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"placeholder", &pwszVal)) && pwszVal)
        socket.SetPlaceholderId(pwszVal);

    return S_OK;
}

HRESULT TemplateSocketsParser::ParsePlaceholderAttributes(IXmlReader* pReader, TemplatePlaceholder& placeholder)
{
    LPCWSTR pwszVal = nullptr;

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"id", &pwszVal)) && pwszVal)
        placeholder.SetId(pwszVal);

    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"name", &pwszVal)) && pwszVal)
        placeholder.SetName(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"type", &pwszVal)) && pwszVal)
    {
        if (wcscmp(pwszVal, L"video") == 0)
            placeholder.SetType(TemplateSocketTypeVideo);
        else if (wcscmp(pwszVal, L"audio") == 0)
            placeholder.SetType(TemplateSocketTypeAudio);
        else if (wcscmp(pwszVal, L"image") == 0)
            placeholder.SetType(TemplateSocketTypeImage);
        else if (wcscmp(pwszVal, L"text") == 0)
            placeholder.SetType(TemplateSocketTypeText);
    }

    return S_OK;
}

HRESULT TemplateSocketsParser::ParseSocketProperties(IXmlReader* pReader, TemplateSocket& socket)
{
    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pwszName = nullptr;
            pReader->GetLocalName(&pwszName, nullptr);
            if (pwszName && wcscmp(pwszName, L"property") == 0)
            {
                TemplateProperty prop;
                LPCWSTR pwszKey = nullptr;
                LPCWSTR pwszVal = nullptr;
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"key", &pwszKey)) && pwszKey)
                    prop.SetKey(pwszKey);
                if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"value", &pwszVal)) && pwszVal)
                    prop.SetValue(pwszVal);
                socket.AddProperty(prop);
            }
        }
        else if (nodeType == XmlNodeType_EndElement)
        {
            break;
        }
    }
    return S_OK;
}

// ============================================================================
// BaseTemplate implementation
// ============================================================================

BaseTemplate::BaseTemplate()
    : m_llDurationHns(0)
{
}

BaseTemplate::~BaseTemplate()
{
}

ATL::CString BaseTemplate::GetId() const
{
    return m_strId;
}

void BaseTemplate::SetId(LPCWSTR pszId)
{
    m_strId = pszId ? pszId : L"";
}

ATL::CString BaseTemplate::GetName() const
{
    return m_strName;
}

void BaseTemplate::SetName(LPCWSTR pszName)
{
    m_strName = pszName ? pszName : L"";
}

ATL::CString BaseTemplate::GetTemplateType() const
{
    return L"BaseTemplate";
}

size_t BaseTemplate::GetPropertyCount() const throw()
{
    return m_arrProperties.GetCount();
}

TemplateProperty* BaseTemplate::GetProperty(size_t nIndex)
{
    if (nIndex >= m_arrProperties.GetCount())
        return nullptr;
    return &m_arrProperties.GetAt(nIndex);
}

const TemplateProperty* BaseTemplate::GetProperty(size_t nIndex) const
{
    if (nIndex >= m_arrProperties.GetCount())
        return nullptr;
    return &m_arrProperties.GetAt(nIndex);
}

TemplateProperty* BaseTemplate::FindProperty(LPCWSTR pszKey)
{
    if (!pszKey)
        return nullptr;

    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        if (m_arrProperties.GetAt(i).GetKey().CompareNoCase(pszKey) == 0)
            return &m_arrProperties.GetAt(i);
    }
    return nullptr;
}

size_t BaseTemplate::AddProperty(const TemplateProperty& prop)
{
    return m_arrProperties.Add(prop);
}

void BaseTemplate::RemoveProperty(size_t nIndex)
{
    if (nIndex < m_arrProperties.GetCount())
        m_arrProperties.RemoveAt(nIndex);
}

void BaseTemplate::RemoveAllProperties()
{
    m_arrProperties.RemoveAll();
}

size_t BaseTemplate::GetSocketCount() const throw()
{
    return m_arrSockets.GetCount();
}

TemplateSocket* BaseTemplate::GetSocket(size_t nIndex)
{
    if (nIndex >= m_arrSockets.GetCount())
        return nullptr;
    return &m_arrSockets.GetAt(nIndex);
}

const TemplateSocket* BaseTemplate::GetSocket(size_t nIndex) const
{
    if (nIndex >= m_arrSockets.GetCount())
        return nullptr;
    return &m_arrSockets.GetAt(nIndex);
}

TemplateSocket* BaseTemplate::FindSocket(LPCWSTR pszId)
{
    if (!pszId)
        return nullptr;

    for (size_t i = 0; i < m_arrSockets.GetCount(); ++i)
    {
        if (m_arrSockets.GetAt(i).GetId().CompareNoCase(pszId) == 0)
            return &m_arrSockets.GetAt(i);
    }
    return nullptr;
}

size_t BaseTemplate::AddSocket(const TemplateSocket& socket)
{
    return m_arrSockets.Add(socket);
}

void BaseTemplate::RemoveSocket(size_t nIndex)
{
    if (nIndex < m_arrSockets.GetCount())
        m_arrSockets.RemoveAt(nIndex);
}

void BaseTemplate::RemoveAllSockets()
{
    m_arrSockets.RemoveAll();
}

size_t BaseTemplate::GetPlaceholderCount() const throw()
{
    return m_arrPlaceholders.GetCount();
}

TemplatePlaceholder* BaseTemplate::GetPlaceholder(size_t nIndex)
{
    if (nIndex >= m_arrPlaceholders.GetCount())
        return nullptr;
    return &m_arrPlaceholders.GetAt(nIndex);
}

const TemplatePlaceholder* BaseTemplate::GetPlaceholder(size_t nIndex) const
{
    if (nIndex >= m_arrPlaceholders.GetCount())
        return nullptr;
    return &m_arrPlaceholders.GetAt(nIndex);
}

TemplatePlaceholder* BaseTemplate::FindPlaceholder(LPCWSTR pszId)
{
    if (!pszId)
        return nullptr;

    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        if (m_arrPlaceholders.GetAt(i).GetId().CompareNoCase(pszId) == 0)
            return &m_arrPlaceholders.GetAt(i);
    }
    return nullptr;
}

size_t BaseTemplate::AddPlaceholder(const TemplatePlaceholder& placeholder)
{
    return m_arrPlaceholders.Add(placeholder);
}

void BaseTemplate::RemovePlaceholder(size_t nIndex)
{
    if (nIndex < m_arrPlaceholders.GetCount())
        m_arrPlaceholders.RemoveAt(nIndex);
}

void BaseTemplate::RemoveAllPlaceholders()
{
    m_arrPlaceholders.RemoveAll();
}

HRESULT BaseTemplate::LoadFromXml(IXmlReader* pReader)
{
    if (!pReader)
        return E_INVALIDARG;

    LPCWSTR pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"id", &pwszVal)) && pwszVal)
        SetId(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"name", &pwszVal)) && pwszVal)
        SetName(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"duration", &pwszVal)) && pwszVal)
        SetDurationHns(_wtoi64(pwszVal));

    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pwszName = nullptr;
            pReader->GetLocalName(&pwszName, nullptr);

            if (pwszName && wcscmp(pwszName, L"properties") == 0)
            {
                while (pReader->Read(&nodeType) == S_OK)
                {
                    if (nodeType == XmlNodeType_Element)
                    {
                        LPCWSTR pwszChild = nullptr;
                        pReader->GetLocalName(&pwszChild, nullptr);
                        if (pwszChild && wcscmp(pwszChild, L"property") == 0)
                        {
                            TemplateProperty prop;
                            pwszVal = nullptr;
                            if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"key", &pwszVal)) && pwszVal)
                                prop.SetKey(pwszVal);
                            pwszVal = nullptr;
                            if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"value", &pwszVal)) && pwszVal)
                                prop.SetValue(pwszVal);
                            AddProperty(prop);
                        }
                    }
                    else if (nodeType == XmlNodeType_EndElement)
                    {
                        break;
                    }
                }
            }
            else if (pwszName && wcscmp(pwszName, L"sockets") == 0)
            {
                TemplateSocketsParser parser;
                parser.Parse(pReader, m_arrSockets, m_arrPlaceholders);
            }
        }
        else if (nodeType == XmlNodeType_EndElement)
        {
            break;
        }
    }

    return S_OK;
}

HRESULT BaseTemplate::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_INVALIDARG;

    if (!m_strId.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"id", nullptr, m_strId);

    if (!m_strName.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"name", nullptr, m_strName);

    if (m_llDurationHns != 0)
    {
        WCHAR szValue[64] = { 0 };
        swprintf_s(szValue, L"%lld", m_llDurationHns);
        pWriter->WriteAttributeString(nullptr, L"duration", nullptr, szValue);
    }

    if (m_arrProperties.GetCount() > 0)
    {
        pWriter->WriteStartElement(nullptr, L"properties", nullptr);
        for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
        {
            const TemplateProperty& prop = m_arrProperties.GetAt(i);
            pWriter->WriteStartElement(nullptr, L"property", nullptr);
            pWriter->WriteAttributeString(nullptr, L"key", nullptr, prop.GetKey());
            pWriter->WriteAttributeString(nullptr, L"value", nullptr, prop.GetValue());
            pWriter->WriteEndElement();
        }
        pWriter->WriteEndElement();
    }

    return S_OK;
}

LONGLONG BaseTemplate::GetDurationHns() const throw()
{
    return m_llDurationHns;
}

void BaseTemplate::SetDurationHns(LONGLONG llDuration) throw()
{
    m_llDurationHns = llDuration;
}

// ============================================================================
// BaseX3DTemplate implementation
// ============================================================================

BaseX3DTemplate::BaseX3DTemplate()
    : m_uRenderWidth(1920)
    , m_uRenderHeight(1080)
    , m_dwFrameRate(30)
{
}

BaseX3DTemplate::~BaseX3DTemplate()
{
}

ATL::CString BaseX3DTemplate::GetX3dScenePath() const
{
    return m_strX3dScenePath;
}

void BaseX3DTemplate::SetX3dScenePath(LPCWSTR pszPath)
{
    m_strX3dScenePath = pszPath ? pszPath : L"";
}

const BYTE* BaseX3DTemplate::GetX3dSceneData() const throw()
{
    if (m_arrX3dSceneData.GetCount() == 0)
        return nullptr;
    return m_arrX3dSceneData.GetData();
}

DWORD BaseX3DTemplate::GetX3dSceneDataSize() const throw()
{
    return static_cast<DWORD>(m_arrX3dSceneData.GetCount());
}

HRESULT BaseX3DTemplate::SetX3dSceneData(const BYTE* pData, DWORD dwSize)
{
    if (!pData || dwSize == 0)
        return E_INVALIDARG;

    m_arrX3dSceneData.RemoveAll();
    m_arrX3dSceneData.SetCount(dwSize);
    memcpy(m_arrX3dSceneData.GetData(), pData, dwSize);
    return S_OK;
}

UINT BaseX3DTemplate::GetRenderWidth() const throw()
{
    return m_uRenderWidth;
}

UINT BaseX3DTemplate::GetRenderHeight() const throw()
{
    return m_uRenderHeight;
}

void BaseX3DTemplate::SetRenderDimensions(UINT cx, UINT cy) throw()
{
    m_uRenderWidth = cx;
    m_uRenderHeight = cy;
}

DWORD BaseX3DTemplate::GetFrameRate() const throw()
{
    return m_dwFrameRate;
}

void BaseX3DTemplate::SetFrameRate(DWORD dwFrameRate) throw()
{
    m_dwFrameRate = dwFrameRate;
}

DWORD BaseX3DTemplate::GetKeyframeCount() const throw()
{
    return static_cast<DWORD>(m_arrKeyframes.GetCount());
}

HRESULT BaseX3DTemplate::AddKeyframe(LONGLONG llTimeHns, LPCWSTR pszTargetId,
                                     LPCWSTR pszProperty, LPCWSTR pszValue)
{
    if (!pszTargetId || !pszProperty)
        return E_INVALIDARG;

    Keyframe kf;
    kf.llTimeHns = llTimeHns;
    kf.strTargetId = pszTargetId;
    kf.strProperty = pszProperty;
    kf.strValue = pszValue ? pszValue : L"";
    m_arrKeyframes.Add(kf);
    return S_OK;
}

HRESULT BaseX3DTemplate::ClearKeyframes()
{
    m_arrKeyframes.RemoveAll();
    return S_OK;
}

ATL::CString BaseX3DTemplate::GetTemplateType() const
{
    return L"BaseX3DTemplate";
}

HRESULT BaseX3DTemplate::LoadFromXml(IXmlReader* pReader)
{
    if (!pReader)
        return E_INVALIDARG;

    BaseTemplate::LoadFromXml(pReader);

    LPCWSTR pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"x3dScenePath", &pwszVal)) && pwszVal)
        SetX3dScenePath(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"renderWidth", &pwszVal)) && pwszVal)
    {
        UINT cx = static_cast<UINT>(_wtoi(pwszVal));
        pwszVal = nullptr;
        UINT cy = 1080;
        if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"renderHeight", &pwszVal)) && pwszVal)
            cy = static_cast<UINT>(_wtoi(pwszVal));
        SetRenderDimensions(cx, cy);
    }

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"frameRate", &pwszVal)) && pwszVal)
        SetFrameRate(static_cast<DWORD>(_wtoi(pwszVal)));

    return S_OK;
}

HRESULT BaseX3DTemplate::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_INVALIDARG;

    BaseTemplate::SaveToXml(pWriter);

    if (!m_strX3dScenePath.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"x3dScenePath", nullptr, m_strX3dScenePath);

    WCHAR szValue[64] = { 0 };

    if (m_uRenderWidth != 1920 || m_uRenderHeight != 1080)
    {
        swprintf_s(szValue, L"%u", m_uRenderWidth);
        pWriter->WriteAttributeString(nullptr, L"renderWidth", nullptr, szValue);
        swprintf_s(szValue, L"%u", m_uRenderHeight);
        pWriter->WriteAttributeString(nullptr, L"renderHeight", nullptr, szValue);
    }

    if (m_dwFrameRate != 30)
    {
        swprintf_s(szValue, L"%u", m_dwFrameRate);
        pWriter->WriteAttributeString(nullptr, L"frameRate", nullptr, szValue);
    }

    return S_OK;
}

// ============================================================================
// CompositeX3DTemplate implementation
// ============================================================================

CompositeX3DTemplate::CompositeX3DTemplate()
{
}

CompositeX3DTemplate::~CompositeX3DTemplate()
{
    RemoveAllSubTemplates();
}

size_t CompositeX3DTemplate::GetSubTemplateCount() const throw()
{
    return m_arrSubTemplates.GetCount();
}

BaseX3DTemplate* CompositeX3DTemplate::GetSubTemplate(size_t nIndex)
{
    if (nIndex >= m_arrSubTemplates.GetCount())
        return nullptr;
    return m_arrSubTemplates.GetAt(nIndex);
}

const BaseX3DTemplate* CompositeX3DTemplate::GetSubTemplate(size_t nIndex) const
{
    if (nIndex >= m_arrSubTemplates.GetCount())
        return nullptr;
    return m_arrSubTemplates.GetAt(nIndex);
}

size_t CompositeX3DTemplate::AddSubTemplate(BaseX3DTemplate* pTemplate)
{
    ATLASSERT(pTemplate != nullptr);
    return m_arrSubTemplates.Add(pTemplate);
}

void CompositeX3DTemplate::RemoveSubTemplate(size_t nIndex)
{
    if (nIndex < m_arrSubTemplates.GetCount())
    {
        delete m_arrSubTemplates.GetAt(nIndex);
        m_arrSubTemplates.RemoveAt(nIndex);
    }
}

void CompositeX3DTemplate::RemoveAllSubTemplates()
{
    for (size_t i = 0; i < m_arrSubTemplates.GetCount(); ++i)
    {
        delete m_arrSubTemplates.GetAt(i);
    }
    m_arrSubTemplates.RemoveAll();
}

void CompositeX3DTemplate::MoveSubTemplate(size_t nIndexFrom, size_t nIndexTo)
{
    if (nIndexFrom >= m_arrSubTemplates.GetCount() || nIndexTo >= m_arrSubTemplates.GetCount())
        return;
    if (nIndexFrom == nIndexTo)
        return;

    BaseX3DTemplate* pTemplate = m_arrSubTemplates.GetAt(nIndexFrom);
    m_arrSubTemplates.RemoveAt(nIndexFrom);
    m_arrSubTemplates.InsertAt(nIndexTo, pTemplate);
}

BaseX3DTemplate* CompositeX3DTemplate::FindSubTemplateForPlaceholder(LPCWSTR pszPlaceholderId)
{
    if (!pszPlaceholderId)
        return nullptr;

    for (size_t i = 0; i < m_arrSubTemplates.GetCount(); ++i)
    {
        BaseX3DTemplate* pSub = m_arrSubTemplates.GetAt(i);
        TemplatePlaceholder* pPlaceholder = pSub->FindPlaceholder(pszPlaceholderId);
        if (pPlaceholder)
            return pSub;
    }
    return nullptr;
}

ATL::CString CompositeX3DTemplate::GetTemplateType() const
{
    return L"CompositeX3DTemplate";
}

HRESULT CompositeX3DTemplate::LoadFromXml(IXmlReader* pReader)
{
    if (!pReader)
        return E_INVALIDARG;

    BaseX3DTemplate::LoadFromXml(pReader);
    return S_OK;
}

HRESULT CompositeX3DTemplate::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_INVALIDARG;

    pWriter->WriteStartElement(nullptr, L"compositeX3d", nullptr);
    BaseX3DTemplate::SaveToXml(pWriter);

    for (size_t i = 0; i < m_arrSubTemplates.GetCount(); ++i)
    {
        BaseX3DTemplate* pSub = m_arrSubTemplates.GetAt(i);
        if (pSub)
            pSub->SaveToXml(pWriter);
    }

    pWriter->WriteEndElement();
    return S_OK;
}

// ============================================================================
// ThemeX3DTemplate implementation
// ============================================================================

ThemeX3DTemplate::ThemeX3DTemplate()
    : m_sectionType(SectionIntro)
    , m_flCameraFov(60.0f)
    , m_flCameraNear(0.1f)
    , m_flCameraFar(1000.0f)
    , m_dwAmbientColor(0xFF404040)
    , m_dwDirectionalColor(0xFFFFFFFF)
{
}

ThemeX3DTemplate::~ThemeX3DTemplate()
{
}

ThemeX3DTemplate::SectionType ThemeX3DTemplate::GetSectionType() const throw()
{
    return m_sectionType;
}

void ThemeX3DTemplate::SetSectionType(SectionType type) throw()
{
    m_sectionType = type;
}

ATL::CString ThemeX3DTemplate::GetBackgroundModelPath() const
{
    return m_strBackgroundModelPath;
}

void ThemeX3DTemplate::SetBackgroundModelPath(LPCWSTR pszPath)
{
    m_strBackgroundModelPath = pszPath ? pszPath : L"";
}

float ThemeX3DTemplate::GetCameraFov() const throw() { return m_flCameraFov; }
void ThemeX3DTemplate::SetCameraFov(float flFov) throw() { m_flCameraFov = flFov; }

float ThemeX3DTemplate::GetCameraNear() const throw() { return m_flCameraNear; }
void ThemeX3DTemplate::SetCameraNear(float flNear) throw() { m_flCameraNear = flNear; }

float ThemeX3DTemplate::GetCameraFar() const throw() { return m_flCameraFar; }
void ThemeX3DTemplate::SetCameraFar(float flFar) throw() { m_flCameraFar = flFar; }

DWORD ThemeX3DTemplate::GetAmbientColor() const throw() { return m_dwAmbientColor; }
void ThemeX3DTemplate::SetAmbientColor(DWORD dwColor) throw() { m_dwAmbientColor = dwColor; }

DWORD ThemeX3DTemplate::GetDirectionalColor() const throw() { return m_dwDirectionalColor; }
void ThemeX3DTemplate::SetDirectionalColor(DWORD dwColor) throw() { m_dwDirectionalColor = dwColor; }

ATL::CString ThemeX3DTemplate::GetTemplateType() const
{
    return L"ThemeX3DTemplate";
}

HRESULT ThemeX3DTemplate::LoadFromXml(IXmlReader* pReader)
{
    if (!pReader)
        return E_INVALIDARG;

    BaseX3DTemplate::LoadFromXml(pReader);

    LPCWSTR pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"sectionType", &pwszVal)) && pwszVal)
        SetSectionType(static_cast<SectionType>(_wtoi(pwszVal)));

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"backgroundModel", &pwszVal)) && pwszVal)
        SetBackgroundModelPath(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"cameraFov", &pwszVal)) && pwszVal)
        SetCameraFov(static_cast<float>(_wtof(pwszVal)));

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"cameraNear", &pwszVal)) && pwszVal)
        SetCameraNear(static_cast<float>(_wtof(pwszVal)));

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"cameraFar", &pwszVal)) && pwszVal)
        SetCameraFar(static_cast<float>(_wtof(pwszVal)));

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"ambientColor", &pwszVal)) && pwszVal)
        SetAmbientColor(static_cast<DWORD>(wcstoul(pwszVal, nullptr, 16)));

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"directionalColor", &pwszVal)) && pwszVal)
        SetDirectionalColor(static_cast<DWORD>(wcstoul(pwszVal, nullptr, 16)));

    return S_OK;
}

HRESULT ThemeX3DTemplate::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_INVALIDARG;

    pWriter->WriteStartElement(nullptr, L"themeX3d", nullptr);
    BaseX3DTemplate::SaveToXml(pWriter);

    WCHAR szValue[64] = { 0 };

    if (m_sectionType != SectionIntro)
    {
        swprintf_s(szValue, L"%d", static_cast<int>(m_sectionType));
        pWriter->WriteAttributeString(nullptr, L"sectionType", nullptr, szValue);
    }

    if (!m_strBackgroundModelPath.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"backgroundModel", nullptr, m_strBackgroundModelPath);

    if (m_flCameraFov != 60.0f)
    {
        swprintf_s(szValue, L"%g", m_flCameraFov);
        pWriter->WriteAttributeString(nullptr, L"cameraFov", nullptr, szValue);
    }

    if (m_flCameraNear != 0.1f)
    {
        swprintf_s(szValue, L"%g", m_flCameraNear);
        pWriter->WriteAttributeString(nullptr, L"cameraNear", nullptr, szValue);
    }

    if (m_flCameraFar != 1000.0f)
    {
        swprintf_s(szValue, L"%g", m_flCameraFar);
        pWriter->WriteAttributeString(nullptr, L"cameraFar", nullptr, szValue);
    }

    if (m_dwAmbientColor != 0xFF404040)
    {
        swprintf_s(szValue, L"0x%08X", m_dwAmbientColor);
        pWriter->WriteAttributeString(nullptr, L"ambientColor", nullptr, szValue);
    }

    if (m_dwDirectionalColor != 0xFFFFFFFF)
    {
        swprintf_s(szValue, L"0x%08X", m_dwDirectionalColor);
        pWriter->WriteAttributeString(nullptr, L"directionalColor", nullptr, szValue);
    }

    pWriter->WriteEndElement();
    return S_OK;
}

// ============================================================================
// EffectX3DTemplate implementation
// ============================================================================

EffectX3DTemplate::EffectX3DTemplate()
    : m_dwEffectType(0)
    , m_dwBlendMode(0)
{
}

EffectX3DTemplate::~EffectX3DTemplate()
{
}

DWORD EffectX3DTemplate::GetEffectType() const throw() { return m_dwEffectType; }
void EffectX3DTemplate::SetEffectType(DWORD dwType) throw() { m_dwEffectType = dwType; }

ATL::CString EffectX3DTemplate::GetVertexShaderPath() const
{
    return m_strVertexShaderPath;
}

void EffectX3DTemplate::SetVertexShaderPath(LPCWSTR pszPath)
{
    m_strVertexShaderPath = pszPath ? pszPath : L"";
}

ATL::CString EffectX3DTemplate::GetPixelShaderPath() const
{
    return m_strPixelShaderPath;
}

void EffectX3DTemplate::SetPixelShaderPath(LPCWSTR pszPath)
{
    m_strPixelShaderPath = pszPath ? pszPath : L"";
}

size_t EffectX3DTemplate::GetTextureCount() const throw()
{
    return m_arrTexturePaths.GetCount();
}

ATL::CString EffectX3DTemplate::GetTexturePath(size_t nIndex) const
{
    if (nIndex >= m_arrTexturePaths.GetCount())
        return ATL::CString();
    return m_arrTexturePaths.GetAt(nIndex);
}

void EffectX3DTemplate::AddTexturePath(LPCWSTR pszPath)
{
    m_arrTexturePaths.Add(ATL::CString(pszPath ? pszPath : L""));
}

void EffectX3DTemplate::RemoveAllTextures()
{
    m_arrTexturePaths.RemoveAll();
}

DWORD EffectX3DTemplate::GetBlendMode() const throw() { return m_dwBlendMode; }
void EffectX3DTemplate::SetBlendMode(DWORD dwMode) throw() { m_dwBlendMode = dwMode; }

ATL::CString EffectX3DTemplate::GetTemplateType() const
{
    return L"EffectX3DTemplate";
}

HRESULT EffectX3DTemplate::LoadFromXml(IXmlReader* pReader)
{
    if (!pReader)
        return E_INVALIDARG;

    BaseX3DTemplate::LoadFromXml(pReader);

    LPCWSTR pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"effectType", &pwszVal)) && pwszVal)
        SetEffectType(static_cast<DWORD>(_wtoi(pwszVal)));

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"vertexShader", &pwszVal)) && pwszVal)
        SetVertexShaderPath(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"pixelShader", &pwszVal)) && pwszVal)
        SetPixelShaderPath(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"blendMode", &pwszVal)) && pwszVal)
        SetBlendMode(static_cast<DWORD>(_wtoi(pwszVal)));

    return S_OK;
}

HRESULT EffectX3DTemplate::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_INVALIDARG;

    pWriter->WriteStartElement(nullptr, L"effectX3d", nullptr);
    BaseX3DTemplate::SaveToXml(pWriter);

    WCHAR szValue[64] = { 0 };

    if (m_dwEffectType != 0)
    {
        swprintf_s(szValue, L"%u", m_dwEffectType);
        pWriter->WriteAttributeString(nullptr, L"effectType", nullptr, szValue);
    }

    if (!m_strVertexShaderPath.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"vertexShader", nullptr, m_strVertexShaderPath);

    if (!m_strPixelShaderPath.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"pixelShader", nullptr, m_strPixelShaderPath);

    if (m_dwBlendMode != 0)
    {
        swprintf_s(szValue, L"%u", m_dwBlendMode);
        pWriter->WriteAttributeString(nullptr, L"blendMode", nullptr, szValue);
    }

    for (size_t i = 0; i < m_arrTexturePaths.GetCount(); ++i)
    {
        if (!m_arrTexturePaths.GetAt(i).IsEmpty())
        {
            pWriter->WriteStartElement(nullptr, L"texture", nullptr);
            pWriter->WriteAttributeString(nullptr, L"path", nullptr, m_arrTexturePaths.GetAt(i));
            pWriter->WriteEndElement();
        }
    }

    pWriter->WriteEndElement();
    return S_OK;
}

// ============================================================================
// ExtentX3DTemplate implementation
// ============================================================================

ExtentX3DTemplate::ExtentX3DTemplate()
    : m_durationMode(DurationModeFitToMedia)
    , m_aspectRatioMode(AspectRatioCrop)
{
}

ExtentX3DTemplate::~ExtentX3DTemplate()
{
}

ATL::CString ExtentX3DTemplate::GetMediaSocketId() const
{
    return m_strMediaSocketId;
}

void ExtentX3DTemplate::SetMediaSocketId(LPCWSTR pszId)
{
    m_strMediaSocketId = pszId ? pszId : L"";
}

ATL::CString ExtentX3DTemplate::GetEntranceAnimation() const
{
    return m_strEntranceAnimation;
}

void ExtentX3DTemplate::SetEntranceAnimation(LPCWSTR pszAnimation)
{
    m_strEntranceAnimation = pszAnimation ? pszAnimation : L"";
}

ATL::CString ExtentX3DTemplate::GetExitAnimation() const
{
    return m_strExitAnimation;
}

void ExtentX3DTemplate::SetExitAnimation(LPCWSTR pszAnimation)
{
    m_strExitAnimation = pszAnimation ? pszAnimation : L"";
}

ExtentX3DTemplate::DurationMode ExtentX3DTemplate::GetDurationMode() const throw()
{
    return m_durationMode;
}

void ExtentX3DTemplate::SetDurationMode(DurationMode mode) throw()
{
    m_durationMode = mode;
}

ExtentX3DTemplate::AspectRatioMode ExtentX3DTemplate::GetAspectRatioMode() const throw()
{
    return m_aspectRatioMode;
}

void ExtentX3DTemplate::SetAspectRatioMode(AspectRatioMode mode) throw()
{
    m_aspectRatioMode = mode;
}

ATL::CString ExtentX3DTemplate::GetTemplateType() const
{
    return L"ExtentX3DTemplate";
}

HRESULT ExtentX3DTemplate::LoadFromXml(IXmlReader* pReader)
{
    if (!pReader)
        return E_INVALIDARG;

    BaseX3DTemplate::LoadFromXml(pReader);

    LPCWSTR pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"mediaSocketId", &pwszVal)) && pwszVal)
        SetMediaSocketId(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"entranceAnimation", &pwszVal)) && pwszVal)
        SetEntranceAnimation(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"exitAnimation", &pwszVal)) && pwszVal)
        SetExitAnimation(pwszVal);

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"durationMode", &pwszVal)) && pwszVal)
        SetDurationMode(static_cast<DurationMode>(_wtoi(pwszVal)));

    pwszVal = nullptr;
    if (SUCCEEDED(XmlReaderGetAttribute(pReader, L"aspectRatioMode", &pwszVal)) && pwszVal)
        SetAspectRatioMode(static_cast<AspectRatioMode>(_wtoi(pwszVal)));

    return S_OK;
}

HRESULT ExtentX3DTemplate::SaveToXml(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_INVALIDARG;

    pWriter->WriteStartElement(nullptr, L"extentX3d", nullptr);
    BaseX3DTemplate::SaveToXml(pWriter);

    if (!m_strMediaSocketId.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"mediaSocketId", nullptr, m_strMediaSocketId);

    if (!m_strEntranceAnimation.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"entranceAnimation", nullptr, m_strEntranceAnimation);

    if (!m_strExitAnimation.IsEmpty())
        pWriter->WriteAttributeString(nullptr, L"exitAnimation", nullptr, m_strExitAnimation);

    WCHAR szValue[64] = { 0 };

    if (m_durationMode != DurationModeFitToMedia)
    {
        swprintf_s(szValue, L"%d", static_cast<int>(m_durationMode));
        pWriter->WriteAttributeString(nullptr, L"durationMode", nullptr, szValue);
    }

    if (m_aspectRatioMode != AspectRatioCrop)
    {
        swprintf_s(szValue, L"%d", static_cast<int>(m_aspectRatioMode));
        pWriter->WriteAttributeString(nullptr, L"aspectRatioMode", nullptr, szValue);
    }

    pWriter->WriteEndElement();
    return S_OK;
}

// ============================================================================
// EffectTemplateHandler implementation
// ============================================================================

EffectTemplateHandler::EffectTemplateHandler()
    : m_fDefaultsLoaded(false)
{
}

EffectTemplateHandler::~EffectTemplateHandler()
{
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        delete m_arrEntries.GetAt(i).pTemplate;
    }
    m_arrEntries.RemoveAll();
}

HRESULT EffectTemplateHandler::RegisterEffectTemplate(DWORD dwEffectType, EffectX3DTemplate* pTemplate)
{
    if (!pTemplate)
        return E_INVALIDARG;

    // Check for existing registration
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).dwEffectType == dwEffectType)
        {
            delete m_arrEntries.GetAt(i).pTemplate;
            m_arrEntries.GetAt(i).pTemplate = pTemplate;
            return S_OK;
        }
    }

    EffectTemplateEntry entry;
    entry.dwEffectType = dwEffectType;
    entry.pTemplate = pTemplate;
    m_arrEntries.Add(entry);
    return S_OK;
}

HRESULT EffectTemplateHandler::UnregisterEffectTemplate(DWORD dwEffectType)
{
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).dwEffectType == dwEffectType)
        {
            delete m_arrEntries.GetAt(i).pTemplate;
            m_arrEntries.RemoveAt(i);
            return S_OK;
        }
    }
    return S_FALSE;
}

EffectX3DTemplate* EffectTemplateHandler::FindTemplate(DWORD dwEffectType)
{
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).dwEffectType == dwEffectType)
            return m_arrEntries.GetAt(i).pTemplate;
    }
    return nullptr;
}

const EffectX3DTemplate* EffectTemplateHandler::FindTemplate(DWORD dwEffectType) const
{
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).dwEffectType == dwEffectType)
            return m_arrEntries.GetAt(i).pTemplate;
    }
    return nullptr;
}

HRESULT EffectTemplateHandler::LoadDefaultTemplates()
{
    m_fDefaultsLoaded = true;
    return S_OK;
}

bool EffectTemplateHandler::HasDefaultTemplates() const throw()
{
    return m_fDefaultsLoaded;
}

EffectX3DTemplate* EffectTemplateHandler::CreateTemplate(DWORD dwEffectType)
{
    EffectX3DTemplate* pTemplate = new EffectX3DTemplate();
    pTemplate->SetEffectType(dwEffectType);
    RegisterEffectTemplate(dwEffectType, pTemplate);
    return pTemplate;
}

void EffectTemplateHandler::ReleaseTemplate(EffectX3DTemplate* pTemplate)
{
    if (!pTemplate)
        return;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).pTemplate == pTemplate)
        {
            m_arrEntries.RemoveAt(i);
            delete pTemplate;
            return;
        }
    }
}

size_t EffectTemplateHandler::GetTemplateCount() const throw()
{
    return m_arrEntries.GetCount();
}

// ============================================================================
// ExtentSocketsHandler implementation
// ============================================================================

ExtentSocketsHandler::ExtentSocketsHandler()
    : m_pTemplate(nullptr)
{
}

ExtentSocketsHandler::~ExtentSocketsHandler()
{
}

HRESULT ExtentSocketsHandler::Initialize(BaseTemplate* pTemplate)
{
    if (!pTemplate)
        return E_INVALIDARG;

    m_pTemplate = pTemplate;
    ResetMappings();
    return S_OK;
}

HRESULT ExtentSocketsHandler::MapExtentToSocket(DWORD dwExtentId, LPCWSTR pszSocketId)
{
    if (!m_pTemplate || !pszSocketId)
        return E_INVALIDARG;

    // Verify socket exists
    TemplateSocket* pSocket = m_pTemplate->FindSocket(pszSocketId);
    if (!pSocket)
        return E_INVALIDARG;

    // Remove existing mapping for this extent
    UnmapExtentFromSocket(dwExtentId);

    ExtentSocketMapping mapping;
    mapping.dwExtentId = dwExtentId;
    mapping.strSocketId = pszSocketId;
    m_arrMappings.Add(mapping);

    pSocket->SetConnected(true);
    return S_OK;
}

HRESULT ExtentSocketsHandler::UnmapExtentFromSocket(DWORD dwExtentId)
{
    for (size_t i = 0; i < m_arrMappings.GetCount(); ++i)
    {
        if (m_arrMappings.GetAt(i).dwExtentId == dwExtentId)
        {
            // Disconnect the socket
            if (m_pTemplate)
            {
                TemplateSocket* pSocket = m_pTemplate->FindSocket(m_arrMappings.GetAt(i).strSocketId);
                if (pSocket)
                    pSocket->SetConnected(false);
            }
            m_arrMappings.RemoveAt(i);
            return S_OK;
        }
    }
    return S_FALSE;
}

LPCWSTR ExtentSocketsHandler::FindSocketForExtent(DWORD dwExtentId) const
{
    for (size_t i = 0; i < m_arrMappings.GetCount(); ++i)
    {
        if (m_arrMappings.GetAt(i).dwExtentId == dwExtentId)
            return m_arrMappings.GetAt(i).strSocketId;
    }
    return nullptr;
}

DWORD ExtentSocketsHandler::FindExtentForSocket(LPCWSTR pszSocketId) const
{
    if (!pszSocketId)
        return 0;

    for (size_t i = 0; i < m_arrMappings.GetCount(); ++i)
    {
        if (m_arrMappings.GetAt(i).strSocketId.CompareNoCase(pszSocketId) == 0)
            return m_arrMappings.GetAt(i).dwExtentId;
    }
    return 0;
}

bool ExtentSocketsHandler::IsSocketOccupied(LPCWSTR pszSocketId) const
{
    return FindExtentForSocket(pszSocketId) != 0;
}

size_t ExtentSocketsHandler::GetAvailableSocketCount() const throw()
{
    if (!m_pTemplate)
        return 0;
    return m_pTemplate->GetSocketCount() - GetOccupiedSocketCount();
}

size_t ExtentSocketsHandler::GetOccupiedSocketCount() const throw()
{
    return m_arrMappings.GetCount();
}

size_t ExtentSocketsHandler::GetTotalSocketCount() const throw()
{
    if (!m_pTemplate)
        return 0;
    return m_pTemplate->GetSocketCount();
}

void ExtentSocketsHandler::ResetMappings()
{
    // Disconnect all sockets
    if (m_pTemplate)
    {
        for (size_t i = 0; i < m_arrMappings.GetCount(); ++i)
        {
            TemplateSocket* pSocket = m_pTemplate->FindSocket(m_arrMappings.GetAt(i).strSocketId);
            if (pSocket)
                pSocket->SetConnected(false);
        }
    }
    m_arrMappings.RemoveAll();
}

HRESULT ExtentSocketsHandler::ValidateMappings() const
{
    if (!m_pTemplate)
        return E_INVALIDARG;

    for (size_t i = 0; i < m_arrMappings.GetCount(); ++i)
    {
        TemplateSocket* pSocket = const_cast<BaseTemplate*>(m_pTemplate)->FindSocket(
            m_arrMappings.GetAt(i).strSocketId);
        if (!pSocket)
            return E_INVALIDARG;
    }
    return S_OK;
}

} // namespace StoryboardManager
