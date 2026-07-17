#pragma once
#ifndef SUNDANCE_IMPORT_CONTROLLER_H
#define SUNDANCE_IMPORT_CONTROLLER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class ImportController
{
public:
    ImportController();
    ~ImportController();

    HRESULT ImportFiles(int cFiles, LPCWSTR* ppszFiles);
    float GetImportProgress() const throw();
    bool IsImporting() const throw();
    int GetImportedCount() const throw();

private:
    ImportController(const ImportController&);
    ImportController& operator=(const ImportController&);

    bool  m_bImporting;
    float m_flProgress;
    int   m_cImported;
};

#endif
