/*
 * DatabasePropertyManager.h
 *
 * Project property persistence for metadata and settings storage.
 * Manages reading/writing of project properties to/from an ESE
 * (Extensible Storage Engine) database and/or the .wlmp XML file.
 *
 * RTTI: ?AVDatabasePropertyManager@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef DATABASEPROPERTYMANAGER_H
#define DATABASEPROPERTYMANAGER_H

#include "../StoryboardManager.h"

namespace StoryboardManager
{

class MovieProject;

// ============================================================================
// PropertyType
// ============================================================================
enum PropertyType
{
    PropertyTypeString    = 0,
    PropertyTypeInt       = 1,
    PropertyTypeLongLong  = 2,
    PropertyTypeDouble    = 3,
    PropertyTypeBool      = 4,
    PropertyTypeBlob      = 5
};

// ============================================================================
// PropertyValue
// ============================================================================
// Variant-like value storage for a single project property.
//
struct PropertyValue
{
    PropertyType type;
    union
    {
        int         nValue;
        LONGLONG    llValue;
        double      dblValue;
        bool        fValue;
    };
    ATL::CString strValue;
    ATL::CAtlArray<BYTE> blobValue;

    PropertyValue()
        : type(PropertyTypeString)
        , nValue(0)
    {
        fValue = false;
    }

    PropertyValue(const PropertyValue& other)
        : type(other.type)
        , nValue(other.nValue)
        , strValue(other.strValue)
    {
        blobValue.Copy(other.blobValue);
    }

    PropertyValue& operator=(const PropertyValue& other)
    {
        if (this != &other)
        {
            type = other.type;
            nValue = other.nValue;
            strValue = other.strValue;
            blobValue.Copy(other.blobValue);
        }
        return *this;
    }
};

// ============================================================================
// PropertyDefinition
// ============================================================================
// Defines a named property with its type and default value. Used to
// register the set of known project properties with the manager.
//
struct PropertyDefinition
{
    LPCWSTR     pwszName;
    PropertyType type;
    LPCWSTR     pwszDefaultValue;

    PropertyDefinition()
        : pwszName(nullptr)
        , type(PropertyTypeString)
        , pwszDefaultValue(nullptr)
    {
    }
};

// ============================================================================
// DatabasePropertyManager
// ============================================================================
// Manages project properties (metadata, settings, custom key-value pairs)
// with support for ESE database persistence and XML serialization. Tracks
// which properties have been modified since the last save.
//
class STORYBOARD_API DatabasePropertyManager
{
public:
    DatabasePropertyManager();
    ~DatabasePropertyManager();

    // -- Property registration --
    void RegisterProperty(LPCWSTR pszName, PropertyType type,
                          LPCWSTR pszDefaultValue = nullptr);
    void RegisterProperties(const PropertyDefinition* pDefs, size_t cDefs);

    // -- Property access --
    HRESULT GetProperty(LPCWSTR pszName, PropertyValue& value) const;
    HRESULT SetProperty(LPCWSTR pszName, const PropertyValue& value);
    HRESULT GetPropertyString(LPCWSTR pszName, ATL::CString& strValue) const;
    HRESULT SetPropertyString(LPCWSTR pszName, LPCWSTR pszValue);
    HRESULT GetPropertyInt(LPCWSTR pszName, int* pValue) const;
    HRESULT SetPropertyInt(LPCWSTR pszName, int nValue);
    HRESULT GetPropertyLongLong(LPCWSTR pszName, LONGLONG* pValue) const;
    HRESULT SetPropertyLongLong(LPCWSTR pszName, LONGLONG llValue);
    HRESULT GetPropertyDouble(LPCWSTR pszName, double* pValue) const;
    HRESULT SetPropertyDouble(LPCWSTR pszName, double dblValue);
    HRESULT GetPropertyBool(LPCWSTR pszName, bool* pValue) const;
    HRESULT SetPropertyBool(LPCWSTR pszName, bool fValue);

    // -- Property existence --
    bool HasProperty(LPCWSTR pszName) const;
    bool IsPropertyModified(LPCWSTR pszName) const;

    // -- Property enumeration --
    size_t GetPropertyCount() const throw();
    LPCWSTR GetPropertyNameAt(size_t nIndex) const;
    PropertyType GetPropertyType(LPCWSTR pszName) const;

    // -- Persistence --
    HRESULT SaveToProject(MovieProject* pProject);
    HRESULT LoadFromProject(MovieProject* pProject);

    // -- ESE database operations --
    HRESULT OpenDatabase(LPCWSTR pszDatabasePath);
    HRESULT CloseDatabase();
    HRESULT SaveToDatabase();
    HRESULT LoadFromDatabase();
    bool    IsDatabaseOpen() const throw();

    // -- Dirty tracking --
    bool IsDirty() const throw();
    void ClearDirty() throw();
    size_t GetModifiedCount() const throw();

    // -- Clear all properties --
    void Clear();
    void ResetToDefaults();

private:
    // -- Internal property storage --
    struct PropertyEntry
    {
        ATL::CString strName;
        PropertyType type;
        PropertyValue value;
        PropertyValue defaultValue;
        bool fModified;

        PropertyEntry()
            : type(PropertyTypeString)
            , fModified(false)
        {
        }
    };

    // -- Lookup helper --
    int FindProperty(LPCWSTR pszName) const;

    // -- Storage --
    ATL::CAtlArray<PropertyEntry> m_arrProperties;

    // -- ESE handle (opaque) --
    void* m_pEseSession;
    ATL::CString m_strDatabasePath;
    bool m_fDatabaseOpen;
    bool m_fDirty;
};

} // namespace StoryboardManager

#endif // DATABASEPROPERTYMANAGER_H
