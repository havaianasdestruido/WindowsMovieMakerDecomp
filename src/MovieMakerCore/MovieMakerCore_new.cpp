/*
 * MovieMakerCore.cpp
 *
 * Implementation of MovieMakerMain -- the sole exported function from
 * MovieMakerCore.dll. This is the entry point for the Sundance application
 * framework (Windows Live Movie Maker 2012).
 *
 * The function performs:
 *   1. Command-line parsing
 *   2. Single-instance validation
 *   3. Subsystem initialization (COM, GDI+, MF, D3D, etc.)
 *   4. Sundance application bootstrap and run
 *   5. Cleanup and shutdown
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"

// ============================================================================
// External initialization/shutdown from dllmain.cpp
// ============================================================================
extern "C"
{
    BOOL MovieCore_Initialize(HINSTANCE hInstance);
    void MovieCore_Shutdown(void);
    HINSTANCE MovieCore_GetInstance(void);
    ATL::CComModule& MovieCore_GetModule(void);
}

// ============================================================================
// Sundance forward declarations
// ============================================================================

#include "SundanceApp/SundanceAppMain.h"
#include "StoryboardManager/MovieProject.h"

namespace Sundance
{

// ============================================================================
// AppCommandIds -- accelerator and WM_COMMAND identifiers
// ============================================================================
enum AppCommandIds
{
    ID_APP_IMPORT     = 0xE101,
    ID_APP_SAVE       = 0xE103,
    ID_APP_UNDO       = 0xE12B,
    ID_APP_REDO       = 0xE12C,
    ID_APP_COPY       = 0xE122,
    ID_APP_PASTE      = 0xE125,
    ID_APP_DELETE     = 0xE200,
    ID_APP_PLAY_PAUSE = 0xE201,
    ID_APP_REMOVE     = 0xE202,
    ID_APP_EXPORT     = 0xE203,
};

// ============================================================================
// ApplicationOptions -- command-line and configuration state
// ============================================================================
class ApplicationOptions
{
public:
    ApplicationOptions()
        : m_bShowSplash(true)
        , m_bEnableTelemetry(true)
        , m_bSafeMode(false)
        , m_bImportMode(false)
        , m_bExportMode(false)
        , m_bShowHelp(false)
        , m_dwConcurrencyLimit(0)
    {
    }

    bool        m_bShowSplash;
    bool        m_bEnableTelemetry;
    bool        m_bSafeMode;
    bool        m_bImportMode;
    bool        m_bExportMode;
    bool        m_bShowHelp;
    DWORD       m_dwConcurrencyLimit;
    std::wstring m_strProjectFile;
    std::wstring m_strImportFile;
    std::wstring m_strExportFile;
    std::wstring m_strExportFormat;
};

// ============================================================================
// CommandLineParser -- parses argc/argv into ApplicationOptions
// ============================================================================
class CommandLineParser
{
public:
    CommandLineParser() {}

    bool Parse(int argc, wchar_t** argv, ApplicationOptions& options)
    {
        if (argc < 1 || !argv)
            return false;

        for (int i = 1; i < argc; ++i)
        {
            if (!argv[i])
                continue;

            std::wstring arg = argv[i];

            if (arg == L"/?" || arg == L"--help" || arg == L"-h")
            {
                options.m_bShowHelp = true;
            }
            else if (arg == L"/safemode" || arg == L"--safemode")
            {
                options.m_bSafeMode = true;
            }
            else if (arg == L"/nosplash" || arg == L"--nosplash")
            {
                options.m_bShowSplash = false;
            }
            else if (arg == L"/notelemetry" || arg == L"--notelemetry")
            {
                options.m_bEnableTelemetry = false;
            }
            else if (arg == L"/import" || arg == L"-import")
            {
                if (i + 1 < argc && argv[i + 1])
                {
                    options.m_bImportMode = true;
                    options.m_strImportFile = argv[++i];
                }
            }
            else if (arg == L"/export" || arg == L"-export")
            {
                if (i + 1 < argc && argv[i + 1])
                {
                    options.m_bExportMode = true;
                    options.m_strExportFile = argv[++i];
                }
            }
            else if (arg == L"/format" || arg == L"-format")
            {
                if (i + 1 < argc && argv[i + 1])
                {
                    options.m_strExportFormat = argv[++i];
                }
            }
            else if (arg == L"/concurrency" || arg == L"-concurrency")
            {
                if (i + 1 < argc && argv[i + 1])
                {
                    options.m_dwConcurrencyLimit = wcstoul(argv[++i], NULL, 10);
                }
            }
            else if (arg[0] == L'"' || arg[0] != L'/')
            {
                // Treat as a project file path
                std::wstring path = arg;
                // Strip surrounding quotes if present
                if (path.size() >= 2 && path.front() == L'"' && path.back() == L'"')
                {
                    path = path.substr(1, path.size() - 2);
                }
                if (!path.empty())
                {
                    options.m_strProjectFile = path;
                }
            }
        }

        return true;
    }
};

// ============================================================================
// AppMain -- the Sundance application object
// ============================================================================
static const wchar_t* kMainWindowClass = L"WindowsLiveMovieMakerMain";
static const wchar_t* kMainWindowTitle = L"Windows Live Movie Maker";
static const wchar_t* kSplashWindowClass = L"SundanceSplash";
static const UINT_PTR kSplashTimerId = 1;
static const UINT kSplashTimeoutMs = 2000;

// enum removed – duplicate – keep in MovieMakerCore.cpp


static LRESULT CALLBACK SundanceWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class AppMain
{

public:
    AppMain()
        : m_hInstance(NULL)
        , m_hWnd(NULL)
        , m_hAccelTable(NULL)
        , m_pSundanceApp(NULL)
        , m_bInitialized(false)
        , m_bRunning(false)
        , m_nExitCode(SUNDANCE_EXIT_SUCCESS)
    {
    }

    ~AppMain()
    {
        Shutdown();
    }

    // Initialize the application. Called after all subsystems are up.
    HRESULT Initialize(HINSTANCE hInstance, const ApplicationOptions& options)
    {
        if (m_bInitialized)
            return E_UNEXPECTED;

        m_hInstance = hInstance;
        m_options = options;

        // Initialize common controls (needed for Rebar, StatusBar, etc.)
        INITCOMMONCONTROLSEX icex = { 0 };
        icex.dwSize = sizeof(icex);
        icex.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES |
                      ICC_TAB_CLASSES | ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        // Create and initialize the SundanceAppMain subsystem
        m_pSundanceApp = new (std::nothrow) ::SundanceAppMain();
        if (!m_pSundanceApp)
            return E_OUTOFMEMORY;

        int argc = 0;
        wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        HRESULT hrApp = m_pSundanceApp->Initialize(hInstance, argc, argv);
        if (argv) LocalFree(argv);

        if (FAILED(hrApp))
        {
            delete m_pSundanceApp;
            m_pSundanceApp = NULL;
            return hrApp;
        }

        // Register main window class
        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = SundanceWndProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = kMainWindowClass;
        RegisterClassExW(&wc);

        // Create main application window
        m_hWnd = CreateWindowExW(
            WS_EX_ACCEPTFILES,
            kMainWindowClass,
            kMainWindowTitle,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            1200, 800,
            NULL, NULL, hInstance, this);

        if (!m_hWnd)
            return E_FAIL;

        m_pSundanceApp->OnMainWindowCreated(m_hWnd);

        m_hAccelTable = CreateAccelerators();

        // Show the window
        ShowWindow(m_hWnd, SW_SHOW);
        UpdateWindow(m_hWnd);

        // If a project file was specified on the command line, open it now
        if (!options.m_strProjectFile.empty())
        {
            m_pSundanceApp->OpenProject(options.m_strProjectFile.c_str());
            UpdateTitleFromProject();
        }
        else
        {
            // Start with a new empty project
            m_pSundanceApp->NewProject();
        }

        m_bInitialized = true;

        return S_OK;
    }

    // Run the application message loop.
    // Returns the application exit code.
    int Run()
    {
        if (!m_bInitialized)
            return SUNDANCE_EXIT_INIT_FAILED;

        m_bRunning = true;

        MSG msg = { 0 };
        BOOL bRet;

        while ((bRet = ::GetMessage(&msg, NULL, 0, 0)) != 0)
        {
            if (bRet == -1)
            {
                m_nExitCode = SUNDANCE_EXIT_APP_ERROR;
                break;
            }

            if (!::TranslateAccelerator(msg.hwnd, m_hAccelTable, &msg))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }

            if (msg.message == WM_QUIT)
            {
                m_nExitCode = static_cast<int>(msg.wParam);
                break;
            }
        }

        m_bRunning = false;

        return m_nExitCode;
    }

    // Shutdown and clean up application resources.
    void Shutdown()
    {
        if (!m_bInitialized)
            return;

        if (m_pSundanceApp)
        {
            m_pSundanceApp->OnMainWindowDestroyed();
            m_pSundanceApp->Shutdown();
            delete m_pSundanceApp;
            m_pSundanceApp = NULL;
        }

        if (m_hWnd)
        {
            DestroyWindow(m_hWnd);
            m_hWnd = NULL;
        }

        if (m_hAccelTable)
        {
            ::DestroyAcceleratorTable(m_hAccelTable);
            m_hAccelTable = NULL;
        }

        UnregisterClassW(kMainWindowClass, m_hInstance);

        m_bInitialized = false;
    }

    HINSTANCE GetInstance() const { return m_hInstance; }
    HWND GetHWnd() const { return m_hWnd; }
    bool IsInitialized() const { return m_bInitialized; }
    bool IsRunning() const { return m_bRunning; }
    ::SundanceAppMain* GetSundanceApp() const { return m_pSundanceApp; }

private:
    HACCEL CreateAccelerators()
    {
        static const ACCEL s_accels[] =
        {
            { FVIRTKEY | FCONTROL, 'O', ID_APP_IMPORT },
            { FVIRTKEY | FCONTROL, 'S', ID_APP_SAVE },
            { FVIRTKEY | FCONTROL, 'Z', ID_APP_UNDO },
            { FVIRTKEY | FCONTROL, 'Y', ID_APP_REDO },
            { FVIRTKEY | FCONTROL, 'C', ID_APP_COPY },
            { FVIRTKEY | FCONTROL, 'V', ID_APP_PASTE },
            { FVIRTKEY | FCONTROL, 'X', ID_APP_DELETE },
            { FVIRTKEY,             VK_SPACE, ID_APP_PLAY_PAUSE },
            { FVIRTKEY,             VK_DELETE, ID_APP_REMOVE },
            { FVIRTKEY | FCONTROL, 'E', ID_APP_EXPORT },
        };
        return ::CreateAcceleratorTableW(
            const_cast<ACCEL*>(s_accels),
            ARRAYSIZE(s_accels));
    }

    void UpdateTitleFromProject()
    {
        if (!m_hWnd || !m_pSundanceApp)
            return;

        ::SundanceAppMain* app = m_pSundanceApp;
        if (app->IsProjectOpen() && app->GetProject())
        {
            ATL::CString strTitle;
            ATL::CString strName = app->GetProject()->GetProjectName();
            if (!strName.IsEmpty())
                strTitle.Format(L"%s - %s", kMainWindowTitle, strName.GetString());
            else
                strTitle = kMainWindowTitle;

            if (app->IsProjectDirty())
                strTitle += L" *";

            ::SetWindowTextW(m_hWnd, strTitle);
        }
        else
        {
            ::SetWindowTextW(m_hWnd, kMainWindowTitle);
        }
    }

    HINSTANCE           m_hInstance;
    HWND                m_hWnd;
    HACCEL              m_hAccelTable;
    ::SundanceAppMain*  m_pSundanceApp;
    ApplicationOptions  m_options;
    bool                m_bInitialized;
    bool                m_bRunning;
    int                 m_nExitCode;
};

// ============================================================================
// SundanceWndProc -- main window procedure
// ============================================================================
static LRESULT CALLBACK SundanceWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    AppMain* pApp = reinterpret_cast<AppMain*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    ::SundanceAppMain* pSundanceApp = pApp ? pApp->GetSundanceApp() : NULL;

    switch (uMsg)
    {
    case WM_CREATE:
    {
        CREATESTRUCT* pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
        if (pcs && pcs->lpCreateParams)
        {
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(pcs->lpCreateParams));
        }
        return 0;
    }

    case WM_SIZE:
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));

        if (pSundanceApp && pSundanceApp->IsProjectOpen())
        {
            StoryboardManager::MovieProject* pProject = pSundanceApp->GetProject();
            if (pProject)
            {
                size_t cMedia = pProject->GetMediaItemCount();
                size_t cExtents = pProject->GetTotalExtentCount();

                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(60, 60, 60));
                HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
                HFONT hOld = (HFONT)SelectObject(hdc, hFont);

                ATL::CString strInfo;
                strInfo.Format(
                    L"Windows Live Movie Maker\n\n"
                    L"Project: %s\n"
                    L"Media items: %zu\n"
                    L"Timeline extents: %zu\n\n"
                    L"Use File > Add videos and photos to get started.",
                    pProject->GetProjectName().IsEmpty()
                        ? L"(untitled)" : pProject->GetProjectName().GetString(),
                    cMedia, cExtents);
                DrawTextW(hdc, strInfo, -1, &rc, DT_CENTER | DT_VCENTER | DT_WORDBREAK);

                SelectObject(hdc, hOld);
                DeleteObject(hFont);
            }
        }
        else
        {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(100, 100, 100));
            DrawTextW(hdc, L"Windows Live Movie Maker\n\nImport media to get started.",
                      -1, &rc, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
        }

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_COMMAND:
    {
        WORD wId = LOWORD(wParam);

        if (pSundanceApp)
        {
            switch (wId)
            {
            case ID_APP_IMPORT:
            {
                WCHAR szFiles[MAX_PATH * 4] = { 0 };
                OPENFILENAMEW ofn;
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize  = sizeof(ofn);
                ofn.hwndOwner    = hWnd;
                ofn.lpstrFilter  =
                    L"Media Files\0*.wmv;*.wma;*.avi;*.mp3;*.mp4;*.mov;*.mkv;*.jpg;*.jpeg;*.png;*.bmp;*.gif;*.tiff\0"
                    L"Video Files\0*.wmv;*.avi;*.mp4;*.mov;*.mkv;*.3gp;*.m4v\0"
                    L"Audio Files\0*.wma;*.mp3;*.wav;*.aac;*.flac\0"
                    L"Image Files\0*.jpg;*.jpeg;*.png;*.bmp;*.gif;*.tiff;*.tif\0"
                    L"All Files\0*.*\0";
                ofn.lpstrFile    = szFiles;
                ofn.nMaxFile     = ARRAYSIZE(szFiles);
                ofn.lpstrTitle   = L"Add Videos and Photos";
                ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                                   OFN_ALLOWMULTISELECT | OFN_EXPLORER;

                if (::GetOpenFileNameW(&ofn))
                {
                    LPCWSTR pszDir = szFiles;
                    LPCWSTR pszAfterDir = szFiles + wcslen(szFiles) + 1;

                    if (*pszAfterDir != L'\0')
                    {
                        std::vector<std::wstring> files;
                        LPCWSTR pszFile = pszAfterDir;
                        while (*pszFile)
                        {
                            WCHAR szFull[MAX_PATH];
                            if (SUCCEEDED(StringCchPrintfW(szFull, MAX_PATH, L"%s\\%s", pszDir, pszFile)))
                                files.push_back(szFull);
                            pszFile += wcslen(pszFile) + 1;
                        }

                        if (!files.empty())
                        {
                            std::vector<LPCWSTR> pFiles;
                            pFiles.reserve(files.size());
                            for (auto& f : files)
                                pFiles.push_back(f.c_str());

                            pSundanceApp->ImportMediaFiles(
                                static_cast<int>(pFiles.size()), &pFiles[0]);

                            for (auto& f : files)
                                pSundanceApp->AddMediaToTimeline(
                                    f.c_str(), TimelineTrack_Video);
                        }
                    }
                    else if (szFiles[0] != L'\0')
                    {
                        LPCWSTR szFilePtr = szFiles; pSundanceApp->ImportMediaFiles(1, &szFilePtr);
                        pSundanceApp->AddMediaToTimeline(szFiles, TimelineTrack_Video);
                    }
                }
                return 0;
            }

            case ID_APP_SAVE:
                pSundanceApp->SaveProject();
                return 0;

            case ID_APP_UNDO:
                pSundanceApp->Undo();
                return 0;

            case ID_APP_REDO:
                pSundanceApp->Redo();
                return 0;

            case ID_APP_COPY:
                pSundanceApp->CopySelection();
                return 0;

            case ID_APP_PASTE:
                pSundanceApp->PasteFromClipboard();
                return 0;

            case ID_APP_DELETE:
                pSundanceApp->CutSelection();
                return 0;

            case ID_APP_PLAY_PAUSE:
                if (pSundanceApp->IsPlaying())
                    pSundanceApp->PausePlayback();
                else
                    pSundanceApp->StartPlayback();
                return 0;

            case ID_APP_EXPORT:
            {
                WCHAR szExport[MAX_PATH] = { 0 };
                OPENFILENAMEW ofn;
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize  = sizeof(ofn);
                ofn.hwndOwner    = hWnd;
                ofn.lpstrFilter  = L"Windows Media Video (*.wmv)\0*.wmv\0"
                                   L"MPEG-4 (*.mp4)\0*.mp4\0";
                ofn.lpstrFile    = szExport;
                ofn.nMaxFile     = ARRAYSIZE(szExport);
                ofn.lpstrTitle   = L"Save Movie";
                ofn.lpstrDefExt  = L"wmv";
                ofn.Flags        = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

                if (::GetSaveFileNameW(&ofn))
                    pSundanceApp->PublishMovie(szExport, 0);
                return 0;
            }

            default:
                pSundanceApp->OnRibbonCommand(wId);
                break;
            }
        }
        return 0;
    }

    case WM_DROPFILES:
    {
        if (pSundanceApp)
        {
            HDROP hDrop = reinterpret_cast<HDROP>(wParam);
            UINT cFiles = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);

            if (cFiles > 0)
            {
                std::vector<std::wstring> files;
                files.reserve(cFiles);
                for (UINT i = 0; i < cFiles; ++i)
                {
                    UINT cch = DragQueryFileW(hDrop, i, NULL, 0);
                    if (cch > 0)
                    {
                        std::wstring strPath(cch + 1, L'\0');
                        DragQueryFileW(hDrop, i, &strPath[0], cch + 1);
                        strPath.resize(cch);
                        files.push_back(strPath);
                    }
                }

                if (!files.empty())
                {
                    std::vector<LPCWSTR> pFiles;
                    pFiles.reserve(files.size());
                    for (auto& f : files)
                        pFiles.push_back(f.c_str());

                    pSundanceApp->ImportMediaFiles(
                        static_cast<int>(pFiles.size()), &pFiles[0]);

                    for (auto& f : files)
                        pSundanceApp->AddMediaToTimeline(
                            f.c_str(), TimelineTrack_Video);
                }
            }
            DragFinish(hDrop);
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
}

} // namespace Sundance

// ============================================================================
// MovieMakerMain -- the sole DLL export
// ============================================================================
// This is the single entry point for MovieMakerCore.dll.
// Called by MovieMaker.exe after loading this DLL dynamically.
//
static const int SUNDANCE_EXIT_CRASH = 10;

static int __cdecl MovieMakerMainCore(int argc, wchar_t** argv)
{
    int nExitCode = SUNDANCE_EXIT_SUCCESS;

    // ------------------------------------------------------------------
    // 1. Parse command line
    // ------------------------------------------------------------------
    Sundance::CommandLineParser parser;
    Sundance::ApplicationOptions options;

    if (!parser.Parse(argc, argv, options))
    {
        return SUNDANCE_EXIT_INIT_FAILED;
    }

    // If --help requested, print usage and exit
    if (options.m_bShowHelp)
    {
        ::OutputDebugStringW(
            L"Windows Live Movie Maker 2012 (Sundance)\n"
            L"Usage: MovieMaker.exe [options] [projectfile]\n"
            L"\n"
            L"Options:\n"
            L"  /?, --help          Show this help message\n"
            L"  /safemode           Start in safe mode (no add-ins)\n"
            L"  /nosplash           Skip splash screen\n"
            L"  /notelemetry        Disable SQM telemetry\n"
            L"  /import <file>      Import media file\n"
            L"  /export <file>      Export/render to file\n"
            L"  /format <fmt>       Export format (mp4, wmv, etc.)\n"
            L"  /concurrency <n>    Limit worker thread count\n"
        );
        return SUNDANCE_EXIT_SUCCESS;
    }

    // ------------------------------------------------------------------
    // 2. Validate single instance
    // ------------------------------------------------------------------
    HANDLE hMutex = ::CreateMutexW(
        NULL,
        FALSE,
        L"Global\\WindowsLiveMovieMaker_Sundance_SingleInstance");

    if (::GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND hExisting = ::FindWindowW(
            L"WindowsLiveMovieMakerMain",
            SUNDANCE_PRODUCT_NAME);

        if (hExisting)
        {
            ::SetForegroundWindow(hExisting);
            ::ShowWindow(hExisting, SW_RESTORE);
        }

        return SUNDANCE_EXIT_SINGLE_INSTANCE;
    }

    // ------------------------------------------------------------------
    // 3. Initialize subsystems
    // ------------------------------------------------------------------
    HINSTANCE hInstance = ::GetModuleHandleW(L"MovieMakerCore.dll");
    if (!hInstance)
    {
        hInstance = ::GetModuleHandleW(NULL);
    }

    if (!MovieCore_Initialize(hInstance))
    {
        nExitCode = SUNDANCE_EXIT_INIT_FAILED;
        goto cleanup;
    }

    // ------------------------------------------------------------------
    // 4. Create and initialize Sundance application
    // ------------------------------------------------------------------
    {
        Sundance::AppMain app;

        {
            HRESULT hr = app.Initialize(hInstance, options);
            if (FAILED(hr))
            {
                nExitCode = SUNDANCE_EXIT_INIT_FAILED;
                goto shutdown;
            }

            // ------------------------------------------------------------------
            // 5. Run the application (message loop)
            // ------------------------------------------------------------------
            nExitCode = app.Run();

            // ------------------------------------------------------------------
            // 6. Shutdown the application
            // ------------------------------------------------------------------
            app.Shutdown();
        }
    }

shutdown:
    // ------------------------------------------------------------------
    // 7. Shutdown subsystems in reverse order
    // ------------------------------------------------------------------
    MovieCore_Shutdown();

cleanup:
    // ------------------------------------------------------------------
    // 8. Release single-instance mutex
    // ------------------------------------------------------------------
    if (hMutex)
    {
        ::ReleaseMutex(hMutex);
        ::CloseHandle(hMutex);
        hMutex = NULL;
    }

    return nExitCode;
}

extern "C" int __cdecl MovieMakerMain(int argc, wchar_t** argv)
{
    return MovieMakerMainCore(argc, argv);
}