/*
 * SundanceAppMain.cpp
 *
 * Implementation of the central Sundance application class.
 * Coordinates project management, timeline operations, media import/export,
 * undo/redo, thumbnail generation, video encoding/publishing, and UI
 * coordination via DirectUI.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "SundanceAppMain.h"
#include "CommandLineParser.h"
#include "AutoSaveManager.h"
#include "ClipboardManager.h"
#include "MediaBrowser.h"

// ============================================================================
// Local helper classes referenced by RTTI (defined in the original binary
// as nested classes within the methods that use them).
//
// RTTI names:
//   ?AVInvalidAssetFinder@?1??GetFirstInvalidAssetOnTrack@SundanceAppMain@@AAE_NW4TimelineTrack@@PAI@Z@
//   ?AVMediaItemExtentHarvester@?1??GetMediaItemExtents@SundanceAppMain@@QAEXW4TimelineTrack@@KAAVIntSet@DataStructs@@@Z@
//   ?AVMediaItemExtentFinder@?1??GetMediaItemExtent@SundanceAppMain@@QAEXKAAV?$PtrRef@VMovieExtent@StoryboardManagerNamespace@@@Base@@@Z@
//   ?AVStartEncodeProxyTranscodesIfNecessary@?1??StartEncodeProxyTranscodes@SundanceAppMain@@AAE_NXZ@
//   ?AVResetThumbnail@?1??ResetThumbnails@SundanceAppMain@@QAEXXZ@
//   ?AVRetranscodeRequester@?1??RequestRetranscodeForAllExtents@SundanceAppMain@@QAEXW4TimelineTrack@@@Z@
// ============================================================================

// ============================================================================
// Module-level singleton
// ============================================================================
static SundanceAppMain* g_pSundanceAppMain = NULL;

SundanceAppMain* GetSundanceAppMain()
{
    return g_pSundanceAppMain;
}

// ============================================================================
// Construction / destruction
// ============================================================================
SundanceAppMain::SundanceAppMain()
    : m_bInitialized(false)
    , m_bProjectOpen(false)
    , m_bProjectDirty(false)
    , m_bPublishing(false)
    , m_bEncoding(false)
    , m_hWndMain(NULL)
    , m_hInstance(NULL)
    , m_pProject(NULL)
    , m_pCommandLineParser(NULL)
    , m_pAutoSaveManager(NULL)
    , m_pClipboardManager(NULL)
    , m_pMediaBrowser(NULL)
    , m_pThumbnailCache(NULL)
    , m_pUndoManager(NULL)
    , m_pTimelineController(NULL)
    , m_pProjectManager(NULL)
    , m_pExportController(NULL)
    , m_pImportController(NULL)
    , m_pPlaybackController(NULL)
    , m_pSqmLogger(NULL)
    , m_pTelemetrySession(NULL)
{
    ATLASSERT(g_pSundanceAppMain == NULL);
    g_pSundanceAppMain = this;
}

SundanceAppMain::~SundanceAppMain()
{
    Shutdown();
    g_pSundanceAppMain = NULL;
}

// ============================================================================
// Initialize
// ============================================================================
HRESULT SundanceAppMain::Initialize(HINSTANCE hInstance, int argc, wchar_t** argv)
{
    if (m_bInitialized)
        return E_UNEXPECTED;

    m_hInstance = hInstance;

    HRESULT hr = InitializeSubsystems();
    if (FAILED(hr))
        return hr;

    ProcessCommandLine(argc, argv);

    hr = InitializeUI(hInstance);
    if (FAILED(hr))
        return hr;

    m_bInitialized = true;
    return S_OK;
}

// ============================================================================
// Shutdown
// ============================================================================
void SundanceAppMain::Shutdown()
{
    if (!m_bInitialized)
        return;

    if (m_bProjectOpen)
        CloseProject();

    ReleaseSubsystems();
    m_bInitialized = false;
}

// ============================================================================
// InitializeSubsystems
// ============================================================================
HRESULT SundanceAppMain::InitializeSubsystems()
{
    HRESULT hr;

    m_pCommandLineParser = new (std::nothrow) CommandLineParser();
    if (!m_pCommandLineParser)
        return E_OUTOFMEMORY;

    m_pAutoSaveManager = new (std::nothrow) AutoSaveManager();
    if (!m_pAutoSaveManager)
        return E_OUTOFMEMORY;

    m_pClipboardManager = new (std::nothrow) ClipboardManager();
    if (!m_pClipboardManager)
        return E_OUTOFMEMORY;

    m_pMediaBrowser = new (std::nothrow) MediaBrowser();
    if (!m_pMediaBrowser)
        return E_OUTOFMEMORY;

    m_pThumbnailCache = new (std::nothrow) ThumbnailCache();
    if (!m_pThumbnailCache)
        return E_OUTOFMEMORY;

    m_pUndoManager = new (std::nothrow) UndoManager();
    if (!m_pUndoManager)
        return E_OUTOFMEMORY;

    m_pTimelineController = new (std::nothrow) TimelineController();
    if (!m_pTimelineController)
        return E_OUTOFMEMORY;

    m_pProjectManager = new (std::nothrow) ProjectManager();
    if (!m_pProjectManager)
        return E_OUTOFMEMORY;

    m_pExportController = new (std::nothrow) ExportController();
    if (!m_pExportController)
        return E_OUTOFMEMORY;

    m_pImportController = new (std::nothrow) ImportController();
    if (!m_pImportController)
        return E_OUTOFMEMORY;

    m_pPlaybackController = new (std::nothrow) PlaybackController();
    if (!m_pPlaybackController)
        return E_OUTOFMEMORY;

    hr = m_pAutoSaveManager->Initialize(this);
    if (FAILED(hr))
        return hr;

    hr = m_pMediaBrowser->Initialize();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ============================================================================
// ReleaseSubsystems
// ============================================================================
void SundanceAppMain::ReleaseSubsystems()
{
    delete m_pPlaybackController;
    m_pPlaybackController = NULL;

    delete m_pImportController;
    m_pImportController = NULL;

    delete m_pExportController;
    m_pExportController = NULL;

    delete m_pProjectManager;
    m_pProjectManager = NULL;

    delete m_pTimelineController;
    m_pTimelineController = NULL;

    delete m_pUndoManager;
    m_pUndoManager = NULL;

    delete m_pThumbnailCache;
    m_pThumbnailCache = NULL;

    delete m_pMediaBrowser;
    m_pMediaBrowser = NULL;

    delete m_pClipboardManager;
    m_pClipboardManager = NULL;

    delete m_pAutoSaveManager;
    m_pAutoSaveManager = NULL;

    delete m_pCommandLineParser;
    m_pCommandLineParser = NULL;
}

// ============================================================================
// InitializeUI
// ============================================================================
HRESULT SundanceAppMain::InitializeUI(HINSTANCE hInstance)
{
    UNREFERENCED_PARAMETER(hInstance);
    return S_OK;
}

// ============================================================================
// ProcessCommandLine
// ============================================================================
void SundanceAppMain::ProcessCommandLine(int argc, wchar_t** argv)
{
    if (m_pCommandLineParser)
        m_pCommandLineParser->Parse(argc, argv);
}

// ============================================================================
// IsInitialized / IsProjectOpen / IsProjectDirty
// ============================================================================
bool SundanceAppMain::IsInitialized() const throw()
{
    return m_bInitialized;
}

bool SundanceAppMain::IsProjectOpen() const throw()
{
    return m_bProjectOpen;
}

bool SundanceAppMain::IsProjectDirty() const throw()
{
    return m_bProjectDirty;
}

// ============================================================================
// GetProject
// ============================================================================
StoryboardManagerNamespace::MovieProject* SundanceAppMain::GetProject() const throw()
{
    return m_pProject;
}

// ============================================================================
// NewProject
// ============================================================================
HRESULT SundanceAppMain::NewProject()
{
    if (m_bProjectOpen)
    {
        HRESULT hr = CloseProject();
        if (FAILED(hr))
            return hr;
    }

    m_pProject = new (std::nothrow) StoryboardManagerNamespace::MovieProject();
    if (!m_pProject)
        return E_OUTOFMEMORY;

    m_bProjectOpen = true;
    m_bProjectDirty = false;

    if (m_pAutoSaveManager)
        m_pAutoSaveManager->OnProjectOpened();

    OnProjectChanged();
    return S_OK;
}

// ============================================================================
// OpenProject
// ============================================================================
HRESULT SundanceAppMain::OpenProject(LPCWSTR pszFilePath)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    if (m_bProjectOpen)
    {
        HRESULT hr = CloseProject();
        if (FAILED(hr))
            return hr;
    }

    m_pProject = new (std::nothrow) StoryboardManagerNamespace::MovieProject();
    if (!m_pProject)
        return E_OUTOFMEMORY;

    HRESULT hr = m_pProject->Load(pszFilePath);
    if (FAILED(hr))
    {
        delete m_pProject;
        m_pProject = NULL;
        return hr;
    }

    m_bProjectOpen = true;
    m_bProjectDirty = false;

    if (m_pAutoSaveManager)
        m_pAutoSaveManager->OnProjectOpened();

    OnProjectChanged();
    return S_OK;
}

// ============================================================================
// SaveProject
// ============================================================================
HRESULT SundanceAppMain::SaveProject()
{
    if (!m_bProjectOpen || !m_pProject)
        return E_UNEXPECTED;

    if (m_pProject->GetFilePath().IsEmpty())
    {
        return E_NOTIMPL;
    }

    HRESULT hr = m_pProject->Save();
    if (SUCCEEDED(hr))
        m_bProjectDirty = false;

    return hr;
}

// ============================================================================
// SaveProjectAs
// ============================================================================
HRESULT SundanceAppMain::SaveProjectAs(LPCWSTR pszFilePath)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    if (!m_bProjectOpen || !m_pProject)
        return E_UNEXPECTED;

    HRESULT hr = m_pProject->SaveAs(pszFilePath);
    if (SUCCEEDED(hr))
        m_bProjectDirty = false;

    return hr;
}

// ============================================================================
// CloseProject
// ============================================================================
HRESULT SundanceAppMain::CloseProject()
{
    if (!m_bProjectOpen)
        return S_OK;

    if (m_pUndoManager)
        m_pUndoManager->Clear();

    if (m_pAutoSaveManager)
        m_pAutoSaveManager->OnProjectClosed();

    if (m_pProject)
    {
        delete m_pProject;
        m_pProject = NULL;
    }

    m_bProjectOpen = false;
    m_bProjectDirty = false;

    OnProjectChanged();
    return S_OK;
}

// ============================================================================
// AddMediaToTimeline
// ============================================================================
HRESULT SundanceAppMain::AddMediaToTimeline(LPCWSTR pszFilePath, TimelineTrack track)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    if (!m_bProjectOpen || !m_pProject)
        return E_UNEXPECTED;

    HRESULT hr = m_pProject->ImportMedia(pszFilePath, track);
    if (SUCCEEDED(hr))
    {
        m_bProjectDirty = true;
        ResetThumbnails();
    }

    return hr;
}

// ============================================================================
// RemoveItemFromTimeline
// ============================================================================
HRESULT SundanceAppMain::RemoveItemFromTimeline(DWORD dwItemId, TimelineTrack track)
{
    if (!m_bProjectOpen || !m_pProject)
        return E_UNEXPECTED;

    HRESULT hr = m_pProject->RemoveItem(dwItemId, track);
    if (SUCCEEDED(hr))
    {
        m_bProjectDirty = true;
        ResetThumbnails();
    }

    return hr;
}

// ============================================================================
// MoveItemOnTimeline
// ============================================================================
HRESULT SundanceAppMain::MoveItemOnTimeline(DWORD dwItemId, TimelineTrack track, DWORD dwNewPosition)
{
    if (!m_bProjectOpen || !m_pProject)
        return E_UNEXPECTED;

    HRESULT hr = m_pProject->MoveItem(dwItemId, track, dwNewPosition);
    if (SUCCEEDED(hr))
    {
        m_bProjectDirty = true;
        ResetThumbnails();
    }

    return hr;
}

// ============================================================================
// GetFirstInvalidAssetOnTrack (private)
//
// Scans the specified track for the first asset that fails validation
// (missing file, corrupt metadata, etc.) and returns its index.
// RTTI: ?AVInvalidAssetFinder@?1??GetFirstInvalidAssetOnTrack@SundanceAppMain@@AAE_NW4TimelineTrack@@PAI@Z@
// ============================================================================
bool SundanceAppMain::GetFirstInvalidAssetOnTrack(TimelineTrack track, UINT* pIndex)
{
    if (!pIndex)
        return false;

    *pIndex = 0;

    if (!m_bProjectOpen || !m_pProject)
        return false;

    DWORD cItems = m_pProject->GetItemCount(track);
    for (DWORD i = 0; i < cItems; ++i)
    {
        if (!m_pProject->IsItemValid(i, track))
        {
            *pIndex = static_cast<UINT>(i);
            return true;
        }
    }

    return false;
}

// ============================================================================
// GetMediaItemExtents (public)
//
// Harvests all extent IDs associated with a given media item on the
// specified track and adds them to the provided IntSet.
// RTTI: ?AVMediaItemExtentHarvester@?1??GetMediaItemExtents@SundanceAppMain@@QAEXW4TimelineTrack@@KAAVIntSet@DataStructs@@@Z@
// ============================================================================
void SundanceAppMain::GetMediaItemExtents(TimelineTrack track, DWORD dwItemId, Base::DataStructs::IntSet& extentSet)
{
    if (!m_bProjectOpen || !m_pProject)
        return;

    Base::Array<DWORD> extentIds;
    m_pProject->GetExtentIdsForMediaItem(dwItemId, track, extentIds);

    for (size_t i = 0; i < extentIds.GetCount(); ++i)
    {
        extentSet.Add(static_cast<int>(extentIds[i]));
    }
}

// ============================================================================
// GetMediaItemExtent (public)
//
// Resolves a single media item ID to its associated MovieExtent object.
// RTTI: ?AVMediaItemExtentFinder@?1??GetMediaItemExtent@SundanceAppMain@@QAEXKAAV?$PtrRef@VMovieExtent@StoryboardManagerNamespace@@@Base@@@Z@
// ============================================================================
void SundanceAppMain::GetMediaItemExtent(DWORD dwItemId, Base::PtrRef<StoryboardManagerNamespace::MovieExtent>& extentOut)
{
    if (!m_bProjectOpen || !m_pProject)
        return;

    m_pProject->GetExtentForMediaItem(dwItemId, extentOut);
}

// ============================================================================
// StartEncodeProxyTranscodes (private)
//
// Checks if any items require proxy transcoding and initiates the process.
// RTTI: ?AVStartEncodeProxyTranscodesIfNecessary@?1??StartEncodeProxyTranscodes@SundanceAppMain@@AAE_NXZ@
// ============================================================================
bool SundanceAppMain::StartEncodeProxyTranscodes()
{
    if (!m_bProjectOpen || !m_pProject)
        return false;

    if (m_bEncoding)
        return false;

    bool bNeedsTranscode = m_pProject->HasItemsNeedingProxyTranscode();
    if (!bNeedsTranscode)
        return false;

    m_bEncoding = true;
    HRESULT hr = m_pProject->StartProxyTranscode();
    if (FAILED(hr))
    {
        m_bEncoding = false;
        return false;
    }

    return true;
}

// ============================================================================
// RequestRetranscodeForAllExtents (public)
//
// Marks all extents on the given track as needing re-transcoding.
// RTTI: ?AVRetranscodeRequester@?1??RequestRetranscodeForAllExtents@SundanceAppMain@@QAEXW4TimelineTrack@@@Z@
// ============================================================================
void SundanceAppMain::RequestRetranscodeForAllExtents(TimelineTrack track)
{
    if (!m_bProjectOpen || !m_pProject)
        return;

    m_pProject->MarkAllExtentsForRetranscode(track);
}

// ============================================================================
// ResetThumbnails (public)
//
// Invalidates all cached thumbnails so they are regenerated on next render.
// RTTI: ?AVResetThumbnail@?1??ResetThumbnails@SundanceAppMain@@QAEXXZ@
// ============================================================================
void SundanceAppMain::ResetThumbnails()
{
    if (m_pThumbnailCache)
        m_pThumbnailCache->InvalidateAll();
}

// ============================================================================
// ImportMediaFiles
// ============================================================================
HRESULT SundanceAppMain::ImportMediaFiles(int cFiles, LPCWSTR* ppszFiles)
{
    if (cFiles <= 0 || !ppszFiles)
        return E_INVALIDARG;

    if (!m_bProjectOpen || !m_pProject)
        return E_UNEXPECTED;

    if (m_pImportController)
    {
        HRESULT hr = m_pImportController->ImportFiles(cFiles, ppszFiles);
        if (SUCCEEDED(hr))
        {
            m_bProjectDirty = true;
            ResetThumbnails();
        }
        return hr;
    }

    return E_NOTIMPL;
}

// ============================================================================
// PublishMovie
// ============================================================================
HRESULT SundanceAppMain::PublishMovie(LPCWSTR pszOutputPath, DWORD dwProfileIndex)
{
    if (!pszOutputPath || !pszOutputPath[0])
        return E_INVALIDARG;

    if (!m_bProjectOpen || !m_pProject)
        return E_UNEXPECTED;

    if (m_bPublishing)
        return E_UNEXPECTED;

    m_bPublishing = true;

    HRESULT hr = m_pExportController
        ? m_pExportController->Publish(pszOutputPath, dwProfileIndex)
        : E_NOTIMPL;

    if (FAILED(hr))
        m_bPublishing = false;

    return hr;
}

// ============================================================================
// PublishMovieToService
// ============================================================================
HRESULT SundanceAppMain::PublishMovieToService(LPCWSTR pszServiceName)
{
    if (!pszServiceName || !pszServiceName[0])
        return E_INVALIDARG;

    if (!m_bProjectOpen || !m_pProject)
        return E_UNEXPECTED;

    if (m_bPublishing)
        return E_UNEXPECTED;

    m_bPublishing = true;

    HRESULT hr = m_pExportController
        ? m_pExportController->PublishToService(pszServiceName)
        : E_NOTIMPL;

    if (FAILED(hr))
        m_bPublishing = false;

    return hr;
}

bool SundanceAppMain::IsPublishing() const throw()
{
    return m_bPublishing;
}

bool SundanceAppMain::IsEncoding() const throw()
{
    return m_bEncoding;
}

// ============================================================================
// Undo / Redo
// ============================================================================
HRESULT SundanceAppMain::Undo()
{
    if (!m_pUndoManager)
        return E_NOTIMPL;

    HRESULT hr = m_pUndoManager->Undo();
    if (SUCCEEDED(hr))
    {
        m_bProjectDirty = true;
        NotifyUIRefresh();
    }

    return hr;
}

HRESULT SundanceAppMain::Redo()
{
    if (!m_pUndoManager)
        return E_NOTIMPL;

    HRESULT hr = m_pUndoManager->Redo();
    if (SUCCEEDED(hr))
    {
        m_bProjectDirty = true;
        NotifyUIRefresh();
    }

    return hr;
}

bool SundanceAppMain::CanUndo() const throw()
{
    return m_pUndoManager ? m_pUndoManager->CanUndo() : false;
}

bool SundanceAppMain::CanRedo() const throw()
{
    return m_pUndoManager ? m_pUndoManager->CanRedo() : false;
}

// ============================================================================
// Clipboard
// ============================================================================
HRESULT SundanceAppMain::CutSelection()
{
    if (!m_pClipboardManager)
        return E_NOTIMPL;

    HRESULT hr = m_pClipboardManager->Cut();
    if (SUCCEEDED(hr))
    {
        m_bProjectDirty = true;
        NotifyUIRefresh();
    }

    return hr;
}

HRESULT SundanceAppMain::CopySelection()
{
    if (!m_pClipboardManager)
        return E_NOTIMPL;

    return m_pClipboardManager->Copy();
}

HRESULT SundanceAppMain::PasteFromClipboard()
{
    if (!m_pClipboardManager)
        return E_NOTIMPL;

    HRESULT hr = m_pClipboardManager->Paste();
    if (SUCCEEDED(hr))
    {
        m_bProjectDirty = true;
        NotifyUIRefresh();
    }

    return hr;
}

bool SundanceAppMain::CanCut() const throw()
{
    return m_pClipboardManager ? m_pClipboardManager->CanCut() : false;
}

bool SundanceAppMain::CanCopy() const throw()
{
    return m_pClipboardManager ? m_pClipboardManager->CanCopy() : false;
}

bool SundanceAppMain::CanPaste() const throw()
{
    return m_pClipboardManager ? m_pClipboardManager->CanPaste() : false;
}

// ============================================================================
// Playback
// ============================================================================
HRESULT SundanceAppMain::StartPlayback()
{
    if (!m_pPlaybackController)
        return E_NOTIMPL;

    return m_pPlaybackController->Play();
}

HRESULT SundanceAppMain::StopPlayback()
{
    if (!m_pPlaybackController)
        return E_NOTIMPL;

    return m_pPlaybackController->Stop();
}

HRESULT SundanceAppMain::PausePlayback()
{
    if (!m_pPlaybackController)
        return E_NOTIMPL;

    return m_pPlaybackController->Pause();
}

bool SundanceAppMain::IsPlaying() const throw()
{
    return m_pPlaybackController ? m_pPlaybackController->IsPlaying() : false;
}

// ============================================================================
// UI coordination
// ============================================================================
void SundanceAppMain::OnMainWindowCreated(HWND hWnd)
{
    m_hWndMain = hWnd;

    if (m_pMediaBrowser)
        m_pMediaBrowser->SetOwnerWindow(hWnd);
}

void SundanceAppMain::OnMainWindowDestroyed()
{
    m_hWndMain = NULL;
}

HWND SundanceAppMain::GetMainWindow() const throw()
{
    return m_hWndMain;
}

void SundanceAppMain::UpdateCommandState()
{
    if (m_hWndMain)
        ::SendMessage(m_hWndMain, WM_COMMAND, MAKEWPARAM(0, 0), 0);
}

void SundanceAppMain::NotifyUIRefresh()
{
    if (m_hWndMain)
        ::InvalidateRect(m_hWndMain, NULL, FALSE);
}

// ============================================================================
// Subsystem accessors
// ============================================================================
CommandLineParser* SundanceAppMain::GetCommandLineParser() const throw()
{
    return m_pCommandLineParser;
}

AutoSaveManager* SundanceAppMain::GetAutoSaveManager() const throw()
{
    return m_pAutoSaveManager;
}

ClipboardManager* SundanceAppMain::GetClipboardManager() const throw()
{
    return m_pClipboardManager;
}

MediaBrowser* SundanceAppMain::GetMediaBrowser() const throw()
{
    return m_pMediaBrowser;
}

ThumbnailCache* SundanceAppMain::GetThumbnailCache() const throw()
{
    return m_pThumbnailCache;
}

UndoManager* SundanceAppMain::GetUndoManager() const throw()
{
    return m_pUndoManager;
}

TimelineController* SundanceAppMain::GetTimelineController() const throw()
{
    return m_pTimelineController;
}

ProjectManager* SundanceAppMain::GetProjectManager() const throw()
{
    return m_pProjectManager;
}

ExportController* SundanceAppMain::GetExportController() const throw()
{
    return m_pExportController;
}

ImportController* SundanceAppMain::GetImportController() const throw()
{
    return m_pImportController;
}

// ============================================================================
// ShowApplicationOptionsDialog
// ============================================================================
void SundanceAppMain::ShowApplicationOptionsDialog(HWND hWndParent)
{
    UNREFERENCED_PARAMETER(hWndParent);
}

// ============================================================================
// DontShow prompt management
// ============================================================================
bool SundanceAppMain::ShouldShowPrompt(LPCWSTR pszPromptKey) const
{
    if (!pszPromptKey || !pszPromptKey[0])
        return true;

    auto it = m_dontShowPrompts.find(ATL::CString(pszPromptKey));
    if (it != m_dontShowPrompts.end())
        return !it->second;

    return true;
}

void SundanceAppMain::SetDontShowPrompt(LPCWSTR pszPromptKey, bool bDontShow)
{
    if (!pszPromptKey || !pszPromptKey[0])
        return;

    m_dontShowPrompts[ATL::CString(pszPromptKey)] = bDontShow;
}

// ============================================================================
// OnProjectChanged
// ============================================================================
void SundanceAppMain::OnProjectChanged()
{
    NotifyUIRefresh();
}

// ============================================================================
// OnProjectDirtyStateChanged
// ============================================================================
void SundanceAppMain::OnProjectDirtyStateChanged(bool bDirty)
{
    m_bProjectDirty = bDirty;
}
