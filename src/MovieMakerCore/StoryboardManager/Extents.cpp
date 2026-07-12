/*
 * Extents.cpp
 *
 * Implementation of extent collection management, selection sets, and
 * iteration classes for the StoryboardManager namespace.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "Extents.h"
#include <algorithm>

namespace StoryboardManager
{

// ============================================================================
// ExtentCollection implementation
// ============================================================================

ExtentCollection::ExtentCollection()
{
}

ExtentCollection::~ExtentCollection()
{
    RemoveAll();
}

ExtentCollection::ExtentCollection(const ExtentCollection& other)
{
    for (size_t i = 0; i < other.m_arrExtents.GetCount(); ++i)
    {
        MovieExtent* pClone = other.m_arrExtents.GetAt(i)->Clone();
        m_arrExtents.Add(pClone);
    }
}

ExtentCollection& ExtentCollection::operator=(const ExtentCollection& other)
{
    if (this != &other)
    {
        RemoveAll();
        for (size_t i = 0; i < other.m_arrExtents.GetCount(); ++i)
        {
            MovieExtent* pClone = other.m_arrExtents.GetAt(i)->Clone();
            m_arrExtents.Add(pClone);
        }
    }
    return *this;
}

size_t ExtentCollection::AddExtent(MovieExtent* pExtent)
{
    ATLASSERT(pExtent != nullptr);
    return m_arrExtents.Add(pExtent);
}

MovieExtent* ExtentCollection::CreateExtent(DWORD dwExtentId, DWORD dwMediaId)
{
    MovieExtent* pExtent = new MovieExtent(dwExtentId, dwMediaId);
    m_arrExtents.Add(pExtent);
    return pExtent;
}

void ExtentCollection::RemoveExtent(DWORD dwExtentId)
{
    for (size_t i = 0; i < m_arrExtents.GetCount(); ++i)
    {
        if (m_arrExtents.GetAt(i)->GetExtentId() == dwExtentId)
        {
            delete m_arrExtents.GetAt(i);
            m_arrExtents.RemoveAt(i);
            return;
        }
    }
}

void ExtentCollection::RemoveAll()
{
    for (size_t i = 0; i < m_arrExtents.GetCount(); ++i)
    {
        delete m_arrExtents.GetAt(i);
    }
    m_arrExtents.RemoveAll();
}

MovieExtent* ExtentCollection::FindExtent(DWORD dwExtentId)
{
    for (size_t i = 0; i < m_arrExtents.GetCount(); ++i)
    {
        if (m_arrExtents.GetAt(i)->GetExtentId() == dwExtentId)
            return m_arrExtents.GetAt(i);
    }
    return nullptr;
}

const MovieExtent* ExtentCollection::FindExtent(DWORD dwExtentId) const
{
    for (size_t i = 0; i < m_arrExtents.GetCount(); ++i)
    {
        if (m_arrExtents.GetAt(i)->GetExtentId() == dwExtentId)
            return m_arrExtents.GetAt(i);
    }
    return nullptr;
}

MovieExtent* ExtentCollection::GetExtentAt(size_t nIndex)
{
    if (nIndex >= m_arrExtents.GetCount())
        return nullptr;
    return m_arrExtents.GetAt(nIndex);
}

const MovieExtent* ExtentCollection::GetExtentAt(size_t nIndex) const
{
    if (nIndex >= m_arrExtents.GetCount())
        return nullptr;
    return m_arrExtents.GetAt(nIndex);
}

size_t ExtentCollection::GetCount() const throw()
{
    return m_arrExtents.GetCount();
}

int ExtentCollection::IndexOf(DWORD dwExtentId) const
{
    for (size_t i = 0; i < m_arrExtents.GetCount(); ++i)
    {
        if (m_arrExtents.GetAt(i)->GetExtentId() == dwExtentId)
            return static_cast<int>(i);
    }
    return -1;
}

void ExtentCollection::SortByStartTime()
{
    // Simple insertion sort - timeline extents are typically small collections
    for (size_t i = 1; i < m_arrExtents.GetCount(); ++i)
    {
        MovieExtent* pKey = m_arrExtents.GetAt(i);
        LONGLONG llKeyTime = pKey->GetStartTimeHns();
        size_t j = i;

        while (j > 0 && m_arrExtents.GetAt(j - 1)->GetStartTimeHns() > llKeyTime)
        {
            m_arrExtents.SetAt(j, m_arrExtents.GetAt(j - 1));
            --j;
        }

        m_arrExtents.SetAt(j, pKey);
    }
}

void ExtentCollection::MoveExtent(DWORD dwExtentId, size_t nNewIndex)
{
    int nIndex = IndexOf(dwExtentId);
    if (nIndex < 0 || static_cast<size_t>(nIndex) == nNewIndex)
        return;

    MovieExtent* pExtent = m_arrExtents.GetAt(static_cast<size_t>(nIndex));
    m_arrExtents.RemoveAt(static_cast<size_t>(nIndex));

    if (nNewIndex > m_arrExtents.GetCount())
        nNewIndex = m_arrExtents.GetCount();

    m_arrExtents.InsertAt(nNewIndex, pExtent);
}

void ExtentCollection::ForEach(SimpleFunctionalBase<MovieExtent*>& func)
{
    for (size_t i = 0; i < m_arrExtents.GetCount(); ++i)
    {
        func(m_arrExtents.GetAt(i));
        if (!func.ShouldContinue())
            break;
    }
}

// ============================================================================
// ExtentIdSet implementation
// ============================================================================

ExtentIdSet::ExtentIdSet()
{
}

ExtentIdSet::~ExtentIdSet()
{
}

ExtentIdSet::ExtentIdSet(const ExtentIdSet& other)
    : m_arrIds(other.m_arrIds)
{
}

ExtentIdSet& ExtentIdSet::operator=(const ExtentIdSet& other)
{
    if (this != &other)
    {
        m_arrIds.RemoveAll();
        m_arrIds.Copy(other.m_arrIds);
    }
    return *this;
}

void ExtentIdSet::Add(DWORD dwExtentId)
{
    // Avoid duplicates
    for (size_t i = 0; i < m_arrIds.GetCount(); ++i)
    {
        if (m_arrIds.GetAt(i) == dwExtentId)
            return;
    }
    m_arrIds.Add(dwExtentId);
}

void ExtentIdSet::Remove(DWORD dwExtentId)
{
    for (size_t i = 0; i < m_arrIds.GetCount(); ++i)
    {
        if (m_arrIds.GetAt(i) == dwExtentId)
        {
            m_arrIds.RemoveAt(i);
            return;
        }
    }
}

bool ExtentIdSet::Contains(DWORD dwExtentId) const
{
    for (size_t i = 0; i < m_arrIds.GetCount(); ++i)
    {
        if (m_arrIds.GetAt(i) == dwExtentId)
            return true;
    }
    return false;
}

void ExtentIdSet::Clear()
{
    m_arrIds.RemoveAll();
}

size_t ExtentIdSet::GetCount() const throw()
{
    return m_arrIds.GetCount();
}

bool ExtentIdSet::IsEmpty() const throw()
{
    return m_arrIds.IsEmpty();
}

DWORD ExtentIdSet::GetAt(size_t nIndex) const
{
    ATLASSERT(nIndex < m_arrIds.GetCount());
    return m_arrIds.GetAt(nIndex);
}

void ExtentIdSet::Union(const ExtentIdSet& other)
{
    for (size_t i = 0; i < other.m_arrIds.GetCount(); ++i)
    {
        Add(other.m_arrIds.GetAt(i));
    }
}

void ExtentIdSet::Intersect(const ExtentIdSet& other)
{
    ExtentIdSet result;
    for (size_t i = 0; i < m_arrIds.GetCount(); ++i)
    {
        if (other.Contains(m_arrIds.GetAt(i)))
        {
            result.Add(m_arrIds.GetAt(i));
        }
    }
    m_arrIds.RemoveAll();
    m_arrIds.Copy(result.m_arrIds);
}

void ExtentIdSet::Difference(const ExtentIdSet& other)
{
    for (size_t i = 0; i < other.m_arrIds.GetCount(); ++i)
    {
        Remove(other.m_arrIds.GetAt(i));
    }
}

bool ExtentIdSet::operator==(const ExtentIdSet& other) const
{
    if (m_arrIds.GetCount() != other.m_arrIds.GetCount())
        return false;

    for (size_t i = 0; i < m_arrIds.GetCount(); ++i)
    {
        if (!other.Contains(m_arrIds.GetAt(i)))
            return false;
    }
    return true;
}

bool ExtentIdSet::operator!=(const ExtentIdSet& other) const
{
    return !(*this == other);
}

void ExtentIdSet::AddRange(const DWORD* pdwIds, size_t cIds)
{
    if (!pdwIds)
        return;

    for (size_t i = 0; i < cIds; ++i)
    {
        Add(pdwIds[i]);
    }
}

// ============================================================================
// ExtentIdSetSelectionRangeIterator implementation
// ============================================================================

ExtentIdSetSelectionRangeIterator::ExtentIdSetSelectionRangeIterator(const ExtentIdSet& idSet)
    : m_idSet(idSet)
    , m_nStartIndex(0)
    , m_nEndIndex(0)
    , m_nCurrentIndex(0)
    , m_fActive(false)
{
}

ExtentIdSetSelectionRangeIterator::~ExtentIdSetSelectionRangeIterator()
{
}

void ExtentIdSetSelectionRangeIterator::BeginRange()
{
    m_nStartIndex = 0;
    m_nEndIndex = m_idSet.GetCount();
    m_nCurrentIndex = m_nStartIndex;
    m_fActive = (m_nStartIndex < m_nEndIndex);
}

void ExtentIdSetSelectionRangeIterator::BeginRange(size_t nStartIndex, size_t nCount)
{
    m_nStartIndex = nStartIndex;
    m_nEndIndex = nStartIndex + nCount;
    if (m_nEndIndex > m_idSet.GetCount())
        m_nEndIndex = m_idSet.GetCount();
    m_nCurrentIndex = m_nStartIndex;
    m_fActive = (m_nStartIndex < m_nEndIndex);
}

bool ExtentIdSetSelectionRangeIterator::MoveNext()
{
    if (!m_fActive)
        return false;

    ++m_nCurrentIndex;
    return m_nCurrentIndex < m_nEndIndex;
}

DWORD ExtentIdSetSelectionRangeIterator::GetCurrent() const
{
    if (!m_fActive || m_nCurrentIndex >= m_nEndIndex)
        return 0;
    return m_idSet.GetAt(m_nCurrentIndex);
}

bool ExtentIdSetSelectionRangeIterator::IsDone() const throw()
{
    return !m_fActive || m_nCurrentIndex >= m_nEndIndex;
}

void ExtentIdSetSelectionRangeIterator::Reset()
{
    m_nCurrentIndex = m_nStartIndex;
    m_fActive = (m_nStartIndex < m_nEndIndex);
}

size_t ExtentIdSetSelectionRangeIterator::GetRemaining() const throw()
{
    if (!m_fActive || m_nCurrentIndex >= m_nEndIndex)
        return 0;
    return m_nEndIndex - m_nCurrentIndex;
}

size_t ExtentIdSetSelectionRangeIterator::GetRangeCount() const throw()
{
    if (!m_fActive)
        return 0;
    return m_nEndIndex - m_nStartIndex;
}

// ============================================================================
// SelectionIndex implementation
// ============================================================================

SelectionIndex::SelectionIndex()
    : m_dwSelectedExtentId(0)
    , m_dwAnchorExtentId(0)
    , mSelectionMode(SelectionModeNone)
{
}

SelectionIndex::~SelectionIndex()
{
}

DWORD SelectionIndex::GetSelectedExtentId() const throw()
{
    return m_dwSelectedExtentId;
}

void SelectionIndex::SetSelectedExtentId(DWORD dwExtentId) throw()
{
    m_dwSelectedExtentId = dwExtentId;
}

ExtentIdSet& SelectionIndex::GetSelectionSet()
{
    return m_selectionSet;
}

const ExtentIdSet& SelectionIndex::GetSelectionSet() const
{
    return m_selectionSet;
}

void SelectionIndex::SelectSingle(DWORD dwExtentId)
{
    m_selectionSet.Clear();
    m_selectionSet.Add(dwExtentId);
    m_dwSelectedExtentId = dwExtentId;
    m_dwAnchorExtentId = dwExtentId;
    mSelectionMode = SelectionModeSingle;
}

void SelectionIndex::ToggleExtent(DWORD dwExtentId)
{
    if (m_selectionSet.Contains(dwExtentId))
    {
        m_selectionSet.Remove(dwExtentId);
    }
    else
    {
        m_selectionSet.Add(dwExtentId);
        m_dwSelectedExtentId = dwExtentId;
    }

    mSelectionMode = (m_selectionSet.GetCount() > 1)
        ? SelectionModeMulti
        : (m_selectionSet.GetCount() == 1)
            ? SelectionModeSingle
            : SelectionModeNone;
}

void SelectionIndex::SelectRange(DWORD dwAnchorId, DWORD dwTargetId)
{
    // The range selection includes all IDs between anchor and target.
    // Since extent IDs may not be contiguous, we just select both endpoints
    // and let the UI determine the visual range.
    m_selectionSet.Clear();
    m_selectionSet.Add(dwAnchorId);
    m_selectionSet.Add(dwTargetId);
    m_dwAnchorExtentId = dwAnchorId;
    m_dwSelectedExtentId = dwTargetId;
    mSelectionMode = SelectionModeRange;
}

void SelectionIndex::SelectAll(const ExtentIdSet& allIds)
{
    m_selectionSet = allIds;
    if (allIds.GetCount() > 0)
    {
        m_dwSelectedExtentId = allIds.GetAt(0);
    }
    mSelectionMode = (allIds.GetCount() > 0) ? SelectionModeMulti : SelectionModeNone;
}

void SelectionIndex::ClearSelection()
{
    m_selectionSet.Clear();
    m_dwSelectedExtentId = 0;
    m_dwAnchorExtentId = 0;
    mSelectionMode = SelectionModeNone;
}

size_t SelectionIndex::GetSelectionCount() const throw()
{
    return m_selectionSet.GetCount();
}

bool SelectionIndex::HasSelection() const throw()
{
    return !m_selectionSet.IsEmpty();
}

DWORD SelectionIndex::GetAnchorExtentId() const throw()
{
    return m_dwAnchorExtentId;
}

void SelectionIndex::SetAnchorExtentId(DWORD dwAnchorId) throw()
{
    m_dwAnchorExtentId = dwAnchorId;
}

SelectionIndex::SelectionMode SelectionIndex::GetSelectionMode() const throw()
{
    return mSelectionMode;
}

void SelectionIndex::SetSelectionMode(SelectionMode mode) throw()
{
    mSelectionMode = mode;
}

void SelectionIndex::ValidateSelection(const ExtentCollection& collection)
{
    ExtentIdSet validSet;

    for (size_t i = 0; i < m_selectionSet.GetCount(); ++i)
    {
        DWORD dwId = m_selectionSet.GetAt(i);
        if (collection.FindExtent(dwId) != nullptr)
        {
            validSet.Add(dwId);
        }
    }

    m_selectionSet = validSet;

    if (!m_selectionSet.Contains(m_dwSelectedExtentId))
    {
        m_dwSelectedExtentId = m_selectionSet.GetCount() > 0
            ? m_selectionSet.GetAt(0)
            : 0;
    }

    if (!m_selectionSet.Contains(m_dwAnchorExtentId))
    {
        m_dwAnchorExtentId = m_dwSelectedExtentId;
    }
}

} // namespace StoryboardManager
