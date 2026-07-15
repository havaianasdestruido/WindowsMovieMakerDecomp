// StringSet.cpp - String set implementation

#include "pch.h"
#include "StringSet.h"

namespace Base
{
namespace DataStructs
{

// ============================================================================
// Construction / Destruction
// ============================================================================

StringSet::StringSet()
    : m_fCaseInsensitive(true)  // Default: case-insensitive for file paths
{
}

StringSet::~StringSet()
{
}

// ============================================================================
// Copy / Move
// ============================================================================

StringSet::StringSet(const StringSet& other)
    : m_arrValues()
    , m_fCaseInsensitive(other.m_fCaseInsensitive)
{
    m_arrValues.Copy(other.m_arrValues);
}

StringSet& StringSet::operator=(const StringSet& other)
{
    if (this != &other)
    {
        m_arrValues.Copy(other.m_arrValues);
        m_fCaseInsensitive = other.m_fCaseInsensitive;
    }
    return *this;
}

StringSet::StringSet(StringSet&& other) throw()
    : m_fCaseInsensitive(other.m_fCaseInsensitive)
{
    m_arrValues.Copy(other.m_arrValues);
    other.m_arrValues.RemoveAll();
}

StringSet& StringSet::operator=(StringSet&& other) throw()
{
    if (this != &other)
    {
        m_arrValues.Copy(other.m_arrValues);
        other.m_arrValues.RemoveAll();
        m_fCaseInsensitive = other.m_fCaseInsensitive;
    }
    return *this;
}

// ============================================================================
// Add
// ============================================================================

void StringSet::Add(LPCWSTR pszValue)
{
    if (!pszValue)
        return;

    int nIndex = BinarySearch(pszValue);
    if (nIndex >= 0)
        return; // Already exists

    size_t nInsertPos = LowerBound(pszValue);
    ATL::CString strValue(pszValue);
    m_arrValues.InsertAt(nInsertPos, strValue);
}

void StringSet::Add(const ATL::CString& strValue)
{
    Add(strValue.GetString());
}

// ============================================================================
// Remove
// ============================================================================

bool StringSet::Remove(LPCWSTR pszValue)
{
    if (!pszValue)
        return false;

    int nIndex = BinarySearch(pszValue);
    if (nIndex < 0)
        return false;

    m_arrValues.RemoveAt(nIndex);
    return true;
}

// ============================================================================
// Contains
// ============================================================================

bool StringSet::Contains(LPCWSTR pszValue) const
{
    if (!pszValue)
        return false;

    return BinarySearch(pszValue) >= 0;
}

// ============================================================================
// Clear / Count
// ============================================================================

void StringSet::Clear()
{
    m_arrValues.RemoveAll();
}

size_t StringSet::GetCount() const throw()
{
    return m_arrValues.GetCount();
}

bool StringSet::IsEmpty() const throw()
{
    return m_arrValues.GetCount() == 0;
}

// ============================================================================
// Access
// ============================================================================

ATL::CString StringSet::GetAt(size_t nIndex) const
{
    return m_arrValues[nIndex];
}

int StringSet::IndexOf(LPCWSTR pszValue) const
{
    if (!pszValue)
        return -1;

    return BinarySearch(pszValue);
}

// ============================================================================
// Comparison
// ============================================================================

bool StringSet::operator==(const StringSet& other) const
{
    if (m_arrValues.GetCount() != other.m_arrValues.GetCount())
        return false;

    for (size_t i = 0; i < m_arrValues.GetCount(); ++i)
    {
        if (CompareStrings(m_arrValues[i], other.m_arrValues[i]) != 0)
            return false;
    }

    return true;
}

bool StringSet::operator!=(const StringSet& other) const
{
    return !(*this == other);
}

// ============================================================================
// Set operations
// ============================================================================

StringSet StringSet::Union(const StringSet& other) const
{
    StringSet result(*this);
    result.UnionWith(other);
    return result;
}

StringSet StringSet::Intersect(const StringSet& other) const
{
    StringSet result;
    size_t i = 0, j = 0;

    while (i < m_arrValues.GetCount() && j < other.m_arrValues.GetCount())
    {
        int cmp = CompareStrings(m_arrValues[i], other.m_arrValues[j]);
        if (cmp < 0)
            i++;
        else if (cmp > 0)
            j++;
        else
        {
            result.m_arrValues.Add(m_arrValues[i]);
            i++;
            j++;
        }
    }

    return result;
}

StringSet StringSet::Difference(const StringSet& other) const
{
    StringSet result(*this);
    result.Subtract(other);
    return result;
}

void StringSet::UnionWith(const StringSet& other)
{
    for (size_t i = 0; i < other.m_arrValues.GetCount(); ++i)
        Add(other.m_arrValues[i]);
}

void StringSet::IntersectWith(const StringSet& other)
{
    StringSet result = this->Intersect(other);
    m_arrValues.Copy(result.m_arrValues);
}

void StringSet::Subtract(const StringSet& other)
{
    for (size_t i = 0; i < other.m_arrValues.GetCount(); ++i)
        Remove(other.m_arrValues[i]);
}

// ============================================================================
// Bulk operations
// ============================================================================

void StringSet::AddRange(const ATL::CAtlArray<ATL::CString>& arrValues)
{
    for (size_t i = 0; i < arrValues.GetCount(); ++i)
        Add(arrValues[i]);
}

// ============================================================================
// Case sensitivity
// ============================================================================

void StringSet::SetCaseInsensitive(bool fCaseInsensitive) throw()
{
    if (m_fCaseInsensitive != fCaseInsensitive)
    {
        // Rebuild sorted array with new comparison
        ATL::CAtlArray<ATL::CString> arrTemp;
        arrTemp.Copy(m_arrValues);
        m_arrValues.RemoveAll();

        m_fCaseInsensitive = fCaseInsensitive;

        for (size_t i = 0; i < arrTemp.GetCount(); ++i)
            Add(arrTemp[i]);
    }
}

bool StringSet::IsCaseInsensitive() const throw()
{
    return m_fCaseInsensitive;
}

// ============================================================================
// Private helpers
// ============================================================================

int StringSet::BinarySearch(LPCWSTR pszValue) const
{
    if (!pszValue)
        return -1;

    size_t nLow = 0;
    size_t nHigh = m_arrValues.GetCount();

    while (nLow < nHigh)
    {
        size_t nMid = nLow + (nHigh - nLow) / 2;
        int cmp = CompareStrings(m_arrValues[nMid], pszValue);

        if (cmp == 0)
            return static_cast<int>(nMid);
        else if (cmp < 0)
            nLow = nMid + 1;
        else
            nHigh = nMid;
    }

    return -1;
}

size_t StringSet::LowerBound(LPCWSTR pszValue) const
{
    if (!pszValue)
        return m_arrValues.GetCount();

    size_t nLow = 0;
    size_t nHigh = m_arrValues.GetCount();

    while (nLow < nHigh)
    {
        size_t nMid = nLow + (nHigh - nLow) / 2;
        int cmp = CompareStrings(m_arrValues[nMid], pszValue);

        if (cmp < 0)
            nLow = nMid + 1;
        else
            nHigh = nMid;
    }

    return nLow;
}

int StringSet::CompareStrings(LPCWSTR psz1, LPCWSTR psz2) const
{
    if (!psz1 && !psz2) return 0;
    if (!psz1) return -1;
    if (!psz2) return 1;

    if (m_fCaseInsensitive)
        return lstrcmpiW(psz1, psz2);
    else
        return lstrcmpW(psz1, psz2);
}

} // namespace DataStructs
} // namespace Base
