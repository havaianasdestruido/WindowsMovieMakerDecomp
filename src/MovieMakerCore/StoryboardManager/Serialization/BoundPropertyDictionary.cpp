/*
 * BoundPropertyDictionary.cpp
 *
 * Implementation of the X3D template property dictionary for the
 * StoryboardManager serialization system.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "BoundPropertyDictionary.h"

namespace StoryboardManager
{

// ============================================================================
// BoundPropertyDictionary implementation
// ============================================================================

BoundPropertyDictionary::BoundPropertyDictionary()
{
}

BoundPropertyDictionary::~BoundPropertyDictionary()
{
}

BoundPropertyDictionary::BoundPropertyDictionary(const BoundPropertyDictionary& other)
    : m_arrProperties(other.m_arrProperties)
{
}

BoundPropertyDictionary& BoundPropertyDictionary::operator=(const BoundPropertyDictionary& other)
{
    if (this != &other)
    {
        m_arrProperties.Copy(other.m_arrProperties);
    }
    return *this;
}

// ============================================================================
// Property access
// ============================================================================

HRESULT BoundPropertyDictionary::GetProperty(LPCWSTR pszName, ValueVariantRef& var) const
{
    int nIndex = FindKey(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    var = m_arrProperties.GetAt(static_cast<size_t>(nIndex)).value;
    return S_OK;
}

HRESULT BoundPropertyDictionary::SetProperty(LPCWSTR pszName, const ValueVariantRef& var)
{
    if (!pszName)
        return E_INVALIDARG;

    int nIndex = FindKey(pszName);
    if (nIndex >= 0)
    {
        m_arrProperties.GetAt(static_cast<size_t>(nIndex)).value = var;
    }
    else
    {
        PropertyEntry entry;
        entry.strKey = pszName;
        entry.value = var;
        m_arrProperties.Add(entry);
    }

    return S_OK;
}

HRESULT BoundPropertyDictionary::SetPropertyString(LPCWSTR pszName, LPCWSTR pszValue)
{
    ValueVariantRef var(pszValue);
    return SetProperty(pszName, var);
}

HRESULT BoundPropertyDictionary::SetPropertyInt(LPCWSTR pszName, int nValue)
{
    ValueVariantRef var(nValue);
    return SetProperty(pszName, var);
}

HRESULT BoundPropertyDictionary::SetPropertyDouble(LPCWSTR pszName, double dblValue)
{
    ValueVariantRef var(dblValue);
    return SetProperty(pszName, var);
}

HRESULT BoundPropertyDictionary::SetPropertyBool(LPCWSTR pszName, bool fValue)
{
    ValueVariantRef var(fValue);
    return SetProperty(pszName, var);
}

// ============================================================================
// Typed getters
// ============================================================================

HRESULT BoundPropertyDictionary::GetString(LPCWSTR pszName, ATL::CString& strValue) const
{
    ValueVariantRef var;
    HRESULT hr = GetProperty(pszName, var);
    if (SUCCEEDED(hr))
    {
        strValue = var.AsString();
    }
    return hr;
}

HRESULT BoundPropertyDictionary::GetInt(LPCWSTR pszName, int* pValue) const
{
    if (!pValue)
        return E_POINTER;

    ValueVariantRef var;
    HRESULT hr = GetProperty(pszName, var);
    if (SUCCEEDED(hr))
    {
        *pValue = var.AsInt();
    }
    return hr;
}

HRESULT BoundPropertyDictionary::GetDouble(LPCWSTR pszName, double* pValue) const
{
    if (!pValue)
        return E_POINTER;

    ValueVariantRef var;
    HRESULT hr = GetProperty(pszName, var);
    if (SUCCEEDED(hr))
    {
        *pValue = var.AsDouble();
    }
    return hr;
}

HRESULT BoundPropertyDictionary::GetBool(LPCWSTR pszName, bool* pValue) const
{
    if (!pValue)
        return E_POINTER;

    ValueVariantRef var;
    HRESULT hr = GetProperty(pszName, var);
    if (SUCCEEDED(hr))
    {
        *pValue = var.AsBool();
    }
    return hr;
}

// ============================================================================
// Existence
// ============================================================================

bool BoundPropertyDictionary::HasProperty(LPCWSTR pszName) const
{
    return FindKey(pszName) >= 0;
}

bool BoundPropertyDictionary::RemoveProperty(LPCWSTR pszName)
{
    int nIndex = FindKey(pszName);
    if (nIndex >= 0)
    {
        m_arrProperties.RemoveAt(static_cast<size_t>(nIndex));
        return true;
    }
    return false;
}

// ============================================================================
// Enumeration
// ============================================================================

size_t BoundPropertyDictionary::GetCount() const throw()
{
    return m_arrProperties.GetCount();
}

LPCWSTR BoundPropertyDictionary::GetKeyNameAt(size_t nIndex) const
{
    if (nIndex >= m_arrProperties.GetCount())
        return nullptr;
    return m_arrProperties.GetAt(nIndex).strKey;
}

const ValueVariantRef& BoundPropertyDictionary::GetValueAt(size_t nIndex) const
{
    static ValueVariantRef s_empty;
    if (nIndex >= m_arrProperties.GetCount())
        return s_empty;
    return m_arrProperties.GetAt(nIndex).value;
}

// ============================================================================
// Bulk operations
// ============================================================================

void BoundPropertyDictionary::Merge(const BoundPropertyDictionary& other)
{
    for (size_t i = 0; i < other.m_arrProperties.GetCount(); ++i)
    {
        const PropertyEntry& entry = other.m_arrProperties.GetAt(i);
        SetProperty(entry.strKey, entry.value);
    }
}

void BoundPropertyDictionary::Clear()
{
    m_arrProperties.RemoveAll();
}

// ============================================================================
// XML serialization support
// ============================================================================

HRESULT BoundPropertyDictionary::WriteToElement(IXmlWriter* pWriter, LPCWSTR pszElementName)
{
    if (!pWriter || !pszElementName)
        return E_POINTER;

    HRESULT hr = pWriter->WriteStartElement(nullptr, pszElementName, nullptr);
    if (FAILED(hr))
        return hr;

    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        const PropertyEntry& entry = m_arrProperties.GetAt(i);
        hr = pWriter->WriteAttributeString(nullptr, entry.strKey, nullptr,
            entry.value.AsString());
        if (FAILED(hr))
            break;
    }

    if (SUCCEEDED(hr))
    {
        hr = pWriter->WriteEndElement(nullptr);
    }

    return hr;
}

HRESULT BoundPropertyDictionary::ReadFromElement(IXmlReader* pReader, LPCWSTR pszElementName)
{
    UNREFERENCED_PARAMETER(pszElementName);
    if (!pReader)
        return E_POINTER;

    HRESULT hr = S_OK;

    while (pReader->MoveToNextAttribute() == S_OK)
    {
        LPCWSTR pszAttrName = nullptr;
        LPCWSTR pszAttrValue = nullptr;

        hr = pReader->GetLocalName(&pszAttrName, nullptr);
        if (FAILED(hr) || !pszAttrName)
            continue;

        hr = pReader->GetValue(&pszAttrValue, nullptr);
        if (FAILED(hr) || !pszAttrValue)
            continue;

        ValueVariantRef var(pszAttrValue);
        SetProperty(pszAttrName, var);
    }

    pReader->MoveToElement();
    return S_OK;
}

// ============================================================================
// Template binding validation
// ============================================================================

bool BoundPropertyDictionary::ValidateRequiredProperties(
    const LPCWSTR* ppszRequired, size_t cRequired) const
{
    if (!ppszRequired)
        return true;

    for (size_t i = 0; i < cRequired; ++i)
    {
        if (!HasProperty(ppszRequired[i]))
            return false;
    }

    return true;
}

// ============================================================================
// FindKey (internal)
// ============================================================================

int BoundPropertyDictionary::FindKey(LPCWSTR pszName) const
{
    if (!pszName)
        return -1;

    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        if (m_arrProperties.GetAt(i).strKey.CompareNoCase(pszName) == 0)
            return static_cast<int>(i);
    }

    return -1;
}

} // namespace StoryboardManager
