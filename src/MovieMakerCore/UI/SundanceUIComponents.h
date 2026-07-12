/*
 * SundanceUIComponents.h
 *
 * Core Sundance UI component behaviors: main element, behavior factory,
 * AMP main window, AMP command bar, and application data context.
 *
 * RTTI classes:
 *   ?AVSundanceMainElementBehavior@@
 *   ?AVSundanceBehaviorFactory@@
 *   ?AVAMPMainWindowBehavior@@
 *   ?AVAMPCommandBarBehavior@@
 *   ?AVAMPDataContext@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_UI_COMPONENTS_H
#define SUNDANCE_UI_COMPONENTS_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

class SundanceAppMain;

namespace Sundance
{

// ============================================================================
// SundanceMainElementBehavior
// ============================================================================
// Root behavior attached to the main Sundance element in the DirectUI tree.
// This is the top-level coordinator that initializes all child behaviors,
// manages the main element lifecycle, and routes application-level events
// (project open/close, view mode changes) to child elements.
//
// RTTI: ?AVSundanceMainElementBehavior@@
// ATL:  CComObjectNoLock<SundanceMainElementBehavior>
//
class ATL_NO_VTABLE SundanceMainElementBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SundanceMainElementBehavior>,
    public IDuiBehaviorImpl
{
public:
    SundanceMainElementBehavior();
    virtual ~SundanceMainElementBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_SUNDANCE_MAIN_ELEMENT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(SundanceMainElementBehavior)

    BEGIN_COM_MAP(SundanceMainElementBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiBehavior --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;
    STDMETHOD(OnBehaviorChanged)() override;

    // -- Main element operations --
    HRESULT LoadLayout(LPCWSTR pszDuxtResource);
    HRESULT RefreshLayout();
    HWND    GetHostHwnd() const throw();
    void    SetHostHwnd(HWND hWnd) throw();

    // -- View mode management --
    enum ViewMode
    {
        ViewModeNormal = 0,
        ViewModeFullscreen,
        ViewModePreview,
        ViewModePublish
    };

    HRESULT SetViewMode(ViewMode mode);
    ViewMode GetViewMode() const throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    HWND            m_hWndHost;
    ViewMode        m_viewMode;
};

// ============================================================================
// SundanceBehaviorFactory
// ============================================================================
// Factory for creating Sundance behavior instances by class name.
// Registered with the DirectUI element builder so that .duxt markup
// references to behavior names are resolved to concrete C++ classes.
// This is the public-facing wrapper around the internal registration
// table in SundanceBehaviors.cpp.
//
// RTTI: ?AVSundanceBehaviorFactory@@
//
class SundanceBehaviorFactory
{
public:
    SundanceBehaviorFactory();
    ~SundanceBehaviorFactory();

    // -- Factory operations --
    HRESULT CreateBehaviorByName(LPCWSTR pszClassName, IDuiBehavior** ppBehavior);
    bool    IsBehaviorClass(LPCWSTR pszClassName) const;
    HRESULT GetRegisteredClasses(std::vector<ATL::CString>& classNames);

    // -- Registration --
    HRESULT RegisterBehaviorClass(LPCWSTR pszClassName, void* pfnFactory);
    HRESULT UnregisterBehaviorClass(LPCWSTR pszClassName);

    // -- Singleton --
    static SundanceBehaviorFactory& GetInstance();

private:
    struct FactoryEntry
    {
        ATL::CString strClassName;
        void*        pfnFactory;
    };

    std::vector<FactoryEntry> m_entries;
    bool m_bInitialized;

    void EnsureInitialized();
};

// ============================================================================
// AMPMainWindowBehavior
// ============================================================================
// Behavior for the AMP (Application Model Presentation) main window element.
// AMP is the presentation layer framework used by Windows Live applications.
// This behavior manages the main window chrome, title bar integration,
// DWM composition, non-client area rendering, and system menu handling.
//
// RTTI: ?AVAMPMainWindowBehavior@@
// ATL:  CComObjectNoLock<AMPMainWindowBehavior>
//
class ATL_NO_VTABLE AMPMainWindowBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<AMPMainWindowBehavior>,
    public IDuiBehaviorImpl
{
public:
    AMPMainWindowBehavior();
    virtual ~AMPMainWindowBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_AMP_MAIN_WINDOW_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(AMPMainWindowBehavior)

    BEGIN_COM_MAP(AMPMainWindowBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiBehavior --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;
    STDMETHOD(OnBehaviorChanged)() override;

    // -- Window management --
    HRESULT SetTitle(LPCWSTR pszTitle);
    HRESULT SetIcon(HICON hIcon);
    HRESULT SetMinimizeBox(bool bEnable);
    HRESULT SetMaximizeBox(bool bEnable);
    HRESULT SetCloseButton(bool bEnable);

    // -- Window state --
    HRESULT Maximize();
    HRESULT Restore();
    HRESULT Minimize();
    HRESULT SetFullscreen(bool bFullscreen);
    bool    IsFullscreen() const throw();

    // -- DWM integration --
    HRESULT EnableAeroGlass(bool bEnable);
    HRESULT SetWindowBackgroundColor(COLORREF clrBackground);
    HRESULT UpdateNonClientArea();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    HWND            m_hWndMain;
    ATL::CString    m_strTitle;
    HICON           m_hIcon;
    bool            m_bFullscreen;
    bool            m_bMinimizeBox;
    bool            m_bMaximizeBox;
    bool            m_bCloseButton;
    COLORREF        m_clrBackground;

    void OnNcCalcSize(WPARAM wParam, LPARAM lParam);
    void OnNcPaint(WPARAM wParam, LPARAM lParam);
    void OnNcHitTest(LPARAM lParam, LRESULT* plResult);
    void OnDwmCompositionChanged();
};

// ============================================================================
// AMPCommandBarBehavior
// ============================================================================
// Behavior for the AMP command bar (ribbon/toolbar area). Manages command
// buttons, split buttons, dropdown menus, and command state updates.
// Interacts with the SundanceAppMain command state to enable/disable
// buttons based on current application state.
//
// RTTI: ?AVAMPCommandBarBehavior@@
// ATL:  CComObjectNoLock<AMPCommandBarBehavior>
//
class ATL_NO_VTABLE AMPCommandBarBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<AMPCommandBarBehavior>,
    public IDuiBehaviorImpl
{
public:
    AMPCommandBarBehavior();
    virtual ~AMPCommandBarBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_AMP_COMMAND_BAR_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(AMPCommandBarBehavior)

    BEGIN_COM_MAP(AMPCommandBarBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiBehavior --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Command state --
    HRESULT UpdateCommandStates();
    HRESULT SetButtonEnabled(LPCWSTR pszCommandId, bool bEnabled);
    HRESULT SetButtonChecked(LPCWSTR pszCommandId, bool bChecked);
    HRESULT SetButtonLabel(LPCWSTR pszCommandId, LPCWSTR pszLabel);

    // -- Command bar layout --
    HRESULT SetVisible(bool bVisible) throw();
    bool    IsVisible() const throw();
    int     GetHeight() const throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    bool            m_bVisible;
    int             m_nHeight;

    struct CommandState
    {
        ATL::CString strCommandId;
        bool        bEnabled;
        bool        bChecked;
        ATL::CString strLabel;

        CommandState() : bEnabled(true), bChecked(false) {}
    };

    std::vector<CommandState> m_commands;

    void RefreshCommandStates();
    bool IsCommandRelevant(LPCWSTR pszCommandId) const;
};

// ============================================================================
// AMPDataContext (IDispatch)
// ============================================================================
// COM data context for the AMP presentation layer. Exposes application
// state as IDispatch properties for DirectUI data binding via
// @{PropertyName} syntax in .duxt markup files. Wraps SundanceAppMain
// state queries into a generic property bag.
//
// RTTI: ?AVAMPDataContext@@
// ATL:  CComObjectNoLock<AMPDataContext>
//
class ATL_NO_VTABLE AMPDataContext :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<AMPDataContext>,
    public IDispatchImpl<IDispatch, &IID_IDispatch>
{
public:
    AMPDataContext();
    ~AMPDataContext();

    DECLARE_REGISTRY_RESOURCEID(IDR_AMP_DATA_CONTEXT)
    DECLARE_NOT_AGGREGATABLE(AMPDataContext)

    BEGIN_COM_MAP(AMPDataContext)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    // -- Initialization --
    HRESULT Initialize(SundanceAppMain* pAppMain);

    // -- IDataContext support for DirectUI binding --
    HRESULT GetProperty(LPCWSTR pszName, VARIANT* pvarValue);
    HRESULT SetProperty(LPCWSTR pszName, const VARIANT* varValue);

    // -- Property refresh --
    void RefreshAll();
    void RefreshProperty(LPCWSTR pszName);

    // -- IDispatch --
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgDispId) override;
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override;
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
        EXCEPINFO* pExcepInfo, UINT* puArgErr) override;

private:
    SundanceAppMain*    m_pAppMain;
    std::map<ATL::CString, VARIANT> m_propertyCache;

    // Known property names
    static const WCHAR kPropProjectName[];
    static const WCHAR kPropIsProjectOpen[];
    static const WCHAR kPropIsProjectDirty[];
    static const WCHAR kPropCanUndo[];
    static const WCHAR kPropCanRedo[];
    static const WCHAR kPropCanCut[];
    static const WCHAR kPropCanCopy[];
    static const WCHAR kPropCanPaste[];
    static const WCHAR kPropIsPlaying[];
    static const WCHAR kPropIsPublishing[];
    static const WCHAR kPropIsEncoding[];
    static const WCHAR kPropTimelinePosition[];
    static const WCHAR kPropTimelineDuration[];
    static const WCHAR kPropVersionString[];

    void InitPropertyCache();
    DISPID FindProperty(LPCWSTR pszName) const;
};

} // namespace Sundance

#endif // SUNDANCE_UI_COMPONENTS_H
