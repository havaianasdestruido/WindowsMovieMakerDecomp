/*
 * pch.h
 *
 * Precompiled header for MovieMakerCore.dll.
 * Includes the full set of Windows SDK, ATL, WTL, GDI+, Direct3D,
 * Media Foundation, and project headers used throughout the DLL.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef MOVIECORE_PCH_H
#define MOVIECORE_PCH_H

// ============================================================================
// Target version
// ============================================================================
#define WINVER          0x0602
#define _WIN32_WINNT    0x0602
#define _WIN32_IE       0x0800
#define NTDDI_VERSION   0x06020000

// ============================================================================
// Compile definitions
// ============================================================================
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _WINDLL
#define _AFXDLL
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_ALL_WARNINGS

// ============================================================================
// Windows SDK
// ============================================================================
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <ole2.h>
#include <olectl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <commctrl.h>
#include <commdlg.h>
#include <prsht.h>
#include <unknwn.h>
#include <exdisp.h>
#include <mshtmhst.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <strsafe.h>
#include <process.h>
#include <tchar.h>
#include <appmodel.h>
#include <versionhelpers.h>
#include <synchapi.h>

// ============================================================================
// COM / OLE
// ============================================================================
#include <comdef.h>
#include <comsvcs.h>

// ============================================================================
// ATL (Active Template Library)
// ============================================================================
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlconv.h>
#include <atlsync.h>

// ============================================================================
// WTL (Windows Template Library) - from NuGet WTL 10
// ============================================================================
#include <atlapp.h>
extern CAppModule _Module;
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atldlgs.h>
#include <atlframe.h>
#include <atlmisc.h>
#include <atlprint.h>
#include <atlscrl.h>
#include <atlsplit.h>

// ============================================================================
// GDI+
// ============================================================================
#include <gdiplus.h>

// ============================================================================
// Direct2D / DirectWrite
// ============================================================================
#include <d2d1.h>
#include <d2d1helper.h>
#include <d2d1effects.h>
#include <dwrite.h>

// ============================================================================
// Direct3D 11 / Direct3D 9 / D3D Compiler
// ============================================================================
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d3d9.h>
#include <d3dcompiler.h>
#include <dxva2api.h>

// ============================================================================
// Media Foundation
// ============================================================================
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mfmediaengine.h>
#include <mfobjects.h>
#include <mfplay.h>
#include <evr.h>
#include <evr9.h>
#include <wmcodecdsp.h>

// ============================================================================
// Desktop Window Manager / Visual Styles
// ============================================================================
#include <dwmapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <vsstyle.h>

// ============================================================================
// Windows Imaging Component (WIC)
// ============================================================================
#include <wincodec.h>
#include <wincodecsdk.h>

// ============================================================================
// XML
// ============================================================================
#include <xmllite.h>

// XmlWriterProperty_ProcessNamespaces was removed from Win10 SDK
#ifndef XmlWriterProperty_ProcessNamespaces
#define XmlWriterProperty_ProcessNamespaces ((XmlWriterProperty)1)
#endif

// ============================================================================
// Accessibility / Property System
// ============================================================================
#include <oleacc.h>
#include <propkey.h>
#include <propvarutil.h>
#include <propsys.h>

// ============================================================================
// ESE (Extensible Storage Engine)
// ============================================================================
#include <esent.h>

// ============================================================================
// Multimedia
// ============================================================================
#include <mmsystem.h>

// ============================================================================
// C++ Standard Library
// ============================================================================
#include <new>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <deque>
#include <queue>
#include <array>
#include <algorithm>
#include <functional>
#include <utility>
#include <type_traits>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <cassert>
#include <cwchar>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <initializer_list>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

// ============================================================================
// Project headers
// ============================================================================
#include "common.h"
#include "exports.h"
#include "MovieMakerCore.h"
#include "WLXPhotoBase.h"

// ============================================================================
// PCH guard verification
// ============================================================================
#ifdef _MSC_VER
    #pragma warning(disable: 4505) // unreferenced local function has been removed
    #pragma warning(disable: 4100) // unreferenced formal parameter
    #pragma warning(disable: 4127) // conditional expression is constant
#endif

// ============================================================================
// SDK compatibility definitions
// This project targets Windows 6.2+ (Win8+) and was originally built with
// the Windows 8 SDK. The Windows 10 SDK (10.0.26100.0) removed or moved
// several constants, GUIDs, and interfaces. These definitions restore them.
// ============================================================================

// Need INITGUID so DEFINE_GUID creates actual definitions (not just declarations)
#include <initguid.h>

// --- MF attribute GUIDs removed from Win10 SDK ---
#ifndef MF_MT_BLOCK_ALIGNMENT
DEFINE_GUID(MF_MT_BLOCK_ALIGNMENT,
    0x3afd0ccd, 0x541f, 0x4c5c, 0xb1, 0x2e, 0x0c, 0x5d, 0xcb, 0xc5, 0xb1, 0x1b);
#endif

#ifndef MF_MT_AUDIO_AVG_BYTES_PER_SECTION
DEFINE_GUID(MF_MT_AUDIO_AVG_BYTES_PER_SECTION,
    0x79912f02, 0x39f2, 0x4791, 0x99, 0x67, 0x63, 0x31, 0x84, 0x24, 0xcf, 0x8e);
#endif

// --- MF video format GUIDs removed from Win10 SDK ---
#ifndef MFVideoFormat_WMVVC1
DEFINE_GUID(MFVideoFormat_WMVVC1,
    0x31435657, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
#endif

// --- MF error codes removed from Win10 SDK ---
#ifndef MF_E_TIMEOUT
#define MF_E_TIMEOUT ((HRESULT)0xC00D3E80L)
#endif

#ifndef MF_E_END_OFSTREAM
#define MF_E_END_OFSTREAM ((HRESULT)0xC00D3E84L)
#endif

#ifndef MF_S_TRANSFORM_STREAM_CHANGE
#define MF_S_TRANSFORM_STREAM_CHANGE ((HRESULT)0x000D6D63L)
#endif

// --- MF_OBJECT_TYPE enum value (old name removed from Win10 SDK) ---
#ifndef MF_OBJECT_UNKNOWN
#define MF_OBJECT_UNKNOWN ((MF_OBJECT_TYPE)0)
#endif

// --- IMFByteStream capability flags removed from Win10 SDK ---
#ifndef MFBYTESTREAM_IS_LOADING
#define MFBYTESTREAM_IS_LOADING 0x00000100
#endif

#ifndef MFBYTESTREAM_IS_SEEKING
#define MFBYTESTREAM_IS_SEEKING 0x00000400
#endif

// --- MFVP_MESSAGE_TYPE values removed from Win10 SDK ---
#ifndef MFVP_MESSAGE_DISPLAYSAMPLE
#define MFVP_MESSAGE_DISPLAYSAMPLE ((DWORD)4)
#endif

// --- CLSIDs removed from Win10 SDK ---
#ifndef __CLSID_SampleGrabber_DEFINED
#define __CLSID_SampleGrabber_DEFINED
// {C1F400A0-3F10-11D3-9F00-00C04F79A0A3}
DEFINE_GUID(CLSID_SampleGrabber,
    0xc1f400a0, 0x3f10, 0x11d3, 0x9f, 0x00, 0x00, 0xc0, 0x4f, 0x79, 0xa0, 0xa3);
#endif

#ifndef CLSID_CaptureEngineManager
// {E4A8692E-50C0-42FE-8CE0-2F66B28BEAF1}
DEFINE_GUID(CLSID_CaptureEngineManager,
    0xe4a8692e, 0x50c0, 0x42fe, 0x8c, 0xe0, 0x2f, 0x66, 0xb2, 0x8b, 0xea, 0xf1);
#endif

#ifndef CLSID_VideoProcessorMFT
DEFINE_GUID(CLSID_VideoProcessorMFT,
    0x88753b26, 0x5b24, 0x49bd, 0xb2, 0xe7, 0x0c, 0x44, 0x5c, 0x78, 0xc9, 0x82);
#endif

// --- MF audio resampler CLSID (removed from Win10 SDK) ---
#ifndef CLSID_CResamplerMediaObject
DEFINE_GUID(CLSID_CResamplerMediaObject,
    0xf447b69e, 0x128a, 0x4994, 0x83, 0x23, 0x76, 0x7c, 0x0b, 0x22, 0x1a, 0xe1);
#endif

#ifndef MFT_RESAMPLE_QUALITY
#define MFT_RESAMPLE_QUALITY 0
#endif

// --- XmlLite enum values removed from Win10 SDK ---
#ifndef XmlNodeTypeEndElement
#define XmlNodeTypeEndElement XmlNodeType_EndElement
#endif

#ifndef XmlWriterProperty_ImplicitFragmentEscape
#define XmlWriterProperty_ImplicitFragmentEscape ((UINT)6)
#endif

// --- MF_SOURCE_READER_DISABLE_THINNING removed from Win10 SDK ---
#ifndef MF_SOURCE_READER_DISABLE_THINNING
DEFINE_GUID(MF_SOURCE_READER_DISABLE_THINNING,
    0x4164f65f, 0x5793, 0x4bf2, 0xb2, 0x95, 0x34, 0xab, 0x4e, 0x48, 0x79, 0x43);
#endif

// --- DXVA2 video processing interfaces removed from Win10 SDK ---
// IDXVA2VideoProcessor and IDXVA2VideoProcessorEnumerator were removed
// from dxva2api.h in the Windows 10 SDK. These need the Windows 8.1 SDK
// dxva2api.h or a standalone forward declaration with full vtable layout.
// For now, provide minimal forward declarations for CComPtr usage.
#ifndef __IDXVA2VideoProcessor_INTERFACE_DEFINED__
#define __IDXVA2VideoProcessor_INTERFACE_DEFINED__
MIDL_INTERFACE("FA5CB1C0-08BE-46AA-B0AC-036B8D4F33D0")
IDXVA2VideoProcessor : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorCaps(
        void *pCaps) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetProcAmpRange(
        UINT ProcAmpCap, void *pRange) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFilterModeRange(
        UINT FilterCaps, void *pRange) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetProcAmpValues(
        UINT ProcAmpCap, const void *pRange) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetFilterModeRange(
        UINT FilterCaps, const void *pRange) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSurface(
        UINT OutputPlane, UINT SurfaceIndex, IDirect3DSurface9 **ppSurface) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorRenderTargetWidth(
        UINT *pWidth) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorRenderTargetHeight(
        UINT *pHeight) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetRenderTargetWidth(
        UINT Width) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetRenderTargetHeight(
        UINT Height) = 0;
    virtual HRESULT STDMETHODCALLTYPE ProcessBlt(
        IDirect3DSurface9 *pRenderTarget,
        const void *pBltParams,
        const void *pSamples,
        UINT NumSamples,
        void *pHandleComplete,
        void *pReserved) = 0;
};
#endif

#ifndef __IDXVA2VideoProcessorEnumerator_INTERFACE_DEFINED__
#define __IDXVA2VideoProcessorEnumerator_INTERFACE_DEFINED__
MIDL_INTERFACE("FA5CB1C1-08BE-46AA-B0AC-036B8D4F33D0")
IDXVA2VideoProcessorEnumerator : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE CreateVideoProcessor(
        IDXVA2VideoProcessor **ppVideoProcessor) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorRenderTargetCount(
        UINT *pCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorRenderTargets(
        UINT Count, void *pFormats, void *pSubTypes) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetProcAmpRangeCount(
        UINT *pCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetProcAmpRange(
        UINT ProcAmpCap, void *pRange) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFilterModeRangeCount(
        UINT *pCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFilterModeRange(
        UINT FilterCaps, void *pRange) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVideoProcessorCaps(
        void *pCaps) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFilterPropertyCount(
        UINT *pCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFilterProperty(
        UINT WhichProperty, UINT *pPropertyValue) = 0;
};
#endif

// --- DXVA2 video processor creation functions removed from Win10 SDK ---
// Implemented in dxva2stubs.cpp
STDAPI DXVA2CreateVideoProcessorEnumerator(
    const void *pVideoDesc,
    IDXVA2VideoProcessorEnumerator **ppVideoProcessorEnumerator);

STDAPI DXVA2CreateVideoProcessor(
    IDirect3DDevice9 *pDD,
    IDXVA2VideoProcessorEnumerator *pVideoProcessorEnumerator,
    void *pVPCaps,
    IDXVA2VideoProcessor **ppVideoProcessor);

// --- DXVA2 custom types not present in the Win10 SDK ---
// These types were used by the original DXVA2 video processing pipeline.

// Video processor device capability flag
#define DXVA2_VPDev_HardwareDeinterlaceOrScaling 0x00000001

typedef struct _DXVA2_Ratio
{
    UINT Numerator;
    UINT Denominator;
} DXVA2_Ratio;

typedef struct _DXVA2_VideoProcessorDesc
{
    UINT            DeviceFrameWidth;
    UINT            DeviceFrameHeight;
    GUID            DeviceGUID;
    UINT            Usage;
    UINT            TargetFrameWidth;
    UINT            TargetFrameHeight;
    UINT            TargetMinWidth;
    UINT            TargetMinHeight;
    UINT            TargetMaxWidth;
    UINT            TargetMaxHeight;
    UINT            TargetProcFreqNumerator;
    UINT            TargetProcFreqDenominator;
    UINT            RateControlFreqNumerator;
    UINT            RateControlFreqDenominator;
    DXVA2_Ratio     FrameRate;
    UINT            Uid;
} DXVA2_VideoProcessorDesc;

typedef struct _DXVA2_VideoProcessBltParameters
{
    DXVA2_Fixed32   TargetFrame;
    RECT            TargetRect;
    RECT            SourceRect;
    RECT            StreamRect;
    COLORREF        BackgroundColor;
    DXVA2_Fixed32   Alpha;
    LONGLONG        NoiseFilterTimeLapse;
    LONGLONG        DeinterlaceProcessingTimeStamp;
    LONGLONG        OutputProcessTimeStamp;
} DXVA2_VideoProcessBltParameters;

inline DXVA2_Fixed32 DXVA2_Fixed32Opaque()
{
    DXVA2_Fixed32 f = {};
    f.Fraction = 0;
    f.Value = 1;
    return f;
}

inline DXVA2_Fixed32 DXVA2_Fixed32FromDouble(double d)
{
    DXVA2_Fixed32 f = {};
    f.Fraction = static_cast<WORD>((d - static_cast<int>(d)) * 65536.0);
    f.Value = static_cast<SHORT>(static_cast<int>(d));
    return f;
}

// ============================================================================
// XmlLite helper — IXmlReader has no GetAttribute method; use
// MoveToAttributeByName + GetValue instead.
// ============================================================================
inline HRESULT XmlReaderGetAttribute(IXmlReader* pReader, LPCWSTR pwszLocalName, LPCWSTR* ppValue)
{
    if (!pReader || !pwszLocalName || !ppValue)
        return E_INVALIDARG;
    *ppValue = nullptr;
    HRESULT hr = pReader->MoveToAttributeByName(pwszLocalName, nullptr);
    if (SUCCEEDED(hr))
        hr = pReader->GetValue(ppValue, nullptr);
    pReader->MoveToElement();
    return hr;
}

#endif // MOVIECORE_PCH_H
