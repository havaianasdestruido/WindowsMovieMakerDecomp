/*
 * MovieProject.h
 *
 * MovieProject class - the top-level project model for Windows Live Movie Maker.
 * Manages the .wlmp project file format (XML-based), project state, media items,
 * timeline, and auto-save functionality.
 *
 * The .wlmp file format stores:
 *   <project>          - root element with version attributes
 *     <media>          - media item references (file paths, durations, thumbnails)
 *     <timeline>       - timeline tracks with extent ordering
 *     <transitions>    - transition effects between extents
 *     <effects>        - visual effects applied to extents
 *     <titles>         - text overlay definitions
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef MOVIEPROJECT_H
#define MOVIEPROJECT_H

#include "StoryboardManager.h"
#include <vector>

namespace StoryboardManager
{

// ============================================================================
// Project version constants
// ============================================================================
static const DWORD kProjectVersionMajor    = 1;
static const DWORD kProjectVersionMinor    = 0;
static const LPCWSTR kProjectRootElement   = L"project";
static const LPCWSTR kProjectVersionAttr   = L"version";

// Auto-save interval in milliseconds (5 minutes)
static const DWORD kAutoSaveIntervalMs     = 300000;
static const LPCWSTR kAutoSaveExtension    = L".wlmp.autosave";

// ============================================================================
// ProjectMediaItem
// ============================================================================
// Represents a single media item (video, photo, audio) referenced by the project.
// Stores the original file path, thumbnail path, and intrinsic metadata.
//
class STORYBOARD_API ProjectMediaItem
{
public:
    ProjectMediaItem();
    ~ProjectMediaItem();

    // Identity
    DWORD GetMediaId() const throw();
    void SetMediaId(DWORD dwId) throw();

    // File paths
    ATL::CString GetSourcePath() const;
    void SetSourcePath(LPCWSTR pszPath);

    ATL::CString GetThumbnailPath() const;
    void SetThumbnailPath(LPCWSTR pszPath);

    // Metadata
    LONGLONG GetDurationHns() const throw();  // duration in hundred-nanoseconds
    void SetDurationHns(LONGLONG llDuration) throw();

    LONGLONG GetStartTimeHns() const throw();
    void SetStartTimeHns(LONGLONG llStartTime) throw();

    // Media type (video, photo, audio)
    DWORD GetMediaType() const throw();
    void SetMediaType(DWORD dwType) throw();

    // Dimensions (for photos/video)
    UINT GetWidth() const throw();
    UINT GetHeight() const throw();
    void SetDimensions(UINT cx, UINT cy) throw();

    // Star rating (0-5)
    UINT GetRating() const throw();
    void SetRating(UINT uRating) throw();

    // Tags
    ATL::CString GetTags() const;
    void SetTags(LPCWSTR pszTags);

    // Date/time
    FILETIME GetDateTaken() const throw();
    void SetDateTaken(const FILETIME& ft) throw();

    // Frame rate (for video, in Hz * 100)
    DWORD GetFrameRate() const throw();
    void SetFrameRate(DWORD dwFrameRate) throw();

    // Codec info
    ATL::CString GetVideoCodec() const;
    void SetVideoCodec(LPCWSTR pszCodec);

    ATL::CString GetAudioCodec() const;
    void SetAudioCodec(LPCWSTR pszCodec);

    // Width/height of audio sample
    DWORD GetAudioChannels() const throw();
    void SetAudioChannels(DWORD dwChannels) throw();

    DWORD GetAudioSampleRate() const throw();
    void SetAudioSampleRate(DWORD dwSampleRate) throw();

    // Validity
    bool IsValid() const throw();
    bool FileExists() const;

private:
    DWORD       m_dwMediaId;
    ATL::CString m_strSourcePath;
    ATL::CString m_strThumbnailPath;
    LONGLONG    m_llDurationHns;
    LONGLONG    m_llStartTimeHns;
    DWORD       m_dwMediaType;
    UINT        m_uWidth;
    UINT        m_uHeight;
    UINT        m_uRating;
    ATL::CString m_strTags;
    FILETIME    m_ftDateTaken;
    DWORD       m_dwFrameRate;
    ATL::CString m_strVideoCodec;
    ATL::CString m_strAudioCodec;
    DWORD       m_dwAudioChannels;
    DWORD       m_dwAudioSampleRate;
};

// ============================================================================
// ProjectTimeline
// ============================================================================
// Stores the ordered list of extents for the project timeline. Each extent
// references a media item and has start/end trim points.
//
class STORYBOARD_API ProjectTimeline
{
public:
    ProjectTimeline();
    ~ProjectTimeline();

    // Extent count
    size_t GetExtentCount() const throw();

    // Extent access
    DWORD GetExtentIdAt(size_t nIndex) const;
    void SetExtentIdAt(size_t nIndex, DWORD dwExtentId);

    // Add/remove extents
    size_t AddExtent(DWORD dwExtentId);
    void RemoveExtentAt(size_t nIndex);
    void RemoveExtent(DWORD dwExtentId);
    void RemoveAllExtents();

    // Move extents (reorder)
    void MoveExtent(size_t nIndexFrom, size_t nIndexTo);

    // Find extent position
    int FindExtent(DWORD dwExtentId) const;

    // Track type for this timeline
    TimelineTrackType GetTrackType() const throw();
    void SetTrackType(TimelineTrackType type) throw();

    // Duration (computed from extents)
    LONGLONG GetTotalDurationHns() const throw();
    void SetTotalDurationHns(LONGLONG llDuration) throw();

private:
    ATL::CAtlArray<DWORD>   m_arrExtentIds;
    TimelineTrackType       m_trackType;
    LONGLONG                m_llTotalDurationHns;
};

// ============================================================================
// MovieProjectSettings
// ============================================================================
// Project-level settings (aspect ratio, output size, audio bit rate, etc.)
//
class STORYBOARD_API MovieProjectSettings
{
public:
    MovieProjectSettings();
    ~MovieProjectSettings();

    // Output video dimensions
    UINT GetOutputWidth() const throw();
    UINT GetOutputHeight() const throw();
    void SetOutputDimensions(UINT cx, UINT cy) throw();

    // Aspect ratio (e.g., 16:9 = 16/9.0)
    double GetAspectRatio() const throw();
    void SetAspectRatio(double dblRatio) throw();

    // Audio bit rate (bps)
    DWORD GetAudioBitRate() const throw();
    void SetAudioBitRate(DWORD dwBitRate) throw();

    // Video bit rate (bps)
    DWORD GetVideoBitRate() const throw();
    void SetVideoBitRate(DWORD dwBitRate) throw();

    // Frame rate
    DWORD GetFrameRate() const throw();
    void SetFrameRate(DWORD dwFrameRate) throw();

    // Sample rate
    DWORD GetSampleRate() const throw();
    void SetSampleRate(DWORD dwSampleRate) throw();

    // Auto-save enabled
    bool IsAutoSaveEnabled() const throw();
    void SetAutoSaveEnabled(bool fEnabled) throw();

    // Auto-save interval (milliseconds)
    DWORD GetAutoSaveIntervalMs() const throw();
    void SetAutoSaveIntervalMs(DWORD dwIntervalMs) throw();

    // Default transition duration (hundred-nanoseconds)
    LONGLONG GetDefaultTransitionDurationHns() const throw();
    void SetDefaultTransitionDurationHns(LONGLONG llDuration) throw();

    // Project name/title
    ATL::CString GetProjectName() const;
    void SetProjectName(LPCWSTR pszName);

    // Author
    ATL::CString GetAuthor() const;
    void SetAuthor(LPCWSTR pszAuthor);

private:
    UINT        m_uOutputWidth;
    UINT        m_uOutputHeight;
    double      m_dblAspectRatio;
    DWORD       m_dwAudioBitRate;
    DWORD       m_dwVideoBitRate;
    DWORD       m_dwFrameRate;
    DWORD       m_dwSampleRate;
    bool        m_fAutoSaveEnabled;
    DWORD       m_dwAutoSaveIntervalMs;
    LONGLONG    m_llDefaultTransitionDurationHns;
    ATL::CString m_strProjectName;
    ATL::CString m_strAuthor;
};

// ============================================================================
// MovieProjectState
// ============================================================================
// Tracks the current state of a project (modified, saving, etc.)
//
class STORYBOARD_API MovieProjectState
{
public:
    MovieProjectState();
    ~MovieProjectState();

    // Dirty tracking
    bool IsDirty() const throw();
    void SetDirty(DWORD dwFlags) throw();
    void ClearDirty(DWORD dwFlags) throw();
    DWORD GetDirtyFlags() const throw();

    // Save state
    bool IsSaving() const throw();
    void SetSaving(bool fSaving) throw();

    // Load state
    bool IsLoading() const throw();
    void SetLoading(bool fLoading) throw();

    // Modified timestamp
    FILETIME GetLastModifiedTime() const throw();
    void SetLastModifiedTime(const FILETIME& ft) throw();

    // Undo support (extent-level)
    bool CanUndo() const throw();
    bool CanRedo() const throw();
    void SetUndoAvailable(bool bAvailable) throw();
    void SetRedoAvailable(bool bAvailable) throw();

private:
    DWORD       m_dwDirtyFlags;
    bool        m_fSaving;
    bool        m_fLoading;
    bool        m_bCanUndo;
    bool        m_bCanRedo;
    FILETIME    m_ftLastModified;
};

// ============================================================================
// MovieProject
// ============================================================================
// The top-level project object. Owns media items, timelines, settings, and
// handles serialization to/from .wlmp XML files.
//
// This is the primary class managed by the Sundance ProjectManager.
//
class STORYBOARD_API MovieProject
{
public:
    MovieProject();
    virtual ~MovieProject();

    // Factory method - creates a new empty project
    static MovieProject* CreateEmpty();

    // Project file path
    ATL::CString GetProjectPath() const;
    void SetProjectPath(LPCWSTR pszPath);

    ATL::CString GetProjectName() const;

    // Serialization
    HRESULT Save(LPCWSTR pszPath = nullptr);
    HRESULT Load(LPCWSTR pszPath);
    HRESULT SaveToStream(IStream* pStream);
    HRESULT LoadFromStream(IStream* pStream);

    // Auto-save
    HRESULT AutoSave();
    bool HasAutoSaveFile() const;
    HRESULT RecoverAutoSave();
    void DeleteAutoSaveFile();

    // Settings
    MovieProjectSettings& GetSettings();
    const MovieProjectSettings& GetSettings() const;

    // State
    MovieProjectState& GetState();
    const MovieProjectState& GetState() const;

    // Media items
    size_t GetMediaItemCount() const throw();
    const ProjectMediaItem* GetMediaItem(size_t nIndex) const;
    ProjectMediaItem* GetMediaItem(size_t nIndex);
    size_t AddMediaItem(const ProjectMediaItem& item);
    size_t AddMediaItemFromFile(LPCWSTR pszFilePath);
    void RemoveMediaItem(size_t nIndex);
    void RemoveMediaItemById(DWORD dwMediaId);
    int FindMediaItemById(DWORD dwMediaId) const;
    void RemoveAllMediaItems();

    // Timelines (one per track type)
    ProjectTimeline* GetTimeline(TimelineTrackType type);
    const ProjectTimeline* GetTimeline(TimelineTrackType type) const;

    // Combined timeline extent count across all tracks
    size_t GetTotalExtentCount() const throw();

    // Project version
    DWORD GetVersionMajor() const throw();
    DWORD GetVersionMinor() const throw();

    // Aliases used by UI layer
    ATL::CString GetFilePath() const;
    ATL::CString GetDisplayName() const;
    HRESULT SaveAs(LPCWSTR pszPath);
    HRESULT ImportMedia(LPCWSTR pszPath, TimelineTrackType trackType);
    HRESULT RemoveItem(DWORD dwItemId, TimelineTrackType trackType);
    HRESULT MoveItem(DWORD dwItemId, TimelineTrackType trackType, DWORD dwNewPosition);
    size_t GetItemCount(TimelineTrackType trackType) const throw();
    bool IsItemValid(DWORD dwItemId, TimelineTrackType trackType) const throw();
    void GetExtentIdsForMediaItem(DWORD dwItemId, TimelineTrackType trackType, Base::Array<DWORD>& ids) const;
    HRESULT GetExtentForMediaItem(DWORD dwItemId, Base::PtrRef<MovieExtent>& extentOut);
    bool HasItemsNeedingProxyTranscode() const;
    HRESULT StartProxyTranscode();
    void MarkAllExtentsForRetranscode(TimelineTrackType trackType);

    // Undo/redo
    HRESULT Undo();
    HRESULT Redo();
    void ClearUndoHistory();

    // Thumbnail management
    HRESULT GenerateThumbnails();

    // Validation
    HRESULT Validate() const;

private:
    // Internal helpers for serialization
    HRESULT WriteProjectElement(IXmlWriter* pWriter);
    HRESULT WriteMediaElement(IXmlWriter* pWriter);
    HRESULT WriteTimelineElement(IXmlWriter* pWriter);
    HRESULT WriteTransitionsElement(IXmlWriter* pWriter);
    HRESULT WriteEffectsElement(IXmlWriter* pWriter);
    HRESULT WriteTitlesElement(IXmlWriter* pWriter);

    HRESULT ReadProjectElement(IXmlReader* pReader);
    HRESULT ReadMediaElement(IXmlReader* pReader);
    HRESULT ReadTimelineElement(IXmlReader* pReader);
    HRESULT ReadTransitionsElement(IXmlReader* pReader);
    HRESULT ReadEffectsElement(IXmlReader* pReader);
    HRESULT ReadTitlesElement(IXmlReader* pReader);

    HRESULT WriteAutoSave();
    HRESULT ReadAutoSaveFile(LPCWSTR pszAutoSavePath);

    DWORD   GenerateNextMediaId();
    DWORD   GenerateNextExtentId();

    void    PushUndoSnapshot();
    void    UpdateUndoState();

    ATL::CString        m_strProjectPath;
    MovieProjectSettings m_settings;
    MovieProjectState    m_state;

    ATL::CAtlArray<ProjectMediaItem>   m_arrMediaItems;
    ProjectTimeline    m_arrTimelines[6];  // one per TimelineTrackType

    DWORD               m_dwVersionMajor;
    DWORD               m_dwVersionMinor;
    DWORD               m_dwNextMediaId;
    DWORD               m_dwNextExtentId;

    // Undo stack
    class UndoEntry;
    ATL::CAtlArray<UndoEntry*> m_arrUndoStack;
    int                 m_nUndoPosition;
};

} // namespace StoryboardManager

#endif // MOVIEPROJECT_H
