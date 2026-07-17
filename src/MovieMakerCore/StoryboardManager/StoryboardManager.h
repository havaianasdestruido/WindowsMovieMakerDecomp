/*
 * StoryboardManager.h
 *
 * Root header for the StoryboardManager namespace within MovieMakerCore.dll.
 * Manages the project model, timeline extents, templates, tracks, and
 * serialization for Windows Live Movie Maker 2012 (.wlmp project files).
 *
 * RTTI analysis identifies 143 classes in this namespace. Key classes include
 * MovieProject, MovieExtent, ExtentIdSet, TemplateTable, and various
 * timeline track types.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef STORYBOARDMANAGER_H
#define STORYBOARDMANAGER_H

#ifndef STRICT
#define STRICT
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WINVER        0x0602
#define _WIN32_WINNT  0x0602
#define _WIN32_IE     0x0800

#include <windows.h>
#include <objbase.h>
#include <shlwapi.h>
#include <xmllite.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlsync.h>
#include <gdiplus.h>

#include "WLXPhotoBase.h"
#include "../MovieMakerCore.h"

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4505) // unreferenced local function has been removed
#pragma warning(disable: 4512) // assignment operator could not be generated

// ============================================================================
// StoryboardManager namespace
// ============================================================================
// The StoryboardManager namespace contains all classes responsible for the
// project model, timeline representation, extent management, template/theme
// handling, and .wlmp file serialization used by Windows Live Movie Maker.
//
namespace StoryboardManager
{

// ============================================================================
// Forward declarations - exceptions
// ============================================================================
class TemplateInitializationException;
class ScriptInitializationException;
class EncodeInitializationException;

// ============================================================================
// Forward declarations - core project types
// ============================================================================
class MovieProject;
class MovieProjectState;
class MovieProjectSettings;

// ============================================================================
// Forward declarations - extents and selection
// ============================================================================
class MovieExtent;
class ExtentCollection;
class ExtentIdSet;
class ExtentIdSetSelectionRangeIterator;
class SelectionIndex;

// ============================================================================
// Forward declarations - timeline tracks
// ============================================================================
class TimelineTrack;
class VideoTrack;
class AudioTrack;
class MusicTrack;
class TitleTrack;
class CreditsTrack;
class TransitionTrack;

// ============================================================================
// Forward declarations - templates and themes
// ============================================================================
class TemplateTable;
class TemplateEntry;
class TimelineTemplateSource;
class AutoMovieTheme;
class ThemeEffect;

// ============================================================================
// Forward declarations - X3D scene management
// ============================================================================
class X3dScene;
class X3dTransition;
class X3dEffect;

// ============================================================================
// Forward declarations - serialization
// ============================================================================
class ProjectSerializer;
class ProjectDeserializer;
class XmlProjectWriter;
class XmlProjectReader;

// ============================================================================
// Forward declarations - internal
// ============================================================================
class ProjectMediaItem;
class ProjectTimeline;

// ============================================================================
// Enums
// ============================================================================

// Timeline track types matching the UI and project model
enum TimelineTrackType
{
    TimelineTrackTypeUnknown   = -1,
    TimelineTrackTypeVideo     = 0,
    TimelineTrackTypeAudio     = 1,
    TimelineTrackTypeMusic     = 2,
    TimelineTrackTypeTitle     = 3,
    TimelineTrackTypeCredits   = 4,
    TimelineTrackTypeTransition = 5
};

// Legacy alias for ClipboardManager compatibility
const auto TimelineTrack_Unknown = TimelineTrackTypeUnknown;

// Extent transcode states
enum ExtentTranscodeState
{
    ExtentTranscodeStateNone        = 0,
    ExtentTranscodeStatePending     = 1,
    ExtentTranscodeStateInProgress  = 2,
    ExtentTranscodeStateComplete    = 3,
    ExtentTranscodeStateFailed      = 4,
    ExtentTranscodeStateCancelled   = 5
};

// Project dirty flags
enum ProjectDirtyFlags
{
    ProjectDirtyFlagNone            = 0x0000,
    ProjectDirtyFlagMedia           = 0x0001,
    ProjectDirtyFlagTimeline        = 0x0002,
    ProjectDirtyFlagTransitions     = 0x0004,
    ProjectDirtyFlagEffects         = 0x0008,
    ProjectDirtyFlagTitles          = 0x0010,
    ProjectDirtyFlagSettings        = 0x0020,
    ProjectDirtyFlagAll             = 0xFFFF
};

// ============================================================================
// ExtentId
// ============================================================================
// Value type wrapping a DWORD extent ID with comparison and generation.
// ExtentIds are used throughout the timeline to uniquely identify extents.
//
class STORYBOARD_API ExtentId
{
public:
    ExtentId();
    ExtentId(DWORD dwId);
    ExtentId(const ExtentId& other);
    ExtentId& operator=(const ExtentId& other);

    DWORD GetValue() const throw();
    bool IsValid() const throw();

    static ExtentId Generate();

    bool operator==(const ExtentId& other) const;
    bool operator!=(const ExtentId& other) const;
    bool operator<(const ExtentId& other) const;

private:
    DWORD m_dwId;
};

// ============================================================================
// TranscodeState
// ============================================================================
// Tracks the transcode state and HRESULT for an extent being exported.
//
class STORYBOARD_API TranscodeState
{
public:
    TranscodeState();
    TranscodeState(ExtentTranscodeState initialState);
    ~TranscodeState();

    ExtentTranscodeState GetState() const throw();
    void SetState(ExtentTranscodeState state) throw();

    HRESULT GetHResult() const throw();
    void SetHResult(HRESULT hr) throw();

    bool IsComplete() const throw();
    bool IsFailed() const throw();
    bool IsInProgress() const throw();

private:
    ExtentTranscodeState m_state;
    HRESULT m_hresult;
};

// ============================================================================
// StoryboardManager initialization
// ============================================================================

// Initializes the StoryboardManager subsystem. Must be called before any
// other StoryboardManager functions. Typically called during DLL startup.
STORYBOARD_API HRESULT StoryboardManagerInitialize();

// Shuts down the StoryboardManager subsystem. Releases all global resources.
STORYBOARD_API void StoryboardManagerShutdown();

// Returns true if StoryboardManager has been successfully initialized.
STORYBOARD_API bool StoryboardManagerIsInitialized();

// ============================================================================
// StoryboardManager
// ============================================================================
// Top-level project manager. Manages the current project, active track,
// template table, and undo state for the storyboard editor.
//
class STORYBOARD_API StoryboardManager
{
public:
    StoryboardManager();
    ~StoryboardManager();

    // Singleton access
    static StoryboardManager& GetInstance();

    // Project management
    MovieProject* NewProject();
    HRESULT OpenProject(LPCWSTR pszPath);
    HRESULT SaveProject(LPCWSTR pszPath = nullptr);
    MovieProject* GetProject() const;

    // Undo manager (delegates to project undo stack)
    bool CanUndo() const;
    bool CanRedo() const;
    HRESULT Undo();
    HRESULT Redo();

    // Active track
    TimelineTrack* GetCurrentTrack() const;
    void SetCurrentTrack(TimelineTrack* pTrack);

    // Template table
    TemplateTable* GetTemplateTable();

private:
    MovieProject*       m_pCurrentProject;
    TimelineTrack*      m_pCurrentTrack;
    TemplateTable*      m_pTemplateTable;
};

} // namespace StoryboardManager

// ============================================================================
// Include sub-headers
// ============================================================================
#include "StoryboardManagerExceptions.h"
#include "MovieProject.h"
#include "TimelineTrack.h"
#include "Extents.h"
#include "MovieExtent.h"
#include "Templates.h"

#pragma warning(pop)

#endif // STORYBOARDMANAGER_H
