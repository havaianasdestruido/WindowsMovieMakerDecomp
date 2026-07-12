/*
 * MediaBrowser.h
 *
 * Media browser panel for the Sundance application framework. Provides
 * the collection/media pane that displays available photos, videos, and
 * audio files for drag-and-drop import into the timeline.
 *
 * RTTI: ?AVMediaBrowser@@
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_MEDIA_BROWSER_H
#define SUNDANCE_MEDIA_BROWSER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class SundanceAppMain;

// ============================================================================
// MediaBrowserItem
// ============================================================================
// Represents a single browsable media item in the collection pane.
// Holds metadata, thumbnail, and file path for a media file that
// has been scanned from the user's media directories.
//
struct MediaBrowserItem
{
    ATL::CString    strFilePath;
    ATL::CString    strDisplayName;
    ATL::CString    strContentType;     // "photo", "video", "audio"
    ULONGLONG       cbFileSize;
    FILETIME        ftDateModified;
    int             iThumbnailIndex;    // Index into thumbnail cache; -1 = not loaded
    DWORD           dwDurationMs;       // Duration for audio/video; 0 for photos
    bool            bIsSelected;

    MediaBrowserItem()
        : cbFileSize(0)
        , iThumbnailIndex(-1)
        , dwDurationMs(0)
        , bIsSelected(false)
    {
        ZeroMemory(&ftDateModified, sizeof(ftDateModified));
    }
};

// ============================================================================
// MediaBrowser
// ============================================================================
// Manages the media collection pane. Scans designated media directories,
// builds a list of browsable items, generates thumbnails on demand, and
// supports drag-and-drop to the timeline.
//
class MediaBrowser
{
public:
    MediaBrowser();
    ~MediaBrowser();

    // -- Lifecycle --
    HRESULT Initialize();
    void    Shutdown();
    void    SetOwnerWindow(HWND hWnd);

    // -- Directory scanning --
    HRESULT AddMediaDirectory(LPCWSTR pszDirectory);
    HRESULT RemoveMediaDirectory(LPCWSTR pszDirectory);
    HRESULT ScanDirectories();
    DWORD   GetItemCount() const throw();
    const MediaBrowserItem* GetItem(DWORD dwIndex) const throw();

    // -- Filtering --
    void    SetFilter(LPCWSTR pszContentType);  // "all", "photo", "video", "audio"
    LPCWSTR GetFilter() const throw();
    DWORD   GetFilteredItemCount() const throw();
    const MediaBrowserItem* GetFilteredItem(DWORD dwIndex) const throw();

    // -- Search --
    HRESULT Search(LPCWSTR pszQuery);
    void    ClearSearch();

    // -- Thumbnail management --
    HRESULT LoadThumbnail(DWORD dwIndex);
    HRESULT LoadAllThumbnails();
    HBITMAP GetThumbnail(DWORD dwIndex) const throw();
    void    CancelPendingThumbnails();

    // -- Selection --
    void    SelectItem(DWORD dwIndex, bool bSelect);
    void    SelectAll();
    void    DeselectAll();
    DWORD   GetSelectedCount() const throw();
    HRESULT GetSelectedFilePaths(std::vector<ATL::CString>& paths) const;

    // -- Drag and drop --
    HRESULT BeginDrag(DWORD dwIndex);
    bool    IsDragging() const throw();

    // -- Import from browser --
    HRESULT ImportSelectedToTimeline();

private:
    // Configuration
    static const DWORD kMaxThumbnails = 1000;

    // State
    HWND                                m_hWndOwner;
    bool                                m_bDragging;
    ATL::CString                        m_strFilter;
    ATL::CString                        m_strSearchQuery;
    std::vector<ATL::CString>           m_mediaDirectories;
    std::vector<MediaBrowserItem>       m_items;
    std::vector<MediaBrowserItem*>      m_filteredItems;
    std::vector<HBITMAP>                m_thumbnails;

    // Internal helpers
    void    RebuildFilteredList();
    void    ReleaseThumbnails();
    void    ScanDirectory(LPCWSTR pszDirectory);
    bool    IsSupportedMediaFile(LPCWSTR pszFilePath) const;
    bool    MatchesFilter(const MediaBrowserItem& item) const;
    bool    MatchesSearch(const MediaBrowserItem& item, LPCWSTR pszQuery) const;

    // Noncopyable
    MediaBrowser(const MediaBrowser&);
    MediaBrowser& operator=(const MediaBrowser&);
};

#endif // SUNDANCE_MEDIA_BROWSER_H
