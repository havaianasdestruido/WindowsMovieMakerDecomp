#include "pch.h"

// DXVA2 type definitions and stubs for Win10 SDK compatibility.
// dxva2.h was removed from the Windows 10 SDK; these stubs allow
// the decompiled code to reference the DXVA2 types and functions.

#ifndef __dxva2api_h__
#define __dxva2api_h__

typedef struct _DXVA2_Fixed32
{
    union
    {
        DWORD   dw32;
        struct
        {
            WORD    fraction;
            SHORT   value;
        };
    };
} DXVA2_Fixed32;

typedef enum _DXVA2_VPDevCaps
{
    DXVA2_VPDev_HardwareDeinterlaceOrScaling = 0x00000001
} DXVA2_VPDevCaps;

typedef struct _DXVA2_VideoProcessorDesc
{
    UINT            DeviceFrameWidth;
    UINT            DeviceFrameHeight;
    GUID            DeviceGUID;
    DXVA2_VPDevCaps Usage;
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
    DXVA2_Fixed32   FrameRate;
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

static inline DXVA2_Fixed32 DXVA2_Fixed32Opaque()
{
    DXVA2_Fixed32 f;
    f.dw32 = 0x00010000;
    return f;
}

#endif // __dxva2api_h__

extern "C" {

STDAPI DXVA2CreateVideoProcessorEnumerator(
    const void* pVideoDesc,
    IDXVA2VideoProcessorEnumerator** ppVideoProcessorEnumerator)
{
    return E_NOTIMPL;
}

STDAPI DXVA2CreateVideoProcessor(
    IDirect3DDevice9* pDD,
    IDXVA2VideoProcessorEnumerator* pVideoProcessorEnumerator,
    void* pVPCaps,
    IDXVA2VideoProcessor** ppVideoProcessor)
{
    return E_NOTIMPL;
}

} // extern "C"
