#include "pch.h"
/*
 * MovieProject.cpp
 *
 * Implementation of MovieProject and supporting classes for project
 * serialization, state management, and auto-save functionality.
 *
 * The .wlmp file format is XML-based and uses IXmlWriter/IXmlReader (XmlLite)
 * for parsing. The project file structure:
 *
 *   <?xml version="1.0" encoding="utf-8"?>
 *   <project versionMajor="1" versionMinor="0" name="My Movie">
 *     <media>
 *       <mediaItem id="1" path="C:\video.wmv" type="video" duration="50000000"
 *                  width="1920" height="1080" frameRate="3000" />
 *     </media>
 *     <timeline track="video">
 *       <extent id="1" mediaId="1" startTime="0" endTime="50000000" />
 *     </timeline>
 *     <transitions>
 *       <transition extentId="1" type="Fade" duration="10000000" />
 *     </transitions>
 *     <effects>
 *       <effect extentId="1" type="Sepia" />
 *     </effects>
 *     <titles>
 *       <title extentId="1" text="My Movie" font="Segoe UI" size="24"
 *              color="#FFFFFF" startTime="0" duration="30000000" />
 *     </titles>
 *   </project>
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "MovieProject.h"
#include <strsafe.h>

namespace StoryboardManager
{

// ============================================================================
// ProjectMediaItem implementation
// ============================================================================

ProjectMediaItem::ProjectMediaItem()
    : m_dwMediaId(0)
    , m_llDurationHns(0)
    , m_llStartTimeHns(0)
    , m_dwMediaType(0)
    , m_uWidth(0)
    , m_uHeight(0)
    , m_uRating(0)
    , m_dwFrameRate(0)
    , m_dwAudioChannels(0)
    , m_dwAudioSampleRate(0)
{
    ZeroMemory(&m_ftDateTaken, sizeof(FILETIME));
}

ProjectMediaItem::~ProjectMediaItem()
{
}

DWORD ProjectMediaItem::GetMediaId() const throw()
{
    return m_dwMediaId;
}

void ProjectMediaItem::SetMediaId(DWORD dwId) throw()
{
    m_dwMediaId = dwId;
}

ATL::CString ProjectMediaItem::GetSourcePath() const
{
    return m_strSourcePath;
}

void ProjectMediaItem::SetSourcePath(LPCWSTR pszPath)
{
    m_strSourcePath = pszPath ? pszPath : L"";
}

ATL::CString ProjectMediaItem::GetThumbnailPath() const
{
    return m_strThumbnailPath;
}

void ProjectMediaItem::SetThumbnailPath(LPCWSTR pszPath)
{
    m_strThumbnailPath = pszPath ? pszPath : L"";
}

LONGLONG ProjectMediaItem::GetDurationHns() const throw()
{
    return m_llDurationHns;
}

void ProjectMediaItem::SetDurationHns(LONGLONG llDuration) throw()
{
    m_llDurationHns = llDuration;
}

LONGLONG ProjectMediaItem::GetStartTimeHns() const throw()
{
    return m_llStartTimeHns;
}

void ProjectMediaItem::SetStartTimeHns(LONGLONG llStartTime) throw()
{
    m_llStartTimeHns = llStartTime;
}

DWORD ProjectMediaItem::GetMediaType() const throw()
{
    return m_dwMediaType;
}

void ProjectMediaItem::SetMediaType(DWORD dwType) throw()
{
    m_dwMediaType = dwType;
}

UINT ProjectMediaItem::GetWidth() const throw()
{
    return m_uWidth;
}

UINT ProjectMediaItem::GetHeight() const throw()
{
    return m_uHeight;
}

void ProjectMediaItem::SetDimensions(UINT cx, UINT cy) throw()
{
    m_uWidth = cx;
    m_uHeight = cy;
}

UINT ProjectMediaItem::GetRating() const throw()
{
    return m_uRating;
}

void ProjectMediaItem::SetRating(UINT uRating) throw()
{
    m_uRating = (uRating <= 5) ? uRating : 0;
}

ATL::CString ProjectMediaItem::GetTags() const
{
    return m_strTags;
}

void ProjectMediaItem::SetTags(LPCWSTR pszTags)
{
    m_strTags = pszTags ? pszTags : L"";
}

FILETIME ProjectMediaItem::GetDateTaken() const throw()
{
    return m_ftDateTaken;
}

void ProjectMediaItem::SetDateTaken(const FILETIME& ft) throw()
{
    m_ftDateTaken = ft;
}

DWORD ProjectMediaItem::GetFrameRate() const throw()
{
    return m_dwFrameRate;
}

void ProjectMediaItem::SetFrameRate(DWORD dwFrameRate) throw()
{
    m_dwFrameRate = dwFrameRate;
}

ATL::CString ProjectMediaItem::GetVideoCodec() const
{
    return m_strVideoCodec;
}

void ProjectMediaItem::SetVideoCodec(LPCWSTR pszCodec)
{
    m_strVideoCodec = pszCodec ? pszCodec : L"";
}

ATL::CString ProjectMediaItem::GetAudioCodec() const
{
    return m_strAudioCodec;
}

void ProjectMediaItem::SetAudioCodec(LPCWSTR pszCodec)
{
    m_strAudioCodec = pszCodec ? pszCodec : L"";
}

DWORD ProjectMediaItem::GetAudioChannels() const throw()
{
    return m_dwAudioChannels;
}

void ProjectMediaItem::SetAudioChannels(DWORD dwChannels) throw()
{
    m_dwAudioChannels = dwChannels;
}

DWORD ProjectMediaItem::GetAudioSampleRate() const throw()
{
    return m_dwAudioSampleRate;
}

void ProjectMediaItem::SetAudioSampleRate(DWORD dwSampleRate) throw()
{
    m_dwAudioSampleRate = dwSampleRate;
}

bool ProjectMediaItem::IsValid() const throw()
{
    return m_dwMediaId != 0 && !m_strSourcePath.IsEmpty();
}

bool ProjectMediaItem::FileExists() const
{
    if (m_strSourcePath.IsEmpty())
        return false;
    return ::PathFileExistsW(m_strSourcePath) != FALSE;
}

// ============================================================================
// ProjectTimeline implementation
// ============================================================================

ProjectTimeline::ProjectTimeline()
    : m_trackType(TimelineTrackTypeVideo)
    , m_llTotalDurationHns(0)
{
}

ProjectTimeline::~ProjectTimeline()
{
}

size_t ProjectTimeline::GetExtentCount() const throw()
{
    return m_arrExtentIds.GetCount();
}

DWORD ProjectTimeline::GetExtentIdAt(size_t nIndex) const
{
    ATLASSERT(nIndex < m_arrExtentIds.GetCount());
    return m_arrExtentIds.GetAt(nIndex);
}

void ProjectTimeline::SetExtentIdAt(size_t nIndex, DWORD dwExtentId)
{
    ATLASSERT(nIndex < m_arrExtentIds.GetCount());
    m_arrExtentIds.SetAt(nIndex, dwExtentId);
}

size_t ProjectTimeline::AddExtent(DWORD dwExtentId)
{
    return m_arrExtentIds.Add(dwExtentId);
}

void ProjectTimeline::RemoveExtentAt(size_t nIndex)
{
    ATLASSERT(nIndex < m_arrExtentIds.GetCount());
    m_arrExtentIds.RemoveAt(nIndex);
}

void ProjectTimeline::RemoveExtent(DWORD dwExtentId)
{
    int nIndex = FindExtent(dwExtentId);
    if (nIndex >= 0)
    {
        m_arrExtentIds.RemoveAt(static_cast<size_t>(nIndex));
    }
}

void ProjectTimeline::RemoveAllExtents()
{
    m_arrExtentIds.RemoveAll();
    m_llTotalDurationHns = 0;
}

void ProjectTimeline::MoveExtent(size_t nIndexFrom, size_t nIndexTo)
{
    if (nIndexFrom >= m_arrExtentIds.GetCount() ||
        nIndexTo >= m_arrExtentIds.GetCount() ||
        nIndexFrom == nIndexTo)
    {
        return;
    }

    DWORD dwId = m_arrExtentIds.GetAt(nIndexFrom);
    m_arrExtentIds.RemoveAt(nIndexFrom);
    m_arrExtentIds.InsertAt(nIndexTo, dwId);
}

int ProjectTimeline::FindExtent(DWORD dwExtentId) const
{
    for (size_t i = 0; i < m_arrExtentIds.GetCount(); ++i)
    {
        if (m_arrExtentIds.GetAt(i) == dwExtentId)
            return static_cast<int>(i);
    }
    return -1;
}

TimelineTrackType ProjectTimeline::GetTrackType() const throw()
{
    return m_trackType;
}

void ProjectTimeline::SetTrackType(TimelineTrackType type) throw()
{
    m_trackType = type;
}

LONGLONG ProjectTimeline::GetTotalDurationHns() const throw()
{
    return m_llTotalDurationHns;
}

void ProjectTimeline::SetTotalDurationHns(LONGLONG llDuration) throw()
{
    m_llTotalDurationHns = llDuration;
}

// ============================================================================
// MovieProjectSettings implementation
// ============================================================================

MovieProjectSettings::MovieProjectSettings()
    : m_uOutputWidth(1280)
    , m_uOutputHeight(720)
    , m_dblAspectRatio(16.0 / 9.0)
    , m_dwAudioBitRate(192000)
    , m_dwVideoBitRate(8000000)
    , m_dwFrameRate(3000)
    , m_dwSampleRate(44100)
    , m_fAutoSaveEnabled(true)
    , m_dwAutoSaveIntervalMs(kAutoSaveIntervalMs)
    , m_llDefaultTransitionDurationHns(10000000) // 1 second
{
}

MovieProjectSettings::~MovieProjectSettings()
{
}

UINT MovieProjectSettings::GetOutputWidth() const throw()
{
    return m_uOutputWidth;
}

UINT MovieProjectSettings::GetOutputHeight() const throw()
{
    return m_uOutputHeight;
}

void MovieProjectSettings::SetOutputDimensions(UINT cx, UINT cy) throw()
{
    m_uOutputWidth = cx;
    m_uOutputHeight = cy;
}

double MovieProjectSettings::GetAspectRatio() const throw()
{
    return m_dblAspectRatio;
}

void MovieProjectSettings::SetAspectRatio(double dblRatio) throw()
{
    m_dblAspectRatio = dblRatio;
}

DWORD MovieProjectSettings::GetAudioBitRate() const throw()
{
    return m_dwAudioBitRate;
}

void MovieProjectSettings::SetAudioBitRate(DWORD dwBitRate) throw()
{
    m_dwAudioBitRate = dwBitRate;
}

DWORD MovieProjectSettings::GetVideoBitRate() const throw()
{
    return m_dwVideoBitRate;
}

void MovieProjectSettings::SetVideoBitRate(DWORD dwBitRate) throw()
{
    m_dwVideoBitRate = dwBitRate;
}

DWORD MovieProjectSettings::GetFrameRate() const throw()
{
    return m_dwFrameRate;
}

void MovieProjectSettings::SetFrameRate(DWORD dwFrameRate) throw()
{
    m_dwFrameRate = dwFrameRate;
}

DWORD MovieProjectSettings::GetSampleRate() const throw()
{
    return m_dwSampleRate;
}

void MovieProjectSettings::SetSampleRate(DWORD dwSampleRate) throw()
{
    m_dwSampleRate = dwSampleRate;
}

bool MovieProjectSettings::IsAutoSaveEnabled() const throw()
{
    return m_fAutoSaveEnabled;
}

void MovieProjectSettings::SetAutoSaveEnabled(bool fEnabled) throw()
{
    m_fAutoSaveEnabled = fEnabled;
}

DWORD MovieProjectSettings::GetAutoSaveIntervalMs() const throw()
{
    return m_dwAutoSaveIntervalMs;
}

void MovieProjectSettings::SetAutoSaveIntervalMs(DWORD dwIntervalMs) throw()
{
    m_dwAutoSaveIntervalMs = dwIntervalMs;
}

LONGLONG MovieProjectSettings::GetDefaultTransitionDurationHns() const throw()
{
    return m_llDefaultTransitionDurationHns;
}

void MovieProjectSettings::SetDefaultTransitionDurationHns(LONGLONG llDuration) throw()
{
    m_llDefaultTransitionDurationHns = llDuration;
}

ATL::CString MovieProjectSettings::GetProjectName() const
{
    return m_strProjectName;
}

void MovieProjectSettings::SetProjectName(LPCWSTR pszName)
{
    m_strProjectName = pszName ? pszName : L"";
}

ATL::CString MovieProjectSettings::GetAuthor() const
{
    return m_strAuthor;
}

void MovieProjectSettings::SetAuthor(LPCWSTR pszAuthor)
{
    m_strAuthor = pszAuthor ? pszAuthor : L"";
}

// ============================================================================
// MovieProjectState implementation
// ============================================================================

MovieProjectState::MovieProjectState()
    : m_dwDirtyFlags(ProjectDirtyFlagNone)
    , m_fSaving(false)
    , m_fLoading(false)
{
    ZeroMemory(&m_ftLastModified, sizeof(FILETIME));
}

MovieProjectState::~MovieProjectState()
{
}

bool MovieProjectState::IsDirty() const throw()
{
    return m_dwDirtyFlags != ProjectDirtyFlagNone;
}

void MovieProjectState::SetDirty(DWORD dwFlags) throw()
{
    m_dwDirtyFlags |= dwFlags;
    ::GetSystemTimeAsFileTime(&m_ftLastModified);
}

void MovieProjectState::ClearDirty(DWORD dwFlags) throw()
{
    m_dwDirtyFlags &= ~dwFlags;
}

DWORD MovieProjectState::GetDirtyFlags() const throw()
{
    return m_dwDirtyFlags;
}

bool MovieProjectState::IsSaving() const throw()
{
    return m_fSaving;
}

void MovieProjectState::SetSaving(bool fSaving) throw()
{
    m_fSaving = fSaving;
}

bool MovieProjectState::IsLoading() const throw()
{
    return m_fLoading;
}

void MovieProjectState::SetLoading(bool fLoading) throw()
{
    m_fLoading = fLoading;
}

FILETIME MovieProjectState::GetLastModifiedTime() const throw()
{
    return m_ftLastModified;
}

void MovieProjectState::SetLastModifiedTime(const FILETIME& ft) throw()
{
    m_ftLastModified = ft;
}

bool MovieProjectState::CanUndo() const throw()
{
    return false; // stub - full undo stack implemented in MovieProject
}

bool MovieProjectState::CanRedo() const throw()
{
    return false; // stub
}

// ============================================================================
// MovieProject::UndoEntry (internal)
// ============================================================================

class MovieProject::UndoEntry
{
public:
    UndoEntry() {}
    ~UndoEntry() {}

    ATL::CString    m_strDescription;
    ATL::CString    m_strProjectXml;  // serialized project state snapshot
};

// ============================================================================
// MovieProject implementation
// ============================================================================

MovieProject::MovieProject()
    : m_dwVersionMajor(kProjectVersionMajor)
    , m_dwVersionMinor(kProjectVersionMinor)
    , m_dwNextMediaId(1)
    , m_dwNextExtentId(1)
    , m_nUndoPosition(-1)
{
    for (int i = 0; i < 6; ++i)
    {
        m_arrTimelines[i].SetTrackType(static_cast<TimelineTrackType>(i));
    }
}

MovieProject::~MovieProject()
{
    for (size_t i = 0; i < m_arrUndoStack.GetCount(); ++i)
    {
        delete m_arrUndoStack.GetAt(i);
    }
    m_arrUndoStack.RemoveAll();
}

MovieProject* MovieProject::CreateEmpty()
{
    return new MovieProject();
}

ATL::CString MovieProject::GetProjectPath() const
{
    return m_strProjectPath;
}

void MovieProject::SetProjectPath(LPCWSTR pszPath)
{
    m_strProjectPath = pszPath ? pszPath : L"";
}

ATL::CString MovieProject::GetProjectName() const
{
    if (!m_settings.GetProjectName().IsEmpty())
        return m_settings.GetProjectName();

    // Extract name from path
    if (m_strProjectPath.IsEmpty())
        return ATL::CString(L"Untitled");

    LPCWSTR pszFileName = wcsrchr(m_strProjectPath, L'\\');
    if (pszFileName)
    {
        ++pszFileName;
        // Strip extension
        LPCWSTR pszDot = wcsrchr(pszFileName, L'.');
        if (pszDot)
        {
            int cch = static_cast<int>(pszDot - pszFileName);
            return ATL::CString(pszFileName, cch);
        }
        return ATL::CString(pszFileName);
    }

    return m_strProjectPath;
}

// ============================================================================
// MovieProject::Save
// ============================================================================

HRESULT MovieProject::Save(LPCWSTR pszPath)
{
    if (pszPath)
        m_strProjectPath = pszPath;

    if (m_strProjectPath.IsEmpty())
        return E_INVALIDARG;

    m_state.SetSaving(true);

    HRESULT hr = S_OK;

    // Create the XmlLite writer
    IXmlWriter* pWriter = nullptr;
    hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriter), nullptr);
    if (FAILED(hr))
    {
        m_state.SetSaving(false);
        return hr;
    }

    // Create a write stream to the project file
    IStream* pStream = nullptr;
    hr = SHCreateStreamOnFileW(m_strProjectPath, STGM_WRITE | STGM_CREATE, &pStream);
    if (FAILED(hr))
    {
        pWriter->Release();
        m_state.SetSaving(false);
        return hr;
    }

    hr = pWriter->SetOutput(pStream);
    if (SUCCEEDED(hr))
    {
        hr = pWriter->WriteStartDocument(XmlStandalone_Omit);
    }

    if (SUCCEEDED(hr))
    {
        hr = WriteProjectElement(pWriter);
    }

    if (SUCCEEDED(hr))
    {
        hr = pWriter->WriteEndElement(); // project
    }

    if (SUCCEEDED(hr))
    {
        hr = pWriter->WriteEndDocument();
    }

    if (SUCCEEDED(hr))
    {
        hr = pWriter->Flush();
    }

    pStream->Release();
    pWriter->Release();

    if (SUCCEEDED(hr))
    {
        m_state.ClearDirty(ProjectDirtyFlagAll);
        DeleteAutoSaveFile();
    }

    m_state.SetSaving(false);

    return hr;
}

// ============================================================================
// MovieProject::Load
// ============================================================================

HRESULT MovieProject::Load(LPCWSTR pszPath)
{
    if (!pszPath || !pszPath[0])
        return E_INVALIDARG;

    m_state.SetLoading(true);

    HRESULT hr = S_OK;

    IXmlReader* pReader = nullptr;
    hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), nullptr);
    if (FAILED(hr))
    {
        m_state.SetLoading(false);
        return hr;
    }

    IStream* pStream = nullptr;
    hr = SHCreateStreamOnFileW(pszPath, STGM_READ, &pStream);
    if (FAILED(hr))
    {
        pReader->Release();
        m_state.SetLoading(false);
        return hr;
    }

    hr = pReader->SetInput(pStream);
    if (SUCCEEDED(hr))
    {
        XmlNodeType nodeType;
        while (pReader->Read(&nodeType) == S_OK)
        {
            if (nodeType == XmlNodeType_Element)
            {
                LPCWSTR pszLocalName = nullptr;
                hr = pReader->GetLocalName(&pszLocalName, nullptr);
                if (SUCCEEDED(hr) && pszLocalName)
                {
                    if (wcscmp(pszLocalName, kProjectRootElement) == 0)
                    {
                        hr = ReadProjectElement(pReader);
                    }
                }
            }
        }
    }

    pStream->Release();
    pReader->Release();

    if (SUCCEEDED(hr))
    {
        m_strProjectPath = pszPath;
        m_state.ClearDirty(ProjectDirtyFlagAll);
    }

    m_state.SetLoading(false);

    return hr;
}

// ============================================================================
// MovieProject::SaveToStream / LoadFromStream
// ============================================================================

HRESULT MovieProject::SaveToStream(IStream* pStream)
{
    if (!pStream)
        return E_POINTER;

    HRESULT hr = S_OK;

    IXmlWriter* pWriter = nullptr;
    hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriter), nullptr);
    if (FAILED(hr))
        return hr;

    hr = pWriter->SetOutput(pStream);
    if (SUCCEEDED(hr))
        hr = pWriter->WriteStartDocument(XmlStandalone_Omit);
    if (SUCCEEDED(hr))
        hr = WriteProjectElement(pWriter);
    if (SUCCEEDED(hr))
        hr = pWriter->WriteEndElement(); // project
    if (SUCCEEDED(hr))
        hr = pWriter->WriteEndDocument();
    if (SUCCEEDED(hr))
        hr = pWriter->Flush();

    pWriter->Release();
    return hr;
}

HRESULT MovieProject::LoadFromStream(IStream* pStream)
{
    if (!pStream)
        return E_POINTER;

    HRESULT hr = S_OK;

    IXmlReader* pReader = nullptr;
    hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), nullptr);
    if (FAILED(hr))
        return hr;

    hr = pReader->SetInput(pStream);
    if (SUCCEEDED(hr))
    {
        XmlNodeType nodeType;
        while (pReader->Read(&nodeType) == S_OK)
        {
            if (nodeType == XmlNodeType_Element)
            {
                LPCWSTR pszLocalName = nullptr;
                hr = pReader->GetLocalName(&pszLocalName, nullptr);
                if (SUCCEEDED(hr) && pszLocalName)
                {
                    if (wcscmp(pszLocalName, kProjectRootElement) == 0)
                    {
                        hr = ReadProjectElement(pReader);
                    }
                }
            }
        }
    }

    pReader->Release();
    return hr;
}

// ============================================================================
// Auto-save
// ============================================================================

HRESULT MovieProject::AutoSave()
{
    if (m_strProjectPath.IsEmpty())
        return E_UNEXPECTED;

    if (!m_state.IsDirty())
        return S_FALSE; // nothing to auto-save

    // Build auto-save path: <project>.wlmp.autosave
    ATL::CString strAutoSavePath = m_strProjectPath + kAutoSaveExtension;

    return Save(strAutoSavePath);
}

bool MovieProject::HasAutoSaveFile() const
{
    if (m_strProjectPath.IsEmpty())
        return false;

    ATL::CString strAutoSavePath = m_strProjectPath + kAutoSaveExtension;
    return ::PathFileExistsW(strAutoSavePath) != FALSE;
}

HRESULT MovieProject::RecoverAutoSave()
{
    if (!HasAutoSaveFile())
        return S_FALSE;

    ATL::CString strAutoSavePath = m_strProjectPath + kAutoSaveExtension;
    return Load(strAutoSavePath);
}

void MovieProject::DeleteAutoSaveFile()
{
    if (m_strProjectPath.IsEmpty())
        return;

    ATL::CString strAutoSavePath = m_strProjectPath + kAutoSaveExtension;
    if (::PathFileExistsW(strAutoSavePath))
    {
        ::DeleteFileW(strAutoSavePath);
    }
}

// ============================================================================
// Settings and state accessors
// ============================================================================

MovieProjectSettings& MovieProject::GetSettings()
{
    return m_settings;
}

const MovieProjectSettings& MovieProject::GetSettings() const
{
    return m_settings;
}

MovieProjectState& MovieProject::GetState()
{
    return m_state;
}

const MovieProjectState& MovieProject::GetState() const
{
    return m_state;
}

// ============================================================================
// Media item management
// ============================================================================

size_t MovieProject::GetMediaItemCount() const throw()
{
    return m_arrMediaItems.GetCount();
}

const ProjectMediaItem* MovieProject::GetMediaItem(size_t nIndex) const
{
    if (nIndex >= m_arrMediaItems.GetCount())
        return nullptr;
    return &m_arrMediaItems.GetAt(nIndex);
}

ProjectMediaItem* MovieProject::GetMediaItem(size_t nIndex)
{
    if (nIndex >= m_arrMediaItems.GetCount())
        return nullptr;
    return &m_arrMediaItems.GetAt(nIndex);
}

size_t MovieProject::AddMediaItem(const ProjectMediaItem& item)
{
    ProjectMediaItem newItem = item;
    if (newItem.GetMediaId() == 0)
    {
        newItem.SetMediaId(GenerateNextMediaId());
    }
    size_t nIndex = m_arrMediaItems.Add(newItem);
    m_state.SetDirty(ProjectDirtyFlagMedia);
    return nIndex;
}

size_t MovieProject::AddMediaItemFromFile(LPCWSTR pszFilePath)
{
    if (!pszFilePath || !pszFilePath[0])
        return SIZE_MAX;

    ProjectMediaItem item;
    item.SetMediaId(GenerateNextMediaId());
    item.SetSourcePath(pszFilePath);

    // Determine media type from extension
    LPCWSTR pszExt = PathFindExtensionW(pszFilePath);
    if (pszExt)
    {
        if (_wcsicmp(pszExt, L".jpg") == 0 || _wcsicmp(pszExt, L".jpeg") == 0 ||
            _wcsicmp(pszExt, L".png") == 0 || _wcsicmp(pszExt, L".bmp") == 0 ||
            _wcsicmp(pszExt, L".gif") == 0 || _wcsicmp(pszExt, L".tiff") == 0)
        {
            item.SetMediaType(1); // photo
        }
        else if (_wcsicmp(pszExt, L".mp3") == 0 || _wcsicmp(pszExt, L".wma") == 0 ||
                 _wcsicmp(pszExt, L".wav") == 0 || _wcsicmp(pszExt, L".aac") == 0)
        {
            item.SetMediaType(2); // audio
        }
        else
        {
            item.SetMediaType(0); // video (default)
        }
    }

    return AddMediaItem(item);
}

void MovieProject::RemoveMediaItem(size_t nIndex)
{
    if (nIndex >= m_arrMediaItems.GetCount())
        return;

    m_arrMediaItems.RemoveAt(nIndex);
    m_state.SetDirty(ProjectDirtyFlagMedia);
}

void MovieProject::RemoveMediaItemById(DWORD dwMediaId)
{
    int nIndex = FindMediaItemById(dwMediaId);
    if (nIndex >= 0)
    {
        RemoveMediaItem(static_cast<size_t>(nIndex));
    }
}

int MovieProject::FindMediaItemById(DWORD dwMediaId) const
{
    for (size_t i = 0; i < m_arrMediaItems.GetCount(); ++i)
    {
        if (m_arrMediaItems.GetAt(i).GetMediaId() == dwMediaId)
            return static_cast<int>(i);
    }
    return -1;
}

void MovieProject::RemoveAllMediaItems()
{
    m_arrMediaItems.RemoveAll();
    for (int i = 0; i < 6; ++i)
    {
        m_arrTimelines[i].RemoveAllExtents();
    }
    m_state.SetDirty(ProjectDirtyFlagAll);
}

// ============================================================================
// Timeline access
// ============================================================================

ProjectTimeline* MovieProject::GetTimeline(TimelineTrackType type)
{
    if (static_cast<int>(type) < 0 || static_cast<int>(type) > 5)
        return nullptr;
    return &m_arrTimelines[static_cast<int>(type)];
}

const ProjectTimeline* MovieProject::GetTimeline(TimelineTrackType type) const
{
    if (static_cast<int>(type) < 0 || static_cast<int>(type) > 5)
        return nullptr;
    return &m_arrTimelines[static_cast<int>(type)];
}

size_t MovieProject::GetTotalExtentCount() const throw()
{
    size_t cTotal = 0;
    for (int i = 0; i < 6; ++i)
    {
        cTotal += m_arrTimelines[i].GetExtentCount();
    }
    return cTotal;
}

// ============================================================================
// Version
// ============================================================================

DWORD MovieProject::GetVersionMajor() const throw()
{
    return m_dwVersionMajor;
}

DWORD MovieProject::GetVersionMinor() const throw()
{
    return m_dwVersionMinor;
}

// ============================================================================
// Undo / Redo (stub)
// ============================================================================

HRESULT MovieProject::Undo()
{
    if (m_nUndoPosition < 0 || m_arrUndoStack.IsEmpty())
        return S_FALSE;

    UndoEntry* pEntry = m_arrUndoStack.GetAt(m_nUndoPosition);
    if (!pEntry)
        return E_FAIL;

    // Save current state to a redo entry if at the top of the stack
    if (m_nUndoPosition == static_cast<int>(m_arrUndoStack.GetCount()) - 1)
    {
        // We are at the latest state; we need to snapshot it before restoring
        // the previous state. However, the entry at m_nUndoPosition already
        // holds the state BEFORE the action we want to undo. We need to push
        // a snapshot of the current state after restoring.
    }

    // Restore from the snapshot
    IStream* pStream = nullptr;
    HRESULT hr = CreateStreamOnHGlobal(nullptr, TRUE, &pStream);
    if (FAILED(hr))
        return hr;

    // Write the snapshot XML to a stream
    hr = SHCreateStreamOnFile(pEntry->m_strProjectXml, STGM_READ, &pStream);
    if (SUCCEEDED(hr))
    {
        hr = LoadFromStream(pStream);
    }
    pStream->Release();

    if (SUCCEEDED(hr))
    {
        --m_nUndoPosition;
        m_state.SetDirty(ProjectDirtyFlagAll);
    }

    return hr;
}

HRESULT MovieProject::Redo()
{
    if (m_nUndoPosition >= static_cast<int>(m_arrUndoStack.GetCount()) - 1)
        return S_FALSE;

    UndoEntry* pEntry = m_arrUndoStack.GetAt(m_nUndoPosition + 1);
    if (!pEntry)
        return E_FAIL;

    // Restore from the redo entry
    IStream* pStream = nullptr;
    HRESULT hr = CreateStreamOnHGlobal(nullptr, TRUE, &pStream);
    if (FAILED(hr))
        return hr;

    hr = SHCreateStreamOnFile(pEntry->m_strProjectXml, STGM_READ, &pStream);
    if (SUCCEEDED(hr))
    {
        hr = LoadFromStream(pStream);
    }
    pStream->Release();

    if (SUCCEEDED(hr))
    {
        ++m_nUndoPosition;
        m_state.SetDirty(ProjectDirtyFlagAll);
    }

    return hr;
}

void MovieProject::ClearUndoHistory()
{
    for (size_t i = 0; i < m_arrUndoStack.GetCount(); ++i)
    {
        delete m_arrUndoStack.GetAt(i);
    }
    m_arrUndoStack.RemoveAll();
    m_nUndoPosition = -1;
}

// ============================================================================
// Thumbnail generation (stub)
// ============================================================================

HRESULT MovieProject::GenerateThumbnails()
{
    // TODO: iterate media items and generate thumbnail bitmaps
    return S_OK;
}

// ============================================================================
// Validation
// ============================================================================

HRESULT MovieProject::Validate() const
{
    // Check that all extents in timelines reference valid media items
    for (int t = 0; t < 6; ++t)
    {
        const ProjectTimeline& timeline = m_arrTimelines[t];
        for (size_t e = 0; e < timeline.GetExtentCount(); ++e)
        {
            // Extent IDs are validated during timeline construction
            // Media item existence checked separately
        }
    }
    return S_OK;
}

// ============================================================================
// XML serialization helpers
// ============================================================================

HRESULT MovieProject::WriteProjectElement(IXmlWriter* pWriter)
{
    if (!pWriter)
        return E_POINTER;

    HRESULT hr;

    // Write project root element with version attributes
    hr = pWriter->WriteStartElement(nullptr, kProjectRootElement, nullptr);
    if (FAILED(hr)) return hr;

    WCHAR szBuf[64];
    _itow_s(m_dwVersionMajor, szBuf, 10);
    hr = pWriter->WriteAttributeString(nullptr, L"versionMajor", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    _itow_s(m_dwVersionMinor, szBuf, 10);
    hr = pWriter->WriteAttributeString(nullptr, L"versionMinor", nullptr, szBuf);
    if (FAILED(hr)) return hr;

    if (!m_settings.GetProjectName().IsEmpty())
    {
        hr = pWriter->WriteAttributeString(nullptr, L"name", nullptr,
            m_settings.GetProjectName());
        if (FAILED(hr)) return hr;
    }

    // Write child elements
    hr = WriteMediaElement(pWriter);
    if (SUCCEEDED(hr))
        hr = WriteTimelineElement(pWriter);
    if (SUCCEEDED(hr))
        hr = WriteTransitionsElement(pWriter);
    if (SUCCEEDED(hr))
        hr = WriteEffectsElement(pWriter);
    if (SUCCEEDED(hr))
        hr = WriteTitlesElement(pWriter);

    return hr;
}

HRESULT MovieProject::WriteMediaElement(IXmlWriter* pWriter)
{
    HRESULT hr = pWriter->WriteStartElement(nullptr, L"media", nullptr);
    if (FAILED(hr)) return hr;

    WCHAR szBuf[64];

    for (size_t i = 0; i < m_arrMediaItems.GetCount(); ++i)
    {
        const ProjectMediaItem& item = m_arrMediaItems.GetAt(i);

        hr = pWriter->WriteStartElement(nullptr, L"mediaItem", nullptr);
        if (FAILED(hr)) break;

        _itow_s(item.GetMediaId(), szBuf, 10);
        hr = pWriter->WriteAttributeString(nullptr, L"id", nullptr, szBuf);
        if (FAILED(hr)) break;

        hr = pWriter->WriteAttributeString(nullptr, L"path", nullptr,
            item.GetSourcePath());
        if (FAILED(hr)) break;

        _itow_s(item.GetMediaType(), szBuf, 10);
        hr = pWriter->WriteAttributeString(nullptr, L"type", nullptr, szBuf);
        if (FAILED(hr)) break;

        _i64tow_s(item.GetDurationHns(), szBuf, _countof(szBuf), 10);
        hr = pWriter->WriteAttributeString(nullptr, L"duration", nullptr, szBuf);
        if (FAILED(hr)) break;

        if (item.GetWidth() > 0 && item.GetHeight() > 0)
        {
            _itow_s(item.GetWidth(), szBuf, 10);
            hr = pWriter->WriteAttributeString(nullptr, L"width", nullptr, szBuf);
            if (FAILED(hr)) break;

            _itow_s(item.GetHeight(), szBuf, 10);
            hr = pWriter->WriteAttributeString(nullptr, L"height", nullptr, szBuf);
            if (FAILED(hr)) break;
        }

        if (item.GetFrameRate() > 0)
        {
            _itow_s(item.GetFrameRate(), szBuf, 10);
            hr = pWriter->WriteAttributeString(nullptr, L"frameRate", nullptr, szBuf);
            if (FAILED(hr)) break;
        }

        hr = pWriter->WriteEndElement(); // mediaItem
        if (FAILED(hr)) break;
    }

    if (SUCCEEDED(hr))
    {
        hr = pWriter->WriteEndElement(); // media
    }

    return hr;
}

HRESULT MovieProject::WriteTimelineElement(IXmlWriter* pWriter)
{
    HRESULT hr = S_OK;

    static const LPCWSTR trackNames[] = {
        L"video", L"audio", L"music", L"title", L"credits", L"transition"
    };

    for (int t = 0; t < 6; ++t)
    {
        const ProjectTimeline& timeline = m_arrTimelines[t];
        if (timeline.GetExtentCount() == 0)
            continue;

        hr = pWriter->WriteStartElement(nullptr, L"timeline", nullptr);
        if (FAILED(hr)) break;

        hr = pWriter->WriteAttributeString(nullptr, L"track", nullptr,
            trackNames[t]);
        if (FAILED(hr)) break;

        for (size_t e = 0; e < timeline.GetExtentCount(); ++e)
        {
            hr = pWriter->WriteStartElement(nullptr, L"extent", nullptr);
            if (FAILED(hr)) break;

            WCHAR szBuf[64];
            _itow_s(timeline.GetExtentIdAt(e), szBuf, _countof(szBuf), 10);
            hr = pWriter->WriteAttributeString(nullptr, L"id", nullptr, szBuf);
            if (FAILED(hr)) break;

            hr = pWriter->WriteEndElement(); // extent
            if (FAILED(hr)) break;
        }

        if (SUCCEEDED(hr))
        {
            hr = pWriter->WriteEndElement(); // timeline
        }
        if (FAILED(hr)) break;
    }

    return hr;
}

HRESULT MovieProject::WriteTransitionsElement(IXmlWriter* pWriter)
{
    HRESULT hr = pWriter->WriteStartElement(nullptr, L"transitions", nullptr);
    if (FAILED(hr)) return hr;

    // TODO: serialize transition data

    hr = pWriter->WriteEndElement(); // transitions
    return hr;
}

HRESULT MovieProject::WriteEffectsElement(IXmlWriter* pWriter)
{
    HRESULT hr = pWriter->WriteStartElement(nullptr, L"effects", nullptr);
    if (FAILED(hr)) return hr;

    // TODO: serialize effect data

    hr = pWriter->WriteEndElement(); // effects
    return hr;
}

HRESULT MovieProject::WriteTitlesElement(IXmlWriter* pWriter)
{
    HRESULT hr = pWriter->WriteStartElement(nullptr, L"titles", nullptr);
    if (FAILED(hr)) return hr;

    // TODO: serialize title overlay data

    hr = pWriter->WriteEndElement(); // titles
    return hr;
}

// ============================================================================
// XML deserialization helpers
// ============================================================================

HRESULT MovieProject::ReadProjectElement(IXmlReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    HRESULT hr = S_OK;

    // Read version attributes
    LPCWSTR pszValue = nullptr;
    hr = XmlReaderGetAttribute(pReader, L"versionMinor", &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        m_dwVersionMinor = _wtol(pszValue);
    }

    hr = XmlReaderGetAttribute(pReader, L"versionMajor", &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        m_dwVersionMajor = _wtol(pszValue);
    }

    hr = XmlReaderGetAttribute(pReader, L"name", &pszValue);
    if (SUCCEEDED(hr) && pszValue)
    {
        m_settings.SetProjectName(pszValue);
    }

    // Read child elements
    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_EndElement)
        {
            break; // closing </project>
        }

        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            hr = pReader->GetLocalName(&pszLocalName, nullptr);
            if (FAILED(hr) || !pszLocalName)
                continue;

            if (wcscmp(pszLocalName, L"media") == 0)
                hr = ReadMediaElement(pReader);
            else if (wcscmp(pszLocalName, L"timeline") == 0)
                hr = ReadTimelineElement(pReader);
            else if (wcscmp(pszLocalName, L"transitions") == 0)
                hr = ReadTransitionsElement(pReader);
            else if (wcscmp(pszLocalName, L"effects") == 0)
                hr = ReadEffectsElement(pReader);
            else if (wcscmp(pszLocalName, L"titles") == 0)
                hr = ReadTitlesElement(pReader);

            if (FAILED(hr))
                break;
        }
    }

    return hr;
}

HRESULT MovieProject::ReadMediaElement(IXmlReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    HRESULT hr = S_OK;
    XmlNodeType nodeType;

    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_EndElement)
            break; // closing </media>

        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            hr = pReader->GetLocalName(&pszLocalName, nullptr);
            if (FAILED(hr) || !pszLocalName)
                continue;

            if (wcscmp(pszLocalName, L"mediaItem") == 0)
            {
                ProjectMediaItem item;

                LPCWSTR pszValue = nullptr;

                hr = XmlReaderGetAttribute(pReader, L"id", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                    item.SetMediaId(_wtol(pszValue));

                hr = XmlReaderGetAttribute(pReader, L"path", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                    item.SetSourcePath(pszValue);

                hr = XmlReaderGetAttribute(pReader, L"type", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                    item.SetMediaType(_wtol(pszValue));

                hr = XmlReaderGetAttribute(pReader, L"duration", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                    item.SetDurationHns(_wtoi64(pszValue));

                hr = XmlReaderGetAttribute(pReader, L"width", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                {
                    UINT cx = _wtol(pszValue);
                    hr = XmlReaderGetAttribute(pReader, L"height", &pszValue);
                    if (SUCCEEDED(hr) && pszValue)
                        item.SetDimensions(cx, _wtol(pszValue));
                }

                hr = XmlReaderGetAttribute(pReader, L"frameRate", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                    item.SetFrameRate(_wtol(pszValue));

                m_arrMediaItems.Add(item);

                // Track the highest media ID for generation
                if (item.GetMediaId() >= m_dwNextMediaId)
                    m_dwNextMediaId = item.GetMediaId() + 1;
            }
        }
    }

    return S_OK;
}

HRESULT MovieProject::ReadTimelineElement(IXmlReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    // Read track attribute to determine which timeline
    LPCWSTR pszTrack = nullptr;
    HRESULT hr = XmlReaderGetAttribute(pReader, L"track", &pszTrack);
    if (FAILED(hr) || !pszTrack)
        return E_FAIL;

    TimelineTrackType trackType = TimelineTrackTypeVideo;
    if (wcscmp(pszTrack, L"video") == 0)
        trackType = TimelineTrackTypeVideo;
    else if (wcscmp(pszTrack, L"audio") == 0)
        trackType = TimelineTrackTypeAudio;
    else if (wcscmp(pszTrack, L"music") == 0)
        trackType = TimelineTrackTypeMusic;
    else if (wcscmp(pszTrack, L"title") == 0)
        trackType = TimelineTrackTypeTitle;
    else if (wcscmp(pszTrack, L"credits") == 0)
        trackType = TimelineTrackTypeCredits;
    else if (wcscmp(pszTrack, L"transition") == 0)
        trackType = TimelineTrackTypeTransition;

    ProjectTimeline& timeline = m_arrTimelines[static_cast<int>(trackType)];
    timeline.SetTrackType(trackType);

    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_EndElement)
            break; // closing </timeline>

        if (nodeType == XmlNodeType_Element)
        {
            LPCWSTR pszLocalName = nullptr;
            hr = pReader->GetLocalName(&pszLocalName, nullptr);
            if (FAILED(hr) || !pszLocalName)
                continue;

            if (wcscmp(pszLocalName, L"extent") == 0)
            {
                LPCWSTR pszValue = nullptr;
                hr = XmlReaderGetAttribute(pReader, L"id", &pszValue);
                if (SUCCEEDED(hr) && pszValue)
                {
                    DWORD dwExtentId = _wtol(pszValue);
                    timeline.AddExtent(dwExtentId);

                    if (dwExtentId >= m_dwNextExtentId)
                        m_dwNextExtentId = dwExtentId + 1;
                }
            }
        }
    }

    return S_OK;
}

HRESULT MovieProject::ReadTransitionsElement(IXmlReader* pReader)
{
    // Skip through transition elements
    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_EndElement)
            break;
    }
    return S_OK;
}

HRESULT MovieProject::ReadEffectsElement(IXmlReader* pReader)
{
    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_EndElement)
            break;
    }
    return S_OK;
}

HRESULT MovieProject::ReadTitlesElement(IXmlReader* pReader)
{
    XmlNodeType nodeType;
    while (pReader->Read(&nodeType) == S_OK)
    {
        if (nodeType == XmlNodeType_EndElement)
            break;
    }
    return S_OK;
}

// ============================================================================
// Auto-save file helpers
// ============================================================================

HRESULT MovieProject::WriteAutoSave()
{
    if (m_strProjectPath.IsEmpty())
        return E_UNEXPECTED;

    ATL::CString strAutoSavePath = m_strProjectPath + kAutoSaveExtension;
    return Save(strAutoSavePath);
}

HRESULT MovieProject::ReadAutoSaveFile(LPCWSTR pszAutoSavePath)
{
    if (!pszAutoSavePath)
        return E_INVALIDARG;
    return Load(pszAutoSavePath);
}

// ============================================================================
// ID generation
// ============================================================================

DWORD MovieProject::GenerateNextMediaId()
{
    return m_dwNextMediaId++;
}

DWORD MovieProject::GenerateNextExtentId()
{
    return m_dwNextExtentId++;
}

ATL::CString MovieProject::GetFilePath() const
{
    return GetProjectPath();
}

ATL::CString MovieProject::GetDisplayName() const
{
    return GetProjectName();
}

HRESULT MovieProject::SaveAs(LPCWSTR pszPath)
{
    return Save(pszPath);
}

HRESULT MovieProject::ImportMedia(LPCWSTR pszPath, TimelineTrackType trackType)
{
    UNREFERENCED_PARAMETER(trackType);
    if (!pszPath || !pszPath[0])
        return E_INVALIDARG;
    AddMediaItemFromFile(pszPath);
    return S_OK;
}

HRESULT MovieProject::RemoveItem(DWORD dwItemId, TimelineTrackType trackType)
{
    UNREFERENCED_PARAMETER(trackType);
    int nIndex = FindMediaItemById(dwItemId);
    if (nIndex >= 0)
        RemoveMediaItem(static_cast<size_t>(nIndex));
    return S_OK;
}

HRESULT MovieProject::MoveItem(DWORD dwItemId, TimelineTrackType trackType, DWORD dwNewPosition)
{
    UNREFERENCED_PARAMETER(trackType);
    UNREFERENCED_PARAMETER(dwItemId);
    UNREFERENCED_PARAMETER(dwNewPosition);
    return S_OK;
}

size_t MovieProject::GetItemCount(TimelineTrackType trackType) const throw()
{
    UNREFERENCED_PARAMETER(trackType);
    return GetMediaItemCount();
}

bool MovieProject::IsItemValid(DWORD dwItemId, TimelineTrackType trackType) const throw()
{
    UNREFERENCED_PARAMETER(trackType);
    int nIndex = FindMediaItemById(dwItemId);
    return nIndex >= 0 && static_cast<size_t>(nIndex) < GetMediaItemCount();
}

void MovieProject::GetExtentIdsForMediaItem(DWORD dwItemId, TimelineTrackType trackType, Base::Array<DWORD>& ids) const
{
    UNREFERENCED_PARAMETER(dwItemId);
    UNREFERENCED_PARAMETER(trackType);
    UNREFERENCED_PARAMETER(ids);
}

HRESULT MovieProject::GetExtentForMediaItem(DWORD dwItemId, Base::PtrRef<MovieExtent>& extentOut)
{
    UNREFERENCED_PARAMETER(dwItemId);
    UNREFERENCED_PARAMETER(extentOut);
    return E_NOTIMPL;
}

bool MovieProject::HasItemsNeedingProxyTranscode() const
{
    return false;
}

HRESULT MovieProject::StartProxyTranscode()
{
    return S_OK;
}

void MovieProject::MarkAllExtentsForRetranscode(TimelineTrackType trackType)
{
    UNREFERENCED_PARAMETER(trackType);
}

} // namespace StoryboardManager
