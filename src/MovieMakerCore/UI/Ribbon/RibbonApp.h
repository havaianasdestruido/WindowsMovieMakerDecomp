/*
 * RibbonApp.h
 *
 * Ribbon application integration for MovieMakerCore.dll's UI layer.
 * Provides the RibbonApp class that hosts the IUIFramework ribbon
 * integration and dispatches ribbon events to SundanceAppMain.
 *
 * RTTI classes:
 *   ?AVRibbonApp@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef RIBBONAPP_H
#define RIBBONAPP_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"

// Ribbon UI headers (Windows 8+)
#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>

#ifndef REFUIKEY
typedef const PROPVARIANT& REFUIKEY;
#endif

// UI_VIEWTYPE and UI_VIEWVERB are defined by <UIRibbon.h> as enums

interface IUIView;

// Forward declaration of SundanceAppMain (global namespace)
class SundanceAppMain;

namespace SundanceUI
{

// ============================================================================
// Ribbon command IDs (from ribbon XML resource)
// ============================================================================
static const UINT kRibbonCmdHome         = 100;
static const UINT kRibbonCmdShare         = 101;
static const UINT kRibbonCmdView          = 102;

// Home tab commands
static const UINT kRibbonCmdAddVideos     = 200;
static const UINT kRibbonCmdAddPhotos     = 201;
static const UINT kRibbonCmdAddMusic      = 202;
static const UINT kRibbonCmdWebcam        = 203;
static const UINT kRibbonCmdNarrate       = 204;

static const UINT kRibbonCmdUndo          = 210;
static const UINT kRibbonCmdRedo          = 211;

static const UINT kRibbonCmdCut           = 220;
static const UINT kRibbonCmdCopy          = 221;
static const UINT kRibbonCmdPaste         = 222;
static const UINT kRibbonCmdDelete        = 223;

static const UINT kRibbonCmdSelectAll     = 230;

static const UINT kRibbonCmdTheme         = 240;
static const UINT kRibbonCmdAutoMovie     = 241;
static const UINT kRibbonCmdSnapshot      = 242;

static const UINT kRibbonCmdTrim          = 250;
static const UINT kRibbonCmdSplit         = 251;
static const UINT kRibbonCmdSetTitle      = 252;
static const UINT kRibbonCmdSetCredits    = 253;

static const UINT kRibbonCmdSpeed         = 260;
static const UINT kRibbonCmdVolume        = 261;

// Share tab commands
static const UINT kRibbonCmdSaveMovie     = 300;
static const UINT kRibbonCmdSaveFile      = 301;
static const UINT kRibbonCmdPublish        = 302;

// Standard application command IDs
// Undef Windows SDK macros that conflict with our IDs
#ifdef ID_FILE_SAVE
#undef ID_FILE_SAVE
#endif
#ifdef ID_EDIT_UNDO
#undef ID_EDIT_UNDO
#endif
#ifdef ID_EDIT_REDO
#undef ID_EDIT_REDO
#endif

static const UINT ID_FILE_IMPORT    = 0xE150;
static const UINT ID_FILE_SAVE      = 0xE103;
static const UINT ID_FILE_EXPORT    = 0xE151;

static const UINT ID_EDIT_UNDO      = 0xE12B;
static const UINT ID_EDIT_REDO      = 0xE12C;

static const UINT ID_PLAY_PLAY      = 0xE230;
static const UINT ID_PLAY_PAUSE     = 0xE231;
static const UINT ID_PLAY_STOP      = 0xE232;

// ============================================================================
// RibbonCommandHandler callback type
// ============================================================================
typedef HRESULT (::SundanceAppMain::*RibbonCommandCallback)(UINT nCmdId, UI_COMMANDTYPE commandType, IUISimplePropertySet* pArgs);

// ============================================================================
// RibbonCommandEntry - per-command registration data
// ============================================================================
struct RIBBON_API RibbonCommandEntry
{
    UINT                    nCmdId;
    RibbonCommandCallback   pCallback;
    bool                    fEnabled;
    bool                    fVisible;

    RibbonCommandEntry()
        : nCmdId(0)
        , pCallback(nullptr)
        , fEnabled(true)
        , fVisible(true)
    {
    }
};

// ============================================================================
// RibbonApp
// ============================================================================
// Main ribbon application host. Implements IUICommandHandler and
// IUIApplication for the Windows 8+ ribbon framework.
//
class RIBBON_API RibbonApp
{
public:
    RibbonApp();
    ~RibbonApp();

    // Lifecycle
    HRESULT Initialize(SundanceAppMain* pAppMain, HWND hWnd);
    HRESULT Shutdown();

    // Ribbon framework access
    IUIFramework* GetFramework();
    IUIRibbon* GetRibbon();
    HRESULT LoadUI(HINSTANCE hInstance, LPCWSTR pszRibbonResource);

    // Command handling
    HRESULT OnCommand(UINT nCmdId, UI_COMMANDTYPE commandType, IUICommandHandler* pHandler);
    HRESULT OnUpdateProperty(UINT nCmdId, REFUIKEY key, const PROPVARIANT* pCurrentValue, PROPVARIANT* pNewValue);

    // Application menu
    HRESULT PopulateApplicationMenu(IUIRibbon* pRibbon);

    // Ribbon state
    bool IsInitialized() const throw();
    HWND GetOwnerHwnd() const throw();
    void SetOwnerHwnd(HWND hwnd) throw();

    // Ribbon mode changes
    HRESULT OnViewChanged(UINT nViewId, UI_VIEWTYPE type, IUIView* pView, UI_VIEWVERB verb, UINT uParam);

    // Update ribbon state from application
    HRESULT UpdateCommandState(UINT nCmdId, bool fEnabled, bool fHidden);
    HRESULT UpdateTooltip(UINT nCmdId, LPCWSTR pszTooltip);
    HRESULT UpdateLabel(UINT nCmdId, LPCWSTR pszLabel);

    // Command handler registration
    HRESULT RegisterCommandHandler(UINT nCmdId, RibbonCommandCallback pCallback);
    HRESULT UnregisterCommandHandler(UINT nCmdId);

    // Simple command handler registration (std::function<void()>)
    void RegisterCommandHandler(UINT nCmdId, std::function<void()> handler);
    void ExecuteSimpleCommand(UINT nCmdId);

    // Batch UI update
    void UpdateUI();

    // Per-command UI manipulation
    HRESULT SetEnabled(UINT nCmdId, bool fEnabled);
    HRESULT SetVisible(UINT nCmdId, bool fVisible);
    HRESULT SetText(UINT nCmdId, LPCWSTR pszText);

    // Contextual tabs
    HRESULT ShowContextualTab(UINT nTabId);
    HRESULT HideContextualTab(UINT nTabId);

    // IUICommandHandler methods (called by framework via CCommandHandler)
    HRESULT Execute(UINT nCmdId, UI_COMMANDTYPE commandType, IUISimplePropertySet* pArgs);
    HRESULT UpdateState(UINT nCmdId, REFPROPERTYKEY key, const PROPVARIANT* pCurrentValue, PROPVARIANT* pNewValue);

private:
    HRESULT CreateUIFramework();
    HRESULT LoadRibbonFromResource(HINSTANCE hInstance, LPCWSTR pszResource);
    HRESULT RegisterFrameworkCommands();
    HRESULT GetRibbonView();
    void    RegisterCommonCommands();

    // Command handler map
    std::map<UINT, RibbonCommandEntry> m_commandHandlers;

    // Simple command handler map (void lambdas for common commands)
    std::map<UINT, std::function<void()>> m_commandMap;

    CComPtr<IUIFramework>       m_spFramework;
    CComPtr<IUIRibbon>          m_spRibbon;
    SundanceAppMain*            m_pAppMain;
    HWND                        m_hwndOwner;
    bool                        m_fInitialized;
    HINSTANCE                   m_hInstance;

    // Ribbon hosting element ID
    static const UINT UI_HOSTING_ELEMENT = 2;
};

} // namespace SundanceUI

#endif // RIBBONAPP_H
