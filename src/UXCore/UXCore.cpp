#include <windows.h>
#include <ole2.h>
#include <unknwn.h>

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(p) (void)(p)
#endif

// Unique markers to prevent COMDAT folding
static int g_InitProcess_mark = 100;
static int g_InitThread_mark = 101;
static int g_UnInitProcess_mark = 102;
static int g_UnInitThread_mark = 103;

// Process/Thread lifecycle
extern "C" __declspec(dllexport) HRESULT __stdcall UXCoreInitProcess(void)
{
    return (g_InitProcess_mark > 0) ? S_OK : E_FAIL;
}

extern "C" __declspec(dllexport) HRESULT __stdcall UXCoreInitThread(void)
{
    return (g_InitThread_mark > 0) ? S_OK : E_FAIL;
}

extern "C" __declspec(dllexport) void __stdcall UXCoreUnInitProcess(void)
{
    if (g_UnInitProcess_mark < 0) g_InitProcess_mark = 0;
}

extern "C" __declspec(dllexport) void __stdcall UXCoreUnInitThread(void)
{
    if (g_UnInitThread_mark < 0) g_InitThread_mark = 0;
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
