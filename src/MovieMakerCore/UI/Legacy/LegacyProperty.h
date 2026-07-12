/*
 * LegacyProperty.h
 *
 * Legacy property binding classes for pre-DUI era property management.
 *
 * ComplexProperty: Multi-value property binding (pre-DUI era).
 * SingleProperty: Single value property binding.
 * TransformProperty: Transform-specific property.
 *
 * RTTI: ?AVComplexProperty@@, ?AVSingleProperty@@, ?AVTransformProperty@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef LEGACY_PROPERTY_H
#define LEGACY_PROPERTY_H

#include "../../pch.h"
#include "LegacyText.h"

// ============================================================================
// PropertyType enum
// ============================================================================
enum LegacyPropertyType
{
    LegacyPropertyTypeUnknown     = 0,
    LegacyPropertyTypeInteger     = 1,
    LegacyPropertyTypeFloat       = 2,
    LegacyPropertyTypeString      = 3,
    LegacyPropertyTypeColor       = 4,
    LegacyPropertyTypeBoolean     = 5,
    LegacyPropertyTypeExtent      = 6,
    LegacyPropertyTypeTransform   = 7
};

// ============================================================================
// PropertyValue
// ============================================================================
// Union-based variant type for legacy property values.
//
struct PropertyValue
{
    LegacyPropertyType type;
    union
    {
        int     nInt;
        float   flFloat;
        bool    bBool;
        COLORREF crColor;
    };
    ATL::CString strValue;

    PropertyValue() : type(LegacyPropertyTypeUnknown), nInt(0) {}
    explicit PropertyValue(int val) : type(LegacyPropertyTypeInteger), nInt(val) {}
    explicit PropertyValue(float val) : type(LegacyPropertyTypeFloat), flFloat(val) {}
    explicit PropertyValue(bool val) : type(LegacyPropertyTypeBoolean), bBool(val) {}
    explicit PropertyValue(COLORREF val) : type(LegacyPropertyTypeColor), crColor(val) {}
    explicit PropertyValue(LPCWSTR val) : type(LegacyPropertyTypeString), nInt(0), strValue(val ? val : L"") {}

    int     AsInt() const { return nInt; }
    float   AsFloat() const { return flFloat; }
    bool    AsBool() const { return bBool; }
    COLORREF AsColor() const { return crColor; }
    LPCWSTR AsString() const { return strValue; }
};

// ============================================================================
// SingleProperty
// ============================================================================
// Single value property binding. Holds a single named property with a
// variant value and change notification. Used by the legacy property
// system to bind UI element attributes to model properties.
//
class SingleProperty
{
public:
    SingleProperty();
    explicit SingleProperty(LPCWSTR pszName);
    ~SingleProperty();

    // -- Identity --
    void SetName(LPCWSTR pszName);
    ATL::CString GetName() const;

    void SetId(DWORD dwId);
    DWORD GetId() const throw();

    // -- Value --
    void SetValue(const PropertyValue& value);
    PropertyValue GetValue() const;

    void SetInt(int nValue);
    int GetInt() const throw();

    void SetFloat(float flValue);
    float GetFloat() const throw();

    void SetString(LPCWSTR pszValue);
    ATL::CString GetString() const;

    void SetColor(COLORREF crValue);
    COLORREF GetColor() const throw();

    void SetBool(bool bValue);
    bool GetBool() const throw();

    // -- Change notification --
    void SetDirty(bool bDirty);
    bool IsDirty() const throw();

    // -- Cloning --
    SingleProperty* Clone() const;

private:
    ATL::CString   m_strName;
    DWORD          m_dwId;
    PropertyValue  m_value;
    bool           m_bDirty;
};

// ============================================================================
// ComplexProperty
// ============================================================================
// Multi-value property binding. Contains a collection of SingleProperty
// entries organized as a property bag. Used for composite properties like
// transition effects, text styles, and animation parameters in the legacy
// project format.
//
class ComplexProperty
{
public:
    ComplexProperty();
    ~ComplexProperty();

    // -- Identity --
    void SetName(LPCWSTR pszName);
    ATL::CString GetName() const;

    void SetType(DWORD dwType);
    DWORD GetType() const throw();

    // -- Sub-property management --
    void AddProperty(SingleProperty* pProperty);
    void RemoveProperty(LPCWSTR pszName);
    void RemoveAllProperties();
    size_t GetPropertyCount() const throw();

    SingleProperty* GetProperty(LPCWSTR pszName) const;
    SingleProperty* GetPropertyAt(size_t nIndex) const;
    SingleProperty* GetPropertyById(DWORD dwId) const;

    bool HasProperty(LPCWSTR pszName) const;

    // -- Value accessors (convenience) --
    void SetSubValue(LPCWSTR pszName, const PropertyValue& value);
    PropertyValue GetSubValue(LPCWSTR pszName, const PropertyValue& defaultValue = PropertyValue()) const;

    // -- Dirty tracking --
    bool IsDirty() const throw();

    // -- Serialization --
    HRESULT SaveToStream(IStream* pStream);
    HRESULT LoadFromStream(IStream* pStream);

private:
    ATL::CString                    m_strName;
    DWORD                           m_dwType;
    std::vector<SingleProperty*>    m_properties;
    bool                            m_bDirty;
};

// ============================================================================
// TransformProperty
// ============================================================================
// Transform-specific property. Wraps a LegacyTransform with property
// binding semantics for use in the DirectUI property system. Supports
// animation interpolation and keyframe value tracking.
//
class TransformProperty
{
public:
    TransformProperty();
    ~TransformProperty();

    // -- Property identity --
    void SetName(LPCWSTR pszName);
    ATL::CString GetName() const;

    // -- Transform --
    void SetTransform(const LegacyTransform& transform);
    LegacyTransform GetTransform() const;

    // -- Individual transform properties --
    void SetOffsetX(float flX);
    float GetOffsetX() const throw();

    void SetOffsetY(float flY);
    float GetOffsetY() const throw();

    void SetRotation(float flDegrees);
    float GetRotation() const throw();

    void SetScale(float flScaleX, float flScaleY);
    float GetScaleX() const throw();
    float GetScaleY() const throw();

    // -- Animation support --
    void SetAnimated(bool bAnimated);
    bool IsAnimated() const throw();

    void SetKeyframeTime(float flTime);
    float GetKeyframeTime() const throw();

    // -- Dirty --
    void SetDirty(bool bDirty);
    bool IsDirty() const throw();

private:
    ATL::CString     m_strName;
    LegacyTransform  m_transform;
    bool             m_bAnimated;
    float            m_flKeyframeTime;
    bool             m_bDirty;
};

#endif // LEGACY_PROPERTY_H
