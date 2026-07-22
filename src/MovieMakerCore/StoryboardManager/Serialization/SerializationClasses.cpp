#include "pch.h"
/*
 * SerializationClasses.cpp
 *
 * Stub implementations for the StoryboardManager serialization/binding
 * framework RTTI classes. Provides placeholder implementations for ~100
 * classes identified in the original Windows Live Movie Maker 2012 binary
 * but missing from the source recreation.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "SerializationClasses.h"
#include "SerializationWriter.h"
#include "SerializationReader.h"
#include "ModElements.h"
#include "../MediaItems/MediaItem.h"
#include "../Templates.h"

namespace StoryboardManager
{

// ============================================================================
// ModAttribute implementation
// ============================================================================

ModAttribute::ModAttribute()
{
}

ModAttribute::~ModAttribute()
{
}

HRESULT ModAttribute::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    UNREFERENCED_PARAMETER(pszAttrName);
    return S_OK;
}

HRESULT ModAttribute::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    UNREFERENCED_PARAMETER(pszAttrName);
    return S_OK;
}

bool ModAttribute::IsDefault() const throw()
{
    return true;
}

ATL::CString ModAttribute::GetName() const
{
    return m_strName;
}

void ModAttribute::SetName(LPCWSTR pszName)
{
    m_strName = pszName ? pszName : L"";
}

// ============================================================================
// ModAttributeString implementation
// ============================================================================

ModAttributeString::ModAttributeString()
{
}

ModAttributeString::~ModAttributeString()
{
}

HRESULT ModAttributeString::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        m_strValue = strValue;
    }
    return hr;
}

HRESULT ModAttributeString::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    return pElement->SetAttribute(pszAttrName, m_strValue);
}

bool ModAttributeString::IsDefault() const throw()
{
    return m_strValue.IsEmpty();
}

ATL::CString ModAttributeString::GetValue() const
{
    return m_strValue;
}

void ModAttributeString::SetValue(LPCWSTR pszValue)
{
    m_strValue = pszValue ? pszValue : L"";
}

// ============================================================================
// ModAttributeInt implementation
// ============================================================================

ModAttributeInt::ModAttributeInt()
    : m_nValue(0)
{
}

ModAttributeInt::~ModAttributeInt()
{
}

HRESULT ModAttributeInt::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        m_nValue = _wtoi(strValue);
    }
    return hr;
}

HRESULT ModAttributeInt::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    strValue.Format(L"%d", m_nValue);
    return pElement->SetAttribute(pszAttrName, strValue);
}

bool ModAttributeInt::IsDefault() const throw()
{
    return m_nValue == 0;
}

int ModAttributeInt::GetValue() const throw()
{
    return m_nValue;
}

void ModAttributeInt::SetValue(int nValue) throw()
{
    m_nValue = nValue;
}

// ============================================================================
// ModAttributeBool implementation
// ============================================================================

ModAttributeBool::ModAttributeBool()
    : m_fValue(false)
{
}

ModAttributeBool::~ModAttributeBool()
{
}

HRESULT ModAttributeBool::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        m_fValue = (_wtoi(strValue) != 0);
    }
    return hr;
}

HRESULT ModAttributeBool::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    return pElement->SetAttribute(pszAttrName, m_fValue ? L"1" : L"0");
}

bool ModAttributeBool::IsDefault() const throw()
{
    return !m_fValue;
}

bool ModAttributeBool::GetValue() const throw()
{
    return m_fValue;
}

void ModAttributeBool::SetValue(bool fValue) throw()
{
    m_fValue = fValue;
}

// ============================================================================
// ModAttributeFloat implementation
// ============================================================================

ModAttributeFloat::ModAttributeFloat()
    : m_flValue(0.0f)
{
}

ModAttributeFloat::~ModAttributeFloat()
{
}

HRESULT ModAttributeFloat::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        m_flValue = static_cast<float>(_wtof(strValue));
    }
    return hr;
}

HRESULT ModAttributeFloat::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    strValue.Format(L"%g", m_flValue);
    return pElement->SetAttribute(pszAttrName, strValue);
}

bool ModAttributeFloat::IsDefault() const throw()
{
    return m_flValue == 0.0f;
}

float ModAttributeFloat::GetValue() const throw()
{
    return m_flValue;
}

void ModAttributeFloat::SetValue(float flValue) throw()
{
    m_flValue = flValue;
}

// ============================================================================
// ModAttributeDouble implementation
// ============================================================================

ModAttributeDouble::ModAttributeDouble()
    : m_dblValue(0.0)
{
}

ModAttributeDouble::~ModAttributeDouble()
{
}

HRESULT ModAttributeDouble::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        m_dblValue = _wtof(strValue);
    }
    return hr;
}

HRESULT ModAttributeDouble::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    strValue.Format(L"%g", m_dblValue);
    return pElement->SetAttribute(pszAttrName, strValue);
}

bool ModAttributeDouble::IsDefault() const throw()
{
    return m_dblValue == 0.0;
}

double ModAttributeDouble::GetValue() const throw()
{
    return m_dblValue;
}

void ModAttributeDouble::SetValue(double dblValue) throw()
{
    m_dblValue = dblValue;
}

// ============================================================================
// ModAttributeGuid implementation
// ============================================================================

ModAttributeGuid::ModAttributeGuid()
{
    ZeroMemory(&m_guid, sizeof(GUID));
}

ModAttributeGuid::~ModAttributeGuid()
{
}

HRESULT ModAttributeGuid::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr) && !strValue.IsEmpty())
    {
        CLSIDFromString(strValue, &m_guid);
    }
    return hr;
}

HRESULT ModAttributeGuid::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    LPOLESTR pszGuid = nullptr;
    HRESULT hr = StringFromIID(m_guid, &pszGuid);
    if (SUCCEEDED(hr) && pszGuid)
    {
        hr = pElement->SetAttribute(pszAttrName, pszGuid);
        CoTaskMemFree(pszGuid);
    }
    return hr;
}

bool ModAttributeGuid::IsDefault() const throw()
{
    static const GUID GUID_NULL = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
    return memcmp(&m_guid, &GUID_NULL, sizeof(GUID)) == 0;
}

GUID ModAttributeGuid::GetValue() const throw()
{
    return m_guid;
}

void ModAttributeGuid::SetValue(const GUID& guid) throw()
{
    m_guid = guid;
}

// ============================================================================
// ModAttributeTime implementation
// ============================================================================

ModAttributeTime::ModAttributeTime()
    : m_llTime(0)
{
}

ModAttributeTime::~ModAttributeTime()
{
}

HRESULT ModAttributeTime::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        m_llTime = _wtoi64(strValue);
    }
    return hr;
}

HRESULT ModAttributeTime::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    strValue.Format(L"%lld", m_llTime);
    return pElement->SetAttribute(pszAttrName, strValue);
}

bool ModAttributeTime::IsDefault() const throw()
{
    return m_llTime == 0;
}

LONGLONG ModAttributeTime::GetValue() const throw()
{
    return m_llTime;
}

void ModAttributeTime::SetValue(LONGLONG llTime) throw()
{
    m_llTime = llTime;
}

// ============================================================================
// ModAttributeBlob implementation
// ============================================================================

ModAttributeBlob::ModAttributeBlob()
{
}

ModAttributeBlob::~ModAttributeBlob()
{
}

HRESULT ModAttributeBlob::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr) && !strValue.IsEmpty())
    {
        hr = SerializationTextEncoder::DecodeBase64(strValue, m_arrData);
    }
    return hr;
}

HRESULT ModAttributeBlob::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    if (m_arrData.GetCount() == 0)
        return pElement->SetAttribute(pszAttrName, L"");

    ATL::CString strValue;
    HRESULT hr = SerializationTextEncoder::EncodeBase64(
        m_arrData.GetData(), static_cast<DWORD>(m_arrData.GetCount()), strValue);
    if (SUCCEEDED(hr))
    {
        hr = pElement->SetAttribute(pszAttrName, strValue);
    }
    return hr;
}

bool ModAttributeBlob::IsDefault() const throw()
{
    return m_arrData.GetCount() == 0;
}

const BYTE* ModAttributeBlob::GetData() const throw()
{
    if (m_arrData.GetCount() == 0)
        return nullptr;
    return &m_arrData.GetAt(0);
}

DWORD ModAttributeBlob::GetSize() const throw()
{
    return static_cast<DWORD>(m_arrData.GetCount());
}

HRESULT ModAttributeBlob::SetData(const BYTE* pcbData, DWORD cbSize)
{
    if (!pcbData && cbSize > 0)
        return E_INVALIDARG;

    m_arrData.RemoveAll();
    if (cbSize > 0)
    {
        m_arrData.SetCount(cbSize);
        CopyMemory(&m_arrData.GetAt(0), pcbData, cbSize);
    }
    return S_OK;
}

// ============================================================================
// ModAttributeRect implementation
// ============================================================================

ModAttributeRect::ModAttributeRect()
    : m_dblX(0.0)
    , m_dblY(0.0)
    , m_dblWidth(0.0)
    , m_dblHeight(0.0)
{
}

ModAttributeRect::~ModAttributeRect()
{
}

HRESULT ModAttributeRect::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        if (swscanf_s(strValue, L"%lf,%lf,%lf,%lf",
                       &m_dblX, &m_dblY, &m_dblWidth, &m_dblHeight) != 4)
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT ModAttributeRect::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    strValue.Format(L"%g,%g,%g,%g", m_dblX, m_dblY, m_dblWidth, m_dblHeight);
    return pElement->SetAttribute(pszAttrName, strValue);
}

bool ModAttributeRect::IsDefault() const throw()
{
    return m_dblX == 0.0 && m_dblY == 0.0 && m_dblWidth == 0.0 && m_dblHeight == 0.0;
}

double ModAttributeRect::GetX() const throw() { return m_dblX; }
double ModAttributeRect::GetY() const throw() { return m_dblY; }
double ModAttributeRect::GetWidth() const throw() { return m_dblWidth; }
double ModAttributeRect::GetHeight() const throw() { return m_dblHeight; }

void ModAttributeRect::SetRect(double dblX, double dblY, double dblWidth, double dblHeight) throw()
{
    m_dblX = dblX;
    m_dblY = dblY;
    m_dblWidth = dblWidth;
    m_dblHeight = dblHeight;
}

// ============================================================================
// ModAttributePoint implementation
// ============================================================================

ModAttributePoint::ModAttributePoint()
    : m_dblX(0.0)
    , m_dblY(0.0)
{
}

ModAttributePoint::~ModAttributePoint()
{
}

HRESULT ModAttributePoint::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        if (swscanf_s(strValue, L"%lf,%lf", &m_dblX, &m_dblY) != 2)
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT ModAttributePoint::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    strValue.Format(L"%g,%g", m_dblX, m_dblY);
    return pElement->SetAttribute(pszAttrName, strValue);
}

bool ModAttributePoint::IsDefault() const throw()
{
    return m_dblX == 0.0 && m_dblY == 0.0;
}

double ModAttributePoint::GetX() const throw() { return m_dblX; }
double ModAttributePoint::GetY() const throw() { return m_dblY; }

void ModAttributePoint::SetPoint(double dblX, double dblY) throw()
{
    m_dblX = dblX;
    m_dblY = dblY;
}

// ============================================================================
// ModAttributeSize implementation
// ============================================================================

ModAttributeSize::ModAttributeSize()
    : m_dblWidth(0.0)
    , m_dblHeight(0.0)
{
}

ModAttributeSize::~ModAttributeSize()
{
}

HRESULT ModAttributeSize::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        if (swscanf_s(strValue, L"%lf,%lf", &m_dblWidth, &m_dblHeight) != 2)
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT ModAttributeSize::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    strValue.Format(L"%g,%g", m_dblWidth, m_dblHeight);
    return pElement->SetAttribute(pszAttrName, strValue);
}

bool ModAttributeSize::IsDefault() const throw()
{
    return m_dblWidth == 0.0 && m_dblHeight == 0.0;
}

double ModAttributeSize::GetWidth() const throw() { return m_dblWidth; }
double ModAttributeSize::GetHeight() const throw() { return m_dblHeight; }

void ModAttributeSize::SetSize(double dblWidth, double dblHeight) throw()
{
    m_dblWidth = dblWidth;
    m_dblHeight = dblHeight;
}

// ============================================================================
// ModAttributeColor implementation
// ============================================================================

ModAttributeColor::ModAttributeColor()
    : m_dwColor(0)
{
}

ModAttributeColor::~ModAttributeColor()
{
}

HRESULT ModAttributeColor::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        m_dwColor = static_cast<DWORD>(_wtoi64(strValue));
    }
    return hr;
}

HRESULT ModAttributeColor::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    strValue.Format(L"%u", m_dwColor);
    return pElement->SetAttribute(pszAttrName, strValue);
}

bool ModAttributeColor::IsDefault() const throw()
{
    return m_dwColor == 0;
}

DWORD ModAttributeColor::GetColor() const throw()
{
    return m_dwColor;
}

void ModAttributeColor::SetColor(DWORD dwColor) throw()
{
    m_dwColor = dwColor;
}

// ============================================================================
// ModAttributeVector implementation
// ============================================================================

ModAttributeVector::ModAttributeVector()
    : m_dblX(0.0)
    , m_dblY(0.0)
    , m_dblZ(0.0)
{
}

ModAttributeVector::~ModAttributeVector()
{
}

HRESULT ModAttributeVector::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr))
    {
        if (swscanf_s(strValue, L"%lf,%lf,%lf", &m_dblX, &m_dblY, &m_dblZ) != 3)
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT ModAttributeVector::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    strValue.Format(L"%g,%g,%g", m_dblX, m_dblY, m_dblZ);
    return pElement->SetAttribute(pszAttrName, strValue);
}

bool ModAttributeVector::IsDefault() const throw()
{
    return m_dblX == 0.0 && m_dblY == 0.0 && m_dblZ == 0.0;
}

double ModAttributeVector::GetX() const throw() { return m_dblX; }
double ModAttributeVector::GetY() const throw() { return m_dblY; }
double ModAttributeVector::GetZ() const throw() { return m_dblZ; }

void ModAttributeVector::SetVector(double dblX, double dblY, double dblZ) throw()
{
    m_dblX = dblX;
    m_dblY = dblY;
    m_dblZ = dblZ;
}

// ============================================================================
// ModAttributeMatrix implementation
// ============================================================================

ModAttributeMatrix::ModAttributeMatrix()
{
    ZeroMemory(m_values, sizeof(m_values));
    m_values[0] = 1.0;
    m_values[5] = 1.0;
    m_values[10] = 1.0;
    m_values[15] = 1.0;
}

ModAttributeMatrix::~ModAttributeMatrix()
{
}

HRESULT ModAttributeMatrix::ReadFromElement(ModElementBase* pElement, LPCWSTR pszAttrName)
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = pElement->GetAttribute(pszAttrName, strValue);
    if (SUCCEEDED(hr) && !strValue.IsEmpty())
    {
        if (swscanf_s(strValue, L"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                       &m_values[0], &m_values[1], &m_values[2], &m_values[3],
                       &m_values[4], &m_values[5], &m_values[6], &m_values[7],
                       &m_values[8], &m_values[9], &m_values[10], &m_values[11],
                       &m_values[12], &m_values[13], &m_values[14], &m_values[15]) != 16)
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT ModAttributeMatrix::WriteToElement(ModElementBase* pElement, LPCWSTR pszAttrName) const
{
    if (!pElement || !pszAttrName)
        return E_INVALIDARG;

    ATL::CString strValue;
    strValue.Format(L"%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g",
                    m_values[0], m_values[1], m_values[2], m_values[3],
                    m_values[4], m_values[5], m_values[6], m_values[7],
                    m_values[8], m_values[9], m_values[10], m_values[11],
                    m_values[12], m_values[13], m_values[14], m_values[15]);
    return pElement->SetAttribute(pszAttrName, strValue);
}

bool ModAttributeMatrix::IsDefault() const throw()
{
    static const double identity[16] = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
    return memcmp(m_values, identity, sizeof(m_values)) == 0;
}

const double* ModAttributeMatrix::GetMatrix() const throw()
{
    return m_values;
}

void ModAttributeMatrix::SetIdentity() throw()
{
    ZeroMemory(m_values, sizeof(m_values));
    m_values[0] = 1.0;
    m_values[5] = 1.0;
    m_values[10] = 1.0;
    m_values[15] = 1.0;
}

// ============================================================================
// BoundProperty implementation
// ============================================================================

BoundProperty::BoundProperty()
    : m_fBound(false)
{
}

BoundProperty::~BoundProperty()
{
}

HRESULT BoundProperty::Resolve(const BoundPropertyDictionary& dictionary)
{
    UNREFERENCED_PARAMETER(dictionary);
    return S_OK;
}

HRESULT BoundProperty::Serialize(ModBeginElement* pElement) const
{
    UNREFERENCED_PARAMETER(pElement);
    return S_OK;
}

bool BoundProperty::IsBound() const throw()
{
    return m_fBound;
}

ATL::CString BoundProperty::GetPropertyName() const
{
    return m_strPropertyName;
}

void BoundProperty::SetPropertyName(LPCWSTR pszName)
{
    m_strPropertyName = pszName ? pszName : L"";
}

ATL::CString BoundProperty::GetTargetElement() const
{
    return m_strTargetElement;
}

void BoundProperty::SetTargetElement(LPCWSTR pszElement)
{
    m_strTargetElement = pszElement ? pszElement : L"";
}

// ============================================================================
// BoundPropertyString implementation
// ============================================================================

BoundPropertyString::BoundPropertyString()
{
}

BoundPropertyString::~BoundPropertyString()
{
}

HRESULT BoundPropertyString::Resolve(const BoundPropertyDictionary& dictionary)
{
    if (m_strPropertyName.IsEmpty())
        return E_INVALIDARG;

    ATL::CString strValue;
    HRESULT hr = dictionary.GetString(m_strPropertyName, strValue);
    if (SUCCEEDED(hr))
    {
        m_strValue = strValue;
        m_fBound = true;
    }
    return hr;
}

HRESULT BoundPropertyString::Serialize(ModBeginElement* pElement) const
{
    if (!pElement)
        return E_POINTER;

    return pElement->SetAttribute(m_strPropertyName, m_strValue);
}

ATL::CString BoundPropertyString::GetStringValue() const
{
    return m_strValue;
}

void BoundPropertyString::SetStringValue(LPCWSTR pszValue)
{
    m_strValue = pszValue ? pszValue : L"";
}

// ============================================================================
// BoundPropertyInt implementation
// ============================================================================

BoundPropertyInt::BoundPropertyInt()
    : m_nValue(0)
{
}

BoundPropertyInt::~BoundPropertyInt()
{
}

HRESULT BoundPropertyInt::Resolve(const BoundPropertyDictionary& dictionary)
{
    if (m_strPropertyName.IsEmpty())
        return E_INVALIDARG;

    int nValue = 0;
    HRESULT hr = dictionary.GetInt(m_strPropertyName, &nValue);
    if (SUCCEEDED(hr))
    {
        m_nValue = nValue;
        m_fBound = true;
    }
    return hr;
}

HRESULT BoundPropertyInt::Serialize(ModBeginElement* pElement) const
{
    if (!pElement)
        return E_POINTER;

    WCHAR szBuf[32];
    _itow_s(m_nValue, szBuf, 10);
    return pElement->SetAttribute(m_strPropertyName, szBuf);
}

int BoundPropertyInt::GetIntValue() const throw()
{
    return m_nValue;
}

void BoundPropertyInt::SetIntValue(int nValue) throw()
{
    m_nValue = nValue;
}

// ============================================================================
// BoundPropertyBool implementation
// ============================================================================

BoundPropertyBool::BoundPropertyBool()
    : m_fValue(false)
{
}

BoundPropertyBool::~BoundPropertyBool()
{
}

HRESULT BoundPropertyBool::Resolve(const BoundPropertyDictionary& dictionary)
{
    if (m_strPropertyName.IsEmpty())
        return E_INVALIDARG;

    bool fValue = false;
    HRESULT hr = dictionary.GetBool(m_strPropertyName, &fValue);
    if (SUCCEEDED(hr))
    {
        m_fValue = fValue;
        m_fBound = true;
    }
    return hr;
}

HRESULT BoundPropertyBool::Serialize(ModBeginElement* pElement) const
{
    if (!pElement)
        return E_POINTER;

    return pElement->SetAttribute(m_strPropertyName, m_fValue ? L"1" : L"0");
}

bool BoundPropertyBool::GetBoolValue() const throw()
{
    return m_fValue;
}

void BoundPropertyBool::SetBoolValue(bool fValue) throw()
{
    m_fValue = fValue;
}

// ============================================================================
// BoundPropertyFloat implementation
// ============================================================================

BoundPropertyFloat::BoundPropertyFloat()
    : m_flValue(0.0f)
{
}

BoundPropertyFloat::~BoundPropertyFloat()
{
}

HRESULT BoundPropertyFloat::Resolve(const BoundPropertyDictionary& dictionary)
{
    if (m_strPropertyName.IsEmpty())
        return E_INVALIDARG;

    double dblValue = 0.0;
    HRESULT hr = dictionary.GetDouble(m_strPropertyName, &dblValue);
    if (SUCCEEDED(hr))
    {
        m_flValue = static_cast<float>(dblValue);
        m_fBound = true;
    }
    return hr;
}

HRESULT BoundPropertyFloat::Serialize(ModBeginElement* pElement) const
{
    if (!pElement)
        return E_POINTER;

    WCHAR szBuf[64];
    swprintf_s(szBuf, L"%g", m_flValue);
    return pElement->SetAttribute(m_strPropertyName, szBuf);
}

float BoundPropertyFloat::GetFloatValue() const throw()
{
    return m_flValue;
}

void BoundPropertyFloat::SetFloatValue(float flValue) throw()
{
    m_flValue = flValue;
}

// ============================================================================
// BoundPropertyDouble implementation
// ============================================================================

BoundPropertyDouble::BoundPropertyDouble()
    : m_dblValue(0.0)
{
}

BoundPropertyDouble::~BoundPropertyDouble()
{
}

HRESULT BoundPropertyDouble::Resolve(const BoundPropertyDictionary& dictionary)
{
    if (m_strPropertyName.IsEmpty())
        return E_INVALIDARG;

    double dblValue = 0.0;
    HRESULT hr = dictionary.GetDouble(m_strPropertyName, &dblValue);
    if (SUCCEEDED(hr))
    {
        m_dblValue = dblValue;
        m_fBound = true;
    }
    return hr;
}

HRESULT BoundPropertyDouble::Serialize(ModBeginElement* pElement) const
{
    if (!pElement)
        return E_POINTER;

    WCHAR szBuf[64];
    swprintf_s(szBuf, L"%g", m_dblValue);
    return pElement->SetAttribute(m_strPropertyName, szBuf);
}

double BoundPropertyDouble::GetDoubleValue() const throw()
{
    return m_dblValue;
}

void BoundPropertyDouble::SetDoubleValue(double dblValue) throw()
{
    m_dblValue = dblValue;
}

// ============================================================================
// BoundPropertyGuid implementation
// ============================================================================

BoundPropertyGuid::BoundPropertyGuid()
{
    ZeroMemory(&m_guid, sizeof(GUID));
}

BoundPropertyGuid::~BoundPropertyGuid()
{
}

HRESULT BoundPropertyGuid::Resolve(const BoundPropertyDictionary& dictionary)
{
    if (m_strPropertyName.IsEmpty())
        return E_INVALIDARG;

    ValueVariantRef var;
    HRESULT hr = dictionary.GetProperty(m_strPropertyName, var);
    if (SUCCEEDED(hr) && var.GetType() == ValueVariantRef::VariantTypeString)
    {
        ATL::CString strVal = var.AsString();
        hr = CLSIDFromString(strVal, &m_guid);
        if (SUCCEEDED(hr))
            m_fBound = true;
    }
    return hr;
}

HRESULT BoundPropertyGuid::Serialize(ModBeginElement* pElement) const
{
    if (!pElement)
        return E_POINTER;

    WCHAR szBuf[64];
    if (StringFromGUID2(m_guid, szBuf, 64) == 0)
        return E_FAIL;

    return pElement->SetAttribute(m_strPropertyName, szBuf);
}

GUID BoundPropertyGuid::GetGuidValue() const throw()
{
    return m_guid;
}

void BoundPropertyGuid::SetGuidValue(const GUID& guid) throw()
{
    m_guid = guid;
}

// ============================================================================
// BoundPropertyTime implementation
// ============================================================================

BoundPropertyTime::BoundPropertyTime()
    : m_llTime(0)
{
}

BoundPropertyTime::~BoundPropertyTime()
{
}

HRESULT BoundPropertyTime::Resolve(const BoundPropertyDictionary& dictionary)
{
    if (m_strPropertyName.IsEmpty())
        return E_INVALIDARG;

    double dblValue = 0.0;
    HRESULT hr = dictionary.GetDouble(m_strPropertyName, &dblValue);
    if (SUCCEEDED(hr))
    {
        m_llTime = static_cast<LONGLONG>(dblValue);
        m_fBound = true;
    }
    return hr;
}

HRESULT BoundPropertyTime::Serialize(ModBeginElement* pElement) const
{
    if (!pElement)
        return E_POINTER;

    WCHAR szBuf[32];
    _i64tow_s(m_llTime, szBuf, _countof(szBuf), 10);
    return pElement->SetAttribute(m_strPropertyName, szBuf);
}

LONGLONG BoundPropertyTime::GetTimeValue() const throw()
{
    return m_llTime;
}

void BoundPropertyTime::SetTimeValue(LONGLONG llTime) throw()
{
    m_llTime = llTime;
}

// ============================================================================
// BoundPropertyBlob implementation
// ============================================================================

BoundPropertyBlob::BoundPropertyBlob()
{
}

BoundPropertyBlob::~BoundPropertyBlob()
{
}

HRESULT BoundPropertyBlob::Resolve(const BoundPropertyDictionary& dictionary)
{
    if (m_strPropertyName.IsEmpty())
        return E_INVALIDARG;

    ATL::CString strVal;
    HRESULT hr = dictionary.GetString(m_strPropertyName, strVal);
    if (SUCCEEDED(hr) && !strVal.IsEmpty())
    {
        hr = SerializationTextEncoder::DecodeBase64(strVal, m_arrData);
        if (SUCCEEDED(hr))
            m_fBound = true;
    }
    return hr;
}

HRESULT BoundPropertyBlob::Serialize(ModBeginElement* pElement) const
{
    if (!pElement)
        return E_POINTER;

    if (m_arrData.GetCount() == 0)
        return pElement->SetAttribute(m_strPropertyName, L"");

    ATL::CString strValue;
    HRESULT hr = SerializationTextEncoder::EncodeBase64(
        m_arrData.GetData(), static_cast<DWORD>(m_arrData.GetCount()), strValue);
    if (SUCCEEDED(hr))
    {
        hr = pElement->SetAttribute(m_strPropertyName, strValue);
    }
    return hr;
}

const BYTE* BoundPropertyBlob::GetBlobData() const throw()
{
    if (m_arrData.GetCount() == 0)
        return nullptr;
    return &m_arrData.GetAt(0);
}

DWORD BoundPropertyBlob::GetBlobSize() const throw()
{
    return static_cast<DWORD>(m_arrData.GetCount());
}

// ============================================================================
// BoundPropertyRef implementation
// ============================================================================

BoundPropertyRef::BoundPropertyRef()
    : m_dwRefExtentId(0)
{
}

BoundPropertyRef::~BoundPropertyRef()
{
}

HRESULT BoundPropertyRef::Resolve(const BoundPropertyDictionary& dictionary)
{
    if (m_strPropertyName.IsEmpty())
        return E_INVALIDARG;

    int nValue = 0;
    HRESULT hr = dictionary.GetInt(m_strPropertyName, &nValue);
    if (SUCCEEDED(hr))
    {
        m_dwRefExtentId = static_cast<DWORD>(nValue);
        m_fBound = true;
    }
    return hr;
}

HRESULT BoundPropertyRef::Serialize(ModBeginElement* pElement) const
{
    if (!pElement)
        return E_POINTER;

    WCHAR szBuf[32];
    _ultow_s(m_dwRefExtentId, szBuf, 10);
    return pElement->SetAttribute(m_strPropertyName, szBuf);
}

DWORD BoundPropertyRef::GetRefExtentId() const throw()
{
    return m_dwRefExtentId;
}

void BoundPropertyRef::SetRefExtentId(DWORD dwExtentId) throw()
{
    m_dwRefExtentId = dwExtentId;
}

// ============================================================================
// Serializable implementation
// ============================================================================

Serializable::Serializable()
    : m_dwVersion(1)
{
}

Serializable::~Serializable()
{
}

HRESULT Serializable::Serialize(SerializationWriter* pWriter) const
{
    if (!pWriter)
        return E_POINTER;

    pWriter->WriteAttribute(L"version", static_cast<int>(m_dwVersion));
    return S_OK;
}

HRESULT Serializable::Deserialize(SerializationReader* pReader, SerializationContext& ctx)
{
    UNREFERENCED_PARAMETER(ctx);
    if (!pReader)
        return E_POINTER;

    return S_OK;
}

DWORD Serializable::GetSerializableVersion() const throw()
{
    return m_dwVersion;
}

ATL::CString Serializable::GetSerializableName() const
{
    return L"Serializable";
}

// ============================================================================
// SerializableBase implementation
// ============================================================================

SerializableBase::SerializableBase()
{
}

SerializableBase::~SerializableBase()
{
}

HRESULT SerializableBase::Serialize(SerializationWriter* pWriter) const
{
    if (!pWriter)
        return E_POINTER;

    pWriter->WriteAttribute(L"version", static_cast<int>(m_dwVersion));
    return S_OK;
}

HRESULT SerializableBase::Deserialize(SerializationReader* pReader, SerializationContext& ctx)
{
    UNREFERENCED_PARAMETER(ctx);
    if (!pReader)
        return E_POINTER;

    return S_OK;
}

DWORD SerializableBase::GetSerializableVersion() const throw()
{
    return m_dwVersion;
}

ATL::CString SerializableBase::GetSerializableName() const
{
    return L"SerializableBase";
}

void SerializableBase::SetVersion(DWORD dwVersion) throw()
{
    m_dwVersion = dwVersion;
}

// ============================================================================
// SerializablePartial implementation
// ============================================================================

SerializablePartial::SerializablePartial()
    : m_dwFieldMask(0)
{
}

SerializablePartial::~SerializablePartial()
{
}

HRESULT SerializablePartial::Serialize(SerializationWriter* pWriter) const
{
    if (!pWriter)
        return E_POINTER;

    pWriter->WriteAttribute(L"fieldMask", static_cast<int>(m_dwFieldMask));
    return S_OK;
}

HRESULT SerializablePartial::Deserialize(SerializationReader* pReader, SerializationContext& ctx)
{
    UNREFERENCED_PARAMETER(pReader);
    UNREFERENCED_PARAMETER(ctx);
    return S_OK;
}

bool SerializablePartial::HasField(DWORD dwFieldId) const throw()
{
    return (m_dwFieldMask & (1 << dwFieldId)) != 0;
}

void SerializablePartial::SetFieldPresent(DWORD dwFieldId, bool fPresent) throw()
{
    if (fPresent)
        m_dwFieldMask |= (1 << dwFieldId);
    else
        m_dwFieldMask &= ~(1 << dwFieldId);
}

// ============================================================================
// SerializablePartialImpl implementation
// ============================================================================

SerializablePartialImpl::SerializablePartialImpl()
{
}

SerializablePartialImpl::~SerializablePartialImpl()
{
}

HRESULT SerializablePartialImpl::Serialize(SerializationWriter* pWriter) const
{
    if (!pWriter)
        return E_POINTER;

    HRESULT hr = SerializablePartial::Serialize(pWriter);
    if (FAILED(hr))
        return hr;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        const PartialEntry& entry = m_arrEntries.GetAt(i);
        WCHAR szFieldName[64];
        swprintf_s(szFieldName, L"field_%u", entry.dwFieldId);

        switch (entry.value.GetType())
        {
        case ValueVariantRef::VariantTypeString:
        {
            hr = pWriter->WriteAttribute(szFieldName, entry.value.AsString());
            break;
        }
        case ValueVariantRef::VariantTypeInt:
        {
            hr = pWriter->WriteAttribute(szFieldName, entry.value.AsInt());
            break;
        }
        case ValueVariantRef::VariantTypeDouble:
        {
            hr = pWriter->WriteAttribute(szFieldName, entry.value.AsDouble());
            break;
        }
        case ValueVariantRef::VariantTypeBool:
        {
            hr = pWriter->WriteAttribute(szFieldName, entry.value.AsBool());
            break;
        }
        default:
            hr = S_OK;
            break;
        }

        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

HRESULT SerializablePartialImpl::Deserialize(SerializationReader* pReader, SerializationContext& ctx)
{
    UNREFERENCED_PARAMETER(pReader);
    UNREFERENCED_PARAMETER(ctx);
    return S_OK;
}

void SerializablePartialImpl::SetPartialData(DWORD dwFieldId, const ValueVariantRef& value)
{
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).dwFieldId == dwFieldId)
        {
            PartialEntry& entry = m_arrEntries.GetAt(i);
            entry.value = value;
            return;
        }
    }

    PartialEntry entry;
    entry.dwFieldId = dwFieldId;
    entry.value = value;
    m_arrEntries.Add(entry);
}

bool SerializablePartialImpl::GetPartialData(DWORD dwFieldId, ValueVariantRef& value) const
{
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).dwFieldId == dwFieldId)
        {
            value = m_arrEntries.GetAt(i).value;
            return true;
        }
    }
    return false;
}

// ============================================================================
// SerializableContext implementation
// ============================================================================

SerializableContext::SerializableContext()
    : m_pReader(nullptr)
    , m_pWriter(nullptr)
{
}

SerializableContext::~SerializableContext()
{
}

SerializationContext& SerializableContext::GetContext()
{
    return m_ctx;
}

const SerializationContext& SerializableContext::GetContext() const
{
    return m_ctx;
}

void SerializableContext::SetReader(SerializationReader* pReader)
{
    m_pReader = pReader;
}

SerializationReader* SerializableContext::GetReader() const
{
    return m_pReader;
}

void SerializableContext::SetWriter(SerializationWriter* pWriter)
{
    m_pWriter = pWriter;
}

SerializationWriter* SerializableContext::GetWriter() const
{
    return m_pWriter;
}

// ============================================================================
// SerializableFactory implementation
// ============================================================================

SerializableFactory::SerializableFactory()
{
}

SerializableFactory::~SerializableFactory()
{
}

Serializable* SerializableFactory::CreateInstance(LPCWSTR pszTypeName)
{
    UNREFERENCED_PARAMETER(pszTypeName);
    return nullptr;
}

HRESULT SerializableFactory::RegisterType(LPCWSTR pszTypeName, DWORD dwVersion)
{
    if (!pszTypeName)
        return E_INVALIDARG;

    TypeEntry entry;
    entry.strTypeName = pszTypeName;
    entry.dwVersion = dwVersion;
    m_arrTypes.Add(entry);
    return S_OK;
}

bool SerializableFactory::IsTypeRegistered(LPCWSTR pszTypeName) const
{
    if (!pszTypeName)
        return false;

    for (size_t i = 0; i < m_arrTypes.GetCount(); ++i)
    {
        if (m_arrTypes.GetAt(i).strTypeName.CompareNoCase(pszTypeName) == 0)
            return true;
    }
    return false;
}

// ============================================================================
// SerializableRegistry implementation
// ============================================================================

SerializableRegistry::SerializableRegistry()
{
}

SerializableRegistry::~SerializableRegistry()
{
}

SerializableRegistry& SerializableRegistry::GetInstance()
{
    static SerializableRegistry s_instance;
    return s_instance;
}

HRESULT SerializableRegistry::RegisterFactory(LPCWSTR pszNamespace, SerializableFactory* pFactory)
{
    if (!pszNamespace || !pFactory)
        return E_INVALIDARG;

    RegistryEntry entry;
    entry.strNamespace = pszNamespace;
    entry.pFactory = pFactory;
    m_arrEntries.Add(entry);
    return S_OK;
}

SerializableFactory* SerializableRegistry::FindFactory(LPCWSTR pszNamespace) const
{
    if (!pszNamespace)
        return nullptr;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strNamespace.CompareNoCase(pszNamespace) == 0)
            return m_arrEntries.GetAt(i).pFactory;
    }
    return nullptr;
}

void SerializableRegistry::UnregisterAll()
{
    m_arrEntries.RemoveAll();
}

// ============================================================================
// SerializableFactoryMap implementation
// ============================================================================

SerializableFactoryMap::SerializableFactoryMap()
{
}

SerializableFactoryMap::~SerializableFactoryMap()
{
}

HRESULT SerializableFactoryMap::AddMapping(LPCWSTR pszTypeName, SerializableFactory* pFactory)
{
    if (!pszTypeName || !pFactory)
        return E_INVALIDARG;

    MappingEntry entry;
    entry.strTypeName = pszTypeName;
    entry.pFactory = pFactory;
    m_arrMappings.Add(entry);
    return S_OK;
}

SerializableFactory* SerializableFactoryMap::FindMapping(LPCWSTR pszTypeName) const
{
    if (!pszTypeName)
        return nullptr;

    for (size_t i = 0; i < m_arrMappings.GetCount(); ++i)
    {
        if (m_arrMappings.GetAt(i).strTypeName.CompareNoCase(pszTypeName) == 0)
            return m_arrMappings.GetAt(i).pFactory;
    }
    return nullptr;
}

size_t SerializableFactoryMap::GetCount() const throw()
{
    return m_arrMappings.GetCount();
}

// ============================================================================
// SerializableVersion implementation
// ============================================================================

SerializableVersion::SerializableVersion()
    : m_dwMajor(1)
    , m_dwMinor(0)
{
}

SerializableVersion::SerializableVersion(DWORD dwMajor, DWORD dwMinor)
    : m_dwMajor(dwMajor)
    , m_dwMinor(dwMinor)
{
}

SerializableVersion::~SerializableVersion()
{
}

DWORD SerializableVersion::GetMajor() const throw() { return m_dwMajor; }
DWORD SerializableVersion::GetMinor() const throw() { return m_dwMinor; }
void SerializableVersion::SetMajor(DWORD dwMajor) throw() { m_dwMajor = dwMajor; }
void SerializableVersion::SetMinor(DWORD dwMinor) throw() { m_dwMinor = dwMinor; }

bool SerializableVersion::operator>=(const SerializableVersion& other) const
{
    return (m_dwMajor > other.m_dwMajor) ||
           (m_dwMajor == other.m_dwMajor && m_dwMinor >= other.m_dwMinor);
}

bool SerializableVersion::operator<(const SerializableVersion& other) const
{
    return !(*this >= other);
}

// ============================================================================
// SerializableFlags implementation
// ============================================================================

SerializableFlags::SerializableFlags()
    : m_dwFlags(0)
{
}

SerializableFlags::~SerializableFlags()
{
}

DWORD SerializableFlags::GetFlags() const throw() { return m_dwFlags; }
void SerializableFlags::SetFlags(DWORD dwFlags) throw() { m_dwFlags = dwFlags; }

void SerializableFlags::AddFlag(DWORD dwFlag) throw()
{
    m_dwFlags |= dwFlag;
}

void SerializableFlags::RemoveFlag(DWORD dwFlag) throw()
{
    m_dwFlags &= ~dwFlag;
}

bool SerializableFlags::HasFlag(DWORD dwFlag) const throw()
{
    return (m_dwFlags & dwFlag) != 0;
}

// ============================================================================
// MediaItemSerializer implementation
// ============================================================================

MediaItemSerializer::MediaItemSerializer()
{
}

MediaItemSerializer::~MediaItemSerializer()
{
}

HRESULT MediaItemSerializer::SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter)
{
    if (!pItem || !pWriter)
        return E_INVALIDARG;

    pWriter->WriteAttribute(L"id", static_cast<int>(pItem->GetItemId()));
    pWriter->WriteAttribute(L"source", pItem->GetSourcePath());
    pWriter->WriteAttribute(L"displayName", pItem->GetDisplayName());
    pWriter->WriteAttribute(L"duration", pItem->GetDurationHns());
    return S_OK;
}

HRESULT MediaItemSerializer::DeserializeItem(MediaItemBase* pItem, SerializationReader* pReader,
                                              SerializationContext& ctx)
{
    UNREFERENCED_PARAMETER(ctx);
    if (!pItem || !pReader)
        return E_INVALIDARG;

    return S_OK;
}

LPCWSTR MediaItemSerializer::GetElementName() const
{
    return L"mediaItem";
}

// ============================================================================
// MediaItemVideoSerializer implementation
// ============================================================================

MediaItemVideoSerializer::MediaItemVideoSerializer()
{
}

MediaItemVideoSerializer::~MediaItemVideoSerializer()
{
}

HRESULT MediaItemVideoSerializer::SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter)
{
    if (!pItem || !pWriter)
        return E_INVALIDARG;

    return MediaItemSerializer::SerializeItem(pItem, pWriter);
}

HRESULT MediaItemVideoSerializer::DeserializeItem(MediaItemBase* pItem,
                                                   SerializationReader* pReader,
                                                   SerializationContext& ctx)
{
    if (!pItem || !pReader)
        return E_INVALIDARG;

    return MediaItemSerializer::DeserializeItem(pItem, pReader, ctx);
}

LPCWSTR MediaItemVideoSerializer::GetElementName() const
{
    return L"videoClip";
}

// ============================================================================
// MediaItemAudioSerializer implementation
// ============================================================================

MediaItemAudioSerializer::MediaItemAudioSerializer()
{
}

MediaItemAudioSerializer::~MediaItemAudioSerializer()
{
}

HRESULT MediaItemAudioSerializer::SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter)
{
    if (!pItem || !pWriter)
        return E_INVALIDARG;

    HRESULT hr = MediaItemSerializer::SerializeItem(pItem, pWriter);
    if (FAILED(hr)) return hr;

    AudioClip* pAudio = static_cast<AudioClip*>(pItem);

    hr = pWriter->WriteAttribute(L"audioFadeIn", pAudio->GetAudioFadeInHns());
    if (FAILED(hr)) return hr;

    hr = pWriter->WriteAttribute(L"audioFadeOut", pAudio->GetAudioFadeOutHns());
    if (FAILED(hr)) return hr;

    hr = pWriter->WriteAttribute(L"normalize", pAudio->IsNormalizeEnabled());
    return hr;
}

HRESULT MediaItemAudioSerializer::DeserializeItem(MediaItemBase* pItem,
                                                   SerializationReader* pReader,
                                                   SerializationContext& ctx)
{
    if (!pItem || !pReader)
        return E_INVALIDARG;

    HRESULT hr = MediaItemSerializer::DeserializeItem(pItem, pReader, ctx);
    if (FAILED(hr)) return hr;

    AudioClip* pAudio = static_cast<AudioClip*>(pItem);
    UNREFERENCED_PARAMETER(pAudio);

    return S_OK;
}

LPCWSTR MediaItemAudioSerializer::GetElementName() const
{
    return L"audioClip";
}

// ============================================================================
// MediaItemPhotoSerializer implementation
// ============================================================================

MediaItemPhotoSerializer::MediaItemPhotoSerializer()
{
}

MediaItemPhotoSerializer::~MediaItemPhotoSerializer()
{
}

HRESULT MediaItemPhotoSerializer::SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter)
{
    if (!pItem || !pWriter)
        return E_INVALIDARG;

    return MediaItemSerializer::SerializeItem(pItem, pWriter);
}

HRESULT MediaItemPhotoSerializer::DeserializeItem(MediaItemBase* pItem,
                                                   SerializationReader* pReader,
                                                   SerializationContext& ctx)
{
    if (!pItem || !pReader)
        return E_INVALIDARG;

    return MediaItemSerializer::DeserializeItem(pItem, pReader, ctx);
}

LPCWSTR MediaItemPhotoSerializer::GetElementName() const
{
    return L"photoClip";
}

// ============================================================================
// MediaItemTransitionSerializer implementation
// ============================================================================

MediaItemTransitionSerializer::MediaItemTransitionSerializer()
{
}

MediaItemTransitionSerializer::~MediaItemTransitionSerializer()
{
}

HRESULT MediaItemTransitionSerializer::SerializeItem(MediaItemBase* pItem,
                                                      SerializationWriter* pWriter)
{
    if (!pItem || !pWriter)
        return E_INVALIDARG;

    return MediaItemSerializer::SerializeItem(pItem, pWriter);
}

HRESULT MediaItemTransitionSerializer::DeserializeItem(MediaItemBase* pItem,
                                                        SerializationReader* pReader,
                                                        SerializationContext& ctx)
{
    if (!pItem || !pReader)
        return E_INVALIDARG;

    return MediaItemSerializer::DeserializeItem(pItem, pReader, ctx);
}

LPCWSTR MediaItemTransitionSerializer::GetElementName() const
{
    return L"transition";
}

// ============================================================================
// MediaItemEffectSerializer implementation
// ============================================================================

MediaItemEffectSerializer::MediaItemEffectSerializer()
{
}

MediaItemEffectSerializer::~MediaItemEffectSerializer()
{
}

HRESULT MediaItemEffectSerializer::SerializeItem(MediaItemBase* pItem,
                                                  SerializationWriter* pWriter)
{
    if (!pItem || !pWriter)
        return E_INVALIDARG;

    return MediaItemSerializer::SerializeItem(pItem, pWriter);
}

HRESULT MediaItemEffectSerializer::DeserializeItem(MediaItemBase* pItem,
                                                    SerializationReader* pReader,
                                                    SerializationContext& ctx)
{
    if (!pItem || !pReader)
        return E_INVALIDARG;

    return MediaItemSerializer::DeserializeItem(pItem, pReader, ctx);
}

LPCWSTR MediaItemEffectSerializer::GetElementName() const
{
    return L"effect";
}

// ============================================================================
// MediaItemTextSerializer implementation
// ============================================================================

MediaItemTextSerializer::MediaItemTextSerializer()
{
}

MediaItemTextSerializer::~MediaItemTextSerializer()
{
}

HRESULT MediaItemTextSerializer::SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter)
{
    if (!pItem || !pWriter)
        return E_INVALIDARG;

    return MediaItemSerializer::SerializeItem(pItem, pWriter);
}

HRESULT MediaItemTextSerializer::DeserializeItem(MediaItemBase* pItem,
                                                  SerializationReader* pReader,
                                                  SerializationContext& ctx)
{
    if (!pItem || !pReader)
        return E_INVALIDARG;

    return MediaItemSerializer::DeserializeItem(pItem, pReader, ctx);
}

LPCWSTR MediaItemTextSerializer::GetElementName() const
{
    return L"title";
}

// ============================================================================
// MediaItemGroupSerializer implementation
// ============================================================================

MediaItemGroupSerializer::MediaItemGroupSerializer()
{
}

MediaItemGroupSerializer::~MediaItemGroupSerializer()
{
}

HRESULT MediaItemGroupSerializer::SerializeItem(MediaItemBase* pItem, SerializationWriter* pWriter)
{
    if (!pItem || !pWriter)
        return E_INVALIDARG;

    return MediaItemSerializer::SerializeItem(pItem, pWriter);
}

HRESULT MediaItemGroupSerializer::DeserializeItem(MediaItemBase* pItem,
                                                   SerializationReader* pReader,
                                                   SerializationContext& ctx)
{
    if (!pItem || !pReader)
        return E_INVALIDARG;

    return MediaItemSerializer::DeserializeItem(pItem, pReader, ctx);
}

LPCWSTR MediaItemGroupSerializer::GetElementName() const
{
    return L"group";
}

// ============================================================================
// TemplateCategory implementation
// ============================================================================

TemplateCategory::TemplateCategory()
{
}

TemplateCategory::~TemplateCategory()
{
    RemoveAll();
}

ATL::CString TemplateCategory::GetName() const { return m_strName; }
void TemplateCategory::SetName(LPCWSTR pszName) { m_strName = pszName ? pszName : L""; }

size_t TemplateCategory::GetEntryCount() const throw()
{
    return m_arrEntries.GetCount();
}

TemplateEntry* TemplateCategory::GetEntryAt(size_t nIndex)
{
    if (nIndex >= m_arrEntries.GetCount())
        return nullptr;
    return m_arrEntries.GetAt(nIndex);
}

const TemplateEntry* TemplateCategory::GetEntryAt(size_t nIndex) const
{
    if (nIndex >= m_arrEntries.GetCount())
        return nullptr;
    return m_arrEntries.GetAt(nIndex);
}

void TemplateCategory::AddEntry(TemplateEntry* pEntry)
{
    if (pEntry)
        m_arrEntries.Add(pEntry);
}

void TemplateCategory::RemoveAll()
{
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        delete m_arrEntries.GetAt(i);
    }
    m_arrEntries.RemoveAll();
}

// ============================================================================
// TemplateRegistry implementation
// ============================================================================

TemplateRegistry::TemplateRegistry()
{
}

TemplateRegistry::~TemplateRegistry()
{
}

TemplateRegistry& TemplateRegistry::GetInstance()
{
    static TemplateRegistry s_instance;
    return s_instance;
}

HRESULT TemplateRegistry::RegisterTable(LPCWSTR pszThemeName, TemplateTable* pTable)
{
    if (!pszThemeName || !pTable)
        return E_INVALIDARG;

    RegistryEntry entry;
    entry.strThemeName = pszThemeName;
    entry.pTable = pTable;
    m_arrEntries.Add(entry);
    return S_OK;
}

TemplateTable* TemplateRegistry::FindTable(LPCWSTR pszThemeName) const
{
    if (!pszThemeName)
        return nullptr;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strThemeName.CompareNoCase(pszThemeName) == 0)
            return m_arrEntries.GetAt(i).pTable;
    }
    return nullptr;
}

void TemplateRegistry::UnregisterAll()
{
    m_arrEntries.RemoveAll();
}

// ============================================================================
// TemplateLoader implementation
// ============================================================================

TemplateLoader::TemplateLoader()
{
}

TemplateLoader::~TemplateLoader()
{
}

HRESULT TemplateLoader::LoadFromFile(LPCWSTR pszFilePath, TemplateTable* pTable)
{
    if (!pszFilePath || !pTable)
        return E_INVALIDARG;

    HANDLE hFile = ::CreateFileW(pszFilePath, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        m_strLastError.Format(L"Failed to open file: %s", pszFilePath);
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    IStream* pStream = nullptr;
    HRESULT hr = ::CreateStreamOnHGlobal(nullptr, TRUE, &pStream);
    if (SUCCEEDED(hr))
    {
        LARGE_INTEGER liZero = {};
        ULARGE_INTEGER uliSize = {};
        hr = ::GetFileSizeEx(hFile, reinterpret_cast<LARGE_INTEGER*>(&uliSize));
        if (SUCCEEDED(hr))
        {
            hr = ::IStream_Copy(::SHCreateMemStream(nullptr, 0), pStream, 0);
        }

        DWORD dwFileSize = ::GetFileSize(hFile, nullptr);
        if (dwFileSize != INVALID_FILE_SIZE && dwFileSize > 0)
        {
            HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwFileSize);
            if (hMem)
            {
                void* pMem = ::GlobalLock(hMem);
                DWORD dwRead = 0;
                ::ReadFile(hFile, pMem, dwFileSize, &dwRead, nullptr);
                ::GlobalUnlock(hMem);

                pStream->Release();
                hr = ::CreateStreamOnHGlobal(hMem, TRUE, &pStream);
                if (SUCCEEDED(hr))
                {
                    hr = LoadFromStream(pStream, pTable);
                    pStream->Release();
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    ::CloseHandle(hFile);
    return hr;
}

HRESULT TemplateLoader::LoadFromStream(IStream* pStream, TemplateTable* pTable)
{
    if (!pStream || !pTable)
        return E_INVALIDARG;

    IXmlReader* pReader = nullptr;
    HRESULT hr = ::CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), nullptr);
    if (FAILED(hr))
    {
        m_strLastError = L"Failed to create XmlReader";
        return hr;
    }

    hr = pReader->SetInput(pStream);
    if (SUCCEEDED(hr))
    {
        TemplateParser parser;
        XmlNodeType nodeType;
        while (pReader->Read(&nodeType) == S_OK)
        {
            if (nodeType == XmlNodeType_Element)
            {
                LPCWSTR pszLocalName = nullptr;
                pReader->GetLocalName(&pszLocalName, nullptr);
                if (pszLocalName && wcscmp(pszLocalName, L"templateTable") == 0)
                {
                    hr = parser.ParseElement(pReader, pTable);
                    if (FAILED(hr))
                        m_strLastError = parser.GetLastErrorMessage();
                    break;
                }
            }
        }
    }

    pReader->Release();
    return hr;
}

HRESULT TemplateLoader::LoadFromResource(HMODULE hModule, LPCWSTR pszResourceName,
                                          TemplateTable* pTable)
{
    if (!hModule || !pszResourceName || !pTable)
        return E_INVALIDARG;

    HRSRC hResInfo = ::FindResourceW(hModule, pszResourceName, L"XML");
    if (!hResInfo)
    {
        m_strLastError.Format(L"Resource not found: %s", pszResourceName);
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    HGLOBAL hResData = ::LoadResource(hModule, hResInfo);
    if (!hResData)
        return HRESULT_FROM_WIN32(::GetLastError());

    void* pData = ::LockResource(hResData);
    DWORD cbSize = ::SizeofResource(hModule, hResInfo);
    if (!pData || cbSize == 0)
        return E_FAIL;

    IStream* pStream = nullptr;
    HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, cbSize);
    if (!hMem)
        return E_OUTOFMEMORY;

    void* pMem = ::GlobalLock(hMem);
    CopyMemory(pMem, pData, cbSize);
    ::GlobalUnlock(hMem);

    HRESULT hr = ::CreateStreamOnHGlobal(hMem, TRUE, &pStream);
    if (SUCCEEDED(hr))
    {
        hr = LoadFromStream(pStream, pTable);
        pStream->Release();
    }

    return hr;
}

ATL::CString TemplateLoader::GetLastErrorMessage() const
{
    return m_strLastError;
}

// ============================================================================
// TemplateParser implementation
// ============================================================================

TemplateParser::TemplateParser()
{
}

TemplateParser::~TemplateParser()
{
}

HRESULT TemplateParser::ParseElement(IXmlReader* pReader, TemplateTable* pTable)
{
    if (!pReader || !pTable)
        return E_INVALIDARG;

    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_EndElement)
            break;

        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            pReader->GetLocalName(&pszLocalName, nullptr);
            if (!pszLocalName)
                continue;

            if (wcscmp(pszLocalName, L"entry") == 0)
            {
                TemplateEntry* pEntry = new TemplateEntry();

                if (pReader->MoveToFirstAttribute() == S_OK)
                {
                    do
                    {
                        LPCWSTR pszAttrName = nullptr;
                        LPCWSTR pszAttrValue = nullptr;
                        pReader->GetLocalName(&pszAttrName, nullptr);
                        pReader->GetValue(&pszAttrValue, nullptr);

                        if (pszAttrName && pszAttrValue)
                        {
                            if (wcscmp(pszAttrName, L"name") == 0)
                                pEntry->SetName(pszAttrValue);
                            else if (wcscmp(pszAttrName, L"displayName") == 0)
                                pEntry->SetDisplayName(pszAttrValue);
                        }
                    } while (pReader->MoveToNextAttribute() == S_OK);
                    pReader->MoveToElement();
                }

                pTable->AddTemplate(pEntry);
            }
        }
    }
    return S_OK;
}

HRESULT TemplateParser::ParseEntry(IXmlReader* pReader, TemplateCategory* pCategory)
{
    if (!pReader || !pCategory)
        return E_INVALIDARG;

    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_EndElement)
            break;

        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            pReader->GetLocalName(&pszLocalName, nullptr);
            if (!pszLocalName)
                continue;

            if (wcscmp(pszLocalName, L"entry") == 0)
            {
                TemplateEntry* pEntry = new TemplateEntry();

                if (pReader->MoveToFirstAttribute() == S_OK)
                {
                    do
                    {
                        LPCWSTR pszAttrName = nullptr;
                        LPCWSTR pszAttrValue = nullptr;
                        pReader->GetLocalName(&pszAttrName, nullptr);
                        pReader->GetValue(&pszAttrValue, nullptr);

                        if (pszAttrName && pszAttrValue)
                        {
                            if (wcscmp(pszAttrName, L"name") == 0)
                                pEntry->SetName(pszAttrValue);
                            else if (wcscmp(pszAttrName, L"displayName") == 0)
                                pEntry->SetDisplayName(pszAttrValue);
                        }
                    } while (pReader->MoveToNextAttribute() == S_OK);
                    pReader->MoveToElement();
                }

                pCategory->AddEntry(pEntry);
            }
        }
    }
    return S_OK;
}

ATL::CString TemplateParser::GetLastErrorMessage() const
{
    return m_strLastError;
}

// ============================================================================
// TemplateValidator implementation
// ============================================================================

TemplateValidator::TemplateValidator()
{
}

TemplateValidator::~TemplateValidator()
{
}

bool TemplateValidator::ValidateEntry(const TemplateEntry& entry) const
{
    if (entry.GetName().IsEmpty())
    {
        const_cast<TemplateValidator*>(this)->m_strErrors += L"Template entry has no name.\n";
        return false;
    }
    return true;
}

bool TemplateValidator::ValidateCategory(const TemplateCategory& category) const
{
    if (category.GetName().IsEmpty())
    {
        const_cast<TemplateValidator*>(this)->m_strErrors += L"Template category has no name.\n";
        return false;
    }
    if (category.GetEntryCount() == 0)
    {
        const_cast<TemplateValidator*>(this)->m_strErrors += L"Template category has no entries.\n";
        return false;
    }
    return true;
}

bool TemplateValidator::ValidateTable(const TemplateTable& table) const
{
    if (table.GetTemplateCount() == 0)
    {
        const_cast<TemplateValidator*>(this)->m_strErrors += L"Template table has no templates.\n";
        return false;
    }
    return true;
}

ATL::CString TemplateValidator::GetValidationErrors() const
{
    return m_strErrors;
}

// ============================================================================
// TemplateCache implementation
// ============================================================================

TemplateCache::TemplateCache()
{
}

TemplateCache::~TemplateCache()
{
    Clear();
}

HRESULT TemplateCache::CacheTemplate(LPCWSTR pszKey, TemplateTable* pTable)
{
    if (!pszKey || !pTable)
        return E_INVALIDARG;

    Evict(pszKey);

    CacheEntry entry;
    entry.strKey = pszKey;
    entry.pTable = pTable;
    m_arrEntries.Add(entry);
    return S_OK;
}

TemplateTable* TemplateCache::GetCachedTemplate(LPCWSTR pszKey) const
{
    if (!pszKey)
        return nullptr;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strKey.CompareNoCase(pszKey) == 0)
            return m_arrEntries.GetAt(i).pTable;
    }
    return nullptr;
}

bool TemplateCache::HasCachedTemplate(LPCWSTR pszKey) const
{
    return GetCachedTemplate(pszKey) != nullptr;
}

void TemplateCache::Evict(LPCWSTR pszKey)
{
    if (!pszKey)
        return;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strKey.CompareNoCase(pszKey) == 0)
        {
            m_arrEntries.RemoveAt(i);
            return;
        }
    }
}

void TemplateCache::Clear()
{
    m_arrEntries.RemoveAll();
}

size_t TemplateCache::GetCacheSize() const throw()
{
    return m_arrEntries.GetCount();
}

// ============================================================================
// ExtentBase implementation
// ============================================================================

ExtentBase::ExtentBase()
    : m_dwExtentId(0)
    , m_llStartHns(0)
    , m_llEndHns(0)
{
}

ExtentBase::~ExtentBase()
{
}

DWORD ExtentBase::GetExtentId() const throw() { return m_dwExtentId; }
void ExtentBase::SetExtentId(DWORD dwId) throw() { m_dwExtentId = dwId; }

LONGLONG ExtentBase::GetStartHns() const throw() { return m_llStartHns; }
LONGLONG ExtentBase::GetEndHns() const throw() { return m_llEndHns; }

LONGLONG ExtentBase::GetDurationHns() const throw()
{
    if (m_llEndHns <= m_llStartHns)
        return 0;
    return m_llEndHns - m_llStartHns;
}

bool ExtentBase::IsValid() const throw()
{
    return m_dwExtentId != 0 && m_llEndHns > m_llStartHns;
}

// ============================================================================
// ExtentDuration implementation
// ============================================================================

ExtentDuration::ExtentDuration()
    : m_llDurationHns(0)
{
}

ExtentDuration::~ExtentDuration()
{
}

LONGLONG ExtentDuration::GetDurationHns() const throw() { return m_llDurationHns; }
void ExtentDuration::SetDurationHns(LONGLONG llDuration) throw() { m_llDurationHns = llDuration; }

bool ExtentDuration::IsValid() const throw()
{
    return m_dwExtentId != 0 && m_llDurationHns > 0;
}

// ============================================================================
// ExtentTimeRange implementation
// ============================================================================

ExtentTimeRange::ExtentTimeRange()
{
}

ExtentTimeRange::~ExtentTimeRange()
{
}

LONGLONG ExtentTimeRange::GetStartHns() const throw() { return m_llStartHns; }
LONGLONG ExtentTimeRange::GetEndHns() const throw() { return m_llEndHns; }

LONGLONG ExtentTimeRange::GetDurationHns() const throw()
{
    if (m_llEndHns <= m_llStartHns)
        return 0;
    return m_llEndHns - m_llStartHns;
}

void ExtentTimeRange::SetRange(LONGLONG llStart, LONGLONG llEnd) throw()
{
    m_llStartHns = llStart;
    m_llEndHns = llEnd;
}

bool ExtentTimeRange::Contains(LONGLONG llTime) const throw()
{
    return llTime >= m_llStartHns && llTime < m_llEndHns;
}

bool ExtentTimeRange::IsValid() const throw()
{
    return m_dwExtentId != 0 && m_llEndHns > m_llStartHns;
}

// ============================================================================
// ExtentVideoRange implementation
// ============================================================================

ExtentVideoRange::ExtentVideoRange()
    : m_dwVideoStreamIndex(0)
    , m_fHasMotionEffect(false)
{
}

ExtentVideoRange::~ExtentVideoRange()
{
}

DWORD ExtentVideoRange::GetVideoStreamIndex() const throw() { return m_dwVideoStreamIndex; }
void ExtentVideoRange::SetVideoStreamIndex(DWORD dwIndex) throw() { m_dwVideoStreamIndex = dwIndex; }

bool ExtentVideoRange::HasMotionEffect() const throw() { return m_fHasMotionEffect; }
void ExtentVideoRange::SetHasMotionEffect(bool fHas) throw() { m_fHasMotionEffect = fHas; }

bool ExtentVideoRange::IsValid() const throw()
{
    return m_dwExtentId != 0;
}

// ============================================================================
// ExtentAudioRange implementation
// ============================================================================

ExtentAudioRange::ExtentAudioRange()
    : m_dwAudioStreamIndex(0)
    , m_dblVolume(1.0)
{
}

ExtentAudioRange::~ExtentAudioRange()
{
}

DWORD ExtentAudioRange::GetAudioStreamIndex() const throw() { return m_dwAudioStreamIndex; }
void ExtentAudioRange::SetAudioStreamIndex(DWORD dwIndex) throw() { m_dwAudioStreamIndex = dwIndex; }

double ExtentAudioRange::GetVolume() const throw() { return m_dblVolume; }
void ExtentAudioRange::SetVolume(double dblVolume) throw() { m_dblVolume = dblVolume; }

bool ExtentAudioRange::IsValid() const throw()
{
    return m_dwExtentId != 0;
}

// ============================================================================
// ExtentTextRange implementation
// ============================================================================

ExtentTextRange::ExtentTextRange()
    : m_dwAnimationId(0)
{
}

ExtentTextRange::~ExtentTextRange()
{
}

ATL::CString ExtentTextRange::GetText() const { return m_strText; }
void ExtentTextRange::SetText(LPCWSTR pszText) { m_strText = pszText ? pszText : L""; }

DWORD ExtentTextRange::GetAnimationId() const throw() { return m_dwAnimationId; }
void ExtentTextRange::SetAnimationId(DWORD dwAnimId) throw() { m_dwAnimationId = dwAnimId; }

bool ExtentTextRange::IsValid() const throw()
{
    return m_dwExtentId != 0;
}

// ============================================================================
// ExtentEffectRange implementation
// ============================================================================

ExtentEffectRange::ExtentEffectRange()
    : m_dwEffectId(0)
    , m_dblIntensity(1.0)
{
}

ExtentEffectRange::~ExtentEffectRange()
{
}

DWORD ExtentEffectRange::GetEffectId() const throw() { return m_dwEffectId; }
void ExtentEffectRange::SetEffectId(DWORD dwId) throw() { m_dwEffectId = dwId; }

double ExtentEffectRange::GetIntensity() const throw() { return m_dblIntensity; }
void ExtentEffectRange::SetIntensity(double dblIntensity) throw() { m_dblIntensity = dblIntensity; }

bool ExtentEffectRange::IsValid() const throw()
{
    return m_dwExtentId != 0 && m_dwEffectId != 0;
}

// ============================================================================
// ExtentTransitionRange implementation
// ============================================================================

ExtentTransitionRange::ExtentTransitionRange()
    : m_dwTransitionId(0)
    , m_dblOverlapDuration(0.0)
{
}

ExtentTransitionRange::~ExtentTransitionRange()
{
}

DWORD ExtentTransitionRange::GetTransitionId() const throw() { return m_dwTransitionId; }
void ExtentTransitionRange::SetTransitionId(DWORD dwId) throw() { m_dwTransitionId = dwId; }

double ExtentTransitionRange::GetOverlapDuration() const throw() { return m_dblOverlapDuration; }
void ExtentTransitionRange::SetOverlapDuration(double dblOverlap) throw() { m_dblOverlapDuration = dblOverlap; }

bool ExtentTransitionRange::IsValid() const throw()
{
    return m_dwExtentId != 0 && m_dwTransitionId != 0;
}

// ============================================================================
// ThemeBase implementation
// ============================================================================

ThemeBase::ThemeBase()
    : m_dwThemeId(0)
{
}

ThemeBase::~ThemeBase()
{
}

HRESULT ThemeBase::Apply(MovieProject* pProject)
{
    if (!pProject)
        return E_POINTER;

    if (!IsValid())
        return E_FAIL;

    return S_OK;
}

HRESULT ThemeBase::Remove(MovieProject* pProject)
{
    if (!pProject)
        return E_POINTER;

    return S_OK;
}

bool ThemeBase::IsValid() const throw()
{
    return m_dwThemeId != 0 && !m_strName.IsEmpty();
}

ATL::CString ThemeBase::GetName() const { return m_strName; }
void ThemeBase::SetName(LPCWSTR pszName) { m_strName = pszName ? pszName : L""; }

DWORD ThemeBase::GetThemeId() const throw() { return m_dwThemeId; }
void ThemeBase::SetThemeId(DWORD dwId) throw() { m_dwThemeId = dwId; }

// ============================================================================
// ThemeColor implementation
// ============================================================================

ThemeColor::ThemeColor()
    : m_dwPrimaryColor(0)
    , m_dwSecondaryColor(0)
    , m_dwAccentColor(0)
    , m_dwBackgroundColor(0)
{
}

ThemeColor::~ThemeColor()
{
}

HRESULT ThemeColor::Apply(MovieProject* pProject)
{
    HRESULT hr = ThemeBase::Apply(pProject);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT ThemeColor::Remove(MovieProject* pProject)
{
    return ThemeBase::Remove(pProject);
}

DWORD ThemeColor::GetPrimaryColor() const throw() { return m_dwPrimaryColor; }
void ThemeColor::SetPrimaryColor(DWORD dwColor) throw() { m_dwPrimaryColor = dwColor; }

DWORD ThemeColor::GetSecondaryColor() const throw() { return m_dwSecondaryColor; }
void ThemeColor::SetSecondaryColor(DWORD dwColor) throw() { m_dwSecondaryColor = dwColor; }

DWORD ThemeColor::GetAccentColor() const throw() { return m_dwAccentColor; }
void ThemeColor::SetAccentColor(DWORD dwColor) throw() { m_dwAccentColor = dwColor; }

DWORD ThemeColor::GetBackgroundColor() const throw() { return m_dwBackgroundColor; }
void ThemeColor::SetBackgroundColor(DWORD dwColor) throw() { m_dwBackgroundColor = dwColor; }

// ============================================================================
// ThemeFont implementation
// ============================================================================

ThemeFont::ThemeFont()
    : m_flTitleFontSize(24.0f)
    , m_flBodyFontSize(12.0f)
{
}

ThemeFont::~ThemeFont()
{
}

HRESULT ThemeFont::Apply(MovieProject* pProject)
{
    HRESULT hr = ThemeBase::Apply(pProject);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT ThemeFont::Remove(MovieProject* pProject)
{
    return ThemeBase::Remove(pProject);
}

ATL::CString ThemeFont::GetTitleFontFamily() const { return m_strTitleFont; }
void ThemeFont::SetTitleFontFamily(LPCWSTR pszFont) { m_strTitleFont = pszFont ? pszFont : L""; }

ATL::CString ThemeFont::GetBodyFontFamily() const { return m_strBodyFont; }
void ThemeFont::SetBodyFontFamily(LPCWSTR pszFont) { m_strBodyFont = pszFont ? pszFont : L""; }

float ThemeFont::GetTitleFontSize() const throw() { return m_flTitleFontSize; }
void ThemeFont::SetTitleFontSize(float flSize) throw() { m_flTitleFontSize = flSize; }

float ThemeFont::GetBodyFontSize() const throw() { return m_flBodyFontSize; }
void ThemeFont::SetBodyFontSize(float flSize) throw() { m_flBodyFontSize = flSize; }

// ============================================================================
// ThemeEffectProp implementation
// ============================================================================

ThemeEffectProp::ThemeEffectProp()
    : m_dwEffectId(0)
    , m_dblIntensity(1.0)
{
}

ThemeEffectProp::~ThemeEffectProp()
{
}

HRESULT ThemeEffectProp::Apply(MovieProject* pProject)
{
    HRESULT hr = ThemeBase::Apply(pProject);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT ThemeEffectProp::Remove(MovieProject* pProject)
{
    return ThemeBase::Remove(pProject);
}

DWORD ThemeEffectProp::GetEffectId() const throw() { return m_dwEffectId; }
void ThemeEffectProp::SetEffectId(DWORD dwId) throw() { m_dwEffectId = dwId; }

double ThemeEffectProp::GetIntensity() const throw() { return m_dblIntensity; }
void ThemeEffectProp::SetIntensity(double dblIntensity) throw() { m_dblIntensity = dblIntensity; }

// ============================================================================
// ThemeStyle implementation
// ============================================================================

ThemeStyle::ThemeStyle()
{
}

ThemeStyle::~ThemeStyle()
{
}

HRESULT ThemeStyle::Apply(MovieProject* pProject)
{
    HRESULT hr = ThemeBase::Apply(pProject);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT ThemeStyle::Remove(MovieProject* pProject)
{
    return ThemeBase::Remove(pProject);
}

ATL::CString ThemeStyle::GetStyleName() const { return m_strStyleName; }
void ThemeStyle::SetStyleName(LPCWSTR pszStyle) { m_strStyleName = pszStyle ? pszStyle : L""; }

BoundPropertyDictionary& ThemeStyle::GetProperties() { return m_properties; }
const BoundPropertyDictionary& ThemeStyle::GetProperties() const { return m_properties; }

// ============================================================================
// ThemeTemplate implementation
// ============================================================================

ThemeTemplate::ThemeTemplate()
    : m_pTemplateTable(nullptr)
{
}

ThemeTemplate::~ThemeTemplate()
{
}

HRESULT ThemeTemplate::Apply(MovieProject* pProject)
{
    HRESULT hr = ThemeBase::Apply(pProject);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT ThemeTemplate::Remove(MovieProject* pProject)
{
    return ThemeBase::Remove(pProject);
}

TemplateTable* ThemeTemplate::GetTemplateTable() { return m_pTemplateTable; }
void ThemeTemplate::SetTemplateTable(TemplateTable* pTable) { m_pTemplateTable = pTable; }

bool ThemeTemplate::HasTemplateTable() const throw()
{
    return m_pTemplateTable != nullptr;
}

// ============================================================================
// ThemeRegistry implementation
// ============================================================================

ThemeRegistry::ThemeRegistry()
{
}

ThemeRegistry::~ThemeRegistry()
{
}

ThemeRegistry& ThemeRegistry::GetInstance()
{
    static ThemeRegistry s_instance;
    return s_instance;
}

HRESULT ThemeRegistry::RegisterTheme(LPCWSTR pszName, ThemeBase* pTheme)
{
    if (!pszName || !pTheme)
        return E_INVALIDARG;

    RegistryEntry entry;
    entry.strName = pszName;
    entry.pTheme = pTheme;
    m_arrEntries.Add(entry);
    return S_OK;
}

ThemeBase* ThemeRegistry::FindTheme(LPCWSTR pszName) const
{
    if (!pszName)
        return nullptr;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strName.CompareNoCase(pszName) == 0)
            return m_arrEntries.GetAt(i).pTheme;
    }
    return nullptr;
}

size_t ThemeRegistry::GetThemeCount() const throw()
{
    return m_arrEntries.GetCount();
}

ThemeBase* ThemeRegistry::GetThemeAt(size_t nIndex)
{
    if (nIndex >= m_arrEntries.GetCount())
        return nullptr;
    return m_arrEntries.GetAt(nIndex).pTheme;
}

void ThemeRegistry::UnregisterAll()
{
    m_arrEntries.RemoveAll();
}

// ============================================================================
// PropertyBinding implementation
// ============================================================================

PropertyBinding::PropertyBinding()
    : m_fBound(false)
{
}

PropertyBinding::~PropertyBinding()
{
}

HRESULT PropertyBinding::Bind()
{
    m_fBound = true;
    return S_OK;
}

HRESULT PropertyBinding::Unbind()
{
    m_fBound = false;
    return S_OK;
}

bool PropertyBinding::IsBound() const throw()
{
    return m_fBound;
}

ATL::CString PropertyBinding::GetSourceProperty() const { return m_strSourceProperty; }
void PropertyBinding::SetSourceProperty(LPCWSTR pszProperty)
{
    m_strSourceProperty = pszProperty ? pszProperty : L"";
}

ATL::CString PropertyBinding::GetTargetProperty() const { return m_strTargetProperty; }
void PropertyBinding::SetTargetProperty(LPCWSTR pszProperty)
{
    m_strTargetProperty = pszProperty ? pszProperty : L"";
}

// ============================================================================
// PropertyBindingSource implementation
// ============================================================================

PropertyBindingSource::PropertyBindingSource()
    : m_dwSourceId(0)
{
}

PropertyBindingSource::~PropertyBindingSource()
{
}

HRESULT PropertyBindingSource::Bind()
{
    m_fBound = true;
    return S_OK;
}

HRESULT PropertyBindingSource::Unbind()
{
    m_fBound = false;
    return S_OK;
}

HRESULT PropertyBindingSource::SetPropertyValue(const ValueVariantRef& value)
{
    m_currentValue = value;
    return S_OK;
}

HRESULT PropertyBindingSource::GetPropertyValue(ValueVariantRef& value) const
{
    value = m_currentValue;
    return S_OK;
}

DWORD PropertyBindingSource::GetSourceId() const throw() { return m_dwSourceId; }
void PropertyBindingSource::SetSourceId(DWORD dwId) throw() { m_dwSourceId = dwId; }

// ============================================================================
// PropertyBindingTarget implementation
// ============================================================================

PropertyBindingTarget::PropertyBindingTarget()
    : m_dwTargetExtentId(0)
{
}

PropertyBindingTarget::~PropertyBindingTarget()
{
}

HRESULT PropertyBindingTarget::Bind()
{
    m_fBound = true;
    return S_OK;
}

HRESULT PropertyBindingTarget::Unbind()
{
    m_fBound = false;
    return S_OK;
}

HRESULT PropertyBindingTarget::ApplyValue(const ValueVariantRef& value)
{
    m_currentValue = value;
    return S_OK;
}

HRESULT PropertyBindingTarget::ReadValue(ValueVariantRef& value) const
{
    value = m_currentValue;
    return S_OK;
}

DWORD PropertyBindingTarget::GetTargetExtentId() const throw() { return m_dwTargetExtentId; }
void PropertyBindingTarget::SetTargetExtentId(DWORD dwId) throw() { m_dwTargetExtentId = dwId; }

// ============================================================================
// PropertyBindingConverter implementation
// ============================================================================

PropertyBindingConverter::PropertyBindingConverter()
{
}

PropertyBindingConverter::~PropertyBindingConverter()
{
}

HRESULT PropertyBindingConverter::Convert(const ValueVariantRef& source, ValueVariantRef& target)
{
    target = source;
    return S_OK;
}

bool PropertyBindingConverter::CanConvert(ValueVariantRef::VariantType sourceType, ValueVariantRef::VariantType targetType) const throw()
{
    UNREFERENCED_PARAMETER(sourceType);
    UNREFERENCED_PARAMETER(targetType);
    return true;
}

PropertyBindingConverter* PropertyBindingConverter::GetDefaultConverter()
{
    static PropertyBindingConverter s_default;
    return &s_default;
}

// ============================================================================
// PropertyBindingValidator implementation
// ============================================================================

PropertyBindingValidator::PropertyBindingValidator()
{
}

PropertyBindingValidator::~PropertyBindingValidator()
{
}

bool PropertyBindingValidator::Validate(const ValueVariantRef& value)
{
    UNREFERENCED_PARAMETER(value);
    return true;
}

HRESULT PropertyBindingValidator::GetLastValidationError(ATL::CString& strError)
{
    strError = m_strLastError;
    return S_OK;
}

PropertyBindingValidator* PropertyBindingValidator::GetDefaultValidator()
{
    static PropertyBindingValidator s_default;
    return &s_default;
}

// ============================================================================
// PropertyBindingManager implementation
// ============================================================================

PropertyBindingManager::PropertyBindingManager()
{
}

PropertyBindingManager::~PropertyBindingManager()
{
    RemoveAll();
}

HRESULT PropertyBindingManager::AddBinding(PropertyBinding* pBinding)
{
    if (!pBinding)
        return E_POINTER;

    m_arrBindings.Add(pBinding);
    return S_OK;
}

HRESULT PropertyBindingManager::RemoveBinding(LPCWSTR pszSourceProperty, LPCWSTR pszTargetProperty)
{
    if (!pszSourceProperty || !pszTargetProperty)
        return E_INVALIDARG;

    for (size_t i = 0; i < m_arrBindings.GetCount(); ++i)
    {
        PropertyBinding* pBinding = m_arrBindings.GetAt(i);
        if (pBinding->GetSourceProperty().CompareNoCase(pszSourceProperty) == 0 &&
            pBinding->GetTargetProperty().CompareNoCase(pszTargetProperty) == 0)
        {
            delete pBinding;
            m_arrBindings.RemoveAt(i);
            return S_OK;
        }
    }
    return S_FALSE;
}

HRESULT PropertyBindingManager::ResolveAll(const BoundPropertyDictionary& dictionary)
{
    for (size_t i = 0; i < m_arrBindings.GetCount(); ++i)
    {
        PropertyBinding* pBinding = m_arrBindings.GetAt(i);
        if (pBinding && !pBinding->IsBound())
        {
            pBinding->Bind();
        }
    }
    return S_OK;
}

HRESULT PropertyBindingManager::ApplyAll()
{
    for (size_t i = 0; i < m_arrBindings.GetCount(); ++i)
    {
        m_arrBindings.GetAt(i)->Bind();
    }
    return S_OK;
}

size_t PropertyBindingManager::GetBindingCount() const throw()
{
    return m_arrBindings.GetCount();
}

PropertyBinding* PropertyBindingManager::GetBindingAt(size_t nIndex)
{
    if (nIndex >= m_arrBindings.GetCount())
        return nullptr;
    return m_arrBindings.GetAt(nIndex);
}

const PropertyBinding* PropertyBindingManager::GetBindingAt(size_t nIndex) const
{
    if (nIndex >= m_arrBindings.GetCount())
        return nullptr;
    return m_arrBindings.GetAt(nIndex);
}

void PropertyBindingManager::RemoveAll()
{
    for (size_t i = 0; i < m_arrBindings.GetCount(); ++i)
    {
        delete m_arrBindings.GetAt(i);
    }
    m_arrBindings.RemoveAll();
}

// ============================================================================
// SerializationContextBase implementation
// ============================================================================

SerializationContextBase::SerializationContextBase()
    : m_hrLast(S_OK)
{
}

SerializationContextBase::~SerializationContextBase()
{
}

HRESULT SerializationContextBase::Initialize()
{
    m_ctx.Reset();
    m_hrLast = S_OK;
    return S_OK;
}

void SerializationContextBase::Shutdown()
{
    m_ctx.Reset();
}

SerializationContext& SerializationContextBase::GetContext()
{
    return m_ctx;
}

const SerializationContext& SerializationContextBase::GetContext() const
{
    return m_ctx;
}

HRESULT SerializationContextBase::GetLastResult() const throw()
{
    return m_hrLast;
}

// ============================================================================
// SerializationContextRead implementation
// ============================================================================

SerializationContextRead::SerializationContextRead()
    : m_pReader(nullptr)
{
}

SerializationContextRead::~SerializationContextRead()
{
}

HRESULT SerializationContextRead::Initialize()
{
    SerializationContextBase::Initialize();
    m_ctx.SetMode(SerializationModeRead);
    m_attributes.RemoveAll();
    return S_OK;
}

void SerializationContextRead::SetReader(SerializationReader* pReader)
{
    m_pReader = pReader;
}

SerializationReader* SerializationContextRead::GetReader() const
{
    return m_pReader;
}

HRESULT SerializationContextRead::BeginElement(LPCWSTR pszName)
{
    if (!pszName)
        return E_INVALIDARG;

    if (!m_pReader)
        return E_POINTER;

    m_ctx.IncrementDepth();
    m_attributes.RemoveAll();
    return S_OK;
}

HRESULT SerializationContextRead::EndElement(LPCWSTR pszName)
{
    if (!pszName)
        return E_INVALIDARG;

    if (!m_pReader)
        return E_POINTER;

    m_attributes.RemoveAll();
    m_ctx.DecrementDepth();
    return S_OK;
}

HRESULT SerializationContextRead::ReadAttribute(LPCWSTR pszName, ATL::CString& strValue)
{
    if (!pszName)
        return E_INVALIDARG;

    if (!m_pReader)
        return E_POINTER;

    return m_attributes.GetAttribute(pszName, strValue);
}

HRESULT SerializationContextRead::CacheAttribute(LPCWSTR pszName, LPCWSTR pszValue)
{
    if (!pszName)
        return E_INVALIDARG;

    return m_attributes.AddAttribute(pszName, pszValue);
}

void SerializationContextRead::ClearAttributes()
{
    m_attributes.RemoveAll();
}

// ============================================================================
// SerializationContextWrite implementation
// ============================================================================

SerializationContextWrite::SerializationContextWrite()
    : m_pWriter(nullptr)
{
}

SerializationContextWrite::~SerializationContextWrite()
{
}

HRESULT SerializationContextWrite::Initialize()
{
    SerializationContextBase::Initialize();
    m_ctx.SetMode(SerializationModeWrite);
    return S_OK;
}

void SerializationContextWrite::SetWriter(SerializationWriter* pWriter)
{
    m_pWriter = pWriter;
}

SerializationWriter* SerializationContextWrite::GetWriter() const
{
    return m_pWriter;
}

HRESULT SerializationContextWrite::BeginElement(LPCWSTR pszName)
{
    if (!pszName)
        return E_INVALIDARG;

    if (!m_pWriter)
        return E_POINTER;

    return m_pWriter->BeginElement(pszName);
}

HRESULT SerializationContextWrite::EndElement()
{
    if (!m_pWriter)
        return E_POINTER;

    return m_pWriter->EndElement();
}

HRESULT SerializationContextWrite::WriteAttribute(LPCWSTR pszName, LPCWSTR pszValue)
{
    if (!pszName)
        return E_INVALIDARG;

    if (!m_pWriter)
        return E_POINTER;

    return m_pWriter->WriteAttribute(pszName, pszValue);
}

// ============================================================================
// SerializationContextVersion implementation
// ============================================================================

SerializationContextVersion::SerializationContextVersion()
    : m_dwMajor(1)
    , m_dwMinor(0)
{
}

SerializationContextVersion::~SerializationContextVersion()
{
}

DWORD SerializationContextVersion::GetMajorVersion() const throw() { return m_dwMajor; }
DWORD SerializationContextVersion::GetMinorVersion() const throw() { return m_dwMinor; }

void SerializationContextVersion::SetVersion(DWORD dwMajor, DWORD dwMinor) throw()
{
    m_dwMajor = dwMajor;
    m_dwMinor = dwMinor;
}

bool SerializationContextVersion::IsVersionSupported(DWORD dwMinMajor, DWORD dwMinMinor,
                                                     DWORD dwMaxMajor, DWORD dwMaxMinor) const
{
    if (m_dwMajor < dwMinMajor)
        return false;
    if (m_dwMajor == dwMinMajor && m_dwMinor < dwMinMinor)
        return false;
    if (m_dwMajor > dwMaxMajor)
        return false;
    if (m_dwMajor == dwMaxMajor && m_dwMinor > dwMaxMinor)
        return false;
    return true;
}

SerializationContextVersion SerializationContextVersion::Current()
{
    SerializationContextVersion ver;
    ver.SetVersion(1, 1);
    return ver;
}

// ============================================================================
// SerializationContextRegistry implementation
// ============================================================================

SerializationContextRegistry::SerializationContextRegistry()
{
}

SerializationContextRegistry::~SerializationContextRegistry()
{
}

SerializationContextRegistry& SerializationContextRegistry::GetInstance()
{
    static SerializationContextRegistry s_instance;
    return s_instance;
}

HRESULT SerializationContextRegistry::RegisterContext(LPCWSTR pszName,
                                                      SerializationContextBase* pContext)
{
    if (!pszName || !pContext)
        return E_INVALIDARG;

    RegistryEntry entry;
    entry.strName = pszName;
    entry.pContext = pContext;
    m_arrEntries.Add(entry);
    return S_OK;
}

SerializationContextBase* SerializationContextRegistry::FindContext(LPCWSTR pszName) const
{
    if (!pszName)
        return nullptr;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strName.CompareNoCase(pszName) == 0)
            return m_arrEntries.GetAt(i).pContext;
    }
    return nullptr;
}

void SerializationContextRegistry::UnregisterAll()
{
    m_arrEntries.RemoveAll();
}

// ============================================================================
// SerializationElementMap implementation
// ============================================================================

SerializationElementMap::SerializationElementMap()
{
}

SerializationElementMap::~SerializationElementMap()
{
}

HRESULT SerializationElementMap::AddMapping(LPCWSTR pszElementName, DWORD dwHandlerId)
{
    if (!pszElementName)
        return E_INVALIDARG;

    MapEntry entry;
    entry.strElementName = pszElementName;
    entry.dwHandlerId = dwHandlerId;
    m_arrEntries.Add(entry);
    return S_OK;
}

int SerializationElementMap::FindMapping(LPCWSTR pszElementName) const
{
    if (!pszElementName)
        return -1;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strElementName.CompareNoCase(pszElementName) == 0)
            return static_cast<int>(i);
    }
    return -1;
}

size_t SerializationElementMap::GetCount() const throw()
{
    return m_arrEntries.GetCount();
}

ATL::CString SerializationElementMap::GetElementNameAt(size_t nIndex) const
{
    if (nIndex >= m_arrEntries.GetCount())
        return L"";
    return m_arrEntries.GetAt(nIndex).strElementName;
}

DWORD SerializationElementMap::GetHandlerIdAt(size_t nIndex) const
{
    if (nIndex >= m_arrEntries.GetCount())
        return 0;
    return m_arrEntries.GetAt(nIndex).dwHandlerId;
}

// ============================================================================
// SerializationAttributeList implementation
// ============================================================================

SerializationAttributeList::SerializationAttributeList()
{
}

SerializationAttributeList::~SerializationAttributeList()
{
}

HRESULT SerializationAttributeList::AddAttribute(LPCWSTR pszName, LPCWSTR pszValue)
{
    if (!pszName)
        return E_INVALIDARG;

    AttrEntry entry;
    entry.strName = pszName;
    entry.strValue = pszValue ? pszValue : L"";
    m_arrEntries.Add(entry);
    return S_OK;
}

HRESULT SerializationAttributeList::GetAttribute(LPCWSTR pszName, ATL::CString& strValue) const
{
    if (!pszName)
        return E_INVALIDARG;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strName.CompareNoCase(pszName) == 0)
        {
            strValue = m_arrEntries.GetAt(i).strValue;
            return S_OK;
        }
    }
    strValue.Empty();
    return DISP_E_UNKNOWNNAME;
}

bool SerializationAttributeList::HasAttribute(LPCWSTR pszName) const
{
    if (!pszName)
        return false;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strName.CompareNoCase(pszName) == 0)
            return true;
    }
    return false;
}

size_t SerializationAttributeList::GetCount() const throw()
{
    return m_arrEntries.GetCount();
}

LPCWSTR SerializationAttributeList::GetNameAt(size_t nIndex) const
{
    if (nIndex >= m_arrEntries.GetCount())
        return nullptr;
    return m_arrEntries.GetAt(nIndex).strName;
}

LPCWSTR SerializationAttributeList::GetValueAt(size_t nIndex) const
{
    if (nIndex >= m_arrEntries.GetCount())
        return nullptr;
    return m_arrEntries.GetAt(nIndex).strValue;
}

void SerializationAttributeList::RemoveAll()
{
    m_arrEntries.RemoveAll();
}

// ============================================================================
// SerializationElementState implementation
// ============================================================================

SerializationElementState::SerializationElementState()
    : m_dwDepth(0)
    , m_fComplete(false)
{
}

SerializationElementState::~SerializationElementState()
{
}

ATL::CString SerializationElementState::GetElementName() const { return m_strElementName; }
void SerializationElementState::SetElementName(LPCWSTR pszName)
{
    m_strElementName = pszName ? pszName : L"";
}

DWORD SerializationElementState::GetDepth() const throw() { return m_dwDepth; }
void SerializationElementState::SetDepth(DWORD dwDepth) throw() { m_dwDepth = dwDepth; }

bool SerializationElementState::IsComplete() const throw() { return m_fComplete; }
void SerializationElementState::SetComplete(bool fComplete) throw() { m_fComplete = fComplete; }

SerializationAttributeList& SerializationElementState::GetAttributes() { return m_attributes; }
const SerializationAttributeList& SerializationElementState::GetAttributes() const { return m_attributes; }

// ============================================================================
// SerializationErrorInfo implementation
// ============================================================================

SerializationErrorInfo::SerializationErrorInfo()
    : m_hr(S_OK)
    , m_dwLineNumber(0)
{
}

SerializationErrorInfo::~SerializationErrorInfo()
{
}

HRESULT SerializationErrorInfo::GetHResult() const throw() { return m_hr; }
void SerializationErrorInfo::SetHResult(HRESULT hr) throw() { m_hr = hr; }

ATL::CString SerializationErrorInfo::GetErrorMessage() const { return m_strMessage; }
void SerializationErrorInfo::SetErrorMessage(LPCWSTR pszMessage)
{
    m_strMessage = pszMessage ? pszMessage : L"";
}

ATL::CString SerializationErrorInfo::GetSourceElement() const { return m_strSourceElement; }
void SerializationErrorInfo::SetSourceElement(LPCWSTR pszElement)
{
    m_strSourceElement = pszElement ? pszElement : L"";
}

DWORD SerializationErrorInfo::GetLineNumber() const throw() { return m_dwLineNumber; }
void SerializationErrorInfo::SetLineNumber(DWORD dwLine) throw() { m_dwLineNumber = dwLine; }

bool SerializationErrorInfo::HasError() const throw()
{
    return FAILED(m_hr);
}

void SerializationErrorInfo::Clear()
{
    m_hr = S_OK;
    m_strMessage.Empty();
    m_strSourceElement.Empty();
    m_dwLineNumber = 0;
}

// ============================================================================
// SerializationBookmark implementation
// ============================================================================

SerializationBookmark::SerializationBookmark()
    : m_dwElementDepth(0)
    , m_dwAttributeIndex(0)
{
}

SerializationBookmark::~SerializationBookmark()
{
}

HRESULT SerializationBookmark::SavePosition(IXmlReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    UINT uDepth = 0;
    pReader->GetDepth(&uDepth);
    m_dwElementDepth = uDepth;
    m_dwAttributeIndex = 0;

    return S_OK;
}

HRESULT SerializationBookmark::RestorePosition(IXmlReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    if (!HasPosition())
        return S_OK;

    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_Element || nodeType == XmlNodeTypeEndElement)
        {
            UINT uDepth = 0;
            pReader->GetDepth(&uDepth);
            if (uDepth == m_dwElementDepth)
            {
                return S_OK;
            }
        }
    }

    return S_OK;
}

bool SerializationBookmark::HasPosition() const throw()
{
    return m_dwElementDepth != 0;
}

void SerializationBookmark::Clear()
{
    m_dwElementDepth = 0;
    m_dwAttributeIndex = 0;
}

// ============================================================================
// SerializationNamespaceManager implementation
// ============================================================================

SerializationNamespaceManager::SerializationNamespaceManager()
{
}

SerializationNamespaceManager::~SerializationNamespaceManager()
{
}

HRESULT SerializationNamespaceManager::RegisterNamespace(LPCWSTR pszPrefix, LPCWSTR pszUri)
{
    if (!pszPrefix || !pszUri)
        return E_INVALIDARG;

    NamespaceEntry entry;
    entry.strPrefix = pszPrefix;
    entry.strUri = pszUri;
    m_arrEntries.Add(entry);
    return S_OK;
}

HRESULT SerializationNamespaceManager::LookupNamespace(LPCWSTR pszPrefix,
                                                       ATL::CString& strUri) const
{
    if (!pszPrefix)
        return E_INVALIDARG;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strPrefix.CompareNoCase(pszPrefix) == 0)
        {
            strUri = m_arrEntries.GetAt(i).strUri;
            return S_OK;
        }
    }
    strUri.Empty();
    return DISP_E_UNKNOWNNAME;
}

HRESULT SerializationNamespaceManager::LookupPrefix(LPCWSTR pszUri,
                                                     ATL::CString& strPrefix) const
{
    if (!pszUri)
        return E_INVALIDARG;

    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).strUri.CompareNoCase(pszUri) == 0)
        {
            strPrefix = m_arrEntries.GetAt(i).strPrefix;
            return S_OK;
        }
    }
    strPrefix.Empty();
    return DISP_E_UNKNOWNNAME;
}

size_t SerializationNamespaceManager::GetCount() const throw()
{
    return m_arrEntries.GetCount();
}

void SerializationNamespaceManager::Clear()
{
    m_arrEntries.RemoveAll();
}

// ============================================================================
// SerializationElementStack implementation
// ============================================================================

SerializationElementStack::SerializationElementStack()
{
}

SerializationElementStack::~SerializationElementStack()
{
}

void SerializationElementStack::PushElement(LPCWSTR pszName, DWORD dwDepth)
{
    StackEntry entry;
    entry.strName = pszName ? pszName : L"";
    entry.dwDepth = dwDepth;
    m_arrEntries.Add(entry);
}

bool SerializationElementStack::PopElement(LPCWSTR pszName)
{
    if (m_arrEntries.GetCount() == 0)
        return false;

    size_t lastIndex = m_arrEntries.GetCount() - 1;
    if (!pszName || m_arrEntries.GetAt(lastIndex).strName.CompareNoCase(pszName) == 0)
    {
        m_arrEntries.RemoveAt(lastIndex);
        return true;
    }
    return false;
}

bool SerializationElementStack::PeekElement(ATL::CString& strName) const
{
    if (m_arrEntries.GetCount() == 0)
        return false;

    strName = m_arrEntries.GetAt(m_arrEntries.GetCount() - 1).strName;
    return true;
}

DWORD SerializationElementStack::GetDepth() const throw()
{
    return static_cast<DWORD>(m_arrEntries.GetCount());
}

bool SerializationElementStack::IsEmpty() const throw()
{
    return m_arrEntries.GetCount() == 0;
}

void SerializationElementStack::Clear()
{
    m_arrEntries.RemoveAll();
}

// ============================================================================
// SerializationBuffer implementation
// ============================================================================

SerializationBuffer::SerializationBuffer()
    : m_cbSize(0)
{
}

SerializationBuffer::~SerializationBuffer()
{
}

HRESULT SerializationBuffer::Reserve(DWORD cbSize)
{
    m_arrBuffer.RemoveAll();
    if (cbSize > 0)
    {
        m_arrBuffer.SetCount(cbSize);
    }
    m_cbSize = 0;
    return S_OK;
}

HRESULT SerializationBuffer::Append(const BYTE* pcbData, DWORD cbSize)
{
    if (!pcbData || cbSize == 0)
        return S_OK;

    size_t currentSize = m_arrBuffer.GetCount();
    size_t newSize = currentSize + cbSize;
    m_arrBuffer.SetCount(newSize);
    CopyMemory(&m_arrBuffer.GetAt(currentSize), pcbData, cbSize);
    m_cbSize += cbSize;
    return S_OK;
}

HRESULT SerializationBuffer::Clear()
{
    m_arrBuffer.RemoveAll();
    m_cbSize = 0;
    return S_OK;
}

const BYTE* SerializationBuffer::GetBuffer() const throw()
{
    if (m_arrBuffer.GetCount() == 0)
        return nullptr;
    return &m_arrBuffer.GetAt(0);
}

DWORD SerializationBuffer::GetSize() const throw()
{
    return m_cbSize;
}

HRESULT SerializationBuffer::WriteToFile(LPCWSTR pszFilePath) const
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    HANDLE hFile = ::CreateFileW(pszFilePath, GENERIC_WRITE, 0, nullptr,
                                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(::GetLastError());

    DWORD dwWritten = 0;
    BOOL fResult = ::WriteFile(hFile, m_arrBuffer.GetData(),
                                static_cast<DWORD>(m_arrBuffer.GetCount()), &dwWritten, nullptr);
    ::CloseHandle(hFile);

    return fResult ? S_OK : HRESULT_FROM_WIN32(::GetLastError());
}

HRESULT SerializationBuffer::ReadFromFile(LPCWSTR pszFilePath)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    HANDLE hFile = ::CreateFileW(pszFilePath, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(::GetLastError());

    DWORD dwFileSize = ::GetFileSize(hFile, nullptr);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        ::CloseHandle(hFile);
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    m_arrBuffer.RemoveAll();
    if (dwFileSize > 0)
    {
        m_arrBuffer.SetCount(dwFileSize);
        DWORD dwRead = 0;
        BOOL fResult = ::ReadFile(hFile, &m_arrBuffer.GetAt(0), dwFileSize, &dwRead, nullptr);
        ::CloseHandle(hFile);

        if (!fResult)
            return HRESULT_FROM_WIN32(::GetLastError());

        m_cbSize = dwRead;
    }
    else
    {
        ::CloseHandle(hFile);
        m_cbSize = 0;
    }

    return S_OK;
}

// ============================================================================
// SerializationTextEncoder implementation
// ============================================================================

SerializationTextEncoder::SerializationTextEncoder()
{
}

SerializationTextEncoder::~SerializationTextEncoder()
{
}

HRESULT SerializationTextEncoder::EncodeXmlString(LPCWSTR pszInput, ATL::CString& strOutput)
{
    if (!pszInput)
    {
        strOutput.Empty();
        return S_OK;
    }

    strOutput.Empty();
    LPCWSTR pch = pszInput;
    while (*pch)
    {
        switch (*pch)
        {
        case L'&':
            strOutput += L"&amp;";
            break;
        case L'<':
            strOutput += L"&lt;";
            break;
        case L'>':
            strOutput += L"&gt;";
            break;
        case L'"':
            strOutput += L"&quot;";
            break;
        case L'\'':
            strOutput += L"&apos;";
            break;
        default:
            strOutput += *pch;
            break;
        }
        ++pch;
    }
    return S_OK;
}

HRESULT SerializationTextEncoder::DecodeXmlString(LPCWSTR pszInput, ATL::CString& strOutput)
{
    if (!pszInput)
    {
        strOutput.Empty();
        return S_OK;
    }

    strOutput = pszInput;
    strOutput.Replace(L"&amp;", L"&");
    strOutput.Replace(L"&lt;", L"<");
    strOutput.Replace(L"&gt;", L">");
    strOutput.Replace(L"&quot;", L"\"");
    strOutput.Replace(L"&apos;", L"\'");
    return S_OK;
}

HRESULT SerializationTextEncoder::EncodeBase64(const BYTE* pcbData, DWORD cbData,
                                                ATL::CString& strOutput)
{
    if (!pcbData || cbData == 0)
    {
        strOutput.Empty();
        return S_OK;
    }

    static const WCHAR s_szBase64[] =
        L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    strOutput.Empty();
    DWORD cbFullBlocks = cbData / 3;
    DWORD cbRemainder = cbData % 3;

    for (DWORD i = 0; i < cbFullBlocks; ++i)
    {
        DWORD dwTriple = (static_cast<DWORD>(pcbData[i * 3]) << 16) |
                         (static_cast<DWORD>(pcbData[i * 3 + 1]) << 8) |
                         static_cast<DWORD>(pcbData[i * 3 + 2]);
        strOutput += s_szBase64[(dwTriple >> 18) & 0x3F];
        strOutput += s_szBase64[(dwTriple >> 12) & 0x3F];
        strOutput += s_szBase64[(dwTriple >> 6) & 0x3F];
        strOutput += s_szBase64[dwTriple & 0x3F];
    }

    if (cbRemainder == 1)
    {
        DWORD dwTriple = static_cast<DWORD>(pcbData[cbData - 1]) << 16;
        strOutput += s_szBase64[(dwTriple >> 18) & 0x3F];
        strOutput += s_szBase64[(dwTriple >> 12) & 0x3F];
        strOutput += L"==";
    }
    else if (cbRemainder == 2)
    {
        DWORD dwTriple = (static_cast<DWORD>(pcbData[cbData - 2]) << 16) |
                         (static_cast<DWORD>(pcbData[cbData - 1]) << 8);
        strOutput += s_szBase64[(dwTriple >> 18) & 0x3F];
        strOutput += s_szBase64[(dwTriple >> 12) & 0x3F];
        strOutput += s_szBase64[(dwTriple >> 6) & 0x3F];
        strOutput += L"=";
    }

    return S_OK;
}

HRESULT SerializationTextEncoder::DecodeBase64(LPCWSTR pszInput, ATL::CAtlArray<BYTE>& arrOutput)
{
    arrOutput.RemoveAll();

    if (!pszInput || !pszInput[0])
        return S_OK;

    static const int s_nDecodeMap[128] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
    };

    size_t nLen = wcslen(pszInput);
    if (nLen == 0)
        return S_OK;

    ATL::CAtlArray<BYTE> arrTemp;
    arrTemp.SetCount(nLen);

    size_t nOutput = 0;
    for (size_t i = 0; i < nLen; ++i)
    {
        WCHAR ch = pszInput[i];
        if (ch == L'=' || ch == L'\n' || ch == L'\r')
            continue;

        if (ch >= 128 || s_nDecodeMap[ch] < 0)
            continue;

        arrTemp.GetAt(nOutput++) = static_cast<BYTE>(s_nDecodeMap[ch]);
    }

    DWORD cbDecoded = (static_cast<DWORD>(nOutput) / 4) * 3;
    if (nOutput % 4 > 0)
        cbDecoded += (nOutput % 4) - 1;

    arrOutput.SetCount(cbDecoded);

    size_t nSrc = 0;
    DWORD cbDest = 0;
    while (nSrc + 3 < nOutput)
    {
        DWORD dwQuad = (static_cast<DWORD>(arrTemp.GetAt(nSrc)) << 18) |
                       (static_cast<DWORD>(arrTemp.GetAt(nSrc + 1)) << 12) |
                       (static_cast<DWORD>(arrTemp.GetAt(nSrc + 2)) << 6) |
                       static_cast<DWORD>(arrTemp.GetAt(nSrc + 3));
        if (cbDest < cbDecoded) arrOutput.GetAt(cbDest++) = static_cast<BYTE>((dwQuad >> 16) & 0xFF);
        if (cbDest < cbDecoded) arrOutput.GetAt(cbDest++) = static_cast<BYTE>((dwQuad >> 8) & 0xFF);
        if (cbDest < cbDecoded) arrOutput.GetAt(cbDest++) = static_cast<BYTE>(dwQuad & 0xFF);
        nSrc += 4;
    }

    return S_OK;
}

} // namespace StoryboardManager
