/*
 * TimelineDataSources.cpp
 *
 * Implementation of timeline data source and virtual layout classes.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "TimelineDataSources.h"
#include "../SundanceApp/SundanceAppMain.h"

namespace Sundance
{

// ============================================================================
// TimelineDataSource
// ============================================================================
TimelineDataSource::TimelineDataSource()
    : m_pAppMain(NULL)
    , m_nTrackIndex(-1)
{
}

TimelineDataSource::~TimelineDataSource()
{
    m_pAppMain = NULL;
}

HRESULT TimelineDataSource::Initialize(SundanceAppMain* pAppMain, int nTrackIndex)
{
    if (!pAppMain)
        return E_POINTER;

    m_pAppMain = pAppMain;
    m_nTrackIndex = nTrackIndex;

    RebuildItemList();
    return S_OK;
}

HRESULT TimelineDataSource::GetCount(UINT* pCount)
{
    if (!pCount) return E_POINTER;
    *pCount = static_cast<UINT>(m_items.size());
    return S_OK;
}

HRESULT TimelineDataSource::GetItemData(UINT nIndex, IDuiDataCtx** ppDataCtx)
{
    if (!ppDataCtx) return E_POINTER;
    *ppDataCtx = NULL;

    if (nIndex >= m_items.size())
        return E_BOUNDS;

    // In the full implementation, this creates a CComObjectNoLock<IDuiDataCtx>
    // adapter that exposes the item properties for DirectUI binding.
    // For now, return a null data context.
    return S_OK;
}

HRESULT TimelineDataSource::OnDataChanged()
{
    RebuildItemList();
    return S_OK;
}

void TimelineDataSource::RefreshData()
{
    RebuildItemList();
}

void TimelineDataSource::SetTrackFilter(int nTrackIndex)
{
    m_nTrackIndex = nTrackIndex;
    RebuildItemList();
}

void TimelineDataSource::RebuildItemList()
{
    m_items.clear();

    if (!m_pAppMain || !m_pAppMain->GetProject())
        return;

    // Query items from the project model
    // The actual implementation iterates the project's timeline items
    // and populates the m_items vector with item properties.
}

// ============================================================================
// TimelineTemplateSource
// ============================================================================
TimelineTemplateSource::TimelineTemplateSource()
    : m_pAppMain(NULL)
{
}

TimelineTemplateSource::~TimelineTemplateSource()
{
    m_pAppMain = NULL;
}

HRESULT TimelineTemplateSource::Initialize(SundanceAppMain* pAppMain)
{
    if (!pAppMain)
        return E_POINTER;

    m_pAppMain = pAppMain;
    LoadTemplates();
    return S_OK;
}

HRESULT TimelineTemplateSource::GetCount(UINT* pCount)
{
    if (!pCount) return E_POINTER;
    *pCount = static_cast<UINT>(m_templates.size());
    return S_OK;
}

HRESULT TimelineTemplateSource::GetItemData(UINT nIndex, IDuiDataCtx** ppDataCtx)
{
    if (!ppDataCtx) return E_POINTER;
    *ppDataCtx = NULL;

    if (nIndex >= m_templates.size())
        return E_BOUNDS;

    return S_OK;
}

HRESULT TimelineTemplateSource::OnDataChanged()
{
    LoadTemplates();
    return S_OK;
}

HRESULT TimelineTemplateSource::GetTemplateName(UINT nIndex, LPWSTR pszName, UINT cchName)
{
    if (!pszName || cchName == 0)
        return E_INVALIDARG;

    if (nIndex >= m_templates.size())
        return E_BOUNDS;

    wcscpy_s(pszName, cchName, m_templates[nIndex].strName);
    return S_OK;
}

HRESULT TimelineTemplateSource::GetTemplateId(UINT nIndex, DWORD* pdwTemplateId)
{
    if (!pdwTemplateId) return E_POINTER;

    if (nIndex >= m_templates.size())
        return E_BOUNDS;

    *pdwTemplateId = m_templates[nIndex].dwTemplateId;
    return S_OK;
}

HRESULT TimelineTemplateSource::ApplyTemplate(DWORD dwTemplateId, DWORD dwItemId)
{
    UNREFERENCED_PARAMETER(dwTemplateId);
    UNREFERENCED_PARAMETER(dwItemId);

    // Apply the specified template to the given item
    return S_OK;
}

void TimelineTemplateSource::LoadTemplates()
{
    m_templates.clear();

    // Load templates from the project's template table
    // Default templates are built-in themes shipped with Movie Maker
    TemplateEntry defaultTemplate;
    defaultTemplate.dwTemplateId = 0;
    defaultTemplate.strName = L"Default";
    defaultTemplate.strDescription = L"Default movie theme";
    defaultTemplate.bIsDefault = true;
    m_templates.push_back(defaultTemplate);
}

// ============================================================================
// TimelineLayoutMode
// ============================================================================
TimelineLayoutMode::TimelineLayoutMode()
    : m_pProvider(NULL)
    , m_nItemWidth(200)
    , m_nItemHeight(80)
    , m_nItemSpacing(4)
{
    ZeroMemory(&m_sizeViewport, sizeof(m_sizeViewport));
    ZeroMemory(&m_sizeScrollRange, sizeof(m_sizeScrollRange));
    ZeroMemory(&m_ptScrollOffset, sizeof(m_ptScrollOffset));
}

TimelineLayoutMode::~TimelineLayoutMode()
{
    m_pProvider = NULL;
}

HRESULT TimelineLayoutMode::GetItemSize(UINT nIndex, SIZE* pSize)
{
    if (!pSize) return E_POINTER;

    pSize->cx = m_nItemWidth;
    pSize->cy = m_nItemHeight;

    UNREFERENCED_PARAMETER(nIndex);
    return S_OK;
}

HRESULT TimelineLayoutMode::GetItemCount(UINT* pCount)
{
    if (!pCount) return E_POINTER;

    if (m_pProvider)
        return m_pProvider->GetCount(pCount);

    *pCount = 0;
    return S_OK;
}

HRESULT TimelineLayoutMode::SetViewportSize(SIZE sizeViewport)
{
    m_sizeViewport = sizeViewport;
    RecalculateScrollRange();
    return S_OK;
}

HRESULT TimelineLayoutMode::GetViewportSize(SIZE* pSizeViewport)
{
    if (!pSizeViewport) return E_POINTER;
    *pSizeViewport = m_sizeViewport;
    return S_OK;
}

HRESULT TimelineLayoutMode::GetScrollRange(SIZE* pSizeRange)
{
    if (!pSizeRange) return E_POINTER;
    *pSizeRange = m_sizeScrollRange;
    return S_OK;
}

HRESULT TimelineLayoutMode::SetScrollOffset(POINT ptOffset)
{
    m_ptScrollOffset = ptOffset;

    if (m_ptScrollOffset.x < 0) m_ptScrollOffset.x = 0;
    if (m_ptScrollOffset.y < 0) m_ptScrollOffset.y = 0;

    return S_OK;
}

HRESULT TimelineLayoutMode::GetScrollOffset(POINT* pptOffset)
{
    if (!pptOffset) return E_POINTER;
    *pptOffset = m_ptScrollOffset;
    return S_OK;
}

HRESULT TimelineLayoutMode::HitTest(POINT pt, UINT* pIndex)
{
    if (!pIndex) return E_POINTER;
    *pIndex = static_cast<UINT>(-1);

    int nTotalWidth = m_nItemWidth + m_nItemSpacing;
    if (nTotalWidth <= 0) return S_OK;

    int nAdjustedX = pt.x + m_ptScrollOffset.x;
    int nColumn = nAdjustedX / nTotalWidth;

    if (nColumn >= 0)
    {
        UINT nCount = 0;
        GetItemCount(&nCount);
        if (static_cast<UINT>(nColumn) < nCount)
        {
            *pIndex = static_cast<UINT>(nColumn);
        }
    }

    return S_OK;
}

void TimelineLayoutMode::SetItemWidth(int nWidth)
{
    m_nItemWidth = nWidth;
    RecalculateScrollRange();
}

void TimelineLayoutMode::SetItemHeight(int nHeight)
{
    m_nItemHeight = nHeight;
}

void TimelineLayoutMode::SetItemSpacing(int nSpacing)
{
    m_nItemSpacing = nSpacing;
    RecalculateScrollRange();
}

void TimelineLayoutMode::SetItemProvider(TimelineDataSource* pProvider)
{
    m_pProvider = pProvider;
    RecalculateScrollRange();
}

void TimelineLayoutMode::ZoomByFactor(int nZoomFactor)
{
    m_nItemWidth = max(40, m_nItemWidth * nZoomFactor / 100);
    RecalculateScrollRange();
}

void TimelineLayoutMode::EnsureItemVisible(UINT nIndex)
{
    int nTotalWidth = m_nItemWidth + m_nItemSpacing;
    int nItemLeft = nIndex * nTotalWidth;
    int nItemRight = nItemLeft + m_nItemWidth;

    if (nItemLeft < m_ptScrollOffset.x)
    {
        m_ptScrollOffset.x = nItemLeft;
    }
    else if (nItemRight > m_ptScrollOffset.x + m_sizeViewport.cx)
    {
        m_ptScrollOffset.x = nItemRight - m_sizeViewport.cx;
    }
}

void TimelineLayoutMode::RecalculateScrollRange()
{
    UINT nCount = 0;
    GetItemCount(&nCount);

    int nTotalWidth = (m_nItemWidth + m_nItemSpacing) * static_cast<int>(nCount);
    m_sizeScrollRange.cx = max(0, nTotalWidth - m_sizeViewport.cx);
    m_sizeScrollRange.cy = 0;
}

// ============================================================================
// TimelineItemHandler
// ============================================================================
TimelineItemHandler::TimelineItemHandler()
    : m_pAppMain(NULL)
{
}

TimelineItemHandler::~TimelineItemHandler()
{
    m_pAppMain = NULL;
}

HRESULT TimelineItemHandler::Initialize(SundanceAppMain* pAppMain)
{
    if (!pAppMain)
        return E_POINTER;
    m_pAppMain = pAppMain;
    return S_OK;
}

HRESULT TimelineItemHandler::CreateItem(LPCWSTR pszFilePath, DWORD* pdwItemId)
{
    if (!pszFilePath || !pdwItemId)
        return E_INVALIDARG;

    *pdwItemId = 0;

    if (!m_pAppMain || !m_pAppMain->IsProjectOpen())
        return E_UNEXPECTED;

    HRESULT hr = m_pAppMain->AddMediaToTimeline(pszFilePath, TimelineTrack_Video);
    if (SUCCEEDED(hr))
    {
        // The item ID would be returned by the project model
        *pdwItemId = 1;
    }

    return hr;
}

HRESULT TimelineItemHandler::DeleteItem(DWORD dwItemId)
{
    if (!m_pAppMain || !m_pAppMain->IsProjectOpen())
        return E_UNEXPECTED;

    return m_pAppMain->RemoveItemFromTimeline(dwItemId, TimelineTrack_Video);
}

HRESULT TimelineItemHandler::MoveItem(DWORD dwItemId, int nNewPosition)
{
    if (!m_pAppMain || !m_pAppMain->IsProjectOpen())
        return E_UNEXPECTED;

    return m_pAppMain->MoveItemOnTimeline(
        dwItemId, TimelineTrack_Video, static_cast<DWORD>(nNewPosition));
}

HRESULT TimelineItemHandler::DuplicateItem(DWORD dwItemId, DWORD* pdwNewItemId)
{
    if (!pdwNewItemId)
        return E_POINTER;

    *pdwNewItemId = 0;

    if (!m_pAppMain)
        return E_FAIL;

    SundanceAppMain* pApp = m_pAppMain;
    if (!pApp)
        return E_FAIL;

    StoryboardManager::MovieProject* pProject = pApp->GetProject();
    if (!pProject)
        return E_FAIL;

    int nIndex = pProject->FindMediaItemById(dwItemId);
    if (nIndex < 0)
        return S_FALSE;

    const StoryboardManager::ProjectMediaItem* pSourceItem = pProject->GetMediaItem(nIndex);
    if (!pSourceItem)
        return E_FAIL;

    StoryboardManager::ProjectMediaItem newItem = *pSourceItem;
    DWORD dwNewId = static_cast<DWORD>(pProject->AddMediaItem(newItem));

    *pdwNewItemId = dwNewId;
    return S_OK;
}

HRESULT TimelineItemHandler::GetItemProperty(DWORD dwItemId, LPCWSTR pszProperty, VARIANT* pvarValue)
{
    if (!pszProperty || !pvarValue)
        return E_POINTER;

    if (!m_pAppMain)
        return E_FAIL;

    StoryboardManager::MovieProject* pProject = m_pAppMain->GetProject();
    if (!pProject)
        return E_FAIL;

    int nIndex = pProject->FindMediaItemById(dwItemId);
    if (nIndex < 0)
        return S_FALSE;

    const StoryboardManager::ProjectMediaItem* pItem = pProject->GetMediaItem(nIndex);
    if (!pItem)
        return E_FAIL;

    VariantInit(pvarValue);

    if (_wcsicmp(pszProperty, L"duration") == 0)
    {
        pvarValue->vt = VT_I8;
        pvarValue->llVal = pItem->GetDurationHns();
    }
    else if (_wcsicmp(pszProperty, L"path") == 0)
    {
        pvarValue->vt = VT_BSTR;
        pvarValue->bstrVal = SysAllocString(pItem->GetSourcePath());
    }
    else if (_wcsicmp(pszProperty, L"width") == 0)
    {
        pvarValue->vt = VT_I4;
        pvarValue->lVal = static_cast<LONG>(pItem->GetWidth());
    }
    else if (_wcsicmp(pszProperty, L"height") == 0)
    {
        pvarValue->vt = VT_I4;
        pvarValue->lVal = static_cast<LONG>(pItem->GetHeight());
    }
    else if (_wcsicmp(pszProperty, L"type") == 0)
    {
        pvarValue->vt = VT_I4;
        pvarValue->lVal = static_cast<LONG>(pItem->GetMediaType());
    }
    else if (_wcsicmp(pszProperty, L"frameRate") == 0)
    {
        pvarValue->vt = VT_I4;
        pvarValue->lVal = static_cast<LONG>(pItem->GetFrameRate());
    }
    else if (_wcsicmp(pszProperty, L"rating") == 0)
    {
        pvarValue->vt = VT_I4;
        pvarValue->lVal = static_cast<LONG>(pItem->GetRating());
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT TimelineItemHandler::SetItemProperty(DWORD dwItemId, LPCWSTR pszProperty, const VARIANT* varValue)
{
    if (!pszProperty || !varValue)
        return E_POINTER;

    if (!m_pAppMain)
        return E_FAIL;

    StoryboardManager::MovieProject* pProject = m_pAppMain->GetProject();
    if (!pProject)
        return E_FAIL;

    int nIndex = pProject->FindMediaItemById(dwItemId);
    if (nIndex < 0)
        return S_FALSE;

    StoryboardManager::ProjectMediaItem* pItem = pProject->GetMediaItem(nIndex);
    if (!pItem)
        return E_FAIL;

    if (_wcsicmp(pszProperty, L"rating") == 0 && varValue->vt == VT_I4)
    {
        pItem->SetRating(static_cast<UINT>(varValue->lVal));
    }
    else if (_wcsicmp(pszProperty, L"tags") == 0 && varValue->vt == VT_BSTR)
    {
        pItem->SetTags(varValue->bstrVal);
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT TimelineItemHandler::DeleteSelectedItems(const std::vector<DWORD>& itemIds)
{
    HRESULT hr = S_OK;

    for (size_t i = 0; i < itemIds.size(); ++i)
    {
        HRESULT hrItem = DeleteItem(itemIds[i]);
        if (FAILED(hrItem))
            hr = hrItem;
    }

    return hr;
}

HRESULT TimelineItemHandler::MoveSelectedItems(const std::vector<DWORD>& itemIds, int nDeltaPosition)
{
    HRESULT hr = S_OK;

    for (size_t i = 0; i < itemIds.size(); ++i)
    {
        // Calculate new position based on delta
        int nNewPos = nDeltaPosition;
        HRESULT hrItem = MoveItem(itemIds[i], nNewPos);
        if (FAILED(hrItem))
            hr = hrItem;
    }

    return hr;
}

// ============================================================================
// TimelineDragDrop
// ============================================================================
TimelineDragDrop::TimelineDragDrop()
    : m_pAppMain(NULL)
    , m_bDragging(false)
    , m_dwDraggedItemId(0)
    , m_uTimelineItemFormat(0)
    , m_uMediaFileFormat(0)
{
    ZeroMemory(&m_ptDragStart, sizeof(m_ptDragStart));
}

TimelineDragDrop::~TimelineDragDrop()
{
    m_pAppMain = NULL;
}

HRESULT TimelineDragDrop::Initialize(SundanceAppMain* pAppMain)
{
    if (!pAppMain)
        return E_POINTER;

    m_pAppMain = pAppMain;
    RegisterFormats();
    return S_OK;
}

HRESULT TimelineDragDrop::BeginInternalDrag(DWORD dwItemId, POINT ptStart)
{
    m_bDragging = true;
    m_dwDraggedItemId = dwItemId;
    m_ptDragStart = ptStart;
    return S_OK;
}

HRESULT TimelineDragDrop::BeginExternalDrag(int cItems, LPCWSTR* ppszFiles, POINT ptStart)
{
    if (cItems <= 0 || !ppszFiles)
        return E_INVALIDARG;

    m_bDragging = true;
    m_ptDragStart = ptStart;
    return S_OK;
}

HRESULT TimelineDragDrop::UpdateDrag(POINT ptCurrent)
{
    UNREFERENCED_PARAMETER(ptCurrent);

    if (!m_bDragging)
        return E_UNEXPECTED;

    return S_OK;
}

HRESULT TimelineDragDrop::EndDrag(POINT ptDrop)
{
    if (!m_bDragging)
        return E_UNEXPECTED;

    m_bDragging = false;
    return S_OK;
}

HRESULT TimelineDragDrop::CancelDrag()
{
    m_bDragging = false;
    m_dwDraggedItemId = 0;
    return S_OK;
}

HRESULT TimelineDragDrop::HandleDrop(IDataObject* pDataObject, DWORD dwEffect, POINT ptDrop)
{
    if (!pDataObject)
        return E_POINTER;

    UNREFERENCED_PARAMETER(dwEffect);
    UNREFERENCED_PARAMETER(ptDrop);

    return S_OK;
}

HRESULT TimelineDragDrop::CanDrop(IDataObject* pDataObject, DWORD* pdwEffect)
{
    if (!pDataObject || !pdwEffect)
        return E_POINTER;

    *pdwEffect = DROPEFFECT_NONE;

    FORMATETC formatEtc = { 0 };
    formatEtc.cfFormat = static_cast<CLIPFORMAT>(GetMediaFileClipFormat());
    formatEtc.tymed = TYMED_HGLOBAL;

    if (SUCCEEDED(pDataObject->QueryGetData(&formatEtc)))
    {
        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }

    formatEtc.cfFormat = static_cast<CLIPFORMAT>(GetTimelineItemClipFormat());
    if (SUCCEEDED(pDataObject->QueryGetData(&formatEtc)))
    {
        *pdwEffect = DROPEFFECT_MOVE;
        return S_OK;
    }

    return S_OK;
}

bool TimelineDragDrop::IsDragging() const throw()
{
    return m_bDragging;
}

DWORD TimelineDragDrop::GetDraggedItemId() const throw()
{
    return m_dwDraggedItemId;
}

UINT TimelineDragDrop::GetTimelineItemClipFormat()
{
    static UINT uFormat = 0;
    if (uFormat == 0)
        uFormat = RegisterClipboardFormat(L"SundanceTimelineItem");
    return uFormat;
}

UINT TimelineDragDrop::GetMediaFileClipFormat()
{
    static UINT uFormat = 0;
    if (uFormat == 0)
        uFormat = RegisterClipboardFormat(L"SundanceMediaFile");
    return uFormat;
}

void TimelineDragDrop::RegisterFormats()
{
    GetTimelineItemClipFormat();
    GetMediaFileClipFormat();
}

// ============================================================================
// TimelineExtentUIObject
// ============================================================================
TimelineExtentUIObject::TimelineExtentUIObject()
    : m_dwExtentId(0)
    , m_nTranscodeState(0)
    , m_bThumbnailValid(false)
    , m_hThumbnailBmp(NULL)
    , m_bHighlighted(false)
{
    ZeroMemory(&m_rcBounds, sizeof(m_rcBounds));
}

TimelineExtentUIObject::~TimelineExtentUIObject()
{
    if (m_hThumbnailBmp)
    {
        DeleteObject(m_hThumbnailBmp);
        m_hThumbnailBmp = NULL;
    }
}

void TimelineExtentUIObject::SetExtentId(DWORD dwExtentId) throw() { m_dwExtentId = dwExtentId; }
DWORD TimelineExtentUIObject::GetExtentId() const throw() { return m_dwExtentId; }
void TimelineExtentUIObject::SetBounds(const RECT& rcBounds) { m_rcBounds = rcBounds; }
void TimelineExtentUIObject::GetBounds(RECT* prcBounds) const { if (prcBounds) *prcBounds = m_rcBounds; }
void TimelineExtentUIObject::SetTranscodeState(int nState) throw() { m_nTranscodeState = nState; }
int  TimelineExtentUIObject::GetTranscodeState() const throw() { return m_nTranscodeState; }
void TimelineExtentUIObject::SetThumbnailValid(bool bValid) throw() { m_bThumbnailValid = bValid; }
bool TimelineExtentUIObject::IsThumbnailValid() const throw() { return m_bThumbnailValid; }

void TimelineExtentUIObject::SetThumbnailBitmap(HBITMAP hBmp) throw()
{
    if (m_hThumbnailBmp)
        DeleteObject(m_hThumbnailBmp);
    m_hThumbnailBmp = hBmp;
}

HBITMAP TimelineExtentUIObject::GetThumbnailBitmap() const throw()
{
    return m_hThumbnailBmp;
}

void TimelineExtentUIObject::SetHighlighted(bool bHighlighted) throw() { m_bHighlighted = bHighlighted; }
bool TimelineExtentUIObject::IsHighlighted() const throw() { return m_bHighlighted; }

} // namespace Sundance
