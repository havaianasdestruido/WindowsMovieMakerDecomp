#include "pch.h"

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
