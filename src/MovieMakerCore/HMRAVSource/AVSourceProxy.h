/*
 * AVSourceProxy.h
 *
 * AVSourceProxy - Thread-safe proxy wrapper around AVSource.
 * Marshals interface calls across threads using COM channel marshaling
 * and provides synchronized access to an underlying AVSource instance
 * that may be created on a different thread.
 *
 * RTTI: ?AVAVSourceProxy@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AVSOURCEPROXY_H
#define AVSOURCEPROXY_H

#include "HMRAVSource.h"
#include "AVSource.h"

namespace HMRAVSource
{

// ============================================================================
// AVSourceProxy
// ============================================================================
// Thread-safe proxy that wraps an AVSource instance. The underlying source
// may be created on an apartment thread; this proxy marshals all calls
// to that thread using a message window and COM object marshaling.
//
// Consumer threads call through the proxy transparently; all cross-thread
// marshaling is handled internally.
//
class AVSOURCE_API AVSourceProxy
{
public:
    AVSourceProxy();
    virtual ~AVSourceProxy();

    // Lifecycle
    HRESULT Initialize(AVSource* pSource);
    HRESULT Initialize(AVSource* pSource, DWORD dwCreationThreadId);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Delegated AVSource operations
    HRESULT Open(const AVSourceDesc& desc);
    HRESULT Close();
    bool IsOpen() const throw();

    HRESULT Start();
    HRESULT Stop();
    HRESULT Pause();
    HRESULT Resume();

    LONGLONG GetPositionHns() const throw();
    HRESULT SetPositionHns(LONGLONG llPosition);
    LONGLONG GetDurationHns() const throw();

    HRESULT ReadSample(IMFSample** ppSample, DWORD dwStreamIndex);
    HRESULT Flush();

    // Source info
    AVSourceState GetState() const throw();
    const AVSourceInfo& GetInfo() const;
    AVSourceType GetType() const throw();

    // Thread affinity
    DWORD GetCreationThreadId() const throw();
    bool IsOnCreationThread() const throw();

    // Cross-thread marshaling control
    HRESULT SetMarshalingMode(bool fCrossThread);
    bool IsCrossThreadMarshaling() const throw();

    // Error info
    HRESULT GetLastResult() const throw();

private:
    AVSource*       m_pSource;
    DWORD           m_dwCreationThreadId;
    DWORD           m_dwProxyId;
    bool            m_fCrossThread;
    bool            m_fInitialized;
    HWND            m_hMessageWnd;

    static DWORD s_dwNextProxyId;

    bool EnsureOnCorrectThread() const;
    HRESULT PostToCreationThread(std::function<void()> fn);
    LRESULT SendMessageToCreationThread(UINT uMsg, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK ProxyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

} // namespace HMRAVSource

#endif // AVSOURCEPROXY_H
