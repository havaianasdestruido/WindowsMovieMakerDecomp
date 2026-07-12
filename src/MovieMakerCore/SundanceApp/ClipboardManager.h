/*
 * ClipboardManager.h
 *
 * Clipboard operations manager for the Sundance application framework.
 * Handles cut/copy/paste of timeline items (extents, media clips,
 * effects) using the Windows clipboard and an internal clipboard buffer.
 *
 * RTTI: ?AVClipboardManager@@
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_CLIPBOARD_MANAGER_H
#define SUNDANCE_CLIPBOARD_MANAGER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class SundanceAppMain;

// ============================================================================
// Clipboard data format identifier
// ============================================================================
// Custom clipboard format for Movie Maker timeline items.
// Registered via RegisterClipboardFormat on first use.
//
static const WCHAR kSundanceClipboardFormat[] = L"SundanceTimelineItem";

// ============================================================================
// ClipboardManager
// ============================================================================
// Manages cut/copy/paste operations for timeline items. Maintains an
// internal clipboard buffer that holds serialized timeline item data,
// independent of the system clipboard. This allows paste operations
// to work even after the system clipboard has been overwritten by
// other applications.
//
class ClipboardManager
{
public:
    ClipboardManager();
    ~ClipboardManager();

    // -- Lifecycle --
    HRESULT Initialize(SundanceAppMain* pAppMain);
    void    Shutdown();

    // -- Clipboard operations --
    HRESULT Cut();
    HRESULT Copy();
    HRESULT Paste();
    HRESULT Delete();

    // -- Availability queries --
    bool CanCut() const throw();
    bool CanCopy() const throw();
    bool CanPaste() const throw();
    bool CanDelete() const throw();

    // -- Clipboard state --
    bool    HasInternalClipboardData() const throw();
    DWORD   GetClipboardItemCount() const throw();
    void    ClearClipboard();

    // -- Clipboard change monitoring --
    void    OnClipboardChanged();

private:
    // Internal clipboard buffer entry
    struct ClipboardEntry
    {
        DWORD                   dwItemId;
        TimelineTrack           track;
        DWORD                   dwPosition;
        ATL::CString            strSourceFile;
        DWORD                   dwDuration;
        std::vector<BYTE>       serializedData;

        ClipboardEntry()
            : dwItemId(0)
            , track(TimelineTrack_Unknown)
            , dwPosition(0)
            , dwDuration(0)
        {
        }
    };

    // State
    SundanceAppMain*            m_pAppMain;
    bool                        m_bHasData;
    bool                        m_bCutMode;     // true = cut, false = copy
    std::vector<ClipboardEntry> m_clipboardData;
    UINT                        m_uClipFormat;

    // Internal helpers
    HRESULT SerializeSelection();
    HRESULT DeserializeAndPaste();
    HRESULT RegisterFormat();
    void    ReleaseClipboardData();

    // Noncopyable
    ClipboardManager(const ClipboardManager&);
    ClipboardManager& operator=(const ClipboardManager&);
};

#endif // SUNDANCE_CLIPBOARD_MANAGER_H
