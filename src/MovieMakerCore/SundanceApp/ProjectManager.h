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

private:
    ProjectManager(const ProjectManager&);
    ProjectManager& operator=(const ProjectManager&);
};

#endif
