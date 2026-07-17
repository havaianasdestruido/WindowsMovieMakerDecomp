/*
 * ClipboardManager.cpp
 *
 * Implementation of the clipboard operations manager for Sundance.
 * Handles cut/copy/paste of timeline items via internal and system
 * clipboard mechanisms.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "ClipboardManager.h"
#include "SundanceAppMain.h"
#include "TimelineController.h"

// ============================================================================
// Construction / destruction
// ============================================================================
ClipboardManager::ClipboardManager()
    : m_pAppMain(NULL)
    , m_bHasData(false)
    , m_bCutMode(false)
    , m_uClipFormat(0)
{
}

ClipboardManager::~ClipboardManager()
{
    Shutdown();
}

// ============================================================================
// Initialize
// ============================================================================
HRESULT ClipboardManager::Initialize(SundanceAppMain* pAppMain)
{
    if (!pAppMain)
        return E_INVALIDARG;

    m_pAppMain = pAppMain;

    HRESULT hr = RegisterFormat();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ============================================================================
// Shutdown
// ============================================================================
void ClipboardManager::Shutdown()
{
    ReleaseClipboardData();
    m_pAppMain = NULL;
}

// ============================================================================
// RegisterFormat
// ============================================================================
HRESULT ClipboardManager::RegisterFormat()
{
    m_uClipFormat = ::RegisterClipboardFormatW(kSundanceClipboardFormat);
    return m_uClipFormat ? S_OK : E_FAIL;
}

// ============================================================================
// Cut
// ============================================================================
HRESULT ClipboardManager::Cut()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    HRESULT hr = Copy();
    if (FAILED(hr))
        return hr;

    m_bCutMode = true;

    // Mark selected items for deletion on paste
    // The actual deletion happens in the timeline controller
    // after a successful paste operation
    return S_OK;
}

// ============================================================================
// Copy
// ============================================================================
HRESULT ClipboardManager::Copy()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    ReleaseClipboardData();
    m_bCutMode = false;

    HRESULT hr = SerializeSelection();
    if (FAILED(hr))
        return hr;

    m_bHasData = !m_clipboardData.empty();
    return m_bHasData ? S_OK : S_FALSE;
}

// ============================================================================
// Paste
// ============================================================================
HRESULT ClipboardManager::Paste()
{
    if (!m_pAppMain || !m_bHasData)
        return E_UNEXPECTED;

    HRESULT hr = DeserializeAndPaste();
    if (SUCCEEDED(hr))
    {
        if (m_bCutMode)
        {
            // After a successful paste in cut mode, clear the source items
            ReleaseClipboardData();
            m_bCutMode = false;
        }
    }

    return hr;
}

// ============================================================================
// Delete
// ============================================================================
HRESULT ClipboardManager::Delete()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    if (!m_pAppMain->IsProjectOpen())
        return E_UNEXPECTED;

    // Serialize the current selection, then remove items from timeline
    ReleaseClipboardData();
    HRESULT hr = SerializeSelection();
    if (FAILED(hr))
        return hr;

    if (m_clipboardData.empty())
        return S_FALSE;

    // Remove each selected item from the timeline
    for (size_t i = 0; i < m_clipboardData.size(); ++i)
    {
        const ClipboardEntry& entry = m_clipboardData[i];
        HRESULT hrDel = m_pAppMain->RemoveItemFromTimeline(
            entry.dwItemId,
            static_cast<TimelineTrack>(entry.track));
        if (FAILED(hrDel))
            hr = hrDel;
    }

    // Clear clipboard after delete (items no longer exist)
    ReleaseClipboardData();

    return hr;
}

// ============================================================================
// CanCut / CanCopy / CanPaste / CanDelete
// ============================================================================
bool ClipboardManager::CanCut() const throw()
{
    return m_pAppMain && m_pAppMain->IsProjectOpen();
}

bool ClipboardManager::CanCopy() const throw()
{
    return m_pAppMain && m_pAppMain->IsProjectOpen();
}

bool ClipboardManager::CanPaste() const throw()
{
    return m_pAppMain && m_pAppMain->IsProjectOpen() && m_bHasData;
}

bool ClipboardManager::CanDelete() const throw()
{
    return m_pAppMain && m_pAppMain->IsProjectOpen();
}

// ============================================================================
// HasInternalClipboardData / GetClipboardItemCount / ClearClipboard
// ============================================================================
bool ClipboardManager::HasInternalClipboardData() const throw()
{
    return m_bHasData;
}

DWORD ClipboardManager::GetClipboardItemCount() const throw()
{
    return static_cast<DWORD>(m_clipboardData.size());
}

void ClipboardManager::ClearClipboard()
{
    ReleaseClipboardData();
}

// ============================================================================
// OnClipboardChanged
// ============================================================================
void ClipboardManager::OnClipboardChanged()
{
    // Check if our custom format is on the system clipboard
    if (::IsClipboardFormatAvailable(m_uClipFormat))
    {
        // Data available from external source if needed
    }
}

// ============================================================================
// SerializeSelection
// ============================================================================
HRESULT ClipboardManager::SerializeSelection()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    if (!m_pAppMain->IsProjectOpen())
        return E_UNEXPECTED;

    StoryboardManagerNamespace::MovieProject* pProject = m_pAppMain->GetProject();
    if (!pProject)
        return E_UNEXPECTED;

    TimelineController* pTimeline = m_pAppMain->GetTimelineController();
    LONGLONG llCursorPos = pTimeline ? pTimeline->GetCurrentPosition() : 0;

    // Iterate through all track types and collect items
    for (int t = 0; t <= static_cast<int>(StoryboardManager::TimelineTrackTypeTransition); ++t)
    {
        StoryboardManager::TimelineTrackType track =
            static_cast<StoryboardManager::TimelineTrackType>(t);

        const StoryboardManager::ProjectTimeline* pTrackTimeline = pProject->GetTimeline(track);
        if (!pTrackTimeline)
            continue;

        size_t cExtents = pTrackTimeline->GetExtentCount();
        for (size_t i = 0; i < cExtents; ++i)
        {
            DWORD dwExtentId = pTrackTimeline->GetExtentIdAt(i);

            // Find the media item that owns this extent
            // We store the extent id as the item identifier
            int nMediaIndex = pProject->FindMediaItemById(dwExtentId);
            if (nMediaIndex < 0)
                continue;

            const StoryboardManager::ProjectMediaItem* pItem = pProject->GetMediaItem(nMediaIndex);
            if (!pItem)
                continue;

            ClipboardEntry entry;
            entry.dwItemId = dwExtentId;
            entry.track = track;
            entry.dwPosition = static_cast<DWORD>(i);  // position in timeline order
            entry.strSourceFile = pItem->GetSourcePath();
            entry.dwDuration = static_cast<DWORD>(pItem->GetDurationHns() / 10000); // hns to ms

            // Serialize the item metadata
            CStringA strSerialized;
            strSerialized.Format("id:%lu,track:%d,pos:%lu,dur:%lu,file:%S",
                entry.dwItemId, entry.track, entry.dwPosition,
                entry.dwDuration, (LPCWSTR)entry.strSourceFile);
            entry.serializedData.assign(
                reinterpret_cast<const BYTE*>(strSerialized.GetString()),
                reinterpret_cast<const BYTE*>(strSerialized.GetString()) + strSerialized.GetLength());

            m_clipboardData.push_back(entry);
        }
    }

    return m_clipboardData.empty() ? S_FALSE : S_OK;
}

// ============================================================================
// DeserializeAndPaste
// ============================================================================
HRESULT ClipboardManager::DeserializeAndPaste()
{
    if (m_clipboardData.empty())
        return S_FALSE;

    if (!m_pAppMain)
        return E_UNEXPECTED;

    if (!m_pAppMain->IsProjectOpen())
        return E_UNEXPECTED;

    StoryboardManagerNamespace::MovieProject* pProject = m_pAppMain->GetProject();
    if (!pProject)
        return E_UNEXPECTED;

    TimelineController* pTimeline = m_pAppMain->GetTimelineController();
    LONGLONG llPastePosition = pTimeline ? pTimeline->GetCurrentPosition() : 0;

    HRESULT hr = S_OK;

    // Paste each clipboard entry at the current cursor position
    for (size_t i = 0; i < m_clipboardData.size(); ++i)
    {
        const ClipboardEntry& entry = m_clipboardData[i];

        // Re-import the media file at the paste position
        if (!entry.strSourceFile.IsEmpty())
        {
            HRESULT hrPaste = m_pAppMain->AddMediaToTimeline(
                entry.strSourceFile,
                static_cast<TimelineTrack>(entry.track));

            if (FAILED(hrPaste))
                hr = hrPaste;
        }
    }

    return hr;
}

// ============================================================================
// ReleaseClipboardData
// ============================================================================
void ClipboardManager::ReleaseClipboardData()
{
    m_clipboardData.clear();
    m_bHasData = false;
}
