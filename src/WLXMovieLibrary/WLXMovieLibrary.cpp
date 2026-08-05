/*
 * WLXMovieLibrary.cpp
 *
 * Implementation of WLXMovieLibrary.dll -- movie library management
 * and Media Foundation integration for Windows Live Movie Maker 2012.
 *
 * The LibraryManager is a singleton that coordinates:
 *   - Library item management (add / remove / query) backed by an
 *     in-memory item store
 *   - Video file enumeration from known folders and project directories
 *   - Media property queries via Media Foundation source readers
 *   - Thumbnail generation from the first decoded video frame using
 *     the Video Processor MFT with a direct-to-RGB32 fallback
 *   - Hardware-accelerated video processing pipeline setup via a D3D9Ex
 *     device (surface blit / scale / readback)
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXMovieLibrary.h"
#include "WLXPhotoBase.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mfobjects.h>
#include <mftransform.h>
#include <propkey.h>
#include <propsys.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <d3d9.h>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <new>

// ============================================================================
// Internal helpers
// ============================================================================

// MF_E_NO_MORE_SAMPLES (0xC00DA123) was removed from the Win10 SDK mferror.h.
#ifndef MF_E_NO_MORE_SAMPLES
#define MF_E_NO_MORE_SAMPLES ((HRESULT)0xC00DA123L)
#endif

// D3DPMISCCAPS_COLORCONVERSION was removed from the Win10 SDK d3d9caps.h.
#ifndef D3DPMISCCAPS_COLORCONVERSION
#define D3DPMISCCAPS_COLORCONVERSION 0x00040000L
#endif

namespace MovieLibrary
{

namespace
{

// CLSID_VideoProcessorMFT ({88753B26-5B24-49BD-B2E7-0C445C78C982}) was
// removed from the Win10 SDK headers. Keep a private copy so the DLL does
// not depend on removed SDK GUID symbols at link time.
const GUID CLSID_VideoProcessorMFT_Local =
    { 0x88753b26, 0x5b24, 0x49bd, { 0xb2, 0xe7, 0x0c, 0x44, 0x5c, 0x78, 0xc9, 0x82 } };

// Extracts the FOURCC payload of an MF subtype GUID (e.g. 'H264', 'WMV3').
// Uncompressed formats yield the raw Data1 DWORD (e.g. 0x22 == RGB32).
UINT32 FourCCFromSubtype(REFGUID guidSubtype)
{
    return guidSubtype.Data1;
}

// Normalizes a path into a case-insensitive map/vector key. Does not touch
// the file system; CharUpperBuffW (user32) does the case folding.
void NormalizeKey(LPCWSTR pszPath, std::wstring& strOut)
{
    strOut.assign(pszPath ? pszPath : L"");
    if (!strOut.empty())
        CharUpperBuffW(&strOut[0], (DWORD)strOut.size());
}

// Bytes per pixel for formats the software blit path can copy. YUV and
// other formats return 0 (handled by the hardware/MFT paths instead).
UINT BytesPerPixel(D3DFORMAT d3dFormat)
{
    switch (d3dFormat)
    {
    case D3DFMT_A8:             return 1;
    case D3DFMT_L8:             return 1;
    case D3DFMT_R3G3B2:         return 1;
    case D3DFMT_R5G6B5:         return 2;
    case D3DFMT_A1R5G5B5:       return 2;
    case D3DFMT_X1R5G5B5:       return 2;
    case D3DFMT_A4R4G4B4:       return 2;
    case D3DFMT_A8R3G3B2:       return 2;
    case D3DFMT_A8L8:           return 2;
    case D3DFMT_R8G8B8:         return 3;
    case D3DFMT_A8R8G8B8:       return 4;
    case D3DFMT_X8R8G8B8:       return 4;
    case D3DFMT_A8B8G8R8:       return 4;
    case D3DFMT_X8B8G8R8:       return 4;
    case D3DFMT_A2B10G10R10:    return 4;
    case D3DFMT_A2R10G10B10:    return 4;
    case D3DFMT_G16R16:         return 4;
    case D3DFMT_A16B16G16R16:   return 8;
    default:                    return 0;
    }
}

} // namespace

// ============================================================================
// D3D9VPContext -- describes one configured D3D9 video processing session
// ============================================================================
struct D3D9VPContext
{
    UINT       uInputWidth;
    UINT       uInputHeight;
    D3DFORMAT  d3dInputFormat;
    UINT       uOutputWidth;
    UINT       uOutputHeight;
    D3DFORMAT  d3dOutputFormat;
    BOOL       bColorConversion;

    D3D9VPContext()
        : uInputWidth(0)
        , uInputHeight(0)
        , d3dInputFormat(D3DFMT_UNKNOWN)
        , uOutputWidth(0)
        , uOutputHeight(0)
        , d3dOutputFormat(D3DFMT_UNKNOWN)
        , bColorConversion(FALSE)
    {
    }
};

typedef D3D9VPContext* D3D9VideoProcessorHandle;

// ============================================================================
// MediaInfo -- wraps media properties for a single video file
// ============================================================================
class MediaInfo
{
public:
    MediaInfo()
        : m_llDuration(0)
        , m_uWidth(0)
        , m_uHeight(0)
        , m_uFrameRate(0)
        , m_uBitrate(0)
        , m_uVideoCodecFourCC(0)
        , m_uAudioCodecFourCC(0)
        , m_bHasVideo(FALSE)
        , m_bHasAudio(FALSE)
    {
        m_ftLastModified.dwLowDateTime = 0;
        m_ftLastModified.dwHighDateTime = 0;
    }

    ~MediaInfo() {}

    HRESULT LoadFromFile(LPCWSTR pszPath)
    {
        if (!pszPath || !pszPath[0])
            return E_INVALIDARG;

        m_strPath = pszPath;

        WIN32_FILE_ATTRIBUTE_DATA fad = { 0 };
        if (GetFileAttributesExW(pszPath, GetFileExInfoStandard, &fad))
            m_ftLastModified = fad.ftLastWriteTime;

        IMFSourceReader* pReader = NULL;
        HRESULT hr = MFCreateSourceReaderFromURL(
            pszPath, NULL, &pReader);
        if (FAILED(hr))
            return hr;

        // Query video stream properties
        IMFMediaType* pVideoType = NULL;
        hr = pReader->GetNativeMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0, &pVideoType);

        if (SUCCEEDED(hr))
        {
            m_bHasVideo = TRUE;

            MFGetAttributeSize(pVideoType, MF_MT_FRAME_SIZE,
                &m_uWidth, &m_uHeight);

            // Extract frame rate (rounded to nearest whole fps)
            UINT32 uNum = 0, uDen = 0;
            MFGetAttributeRatio(pVideoType, MF_MT_FRAME_RATE, &uNum, &uDen);
            if (uNum > 0 && uDen > 0)
                m_uFrameRate = (uNum + (uDen >> 1)) / uDen;

            m_uBitrate = 0;
            pVideoType->GetUINT32(MF_MT_AVG_BITRATE, &m_uBitrate);

            GUID guidSubtype = GUID_NULL;
            pVideoType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
            m_uVideoCodecFourCC = FourCCFromSubtype(guidSubtype);

            pVideoType->Release();
        }
        else
        {
            // No video stream is not fatal for an audio-only file.
            hr = S_OK;
        }

        // Query audio stream presence
        IMFMediaType* pAudioType = NULL;
        if (SUCCEEDED(pReader->GetNativeMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0, &pAudioType)))
        {
            m_bHasAudio = TRUE;

            GUID guidSubtype = GUID_NULL;
            pAudioType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
            m_uAudioCodecFourCC = FourCCFromSubtype(guidSubtype);

            pAudioType->Release();
        }

        // Query duration
        PROPVARIANT propDuration;
        PropVariantInit(&propDuration);
        hr = pReader->GetPresentationAttribute(
            (DWORD)MF_SOURCE_READER_MEDIASOURCE,
            MF_PD_DURATION, &propDuration);
        if (SUCCEEDED(hr))
        {
            m_llDuration = propDuration.uhVal.QuadPart;
            PropVariantClear(&propDuration);
        }

        pReader->Release();
        return S_OK;
    }

    LPCWSTR GetPath() const { return m_strPath.c_str(); }
    LONGLONG GetDuration() const { return m_llDuration; }
    UINT32 GetWidth() const { return m_uWidth; }
    UINT32 GetHeight() const { return m_uHeight; }
    UINT32 GetFrameRate() const { return m_uFrameRate; }
    UINT32 GetBitrate() const { return m_uBitrate; }
    UINT32 GetVideoCodecFourCC() const { return m_uVideoCodecFourCC; }
    UINT32 GetAudioCodecFourCC() const { return m_uAudioCodecFourCC; }
    BOOL HasVideo() const { return m_bHasVideo; }
    BOOL HasAudio() const { return m_bHasAudio; }
    const FILETIME& GetLastModified() const { return m_ftLastModified; }

private:
    std::wstring m_strPath;
    LONGLONG     m_llDuration;
    UINT32       m_uWidth;
    UINT32       m_uHeight;
    UINT32       m_uFrameRate;
    UINT32       m_uBitrate;
    UINT32       m_uVideoCodecFourCC;
    UINT32       m_uAudioCodecFourCC;
    BOOL         m_bHasVideo;
    BOOL         m_bHasAudio;
    FILETIME     m_ftLastModified;
};

// ============================================================================
// ThumbnailCache -- manages cached thumbnails for video files
// ============================================================================
class ThumbnailCache
{
public:
    ThumbnailCache() {}
    ~ThumbnailCache() { Clear(); }

    HRESULT GetThumbnail(LPCWSTR pszPath, Gdiplus::Bitmap** ppBitmap)
    {
        if (!pszPath || !ppBitmap)
            return E_INVALIDARG;

        *ppBitmap = NULL;

        try
        {
            std::wstring strKey;
            NormalizeKey(pszPath, strKey);

            // Check cache first
            auto it = m_cache.find(strKey);
            if (it != m_cache.end())
            {
                *ppBitmap = it->second;
                return S_OK;
            }

            // Generate a thumbnail from the first decoded video frame.
            Gdiplus::Bitmap* pBitmap = NULL;
            HRESULT hr = GenerateThumbnail(pszPath, &pBitmap);
            if (FAILED(hr))
                return hr;

            m_cache[strKey] = pBitmap;
            *ppBitmap = pBitmap;
            return S_OK;
        }
        catch (...)
        {
            return E_FAIL;
        }
    }

    void Invalidate(LPCWSTR pszPath)
    {
        if (!pszPath) return;
        std::wstring strKey;
        NormalizeKey(pszPath, strKey);
        auto it = m_cache.find(strKey);
        if (it != m_cache.end())
        {
            delete it->second;
            m_cache.erase(it);
        }
    }

    void Clear()
    {
        for (auto& pair : m_cache)
        {
            delete pair.second;
        }
        m_cache.clear();
    }

private:
    // Grabs the first decodable video sample for a path.
    static HRESULT ReadFirstVideoSample(IMFSourceReader* pReader, IMFSample** ppSample);

    // Reads one frame as RGB32 straight from the source reader (fallback
    // used when the Video Processor MFT rejects the native format).
    static HRESULT GenerateThumbnailViaReaderRgb32(
        IMFSourceReader* pReader, UINT32 uWidth, UINT32 uHeight,
        Gdiplus::Bitmap** ppBitmap);

    // Decodes the first video frame and converts it to an RGB32 bitmap.
    HRESULT GenerateThumbnail(LPCWSTR pszPath, Gdiplus::Bitmap** ppBitmap);

    // Runs the Video Processor MFT: input type -> RGB32 output sample.
    static HRESULT ConvertSampleToRgb32(
        IMFMediaType* pInputType, IMFSample* pSample, Gdiplus::Bitmap** ppBitmap);

    // Copies an RGB32 MF buffer (honoring 2-D stride/orientation) into a
    // top-down GDI+ bitmap.
    static HRESULT CopyRgb32BufferToBitmap(
        IMFMediaBuffer* pBuffer, UINT uWidth, UINT uHeight,
        Gdiplus::Bitmap** ppBitmap);

    std::map<std::wstring, Gdiplus::Bitmap*> m_cache;
};

// ============================================================================
// MFSessionManager -- manages Media Foundation sessions for decoding
// ============================================================================
class MFSessionManager
{
public:
    MFSessionManager()
        : m_pSession(NULL)
        , m_pClock(NULL)
    {
    }

    ~MFSessionManager()
    {
        Shutdown();
    }

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        // Create the media session
        hr = MFCreateMediaSession(NULL, &m_pSession);
        if (FAILED(hr))
            return hr;

        // Get the clock
        hr = m_pSession->GetClock(&m_pClock);
        if (FAILED(hr))
        {
            m_pSession->Release();
            m_pSession = NULL;
            return hr;
        }

        return S_OK;
    }

    void Shutdown()
    {
        if (m_pSession)
        {
            m_pSession->Close();
            m_pSession->Release();
            m_pSession = NULL;
        }

        if (m_pClock)
        {
            m_pClock->Release();
            m_pClock = NULL;
        }
    }

private:
    IMFClock*           m_pClock;
    IMFMediaSession*    m_pSession;
};

// ============================================================================
// VideoProcessor -- hardware-accelerated video processing via D3D9Ex
// ============================================================================
class VideoProcessor
{
public:
    VideoProcessor()
        : m_pD3D9(NULL)
        , m_pDevice(NULL)
        , m_bHardware(FALSE)
    {
        ZeroMemory(&m_caps, sizeof(m_caps));
    }

    ~VideoProcessor()
    {
        Shutdown();
    }

    HRESULT Initialize()
    {
        if (m_pDevice)
            return S_OK;

        // Create D3D9 Ex device for DXVA2-class video processing
        IDirect3D9Ex* pD3D9Ex = NULL;
        HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3D9Ex);
        if (FAILED(hr))
            return hr;

        D3DPRESENT_PARAMETERS pp;
        ZeroMemory(&pp, sizeof(pp));
        pp.Windowed = TRUE;
        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.hDeviceWindow = GetDesktopWindow();
        pp.BackBufferFormat = D3DFMT_X8R8G8B8;
        pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

        IDirect3DDevice9* pDevice = NULL;
        hr = pD3D9Ex->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            pp.hDeviceWindow,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING |
            D3DCREATE_MULTITHREADED |
            D3DCREATE_FPU_PRESERVE,
            &pp,
            &pDevice);

        if (FAILED(hr))
        {
            pD3D9Ex->Release();
            return hr;
        }

        m_pD3D9 = pD3D9Ex;
        m_pDevice = static_cast<IDirect3DDevice9Ex*>(pDevice);

        m_bHardware =
            (pD3D9Ex->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &m_caps) == D3D_OK);

        return S_OK;
    }

    void Shutdown()
    {
        if (m_pDevice)
        {
            m_pDevice->Release();
            m_pDevice = NULL;
        }

        if (m_pD3D9)
        {
            m_pD3D9->Release();
            m_pD3D9 = NULL;
        }

        m_bHardware = FALSE;
    }

    BOOL IsInitialized() const { return (m_pDevice != NULL); }
    BOOL IsHardwareAccelerated() const { return m_bHardware; }
    IDirect3DDevice9Ex* GetDevice() const { return m_pDevice; }

    // Creates a video processing session describing one input format and
    // one output format. The handle is released with DestroyVideoProcessor.
    HRESULT CreateVideoProcessor(
        UINT uInputWidth, UINT uInputHeight, D3DFORMAT d3dInputFormat,
        UINT uOutputWidth, UINT uOutputHeight, D3DFORMAT d3dOutputFormat,
        D3D9VideoProcessorHandle* phProcessor)
    {
        if (!phProcessor)
            return E_POINTER;

        *phProcessor = NULL;

        if (!m_pDevice)
            return E_UNEXPECTED;

        if (uInputWidth == 0 || uInputHeight == 0 ||
            uOutputWidth == 0 || uOutputHeight == 0)
            return E_INVALIDARG;

        D3D9VPContext* pCtx = new (std::nothrow) D3D9VPContext();
        if (!pCtx)
            return E_OUTOFMEMORY;

        pCtx->uInputWidth = uInputWidth;
        pCtx->uInputHeight = uInputHeight;
        pCtx->d3dInputFormat = d3dInputFormat;
        pCtx->uOutputWidth = uOutputWidth;
        pCtx->uOutputHeight = uOutputHeight;
        pCtx->d3dOutputFormat = d3dOutputFormat;
        pCtx->bColorConversion =
            (d3dInputFormat != d3dOutputFormat) &&
            (m_caps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORCONVERSION);

        *phProcessor = pCtx;
        return S_OK;
    }

    HRESULT DestroyVideoProcessor(D3D9VideoProcessorHandle hProcessor)
    {
        if (!hProcessor)
            return E_INVALIDARG;

        delete hProcessor;
        return S_OK;
    }

    // Blits/scales a source surface onto a target surface. Prefers the
    // hardware StretchRect path; falls back to a software nearest-neighbor
    // copy for CPU-readable surface pairs of the same bits-per-pixel.
    HRESULT ProcessBlt(
        D3D9VideoProcessorHandle hProcessor,
        IDirect3DSurface9* pSource,
        IDirect3DSurface9* pTarget,
        const RECT* prcSource,
        const RECT* prcTarget)
    {
        if (!hProcessor || !pSource || !pTarget)
            return E_POINTER;

        if (!m_pDevice)
            return E_UNEXPECTED;

        RECT rcSource;
        RECT rcTarget;
        if (prcSource)
        {
            rcSource = *prcSource;
        }
        else
        {
            rcSource.left = 0;
            rcSource.top = 0;
            rcSource.right = (LONG)hProcessor->uInputWidth;
            rcSource.bottom = (LONG)hProcessor->uInputHeight;
        }
        if (prcTarget)
        {
            rcTarget = *prcTarget;
        }
        else
        {
            rcTarget.left = 0;
            rcTarget.top = 0;
            rcTarget.right = (LONG)hProcessor->uOutputWidth;
            rcTarget.bottom = (LONG)hProcessor->uOutputHeight;
        }

        D3DSURFACE_DESC descSrc, descTgt;
        if (SUCCEEDED(pSource->GetDesc(&descSrc)) &&
            SUCCEEDED(pTarget->GetDesc(&descTgt)) &&
            descSrc.Width == descTgt.Width &&
            descSrc.Height == descTgt.Height &&
            descSrc.Format == descTgt.Format &&
            descSrc.Pool == D3DPOOL_SYSTEMMEM &&
            descTgt.Pool != D3DPOOL_SYSTEMMEM)
        {
            // Fast path: system-memory frame into video memory, no scaling.
            RECT rcWhole = { 0, 0, (LONG)descSrc.Width, (LONG)descSrc.Height };
            POINT ptDest = { 0, 0 };
            if (SUCCEEDED(m_pDevice->UpdateSurface(pSource, &rcWhole, pTarget, &ptDest)))
                return S_OK;
        }

        // Scaled / format-converted path (device surfaces).
        HRESULT hr = m_pDevice->StretchRect(
            pSource, &rcSource, pTarget, &rcTarget, D3DTEXF_LINEAR);
        if (SUCCEEDED(hr))
            return S_OK;

        // Software fallback for CPU-readable surface pairs.
        return BlitSoftware(pSource, &rcSource, pTarget, &rcTarget);
    }

    // Copies a device (DEFAULT pool) render target into a caller-provided
    // system-memory surface of the same format and dimensions.
    HRESULT ReadBackSurface(
        IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pSystemMemSurface)
    {
        if (!pRenderTarget || !pSystemMemSurface)
            return E_POINTER;

        if (!m_pDevice)
            return E_UNEXPECTED;

        return m_pDevice->GetRenderTargetData(pRenderTarget, pSystemMemSurface);
    }

    // Convenience: creates a CPU-readable surface for upload/readback.
    HRESULT CreateSystemSurface(
        UINT uWidth, UINT uHeight, D3DFORMAT d3dFormat, IDirect3DSurface9** ppSurface)
    {
        if (!ppSurface)
            return E_POINTER;

        *ppSurface = NULL;

        if (!m_pDevice)
            return E_UNEXPECTED;

        return m_pDevice->CreateOffscreenPlainSurface(
            uWidth, uHeight, d3dFormat, D3DPOOL_SYSTEMMEM, ppSurface, NULL);
    }

    // Convenience: creates a renderable device surface for output.
    HRESULT CreateRenderTargetSurface(
        UINT uWidth, UINT uHeight, D3DFORMAT d3dFormat, IDirect3DSurface9** ppSurface)
    {
        if (!ppSurface)
            return E_POINTER;

        *ppSurface = NULL;

        if (!m_pDevice)
            return E_UNEXPECTED;

        return m_pDevice->CreateRenderTarget(
            uWidth, uHeight, d3dFormat, D3DMULTISAMPLE_NONE, 0, FALSE, ppSurface, NULL);
    }

private:
    // Software nearest-neighbor blit between two lockable surfaces of the
    // same bits-per-pixel. YUV or mismatched bpp pairs return D3DERR_INVALIDCALL.
    HRESULT BlitSoftware(
        IDirect3DSurface9* pSource, const RECT* prcSource,
        IDirect3DSurface9* pTarget, const RECT* prcTarget)
    {
        D3DLOCKED_RECT lockSrc, lockTgt;
        if (FAILED(pSource->LockRect(&lockSrc, NULL, D3DLOCK_READONLY)))
            return E_FAIL;

        HRESULT hr = pTarget->LockRect(&lockTgt, NULL, 0);
        if (FAILED(hr))
        {
            pSource->UnlockRect();
            return E_FAIL;
        }

        D3DSURFACE_DESC descSrc, descTgt;
        pSource->GetDesc(&descSrc);
        pTarget->GetDesc(&descTgt);

        const UINT uSrcBpp = BytesPerPixel(descSrc.Format);
        const UINT uTgtBpp = BytesPerPixel(descTgt.Format);
        if (uSrcBpp == 0 || uTgtBpp == 0 || uSrcBpp != uTgtBpp)
        {
            pTarget->UnlockRect();
            pSource->UnlockRect();
            return D3DERR_INVALIDCALL;
        }

        const LONG srcW = prcSource->right - prcSource->left;
        const LONG srcH = prcSource->bottom - prcSource->top;
        const LONG tgtW = prcTarget->right - prcTarget->left;
        const LONG tgtH = prcTarget->bottom - prcTarget->top;

        const BYTE* pSrcBase =
            (const BYTE*)lockSrc.pBits +
            prcSource->top * lockSrc.Pitch +
            prcSource->left * uSrcBpp;
        BYTE* pTgtBase =
            (BYTE*)lockTgt.pBits +
            prcTarget->top * lockTgt.Pitch +
            prcTarget->left * uTgtBpp;

        if (srcW == tgtW && srcH == tgtH)
        {
            for (LONG y = 0; y < tgtH; ++y)
            {
                memcpy(pTgtBase + y * lockTgt.Pitch,
                       pSrcBase + y * lockSrc.Pitch,
                       (size_t)tgtW * uTgtBpp);
            }
        }
        else
        {
            for (LONG y = 0; y < tgtH; ++y)
            {
                const LONG sy = (y * srcH) / tgtH;
                const BYTE* pSrcRow = pSrcBase + sy * lockSrc.Pitch;
                BYTE* pDstRow = pTgtBase + y * lockTgt.Pitch;
                for (LONG x = 0; x < tgtW; ++x)
                {
                    const LONG sx = (x * srcW) / tgtW;
                    memcpy(pDstRow + x * uTgtBpp,
                           pSrcRow + sx * uSrcBpp,
                           uTgtBpp);
                }
            }
        }

        pTarget->UnlockRect();
        pSource->UnlockRect();
        return S_OK;
    }

    IDirect3D9Ex*       m_pD3D9;
    IDirect3DDevice9Ex* m_pDevice;
    D3DCAPS9            m_caps;
    BOOL                m_bHardware;
};

// ============================================================================
// LibraryManager -- singleton that coordinates all movie library operations
// ============================================================================
class LibraryManager
{
public:
    LibraryManager()
        : m_bInitialized(false)
    {
        InitializeCriticalSection(&m_csLib);
    }

    ~LibraryManager()
    {
        Shutdown();
        DeleteCriticalSection(&m_csLib);
    }

    static LibraryManager& GetInstance()
    {
        static LibraryManager s_instance;
        return s_instance;
    }

    HRESULT Initialize()
    {
        if (m_bInitialized)
            return S_OK;

        m_thumbnailCache.reset(new ThumbnailCache());

        m_mfSession.reset(new MFSessionManager());
        HRESULT hr = m_mfSession->Initialize();
        if (FAILED(hr))
            return hr;

        m_videoProcessor.reset(new VideoProcessor());
        // Video processor init is optional (hardware may not be available)
        m_videoProcessor->Initialize();

        m_bInitialized = true;
        return S_OK;
    }

    void Shutdown()
    {
        if (!m_bInitialized)
            return;

        m_videoProcessor.reset();
        m_mfSession.reset();
        m_thumbnailCache.reset();

        ClearItems();

        m_bInitialized = false;
    }

    // ------------------------------------------------------------------------
    // Library item management (in-memory store)
    // ------------------------------------------------------------------------

    // Adds a file to the library. Returns S_OK if added, S_FALSE if the path
    // is already present, and a failure HRESULT for invalid/nonexistent paths.
    HRESULT AddItem(LPCWSTR pszPath)
    {
        if (!pszPath || !pszPath[0])
            return E_INVALIDARG;

        DWORD dwAttr = GetFileAttributesW(pszPath);
        if (dwAttr == INVALID_FILE_ATTRIBUTES)
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
            return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);

        std::wstring strKey;
        NormalizeKey(pszPath, strKey);

        EnterCriticalSection(&m_csLib);
        if (m_itemMap.find(strKey) != m_itemMap.end())
        {
            LeaveCriticalSection(&m_csLib);
            return S_FALSE;
        }

        m_itemList.push_back(pszPath);
        m_itemMap[strKey] = true;
        LeaveCriticalSection(&m_csLib);

        return S_OK;
    }

    // Removes a file from the library (and any cached media info for it).
    HRESULT RemoveItem(LPCWSTR pszPath)
    {
        if (!pszPath || !pszPath[0])
            return E_INVALIDARG;

        std::wstring strKey;
        NormalizeKey(pszPath, strKey);

        HRESULT hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

        EnterCriticalSection(&m_csLib);
        for (std::vector<std::wstring>::iterator it = m_itemList.begin();
             it != m_itemList.end(); ++it)
        {
            std::wstring existingKey;
            NormalizeKey(it->c_str(), existingKey);
            if (existingKey == strKey)
            {
                m_itemList.erase(it);
                m_itemMap.erase(strKey);
                hr = S_OK;
                break;
            }
        }

        if (SUCCEEDED(hr))
        {
            // Drop any cached media info for this path (case-insensitive).
            for (std::map<std::wstring, std::shared_ptr<MediaInfo>>::iterator
                     itInfo = m_mediaInfoMap.begin();
                 itInfo != m_mediaInfoMap.end(); ++itInfo)
            {
                std::wstring infoKey;
                NormalizeKey(itInfo->first.c_str(), infoKey);
                if (infoKey == strKey)
                {
                    m_mediaInfoMap.erase(itInfo);
                    break;
                }
            }
        }
        LeaveCriticalSection(&m_csLib);

        return hr;
    }

    void ClearItems()
    {
        EnterCriticalSection(&m_csLib);
        m_itemList.clear();
        m_itemMap.clear();
        m_mediaInfoMap.clear();
        LeaveCriticalSection(&m_csLib);
    }

    BOOL HasItem(LPCWSTR pszPath) const
    {
        if (!pszPath)
            return FALSE;

        std::wstring strKey;
        NormalizeKey(pszPath, strKey);

        EnterCriticalSection(&m_csLib);
        BOOL bFound = (m_itemMap.find(strKey) != m_itemMap.end());
        LeaveCriticalSection(&m_csLib);

        return bFound;
    }

    DWORD GetItemCount() const
    {
        EnterCriticalSection(&m_csLib);
        DWORD dwCount = (DWORD)m_itemList.size();
        LeaveCriticalSection(&m_csLib);
        return dwCount;
    }

    // Copies the full path of the item at the given index.
    HRESULT GetItemPath(DWORD dwIndex, LPWSTR pszPath, DWORD cchPath) const
    {
        if (!pszPath || cchPath == 0)
            return E_POINTER;

        pszPath[0] = L'\0';

        EnterCriticalSection(&m_csLib);
        if (dwIndex >= m_itemList.size())
        {
            LeaveCriticalSection(&m_csLib);
            return E_INVALIDARG;
        }

        const std::wstring& strPath = m_itemList[dwIndex];
        const DWORD cchNeeded = (DWORD)strPath.size() + 1;
        if (cchNeeded > cchPath)
        {
            LeaveCriticalSection(&m_csLib);
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }

        wcsncpy_s(pszPath, cchPath, strPath.c_str(), _TRUNCATE);
        LeaveCriticalSection(&m_csLib);
        return S_OK;
    }

    // Returns the media info for the item at the given index.
    HRESULT GetItemInfo(DWORD dwIndex, MovieMediaInfo* pInfo)
    {
        if (!pInfo)
            return E_POINTER;

        WCHAR wszPath[MAX_PATH];
        HRESULT hr = GetItemPath(dwIndex, wszPath, MAX_PATH);
        if (FAILED(hr))
            return hr;

        return GetMediaInfo(wszPath, pInfo);
    }

    // Looks up media info for an arbitrary path (even one not in the store).
    HRESULT QueryItem(LPCWSTR pszPath, MovieMediaInfo* pInfo)
    {
        return GetMediaInfo(pszPath, pInfo);
    }

    // ------------------------------------------------------------------------
    // Media info / thumbnail queries
    // ------------------------------------------------------------------------

    HRESULT GetMediaInfo(LPCWSTR pszPath, MovieMediaInfo* pInfo)
    {
        if (!pszPath || !pInfo)
            return E_INVALIDARG;

        std::wstring strPath(pszPath);

        EnterCriticalSection(&m_csLib);
        auto it = m_mediaInfoMap.find(strPath);
        if (it == m_mediaInfoMap.end())
        {
            std::shared_ptr<MediaInfo> spInfo(new MediaInfo());
            HRESULT hr = spInfo->LoadFromFile(pszPath);
            if (FAILED(hr))
            {
                LeaveCriticalSection(&m_csLib);
                return hr;
            }

            m_mediaInfoMap[strPath] = spInfo;
            it = m_mediaInfoMap.find(strPath);
        }

        const MediaInfo& info = *it->second;
        ZeroMemory(pInfo, sizeof(MovieMediaInfo));
        wcsncpy_s(pInfo->wszFilePath, MAX_PATH, info.GetPath(), _TRUNCATE);
        pInfo->llDuration = info.GetDuration();
        pInfo->uWidth = info.GetWidth();
        pInfo->uHeight = info.GetHeight();
        pInfo->uFrameRate = info.GetFrameRate();
        pInfo->uBitrate = info.GetBitrate();
        pInfo->uVideoCodecFourCC = info.GetVideoCodecFourCC();
        pInfo->uAudioCodecFourCC = info.GetAudioCodecFourCC();
        pInfo->bHasVideo = info.HasVideo();
        pInfo->bHasAudio = info.HasAudio();
        pInfo->ftLastModified = info.GetLastModified();
        LeaveCriticalSection(&m_csLib);

        return S_OK;
    }

    HRESULT GetThumbnail(LPCWSTR pszPath, Gdiplus::Bitmap** ppBitmap)
    {
        if (m_thumbnailCache)
            return m_thumbnailCache->GetThumbnail(pszPath, ppBitmap);
        return E_UNEXPECTED;
    }

    void InvalidateThumbnail(LPCWSTR pszPath)
    {
        if (m_thumbnailCache)
            m_thumbnailCache->Invalidate(pszPath);
    }

    void ClearThumbnails()
    {
        if (m_thumbnailCache)
            m_thumbnailCache->Clear();
    }

private:
    bool                                                      m_bInitialized;
    mutable CRITICAL_SECTION                                  m_csLib;
    std::unique_ptr<ThumbnailCache>                           m_thumbnailCache;
    std::unique_ptr<MFSessionManager>                         m_mfSession;
    std::unique_ptr<VideoProcessor>                           m_videoProcessor;
    std::vector<std::wstring>                                 m_itemList;
    std::map<std::wstring, bool>                              m_itemMap;
    std::map<std::wstring, std::shared_ptr<MediaInfo>>        m_mediaInfoMap;
};

// ============================================================================
// ThumbnailCache implementation
// ============================================================================

HRESULT ThumbnailCache::ReadFirstVideoSample(
    IMFSourceReader* pReader, IMFSample** ppSample)
{
    *ppSample = NULL;

    HRESULT hr = S_OK;
    for (DWORD dwAttempt = 0; dwAttempt < 8; ++dwAttempt)
    {
        IMFSample* pSample = NULL;
        DWORD dwFlags = 0;
        LONGLONG llTime = 0;

        hr = pReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,
            NULL,
            &dwFlags,
            &llTime,
            &pSample);

        if (FAILED(hr))
            break;

        if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            hr = MF_E_NO_MORE_SAMPLES;
            break;
        }

        if (pSample)
        {
            *ppSample = pSample;
            return S_OK;
        }

        // stream tick or event-only result; keep reading
    }

    return hr;
}

HRESULT ThumbnailCache::GenerateThumbnailViaReaderRgb32(
    IMFSourceReader* pReader, UINT32 uWidth, UINT32 uHeight,
    Gdiplus::Bitmap** ppBitmap)
{
    *ppBitmap = NULL;

    IMFMediaType* pRgbType = NULL;
    HRESULT hr = MFCreateMediaType(&pRgbType);
    if (SUCCEEDED(hr))
    {
        pRgbType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        pRgbType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
        MFSetAttributeSize(pRgbType, MF_MT_FRAME_SIZE, uWidth, uHeight);
        pRgbType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

        hr = pReader->SetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pRgbType);
        pRgbType->Release();
    }
    if (FAILED(hr))
        return hr;

    IMFSample* pSample = NULL;
    hr = ReadFirstVideoSample(pReader, &pSample);
    if (FAILED(hr))
        return hr;

    IMFMediaBuffer* pBuffer = NULL;
    hr = pSample->GetBufferByIndex(0, &pBuffer);
    if (SUCCEEDED(hr))
        hr = CopyRgb32BufferToBitmap(pBuffer, uWidth, uHeight, ppBitmap);
    if (pBuffer)
        pBuffer->Release();

    pSample->Release();
    return hr;
}

HRESULT ThumbnailCache::GenerateThumbnail(
    LPCWSTR pszPath, Gdiplus::Bitmap** ppBitmap)
{
    *ppBitmap = NULL;

    IMFSourceReader* pReader = NULL;
    HRESULT hr = MFCreateSourceReaderFromURL(pszPath, NULL, &pReader);
    if (FAILED(hr))
        return hr;

    // Native video type gives the dimensions and subtype for the MFT input.
    IMFMediaType* pNative = NULL;
    hr = pReader->GetNativeMediaType(
        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &pNative);
    if (FAILED(hr))
    {
        pReader->Release();
        return hr;
    }

    UINT32 uWidth = 0, uHeight = 0;
    MFGetAttributeSize(pNative, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
    if (uWidth == 0 || uHeight == 0)
    {
        pNative->Release();
        pReader->Release();
        return MF_E_INVALIDMEDIATYPE;
    }

    // Build the input type the reader will actually deliver and that we
    // feed to the Video Processor MFT.
    IMFMediaType* pInputType = NULL;
    hr = MFCreateMediaType(&pInputType);
    if (SUCCEEDED(hr))
    {
        GUID guidMajor = GUID_NULL;
        GUID guidSubtype = GUID_NULL;
        pNative->GetGUID(MF_MT_MAJOR_TYPE, &guidMajor);
        pNative->GetGUID(MF_MT_SUBTYPE, &guidSubtype);

        pInputType->SetGUID(MF_MT_MAJOR_TYPE, guidMajor);
        pInputType->SetGUID(MF_MT_SUBTYPE, guidSubtype);
        MFSetAttributeSize(pInputType, MF_MT_FRAME_SIZE, uWidth, uHeight);
        pInputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

        // Ask the reader to deliver this exact type so the samples we read
        // match the MFT input type.
        hr = pReader->SetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pInputType);
    }
    pNative->Release();
    if (FAILED(hr))
    {
        if (pInputType)
            pInputType->Release();
        pReader->Release();
        return hr;
    }

    IMFSample* pSample = NULL;
    hr = ReadFirstVideoSample(pReader, &pSample);
    if (SUCCEEDED(hr))
    {
        hr = ConvertSampleToRgb32(pInputType, pSample, ppBitmap);
        pSample->Release();
    }

    // Fallback: if the Video Processor MFT rejected the source format, let
    // the source reader convert straight to RGB32 and grab a frame.
    if (FAILED(hr) || !*ppBitmap)
    {
        if (*ppBitmap)
        {
            delete *ppBitmap;
            *ppBitmap = NULL;
        }
        hr = GenerateThumbnailViaReaderRgb32(pReader, uWidth, uHeight, ppBitmap);
    }

    pInputType->Release();
    pReader->Release();
    return hr;
}

HRESULT ThumbnailCache::ConvertSampleToRgb32(
    IMFMediaType* pInputType, IMFSample* pSample, Gdiplus::Bitmap** ppBitmap)
{
    *ppBitmap = NULL;

    UINT32 uWidth = 0, uHeight = 0;
    MFGetAttributeSize(pInputType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
    if (uWidth == 0 || uHeight == 0)
        return MF_E_INVALIDMEDIATYPE;

    // The Video Processor MFT performs YUV -> RGB32 conversion (and scaling)
    // in a single pass.
    IMFTransform* pTransform = NULL;
    HRESULT hr = CoCreateInstance(
        CLSID_VideoProcessorMFT_Local,
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMFTransform),
        (void**)&pTransform);
    if (FAILED(hr))
        return hr;

    hr = pTransform->SetInputType(0, pInputType, 0);
    if (FAILED(hr))
    {
        pTransform->Release();
        return hr;
    }

    IMFMediaType* pOutputType = NULL;
    hr = MFCreateMediaType(&pOutputType);
    if (SUCCEEDED(hr))
    {
        pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
        MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, uWidth, uHeight);
        pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

        hr = pTransform->SetOutputType(0, pOutputType, 0);
        pOutputType->Release();
    }
    if (FAILED(hr))
    {
        pTransform->Release();
        return hr;
    }

    pTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);

    hr = pTransform->ProcessInput(0, pSample, 0);
    if (FAILED(hr))
    {
        pTransform->Release();
        return hr;
    }

    MFT_OUTPUT_STREAM_INFO streamInfo;
    ZeroMemory(&streamInfo, sizeof(streamInfo));
    hr = pTransform->GetOutputStreamInfo(0, &streamInfo);
    if (FAILED(hr))
    {
        pTransform->Release();
        return hr;
    }

    IMFSample* pOutputSample = NULL;
    hr = MFCreateSample(&pOutputSample);
    if (SUCCEEDED(hr))
    {
        IMFMediaBuffer* pOutputBuffer = NULL;
        const DWORD cbBuffer = (streamInfo.cbSize > 0)
            ? streamInfo.cbSize
            : (uWidth * uHeight * 4);
        hr = MFCreateMemoryBuffer(cbBuffer, &pOutputBuffer);
        if (SUCCEEDED(hr))
        {
            hr = pOutputSample->AddBuffer(pOutputBuffer);
            pOutputBuffer->Release();
        }
    }
    if (FAILED(hr))
    {
        if (pOutputSample)
            pOutputSample->Release();
        pTransform->Release();
        return hr;
    }

    MFT_OUTPUT_DATA_BUFFER outputData;
    ZeroMemory(&outputData, sizeof(outputData));
    outputData.dwStreamID = 0;
    outputData.pSample = pOutputSample;

    DWORD dwStatus = 0;
    hr = pTransform->ProcessOutput(0, 1, &outputData, &dwStatus);
    if (FAILED(hr) || !outputData.pSample)
    {
        pOutputSample->Release();
        pTransform->Release();
        return FAILED(hr) ? hr : E_FAIL;
    }

    // ProcessOutput may substitute an MFT-provided sample; release the one
    // we allocated if it was not used.
    if (outputData.pSample != pOutputSample)
        pOutputSample->Release();

    IMFMediaBuffer* pResultBuffer = NULL;
    hr = outputData.pSample->GetBufferByIndex(0, &pResultBuffer);
    if (SUCCEEDED(hr))
        hr = CopyRgb32BufferToBitmap(pResultBuffer, uWidth, uHeight, ppBitmap);
    if (pResultBuffer)
        pResultBuffer->Release();

    outputData.pSample->Release();
    pTransform->Release();
    return hr;
}

HRESULT ThumbnailCache::CopyRgb32BufferToBitmap(
    IMFMediaBuffer* pBuffer, UINT uWidth, UINT uHeight,
    Gdiplus::Bitmap** ppBitmap)
{
    *ppBitmap = NULL;

    // Prefer the 2-D buffer interface so the real stride / scanline
    // orientation is honored instead of assuming width*4 top-down.
    IMF2DBuffer* pBuffer2D = NULL;
    BYTE* pScan0 = NULL;
    LONG lPitch = 0;
    BOOL fBottomUp = FALSE;

    if (SUCCEEDED(pBuffer->QueryInterface(__uuidof(IMF2DBuffer), (void**)&pBuffer2D)))
    {
        if (FAILED(pBuffer2D->Lock2D(&pScan0, &lPitch)))
        {
            pBuffer2D->Release();
            return E_FAIL;
        }
        if (lPitch < 0)
        {
            fBottomUp = TRUE;
            lPitch = -lPitch;
        }
    }
    else
    {
        BYTE* pData = NULL;
        DWORD cbLength = 0;
        if (FAILED(pBuffer->Lock(&pData, NULL, &cbLength)) || !pData)
            return E_FAIL;
        pScan0 = pData;
        lPitch = (LONG)(uWidth * 4);
    }

    const LONG lStride = (LONG)uWidth * 4;
    std::vector<BYTE> vec((size_t)lStride * uHeight);
    BYTE* pDst = &vec[0];

    // GDI+ Bitmap expects top-down scan0 with a positive stride; normalize
    // rows here (flipping bottom-up buffers) before handing data over.
    for (UINT y = 0; y < uHeight; ++y)
    {
        const BYTE* pSrcRow = fBottomUp
            ? pScan0 + (UINT)(uHeight - 1 - y) * lPitch
            : pScan0 + (UINT)y * lPitch;
        memcpy(pDst + (size_t)y * lStride, pSrcRow, lStride);
    }

    if (pBuffer2D)
    {
        pBuffer2D->Unlock2D();
        pBuffer2D->Release();
    }
    else
    {
        pBuffer->Unlock();
    }

    Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(
        (INT)uWidth, (INT)uHeight, lStride, PixelFormat32bppARGB, &vec[0]);
    if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok)
    {
        delete pBitmap;
        return E_FAIL;
    }

    *ppBitmap = pBitmap;
    return S_OK;
}

} // namespace MovieLibrary

// ============================================================================
// Exported functions -- standard COM DLL entry points
// ============================================================================

extern "C"
{

STDAPI DllCanUnloadNow()
{
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(ppv);
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllRegisterServer()
{
    return S_OK;
}

STDAPI DllUnregisterServer()
{
    return S_OK;
}

} // extern "C"
