#include <windows.h>
#include <ole2.h>
#include <unknwn.h>

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (void)(P)
#endif

// Ord 1: HRESULT WLXPSGetItemPropertyHandler(IUnknown*, DWORD, REFIID, void**)
extern "C" __declspec(dllexport) HRESULT __stdcall WLXPSGetItemPropertyHandler(IUnknown* pItem, DWORD dwAccessMode, REFIID riid, void** ppv)
{
    UNREFERENCED_PARAMETER(pItem);
    UNREFERENCED_PARAMETER(dwAccessMode);
    UNREFERENCED_PARAMETER(riid);
    if (ppv) *ppv = NULL;
    return E_NOTIMPL;
}
