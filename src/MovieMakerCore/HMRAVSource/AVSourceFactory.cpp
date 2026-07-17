// AVSourceFactory.cpp - COM factory for creating AVSource instances

#include "pch.h"
#include "AVSourceFactory.h"

namespace HMRAVSource
{

// ============================================================================
// Static members
// ============================================================================
LONG AVSourceFactory::s_cLockCount = 0;

// ============================================================================
// Construction / Destruction
// ============================================================================

AVSourceFactory::AVSourceFactory()
{
}

AVSourceFactory::~AVSourceFactory()
{
}

// ============================================================================
// IClassFactory
// ============================================================================

STDMETHODIMP AVSourceFactory::CreateInstance(
    IUnknown* pUnkOuter,
    REFIID riid,
    void** ppvObject)
{
    if (!ppvObject)
        return E_POINTER;

    *ppvObject = nullptr;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    CComPtr<AVSource> spSource;
    HRESULT hr = CreateInstance(&spSource);
    if (FAILED(hr))
        return hr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDispatch))
    {
        *ppvObject = static_cast<AVSource*>(spSource.p);
        spSource.p->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP AVSourceFactory::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement(&s_cLockCount);
    else
        InterlockedDecrement(&s_cLockCount);

    return S_OK;
}

// ============================================================================
// Factory helpers
// ============================================================================

HRESULT AVSourceFactory::CreateInstance(AVSource** ppSource)
{
    if (!ppSource)
        return E_POINTER;

    *ppSource = new (std::nothrow) MFSource();
    if (!*ppSource)
        return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT AVSourceFactory::CreateInstanceForFile(LPCWSTR pszFilePath, AVSource** ppSource)
{
    if (!pszFilePath || !ppSource)
        return E_POINTER;

    *ppSource = nullptr;

    if (IsDShowOnlyFormat(pszFilePath))
    {
        DShowSource* pSource = new (std::nothrow) DShowSource();
        if (!pSource)
            return E_OUTOFMEMORY;

        *ppSource = pSource;
        return S_OK;
    }

    MFSource* pSource = new (std::nothrow) MFSource();
    if (!pSource)
        return E_OUTOFMEMORY;

    *ppSource = pSource;
    return S_OK;
}

bool AVSourceFactory::IsDShowOnlyFormat(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return false;

    LPCWSTR pszExtension = PathFindExtensionW(pszFilePath);
    if (!pszExtension || *pszExtension == L'\0')
        return false;

    return IsLegacyVideoFormat(pszExtension) || IsLegacyAudioFormat(pszExtension);
}

bool AVSourceFactory::IsNativeMFSourceFormat(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return false;

    LPCWSTR pszExtension = PathFindExtensionW(pszFilePath);
    if (!pszExtension || *pszExtension == L'\0')
        return false;

    // Standard formats supported natively by Media Foundation
    if (_wcsicmp(pszExtension, L".mp4") == 0 ||
        _wcsicmp(pszExtension, L".mov") == 0 ||
        _wcsicmp(pszExtension, L".m4v") == 0 ||
        _wcsicmp(pszExtension, L".mp3") == 0 ||
        _wcsicmp(pszExtension, L".m4a") == 0 ||
        _wcsicmp(pszExtension, L".wav") == 0 ||
        _wcsicmp(pszExtension, L".aac") == 0 ||
        _wcsicmp(pszExtension, L".wma") == 0 ||
        _wcsicmp(pszExtension, L".wmv") == 0 ||
        _wcsicmp(pszExtension, L".avi") == 0 ||
        _wcsicmp(pszExtension, L".asf") == 0 ||
        _wcsicmp(pszExtension, L".3gp") == 0 ||
        _wcsicmp(pszExtension, L".3g2") == 0)
    {
        return true;
    }

    return false;
}

// ============================================================================
// Private helpers - Legacy format detection
// ============================================================================

bool AVSourceFactory::IsLegacyVideoFormat(LPCWSTR pszExtension)
{
    if (!pszExtension)
        return false;

    // Formats that require DirectShow filters
    if (_wcsicmp(pszExtension, L".mpg") == 0 ||
        _wcsicmp(pszExtension, L".mpeg") == 0 ||
        _wcsicmp(pszExtension, L".mpe") == 0 ||
        _wcsicmp(pszExtension, L".vob") == 0 ||
        _wcsicmp(pszExtension, L".ogv") == 0 ||
        _wcsicmp(pszExtension, L".ogm") == 0 ||
        _wcsicmp(pszExtension, L".mkv") == 0 ||
        _wcsicmp(pszExtension, L".webm") == 0 ||
        _wcsicmp(pszExtension, L".flv") == 0 ||
        _wcsicmp(pszExtension, L".f4v") == 0 ||
        _wcsicmp(pszExtension, L".ts") == 0 ||
        _wcsicmp(pszExtension, L".m2ts") == 0 ||
        _wcsicmp(pszExtension, L".mts") == 0 ||
        _wcsicmp(pszExtension, L".m2t") == 0 ||
        _wcsicmp(pszExtension, L".divx") == 0 ||
        _wcsicmp(pszExtension, L".xvid") == 0)
    {
        return true;
    }

    return false;
}

bool AVSourceFactory::IsLegacyAudioFormat(LPCWSTR pszExtension)
{
    if (!pszExtension)
        return false;

    if (_wcsicmp(pszExtension, L".flac") == 0 ||
        _wcsicmp(pszExtension, L".ogg") == 0 ||
        _wcsicmp(pszExtension, L".oga") == 0 ||
        _wcsicmp(pszExtension, L".opus") == 0 ||
        _wcsicmp(pszExtension, L".ape") == 0 ||
        _wcsicmp(pszExtension, L".wv") == 0)
    {
        return true;
    }

    return false;
}

VideoCodec AVSourceFactory::DetectVideoCodec(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return VideoCodecUnknown;

    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, nullptr, &spReader);
    if (FAILED(hr))
        return VideoCodecUnknown;

    CComPtr<IMFMediaType> spType;
    hr = spReader->GetCurrentMediaType(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM, &spType);
    if (FAILED(hr))
        return VideoCodecUnknown;

    GUID guidSubtype = GUID_NULL;
    hr = spType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
    if (FAILED(hr))
        return VideoCodecUnknown;

    if (guidSubtype == MFVideoFormat_H264)
        return VideoCodecH264;
    if (guidSubtype == MFVideoFormat_WMVVC1 || guidSubtype == MFVideoFormat_WMV3)
        return VideoCodecWMV9;
    if (guidSubtype == MFVideoFormat_MP43 || guidSubtype == MFVideoFormat_MP4V)
        return VideoCodecMPEG4;
    if (guidSubtype == MFVideoFormat_HEVC)
        return VideoCodecH265;

    return VideoCodecUnknown;
}

AudioCodec AVSourceFactory::DetectAudioCodec(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return AudioCodecUnknown;

    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, nullptr, &spReader);
    if (FAILED(hr))
        return AudioCodecUnknown;

    CComPtr<IMFMediaType> spType;
    hr = spReader->GetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM, &spType);
    if (FAILED(hr))
        return AudioCodecUnknown;

    GUID guidSubtype = GUID_NULL;
    hr = spType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
    if (FAILED(hr))
        return AudioCodecUnknown;

    if (guidSubtype == MFAudioFormat_AAC)
        return AudioCodecAAC;
    if (guidSubtype == MFAudioFormat_WMAudioV9 || guidSubtype == MFAudioFormat_WMAudioV8)
        return AudioCodecWMA;
    if (guidSubtype == MFAudioFormat_MP3)
        return AudioCodecMP3;
    if (guidSubtype == MFAudioFormat_PCM)
        return AudioCodecPCM;

    return AudioCodecUnknown;
}

} // namespace HMRAVSource
