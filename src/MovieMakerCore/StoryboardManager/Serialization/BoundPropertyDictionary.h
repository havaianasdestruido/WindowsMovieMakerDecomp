/*
 * BoundPropertyDictionary.h
 *
 * Bound property dictionary container for X3D template properties.
 * Maps string keys to ValueVariantRef values, used to store the
 * property sets that drive X3D rendering templates for transitions
 * and effects.
 *
 * RTTI: ?AVBoundPropertyDictionary@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef BOUNDPROPERTYDICTIONARY_H
#define BOUNDPROPERTYDICTIONARY_H

#include "../StoryboardManager.h"
#include "ValueVariantRef.h"

namespace StoryboardManager
{

// ============================================================================
// BoundPropertyDictionary
// ============================================================================
// String-keyed dictionary of property values used by X3D rendering
// templates. Each transition or effect template defines a set of named
// properties (e.g., "Duration", "Direction", "Intensity") that are
// stored in this dictionary.
//
class STORYBOARD_API BoundPropertyDictionary
{
public:
    BoundPropertyDictionary();
    ~BoundPropertyDictionary();

    // -- Copy semantics --
    BoundPropertyDictionary(const BoundPropertyDictionary& other);
    BoundPropertyDictionary& operator=(const BoundPropertyDictionary& other);

    // -- Property access --
    HRESULT GetProperty(LPCWSTR pszName, ValueVariantRef& var) const;
    HRESULT SetProperty(LPCWSTR pszName, const ValueVariantRef& var);
    HRESULT SetPropertyString(LPCWSTR pszName, LPCWSTR pszValue);
    HRESULT SetPropertyInt(LPCWSTR pszName, int nValue);
    HRESULT SetPropertyDouble(LPCWSTR pszName, double dblValue);
    HRESULT SetPropertyBool(LPCWSTR pszName, bool fValue);

    // -- Typed getters --
    HRESULT GetString(LPCWSTR pszName, ATL::CString& strValue) const;
    HRESULT GetInt(LPCWSTR pszName, int* pValue) const;
    HRESULT GetDouble(LPCWSTR pszName, double* pValue) const;
    HRESULT GetBool(LPCWSTR pszName, bool* pValue) const;

    // -- Existence --
    bool HasProperty(LPCWSTR pszName) const;
    bool RemoveProperty(LPCWSTR pszName);

    // -- Enumeration --
    size_t GetCount() const throw();
    LPCWSTR GetKeyNameAt(size_t nIndex) const;
    const ValueVariantRef& GetValueAt(size_t nIndex) const;

    // -- Bulk operations --
    void Merge(const BoundPropertyDictionary& other);
    void Clear();

    // -- XML serialization support --
    HRESULT WriteToElement(IXmlWriter* pWriter, LPCWSTR pszElementName);
    HRESULT ReadFromElement(IXmlReader* pReader, LPCWSTR pszElementName);

    // -- Template binding validation --
    bool ValidateRequiredProperties(const LPCWSTR* ppszRequired, size_t cRequired) const;

private:
    int FindKey(LPCWSTR pszName) const;

    struct PropertyEntry
    {
        ATL::CString    strKey;
        ValueVariantRef value;
    };

    ATL::CAtlArray<PropertyEntry> m_arrProperties;
};

} // namespace StoryboardManager

#endif // BOUNDPROPERTYDICTIONARY_H
