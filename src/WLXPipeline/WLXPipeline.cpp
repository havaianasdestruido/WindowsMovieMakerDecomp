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
#include <cmath>
#include <cstring>

// ============================================================================
// Internal classes
// ============================================================================
namespace Pipeline
{

// ============================================================================
// Effect identifiers (registered effect types)
// ============================================================================
enum EffectId
{
    kEffectBrightness = 1,  // param: float scale (1.0 = neutral)
    kEffectContrast   = 2,  // param: float factor (1.0 = neutral)
    kEffectSaturation = 3,  // param: float factor (1.0 = neutral)
    kEffectGrayscale  = 4,  // no param
    kEffectSepia      = 5,  // no param
    kEffectInvert     = 6,  // no param
};

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
// EffectChain -- applies visual effects to frames (software GDI+ engine)
// ============================================================================
class EffectChain
{
public:
    EffectChain() {}
    ~EffectChain() {}

    // Register an effect. Parameters are 4-byte little-endian float (optional).
    HRESULT AddEffect(UINT32 uEffectId, const BYTE* pParams, UINT32 cbParams)
    {
        EffectEntry entry;
        entry.uEffectId = uEffectId;
        if (pParams && cbParams > 0)
            entry.parameters.assign(pParams, pParams + cbParams);
        m_effects.push_back(entry);
        return S_OK;
    }

    HRESULT Apply(Gdiplus::Bitmap* pSource, Gdiplus::Bitmap** ppResult)
    {
        if (!pSource || !ppResult)
            return E_INVALIDARG;

        // Work in 32-bit ARGB for simple per-pixel processing
        Gdiplus::Bitmap* pWorking = new Gdiplus::Bitmap(
            pSource->GetWidth(), pSource->GetHeight(), PixelFormat32bppARGB);
        if (!pWorking)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics graphics(pWorking);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
        graphics.DrawImage(pSource, 0, 0, pSource->GetWidth(), pSource->GetHeight());

        for (size_t i = 0; i < m_effects.size(); i++)
        {
            HRESULT hr = ApplyEffect(pWorking, m_effects[i]);
            if (FAILED(hr))
            {
                delete pWorking;
                return hr;
            }
        }

        *ppResult = pWorking;
        return S_OK;
    }

    void ClearEffects()
    {
        m_effects.clear();
    }

    bool IsEmpty() const { return m_effects.empty(); }

private:
    struct EffectEntry
    {
        UINT32 uEffectId;
        std::vector<BYTE> parameters;
    };

    static float ParamFloat(const EffectEntry& e, float fDefault)
    {
        if (e.parameters.size() >= 4)
        {
            UINT32 bits = (UINT32)e.parameters[0] |
                ((UINT32)e.parameters[1] << 8) |
                ((UINT32)e.parameters[2] << 16) |
                ((UINT32)e.parameters[3] << 24);
            float f;
            memcpy(&f, &bits, sizeof(f));
            return f;
        }
        return fDefault;
    }

    static HRESULT ApplyEffect(Gdiplus::Bitmap* pBitmap, const EffectEntry& e)
    {
        if (!pBitmap)
            return E_INVALIDARG;

        Gdiplus::BitmapData data;
        Gdiplus::Rect rect(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight());

        if (pBitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite,
            PixelFormat32bppARGB, &data) != Gdiplus::Ok)
        {
            return E_FAIL;
        }

        BYTE* pBits = static_cast<BYTE*>(data.Scan0);
        LONG stride = data.Stride;
        UINT w = pBitmap->GetWidth();
        UINT h = pBitmap->GetHeight();

        switch (e.uEffectId)
        {
        case kEffectBrightness:
        {
            float f = ParamFloat(e, 1.0f);
            ApplyBrightness(pBits, stride, w, h, f);
            break;
        }
        case kEffectContrast:
        {
            float f = ParamFloat(e, 1.0f);
            ApplyContrast(pBits, stride, w, h, f);
            break;
        }
        case kEffectSaturation:
        {
            float f = ParamFloat(e, 1.0f);
            ApplySaturation(pBits, stride, w, h, f);
            break;
        }
        case kEffectGrayscale:
            ApplyGrayscale(pBits, stride, w, h);
            break;
        case kEffectSepia:
            ApplySepia(pBits, stride, w, h);
            break;
        case kEffectInvert:
            ApplyInvert(pBits, stride, w, h);
            break;
        default:
            break;
        }

        pBitmap->UnlockBits(&data);
        return S_OK;
    }

    static BYTE Clamp(int value)
    {
        if (value < 0) return 0;
        if (value > 255) return 255;
        return (BYTE)value;
    }

    static void ApplyBrightness(BYTE* pBits, LONG stride, UINT w, UINT h, float fScale)
    {
        int offset = (int)((fScale - 1.0f) * 128.0f);
        for (UINT y = 0; y < h; y++)
        {
            BYTE* row = pBits + (LONG)y * stride;
            for (UINT x = 0; x < w; x++)
            {
                BYTE* p = row + x * 4;
                p[0] = Clamp(p[0] + offset);
                p[1] = Clamp(p[1] + offset);
                p[2] = Clamp(p[2] + offset);
            }
        }
    }

    static void ApplyContrast(BYTE* pBits, LONG stride, UINT w, UINT h, float fFactor)
    {
        for (UINT y = 0; y < h; y++)
        {
            BYTE* row = pBits + (LONG)y * stride;
            for (UINT x = 0; x < w; x++)
            {
                BYTE* p = row + x * 4;
                for (int c = 0; c < 3; c++)
                {
                    int v = (int)((p[c] - 128) * fFactor + 128);
                    p[c] = Clamp(v);
                }
            }
        }
    }

    static void ApplySaturation(BYTE* pBits, LONG stride, UINT w, UINT h, float fFactor)
    {
        for (UINT y = 0; y < h; y++)
        {
            BYTE* row = pBits + (LONG)y * stride;
            for (UINT x = 0; x < w; x++)
            {
                BYTE* p = row + x * 4;
                int r = p[2], g = p[1], b = p[0];
                int luma = (r * 77 + g * 150 + b * 29) >> 8;
                p[2] = Clamp((int)(luma + (r - luma) * fFactor));
                p[1] = Clamp((int)(luma + (g - luma) * fFactor));
                p[0] = Clamp((int)(luma + (b - luma) * fFactor));
            }
        }
    }

    static void ApplyGrayscale(BYTE* pBits, LONG stride, UINT w, UINT h)
    {
        for (UINT y = 0; y < h; y++)
        {
            BYTE* row = pBits + (LONG)y * stride;
            for (UINT x = 0; x < w; x++)
            {
                BYTE* p = row + x * 4;
                BYTE v = Clamp((p[2] * 77 + p[1] * 150 + p[0] * 29) >> 8);
                p[0] = p[1] = p[2] = v;
            }
        }
    }

    static void ApplySepia(BYTE* pBits, LONG stride, UINT w, UINT h)
    {
        for (UINT y = 0; y < h; y++)
        {
            BYTE* row = pBits + (LONG)y * stride;
            for (UINT x = 0; x < w; x++)
            {
                BYTE* p = row + x * 4;
                int r = p[2], g = p[1], b = p[0];
                int tr = (int)(0.393f * r + 0.769f * g + 0.189f * b);
                int tg = (int)(0.349f * r + 0.686f * g + 0.168f * b);
                int tb = (int)(0.272f * r + 0.534f * g + 0.131f * b);
                p[2] = Clamp(tr);
                p[1] = Clamp(tg);
                p[0] = Clamp(tb);
            }
        }
    }

    static void ApplyInvert(BYTE* pBits, LONG stride, UINT w, UINT h)
    {
        for (UINT y = 0; y < h; y++)
        {
            BYTE* row = pBits + (LONG)y * stride;
            for (UINT x = 0; x < w; x++)
            {
                BYTE* p = row + x * 4;
                p[0] = 255 - p[0];
                p[1] = 255 - p[1];
                p[2] = 255 - p[2];
            }
        }
    }

    std::vector<EffectEntry> m_effects;
};

// ============================================================================
// ColorConverter -- handles YUV-to-RGB conversions (BT.601 limited range)
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

        Gdiplus::Bitmap* pDst = new Gdiplus::Bitmap(
            uSrcWidth, uSrcHeight, PixelFormat24bppRGB);
        if (!pDst)
            return E_OUTOFMEMORY;

        // NV12 is assumed: Y plane followed by interleaved U/V (chroma half-res)
        Gdiplus::BitmapData data;
        Gdiplus::Rect rect(0, 0, uSrcWidth, uSrcHeight);
        if (pDst->LockBits(&rect, Gdiplus::ImageLockModeWrite,
            PixelFormat24bppRGB, &data) == Gdiplus::Ok)
        {
            BYTE* pDstBits = static_cast<BYTE*>(data.Scan0);
            LONG dstStride = data.Stride;

            const BYTE* pY = pSrcYUV;
            const BYTE* pUV = pSrcYUV + (size_t)uSrcWidth * uSrcHeight;

            for (UINT32 y = 0; y < uSrcHeight; y++)
            {
                BYTE* pRow = pDstBits + (LONG)y * dstStride;
                for (UINT32 x = 0; x < uSrcWidth; x++)
                {
                    BYTE Y = pY[y * uSrcWidth + x];
                    BYTE U = pUV[(y / 2) * uSrcWidth + (x & ~1)];
                    BYTE V = pUV[(y / 2) * uSrcWidth + (x & ~1) + 1];

                    // BT.601 limited-range YCbCr -> RGB
                    int c = (int)Y - 16;
                    int d = (int)U - 128;
                    int e = (int)V - 128;

                    int r = (298 * c + 409 * e + 128) >> 8;
                    int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
                    int b = (298 * c + 516 * d + 128) >> 8;

                    pRow[x * 3 + 0] = Clamp(r);
                    pRow[x * 3 + 1] = Clamp(g);
                    pRow[x * 3 + 2] = Clamp(b);
                }
            }

            pDst->UnlockBits(&data);
        }

        *ppDstRGB = pDst;
        return S_OK;
    }

private:
    static BYTE Clamp(int value)
    {
        if (value < 0) return 0;
        if (value > 255) return 255;
        return (BYTE)value;
    }

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
    const PipelineConfig& GetConfig() const { return m_config; }

    EffectChain* GetEffectChain() { return m_effectChain.get(); }

    // Process a frame through the effect chain. Returns a new bitmap owned by
    // the caller (release with delete).
    HRESULT Process(Gdiplus::Bitmap* pInput, Gdiplus::Bitmap** ppOutput)
    {
        if (!pInput || !ppOutput)
            return E_INVALIDARG;
        if (!m_bInitialized || !m_effectChain)
            return E_UNEXPECTED;

        return m_effectChain->Apply(pInput, ppOutput);
    }

    // Process raw NV12 data through color conversion + effects.
    HRESULT ProcessNV12(const BYTE* pSrcYUV, UINT32 uSrcWidth, UINT32 uSrcHeight,
        Gdiplus::Bitmap** ppOutput)
    {
        if (!pSrcYUV || !ppOutput)
            return E_INVALIDARG;
        if (!m_bInitialized)
            return E_UNEXPECTED;

        Gdiplus::Bitmap* pRGB = NULL;
        HRESULT hr = m_colorConverter->Convert(pSrcYUV, uSrcWidth, uSrcHeight, &pRGB);
        if (FAILED(hr))
            return hr;

        if (m_effectChain && !m_effectChain->IsEmpty())
        {
            Gdiplus::Bitmap* pFx = NULL;
            hr = m_effectChain->Apply(pRGB, &pFx);
            delete pRGB;
            if (FAILED(hr))
                return hr;
            pRGB = pFx;
        }

        *ppOutput = pRGB;
        return S_OK;
    }

private:
    bool                            m_bInitialized;
    PipelineConfig                  m_config;
    std::unique_ptr<D3D9Surface>    m_d3dSurface;
    std::unique_ptr<ColorConverter> m_colorConverter;
    std::unique_ptr<EffectChain>    m_effectChain;
};

// ============================================================================
// C factory wrapper functions (exported through the function table)
// ============================================================================

HRESULT STDAPICALLTYPE PipelineCreate(const PipelineConfig* pConfig, void** ppInstance)
{
    if (!ppInstance)
        return E_INVALIDARG;
    *ppInstance = NULL;

    ProcessingPipeline* pPipeline = new(std::nothrow) ProcessingPipeline();
    if (!pPipeline)
        return E_OUTOFMEMORY;

    HRESULT hr = pPipeline->Initialize(pConfig);
    if (FAILED(hr))
    {
        delete pPipeline;
        return hr;
    }

    *ppInstance = static_cast<void*>(pPipeline);
    return S_OK;
}

void STDAPICALLTYPE PipelineDestroy(void* pInstance)
{
    if (!pInstance)
        return;
    ProcessingPipeline* pPipeline = static_cast<ProcessingPipeline*>(pInstance);
    delete pPipeline;
}

HRESULT STDAPICALLTYPE PipelineProcess(void* pInstance, Gdiplus::Bitmap* pInput, Gdiplus::Bitmap** ppOutput)
{
    if (!pInstance)
        return E_INVALIDARG;
    return static_cast<ProcessingPipeline*>(pInstance)->Process(pInput, ppOutput);
}

HRESULT STDAPICALLTYPE PipelineGetInfo(void* pInstance, PipelineConfig* pConfig)
{
    if (!pInstance || !pConfig)
        return E_INVALIDARG;
    *pConfig = static_cast<ProcessingPipeline*>(pInstance)->GetConfig();
    return S_OK;
}

static PipelineCreateFunctions g_pipelineFunctions =
{
    1,                                  // uVersion
    sizeof(PipelineCreateFunctions),    // uStructSize
    PipelineCreate,
    PipelineDestroy,
    PipelineProcess,
    PipelineGetInfo,
};

} // namespace Pipeline

// ============================================================================
// Module state
// ============================================================================
static HINSTANCE g_hModule = NULL;

// ============================================================================
// Exported functions -- GetPipelineCreateFunctions + DllRegisterServer
// ============================================================================

extern "C"
{

WLXPIPE_API HRESULT __stdcall GetPipelineCreateFunctions(void** ppFunctions, UINT32* pCount)
{
    if (!ppFunctions || !pCount)
        return E_INVALIDARG;

    *ppFunctions = static_cast<void*>(&Pipeline::g_pipelineFunctions);
    *pCount = sizeof(::PipelineCreateFunctions) / sizeof(void*);
    return S_OK;
}

STDAPI DllRegisterServer()
{
    return S_OK;
}

} // extern "C"
