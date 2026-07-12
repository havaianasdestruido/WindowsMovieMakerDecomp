/*
 * AVSourceFactory.h
 *
 * AVSourceFactory - COM class factory for creating AVSource instances.
 * Implements IClassFactory to create MFSource or DShowSource objects
 * based on the file type or media format requested.
 *
 * RTTI: ?AVAVSourceFactory@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AVSOURCEFACTORY_H
#define AVSOURCEFACTORY_H

#include "HMRAVSource.h"
#include "AVSource.h"
#include "MFSource.h"
#include "DShowSource.h"

namespace HMRAVSource
{

// ============================================================================
// AVSourceFactory
// ============================================================================
// COM class factory that creates AVSource instances. Determines the
// appropriate source type (MFSource for standard formats, DShowSource
// for legacy formats) based on the file extension or media type.
//
// Supports IClassFactory for COM registration and external creation.
//
class AVSOURCE_API AVSourceFactory :
    public ATL::CComObjectRootEx<ATL::CComMultiThreadModel>,
    public ATL::CComCoClass<AVSourceFactory, &CLSID_AVSourceFactory>,
    public IClassFactory
{
public:
    AVSourceFactory();
    virtual ~AVSourceFactory();

    DECLARE_REGISTRY_RESOURCEID(IDR_AVSOURCEFACTORY)
    DECLARE_NOT_AGGREGATABLE(AVSourceFactory)

    BEGIN_COM_MAP(AVSourceFactory)
        COM_INTERFACE_ENTRY(IClassFactory)
    END_COM_MAP()

    // IClassFactory
    STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
    STDMETHOD(LockServer)(BOOL fLock) override;

    // Factory helpers
    static HRESULT CreateInstance(AVSource** ppSource);
    static HRESULT CreateInstanceForFile(LPCWSTR pszFilePath, AVSource** ppSource);
    static bool IsDShowOnlyFormat(LPCWSTR pszFilePath);
    static bool IsNativeMFSourceFormat(LPCWSTR pszFilePath);

private:
    static LONG s_cLockCount;

    static bool IsLegacyVideoFormat(LPCWSTR pszExtension);
    static bool IsLegacyAudioFormat(LPCWSTR pszExtension);
    static VideoCodec DetectVideoCodec(LPCWSTR pszFilePath);
    static AudioCodec DetectAudioCodec(LPCWSTR pszFilePath);
};

} // namespace HMRAVSource

#endif // AVSOURCEFACTORY_H
