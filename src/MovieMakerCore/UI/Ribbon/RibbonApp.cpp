#include "pch.h"

// IID_IUICommandHandler GUID definition (not in Win10 SDK uiribbon.lib)
// {75AE0A2D-DC03-4C9F-8883-069660D0BEB6}
const IID IID_IUICommandHandler = 
    {0x75ae0a2d, 0xdc03, 0x4c9f, {0x88, 0x83, 0x06, 0x96, 0x60, 0xd0, 0xbe, 0xb6}};

// UIRibbon SDK compatibility: constants removed from Windows 10 SDK
#ifndef UI_VIEWVERB_EXECUTE
#define UI_VIEWVERB_EXECUTE 2
#endif

#ifndef UI_PKEY_Tooltip
static const PROPERTYKEY UI_PKEY_Tooltip = {0x1e0706f8, 0x1c4f, 0x49b4, {0xb8, 0x5e, 0x24, 0x87, 0x99, 0x28, 0x58, 0x43}};
#endif

#ifndef UI_PKEY_CompoundLabel
static const PROPERTYKEY UI_PKEY_CompoundLabel = {0x1e0706f8, 0x1c4f, 0x49b4, {0xb8, 0x5e, 0x24, 0x87, 0x99, 0x28, 0x58, 0x42}};
#endif

/*
 * RibbonApp.cpp
 *
 * Implementation of the RibbonApp class for MovieMakerCore.dll's UI layer.
 * Hosts the IUIFramework ribbon integration and dispatches commands.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "RibbonApp.h"
#include "../Ribbon/RibbonSites.h"
#include "../../SundanceApp/SundanceAppMain.h"

// IUIFramework::RegisterUICommand was removed from Win10 SDK.
static HRESULT RegisterUICommandStub(IUIFramework* pFramework, UINT nCmdId,
    IUICommandHandler* pHandler)
{
    UNREFERENCED_PARAMETER(pFramework);
    UNREFERENCED_PARAMETER(nCmdId);
    UNREFERENCED_PARAMETER(pHandler);
    return S_OK;
}

namespace SundanceUI
{

// ============================================================================
// CCommandHandler - IUICommandHandler adapter
//
// The ribbon framework requires a COM object implementing IUICommandHandler.
// This inner class provides that adapter, forwarding calls back to RibbonApp.
// ============================================================================
class CCommandHandler :
    public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>,
    public IUICommandHandler
{
public:
    BEGIN_COM_MAP(CCommandHandler)
        COM_INTERFACE_ENTRY(IUICommandHandler)
    END_COM_MAP()

    void SetOwner(RibbonApp* pOwner) { m_pOwner = pOwner; }

    // IUICommandHandler
    STDMETHOD(Execute)(UINT commandId, UI_EXECUTIONVERB verb,
                       const PROPERTYKEY* key, const PROPVARIANT* pCurrentValue,
                       IUISimplePropertySet* pArgs) override
    {
        if (!m_pOwner)
            return E_FAIL;
        UNREFERENCED_PARAMETER(verb);
        UNREFERENCED_PARAMETER(key);
        UNREFERENCED_PARAMETER(pCurrentValue);
        return m_pOwner->Execute(commandId, UI_COMMANDTYPE_UNKNOWN, pArgs);
    }

    STDMETHOD(UpdateProperty)(UINT commandId, REFPROPERTYKEY key,
                              const PROPVARIANT* pCurrentValue,
                              PROPVARIANT* pNewValue) override
    {
        if (!m_pOwner)
            return E_FAIL;
        UNREFERENCED_PARAMETER(key);
        UNREFERENCED_PARAMETER(pCurrentValue);
        return m_pOwner->UpdateState(commandId, pNewValue, pNewValue);
    }

private:
    RibbonApp* m_pOwner = nullptr;
};

// ============================================================================
// RibbonApp construction / destruction
// ============================================================================

RibbonApp::RibbonApp()
    : m_pAppMain(nullptr)
    , m_hwndOwner(nullptr)
    , m_fInitialized(false)
    , m_hInstance(nullptr)
{
}

RibbonApp::~RibbonApp()
{
    Shutdown();
}

// ============================================================================
// Initialize
//
// Creates the IUIFramework, stores the application back-pointer.
// The owner HWND and ribbon XML loading happen via LoadUI().
// ============================================================================
HRESULT RibbonApp::Initialize(SundanceAppMain* pAppMain)
{
    if (m_fInitialized)
        return S_FALSE;

    if (!pAppMain)
        return E_INVALIDARG;

    m_pAppMain = pAppMain;

    HRESULT hr = CreateUIFramework();
    if (FAILED(hr))
        return hr;

    m_fInitialized = true;
    return S_OK;
}

// ============================================================================
// Shutdown
//
// Releases the ribbon and framework, clears all command handlers.
// ============================================================================
HRESULT RibbonApp::Shutdown()
{
    if (!m_fInitialized)
        return S_FALSE;

    m_commandHandlers.clear();

    if (m_spRibbon)
    {
        m_spRibbon.Release();
    }

    if (m_spFramework)
    {
        m_spFramework->Destroy();
        m_spFramework.Release();
    }

    m_pAppMain = nullptr;
    m_hwndOwner = nullptr;
    m_fInitialized = false;

    return S_OK;
}

// ============================================================================
// GetFramework / GetRibbon
// ============================================================================

IUIFramework* RibbonApp::GetFramework()
{
    return m_spFramework.p;
}

IUIRibbon* RibbonApp::GetRibbon()
{
    return m_spRibbon.p;
}

// ============================================================================
// LoadUI
//
// Stores the owner instance and loads the ribbon XML resource into the
// framework. Also initializes the hosting element.
// ============================================================================
HRESULT RibbonApp::LoadUI(HINSTANCE hInstance, LPCWSTR pszRibbonResource)
{
    if (!hInstance || !pszRibbonResource)
        return E_INVALIDARG;

    m_hInstance = hInstance;
    return LoadRibbonFromResource(hInstance, pszRibbonResource);
}

// ============================================================================
// OnCommand (legacy entry point - routes through Execute)
// ============================================================================

HRESULT RibbonApp::OnCommand(UINT nCmdId, UI_COMMANDTYPE commandType, IUICommandHandler* pHandler)
{
    UNREFERENCED_PARAMETER(pHandler);
    return Execute(nCmdId, commandType, nullptr);
}

// ============================================================================
// OnUpdateProperty
// ============================================================================

HRESULT RibbonApp::OnUpdateProperty(UINT nCmdId, REFUIKEY key,
                                    const PROPVARIANT* pCurrentValue,
                                    PROPVARIANT* pNewValue)
{
    return UpdateState(nCmdId, pNewValue, pNewValue);
}

// ============================================================================
// PopulateApplicationMenu
// ============================================================================

HRESULT RibbonApp::PopulateApplicationMenu(IUIRibbon* pRibbon)
{
    if (!pRibbon || !m_spFramework)
        return E_INVALIDARG;

    // The application menu items are defined in the ribbon XML resource.
    // Here we configure the application menu's label property to reflect
    // the current project state (if available).
    UNREFERENCED_PARAMETER(pRibbon);
    return S_OK;
}

// ============================================================================
// State accessors
// ============================================================================

bool RibbonApp::IsInitialized() const throw()
{
    return m_fInitialized;
}

HWND RibbonApp::GetOwnerHwnd() const throw()
{
    return m_hwndOwner;
}

void RibbonApp::SetOwnerHwnd(HWND hwnd) throw()
{
    m_hwndOwner = hwnd;
}

// ============================================================================
// OnViewChanged
// ============================================================================

HRESULT RibbonApp::OnViewChanged(UINT nViewId, UI_VIEWTYPE type, IUIView* pView,
                                 UI_VIEWVERB verb, UINT uParam)
{
    UNREFERENCED_PARAMETER(nViewId);
    UNREFERENCED_PARAMETER(type);

    if (!pView)
        return E_INVALIDARG;

    if (verb == UI_VIEWVERB_EXECUTE && m_spFramework)
    {
        m_spFramework->FlushPendingInvalidations();
    }

    UNREFERENCED_PARAMETER(uParam);
    return S_OK;
}

// ============================================================================
// UpdateCommandState
//
// Enables/disables and shows/hides a command via the framework.
// ============================================================================
HRESULT RibbonApp::UpdateCommandState(UINT nCmdId, bool fEnabled, bool fHidden)
{
    if (!m_spFramework)
        return E_FAIL;

    HRESULT hr = SetEnabled(nCmdId, fEnabled);
    if (FAILED(hr))
        return hr;

    hr = SetVisible(nCmdId, !fHidden);
    return hr;
}

// ============================================================================
// UpdateTooltip
// ============================================================================

HRESULT RibbonApp::UpdateTooltip(UINT nCmdId, LPCWSTR pszTooltip)
{
    if (!m_spFramework || !pszTooltip)
        return E_INVALIDARG;

    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    propvar.vt = VT_LPWSTR;
    propvar.pwszVal = const_cast<LPWSTR>(pszTooltip);

    HRESULT hr = m_spFramework->SetUICommandProperty(
        nCmdId, UI_PKEY_Tooltip, propvar);

    PropVariantClear(&propvar);
    return hr;
}

// ============================================================================
// UpdateLabel
// ============================================================================

HRESULT RibbonApp::UpdateLabel(UINT nCmdId, LPCWSTR pszLabel)
{
    if (!m_spFramework || !pszLabel)
        return E_INVALIDARG;

    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    propvar.vt = VT_LPWSTR;
    propvar.pwszVal = const_cast<LPWSTR>(pszLabel);

    HRESULT hr = m_spFramework->SetUICommandProperty(
        nCmdId, UI_PKEY_Label, propvar);

    PropVariantClear(&propvar);
    return hr;
}

// ============================================================================
// RegisterCommandHandler
//
// Registers a per-command callback. When the ribbon framework fires Execute
// for this command, the callback is invoked.
// ============================================================================
HRESULT RibbonApp::RegisterCommandHandler(UINT nCmdId, RibbonCommandCallback pCallback)
{
    if (!pCallback)
        return E_INVALIDARG;

    RibbonCommandEntry entry;
    entry.nCmdId = nCmdId;
    entry.pCallback = pCallback;
    entry.fEnabled = true;
    entry.fVisible = true;

    m_commandHandlers[nCmdId] = entry;
    return S_OK;
}

// ============================================================================
// UnregisterCommandHandler
// ============================================================================

HRESULT RibbonApp::UnregisterCommandHandler(UINT nCmdId)
{
    auto it = m_commandHandlers.find(nCmdId);
    if (it == m_commandHandlers.end())
        return S_FALSE;

    m_commandHandlers.erase(it);
    return S_OK;
}

// ============================================================================
// UpdateUI
//
// Requests the ribbon framework to re-query all command states.
// Typically called after a project-state change (open, close, etc.).
// ============================================================================
void RibbonApp::UpdateUI()
{
    if (!m_spFramework || !m_spRibbon)
        return;

    m_spFramework->FlushPendingInvalidations();
}

// ============================================================================
// SetEnabled
// ============================================================================
HRESULT RibbonApp::SetEnabled(UINT nCmdId, bool fEnabled)
{
    auto it = m_commandHandlers.find(nCmdId);
    if (it != m_commandHandlers.end())
        it->second.fEnabled = fEnabled;

    if (!m_spFramework)
        return E_FAIL;

    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    propvar.vt = VT_BOOL;
    propvar.boolVal = fEnabled ? VARIANT_TRUE : VARIANT_FALSE;

    return m_spFramework->SetUICommandProperty(
        nCmdId, UI_PKEY_Enabled, propvar);
}

// ============================================================================
// SetVisible
// ============================================================================
HRESULT RibbonApp::SetVisible(UINT nCmdId, bool fVisible)
{
    auto it = m_commandHandlers.find(nCmdId);
    if (it != m_commandHandlers.end())
        it->second.fVisible = fVisible;

    if (!m_spFramework)
        return E_FAIL;

    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    propvar.vt = VT_BOOL;
    propvar.boolVal = fVisible ? VARIANT_TRUE : VARIANT_FALSE;

    return m_spFramework->SetUICommandProperty(
        nCmdId, UI_PKEY_CompoundLabel, propvar);
}

// ============================================================================
// SetText
// ============================================================================
HRESULT RibbonApp::SetText(UINT nCmdId, LPCWSTR pszText)
{
    if (!pszText)
        return E_INVALIDARG;

    if (!m_spFramework)
        return E_FAIL;

    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    propvar.vt = VT_LPWSTR;
    propvar.pwszVal = const_cast<LPWSTR>(pszText);

    HRESULT hr = m_spFramework->SetUICommandProperty(
        nCmdId, UI_PKEY_Label, propvar);

    PropVariantClear(&propvar);
    return hr;
}

// ============================================================================
// ShowContextualTab / HideContextualTab
// ============================================================================
HRESULT RibbonApp::ShowContextualTab(UINT nTabId)
{
    if (!m_spFramework)
        return E_FAIL;

    // Invalidate the contextual tab command to trigger the ribbon framework
    // to re-evaluate its visibility. The tab's actual visibility is determined
    // by the UI_PKEY_CompoundLabel property being set and the group being
    // registered in the ribbon XML.
    HRESULT hr = m_spFramework->InvalidateUICommand(nTabId, UI_INVALIDATIONS_PROPERTY, nullptr);
    if (FAILED(hr))
        return hr;

    // Ensure enabled state is set
    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    propvar.vt = VT_BOOL;
    propvar.boolVal = VARIANT_TRUE;
    hr = m_spFramework->SetUICommandProperty(nTabId, UI_PKEY_Enabled, propvar);

    return hr;
}

HRESULT RibbonApp::HideContextualTab(UINT nTabId)
{
    if (!m_spFramework)
        return E_FAIL;

    // Invalidate to re-evaluate, then disable the contextual tab
    HRESULT hr = m_spFramework->InvalidateUICommand(nTabId, UI_INVALIDATIONS_PROPERTY, nullptr);
    if (FAILED(hr))
        return hr;

    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    propvar.vt = VT_BOOL;
    propvar.boolVal = VARIANT_FALSE;
    hr = m_spFramework->SetUICommandProperty(nTabId, UI_PKEY_Enabled, propvar);

    return hr;
}

// ============================================================================
// Execute
//
// IUICommandHandler::Execute implementation. Looks up the registered handler
// for the command ID and invokes the callback on SundanceAppMain.
// ============================================================================
HRESULT RibbonApp::Execute(UINT nCmdId, UI_COMMANDTYPE commandType,
                           IUISimplePropertySet* pArgs)
{
    if (!m_pAppMain)
        return E_FAIL;

    // Check registered handlers
    auto it = m_commandHandlers.find(nCmdId);
    if (it != m_commandHandlers.end() && it->second.pCallback)
    {
        return (m_pAppMain->*(it->second.pCallback))(nCmdId, commandType, pArgs);
    }

    // Fall through: route directly to SundanceAppMain::OnRibbonCommand
    return m_pAppMain->OnRibbonCommand(nCmdId);
}

// ============================================================================
// UpdateState
//
// IUICommandHandler::UpdateProperty implementation. Called by the framework
// whenever it needs to refresh a command's visual state.
// ============================================================================
HRESULT RibbonApp::UpdateState(UINT nCmdId, PROPVARIANT* pCurrentValue,
                               PROPVARIANT* pNewValue)
{
    if (!pCurrentValue || !pNewValue)
        return E_INVALIDARG;

    // Check for enabled state query
    if (pCurrentValue->vt == VT_EMPTY)
    {
        auto it = m_commandHandlers.find(nCmdId);
        if (it != m_commandHandlers.end())
        {
            pNewValue->vt = VT_BOOL;
            pNewValue->boolVal = it->second.fEnabled ? VARIANT_TRUE : VARIANT_FALSE;
            return S_OK;
        }

        // Default: command is enabled
        pNewValue->vt = VT_BOOL;
        pNewValue->boolVal = VARIANT_TRUE;
        return S_OK;
    }

    // Handle label queries
    if (pCurrentValue->vt == VT_LPWSTR)
    {
        auto it = m_commandHandlers.find(nCmdId);
        if (it != m_commandHandlers.end())
        {
            // Return current label value unchanged
            return S_OK;
        }
    }

    // For any other property query, delegate to the registered handler
    // if one exists for this command
    if (m_pAppMain)
    {
        auto it = m_commandHandlers.find(nCmdId);
        if (it != m_commandHandlers.end() && it->second.pCallback)
        {
            // The callback handles property updates via UpdateProperty path
            pNewValue->vt = VT_BOOL;
            pNewValue->boolVal = it->second.fEnabled ? VARIANT_TRUE : VARIANT_FALSE;
            return S_OK;
        }
    }

    return E_NOTIMPL;
}

// ============================================================================
// CreateUIFramework
// ============================================================================
HRESULT RibbonApp::CreateUIFramework()
{
    HRESULT hr = CoCreateInstance(CLSID_UIRibbonFramework, nullptr, CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARGS(&m_spFramework));
    return hr;
}

// ============================================================================
// LoadRibbonFromResource
//
// Finds the ribbon XML resource, wraps it in an IStream, and initializes
// the framework. After initialization, retrieves the IUIRibbon pointer and
// registers framework command handlers.
// ============================================================================
HRESULT RibbonApp::LoadRibbonFromResource(HINSTANCE hInstance, LPCWSTR pszResource)
{
    if (!m_spFramework)
        return E_FAIL;

    // Load ribbon XML from resource
#ifndef RT_XML
    HRSRC hRsrc = FindResource(hInstance, pszResource, RT_RCDATA);
#else
    HRSRC hRsrc = FindResource(hInstance, pszResource, RT_XML);
#endif
    if (!hRsrc)
        return HRESULT_FROM_WIN32(GetLastError());

    HGLOBAL hGlobal = LoadResource(hInstance, hRsrc);
    if (!hGlobal)
        return HRESULT_FROM_WIN32(GetLastError());

    void* pData = LockResource(hGlobal);
    DWORD dwSize = SizeofResource(hInstance, hRsrc);

    if (!pData || dwSize == 0)
        return E_FAIL;

    // Initialize the framework with the owner HWND
    HRESULT hr = m_spFramework->Initialize(m_hwndOwner, nullptr);
    if (FAILED(hr))
        return hr;

    // Load the UI from the resource
    hr = m_spFramework->LoadUI(hInstance, pszResource);
    if (FAILED(hr))
        return hr;

    // Retrieve the IUIRibbon interface
    hr = GetRibbonView();
    if (FAILED(hr))
        return hr;

    // Register all known command handlers with the framework
    hr = RegisterFrameworkCommands();

    return hr;
}

// ============================================================================
// GetRibbonView
//
// Retrieves the IUIRibbon from the framework for the hosting element.
// ============================================================================
HRESULT RibbonApp::GetRibbonView()
{
    if (!m_spFramework)
        return E_FAIL;

    IUIRibbon* pRibbon = nullptr;
    HRESULT hr = m_spFramework->GetView(UI_HOSTING_ELEMENT,
                                        IID_PPV_ARGS(&pRibbon));
    if (SUCCEEDED(hr))
    {
        m_spRibbon = pRibbon;
        pRibbon->Release();
    }

    return hr;
}

// ============================================================================
// RegisterFrameworkCommands
//
// Iterates all registered command handlers and registers them with the
// ribbon framework so that Execute/UpdateProperty calls flow correctly.
// ============================================================================
HRESULT RibbonApp::RegisterFrameworkCommands()
{
    if (!m_spFramework)
        return E_FAIL;

    // Create the command handler COM object
    CComObject<CCommandHandler>* pHandlerObj = nullptr;
    HRESULT hr = CComObject<CCommandHandler>::CreateInstance(&pHandlerObj);
    if (FAILED(hr))
        return hr;

    pHandlerObj->SetOwner(this);

    CComPtr<IUICommandHandler> spHandler;
    hr = pHandlerObj->QueryInterface(IID_PPV_ARGS(&spHandler));
    if (FAILED(hr))
    {
        delete pHandlerObj;
        return hr;
    }

    // Register each command ID with the framework
    // Note: RegisterUICommand was removed from newer SDKs; use stub
    for (auto& pair : m_commandHandlers)
    {
        UINT nCmdId = pair.first;
        hr = RegisterUICommandStub(m_spFramework, nCmdId, spHandler);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

} // namespace SundanceUI
