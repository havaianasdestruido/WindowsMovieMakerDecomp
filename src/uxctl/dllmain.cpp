#include <windows.h>
#include <ole2.h>

HINSTANCE g_hModule = NULL;
BOOL g_bComInit = FALSE;

STDAPI DllCanUnloadNow(void)
{
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    if (ppv) *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllRegisterServer(void)
{
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    return S_OK;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
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
    }

    return TRUE;
}
