#include <windows.h>
#include <ole2.h>
#include <unknwn.h>

#define UNREFERENCED_PARAMETER(p) (p)

// UxControlsInitProcess - process-level initialization
extern "C" __declspec(dllexport) HRESULT __stdcall UxControlsInitProcess(void)
{
    return S_OK;
}

// UxControlsCreateObject - creates control objects by CLSID
extern "C" __declspec(dllexport) HRESULT __stdcall UxControlsCreateObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    if (ppv) *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}

// UxControlsUninitProcess - process-level cleanup
extern "C" __declspec(dllexport) void __stdcall UxControlsUninitProcess(void)
{
}
