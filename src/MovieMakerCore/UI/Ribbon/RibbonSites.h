/*
 * RibbonSites.h
 *
 * Ribbon UI control site wrappers for MovieMakerCore.dll. Each site
 * class wraps an IUIRibbon or IUISimplePropertySet interface for a
 * specific ribbon control type (MRU, spinner, font, color picker, etc.).
 *
 * RTTI classes:
 *   ?AVRibbonList@@, ?AVRibbonListItem@@
 *   ?AVRibbonCategoryItem@@, ?AVRibbonCategoryList@@
 *   ?AVCMRUSite@@, ?AVCMRUItem@@
 *   ?AVCSpinnerSite@@, ?AVCGroupSite@@
 *   ?AVCFontSite@@, ?AVCColorPickerSite@@
 *   ?AVCGenericSite@@, ?AVCGallerySite@@
 *   ?AVToggleSite@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef RIBBONSITES_H
#define RIBBONSITES_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"

namespace SundanceUI
{

// ============================================================================
// RibbonListItem
// ============================================================================
// A single item in a ribbon list/gallery control.
//
struct RIBBON_API RibbonListItem
{
    UINT        nItemId;
    ATL::CString strLabel;
    ATL::CString strTooltip;
    ATL::CString strIconPath;
    UINT        nIconIndex;
    DWORD       dwData;
    bool        fEnabled;
    bool        fSelected;

    RibbonListItem()
        : nItemId(0)
        , nIconIndex(0)
        , dwData(0)
        , fEnabled(true)
        , fSelected(false)
    {
    }
};

// ============================================================================
// RibbonCategoryItem
// ============================================================================
// A category grouping in the ribbon.
//
struct RIBBON_API RibbonCategoryItem
{
    UINT        nCategoryId;
    ATL::CString strName;
    ATL::CString strLabel;
    UINT        nOrder;
    bool        fVisible;

    RibbonCategoryItem()
        : nCategoryId(0)
        , nOrder(0)
        , fVisible(true)
    {
    }
};

// ============================================================================
// RibbonList
// ============================================================================
// Manages a list of items for a ribbon list/gallery control. Provides
// add/remove/find/enumeration and handles item selection state.
//
class RIBBON_API RibbonList
{
public:
    RibbonList();
    ~RibbonList();

    // Item management
    size_t GetItemCount() const throw();
    const RibbonListItem* GetItem(size_t nIndex) const;
    RibbonListItem* GetItem(size_t nIndex);
    const RibbonListItem* FindItem(UINT nItemId) const;
    RibbonListItem* FindItem(UINT nItemId);

    size_t AddItem(const RibbonListItem& item);
    void RemoveItem(UINT nItemId);
    void RemoveAllItems();

    // Selection
    UINT GetSelectedItemId() const throw();
    HRESULT SetSelectedItemId(UINT nItemId);
    void ClearSelection();

    // Category filtering
    size_t GetCategoryCount() const throw();
    const RibbonCategoryItem* GetCategory(size_t nIndex) const;
    size_t AddCategory(const RibbonCategoryItem& category);
    void RemoveAllCategories();

    // Refresh display
    HRESULT Refresh();

private:
    ATL::CAtlArray<RibbonListItem>      m_arrItems;
    ATL::CAtlArray<RibbonCategoryItem>  m_arrCategories;
    UINT                                m_nSelectedItemId;
};

// ============================================================================
// RibbonListItem - additional accessors (inline)
// ============================================================================
// (Already defined as a struct above)

// ============================================================================
// RibbonCategoryList
// ============================================================================
// A list of categories for grouping ribbon items.
//
class RIBBON_API RibbonCategoryList
{
public:
    RibbonCategoryList();
    ~RibbonCategoryList();

    size_t GetCategoryCount() const throw();
    const RibbonCategoryItem* GetCategory(size_t nIndex) const;
    RibbonCategoryItem* GetCategory(size_t nIndex);

    size_t AddCategory(const RibbonCategoryItem& category);
    void RemoveCategory(UINT nCategoryId);
    void RemoveAllCategories();

    const RibbonCategoryItem* FindCategory(UINT nCategoryId) const;

private:
    ATL::CAtlArray<RibbonCategoryItem> m_arrCategories;
};

// ============================================================================
// CMRUSite / CMRUItem
// ============================================================================
// Most-recently-used item site. Wraps the MRU list in the ribbon
// application menu for quick access to recent files.
//
struct RIBBON_API CMRUItem
{
    ATL::CString strFilePath;
    ATL::CString strDisplayName;
    FILETIME    ftLastAccess;
    DWORD       dwFlags;

    CMRUItem()
        : dwFlags(0)
    {
        ftLastAccess.dwLowDateTime = 0;
        ftLastAccess.dwHighDateTime = 0;
    }
};

class RIBBON_API CMRUSite
{
public:
    CMRUSite();
    ~CMRUSite();

    // Lifecycle
    HRESULT Initialize(UINT uMaxItems = 10);
    HRESULT Shutdown();

    // MRU list
    size_t GetItemCount() const throw();
    const CMRUItem* GetItem(size_t nIndex) const;

    HRESULT AddItem(LPCWSTR pszFilePath, LPCWSTR pszDisplayName = nullptr);
    HRESULT RemoveItem(LPCWSTR pszFilePath);
    void Clear();

    // Find
    const CMRUItem* FindItem(LPCWSTR pszFilePath) const;

    // Persistence
    HRESULT SaveToRegistry(LPCWSTR pszRegKey);
    HRESULT LoadFromRegistry(LPCWSTR pszRegKey);

    // Pinning
    HRESULT PinItem(LPCWSTR pszFilePath, bool fPin);
    bool IsPinned(LPCWSTR pszFilePath) const throw();

private:
    ATL::CAtlArray<CMRUItem> m_arrItems;
    UINT                     m_uMaxItems;
    bool                     m_fInitialized;
};

// ============================================================================
// CSpinnerSite
// ============================================================================
// Wraps a ribbon spinner (numeric up/down) control. Provides value
// get/set and range configuration.
//
class RIBBON_API CSpinnerSite
{
public:
    CSpinnerSite();
    ~CSpinnerSite();

    HRESULT Initialize(UINT nCmdId, int nMin, int nMax, int nValue, int nStep = 1);
    HRESULT Shutdown();

    int GetValue() const throw();
    HRESULT SetValue(int nValue);

    int GetMinValue() const throw();
    int GetMaxValue() const throw();
    int GetStep() const throw();

    void SetRange(int nMin, int nMax) throw();
    void SetStep(int nStep) throw();

    UINT GetCommandId() const throw();

private:
    UINT    m_nCmdId;
    int     m_nValue;
    int     m_nMin;
    int     m_nMax;
    int     m_nStep;
};

// ============================================================================
// CGroupSite
// ============================================================================
// Wraps a ribbon group (toolbar section). Manages visibility and
// enabled state of an entire group of controls.
//
class RIBBON_API CGroupSite
{
public:
    CGroupSite();
    ~CGroupSite();

    HRESULT Initialize(UINT nGroupId, LPCWSTR pszLabel);
    HRESULT Shutdown();

    UINT GetGroupId() const throw();
    ATL::CString GetLabel() const;

    bool IsVisible() const throw();
    HRESULT SetVisible(bool fVisible);

    bool IsEnabled() const throw();
    HRESULT SetEnabled(bool fEnabled);

    // Contextual group support
    bool IsContextual() const throw();
    void SetContextual(bool fContextual) throw();

    DWORD GetContextColor() const throw();
    void SetContextColor(DWORD dwColor) throw();

private:
    UINT        m_nGroupId;
    ATL::CString m_strLabel;
    bool        m_fVisible;
    bool        m_fEnabled;
    bool        m_fContextual;
    DWORD       m_dwContextColor;
};

// ============================================================================
// CFontSite
// ============================================================================
// Wraps a ribbon font picker control. Provides font family, size, style,
// and color selection.
//
class RIBBON_API CFontSite
{
public:
    CFontSite();
    ~CFontSite();

    HRESULT Initialize(UINT nCmdId);
    HRESULT Shutdown();

    // Font family
    ATL::CString GetFontFamily() const;
    HRESULT SetFontFamily(LPCWSTR pszFamily);

    // Font size
    float GetFontSize() const throw();
    HRESULT SetFontSize(float flSize);

    // Font style (bold, italic, underline)
    DWORD GetFontStyle() const throw();
    HRESULT SetFontStyle(DWORD dwStyle);

    // Font color
    DWORD GetFontColor() const throw();
    HRESULT SetFontColor(DWORD dwColor);

    // Preview
    HRESULT UpdatePreview();

    UINT GetCommandId() const throw();

private:
    UINT        m_nCmdId;
    ATL::CString m_strFontFamily;
    float       m_flFontSize;
    DWORD       m_dwFontStyle;
    DWORD       m_dwFontColor;
};

// ============================================================================
// CColorPickerSite
// ============================================================================
// Wraps a ribbon color picker control. Provides color selection with
// recent color history and theme color support.
//
class RIBBON_API CColorPickerSite
{
public:
    CColorPickerSite();
    ~CColorPickerSite();

    HRESULT Initialize(UINT nCmdId, DWORD dwDefaultColor = 0xFFFFFFFF);
    HRESULT Shutdown();

    // Current color
    DWORD GetColor() const throw();
    HRESULT SetColor(DWORD dwColor);

    // Theme colors
    size_t GetThemeColorCount() const throw();
    DWORD GetThemeColor(size_t nIndex) const throw();
    HRESULT SetThemeColors(const DWORD* pColors, size_t cColors);

    // Recent colors
    size_t GetRecentColorCount() const throw();
    DWORD GetRecentColor(size_t nIndex) const throw();
    HRESULT AddRecentColor(DWORD dwColor);

    // Automatic color
    DWORD GetAutomaticColor() const throw();
    void SetAutomaticColor(DWORD dwColor) throw();

    UINT GetCommandId() const throw();

private:
    UINT    m_nCmdId;
    DWORD   m_dwColor;
    DWORD   m_dwAutomaticColor;

    ATL::CAtlArray<DWORD> m_arrThemeColors;
    ATL::CAtlArray<DWORD> m_arrRecentColors;
    size_t                m_nMaxRecentColors;
};

// ============================================================================
// CGenericSite
// ============================================================================
// Generic wrapper for ribbon controls without specialized behavior.
// Provides basic enabled/visible/label state management.
//
class RIBBON_API CGenericSite
{
public:
    CGenericSite();
    ~CGenericSite();

    HRESULT Initialize(UINT nCmdId, LPCWSTR pszLabel = nullptr);
    HRESULT Shutdown();

    UINT GetCommandId() const throw();
    ATL::CString GetLabel() const;

    bool IsEnabled() const throw();
    HRESULT SetEnabled(bool fEnabled);

    bool IsVisible() const throw();
    HRESULT SetVisible(bool fVisible);

    bool IsPressed() const throw();
    HRESULT SetPressed(bool fPressed);

    // Generic data
    DWORD_PTR GetUserData() const throw();
    void SetUserData(DWORD_PTR dwData) throw();

private:
    UINT        m_nCmdId;
    ATL::CString m_strLabel;
    bool        m_fEnabled;
    bool        m_fVisible;
    bool        m_fPressed;
    DWORD_PTR   m_dwUserData;
};

// ============================================================================
// CGallerySite
// ============================================================================
// Wraps a ribbon gallery control. Provides scrolling gallery items with
// thumbnails (used for themes, effects, transitions in the ribbon).
//
class RIBBON_API CGallerySite
{
public:
    CGallerySite();
    ~CGallerySite();

    HRESULT Initialize(UINT nCmdId, UINT uItemWidth = 80, UINT uItemHeight = 80);
    HRESULT Shutdown();

    // Items (delegates to internal RibbonList)
    RibbonList* GetItemList();
    const RibbonList* GetItemList() const;

    // Layout
    UINT GetItemWidth() const throw();
    UINT GetItemHeight() const throw();
    void SetItemDimensions(UINT uWidth, UINT uHeight) throw();

    UINT GetColumns() const throw();
    void SetColumns(UINT uColumns) throw();

    // Selection
    HRESULT SetSelectedItem(UINT nItemId);
    UINT GetSelectedItem() const throw();

    // Dropdown mode
    bool IsDropdown() const throw();
    void SetDropdown(bool fDropdown) throw();

    UINT GetCommandId() const throw();

private:
    UINT        m_nCmdId;
    UINT        m_uItemWidth;
    UINT        m_uItemHeight;
    UINT        m_uColumns;
    bool        m_fDropdown;
    RibbonList  m_list;
};

// ============================================================================
// ToggleSite
// ============================================================================
// Wraps a ribbon toggle button. Tracks checked/unchecked state and
// provides label updates for the two-state button.
//
class RIBBON_API ToggleSite
{
public:
    ToggleSite();
    ~ToggleSite();

    HRESULT Initialize(UINT nCmdId, LPCWSTR pszLabelOn, LPCWSTR pszLabelOff);
    HRESULT Shutdown();

    // Toggle state
    bool IsChecked() const throw();
    HRESULT SetChecked(bool fChecked);
    HRESULT Toggle();

    // Labels
    ATL::CString GetLabelOn() const;
    ATL::CString GetLabelOff() const;
    HRESULT SetLabels(LPCWSTR pszLabelOn, LPCWSTR pszLabelOff);

    UINT GetCommandId() const throw();

private:
    UINT        m_nCmdId;
    ATL::CString m_strLabelOn;
    ATL::CString m_strLabelOff;
    bool        m_fChecked;
};

// ============================================================================
// RibbonSiteRegistry
// ============================================================================
// Maps HWNDs to RibbonApp instances, allowing multiple windows to each
// host their own ribbon. Used by the window proc to route WM_COMMAND
// to the correct RibbonApp.
//
class RibbonApp;

class RIBBON_API RibbonSiteRegistry
{
public:
    static RibbonSiteRegistry& Instance() throw();

    // Register a window as hosting a ribbon
    HRESULT RegisterSite(HWND hWnd, RibbonApp* pRibbon);

    // Unregister a window
    HRESULT UnregisterSite(HWND hWnd);

    // Get the ribbon for a window (returns nullptr if not registered)
    RibbonApp* GetSite(HWND hWnd) const throw();

    // Number of registered sites
    size_t GetSiteCount() const throw();

    // Clear all sites
    void Clear();

private:
    RibbonSiteRegistry();
    ~RibbonSiteRegistry();
    RibbonSiteRegistry(const RibbonSiteRegistry&);
    RibbonSiteRegistry& operator=(const RibbonSiteRegistry&);

    std::map<HWND, RibbonApp*> m_sites;
};

} // namespace SundanceUI

#endif // RIBBONSITES_H
