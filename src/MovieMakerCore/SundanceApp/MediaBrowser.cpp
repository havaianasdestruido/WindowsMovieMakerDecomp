/*
 * MediaBrowser.cpp
 *
 * Implementation of the media browser panel for Sundance.
 * Manages media collection scanning, thumbnails, filtering,
 * search, and drag-and-drop to the timeline.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "MediaBrowser.h"
#include "SundanceAppMain.h"

// ============================================================================
// TODO(reconstruction): Provide a compatible IDataObject implementation for media drag/drop once
// the original format set is recovered; the modern SDK has no CreateDataObject helper.
// ============================================================================
static HRESULT CreateDataObject(
    const FORMATETC* pFormatEtc,
    const STGMEDIUM* pStgMedium,
    DWORD cFormats,
    IDataObject** ppDataObject)
{
    if (!ppDataObject)
        return E_POINTER;
    *ppDataObject = NULL;
    return E_NOTIMPL;
}

// ============================================================================
// Supported file extensions (from disassembly string analysis)
// ============================================================================
namespace
{
    const WCHAR* const kSupportedPhotoExtensions[] =
    {
        L".jpg", L".jpeg", L".png", L".bmp", L".gif",
        L".tiff", L".tif", L".ico", L".wdp", L".hdp",
        L".jxr", L".raw", L".cr2", L".nef", L".dng",
        NULL
    };

    const WCHAR* const kSupportedVideoExtensions[] =
    {
        L".wmv", L".avi", L".mp4", L".mov", L".mpg",
        L".mpeg", L".m4v", L".3gp", L".3g2", L".asf",
        L".ts", L".mts", L".m2ts", L".vob", L".flv",
        L".mkv", L".webm",
        NULL
    };

    const WCHAR* const kSupportedAudioExtensions[] =
    {
        L".mp3", L".wma", L".wav", L".aac", L".m4a",
        L".flac", L".ogg", L".opus", L".aiff", L".ac3",
        NULL
    };

    bool EndsWithExtension(LPCWSTR pszFilePath, const WCHAR* const* ppszExtensions)
    {
        if (!pszFilePath)
            return false;

        LPCWSTR pszDot = wcsrchr(pszFilePath, L'.');
        if (!pszDot)
            return false;

        for (const WCHAR* const* ppszExt = ppszExtensions; *ppszExt; ++ppszExt)
        {
            if (_wcsicmp(pszDot, *ppszExt) == 0)
                return true;
        }

        return false;
    }
}

// ============================================================================
// Construction / destruction
// ============================================================================
MediaBrowser::MediaBrowser()
    : m_hWndOwner(NULL)
    , m_bDragging(false)
    , m_dwDragIndex(0)
    , m_strFilter(L"all")
{
}

MediaBrowser::~MediaBrowser()
{
    Shutdown();
}

// ============================================================================
// Initialize
// ============================================================================
HRESULT MediaBrowser::Initialize()
{
    return S_OK;
}

// ============================================================================
// Shutdown
// ============================================================================
void MediaBrowser::Shutdown()
{
    CancelPendingThumbnails();
    ReleaseThumbnails();
    m_items.clear();
    m_filteredItems.clear();
    m_mediaDirectories.clear();
}

// ============================================================================
// SetOwnerWindow
// ============================================================================
void MediaBrowser::SetOwnerWindow(HWND hWnd)
{
    m_hWndOwner = hWnd;
}

// ============================================================================
// AddMediaDirectory / RemoveMediaDirectory
// ============================================================================
HRESULT MediaBrowser::AddMediaDirectory(LPCWSTR pszDirectory)
{
    if (!pszDirectory || !pszDirectory[0])
        return E_INVALIDARG;

    // Check for duplicates
    for (size_t i = 0; i < m_mediaDirectories.size(); ++i)
    {
        if (_wcsicmp(m_mediaDirectories[i], pszDirectory) == 0)
            return S_FALSE; // Already added
    }

    m_mediaDirectories.push_back(ATL::CString(pszDirectory));
    return S_OK;
}

HRESULT MediaBrowser::RemoveMediaDirectory(LPCWSTR pszDirectory)
{
    if (!pszDirectory || !pszDirectory[0])
        return E_INVALIDARG;

    for (auto it = m_mediaDirectories.begin(); it != m_mediaDirectories.end(); ++it)
    {
        if (_wcsicmp(*it, pszDirectory) == 0)
        {
            m_mediaDirectories.erase(it);
            return S_OK;
        }
    }

    return S_FALSE;
}

// ============================================================================
// ScanDirectories
// ============================================================================
HRESULT MediaBrowser::ScanDirectories()
{
    m_items.clear();
    m_filteredItems.clear();
    ReleaseThumbnails();

    for (size_t d = 0; d < m_mediaDirectories.size(); ++d)
    {
        ScanDirectory(m_mediaDirectories[d]);
    }

    RebuildFilteredList();
    return S_OK;
}

// ============================================================================
// ScanDirectory (internal)
// ============================================================================
void MediaBrowser::ScanDirectory(LPCWSTR pszDirectory)
{
    if (!pszDirectory || !pszDirectory[0])
        return;

    ATL::CString strPattern;
    strPattern.Format(L"%s\\*.*", pszDirectory);

    WIN32_FIND_DATAW fd = { 0 };
    HANDLE hFind = ::FindFirstFileW(strPattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (fd.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
            continue;

        ATL::CString strFilePath;
        strFilePath.Format(L"%s\\%s", pszDirectory, fd.cFileName);

        if (!IsSupportedMediaFile(strFilePath))
            continue;

        MediaBrowserItem item;
        item.strFilePath = strFilePath;
        item.strDisplayName = fd.cFileName;
        item.cbFileSize = (static_cast<ULONGLONG>(fd.nFileSizeHigh) << 32) | fd.nFileSizeLow;
        item.ftDateModified = fd.ftLastWriteTime;

        // Determine content type
        if (EndsWithExtension(strFilePath, kSupportedPhotoExtensions))
            item.strContentType = L"photo";
        else if (EndsWithExtension(strFilePath, kSupportedVideoExtensions))
            item.strContentType = L"video";
        else if (EndsWithExtension(strFilePath, kSupportedAudioExtensions))
            item.strContentType = L"audio";

        m_items.push_back(item);

    } while (::FindNextFileW(hFind, &fd));

    ::FindClose(hFind);
}

// ============================================================================
// IsSupportedMediaFile (internal)
// ============================================================================
bool MediaBrowser::IsSupportedMediaFile(LPCWSTR pszFilePath) const
{
    return EndsWithExtension(pszFilePath, kSupportedPhotoExtensions) ||
           EndsWithExtension(pszFilePath, kSupportedVideoExtensions) ||
           EndsWithExtension(pszFilePath, kSupportedAudioExtensions);
}

// ============================================================================
// GetItem / GetItemCount
// ============================================================================
DWORD MediaBrowser::GetItemCount() const throw()
{
    return static_cast<DWORD>(m_items.size());
}

const MediaBrowserItem* MediaBrowser::GetItem(DWORD dwIndex) const throw()
{
    if (dwIndex >= m_items.size())
        return NULL;

    return &m_items[dwIndex];
}

// ============================================================================
// Filtering
// ============================================================================
void MediaBrowser::SetFilter(LPCWSTR pszContentType)
{
    if (!pszContentType)
        m_strFilter = L"all";
    else
        m_strFilter = pszContentType;

    RebuildFilteredList();
}

LPCWSTR MediaBrowser::GetFilter() const throw()
{
    return m_strFilter;
}

DWORD MediaBrowser::GetFilteredItemCount() const throw()
{
    return static_cast<DWORD>(m_filteredItems.size());
}

const MediaBrowserItem* MediaBrowser::GetFilteredItem(DWORD dwIndex) const throw()
{
    if (dwIndex >= m_filteredItems.size())
        return NULL;

    return m_filteredItems[dwIndex];
}

// ============================================================================
// MatchesFilter (internal)
// ============================================================================
bool MediaBrowser::MatchesFilter(const MediaBrowserItem& item) const
{
    if (m_strFilter.CompareNoCase(L"all") == 0)
        return true;

    return item.strContentType.CompareNoCase(m_strFilter) == 0;
}

// ============================================================================
// RebuildFilteredList (internal)
// ============================================================================
void MediaBrowser::RebuildFilteredList()
{
    m_filteredItems.clear();

    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (MatchesFilter(m_items[i]))
        {
            if (m_strSearchQuery.IsEmpty() || MatchesSearch(m_items[i], m_strSearchQuery))
            {
                m_filteredItems.push_back(&m_items[i]);
            }
        }
    }
}

// ============================================================================
// Search
// ============================================================================
HRESULT MediaBrowser::Search(LPCWSTR pszQuery)
{
    if (!pszQuery)
        m_strSearchQuery.Empty();
    else
        m_strSearchQuery = pszQuery;

    RebuildFilteredList();
    return S_OK;
}

void MediaBrowser::ClearSearch()
{
    m_strSearchQuery.Empty();
    RebuildFilteredList();
}

// ============================================================================
// MatchesSearch (internal)
// ============================================================================
bool MediaBrowser::MatchesSearch(const MediaBrowserItem& item, LPCWSTR pszQuery) const
{
    if (!pszQuery || !pszQuery[0])
        return true;

    // Simple substring match on display name
    return item.strDisplayName.Find(pszQuery) >= 0;
}

// ============================================================================
// Thumbnail management
// ============================================================================
HRESULT MediaBrowser::LoadThumbnail(DWORD dwIndex)
{
    if (dwIndex >= m_items.size())
        return E_INVALIDARG;

    MediaBrowserItem& item = m_items[dwIndex];

    if (item.iThumbnailIndex >= 0)
        return S_OK;

    SHFILEINFOW sfi = {};
    DWORD_PTR dwResult = SHGetFileInfoW(
        item.strFilePath,
        FILE_ATTRIBUTE_NORMAL,
        &sfi,
        sizeof(sfi),
        SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

    if (dwResult && sfi.hIcon)
    {
        ICONINFO iconInfo = {};
        if (GetIconInfo(sfi.hIcon, &iconInfo))
        {
            HBITMAP hBmp = iconInfo.hbmColor ? iconInfo.hbmColor : iconInfo.hbmMask;
            if (hBmp)
            {
                int nIdx = static_cast<int>(m_thumbnails.size());
                m_thumbnails.push_back(hBmp);
                item.iThumbnailIndex = nIdx;
            }
            if (iconInfo.hbmColor) DeleteObject(iconInfo.hbmColor);
            if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
        }
        DestroyIcon(sfi.hIcon);
    }

    if (item.iThumbnailIndex < 0)
    {
        item.iThumbnailIndex = -2;
    }

    return S_OK;
}

HRESULT MediaBrowser::LoadAllThumbnails()
{
    for (DWORD i = 0; i < static_cast<DWORD>(m_items.size()); ++i)
    {
        if (m_items[i].iThumbnailIndex < 0)
            LoadThumbnail(i);
    }
    return S_OK;
}

HBITMAP MediaBrowser::GetThumbnail(DWORD dwIndex) const throw()
{
    if (dwIndex >= m_items.size())
        return NULL;

    int idx = m_items[dwIndex].iThumbnailIndex;
    if (idx < 0 || idx >= static_cast<int>(m_thumbnails.size()))
        return NULL;

    return m_thumbnails[idx];
}

void MediaBrowser::CancelPendingThumbnails()
{
    // No-op: thumbnail loading is synchronous in this implementation
}

void MediaBrowser::ReleaseThumbnails()
{
    for (size_t i = 0; i < m_thumbnails.size(); ++i)
    {
        if (m_thumbnails[i])
            ::DeleteObject(m_thumbnails[i]);
    }
    m_thumbnails.clear();
}

// ============================================================================
// Selection
// ============================================================================
void MediaBrowser::SelectItem(DWORD dwIndex, bool bSelect)
{
    if (dwIndex < m_items.size())
        m_items[dwIndex].bIsSelected = bSelect;
}

void MediaBrowser::SelectAll()
{
    for (size_t i = 0; i < m_items.size(); ++i)
        m_items[i].bIsSelected = true;
}

void MediaBrowser::DeselectAll()
{
    for (size_t i = 0; i < m_items.size(); ++i)
        m_items[i].bIsSelected = false;
}

DWORD MediaBrowser::GetSelectedCount() const throw()
{
    DWORD cSelected = 0;
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i].bIsSelected)
            ++cSelected;
    }
    return cSelected;
}

HRESULT MediaBrowser::GetSelectedFilePaths(std::vector<ATL::CString>& paths) const
{
    paths.clear();
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i].bIsSelected)
            paths.push_back(m_items[i].strFilePath);
    }
    return S_OK;
}

// ============================================================================
// Drag and drop
// ============================================================================
HRESULT MediaBrowser::BeginDrag(DWORD dwIndex)
{
    if (dwIndex >= m_items.size())
        return E_INVALIDARG;

    m_bDragging = true;
    m_dwDragIndex = dwIndex;

    IDataObject* pDataObject = NULL;
    IDropSource* pDropSource = NULL;
    DWORD dwEffect = DROPEFFECT_COPY;

    STGMEDIUM stgMedium = {};
    stgMedium.tymed = TYMED_HGLOBAL;
    stgMedium.hGlobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(CF_HDROP) + (m_items[dwIndex].strFilePath.GetLength() + 2) * sizeof(WCHAR));
    if (!stgMedium.hGlobal)
    {
        m_bDragging = false;
        return E_OUTOFMEMORY;
    }

    DROPFILES* pDropFiles = (DROPFILES*)GlobalLock(stgMedium.hGlobal);
    pDropFiles->pFiles = sizeof(DROPFILES);
    pDropFiles->fWide = TRUE;
    WCHAR* pFiles = (WCHAR*)((BYTE*)pDropFiles + sizeof(DROPFILES));
    wcscpy_s(pFiles, m_items[dwIndex].strFilePath.GetLength() + 1, m_items[dwIndex].strFilePath);
    pFiles[m_items[dwIndex].strFilePath.GetLength() + 1] = L'\0';
    GlobalUnlock(stgMedium.hGlobal);

    FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

    HRESULT hr = CreateDataObject(&fmtetc, &stgMedium, 1, &pDataObject);
    if (SUCCEEDED(hr))
    {
        hr = DoDragDrop(pDataObject, NULL, dwEffect, &dwEffect);
        pDataObject->Release();
    }

    m_bDragging = false;
    m_dwDragIndex = 0;
    return hr;
}

bool MediaBrowser::IsDragging() const throw()
{
    return m_bDragging;
}

// ============================================================================
// ImportSelectedToTimeline
// ============================================================================
HRESULT MediaBrowser::ImportSelectedToTimeline()
{
    SundanceAppMain* pApp = GetSundanceAppMain();
    if (!pApp)
        return E_UNEXPECTED;

    std::vector<ATL::CString> selectedPaths;
    GetSelectedFilePaths(selectedPaths);

    if (selectedPaths.empty())
        return S_FALSE;

    // Build array of LPCWSTR for the import API
    std::vector<LPCWSTR> arrPaths;
    arrPaths.reserve(selectedPaths.size());
    for (size_t i = 0; i < selectedPaths.size(); ++i)
        arrPaths.push_back(selectedPaths[i]);

    return pApp->ImportMediaFiles(
        static_cast<int>(arrPaths.size()),
        arrPaths.data());
}
