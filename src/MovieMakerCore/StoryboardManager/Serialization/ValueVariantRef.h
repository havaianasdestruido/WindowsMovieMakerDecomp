/*
 * ValueVariantRef.h
 *
 * Variant property value reference used in the serialization element
 * system. Provides a tagged union for holding string, integer, double,
 * and boolean values extracted from XML attributes.
 *
 * RTTI: ?AVValueVariantRef@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef VALUEVARIANTREF_H
#define VALUEVARIANTREF_H

#include "../StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// ValueVariantRef
// ============================================================================
// Lightweight variant type for serialization property values. Stores a
// tagged value as either a string, int, LONGLONG, double, or bool, plus
// a reference to the owning element name for error reporting.
//
class STORYBOARD_API ValueVariantRef
{
public:
    ValueVariantRef();
    ValueVariantRef(LPCWSTR pszValue);
    ValueVariantRef(int nValue);
    ValueVariantRef(LONGLONG llValue);
    ValueVariantRef(double dblValue);
    ValueVariantRef(bool fValue);
    ~ValueVariantRef();

    // -- Type --
    enum VariantType
    {
        VariantTypeNone    = 0,
        VariantTypeString  = 1,
        VariantTypeInt     = 2,
        VariantTypeLongLong = 3,
        VariantTypeDouble  = 4,
        VariantTypeBool    = 5
    };

    VariantType GetType() const throw();

    // -- Value getters --
    ATL::CString AsString() const;
    int AsInt() const;
    LONGLONG AsLongLong() const;
    double AsDouble() const;
    bool AsBool() const;

    // -- Value setters --
    void SetString(LPCWSTR pszValue);
    void SetInt(int nValue);
    void SetLongLong(LONGLONG llValue);
    void SetDouble(double dblValue);
    void SetBool(bool fValue);

    // -- Owner reference --
    LPCWSTR GetOwnerElementName() const;
    void SetOwnerElementName(LPCWSTR pszName);

    // -- Empty check --
    bool IsEmpty() const throw();

    // -- Comparison --
    bool operator==(const ValueVariantRef& other) const;
    bool operator!=(const ValueVariantRef& other) const;

private:
    VariantType  m_type;
    union
    {
        int      nValue;
        LONGLONG llValue;
        double   dblValue;
        bool     fValue;
    };
    ATL::CString m_strValue;
    ATL::CString m_strOwnerElement;
};

// ============================================================================
// ValueVariantRefArray
// ============================================================================
// Array of ValueVariantRef objects, used for batch property operations
// during serialization.
//
class STORYBOARD_API ValueVariantRefArray
{
public:
    ValueVariantRefArray();
    ~ValueVariantRefArray();

    // -- Add --
    size_t Add(const ValueVariantRef& var);
    void AddString(LPCWSTR pszName, LPCWSTR pszValue);
    void AddInt(LPCWSTR pszName, int nValue);
    void AddLongLong(LPCWSTR pszName, LONGLONG llValue);
    void AddDouble(LPCWSTR pszName, double dblValue);
    void AddBool(LPCWSTR pszName, bool fValue);

    // -- Access --
    size_t GetCount() const throw();
    const ValueVariantRef& GetAt(size_t nIndex) const;
    ValueVariantRef& GetAt(size_t nIndex);

    // -- Lookup --
    int FindByName(LPCWSTR pszName) const;

    // -- Clear --
    void RemoveAll();

private:
    ATL::CAtlArray<ValueVariantRef> m_arrVariants;
};

} // namespace StoryboardManager

#endif // VALUEVARIANTREF_H
