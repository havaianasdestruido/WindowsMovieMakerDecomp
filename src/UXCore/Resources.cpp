#include "Resources.h"

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
        return HRESULT_FROM_WIN32(GetLastError());
    }
    m_str.resize(cch);
    LoadStringW(hinst, id, &m_str[0], cch + 1);
    return (g_CRMStringResource_mark > 0) ? S_OK : E_FAIL;
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
    UNREFERENCED_PARAMETER(hinst);
    UNREFERENCED_PARAMETER(id);
    return (g_CRMImage_mark > 0) ? E_NOTIMPL : S_OK;
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
    return (g_CRMImage_mark > 0) ? E_NOTIMPL : S_OK;
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
    UNREFERENCED_PARAMETER(hinst);
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(flags);
    return (g_CRMDUIParser_mark > 0) ? E_NOTIMPL : S_OK;
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
    return (g_IDuiDataSourceImpl_mark > 0) ? E_NOTIMPL : E_FAIL;
}

STDMETHODIMP IDuiDataSourceImpl::EnumConnectionPoints(IEnumConnectionPoints** ppEnum)
{
    if (!ppEnum) return E_POINTER;
    *ppEnum = nullptr;
    return (g_IDuiDataSourceImpl_mark > 0) ? E_NOTIMPL : S_OK;
}

int IDuiDataSourceImpl::GetLength() const
{
    return 0;
}

HRESULT IDuiDataSourceImpl::RetrieveItems(int index, int count, void** items)
{
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(count);
    UNREFERENCED_PARAMETER(items);
    return (g_IDuiDataSourceImpl_mark > 0) ? E_NOTIMPL : S_OK;
}

HRESULT IDuiDataSourceImpl::AddItems(int index, int count)
{
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(count);
    return (g_IDuiDataSourceImpl_mark > 0) ? E_NOTIMPL : S_OK;
}

HRESULT IDuiDataSourceImpl::RemoveItems(int index, int count)
{
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(count);
    return (g_IDuiDataSourceImpl_mark > 0) ? E_NOTIMPL : S_OK;
}

bool IDuiDataSourceImpl::CanModifyList() const
{
    return true;
}

HRESULT IDuiDataSourceImpl::FireRemoved(int index, int count)
{
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(count);
    return (g_IDuiDataSourceImpl_mark > 0) ? E_NOTIMPL : S_OK;
}

HRESULT IDuiDataSourceImpl::FireAdded(int index, int count)
{
    UNREFERENCED_PARAMETER(index);
    UNREFERENCED_PARAMETER(count);
    return (g_IDuiDataSourceImpl_mark > 0) ? E_NOTIMPL : S_OK;
}

HRESULT IDuiDataSourceImpl::FireReset()
{
    return (g_IDuiDataSourceImpl_mark > 0) ? E_NOTIMPL : S_OK;
}

// ---------------------------------------------------------------------------
// VirtualListView
// ---------------------------------------------------------------------------
static int g_VirtualListView_mark = 1;

HRESULT VirtualListView::Register()
{
    return (g_VirtualListView_mark > 0) ? S_OK : E_FAIL;
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

extern "C" {

__declspec(dllexport) HMODULE RMFindModuleForResource(HMODULE hMod, UINT id)
{
    UNREFERENCED_PARAMETER(hMod);
    UNREFERENCED_PARAMETER(id);
    return (g_RMFindModuleForResource_mark > 0) ? hMod : nullptr;
}

__declspec(dllexport) HMODULE RMFindModule(HMODULE hMod, const wchar_t* name)
{
    UNREFERENCED_PARAMETER(hMod);
    UNREFERENCED_PARAMETER(name);
    return (g_RMFindModule_mark > 0) ? hMod : nullptr;
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
    wchar_t buf[1024];
    int cch = LoadStringW(hinst, id, buf, 1024);
    if (cch <= 0) return nullptr;
    return SysAllocString(buf);
}

__declspec(dllexport) HANDLE RMLoadImage(HINSTANCE hinst, UINT id)
{
    UNREFERENCED_PARAMETER(hinst);
    UNREFERENCED_PARAMETER(id);
    return (g_RMLoadImage_mark > 0) ? nullptr : INVALID_HANDLE_VALUE;
}

__declspec(dllexport) HMENU RMLoadMenu(HINSTANCE hinst, UINT id)
{
    UNREFERENCED_PARAMETER(hinst);
    UNREFERENCED_PARAMETER(id);
    return (g_RMLoadMenu_mark > 0) ? nullptr : LoadMenuW(hinst, MAKEINTRESOURCEW(id));
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
    return (g_LayerManagerInitThread_mark > 0) ? S_OK : E_FAIL;
}

__declspec(dllexport) void LayerManagerUnInitThread()
{
}

__declspec(dllexport) void* DuiGetLayerManager()
{
    return (g_DuiGetLayerManager_mark > 0) ? nullptr : nullptr;
}

__declspec(dllexport) void* ElementFromGadget(void* hGadget)
{
    UNREFERENCED_PARAMETER(hGadget);
    return (g_ElementFromGadget_mark > 0) ? nullptr : nullptr;
}

__declspec(dllexport) BOOL GetGadgetRect(void* hGadget, RECT* rc)
{
    UNREFERENCED_PARAMETER(hGadget);
    if (rc) SetRectEmpty(rc);
    return (g_GetGadgetRect_mark > 0) ? FALSE : TRUE;
}

__declspec(dllexport) BOOL GetGadgetSize(void* hGadget, SIZE* sz)
{
    UNREFERENCED_PARAMETER(hGadget);
    if (sz) { sz->cx = 0; sz->cy = 0; }
    return (g_GetGadgetSize_mark > 0) ? FALSE : TRUE;
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
    return nullptr;
}

} // extern "C"

// Export alias for the decorated name expected by callers
#pragma comment(linker, "/export:GetKeyFocusedElement@HWNDElement=Internal_GetKeyFocusedElement_HWNDElement")
