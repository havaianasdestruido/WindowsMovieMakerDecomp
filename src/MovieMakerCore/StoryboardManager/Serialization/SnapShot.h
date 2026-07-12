/*
 * SnapShot.h
 *
 * Project state snapshot for undo/redo and preview thumbnail generation.
 * Captures the complete state of the timeline and media items at a point
 * in time, with an extent map container for fast extent-to-snapshot lookup.
 *
 * RTTI: ?AVSnapShot@@, ?AVSnapShotExtentMapContainer@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "../StoryboardManager.h"
#include "../MediaItems/MediaItem.h"

namespace StoryboardManager
{

// ============================================================================
// SnapShotExtentMapEntry
// ============================================================================
// A single entry in the extent map, binding an extent ID to the snapshot
// data that represents it at the time the snapshot was taken.
//
struct SnapShotExtentMapEntry
{
    DWORD        dwExtentId;
    DWORD        dwItemId;
    LONGLONG     llStartHns;
    LONGLONG     llEndHns;
    DWORD        dwFlags;

    SnapShotExtentMapEntry()
        : dwExtentId(0)
        , dwItemId(0)
        , llStartHns(0)
        , llEndHns(0)
        , dwFlags(0)
    {
    }
};

// ============================================================================
// SnapShotExtentMapContainer
// ============================================================================
// Container mapping extent IDs to snapshot data. Provides O(n) lookup
// by extent ID or item ID, and supports iteration over all mapped extents.
//
// RTTI: ?AVSnapShotExtentMapContainer@@
//
class STORYBOARD_API SnapShotExtentMapContainer
{
public:
    SnapShotExtentMapContainer();
    ~SnapShotExtentMapContainer();

    // -- Copy semantics --
    SnapShotExtentMapContainer(const SnapShotExtentMapContainer& other);
    SnapShotExtentMapContainer& operator=(const SnapShotExtentMapContainer& other);

    // -- Add mapping --
    void AddEntry(DWORD dwExtentId, DWORD dwItemId,
                  LONGLONG llStartHns, LONGLONG llEndHns,
                  DWORD dwFlags = 0);

    // -- Remove mapping --
    void RemoveEntry(DWORD dwExtentId);
    void RemoveAllEntries();

    // -- Lookup --
    bool ContainsExtent(DWORD dwExtentId) const;
    bool ContainsItem(DWORD dwItemId) const;
    int FindByExtentId(DWORD dwExtentId) const;
    int FindByItemId(DWORD dwItemId) const;

    // -- Access --
    size_t GetCount() const throw();
    const SnapShotExtentMapEntry& GetAt(size_t nIndex) const;
    SnapShotExtentMapEntry& GetAt(size_t nIndex);

    // -- Get entry by extent ID --
    const SnapShotExtentMapEntry* GetByExtentId(DWORD dwExtentId) const;
    SnapShotExtentMapEntry* GetByExtentId(DWORD dwExtentId);

    // -- Get all extent IDs --
    void GetExtentIds(ATL::CAtlArray<DWORD>& arrIds) const;

    // -- Merge another map into this one --
    void MergeFrom(const SnapShotExtentMapContainer& source);

private:
    ATL::CAtlArray<SnapShotExtentMapEntry> m_arrEntries;
};

// ============================================================================
// SnapShot
// ============================================================================
// Represents a complete point-in-time snapshot of a movie project. Contains
// a copy of all media items, the extent map, project settings, and timing
// information needed to restore the project to this exact state.
//
// Snapshots are used for:
//   - Undo/redo history (each edit produces a new snapshot)
//   - Auto-save checkpoints
//   - Preview thumbnail generation at specific timeline positions
//
// RTTI: ?AVSnapShot@@
//
class STORYBOARD_API SnapShot
{
public:
    SnapShot();
    ~SnapShot();

    // -- Copy semantics --
    SnapShot(const SnapShot& other);
    SnapShot& operator=(const SnapShot& other);

    // -- Snapshot identity --
    DWORD GetSnapshotId() const throw();
    void SetSnapshotId(DWORD dwId) throw();

    // -- Timestamp (when snapshot was taken) --
    FILETIME GetTimestamp() const throw();
    void SetTimestamp(const FILETIME& ft) throw();

    // -- Sequence number (monotonically increasing) --
    DWORD GetSequenceNumber() const throw();
    void SetSequenceNumber(DWORD dwSeq) throw();

    // -- Extent map --
    SnapShotExtentMapContainer& GetExtentMap();
    const SnapShotExtentMapContainer& GetExtentMap() const;

    // -- Media items snapshot --
    size_t GetMediaItemCount() const throw();
    void AddMediaItem(MediaItemBase* pItem);
    MediaItemBase* GetMediaItemAt(size_t nIndex);
    const MediaItemBase* GetMediaItemAt(size_t nIndex) const;
    void RemoveAllMediaItems();

    // -- Find media item by ID --
    MediaItemBase* FindMediaItemById(DWORD dwItemId);
    const MediaItemBase* FindMediaItemById(DWORD dwItemId) const;

    // -- Project file path at time of snapshot --
    ATL::CString GetProjectPath() const;
    void SetProjectPath(LPCWSTR pszPath);

    // -- Project title at time of snapshot --
    ATL::CString GetProjectTitle() const;
    void SetProjectTitle(LPCWSTR pszTitle);

    // -- Total duration (hundred-nanoseconds) --
    LONGLONG GetTotalDurationHns() const throw();
    void SetTotalDurationHns(LONGLONG llDuration) throw();

    // -- Thumbnail path (for preview display) --
    ATL::CString GetThumbnailPath() const;
    void SetThumbnailPath(LPCWSTR pszPath);

    // -- Dirty flags at time of snapshot --
    DWORD GetDirtyFlags() const throw();
    void SetDirtyFlags(DWORD dwFlags) throw();

    // -- Validation --
    bool IsValid() const throw();
    bool HasExtentMap() const throw();
    bool HasMediaItems() const throw();

    // -- Merge (combine with another snapshot, used for incremental undo) --
    void MergeFrom(const SnapShot& source);

    // -- Clear all data --
    void Clear();

private:
    DWORD                m_dwSnapshotId;
    FILETIME             m_ftTimestamp;
    DWORD                m_dwSequenceNumber;
    SnapShotExtentMapContainer m_extentMap;
    ATL::CAtlArray<MediaItemBase*> m_arrMediaItems;
    ATL::CString         m_strProjectPath;
    ATL::CString         m_strProjectTitle;
    LONGLONG             m_llTotalDurationHns;
    ATL::CString         m_strThumbnailPath;
    DWORD                m_dwDirtyFlags;
};

// ============================================================================
// SnapShotCollection
// ============================================================================
// Ordered collection of snapshots forming the undo/redo history stack.
// Manages creation, access, and pruning of snapshot entries.
//
class STORYBOARD_API SnapShotCollection
{
public:
    SnapShotCollection();
    ~SnapShotCollection();

    // -- Add snapshot (takes ownership) --
    void AddSnapshot(SnapShot* pSnapshot);

    // -- Remove snapshot at index --
    void RemoveSnapshotAt(size_t nIndex);

    // -- Remove all snapshots --
    void RemoveAllSnapshots();

    // -- Access --
    size_t GetCount() const throw();
    SnapShot* GetAt(size_t nIndex);
    const SnapShot* GetAt(size_t nIndex) const;

    // -- Latest/oldest --
    SnapShot* GetLatest();
    const SnapShot* GetLatest() const;
    SnapShot* GetOldest();
    const SnapShot* GetOldest() const;

    // -- By sequence number --
    SnapShot* FindBySequenceNumber(DWORD dwSeq);
    int FindIndexBySequenceNumber(DWORD dwSeq) const;

    // -- Prune (remove oldest N snapshots) --
    void PruneOldest(size_t nCount);

    // -- Max size enforcement --
    size_t GetMaxSize() const throw();
    void SetMaxSize(size_t nMaxSize) throw();

private:
    ATL::CAtlArray<SnapShot*> m_arrSnapshots;
    size_t                    m_nMaxSize;
};

} // namespace StoryboardManager

#endif // SNAPSHOT_H
