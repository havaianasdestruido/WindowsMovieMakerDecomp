#include <windows.h>
#include <objbase.h>
#include <urlmon.h>
#include <stdio.h>
#include <assert.h>

// CLSID_AlbumDownloadLauncher
// {E43EF6CD-A37A-4A9B-9E6F-83F89B8E6324}
static const GUID CLSID_AlbumDownloadLauncher = {
    0xe43ef6cd, 0xa37a, 0x4a9b, { 0x9e, 0x6f, 0x83, 0xf8, 0x9b, 0x8e, 0x63, 0x24 }
};

int main()
{
    HRESULT hr;
    IClassFactory *pFactory = NULL;
    IUnknown *pUnk = NULL;
    IInternetProtocol *pProt = NULL;
    IInternetProtocolInfo *pInfo = NULL;

    // Add the Photo Gallery directory to the DLL search path so WLXPhotoBase.dll can be found
    SetDllDirectoryW(L"C:\\Users\\mcmco\\Desktop\\WMMR\\undecomp\\Photo Gallery");

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    assert(SUCCEEDED(hr));

    wprintf(L"AlbumDownloadProtocolHandler Test Harness\n");
    wprintf(L"========================================\n\n");

    // Test 1: Load via CoGetClassObject
    hr = CoGetClassObject(
        CLSID_AlbumDownloadLauncher,
        CLSCTX_INPROC_SERVER,
        NULL,
        IID_IClassFactory,
        (void**)&pFactory
    );
    if (FAILED(hr)) {
        wprintf(L"[FAIL] CoGetClassObject: hr=0x%08X (DLL not registered)\n", hr);
        wprintf(L"[SKIP] Register with: regsvr32 \"path\\to\\AlbumDownloadProtocolHandler.dll\"\n");
        goto cleanup;
    }
    wprintf(L"[PASS] CoGetClassObject succeeded\n");

    // Test 2: Create instance
    hr = pFactory->CreateInstance(NULL, IID_IUnknown, (void**)&pUnk);
    assert(SUCCEEDED(hr));
    wprintf(L"[PASS] CreateInstance (IUnknown) succeeded\n");

    // Test 3: QI for IInternetProtocol
    hr = pUnk->QueryInterface(IID_IInternetProtocol, (void**)&pProt);
    assert(SUCCEEDED(hr));
    wprintf(L"[PASS] QI for IInternetProtocol succeeded\n");

    // Test 4: QI for IInternetProtocolInfo
    hr = pUnk->QueryInterface(IID_IInternetProtocolInfo, (void**)&pInfo);
    assert(SUCCEEDED(hr));
    wprintf(L"[PASS] QI for IInternetProtocolInfo succeeded\n");

    // Test 5: Start() with sample wlalbumdownload: URL
    LPCWSTR testUrl = L"wlalbumdownload:{E43EF6CD-A37A-4A9B-9E6F-83F89B8E6324}";

    hr = pProt->Start(
        testUrl,
        NULL,       // pSink (no sink needed)
        NULL,       // pBindInfo
        0,          // grfPI
        0           // dwReserved
    );
    wprintf(L"[INFO]  Start(\"%ls\") returned hr=0x%08X\n",
        testUrl, hr);

    // Test 6: ParseUrl
    const int bufLen = 256;
    WCHAR scheme[bufLen] = { 0 };
    DWORD schemeLen = bufLen;

    hr = pInfo->ParseUrl(
        testUrl,
        PARSE_SCHEMA,
        NULL,           // no context
        scheme,
        bufLen,
        &schemeLen,
        0
    );
    if (SUCCEEDED(hr)) {
        wprintf(L"[PASS] ParseUrl(PARSE_SCHEMA) returned scheme: \"%ls\"\n", scheme);
    } else {
        wprintf(L"[INFO]  ParseUrl(PARSE_SCHEMA) hr=0x%08X (expected=INET_E_DEFAULT_ACTION)\n", hr);
    }

    // Test 7: Terminate (should not crash)
    hr = pProt->Abort(NULL, 0);
    wprintf(L"[PASS] Abort() returned hr=0x%08X\n", hr);

    // Test 8: Read (should return E_NOTIMPL)
    char buf[64];
    ULONG bytesRead = 0;
    hr = pProt->Read(buf, sizeof(buf), &bytesRead);
    wprintf(L"[INFO]  Read() returned hr=0x%08X (expected=E_NOTIMPL 0x80004001)\n", hr);
    assert(hr == 0x80004001);

cleanup:
    if (pInfo)  pInfo->Release();
    if (pProt)  pProt->Release();
    if (pUnk)   pUnk->Release();
    if (pFactory) pFactory->Release();

    CoUninitialize();

    wprintf(L"\nTest harness complete.\n");
    return 0;
}
