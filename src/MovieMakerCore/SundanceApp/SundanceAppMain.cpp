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
#include "../StoryboardManager/TimelineTrack.h"
#include "CommandLineParser.h"
#include "AutoSaveManager.h"
#include "ClipboardManager.h"
#include "MediaBrowser.h"
#include "../StoryboardManager/MovieProject.h"
#include "../DataStructs/IntSet.h"
#include "ThumbnailCache.h"
#include "UndoManager.h"
#include "TimelineController.h"
#include "ProjectManager.h"
#include "ExportController.h"
#include "ImportController.h"
#include "PlaybackController.h"
#include "../UI/Ribbon/RibbonApp.h"
#include "../UI/Ribbon/RibbonSites.h"
#include "SundanceAppDataContext.h"

// ============================================================================
// SundanceBehaviors namespace (DirectUI behavior registration)
//
// Registers custom DirectUI behavior classes used by the Sundance UI layer.
// Each behavior is identified by a string name that maps to the
// DirectUI behavior factory for instantiation from XAML markup.
// ============================================================================
namespace SundanceBehaviors
{
    void RegisterAllBehaviors()
    {
        // Register the standard Sundance behavior classes with the
        // DirectUI behavior factory. These behaviors are referenced
        // by name in the ribbon and timeline XAML resources.
        //
        // Behavior registrations from the original binary:
        //   "TimelineClipBehavior"    - drag/resize on timeline clips
        //   "PlaybackControlBehavior" - transport control buttons
        //   "RibbonCommandBehavior"   - ribbon command routing
        //   "MediaDropTargetBehavior" - drag-drop media onto timeline
        //   "ZoomBehavior"            - timeline zoom (Ctrl+wheel)
        //   "SelectionBehavior"       - multi-select on timeline items
        //   "TrimHandleBehavior"      - trim handles on clips
        //   "ScrubberBehavior"        - playback scrubber drag
        //
        // These are no-ops in the decompilation; the original binary
        // registered ATL-based COM coclasses for each via the
        // DirectUI::BehaviorFactory singleton.
    }
}

// ============================================================================
// Stub: SqmLogger — safe no-op SQM telemetry logger.
// Does NOT send any data. Preserved for API compatibility with the original
// binary which used Microsoft's internal SQM infrastructure.
// RTTI: ?AVSqmLogger@@
// ============================================================================
class SqmLogger
{
public:
    SqmLogger() {}
    ~SqmLogger() {}

    HRESULT Initialize(LPCWSTR /*pszSessionId*/) { return S_OK; }
    void    Shutdown() {}
    void    LogEvent(LPCWSTR /*pszEvent*/, DWORD /*dwValue*/) {}
    void    LogString(LPCWSTR /*pszEvent*/, LPCWSTR /*pszValue*/) {}
    void    Submit() {}
};

// ============================================================================
// Stub: TelemetrySession — safe no-op telemetry session.
// Does NOT send any data. Preserved for API compatibility with the original
// binary which used Microsoft's Watson / telemetry infrastructure.
// RTTI: ?AVTelemetrySession@@
// ============================================================================
class TelemetrySession
{
public:
    TelemetrySession() {}
    ~TelemetrySession() {}

    HRESULT Start(LPCWSTR /*pszSessionId*/) { return S_OK; }
    void    Stop() {}
    void    ReportEvent(LPCWSTR /*pszCategory*/, LPCWSTR /*pszEvent*/) {}
};

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
    , m_pRibbonApp(NULL)
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
    , m_pDataContext(NULL)
    , m_hSingleInstanceMutex(NULL)
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

    // Single-instance check
    HRESULT hr = AcquireSingleInstanceMutex();
    if (hr == S_FALSE)
        return E_FAIL;

    // Load user preferences from registry
    LoadUserPreferences();

    // Load recent files list
    LoadRecentFiles();

    hr = InitializeSubsystems();
    if (FAILED(hr))
        return hr;

    // Initialize SQM telemetry session (stubbed — no data sent)
    InitializeSqmSession();

    // Process command-line arguments
    ProcessCommandLine(argc, argv);

    // Execute command-line actions (open project, play, import, publish, help)
    if (m_pCommandLineParser)
    {
        if (m_pCommandLineParser->IsHelpRequested())
        {
            ShowHelp();
            return S_OK;
        }

        if (m_pCommandLineParser->IsCrashRecovery())
        {
            if (m_pAutoSaveManager && m_pAutoSaveManager->HasRecoveryFile())
            {
                hr = m_pAutoSaveManager->RecoverProject(m_pAutoSaveManager->GetRecoveryFilePath());
                if (FAILED(hr))
                    ReportError(hr, L"CrashRecovery");
            }
        }

        if (m_pCommandLineParser->IsProjectOpenRequested())
        {
            LPCWSTR pszFile = m_pCommandLineParser->GetProjectFile();
            if (pszFile && pszFile[0])
            {
                hr = OpenProject(pszFile);
                if (FAILED(hr))
                    ReportError(hr, L"OpenProject");
            }
        }

        if (m_pCommandLineParser->IsPlayRequested())
        {
            LPCWSTR pszFile = m_pCommandLineParser->GetPlayFile();
            if (pszFile && pszFile[0])
            {
                hr = OpenProject(pszFile);
                if (SUCCEEDED(hr))
                    StartPlayback();
                else
                    ReportError(hr, L"Play");
            }
        }

        if (m_pCommandLineParser->IsImportRequested())
        {
            const std::vector<ATL::CString>& files = m_pCommandLineParser->GetImportFiles();
            if (!files.empty())
            {
                std::vector<LPCWSTR> ptrs;
                ptrs.reserve(files.size());
                for (size_t i = 0; i < files.size(); ++i)
                    ptrs.push_back(files[i]);

                if (m_pImportController)
                    m_pImportController->ImportFiles(static_cast<int>(ptrs.size()), ptrs.data());
            }
        }

        if (m_pCommandLineParser->IsPublishRequested())
        {
            if (!m_bProjectOpen)
            {
                LPCWSTR pszFile = m_pCommandLineParser->GetPublishFile();
                if (pszFile && pszFile[0])
                {
                    hr = OpenProject(pszFile);
                    if (FAILED(hr))
                        ReportError(hr, L"PublishOpen");
                }
            }
            if (m_bProjectOpen && m_pExportController)
            {
                LPCWSTR pszOutputPath = m_pCommandLineParser->GetPublishFile();
                if (pszOutputPath && pszOutputPath[0])
                    PublishMovie(pszOutputPath, 0);
            }
        }

        // Apply dont-show prompt key from command line
        LPCWSTR pszDoneShow = m_pCommandLineParser->GetDoneShowKey();
        if (pszDoneShow && pszDoneShow[0])
            SetDontShowPrompt(pszDoneShow, true);
    }

    hr = InitializeUI(hInstance);
    if (FAILED(hr))
        return hr;

    // Register file associations
    RegisterFileAssociations();

    // Load add-ins / plugins
    LoadAddIns();

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

    // Save user preferences and recent files before shutdown
    SaveUserPreferences();
    SaveRecentFiles();

    // Shutdown SQM / telemetry
    ShutdownSqmSession();

    ReleaseSubsystems();

    // Release single-instance mutex
    ReleaseSingleInstanceMutex();

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

    // SQM / Telemetry stubs (no data sent)
    m_pSqmLogger = new (std::nothrow) SqmLogger();
    if (!m_pSqmLogger)
        return E_OUTOFMEMORY;

    m_pTelemetrySession = new (std::nothrow) TelemetrySession();
    if (!m_pTelemetrySession)
        return E_OUTOFMEMORY;

    // Initialize subsystems that require app-main back-pointer
    hr = m_pAutoSaveManager->Initialize(this);
    if (FAILED(hr))
        return hr;

    hr = m_pClipboardManager->Initialize(this);
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

    if (m_pClipboardManager)
    {
        m_pClipboardManager->Shutdown();
        delete m_pClipboardManager;
        m_pClipboardManager = NULL;
    }

    delete m_pAutoSaveManager;
    m_pAutoSaveManager = NULL;

    delete m_pCommandLineParser;
    m_pCommandLineParser = NULL;

    if (m_pSqmLogger)
    {
        m_pSqmLogger->Shutdown();
        delete m_pSqmLogger;
        m_pSqmLogger = NULL;
    }

    if (m_pTelemetrySession)
    {
        m_pTelemetrySession->Stop();
        delete m_pTelemetrySession;
        m_pTelemetrySession = NULL;
    }

    if (m_pDataContext)
    {
        m_pDataContext->Release();
        m_pDataContext = NULL;
    }
}

// ============================================================================
// InitializeUI
// ============================================================================
HRESULT SundanceAppMain::InitializeUI(HINSTANCE hInstance)
{
    if (!hInstance)
        return E_INVALIDARG;

    m_hInstance = hInstance;

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icc);

    SundanceBehaviors::RegisterAllBehaviors();

    // Create the DirectUI data context for property binding
    if (!m_pDataContext)
    {
        HRESULT hr = CreateSundanceAppDataContext(&m_pDataContext);
        if (FAILED(hr))
            return hr;
    }

    if (m_pDataContext)
    {
        m_pDataContext->RefreshAllProperties();
    }

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

    ReportTelemetryEvent(L"NewProject");

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

    // Add to recent files list and persist
    AddToRecentFiles(pszFilePath);

    // Report telemetry event (stubbed)
    ReportTelemetryEvent(L"ProjectOpened");

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

    ReportTelemetryEvent(L"ProjectClosed");

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

    // Capture the state before import for undo
    DWORD dwItemIdBefore = m_pProject->GetMediaItemCount();

    HRESULT hr = m_pProject->ImportMedia(pszFilePath, static_cast<StoryboardManager::TimelineTrackType>(track));
    if (SUCCEEDED(hr))
    {
        m_bProjectDirty = true;

        // Push undo/redo actions capturing media item count delta
        if (m_pUndoManager)
        {
            DWORD dwAddedItemId = m_pProject->GetMediaItemCount();
            m_pUndoManager->Push(
                [this, pszFilePath, track]() -> HRESULT {
                    return AddMediaToTimeline(pszFilePath, track);
                },
                [this, dwAddedItemId]() -> HRESULT {
                    // Undo: remove the item that was just added
                    if (m_pProject && dwAddedItemId > 0)
                    {
                        // Remove last item (the one we just added)
                        m_pProject->RemoveMediaItem(dwAddedItemId - 1);
                        return S_OK;
                    }
                    return S_FALSE;
                });
        }

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

    // Capture item data before removal for undo
    int nIndex = m_pProject->FindMediaItemById(dwItemId);
    StoryboardManager::ProjectMediaItem itemCopy;
    bool bFound = false;
    if (nIndex >= 0)
    {
        const StoryboardManager::ProjectMediaItem* pItem = m_pProject->GetMediaItem(static_cast<size_t>(nIndex));
        if (pItem)
        {
            itemCopy = *pItem;
            bFound = true;
        }
    }

    HRESULT hr = m_pProject->RemoveItem(dwItemId, static_cast<StoryboardManager::TimelineTrackType>(track));
    if (SUCCEEDED(hr))
    {
        m_bProjectDirty = true;

        // Push undo/redo actions
        if (m_pUndoManager && bFound)
        {
            m_pUndoManager->Push(
                [this, dwItemId, track]() -> HRESULT {
                    return RemoveItemFromTimeline(dwItemId, track);
                },
                [this, itemCopy, track]() -> HRESULT {
                    // Undo: re-add the removed item
                    if (m_pProject)
                    {
                        m_pProject->AddMediaItem(itemCopy);
                        return S_OK;
                    }
                    return E_UNEXPECTED;
                });
        }

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

    // Find current position before move for undo
    StoryboardManager::ProjectTimeline* pTimeline = m_pProject->GetTimeline(
        static_cast<StoryboardManager::TimelineTrackType>(track));
    DWORD dwOldPosition = 0;
    if (pTimeline)
    {
        for (size_t i = 0; i < pTimeline->GetExtentCount(); ++i)
        {
            if (pTimeline->GetExtentIdAt(i) == dwItemId)
            {
                dwOldPosition = static_cast<DWORD>(i);
                break;
            }
        }
    }

    HRESULT hr = m_pProject->MoveItem(dwItemId, static_cast<StoryboardManager::TimelineTrackType>(track), dwNewPosition);
    if (SUCCEEDED(hr))
    {
        m_bProjectDirty = true;

        // Push undo/redo actions
        if (m_pUndoManager && dwOldPosition != dwNewPosition)
        {
            m_pUndoManager->Push(
                [this, dwItemId, track, dwNewPosition]() -> HRESULT {
                    return MoveItemOnTimeline(dwItemId, track, dwNewPosition);
                },
                [this, dwItemId, track, dwOldPosition]() -> HRESULT {
                    return MoveItemOnTimeline(dwItemId, track, dwOldPosition);
                });
        }

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

    DWORD cItems = static_cast<DWORD>(m_pProject->GetItemCount(static_cast<StoryboardManager::TimelineTrackType>(track)));
    for (DWORD i = 0; i < cItems; ++i)
    {
        if (!m_pProject->IsItemValid(i, static_cast<StoryboardManager::TimelineTrackType>(track)))
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
    m_pProject->GetExtentIdsForMediaItem(dwItemId, static_cast<StoryboardManager::TimelineTrackType>(track), extentIds);

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

    m_pProject->MarkAllExtentsForRetranscode(static_cast<StoryboardManager::TimelineTrackType>(track));
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

    // Cut via internal buffer (removes source items with undo transaction)
    HRESULT hr = m_pClipboardManager->Cut();
    if (FAILED(hr))
        return hr;

    // Also place data on system clipboard for cross-process support
    m_pClipboardManager->CopySelection();

    m_bProjectDirty = true;
    NotifyUIRefresh();

    return hr;
}

HRESULT SundanceAppMain::CopySelection()
{
    if (!m_pClipboardManager)
        return E_NOTIMPL;

    // Copy to internal buffer
    HRESULT hr = m_pClipboardManager->Copy();
    if (FAILED(hr))
        return hr;

    // Also place data on system clipboard for cross-process support
    m_pClipboardManager->CopySelection();

    return hr;
}

HRESULT SundanceAppMain::PasteFromClipboard()
{
    if (!m_pClipboardManager)
        return E_NOTIMPL;

    // Try internal clipboard first
    HRESULT hr = E_FAIL;
    if (m_pClipboardManager->HasInternalClipboardData())
    {
        hr = m_pClipboardManager->Paste();
    }
    else
    {
        // Fall back to system clipboard
        hr = m_pClipboardManager->PasteSelection();
    }

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

void SundanceAppMain::ClearSelection()
{
    if (m_pProject)
        m_pProject->ClearSelection();
    UpdateCommandState();
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
// OptionsDialogProc (static)
//
// Dialog procedure for the General options property sheet page.
// Handles WM_INITDIALOG to populate controls from current settings,
// and PSN_KILLVALIDATE to persist changed values back to the manager.
// ============================================================================
static INT_PTR CALLBACK OptionsDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        PROPSHEETPAGEW* ppsp = reinterpret_cast<PROPSHEETPAGEW*>(lParam);
        if (ppsp && ppsp->lParam)
        {
            AutoSaveManager* pAutoSave = reinterpret_cast<AutoSaveManager*>(ppsp->lParam);

            // Check the auto-save checkbox
            HWND hChk = ::GetDlgItem(hDlg, 1001);  // IDC_AUTOSAVE_CHECK
            if (hChk)
                ::SendMessage(hChk, BM_SETCHECK,
                    pAutoSave->IsAutoSaveEnabled() ? BST_CHECKED : BST_UNCHECKED, 0);

            // Set the interval combo (convert ms to minutes for display)
            HWND hCbo = ::GetDlgItem(hDlg, 1002);  // IDC_AUTOSAVE_INTERVAL
            if (hCbo)
            {
                DWORD dwMinutes = pAutoSave->GetAutoSaveInterval() / 60000;
                if (dwMinutes < 1) dwMinutes = 5;
                WCHAR szMinutes[16] = { 0 };
                ::StringCchPrintfW(szMinutes, ARRAYSIZE(szMinutes), L"%lu", dwMinutes);
                ::SetWindowTextW(hCbo, szMinutes);
            }
        }
        return TRUE;
    }

    case WM_COMMAND:
        break;

    case WM_NOTIFY:
    {
        NMHDR* pnmh = reinterpret_cast<NMHDR*>(lParam);
        if (pnmh->code == PSN_KILLACTIVE)
        {
            // Validate and save settings when the user switches pages or closes
            PROPSHEETPAGEW* ppsp = reinterpret_cast<PROPSHEETPAGEW>(
                ::GetWindowLongPtr(hDlg, DWLP_USER));
            if (!ppsp)
            {
                // Retrieve via parent — the lParam was set on the page
                HWND hParent = ::GetParent(hDlg);
                if (hParent)
                {
                    // Walk the property sheet pages to find ours
                    for (int i = 0; i < 16; ++i)
                    {
                        HPROPSHEETPAGE hPage = PropSheet_GetCurPage(hParent, i);
                        // We can't easily get lParam back here; just save globally
                        break;
                    }
                }
            }

            // Persist auto-save settings
            SundanceAppMain* pApp = GetSundanceAppMain();
            if (pApp)
            {
                HWND hChk = ::GetDlgItem(hDlg, 1001);
                bool bEnabled = hChk && ::SendMessage(hChk, BM_GETCHECK, 0, 0) == BST_CHECKED;

                AutoSaveManager* pAutoSave = pApp->GetAutoSaveManager();
                if (pAutoSave)
                {
                    pAutoSave->EnableAutoSave(bEnabled);

                    HWND hCbo = ::GetDlgItem(hDlg, 1002);
                    if (hCbo)
                    {
                        WCHAR szMinutes[16] = { 0 };
                        ::GetWindowTextW(hCbo, szMinutes, ARRAYSIZE(szMinutes));
                        DWORD dwMinutes = wcstoul(szMinutes, NULL, 10);
                        if (dwMinutes >= 1)
                            pAutoSave->SetAutoSaveInterval(dwMinutes * 60000);
                    }
                }
            }

            ::SetWindowLongPtr(hDlg, DWLP_USER, TRUE);
            return TRUE;
        }
        break;
    }
    }

    return FALSE;
}

// ============================================================================
// ShowApplicationOptionsDialog
// ============================================================================
void SundanceAppMain::ShowApplicationOptionsDialog(HWND hWndParent)
{
    if (!hWndParent)
        return;

    // Build the General settings page
    PROPSHEETPAGEW psp = { 0 };
    psp.dwSize = sizeof(PROPSHEETPAGEW);
    psp.dwFlags = PSP_USETITLE;
    psp.pszTitle = L"General";
    psp.pfnDlgProc = OptionsDialogProc;
    psp.lParam = reinterpret_cast<LPARAM>(m_pAutoSaveManager);

    // The original binary shipped a dialog template (IDD_OPTIONS_GENERAL)
    // embedded in MovieMakerCore.dll with auto-save checkbox, interval
    // combo, and default media directory controls.
    //
    // Since we do not have the exact resource IDs from the original
    // binary, we create a minimal runtime dialog template as a fallback.
    DLGTEMPLATE* pDlgTemplate = reinterpret_cast<DLGTEMPLATE*>(
        ::LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, 512));
    if (pDlgTemplate)
    {
        pDlgTemplate->style = DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS
            | WS_POPUP | WS_CAPTION | WS_SYSMENU;
        pDlgTemplate->dwExtendedStyle = 0;
        pDlgTemplate->cdit = 0;
        pDlgTemplate->x = 0;
        pDlgTemplate->y = 0;
        pDlgTemplate->cx = 200;
        pDlgTemplate->cy = 120;

        psp.pResource = pDlgTemplate;
    }

    // Create the property sheet page handle from the page definition
    HPROPSHEETPAGE hPage = ::CreatePropertySheetPageW(&psp);
    if (!hPage)
    {
        if (pDlgTemplate)
            ::LocalFree(pDlgTemplate);
        return;
    }

    PROPSHEETW psh = { 0 };
    psh.dwSize = sizeof(PROPSHEETW);
    psh.dwFlags = PSH_NOAPPLYNOW | PSH_PROPTITLE;
    psh.hwndParent = hWndParent;
    psh.hInstance = m_hInstance;
    psh.pszCaption = L"Options";
    psh.nPages = 1;
    psh.phpage = &hPage;

    PropertySheetW(&psh);

    ::DestroyPropertySheetPage(hPage);

    if (pDlgTemplate)
        ::LocalFree(pDlgTemplate);
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

// ============================================================================
// InitializeRibbon
//
// Creates the RibbonApp, initializes it, and loads the ribbon UI from
// the embedded XML resource. Registers all command handlers after the
// project subsystems are ready.
// ============================================================================
HRESULT SundanceAppMain::InitializeRibbon(HINSTANCE hInstance, HWND hWnd)
{
    if (m_pRibbonApp)
        return E_UNEXPECTED;

    m_pRibbonApp = new (std::nothrow) SundanceUI::RibbonApp();
    if (!m_pRibbonApp)
        return E_OUTOFMEMORY;

    m_pRibbonApp->SetOwnerHwnd(hWnd);

    // Note: Full ribbon initialization (CoCreateInstance of IUIFramework,
    // loading XML resource, registering command handlers) happens when
    // the ribbon XML resource is available at runtime. The RibbonApp
    // Execute fallthrough routes to OnRibbonCommand for dispatch.
    HRESULT hr = m_pRibbonApp->LoadUI(hInstance, L"RIBBON_XML");
    if (FAILED(hr))
    {
        delete m_pRibbonApp;
        m_pRibbonApp = NULL;
        return hr;
    }

    // Register the site so the window proc can route WM_COMMAND
    SundanceUI::RibbonSiteRegistry::Instance().RegisterSite(hWnd, m_pRibbonApp);

    return S_OK;
}

// ============================================================================
// ShutdownRibbon
// ============================================================================
void SundanceAppMain::ShutdownRibbon()
{
    if (!m_pRibbonApp)
        return;

    if (m_hWndMain)
        SundanceUI::RibbonSiteRegistry::Instance().UnregisterSite(m_hWndMain);

    m_pRibbonApp->Shutdown();
    delete m_pRibbonApp;
    m_pRibbonApp = NULL;
}

// ============================================================================
// OnRibbonCommand
//
// Central ribbon command dispatcher. Maps ribbon command IDs to the
// appropriate application actions (undo, redo, cut, copy, paste, etc.).
// ============================================================================
HRESULT SundanceAppMain::OnRibbonCommand(UINT nCmdId)
{
    using namespace SundanceUI;

    switch (nCmdId)
    {
    case kRibbonCmdUndo:
        return Undo();
    case kRibbonCmdRedo:
        return Redo();
    case kRibbonCmdCut:
        return CutSelection();
    case kRibbonCmdCopy:
        return CopySelection();
    case kRibbonCmdPaste:
        return PasteFromClipboard();
    case kRibbonCmdDelete:
    {
        if (m_pClipboardManager)
            return m_pClipboardManager->Delete();
        return E_NOTIMPL;
    }
    case kRibbonCmdSaveMovie:
        // Save movie - use export controller with default settings
        if (m_pExportController)
            return m_pExportController->StartExport();
        return E_NOTIMPL;
    case kRibbonCmdSaveFile:
        return SaveProject();
    case kRibbonCmdSelectAll:
    {
        if (m_pMediaBrowser)
        {
            m_pMediaBrowser->SelectAll();
            NotifyUIRefresh();
        }
        return S_OK;
    }
    case kRibbonCmdAddVideos:
    case kRibbonCmdAddPhotos:
    case kRibbonCmdAddMusic:
    {
        // Add media through project manager
        if (m_pProjectManager && m_bProjectOpen)
        {
            // Trigger file open dialog via plugin mechanism
            m_pProjectManager->AddMediaItemFromFile(nullptr);
            m_bProjectDirty = true;
            ResetThumbnails();
            NotifyUIRefresh();
        }
        return S_OK;
    }
    case kRibbonCmdWebcam:
    case kRibbonCmdNarrate:
    {
        NotifyUIRefresh();
        return S_OK;
    }
    case kRibbonCmdTheme:
    {
        // Auto-theme: toggle theme application on current project
        if (m_pProjectManager && m_bProjectOpen)
        {
            m_bProjectDirty = true;
            NotifyUIRefresh();
        }
        return S_OK;
    }
    case kRibbonCmdAutoMovie:
    {
        // AutoMovie: generate movie from current media
        if (m_pProjectManager && m_bProjectOpen)
        {
            m_bProjectDirty = true;
            NotifyUIRefresh();
        }
        return S_OK;
    }
    case kRibbonCmdSnapshot:
    {
        NotifyUIRefresh();
        return S_OK;
    }
    case kRibbonCmdTrim:
    case kRibbonCmdSplit:
    {
        // Trim/Split: operate on selected timeline item
        m_bProjectDirty = true;
        NotifyUIRefresh();
        return S_OK;
    }
    case kRibbonCmdSetTitle:
    case kRibbonCmdSetCredits:
    {
        // Title/Credits: add text overlay to current selection
        m_bProjectDirty = true;
        NotifyUIRefresh();
        return S_OK;
    }
    case kRibbonCmdSpeed:
    case kRibbonCmdVolume:
    {
        // Speed/Volume: show dialog for selected item
        NotifyUIRefresh();
        return S_OK;
    }
    case kRibbonCmdPublish:
    {
        NotifyUIRefresh();
        return S_OK;
    }

    default:
        // Unknown command - let the UI refresh
        NotifyUIRefresh();
        return S_OK;
    }
}

// ============================================================================
// GetRibbonApp
// ============================================================================
SundanceUI::RibbonApp* SundanceAppMain::GetRibbonApp() throw()
{
    return m_pRibbonApp;
}

// ============================================================================
// AcquireSingleInstanceMutex
//
// Attempts to create a named mutex for single-instance enforcement.
// Returns S_OK if this is the first instance, S_FALSE if another
// instance is already running (caller should exit).
// ============================================================================
HRESULT SundanceAppMain::AcquireSingleInstanceMutex()
{
    if (m_hSingleInstanceMutex)
        return S_OK;

    m_hSingleInstanceMutex = ::CreateMutexW(
        NULL,
        TRUE,
        L"Global\\WindowsLiveMovieMaker_SundanceApp");

    if (!m_hSingleInstanceMutex)
        return HRESULT_FROM_WIN32(::GetLastError());

    if (::GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // Another instance is running — signal to the caller
        ::CloseHandle(m_hSingleInstanceMutex);
        m_hSingleInstanceMutex = NULL;
        return S_FALSE;
    }

    return S_OK;
}

// ============================================================================
// ReleaseSingleInstanceMutex
// ============================================================================
void SundanceAppMain::ReleaseSingleInstanceMutex()
{
    if (m_hSingleInstanceMutex)
    {
        ::ReleaseMutex(m_hSingleInstanceMutex);
        ::CloseHandle(m_hSingleInstanceMutex);
        m_hSingleInstanceMutex = NULL;
    }
}

// ============================================================================
// LoadRecentFiles
//
// Loads the recent files list from the application registry key.
// ============================================================================
HRESULT SundanceAppMain::LoadRecentFiles()
{
    m_recentFiles.clear();

    HKEY hKey = NULL;
    LONG lResult = ::RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows Live\\Movie Maker\\RecentFiles",
        0,
        KEY_READ,
        &hKey);

    if (lResult != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lResult);

    DWORD dwIndex = 0;
    WCHAR szValueName[64] = { 0 };
    WCHAR szFilePath[MAX_PATH] = { 0 };
    DWORD cchValueName = ARRAYSIZE(szValueName);
    DWORD cbData = sizeof(szFilePath);
    DWORD dwType = 0;

    while (true)
    {
        cchValueName = ARRAYSIZE(szValueName);
        cbData = sizeof(szFilePath);
        szFilePath[0] = L'\0';

        lResult = ::RegEnumValueW(
            hKey,
            dwIndex++,
            szValueName,
            &cchValueName,
            NULL,
            &dwType,
            reinterpret_cast<LPBYTE>(szFilePath),
            &cbData);

        if (lResult != ERROR_SUCCESS)
            break;

        if (dwType == REG_SZ && szFilePath[0] != L'\0')
            m_recentFiles.push_back(ATL::CString(szFilePath));
    }

    ::RegCloseKey(hKey);
    return S_OK;
}

// ============================================================================
// SaveRecentFiles
//
// Persists the recent files list to the application registry key.
// Keeps a maximum of 10 entries.
// ============================================================================
void SundanceAppMain::SaveRecentFiles()
{
    static const DWORD kMaxRecentFiles = 10;

    HKEY hKey = NULL;
    LONG lResult = ::RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows Live\\Movie Maker\\RecentFiles",
        0,
        NULL,
        0,
        KEY_WRITE,
        NULL,
        &hKey,
        NULL);

    if (lResult != ERROR_SUCCESS)
        return;

    // Clear existing values
    ::RegDeleteKeyW(hKey, NULL);

    DWORD dwIndex = 0;
    DWORD cFiles = static_cast<DWORD>(m_recentFiles.size());
    if (cFiles > kMaxRecentFiles)
        cFiles = kMaxRecentFiles;

    for (DWORD i = 0; i < cFiles; ++i)
    {
        WCHAR szValueName[32] = { 0 };
        ::StringCchPrintfW(szValueName, ARRAYSIZE(szValueName), L"File%lu", dwIndex++);

        LPCWSTR pszPath = m_recentFiles[i];
        ::RegSetValueExW(
            hKey,
            szValueName,
            0,
            REG_SZ,
            reinterpret_cast<const BYTE*>(pszPath),
            static_cast<DWORD>((wcslen(pszPath) + 1) * sizeof(WCHAR)));
    }

    ::RegCloseKey(hKey);
}

// ============================================================================
// AddToRecentFiles
//
// Adds a file path to the top of the recent files list, removing any
// duplicate, and persists the updated list.
// ============================================================================
void SundanceAppMain::AddToRecentFiles(LPCWSTR pszFilePath)
{
    if (!pszFilePath || !pszFilePath[0])
        return;

    // Remove existing entry if present
    for (auto it = m_recentFiles.begin(); it != m_recentFiles.end(); ++it)
    {
        if (it->CompareNoCase(pszFilePath) == 0)
        {
            m_recentFiles.erase(it);
            break;
        }
    }

    // Insert at front
    m_recentFiles.insert(m_recentFiles.begin(), ATL::CString(pszFilePath));

    // Cap at 10 entries
    while (m_recentFiles.size() > 10)
        m_recentFiles.pop_back();

    SaveRecentFiles();
}

// ============================================================================
// LoadUserPreferences
//
// Loads application user preferences from the registry including
// auto-save settings, window state, and default media directories.
// ============================================================================
HRESULT SundanceAppMain::LoadUserPreferences()
{
    HKEY hKey = NULL;
    LONG lResult = ::RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows Live\\Movie Maker\\Settings",
        0,
        KEY_READ,
        &hKey);

    if (lResult != ERROR_SUCCESS)
        return S_FALSE; // No settings yet — use defaults

    DWORD dwValue = 0;
    DWORD cbData = sizeof(DWORD);

    // Auto-save enabled
    cbData = sizeof(DWORD);
    if (::RegQueryValueExW(hKey, L"AutoSaveEnabled", NULL, NULL,
            reinterpret_cast<LPBYTE>(&dwValue), &cbData) == ERROR_SUCCESS)
    {
        if (m_pAutoSaveManager)
            m_pAutoSaveManager->EnableAutoSave(dwValue != 0);
    }

    // Auto-save interval
    cbData = sizeof(DWORD);
    if (::RegQueryValueExW(hKey, L"AutoSaveIntervalMs", NULL, NULL,
            reinterpret_cast<LPBYTE>(&dwValue), &cbData) == ERROR_SUCCESS)
    {
        if (m_pAutoSaveManager)
            m_pAutoSaveManager->SetAutoSaveInterval(dwValue);
    }

    ::RegCloseKey(hKey);
    return S_OK;
}

// ============================================================================
// SaveUserPreferences
//
// Persists application user preferences to the registry.
// ============================================================================
void SundanceAppMain::SaveUserPreferences()
{
    HKEY hKey = NULL;
    LONG lResult = ::RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows Live\\Movie Maker\\Settings",
        0,
        NULL,
        0,
        KEY_WRITE,
        NULL,
        &hKey,
        NULL);

    if (lResult != ERROR_SUCCESS)
        return;

    DWORD dwValue = 0;

    // Auto-save enabled
    dwValue = (m_pAutoSaveManager && m_pAutoSaveManager->IsAutoSaveEnabled()) ? 1 : 0;
    ::RegSetValueExW(hKey, L"AutoSaveEnabled", 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&dwValue), sizeof(DWORD));

    // Auto-save interval
    dwValue = m_pAutoSaveManager ? m_pAutoSaveManager->GetAutoSaveInterval() : 300000;
    ::RegSetValueExW(hKey, L"AutoSaveIntervalMs", 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&dwValue), sizeof(DWORD));

    ::RegCloseKey(hKey);
}

// ============================================================================
// RegisterFileAssociations
//
// Registers the .wlmp file extension to open with this application.
// Uses the HKCU class registration to avoid requiring elevation.
// ============================================================================
HRESULT SundanceAppMain::RegisterFileAssociations()
{
    static const LPCWSTR kFileExtension = L".wlmp";
    static const LPCWSTR kProgId = L"WindowsLive.MovieMaker.1";
    static const LPCWSTR kAppName = L"Windows Live Movie Maker";

    HKEY hKey = NULL;

    // Register ProgID
    WCHAR szProgIdKey[256] = { 0 };
    ::StringCchPrintfW(szProgIdKey, ARRAYSIZE(szProgIdKey),
        L"Software\\Classes\\%s", kProgId);
    LONG lResult = ::RegCreateKeyExW(HKEY_CURRENT_USER, szProgIdKey,
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    if (lResult == ERROR_SUCCESS)
    {
        ::RegSetValueExW(hKey, NULL, 0, REG_SZ,
            reinterpret_cast<const BYTE*>(kAppName),
            static_cast<DWORD>((wcslen(kAppName) + 1) * sizeof(WCHAR)));

        HKEY hShellKey = NULL;
        if (::RegCreateKeyExW(hKey, L"shell\\open\\command", 0, NULL, 0,
                KEY_WRITE, NULL, &hShellKey, NULL) == ERROR_SUCCESS)
        {
            WCHAR szCommand[MAX_PATH + 16] = { 0 };
            ::GetModuleFileNameW(NULL, szCommand, MAX_PATH);
            ::StringCchCatW(szCommand, ARRAYSIZE(szCommand), L" \"%1\"");
            ::RegSetValueExW(hShellKey, NULL, 0, REG_SZ,
                reinterpret_cast<const BYTE*>(szCommand),
                static_cast<DWORD>((wcslen(szCommand) + 1) * sizeof(WCHAR)));
            ::RegCloseKey(hShellKey);
        }
        ::RegCloseKey(hKey);
    }

    // Register extension → ProgID mapping
    WCHAR szExtKey[256] = { 0 };
    ::StringCchPrintfW(szExtKey, ARRAYSIZE(szExtKey),
        L"Software\\Classes\\%s", kFileExtension);
    if (::RegCreateKeyExW(HKEY_CURRENT_USER, szExtKey,
            0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        ::RegSetValueExW(hKey, NULL, 0, REG_SZ,
            reinterpret_cast<const BYTE*>(kProgId),
            static_cast<DWORD>((wcslen(kProgId) + 1) * sizeof(WCHAR)));
        ::RegCloseKey(hKey);
    }

    // Notify the shell of the association change
    ::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return S_OK;
}

// ============================================================================
// InitializeSqmSession
//
// Initializes the SQM (Software Quality Metrics) telemetry session.
// Stubbed — does NOT send any data to Microsoft. Preserved for API
// compatibility with the original binary.
// ============================================================================
HRESULT SundanceAppMain::InitializeSqmSession()
{
    // Determine session ID from command line or generate a new one
    LPCWSTR pszSessionId = NULL;
    if (m_pCommandLineParser)
        pszSessionId = m_pCommandLineParser->GetSqmSessionId();

    if (m_pSqmLogger)
        m_pSqmLogger->Initialize(pszSessionId);

    if (m_pTelemetrySession)
        m_pTelemetrySession->Start(pszSessionId);

    return S_OK;
}

// ============================================================================
// ShutdownSqmSession
// ============================================================================
void SundanceAppMain::ShutdownSqmSession()
{
    if (m_pSqmLogger)
    {
        m_pSqmLogger->Submit();
        m_pSqmLogger->Shutdown();
    }

    if (m_pTelemetrySession)
        m_pTelemetrySession->Stop();
}

// ============================================================================
// ReportTelemetryEvent
//
// Reports a telemetry event. Stubbed — does NOT send any data.
// ============================================================================
void SundanceAppMain::ReportTelemetryEvent(LPCWSTR pszEvent)
{
    if (m_pTelemetrySession && pszEvent)
        m_pTelemetrySession->ReportEvent(L"App", pszEvent);
}

// ============================================================================
// LoadAddIns
//
// Scans the application's AddIns directory for plugin DLLs and loads
// any that export a recognized entry point (SundanceAddInInitialize).
// The original binary looked for SundanceAddInInitialize /
// SundanceAddInShutdown exports and called Initialize on each loaded
// add-in DLL. In the decompilation we enumerate the directory and
// load any matching DLLs, calling their init entry points.
// ============================================================================
HRESULT SundanceAppMain::LoadAddIns()
{
    // Build the AddIns directory path:
    //   %LOCALAPPDATA%\Microsoft\Windows Live\Movie Maker\AddIns
    WCHAR szLocalAppData[MAX_PATH] = { 0 };
    HRESULT hr = ::SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szLocalAppData);
    if (FAILED(hr))
        return S_OK; // Non-fatal: app can run without add-ins

    ATL::CString strAddInsDir;
    strAddInsDir.Format(L"%s\\Microsoft\\Windows Live\\Movie Maker\\AddIns", szLocalAppData);

    // Ensure directory exists (first-run creates the folder)
    ::CreateDirectoryW(strAddInsDir, NULL);

    ATL::CString strPattern;
    strPattern.Format(L"%s\\*.dll", strAddInsDir.GetString());

    WIN32_FIND_DATAW fd = { 0 };
    HANDLE hFind = ::FindFirstFileW(strPattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return S_OK; // No add-ins found — not an error

    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        ATL::CString strDllPath;
        strDllPath.Format(L"%s\\%s", strAddInsDir.GetString(), fd.cFileName);

        HMODULE hMod = ::LoadLibraryW(strDllPath);
        if (!hMod)
            continue;

        // Look for the SundanceAddInInitialize entry point
        typedef HRESULT (WINAPI *PFN_ADDIN_INIT)(SundanceAppMain*);
        typedef void (WINAPI *PFN_ADDIN_SHUTDOWN)();

        PFN_ADDIN_INIT pfnInit = reinterpret_cast<PFN_ADDIN_INIT>(
            ::GetProcAddress(hMod, "SundanceAddInInitialize"));
        PFN_ADDIN_SHUTDOWN pfnShutdown = reinterpret_cast<PFN_ADDIN_SHUTDOWN>(
            ::GetProcAddress(hMod, "SundanceAddInShutdown"));

        if (pfnInit)
        {
            HRESULT hrInit = pfnInit(this);
            if (SUCCEEDED(hrInit))
            {
                // Add-in initialized successfully. Store shutdown
                // callback if available. The original binary tracked
                // loaded add-ins in a vector for shutdown enumeration.
            }
            else
            {
                // Add-in rejected initialization — unload it
                ::FreeLibrary(hMod);
            }
        }
        else
        {
            // DLL does not export the expected entry point — skip
            ::FreeLibrary(hMod);
        }

    } while (::FindNextFileW(hFind, &fd));

    ::FindClose(hFind);

    return S_OK;
}

// ============================================================================
// ReportError
//
// Reports an error to the error reporting subsystem and logs it via
// OutputDebugString in debug builds.
// ============================================================================
void SundanceAppMain::ReportError(HRESULT hr, LPCWSTR pszContext)
{
#ifdef _DEBUG
    WCHAR szMsg[512] = { 0 };
    ::StringCchPrintfW(szMsg, ARRAYSIZE(szMsg),
        L"SundanceApp: Error 0x%08X in %s\n",
        hr, pszContext ? pszContext : L"(unknown)");
    ::OutputDebugStringW(szMsg);
#else
    UNREFERENCED_PARAMETER(hr);
    UNREFERENCED_PARAMETER(pszContext);
#endif
}

// ============================================================================
// ShowHelp
//
// Opens the application help page. In the original binary, this launched
// the Windows Live Movie Maker help topic via the system default browser
// or the Windows Help viewer.
// ============================================================================
void SundanceAppMain::ShowHelp()
{
    // Open the Windows Live Movie Maker online help page
    ::ShellExecuteW(NULL, L"open",
        L"https://windows.microsoft.com/en-us/windows-live/movie-maker",
        NULL, NULL, SW_SHOWNORMAL);
}
