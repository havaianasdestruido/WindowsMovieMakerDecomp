#include "pch.h"
#include "ImportController.h"

ImportController::ImportController()
    : m_bImporting(false)
    , m_flProgress(0.0f)
    , m_cImported(0)
{
}

ImportController::~ImportController() {}

HRESULT ImportController::ImportFiles(int cFiles, LPCWSTR* ppszFiles)
{
    if (cFiles <= 0)
        return E_INVALIDARG;
    if (!ppszFiles)
        return E_INVALIDARG;

    m_bImporting = true;
    m_flProgress = 0.0f;
    m_cImported = 0;

    for (int i = 0; i < cFiles; ++i)
    {
        if (ppszFiles[i] && ppszFiles[i][0])
        {
            DWORD dwAttr = ::GetFileAttributesW(ppszFiles[i]);
            if (dwAttr != INVALID_FILE_ATTRIBUTES &&
                !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
            {
                ++m_cImported;
            }
        }

        m_flProgress = static_cast<float>(i + 1) / static_cast<float>(cFiles);
    }

    m_bImporting = false;
    m_flProgress = 1.0f;
    return S_OK;
}

float ImportController::GetImportProgress() const throw()
{
    return m_flProgress;
}

bool ImportController::IsImporting() const throw()
{
    return m_bImporting;
}

int ImportController::GetImportedCount() const throw()
{
    return m_cImported;
}
