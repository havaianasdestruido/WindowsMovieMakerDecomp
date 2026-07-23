#include "pch.h"

// DXVA2 stubs for Win10 SDK compatibility.
// dxva2.h was removed from the Windows 10 SDK; these stubs provide
// no-op implementations of the DXVA2 creation functions.
// Type definitions are in pch.h.

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
