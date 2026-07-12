// StreamSinkHelper.cpp - Stream sink helper utilities implementation

#include "pch.h"
#include "StreamSinkHelper.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

StreamSinkHelper::StreamSinkHelper()
{
}

StreamSinkHelper::~StreamSinkHelper()
{
}

// ============================================================================
// Video media type creation
// ============================================================================

HRESULT StreamSinkHelper::CreateVideoMediaType(
    const EncodeVideoParams& params,
    IMFMediaType** ppMediaType)
{
    if (!ppMediaType)
        return E_POINTER;

    *ppMediaType = nullptr;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = CreateBaseVideoType(&spType);
    if (FAILED(hr))
        return hr;

    // Set subtype based on codec
    GUID guidSubtype = GUID_NULL;
    hr = GetVideoSubtypeFromCodec(params.codec, &guidSubtype);
    if (FAILED(hr))
        return hr;

    spType->SetGUID(MF_MT_SUBTYPE, guidSubtype);
    spType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

    MFSetAttributeSize(spType, MF_MT_FRAME_SIZE, params.uWidth, params.uHeight);
    MFSetAttributeRatio(spType, MF_MT_FRAME_RATE,
        static_cast<UINT32>(params.dblFrameRate * 100), 100);
    spType->SetUINT32(MF_MT_AVG_BITRATE, params.dwBitRate);

    if (params.codec == VideoCodecH264)
    {
        spType->SetUINT32(MF_MT_MPEG2_PROFILE, params.uProfile);
        spType->SetUINT32(MF_MT_MPEG2_LEVEL, params.uLevel);
    }

    *ppMediaType = spType.Detach();
    return S_OK;
}

HRESULT StreamSinkHelper::CreateVideoInputType(
    UINT uWidth,
    UINT uHeight,
    double dblFrameRate,
    IMFMediaType** ppMediaType)
{
    if (!ppMediaType)
        return E_POINTER;

    *ppMediaType = nullptr;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = CreateBaseVideoType(&spType);
    if (FAILED(hr))
        return hr;

    spType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    MFSetAttributeSize(spType, MF_MT_FRAME_SIZE, uWidth, uHeight);
    MFSetAttributeRatio(spType, MF_MT_FRAME_RATE,
        static_cast<UINT32>(dblFrameRate * 100), 100);

    *ppMediaType = spType.Detach();
    return S_OK;
}

HRESULT StreamSinkHelper::CreateVideoOutputType(
    const EncodeVideoParams& params,
    IMFMediaType** ppMediaType)
{
    return CreateVideoMediaType(params, ppMediaType);
}

// ============================================================================
// Audio media type creation
// ============================================================================

HRESULT StreamSinkHelper::CreateAudioMediaType(
    const EncodeAudioParams& params,
    IMFMediaType** ppMediaType)
{
    if (!ppMediaType)
        return E_POINTER;

    *ppMediaType = nullptr;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = CreateBaseAudioType(&spType);
    if (FAILED(hr))
        return hr;

    GUID guidSubtype = GUID_NULL;
    hr = GetAudioSubtypeFromCodec(params.codec, &guidSubtype);
    if (FAILED(hr))
        return hr;

    spType->SetGUID(MF_MT_SUBTYPE, guidSubtype);
    spType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, params.dwSampleRate);
    spType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, params.dwChannels);
    spType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, params.dwBitsPerSample);
    spType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECTION, params.dwBitRate / 8);
    spType->SetUINT32(MF_MT_BLOCK_ALIGNMENT, params.dwChannels * (params.dwBitsPerSample / 8));

    *ppMediaType = spType.Detach();
    return S_OK;
}

HRESULT StreamSinkHelper::CreateAudioInputType(
    DWORD dwSampleRate,
    DWORD dwChannels,
    DWORD dwBitsPerSample,
    IMFMediaType** ppMediaType)
{
    if (!ppMediaType)
        return E_POINTER;

    *ppMediaType = nullptr;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = CreateBaseAudioType(&spType);
    if (FAILED(hr))
        return hr;

    spType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    spType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, dwSampleRate);
    spType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, dwChannels);
    spType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, dwBitsPerSample);
    spType->SetUINT32(MF_MT_BLOCK_ALIGNMENT, dwChannels * (dwBitsPerSample / 8));

    *ppMediaType = spType.Detach();
    return S_OK;
}

HRESULT StreamSinkHelper::CreateAudioOutputType(
    const EncodeAudioParams& params,
    IMFMediaType** ppMediaType)
{
    return CreateAudioMediaType(params, ppMediaType);
}

// ============================================================================
// Format negotiation
// ============================================================================

HRESULT StreamSinkHelper::NegotiateMediaType(
    IMFSourceReader* pReader,
    DWORD dwStreamIndex,
    IMFMediaType* pDesiredOutputType,
    IMFMediaType** ppAcceptedInputType)
{
    if (!pReader || !pDesiredOutputType || !ppAcceptedInputType)
        return E_POINTER;

    *ppAcceptedInputType = nullptr;

    DWORD dwMediaTypeIndex = 0;
    for (;;)
    {
        CComPtr<IMFMediaType> spNativeType;
        HRESULT hr = pReader->GetNativeMediaType(dwStreamIndex, dwMediaTypeIndex, &spNativeType);
        if (hr == MF_E_NO_MORE_TYPES)
            break;
        if (FAILED(hr))
            return hr;

        CComPtr<IMFMediaType> spTransformed;
        hr = pReader->GetCurrentMediaType(dwStreamIndex, &spTransformed);
        if (SUCCEEDED(hr))
        {
            *ppAcceptedInputType = spTransformed.Detach();
            return S_OK;
        }

        dwMediaTypeIndex++;
    }

    return MF_E_INVALIDMEDIATYPE;
}

HRESULT StreamSinkHelper::FindBestOutputType(
    IMFMediaType* pInputType,
    REFGUID guidMajorType,
    IMFMediaType** ppBestOutputType)
{
    if (!pInputType || !ppBestOutputType)
        return E_POINTER;

    *ppBestOutputType = nullptr;

    // Clone the input type as a starting point
    CComPtr<IMFMediaType> spBestType;
    HRESULT hr = MFCreateMediaType(&spBestType);
    if (FAILED(hr))
        return hr;

    UINT32 cAttrs = pInputType->GetCount();
    for (UINT32 i = 0; i < cAttrs; ++i)
    {
        GUID guidKey;
        PROPVARIANT var;
        hr = pInputType->GetItemByIndex(i, &guidKey, &var);
        if (SUCCEEDED(hr))
        {
            spBestType->SetItem(guidKey, var);
            PropVariantClear(&var);
        }
    }

    *ppBestOutputType = spBestType.Detach();
    return S_OK;
}

// ============================================================================
// Codec helpers
// ============================================================================

HRESULT StreamSinkHelper::GetVideoSubtypeFromCodec(VideoCodec codec, GUID* pSubtype)
{
    if (!pSubtype)
        return E_POINTER;

    switch (codec)
    {
    case VideoCodecH264:    *pSubtype = MFVideoFormat_H264;    return S_OK;
    case VideoCodecWMV9:    *pSubtype = MFVideoFormat_WMVVC1;  return S_OK;
    case VideoCodecMPEG4:   *pSubtype = MFVideoFormat_MP43;     return S_OK;
    case VideoCodecH265:    *pSubtype = MFVideoFormat_HEVC;     return S_OK;
    default:                *pSubtype = MFVideoFormat_H264;     return S_OK;
    }
}

HRESULT StreamSinkHelper::GetAudioSubtypeFromCodec(AudioCodec codec, GUID* pSubtype)
{
    if (!pSubtype)
        return E_POINTER;

    switch (codec)
    {
    case AudioCodecAAC:     *pSubtype = MFAudioFormat_AAC;         return S_OK;
    case AudioCodecWMA:     *pSubtype = MFAudioFormat_WMAudioV9;   return S_OK;
    case AudioCodecMP3:     *pSubtype = MFAudioFormat_MP3;         return S_OK;
    case AudioCodecPCM:     *pSubtype = MFAudioFormat_PCM;         return S_OK;
    default:                *pSubtype = MFAudioFormat_AAC;         return S_OK;
    }
}

VideoCodec StreamSinkHelper::GetCodecFromVideoSubtype(REFGUID guidSubtype)
{
    if (guidSubtype == MFVideoFormat_H264)     return VideoCodecH264;
    if (guidSubtype == MFVideoFormat_WMVVC1)   return VideoCodecWMV9;
    if (guidSubtype == MFVideoFormat_MP43)      return VideoCodecMPEG4;
    if (guidSubtype == MFVideoFormat_HEVC)      return VideoCodecH265;
    return VideoCodecUnknown;
}

AudioCodec StreamSinkHelper::GetCodecFromAudioSubtype(REFGUID guidSubtype)
{
    if (guidSubtype == MFAudioFormat_AAC)       return AudioCodecAAC;
    if (guidSubtype == MFAudioFormat_WMAudioV9) return AudioCodecWMA;
    if (guidSubtype == MFAudioFormat_MP3)       return AudioCodecMP3;
    if (guidSubtype == MFAudioFormat_PCM)       return AudioCodecPCM;
    return AudioCodecUnknown;
}

// ============================================================================
// Stream index helpers
// ============================================================================

bool StreamSinkHelper::IsVideoStream(IMFMediaType* pType)
{
    if (!pType)
        return false;

    GUID guidMajorType = GUID_NULL;
    if (FAILED(pType->GetGUID(MF_MT_MAJOR_TYPE, &guidMajorType)))
        return false;

    return guidMajorType == MFMediaType_Video;
}

bool StreamSinkHelper::IsAudioStream(IMFMediaType* pType)
{
    if (!pType)
        return false;

    GUID guidMajorType = GUID_NULL;
    if (FAILED(pType->GetGUID(MF_MT_MAJOR_TYPE, &guidMajorType)))
        return false;

    return guidMajorType == MFMediaType_Audio;
}

HRESULT StreamSinkHelper::GetStreamType(IMFSourceReader* pReader, DWORD dwStreamIndex, AVMediaType* pMediaType)
{
    if (!pReader || !pMediaType)
        return E_POINTER;

    *pMediaType = AVMediaTypeUnknown;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = pReader->GetCurrentMediaType(dwStreamIndex, &spType);
    if (FAILED(hr))
        return hr;

    if (IsVideoStream(spType))
        *pMediaType = AVMediaTypeVideo;
    else if (IsAudioStream(spType))
        *pMediaType = AVMediaTypeAudio;

    return S_OK;
}

// ============================================================================
// Frame rate helpers
// ============================================================================

HRESULT StreamSinkHelper::SetFrameRate(IMFMediaType* pType, UINT32 numerator, UINT32 denominator)
{
    if (!pType)
        return E_POINTER;
    return MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, numerator, denominator);
}

HRESULT StreamSinkHelper::GetFrameRate(IMFMediaType* pType, UINT32* pNumerator, UINT32* pDenominator)
{
    if (!pType)
        return E_POINTER;
    return MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, pNumerator, pDenominator);
}

HRESULT StreamSinkHelper::SetFrameRateAsDouble(IMFMediaType* pType, double dblFps)
{
    if (!pType)
        return E_POINTER;
    return MFSetAttributeRatio(pType, MF_MT_FRAME_RATE,
        static_cast<UINT32>(dblFps * 100), 100);
}

HRESULT StreamSinkHelper::GetFrameRateAsDouble(IMFMediaType* pType, double* pFps)
{
    if (!pType || !pFps)
        return E_POINTER;

    UINT32 num = 0, den = 0;
    HRESULT hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &num, &den);
    if (SUCCEEDED(hr) && den > 0)
        *pFps = static_cast<double>(num) / static_cast<double>(den);
    else
        *pFps = 0.0;

    return hr;
}

// ============================================================================
// Resolution helpers
// ============================================================================

HRESULT StreamSinkHelper::SetResolution(IMFMediaType* pType, UINT uWidth, UINT uHeight)
{
    if (!pType)
        return E_POINTER;
    return MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, uWidth, uHeight);
}

HRESULT StreamSinkHelper::GetResolution(IMFMediaType* pType, UINT* puWidth, UINT* puHeight)
{
    if (!pType)
        return E_POINTER;
    return MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, puWidth, puHeight);
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT StreamSinkHelper::CreateBaseVideoType(IMFMediaType** ppType)
{
    if (!ppType)
        return E_POINTER;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = MFCreateMediaType(&spType);
    if (FAILED(hr))
        return hr;

    spType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    spType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

    *ppType = spType.Detach();
    return S_OK;
}

HRESULT StreamSinkHelper::CreateBaseAudioType(IMFMediaType** ppType)
{
    if (!ppType)
        return E_POINTER;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = MFCreateMediaType(&spType);
    if (FAILED(hr))
        return hr;

    spType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);

    *ppType = spType.Detach();
    return S_OK;
}

} // namespace HMRAVSource
