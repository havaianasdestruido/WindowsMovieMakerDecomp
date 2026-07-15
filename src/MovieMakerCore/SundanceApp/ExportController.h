#pragma once
#ifndef SUNDANCE_EXPORT_CONTROLLER_H
#define SUNDANCE_EXPORT_CONTROLLER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class ExportController
{
public:
    ExportController();
    ~ExportController();

    HRESULT Publish(LPCWSTR pszOutputPath, DWORD dwProfileIndex);
    HRESULT PublishToService(LPCWSTR pszServiceName);

private:
    ExportController(const ExportController&);
    ExportController& operator=(const ExportController&);
};

#endif
