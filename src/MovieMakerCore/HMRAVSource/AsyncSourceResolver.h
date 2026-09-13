/*
 * AsyncSourceResolver.h
 *
 * AsyncSourceResolver - Asynchronous media source resolution for
 * network streams and protocol-based sources. Handles URL resolution,
 * protocol handler selection, and media source creation for streaming.
 *
 * RTTI: ?AVAsyncSourceResolver@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef ASYNCSOURCERESOLVER_H
#define ASYNCSOURCERESOLVER_H

#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// AsyncSourceResolverState
// ============================================================================
enum AsyncSourceResolverState
{
    AsyncResolverIdle       = 0,
    AsyncResolverResolving  = 1,
    AsyncResolverResolved   = 2,
    AsyncResolverError      = 3
};

// ============================================================================
// ResolvedSourceInfo
// ============================================================================
struct ResolvedSourceInfo
{
    ATL::CString    strResolvedUrl;
    ATL::CString    strProtocol;
    ATL::CString    strMimeType;
    LONGLONG        llDurationHns;
    bool            fIsStreaming;
    bool            fIsProtected;

    ResolvedSourceInfo()
        : llDurationHns(0)
        , fIsStreaming(false)
        , fIsProtected(false)
    {
    }
};

// ============================================================================
// AsyncSourceResolver
// ============================================================================
// Resolves media sources asynchronously for network URLs and protocol-based
// streams. Uses IMFSourceResolver to determine the correct source type
// (scheme handler or byte stream handler) and creates the media source
// on a background thread.
//
class AVSOURCE_API AsyncSourceResolver
{
public:
    AsyncSourceResolver();
    ~AsyncSourceResolver();

    // Lifecycle
    HRESULT Initialize();
    HRESULT Shutdown();

    // Async resolve
    HRESULT BeginResolve(LPCWSTR pszUrl);
    HRESULT EndResolve(ResolvedSourceInfo* pInfo);
    HRESULT CancelResolve();

    // Sync resolve (blocks until complete)
    HRESULT Resolve(LPCWSTR pszUrl, ResolvedSourceInfo* pInfo);

    // State
    AsyncSourceResolverState GetState() const throw();
    bool IsResolving() const throw();

    // URL info
    ATL::CString GetOriginalUrl() const;
    ATL::CString GetResolvedUrl() const;

    // Result access
    HRESULT GetAsyncResult() const throw();
    IMFMediaSource* GetResolvedSource();

    // Callbacks
    using ResolveCompleteCallback = std::function<void(HRESULT, const ResolvedSourceInfo&)>;
    void SetResolveCompleteCallback(ResolveCompleteCallback cb) { m_completeCb = cb; }

    // Protocol support
    static bool IsStreamingProtocol(LPCWSTR pszProtocol);
    static bool IsSupportedScheme(LPCWSTR pszUrl);
    static HRESULT ParseUrl(LPCWSTR pszUrl, ATL::CString* pstrProtocol, ATL::CString* pstrHost, ATL::CString* pstrPath);

private:
    AsyncSourceResolverState    m_state;
    ATL::CString                m_strOriginalUrl;
    ATL::CString                m_strResolvedUrl;
    ATL::CString                m_strProtocol;
    HRESULT                     m_hrAsyncResult;
    CComPtr<IMFMediaSource>     m_spResolvedSource;
    ResolvedSourceInfo          m_resolvedInfo;
    ResolveCompleteCallback     m_completeCb;

    CComPtr<IMFSourceResolver>  m_spResolver;
    HANDLE                      m_hResolveThread;
    HANDLE                      m_hResolveEvent;
    std::atomic<bool>           m_bCancelRequested;

    static DWORD WINAPI ResolveThreadProc(LPVOID lpParam);
    HRESULT ResolveOnBackgroundThread();
    HRESULT CreateSourceResolver();
    void DetectProtocol(LPCWSTR pszUrl);
    void SetState(AsyncSourceResolverState state);
};

} // namespace HMRAVSource

#endif // ASYNCSOURCERESOLVER_H
