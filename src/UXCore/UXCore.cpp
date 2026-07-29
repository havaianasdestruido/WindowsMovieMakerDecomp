#include <windows.h>
#include <ole2.h>
#include <unknwn.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(p) (void)(p)
#endif

static ID2D1Factory* g_pD2DFactory = nullptr;
static IDWriteFactory* g_pDWriteFactory = nullptr;
static IWICImagingFactory* g_pWICFactory = nullptr;

// Process/Thread lifecycle
extern "C" __declspec(dllexport) HRESULT __stdcall UXCoreInitProcess(void)
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
    if (SUCCEEDED(hr))
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_pDWriteFactory));
    if (SUCCEEDED(hr))
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), reinterpret_cast<void**>(&g_pWICFactory));
    return hr;
}

extern "C" __declspec(dllexport) HRESULT __stdcall UXCoreInitThread(void)
{
    return S_OK;
}

extern "C" __declspec(dllexport) void __stdcall UXCoreUnInitProcess(void)
{
    if (g_pWICFactory) { g_pWICFactory->Release(); g_pWICFactory = nullptr; }
    if (g_pDWriteFactory) { g_pDWriteFactory->Release(); g_pDWriteFactory = nullptr; }
    if (g_pD2DFactory) { g_pD2DFactory->Release(); g_pD2DFactory = nullptr; }
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
