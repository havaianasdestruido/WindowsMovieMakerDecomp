#include "pch.h"
#include "ProjectManager.h"

ProjectManager::ProjectManager()
    : m_bDirty(false)
{
}

ProjectManager::~ProjectManager() {}

HRESULT ProjectManager::NewProject()
{
    m_strProjectPath.Empty();
    m_bDirty = false;
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

    m_strProjectPath = pszProjectPath;
    m_bDirty = false;
    return S_OK;
}

HRESULT ProjectManager::SaveProject(LPCWSTR pszProjectPath)
{
    if (!pszProjectPath || !pszProjectPath[0])
        return E_INVALIDARG;

    m_strProjectPath = pszProjectPath;
    m_bDirty = false;
    return S_OK;
}

HRESULT ProjectManager::SaveProject()
{
    if (m_strProjectPath.IsEmpty())
        return E_UNEXPECTED;

    m_bDirty = false;
    return S_OK;
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
