/*
 * SliderUI.h
 *
 * Popup slider and track bar controls for volume/trim UI.
 * Implements popup slider as a CWindowImpl that hosts a custom
 * track bar control.
 *
 * RTTI classes:
 *   ?AVPopUpSlider@@
 *   ?AVPopUpSliderTrackBar@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_SLIDER_UI_H
#define SUNDANCE_SLIDER_UI_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

class SundanceAppMain;

namespace Sundance
{

// ============================================================================
// PopUpSlider
// ============================================================================
// Popup volume/trim slider. Created as a CWindowImpl that appears as a
// tooltip-style popup when the user clicks a volume button or trim handle.
// Hosts a PopUpSliderTrackBar child control.
//
// RTTI: ?AVPopUpSlider@@
// ATL:  CComObjectNoLock<PopUpSlider> (also CWindowImpl for HWND)
//
class ATL_NO_VTABLE PopUpSlider :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<PopUpSlider>,
    public IDuiBehaviorImpl,
    public CWindowImpl<PopUpSlider>
{
public:
    PopUpSlider();
    virtual ~PopUpSlider();

    DECLARE_REGISTRY_RESOURCEID(IDR_POPUP_SLIDER)
    DECLARE_NOT_AGGREGATABLE(PopUpSlider)

    BEGIN_COM_MAP(PopUpSlider)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    DECLARE_WND_CLASS(L"SundancePopUpSlider")

    BEGIN_MSG_MAP(PopUpSlider)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
        MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    END_MSG_MAP()

    // -- IDuiBehavior --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    // -- Slider operations --
    HRESULT ShowPopup(HWND hWndParent, POINT ptPosition);
    HRESULT HidePopup();
    bool    IsPopupVisible() const throw();

    // -- Slider value --
    void    SetValue(int nValue);
    int     GetValue() const throw();
    void    SetRange(int nMin, int nMax);
    void    GetRange(int* pnMin, int* pnMax) const;

    // -- Slider mode --
    enum SliderMode { SliderModeVolume, SliderModeTrimStart, SliderModeTrimEnd };
    void    SetSliderMode(SliderMode mode) throw();
    SliderMode GetSliderMode() const throw();

    // -- Callback --
    typedef void (CALLBACK* PFN_SLIDER_CHANGED)(int nValue, void* pUserData);
    void    SetCallback(PFN_SLIDER_CHANGED pfnCallback, void* pUserData);

    // WTL message handlers
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    IDuiElement*    m_pElement;
    HWND            m_hWndPopup;
    HWND            m_hWndTrackBar;

    int             m_nValue;
    int             m_nMin;
    int             m_nMax;
    SliderMode      m_mode;

    PFN_SLIDER_CHANGED m_pfnCallback;
    void*           m_pUserData;

    void CreateTrackBar(HWND hWndParent);
    void LayoutControls();
};

// ============================================================================
// PopUpSliderTrackBar
// ============================================================================
// Custom track bar control used by PopUpSlider. Renders a simplified
// slider track with a thumb that the user can drag. Uses owner-draw
// painting with GDI+ for smooth rendering.
//
// RTTI: ?AVPopUpSliderTrackBar@@
// ATL:  CWindowImpl<PopUpSliderTrackBar, CWindow>
//
class PopUpSliderTrackBar :
    public CWindowImpl<PopUpSliderTrackBar, CWindow>,
    public CCustomDraw<PopUpSliderTrackBar>
{
public:
    PopUpSliderTrackBar();
    ~PopUpSliderTrackBar();

    DECLARE_WND_CLASS(L"SundancePopUpSliderTrackBar")

    BEGIN_MSG_MAP(PopUpSliderTrackBar)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
    END_MSG_MAP()

    // -- Value --
    void    SetValue(int nValue);
    int     GetValue() const throw();
    void    SetRange(int nMin, int nMax);

    // -- Visual configuration --
    void    SetTrackHeight(int nHeight);
    void    SetThumbWidth(int nWidth);
    void    SetTrackColor(COLORREF clrTrack);
    void    SetFillColor(COLORREF clrFill);
    void    SetThumbColor(COLORREF clrThumb);

    // WTL message handlers
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    int     m_nValue;
    int     m_nMin;
    int     m_nMax;
    int     m_nTrackHeight;
    int     m_nThumbWidth;
    bool    m_bDragging;

    COLORREF m_clrTrack;
    COLORREF m_clrFill;
    COLORREF m_clrThumb;

    int ValueFromPoint(int x) const;
    int PointFromValue(int nValue) const;
    RECT GetThumbRect() const;
    void PaintTrack(HDC hdc, const RECT& rcClient);
    void PaintFill(HDC hdc, const RECT& rcTrack);
    void PaintThumb(HDC hdc, const RECT& rcThumb);
};

} // namespace Sundance

#endif // SUNDANCE_SLIDER_UI_H
