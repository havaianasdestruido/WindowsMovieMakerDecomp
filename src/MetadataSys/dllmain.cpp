#include <windows.h>
#include <ole2.h>

HMODULE g_hModule = NULL;
bool    g_bComInit = false;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);
        g_hModule = (HMODULE)hInstance;
        if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
            g_bComInit = true;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        if (g_bComInit)
        {
            CoUninitialize();
            g_bComInit = false;
        }
    }
    return TRUE;
}

STDAPI DllCanUnloadNow(void)
{
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID, REFIID, LPVOID*)
{
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
