// MovieMaker.exe -- Thin launcher for Windows Live Movie Maker 2012
// Disassembly-based recreation. Entry point jumps to __tmainCRTStartup,
// which validates PE headers then calls WinMain.

#include <windows.h>
#include <shellapi.h>
#include <wchar.h>

// WLXPhotoBase forward -- single import from this DLL
extern "C" void __declspec(dllimport) __stdcall WLXPhotoBase_Init(void);

// MovieMakerMain exported from MovieMakerCore.dll
extern "C" int __cdecl MovieMakerMain(int argc, wchar_t** argv);

// Clause-level SEH filter for C++ exception magic 0xe06d7363.
// The original binary registers a __except handler around the
// LoadLibrary/GetProcAddress region that catches C++ exceptions
// with subcodes 0x19930520-22 and 0x01994000.
static LONG WINAPI VexHandler(PEXCEPTION_POINTERS ep)
{
    if (ep->ExceptionRecord->ExceptionCode == 0xe06d7363)
    {
        DWORD sub = ep->ExceptionRecord->ExceptionInformation[1];
        if (sub == 0x19930520 || sub == 0x19930521 ||
            sub == 0x19930522 || sub == 0x01994000)
        {
            return EXCEPTION_EXECUTE_HANDLER;
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
                   LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
    OutputDebugStringW(L"[WMMR] WinMain: Entry\n");

    // -- Step 1: Resolve the application directory once --
    wchar_t exeDir[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, exeDir, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
    {
        OutputDebugStringW(L"[WMMR] WinMain: GetModuleFileNameW failed\n");
        return 1;
    }
    wchar_t* slash = wcsrchr(exeDir, L'\\');
    if (slash != NULL) *(slash + 1) = L'\0';

    const DWORD safeFlags = LOAD_LIBRARY_SEARCH_APPLICATION_DIR |
                            LOAD_LIBRARY_SEARCH_USER_DIRS |
                            LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;

    // -- Step 2: Bootstrap WLXPhotoBase (the shared "base" DLL) --
    wchar_t photoBasePath[MAX_PATH];
    wcscpy_s(photoBasePath, exeDir);
    wcscat_s(photoBasePath, L"WLXPhotoBase.dll");

    HMODULE hPhotoBase = LoadLibraryExW(photoBasePath, NULL, safeFlags);
    if (hPhotoBase)
    {
        __try { WLXPhotoBase_Init(); } __except(EXCEPTION_EXECUTE_HANDLER) {
        }
    }
    else
    {
        OutputDebugStringW(L"[WMMR] WinMain: WLXPhotoBase.dll not loaded (optional)\n");
    }

    // -- Step 3: Load MovieMakerCore.dll --
    wchar_t corePath[MAX_PATH];
    wcscpy_s(corePath, exeDir);
    wcscat_s(corePath, L"MovieMakerCore.dll");

    HMODULE hCore = LoadLibraryExW(corePath, NULL, safeFlags);

    if (!hCore)
    {
        hCore = LoadLibraryExW(corePath, NULL, LOAD_LIBRARY_AS_DATAFILE);
    }

    if (!hCore)
    {
        OutputDebugStringW(L"[WMMR] WinMain: failed to load MovieMakerCore.dll\n");
        return 1;
    }

    // -- Step 4: Resolve entry point --
    auto pfnMain = reinterpret_cast<decltype(&MovieMakerMain)>(
        GetProcAddress(hCore, "MovieMakerMain"));

    if (!pfnMain)
    {
        OutputDebugStringW(L"[WMMR] WinMain: MovieMakerMain export not found\n");
        return 2;
    }

    // -- Step 5: Call with VEH protecting against C++ exceptions --
    int ret = 0;
    int argc = 0;
    wchar_t** argv = NULL;

    PVOID veh = AddVectoredExceptionHandler(0, VexHandler);
    if (!veh) OutputDebugStringW(L"[WMMR] WinMain: AddVectoredExceptionHandler failed\n");

    __try
    {
        argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!argv)
        {
            OutputDebugStringW(L"[WMMR] WinMain: CommandLineToArgvW failed\n");
            ret = 4;
        }
        else
        {
            ret = pfnMain(argc, argv);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        ret = 3;
    }

    if (veh) RemoveVectoredExceptionHandler(veh);

    // -- Step 6: Shutdown -- release all acquired resources --
    if (argv)
    {
        LocalFree(argv);
    }
    if (hCore)
    {
        if (hCore) FreeLibrary(hCore);
    }
    if (hPhotoBase)
    {
        if (hPhotoBase) FreeLibrary(hPhotoBase);
    }

    return ret;
}
