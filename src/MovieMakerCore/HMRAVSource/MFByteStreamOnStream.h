/*
 * MFByteStreamOnStream.h
 *
 * MFByteStreamOnStream - Adapts IStream to IMFByteStream for use with
 * Media Foundation source readers. Provides the byte-level access that
 * MF requires on top of COM structured storage streams.
 *
 * MFAsyncResult - Async result wrapper for MF asynchronous operations.
 * Wraps IMFSimpleAsyncResult for pending operation tracking.
 *
 * RTTI: ?AVMFByteStreamOnStream@@, ?AVMFAsyncResult@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef MFBYTESTREAMONSTREAM_H
#define MFBYTESTREAMONSTREAM_H

#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// MFByteStreamOnStream
// ============================================================================
// Adapts an IStream (COM structured storage) interface to IMFByteStream
// for use with Media Foundation APIs. MF requires IMFByteStream for
// source readers and other components that read from byte-level sources.
//
class AVSOURCE_API MFByteStreamOnStream :
    public IMFByteStream,
    public IUnknown
{
public:
    MFByteStreamOnStream();
    virtual ~MFByteStreamOnStream();

    static HRESULT CreateInstance(IStream* pStream, IMFByteStream** ppByteStream);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IMFByteStream
    STDMETHOD(GetCapabilities)(DWORD* pdwCapabilities) override;
    STDMETHOD(GetLength)(QWORD* pqwLength) override;
    STDMETHOD(SetLength)(QWORD qwLength) override;
    STDMETHOD(GetCurrentPosition)(QWORD* pqwPosition) override;
    STDMETHOD(SetCurrentPosition)(QWORD qwPosition) override;
    STDMETHOD(IsEndOfStream)(BOOL* pfEndOfStream) override;
    STDMETHOD(Read)(BYTE* pb, ULONG cb, ULONG* pcbRead) override;
    STDMETHOD(BeginRead)(BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) override;
    STDMETHOD(EndRead)(IMFAsyncResult* pResult, ULONG* pcbRead) override;
    STDMETHOD(Write)(const BYTE* pb, ULONG cb, ULONG* pcbWritten) override;
    STDMETHOD(BeginWrite)(const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState) override;
    STDMETHOD(EndWrite)(IMFAsyncResult* pResult, ULONG* pcbWritten) override;
    STDMETHOD(Seek)(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, DWORD dwSeekFlags, QWORD* pqwCurrentPosition) override;
    STDMETHOD(Flush)() override;

    // IStream access
    HRESULT GetStream(IStream** ppStream);

private:
    LONG            m_cRef;
    CComPtr<IStream> m_spStream;
    QWORD           m_qwPosition;
    CRITICAL_SECTION m_csLock;

    HRESULT EnsureStream();
};

// ============================================================================
// MFAsyncResult
// ============================================================================
// Wraps IMFSimpleAsyncResult for tracking pending asynchronous MF
// operations. Provides a synchronous wait mechanism for operations
// that are initiated asynchronously.
//
class AVSOURCE_API MFAsyncResult :
    public IMFAsyncResult,
    public IUnknown
{
public:
    MFAsyncResult();
    virtual ~MFAsyncResult();

    static HRESULT CreateInstance(IUnknown* pUnkObject, IUnknown* pUnkState, IMFAsyncCallback* pCallback, MFAsyncResult** ppResult);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IMFAsyncResult
    STDMETHOD(GetState)(IUnknown** ppunkState) override;
    STDMETHOD(GetStatus)() override;
    STDMETHOD(SetStatus)(HRESULT hrStatus) override;
    STDMETHOD(GetObject)(IUnknown** ppunkObject) override;

    // Completion
    HRESULT SetAsyncResult(HRESULT hr);
    HRESULT Wait(DWORD dwTimeoutMs = INFINITE);
    HRESULT GetAsyncResult(HRESULT* phr);
    void Reset();

    // Callback invocation
    HRESULT InvokeCallback();

private:
    LONG                m_cRef;
    HRESULT             m_hrStatus;
    CComPtr<IUnknown>   m_spObject;
    CComPtr<IUnknown>   m_spState;
    IMFAsyncCallback*   m_pCallback;
    HANDLE              m_hEvent;
    bool                m_fCompleted;
};

} // namespace HMRAVSource

#endif // MFBYTESTREAMONSTREAM_H
