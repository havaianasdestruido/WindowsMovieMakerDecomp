/*
 * SundanceClipboardChainWindow.h
 *
 * Clipboard chain window for monitoring clipboard changes. Uses
 * SetClipboardViewer / AddClipboardFormatListener to receive
 * WM_DRAWCLIPBOARD / WM_CLIPBOARDUPDATE notifications and relays
 * clipboard data changes to the SundanceAppMain clipboard manager.
 *
 * RTTI: ?AVSundanceClipboardChainWindow@@
 * ATL:  CWindowImpl<SundanceClipboardChainWindow>
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_CLIPBOARD_CHAIN_WINDOW_H
#define SUNDANCE_CLIPBOARD_CHAIN_WINDOW_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"

class SundanceAppMain;

namespace Sundance
{

class SundanceClipboardChainWindow :
    public CWindowImpl<SundanceClipboardChainWindow, CWindow>
{
public:
    SundanceClipboardChainWindow();
    ~SundanceClipboardChainWindow();

    DECLARE_WND_CLASS(L"SundanceClipboardChain")

    BEGIN_MSG_MAP(SundanceClipboardChainWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_DRAWCLIPBOARD, OnDrawClipboard)
        MESSAGE_HANDLER(WM_CHANGECBCHAIN, OnChangeCbChain)
        MESSAGE_HANDLER(WM_CLIPBOARDUPDATE, OnClipboardUpdate)
    END_MSG_MAP()

    // -- Lifecycle --
    HRESULT Create(HWND hWndOwner);
    void    Destroy();

    // -- Configuration --
    void    SetAppMain(SundanceAppMain* pAppMain) throw();
    SundanceAppMain* GetAppMain() const throw();

    // -- Clipboard format monitoring --
    void    RegisterFormat(UINT uFormat);
    void    UnregisterFormat(UINT uFormat);
    bool    IsMonitoringFormat(UINT uFormat) const;

    // -- Clipboard data access --
    HRESULT GetClipboardData(UINT uFormat, HGLOBAL* phGlobal);
    bool    HasClipboardData(UINT uFormat) const;

    // WTL message handlers
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDrawClipboard(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClipboardUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    HWND            m_hWndOwner;
    HWND            m_hWndNextChain;
    SundanceAppMain* m_pAppMain;
    bool            m_bRegistered;
    std::set<UINT>  m_monitoredFormats;

    void OnClipboardChanged();
    bool HasFormat(UINT uFormat) const;
};

} // namespace Sundance

#endif // SUNDANCE_CLIPBOARD_CHAIN_WINDOW_H
