#include "pch.h"
/*
 * TimelineTrack.cpp
 *
 * Implementation of TimelineTrack and TimelineTrackManager classes
 * for the StoryboardManager namespace.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "TimelineTrack.h"

namespace StoryboardManager
{

// ============================================================================
// Default track heights (pixels)
// ============================================================================
namespace
{
    const UINT kDefaultVideoTrackHeight      = 80;
    const UINT kDefaultAudioTrackHeight      = 40;
    const UINT kDefaultMusicTrackHeight      = 40;
    const UINT kDefaultTitleTrackHeight      = 50;
    const UINT kDefaultCreditsTrackHeight    = 50;
    const UINT kDefaultTransitionTrackHeight = 30;

    // Default track colors (ARGB)
    const DWORD kDefaultVideoTrackColor      = 0xFF2196F3; // blue
    const DWORD kDefaultAudioTrackColor      = 0xFF4CAF50; // green
    const DWORD kDefaultMusicTrackColor      = 0xFF9C27B0; // purple
    const DWORD kDefaultTitleTrackColor      = 0xFFFF9800; // orange
    const DWORD kDefaultCreditsTrackColor    = 0xFFFF5722; // deep orange
    const DWORD kDefaultTransitionTrackColor = 0xFF795548; // brown
}

// ============================================================================
// TimelineTrack implementation
// ============================================================================

TimelineTrack::TimelineTrack()
    : m_type(TimelineTrackTypeVideo)
    , m_fVisible(true)
    , m_fLocked(false)
    , m_fMuted(false)
    , m_dblVolume(1.0)
    , m_uTrackHeight(kDefaultVideoTrackHeight)
    , m_dwTrackColor(kDefaultVideoTrackColor)
    , m_timeline(new ProjectTimeline())
{
    m_strDisplayName = GetDefaultName(m_type);
    m_timeline->SetTrackType(m_type);
}

TimelineTrack::TimelineTrack(TimelineTrackType type)
    : m_type(type)
    , m_fVisible(true)
    , m_fLocked(false)
    , m_fMuted(false)
    , m_dblVolume(1.0)
    , m_uTrackHeight(kDefaultVideoTrackHeight)
    , m_dwTrackColor(kDefaultVideoTrackColor)
    , m_timeline(new ProjectTimeline())
{
    switch (type)
    {
    case TimelineTrackTypeVideo:
        m_uTrackHeight = kDefaultVideoTrackHeight;
        m_dwTrackColor = kDefaultVideoTrackColor;
        break;
    case TimelineTrackTypeAudio:
        m_uTrackHeight = kDefaultAudioTrackHeight;
        m_dwTrackColor = kDefaultAudioTrackColor;
        break;
    case TimelineTrackTypeMusic:
        m_uTrackHeight = kDefaultMusicTrackHeight;
        m_dwTrackColor = kDefaultMusicTrackColor;
        break;
    case TimelineTrackTypeTitle:
        m_uTrackHeight = kDefaultTitleTrackHeight;
        m_dwTrackColor = kDefaultTitleTrackColor;
        break;
    case TimelineTrackTypeCredits:
        m_uTrackHeight = kDefaultCreditsTrackHeight;
        m_dwTrackColor = kDefaultCreditsTrackColor;
        break;
    case TimelineTrackTypeTransition:
        m_uTrackHeight = kDefaultTransitionTrackHeight;
        m_dwTrackColor = kDefaultTransitionTrackColor;
        break;
    }

    m_strDisplayName = GetDefaultName(type);
    m_timeline->SetTrackType(type);
}

TimelineTrack::~TimelineTrack()
{
    delete m_timeline;
    m_timeline = nullptr;
}

TimelineTrackType TimelineTrack::GetTrackType() const throw()
{
    return m_type;
}

void TimelineTrack::SetTrackType(TimelineTrackType type) throw()
{
    m_type = type;
    m_timeline->SetTrackType(type);
}

ATL::CString TimelineTrack::GetDisplayName() const
{
    return m_strDisplayName;
}

void TimelineTrack::SetDisplayName(LPCWSTR pszName)
{
    m_strDisplayName = pszName ? pszName : GetDefaultName(m_type);
}

bool TimelineTrack::IsVisible() const throw()
{
    return m_fVisible;
}

void TimelineTrack::SetVisible(bool fVisible) throw()
{
    m_fVisible = fVisible;
}

bool TimelineTrack::IsLocked() const throw()
{
    return m_fLocked;
}

void TimelineTrack::SetLocked(bool fLocked) throw()
{
    m_fLocked = fLocked;
}

bool TimelineTrack::IsMuted() const throw()
{
    return m_fMuted;
}

void TimelineTrack::SetMuted(bool fMuted) throw()
{
    m_fMuted = fMuted;
}

double TimelineTrack::GetVolume() const throw()
{
    return m_dblVolume;
}

void TimelineTrack::SetVolume(double dblVolume) throw()
{
    if (dblVolume < 0.0) dblVolume = 0.0;
    if (dblVolume > 1.0) dblVolume = 1.0;
    m_dblVolume = dblVolume;
}

size_t TimelineTrack::GetExtentCount() const throw()
{
    return m_timeline->GetExtentCount();
}

DWORD TimelineTrack::GetExtentIdAt(size_t nIndex) const
{
    return m_timeline->GetExtentIdAt(nIndex);
}

size_t TimelineTrack::AddExtent(DWORD dwExtentId)
{
    return m_timeline->AddExtent(dwExtentId);
}

void TimelineTrack::RemoveExtent(DWORD dwExtentId)
{
    m_timeline->RemoveExtent(dwExtentId);
}

void TimelineTrack::RemoveAllExtents()
{
    m_timeline->RemoveAllExtents();
}

void TimelineTrack::MoveExtent(size_t nIndexFrom, size_t nIndexTo)
{
    m_timeline->MoveExtent(nIndexFrom, nIndexTo);
}

int TimelineTrack::FindExtent(DWORD dwExtentId) const
{
    return m_timeline->FindExtent(dwExtentId);
}

LONGLONG TimelineTrack::GetTotalDurationHns() const throw()
{
    return m_timeline->GetTotalDurationHns();
}

UINT TimelineTrack::GetTrackHeight() const throw()
{
    return m_uTrackHeight;
}

void TimelineTrack::SetTrackHeight(UINT uHeight) throw()
{
    m_uTrackHeight = uHeight;
}

DWORD TimelineTrack::GetTrackColor() const throw()
{
    return m_dwTrackColor;
}

void TimelineTrack::SetTrackColor(DWORD dwColor) throw()
{
    m_dwTrackColor = dwColor;
}

ProjectTimeline* TimelineTrack::GetTimeline()
{
    return m_timeline;
}

const ProjectTimeline* TimelineTrack::GetTimeline() const
{
    return m_timeline;
}

// ============================================================================
// TimelineTrackManager implementation
// ============================================================================

TimelineTrackManager::TimelineTrackManager()
{
}

TimelineTrackManager::~TimelineTrackManager()
{
    for (size_t i = 0; i < m_arrTracks.GetCount(); ++i)
    {
        delete m_arrTracks.GetAt(i);
    }
    m_arrTracks.RemoveAll();
}

void TimelineTrackManager::InitializeDefaults()
{
    RemoveAllTracks();

    // Create one track per type in standard order
    static const TimelineTrackType defaultTypes[] = {
        TimelineTrackTypeVideo,
        TimelineTrackTypeAudio,
        TimelineTrackTypeMusic,
        TimelineTrackTypeTitle,
        TimelineTrackTypeCredits,
        TimelineTrackTypeTransition
    };

    for (int i = 0; i < 6; ++i)
    {
        TimelineTrack* pTrack = new TimelineTrack(defaultTypes[i]);
        m_arrTracks.Add(pTrack);
    }
}

size_t TimelineTrackManager::GetTrackCount() const throw()
{
    return m_arrTracks.GetCount();
}

TimelineTrack* TimelineTrackManager::GetTrackAt(size_t nIndex)
{
    if (nIndex >= m_arrTracks.GetCount())
        return nullptr;
    return m_arrTracks.GetAt(nIndex);
}

const TimelineTrack* TimelineTrackManager::GetTrackAt(size_t nIndex) const
{
    if (nIndex >= m_arrTracks.GetCount())
        return nullptr;
    return m_arrTracks.GetAt(nIndex);
}

TimelineTrack* TimelineTrackManager::FindTrackByType(TimelineTrackType type)
{
    for (size_t i = 0; i < m_arrTracks.GetCount(); ++i)
    {
        if (m_arrTracks.GetAt(i)->GetTrackType() == type)
            return m_arrTracks.GetAt(i);
    }
    return nullptr;
}

const TimelineTrack* TimelineTrackManager::FindTrackByType(TimelineTrackType type) const
{
    for (size_t i = 0; i < m_arrTracks.GetCount(); ++i)
    {
        if (m_arrTracks.GetAt(i)->GetTrackType() == type)
            return m_arrTracks.GetAt(i);
    }
    return nullptr;
}

size_t TimelineTrackManager::AddTrack(TimelineTrack* pTrack)
{
    ATLASSERT(pTrack != nullptr);
    return m_arrTracks.Add(pTrack);
}

void TimelineTrackManager::RemoveTrackAt(size_t nIndex)
{
    if (nIndex >= m_arrTracks.GetCount())
        return;

    delete m_arrTracks.GetAt(nIndex);
    m_arrTracks.RemoveAt(nIndex);
}

void TimelineTrackManager::RemoveAllTracks()
{
    for (size_t i = 0; i < m_arrTracks.GetCount(); ++i)
    {
        delete m_arrTracks.GetAt(i);
    }
    m_arrTracks.RemoveAll();
}

void TimelineTrackManager::MoveTrack(size_t nIndexFrom, size_t nIndexTo)
{
    if (nIndexFrom >= m_arrTracks.GetCount() ||
        nIndexTo >= m_arrTracks.GetCount() ||
        nIndexFrom == nIndexTo)
    {
        return;
    }

    TimelineTrack* pTrack = m_arrTracks.GetAt(nIndexFrom);
    m_arrTracks.RemoveAt(nIndexFrom);
    m_arrTracks.InsertAt(nIndexTo, pTrack);
}

LONGLONG TimelineTrackManager::GetMaxTrackDurationHns() const throw()
{
    LONGLONG llMaxDuration = 0;
    for (size_t i = 0; i < m_arrTracks.GetCount(); ++i)
    {
        LONGLONG llDuration = m_arrTracks.GetAt(i)->GetTotalDurationHns();
        if (llDuration > llMaxDuration)
            llMaxDuration = llDuration;
    }
    return llMaxDuration;
}

size_t TimelineTrackManager::GetVisibleTrackCount() const throw()
{
    size_t cVisible = 0;
    for (size_t i = 0; i < m_arrTracks.GetCount(); ++i)
    {
        if (m_arrTracks.GetAt(i)->IsVisible())
            ++cVisible;
    }
    return cVisible;
}

TimelineTrack* TimelineTrackManager::FindTrackForExtent(DWORD dwExtentId)
{
    for (size_t i = 0; i < m_arrTracks.GetCount(); ++i)
    {
        if (m_arrTracks.GetAt(i)->FindExtent(dwExtentId) >= 0)
            return m_arrTracks.GetAt(i);
    }
    return nullptr;
}

} // namespace StoryboardManager
