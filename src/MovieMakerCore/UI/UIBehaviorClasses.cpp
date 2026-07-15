/*
 * UIBehaviorClasses.cpp
 *
 * Stub implementations of additional UI behavior RTTI classes for the
 * Sundance DirectUI layer. Each class provides minimal constructor,
 * destructor, and IDuiBehavior overrides returning S_OK or E_NOTIMPL.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "UIBehaviorClasses.h"
#include "../SundanceApp/SundanceAppMain.h"

namespace Sundance
{

// ============================================================================
// TimelineBehaviorSelection
// ============================================================================
TimelineBehaviorSelection::TimelineBehaviorSelection() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorSelection::~TimelineBehaviorSelection() { m_pElement = NULL; }

HRESULT TimelineBehaviorSelection::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorSelection::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorSelection::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return E_NOTIMPL;
}

// ============================================================================
// TimelineBehaviorDragDrop
// ============================================================================
TimelineBehaviorDragDrop::TimelineBehaviorDragDrop() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorDragDrop::~TimelineBehaviorDragDrop() { m_pElement = NULL; }

HRESULT TimelineBehaviorDragDrop::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorDragDrop::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorDragDrop::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return E_NOTIMPL;
}

// ============================================================================
// TimelineBehaviorResize
// ============================================================================
TimelineBehaviorResize::TimelineBehaviorResize() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorResize::~TimelineBehaviorResize() { m_pElement = NULL; }

HRESULT TimelineBehaviorResize::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorResize::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorResize::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return E_NOTIMPL;
}

// ============================================================================
// TimelineBehaviorTrim
// ============================================================================
TimelineBehaviorTrim::TimelineBehaviorTrim() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorTrim::~TimelineBehaviorTrim() { m_pElement = NULL; }

HRESULT TimelineBehaviorTrim::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorTrim::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorTrim::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return E_NOTIMPL;
}

// ============================================================================
// TimelineBehaviorSplit
// ============================================================================
TimelineBehaviorSplit::TimelineBehaviorSplit() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorSplit::~TimelineBehaviorSplit() { m_pElement = NULL; }

HRESULT TimelineBehaviorSplit::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorSplit::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorSplit::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return E_NOTIMPL;
}

// ============================================================================
// TimelineBehaviorReorder
// ============================================================================
TimelineBehaviorReorder::TimelineBehaviorReorder() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorReorder::~TimelineBehaviorReorder() { m_pElement = NULL; }

HRESULT TimelineBehaviorReorder::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorReorder::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorReorder::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return E_NOTIMPL;
}

// ============================================================================
// TimelineBehaviorZoom
// ============================================================================
TimelineBehaviorZoom::TimelineBehaviorZoom() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorZoom::~TimelineBehaviorZoom() { m_pElement = NULL; }

HRESULT TimelineBehaviorZoom::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorZoom::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorZoom::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return E_NOTIMPL;
}

// ============================================================================
// TimelineBehaviorScroll
// ============================================================================
TimelineBehaviorScroll::TimelineBehaviorScroll() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorScroll::~TimelineBehaviorScroll() { m_pElement = NULL; }

HRESULT TimelineBehaviorScroll::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorScroll::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorScroll::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return E_NOTIMPL;
}

// ============================================================================
// TimelineBehaviorSnap
// ============================================================================
TimelineBehaviorSnap::TimelineBehaviorSnap() : m_pElement(NULL), m_pAppMain(NULL) {}
TimelineBehaviorSnap::~TimelineBehaviorSnap() { m_pElement = NULL; }

HRESULT TimelineBehaviorSnap::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT TimelineBehaviorSnap::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT TimelineBehaviorSnap::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return E_NOTIMPL;
}

// ============================================================================
// RibbonCommandHandler
// ============================================================================
RibbonCommandHandler::RibbonCommandHandler() : m_pElement(NULL) {}
RibbonCommandHandler::~RibbonCommandHandler() { m_pElement = NULL; }

HRESULT RibbonCommandHandler::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonCommandHandler::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonEventHandler
// ============================================================================
RibbonEventHandler::RibbonEventHandler() : m_pElement(NULL) {}
RibbonEventHandler::~RibbonEventHandler() { m_pElement = NULL; }

HRESULT RibbonEventHandler::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonEventHandler::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonSiteBehavior
// ============================================================================
RibbonSiteBehavior::RibbonSiteBehavior() : m_pElement(NULL) {}
RibbonSiteBehavior::~RibbonSiteBehavior() { m_pElement = NULL; }

HRESULT RibbonSiteBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonSiteBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonContextualUIBehavior
// ============================================================================
RibbonContextualUIBehavior::RibbonContextualUIBehavior() : m_pElement(NULL) {}
RibbonContextualUIBehavior::~RibbonContextualUIBehavior() { m_pElement = NULL; }

HRESULT RibbonContextualUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonContextualUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonQuickAccessBehavior
// ============================================================================
RibbonQuickAccessBehavior::RibbonQuickAccessBehavior() : m_pElement(NULL) {}
RibbonQuickAccessBehavior::~RibbonQuickAccessBehavior() { m_pElement = NULL; }

HRESULT RibbonQuickAccessBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonQuickAccessBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonGalleryBehavior
// ============================================================================
RibbonGalleryBehavior::RibbonGalleryBehavior() : m_pElement(NULL) {}
RibbonGalleryBehavior::~RibbonGalleryBehavior() { m_pElement = NULL; }

HRESULT RibbonGalleryBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonGalleryBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// RibbonRecentItemsBehavior
// ============================================================================
RibbonRecentItemsBehavior::RibbonRecentItemsBehavior() : m_pElement(NULL) {}
RibbonRecentItemsBehavior::~RibbonRecentItemsBehavior() { m_pElement = NULL; }

HRESULT RibbonRecentItemsBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT RibbonRecentItemsBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SelectionRootImpl
// ============================================================================
SelectionRootImpl::SelectionRootImpl() : m_pElement(NULL), m_pAppMain(NULL) {}
SelectionRootImpl::~SelectionRootImpl() { m_pElement = NULL; }

HRESULT SelectionRootImpl::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SelectionRootImpl::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT SelectionRootImpl::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    if (pbHandled) *pbHandled = FALSE;
    return E_NOTIMPL;
}

// ============================================================================
// SelectionManager
// ============================================================================
SelectionManager::SelectionManager() : m_pElement(NULL), m_pAppMain(NULL) {}
SelectionManager::~SelectionManager() { m_pElement = NULL; }

HRESULT SelectionManager::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SelectionManager::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SelectionRange
// ============================================================================
SelectionRange::SelectionRange() : m_pElement(NULL) {}
SelectionRange::~SelectionRange() { m_pElement = NULL; }

HRESULT SelectionRange::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT SelectionRange::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT SelectionRange::OnPaint(HDC hdc, const RECT* prcBounds)
{
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(prcBounds);
    return S_OK;
}

// ============================================================================
// SelectionContext
// ============================================================================
SelectionContext::SelectionContext() : m_pElement(NULL) {}
SelectionContext::~SelectionContext() { m_pElement = NULL; }

HRESULT SelectionContext::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT SelectionContext::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SelectionUI
// ============================================================================
SelectionUI::SelectionUI() : m_pElement(NULL) {}
SelectionUI::~SelectionUI() { m_pElement = NULL; }

HRESULT SelectionUI::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT SelectionUI::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT SelectionUI::OnPaint(HDC hdc, const RECT* prcBounds)
{
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(prcBounds);
    return S_OK;
}

// ============================================================================
// EditorToolbar
// ============================================================================
EditorToolbar::EditorToolbar() : m_pElement(NULL), m_pAppMain(NULL) {}
EditorToolbar::~EditorToolbar() { m_pElement = NULL; }

HRESULT EditorToolbar::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT EditorToolbar::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// EditorCommandHandler
// ============================================================================
EditorCommandHandler::EditorCommandHandler() : m_pElement(NULL), m_pAppMain(NULL) {}
EditorCommandHandler::~EditorCommandHandler() { m_pElement = NULL; }

HRESULT EditorCommandHandler::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT EditorCommandHandler::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// EditorState
// ============================================================================
EditorState::EditorState() : m_pElement(NULL), m_pAppMain(NULL) {}
EditorState::~EditorState() { m_pElement = NULL; }

HRESULT EditorState::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT EditorState::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SundanceRibbonElementBehavior
// ============================================================================
SundanceRibbonElementBehavior::SundanceRibbonElementBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
SundanceRibbonElementBehavior::~SundanceRibbonElementBehavior() { m_pElement = NULL; }

HRESULT SundanceRibbonElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundanceRibbonElementBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SundanceStatusBarElementBehavior
// ============================================================================
SundanceStatusBarElementBehavior::SundanceStatusBarElementBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
SundanceStatusBarElementBehavior::~SundanceStatusBarElementBehavior() { m_pElement = NULL; }

HRESULT SundanceStatusBarElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundanceStatusBarElementBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SundanceTimelineElementBehavior
// ============================================================================
SundanceTimelineElementBehavior::SundanceTimelineElementBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
SundanceTimelineElementBehavior::~SundanceTimelineElementBehavior() { m_pElement = NULL; }

HRESULT SundanceTimelineElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundanceTimelineElementBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SundancePreviewElementBehavior
// ============================================================================
SundancePreviewElementBehavior::SundancePreviewElementBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
SundancePreviewElementBehavior::~SundancePreviewElementBehavior() { m_pElement = NULL; }

HRESULT SundancePreviewElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundancePreviewElementBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// SundancePropertyElementBehavior
// ============================================================================
SundancePropertyElementBehavior::SundancePropertyElementBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
SundancePropertyElementBehavior::~SundancePropertyElementBehavior() { m_pElement = NULL; }

HRESULT SundancePropertyElementBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT SundancePropertyElementBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// WebcamUIBehavior
// ============================================================================
WebcamUIBehavior::WebcamUIBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
WebcamUIBehavior::~WebcamUIBehavior() { m_pElement = NULL; }

HRESULT WebcamUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT WebcamUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// WebcamPreviewBehavior
// ============================================================================
WebcamPreviewBehavior::WebcamPreviewBehavior() : m_pElement(NULL) {}
WebcamPreviewBehavior::~WebcamPreviewBehavior() { m_pElement = NULL; }

HRESULT WebcamPreviewBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT WebcamPreviewBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT WebcamPreviewBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    if (!hdc || !prcBounds) return E_POINTER;

    HBRUSH hBrush = CreateSolidBrush(RGB(32, 32, 32));
    FillRect(hdc, prcBounds, hBrush);
    DeleteObject(hBrush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(150, 150, 150));
    DrawText(hdc, L"Webcam Preview", -1, const_cast<RECT*>(prcBounds),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    return S_OK;
}

// ============================================================================
// WebcamCaptureBehavior
// ============================================================================
WebcamCaptureBehavior::WebcamCaptureBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
WebcamCaptureBehavior::~WebcamCaptureBehavior() { m_pElement = NULL; }

HRESULT WebcamCaptureBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT WebcamCaptureBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// NarrationUIBehavior
// ============================================================================
NarrationUIBehavior::NarrationUIBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
NarrationUIBehavior::~NarrationUIBehavior() { m_pElement = NULL; }

HRESULT NarrationUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT NarrationUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// NarrationRecordBehavior
// ============================================================================
NarrationRecordBehavior::NarrationRecordBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
NarrationRecordBehavior::~NarrationRecordBehavior() { m_pElement = NULL; }

HRESULT NarrationRecordBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT NarrationRecordBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// NarrationPlaybackBehavior
// ============================================================================
NarrationPlaybackBehavior::NarrationPlaybackBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
NarrationPlaybackBehavior::~NarrationPlaybackBehavior() { m_pElement = NULL; }

HRESULT NarrationPlaybackBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT NarrationPlaybackBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// UserEncodeProfileManager
// ============================================================================
UserEncodeProfileManager::UserEncodeProfileManager() : m_pElement(NULL), m_pAppMain(NULL) {}
UserEncodeProfileManager::~UserEncodeProfileManager() { m_pElement = NULL; }

HRESULT UserEncodeProfileManager::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT UserEncodeProfileManager::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// UserEncodeProfilePreset
// ============================================================================
UserEncodeProfilePreset::UserEncodeProfilePreset() : m_pElement(NULL) {}
UserEncodeProfilePreset::~UserEncodeProfilePreset() { m_pElement = NULL; }

HRESULT UserEncodeProfilePreset::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT UserEncodeProfilePreset::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// UserEncodeProfileCustom
// ============================================================================
UserEncodeProfileCustom::UserEncodeProfileCustom() : m_pElement(NULL) {}
UserEncodeProfileCustom::~UserEncodeProfileCustom() { m_pElement = NULL; }

HRESULT UserEncodeProfileCustom::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT UserEncodeProfileCustom::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// UserEncodeProfileRegistry
// ============================================================================
UserEncodeProfileRegistry::UserEncodeProfileRegistry() : m_pElement(NULL) {}
UserEncodeProfileRegistry::~UserEncodeProfileRegistry() { m_pElement = NULL; }

HRESULT UserEncodeProfileRegistry::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT UserEncodeProfileRegistry::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogBaseBehavior
// ============================================================================
PublishDialogBaseBehavior::PublishDialogBaseBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogBaseBehavior::~PublishDialogBaseBehavior() { m_pElement = NULL; }

HRESULT PublishDialogBaseBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogBaseBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogYouTubeBehavior
// ============================================================================
PublishDialogYouTubeBehavior::PublishDialogYouTubeBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogYouTubeBehavior::~PublishDialogYouTubeBehavior() { m_pElement = NULL; }

HRESULT PublishDialogYouTubeBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogYouTubeBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogFacebookBehavior
// ============================================================================
PublishDialogFacebookBehavior::PublishDialogFacebookBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogFacebookBehavior::~PublishDialogFacebookBehavior() { m_pElement = NULL; }

HRESULT PublishDialogFacebookBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogFacebookBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogSkyDriveBehavior
// ============================================================================
PublishDialogSkyDriveBehavior::PublishDialogSkyDriveBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogSkyDriveBehavior::~PublishDialogSkyDriveBehavior() { m_pElement = NULL; }

HRESULT PublishDialogSkyDriveBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogSkyDriveBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogEmailBehavior
// ============================================================================
PublishDialogEmailBehavior::PublishDialogEmailBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogEmailBehavior::~PublishDialogEmailBehavior() { m_pElement = NULL; }

HRESULT PublishDialogEmailBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogEmailBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// PublishDialogLocalFileBehavior
// ============================================================================
PublishDialogLocalFileBehavior::PublishDialogLocalFileBehavior() : m_pElement(NULL), m_pAppMain(NULL) {}
PublishDialogLocalFileBehavior::~PublishDialogLocalFileBehavior() { m_pElement = NULL; }

HRESULT PublishDialogLocalFileBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    m_pAppMain = GetSundanceAppMain();
    return S_OK;
}

HRESULT PublishDialogLocalFileBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// ProgressBarUIBehavior
// ============================================================================
ProgressBarUIBehavior::ProgressBarUIBehavior() : m_pElement(NULL) {}
ProgressBarUIBehavior::~ProgressBarUIBehavior() { m_pElement = NULL; }

HRESULT ProgressBarUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ProgressBarUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT ProgressBarUIBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(prcBounds);
    return S_OK;
}

// ============================================================================
// ErrorDialogUIBehavior
// ============================================================================
ErrorDialogUIBehavior::ErrorDialogUIBehavior() : m_pElement(NULL) {}
ErrorDialogUIBehavior::~ErrorDialogUIBehavior() { m_pElement = NULL; }

HRESULT ErrorDialogUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ErrorDialogUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// WarningDialogUIBehavior
// ============================================================================
WarningDialogUIBehavior::WarningDialogUIBehavior() : m_pElement(NULL) {}
WarningDialogUIBehavior::~WarningDialogUIBehavior() { m_pElement = NULL; }

HRESULT WarningDialogUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT WarningDialogUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// InfoDialogUIBehavior
// ============================================================================
InfoDialogUIBehavior::InfoDialogUIBehavior() : m_pElement(NULL) {}
InfoDialogUIBehavior::~InfoDialogUIBehavior() { m_pElement = NULL; }

HRESULT InfoDialogUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT InfoDialogUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// ConfirmationDialogUIBehavior
// ============================================================================
ConfirmationDialogUIBehavior::ConfirmationDialogUIBehavior() : m_pElement(NULL) {}
ConfirmationDialogUIBehavior::~ConfirmationDialogUIBehavior() { m_pElement = NULL; }

HRESULT ConfirmationDialogUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ConfirmationDialogUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// FileBrowserUIBehavior
// ============================================================================
FileBrowserUIBehavior::FileBrowserUIBehavior() : m_pElement(NULL) {}
FileBrowserUIBehavior::~FileBrowserUIBehavior() { m_pElement = NULL; }

HRESULT FileBrowserUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT FileBrowserUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// FolderBrowserUIBehavior
// ============================================================================
FolderBrowserUIBehavior::FolderBrowserUIBehavior() : m_pElement(NULL) {}
FolderBrowserUIBehavior::~FolderBrowserUIBehavior() { m_pElement = NULL; }

HRESULT FolderBrowserUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT FolderBrowserUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// ColorPickerUIBehavior
// ============================================================================
ColorPickerUIBehavior::ColorPickerUIBehavior() : m_pElement(NULL) {}
ColorPickerUIBehavior::~ColorPickerUIBehavior() { m_pElement = NULL; }

HRESULT ColorPickerUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT ColorPickerUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// FontPickerUIBehavior
// ============================================================================
FontPickerUIBehavior::FontPickerUIBehavior() : m_pElement(NULL) {}
FontPickerUIBehavior::~FontPickerUIBehavior() { m_pElement = NULL; }

HRESULT FontPickerUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT FontPickerUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

// ============================================================================
// CropUIBehavior
// ============================================================================
CropUIBehavior::CropUIBehavior() : m_pElement(NULL) {}
CropUIBehavior::~CropUIBehavior() { m_pElement = NULL; }

HRESULT CropUIBehavior::OnElementAttached(IDuiElement* pElement)
{
    if (!pElement) return E_POINTER;
    m_pElement = pElement;
    return S_OK;
}

HRESULT CropUIBehavior::OnElementDetached(IDuiElement*) { m_pElement = NULL; return S_OK; }

HRESULT CropUIBehavior::OnPaint(HDC hdc, const RECT* prcBounds)
{
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(prcBounds);
    return S_OK;
}

} // namespace Sundance
