/*
 * IntSet.h
 *
 * IntSet - Sorted integer set for extent ID tracking and other
 * DWORD-keyed set operations. Provides O(log n) lookup, insert,
 * and remove via a sorted array.
 *
 * From RTTI analysis: Base::DataStructs::IntSet
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef INTSET_H
#define INTSET_H

#include "DataStructs.h"

namespace Base
{
namespace DataStructs
{

// ============================================================================
// IntSet
// ============================================================================
// A sorted set of DWORD integers. Uses a sorted CAtlArray for compact
// storage with binary-search-based lookup. Primarily used for tracking
// extent IDs, selection sets, and dirty flags.
//
class DATASTRUCT_API IntSet
{
public:
    IntSet();
    ~IntSet();

    // Copy semantics
    IntSet(const IntSet& other);
    IntSet& operator=(const IntSet& other);

    // Move semantics
    IntSet(IntSet&& other) throw();
    IntSet& operator=(IntSet&& other) throw();

    // Add a value to the set (no duplicates)
    void Add(DWORD dwValue);

    // Add a range of values
    void AddRange(const DWORD* pdwValues, size_t cValues);

    // Remove a value from the set
    bool Remove(DWORD dwValue);

    // Check if the set contains a value
    bool Contains(DWORD dwValue) const;

    // Clear all entries
    void Clear();

    // Count
    size_t GetCount() const throw();

    // Is empty
    bool IsEmpty() const throw();

    // Get value by index (sorted order)
    DWORD GetAt(size_t nIndex) const;

    // Find index of value (-1 if not found)
    int IndexOf(DWORD dwValue) const;

    // Minimum / Maximum
    DWORD GetMin() const;
    DWORD GetMax() const;

    // Set operations
    IntSet Union(const IntSet& other) const;
    IntSet Intersect(const IntSet& other) const;
    IntSet Difference(const IntSet& other) const;

    // In-place set operations
    void UnionWith(const IntSet& other);
    void IntersectWith(const IntSet& other);
    void Subtract(const IntSet& other);

    // Comparison
    bool operator==(const IntSet& other) const;
    bool operator!=(const IntSet& other) const;

    // Bulk operations
    void Reserve(size_t nCapacity);
    void Sort(); // Re-sort internal storage (for manual manipulation)

    // Iterator support
    DWORD* begin() { return m_arrValues.GetData(); }
    DWORD* end() { return m_arrValues.GetData() + m_arrValues.GetCount(); }
    const DWORD* begin() const { return m_arrValues.GetData(); }
    const DWORD* end() const { return m_arrValues.GetData() + m_arrValues.GetCount(); }

private:
    ATL::CAtlArray<DWORD> m_arrValues;

    // Binary search helper
    int BinarySearch(DWORD dwValue) const;
    size_t LowerBound(DWORD dwValue) const;
};

} // namespace DataStructs
} // namespace Base

#endif // INTSET_H
