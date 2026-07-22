#include "pch.h"
#include "ImportController.h"
#include "ProjectManager.h"
#include "SundanceAppMain.h"

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

    // Obtain the active project via the application singleton so that
    // validated files are actually added to the timeline.
    SundanceAppMain* pApp = GetSundanceAppMain();
    StoryboardManagerNamespace::MovieProject* pProject =
        pApp ? pApp->GetProject() : NULL;

    for (int i = 0; i < cFiles; ++i)
    {
        if (ppszFiles[i] && ppszFiles[i][0])
        {
            DWORD dwAttr = ::GetFileAttributesW(ppszFiles[i]);
            if (dwAttr != INVALID_FILE_ATTRIBUTES &&
                !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
            {
                // File exists and is not a directory — add it to the project
                if (pProject)
                {
                    HRESULT hrAdd = pProject->ImportMedia(
                        ppszFiles[i],
                        static_cast<StoryboardManager::TimelineTrackType>(
                            TimelineTrack_Video));
                    if (SUCCEEDED(hrAdd))
                        ++m_cImported;
                }
                else
                {
                    // No project open; just count the valid file
                    ++m_cImported;
                }
            }
        }

        m_flProgress = static_cast<float>(i + 1) / static_cast<float>(cFiles);
    }

    m_bImporting = false;
    m_flProgress = 1.0f;
    return (m_cImported > 0) ? S_OK : S_FALSE;
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

HRESULT ImportController::ImportMedia(ProjectManager* pProjectManager)
{
    if (!pProjectManager)
        return E_INVALIDARG;

    WCHAR szFilePath[MAX_PATH] = { 0 };

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = nullptr;
    ofn.lpstrFilter  = L"Media Files\0*.wmv;*.wma;*.avi;*.mp3;*.mp4;*.jpg;*.jpeg;*.png;*.bmp;*.gif;*.tiff\0"
                       L"Video Files\0*.wmv;*.avi;*.mp4\0"
                       L"Audio Files\0*.wma;*.mp3\0"
                       L"Image Files\0*.jpg;*.jpeg;*.png;*.bmp;*.gif;*.tiff\0"
                       L"All Files\0*.*\0";
    ofn.lpstrFile    = szFilePath;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrTitle   = L"Import Media";
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    if (!::GetOpenFileNameW(&ofn))
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);

    HRESULT hr = S_OK;
    int cImported = 0;

    if (szFilePath[0] != L'\0')
    {
        LPCWSTR pszAfterDir = szFilePath + wcslen(szFilePath) + 1;
        if (*pszAfterDir != L'\0')
        {
            // Multi-select: szFilePath contains "directory\0file1\0file2\0\0"
            LPCWSTR pszDir = szFilePath;
            LPCWSTR pszFile = pszAfterDir;

            while (*pszFile)
            {
                WCHAR szFull[MAX_PATH];
                if (SUCCEEDED(::StringCchPrintfW(szFull, MAX_PATH, L"%s\\%s", pszDir, pszFile)))
                {
                    pProjectManager->AddMediaItemFromFile(szFull);
                    ++cImported;
                }
                pszFile += wcslen(pszFile) + 1;
            }
        }
        else
        {
            // Single file selected
            pProjectManager->AddMediaItemFromFile(szFilePath);
            ++cImported;
        }
    }

    m_cImported = cImported;
    return (cImported > 0) ? S_OK : S_FALSE;
}
