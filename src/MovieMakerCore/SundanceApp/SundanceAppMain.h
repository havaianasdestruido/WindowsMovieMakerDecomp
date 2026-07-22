/*
 * SundanceAppMain.h
 *
 * Central application class for the Sundance framework (MovieMakerCore.dll).
 * Coordinates project management, timeline operations, media import/export,
 * undo/redo, thumbnail generation, video encoding/publishing, and UI
 * coordination via DirectUI.
 *
 * RTTI: ?AVSundanceAppMain@@
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_APP_MAIN_H
#define SUNDANCE_APP_MAIN_H

#include "../pch.h"
#include "../MovieMakerCore.h"

// ============================================================================
// Forward declarations
// ============================================================================
class TimelineController;
class ProjectManager;
class UndoManager;
class ExportController;
class ImportController;
class PlaybackController;
class ThumbnailCache;
class CommandLineParser;
class AutoSaveManager;
class ClipboardManager;
class MediaBrowser;
class SqmLogger;
class TelemetrySession;

namespace SundanceUI
{
    class RibbonApp;
}

namespace StoryboardManager {
    class MovieProject;
    class MovieExtent;
    class MediaItem;
}
namespace StoryboardManagerNamespace = StoryboardManager;

namespace SundanceErrorReporting {
    class ExceptionWithString;
}

class SundanceAppDataContext;

// ============================================================================
// TimelineTrack enum (RTTI: W4TimelineTrack)
// ============================================================================
enum TimelineTrack
{
    TimelineTrack_Video         = 0,
    TimelineTrack_Audio         = 1,
    TimelineTrack_Narration     = 2,
    TimelineTrack_Music         = 3,
    TimelineTrack_Overlay       = 4,
    TimelineTrack_Unknown       = 0xFF
};

// ============================================================================
// SundanceAppMain
// ============================================================================
// The main application singleton. Owns the project, timeline, encoding,
// import/export, undo/redo, and media subsystems. Created once at startup
// and destroyed at shutdown. DirectUI UI bindings route through the
// behavior factory (SundanceBehaviorFactory) which holds a back-pointer
// to this class.
//
class SundanceAppMain
{
public:
    SundanceAppMain();
    virtual ~SundanceAppMain();

    // -- Lifetime --
    HRESULT Initialize(HINSTANCE hInstance, int argc, wchar_t** argv);
    void Shutdown();
    bool IsInitialized() const throw();

    // -- Project management --
    HRESULT NewProject();
    HRESULT OpenProject(LPCWSTR pszFilePath);
    HRESULT SaveProject();
    HRESULT SaveProjectAs(LPCWSTR pszFilePath);
    HRESULT CloseProject();
    bool    IsProjectDirty() const throw();
    bool    IsProjectOpen() const throw();
    StoryboardManagerNamespace::MovieProject* GetProject() const throw();

    // -- Timeline operations --
    HRESULT AddMediaToTimeline(LPCWSTR pszFilePath, TimelineTrack track);
    HRESULT RemoveItemFromTimeline(DWORD dwItemId, TimelineTrack track);
    HRESULT MoveItemOnTimeline(DWORD dwItemId, TimelineTrack track, DWORD dwNewPosition);
    void    ResetThumbnails();

    // -- Asset validation --
    bool GetFirstInvalidAssetOnTrack(TimelineTrack track, UINT* pIndex);
    void GetMediaItemExtents(TimelineTrack track, DWORD dwItemId, Base::DataStructs::IntSet& extentSet);
    void GetMediaItemExtent(DWORD dwItemId, Base::PtrRef<StoryboardManagerNamespace::MovieExtent>& extentOut);

    // -- Encoding / proxy transcoding --
    bool StartEncodeProxyTranscodes();
    void RequestRetranscodeForAllExtents(TimelineTrack track);

    // -- Import / Export --
    HRESULT ImportMediaFiles(int cFiles, LPCWSTR* ppszFiles);
    HRESULT PublishMovie(LPCWSTR pszOutputPath, DWORD dwProfileIndex);
    HRESULT PublishMovieToService(LPCWSTR pszServiceName);
    bool    IsPublishing() const throw();
    bool    IsEncoding() const throw();

    // -- Undo / Redo --
    HRESULT Undo();
    HRESULT Redo();
    bool    CanUndo() const throw();
    bool    CanRedo() const throw();

    // -- Clipboard --
    HRESULT CutSelection();
    HRESULT CopySelection();
    HRESULT PasteFromClipboard();
    bool    CanCut() const throw();
    bool    CanCopy() const throw();
    bool    CanPaste() const throw();

    // -- Playback --
    HRESULT StartPlayback();
    HRESULT StopPlayback();
    HRESULT PausePlayback();
    bool    IsPlaying() const throw();

    // -- UI coordination --
    void    OnMainWindowCreated(HWND hWnd);
    void    OnMainWindowDestroyed();
    HWND    GetMainWindow() const throw();
    void    UpdateCommandState();
    void    NotifyUIRefresh();
    void    ClearSelection();

    // -- Ribbon integration --
    HRESULT InitializeRibbon(HINSTANCE hInstance, HWND hWnd);
    void    ShutdownRibbon();
    HRESULT OnRibbonCommand(UINT nCmdId);
    SundanceUI::RibbonApp* GetRibbonApp() throw();

    // -- Subsystem access --
    CommandLineParser*  GetCommandLineParser() const throw();
    AutoSaveManager*    GetAutoSaveManager() const throw();
    ClipboardManager*   GetClipboardManager() const throw();
    MediaBrowser*       GetMediaBrowser() const throw();
    ThumbnailCache*     GetThumbnailCache() const throw();
    UndoManager*        GetUndoManager() const throw();
    TimelineController* GetTimelineController() const throw();
    ProjectManager*     GetProjectManager() const throw();
    ExportController*   GetExportController() const throw();
    ImportController*   GetImportController() const throw();

    // -- Application options --
    void ShowApplicationOptionsDialog(HWND hWndParent);

    // -- DontShow prompt management --
    bool ShouldShowPrompt(LPCWSTR pszPromptKey) const;
    void SetDontShowPrompt(LPCWSTR pszPromptKey, bool bDontShow);

protected:
    // -- Internal initialization helpers --
    HRESULT InitializeSubsystems();
    HRESULT InitializeUI(HINSTANCE hInstance);
    void    ReleaseSubsystems();

    // -- Command-line handling --
    void ProcessCommandLine(int argc, wchar_t** argv);

    // -- Project change notification --
    void OnProjectChanged();
    void OnProjectDirtyStateChanged(bool bDirty);

    // -- Recent files --
    HRESULT LoadRecentFiles();
    void    SaveRecentFiles();
    void    AddToRecentFiles(LPCWSTR pszFilePath);

    // -- User preferences (registry) --
    HRESULT LoadUserPreferences();
    void    SaveUserPreferences();

    // -- Single-instance mutex --
    HRESULT AcquireSingleInstanceMutex();
    void    ReleaseSingleInstanceMutex();

    // -- File association --
    HRESULT RegisterFileAssociations();

    // -- SQM / Telemetry (stubbed — no data sent) --
    HRESULT InitializeSqmSession();
    void    ShutdownSqmSession();
    void    ReportTelemetryEvent(LPCWSTR pszEvent);

    // -- Add-in / Plugin loading --
    HRESULT LoadAddIns();

    // -- Error reporting --
    void    ReportError(HRESULT hr, LPCWSTR pszContext);

    // -- Help system --
    void    ShowHelp();

private:
    // State flags
    bool                    m_bInitialized;
    bool                    m_bProjectOpen;
    bool                    m_bProjectDirty;
    bool                    m_bPublishing;
    bool                    m_bEncoding;

    // Main window
    HWND                    m_hWndMain;
    HINSTANCE               m_hInstance;

    // Ribbon
    SundanceUI::RibbonApp*  m_pRibbonApp;

    // Core project
    StoryboardManagerNamespace::MovieProject* m_pProject;

    // Subsystem objects (owned)
    CommandLineParser*      m_pCommandLineParser;
    AutoSaveManager*        m_pAutoSaveManager;
    ClipboardManager*       m_pClipboardManager;
    MediaBrowser*           m_pMediaBrowser;
    ThumbnailCache*         m_pThumbnailCache;
    UndoManager*            m_pUndoManager;
    TimelineController*     m_pTimelineController;
    ProjectManager*         m_pProjectManager;
    ExportController*       m_pExportController;
    ImportController*       m_pImportController;
    PlaybackController*     m_pPlaybackController;
    SqmLogger*              m_pSqmLogger;
    TelemetrySession*       m_pTelemetrySession;

    // DontShow prompt map
    std::map<ATL::CString, bool> m_dontShowPrompts;

    // Data context for DirectUI binding
    SundanceAppDataContext* m_pDataContext;

    // Single-instance mutex
    HANDLE              m_hSingleInstanceMutex;

    // Recent files list
    std::vector<ATL::CString> m_recentFiles;

    // Prevent copy
    SundanceAppMain(const SundanceAppMain&);
    SundanceAppMain& operator=(const SundanceAppMain&);
};

// ============================================================================
// Singleton access
// ============================================================================
SundanceAppMain* GetSundanceAppMain();

#endif // SUNDANCE_APP_MAIN_H
