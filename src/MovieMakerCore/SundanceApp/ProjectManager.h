#pragma once
#ifndef SUNDANCE_PROJECT_MANAGER_H
#define SUNDANCE_PROJECT_MANAGER_H

#include "../pch.h"
#include "../MovieMakerCore.h"
#include "../StoryboardManager/MovieProject.h"
#include "../StoryboardManager/Serialization/SerializationWriter.h"
#include "../StoryboardManager/Serialization/SerializationReader.h"

class ProjectManager
{
public:
    ProjectManager();
    ~ProjectManager();

    HRESULT NewProject();
    HRESULT OpenProject(LPCWSTR pszProjectPath);
    HRESULT SaveProject(LPCWSTR pszProjectPath);
    HRESULT SaveProject();
    bool IsDirty() const throw();
    void SetDirty(bool bDirty) throw();
    void ClearDirty() throw();
    ATL::CString GetDirtyStatus() const throw();
    LPCWSTR GetCurrentProjectPath() const throw();
    bool HasProject() const throw();

    HRESULT SaveAuto();
    void    CheckAutoSave();

    StoryboardManager::MovieProject* GetActiveProject() throw();
    HRESULT SetActiveProject(StoryboardManager::MovieProject* pProject);
    HRESULT AddMediaItem(const StoryboardManager::ProjectMediaItem& item);
    HRESULT AddMediaItemFromFile(LPCWSTR pszFilePath);

private:
    static const DWORD kAutoSaveIntervalMs = 300000; // 5 minutes

    ProjectManager(const ProjectManager&);
    ProjectManager& operator=(const ProjectManager&);

    ATL::CString m_strProjectPath;
    bool         m_bDirty;
    DWORD        m_dwLastSaveTime;
    StoryboardManager::MovieProject* m_pActiveProject;
};

#endif
