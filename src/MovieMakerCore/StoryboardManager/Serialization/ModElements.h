/*
 * ModElements.h
 *
 * Serialization element containers for the .wlmp XML format. Provides
 * begin/end element markers and container elements with ID lookahead
 * used during project file parsing.
 *
 * RTTI: ?AVModEndElement@@, ?AVModBeginElement@@, ?AVModContainerWithIDLookAhead@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef MODELEMENTS_H
#define MODELEMENTS_H

#include "../StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// ModElementBase
// ============================================================================
// Base class for serialization element containers. Represents an XML
// element boundary (begin or end) with optional attributes.
//
class STORYBOARD_API ModElementBase
{
public:
    ModElementBase();
    virtual ~ModElementBase();

    // -- Element name --
    ATL::CString GetElementName() const;
    void SetElementName(LPCWSTR pszName);

    // -- Element depth --
    DWORD GetDepth() const throw();
    void SetDepth(DWORD dwDepth) throw();

    // -- Attributes --
    HRESULT GetAttribute(LPCWSTR pszName, ATL::CString& strValue) const;
    HRESULT SetAttribute(LPCWSTR pszName, LPCWSTR pszValue);
    size_t  GetAttributeCount() const throw();

    // -- Element type --
    enum ElementType
    {
        ElementTypeUnknown = 0,
        ElementTypeBegin   = 1,
        ElementTypeEnd     = 2,
        ElementTypeText    = 3
    };

    virtual ElementType GetElementType() const throw();

protected:
    struct Attribute
    {
        ATL::CString strName;
        ATL::CString strValue;
    };

    ATL::CAtlArray<Attribute> m_arrAttributes;
    ATL::CString m_strElementName;
    DWORD        m_dwDepth;
};

// ============================================================================
// ModBeginElement
// ============================================================================
// Represents the opening of an XML element (<element>). Carries the
// element name and attributes parsed from the start tag.
//
class STORYBOARD_API ModBeginElement : public ModElementBase
{
public:
    ModBeginElement();
    ModBeginElement(LPCWSTR pszName);
    virtual ~ModBeginElement();

    virtual ElementType GetElementType() const throw() override;

    // -- Self-closing flag (e.g., <element />) --
    bool IsSelfClosing() const throw();
    void SetSelfClosing(bool fSelfClosing) throw();

    // -- Attribute lookup helpers --
    HRESULT GetIntAttribute(LPCWSTR pszName, int* pValue) const;
    HRESULT GetDwordAttribute(LPCWSTR pszName, DWORD* pValue) const;
    HRESULT GetLONGLONGAttribute(LPCWSTR pszName, LONGLONG* pValue) const;
    HRESULT GetDoubleAttribute(LPCWSTR pszName, double* pValue) const;
    HRESULT GetBoolAttribute(LPCWSTR pszName, bool* pValue) const;

private:
    bool m_fSelfClosing;
};

// ============================================================================
// ModEndElement
// ============================================================================
// Represents the closing of an XML element (</element>).
//
class STORYBOARD_API ModEndElement : public ModElementBase
{
public:
    ModEndElement();
    ModEndElement(LPCWSTR pszName);
    virtual ~ModEndElement();

    virtual ElementType GetElementType() const throw() override;
};

// ============================================================================
// ModContainerWithIDLookAhead
// ============================================================================
// A container element that supports ID-based lookup with one-element
// lookahead. Used during parsing to associate child elements with their
// parent's ID attribute before the child's content is fully parsed.
//
// RTTI: ?AVModContainerWithIDLookAhead@@
//
class STORYBOARD_API ModContainerWithIDLookAhead : public ModBeginElement
{
public:
    ModContainerWithIDLookAhead();
    ModContainerWithIDLookAhead(LPCWSTR pszName, DWORD dwId);
    virtual ~ModContainerWithIDLookAhead();

    // -- Container ID --
    DWORD GetContainerId() const throw();
    void SetContainerId(DWORD dwId) throw();

    // -- Child element count --
    size_t GetChildCount() const throw();

    // -- Child element access --
    ModElementBase* GetChildAt(size_t nIndex);
    const ModElementBase* GetChildAt(size_t nIndex) const;

    // -- Add child element (takes ownership) --
    void AddChild(ModElementBase* pChild);

    // -- Remove all children --
    void RemoveAllChildren();

    // -- ID-based child lookup --
    ModElementBase* FindChildById(DWORD dwId);
    int FindChildIndex(ModElementBase* pChild);

private:
    DWORD   m_dwContainerId;
    ATL::CAtlArray<ModElementBase*> m_arrChildren;
};

// ============================================================================
// ModElementFactory
// ============================================================================
// Factory for creating serialization elements by type name.
//
class STORYBOARD_API ModElementFactory
{
public:
    static ModBeginElement* CreateBegin(LPCWSTR pszName);
    static ModEndElement* CreateEnd(LPCWSTR pszName);
    static ModContainerWithIDLookAhead* CreateContainer(LPCWSTR pszName, DWORD dwId);
};

} // namespace StoryboardManager

#endif // MODELEMENTS_H
