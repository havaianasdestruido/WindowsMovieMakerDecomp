// MovieMaker.exe -- Thin launcher for Windows Live Movie Maker 2012
// Disassembly-based recreation. Entry point jumps to __tmainCRTStartup,
// which validates PE headers then calls WinMain.

#include <windows.h>
#include <shellapi.h>

// WLXPhotoBase forward -- single import from this DLL
extern "C" void __declspec(dllimport) __stdcall WLXPhotoBase_Init(void);

// MovieMakerMain exported from MovieMakerCore.dll
extern "C" int __cdecl MovieMakerMain(int argc, wchar_t** argv);

// Clause-level SEH filter for C++ exception magic 0xe06d7363.
// The original binary registers a __except handler around the
// LoadLibrary/GetProcAddress region that catches C++ exceptions
// with subcodes 0x19930520-22 and 0x01994000.
static DWORD WINAPI VexHandler(PEXCEPTION_POINTERS ep)
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
    // -- Step 1: Bootstrap WLXPhotoBase (the shared "base" DLL) --
    HMODULE hPhotoBase = LoadLibraryA("WLXPhotoBase.dll");
    if (hPhotoBase)
    {
        WLXPhotoBase_Init();
    }

    // -- Step 2: Load MovieMakerCore.dll --
    HMODULE hCore = LoadLibraryExA(
        "MovieMakerCore.dll",
        NULL,
        0); // LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR is implicit on Vista+

    if (!hCore)
    {
        hCore = LoadLibraryExA("MovieMakerCore.dll", NULL,
                               LOAD_LIBRARY_AS_DATAFILE |
                               LOAD_LIBRARY_SEARCH_SYSTEM32);
    }

    if (!hCore)
    {
        return 1;
    }

    // -- Step 3: Resolve entry point --
    auto pfnMain = reinterpret_cast<decltype(&MovieMakerMain)>(
        GetProcAddress(hCore, "MovieMakerMain"));

    if (!pfnMain)
    {
        return 2;
    }

    // -- Step 4: Call with VEH protecting against C++ exceptions --
    int ret = 0;

    PVOID veh = AddVectoredExceptionHandler(0, VexHandler);

    __try
    {
        int argc;
        wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

        ret = pfnMain(argc, argv);

        LocalFree(argv);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        ret = 3;
    }

    RemoveVectoredExceptionHandler(veh);

    return ret;
}
