#include "pch.h"
/*
 * ModElements.cpp
 *
 * Implementation of the serialization element containers for the
 * .wlmp XML format.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "ModElements.h"

namespace StoryboardManager
{

// ============================================================================
// ModElementBase implementation
// ============================================================================

ModElementBase::ModElementBase()
    : m_dwDepth(0)
{
}

ModElementBase::~ModElementBase()
{
}

ATL::CString ModElementBase::GetElementName() const
{
    return m_strElementName;
}

void ModElementBase::SetElementName(LPCWSTR pszName)
{
    m_strElementName = pszName ? pszName : L"";
}

DWORD ModElementBase::GetDepth() const throw()
{
    return m_dwDepth;
}

void ModElementBase::SetDepth(DWORD dwDepth) throw()
{
    m_dwDepth = dwDepth;
}

HRESULT ModElementBase::GetAttribute(LPCWSTR pszName, ATL::CString& strValue) const
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

HRESULT ModElementBase::SetAttribute(LPCWSTR pszName, LPCWSTR pszValue)
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
    attr.strName = pszName;
    attr.strValue = pszValue ? pszValue : L"";
    m_arrAttributes.Add(attr);

    return S_OK;
}

size_t ModElementBase::GetAttributeCount() const throw()
{
    return m_arrAttributes.GetCount();
}

ModElementBase::ElementType ModElementBase::GetElementType() const throw()
{
    return ElementTypeUnknown;
}

// ============================================================================
// ModBeginElement implementation
// ============================================================================

ModBeginElement::ModBeginElement()
    : m_fSelfClosing(false)
{
}

ModBeginElement::ModBeginElement(LPCWSTR pszName)
    : m_fSelfClosing(false)
{
    SetElementName(pszName);
}

ModBeginElement::~ModBeginElement()
{
}

ModElementBase::ElementType ModBeginElement::GetElementType() const throw()
{
    return ElementTypeBegin;
}

bool ModBeginElement::IsSelfClosing() const throw()
{
    return m_fSelfClosing;
}

void ModBeginElement::SetSelfClosing(bool fSelfClosing) throw()
{
    m_fSelfClosing = fSelfClosing;
}

HRESULT ModBeginElement::GetIntAttribute(LPCWSTR pszName, int* pValue) const
{
    if (!pValue)
        return E_POINTER;

    ATL::CString strValue;
    HRESULT hr = GetAttribute(pszName, strValue);
    if (SUCCEEDED(hr))
    {
        *pValue = _wtoi(strValue);
    }
    return hr;
}

HRESULT ModBeginElement::GetDwordAttribute(LPCWSTR pszName, DWORD* pValue) const
{
    if (!pValue)
        return E_POINTER;

    ATL::CString strValue;
    HRESULT hr = GetAttribute(pszName, strValue);
    if (SUCCEEDED(hr))
    {
        *pValue = _wtol(strValue);
    }
    return hr;
}

HRESULT ModBeginElement::GetLONGLONGAttribute(LPCWSTR pszName, LONGLONG* pValue) const
{
    if (!pValue)
        return E_POINTER;

    ATL::CString strValue;
    HRESULT hr = GetAttribute(pszName, strValue);
    if (SUCCEEDED(hr))
    {
        *pValue = _wtoi64(strValue);
    }
    return hr;
}

HRESULT ModBeginElement::GetDoubleAttribute(LPCWSTR pszName, double* pValue) const
{
    if (!pValue)
        return E_POINTER;

    ATL::CString strValue;
    HRESULT hr = GetAttribute(pszName, strValue);
    if (SUCCEEDED(hr))
    {
        *pValue = _wtof(strValue);
    }
    return hr;
}

HRESULT ModBeginElement::GetBoolAttribute(LPCWSTR pszName, bool* pValue) const
{
    if (!pValue)
        return E_POINTER;

    ATL::CString strValue;
    HRESULT hr = GetAttribute(pszName, strValue);
    if (SUCCEEDED(hr))
    {
        *pValue = (_wtoi(strValue) != 0);
    }
    return hr;
}

// ============================================================================
// ModEndElement implementation
// ============================================================================

ModEndElement::ModEndElement()
{
}

ModEndElement::ModEndElement(LPCWSTR pszName)
{
    SetElementName(pszName);
}

ModEndElement::~ModEndElement()
{
}

ModElementBase::ElementType ModEndElement::GetElementType() const throw()
{
    return ElementTypeEnd;
}

// ============================================================================
// ModContainerWithIDLookAhead implementation
// ============================================================================

ModContainerWithIDLookAhead::ModContainerWithIDLookAhead()
    : m_dwContainerId(0)
{
}

ModContainerWithIDLookAhead::ModContainerWithIDLookAhead(LPCWSTR pszName, DWORD dwId)
    : m_dwContainerId(dwId)
{
    SetElementName(pszName);
}

ModContainerWithIDLookAhead::~ModContainerWithIDLookAhead()
{
    RemoveAllChildren();
}

DWORD ModContainerWithIDLookAhead::GetContainerId() const throw()
{
    return m_dwContainerId;
}

void ModContainerWithIDLookAhead::SetContainerId(DWORD dwId) throw()
{
    m_dwContainerId = dwId;
}

size_t ModContainerWithIDLookAhead::GetChildCount() const throw()
{
    return m_arrChildren.GetCount();
}

ModElementBase* ModContainerWithIDLookAhead::GetChildAt(size_t nIndex)
{
    if (nIndex >= m_arrChildren.GetCount())
        return nullptr;
    return m_arrChildren.GetAt(nIndex);
}

const ModElementBase* ModContainerWithIDLookAhead::GetChildAt(size_t nIndex) const
{
    if (nIndex >= m_arrChildren.GetCount())
        return nullptr;
    return m_arrChildren.GetAt(nIndex);
}

void ModContainerWithIDLookAhead::AddChild(ModElementBase* pChild)
{
    if (pChild)
    {
        m_arrChildren.Add(pChild);
    }
}

void ModContainerWithIDLookAhead::RemoveAllChildren()
{
    for (size_t i = 0; i < m_arrChildren.GetCount(); ++i)
    {
        delete m_arrChildren.GetAt(i);
    }
    m_arrChildren.RemoveAll();
}

ModElementBase* ModContainerWithIDLookAhead::FindChildById(DWORD dwId)
{
    for (size_t i = 0; i < m_arrChildren.GetCount(); ++i)
    {
        ModElementBase* pChild = m_arrChildren.GetAt(i);
        if (pChild && pChild->GetElementType() == ModElementBase::ElementTypeBegin)
        {
            ModBeginElement* pBegin = static_cast<ModBeginElement*>(pChild);
            DWORD dwChildId = 0;
            if (SUCCEEDED(pBegin->GetDwordAttribute(L"id", &dwChildId)) && dwChildId == dwId)
            {
                return pChild;
            }
        }
    }
    return nullptr;
}

int ModContainerWithIDLookAhead::FindChildIndex(ModElementBase* pChild)
{
    for (size_t i = 0; i < m_arrChildren.GetCount(); ++i)
    {
        if (m_arrChildren.GetAt(i) == pChild)
            return static_cast<int>(i);
    }
    return -1;
}

// ============================================================================
// ModElementFactory implementation
// ============================================================================

ModBeginElement* ModElementFactory::CreateBegin(LPCWSTR pszName)
{
    if (!pszName)
        return nullptr;
    return new ModBeginElement(pszName);
}

ModEndElement* ModElementFactory::CreateEnd(LPCWSTR pszName)
{
    if (!pszName)
        return nullptr;
    return new ModEndElement(pszName);
}

ModContainerWithIDLookAhead* ModElementFactory::CreateContainer(LPCWSTR pszName, DWORD dwId)
{
    if (!pszName)
        return nullptr;
    return new ModContainerWithIDLookAhead(pszName, dwId);
}

} // namespace StoryboardManager
