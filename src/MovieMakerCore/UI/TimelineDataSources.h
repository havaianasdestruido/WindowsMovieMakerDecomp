/*
 * TimelineDataSources.h
 *
 * DirectUI data source and virtual layout classes for the timeline.
 * Provides data binding between the project model and the DirectUI
 * VirtualListView/VirtualLayout timeline controls.
 *
 * RTTI classes:
 *   ?AVTimelineDataSource@@
 *   ?AVTimelineTemplateSource@@
 *   ?AVTimelineLayoutMode@@
 *   ?AVTimelineItemHandler@@
 *   ?AVTimelineDragDrop@@
 *   ?AVTimelineExtentUIObject@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef TIMELINE_DATASOURCES_H
#define TIMELINE_DATASOURCES_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

class SundanceAppMain;

namespace Sundance
{

// ============================================================================
// TimelineDataSource
// ============================================================================
// Data source implementation for the timeline VirtualListView. Provides
// item count and per-item data contexts containing properties like
// ItemId, Duration, TrackIndex, ThumbnailPath, DisplayName, etc.
// Implements IDuiDataSourceImpl (CComObjectNoLock pattern).
//
// RTTI: ?AVTimelineDataSource@@
// ATL:  CComObjectNoLock<TimelineDataSource>
//
class ATL_NO_VTABLE TimelineDataSource :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineDataSource>,
    public IDuiDataSourceImpl
{
public:
    TimelineDataSource();
    virtual ~TimelineDataSource();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_DATA_SOURCE)
    DECLARE_NOT_AGGREGATABLE(TimelineDataSource)

    BEGIN_COM_MAP(TimelineDataSource)
        COM_INTERFACE_ENTRY(IDuiDataSource)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- Initialization --
    HRESULT Initialize(SundanceAppMain* pAppMain, int nTrackIndex);

    // -- IDuiDataSource --
    STDMETHOD(GetCount)(UINT* pCount) override;
    STDMETHOD(GetItemData)(UINT nIndex, IDuiDataCtx** ppDataCtx) override;
    STDMETHOD(OnDataChanged)() override;

    // -- Data refresh --
    void RefreshData();
    void SetTrackFilter(int nTrackIndex);

private:
    SundanceAppMain*    m_pAppMain;
    int                 m_nTrackIndex;

    struct ItemProperties
    {
        DWORD   dwItemId;
        DWORD   dwDurationMs;
        int     nTrackIndex;
        ATL::CString strDisplayName;
        ATL::CString strFilePath;
        ATL::CString strThumbnailPath;
        bool    bIsSelected;
        bool    bIsInvalid;

        ItemProperties()
            : dwItemId(0)
            , dwDurationMs(0)
            , nTrackIndex(0)
            , bIsSelected(false)
            , bIsInvalid(false)
        {
        }
    };

    std::vector<ItemProperties> m_items;

    void RebuildItemList();
};

// ============================================================================
// TimelineTemplateSource
// ============================================================================
// Template data provider for the timeline. Supplies template information
// (themes, effects, transitions) that can be applied to timeline items.
// Used by the effects/transitions picker UI.
//
// RTTI: ?AVTimelineTemplateSource@@
// ATL:  CComObjectNoLock<TimelineTemplateSource>
//
class ATL_NO_VTABLE TimelineTemplateSource :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineTemplateSource>,
    public IDuiDataSourceImpl
{
public:
    TimelineTemplateSource();
    virtual ~TimelineTemplateSource();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_TEMPLATE_SOURCE)
    DECLARE_NOT_AGGREGATABLE(TimelineTemplateSource)

    BEGIN_COM_MAP(TimelineTemplateSource)
        COM_INTERFACE_ENTRY(IDuiDataSource)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- Initialization --
    HRESULT Initialize(SundanceAppMain* pAppMain);

    // -- IDuiDataSource --
    STDMETHOD(GetCount)(UINT* pCount) override;
    STDMETHOD(GetItemData)(UINT nIndex, IDuiDataCtx** ppDataCtx) override;
    STDMETHOD(OnDataChanged)() override;

    // -- Template queries --
    HRESULT GetTemplateName(UINT nIndex, LPWSTR pszName, UINT cchName);
    HRESULT GetTemplateId(UINT nIndex, DWORD* pdwTemplateId);
    HRESULT ApplyTemplate(DWORD dwTemplateId, DWORD dwItemId);

private:
    SundanceAppMain* m_pAppMain;

    struct TemplateEntry
    {
        DWORD       dwTemplateId;
        ATL::CString strName;
        ATL::CString strDescription;
        ATL::CString strPreviewImagePath;
        bool        bIsDefault;

        TemplateEntry()
            : dwTemplateId(0)
            , bIsDefault(false)
        {
        }
    };

    std::vector<TemplateEntry> m_templates;

    void LoadTemplates();
};

// ============================================================================
// TimelineLayoutMode
// ============================================================================
// Layout mode for the timeline VirtualListView. Controls item sizing,
// scrolling, hit-testing, and viewport management. Implements
// IDuiVirtualLayoutModeImpl.
//
// RTTI: ?AVTimelineLayoutMode@@
// ATL:  CComObjectNoLock<TimelineLayoutMode>
//
class ATL_NO_VTABLE TimelineLayoutMode :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TimelineLayoutMode>,
    public IDuiVirtualLayoutModeImpl
{
public:
    TimelineLayoutMode();
    virtual ~TimelineLayoutMode();

    DECLARE_REGISTRY_RESOURCEID(IDR_TIMELINE_LAYOUT_MODE)
    DECLARE_NOT_AGGREGATABLE(TimelineLayoutMode)

    BEGIN_COM_MAP(TimelineLayoutMode)
        COM_INTERFACE_ENTRY(IDuiVirtualLayoutMode)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiVirtualLayoutMode --
    STDMETHOD(GetItemSize)(UINT nIndex, SIZE* pSize) override;
    STDMETHOD(GetItemCount)(UINT* pCount) override;
    STDMETHOD(SetViewportSize)(SIZE sizeViewport) override;
    STDMETHOD(GetViewportSize)(SIZE* pSizeViewport) override;
    STDMETHOD(GetScrollRange)(SIZE* pSizeRange) override;
    STDMETHOD(SetScrollOffset)(POINT ptOffset) override;
    STDMETHOD(GetScrollOffset)(POINT* pptOffset) override;
    STDMETHOD(HitTest)(POINT pt, UINT* pIndex) override;

    // -- Timeline-specific --
    void SetItemWidth(int nWidth);
    void SetItemHeight(int nHeight);
    void SetItemSpacing(int nSpacing);
    void SetItemProvider(TimelineDataSource* pProvider);
    void ZoomByFactor(int nZoomFactor);
    void EnsureItemVisible(UINT nIndex);

private:
    TimelineDataSource* m_pProvider;
    SIZE    m_sizeViewport;
    SIZE    m_sizeScrollRange;
    POINT   m_ptScrollOffset;
    int     m_nItemWidth;
    int     m_nItemHeight;
    int     m_nItemSpacing;

    void RecalculateScrollRange();
};

// ============================================================================
// TimelineItemHandler
// ============================================================================
// Item handler for the timeline. Manages item lifecycle (creation,
// deletion, rearrangement) and provides drag-drop data transfer.
//
// RTTI: ?AVTimelineItemHandler@@
//
class TimelineItemHandler
{
public:
    TimelineItemHandler();
    ~TimelineItemHandler();

    HRESULT Initialize(SundanceAppMain* pAppMain);

    // -- Item operations --
    HRESULT CreateItem(LPCWSTR pszFilePath, DWORD* pdwItemId);
    HRESULT DeleteItem(DWORD dwItemId);
    HRESULT MoveItem(DWORD dwItemId, int nNewPosition);
    HRESULT DuplicateItem(DWORD dwItemId, DWORD* pdwNewItemId);

    // -- Item queries --
    HRESULT GetItemProperty(DWORD dwItemId, LPCWSTR pszProperty, VARIANT* pvarValue);
    HRESULT SetItemProperty(DWORD dwItemId, LPCWSTR pszProperty, const VARIANT* varValue);

    // -- Bulk operations --
    HRESULT DeleteSelectedItems(const std::vector<DWORD>& itemIds);
    HRESULT MoveSelectedItems(const std::vector<DWORD>& itemIds, int nDeltaPosition);

private:
    SundanceAppMain* m_pAppMain;

    // Noncopyable
    TimelineItemHandler(const TimelineItemHandler&);
    TimelineItemHandler& operator=(const TimelineItemHandler&);
};

// ============================================================================
// TimelineDragDrop
// ============================================================================
// Drag-drop manager for timeline operations. Handles both internal
// (reorder) and external (from media browser) drag-drop scenarios.
// Serializes/deserializes timeline item data for IDataObject transfer.
//
// RTTI: ?AVTimelineDragDrop@@
//
class TimelineDragDrop
{
public:
    TimelineDragDrop();
    ~TimelineDragDrop();

    HRESULT Initialize(SundanceAppMain* pAppMain);

    // -- Drag operations --
    HRESULT BeginInternalDrag(DWORD dwItemId, POINT ptStart);
    HRESULT BeginExternalDrag(int cItems, LPCWSTR* ppszFiles, POINT ptStart);
    HRESULT UpdateDrag(POINT ptCurrent);
    HRESULT EndDrag(POINT ptDrop);
    HRESULT CancelDrag();

    // -- Drop operations --
    HRESULT HandleDrop(IDataObject* pDataObject, DWORD dwEffect, POINT ptDrop);
    HRESULT CanDrop(IDataObject* pDataObject, DWORD* pdwEffect);

    // -- State queries --
    bool IsDragging() const throw();
    DWORD GetDraggedItemId() const throw();

    // -- Clipboard format --
    static UINT GetTimelineItemClipFormat();
    static UINT GetMediaFileClipFormat();

private:
    SundanceAppMain*    m_pAppMain;
    bool                m_bDragging;
    DWORD               m_dwDraggedItemId;
    POINT               m_ptDragStart;
    UINT                m_uTimelineItemFormat;
    UINT                m_uMediaFileFormat;

    void RegisterFormats();
};

// ============================================================================
// TimelineExtentUIObject
// ============================================================================
// UI object representing a timeline extent (rendered segment). Wraps
// the underlying MovieExtent data and provides UI-specific properties
// like visual bounds, thumbnail state, and transcode status for
// display in the DirectUI element tree.
//
// RTTI: ?AVTimelineExtentUIObject@@
//
class TimelineExtentUIObject
{
public:
    TimelineExtentUIObject();
    ~TimelineExtentUIObject();

    // -- Extent data --
    void SetExtentId(DWORD dwExtentId) throw();
    DWORD GetExtentId() const throw();
    void SetBounds(const RECT& rcBounds);
    void GetBounds(RECT* prcBounds) const;

    // -- Transcode state --
    void SetTranscodeState(int nState) throw();
    int  GetTranscodeState() const throw();

    // -- Thumbnail --
    void SetThumbnailValid(bool bValid) throw();
    bool IsThumbnailValid() const throw();
    void SetThumbnailBitmap(HBITMAP hBmp) throw();
    HBITMAP GetThumbnailBitmap() const throw();

    // -- UI state --
    void SetHighlighted(bool bHighlighted) throw();
    bool IsHighlighted() const throw();

private:
    DWORD   m_dwExtentId;
    RECT    m_rcBounds;
    int     m_nTranscodeState;
    bool    m_bThumbnailValid;
    HBITMAP m_hThumbnailBmp;
    bool    m_bHighlighted;
};

} // namespace Sundance

#endif // TIMELINE_DATASOURCES_H
