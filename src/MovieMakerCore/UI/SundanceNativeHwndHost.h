/*
 * SundanceNativeHwndHost.h
 *
 * HWND hosting for DirectUI elements. Provides a bridge between
 * traditional Win32 HWND controls and the DirectUI windowless element
// tree. Hosts child HWNDs (like RichEdit controls, COM ActiveX controls,
 * or webcam preview windows) within the DirectUI layout.
 *
 * RTTI: ?AVSundanceNativeHwndHost@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_NATIVE_HWND_HOST_H
#define SUNDANCE_NATIVE_HWND_HOST_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

namespace Sundance
{

class SundanceNativeHwndHost
{
public:
    SundanceNativeHwndHost();
    ~SundanceNativeHwndHost();

    // -- Lifecycle --
    HRESULT Initialize(IDuiElement* pHostElement);
    void    Shutdown();

    // -- HWND hosting --
    HWND    CreateChildHwnd(LPCWSTR pszClassName, LPCWSTR pszWindowText,
                DWORD dwStyle, DWORD dwExStyle, const RECT* prc);
    HWND    CreateChildHwndEx(LPCWSTR pszClassName, LPCWSTR pszWindowText,
                DWORD dwStyle, DWORD dwExStyle, const RECT* prc,
                HMENU hMenu, HINSTANCE hInstance);

    void    DestroyChildHwnd(HWND hWndChild);
    HWND    GetChildHwnd() const throw();

    // -- Layout --
    void    SetHostBounds(const RECT& rcBounds);
    void    GetHostBounds(RECT* prcBounds) const;
    void    UpdateChildLayout();

    // -- Visibility --
    void    SetVisible(bool bVisible);
    bool    IsVisible() const throw();

    // -- Z-order --
    void    SetZOrder(HWND hWndInsertAfter);

    // -- Message forwarding --
    HRESULT ForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

    // -- IDuiElement integration --
    void    OnHostElementResized(int nWidth, int nHeight);
    void    OnHostElementMoved(int x, int y);
    void    OnHostElementDestroyed();

private:
    IDuiElement*    m_pHostElement;
    HWND            m_hWndChild;
    HWND            m_hWndParent;
    HWND            m_hWndZOrder;
    RECT            m_rcBounds;
    bool            m_bVisible;
    bool            m_bInitialized;

    static const HWND HWND_TOPMOST;
    static const HWND HWND_NOTOPMOST;

    void EnsureParentHwnd();
};

} // namespace Sundance

#endif // SUNDANCE_NATIVE_HWND_HOST_H
