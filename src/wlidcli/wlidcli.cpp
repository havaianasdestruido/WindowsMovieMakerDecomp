#include <windows.h>
#include <ole2.h>

static volatile LONG g_bSignedIn = 0;
static volatile LONG g_dwNextHandle = 0x1000;

static const WCHAR g_wszTicket[] = L"ticket=st%3d1%26token%3dWLID_SIMULATED_TOKEN";
static const WCHAR g_wszEnvProduction[] = L"production";

extern "C"
{

HRESULT __stdcall WLClogin(HWND hwndParent, LPCWSTR szCred, DWORD dwFlags, LPVOID* ppvAuthState)
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(szCred);
    UNREFERENCED_PARAMETER(dwFlags);
    InterlockedExchange(&g_bSignedIn, 1);
    if (ppvAuthState)
        *ppvAuthState = const_cast<LONG*>(&g_bSignedIn);
    return S_OK;
}

HRESULT __stdcall WLCheckCredentials(LPCWSTR szCred)
{
    UNREFERENCED_PARAMETER(szCred);
    InterlockedExchange(&g_bSignedIn, 1);
    return S_OK;
}

DWORD __stdcall WLCreateIdentityHandle()
{
    return InterlockedIncrement(&g_dwNextHandle);
}

HRESULT __stdcall WLGetTicket(DWORD dwHandle, LPWSTR* ppszTicket)
{
    UNREFERENCED_PARAMETER(dwHandle);
    if (!ppszTicket)
        return E_POINTER;
    size_t len = wcslen(g_wszTicket) + 1;
    *ppszTicket = (LPWSTR)CoTaskMemAlloc(len * sizeof(WCHAR));
    if (!*ppszTicket)
        return E_OUTOFMEMORY;
    CopyMemory(*ppszTicket, g_wszTicket, len * sizeof(WCHAR));
    return S_OK;
}

BOOL __stdcall WLIsSignedIn(DWORD dwHandle)
{
    UNREFERENCED_PARAMETER(dwHandle);
    return InterlockedCompareExchange(&g_bSignedIn, 0, 0) ? TRUE : FALSE;
}

void __stdcall WLFreeMemory(LPVOID pv)
{
    if (pv)
        CoTaskMemFree(pv);
}

HRESULT __stdcall WLGetEnvironment(LPCWSTR* ppszEnvironment)
{
    if (!ppszEnvironment)
        return E_POINTER;
    size_t len = wcslen(g_wszEnvProduction) + 1;
    *ppszEnvironment = (LPCWSTR)CoTaskMemAlloc(len * sizeof(WCHAR));
    if (!*ppszEnvironment)
        return E_OUTOFMEMORY;
    CopyMemory((LPWSTR)*ppszEnvironment, g_wszEnvProduction, len * sizeof(WCHAR));
    return S_OK;
}

}
