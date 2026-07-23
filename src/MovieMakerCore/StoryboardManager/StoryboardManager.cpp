#include "pch.h"
/*
 * StoryboardManager.cpp
 *
 * Implementation of the StoryboardManager namespace initialization and
 * global state management for MovieMakerCore.dll.
 *
 * The StoryboardManager manages the entire project model, timeline extents,
 * templates/themes, and .wlmp file serialization. This file provides the
 * subsystem lifecycle (initialize/shutdown) and global state.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "StoryboardManager.h"

// ============================================================================
// Internal state
// ============================================================================
namespace
{
    bool g_fStoryboardInitialized = false;

    // Global template table, created once during initialization
    StoryboardManager::TemplateTable* g_pGlobalTemplateTable = nullptr;

    // Cached CLSID for the X3D transitions effect
    CLSID g_clsidX3dTransition = { 0 };
    bool g_fX3dTransitionCLSIDResolved = false;
}

// ============================================================================
// StoryboardManager namespace implementation
// ============================================================================
namespace StoryboardManager
{

// ============================================================================
// StoryboardManagerInitialize
// ============================================================================
STORYBOARD_API HRESULT StoryboardManagerInitialize()
{
    if (g_fStoryboardInitialized)
        return S_FALSE; // already initialized

    // Create and initialize the global template table
    if (!g_pGlobalTemplateTable)
    {
        g_pGlobalTemplateTable = new TemplateTable();
        g_pGlobalTemplateTable->InitializeDefaults();
    }

    g_fStoryboardInitialized = true;

    return S_OK;
}

// ============================================================================
// StoryboardManagerShutdown
// ============================================================================
STORYBOARD_API void StoryboardManagerShutdown()
{
    if (!g_fStoryboardInitialized)
        return;

    if (g_pGlobalTemplateTable)
    {
        delete g_pGlobalTemplateTable;
        g_pGlobalTemplateTable = nullptr;
    }

    g_fStoryboardInitialized = false;
}

// ============================================================================
// StoryboardManagerIsInitialized
// ============================================================================
STORYBOARD_API bool StoryboardManagerIsInitialized()
{
    return g_fStoryboardInitialized;
}

// ============================================================================
// TemplateTable global access
// ============================================================================

TemplateTable* GetGlobalTemplateTable()
{
    return g_pGlobalTemplateTable;
}

void SetGlobalTemplateTable(TemplateTable* pTable)
{
    if (g_pGlobalTemplateTable && g_pGlobalTemplateTable != pTable)
    {
        delete g_pGlobalTemplateTable;
    }
    g_pGlobalTemplateTable = pTable;
}

// ============================================================================
// ExtentId implementation
// ============================================================================

ExtentId::ExtentId()
    : m_dwId(0)
{
}

ExtentId::ExtentId(DWORD dwId)
    : m_dwId(dwId)
{
}

ExtentId::ExtentId(const ExtentId& other)
    : m_dwId(other.m_dwId)
{
}

ExtentId& ExtentId::operator=(const ExtentId& other)
{
    if (this != &other)
    {
        m_dwId = other.m_dwId;
    }
    return *this;
}

bool ExtentId::operator==(const ExtentId& other) const
{
    return m_dwId == other.m_dwId;
}

bool ExtentId::operator!=(const ExtentId& other) const
{
    return m_dwId != other.m_dwId;
}

bool ExtentId::operator<(const ExtentId& other) const
{
    return m_dwId < other.m_dwId;
}

DWORD ExtentId::GetValue() const throw()
{
    return m_dwId;
}

bool ExtentId::IsValid() const throw()
{
    return m_dwId != 0;
}

ExtentId ExtentId::Generate()
{
    static DWORD s_dwNextId = 1;
    return ExtentId(InterlockedIncrement(reinterpret_cast<volatile LONG*>(&s_dwNextId)));
}

// ============================================================================
// TranscodeState implementation
// ============================================================================

TranscodeState::TranscodeState()
    : m_state(ExtentTranscodeStateNone)
    , m_hresult(S_OK)
{
}

TranscodeState::TranscodeState(ExtentTranscodeState initialState)
    : m_state(initialState)
    , m_hresult(S_OK)
{
}

TranscodeState::~TranscodeState()
{
}

ExtentTranscodeState TranscodeState::GetState() const throw()
{
    return m_state;
}

void TranscodeState::SetState(ExtentTranscodeState state) throw()
{
    m_state = state;
}

HRESULT TranscodeState::GetHResult() const throw()
{
    return m_hresult;
}

void TranscodeState::SetHResult(HRESULT hr) throw()
{
    m_hresult = hr;
}

bool TranscodeState::IsComplete() const throw()
{
    return m_state == ExtentTranscodeStateComplete;
}

bool TranscodeState::IsFailed() const throw()
{
    return m_state == ExtentTranscodeStateFailed;
}

bool TranscodeState::IsInProgress() const throw()
{
    return m_state == ExtentTranscodeStateInProgress;
}

// ============================================================================
// StoryboardManager class implementation
// ============================================================================

StoryboardManager::StoryboardManager()
    : m_pCurrentProject(nullptr)
    , m_pCurrentTrack(nullptr)
    , m_pTemplateTable(g_pGlobalTemplateTable)
{
}

StoryboardManager::~StoryboardManager()
{
    delete m_pCurrentProject;
    m_pCurrentProject = nullptr;
    m_pCurrentTrack = nullptr;
}

StoryboardManager& StoryboardManager::GetInstance()
{
    static StoryboardManager s_instance;
    return s_instance;
}

MovieProject* StoryboardManager::NewProject()
{
    delete m_pCurrentProject;
    m_pCurrentProject = MovieProject::CreateEmpty();
    m_pCurrentTrack = nullptr;
    return m_pCurrentProject;
}

HRESULT StoryboardManager::OpenProject(LPCWSTR pszPath)
{
    if (!pszPath || !pszPath[0])
        return E_INVALIDARG;

    MovieProject* pProject = MovieProject::CreateEmpty();
    HRESULT hr = pProject->Load(pszPath);
    if (FAILED(hr))
    {
        delete pProject;
        return hr;
    }

    delete m_pCurrentProject;
    m_pCurrentProject = pProject;
    m_pCurrentTrack = nullptr;
    return S_OK;
}

HRESULT StoryboardManager::SaveProject(LPCWSTR pszPath)
{
    if (!m_pCurrentProject)
        return E_UNEXPECTED;

    return m_pCurrentProject->Save(pszPath);
}

MovieProject* StoryboardManager::GetProject() const
{
    return m_pCurrentProject;
}

bool StoryboardManager::CanUndo() const
{
    return m_pCurrentProject && m_pCurrentProject->GetState().CanUndo();
}

bool StoryboardManager::CanRedo() const
{
    return m_pCurrentProject && m_pCurrentProject->GetState().CanRedo();
}

HRESULT StoryboardManager::Undo()
{
    if (!m_pCurrentProject)
        return E_UNEXPECTED;
    return m_pCurrentProject->Undo();
}

HRESULT StoryboardManager::Redo()
{
    if (!m_pCurrentProject)
        return E_UNEXPECTED;
    return m_pCurrentProject->Redo();
}

TimelineTrack* StoryboardManager::GetCurrentTrack() const
{
    return m_pCurrentTrack;
}

void StoryboardManager::SetCurrentTrack(TimelineTrack* pTrack)
{
    m_pCurrentTrack = pTrack;
}

HRESULT StoryboardManager::AddTrack(TimelineTrackType trackType, TimelineTrack** ppTrack)
{
    if (!m_pCurrentProject)
        return E_UNEXPECTED;

    if (trackType == TimelineTrackTypeUnknown)
        return E_INVALIDARG;

    ProjectTimeline* pTimeline = m_pCurrentProject->GetTimeline(trackType);
    if (!pTimeline)
        return E_FAIL;

    TimelineTrack* pTrack = new TimelineTrack(trackType);
    pTrack->SetDisplayName(TimelineTrack::GetDefaultName(trackType));

    pTimeline->SetTrackType(trackType);

    m_pCurrentProject->GetState().SetDirty(ProjectDirtyFlagTimeline);

    if (ppTrack)
        *ppTrack = pTrack;

    return S_OK;
}

HRESULT StoryboardManager::RemoveTrack(TimelineTrack* pTrack)
{
    if (!pTrack)
        return E_POINTER;

    if (!m_pCurrentProject)
        return E_UNEXPECTED;

    TimelineTrackType type = pTrack->GetTrackType();
    ProjectTimeline* pTimeline = m_pCurrentProject->GetTimeline(type);
    if (!pTimeline)
        return E_FAIL;

    pTimeline->RemoveAllExtents();

    if (m_pCurrentTrack == pTrack)
        m_pCurrentTrack = nullptr;

    m_pCurrentProject->GetState().SetDirty(ProjectDirtyFlagTimeline);

    delete pTrack;

    return S_OK;
}

HRESULT StoryboardManager::RemoveTrackAt(size_t nIndex)
{
    if (!m_pCurrentProject)
        return E_UNEXPECTED;

    TimelineTrackType types[] = {
        TimelineTrackTypeVideo,
        TimelineTrackTypeAudio,
        TimelineTrackTypeMusic,
        TimelineTrackTypeTitle,
        TimelineTrackTypeCredits,
        TimelineTrackTypeTransition
    };

    if (nIndex >= _countof(types))
        return E_INVALIDARG;

    ProjectTimeline* pTimeline = m_pCurrentProject->GetTimeline(types[nIndex]);
    if (!pTimeline)
        return E_FAIL;

    pTimeline->RemoveAllExtents();
    m_pCurrentProject->GetState().SetDirty(ProjectDirtyFlagTimeline);

    return S_OK;
}

size_t StoryboardManager::GetTrackCount() const
{
    if (!m_pCurrentProject)
        return 0;

    size_t count = 0;
    TimelineTrackType types[] = {
        TimelineTrackTypeVideo,
        TimelineTrackTypeAudio,
        TimelineTrackTypeMusic,
        TimelineTrackTypeTitle,
        TimelineTrackTypeCredits,
        TimelineTrackTypeTransition
    };

    for (size_t i = 0; i < _countof(types); i++)
    {
        const ProjectTimeline* pTimeline = m_pCurrentProject->GetTimeline(types[i]);
        if (pTimeline && pTimeline->GetExtentCount() > 0)
            count++;
    }

    return count;
}

TimelineTrack* StoryboardManager::GetTrackAt(size_t nIndex) const
{
    if (!m_pCurrentProject)
        return nullptr;

    TimelineTrackType types[] = {
        TimelineTrackTypeVideo,
        TimelineTrackTypeAudio,
        TimelineTrackTypeMusic,
        TimelineTrackTypeTitle,
        TimelineTrackTypeCredits,
        TimelineTrackTypeTransition
    };

    size_t activeIndex = 0;
    for (size_t i = 0; i < _countof(types); i++)
    {
        const ProjectTimeline* pTimeline = m_pCurrentProject->GetTimeline(types[i]);
        if (pTimeline && pTimeline->GetExtentCount() > 0)
        {
            if (activeIndex == nIndex)
            {
                TimelineTrack* pTrack = new TimelineTrack(types[i]);
                pTrack->SetDisplayName(TimelineTrack::GetDefaultName(types[i]));
                return pTrack;
            }
            activeIndex++;
        }
    }

    return nullptr;
}

TimelineTrack* StoryboardManager::FindTrack(TimelineTrackType trackType) const
{
    if (!m_pCurrentProject)
        return nullptr;

    if (trackType == TimelineTrackTypeUnknown)
        return nullptr;

    const ProjectTimeline* pTimeline = m_pCurrentProject->GetTimeline(trackType);
    if (!pTimeline)
        return nullptr;

    TimelineTrack* pTrack = new TimelineTrack(trackType);
    pTrack->SetDisplayName(TimelineTrack::GetDefaultName(trackType));
    return pTrack;
}

HRESULT StoryboardManager::ValidateProject() const
{
    if (!m_pCurrentProject)
        return E_UNEXPECTED;

    return m_pCurrentProject->Validate();
}

LONGLONG StoryboardManager::GetTotalDurationHns() const
{
    if (!m_pCurrentProject)
        return 0;

    LONGLONG llMaxDuration = 0;

    TimelineTrackType types[] = {
        TimelineTrackTypeVideo,
        TimelineTrackTypeAudio,
        TimelineTrackTypeMusic,
        TimelineTrackTypeTitle,
        TimelineTrackTypeCredits,
        TimelineTrackTypeTransition
    };

    for (size_t i = 0; i < _countof(types); i++)
    {
        const ProjectTimeline* pTimeline = m_pCurrentProject->GetTimeline(types[i]);
        if (pTimeline)
        {
            LONGLONG llDuration = pTimeline->GetTotalDurationHns();
            if (llDuration > llMaxDuration)
                llMaxDuration = llDuration;
        }
    }

    return llMaxDuration;
}

TemplateTable* StoryboardManager::GetTemplateTable()
{
    return m_pTemplateTable;
}

} // namespace StoryboardManager
