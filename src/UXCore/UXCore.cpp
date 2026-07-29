#include <windows.h>
#include <ole2.h>
#include <unknwn.h>

#define UNREFERENCED_PARAMETER(p) (p)

// Process/Thread lifecycle
extern "C" __declspec(dllexport) HRESULT __stdcall UXCoreInitProcess(void)
{
    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT __stdcall UXCoreInitThread(void)
{
    return S_OK;
}

extern "C" __declspec(dllexport) void __stdcall UXCoreUnInitProcess(void)
{
}

extern "C" __declspec(dllexport) void __stdcall UXCoreUnInitThread(void)
{
}

// Class factory
extern "C" __declspec(dllexport) HRESULT __stdcall UxGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    if (ppv) *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}

// DuiCreateObject - creates DirectUI elements by class name
extern "C" __declspec(dllexport) HRESULT __stdcall DuiCreateObject(const wchar_t* className, void** ppElement)
{
    UNREFERENCED_PARAMETER(className);
    if (ppElement) *ppElement = NULL;
    return E_NOTIMPL;
}
