#include "pch.h"
/*
 * BoundPlaceholder.cpp
 *
 * Implementation of the serialization placeholder element for bound
 * property resolution during .wlmp project file read/write.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "BoundPlaceholder.h"

namespace StoryboardManager
{

// ============================================================================
// BoundPlaceholder implementation
// ============================================================================

BoundPlaceholder::BoundPlaceholder()
    : m_dwDepth(0)
    , m_dwBoundExtentId(0)
    , m_dwBoundItemId(0)
    , m_fResolved(false)
    , m_resolutionSource(ResolutionSourceNone)
{
}

BoundPlaceholder::BoundPlaceholder(LPCWSTR pszElementName)
    : m_dwDepth(0)
    , m_dwBoundExtentId(0)
    , m_dwBoundItemId(0)
    , m_fResolved(false)
    , m_resolutionSource(ResolutionSourceNone)
{
    SetElementName(pszElementName);
}

BoundPlaceholder::~BoundPlaceholder()
{
}

BoundPlaceholder::BoundPlaceholder(const BoundPlaceholder& other)
    : m_strElementName(other.m_strElementName)
    , m_dwDepth(other.m_dwDepth)
    , m_strBoundTemplateName(other.m_strBoundTemplateName)
    , m_strTemplateSelector(other.m_strTemplateSelector)
    , m_dwBoundExtentId(other.m_dwBoundExtentId)
    , m_dwBoundItemId(other.m_dwBoundItemId)
    , m_properties(other.m_properties)
    , m_fResolved(other.m_fResolved)
    , m_resolutionSource(other.m_resolutionSource)
{
    m_arrAttributes.Copy(other.m_arrAttributes);
}

BoundPlaceholder& BoundPlaceholder::operator=(const BoundPlaceholder& other)
{
    if (this != &other)
    {
        m_strElementName       = other.m_strElementName;
        m_dwDepth              = other.m_dwDepth;
        m_strBoundTemplateName = other.m_strBoundTemplateName;
        m_strTemplateSelector  = other.m_strTemplateSelector;
        m_dwBoundExtentId      = other.m_dwBoundExtentId;
        m_dwBoundItemId        = other.m_dwBoundItemId;
        m_properties           = other.m_properties;
        m_arrAttributes.Copy(other.m_arrAttributes);
        m_fResolved            = other.m_fResolved;
        m_resolutionSource     = other.m_resolutionSource;
    }
    return *this;
}

ATL::CString BoundPlaceholder::GetElementName() const
{
    return m_strElementName;
}

void BoundPlaceholder::SetElementName(LPCWSTR pszName)
{
    m_strElementName = pszName ? pszName : L"";
}

DWORD BoundPlaceholder::GetDepth() const throw()
{
    return m_dwDepth;
}

void BoundPlaceholder::SetDepth(DWORD dwDepth) throw()
{
    m_dwDepth = dwDepth;
}

ATL::CString BoundPlaceholder::GetBoundTemplateName() const
{
    return m_strBoundTemplateName;
}

void BoundPlaceholder::SetBoundTemplateName(LPCWSTR pszTemplateName)
{
    m_strBoundTemplateName = pszTemplateName ? pszTemplateName : L"";
}

ATL::CString BoundPlaceholder::GetTemplateSelector() const
{
    return m_strTemplateSelector;
}

void BoundPlaceholder::SetTemplateSelector(LPCWSTR pszSelector)
{
    m_strTemplateSelector = pszSelector ? pszSelector : L"";
}

DWORD BoundPlaceholder::GetBoundExtentId() const throw()
{
    return m_dwBoundExtentId;
}

void BoundPlaceholder::SetBoundExtentId(DWORD dwExtentId) throw()
{
    m_dwBoundExtentId = dwExtentId;
}

DWORD BoundPlaceholder::GetBoundItemId() const throw()
{
    return m_dwBoundItemId;
}

void BoundPlaceholder::SetBoundItemId(DWORD dwItemId) throw()
{
    m_dwBoundItemId = dwItemId;
}

BoundPropertyDictionary& BoundPlaceholder::GetProperties()
{
    return m_properties;
}

const BoundPropertyDictionary& BoundPlaceholder::GetProperties() const
{
    return m_properties;
}

void BoundPlaceholder::SetProperty(LPCWSTR pszName, const ValueVariantRef& value)
{
    m_properties.SetProperty(pszName, value);
}

bool BoundPlaceholder::GetProperty(LPCWSTR pszName, ValueVariantRef* pValue) const
{
    if (!pValue)
        return false;
    return SUCCEEDED(m_properties.GetProperty(pszName, *pValue));
}

HRESULT BoundPlaceholder::GetAttribute(LPCWSTR pszName, ATL::CString& strValue) const
{
    if (!pszName)
        return E_INVALIDARG;

    for (size_t i = 0; i < m_arrAttributes.GetCount(); ++i)
    {
        if (m_arrAttributes.GetAt(i).strName.CompareNoCase(pszName) == 0)
        {
            strValue = m_arrAttributes.GetAt(i).strValue;
            return S_OK;
        }
    }

    strValue.Empty();
    return DISP_E_UNKNOWNNAME;
}

HRESULT BoundPlaceholder::SetAttribute(LPCWSTR pszName, LPCWSTR pszValue)
{
    if (!pszName)
        return E_INVALIDARG;

    for (size_t i = 0; i < m_arrAttributes.GetCount(); ++i)
    {
        if (m_arrAttributes.GetAt(i).strName.CompareNoCase(pszName) == 0)
        {
            m_arrAttributes.GetAt(i).strValue = pszValue ? pszValue : L"";
            return S_OK;
        }
    }

    Attribute attr;
    attr.strName  = pszName;
    attr.strValue = pszValue ? pszValue : L"";
    m_arrAttributes.Add(attr);
    return S_OK;
}

size_t BoundPlaceholder::GetAttributeCount() const throw()
{
    return m_arrAttributes.GetCount();
}

bool BoundPlaceholder::IsResolved() const throw()
{
    return m_fResolved;
}

void BoundPlaceholder::SetResolved(bool fResolved) throw()
{
    m_fResolved = fResolved;
}

BoundPlaceholder::ResolutionSource BoundPlaceholder::GetResolutionSource() const throw()
{
    return m_resolutionSource;
}

void BoundPlaceholder::SetResolutionSource(ResolutionSource source) throw()
{
    m_resolutionSource = source;
}

HRESULT BoundPlaceholder::ResolveFromDictionary(const BoundPropertyDictionary& dictionary)
{
    if (m_strBoundTemplateName.IsEmpty())
        return E_INVALIDARG;

    ATL::CString strPropName;
    strPropName.Format(L"template.%s", static_cast<LPCWSTR>(m_strBoundTemplateName));

    ValueVariantRef varTemplate;
    if (SUCCEEDED(dictionary.GetProperty(strPropName, varTemplate)))
    {
        SetBoundTemplateName(varTemplate.AsString());
    }

    if (!m_strTemplateSelector.IsEmpty())
    {
        ATL::CString strSelectorProp;
        strSelectorProp.Format(L"selector.%s", static_cast<LPCWSTR>(m_strTemplateSelector));

        ValueVariantRef varSelector;
        if (SUCCEEDED(dictionary.GetProperty(strSelectorProp, varSelector)))
        {
            SetTemplateSelector(varSelector.AsString());
        }
    }

    m_fResolved = true;
    m_resolutionSource = ResolutionSourceDictionary;
    return S_OK;
}

HRESULT BoundPlaceholder::ResolveFromElement(const ModBeginElement* pElement)
{
    if (!pElement)
        return E_POINTER;

    for (size_t i = 0; i < pElement->GetAttributeCount(); ++i)
    {
        ATL::CString strValue;
        // Note: ModBeginElement doesn't expose iteration, so we copy known attributes
        // based on the element type. This is a simplified implementation.
    }

    m_fResolved = true;
    m_resolutionSource = ResolutionSourceTemplate;
    return S_OK;
}

HRESULT BoundPlaceholder::EmitToElement(ModBeginElement* pElement) const
{
    if (!pElement)
        return E_POINTER;

    for (size_t i = 0; i < m_arrAttributes.GetCount(); ++i)
    {
        pElement->SetAttribute(m_arrAttributes.GetAt(i).strName,
                               m_arrAttributes.GetAt(i).strValue);
    }

    return S_OK;
}

HRESULT BoundPlaceholder::EmitAsXml(ATL::CString& strXml) const
{
    strXml.Format(L"<%s", static_cast<LPCWSTR>(m_strElementName));

    for (size_t i = 0; i < m_arrAttributes.GetCount(); ++i)
    {
        ATL::CString strAttr;
        strAttr.Format(L" %s=\"%s\"",
                       static_cast<LPCWSTR>(m_arrAttributes.GetAt(i).strName),
                       static_cast<LPCWSTR>(m_arrAttributes.GetAt(i).strValue));
        strXml += strAttr;
    }

    if (m_arrAttributes.GetCount() > 0 || !m_strBoundTemplateName.IsEmpty())
    {
        strXml += L"/>";
    }
    else
    {
        strXml += L">";
    }

    return S_OK;
}

bool BoundPlaceholder::HasRequiredProperties() const
{
    if (m_strElementName.IsEmpty())
        return false;

    if (m_strBoundTemplateName.IsEmpty() && m_strTemplateSelector.IsEmpty())
    {
        return m_arrAttributes.GetCount() > 0;
    }

    return true;
}

bool BoundPlaceholder::IsValid() const throw()
{
    return !m_strElementName.IsEmpty();
}

void BoundPlaceholder::Reset()
{
    m_fResolved = false;
    m_resolutionSource = ResolutionSourceNone;
}

// ============================================================================
// BoundPlaceholderContainer implementation
// ============================================================================

BoundPlaceholderContainer::BoundPlaceholderContainer()
{
}

BoundPlaceholderContainer::~BoundPlaceholderContainer()
{
    RemoveAllPlaceholders();
}

void BoundPlaceholderContainer::AddPlaceholder(BoundPlaceholder* pPlaceholder)
{
    if (pPlaceholder)
    {
        m_arrPlaceholders.Add(pPlaceholder);
    }
}

void BoundPlaceholderContainer::RemovePlaceholderAt(size_t nIndex)
{
    if (nIndex < m_arrPlaceholders.GetCount())
    {
        delete m_arrPlaceholders.GetAt(nIndex);
        m_arrPlaceholders.RemoveAt(nIndex);
    }
}

void BoundPlaceholderContainer::RemoveAllPlaceholders()
{
    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        delete m_arrPlaceholders.GetAt(i);
    }
    m_arrPlaceholders.RemoveAll();
}

size_t BoundPlaceholderContainer::GetCount() const throw()
{
    return m_arrPlaceholders.GetCount();
}

BoundPlaceholder* BoundPlaceholderContainer::GetAt(size_t nIndex)
{
    if (nIndex >= m_arrPlaceholders.GetCount())
        return nullptr;
    return m_arrPlaceholders.GetAt(nIndex);
}

const BoundPlaceholder* BoundPlaceholderContainer::GetAt(size_t nIndex) const
{
    if (nIndex >= m_arrPlaceholders.GetCount())
        return nullptr;
    return m_arrPlaceholders.GetAt(nIndex);
}

BoundPlaceholder* BoundPlaceholderContainer::FindByElementName(LPCWSTR pszName)
{
    if (!pszName)
        return nullptr;

    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        if (m_arrPlaceholders.GetAt(i)->GetElementName().CompareNoCase(pszName) == 0)
            return m_arrPlaceholders.GetAt(i);
    }
    return nullptr;
}

int BoundPlaceholderContainer::FindIndexByElementName(LPCWSTR pszName) const
{
    if (!pszName)
        return -1;

    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        if (m_arrPlaceholders.GetAt(i)->GetElementName().CompareNoCase(pszName) == 0)
            return static_cast<int>(i);
    }
    return -1;
}

BoundPlaceholder* BoundPlaceholderContainer::FindByExtentId(DWORD dwExtentId)
{
    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        if (m_arrPlaceholders.GetAt(i)->GetBoundExtentId() == dwExtentId)
            return m_arrPlaceholders.GetAt(i);
    }
    return nullptr;
}

int BoundPlaceholderContainer::FindIndexByExtentId(DWORD dwExtentId) const
{
    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        if (m_arrPlaceholders.GetAt(i)->GetBoundExtentId() == dwExtentId)
            return static_cast<int>(i);
    }
    return -1;
}

BoundPlaceholder* BoundPlaceholderContainer::FindByItemId(DWORD dwItemId)
{
    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        if (m_arrPlaceholders.GetAt(i)->GetBoundItemId() == dwItemId)
            return m_arrPlaceholders.GetAt(i);
    }
    return nullptr;
}

size_t BoundPlaceholderContainer::ResolveAll(const BoundPropertyDictionary& dictionary)
{
    size_t nResolved = 0;
    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        if (!m_arrPlaceholders.GetAt(i)->IsResolved())
        {
            if (SUCCEEDED(m_arrPlaceholders.GetAt(i)->ResolveFromDictionary(dictionary)))
            {
                ++nResolved;
            }
        }
    }
    return nResolved;
}

HRESULT BoundPlaceholderContainer::EmitAll(ATL::CAtlArray<ATL::CString>& arrXml) const
{
    arrXml.RemoveAll();

    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        const BoundPlaceholder* pPlaceholder = m_arrPlaceholders.GetAt(i);
        if (pPlaceholder && pPlaceholder->IsResolved())
        {
            ATL::CString strXml;
            HRESULT hr = pPlaceholder->EmitAsXml(strXml);
            if (SUCCEEDED(hr))
            {
                arrXml.Add(strXml);
            }
        }
    }

    return S_OK;
}

size_t BoundPlaceholderContainer::GetResolvedCount() const throw()
{
    size_t nCount = 0;
    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        if (m_arrPlaceholders.GetAt(i)->IsResolved())
            ++nCount;
    }
    return nCount;
}

size_t BoundPlaceholderContainer::GetUnresolvedCount() const throw()
{
    size_t nCount = 0;
    for (size_t i = 0; i < m_arrPlaceholders.GetCount(); ++i)
    {
        if (!m_arrPlaceholders.GetAt(i)->IsResolved())
            ++nCount;
    }
    return nCount;
}

} // namespace StoryboardManager
