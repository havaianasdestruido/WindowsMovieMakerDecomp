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

private:
    ImportController(const ImportController&);
    ImportController& operator=(const ImportController&);
};

#endif
