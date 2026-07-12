/*
 * WLXPipeline.cpp
 *
 * Implementation of WLXPipeline.dll -- video processing pipeline for
 * Windows Live Movie Maker 2012.
 *
 * The pipeline coordinates frame processing through a configurable chain:
 *   1. FrameSource: pulls raw frames from decoders (MF, DirectShow, or file)
 *   2. EffectChain: applies visual effects, transitions, and overlays
 *   3. ColorConverter: handles YUV-to-RGB and format conversions via DMO/D3D9
 *   4. FrameSink: pushes processed frames to encoders or render targets
 *
 * Hardware processing uses D3D9 surfaces and DirectDraw for GPU-accelerated
 * color space conversion. Software fallback uses GDI+ Bitmap operations.
 * DMO (DirectX Media Objects) via msdmo.dll handles codec operations.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXPipeline.h"
#include "WLXPhotoBase.h"

#include <d3d9.h>
#include <ddraw.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

// ============================================================================
// Internal classes
// ============================================================================
namespace Pipeline
{

// ============================================================================
// D3D9Surface -- wrapper for Direct3D 9 render targets
// ============================================================================
class D3D9Surface
{
public:
    D3D9Surface()
        : m_pD3D(NULL)
        , m_pDevice(NULL)
        , m_pRenderTarget(NULL)
        , m_pDepthStencil(NULL)
        , m_bInitialized(false)
    {
    }

    ~D3D9Surface()
    {
        Release();
    }

    HRESULT Initialize(UINT32 uWidth, UINT32 uHeight, D3DFORMAT format)
    {
        HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D);
        if (FAILED(hr))
            return hr;

        D3DPRESENT_PARAMETERS pp = { 0 };
        pp.Windowed = TRUE;
        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.hDeviceWindow = GetDesktopWindow();
        pp.BackBufferWidth = uWidth;
        pp.BackBufferHeight = uHeight;
        pp.BackBufferFormat = format;
        pp.EnableAutoDepthStencil = TRUE;
        pp.AutoDepthStencilFormat = D3DFMT_D16;

        hr = m_pD3D->CreateDeviceEx(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            pp.hDeviceWindow,
            D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
            &pp,
            NULL,
            &m_pDevice);

        if (FAILED(hr))
        {
            // Fallback to software processing
            pp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
            pp.EnableAutoDepthStencil = FALSE;

            hr = m_pD3D->CreateDeviceEx(
                D3DADAPTER_DEFAULT,
                D3DDEVTYPE_HAL,
                pp.hDeviceWindow,
                D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                &pp,
                NULL,
                &m_pDevice);
        }

        if (SUCCEEDED(hr))
        {
            m_uWidth = uWidth;
            m_uHeight = uHeight;
            m_format = format;
            m_bInitialized = true;
        }

        return hr;
    }

    void Release()
    {
        if (m_pRenderTarget)  { m_pRenderTarget->Release();    m_pRenderTarget = NULL; }
        if (m_pDepthStencil)  { m_pDepthStencil->Release();    m_pDepthStencil = NULL; }
        if (m_pDevice)        { m_pDevice->Release();          m_pDevice = NULL; }
        if (m_pD3D)           { m_pD3D->Release();             m_pD3D = NULL; }
        m_bInitialized = false;
    }

    bool IsInitialized() const { return m_bInitialized; }
    IDirect3DDevice9Ex* GetDevice() { return m_pDevice; }

private:
    IDirect3D9Ex*           m_pD3D;
    IDirect3DDevice9Ex*     m_pDevice;
    IDirect3DSurface9*      m_pRenderTarget;
    IDirect3DSurface9*      m_pDepthStencil;
    bool                    m_bInitialized;
    UINT32                  m_uWidth;
    UINT32                  m_uHeight;
    D3DFORMAT               m_format;
};

// ============================================================================
// FrameSource -- pulls raw video frames
// ============================================================================
class FrameSource
{
public:
    FrameSource() : m_llFrameIndex(0) {}
    virtual ~FrameSource() {}

    virtual HRESULT GetNextFrame(Gdiplus::Bitmap** ppBitmap) = 0;
    virtual HRESULT SeekToFrame(LONGLONG llFrameIndex) = 0;

    LONGLONG GetCurrentFrameIndex() const { return m_llFrameIndex; }

protected:
    LONGLONG m_llFrameIndex;
};

// ============================================================================
// FrameSink -- pushes processed frames
// ============================================================================
class FrameSink
{
public:
    FrameSink() {}
    virtual ~FrameSink() {}

    virtual HRESULT WriteFrame(const Gdiplus::Bitmap* pBitmap, LONGLONG llTimestamp) = 0;
    virtual HRESULT Flush() = 0;
};

// ============================================================================
// EffectChain -- applies visual effects to frames
// ============================================================================
class EffectChain
{
public:
    EffectChain() {}
    ~EffectChain() {}

    HRESULT Apply(Gdiplus::Bitmap* pSource, Gdiplus::Bitmap** ppResult)
    {
        if (!pSource || !ppResult)
            return E_INVALIDARG;

        // In the full implementation, this applies each registered effect
        // in sequence. Effects include: brightness, contrast, saturation,
        // blur, sharpen, black & white, sepia, etc.
        *ppResult = pSource->Clone(0, 0, pSource->GetWidth(), pSource->GetHeight(),
            pSource->GetPixelFormat());

        return S_OK;
    }

    void ClearEffects()
    {
        m_effects.clear();
    }

private:
    struct EffectEntry
    {
        UINT32 uEffectId;
        std::vector<BYTE> parameters;
    };

    std::vector<EffectEntry> m_effects;
};

// ============================================================================
// ColorConverter -- handles YUV-to-RGB conversions
// ============================================================================
class ColorConverter
{
public:
    ColorConverter()
        : m_bHardware(false)
    {
    }

    ~ColorConverter()
    {
    }

    HRESULT Initialize(bool bUseHardware)
    {
        m_bHardware = bUseHardware;
        return S_OK;
    }

    HRESULT Convert(const BYTE* pSrcYUV, UINT32 uSrcWidth, UINT32 uSrcHeight,
        Gdiplus::Bitmap** ppDstRGB)
    {
        if (!pSrcYUV || !ppDstRGB || uSrcWidth == 0 || uSrcHeight == 0)
            return E_INVALIDARG;

        // Software fallback: create RGB bitmap
        *ppDstRGB = new Gdiplus::Bitmap(uSrcWidth, uSrcHeight, PixelFormat24bppRGB);

        if (!*ppDstRGB)
            return E_OUTOFMEMORY;

        // In full implementation: use DMO color converter or D3D9 GPU shader
        return S_OK;
    }

private:
    bool m_bHardware;
};

// ============================================================================
// ProcessingPipeline -- main pipeline coordinator
// ============================================================================
class ProcessingPipeline
{
public:
    ProcessingPipeline()
        : m_bInitialized(false)
    {
    }

    ~ProcessingPipeline()
    {
        Shutdown();
    }

    HRESULT Initialize(const PipelineConfig* pConfig)
    {
        if (!pConfig)
            return E_INVALIDARG;

        m_config = *pConfig;

        m_colorConverter.reset(new ColorConverter());
        HRESULT hr = m_colorConverter->Initialize(pConfig->bUseHardwareProcessing);
        if (FAILED(hr))
            return hr;

        m_effectChain.reset(new EffectChain());

        if (pConfig->bUseHardwareProcessing)
        {
            m_d3dSurface.reset(new D3D9Surface());
            hr = m_d3dSurface->Initialize(
                pConfig->uOutputWidth, pConfig->uOutputHeight, pConfig->d3dFormat);
            // Hardware is optional; fall back to software if unavailable
            if (FAILED(hr))
                m_d3dSurface.reset();
        }

        m_bInitialized = true;
        return S_OK;
    }

    void Shutdown()
    {
        if (!m_bInitialized)
            return;

        m_effectChain.reset();
        m_colorConverter.reset();
        m_d3dSurface.reset();

        m_bInitialized = false;
    }

    bool IsInitialized() const { return m_bInitialized; }

private:
    bool                            m_bInitialized;
    PipelineConfig                  m_config;
    std::unique_ptr<D3D9Surface>    m_d3dSurface;
    std::unique_ptr<ColorConverter> m_colorConverter;
    std::unique_ptr<EffectChain>    m_effectChain;
};

} // namespace Pipeline

// ============================================================================
// Module state
// ============================================================================
static HINSTANCE g_hModule = NULL;

// ============================================================================
// Exported functions (2 exports)
// ============================================================================

extern "C"
{

WLXPIPE_API HANDLE __stdcall Pipeline_Create(const PipelineConfig* pConfig)
{
    Pipeline::ProcessingPipeline* pPipeline = new(std::nothrow) Pipeline::ProcessingPipeline();
    if (!pPipeline)
        return NULL;

    HRESULT hr = pPipeline->Initialize(pConfig);
    if (FAILED(hr))
    {
        delete pPipeline;
        return NULL;
    }

    return static_cast<HANDLE>(pPipeline);
}

WLXPIPE_API void __stdcall Pipeline_Destroy(HANDLE hPipeline)
{
    if (!hPipeline)
        return;
    delete static_cast<Pipeline::ProcessingPipeline*>(hPipeline);
}

} // extern "C"
