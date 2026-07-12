/*
 * SundanceClipboardChainWindow.cpp
 *
 * Implementation of the clipboard chain window.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "SundanceClipboardChainWindow.h"
#include "../SundanceApp/SundanceAppMain.h"
#include "../SundanceApp/ClipboardManager.h"

namespace Sundance
{

SundanceClipboardChainWindow::SundanceClipboardChainWindow()
    : m_hWndOwner(NULL)
    , m_hWndNextChain(NULL)
    , m_pAppMain(NULL)
    , m_bRegistered(false)
{
}

SundanceClipboardChainWindow::~SundanceClipboardChainWindow()
{
    Destroy();
}

HRESULT SundanceClipboardChainWindow::Create(HWND hWndOwner)
{
    if (!hWndOwner)
        return E_INVALIDARG;

    m_hWndOwner = hWndOwner;

    HWND hWnd = CWindowImpl<SundanceClipboardChainWindow, CWindow>::Create(
        NULL, NULL, NULL,
        WS_CHILD | WS_DISABLED, 0, reinterpret_cast<UINT_PTR>(this));

    if (!hWnd)
        return HRESULT_FROM_WIN32(GetLastError());

    // Add to clipboard viewer chain
    m_hWndNextChain = SetClipboardViewer(m_hWnd);

    // Also register for clipboard update notifications (Vista+)
    AddClipboardFormatListener(m_hWnd);

    m_bRegistered = true;
    return S_OK;
}

void SundanceClipboardChainWindow::Destroy()
{
    if (m_bRegistered && IsWindow())
    {
        ChangeClipboardChain(m_hWnd, m_hWndNextChain);
        RemoveClipboardFormatListener(m_hWnd);
        m_bRegistered = false;
    }

    if (IsWindow())
        DestroyWindow();

    m_hWndNextChain = NULL;
    m_hWndOwner = NULL;
}

void SundanceClipboardChainWindow::SetAppMain(SundanceAppMain* pAppMain) throw()
{
    m_pAppMain = pAppMain;
}

SundanceAppMain* SundanceClipboardChainWindow::GetAppMain() const throw()
{
    return m_pAppMain;
}

void SundanceClipboardChainWindow::RegisterFormat(UINT uFormat)
{
    m_monitoredFormats.insert(uFormat);
}

void SundanceClipboardChainWindow::UnregisterFormat(UINT uFormat)
{
    m_monitoredFormats.erase(uFormat);
}

bool SundanceClipboardChainWindow::IsMonitoringFormat(UINT uFormat) const
{
    return m_monitoredFormats.find(uFormat) != m_monitoredFormats.end();
}

HRESULT SundanceClipboardChainWindow::GetClipboardData(UINT uFormat, HGLOBAL* phGlobal)
{
    if (!phGlobal)
        return E_POINTER;

    *phGlobal = NULL;

    if (!::OpenClipboard(m_hWnd))
        return E_FAIL;

    HANDLE hData = ::GetClipboardData(uFormat);
    if (hData)
    {
        *phGlobal = static_cast<HGLOBAL>(::GlobalAlloc(GMEM_MOVEABLE, ::GlobalSize(hData)));
        if (*phGlobal)
        {
            void* pSrc = ::GlobalLock(hData);
            void* pDst = ::GlobalLock(*phGlobal);
            if (pSrc && pDst)
            {
                memcpy(pDst, pSrc, ::GlobalSize(hData));
            }
            ::GlobalUnlock(hData);
            ::GlobalUnlock(*phGlobal);
        }
    }

    ::CloseClipboard();

    return *phGlobal ? S_OK : E_FAIL;
}

bool SundanceClipboardChainWindow::HasClipboardData(UINT uFormat) const
{
    return const_cast<SundanceClipboardChainWindow*>(this)->HasFormat(uFormat);
}

LRESULT SundanceClipboardChainWindow::OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    return 0;
}

LRESULT SundanceClipboardChainWindow::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    m_bRegistered = false;
    return 0;
}

LRESULT SundanceClipboardChainWindow::OnDrawClipboard(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;

    OnClipboardChanged();

    // Forward to next window in the clipboard viewer chain
    if (m_hWndNextChain && ::IsWindow(m_hWndNextChain))
        ::SendMessage(m_hWndNextChain, WM_DRAWCLIPBOARD, 0, 0);

    return 0;
}

LRESULT SundanceClipboardChainWindow::OnChangeCbChain(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;

    HWND hWndRemove = reinterpret_cast<HWND>(wParam);
    HWND hWndNew = reinterpret_cast<HWND>(lParam);

    if (hWndRemove == m_hWndNextChain)
    {
        m_hWndNextChain = hWndNew;
    }
    else if (m_hWndNextChain && ::IsWindow(m_hWndNextChain))
    {
        ::SendMessage(m_hWndNextChain, WM_CHANGECBCHAIN, wParam, lParam);
    }

    return 0;
}

LRESULT SundanceClipboardChainWindow::OnClipboardUpdate(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    OnClipboardChanged();
    return 0;
}

void SundanceClipboardChainWindow::OnClipboardChanged()
{
    if (m_pAppMain)
    {
        ClipboardManager* pClipboardMgr = m_pAppMain->GetClipboardManager();
        if (pClipboardMgr)
        {
            pClipboardMgr->OnClipboardChanged();
        }
    }
}

bool SundanceClipboardChainWindow::HasFormat(UINT uFormat) const
{
    if (!::IsClipboardFormatAvailable(uFormat))
        return false;

    // Check if it's one of our monitored formats
    if (!m_monitoredFormats.empty())
    {
        return m_monitoredFormats.find(uFormat) != m_monitoredFormats.end();
    }

    return true;
}

} // namespace Sundance
