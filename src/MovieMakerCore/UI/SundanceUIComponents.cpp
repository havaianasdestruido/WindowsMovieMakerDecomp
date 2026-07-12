/*
 * SundanceUIComponents.cpp
 *
 * Implementation of core Sundance UI component behaviors.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "SundanceUIComponents.h"
#include "../SundanceApp/SundanceAppMain.h"

namespace Sundance
{

// ============================================================================
// SundanceMainElementBehavior
// ============================================================================
SundanceMainElementBehavior::SundanceMainElementBehavior()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_hWndHost(NULL)
    , m_viewMode(ViewModeNormal)
{
}

SundanceMainElementBehavior::~SundanceMainElementBehavior()
{
    m_pElement = NULL;
    m_pAppMain = NULL;
}

HRESULT SundanceMainElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement)
        return E_POINTER;

    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundanceMainElementBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    m_pElement = NULL;
    return S_OK;
}

HRESULT SundanceMainElementBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled)
        *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_SIZE:
        if (m_pElement)
            m_pElement->InvalidateRect(NULL);
        if (pbHandled) *pbHandled = TRUE;
        break;

    case WM_SETTINGCHANGE:
        RefreshLayout();
        if (pbHandled) *pbHandled = TRUE;
        break;
    }

    return S_OK;
}

HRESULT SundanceMainElementBehavior::OnBehaviorChanged()
{
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
    return S_OK;
}

HRESULT SundanceMainElementBehavior::LoadLayout(LPCWSTR pszDuxtResource)
{
    UNREFERENCED_PARAMETER(pszDuxtResource);
    // In the real implementation, this parses a .duxt binary resource
    // and populates the DirectUI element tree.
    return S_OK;
}

HRESULT SundanceMainElementBehavior::RefreshLayout()
{
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
    return S_OK;
}

HWND SundanceMainElementBehavior::GetHostHwnd() const throw()
{
    return m_hWndHost;
}

void SundanceMainElementBehavior::SetHostHwnd(HWND hWnd) throw()
{
    m_hWndHost = hWnd;
}

HRESULT SundanceMainElementBehavior::SetViewMode(ViewMode mode)
{
    m_viewMode = mode;

    if (m_pElement)
        m_pElement->InvalidateRect(NULL);

    return S_OK;
}

SundanceMainElementBehavior::ViewMode SundanceMainElementBehavior::GetViewMode() const throw()
{
    return m_viewMode;
}

// ============================================================================
// SundanceBehaviorFactory
// ============================================================================
SundanceBehaviorFactory::SundanceBehaviorFactory()
    : m_bInitialized(false)
{
}

SundanceBehaviorFactory::~SundanceBehaviorFactory()
{
}

SundanceBehaviorFactory& SundanceBehaviorFactory::GetInstance()
{
    static SundanceBehaviorFactory s_instance;
    return s_instance;
}

HRESULT SundanceBehaviorFactory::CreateBehaviorByName(LPCWSTR pszClassName, IDuiBehavior** ppBehavior)
{
    if (!ppBehavior)
        return E_POINTER;

    *ppBehavior = NULL;

    EnsureInitialized();

    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        if (m_entries[i].strClassName.CompareNoCase(pszClassName) == 0)
        {
            // Found a registered factory -- invoke it
            typedef HRESULT (CALLBACK* PFN_CREATE)(IDuiBehavior**);
            PFN_CREATE pfn = reinterpret_cast<PFN_CREATE>(m_entries[i].pfnFactory);
            if (pfn)
                return pfn(ppBehavior);
        }
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

bool SundanceBehaviorFactory::IsBehaviorClass(LPCWSTR pszClassName) const
{
    if (!pszClassName)
        return false;

    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        if (m_entries[i].strClassName.CompareNoCase(pszClassName) == 0)
            return true;
    }

    return false;
}

HRESULT SundanceBehaviorFactory::GetRegisteredClasses(std::vector<ATL::CString>& classNames)
{
    classNames.clear();

    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        classNames.push_back(m_entries[i].strClassName);
    }

    return S_OK;
}

HRESULT SundanceBehaviorFactory::RegisterBehaviorClass(LPCWSTR pszClassName, void* pfnFactory)
{
    if (!pszClassName || !pfnFactory)
        return E_INVALIDARG;

    FactoryEntry entry;
    entry.strClassName = pszClassName;
    entry.pfnFactory = pfnFactory;
    m_entries.push_back(entry);

    return S_OK;
}

HRESULT SundanceBehaviorFactory::UnregisterBehaviorClass(LPCWSTR pszClassName)
{
    if (!pszClassName)
        return E_INVALIDARG;

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        if (it->strClassName.CompareNoCase(pszClassName) == 0)
        {
            m_entries.erase(it);
            return S_OK;
        }
    }

    return S_FALSE;
}

void SundanceBehaviorFactory::EnsureInitialized()
{
    if (m_bInitialized)
        return;

    // The actual registration happens in SundanceBehaviors.cpp
    // via RegisterSundanceBehaviors(). This method ensures the
    // local factory is populated.
    m_bInitialized = true;
}

// ============================================================================
// AMPMainWindowBehavior
// ============================================================================
AMPMainWindowBehavior::AMPMainWindowBehavior()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_hWndMain(NULL)
    , m_hIcon(NULL)
    , m_bFullscreen(false)
    , m_bMinimizeBox(true)
    , m_bMaximizeBox(true)
    , m_bCloseButton(true)
    , m_clrBackground(RGB(240, 240, 240))
{
}

AMPMainWindowBehavior::~AMPMainWindowBehavior()
{
    m_pElement = NULL;
    m_pAppMain = NULL;
}

HRESULT AMPMainWindowBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement)
        return E_POINTER;

    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();

    if (m_pAppMain)
        m_hWndMain = m_pAppMain->GetMainWindow();

    return S_OK;
}

HRESULT AMPMainWindowBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    m_pElement = NULL;
    m_hWndMain = NULL;
    return S_OK;
}

HRESULT AMPMainWindowBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    if (pbHandled)
        *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_NCCALCSIZE:
        OnNcCalcSize(wParam, lParam);
        if (pbHandled) *pbHandled = TRUE;
        break;

    case WM_NCPAINT:
        OnNcPaint(wParam, lParam);
        if (pbHandled) *pbHandled = TRUE;
        break;

    case WM_NCHITTEST:
    {
        LRESULT lr = 0;
        OnNcHitTest(lParam, &lr);
        if (pbHandled) *pbHandled = TRUE;
        return lr;
    }

    case WM_DWMCOMPOSITIONCHANGED:
        OnDwmCompositionChanged();
        if (pbHandled) *pbHandled = TRUE;
        break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMMI = reinterpret_cast<MINMAXINFO*>(lParam);
        if (pMMI)
        {
            pMMI->ptMinTrackSize.x = 640;
            pMMI->ptMinTrackSize.y = 480;
        }
        if (pbHandled) *pbHandled = TRUE;
        break;
    }
    }

    return S_OK;
}

HRESULT AMPMainWindowBehavior::OnBehaviorChanged()
{
    if (m_pElement)
        m_pElement->InvalidateRect(NULL);
    return S_OK;
}

HRESULT AMPMainWindowBehavior::SetTitle(LPCWSTR pszTitle)
{
    if (!pszTitle)
        return E_INVALIDARG;

    m_strTitle = pszTitle;

    if (m_hWndMain)
        ::SetWindowText(m_hWndMain, pszTitle);

    return S_OK;
}

HRESULT AMPMainWindowBehavior::SetIcon(HICON hIcon)
{
    m_hIcon = hIcon;

    if (m_hWndMain)
        ::SendMessage(m_hWndMain, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));

    return S_OK;
}

HRESULT AMPMainWindowBehavior::SetMinimizeBox(bool bEnable)  { m_bMinimizeBox = bEnable; return S_OK; }
HRESULT AMPMainWindowBehavior::SetMaximizeBox(bool bEnable)  { m_bMaximizeBox = bEnable; return S_OK; }
HRESULT AMPMainWindowBehavior::SetCloseButton(bool bEnable)  { m_bCloseButton = bEnable; return S_OK; }

HRESULT AMPMainWindowBehavior::Maximize()
{
    if (m_hWndMain)
        ::ShowWindow(m_hWndMain, SW_MAXIMIZE);
    return S_OK;
}

HRESULT AMPMainWindowBehavior::Restore()
{
    if (m_hWndMain)
        ::ShowWindow(m_hWndMain, SW_RESTORE);
    return S_OK;
}

HRESULT AMPMainWindowBehavior::Minimize()
{
    if (m_hWndMain)
        ::ShowWindow(m_hWndMain, SW_MINIMIZE);
    return S_OK;
}

HRESULT AMPMainWindowBehavior::SetFullscreen(bool bFullscreen)
{
    m_bFullscreen = bFullscreen;

    if (m_hWndMain)
    {
        if (bFullscreen)
        {
            // Store current window placement before entering fullscreen
            ::ShowWindow(m_hWndMain, SW_MAXIMIZE);
        }
        else
        {
            ::ShowWindow(m_hWndMain, SW_RESTORE);
        }
    }

    return S_OK;
}

bool AMPMainWindowBehavior::IsFullscreen() const throw()
{
    return m_bFullscreen;
}

HRESULT AMPMainWindowBehavior::EnableAeroGlass(bool bEnable)
{
    if (m_hWndMain)
    {
        DWMWA_SYSTEMBACKDROP_TYPE dwAttribute = bEnable
            ? DWMWA_SYSTEMBACKDROP_TYPE_AUTO
            : DWMWA_SYSTEMBACKDROP_TYPE_NONE;

        DwmSetWindowAttribute(m_hWndMain, DWMWA_SYSTEMBACKDROP_TYPE,
            &dwAttribute, sizeof(dwAttribute));
    }

    return S_OK;
}

HRESULT AMPMainWindowBehavior::SetWindowBackgroundColor(COLORREF clrBackground)
{
    m_clrBackground = clrBackground;
    return S_OK;
}

HRESULT AMPMainWindowBehavior::UpdateNonClientArea()
{
    if (m_hWndMain)
        ::SetWindowPos(m_hWndMain, NULL, 0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
    return S_OK;
}

void AMPMainWindowBehavior::OnNcCalcSize(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    // Custom non-client area calculation for AMP chrome
}

void AMPMainWindowBehavior::OnNcPaint(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    // Custom non-client area painting for AMP chrome
}

void AMPMainWindowBehavior::OnNcHitTest(LPARAM lParam, LRESULT* plResult)
{
    if (!plResult) return;

    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);

    POINT pt = { x, y };

    if (m_hWndMain)
    {
        ::ScreenToClient(m_hWndMain, &pt);

        const int kBorderSize = 4;
        RECT rcClient;
        ::GetClientRect(m_hWndMain, &rcClient);

        if (pt.x < kBorderSize)
            *plResult = HTLEFT;
        else if (pt.x >= rcClient.right - kBorderSize)
            *plResult = HTRIGHT;
        else if (pt.y < kBorderSize)
            *plResult = HTTOP;
        else if (pt.y >= rcClient.bottom - kBorderSize)
            *plResult = HTBOTTOM;
        else
            *plResult = HTCLIENT;
    }
    else
    {
        *plResult = HTCLIENT;
    }
}

void AMPMainWindowBehavior::OnDwmCompositionChanged()
{
    // Respond to DWM composition changes (aero on/off)
}

// ============================================================================
// AMPCommandBarBehavior
// ============================================================================
AMPCommandBarBehavior::AMPCommandBarBehavior()
    : m_pElement(NULL)
    , m_pAppMain(NULL)
    , m_bVisible(true)
    , m_nHeight(48)
{
}

AMPCommandBarBehavior::~AMPCommandBarBehavior()
{
    m_pElement = NULL;
    m_pAppMain = NULL;
}

HRESULT AMPCommandBarBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT AMPCommandBarBehavior::OnElementDetached(IDuiElement* /*pElement*/)
{
    m_pElement = NULL;
    return S_OK;
}

HRESULT AMPCommandBarBehavior::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    if (pbHandled) *pbHandled = FALSE;

    switch (uMsg)
    {
    case WM_COMMAND:
        // Route command to SundanceAppMain
        if (m_pAppMain)
            m_pAppMain->UpdateCommandState();
        break;
    }

    return S_OK;
}

HRESULT AMPCommandBarBehavior::UpdateCommandStates()
{
    RefreshCommandStates();

    if (m_pElement)
        m_pElement->InvalidateRect(NULL);

    return S_OK;
}

HRESULT AMPCommandBarBehavior::SetButtonEnabled(LPCWSTR pszCommandId, bool bEnabled)
{
    if (!pszCommandId) return E_INVALIDARG;

    for (size_t i = 0; i < m_commands.size(); ++i)
    {
        if (m_commands[i].strCommandId.CompareNoCase(pszCommandId) == 0)
        {
            m_commands[i].bEnabled = bEnabled;
            return S_OK;
        }
    }

    return S_FALSE;
}

HRESULT AMPCommandBarBehavior::SetButtonChecked(LPCWSTR pszCommandId, bool bChecked)
{
    if (!pszCommandId) return E_INVALIDARG;

    for (size_t i = 0; i < m_commands.size(); ++i)
    {
        if (m_commands[i].strCommandId.CompareNoCase(pszCommandId) == 0)
        {
            m_commands[i].bChecked = bChecked;
            return S_OK;
        }
    }

    return S_FALSE;
}

HRESULT AMPCommandBarBehavior::SetButtonLabel(LPCWSTR pszCommandId, LPCWSTR pszLabel)
{
    if (!pszCommandId || !pszLabel) return E_INVALIDARG;

    for (size_t i = 0; i < m_commands.size(); ++i)
    {
        if (m_commands[i].strCommandId.CompareNoCase(pszCommandId) == 0)
        {
            m_commands[i].strLabel = pszLabel;
            return S_OK;
        }
    }

    return S_FALSE;
}

HRESULT AMPCommandBarBehavior::SetVisible(bool bVisible) throw()
{
    m_bVisible = bVisible;
    return S_OK;
}

bool AMPCommandBarBehavior::IsVisible() const throw()
{
    return m_bVisible;
}

int AMPCommandBarBehavior::GetHeight() const throw()
{
    return m_bVisible ? m_nHeight : 0;
}

void AMPCommandBarBehavior::RefreshCommandStates()
{
    if (!m_pAppMain)
        return;

    SetButtonEnabled(L"Undo", m_pAppMain->CanUndo());
    SetButtonEnabled(L"Redo", m_pAppMain->CanRedo());
    SetButtonEnabled(L"Cut", m_pAppMain->CanCut());
    SetButtonEnabled(L"Copy", m_pAppMain->CanCopy());
    SetButtonEnabled(L"Paste", m_pAppMain->CanPaste());
}

bool AMPCommandBarBehavior::IsCommandRelevant(LPCWSTR pszCommandId) const
{
    UNREFERENCED_PARAMETER(pszCommandId);
    return true;
}

// ============================================================================
// AMPDataContext
// ============================================================================
const WCHAR AMPDataContext::kPropProjectName[]         = L"ProjectName";
const WCHAR AMPDataContext::kPropIsProjectOpen[]       = L"IsProjectOpen";
const WCHAR AMPDataContext::kPropIsProjectDirty[]      = L"IsProjectDirty";
const WCHAR AMPDataContext::kPropCanUndo[]             = L"CanUndo";
const WCHAR AMPDataContext::kPropCanRedo[]             = L"CanRedo";
const WCHAR AMPDataContext::kPropCanCut[]              = L"CanCut";
const WCHAR AMPDataContext::kPropCanCopy[]             = L"CanCopy";
const WCHAR AMPDataContext::kPropCanPaste[]            = L"CanPaste";
const WCHAR AMPDataContext::kPropIsPlaying[]           = L"IsPlaying";
const WCHAR AMPDataContext::kPropIsPublishing[]        = L"IsPublishing";
const WCHAR AMPDataContext::kPropIsEncoding[]          = L"IsEncoding";
const WCHAR AMPDataContext::kPropTimelinePosition[]    = L"TimelinePosition";
const WCHAR AMPDataContext::kPropTimelineDuration[]    = L"TimelineDuration";
const WCHAR AMPDataContext::kPropVersionString[]       = L"VersionString";

AMPDataContext::AMPDataContext()
    : m_pAppMain(NULL)
{
}

AMPDataContext::~AMPDataContext()
{
    m_pAppMain = NULL;

    for (auto it = m_propertyCache.begin(); it != m_propertyCache.end(); ++it)
    {
        VariantClear(&it->second);
    }
    m_propertyCache.clear();
}

HRESULT AMPDataContext::Initialize(SundanceAppMain* pAppMain)
{
    if (!pAppMain)
        return E_POINTER;

    m_pAppMain = pAppMain;
    InitPropertyCache();
    RefreshAll();

    return S_OK;
}

HRESULT AMPDataContext::GetProperty(LPCWSTR pszName, VARIANT* pvarValue)
{
    if (!pszName || !pvarValue)
        return E_INVALIDARG;

    VariantClear(pvarValue);

    auto it = m_propertyCache.find(ATL::CString(pszName));
    if (it != m_propertyCache.end())
    {
        return VariantCopy(pvarValue, &it->second);
    }

    return E_FAIL;
}

HRESULT AMPDataContext::SetProperty(LPCWSTR pszName, const VARIANT* varValue)
{
    UNREFERENCED_PARAMETER(pszName);
    UNREFERENCED_PARAMETER(varValue);
    // Read-only properties -- setting is not supported
    return E_NOTIMPL;
}

void AMPDataContext::RefreshAll()
{
    RefreshProperty(kPropProjectName);
    RefreshProperty(kPropIsProjectOpen);
    RefreshProperty(kPropIsProjectDirty);
    RefreshProperty(kPropCanUndo);
    RefreshProperty(kPropCanRedo);
    RefreshProperty(kPropCanCut);
    RefreshProperty(kPropCanCopy);
    RefreshProperty(kPropCanPaste);
    RefreshProperty(kPropIsPlaying);
    RefreshProperty(kPropIsPublishing);
    RefreshProperty(kPropIsEncoding);
    RefreshProperty(kPropTimelinePosition);
    RefreshProperty(kPropTimelineDuration);
    RefreshProperty(kPropVersionString);
}

void AMPDataContext::RefreshProperty(LPCWSTR pszName)
{
    if (!pszName || !m_pAppMain)
        return;

    VARIANT varValue;
    VariantInit(&varValue);

    if (wcscmp(pszName, kPropProjectName) == 0)
    {
        varValue.vt = VT_BSTR;
        varValue.bstrVal = SysAllocString(L"Untitled");
    }
    else if (wcscmp(pszName, kPropIsProjectOpen) == 0)
    {
        varValue.vt = VT_BOOL;
        varValue.boolVal = m_pAppMain->IsProjectOpen() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropIsProjectDirty) == 0)
    {
        varValue.vt = VT_BOOL;
        varValue.boolVal = m_pAppMain->IsProjectDirty() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropCanUndo) == 0)
    {
        varValue.vt = VT_BOOL;
        varValue.boolVal = m_pAppMain->CanUndo() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropCanRedo) == 0)
    {
        varValue.vt = VT_BOOL;
        varValue.boolVal = m_pAppMain->CanRedo() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropCanCut) == 0)
    {
        varValue.vt = VT_BOOL;
        varValue.boolVal = m_pAppMain->CanCut() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropCanCopy) == 0)
    {
        varValue.vt = VT_BOOL;
        varValue.boolVal = m_pAppMain->CanCopy() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropCanPaste) == 0)
    {
        varValue.vt = VT_BOOL;
        varValue.boolVal = m_pAppMain->CanPaste() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropIsPlaying) == 0)
    {
        varValue.vt = VT_BOOL;
        varValue.boolVal = m_pAppMain->IsPlaying() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropIsPublishing) == 0)
    {
        varValue.vt = VT_BOOL;
        varValue.boolVal = m_pAppMain->IsPublishing() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropIsEncoding) == 0)
    {
        varValue.vt = VT_BOOL;
        varValue.boolVal = m_pAppMain->IsEncoding() ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if (wcscmp(pszName, kPropTimelinePosition) == 0)
    {
        varValue.vt = VT_R8;
        varValue.dblVal = 0.0;
    }
    else if (wcscmp(pszName, kPropTimelineDuration) == 0)
    {
        varValue.vt = VT_R8;
        varValue.dblVal = 0.0;
    }
    else if (wcscmp(pszName, kPropVersionString) == 0)
    {
        varValue.vt = VT_BSTR;
        varValue.bstrVal = SysAllocString(SUNDANCE_VERSION_STRING);
    }
    else
    {
        return;
    }

    // Update cache
    VARIANT& cachedValue = m_propertyCache[ATL::CString(pszName)];
    VariantClear(&cachedValue);
    VariantCopy(&cachedValue, &varValue);
    VariantClear(&varValue);
}

void AMPDataContext::InitPropertyCache()
{
    // Pre-initialize all known properties with default values
    VARIANT varDefault;
    VariantInit(&varDefault);

    varDefault.vt = VT_BSTR;
    varDefault.bstrVal = SysAllocString(L"");
    m_propertyCache[kPropProjectName] = varDefault;

    varDefault.vt = VT_BOOL;
    varDefault.boolVal = VARIANT_FALSE;
    m_propertyCache[kPropIsProjectOpen] = varDefault;
    m_propertyCache[kPropIsProjectDirty] = varDefault;
    m_propertyCache[kPropCanUndo] = varDefault;
    m_propertyCache[kPropCanRedo] = varDefault;
    m_propertyCache[kPropCanCut] = varDefault;
    m_propertyCache[kPropCanCopy] = varDefault;
    m_propertyCache[kPropCanPaste] = varDefault;
    m_propertyCache[kPropIsPlaying] = varDefault;
    m_propertyCache[kPropIsPublishing] = varDefault;
    m_propertyCache[kPropIsEncoding] = varDefault;

    varDefault.vt = VT_R8;
    varDefault.dblVal = 0.0;
    m_propertyCache[kPropTimelinePosition] = varDefault;
    m_propertyCache[kPropTimelineDuration] = varDefault;

    varDefault.vt = VT_BSTR;
    varDefault.bstrVal = SysAllocString(SUNDANCE_VERSION_STRING);
    m_propertyCache[kPropVersionString] = varDefault;
}

DISPID AMPDataContext::FindProperty(LPCWSTR pszName) const
{
    if (!pszName)
        return -1;

    // Map property names to DISPIDs
    static const struct { LPCWSTR pszName; DISPID id; } s_dispMap[] = {
        { kPropProjectName,       1 },
        { kPropIsProjectOpen,     2 },
        { kPropIsProjectDirty,    3 },
        { kPropCanUndo,           4 },
        { kPropCanRedo,           5 },
        { kPropCanCut,            6 },
        { kPropCanCopy,           7 },
        { kPropCanPaste,          8 },
        { kPropIsPlaying,         9 },
        { kPropIsPublishing,     10 },
        { kPropIsEncoding,       11 },
        { kPropTimelinePosition, 12 },
        { kPropTimelineDuration, 13 },
        { kPropVersionString,    14 },
    };

    for (int i = 0; i < ARRAYSIZE(s_dispMap); ++i)
    {
        if (wcscmp(pszName, s_dispMap[i].pszName) == 0)
            return s_dispMap[i].id;
    }

    return -1;
}

STDMETHODIMP AMPDataContext::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames,
    UINT cNames, LCID /*lcid*/, DISPID* rgDispId)
{
    if (!rgszNames || !rgDispId || cNames == 0)
        return E_INVALIDARG;

    for (UINT i = 0; i < cNames; ++i)
    {
        rgDispId[i] = FindProperty(rgszNames[i]);
    }

    return S_OK;
}

STDMETHODIMP AMPDataContext::GetTypeInfo(UINT /*iTInfo*/, LCID /*lcid*/, ITypeInfo** ppTInfo)
{
    if (ppTInfo) *ppTInfo = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP AMPDataContext::GetTypeInfoCount(UINT* pctinfo)
{
    if (pctinfo) *pctinfo = 0;
    return S_OK;
}

STDMETHODIMP AMPDataContext::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/,
    WORD /*wFlags*/, DISPPARAMS* /*pDispParams*/, VARIANT* pVarResult,
    EXCEPINFO* /*pExcepInfo*/, UINT* /*puArgErr*/)
{
    if (!pVarResult)
        return E_POINTER;

    // Map DISPID to property name and refresh
    static const struct { DISPID id; LPCWSTR pszName; } s_idMap[] = {
        { 1,  kPropProjectName },
        { 2,  kPropIsProjectOpen },
        { 3,  kPropIsProjectDirty },
        { 4,  kPropCanUndo },
        { 5,  kPropCanRedo },
        { 6,  kPropCanCut },
        { 7,  kPropCanCopy },
        { 8,  kPropCanPaste },
        { 9,  kPropIsPlaying },
        { 10, kPropIsPublishing },
        { 11, kPropIsEncoding },
        { 12, kPropTimelinePosition },
        { 13, kPropTimelineDuration },
        { 14, kPropVersionString },
    };

    for (int i = 0; i < ARRAYSIZE(s_idMap); ++i)
    {
        if (s_idMap[i].id == dispIdMember)
        {
            RefreshProperty(s_idMap[i].pszName);
            return GetProperty(s_idMap[i].pszName, pVarResult);
        }
    }

    return DISP_E_MEMBERNOTFOUND;
}

} // namespace Sundance
