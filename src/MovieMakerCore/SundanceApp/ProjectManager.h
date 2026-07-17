#pragma once
#ifndef SUNDANCE_PROJECT_MANAGER_H
#define SUNDANCE_PROJECT_MANAGER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

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
    LPCWSTR GetCurrentProjectPath() const throw();
    bool HasProject() const throw();

private:
    ProjectManager(const ProjectManager&);
    ProjectManager& operator=(const ProjectManager&);

    ATL::CString m_strProjectPath;
    bool         m_bDirty;
};

#endif
