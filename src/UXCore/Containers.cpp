#include "Containers.h"
#include "Element.h"
#include "Value.h"

namespace DirectUI {

// Unique markers to prevent COMDAT folding of identical stub bodies
static int g_CreateHWNDElement_mark = 101;
static int g_GetRootHWND_mark = 102;
static int g_GetKeyFocusedElement_mark = 103;
static int g_Host_mark = 104;
static int g_SetDefaultFocus_mark = 105;
static int g_DestroyWindow_mark = 106;
static int g_Initialize_mark = 107;
static int g_OnCreateRegion_mark = 108;
static int g_OnUpdateFrame_mark = 109;
static int g_OnDefaultFrameColorChanged_mark = 110;
static int g_DoModal_mark = 111;
static int g_DoModeless_mark = 112;
static int g_ExitDialog_mark = 113;
static int g_ShowDialog_mark = 114;
static int g_GetDialogHWND_mark = 115;
static int g_FindDialogElement_mark = 116;
static int g_GetDUIParser_mark = 117;
static int g_CreatePopupWindow_mark = 118;
static int g_CreateSuperPopup_mark = 119;
static int g_InsertItem_mark = 120;
static int g_SetNoPrefixOption_mark = 121;
static int g_CreatePopupMenu_mark = 122;
static int g_SetFocusOnChild_mark = 123;
static int g_ResizeBorderSplitter_mark = 124;

// PropertyInfo definitions
PropertyInfo g_FrameTitleProp = { L"FrameTitle" };

PropertyInfo* DialogHost::FrameTitleProp = &g_FrameTitleProp;

// ====================================================================
// HWNDElement
// ====================================================================
HRESULT HWNDElement::Create(HWNDElement** ppElement)
{
    if (!ppElement) return E_POINTER;
    *ppElement = new HWNDElement();
    return (*ppElement) ? S_OK : E_OUTOFMEMORY;
}

HWND HWNDElement::GetRootHWND()
{
    return (g_GetRootHWND_mark > 0) ? m_hwnd : NULL;
}

HWNDElement* HWNDElement::GetKeyFocusedElement()
{
    return (g_GetKeyFocusedElement_mark > 0) ? nullptr : this;
}

// ====================================================================
// HWNDHost
// ====================================================================

// ====================================================================
// NativeHWNDHost
// ====================================================================
HRESULT NativeHWNDHost::Host(HWND hwnd)
{
    UNREFERENCED_PARAMETER(hwnd);
    return (g_Host_mark > 0) ? S_OK : E_FAIL;
}

HRESULT NativeHWNDHost::SetDefaultFocus()
{
    return (g_SetDefaultFocus_mark > 0) ? S_OK : E_FAIL;
}

HRESULT NativeHWNDHost::DestroyWindow()
{
    return (g_DestroyWindow_mark > 0) ? S_OK : E_FAIL;
}

HRESULT NativeHWNDHost::Initialize()
{
    return (g_Initialize_mark > 0) ? S_OK : E_FAIL;
}

// ====================================================================
// CFramelessHost
// ====================================================================
HRESULT CFramelessHost::OnCreateRegion()
{
    return (g_OnCreateRegion_mark > 0) ? S_OK : E_FAIL;
}

HRESULT CFramelessHost::OnUpdateFrame()
{
    return (g_OnUpdateFrame_mark > 0) ? S_OK : E_FAIL;
}

HRESULT CFramelessHost::OnDefaultFrameColorChanged()
{
    return (g_OnDefaultFrameColorChanged_mark > 0) ? S_OK : E_FAIL;
}

// ====================================================================
// CDUIDialog
// ====================================================================
CDUIDialog::CDUIDialog()
    : m_hInstance(nullptr)
    , m_dlgId(0)
    , m_result(0)
    , m_isModal(false)
    , m_parser(nullptr)
{
}

CDUIDialog::~CDUIDialog()
{
}

HRESULT CDUIDialog::OnCreate()
{
    return S_OK;
}

HRESULT CDUIDialog::OnInitDialog()
{
    return S_OK;
}

HRESULT CDUIDialog::OnPostCreateDialog()
{
    return S_OK;
}

HRESULT CDUIDialog::OnClose()
{
    DestroyWindow(m_hwnd);
    m_hwnd = nullptr;
    return S_OK;
}

HRESULT CDUIDialog::OnDestroy()
{
    m_hwnd = nullptr;
    return S_OK;
}

HRESULT CDUIDialog::OnNCDestroy()
{
    return S_OK;
}

HRESULT CDUIDialog::OnPaint()
{
    if (!m_hwnd)
        return S_OK;

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);
    if (!hdc)
        return E_FAIL;

    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);

    Paint(hdc, &clientRect);

    EndPaint(m_hwnd, &ps);

    return S_OK;
}

HRESULT CDUIDialog::OnSize(UINT type, int cx, int cy)
{
    UNREFERENCED_PARAMETER(type);

    m_rect.left = 0;
    m_rect.top = 0;
    m_rect.right = cx;
    m_rect.bottom = cy;

    _UpdateDesiredSize();
    _UpdateLayoutSize();
    _UpdateLayoutPosition();
    Invalidate();

    return S_OK;
}

HRESULT CDUIDialog::OnCommand(UINT ctrlId, UINT notifyCode, HWND hwndCtrl)
{
    UNREFERENCED_PARAMETER(ctrlId);
    UNREFERENCED_PARAMETER(notifyCode);
    UNREFERENCED_PARAMETER(hwndCtrl);
    return S_OK;
}

HRESULT CDUIDialog::OnActivate(UINT state, HWND hwndOther, bool minimized)
{
    UNREFERENCED_PARAMETER(state);
    UNREFERENCED_PARAMETER(hwndOther);
    UNREFERENCED_PARAMETER(minimized);
    return S_OK;
}

HRESULT CDUIDialog::OnSysCommand(UINT cmd, POINT pt)
{
    UNREFERENCED_PARAMETER(cmd);
    UNREFERENCED_PARAMETER(pt);
    return S_OK;
}

HRESULT CDUIDialog::OnGetMinMaxInfo(MINMAXINFO* mmi)
{
    UNREFERENCED_PARAMETER(mmi);
    return S_OK;
}

HRESULT CDUIDialog::OnInitIcon()
{
    return S_OK;
}

HRESULT CDUIDialog::OnShowWindow(bool show, UINT status)
{
    UNREFERENCED_PARAMETER(show);
    UNREFERENCED_PARAMETER(status);
    return S_OK;
}

HRESULT CDUIDialog::OnSaveHiddenState()
{
    return S_OK;
}

HRESULT CDUIDialog::OnLoadHiddenState()
{
    return S_OK;
}

HRESULT CDUIDialog::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(msg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pResult) *pResult = FALSE;
    return S_OK;
}

HRESULT CDUIDialog::FilterMessage(MSG* msg)
{
    UNREFERENCED_PARAMETER(msg);
    return S_FALSE;
}

int CDUIDialog::DoModal(HWND hwndParent, HINSTANCE hInstance, int dlgId, Element* root)
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(dlgId);
    UNREFERENCED_PARAMETER(root);
    m_isModal = true;
    m_result = IDOK;
    return (g_DoModal_mark > 0) ? m_result : 0;
}

int CDUIDialog::DoModeless(HWND hwndParent, HINSTANCE hInstance, int dlgId, Element* root)
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(dlgId);
    UNREFERENCED_PARAMETER(root);
    m_isModal = false;
    return (g_DoModeless_mark > 0) ? S_OK : E_FAIL;
}

HRESULT CDUIDialog::ExitDialog(int result)
{
    m_result = result;
    if (m_isModal && m_hwnd)
    {
        ::EndDialog(m_hwnd, result);
    }
    return S_OK;
}

HRESULT CDUIDialog::ShowDialog(bool show)
{
    if (m_hwnd)
    {
        ShowWindow(m_hwnd, show ? SW_SHOW : SW_HIDE);
    }
    return S_OK;
}

HWND CDUIDialog::GetDialogHWND()
{
    return (g_GetDialogHWND_mark > 0) ? m_hwnd : NULL;
}

Element* CDUIDialog::FindDialogElement(int id)
{
    UNREFERENCED_PARAMETER(id);
    return (g_FindDialogElement_mark > 0) ? nullptr : this;
}

CRMDUIParser* CDUIDialog::GetDUIParser()
{
    return (g_GetDUIParser_mark > 0) ? m_parser : nullptr;
}

// ====================================================================
// PopupWindow
// ====================================================================
HRESULT PopupWindow::Create(PopupWindow** ppPopup)
{
    if (!ppPopup) return E_POINTER;
    *ppPopup = new PopupWindow();
    return (*ppPopup) ? S_OK : E_OUTOFMEMORY;
}

// ====================================================================
// SuperPopup
// ====================================================================
HRESULT SuperPopup::Create(SuperPopup** ppPopup)
{
    if (!ppPopup) return E_POINTER;
    *ppPopup = new SuperPopup();
    return (*ppPopup) ? S_OK : E_OUTOFMEMORY;
}

HRESULT SuperPopup::InsertItem(int index, Element* item)
{
    UNREFERENCED_PARAMETER(index);
    if (!item) return E_POINTER;
    m_children.push_back(item);
    return (g_InsertItem_mark > 0) ? S_OK : E_FAIL;
}

HRESULT SuperPopup::SetNoPrefixOption(bool noPrefix)
{
    UNREFERENCED_PARAMETER(noPrefix);
    return (g_SetNoPrefixOption_mark > 0) ? S_OK : E_FAIL;
}

HRESULT SuperPopup::CreatePopupMenu()
{
    return (g_CreatePopupMenu_mark > 0) ? S_OK : E_FAIL;
}

// ====================================================================
// DialogHost
// ====================================================================

// ====================================================================
// TabButton
// ====================================================================

// ====================================================================
// TabPage
// ====================================================================

// ====================================================================
// TabControl
// ====================================================================
HRESULT TabControl::SetFocusOnChild(int index)
{
    UNREFERENCED_PARAMETER(index);
    return (g_SetFocusOnChild_mark > 0) ? S_OK : E_FAIL;
}

// ====================================================================
// BorderSplitter
// ====================================================================
HRESULT BorderSplitter::Resize()
{
    return (g_ResizeBorderSplitter_mark > 0) ? S_OK : E_FAIL;
}

} // namespace DirectUI
