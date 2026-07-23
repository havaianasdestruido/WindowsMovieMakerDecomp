#include "pch.h"
#include "ProjectManager.h"
#include "../StoryboardManager/Serialization/SerializationWriter.h"

ProjectManager::ProjectManager()
    : m_bDirty(false)
    , m_dwLastSaveTime(0)
    , m_pActiveProject(nullptr)
{
}

ProjectManager::~ProjectManager()
{
    delete m_pActiveProject;
    m_pActiveProject = nullptr;
}

HRESULT ProjectManager::NewProject()
{
    // Free any existing project first
    delete m_pActiveProject;

    m_pActiveProject = StoryboardManager::MovieProject::CreateEmpty();
    if (!m_pActiveProject)
        return E_OUTOFMEMORY;

    m_strProjectPath.Empty();
    m_bDirty = false;
    m_dwLastSaveTime = ::GetTickCount();

    return S_OK;
}

HRESULT ProjectManager::OpenProject(LPCWSTR pszProjectPath)
{
    if (!pszProjectPath || !pszProjectPath[0])
        return E_INVALIDARG;

    DWORD dwAttr = ::GetFileAttributesW(pszProjectPath);
    if (dwAttr == INVALID_FILE_ATTRIBUTES)
        return HRESULT_FROM_WIN32(::GetLastError());

    if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
        return E_INVALIDARG;

    // Load the .wlmp project file through the MovieProject serializer
    if (!m_pActiveProject)
    {
        m_pActiveProject = StoryboardManager::MovieProject::CreateEmpty();
        if (!m_pActiveProject)
            return E_OUTOFMEMORY;
    }

    HRESULT hr = m_pActiveProject->Load(pszProjectPath);
    if (FAILED(hr))
    {
        delete m_pActiveProject;
        m_pActiveProject = nullptr;
        return hr;
    }

    m_strProjectPath = pszProjectPath;
    m_bDirty = false;
    m_dwLastSaveTime = ::GetTickCount();
    return S_OK;
}

HRESULT ProjectManager::SaveProject(LPCWSTR pszProjectPath)
{
    if (!pszProjectPath || !pszProjectPath[0])
        return E_INVALIDARG;

    if (!m_pActiveProject)
        return E_UNEXPECTED;

    // Ensure the directory exists before writing
    WCHAR szDir[MAX_PATH] = { 0 };
    ::StringCchCopyW(szDir, MAX_PATH, pszProjectPath);
    PathRemoveFileSpecW(szDir);
    ::CreateDirectoryW(szDir, NULL);

    HRESULT hr = m_pActiveProject->SaveAs(pszProjectPath);
    if (SUCCEEDED(hr))
    {
        m_strProjectPath = pszProjectPath;
        m_bDirty = false;
        m_dwLastSaveTime = ::GetTickCount();
    }
    return hr;
}

HRESULT ProjectManager::SaveProject()
{
    if (m_strProjectPath.IsEmpty())
        return E_UNEXPECTED;

    if (!m_pActiveProject)
        return E_UNEXPECTED;

    HRESULT hr = m_pActiveProject->Save(m_strProjectPath);
    if (SUCCEEDED(hr))
    {
        m_bDirty = false;
        m_dwLastSaveTime = ::GetTickCount();
    }
    return hr;
}

bool ProjectManager::IsDirty() const throw()
{
    return m_bDirty;
}

void ProjectManager::SetDirty(bool bDirty) throw()
{
    m_bDirty = bDirty;
}

LPCWSTR ProjectManager::GetCurrentProjectPath() const throw()
{
    return m_strProjectPath;
}

bool ProjectManager::HasProject() const throw()
{
    return !m_strProjectPath.IsEmpty();
}

StoryboardManager::MovieProject* ProjectManager::GetActiveProject() throw()
{
    return m_pActiveProject;
}

HRESULT ProjectManager::SetActiveProject(StoryboardManager::MovieProject* pProject)
{
    delete m_pActiveProject;
    m_pActiveProject = pProject;
    return S_OK;
}

HRESULT ProjectManager::AddMediaItem(const StoryboardManager::ProjectMediaItem& item)
{
    if (!m_pActiveProject)
        return E_UNEXPECTED;

    m_pActiveProject->AddMediaItem(item);
    m_bDirty = true;
    return S_OK;
}

HRESULT ProjectManager::AddMediaItemFromFile(LPCWSTR pszFilePath)
{
    if (!m_pActiveProject)
        return E_UNEXPECTED;

    m_pActiveProject->AddMediaItemFromFile(pszFilePath);
    m_bDirty = true;
    return S_OK;
}

// ============================================================================
// ClearDirty
//
// Resets the dirty flag. Called after a successful save or when the
// project state is known to be clean.
// ============================================================================
void ProjectManager::ClearDirty() throw()
{
    m_bDirty = false;
}

// ============================================================================
// GetDirtyStatus
//
// Returns a human-readable string describing the current dirty state.
// Used by the UI to display the unsaved-changes indicator in the
// title bar and status bar.
// ============================================================================
ATL::CString ProjectManager::GetDirtyStatus() const throw()
{
    if (m_bDirty)
        return ATL::CString(L"Unsaved changes");
    else if (HasProject())
        return ATL::CString(L"Saved");
    else
        return ATL::CString(L"No project");
}

// ============================================================================
// SaveAuto
//
// Performs an automatic save to the current project path. Called by
// CheckAutoSave when the auto-save interval has elapsed. If no path
// has been set, saves to the recovery directory instead.
// ============================================================================
HRESULT ProjectManager::SaveAuto()
{
    if (!m_pActiveProject)
        return E_UNEXPECTED;

    if (m_strProjectPath.IsEmpty())
    {
        // No explicit path yet — save to the auto-save recovery location
        WCHAR szLocalAppData[MAX_PATH] = { 0 };
        HRESULT hr = ::SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szLocalAppData);
        if (FAILED(hr))
            return hr;

        ATL::CString strAutoSavePath;
        strAutoSavePath.Format(
            L"%s\\Microsoft\\Windows Live\\Movie Maker\\AutoSave\\autosave.wlmp",
            szLocalAppData);

        // Ensure directory exists
        ATL::CString strDir = strAutoSavePath.Left(strAutoSavePath.ReverseFind(L'\\'));
        ::CreateDirectoryW(strDir, NULL);

        hr = m_pActiveProject->SaveAs(strAutoSavePath);
        if (SUCCEEDED(hr))
        {
            m_dwLastSaveTime = ::GetTickCount();
        }
        return hr;
    }

    // Save to the current project path
    HRESULT hr = m_pActiveProject->Save();
    if (SUCCEEDED(hr))
    {
        m_bDirty = false;
        m_dwLastSaveTime = ::GetTickCount();
    }
    return hr;
}

// ============================================================================
// CheckAutoSave
//
// Called periodically (e.g. from the auto-save timer) to determine
// whether enough time has elapsed since the last save to warrant
// another automatic save. If so, calls SaveAuto.
// ============================================================================
void ProjectManager::CheckAutoSave()
{
    if (m_strProjectPath.IsEmpty() && !m_pActiveProject)
        return;

    DWORD dwNow = ::GetTickCount();
    DWORD dwElapsed = dwNow - m_dwLastSaveTime;

    if (dwElapsed >= kAutoSaveIntervalMs)
    {
        SaveAuto();
    }
}
