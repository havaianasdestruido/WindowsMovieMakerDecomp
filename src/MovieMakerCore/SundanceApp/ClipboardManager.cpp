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
#include "UndoManager.h"

// ============================================================================
// Construction / destruction
// ============================================================================
ClipboardManager::ClipboardManager()
    : m_pAppMain(NULL)
    , m_bHasData(false)
    , m_bCutMode(false)
    , m_uClipFormat(0)
    , m_uWmmrFormat(0)
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
    if (!m_uClipFormat)
        return E_FAIL;

    m_uWmmrFormat = ::RegisterClipboardFormatW(kWmmrClipboardFormat);
    if (!m_uWmmrFormat)
        return E_FAIL;

    return S_OK;
}

// ============================================================================
// Cut
//
// Serializes the current selection into the internal clipboard buffer,
// then removes the source items from the timeline inside a single undo
// transaction so the entire cut can be undone in one step.
// ============================================================================
HRESULT ClipboardManager::Cut()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    HRESULT hr = Copy();
    if (FAILED(hr))
        return hr;

    m_bCutMode = true;

    // Remove the source items from the timeline, grouped as one undo entry
    UndoManager* pUndoMgr = m_pAppMain->GetUndoManager();
    if (pUndoMgr)
        pUndoMgr->BeginTransaction();

    for (size_t i = 0; i < m_clipboardData.size(); ++i)
    {
        const ClipboardEntry& entry = m_clipboardData[i];
        HRESULT hrDel = m_pAppMain->RemoveItemFromTimeline(
            entry.dwItemId,
            static_cast<TimelineTrack>(entry.track));
        if (FAILED(hrDel))
            hr = hrDel;
    }

    if (pUndoMgr)
        pUndoMgr->EndTransaction();

    return hr;
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
//
// Deserializes clipboard data and adds the items to the timeline.
// Each paste operation is grouped as a single undo transaction.
// ============================================================================
HRESULT ClipboardManager::Paste()
{
    if (!m_pAppMain || !m_bHasData)
        return E_UNEXPECTED;

    UndoManager* pUndoMgr = m_pAppMain->GetUndoManager();
    if (pUndoMgr)
        pUndoMgr->BeginTransaction();

    HRESULT hr = DeserializeAndPaste();

    if (FAILED(hr))
    {
        if (pUndoMgr)
            pUndoMgr->CancelTransaction();
        return hr;
    }

    if (pUndoMgr)
        pUndoMgr->EndTransaction();

    if (m_bCutMode)
    {
        // After a successful paste in cut mode, clear the source items
        ReleaseClipboardData();
        m_bCutMode = false;
    }

    return hr;
}

// ============================================================================
// Delete
//
// Serializes the current selection for undo, then removes each selected
// item from the timeline inside a single undo transaction.
// ============================================================================
HRESULT ClipboardManager::Delete()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    if (!m_pAppMain->IsProjectOpen())
        return E_UNEXPECTED;

    // Serialize the current selection so we can reference it
    ReleaseClipboardData();
    HRESULT hr = SerializeSelection();
    if (FAILED(hr))
        return hr;

    if (m_clipboardData.empty())
        return S_FALSE;

    // Group all removals as a single undo entry
    UndoManager* pUndoMgr = m_pAppMain->GetUndoManager();
    if (pUndoMgr)
        pUndoMgr->BeginTransaction();

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

    if (pUndoMgr)
        pUndoMgr->EndTransaction();

    // Clear clipboard after delete (items no longer exist)
    ReleaseClipboardData();

    return hr;
}

// ============================================================================
// CopySelection — system clipboard copy (WMMR_MediaItems format)
//
// Serializes the selection and places it on the Windows system clipboard
// using the WMMR_MediaItems format, enabling cross-process paste.
// ============================================================================
HRESULT ClipboardManager::CopySelection()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    if (!m_pAppMain->IsProjectOpen())
        return E_UNEXPECTED;

    // Serialize the current selection
    ReleaseClipboardData();
    HRESULT hr = SerializeSelection();
    if (FAILED(hr))
        return hr;

    if (m_clipboardData.empty())
        return S_FALSE;

    const ULONGLONG kMaxClipboardBytes = (256ULL * 1024ULL * 1024ULL);
    const ULONGLONG kDwordMax = 0xFFFFFFFFULL;

    // Compute total size using 64-bit arithmetic to avoid DWORD overflow
    ULONGLONG ullTotal = sizeof(DWORD); // item count header

    for (size_t i = 0; i < m_clipboardData.size(); ++i)
    {
        // Per entry: itemId(4) + track(4) + position(4) + duration(4) +
        //            sourceFileLen(4) + sourceFileBytes + dataLen(4) + dataBytes
        const ClipboardEntry& entry = m_clipboardData[i];
        int cchFile = entry.strSourceFile.GetLength();
        ullTotal += sizeof(DWORD) * 4;                              // id, track, pos, dur
        ullTotal += sizeof(DWORD);                                   // sourceFileLen
        ullTotal += static_cast<ULONGLONG>(cchFile) * sizeof(WCHAR); // sourceFileBytes
        ullTotal += sizeof(DWORD);                                   // dataLen
        ullTotal += entry.serializedData.size();                     // dataBytes
    }

    if (ullTotal > kMaxClipboardBytes || ullTotal > kDwordMax)
        return E_FAIL;

    DWORD dwCount = static_cast<DWORD>(m_clipboardData.size());
    DWORD cbTotal = static_cast<DWORD>(ullTotal);

    HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, cbTotal);
    if (!hMem)
        return E_OUTOFMEMORY;

    BYTE* pDst = static_cast<BYTE*>(::GlobalLock(hMem));
    if (!pDst)
    {
        ::GlobalFree(hMem);
        return E_OUTOFMEMORY;
    }

    SIZE_T remaining = cbTotal;
    BYTE* cur = pDst;

    auto Write = [&](const void* p, SIZE_T n)
    {
        if (remaining < n)
        {
            ::SetLastError(ERROR_INVALID_DATA);
            return false;
        }
        ::memcpy(cur, p, n);
        cur += n;
        remaining -= n;
        return true;
    };

    // Write header
    bool bOk = Write(&dwCount, sizeof(DWORD));

    // Write each entry
    for (size_t i = 0; bOk && i < m_clipboardData.size(); ++i)
    {
        const ClipboardEntry& entry = m_clipboardData[i];

        bOk = Write(&entry.dwItemId, sizeof(DWORD));
        if (!bOk) break;

        DWORD dwTrack = static_cast<DWORD>(entry.track);
        bOk = Write(&dwTrack, sizeof(DWORD));
        if (!bOk) break;

        bOk = Write(&entry.dwPosition, sizeof(DWORD));
        if (!bOk) break;

        bOk = Write(&entry.dwDuration, sizeof(DWORD));
        if (!bOk) break;

        int cchFile = entry.strSourceFile.GetLength();
        DWORD cchFileDw = static_cast<DWORD>(cchFile);
        bOk = Write(&cchFileDw, sizeof(DWORD));
        if (!bOk) break;

        if (cchFile > 0)
        {
            bOk = Write(static_cast<LPCWSTR>(entry.strSourceFile),
                        static_cast<SIZE_T>(cchFile) * sizeof(WCHAR));
            if (!bOk) break;
        }

        DWORD cbData = static_cast<DWORD>(entry.serializedData.size());
        bOk = Write(&cbData, sizeof(DWORD));
        if (!bOk) break;

        if (cbData > 0)
        {
            bOk = Write(entry.serializedData.data(), cbData);
            if (!bOk) break;
        }
    }

    ::GlobalUnlock(hMem);

    if (!bOk)
    {
        ::GlobalFree(hMem);
        return E_FAIL;
    }

    // Place data on the system clipboard
    if (!::OpenClipboard(m_pAppMain->GetMainWindow()))
    {
        ::GlobalFree(hMem);
        return E_FAIL;
    }

    ::EmptyClipboard();
    HANDLE hSet = ::SetClipboardData(m_uWmmrFormat, hMem);
    ::CloseClipboard();

    if (!hSet)
        return E_FAIL;

    return S_OK;
}

// ============================================================================
// PasteSelection — system clipboard paste (WMMR_MediaItems format)
//
// Reads serialized selection data from the Windows system clipboard and
// adds the items to the current timeline.
// ============================================================================
HRESULT ClipboardManager::PasteSelection()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    if (!m_pAppMain->IsProjectOpen())
        return E_UNEXPECTED;

    // Check if our format is available on the system clipboard
    if (!::IsClipboardFormatAvailable(m_uWmmrFormat))
        return S_FALSE;

    if (!::OpenClipboard(m_pAppMain->GetMainWindow()))
        return E_FAIL;

    HANDLE hData = ::GetClipboardData(m_uWmmrFormat);
    if (!hData)
    {
        ::CloseClipboard();
        return S_FALSE;
    }

    BYTE* pSrc = static_cast<BYTE*>(::GlobalLock(hData));
    if (!pSrc)
    {
        ::CloseClipboard();
        return E_FAIL;
    }

    const DWORD kMaxClipboardItemCount = 10000;
    const DWORD kMaxClipboardPathChars = 32768;

    SIZE_T cbBuf = ::GlobalSize(hData);
    SIZE_T cbOffset = 0;

    bool bValid = true;
    std::vector<ClipboardEntry> entries;

    // Read header: item count
    if (cbBuf < sizeof(DWORD))
        bValid = false;

    DWORD dwCount = 0;
    if (bValid)
    {
        ::memcpy(&dwCount, pSrc + cbOffset, sizeof(DWORD));
        cbOffset += sizeof(DWORD);

        if (dwCount > kMaxClipboardItemCount)
            bValid = false;
    }

    if (bValid)
    {
        // Each entry carries at least 6 DWORDs (fixed fields + length fields)
        ULONGLONG ullMinBytes = static_cast<ULONGLONG>(dwCount) * (sizeof(DWORD) * 6);
        if (cbBuf - cbOffset < ullMinBytes)
            bValid = false;
        else
            entries.reserve(dwCount);
    }

    // Parse each entry
    for (DWORD i = 0; bValid && i < dwCount; ++i)
    {
        if (cbBuf - cbOffset < sizeof(DWORD) * 5) // id, track, pos, dur, fileLen
        {
            bValid = false;
            break;
        }

        ClipboardEntry entry;

        ::memcpy(&entry.dwItemId, pSrc + cbOffset, sizeof(DWORD));
        cbOffset += sizeof(DWORD);

        DWORD dwTrack = 0;
        ::memcpy(&dwTrack, pSrc + cbOffset, sizeof(DWORD));
        cbOffset += sizeof(DWORD);

        ::memcpy(&entry.dwPosition, pSrc + cbOffset, sizeof(DWORD));
        cbOffset += sizeof(DWORD);

        ::memcpy(&entry.dwDuration, pSrc + cbOffset, sizeof(DWORD));
        cbOffset += sizeof(DWORD);

        DWORD cchFile = 0;
        ::memcpy(&cchFile, pSrc + cbOffset, sizeof(DWORD));
        cbOffset += sizeof(DWORD);

        ULONGLONG ullFileBytes = static_cast<ULONGLONG>(cchFile) * sizeof(WCHAR);
        if (cchFile > kMaxClipboardPathChars || ullFileBytes > (cbBuf - cbOffset))
        {
            bValid = false;
            break;
        }

        if (cchFile > 0)
        {
            entry.strSourceFile = ATL::CString(
                reinterpret_cast<LPCWSTR>(pSrc + cbOffset), static_cast<int>(cchFile));
            cbOffset += static_cast<SIZE_T>(ullFileBytes);
        }

        if (cbBuf - cbOffset < sizeof(DWORD))
        {
            bValid = false;
            break;
        }

        DWORD cbData = 0;
        ::memcpy(&cbData, pSrc + cbOffset, sizeof(DWORD));
        cbOffset += sizeof(DWORD);

        if (cbData > (cbBuf - cbOffset))
        {
            bValid = false;
            break;
        }

        if (cbData > 0)
        {
            entry.serializedData.assign(pSrc + cbOffset, pSrc + cbOffset + cbData);
            cbOffset += cbData;
        }

        if (dwTrack > static_cast<DWORD>(StoryboardManager::TimelineTrackTypeTransition))
        {
            bValid = false;
            break;
        }
        entry.track = static_cast<StoryboardManager::TimelineTrackType>(dwTrack);

        entries.push_back(entry);
    }

    ::GlobalUnlock(hData);
    ::CloseClipboard();

    if (!bValid || entries.empty())
        return bValid ? S_FALSE : E_FAIL;

    // Paste each entry as a grouped undo transaction
    UndoManager* pUndoMgr = m_pAppMain->GetUndoManager();
    if (pUndoMgr)
        pUndoMgr->BeginTransaction();

    HRESULT hr = S_OK;
    for (size_t i = 0; i < entries.size(); ++i)
    {
        const ClipboardEntry& entry = entries[i];
        if (!entry.strSourceFile.IsEmpty())
        {
            HRESULT hrPaste = m_pAppMain->AddMediaToTimeline(
                entry.strSourceFile,
                static_cast<TimelineTrack>(entry.track));
            if (FAILED(hrPaste))
                hr = hrPaste;
        }
    }

    if (pUndoMgr)
        pUndoMgr->EndTransaction();

    return hr;
}

// ============================================================================
// DeleteSelection
//
// Serializes the current selection, removes the items from the timeline
// inside a single undo transaction, then clears the internal buffer.
// ============================================================================
HRESULT ClipboardManager::DeleteSelection()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    if (!m_pAppMain->IsProjectOpen())
        return E_UNEXPECTED;

    ReleaseClipboardData();
    HRESULT hr = SerializeSelection();
    if (FAILED(hr))
        return hr;

    if (m_clipboardData.empty())
        return S_FALSE;

    UndoManager* pUndoMgr = m_pAppMain->GetUndoManager();
    if (pUndoMgr)
        pUndoMgr->BeginTransaction();

    for (size_t i = 0; i < m_clipboardData.size(); ++i)
    {
        const ClipboardEntry& entry = m_clipboardData[i];
        HRESULT hrDel = m_pAppMain->RemoveItemFromTimeline(
            entry.dwItemId,
            static_cast<TimelineTrack>(entry.track));
        if (FAILED(hrDel))
            hr = hrDel;
    }

    if (pUndoMgr)
        pUndoMgr->EndTransaction();

    ReleaseClipboardData();
    return hr;
}

// ============================================================================
// HasData — checks both internal clipboard and system clipboard
// ============================================================================
bool ClipboardManager::HasData() const throw()
{
    if (m_bHasData)
        return true;

    // Check system clipboard for our custom format
    if (m_uWmmrFormat && ::IsClipboardFormatAvailable(m_uWmmrFormat))
        return true;

    return false;
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
    return m_pAppMain && m_pAppMain->IsProjectOpen() &&
           (m_bHasData || (m_uWmmrFormat && ::IsClipboardFormatAvailable(m_uWmmrFormat)));
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
    if (::IsClipboardFormatAvailable(m_uClipFormat) ||
        ::IsClipboardFormatAvailable(m_uWmmrFormat))
    {
        // Clear internal buffer if external clipboard has taken precedence
        if (!m_bCutMode)
        {
            ReleaseClipboardData();
        }
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
