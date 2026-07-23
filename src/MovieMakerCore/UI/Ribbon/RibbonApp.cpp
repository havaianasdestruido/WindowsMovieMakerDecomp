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

#ifndef UI_PKEY_Pressed
static const PROPERTYKEY UI_PKEY_Pressed = {0xc3b3c2f2, 0x05db, 0x4b53, {0x86, 0xb5, 0x92, 0xd7, 0x3c, 0xfb, 0xf4, 0x3b}};
#endif

// UI_PKEY_BooleanValue and UI_PKEY_SelectedItem are provided by UIRibbon.h
// UI_INVALIDATIONS_PROPERTY is provided by UIRibbon.h as an enum value.
// Do NOT preprocessor-define it here, as that would conflict with the enum.

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
        return m_pOwner->UpdateState(commandId, key, pCurrentValue, pNewValue);
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
    , m_nActiveTab(kRibbonCmdHome)
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
HRESULT RibbonApp::Initialize(SundanceAppMain* pAppMain, HWND hWnd)
{
    if (m_fInitialized)
        return S_FALSE;

    if (!pAppMain)
        return E_INVALIDARG;

    m_pAppMain = pAppMain;
    m_hwndOwner = hWnd;

    HRESULT hr = CreateUIFramework();
    if (FAILED(hr))
        return hr;

    RegisterCommonCommands();

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
    m_commandMap.clear();

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

HRESULT RibbonApp::OnUpdateProperty(UINT nCmdId, REFPROPERTYKEY key,
                                    const PROPVARIANT* pCurrentValue,
                                    PROPVARIANT* pNewValue)
{
    return UpdateState(nCmdId, key, pCurrentValue, pNewValue);
}

// ============================================================================
// PopulateApplicationMenu
// ============================================================================

HRESULT RibbonApp::PopulateApplicationMenu(IUIRibbon* pRibbon)
{
    if (!pRibbon || !m_spFramework)
        return E_INVALIDARG;

    // The application menu items are defined in the ribbon XML resource.
    // Configure the application menu's label property to reflect
    // the current project state (if available).
    if (m_pAppMain)
    {
        // Update the File menu label to reflect dirty state
        LPCWSTR pszProjectTitle = L"Movie Maker";
        if (m_pAppMain->IsProjectOpen())
        {
            if (m_pAppMain->IsProjectDirty())
            {
                // Append asterisk for unsaved changes
                UpdateLabel(ID_FILE_SAVE, L"Save *");
            }
            else
            {
                UpdateLabel(ID_FILE_SAVE, L"Save");
            }
        }

        // Update Recent Files submenu in the application menu
        // The MRU list is populated via CMRUSite::AddItem calls from
        // the recent files subsystem. Framework invalidation triggers
        // the ribbon to re-query our property values.

        UNREFERENCED_PARAMETER(pszProjectTitle);
    }

    // Invalidate all commands so the framework re-queries their states
    if (m_spFramework)
    {
        m_spFramework->FlushPendingInvalidations();
    }

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

    // Store the tooltip string in the command entry for UpdateState queries
    auto it = m_commandHandlers.find(nCmdId);
    if (it != m_commandHandlers.end())
        it->second.strTooltip = pszTooltip;

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

    // Store the label string in the command entry for UpdateState queries
    auto it = m_commandHandlers.find(nCmdId);
    if (it != m_commandHandlers.end())
        it->second.strLabel = pszLabel;

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
        nCmdId, UI_PKEY_BooleanValue, propvar);
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
    HRESULT hr = m_spFramework->InvalidateUICommand(nTabId, static_cast<UI_INVALIDATIONS>(UI_INVALIDATIONS_PROPERTY), nullptr);
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
    HRESULT hr = m_spFramework->InvalidateUICommand(nTabId, static_cast<UI_INVALIDATIONS>(UI_INVALIDATIONS_PROPERTY), nullptr);
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
// RegisterCommandHandler (simple std::function overload)
// ============================================================================
void RibbonApp::RegisterCommandHandler(UINT nCmdId, std::function<void()> handler)
{
    if (handler)
        m_commandMap[nCmdId] = std::move(handler);
}

// ============================================================================
// ExecuteSimpleCommand
// ============================================================================
void RibbonApp::ExecuteSimpleCommand(UINT nCmdId)
{
    auto it = m_commandMap.find(nCmdId);
    if (it != m_commandMap.end() && it->second)
    {
        it->second();
    }
}

// ============================================================================
// RegisterCommonCommands
//
// Populates m_commandMap with lambdas that delegate to SundanceAppMain's
// subsystem controllers (ImportController, ProjectManager, ExportController,
// UndoManager, PlaybackController).
// ============================================================================
void RibbonApp::RegisterCommonCommands()
{
    if (!m_pAppMain)
        return;

    m_commandMap[ID_FILE_IMPORT] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(ID_FILE_IMPORT);
    };

    m_commandMap[ID_FILE_SAVE] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->SaveProject();
    };

    m_commandMap[ID_FILE_EXPORT] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(ID_FILE_EXPORT);
    };

    m_commandMap[ID_EDIT_UNDO] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->Undo();
    };

    m_commandMap[ID_EDIT_REDO] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->Redo();
    };

    m_commandMap[ID_PLAY_PLAY] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->StartPlayback();
    };

    m_commandMap[ID_PLAY_PAUSE] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->PausePlayback();
    };

    m_commandMap[ID_PLAY_STOP] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->StopPlayback();
    };

    // Ribbon command registrations for all kRibbonCmd* IDs
    m_commandMap[kRibbonCmdAddVideos] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdAddVideos);
    };

    m_commandMap[kRibbonCmdAddPhotos] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdAddPhotos);
    };

    m_commandMap[kRibbonCmdAddMusic] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdAddMusic);
    };

    m_commandMap[kRibbonCmdWebcam] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdWebcam);
    };

    m_commandMap[kRibbonCmdNarrate] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdNarrate);
    };

    m_commandMap[kRibbonCmdUndo] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->Undo();
    };

    m_commandMap[kRibbonCmdRedo] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->Redo();
    };

    m_commandMap[kRibbonCmdCut] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->CutSelection();
    };

    m_commandMap[kRibbonCmdCopy] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->CopySelection();
    };

    m_commandMap[kRibbonCmdPaste] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->PasteFromClipboard();
    };

    m_commandMap[kRibbonCmdDelete] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdDelete);
    };

    m_commandMap[kRibbonCmdSelectAll] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSelectAll);
    };

    m_commandMap[kRibbonCmdTheme] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdTheme);
    };

    m_commandMap[kRibbonCmdAutoMovie] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdAutoMovie);
    };

    m_commandMap[kRibbonCmdSnapshot] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSnapshot);
    };

    m_commandMap[kRibbonCmdTrim] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdTrim);
    };

    m_commandMap[kRibbonCmdSplit] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSplit);
    };

    m_commandMap[kRibbonCmdSetTitle] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSetTitle);
    };

    m_commandMap[kRibbonCmdSetCredits] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSetCredits);
    };

    m_commandMap[kRibbonCmdSpeed] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSpeed);
    };

    m_commandMap[kRibbonCmdVolume] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdVolume);
    };

    m_commandMap[kRibbonCmdSaveMovie] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSaveMovie);
    };

    m_commandMap[kRibbonCmdSaveFile] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->SaveProject();
    };

    m_commandMap[kRibbonCmdPublish] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdPublish);
    };

    m_commandMap[kRibbonCmdZoomToTimeline] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdZoomToTimeline);
    };

    // Visual Effects tab commands
    m_commandMap[kRibbonCmdEffectsGallery] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdEffectsGallery);
    };

    m_commandMap[kRibbonCmdEffectNone] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdEffectNone);
    };

    m_commandMap[kRibbonCmdEffectGrayscale] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdEffectGrayscale);
    };

    m_commandMap[kRibbonCmdEffectSepia] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdEffectSepia);
    };

    m_commandMap[kRibbonCmdEffectNegative] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdEffectNegative);
    };

    m_commandMap[kRibbonCmdEffectFadeIn] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdEffectFadeIn);
    };

    m_commandMap[kRibbonCmdEffectFadeOut] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdEffectFadeOut);
    };

    m_commandMap[kRibbonCmdBrightness] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdBrightness);
    };

    m_commandMap[kRibbonCmdContrast] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdContrast);
    };

    m_commandMap[kRibbonCmdSaturation] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSaturation);
    };

    m_commandMap[kRibbonCmdSharpen] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSharpen);
    };

    // Animations tab commands
    m_commandMap[kRibbonCmdTransitionGallery] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdTransitionGallery);
    };

    m_commandMap[kRibbonCmdTransitionNone] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdTransitionNone);
    };

    m_commandMap[kRibbonCmdTransitionCrossfade] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdTransitionCrossfade);
    };

    m_commandMap[kRibbonCmdTransitionWipe] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdTransitionWipe);
    };

    m_commandMap[kRibbonCmdTransitionSlide] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdTransitionSlide);
    };

    m_commandMap[kRibbonCmdTransitionFade] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdTransitionFade);
    };

    m_commandMap[kRibbonCmdPanZoomGallery] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdPanZoomGallery);
    };

    m_commandMap[kRibbonCmdPanZoomNone] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdPanZoomNone);
    };

    m_commandMap[kRibbonCmdPanZoomSlowPan] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdPanZoomSlowPan);
    };

    m_commandMap[kRibbonCmdPanZoomZoomIn] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdPanZoomZoomIn);
    };

    m_commandMap[kRibbonCmdPanZoomZoomOut] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdPanZoomZoomOut);
    };

    // Project tab commands
    m_commandMap[kRibbonCmdAspectRatio] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdAspectRatio);
    };

    m_commandMap[kRibbonCmdAudioMix] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdAudioMix);
    };

    m_commandMap[kRibbonCmdVideoVolume] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdVideoVolume);
    };

    m_commandMap[kRibbonCmdNarrationVolume] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdNarrationVolume);
    };

    m_commandMap[kRibbonCmdMusicVolume] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdMusicVolume);
    };

    m_commandMap[kRibbonCmdSetStartPoint] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSetStartPoint);
    };

    m_commandMap[kRibbonCmdSetEndPoint] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdSetEndPoint);
    };

    // View tab commands
    m_commandMap[kRibbonCmdZoomToFit] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdZoomToFit);
    };

    m_commandMap[kRibbonCmdZoomIn] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdZoomIn);
    };

    m_commandMap[kRibbonCmdZoomOut] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdZoomOut);
    };

    m_commandMap[kRibbonCmdShowStoryboard] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdShowStoryboard);
    };

    m_commandMap[kRibbonCmdPreviewQuality] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdPreviewQuality);
    };

    // Tab activation commands
    m_commandMap[kRibbonCmdHome] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdHome);
    };

    m_commandMap[kRibbonCmdShare] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdShare);
    };

    m_commandMap[kRibbonCmdView] = [this]()
    {
        if (m_pAppMain)
            m_pAppMain->OnRibbonCommand(kRibbonCmdView);
    };
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

    // Check simple command handler map first
    auto itSimple = m_commandMap.find(nCmdId);
    if (itSimple != m_commandMap.end() && itSimple->second)
    {
        itSimple->second();
        return S_OK;
    }

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
HRESULT RibbonApp::UpdateState(UINT nCmdId, REFPROPERTYKEY key,
                               const PROPVARIANT* pCurrentValue,
                               PROPVARIANT* pNewValue)
{
    if (!pNewValue)
        return E_INVALIDARG;

    UNREFERENCED_PARAMETER(pCurrentValue);

    auto it = m_commandHandlers.find(nCmdId);

    // UI_PKEY_Enabled query - should this command be active?
    if (IsEqualPropertyKey(key, UI_PKEY_Enabled))
    {
        bool fEnabled = false;

        if (it != m_commandHandlers.end())
        {
            fEnabled = it->second.fEnabled;
        }
        else
        {
            // Context-aware defaults: enable commands based on app state
            fEnabled = ComputeCommandEnabled(nCmdId);
        }

        pNewValue->vt = VT_BOOL;
        pNewValue->boolVal = fEnabled ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    // UI_PKEY_Boolean / UI_PKEY_BooleanValue query (checkable/toggle buttons)
    if (IsEqualPropertyKey(key, UI_PKEY_BooleanValue))
    {
        bool fBoolean = false;

        if (it != m_commandHandlers.end())
        {
            fBoolean = it->second.fVisible;
        }

        pNewValue->vt = VT_BOOL;
        pNewValue->boolVal = fBoolean ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    // UI_PKEY_Pressed query (toggle button pressed state)
    if (IsEqualPropertyKey(key, UI_PKEY_Pressed))
    {
        bool fPressed = false;

        if (it != m_commandHandlers.end())
        {
            fPressed = it->second.fPressed;
        }
        else
        {
            // Playback play/pause toggle state
            if (nCmdId == ID_PLAY_PLAY && m_pAppMain)
                fPressed = m_pAppMain->IsPlaying();
        }

        pNewValue->vt = VT_BOOL;
        pNewValue->boolVal = fPressed ? VARIANT_TRUE : VARIANT_FALSE;
        return S_OK;
    }

    // UI_PKEY_Label query
    if (IsEqualPropertyKey(key, UI_PKEY_Label))
    {
        if (it != m_commandHandlers.end() && !it->second.strLabel.IsEmpty())
        {
            pNewValue->vt = VT_LPWSTR;
            pNewValue->pwszVal = static_cast<LPWSTR>(
                CoTaskMemAlloc((it->second.strLabel.GetLength() + 1) * sizeof(WCHAR)));
            if (pNewValue->pwszVal)
            {
                wcscpy_s(pNewValue->pwszVal, it->second.strLabel.GetLength() + 1,
                         it->second.strLabel.GetString());
            }
            return S_OK;
        }
        return S_OK;
    }

    // UI_PKEY_Tooltip query
    if (IsEqualPropertyKey(key, UI_PKEY_Tooltip))
    {
        if (it != m_commandHandlers.end() && !it->second.strTooltip.IsEmpty())
        {
            pNewValue->vt = VT_LPWSTR;
            pNewValue->pwszVal = static_cast<LPWSTR>(
                CoTaskMemAlloc((it->second.strTooltip.GetLength() + 1) * sizeof(WCHAR)));
            if (pNewValue->pwszVal)
            {
                wcscpy_s(pNewValue->pwszVal, it->second.strTooltip.GetLength() + 1,
                         it->second.strTooltip.GetString());
            }
            return S_OK;
        }
        return S_OK;
    }

    // UI_PKEY_CompoundLabel query (visibility of contextual tabs/groups)
    if (IsEqualPropertyKey(key, UI_PKEY_CompoundLabel))
    {
        bool fVisible = false;
        if (it != m_commandHandlers.end())
            fVisible = it->second.fVisible;

        pNewValue->vt = VT_LPWSTR;
        if (fVisible && it != m_commandHandlers.end() && !it->second.strLabel.IsEmpty())
        {
            pNewValue->pwszVal = static_cast<LPWSTR>(
                CoTaskMemAlloc((it->second.strLabel.GetLength() + 1) * sizeof(WCHAR)));
            if (pNewValue->pwszVal)
            {
                wcscpy_s(pNewValue->pwszVal, it->second.strLabel.GetLength() + 1,
                         it->second.strLabel.GetString());
            }
        }
        else
        {
            pNewValue->pwszVal = nullptr;
        }
        return S_OK;
    }

    // UI_PKEY_RepresentativeString (gallery item display text)
    if (IsEqualPropertyKey(key, UI_PKEY_RepresentativeString))
    {
        if (it != m_commandHandlers.end() && !it->second.strRepresentativeString.IsEmpty())
        {
            pNewValue->vt = VT_LPWSTR;
            pNewValue->pwszVal = static_cast<LPWSTR>(
                CoTaskMemAlloc((it->second.strRepresentativeString.GetLength() + 1) * sizeof(WCHAR)));
            if (pNewValue->pwszVal)
            {
                wcscpy_s(pNewValue->pwszVal,
                         it->second.strRepresentativeString.GetLength() + 1,
                         it->second.strRepresentativeString.GetString());
            }
            return S_OK;
        }
        return S_OK;
    }

    // UI_PKEY_LabelDescription (supplemental tooltip text)
    if (IsEqualPropertyKey(key, UI_PKEY_LabelDescription))
    {
        if (it != m_commandHandlers.end() && !it->second.strLabelDescription.IsEmpty())
        {
            pNewValue->vt = VT_LPWSTR;
            pNewValue->pwszVal = static_cast<LPWSTR>(
                CoTaskMemAlloc((it->second.strLabelDescription.GetLength() + 1) * sizeof(WCHAR)));
            if (pNewValue->pwszVal)
            {
                wcscpy_s(pNewValue->pwszVal,
                         it->second.strLabelDescription.GetLength() + 1,
                         it->second.strLabelDescription.GetString());
            }
            return S_OK;
        }
        return S_OK;
    }

    return E_NOTIMPL;
}

// ============================================================================
// ComputeCommandEnabled
//
// Context-aware command enablement. Queries application state to determine
// whether a given ribbon command should be enabled or disabled.
// ============================================================================
bool RibbonApp::ComputeCommandEnabled(UINT nCmdId)
{
    if (!m_pAppMain)
        return false;

    // File menu commands
    switch (nCmdId)
    {
    case ID_FILE_SAVE:
        return m_pAppMain->IsProjectOpen();

    case ID_FILE_EXPORT:
    case kRibbonCmdSaveMovie:
    case kRibbonCmdPublish:
        return m_pAppMain->IsProjectOpen() && !m_pAppMain->IsEncoding();

    case ID_FILE_IMPORT:
        return m_pAppMain->IsProjectOpen();

    // Undo/Redo
    case ID_EDIT_UNDO:
    case kRibbonCmdUndo:
        return m_pAppMain->CanUndo();

    case ID_EDIT_REDO:
    case kRibbonCmdRedo:
        return m_pAppMain->CanRedo();

    // Clipboard
    case kRibbonCmdCut:
        return m_pAppMain->CanCut();

    case kRibbonCmdCopy:
        return m_pAppMain->CanCopy();

    case kRibbonCmdPaste:
        return m_pAppMain->CanPaste();

    case kRibbonCmdDelete:
    case kRibbonCmdSelectAll:
        return m_pAppMain->IsProjectOpen();

    // Playback
    case ID_PLAY_PLAY:
        return m_pAppMain->IsProjectOpen();

    case ID_PLAY_PAUSE:
        return m_pAppMain->IsPlaying();

    case ID_PLAY_STOP:
        return m_pAppMain->IsPlaying();

    // Home tab - media add commands
    case kRibbonCmdAddVideos:
    case kRibbonCmdAddPhotos:
    case kRibbonCmdAddMusic:
        return m_pAppMain->IsProjectOpen();

    // Home tab - editing commands (need selection)
    case kRibbonCmdTrim:
    case kRibbonCmdSplit:
    case kRibbonCmdSpeed:
    case kRibbonCmdVolume:
    case kRibbonCmdSetTitle:
    case kRibbonCmdSetCredits:
        return m_pAppMain->IsProjectOpen();

    // Home tab - always available when project is open
    case kRibbonCmdTheme:
    case kRibbonCmdAutoMovie:
    case kRibbonCmdSnapshot:
    case kRibbonCmdWebcam:
    case kRibbonCmdNarrate:
        return m_pAppMain->IsProjectOpen();

    // Project tab
    case kRibbonCmdAspectRatio:
    case kRibbonCmdAudioMix:
    case kRibbonCmdVideoVolume:
    case kRibbonCmdNarrationVolume:
    case kRibbonCmdMusicVolume:
    case kRibbonCmdSetStartPoint:
    case kRibbonCmdSetEndPoint:
        return m_pAppMain->IsProjectOpen();

    // Visual Effects tab
    case kRibbonCmdEffectsGallery:
    case kRibbonCmdEffectNone:
    case kRibbonCmdEffectGrayscale:
    case kRibbonCmdEffectSepia:
    case kRibbonCmdEffectNegative:
    case kRibbonCmdEffectFadeIn:
    case kRibbonCmdEffectFadeOut:
    case kRibbonCmdBrightness:
    case kRibbonCmdContrast:
    case kRibbonCmdSaturation:
    case kRibbonCmdSharpen:
        return m_pAppMain->IsProjectOpen();

    // Animations tab
    case kRibbonCmdTransitionGallery:
    case kRibbonCmdTransitionNone:
    case kRibbonCmdTransitionCrossfade:
    case kRibbonCmdTransitionWipe:
    case kRibbonCmdTransitionSlide:
    case kRibbonCmdTransitionFade:
    case kRibbonCmdPanZoomGallery:
    case kRibbonCmdPanZoomNone:
    case kRibbonCmdPanZoomSlowPan:
    case kRibbonCmdPanZoomZoomIn:
    case kRibbonCmdPanZoomZoomOut:
        return m_pAppMain->IsProjectOpen();

    // View tab
    case kRibbonCmdZoomToFit:
    case kRibbonCmdZoomIn:
    case kRibbonCmdZoomOut:
    case kRibbonCmdShowStoryboard:
    case kRibbonCmdPreviewQuality:
        return m_pAppMain->IsProjectOpen();

    // Save commands
    case kRibbonCmdSaveFile:
        return m_pAppMain->IsProjectOpen();

    // Tab activation commands - always enabled
    case kRibbonCmdHome:
    case kRibbonCmdShare:
    case kRibbonCmdView:
        return true;

    // Zoom
    case kRibbonCmdZoomToTimeline:
        return m_pAppMain->IsProjectOpen();

    default:
        return true;
    }
}

// ============================================================================
// SetPressed
// ============================================================================
HRESULT RibbonApp::SetPressed(UINT nCmdId, bool fPressed)
{
    auto it = m_commandHandlers.find(nCmdId);
    if (it != m_commandHandlers.end())
        it->second.fPressed = fPressed;

    if (!m_spFramework)
        return E_FAIL;

    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    propvar.vt = VT_BOOL;
    propvar.boolVal = fPressed ? VARIANT_TRUE : VARIANT_FALSE;

    return m_spFramework->SetUICommandProperty(
        nCmdId, UI_PKEY_Pressed, propvar);
}

// ============================================================================
// UpdateCommandStateFromApp
//
// Synchronizes all ribbon command states with the current application state.
// Called after project open/close, undo/redo stack change, selection change,
// or any other state mutation that affects command enablement.
// ============================================================================
HRESULT RibbonApp::UpdateCommandStateFromApp()
{
    if (!m_pAppMain || !m_spFramework)
        return E_FAIL;

    // Update all known commands from m_commandHandlers
    for (auto& pair : m_commandHandlers)
    {
        UINT nCmdId = pair.first;
        bool fEnabled = ComputeCommandEnabled(nCmdId);
        pair.second.fEnabled = fEnabled;
    }

    // Flush framework to trigger re-query of all command states
    m_spFramework->FlushPendingInvalidations();

    return S_OK;
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

    // Register each command ID from m_commandHandlers with the framework
    // Note: RegisterUICommand was removed from newer SDKs; use stub
    for (auto& pair : m_commandHandlers)
    {
        UINT nCmdId = pair.first;
        hr = RegisterUICommandStub(m_spFramework, nCmdId, spHandler);
        if (FAILED(hr))
            return hr;
    }

    // Also register simple command handler IDs from m_commandMap so the
    // framework knows about them and will route Execute calls through us
    for (auto& pair : m_commandMap)
    {
        UINT nCmdId = pair.first;

        // Skip if already registered via m_commandHandlers
        if (m_commandHandlers.find(nCmdId) != m_commandHandlers.end())
            continue;

        hr = RegisterUICommandStub(m_spFramework, nCmdId, spHandler);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

} // namespace SundanceUI
