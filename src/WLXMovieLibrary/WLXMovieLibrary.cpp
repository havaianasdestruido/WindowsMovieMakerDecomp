/*
 * WLXMovieLibrary.cpp
 *
 * Implementation of WLXMovieLibrary.dll -- movie library management
 * and Media Foundation integration for Windows Live Movie Maker 2012.
 *
 * The LibraryManager is a singleton that coordinates:
 *   - Video file enumeration from known folders and project directories
 *   - Media property queries via Media Foundation source readers
 *   - Thumbnail generation using MFThumbnailProvider / IThumbnailProvider
 *   - Hardware-accelerated video processing pipeline setup
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
#include <propkey.h>
#include <propsys.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <vector>
#include <map>
#include <string>

// ============================================================================
// Internal classes
// ============================================================================
namespace MovieLibrary
{

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
        , m_bHasVideo(FALSE)
        , m_bHasAudio(FALSE)
    {
    }

    ~MediaInfo() {}

    HRESULT LoadFromFile(LPCWSTR pszPath)
    {
        if (!pszPath || !pszPath[0])
            return E_INVALIDARG;

        m_strPath = pszPath;

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

            UINT32 uW = 0, uH = 0;
            pVideoType->GetUINT32(MF_MT_FRAME_SIZE, &uW);
            m_uWidth = uW;
            m_uHeight = uH;

            // Extract frame rate
            UINT32 uNum = 0, uDen = 0;
            MFGetAttributeRatio(pVideoType, MF_MT_FRAME_RATE, &uNum, &uDen);
            if (uDen > 0)
                m_uFrameRate = uNum / uDen;

            UINT32 uBitrate = 0;
            pVideoType->GetUINT32(MF_MT_AVG_BITRATE, &uBitrate);
            m_uBitrate = uBitrate;

            pVideoType->Release();
        }

        // Query audio stream presence
        IMFMediaType* pAudioType = NULL;
        hr = pReader->GetNativeMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0, &pAudioType);
        if (SUCCEEDED(hr))
        {
            m_bHasAudio = TRUE;
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
    BOOL HasVideo() const { return m_bHasVideo; }
    BOOL HasAudio() const { return m_bHasAudio; }

private:
    std::wstring m_strPath;
    LONGLONG     m_llDuration;
    UINT32       m_uWidth;
    UINT32       m_uHeight;
    UINT32       m_uFrameRate;
    UINT32       m_uBitrate;
    BOOL         m_bHasVideo;
    BOOL         m_bHasAudio;
};

// ============================================================================
// ThumbnailCache -- manages cached thumbnails for video files
// ============================================================================
class ThumbnailCache
{
public:
    ThumbnailCache() {}
    ~ThumbnailCache() {}

    HRESULT GetThumbnail(LPCWSTR pszPath, Gdiplus::Bitmap** ppBitmap)
    {
        if (!pszPath || !ppBitmap)
            return E_INVALIDARG;

        *ppBitmap = NULL;

        // Check cache first
        auto it = m_cache.find(pszPath);
        if (it != m_cache.end())
        {
            *ppBitmap = it->second;
            return S_OK;
        }

        // Generate thumbnail using MF
        // In the full implementation, this uses IMFGetService to get
        // IThumbnailProvider from the MF source reader.
        return E_NOTIMPL;
    }

    void Invalidate(LPCWSTR pszPath)
    {
        if (pszPath)
            m_cache.erase(pszPath);
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
// VideoProcessor -- hardware-accelerated video processing via D3D/DXVA2
// ============================================================================
class VideoProcessor
{
public:
    VideoProcessor()
        : m_pD3D9(NULL)
        , m_pDevice(NULL)
    {
    }

    ~VideoProcessor()
    {
        Shutdown();
    }

    HRESULT Initialize()
    {
        // Create D3D9 Ex device for DXVA2 video processing
        IDirect3D9Ex* pD3D9Ex = NULL;
        HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3D9Ex);
        if (FAILED(hr))
            return hr;

        D3DPRESENT_PARAMETERS pp = { 0 };
        pp.Windowed = TRUE;
        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.hDeviceWindow = GetDesktopWindow();

        hr = pD3D9Ex->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            pp.hDeviceWindow,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
            &pp,
            &m_pDevice);

        if (SUCCEEDED(hr))
        {
            m_pD3D9 = pD3D9Ex;
        }
        else
        {
            pD3D9Ex->Release();
        }

        return hr;
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
    }

private:
    IDirect3D9Ex*       m_pD3D9;
    IDirect3DDevice9*   m_pDevice;
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
    }

    ~LibraryManager()
    {
        Shutdown();
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
        m_mediaInfoMap.clear();

        m_bInitialized = false;
    }

    HRESULT GetMediaInfo(LPCWSTR pszPath, MovieMediaInfo* pInfo)
    {
        if (!pszPath || !pInfo)
            return E_INVALIDARG;

        std::wstring strPath(pszPath);

        auto it = m_mediaInfoMap.find(strPath);
        if (it == m_mediaInfoMap.end())
        {
            std::shared_ptr<MediaInfo> spInfo(new MediaInfo());
            HRESULT hr = spInfo->LoadFromFile(pszPath);
            if (FAILED(hr))
                return hr;

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
        pInfo->bHasVideo = info.HasVideo();
        pInfo->bHasAudio = info.HasAudio();

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

private:
    bool                                                m_bInitialized;
    std::unique_ptr<ThumbnailCache>                     m_thumbnailCache;
    std::unique_ptr<MFSessionManager>                   m_mfSession;
    std::unique_ptr<VideoProcessor>                     m_videoProcessor;
    std::map<std::wstring, std::shared_ptr<MediaInfo>>  m_mediaInfoMap;
};

} // namespace MovieLibrary

// ============================================================================
// Exported function (1 export)
// ============================================================================

extern "C"
{

WLXML_API void* __cdecl MovieLibrary_GetInstance()
{
    MovieLibrary::LibraryManager& mgr = MovieLibrary::LibraryManager::GetInstance();
    mgr.Initialize();
    return static_cast<void*>(&mgr);
}

} // extern "C"
