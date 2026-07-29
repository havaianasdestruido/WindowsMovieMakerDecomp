#pragma once
#include <windows.h>
#include <ole2.h>
#include <ocidl.h>
#include <vector>
#include <string>

#pragma warning(push)
#pragma warning(disable: 4251)

namespace DirectUI {

// ---------------------------------------------------------------------------
// CRMStringResource — loads localized strings from modules
// ---------------------------------------------------------------------------
class __declspec(dllexport) CRMStringResource {
public:
    CRMStringResource();
    CRMStringResource(HINSTANCE hinst, UINT id);
    ~CRMStringResource();

    HRESULT Load(HINSTANCE hinst, UINT id);
    int Length() const;
    const wchar_t* ToString() const;

private:
    std::wstring m_str;
};

// ---------------------------------------------------------------------------
// CRMResource — base resource loader
// ---------------------------------------------------------------------------
class __declspec(dllexport) CRMResource {
public:
    CRMResource();
    virtual ~CRMResource();
};

// ---------------------------------------------------------------------------
// CRMImage — image resource (GDI+ wrapper)
// ---------------------------------------------------------------------------
class __declspec(dllexport) CRMImage {
public:
    CRMImage();
    ~CRMImage();

    HRESULT LoadFromResource(HINSTANCE hinst, UINT id);
    HBITMAP Detach();
    void Attach(HBITMAP hbm);
    HRESULT ConvertToARGB();

private:
    HBITMAP m_hBitmap;
};

// ---------------------------------------------------------------------------
// CRMDUIParser — UIFILE parser
// ---------------------------------------------------------------------------
class __declspec(dllexport) CRMDUIParser {
public:
    CRMDUIParser();
    ~CRMDUIParser();

    HRESULT Load(HINSTANCE hinst, UINT id, DWORD flags = 0);
};

// ---------------------------------------------------------------------------
// IDuiDataSourceImpl — virtual list data source (IConnectionPointContainer)
// ---------------------------------------------------------------------------
class __declspec(dllexport) IDuiDataSourceImpl : public IConnectionPointContainer {
public:
    IDuiDataSourceImpl();
    virtual ~IDuiDataSourceImpl();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IConnectionPointContainer
    STDMETHODIMP FindConnectionPoint(REFIID riid, IConnectionPoint** ppCp) override;
    STDMETHODIMP EnumConnectionPoints(IEnumConnectionPoints** ppEnum) override;

    // Data source methods
    virtual int GetLength() const;
    virtual HRESULT RetrieveItems(int index, int count, void** items);
    virtual HRESULT AddItems(int index, int count);
    virtual HRESULT RemoveItems(int index, int count);
    virtual bool CanModifyList() const;

    // Firing notifications
    HRESULT FireRemoved(int index, int count);
    HRESULT FireAdded(int index, int count);
    HRESULT FireReset();

protected:
    LONG m_refCount;
};

// ---------------------------------------------------------------------------
// VirtualListView — virtual list view element
// ---------------------------------------------------------------------------
class __declspec(dllexport) VirtualListView {
public:
    static HRESULT Register();
};

} // namespace DirectUI

// ---------------------------------------------------------------------------
// Free functions (extern "C")
// ---------------------------------------------------------------------------
extern "C" {

__declspec(dllexport) HMODULE RMFindModuleForResource(HMODULE hMod, UINT id);
__declspec(dllexport) HMODULE RMFindModule(HMODULE hMod, const wchar_t* name);
__declspec(dllexport) void RMUpdateResourceSet(HMODULE hMod);
__declspec(dllexport) int RMLoadString(HINSTANCE hinst, UINT id, wchar_t* buf, int cch);
__declspec(dllexport) BSTR RMLoadStringBSTR(HINSTANCE hinst, UINT id);
__declspec(dllexport) HANDLE RMLoadImage(HINSTANCE hinst, UINT id);
__declspec(dllexport) HMENU RMLoadMenu(HINSTANCE hinst, UINT id);
__declspec(dllexport) int StrToID(const wchar_t* str);

__declspec(dllexport) BOOL GetMessageEx(MSG* msg, HWND hWnd, UINT msgFilterMin, UINT msgFilterMax, UINT flags);
__declspec(dllexport) BOOL PeekMessageEx(MSG* msg, HWND hWnd, UINT msgFilterMin, UINT msgFilterMax, UINT removeMsg, UINT flags);

__declspec(dllexport) HRESULT LayerManagerInitThread();
__declspec(dllexport) void LayerManagerUnInitThread();
__declspec(dllexport) void* DuiGetLayerManager();

__declspec(dllexport) void* ElementFromGadget(void* hGadget);
__declspec(dllexport) BOOL GetGadgetRect(void* hGadget, RECT* rc);
__declspec(dllexport) BOOL GetGadgetSize(void* hGadget, SIZE* sz);
__declspec(dllexport) HWND GetTopHWNDParent(HWND hWnd);

// Exported as GetKeyFocusedElement@HWNDElement via pragma in Resources.cpp
__declspec(dllexport) void* Internal_GetKeyFocusedElement_HWNDElement(void* hwndElement);

} // extern "C"

#pragma warning(pop)
