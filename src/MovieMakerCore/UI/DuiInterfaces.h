/*
 * DuiInterfaces.h
 *
 * Forward declarations and base interface stubs for DirectUI (DUI)
 * framework types used throughout the Sundance UI behavior layer.
 * DirectUI is a lightweight windowless UI framework used by Windows Live
 * applications. These interfaces are provided by directui.dll / uxcore.dll
 * and consumed by the behavior classes in MovieMakerCore.dll.
 *
 * RTTI: DirectUI framework types (loaded from directui.dll)
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef DUI_INTERFACES_H
#define DUI_INTERFACES_H

#include "../pch.h"
#include "SundanceResourceIds.h"

// ============================================================================
// DirectUI namespace declarations
// ============================================================================
// DirectUI is a windowless UI framework shipped with Windows Live Essentials.
// It provides element trees, behaviors, data sources, virtual layouts,
// and XAML-like markup (.duxt resource files) for rich UI composition.
//
// These are forward declarations and interface stubs matching the exported
// RTTI signatures from the original directui.dll binary.
//

// ============================================================================
// DUI element types
// ============================================================================
struct IDuiElement;
struct IDuiElementBehavior;
struct IDuiDataSource;
struct IDuiVirtualLayout;
struct IDuiVirtualLayoutMode;
struct IDuiDataCtx;
struct IDuiValue;

// ============================================================================
// IDuiBehavior -- base behavior interface
// ============================================================================
// Every DirectUI behavior implements this interface. Behaviors are attached
// to DUI elements and receive notifications for layout, paint, input,
// and property changes.
//
// RTTI: ?AUIDuiBehavior@@
//
struct __declspec(uuid("e773fbf7-37c0-48f1-9507-527a0ee82c48")) IDuiBehavior : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE OnElementAttached(IDuiElement* pElement) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnElementDetached(IDuiElement* pElement) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnPaint(HDC hdc, const RECT* prcBounds) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnBehaviorChanged() = 0;
};

// ============================================================================
// IDuiBehaviorImpl -- ATL-style behavior implementation base
// ============================================================================
// Default implementation of IDuiBehavior that provides empty stubs for all
// methods. Behavior classes derive from this to get sensible defaults and
// override only the methods they need.
//
// RTTI: ?AVIDuiBehaviorImpl@@
//
struct IDuiBehaviorImpl : public IDuiBehavior
{
    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) throw()
    {
        if (!ppv) return E_POINTER;
        *ppv = NULL;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IDuiBehavior)))
        {
            *ppv = static_cast<IDuiBehaviorImpl*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() throw() { return 1; }
    STDMETHOD_(ULONG, Release)() throw() { return 0; }

    // IDuiBehavior
    STDMETHOD(OnElementAttached)(IDuiElement* /*pElement*/) throw() { return S_OK; }
    STDMETHOD(OnElementDetached)(IDuiElement* /*pElement*/) throw() { return S_OK; }
    STDMETHOD(OnPaint)(HDC /*hdc*/, const RECT* /*prcBounds*/) throw() { return S_OK; }
    STDMETHOD(OnMessage)(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL* pbHandled) throw()
    {
        if (pbHandled) *pbHandled = FALSE;
        return S_OK;
    }
    STDMETHOD(OnBehaviorChanged)() throw() { return S_OK; }
};

// ============================================================================
// IDuiLayerBehavior -- layer behavior for compositing
// ============================================================================
// A specialized behavior that manages a compositing layer for rendering.
// Used by HomerHeavyLayerBehavior and StandardLayerBehavior.
//
// RTTI: ?AVIDuiLayerBehavior@@
//
struct __declspec(uuid("4e90fa69-7bb9-4d5d-a7cd-9bf180dc9731")) IDuiLayerBehavior : public IDuiBehavior
{
    virtual HRESULT STDMETHODCALLTYPE BeginLayer(HDC hdc, const RECT* prcBounds) = 0;
    virtual HRESULT STDMETHODCALLTYPE EndLayer(HDC hdc, const RECT* prcBounds) = 0;
    virtual HRESULT STDMETHODCALLTYPE InvalidateLayer() = 0;
};

// ============================================================================
// IDuiLayerBehaviorImpl -- default layer behavior implementation
// ============================================================================
// RTTI: ?AV?$CComObjectNoLock@VDuiLayerBehaviorImpl@@@ATL@@
//
struct IDuiLayerBehaviorImpl : public IDuiLayerBehavior
{
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) throw()
    {
        if (!ppv) return E_POINTER;
        *ppv = NULL;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IDuiLayerBehavior)))
        {
            *ppv = static_cast<IDuiLayerBehaviorImpl*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() throw() { return 1; }
    STDMETHOD_(ULONG, Release)() throw() { return 0; }

    STDMETHOD(OnElementAttached)(IDuiElement*) throw() { return S_OK; }
    STDMETHOD(OnElementDetached)(IDuiElement*) throw() { return S_OK; }
    STDMETHOD(OnPaint)(HDC, const RECT*) throw() { return S_OK; }
    STDMETHOD(OnMessage)(UINT, WPARAM, LPARAM, BOOL* pbHandled) throw()
    {
        if (pbHandled) *pbHandled = FALSE;
        return S_OK;
    }
    STDMETHOD(OnBehaviorChanged)() throw() { return S_OK; }
    STDMETHOD(BeginLayer)(HDC, const RECT*) throw() { return S_OK; }
    STDMETHOD(EndLayer)(HDC, const RECT*) throw() { return S_OK; }
    STDMETHOD(InvalidateLayer)() throw() { return S_OK; }
};

// ============================================================================
// IDuiDataSource -- data source interface
// ============================================================================
// Provides data to DirectUI virtual layout controls (VirtualListView, etc.).
//
// RTTI: ?AVIDuiDataSource@@
//
struct __declspec(uuid("7de96de3-dc9d-4c44-9028-4687a4847355")) IDuiDataSource : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetCount(UINT* pCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemData(UINT nIndex, IDuiDataCtx** ppDataCtx) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnDataChanged() = 0;
};

// ============================================================================
// IDuiDataSourceImpl -- default data source implementation
// ============================================================================
// RTTI: ?AV?$CComObjectNoLock@VDuiDataSourceImpl@@@ATL@@
//
struct IDuiDataSourceImpl : public IDuiDataSource
{
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) throw()
    {
        if (!ppv) return E_POINTER;
        *ppv = NULL;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IDuiDataSource)))
        {
            *ppv = static_cast<IDuiDataSourceImpl*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() throw() { return 1; }
    STDMETHOD_(ULONG, Release)() throw() { return 0; }

    STDMETHOD(GetCount)(UINT* pCount) throw()
    {
        if (pCount) *pCount = 0;
        return S_OK;
    }

    STDMETHOD(GetItemData)(UINT /*nIndex*/, IDuiDataCtx** ppDataCtx) throw()
    {
        if (ppDataCtx) *ppDataCtx = NULL;
        return S_OK;
    }

    STDMETHOD(OnDataChanged)() throw() { return S_OK; }
};

// ============================================================================
// IDuiVirtualLayoutMode -- layout mode for virtual lists
// ============================================================================
// RTTI: ?AVIDuiVirtualLayoutMode@@
//
struct __declspec(uuid("d14877c2-103d-44b5-9e1a-a22e74f39b7f")) IDuiVirtualLayoutMode : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetItemSize(UINT nIndex, SIZE* pSize) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItemCount(UINT* pCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetViewportSize(SIZE sizeViewport) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetViewportSize(SIZE* pSizeViewport) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetScrollRange(SIZE* pSizeRange) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetScrollOffset(POINT ptOffset) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetScrollOffset(POINT* pptOffset) = 0;
    virtual HRESULT STDMETHODCALLTYPE HitTest(POINT pt, UINT* pIndex) = 0;
};

// ============================================================================
// IDuiVirtualLayoutModeImpl -- default layout mode implementation
// ============================================================================
// RTTI: ?AV?$CComObjectNoLock@VDuiVirtualLayoutModeImpl@@@ATL@@
//
struct IDuiVirtualLayoutModeImpl : public IDuiVirtualLayoutMode
{
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) throw()
    {
        if (!ppv) return E_POINTER;
        *ppv = NULL;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IDuiVirtualLayoutMode)))
        {
            *ppv = static_cast<IDuiVirtualLayoutModeImpl*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() throw() { return 1; }
    STDMETHOD_(ULONG, Release)() throw() { return 0; }

    STDMETHOD(GetItemSize)(UINT /*nIndex*/, SIZE* pSize) throw()
    {
        if (pSize) { pSize->cx = 0; pSize->cy = 0; }
        return S_OK;
    }

    STDMETHOD(GetItemCount)(UINT* pCount) throw()
    {
        if (pCount) *pCount = 0;
        return S_OK;
    }

    STDMETHOD(SetViewportSize)(SIZE /*sizeViewport*/) throw() { return S_OK; }
    STDMETHOD(GetViewportSize)(SIZE* pSizeViewport) throw()
    {
        if (pSizeViewport) { pSizeViewport->cx = 0; pSizeViewport->cy = 0; }
        return S_OK;
    }

    STDMETHOD(GetScrollRange)(SIZE* pSizeRange) throw()
    {
        if (pSizeRange) { pSizeRange->cx = 0; pSizeRange->cy = 0; }
        return S_OK;
    }

    STDMETHOD(SetScrollOffset)(POINT /*ptOffset*/) throw() { return S_OK; }
    STDMETHOD(GetScrollOffset)(POINT* pptOffset) throw()
    {
        if (pptOffset) { pptOffset->x = 0; pptOffset->y = 0; }
        return S_OK;
    }

    STDMETHOD(HitTest)(POINT /*pt*/, UINT* pIndex) throw()
    {
        if (pIndex) *pIndex = static_cast<UINT>(-1);
        return S_OK;
    }
};

// ============================================================================
// IDuiDataCtx -- data context for property binding
// ============================================================================
// RTTI: ?AVIDuiDataCtx@@
//
struct IDuiDataCtx : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetProperty(LPCWSTR pszName, VARIANT* pvarValue) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetProperty(LPCWSTR pszName, const VARIANT* varValue) = 0;
};

// ============================================================================
// IDuiElement -- element interface
// ============================================================================
// Forward declaration only. The full IDuiElement interface is provided by
// directui.dll and contains methods for layout, rendering, property access,
// child management, and behavior attachment.
//
// RTTI: ?AUIDuiElement@@
//
struct IDuiElement : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetBehavior(IDuiBehavior** ppBehavior) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBehavior(IDuiBehavior* pBehavior) = 0;
    virtual HRESULT STDMETHODCALLTYPE InvalidateRect(const RECT* prc) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBounds(RECT* prcBounds) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBounds(const RECT* rcBounds) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetChildElementsCount(UINT* pCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetParentElement(IDuiElement** ppParent) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFirstChildElement(IDuiElement** ppChild) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetNextSiblingElement(IDuiElement** ppSibling) = 0;
};

// ============================================================================
// Helper: Create a CComObjectNoLock for a DirectUI behavior
// ============================================================================
// Template helper that creates an ATL CComObjectNoLock wrapping the given
// behavior class. This is the standard way behaviors are instantiated in
// the Sundance framework -- they are not reference-counted by ATL's
// aggregation machinery (NoLock variant) because DirectUI manages their
// lifetime directly.
//
template <typename TBehavior>
static HRESULT CreateDuiBehavior(TBehavior** ppBehavior)
{
    if (!ppBehavior)
        return E_POINTER;

    *ppBehavior = NULL;

    CComObjectNoLock<TBehavior>* pObj = NULL;
    HRESULT hr = pObj->CreateInstance(&pObj);
    if (FAILED(hr))
        return hr;

    pObj->AddRef();
    *ppBehavior = pObj;
    return S_OK;
}

#endif // DUI_INTERFACES_H
