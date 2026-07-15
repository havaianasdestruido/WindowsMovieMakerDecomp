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

// {D45D4E50-6C4F-4F65-8B4F-B5E2C4E0A027}
DEFINE_GUID(CLSID_AVSourceFactory,
    0xD45D4E50, 0x6C4F, 0x4F65, 0x8B, 0x4F, 0xB5, 0xE2, 0xC4, 0xE0, 0xA0, 0x27);

#ifndef IDR_AVSOURCEFACTORY
#define IDR_AVSOURCEFACTORY 10300
#endif

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
