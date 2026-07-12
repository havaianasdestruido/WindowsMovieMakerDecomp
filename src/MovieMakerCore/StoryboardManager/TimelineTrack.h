/*
 * TimelineTrack.h
 *
 * Timeline track types and management for the StoryboardManager namespace.
 * Defines the TimelineTrackType enum and track management utilities.
 *
 * Tracks represent the different layers in the project timeline:
 *   - Video: primary video clips (the main storyline)
 *   - Audio: voice narration and audio clips synced to video
 *   - Music: background music tracks
 *   - Title: text overlay titles and credits
 *   - Credits: end credits sequence
 *   - Transition: transition effects between video extents
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef TIMELINETRACK_H
#define TIMELINETRACK_H

#include "StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// TimelineTrack
// ============================================================================
// Represents a single track layer in the project timeline. Tracks are typed
// (video, audio, music, etc.) and contain an ordered sequence of extents.
// The TimelineTrack class wraps ProjectTimeline with additional UI-oriented
// state (visibility, lock status, name).
//
class STORYBOARD_API TimelineTrack
{
public:
    TimelineTrack();
    TimelineTrack(TimelineTrackType type);
    ~TimelineTrack();

    // Track type
    TimelineTrackType GetTrackType() const throw();
    void SetTrackType(TimelineTrackType type) throw();

    // Display name
    ATL::CString GetDisplayName() const;
    void SetDisplayName(LPCWSTR pszName);

    // Default display name for track type
    static ATL::CString GetDefaultName(TimelineTrackType type);

    // Visibility (UI show/hide)
    bool IsVisible() const throw();
    void SetVisible(bool fVisible) throw();

    // Locked (prevents editing)
    bool IsLocked() const throw();
    void SetLocked(bool fLocked) throw();

    // Muted (audio tracks can be individually muted)
    bool IsMuted() const throw();
    void SetMuted(bool fMuted) throw();

    // Volume for audio tracks (0.0 - 1.0)
    double GetVolume() const throw();
    void SetVolume(double dblVolume) throw();

    // Extent management (delegates to internal ProjectTimeline)
    size_t GetExtentCount() const throw();
    DWORD GetExtentIdAt(size_t nIndex) const;
    size_t AddExtent(DWORD dwExtentId);
    void RemoveExtent(DWORD dwExtentId);
    void RemoveAllExtents();
    void MoveExtent(size_t nIndexFrom, size_t nIndexTo);
    int FindExtent(DWORD dwExtentId) const;

    // Total duration (computed from extents)
    LONGLONG GetTotalDurationHns() const throw();

    // Track height in UI (pixels)
    UINT GetTrackHeight() const throw();
    void SetTrackHeight(UINT uHeight) throw();

    // Color for track display in storyboard grid
    DWORD GetTrackColor() const throw();
    void SetTrackColor(DWORD dwColor) throw();

    // Associated underlying timeline
    ProjectTimeline* GetTimeline();
    const ProjectTimeline* GetTimeline() const;

private:
    TimelineTrackType   m_type;
    ATL::CString        m_strDisplayName;
    bool                m_fVisible;
    bool                m_fLocked;
    bool                m_fMuted;
    double              m_dblVolume;
    UINT                m_uTrackHeight;
    DWORD               m_dwTrackColor;
    ProjectTimeline     m_timeline;
};

// ============================================================================
// TimelineTrackManager
// ============================================================================
// Manages the set of all tracks in a project. Provides track lookup by type,
// enumeration, and aggregate operations.
//
class STORYBOARD_API TimelineTrackManager
{
public:
    TimelineTrackManager();
    ~TimelineTrackManager();

    // Initialize with default track set (one of each type)
    void InitializeDefaults();

    // Track access
    size_t GetTrackCount() const throw();
    TimelineTrack* GetTrackAt(size_t nIndex);
    const TimelineTrack* GetTrackAt(size_t nIndex) const;

    // Find by type
    TimelineTrack* FindTrackByType(TimelineTrackType type);
    const TimelineTrack* FindTrackByType(TimelineTrackType type) const;

    // Add/remove custom tracks
    size_t AddTrack(TimelineTrack* pTrack);
    void RemoveTrackAt(size_t nIndex);
    void RemoveAllTracks();

    // Reorder tracks
    void MoveTrack(size_t nIndexFrom, size_t nIndexTo);

    // Aggregate total duration across all tracks
    LONGLONG GetMaxTrackDurationHns() const throw();

    // Visible track count
    size_t GetVisibleTrackCount() const throw();

    // Find track containing a given extent
    TimelineTrack* FindTrackForExtent(DWORD dwExtentId);

private:
    ATL::CAtlArray<TimelineTrack*> m_arrTracks;
};

// ============================================================================
// Inline helpers
// ============================================================================

inline ATL::CString TimelineTrack::GetDefaultName(TimelineTrackType type)
{
    switch (type)
    {
    case TimelineTrackTypeVideo:      return ATL::CString(L"Video");
    case TimelineTrackTypeAudio:      return ATL::CString(L"Audio");
    case TimelineTrackTypeMusic:      return ATL::CString(L"Music");
    case TimelineTrackTypeTitle:      return ATL::CString(L"Titles");
    case TimelineTrackTypeCredits:    return ATL::CString(L"Credits");
    case TimelineTrackTypeTransition: return ATL::CString(L"Transitions");
    default:                          return ATL::CString(L"Track");
    }
}

} // namespace StoryboardManager

#endif // TIMELINETRACK_H
