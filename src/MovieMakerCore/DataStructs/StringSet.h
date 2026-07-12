/*
 * StringSet.h
 *
 * StringSet - String set for media item tracking and other
 * string-keyed set operations. Provides O(log n) lookup via sorted storage.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef STRINGSET_H
#define STRINGSET_H

#include "DataStructs.h"

namespace Base
{
namespace DataStructs
{

// ============================================================================
// StringSet
// ============================================================================
// A sorted set of ATL::CString values. Used for tracking unique media
// item paths, tag strings, and other string-keyed collections. Provides
// O(log n) lookup via binary search on the sorted internal array.
//
class DATASTRUCT_API StringSet
{
public:
    StringSet();
    ~StringSet();

    // Copy semantics
    StringSet(const StringSet& other);
    StringSet& operator=(const StringSet& other);

    // Move semantics
    StringSet(StringSet&& other) throw();
    StringSet& operator=(StringSet&& other) throw();

    // Add a string to the set (no duplicates)
    void Add(LPCWSTR pszValue);

    // Add from CString
    void Add(const ATL::CString& strValue);

    // Remove a string from the set
    bool Remove(LPCWSTR pszValue);

    // Check if the set contains a string
    bool Contains(LPCWSTR pszValue) const;

    // Clear all entries
    void Clear();

    // Count
    size_t GetCount() const throw();

    // Is empty
    bool IsEmpty() const throw();

    // Get string by index (sorted order)
    ATL::CString GetAt(size_t nIndex) const;

    // Find index of string (-1 if not found)
    int IndexOf(LPCWSTR pszValue) const;

    // Comparison
    bool operator==(const StringSet& other) const;
    bool operator!=(const StringSet& other) const;

    // Set operations
    StringSet Union(const StringSet& other) const;
    StringSet Intersect(const StringSet& other) const;
    StringSet Difference(const StringSet& other) const;

    // In-place set operations
    void UnionWith(const StringSet& other);
    void IntersectWith(const StringSet& other);
    void Subtract(const StringSet& other);

    // Bulk operations
    void AddRange(const ATL::CAtlArray<ATL::CString>& arrValues);

    // Case-insensitive option
    void SetCaseInsensitive(bool fCaseInsensitive) throw();
    bool IsCaseInsensitive() const throw();

private:
    ATL::CAtlArray<ATL::CString> m_arrValues;
    bool m_fCaseInsensitive;

    int BinarySearch(LPCWSTR pszValue) const;
    size_t LowerBound(LPCWSTR pszValue) const;
    int CompareStrings(LPCWSTR psz1, LPCWSTR psz2) const;
};

} // namespace DataStructs
} // namespace Base

#endif // STRINGSET_H
