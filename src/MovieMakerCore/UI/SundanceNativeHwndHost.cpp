/*
 * SundanceNativeHwndHost.cpp
 *
 * Implementation of HWND hosting for DirectUI elements.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "SundanceNativeHwndHost.h"

namespace Sundance
{

const HWND SundanceNativeHwndHost::HWND_TOPMOST = ::HWND_TOPMOST;
const HWND SundanceNativeHwndHost::HWND_NOTOPMOST = ::HWND_NOTOPMOST;

SundanceNativeHwndHost::SundanceNativeHwndHost()
    : m_pHostElement(NULL)
    , m_hWndChild(NULL)
    , m_hWndParent(NULL)
    , m_hWndZOrder(HWND_TOP)
    , m_bVisible(true)
    , m_bInitialized(false)
{
    SetRectEmpty(&m_rcBounds);
}

SundanceNativeHwndHost::~SundanceNativeHwndHost()
{
    Shutdown();
}

HRESULT SundanceNativeHwndHost::Initialize(IDuiElement* pHostElement)
{
    if (!pHostElement)
        return E_POINTER;

    if (m_bInitialized)
        return E_UNEXPECTED;

    m_pHostElement = pHostElement;

    // Get the host HWND from the DirectUI element
    // In the real implementation, IDuiElement provides a method to
    // get the owning HWND of the DirectUI host window.
    m_hWndParent = NULL;

    m_bInitialized = true;
    return S_OK;
}

void SundanceNativeHwndHost::Shutdown()
{
    if (m_hWndChild && ::IsWindow(m_hWndChild))
    {
        ::DestroyWindow(m_hWndChild);
    }
    m_hWndChild = NULL;

    if (m_hWndParent && ::IsWindow(m_hWndParent))
    {
        // Release the parent reference
    }
    m_hWndParent = NULL;
    m_pHostElement = NULL;
    m_bInitialized = false;
}

HWND SundanceNativeHwndHost::CreateChildHwnd(LPCWSTR pszClassName, LPCWSTR pszWindowText,
    DWORD dwStyle, DWORD dwExStyle, const RECT* prc)
{
    EnsureParentHwnd();

    if (!m_hWndParent)
        return NULL;

    RECT rcChild = prc ? *prc : m_rcBounds;

    m_hWndChild = ::CreateWindowEx(
        dwExStyle,
        pszClassName,
        pszWindowText,
        dwStyle | WS_CHILD,
        rcChild.left, rcChild.top,
        rcChild.right - rcChild.left,
        rcChild.bottom - rcChild.top,
        m_hWndParent,
        NULL,
        ModuleHelper::GetModuleInstance(),
        NULL);

    return m_hWndChild;
}

HWND SundanceNativeHwndHost::CreateChildHwndEx(LPCWSTR pszClassName, LPCWSTR pszWindowText,
    DWORD dwStyle, DWORD dwExStyle, const RECT* prc, HMENU hMenu, HINSTANCE hInstance)
{
    EnsureParentHwnd();

    if (!m_hWndParent)
        return NULL;

    RECT rcChild = prc ? *prc : m_rcBounds;

    m_hWndChild = ::CreateWindowEx(
        dwExStyle,
        pszClassName,
        pszWindowText,
        dwStyle | WS_CHILD,
        rcChild.left, rcChild.top,
        rcChild.right - rcChild.left,
        rcChild.bottom - rcChild.top,
        m_hWndParent,
        hMenu,
        hInstance ? hInstance : ModuleHelper::GetModuleInstance(),
        NULL);

    return m_hWndChild;
}

void SundanceNativeHwndHost::DestroyChildHwnd(HWND hWndChild)
{
    if (hWndChild && ::IsWindow(hWndChild))
    {
        ::DestroyWindow(hWndChild);
    }

    if (hWndChild == m_hWndChild)
    {
        m_hWndChild = NULL;
    }
}

HWND SundanceNativeHwndHost::GetChildHwnd() const throw()
{
    return m_hWndChild;
}

void SundanceNativeHwndHost::SetHostBounds(const RECT& rcBounds)
{
    m_rcBounds = rcBounds;

    if (m_hWndChild && ::IsWindow(m_hWndChild))
    {
        ::SetWindowPos(m_hWndChild, NULL,
            rcBounds.left, rcBounds.top,
            rcBounds.right - rcBounds.left,
            rcBounds.bottom - rcBounds.top,
            SWP_NOZORDER);
    }
}

void SundanceNativeHwndHost::GetHostBounds(RECT* prcBounds) const
{
    if (prcBounds) *prcBounds = m_rcBounds;
}

void SundanceNativeHwndHost::UpdateChildLayout()
{
    if (m_hWndChild && ::IsWindow(m_hWndChild))
    {
        ::SetWindowPos(m_hWndChild, NULL,
            m_rcBounds.left, m_rcBounds.top,
            m_rcBounds.right - m_rcBounds.left,
            m_rcBounds.bottom - m_rcBounds.top,
            SWP_NOZORDER);
    }
}

void SundanceNativeHwndHost::SetVisible(bool bVisible)
{
    m_bVisible = bVisible;

    if (m_hWndChild && ::IsWindow(m_hWndChild))
    {
        ::ShowWindow(m_hWndChild, bVisible ? SW_SHOW : SW_HIDE);
    }
}

bool SundanceNativeHwndHost::IsVisible() const throw()
{
    return m_bVisible;
}

void SundanceNativeHwndHost::SetZOrder(HWND hWndInsertAfter)
{
    m_hWndZOrder = hWndInsertAfter;

    if (m_hWndChild && ::IsWindow(m_hWndChild))
    {
        ::SetWindowPos(m_hWndChild, hWndInsertAfter, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE);
    }
}

HRESULT SundanceNativeHwndHost::ForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
    if (!plResult)
        return E_POINTER;

    *plResult = 0;

    if (!m_hWndChild || !::IsWindow(m_hWndChild))
        return S_FALSE;

    *plResult = ::SendMessage(m_hWndChild, uMsg, wParam, lParam);
    return S_OK;
}

void SundanceNativeHwndHost::OnHostElementResized(int nWidth, int nHeight)
{
    m_rcBounds.right = m_rcBounds.left + nWidth;
    m_rcBounds.bottom = m_rcBounds.top + nHeight;
    UpdateChildLayout();
}

void SundanceNativeHwndHost::OnHostElementMoved(int x, int y)
{
    int nWidth = m_rcBounds.right - m_rcBounds.left;
    int nHeight = m_rcBounds.bottom - m_rcBounds.top;
    m_rcBounds.left = x;
    m_rcBounds.top = y;
    m_rcBounds.right = x + nWidth;
    m_rcBounds.bottom = y + nHeight;
    UpdateChildLayout();
}

void SundanceNativeHwndHost::OnHostElementDestroyed()
{
    Shutdown();
}

void SundanceNativeHwndHost::EnsureParentHwnd()
{
    if (m_hWndParent && ::IsWindow(m_hWndParent))
        return;

    // In the real implementation, we obtain the parent HWND from the
    // DirectUI element tree. The host element knows its owning HWND.
    m_hWndParent = NULL;
}

} // namespace Sundance
