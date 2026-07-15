#include "pch.h"
/*
 * DatabasePropertyManager.cpp
 *
 * Implementation of the project property persistence manager. Handles
 * property registration, access, dirty tracking, and ESE database
 * operations for project metadata and settings.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "DatabasePropertyManager.h"
#include "../MovieProject.h"

namespace StoryboardManager
{

// ============================================================================
// Construction / destruction
// ============================================================================

DatabasePropertyManager::DatabasePropertyManager()
    : m_pEseSession(nullptr)
    , m_fDatabaseOpen(false)
    , m_fDirty(false)
{
}

DatabasePropertyManager::~DatabasePropertyManager()
{
    CloseDatabase();
}

// ============================================================================
// Property registration
// ============================================================================

void DatabasePropertyManager::RegisterProperty(LPCWSTR pszName, PropertyType type,
                                               LPCWSTR pszDefaultValue)
{
    if (!pszName || !pszName[0])
        return;

    if (FindProperty(pszName) >= 0)
        return;

    PropertyEntry entry;
    entry.strName = pszName;
    entry.type = type;
    entry.fModified = false;

    if (pszDefaultValue)
    {
        entry.defaultValue.strValue = pszDefaultValue;
        entry.defaultValue.type = type;

        switch (type)
        {
        case PropertyTypeString:
            entry.value.strValue = pszDefaultValue;
            break;
        case PropertyTypeInt:
            entry.value.nValue = _wtoi(pszDefaultValue);
            entry.defaultValue.nValue = entry.value.nValue;
            break;
        case PropertyTypeLongLong:
            entry.value.llValue = _wtoi64(pszDefaultValue);
            entry.defaultValue.llValue = entry.value.llValue;
            break;
        case PropertyTypeDouble:
            entry.value.dblValue = _wtof(pszDefaultValue);
            entry.defaultValue.dblValue = entry.value.dblValue;
            break;
        case PropertyTypeBool:
            entry.value.fValue = (_wtoi(pszDefaultValue) != 0);
            entry.defaultValue.fValue = entry.value.fValue;
            break;
        }
    }

    m_arrProperties.Add(entry);
}

void DatabasePropertyManager::RegisterProperties(const PropertyDefinition* pDefs, size_t cDefs)
{
    if (!pDefs)
        return;

    for (size_t i = 0; i < cDefs; ++i)
    {
        if (pDefs[i].pwszName)
        {
            RegisterProperty(pDefs[i].pwszName, pDefs[i].type, pDefs[i].pwszDefaultValue);
        }
    }
}

// ============================================================================
// Property access (typed)
// ============================================================================

HRESULT DatabasePropertyManager::GetProperty(LPCWSTR pszName, PropertyValue& value) const
{
    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    value = m_arrProperties.GetAt(static_cast<size_t>(nIndex)).value;
    return S_OK;
}

HRESULT DatabasePropertyManager::SetProperty(LPCWSTR pszName, const PropertyValue& value)
{
    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    entry.value = value;
    entry.fModified = true;
    m_fDirty = true;

    return S_OK;
}

HRESULT DatabasePropertyManager::GetPropertyString(LPCWSTR pszName, ATL::CString& strValue) const
{
    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    const PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    if (entry.type != PropertyTypeString)
        return E_FAIL;

    strValue = entry.value.strValue;
    return S_OK;
}

HRESULT DatabasePropertyManager::SetPropertyString(LPCWSTR pszName, LPCWSTR pszValue)
{
    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    entry.value.strValue = pszValue ? pszValue : L"";
    entry.value.type = PropertyTypeString;
    entry.fModified = true;
    m_fDirty = true;

    return S_OK;
}

HRESULT DatabasePropertyManager::GetPropertyInt(LPCWSTR pszName, int* pValue) const
{
    if (!pValue)
        return E_POINTER;

    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    const PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    if (entry.type != PropertyTypeInt)
        return E_FAIL;

    *pValue = entry.value.nValue;
    return S_OK;
}

HRESULT DatabasePropertyManager::SetPropertyInt(LPCWSTR pszName, int nValue)
{
    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    entry.value.nValue = nValue;
    entry.value.type = PropertyTypeInt;
    entry.fModified = true;
    m_fDirty = true;

    return S_OK;
}

HRESULT DatabasePropertyManager::GetPropertyLongLong(LPCWSTR pszName, LONGLONG* pValue) const
{
    if (!pValue)
        return E_POINTER;

    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    const PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    if (entry.type != PropertyTypeLongLong)
        return E_FAIL;

    *pValue = entry.value.llValue;
    return S_OK;
}

HRESULT DatabasePropertyManager::SetPropertyLongLong(LPCWSTR pszName, LONGLONG llValue)
{
    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    entry.value.llValue = llValue;
    entry.value.type = PropertyTypeLongLong;
    entry.fModified = true;
    m_fDirty = true;

    return S_OK;
}

HRESULT DatabasePropertyManager::GetPropertyDouble(LPCWSTR pszName, double* pValue) const
{
    if (!pValue)
        return E_POINTER;

    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    const PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    if (entry.type != PropertyTypeDouble)
        return E_FAIL;

    *pValue = entry.value.dblValue;
    return S_OK;
}

HRESULT DatabasePropertyManager::SetPropertyDouble(LPCWSTR pszName, double dblValue)
{
    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    entry.value.dblValue = dblValue;
    entry.value.type = PropertyTypeDouble;
    entry.fModified = true;
    m_fDirty = true;

    return S_OK;
}

HRESULT DatabasePropertyManager::GetPropertyBool(LPCWSTR pszName, bool* pValue) const
{
    if (!pValue)
        return E_POINTER;

    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    const PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    if (entry.type != PropertyTypeBool)
        return E_FAIL;

    *pValue = entry.value.fValue;
    return S_OK;
}

HRESULT DatabasePropertyManager::SetPropertyBool(LPCWSTR pszName, bool fValue)
{
    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return E_INVALIDARG;

    PropertyEntry& entry = m_arrProperties.GetAt(static_cast<size_t>(nIndex));
    entry.value.fValue = fValue;
    entry.value.type = PropertyTypeBool;
    entry.fModified = true;
    m_fDirty = true;

    return S_OK;
}

// ============================================================================
// Property existence
// ============================================================================

bool DatabasePropertyManager::HasProperty(LPCWSTR pszName) const
{
    return FindProperty(pszName) >= 0;
}

bool DatabasePropertyManager::IsPropertyModified(LPCWSTR pszName) const
{
    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return false;

    return m_arrProperties.GetAt(static_cast<size_t>(nIndex)).fModified;
}

// ============================================================================
// Property enumeration
// ============================================================================

size_t DatabasePropertyManager::GetPropertyCount() const throw()
{
    return m_arrProperties.GetCount();
}

LPCWSTR DatabasePropertyManager::GetPropertyNameAt(size_t nIndex) const
{
    if (nIndex >= m_arrProperties.GetCount())
        return nullptr;

    return m_arrProperties.GetAt(nIndex).strName;
}

PropertyType DatabasePropertyManager::GetPropertyType(LPCWSTR pszName) const
{
    int nIndex = FindProperty(pszName);
    if (nIndex < 0)
        return PropertyTypeString;

    return m_arrProperties.GetAt(static_cast<size_t>(nIndex)).type;
}

// ============================================================================
// Project persistence
// ============================================================================

HRESULT DatabasePropertyManager::SaveToProject(MovieProject* pProject)
{
    if (!pProject)
        return E_POINTER;

    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        const PropertyEntry& entry = m_arrProperties.GetAt(i);
        if (!entry.fModified)
            continue;

        MovieProjectSettings& settings = pProject->GetSettings();

        if (entry.strName.CompareNoCase(L"ProjectName") == 0)
            settings.SetProjectName(entry.value.strValue);
        else if (entry.strName.CompareNoCase(L"Author") == 0)
            settings.SetAuthor(entry.value.strValue);
    }

    return S_OK;
}

HRESULT DatabasePropertyManager::LoadFromProject(MovieProject* pProject)
{
    if (!pProject)
        return E_POINTER;

    const MovieProjectSettings& settings = pProject->GetSettings();

    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        PropertyEntry& entry = m_arrProperties.GetAt(i);

        if (entry.strName.CompareNoCase(L"ProjectName") == 0)
        {
            entry.value.strValue = settings.GetProjectName();
            entry.value.type = PropertyTypeString;
        }
        else if (entry.strName.CompareNoCase(L"Author") == 0)
        {
            entry.value.strValue = settings.GetAuthor();
            entry.value.type = PropertyTypeString;
        }

        entry.fModified = false;
    }

    m_fDirty = false;
    return S_OK;
}

// ============================================================================
// ESE database operations (stubs - ESE integration is complex)
// ============================================================================

HRESULT DatabasePropertyManager::OpenDatabase(LPCWSTR pszDatabasePath)
{
    if (!pszDatabasePath || !pszDatabasePath[0])
        return E_INVALIDARG;

    if (m_fDatabaseOpen)
        return S_FALSE;

    m_strDatabasePath = pszDatabasePath;
    m_fDatabaseOpen = true;

    return S_OK;
}

HRESULT DatabasePropertyManager::CloseDatabase()
{
    if (!m_fDatabaseOpen)
        return S_FALSE;

    m_pEseSession = nullptr;
    m_fDatabaseOpen = false;
    m_strDatabasePath.Empty();

    return S_OK;
}

HRESULT DatabasePropertyManager::SaveToDatabase()
{
    if (!m_fDatabaseOpen)
        return E_UNEXPECTED;

    return S_OK;
}

HRESULT DatabasePropertyManager::LoadFromDatabase()
{
    if (!m_fDatabaseOpen)
        return E_UNEXPECTED;

    return S_OK;
}

bool DatabasePropertyManager::IsDatabaseOpen() const throw()
{
    return m_fDatabaseOpen;
}

// ============================================================================
// Dirty tracking
// ============================================================================

bool DatabasePropertyManager::IsDirty() const throw()
{
    return m_fDirty;
}

void DatabasePropertyManager::ClearDirty() throw()
{
    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        m_arrProperties.GetAt(i).fModified = false;
    }
    m_fDirty = false;
}

size_t DatabasePropertyManager::GetModifiedCount() const throw()
{
    size_t cModified = 0;
    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        if (m_arrProperties.GetAt(i).fModified)
            ++cModified;
    }
    return cModified;
}

// ============================================================================
// Clear / Reset
// ============================================================================

void DatabasePropertyManager::Clear()
{
    m_arrProperties.RemoveAll();
    m_fDirty = false;
}

void DatabasePropertyManager::ResetToDefaults()
{
    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        PropertyEntry& entry = m_arrProperties.GetAt(i);
        entry.value = entry.defaultValue;
        entry.fModified = false;
    }
    m_fDirty = false;
}

// ============================================================================
// FindProperty (internal)
// ============================================================================

int DatabasePropertyManager::FindProperty(LPCWSTR pszName) const
{
    if (!pszName)
        return -1;

    for (size_t i = 0; i < m_arrProperties.GetCount(); ++i)
    {
        if (m_arrProperties.GetAt(i).strName.CompareNoCase(pszName) == 0)
            return static_cast<int>(i);
    }

    return -1;
}

} // namespace StoryboardManager
