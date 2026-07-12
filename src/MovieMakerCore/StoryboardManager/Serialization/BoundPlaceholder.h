/*
 * BoundPlaceholder.h
 *
 * Placeholder element used during serialization to represent an XML
 * element whose content is bound to a property dictionary or template.
 * Acts as a deferred-binding marker that resolves to actual content
 * during the serialization write pass.
 *
 * RTTI: ?AVBoundPlaceholder@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef BOUNDPLACEHOLDER_H
#define BOUNDPLACEHOLDER_H

#include "../StoryboardManager.h"
#include "ModElements.h"
#include "BoundPropertyDictionary.h"
#include "ValueVariantRef.h"

namespace StoryboardManager
{

// ============================================================================
// BoundPlaceholder
// ============================================================================
// A serialization element placeholder that represents an XML element whose
// content will be resolved from bound properties at write time. During
// reading, the placeholder captures the element name and attributes; during
// writing, it resolves property references to emit the final XML content.
//
// BoundPlaceholder is used for:
//   - X3D template elements whose properties come from BoundPropertyDictionary
//   - Media item elements that reference shared template properties
//   - Transition and effect elements with deferred property resolution
//
// RTTI: ?AVBoundPlaceholder@@
//
class STORYBOARD_API BoundPlaceholder
{
public:
    BoundPlaceholder();
    BoundPlaceholder(LPCWSTR pszElementName);
    ~BoundPlaceholder();

    // -- Copy semantics --
    BoundPlaceholder(const BoundPlaceholder& other);
    BoundPlaceholder& operator=(const BoundPlaceholder& other);

    // -- Element name --
    ATL::CString GetElementName() const;
    void SetElementName(LPCWSTR pszName);

    // -- Element depth in XML tree --
    DWORD GetDepth() const throw();
    void SetDepth(DWORD dwDepth) throw();

    // -- Bound template name (resolved from dictionary) --
    ATL::CString GetBoundTemplateName() const;
    void SetBoundTemplateName(LPCWSTR pszTemplateName);

    // -- Template selector (which template variant to use) --
    ATL::CString GetTemplateSelector() const;
    void SetTemplateSelector(LPCWSTR pszSelector);

    // -- Bound extent ID (which extent this placeholder represents) --
    DWORD GetBoundExtentId() const throw();
    void SetBoundExtentId(DWORD dwExtentId) throw();

    // -- Bound item ID (which media item this placeholder references) --
    DWORD GetBoundItemId() const throw();
    void SetBoundItemId(DWORD dwItemId) throw();

    // -- Property dictionary for this placeholder --
    BoundPropertyDictionary& GetProperties();
    const BoundPropertyDictionary& GetProperties() const;

    // -- Set a single property --
    void SetProperty(LPCWSTR pszName, const ValueVariantRef& value);
    bool GetProperty(LPCWSTR pszName, ValueVariantRef* pValue) const;

    // -- Original element attributes (captured during read) --
    HRESULT GetAttribute(LPCWSTR pszName, ATL::CString& strValue) const;
    HRESULT SetAttribute(LPCWSTR pszName, LPCWSTR pszValue);
    size_t  GetAttributeCount() const throw();

    // -- Resolved flag (true once properties have been resolved) --
    bool IsResolved() const throw();
    void SetResolved(bool fResolved) throw();

    // -- Resolution source --
    enum ResolutionSource
    {
        ResolutionSourceNone        = 0,
        ResolutionSourceDictionary  = 1,
        ResolutionSourceTemplate    = 2,
        ResolutionSourceInherited   = 3
    };

    ResolutionSource GetResolutionSource() const throw();
    void SetResolutionSource(ResolutionSource source) throw();

    // -- Resolve properties from a dictionary --
    HRESULT ResolveFromDictionary(const BoundPropertyDictionary& dictionary);

    // -- Resolve properties from a ModBeginElement (direct attribute copy) --
    HRESULT ResolveFromElement(const ModBeginElement* pElement);

    // -- Emit resolved properties to a ModBeginElement --
    HRESULT EmitToElement(ModBeginElement* pElement) const;

    // -- Emit resolved properties as XML text --
    HRESULT EmitAsXml(ATL::CString& strXml) const;

    // -- Validation --
    bool HasRequiredProperties() const;
    bool IsValid() const throw();

    // -- Reset to unresolved state --
    void Reset();

private:
    struct Attribute
    {
        ATL::CString strName;
        ATL::CString strValue;
    };

    ATL::CString             m_strElementName;
    DWORD                    m_dwDepth;
    ATL::CString             m_strBoundTemplateName;
    ATL::CString             m_strTemplateSelector;
    DWORD                    m_dwBoundExtentId;
    DWORD                    m_dwBoundItemId;
    BoundPropertyDictionary  m_properties;
    ATL::CAtlArray<Attribute> m_arrAttributes;
    bool                     m_fResolved;
    ResolutionSource         m_resolutionSource;
};

// ============================================================================
// BoundPlaceholderContainer
// ============================================================================
// Container managing a collection of BoundPlaceholder objects during
// a serialization pass. Provides lookup by element name, extent ID,
// or template name.
//
class STORYBOARD_API BoundPlaceholderContainer
{
public:
    BoundPlaceholderContainer();
    ~BoundPlaceholderContainer();

    // -- Add placeholder (takes ownership) --
    void AddPlaceholder(BoundPlaceholder* pPlaceholder);

    // -- Remove placeholder --
    void RemovePlaceholderAt(size_t nIndex);
    void RemoveAllPlaceholders();

    // -- Access --
    size_t GetCount() const throw();
    BoundPlaceholder* GetAt(size_t nIndex);
    const BoundPlaceholder* GetAt(size_t nIndex) const;

    // -- Lookup by element name --
    BoundPlaceholder* FindByElementName(LPCWSTR pszName);
    int FindIndexByElementName(LPCWSTR pszName) const;

    // -- Lookup by extent ID --
    BoundPlaceholder* FindByExtentId(DWORD dwExtentId);
    int FindIndexByExtentId(DWORD dwExtentId) const;

    // -- Lookup by item ID --
    BoundPlaceholder* FindByItemId(DWORD dwItemId);

    // -- Resolve all unresolved placeholders --
    size_t ResolveAll(const BoundPropertyDictionary& dictionary);

    // -- Emit all resolved placeholders --
    HRESULT EmitAll(ATL::CAtlArray<ATL::CString>& arrXml) const;

    // -- Count resolved/unresolved --
    size_t GetResolvedCount() const throw();
    size_t GetUnresolvedCount() const throw();

private:
    ATL::CAtlArray<BoundPlaceholder*> m_arrPlaceholders;
};

} // namespace StoryboardManager

#endif // BOUNDPLACEHOLDER_H
