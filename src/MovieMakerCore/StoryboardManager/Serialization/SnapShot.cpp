#include "pch.h"
/*
 * SnapShot.cpp
 *
 * Implementation of project state snapshots for undo/redo and preview
 * thumbnail generation.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "SnapShot.h"

namespace StoryboardManager
{

// ============================================================================
// SnapShotExtentMapContainer implementation
// ============================================================================

SnapShotExtentMapContainer::SnapShotExtentMapContainer()
{
}

SnapShotExtentMapContainer::~SnapShotExtentMapContainer()
{
}

SnapShotExtentMapContainer::SnapShotExtentMapContainer(const SnapShotExtentMapContainer& other)
{
    m_arrEntries.Copy(other.m_arrEntries);
}

SnapShotExtentMapContainer& SnapShotExtentMapContainer::operator=(const SnapShotExtentMapContainer& other)
{
    if (this != &other)
    {
        m_arrEntries.Copy(other.m_arrEntries);
    }
    return *this;
}

void SnapShotExtentMapContainer::AddEntry(DWORD dwExtentId, DWORD dwItemId,
                                          LONGLONG llStartHns, LONGLONG llEndHns,
                                          DWORD dwFlags)
{
    SnapShotExtentMapEntry entry;
    entry.dwExtentId = dwExtentId;
    entry.dwItemId   = dwItemId;
    entry.llStartHns = llStartHns;
    entry.llEndHns   = llEndHns;
    entry.dwFlags    = dwFlags;
    m_arrEntries.Add(entry);
}

void SnapShotExtentMapContainer::RemoveEntry(DWORD dwExtentId)
{
    int nIndex = FindByExtentId(dwExtentId);
    if (nIndex >= 0)
    {
        m_arrEntries.RemoveAt(static_cast<size_t>(nIndex));
    }
}

void SnapShotExtentMapContainer::RemoveAllEntries()
{
    m_arrEntries.RemoveAll();
}

bool SnapShotExtentMapContainer::ContainsExtent(DWORD dwExtentId) const
{
    return FindByExtentId(dwExtentId) >= 0;
}

bool SnapShotExtentMapContainer::ContainsItem(DWORD dwItemId) const
{
    return FindByItemId(dwItemId) >= 0;
}

int SnapShotExtentMapContainer::FindByExtentId(DWORD dwExtentId) const
{
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).dwExtentId == dwExtentId)
            return static_cast<int>(i);
    }
    return -1;
}

int SnapShotExtentMapContainer::FindByItemId(DWORD dwItemId) const
{
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries.GetAt(i).dwItemId == dwItemId)
            return static_cast<int>(i);
    }
    return -1;
}

size_t SnapShotExtentMapContainer::GetCount() const throw()
{
    return m_arrEntries.GetCount();
}

const SnapShotExtentMapEntry& SnapShotExtentMapContainer::GetAt(size_t nIndex) const
{
    return m_arrEntries.GetAt(nIndex);
}

SnapShotExtentMapEntry& SnapShotExtentMapContainer::GetAt(size_t nIndex)
{
    return m_arrEntries.GetAt(nIndex);
}

const SnapShotExtentMapEntry* SnapShotExtentMapContainer::GetByExtentId(DWORD dwExtentId) const
{
    int nIndex = FindByExtentId(dwExtentId);
    if (nIndex >= 0)
        return &m_arrEntries.GetAt(static_cast<size_t>(nIndex));
    return nullptr;
}

SnapShotExtentMapEntry* SnapShotExtentMapContainer::GetByExtentId(DWORD dwExtentId)
{
    int nIndex = FindByExtentId(dwExtentId);
    if (nIndex >= 0)
        return &m_arrEntries.GetAt(static_cast<size_t>(nIndex));
    return nullptr;
}

void SnapShotExtentMapContainer::GetExtentIds(ATL::CAtlArray<DWORD>& arrIds) const
{
    arrIds.RemoveAll();
    arrIds.SetCount(m_arrEntries.GetCount());
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        arrIds.SetAt(i, m_arrEntries.GetAt(i).dwExtentId);
    }
}

void SnapShotExtentMapContainer::MergeFrom(const SnapShotExtentMapContainer& source)
{
    for (size_t i = 0; i < source.m_arrEntries.GetCount(); ++i)
    {
        const SnapShotExtentMapEntry& srcEntry = source.m_arrEntries.GetAt(i);
        int nIndex = FindByExtentId(srcEntry.dwExtentId);
        if (nIndex >= 0)
        {
            m_arrEntries.GetAt(static_cast<size_t>(nIndex)) = srcEntry;
        }
        else
        {
            m_arrEntries.Add(srcEntry);
        }
    }
}

// ============================================================================
// SnapShot implementation
// ============================================================================

SnapShot::SnapShot()
    : m_dwSnapshotId(0)
    , m_dwSequenceNumber(0)
    , m_llTotalDurationHns(0)
    , m_dwDirtyFlags(0)
{
    m_ftTimestamp.dwLowDateTime  = 0;
    m_ftTimestamp.dwHighDateTime = 0;
}

SnapShot::~SnapShot()
{
    RemoveAllMediaItems();
}

SnapShot::SnapShot(const SnapShot& other)
    : m_dwSnapshotId(other.m_dwSnapshotId)
    , m_ftTimestamp(other.m_ftTimestamp)
    , m_dwSequenceNumber(other.m_dwSequenceNumber)
    , m_extentMap(other.m_extentMap)
    , m_strProjectPath(other.m_strProjectPath)
    , m_strProjectTitle(other.m_strProjectTitle)
    , m_llTotalDurationHns(other.m_llTotalDurationHns)
    , m_strThumbnailPath(other.m_strThumbnailPath)
    , m_dwDirtyFlags(other.m_dwDirtyFlags)
{
    for (size_t i = 0; i < other.m_arrMediaItems.GetCount(); ++i)
    {
        MediaItemBase* pClone = other.m_arrMediaItems.GetAt(i)->Clone();
        m_arrMediaItems.Add(pClone);
    }
}

SnapShot& SnapShot::operator=(const SnapShot& other)
{
    if (this != &other)
    {
        RemoveAllMediaItems();

        m_dwSnapshotId       = other.m_dwSnapshotId;
        m_ftTimestamp        = other.m_ftTimestamp;
        m_dwSequenceNumber   = other.m_dwSequenceNumber;
        m_extentMap          = other.m_extentMap;
        m_strProjectPath     = other.m_strProjectPath;
        m_strProjectTitle    = other.m_strProjectTitle;
        m_llTotalDurationHns = other.m_llTotalDurationHns;
        m_strThumbnailPath   = other.m_strThumbnailPath;
        m_dwDirtyFlags       = other.m_dwDirtyFlags;

        for (size_t i = 0; i < other.m_arrMediaItems.GetCount(); ++i)
        {
            MediaItemBase* pClone = other.m_arrMediaItems.GetAt(i)->Clone();
            m_arrMediaItems.Add(pClone);
        }
    }
    return *this;
}

DWORD SnapShot::GetSnapshotId() const throw()
{
    return m_dwSnapshotId;
}

void SnapShot::SetSnapshotId(DWORD dwId) throw()
{
    m_dwSnapshotId = dwId;
}

FILETIME SnapShot::GetTimestamp() const throw()
{
    return m_ftTimestamp;
}

void SnapShot::SetTimestamp(const FILETIME& ft) throw()
{
    m_ftTimestamp = ft;
}

DWORD SnapShot::GetSequenceNumber() const throw()
{
    return m_dwSequenceNumber;
}

void SnapShot::SetSequenceNumber(DWORD dwSeq) throw()
{
    m_dwSequenceNumber = dwSeq;
}

SnapShotExtentMapContainer& SnapShot::GetExtentMap()
{
    return m_extentMap;
}

const SnapShotExtentMapContainer& SnapShot::GetExtentMap() const
{
    return m_extentMap;
}

size_t SnapShot::GetMediaItemCount() const throw()
{
    return m_arrMediaItems.GetCount();
}

void SnapShot::AddMediaItem(MediaItemBase* pItem)
{
    if (pItem)
    {
        m_arrMediaItems.Add(pItem);
    }
}

MediaItemBase* SnapShot::GetMediaItemAt(size_t nIndex)
{
    if (nIndex >= m_arrMediaItems.GetCount())
        return nullptr;
    return m_arrMediaItems.GetAt(nIndex);
}

const MediaItemBase* SnapShot::GetMediaItemAt(size_t nIndex) const
{
    if (nIndex >= m_arrMediaItems.GetCount())
        return nullptr;
    return m_arrMediaItems.GetAt(nIndex);
}

void SnapShot::RemoveAllMediaItems()
{
    for (size_t i = 0; i < m_arrMediaItems.GetCount(); ++i)
    {
        delete m_arrMediaItems.GetAt(i);
    }
    m_arrMediaItems.RemoveAll();
}

MediaItemBase* SnapShot::FindMediaItemById(DWORD dwItemId)
{
    for (size_t i = 0; i < m_arrMediaItems.GetCount(); ++i)
    {
        if (m_arrMediaItems.GetAt(i)->GetItemId() == dwItemId)
            return m_arrMediaItems.GetAt(i);
    }
    return nullptr;
}

const MediaItemBase* SnapShot::FindMediaItemById(DWORD dwItemId) const
{
    for (size_t i = 0; i < m_arrMediaItems.GetCount(); ++i)
    {
        if (m_arrMediaItems.GetAt(i)->GetItemId() == dwItemId)
            return m_arrMediaItems.GetAt(i);
    }
    return nullptr;
}

ATL::CString SnapShot::GetProjectPath() const
{
    return m_strProjectPath;
}

void SnapShot::SetProjectPath(LPCWSTR pszPath)
{
    m_strProjectPath = pszPath ? pszPath : L"";
}

ATL::CString SnapShot::GetProjectTitle() const
{
    return m_strProjectTitle;
}

void SnapShot::SetProjectTitle(LPCWSTR pszTitle)
{
    m_strProjectTitle = pszTitle ? pszTitle : L"";
}

LONGLONG SnapShot::GetTotalDurationHns() const throw()
{
    return m_llTotalDurationHns;
}

void SnapShot::SetTotalDurationHns(LONGLONG llDuration) throw()
{
    m_llTotalDurationHns = llDuration;
}

ATL::CString SnapShot::GetThumbnailPath() const
{
    return m_strThumbnailPath;
}

void SnapShot::SetThumbnailPath(LPCWSTR pszPath)
{
    m_strThumbnailPath = pszPath ? pszPath : L"";
}

DWORD SnapShot::GetDirtyFlags() const throw()
{
    return m_dwDirtyFlags;
}

void SnapShot::SetDirtyFlags(DWORD dwFlags) throw()
{
    m_dwDirtyFlags = dwFlags;
}

bool SnapShot::IsValid() const throw()
{
    return m_dwSnapshotId != 0 && m_dwSequenceNumber != 0;
}

bool SnapShot::HasExtentMap() const throw()
{
    return m_extentMap.GetCount() > 0;
}

bool SnapShot::HasMediaItems() const throw()
{
    return m_arrMediaItems.GetCount() > 0;
}

void SnapShot::MergeFrom(const SnapShot& source)
{
    m_extentMap.MergeFrom(source.m_extentMap);

    for (size_t i = 0; i < source.m_arrMediaItems.GetCount(); ++i)
    {
        MediaItemBase* pClone = source.m_arrMediaItems.GetAt(i)->Clone();
        m_arrMediaItems.Add(pClone);
    }

    if (source.m_llTotalDurationHns > 0)
        m_llTotalDurationHns = source.m_llTotalDurationHns;

    if (!source.m_strThumbnailPath.IsEmpty())
        m_strThumbnailPath = source.m_strThumbnailPath;

    m_dwDirtyFlags |= source.m_dwDirtyFlags;
}

void SnapShot::Clear()
{
    m_dwSnapshotId       = 0;
    m_ftTimestamp.dwLowDateTime  = 0;
    m_ftTimestamp.dwHighDateTime = 0;
    m_dwSequenceNumber   = 0;
    m_extentMap.RemoveAllEntries();
    RemoveAllMediaItems();
    m_strProjectPath.Empty();
    m_strProjectTitle.Empty();
    m_llTotalDurationHns = 0;
    m_strThumbnailPath.Empty();
    m_dwDirtyFlags       = 0;
}

// ============================================================================
// SnapShotCollection implementation
// ============================================================================

SnapShotCollection::SnapShotCollection()
    : m_nMaxSize(100)
{
}

SnapShotCollection::~SnapShotCollection()
{
    RemoveAllSnapshots();
}

void SnapShotCollection::AddSnapshot(SnapShot* pSnapshot)
{
    if (pSnapshot)
    {
        m_arrSnapshots.Add(pSnapshot);

        while (m_arrSnapshots.GetCount() > m_nMaxSize)
        {
            SnapShot* pOldest = m_arrSnapshots.GetAt(0);
            delete pOldest;
            m_arrSnapshots.RemoveAt(0);
        }
    }
}

void SnapShotCollection::RemoveSnapshotAt(size_t nIndex)
{
    if (nIndex < m_arrSnapshots.GetCount())
    {
        delete m_arrSnapshots.GetAt(nIndex);
        m_arrSnapshots.RemoveAt(nIndex);
    }
}

void SnapShotCollection::RemoveAllSnapshots()
{
    for (size_t i = 0; i < m_arrSnapshots.GetCount(); ++i)
    {
        delete m_arrSnapshots.GetAt(i);
    }
    m_arrSnapshots.RemoveAll();
}

size_t SnapShotCollection::GetCount() const throw()
{
    return m_arrSnapshots.GetCount();
}

SnapShot* SnapShotCollection::GetAt(size_t nIndex)
{
    if (nIndex >= m_arrSnapshots.GetCount())
        return nullptr;
    return m_arrSnapshots.GetAt(nIndex);
}

const SnapShot* SnapShotCollection::GetAt(size_t nIndex) const
{
    if (nIndex >= m_arrSnapshots.GetCount())
        return nullptr;
    return m_arrSnapshots.GetAt(nIndex);
}

SnapShot* SnapShotCollection::GetLatest()
{
    if (m_arrSnapshots.GetCount() == 0)
        return nullptr;
    return m_arrSnapshots.GetAt(m_arrSnapshots.GetCount() - 1);
}

const SnapShot* SnapShotCollection::GetLatest() const
{
    if (m_arrSnapshots.GetCount() == 0)
        return nullptr;
    return m_arrSnapshots.GetAt(m_arrSnapshots.GetCount() - 1);
}

SnapShot* SnapShotCollection::GetOldest()
{
    if (m_arrSnapshots.GetCount() == 0)
        return nullptr;
    return m_arrSnapshots.GetAt(0);
}

const SnapShot* SnapShotCollection::GetOldest() const
{
    if (m_arrSnapshots.GetCount() == 0)
        return nullptr;
    return m_arrSnapshots.GetAt(0);
}

SnapShot* SnapShotCollection::FindBySequenceNumber(DWORD dwSeq)
{
    for (size_t i = 0; i < m_arrSnapshots.GetCount(); ++i)
    {
        if (m_arrSnapshots.GetAt(i)->GetSequenceNumber() == dwSeq)
            return m_arrSnapshots.GetAt(i);
    }
    return nullptr;
}

int SnapShotCollection::FindIndexBySequenceNumber(DWORD dwSeq) const
{
    for (size_t i = 0; i < m_arrSnapshots.GetCount(); ++i)
    {
        if (m_arrSnapshots.GetAt(i)->GetSequenceNumber() == dwSeq)
            return static_cast<int>(i);
    }
    return -1;
}

void SnapShotCollection::PruneOldest(size_t nCount)
{
    size_t nToRemove = std::min(nCount, m_arrSnapshots.GetCount());
    for (size_t i = 0; i < nToRemove; ++i)
    {
        delete m_arrSnapshots.GetAt(0);
        m_arrSnapshots.RemoveAt(0);
    }
}

size_t SnapShotCollection::GetMaxSize() const throw()
{
    return m_nMaxSize;
}

void SnapShotCollection::SetMaxSize(size_t nMaxSize) throw()
{
    m_nMaxSize = nMaxSize;
}

} // namespace StoryboardManager
