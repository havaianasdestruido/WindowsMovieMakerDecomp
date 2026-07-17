#include "pch.h"
/*
 * RibbonSites.cpp
 *
 * Implementation of ribbon UI control site wrappers for MovieMakerCore.dll.
 * Provides CMRUSite, CSpinnerSite, CGroupSite, CFontSite, CColorPickerSite,
 * CGenericSite, CGallerySite, ToggleSite, and RibbonList.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "RibbonSites.h"

namespace SundanceUI
{

// ============================================================================
// RibbonList implementation
// ============================================================================

RibbonList::RibbonList()
    : m_nSelectedItemId(0)
{
}

RibbonList::~RibbonList()
{
}

size_t RibbonList::GetItemCount() const throw() { return m_arrItems.GetCount(); }

const RibbonListItem* RibbonList::GetItem(size_t nIndex) const
{
    if (nIndex >= m_arrItems.GetCount()) return nullptr;
    return &m_arrItems.GetAt(nIndex);
}

RibbonListItem* RibbonList::GetItem(size_t nIndex)
{
    if (nIndex >= m_arrItems.GetCount()) return nullptr;
    return &m_arrItems.GetAt(nIndex);
}

const RibbonListItem* RibbonList::FindItem(UINT nItemId) const
{
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).nItemId == nItemId)
            return &m_arrItems.GetAt(i);
    }
    return nullptr;
}

RibbonListItem* RibbonList::FindItem(UINT nItemId)
{
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).nItemId == nItemId)
            return &m_arrItems.GetAt(i);
    }
    return nullptr;
}

size_t RibbonList::AddItem(const RibbonListItem& item) { return m_arrItems.Add(item); }

void RibbonList::RemoveItem(UINT nItemId)
{
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).nItemId == nItemId)
        {
            m_arrItems.RemoveAt(i);
            return;
        }
    }
}

void RibbonList::RemoveAllItems() { m_arrItems.RemoveAll(); }

UINT RibbonList::GetSelectedItemId() const throw() { return m_nSelectedItemId; }

HRESULT RibbonList::SetSelectedItemId(UINT nItemId)
{
    m_nSelectedItemId = nItemId;
    return S_OK;
}

void RibbonList::ClearSelection() { m_nSelectedItemId = 0; }

size_t RibbonList::GetCategoryCount() const throw() { return m_arrCategories.GetCount(); }

const RibbonCategoryItem* RibbonList::GetCategory(size_t nIndex) const
{
    if (nIndex >= m_arrCategories.GetCount()) return nullptr;
    return &m_arrCategories.GetAt(nIndex);
}

size_t RibbonList::AddCategory(const RibbonCategoryItem& category) { return m_arrCategories.Add(category); }

void RibbonList::RemoveAllCategories() { m_arrCategories.RemoveAll(); }

HRESULT RibbonList::Refresh() { return S_OK; }

// ============================================================================
// RibbonCategoryList implementation
// ============================================================================

RibbonCategoryList::RibbonCategoryList() {}

RibbonCategoryList::~RibbonCategoryList() {}

size_t RibbonCategoryList::GetCategoryCount() const throw() { return m_arrCategories.GetCount(); }

const RibbonCategoryItem* RibbonCategoryList::GetCategory(size_t nIndex) const
{
    if (nIndex >= m_arrCategories.GetCount()) return nullptr;
    return &m_arrCategories.GetAt(nIndex);
}

RibbonCategoryItem* RibbonCategoryList::GetCategory(size_t nIndex)
{
    if (nIndex >= m_arrCategories.GetCount()) return nullptr;
    return &m_arrCategories.GetAt(nIndex);
}

size_t RibbonCategoryList::AddCategory(const RibbonCategoryItem& category) { return m_arrCategories.Add(category); }

void RibbonCategoryList::RemoveCategory(UINT nCategoryId)
{
    for (size_t i = 0; i < m_arrCategories.GetCount(); ++i)
    {
        if (m_arrCategories.GetAt(i).nCategoryId == nCategoryId)
        {
            m_arrCategories.RemoveAt(i);
            return;
        }
    }
}

void RibbonCategoryList::RemoveAllCategories() { m_arrCategories.RemoveAll(); }

const RibbonCategoryItem* RibbonCategoryList::FindCategory(UINT nCategoryId) const
{
    for (size_t i = 0; i < m_arrCategories.GetCount(); ++i)
    {
        if (m_arrCategories.GetAt(i).nCategoryId == nCategoryId)
            return &m_arrCategories.GetAt(i);
    }
    return nullptr;
}

// ============================================================================
// CMRUSite implementation
// ============================================================================

CMRUSite::CMRUSite() : m_uMaxItems(10), m_fInitialized(false) {}
CMRUSite::~CMRUSite() {}

HRESULT CMRUSite::Initialize(UINT uMaxItems)
{
    m_uMaxItems = uMaxItems;
    m_fInitialized = true;
    return S_OK;
}

HRESULT CMRUSite::Shutdown()
{
    m_arrItems.RemoveAll();
    m_fInitialized = false;
    return S_OK;
}

size_t CMRUSite::GetItemCount() const throw() { return m_arrItems.GetCount(); }

const CMRUItem* CMRUSite::GetItem(size_t nIndex) const
{
    if (nIndex >= m_arrItems.GetCount()) return nullptr;
    return &m_arrItems.GetAt(nIndex);
}

HRESULT CMRUSite::AddItem(LPCWSTR pszFilePath, LPCWSTR pszDisplayName)
{
    if (!pszFilePath) return E_INVALIDARG;

    CMRUItem item;
    item.strFilePath = pszFilePath;
    item.strDisplayName = pszDisplayName ? pszDisplayName : pszFilePath;
    GetSystemTimeAsFileTime(&item.ftLastAccess);

    // Remove existing entry for same path
    RemoveItem(pszFilePath);

    // Add at front (most recent)
    m_arrItems.InsertAt(0, item);

    // Trim if over max
    while (m_arrItems.GetCount() > m_uMaxItems)
    {
        m_arrItems.RemoveAt(m_arrItems.GetCount() - 1);
    }

    return S_OK;
}

HRESULT CMRUSite::RemoveItem(LPCWSTR pszFilePath)
{
    if (!pszFilePath) return E_INVALIDARG;

    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).strFilePath.CompareNoCase(pszFilePath) == 0)
        {
            m_arrItems.RemoveAt(i);
            return S_OK;
        }
    }
    return S_FALSE;
}

void CMRUSite::Clear() { m_arrItems.RemoveAll(); }

const CMRUItem* CMRUSite::FindItem(LPCWSTR pszFilePath) const
{
    if (!pszFilePath) return nullptr;
    for (size_t i = 0; i < m_arrItems.GetCount(); ++i)
    {
        if (m_arrItems.GetAt(i).strFilePath.CompareNoCase(pszFilePath) == 0)
            return &m_arrItems.GetAt(i);
    }
    return nullptr;
}

HRESULT CMRUSite::SaveToRegistry(LPCWSTR pszRegKey) { UNREFERENCED_PARAMETER(pszRegKey); return S_OK; }
HRESULT CMRUSite::LoadFromRegistry(LPCWSTR pszRegKey) { UNREFERENCED_PARAMETER(pszRegKey); return S_OK; }

HRESULT CMRUSite::PinItem(LPCWSTR pszFilePath, bool fPin)
{
    if (!pszFilePath) return E_INVALIDARG;
    CMRUItem* pItem = const_cast<CMRUItem*>(FindItem(pszFilePath));
    if (pItem)
    {
        if (fPin)
            pItem->dwFlags |= 0x1;
        else
            pItem->dwFlags &= ~0x1;
    }
    return S_OK;
}

bool CMRUSite::IsPinned(LPCWSTR pszFilePath) const throw()
{
    const CMRUItem* pItem = FindItem(pszFilePath);
    return pItem && (pItem->dwFlags & 0x1) != 0;
}

// ============================================================================
// CSpinnerSite implementation
// ============================================================================

CSpinnerSite::CSpinnerSite()
    : m_nCmdId(0)
    , m_nValue(0)
    , m_nMin(0)
    , m_nMax(100)
    , m_nStep(1)
{
}

CSpinnerSite::~CSpinnerSite() {}

HRESULT CSpinnerSite::Initialize(UINT nCmdId, int nMin, int nMax, int nValue, int nStep)
{
    m_nCmdId = nCmdId;
    m_nMin = nMin;
    m_nMax = nMax;
    m_nValue = nValue;
    m_nStep = nStep;
    return S_OK;
}

HRESULT CSpinnerSite::Shutdown() { return S_OK; }

int CSpinnerSite::GetValue() const throw() { return m_nValue; }

HRESULT CSpinnerSite::SetValue(int nValue)
{
    if (nValue < m_nMin || nValue > m_nMax)
        return E_INVALIDARG;
    m_nValue = nValue;
    return S_OK;
}

int CSpinnerSite::GetMinValue() const throw() { return m_nMin; }
int CSpinnerSite::GetMaxValue() const throw() { return m_nMax; }
int CSpinnerSite::GetStep() const throw() { return m_nStep; }

void CSpinnerSite::SetRange(int nMin, int nMax) throw() { m_nMin = nMin; m_nMax = nMax; }
void CSpinnerSite::SetStep(int nStep) throw() { m_nStep = nStep; }

UINT CSpinnerSite::GetCommandId() const throw() { return m_nCmdId; }

// ============================================================================
// CGroupSite implementation
// ============================================================================

CGroupSite::CGroupSite()
    : m_nGroupId(0)
    , m_fVisible(true)
    , m_fEnabled(true)
    , m_fContextual(false)
    , m_dwContextColor(0)
{
}

CGroupSite::~CGroupSite() {}

HRESULT CGroupSite::Initialize(UINT nGroupId, LPCWSTR pszLabel)
{
    m_nGroupId = nGroupId;
    m_strLabel = pszLabel ? pszLabel : L"";
    return S_OK;
}

HRESULT CGroupSite::Shutdown() { return S_OK; }

UINT CGroupSite::GetGroupId() const throw() { return m_nGroupId; }
ATL::CString CGroupSite::GetLabel() const { return m_strLabel; }

bool CGroupSite::IsVisible() const throw() { return m_fVisible; }
HRESULT CGroupSite::SetVisible(bool fVisible) { m_fVisible = fVisible; return S_OK; }

bool CGroupSite::IsEnabled() const throw() { return m_fEnabled; }
HRESULT CGroupSite::SetEnabled(bool fEnabled) { m_fEnabled = fEnabled; return S_OK; }

bool CGroupSite::IsContextual() const throw() { return m_fContextual; }
void CGroupSite::SetContextual(bool fContextual) throw() { m_fContextual = fContextual; }

DWORD CGroupSite::GetContextColor() const throw() { return m_dwContextColor; }
void CGroupSite::SetContextColor(DWORD dwColor) throw() { m_dwContextColor = dwColor; }

// ============================================================================
// CFontSite implementation
// ============================================================================

CFontSite::CFontSite()
    : m_nCmdId(0)
    , m_flFontSize(12.0f)
    , m_dwFontStyle(0)
    , m_dwFontColor(0xFF000000)
{
}

CFontSite::~CFontSite() {}

HRESULT CFontSite::Initialize(UINT nCmdId)
{
    m_nCmdId = nCmdId;
    return S_OK;
}

HRESULT CFontSite::Shutdown() { return S_OK; }

ATL::CString CFontSite::GetFontFamily() const { return m_strFontFamily; }
HRESULT CFontSite::SetFontFamily(LPCWSTR pszFamily) { m_strFontFamily = pszFamily ? pszFamily : L"Segoe UI"; return S_OK; }

float CFontSite::GetFontSize() const throw() { return m_flFontSize; }
HRESULT CFontSite::SetFontSize(float flSize) { m_flFontSize = flSize; return S_OK; }

DWORD CFontSite::GetFontStyle() const throw() { return m_dwFontStyle; }
HRESULT CFontSite::SetFontStyle(DWORD dwStyle) { m_dwFontStyle = dwStyle; return S_OK; }

DWORD CFontSite::GetFontColor() const throw() { return m_dwFontColor; }
HRESULT CFontSite::SetFontColor(DWORD dwColor) { m_dwFontColor = dwColor; return S_OK; }

HRESULT CFontSite::UpdatePreview() { return S_OK; }

UINT CFontSite::GetCommandId() const throw() { return m_nCmdId; }

// ============================================================================
// CColorPickerSite implementation
// ============================================================================

CColorPickerSite::CColorPickerSite()
    : m_nCmdId(0)
    , m_dwColor(0xFFFFFFFF)
    , m_dwAutomaticColor(0xFF000000)
    , m_nMaxRecentColors(10)
{
}

CColorPickerSite::~CColorPickerSite() {}

HRESULT CColorPickerSite::Initialize(UINT nCmdId, DWORD dwDefaultColor)
{
    m_nCmdId = nCmdId;
    m_dwColor = dwDefaultColor;
    return S_OK;
}

HRESULT CColorPickerSite::Shutdown() { return S_OK; }

DWORD CColorPickerSite::GetColor() const throw() { return m_dwColor; }
HRESULT CColorPickerSite::SetColor(DWORD dwColor) { m_dwColor = dwColor; return S_OK; }

size_t CColorPickerSite::GetThemeColorCount() const throw() { return m_arrThemeColors.GetCount(); }

DWORD CColorPickerSite::GetThemeColor(size_t nIndex) const throw()
{
    if (nIndex >= m_arrThemeColors.GetCount()) return 0;
    return m_arrThemeColors.GetAt(nIndex);
}

HRESULT CColorPickerSite::SetThemeColors(const DWORD* pColors, size_t cColors)
{
    if (!pColors && cColors > 0) return E_INVALIDARG;
    m_arrThemeColors.RemoveAll();
    for (size_t i = 0; i < cColors; ++i)
        m_arrThemeColors.Add(pColors[i]);
    return S_OK;
}

size_t CColorPickerSite::GetRecentColorCount() const throw() { return m_arrRecentColors.GetCount(); }

DWORD CColorPickerSite::GetRecentColor(size_t nIndex) const throw()
{
    if (nIndex >= m_arrRecentColors.GetCount()) return 0;
    return m_arrRecentColors.GetAt(nIndex);
}

HRESULT CColorPickerSite::AddRecentColor(DWORD dwColor)
{
    m_arrRecentColors.InsertAt(0, dwColor);
    while (m_arrRecentColors.GetCount() > m_nMaxRecentColors)
        m_arrRecentColors.RemoveAt(m_arrRecentColors.GetCount() - 1);
    return S_OK;
}

DWORD CColorPickerSite::GetAutomaticColor() const throw() { return m_dwAutomaticColor; }
void CColorPickerSite::SetAutomaticColor(DWORD dwColor) throw() { m_dwAutomaticColor = dwColor; }

UINT CColorPickerSite::GetCommandId() const throw() { return m_nCmdId; }

// ============================================================================
// CGenericSite implementation
// ============================================================================

CGenericSite::CGenericSite()
    : m_nCmdId(0)
    , m_fEnabled(true)
    , m_fVisible(true)
    , m_fPressed(false)
    , m_dwUserData(0)
{
}

CGenericSite::~CGenericSite() {}

HRESULT CGenericSite::Initialize(UINT nCmdId, LPCWSTR pszLabel)
{
    m_nCmdId = nCmdId;
    m_strLabel = pszLabel ? pszLabel : L"";
    return S_OK;
}

HRESULT CGenericSite::Shutdown() { return S_OK; }

UINT CGenericSite::GetCommandId() const throw() { return m_nCmdId; }
ATL::CString CGenericSite::GetLabel() const { return m_strLabel; }

bool CGenericSite::IsEnabled() const throw() { return m_fEnabled; }
HRESULT CGenericSite::SetEnabled(bool fEnabled) { m_fEnabled = fEnabled; return S_OK; }

bool CGenericSite::IsVisible() const throw() { return m_fVisible; }
HRESULT CGenericSite::SetVisible(bool fVisible) { m_fVisible = fVisible; return S_OK; }

bool CGenericSite::IsPressed() const throw() { return m_fPressed; }
HRESULT CGenericSite::SetPressed(bool fPressed) { m_fPressed = fPressed; return S_OK; }

DWORD_PTR CGenericSite::GetUserData() const throw() { return m_dwUserData; }
void CGenericSite::SetUserData(DWORD_PTR dwData) throw() { m_dwUserData = dwData; }

// ============================================================================
// CGallerySite implementation
// ============================================================================

CGallerySite::CGallerySite()
    : m_nCmdId(0)
    , m_uItemWidth(80)
    , m_uItemHeight(80)
    , m_uColumns(4)
    , m_fDropdown(false)
{
}

CGallerySite::~CGallerySite() {}

HRESULT CGallerySite::Initialize(UINT nCmdId, UINT uItemWidth, UINT uItemHeight)
{
    m_nCmdId = nCmdId;
    m_uItemWidth = uItemWidth;
    m_uItemHeight = uItemHeight;
    return S_OK;
}

HRESULT CGallerySite::Shutdown() { return S_OK; }

RibbonList* CGallerySite::GetItemList() { return &m_list; }
const RibbonList* CGallerySite::GetItemList() const { return &m_list; }

UINT CGallerySite::GetItemWidth() const throw() { return m_uItemWidth; }
UINT CGallerySite::GetItemHeight() const throw() { return m_uItemHeight; }
void CGallerySite::SetItemDimensions(UINT uWidth, UINT uHeight) throw() { m_uItemWidth = uWidth; m_uItemHeight = uHeight; }

UINT CGallerySite::GetColumns() const throw() { return m_uColumns; }
void CGallerySite::SetColumns(UINT uColumns) throw() { m_uColumns = uColumns; }

HRESULT CGallerySite::SetSelectedItem(UINT nItemId) { return m_list.SetSelectedItemId(nItemId); }
UINT CGallerySite::GetSelectedItem() const throw() { return m_list.GetSelectedItemId(); }

bool CGallerySite::IsDropdown() const throw() { return m_fDropdown; }
void CGallerySite::SetDropdown(bool fDropdown) throw() { m_fDropdown = fDropdown; }

UINT CGallerySite::GetCommandId() const throw() { return m_nCmdId; }

// ============================================================================
// ToggleSite implementation
// ============================================================================

ToggleSite::ToggleSite()
    : m_nCmdId(0)
    , m_fChecked(false)
{
}

ToggleSite::~ToggleSite() {}

HRESULT ToggleSite::Initialize(UINT nCmdId, LPCWSTR pszLabelOn, LPCWSTR pszLabelOff)
{
    m_nCmdId = nCmdId;
    m_strLabelOn = pszLabelOn ? pszLabelOn : L"";
    m_strLabelOff = pszLabelOff ? pszLabelOff : L"";
    return S_OK;
}

HRESULT ToggleSite::Shutdown() { return S_OK; }

bool ToggleSite::IsChecked() const throw() { return m_fChecked; }

HRESULT ToggleSite::SetChecked(bool fChecked) { m_fChecked = fChecked; return S_OK; }

HRESULT ToggleSite::Toggle() { m_fChecked = !m_fChecked; return S_OK; }

ATL::CString ToggleSite::GetLabelOn() const { return m_strLabelOn; }
ATL::CString ToggleSite::GetLabelOff() const { return m_strLabelOff; }

HRESULT ToggleSite::SetLabels(LPCWSTR pszLabelOn, LPCWSTR pszLabelOff)
{
    m_strLabelOn = pszLabelOn ? pszLabelOn : L"";
    m_strLabelOff = pszLabelOff ? pszLabelOff : L"";
    return S_OK;
}

UINT ToggleSite::GetCommandId() const throw() { return m_nCmdId; }

// ============================================================================
// RibbonSiteRegistry implementation
// ============================================================================

RibbonSiteRegistry& RibbonSiteRegistry::Instance() throw()
{
    static RibbonSiteRegistry s_instance;
    return s_instance;
}

RibbonSiteRegistry::RibbonSiteRegistry() {}
RibbonSiteRegistry::~RibbonSiteRegistry() { Clear(); }

HRESULT RibbonSiteRegistry::RegisterSite(HWND hWnd, RibbonApp* pRibbon)
{
    if (!hWnd || !pRibbon)
        return E_INVALIDARG;

    // Replace existing entry if present
    m_sites[hWnd] = pRibbon;
    return S_OK;
}

HRESULT RibbonSiteRegistry::UnregisterSite(HWND hWnd)
{
    if (!hWnd)
        return E_INVALIDARG;

    auto it = m_sites.find(hWnd);
    if (it == m_sites.end())
        return S_FALSE;

    m_sites.erase(it);
    return S_OK;
}

RibbonApp* RibbonSiteRegistry::GetSite(HWND hWnd) const throw()
{
    if (!hWnd)
        return nullptr;

    auto it = m_sites.find(hWnd);
    if (it == m_sites.end())
        return nullptr;

    return it->second;
}

size_t RibbonSiteRegistry::GetSiteCount() const throw()
{
    return m_sites.size();
}

void RibbonSiteRegistry::Clear()
{
    m_sites.clear();
}

} // namespace SundanceUI
