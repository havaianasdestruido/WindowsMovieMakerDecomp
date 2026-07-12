/*
 * Extents.h
 *
 * Extent collection management classes:
 *   - ExtentCollection: ordered container of MovieExtent objects
 *   - ExtentIdSet: set of extent IDs for selection/tracking
 *   - ExtentIdSetSelectionRangeIterator: iterator for range-based selection
 *   - SelectionIndex: manages the current selection in the storyboard view
 *
 * RTTI classes from analysis:
 *   - ExtentIdSetSelectionRangeIterator
 *   - SelectionIndex
 *   - SimpleFunctionalBase<PtrRef<MovieExtent>>
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef EXTENTS_H
#define EXTENTS_H

#include "StoryboardManager.h"
#include "MovieExtent.h"

namespace StoryboardManager
{

// ============================================================================
// SimpleFunctionalBase<T>
// ============================================================================
// Base class template for functional objects (functors) that operate on
// smart-pointer-wrapped objects. From RTTI: SimpleFunctionalBase<PtrRef<MovieExtent>>
//
template <typename T>
class SimpleFunctionalBase
{
public:
    typedef T ValueType;

    SimpleFunctionalBase() {}
    virtual ~SimpleFunctionalBase() {}

    virtual void operator()(T& value) = 0;
    virtual bool ShouldContinue() const { return true; }
};

// ============================================================================
// ExtentCollection
// ============================================================================
// Ordered collection of MovieExtent objects. Provides add, remove, find,
// and iteration. Extents are stored by pointer (owned) and indexed by
// ExtentId for O(1) lookup.
//
class STORYBOARD_API ExtentCollection
{
public:
    ExtentCollection();
    ~ExtentCollection();

    // Copy semantics (deep copy)
    ExtentCollection(const ExtentCollection& other);
    ExtentCollection& operator=(const ExtentCollection& other);

    // Add an extent (takes ownership)
    size_t AddExtent(MovieExtent* pExtent);

    // Create and add a new extent with given parameters
    MovieExtent* CreateExtent(DWORD dwExtentId, DWORD dwMediaId);

    // Remove an extent by ID (frees memory)
    void RemoveExtent(DWORD dwExtentId);

    // Remove all extents
    void RemoveAll();

    // Find by ID
    MovieExtent* FindExtent(DWORD dwExtentId);
    const MovieExtent* FindExtent(DWORD dwExtentId) const;

    // Find by index
    MovieExtent* GetExtentAt(size_t nIndex);
    const MovieExtent* GetExtentAt(size_t nIndex) const;

    // Count
    size_t GetCount() const throw();

    // Index of
    int IndexOf(DWORD dwExtentId) const;

    // Sort by start time
    void SortByStartTime();

    // Move extent to new position
    void MoveExtent(DWORD dwExtentId, size_t nNewIndex);

    // Apply a functional to each extent
    void ForEach(SimpleFunctionalBase<MovieExtent*>& func);

private:
    ATL::CAtlArray<MovieExtent*>    m_arrExtents;
};

// ============================================================================
// ExtentIdSet
// ============================================================================
// A set of extent IDs, used to track selected extents or extents belonging
// to a particular logical group. Supports add, remove, contains, and
// bulk operations (union, intersection, difference).
//
class STORYBOARD_API ExtentIdSet
{
public:
    ExtentIdSet();
    ~ExtentIdSet();

    // Copy semantics
    ExtentIdSet(const ExtentIdSet& other);
    ExtentIdSet& operator=(const ExtentIdSet& other);

    // Add an extent ID to the set
    void Add(DWORD dwExtentId);

    // Remove an extent ID from the set
    void Remove(DWORD dwExtentId);

    // Check if the set contains an extent ID
    bool Contains(DWORD dwExtentId) const;

    // Clear all entries
    void Clear();

    // Count
    size_t GetCount() const throw();

    // Is empty
    bool IsEmpty() const throw();

    // Get an ID by index (iteration order)
    DWORD GetAt(size_t nIndex) const;

    // Set operations
    void Union(const ExtentIdSet& other);
    void Intersect(const ExtentIdSet& other);
    void Difference(const ExtentIdSet& other);

    // Comparison
    bool operator==(const ExtentIdSet& other) const;
    bool operator!=(const ExtentIdSet& other) const;

    // Bulk add from range
    void AddRange(const DWORD* pdwIds, size_t cIds);

private:
    ATL::CAtlArray<DWORD> m_arrIds;
};

// ============================================================================
// ExtentIdSetSelectionRangeIterator
// ============================================================================
// Iterates over a contiguous range of selected extent IDs within an
// ExtentIdSet. Supports forward iteration and range queries.
//
// From RTTI: ExtentIdSetSelectionRangeIterator
//
class STORYBOARD_API ExtentIdSetSelectionRangeIterator
{
public:
    ExtentIdSetSelectionRangeIterator(const ExtentIdSet& idSet);
    ~ExtentIdSetSelectionRangeIterator();

    // Initialize a range iteration
    void BeginRange();
    void BeginRange(size_t nStartIndex, size_t nCount);

    // Advance to next extent ID in range
    bool MoveNext();

    // Get current extent ID
    DWORD GetCurrent() const;

    // Check if iteration is complete
    bool IsDone() const throw();

    // Reset to beginning of range
    void Reset();

    // Number of items remaining in range
    size_t GetRemaining() const throw();

    // Total items in range
    size_t GetRangeCount() const throw();

private:
    const ExtentIdSet&  m_idSet;
    size_t              m_nStartIndex;
    size_t              m_nEndIndex;
    size_t              m_nCurrentIndex;
    bool                m_fActive;
};

// ============================================================================
// SelectionIndex
// ============================================================================
// Manages the current selection state in the storyboard/timeline view.
// Tracks which extents are selected, the anchor point for range selection,
// and the current keyboard/mouse selection mode.
//
// From RTTI: SelectionIndex
//
class STORYBOARD_API SelectionIndex
{
public:
    SelectionIndex();
    ~SelectionIndex();

    // Current selection (single extent)
    DWORD GetSelectedExtentId() const throw();
    void SetSelectedExtentId(DWORD dwExtentId) throw();

    // Multi-selection set
    ExtentIdSet& GetSelectionSet();
    const ExtentIdSet& GetSelectionSet() const;

    // Select a single extent (clears previous selection)
    void SelectSingle(DWORD dwExtentId);

    // Add/toggle extent in selection (for Ctrl+Click)
    void ToggleExtent(DWORD dwExtentId);

    // Range select (for Shift+Click from anchor to target)
    void SelectRange(DWORD dwAnchorId, DWORD dwTargetId);

    // Select all extents in a collection
    void SelectAll(const ExtentIdSet& allIds);

    // Clear selection
    void ClearSelection();

    // Selection count
    size_t GetSelectionCount() const throw();
    bool HasSelection() const throw();

    // Anchor point for range selection
    DWORD GetAnchorExtentId() const throw();
    void SetAnchorExtentId(DWORD dwAnchorId) throw();

    // Selection mode
    enum SelectionMode
    {
        SelectionModeNone       = 0,
        SelectionModeSingle     = 1,
        SelectionModeMulti      = 2,
        SelectionModeRange      = 3
    };

    SelectionMode GetSelectionMode() const throw();
    void SetSelectionMode(SelectionMode mode) throw();

    // Validate that all selected extents still exist in the given collection
    void ValidateSelection(const ExtentCollection& collection);

private:
    ExtentIdSet     m_selectionSet;
    DWORD           m_dwSelectedExtentId;
    DWORD           m_dwAnchorExtentId;
    SelectionMode   mSelectionMode;
};

} // namespace StoryboardManager

#endif // EXTENTS_H
