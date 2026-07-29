#include <windows.h>
#include <ole2.h>

static HINSTANCE g_hModule = NULL;
static BOOL g_bComInit = FALSE;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
        {
            g_bComInit = TRUE;
        }
        break;

    case DLL_PROCESS_DETACH:
        if (g_bComInit)
        {
            CoUninitialize();
            g_bComInit = FALSE;
        }
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
