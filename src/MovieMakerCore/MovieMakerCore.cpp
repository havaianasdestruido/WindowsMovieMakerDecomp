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

namespace Sundance
{

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

static LRESULT CALLBACK SundanceWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return 0;

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
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(100, 100, 100));
        DrawTextW(hdc, L"Windows Live Movie Maker\n\nProject: (empty)\n\nImport media to get started.",
                  -1, &rc, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
}

class AppMain
{
public:
    AppMain()
        : m_hInstance(NULL)
        , m_hWnd(NULL)
        , m_hAccelTable(NULL)
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
            0,
            kMainWindowClass,
            kMainWindowTitle,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            1200, 800,
            NULL, NULL, hInstance, NULL);

        if (!m_hWnd)
            return E_FAIL;

        // Load keyboard accelerators from resource
        m_hAccelTable = ::LoadAccelerators(hInstance,
            MAKEINTRESOURCE(/* IDR_MAIN_ACCEL */ 100));

        // Show the window
        ShowWindow(m_hWnd, SW_SHOW);
        UpdateWindow(m_hWnd);

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

private:
    HINSTANCE           m_hInstance;
    HWND                m_hWnd;
    HACCEL              m_hAccelTable;
    ApplicationOptions  m_options;
    bool                m_bInitialized;
    bool                m_bRunning;
    int                 m_nExitCode;
};

} // namespace Sundance

// ============================================================================
// MovieMakerMain -- the sole DLL export
// ============================================================================
// This is the single entry point for MovieMakerCore.dll.
// Called by MovieMaker.exe after loading this DLL dynamically.
//
extern "C"
MOVIECORE_API int __cdecl MovieMakerMain(int argc, wchar_t** argv)
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
