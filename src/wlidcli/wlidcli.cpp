#include <windows.h>
#include <ole2.h>

static volatile LONG g_bSignedIn = 0;
static DWORD g_dwNextHandle = 0x1000;

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
        *ppvAuthState = &g_bSignedIn;
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
    *ppszTicket = (LPWSTR)CoTaskMemAlloc(sizeof(g_wszTicket));
    if (!*ppszTicket)
        return E_OUTOFMEMORY;
    CopyMemory(*ppszTicket, g_wszTicket, sizeof(g_wszTicket));
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
    *ppszEnvironment = g_wszEnvProduction;
    return S_OK;
}

}
