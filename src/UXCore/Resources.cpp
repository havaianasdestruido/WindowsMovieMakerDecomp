#include "Resources.h"
#include "Element.h"
#include <ocidl.h>

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(p) (void)(p)
#endif

namespace DirectUI {

// ---------------------------------------------------------------------------
// CRMStringResource
// ---------------------------------------------------------------------------
static int g_CRMStringResource_mark = 1;

CRMStringResource::CRMStringResource()
{
}

CRMStringResource::CRMStringResource(HINSTANCE hinst, UINT id)
{
    Load(hinst, id);
}

CRMStringResource::~CRMStringResource()
{
}

HRESULT CRMStringResource::Load(HINSTANCE hinst, UINT id)
{
    if (!hinst) return E_INVALIDARG;
    int cch = LoadStringW(hinst, id, nullptr, 0);
    if (cch <= 0)
    {
        m_str.clear();
        DWORD err = GetLastError();
        if (err == 0) err = ERROR_RESOURCE_NAME_NOT_FOUND;
        return HRESULT_FROM_WIN32(err);
    }
    m_str.resize(cch);
    LoadStringW(hinst, id, &m_str[0], cch + 1);
    return S_OK;
}

int CRMStringResource::Length() const
{
    return static_cast<int>(m_str.size());
}

const wchar_t* CRMStringResource::ToString() const
{
    return m_str.c_str();
}

// ---------------------------------------------------------------------------
// CRMResource
// ---------------------------------------------------------------------------
static int g_CRMResource_mark = 1;

CRMResource::CRMResource()
{
}

CRMResource::~CRMResource()
{
}

// ---------------------------------------------------------------------------
// CRMImage
// ---------------------------------------------------------------------------
static int g_CRMImage_mark = 1;

CRMImage::CRMImage()
    : m_hBitmap(nullptr)
{
}

CRMImage::~CRMImage()
{
    if (m_hBitmap)
    {
        DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }
}

HRESULT CRMImage::LoadFromResource(HINSTANCE hinst, UINT id)
{
    if (!hinst) return E_INVALIDARG;
    HBITMAP hbm = static_cast<HBITMAP>(
        LoadImageW(hinst, MAKEINTRESOURCEW(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION));
    if (!hbm)
    {
        DWORD err = GetLastError();
        if (err == 0) err = ERROR_RESOURCE_NAME_NOT_FOUND;
        return HRESULT_FROM_WIN32(err);
    }
    if (m_hBitmap)
        DeleteObject(m_hBitmap);
    m_hBitmap = hbm;
    return S_OK;
}

HBITMAP CRMImage::Detach()
{
    HBITMAP hbm = m_hBitmap;
    m_hBitmap = nullptr;
    return hbm;
}

void CRMImage::Attach(HBITMAP hbm)
{
    if (m_hBitmap)
        DeleteObject(m_hBitmap);
    m_hBitmap = hbm;
}

HRESULT CRMImage::ConvertToARGB()
{
    if (!m_hBitmap)
        return E_UNEXPECTED;

    BITMAP bm;
    if (!GetObjectW(m_hBitmap, sizeof(bm), &bm))
        return HRESULT_FROM_WIN32(GetLastError());

    if (bm.bmBitsPixel == 32)
        return S_OK;

    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = bm.bmWidth;
    bi.bmiHeader.biHeight = -bm.bmHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HDC hdcScreen = GetDC(nullptr);
    HBITMAP hbmNew = CreateDIBSection(hdcScreen, &bi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdcScreen);

    if (!hbmNew)
        return HRESULT_FROM_WIN32(GetLastError());

    HDC hdcDst = CreateCompatibleDC(nullptr);
    HDC hdcSrc = CreateCompatibleDC(nullptr);
    HGDIOBJ hObjDst = SelectObject(hdcDst, hbmNew);
    HGDIOBJ hObjSrc = SelectObject(hdcSrc, m_hBitmap);

    SetStretchBltMode(hdcDst, COLORONCOLOR);
    StretchBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight,
               hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    SelectObject(hdcDst, hObjDst);
    SelectObject(hdcSrc, hObjSrc);
    DeleteDC(hdcDst);
    DeleteDC(hdcSrc);

    DeleteObject(m_hBitmap);
    m_hBitmap = hbmNew;
    return S_OK;
}

// ---------------------------------------------------------------------------
// CRMDUIParser
// ---------------------------------------------------------------------------
static int g_CRMDUIParser_mark = 1;

CRMDUIParser::CRMDUIParser()
{
}

CRMDUIParser::~CRMDUIParser()
{
}

HRESULT CRMDUIParser::Load(HINSTANCE hinst, UINT id, DWORD flags)
{
    UNREFERENCED_PARAMETER(flags);
    if (!hinst)
        return E_INVALIDARG;

    HRSRC hRes = FindResourceW(hinst, MAKEINTRESOURCEW(id), L"UIFILE");
    if (!hRes)
        hRes = FindResourceW(hinst, MAKEINTRESOURCEW(id), RT_RCDATA);
    if (!hRes)
    {
        DWORD err = GetLastError();
        if (err == 0) err = ERROR_RESOURCE_NAME_NOT_FOUND;
        return HRESULT_FROM_WIN32(err);
    }

    HGLOBAL hData = LoadResource(hinst, hRes);
    if (!hData)
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD cbSize = SizeofResource(hinst, hRes);
    const BYTE* pRaw = static_cast<const BYTE*>(LockResource(hData));
    if (!pRaw || cbSize == 0)
    {
        FreeResource(hData);
        return E_FAIL;
    }

    m_uiText.clear();

    if (cbSize >= 2 && pRaw[0] == 0xFF && pRaw[1] == 0xFE)
    {
        const wchar_t* pText = reinterpret_cast<const wchar_t*>(pRaw + 2);
        int cch = static_cast<int>((cbSize - 2) / sizeof(wchar_t));
        m_uiText.assign(pText, cch);
    }
    else if (cbSize >= 3 && pRaw[0] == 0xEF && pRaw[1] == 0xBB && pRaw[2] == 0xBF)
    {
        const char* pUtf8 = reinterpret_cast<const char*>(pRaw + 3);
        int cch = MultiByteToWideChar(CP_UTF8, 0, pUtf8, static_cast<int>(cbSize - 3), nullptr, 0);
        if (cch > 0)
        {
            m_uiText.resize(cch);
            MultiByteToWideChar(CP_UTF8, 0, pUtf8, static_cast<int>(cbSize - 3), &m_uiText[0], cch);
        }
    }
    else if ((cbSize & 1) == 0)
    {
        const wchar_t* pText = reinterpret_cast<const wchar_t*>(pRaw);
        int cch = static_cast<int>(cbSize / sizeof(wchar_t));
        m_uiText.assign(pText, cch);
    }
    else
    {
        const char* pUtf8 = reinterpret_cast<const char*>(pRaw);
        int cch = MultiByteToWideChar(CP_UTF8, 0, pUtf8, static_cast<int>(cbSize), nullptr, 0);
        if (cch > 0)
        {
            m_uiText.resize(cch);
            MultiByteToWideChar(CP_UTF8, 0, pUtf8, static_cast<int>(cbSize), &m_uiText[0], cch);
        }
    }

    FreeResource(hData);

    if (m_uiText.empty())
        return E_FAIL;

    return S_OK;
}

// ---------------------------------------------------------------------------
// IDuiDataSourceImpl
// ---------------------------------------------------------------------------
static int g_IDuiDataSourceImpl_mark = 1;

IDuiDataSourceImpl::IDuiDataSourceImpl()
    : m_refCount(1)
{
}

IDuiDataSourceImpl::~IDuiDataSourceImpl()
{
}

STDMETHODIMP IDuiDataSourceImpl::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (riid == IID_IUnknown || riid == IID_IConnectionPointContainer)
    {
        *ppv = static_cast<IConnectionPointContainer*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) IDuiDataSourceImpl::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) IDuiDataSourceImpl::Release()
{
    ULONG count = InterlockedDecrement(&m_refCount);
    if (count == 0)
        delete this;
    return count;
}

STDMETHODIMP IDuiDataSourceImpl::FindConnectionPoint(REFIID riid, IConnectionPoint** ppCp)
{
    UNREFERENCED_PARAMETER(riid);
    if (!ppCp) return E_POINTER;
    *ppCp = nullptr;
    return 0x80040202; // CONNECT_E_NOCONNECTION
}

STDMETHODIMP IDuiDataSourceImpl::EnumConnectionPoints(IEnumConnectionPoints** ppEnum)
{
    if (!ppEnum) return E_POINTER;
    *ppEnum = nullptr;
    return E_NOTIMPL;
}

int IDuiDataSourceImpl::GetLength() const
{
    return 0;
}

HRESULT IDuiDataSourceImpl::RetrieveItems(int index, int count, void** items)
{
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(count);
    if (!items) return E_POINTER;
    *items = nullptr;
    return E_INVALIDARG;
}

HRESULT IDuiDataSourceImpl::AddItems(int index, int count)
{
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(count);
    return E_NOTIMPL;
}

HRESULT IDuiDataSourceImpl::RemoveItems(int index, int count)
{
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(count);
    return E_NOTIMPL;
}

bool IDuiDataSourceImpl::CanModifyList() const
{
    return true;
}

HRESULT IDuiDataSourceImpl::FireRemoved(int index, int count)
{
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(count);
    return S_OK;
}

HRESULT IDuiDataSourceImpl::FireAdded(int index, int count)
{
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(count);
    return S_OK;
}

HRESULT IDuiDataSourceImpl::FireReset()
{
    return S_OK;
}

// ---------------------------------------------------------------------------
// VirtualListView
// ---------------------------------------------------------------------------
static int g_VirtualListView_mark = 1;

HRESULT VirtualListView::Register()
{
    return S_OK;
}

} // namespace DirectUI

// ---------------------------------------------------------------------------
// Free functions
// ---------------------------------------------------------------------------
static int g_RMFindModuleForResource_mark = 1;
static int g_RMFindModule_mark = 2;
static int g_RMUpdateResourceSet_mark = 3;
static int g_RMLoadString_mark = 4;
static int g_RMLoadStringBSTR_mark = 5;
static int g_RMLoadImage_mark = 6;
static int g_RMLoadMenu_mark = 7;
static int g_StrToID_mark = 8;
static int g_GetMessageEx_mark = 9;
static int g_PeekMessageEx_mark = 10;
static int g_LayerManagerInitThread_mark = 11;
static int g_LayerManagerUnInitThread_mark = 12;
static int g_DuiGetLayerManager_mark = 13;
static int g_ElementFromGadget_mark = 14;
static int g_GetGadgetRect_mark = 15;
static int g_GetGadgetSize_mark = 16;
static int g_GetTopHWNDParent_mark = 17;

static DWORD s_dwTlsLayerManager = TLS_OUT_OF_INDEXES;

extern "C" {

__declspec(dllexport) HMODULE RMFindModuleForResource(HMODULE hMod, UINT id)
{
    if (!hMod) return nullptr;
    if (FindResourceW(hMod, MAKEINTRESOURCEW(id), RT_STRING) ||
        FindResourceW(hMod, MAKEINTRESOURCEW(id), RT_RCDATA) ||
        FindResourceW(hMod, MAKEINTRESOURCEW(id), RT_BITMAP) ||
        FindResourceW(hMod, MAKEINTRESOURCEW(id), RT_MENU) ||
        FindResourceW(hMod, MAKEINTRESOURCEW(id), L"UIFILE") ||
        FindResourceW(hMod, MAKEINTRESOURCEW(id), L"DUI"))
        return hMod;
    return nullptr;
}

__declspec(dllexport) HMODULE RMFindModule(HMODULE hMod, const wchar_t* name)
{
    if (!name)
        return hMod;
    return GetModuleHandleW(name);
}

__declspec(dllexport) void RMUpdateResourceSet(HMODULE hMod)
{
    UNREFERENCED_PARAMETER(hMod);
}

__declspec(dllexport) int RMLoadString(HINSTANCE hinst, UINT id, wchar_t* buf, int cch)
{
    return LoadStringW(hinst, id, buf, cch);
}

__declspec(dllexport) BSTR RMLoadStringBSTR(HINSTANCE hinst, UINT id)
{
    if (!hinst) return nullptr;
    int cch = LoadStringW(hinst, id, nullptr, 0);
    if (cch <= 0) return nullptr;
    BSTR bstr = SysAllocStringLen(nullptr, cch);
    if (!bstr) return nullptr;
    LoadStringW(hinst, id, bstr, cch + 1);
    return bstr;
}

__declspec(dllexport) HANDLE RMLoadImage(HINSTANCE hinst, UINT id)
{
    if (!hinst) return nullptr;
    return LoadImageW(hinst, MAKEINTRESOURCEW(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
}

__declspec(dllexport) HMENU RMLoadMenu(HINSTANCE hinst, UINT id)
{
    if (!hinst) return nullptr;
    return LoadMenuW(hinst, MAKEINTRESOURCEW(id));
}

__declspec(dllexport) int StrToID(const wchar_t* str)
{
    if (!str) return 0;
    return _wtoi(str);
}

__declspec(dllexport) BOOL GetMessageEx(MSG* msg, HWND hWnd, UINT msgFilterMin, UINT msgFilterMax, UINT flags)
{
    UNREFERENCED_PARAMETER(flags);
    return GetMessageW(msg, hWnd, msgFilterMin, msgFilterMax);
}

__declspec(dllexport) BOOL PeekMessageEx(MSG* msg, HWND hWnd, UINT msgFilterMin, UINT msgFilterMax, UINT removeMsg, UINT flags)
{
    UNREFERENCED_PARAMETER(flags);
    return PeekMessageW(msg, hWnd, msgFilterMin, msgFilterMax, removeMsg);
}

__declspec(dllexport) HRESULT LayerManagerInitThread()
{
    if (s_dwTlsLayerManager == TLS_OUT_OF_INDEXES)
        s_dwTlsLayerManager = TlsAlloc();
    if (s_dwTlsLayerManager == TLS_OUT_OF_INDEXES)
        return E_OUTOFMEMORY;
    if (!TlsGetValue(s_dwTlsLayerManager))
        TlsSetValue(s_dwTlsLayerManager, reinterpret_cast<void*>(1));
    return S_OK;
}

__declspec(dllexport) void LayerManagerUnInitThread()
{
    if (s_dwTlsLayerManager != TLS_OUT_OF_INDEXES)
        TlsSetValue(s_dwTlsLayerManager, nullptr);
}

__declspec(dllexport) void* DuiGetLayerManager()
{
    return nullptr;
}

__declspec(dllexport) void* ElementFromGadget(void* hGadget)
{
    return hGadget;
}

__declspec(dllexport) BOOL GetGadgetRect(void* hGadget, RECT* rc)
{
    if (rc) SetRectEmpty(rc);
    if (!hGadget) return FALSE;
const DirectUI::Element* el = static_cast<const DirectUI::Element*>(hGadget);
    if (rc) *rc = el->GetRect();
    return TRUE;
}

__declspec(dllexport) BOOL GetGadgetSize(void* hGadget, SIZE* sz)
{
    if (sz) { sz->cx = 0; sz->cy = 0; }
    if (!hGadget) return FALSE;
    const DirectUI::Element* el = static_cast<const DirectUI::Element*>(hGadget);
    RECT rc = el->GetRect();
    if (sz) { sz->cx = rc.right - rc.left; sz->cy = rc.bottom - rc.top; }
    return TRUE;
}

__declspec(dllexport) HWND GetTopHWNDParent(HWND hWnd)
{
    HWND parent = hWnd;
    while (parent)
    {
        HWND next = GetParent(parent);
        if (!next) break;
        parent = next;
    }
    return parent;
}

__declspec(dllexport) void* Internal_GetKeyFocusedElement_HWNDElement(void* hwndElement)
{
    UNREFERENCED_PARAMETER(hwndElement);
    if (DirectUI::Element::g_focusedElement)
        return DirectUI::Element::g_focusedElement;
    return nullptr;
}

} // extern "C"

// Export alias for the decorated name expected by callers
#pragma comment(linker, "/export:GetKeyFocusedElement@HWNDElement=Internal_GetKeyFocusedElement_HWNDElement")
