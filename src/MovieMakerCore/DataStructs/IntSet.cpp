// IntSet.cpp - Sorted integer set implementation

#include "pch.h"
#include "IntSet.h"

namespace Base
{
namespace DataStructs
{

// ============================================================================
// Construction / Destruction
// ============================================================================

IntSet::IntSet()
{
}

IntSet::~IntSet()
{
}

// ============================================================================
// Copy / Move
// ============================================================================

IntSet::IntSet(const IntSet& other)
    : m_arrValues(other.m_arrValues)
{
}

IntSet& IntSet::operator=(const IntSet& other)
{
    if (this != &other)
        m_arrValues = other.m_arrValues;
    return *this;
}

IntSet::IntSet(IntSet&& other) throw()
    : m_arrValues()
{
    m_arrValues.Copy(other.m_arrValues);
    other.m_arrValues.RemoveAll();
}

IntSet& IntSet::operator=(IntSet&& other) throw()
{
    if (this != &other)
    {
        m_arrValues.Copy(other.m_arrValues);
        other.m_arrValues.RemoveAll();
    }
    return *this;
}

// ============================================================================
// Add
// ============================================================================

void IntSet::Add(DWORD dwValue)
{
    int nIndex = BinarySearch(dwValue);
    if (nIndex >= 0)
        return; // Already exists

    // Insert at the correct position to maintain sorted order
    size_t nInsertPos = LowerBound(dwValue);
    m_arrValues.InsertAt(nInsertPos, dwValue);
}

void IntSet::AddRange(const DWORD* pdwValues, size_t cValues)
{
    if (!pdwValues)
        return;

    for (size_t i = 0; i < cValues; ++i)
        Add(pdwValues[i]);
}

// ============================================================================
// Remove
// ============================================================================

bool IntSet::Remove(DWORD dwValue)
{
    int nIndex = BinarySearch(dwValue);
    if (nIndex < 0)
        return false;

    m_arrValues.RemoveAt(nIndex);
    return true;
}

// ============================================================================
// Contains
// ============================================================================

bool IntSet::Contains(DWORD dwValue) const
{
    return BinarySearch(dwValue) >= 0;
}

// ============================================================================
// Clear / Count
// ============================================================================

void IntSet::Clear()
{
    m_arrValues.RemoveAll();
}

size_t IntSet::GetCount() const throw()
{
    return m_arrValues.GetCount();
}

bool IntSet::IsEmpty() const throw()
{
    return m_arrValues.GetCount() == 0;
}

// ============================================================================
// Access
// ============================================================================

DWORD IntSet::GetAt(size_t nIndex) const
{
    return m_arrValues[nIndex];
}

int IntSet::IndexOf(DWORD dwValue) const
{
    return BinarySearch(dwValue);
}

// ============================================================================
// Min / Max
// ============================================================================

DWORD IntSet::GetMin() const
{
    if (m_arrValues.GetCount() == 0)
        return 0;
    return m_arrValues[0];
}

DWORD IntSet::GetMax() const
{
    if (m_arrValues.GetCount() == 0)
        return 0;
    return m_arrValues[m_arrValues.GetCount() - 1];
}

// ============================================================================
// Set operations
// ============================================================================

IntSet IntSet::Union(const IntSet& other) const
{
    IntSet result(*this);
    result.UnionWith(other);
    return result;
}

IntSet IntSet::Intersect(const IntSet& other) const
{
    IntSet result;
    size_t i = 0, j = 0;

    while (i < m_arrValues.GetCount() && j < other.m_arrValues.GetCount())
    {
        if (m_arrValues[i] < other.m_arrValues[j])
            i++;
        else if (m_arrValues[i] > other.m_arrValues[j])
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

IntSet IntSet::Difference(const IntSet& other) const
{
    IntSet result(*this);
    result.Subtract(other);
    return result;
}

void IntSet::UnionWith(const IntSet& other)
{
    for (size_t i = 0; i < other.m_arrValues.GetCount(); ++i)
        Add(other.m_arrValues[i]);
}

void IntSet::IntersectWith(const IntSet& other)
{
    IntSet result = this->Intersect(other);
    m_arrValues = result.m_arrValues;
}

void IntSet::Subtract(const IntSet& other)
{
    for (size_t i = 0; i < other.m_arrValues.GetCount(); ++i)
        Remove(other.m_arrValues[i]);
}

// ============================================================================
// Comparison
// ============================================================================

bool IntSet::operator==(const IntSet& other) const
{
    if (m_arrValues.GetCount() != other.m_arrValues.GetCount())
        return false;

    for (size_t i = 0; i < m_arrValues.GetCount(); ++i)
    {
        if (m_arrValues[i] != other.m_arrValues[i])
            return false;
    }

    return true;
}

bool IntSet::operator!=(const IntSet& other) const
{
    return !(*this == other);
}

// ============================================================================
// Bulk operations
// ============================================================================

void IntSet::Reserve(size_t nCapacity)
{
    m_arrValues.SetCount(nCapacity);
    // Reset count to zero after allocation
    // Note: CAtlArray doesn't have a Reserve method, so we pre-allocate
    // by setting count then removing all (this reserves the memory)
    size_t nCurrent = m_arrValues.GetCount();
    m_arrValues.RemoveAll();
    if (nCapacity > nCurrent)
        m_arrValues.SetCount(nCapacity);
    m_arrValues.RemoveAll();
}

void IntSet::Sort()
{
    // Already maintained in sorted order by Add/Remove
    // This is a no-op but exists for API completeness
}

// ============================================================================
// Private helpers
// ============================================================================

int IntSet::BinarySearch(DWORD dwValue) const
{
    size_t nLow = 0;
    size_t nHigh = m_arrValues.GetCount();

    while (nLow < nHigh)
    {
        size_t nMid = nLow + (nHigh - nLow) / 2;
        DWORD dwMid = m_arrValues[nMid];

        if (dwMid == dwValue)
            return static_cast<int>(nMid);
        else if (dwMid < dwValue)
            nLow = nMid + 1;
        else
            nHigh = nMid;
    }

    return -1;
}

size_t IntSet::LowerBound(DWORD dwValue) const
{
    size_t nLow = 0;
    size_t nHigh = m_arrValues.GetCount();

    while (nLow < nHigh)
    {
        size_t nMid = nLow + (nHigh - nLow) / 2;

        if (m_arrValues[nMid] < dwValue)
            nLow = nMid + 1;
        else
            nHigh = nMid;
    }

    return nLow;
}

} // namespace DataStructs
} // namespace Base
