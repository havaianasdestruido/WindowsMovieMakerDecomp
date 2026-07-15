#include "pch.h"

/*
 * LegacyProperty.cpp
 *
 * Implementation of ComplexProperty, SingleProperty, and TransformProperty.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "LegacyProperty.h"

// ============================================================================
// SingleProperty implementation
// ============================================================================

SingleProperty::SingleProperty()
    : m_dwId(0)
    , m_bDirty(false)
{
}

SingleProperty::SingleProperty(LPCWSTR pszName)
    : m_dwId(0)
    , m_bDirty(false)
{
    m_strName = pszName ? pszName : L"";
}

SingleProperty::~SingleProperty()
{
}

void SingleProperty::SetName(LPCWSTR pszName)
{
    m_strName = pszName ? pszName : L"";
}

ATL::CString SingleProperty::GetName() const
{
    return m_strName;
}

void SingleProperty::SetId(DWORD dwId)
{
    m_dwId = dwId;
}

DWORD SingleProperty::GetId() const throw()
{
    return m_dwId;
}

void SingleProperty::SetValue(const PropertyValue& value)
{
    m_value = value;
    m_bDirty = true;
}

PropertyValue SingleProperty::GetValue() const
{
    return m_value;
}

void SingleProperty::SetInt(int nValue)
{
    m_value = PropertyValue(nValue);
    m_bDirty = true;
}

int SingleProperty::GetInt() const throw()
{
    return m_value.AsInt();
}

void SingleProperty::SetFloat(float flValue)
{
    m_value = PropertyValue(flValue);
    m_bDirty = true;
}

float SingleProperty::GetFloat() const throw()
{
    return m_value.AsFloat();
}

void SingleProperty::SetString(LPCWSTR pszValue)
{
    m_value = PropertyValue(pszValue);
    m_bDirty = true;
}

ATL::CString SingleProperty::GetString() const
{
    return m_value.AsString();
}

void SingleProperty::SetColor(COLORREF crValue)
{
    m_value = PropertyValue(crValue);
    m_bDirty = true;
}

COLORREF SingleProperty::GetColor() const throw()
{
    return m_value.AsColor();
}

void SingleProperty::SetBool(bool bValue)
{
    m_value = PropertyValue(bValue);
    m_bDirty = true;
}

bool SingleProperty::GetBool() const throw()
{
    return m_value.AsBool();
}

void SingleProperty::SetDirty(bool bDirty)
{
    m_bDirty = bDirty;
}

bool SingleProperty::IsDirty() const throw()
{
    return m_bDirty;
}

SingleProperty* SingleProperty::Clone() const
{
    SingleProperty* pClone = new SingleProperty();
    pClone->m_strName = m_strName;
    pClone->m_dwId = m_dwId;
    pClone->m_value = m_value;
    pClone->m_bDirty = m_bDirty;
    return pClone;
}

// ============================================================================
// ComplexProperty implementation
// ============================================================================

ComplexProperty::ComplexProperty()
    : m_dwType(0)
    , m_bDirty(false)
{
}

ComplexProperty::~ComplexProperty()
{
    RemoveAllProperties();
}

void ComplexProperty::SetName(LPCWSTR pszName)
{
    m_strName = pszName ? pszName : L"";
}

ATL::CString ComplexProperty::GetName() const
{
    return m_strName;
}

void ComplexProperty::SetType(DWORD dwType)
{
    m_dwType = dwType;
}

DWORD ComplexProperty::GetType() const throw()
{
    return m_dwType;
}

void ComplexProperty::AddProperty(SingleProperty* pProperty)
{
    if (pProperty)
    {
        m_properties.push_back(pProperty);
        m_bDirty = true;
    }
}

void ComplexProperty::RemoveProperty(LPCWSTR pszName)
{
    if (!pszName)
        return;

    for (auto it = m_properties.begin(); it != m_properties.end(); ++it)
    {
        if ((*it)->GetName().CompareNoCase(pszName) == 0)
        {
            delete *it;
            m_properties.erase(it);
            m_bDirty = true;
            return;
        }
    }
}

void ComplexProperty::RemoveAllProperties()
{
    for (auto pProp : m_properties)
        delete pProp;
    m_properties.clear();
    m_bDirty = true;
}

size_t ComplexProperty::GetPropertyCount() const throw()
{
    return m_properties.size();
}

SingleProperty* ComplexProperty::GetProperty(LPCWSTR pszName) const
{
    if (!pszName)
        return nullptr;

    for (auto pProp : m_properties)
    {
        if (pProp->GetName().CompareNoCase(pszName) == 0)
            return pProp;
    }
    return nullptr;
}

SingleProperty* ComplexProperty::GetPropertyAt(size_t nIndex) const
{
    if (nIndex < m_properties.size())
        return m_properties[nIndex];
    return nullptr;
}

SingleProperty* ComplexProperty::GetPropertyById(DWORD dwId) const
{
    for (auto pProp : m_properties)
    {
        if (pProp->GetId() == dwId)
            return pProp;
    }
    return nullptr;
}

bool ComplexProperty::HasProperty(LPCWSTR pszName) const
{
    return (GetProperty(pszName) != nullptr);
}

void ComplexProperty::SetSubValue(LPCWSTR pszName, const PropertyValue& value)
{
    SingleProperty* pProp = GetProperty(pszName);
    if (pProp)
    {
        pProp->SetValue(value);
    }
    else
    {
        pProp = new SingleProperty(pszName);
        pProp->SetValue(value);
        AddProperty(pProp);
    }
    m_bDirty = true;
}

PropertyValue ComplexProperty::GetSubValue(LPCWSTR pszName, const PropertyValue& defaultValue) const
{
    SingleProperty* pProp = GetProperty(pszName);
    if (pProp)
        return pProp->GetValue();
    return defaultValue;
}

bool ComplexProperty::IsDirty() const throw()
{
    if (m_bDirty)
        return true;

    for (auto pProp : m_properties)
    {
        if (pProp->IsDirty())
            return true;
    }
    return false;
}

HRESULT ComplexProperty::SaveToStream(IStream* pStream)
{
    if (!pStream)
        return E_POINTER;

    // In the full implementation, this would serialize all properties
    // to the stream using an XML or binary format.

    UNREFERENCED_PARAMETER(pStream);
    return S_OK;
}

HRESULT ComplexProperty::LoadFromStream(IStream* pStream)
{
    if (!pStream)
        return E_POINTER;

    RemoveAllProperties();
    return S_OK;
}

// ============================================================================
// TransformProperty implementation
// ============================================================================

TransformProperty::TransformProperty()
    : m_bAnimated(false)
    , m_flKeyframeTime(0.0f)
    , m_bDirty(false)
{
}

TransformProperty::~TransformProperty()
{
}

void TransformProperty::SetName(LPCWSTR pszName)
{
    m_strName = pszName ? pszName : L"";
}

ATL::CString TransformProperty::GetName() const
{
    return m_strName;
}

void TransformProperty::SetTransform(const LegacyTransform& transform)
{
    m_transform = transform;
    m_bDirty = true;
}

LegacyTransform TransformProperty::GetTransform() const
{
    return m_transform;
}

void TransformProperty::SetOffsetX(float flX)
{
    m_transform.SetOffsetX(flX);
    m_bDirty = true;
}

float TransformProperty::GetOffsetX() const throw()
{
    return m_transform.GetOffsetX();
}

void TransformProperty::SetOffsetY(float flY)
{
    m_transform.SetOffsetY(flY);
    m_bDirty = true;
}

float TransformProperty::GetOffsetY() const throw()
{
    return m_transform.GetOffsetY();
}

void TransformProperty::SetRotation(float flDegrees)
{
    m_transform.SetRotation(flDegrees);
    m_bDirty = true;
}

float TransformProperty::GetRotation() const throw()
{
    return m_transform.GetRotation();
}

void TransformProperty::SetScale(float flScaleX, float flScaleY)
{
    m_transform.SetScale(flScaleX, flScaleY);
    m_bDirty = true;
}

float TransformProperty::GetScaleX() const throw()
{
    return m_transform.GetScaleX();
}

float TransformProperty::GetScaleY() const throw()
{
    return m_transform.GetScaleY();
}

void TransformProperty::SetAnimated(bool bAnimated)
{
    m_bAnimated = bAnimated;
}

bool TransformProperty::IsAnimated() const throw()
{
    return m_bAnimated;
}

void TransformProperty::SetKeyframeTime(float flTime)
{
    m_flKeyframeTime = flTime;
}

float TransformProperty::GetKeyframeTime() const throw()
{
    return m_flKeyframeTime;
}

void TransformProperty::SetDirty(bool bDirty)
{
    m_bDirty = bDirty;
}

bool TransformProperty::IsDirty() const throw()
{
    return m_bDirty;
}
