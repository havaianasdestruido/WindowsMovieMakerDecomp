#pragma once
#include <windows.h>
#include <ole2.h>
#include <map>
#include <vector>
#include <string>

#pragma warning(push)
#pragma warning(disable: 4251)

#include "Element.h"

namespace DirectUI {

class CRMDUIParser;

class __declspec(dllexport) HWNDElement : public Element {
public:
    static HRESULT Create(HWNDElement** ppElement);
    static wchar_t const* Class() { return L"HWNDElement"; }

    HWND GetRootHWND();
    HWNDElement* GetKeyFocusedElement();

protected:
    HWND m_hwnd = nullptr;
};

class __declspec(dllexport) HWNDHost : public Element {
public:
    static wchar_t const* Class() { return L"HWNDHost"; }
};

class __declspec(dllexport) NativeHWNDHost : public Element {
public:
    HRESULT Host(HWND hwnd);
    HRESULT SetDefaultFocus();
    HRESULT DestroyWindow();
    HRESULT Initialize();
};

class __declspec(dllexport) CFramelessHost : public HWNDElement {
public:
    HRESULT OnCreateRegion();
    HRESULT OnUpdateFrame();
    HRESULT OnDefaultFrameColorChanged();
};

class __declspec(dllexport) CDUIDialog : public CFramelessHost {
public:
    CDUIDialog();
    virtual ~CDUIDialog();

    static wchar_t const* DIALOG_CLASS() { return L"CDUIDialog"; }

    // Dialog lifecycle
    HRESULT OnCreate();
    HRESULT OnInitDialog();
    HRESULT OnPostCreateDialog();
    HRESULT OnClose();
    HRESULT OnDestroy();
    HRESULT OnNCDestroy();
    HRESULT OnSize(UINT type, int cx, int cy);
    HRESULT OnCommand(UINT ctrlId, UINT notifyCode, HWND hwndCtrl);
    HRESULT OnActivate(UINT state, HWND hwndOther, bool minimized);
    HRESULT OnSysCommand(UINT cmd, POINT pt);
    HRESULT OnGetMinMaxInfo(MINMAXINFO* mmi);
    HRESULT OnInitIcon();
    HRESULT OnShowWindow(bool show, UINT status);
    HRESULT OnSaveHiddenState();
    HRESULT OnLoadHiddenState();

    // Message processing
    HRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    HRESULT FilterMessage(MSG* msg);

    // Dialog operations
    int DoModal(HWND hwndParent, HINSTANCE hInstance, int dlgId, Element* root);
    int DoModeless(HWND hwndParent, HINSTANCE hInstance, int dlgId, Element* root);
    HRESULT ExitDialog(int result);
    HRESULT ShowDialog(bool show);
    HWND GetDialogHWND();
    Element* FindDialogElement(int id);
    CRMDUIParser* GetDUIParser();

protected:
    HINSTANCE m_hInstance = nullptr;
    int m_dlgId = 0;
    int m_result = 0;
    bool m_isModal = false;
    CRMDUIParser* m_parser = nullptr;
};

class __declspec(dllexport) PopupWindow : public HWNDElement {
public:
    static HRESULT Create(PopupWindow** ppPopup);

    static wchar_t const* Class() { return L"PopupWindow"; }
};

class __declspec(dllexport) SuperPopup : public Element {
public:
    static HRESULT Create(SuperPopup** ppPopup);
    static wchar_t const* Class() { return L"SuperPopup"; }

    HRESULT InsertItem(int index, Element* item);
    HRESULT SetNoPrefixOption(bool noPrefix);
    HRESULT CreatePopupMenu();
};

class __declspec(dllexport) DialogHost : public Element {
public:
    static wchar_t const* Class() { return L"DialogHost"; }
    static PropertyInfo* FrameTitleProp;
};

class __declspec(dllexport) TabButton : public Element {
public:
    static wchar_t const* Class() { return L"TabButton"; }
};

class __declspec(dllexport) TabPage : public Element {
public:
    static wchar_t const* Class() { return L"TabPage"; }
};

class __declspec(dllexport) TabControl : public Element {
public:
    static wchar_t const* Class() { return L"TabControl"; }
    HRESULT SetFocusOnChild(int index);
};

class __declspec(dllexport) BorderSplitter : public Element {
public:
    static wchar_t const* Class() { return L"BorderSplitter"; }
    HRESULT Resize();
};

} // namespace DirectUI

#pragma warning(pop)
