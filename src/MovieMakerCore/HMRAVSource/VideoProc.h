/*
 * VideoProc.h
 *
 * VideoProc - Video processing pipeline for color conversion, scaling,
 * and hardware-accelerated operations.
 *
 * XVideoProc - Base video processing using software transforms.
 * DXVA2VideoProc - DXVA2 hardware-accelerated video processing.
 *
 * RTTI: ?AVXVideoProc@@, ?AVDXVA2VideoProc@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef VIDEOPROC_H
#define VIDEOPROC_H

#include "HMRAVSource.h"
#include <dxva2api.h>

namespace HMRAVSource
{

// ============================================================================
// VideoProcDesc
// ============================================================================
struct VideoProcDesc
{
    UINT        uInputWidth;
    UINT        uInputHeight;
    GUID        guidInputSubtype;
    UINT        uOutputWidth;
    UINT        uOutputHeight;
    GUID        guidOutputSubtype;
    bool        fDeinterlace;
    bool        fDenoise;
    bool        fBrightness;
    float       flBrightness;       // -1.0 to 1.0
    float       flContrast;         // 0.0 to 2.0
    float       flSaturation;       // 0.0 to 3.0

    VideoProcDesc()
        : uInputWidth(0)
        , uInputHeight(0)
        , guidInputSubtype(MFVideoFormat_NV12)
        , uOutputWidth(0)
        , uOutputHeight(0)
        , guidOutputSubtype(MFVideoFormat_NV12)
        , fDeinterlace(false)
        , fDenoise(false)
        , fBrightness(false)
        , flBrightness(0.0f)
        , flContrast(1.0f)
        , flSaturation(1.0f)
    {
    }
};

// ============================================================================
// XVideoProc
// ============================================================================
// Base video processing using software MF transforms. Handles color
// conversion, scaling, and basic image adjustments using CPU-based
// MFT transforms.
//
class AVSOURCE_API XVideoProc
{
public:
    XVideoProc();
    virtual ~XVideoProc();

    // Lifecycle
    HRESULT Initialize(const VideoProcDesc& desc);
    virtual HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Processing
    virtual HRESULT ProcessFrame(IMFSample* pInputSample, IMFSample** ppOutputSample);
    virtual HRESULT ProcessFrameToSurface(IMFSample* pInputSample, IDirect3DSurface9* pOutputSurface);

    // Format configuration
    HRESULT SetOutputFormat(UINT uWidth, UINT uHeight, const GUID* pSubtype);
    HRESULT GetOutputFormat(UINT* puWidth, UINT* puHeight, GUID* pSubtype);

    // Image adjustments
    HRESULT SetBrightness(float flBrightness);
    HRESULT SetContrast(float flContrast);
    HRESULT SetSaturation(float flSaturation);
    HRESULT SetDeinterlace(bool fEnable);
    HRESULT SetDenoise(bool fEnable);

    // Transform chain
    HRESULT AddTransform(REFGUID guidTransformClsid);
    HRESULT RemoveTransform(DWORD dwIndex);
    DWORD GetTransformCount() const throw();

    // Statistics
    DWORD GetFramesProcessed() const throw();

protected:
    VideoProcDesc       m_desc;
    bool                m_fInitialized;
    DWORD               m_dwFramesProcessed;

    CComPtr<IMFTransform> m_spColorConverter;
    CComPtr<IMFTransform> m_spScaler;
    ATL::CAtlArray<CComPtr<IMFTransform>> m_extraTransforms;

    HRESULT CreateColorConverter();
    HRESULT CreateScaler();
    HRESULT ConfigureTransformChain();
    HRESULT ProcessThroughChain(IMFSample* pInput, IMFSample** ppOutput);
};

// ============================================================================
// DXVA2VideoProc
// ============================================================================
// DXVA2 hardware-accelerated video processing. Uses the DirectX Video
// Acceleration 2 API for GPU-based color conversion, scaling, deinterlacing,
// and image enhancement with hardware acceleration.
//
class AVSOURCE_API DXVA2VideoProc : public XVideoProc
{
public:
    DXVA2VideoProc();
    virtual ~DXVA2VideoProc();

    // Lifecycle
    HRESULT Initialize(const VideoProcDesc& desc, IDirect3DDevice9* pDevice);
    HRESULT Shutdown() override;

    // Override - hardware-accelerated processing
    HRESULT ProcessFrame(IMFSample* pInputSample, IMFSample** ppOutputSample) override;
    HRESULT ProcessFrameToSurface(IMFSample* pInputSample, IDirect3DSurface9* pOutputSurface) override;

    // DXVA2 specific
    HRESULT GetVideoProcessorCaps(DXVA2_VideoProcessorCaps* pCaps);
    HRESULT SetRenderTarget(IDirect3DSurface9* pRenderTarget);
    HRESULT EnableH264Deinterlace(bool fEnable);

    // Device management
    HRESULT SetDevice(IDirect3DDevice9* pDevice);
    IDirect3DDevice9* GetDevice() const;

    // Feature support
    static bool IsDXVA2Available();
    static bool IsHardwareDeinterlaceSupported(IDirect3DDevice9* pDevice);
    static bool IsHardwareScalingSupported(IDirect3DDevice9* pDevice);

private:
    IDirect3DDevice9Ex*                      m_pDevice;
    IDXVA2VideoProcessor*                    m_pVideoProcessor;
    CComPtr<IDXVA2VideoProcessorEnumerator> m_spEnumerator;
    DXVA2_VideoProcessorCaps                m_vpcaps;
    D3DPRESENT_PARAMETERS                   m_d3dpp;
    CComPtr<IDirect3DSurface9>              m_spRenderTarget;
    CComPtr<IDirect3DSurface9>              m_spTempSurface;
    bool                                    m_fH264Deinterlace;

    HRESULT CreateDXVA2Processor();
    HRESULT CreateTempSurface();
    HRESULT ProcessSampleDXVA2(IMFSample* pInput, IMFSample** ppOutput);
    HRESULT ConvertSampleToSurface(IMFSample* pSample, IDirect3DSurface9* pSurface);
    HRESULT ConvertSurfaceToSample(IDirect3DSurface9* pSurface, IMFSample** ppSample);
};

} // namespace HMRAVSource

#endif // VIDEOPROC_H
