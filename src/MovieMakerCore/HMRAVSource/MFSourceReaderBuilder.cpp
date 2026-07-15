// MFSourceReaderBuilder.cpp - Base source reader builder implementation

#include "pch.h"
#include "MFSourceReaderBuilder.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

MFSourceReaderBuilder::MFSourceReaderBuilder()
    : m_fInitialized(false)
{
}

MFSourceReaderBuilder::~MFSourceReaderBuilder()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT MFSourceReaderBuilder::Initialize()
{
    m_fInitialized = true;
    return S_OK;
}

HRESULT MFSourceReaderBuilder::Shutdown()
{
    m_fInitialized = false;
    return S_OK;
}

// ============================================================================
// Source reader creation
// ============================================================================

HRESULT MFSourceReaderBuilder::CreateSourceReader(LPCWSTR pszFilePath, IMFSourceReader** ppReader)
{
    return CreateSourceReaderFromFile(pszFilePath, m_config, ppReader);
}

HRESULT MFSourceReaderBuilder::CreateSourceReaderFromFile(
    LPCWSTR pszFilePath,
    const SourceReaderBuilderConfig& config,
    IMFSourceReader** ppReader)
{
    if (!pszFilePath || !ppReader)
        return E_POINTER;

    *ppReader = nullptr;

    CComPtr<IMFAttributes> spAttributes;
    HRESULT hr = CreateSourceReaderAttributes(&spAttributes);
    if (FAILED(hr))
        return hr;

    if (config.fEnableHardwareAcceleration)
    {
        spAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
    }

    hr = MFCreateSourceReaderFromURL(pszFilePath, spAttributes, ppReader);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ============================================================================
// Codec detection
// ============================================================================

HRESULT MFSourceReaderBuilder::EnumerateVideoCodecs(ATL::CAtlArray<CodecInfo>& codecs)
{
    codecs.RemoveAll();

    struct VideoCodecEntry { GUID guid; LPCWSTR pszName; bool fHw; };
    VideoCodecEntry videoCodecs[] =
    {
        { MFVideoFormat_H264,   L"H.264/AVC",    true  },
        { MFVideoFormat_HEVC,   L"H.265/HEVC",   true  },
        { MFVideoFormat_WMVVC1, L"WMV9/VC-1",     false },
        { MFVideoFormat_MP43,   L"MPEG-4 Part 2", false },
        { MFVideoFormat_YUY2,   L"Uncompressed YUY2", false },
        { MFVideoFormat_NV12,   L"Uncompressed NV12", false },
    };

    for (size_t i = 0; i < _countof(videoCodecs); ++i)
    {
        CodecInfo info = {};
        info.guidSubtype = videoCodecs[i].guid;
        info.strFriendlyName = videoCodecs[i].pszName;
        info.fIsHardwareCodec = videoCodecs[i].fHw;
        codecs.Add(info);
    }

    return S_OK;
}

HRESULT MFSourceReaderBuilder::EnumerateAudioCodecs(ATL::CAtlArray<CodecInfo>& codecs)
{
    codecs.RemoveAll();

    struct AudioCodecEntry { GUID guid; LPCWSTR pszName; };
    AudioCodecEntry audioCodecs[] =
    {
        { MFAudioFormat_AAC,        L"AAC"      },
        { MFAudioFormat_WMAudioV9,  L"WMA 9"    },
        { MFAudioFormat_MP3,        L"MP3"      },
        { MFAudioFormat_PCM,        L"PCM"      },
    };

    for (size_t i = 0; i < ARRAYSIZE(audioCodecs); ++i)
    {
        CodecInfo info = {};
        info.guidSubtype = audioCodecs[i].guid;
        info.strFriendlyName = audioCodecs[i].pszName;
        info.fIsHardwareCodec = false;
        codecs.Add(info);
    }

    return S_OK;
}

bool MFSourceReaderBuilder::IsCodecAvailable(REFGUID guidSubtype)
{
    // Check if a codec MFT is registered
    MFT_REGISTER_TYPE_INFO info = {};
    info.guidMajorType = (guidSubtype == MFVideoFormat_H264 ||
                          guidSubtype == MFVideoFormat_HEVC ||
                          guidSubtype == MFVideoFormat_WMVVC1 ||
                          guidSubtype == MFVideoFormat_MP43)
                         ? MFMediaType_Video : MFMediaType_Audio;
    info.guidSubtype = guidSubtype;

    CLSID* pClsids = nullptr;
    UINT32 cClsids = 0;

    HRESULT hr = MFTEnum(
        MFT_CATEGORY_VIDEO_DECODER,
        0,
        &info,
        nullptr,
        nullptr,
        &pClsids,
        &cClsids);

    if (SUCCEEDED(hr))
    {
        CoTaskMemFree(pClsids);
    }

    return cClsids > 0;
}

HRESULT MFSourceReaderBuilder::GetCodecFriendlyName(REFGUID guidSubtype, ATL::CString* pstrName)
{
    if (!pstrName)
        return E_POINTER;

    if (guidSubtype == MFVideoFormat_H264)       { *pstrName = L"H.264/AVC";      return S_OK; }
    if (guidSubtype == MFVideoFormat_HEVC)        { *pstrName = L"H.265/HEVC";     return S_OK; }
    if (guidSubtype == MFVideoFormat_WMVVC1)      { *pstrName = L"WMV9/VC-1";      return S_OK; }
    if (guidSubtype == MFVideoFormat_MP43)        { *pstrName = L"MPEG-4 Part 2";  return S_OK; }
    if (guidSubtype == MFAudioFormat_AAC)         { *pstrName = L"AAC";            return S_OK; }
    if (guidSubtype == MFAudioFormat_WMAudioV9)   { *pstrName = L"WMA 9";          return S_OK; }
    if (guidSubtype == MFAudioFormat_MP3)         { *pstrName = L"MP3";            return S_OK; }
    if (guidSubtype == MFAudioFormat_PCM)         { *pstrName = L"PCM";            return S_OK; }

    *pstrName = L"Unknown";
    return MF_E_INVALIDMEDIATYPE;
}

// ============================================================================
// Stream enumeration
// ============================================================================

HRESULT MFSourceReaderBuilder::EnumerateStreams(
    IMFSourceReader* pReader,
    DWORD* pdwVideoStream,
    DWORD* pdwAudioStream,
    DWORD* pdwStreamCount)
{
    if (!pReader)
        return E_POINTER;

    DWORD dwVideoStream = DWORD_MAX;
    DWORD dwAudioStream = DWORD_MAX;
    DWORD dwCount = 0;

    for (DWORD dwIndex = 0; ; ++dwIndex)
    {
        CComPtr<IMFMediaType> spType;
        HRESULT hr = pReader->GetNativeMediaType(dwIndex, 0, &spType);
        if (FAILED(hr))
            break;

        GUID guidMajorType = GUID_NULL;
        hr = spType->GetGUID(MF_MT_MAJOR_TYPE, &guidMajorType);
        if (FAILED(hr))
            break;

        if (guidMajorType == MFMediaType_Video && dwVideoStream == DWORD_MAX)
            dwVideoStream = dwIndex;
        else if (guidMajorType == MFMediaType_Audio && dwAudioStream == DWORD_MAX)
            dwAudioStream = dwIndex;

        dwCount++;
    }

    if (pdwVideoStream) *pdwVideoStream = dwVideoStream;
    if (pdwAudioStream) *pdwAudioStream = dwAudioStream;
    if (pdwStreamCount) *pdwStreamCount = dwCount;

    return S_OK;
}

HRESULT MFSourceReaderBuilder::GetStreamInfo(
    IMFSourceReader* pReader,
    DWORD dwStreamIndex,
    AVMediaType* pMediaType,
    IMFMediaType** ppNativeType)
{
    if (!pReader)
        return E_POINTER;

    if (pMediaType)
        *pMediaType = AVMediaTypeUnknown;

    if (ppNativeType)
    {
        HRESULT hr = pReader->GetNativeMediaType(dwStreamIndex, 0, ppNativeType);
        if (FAILED(hr))
            return hr;

        GUID guidMajorType = GUID_NULL;
        (*ppNativeType)->GetGUID(MF_MT_MAJOR_TYPE, &guidMajorType);

        if (pMediaType)
        {
            if (guidMajorType == MFMediaType_Video)
                *pMediaType = AVMediaTypeVideo;
            else if (guidMajorType == MFMediaType_Audio)
                *pMediaType = AVMediaTypeAudio;
        }
    }

    return S_OK;
}

// ============================================================================
// Format negotiation
// ============================================================================

HRESULT MFSourceReaderBuilder::NegotiateVideoFormat(
    IMFSourceReader* pReader,
    DWORD dwStreamIndex,
    UINT uWidth,
    UINT uHeight)
{
    if (!pReader)
        return E_POINTER;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = MFCreateMediaType(&spType);
    if (FAILED(hr))
        return hr;

    spType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    spType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    spType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(spType, MF_MT_FRAME_SIZE, uWidth, uHeight);

    return pReader->SetCurrentMediaType(dwStreamIndex, nullptr, spType);
}

HRESULT MFSourceReaderBuilder::NegotiateAudioFormat(
    IMFSourceReader* pReader,
    DWORD dwStreamIndex,
    DWORD dwSampleRate,
    DWORD dwChannels)
{
    if (!pReader)
        return E_POINTER;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = MFCreateMediaType(&spType);
    if (FAILED(hr))
        return hr;

    spType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    spType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    spType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, dwSampleRate);
    spType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, dwChannels);
    spType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    spType->SetUINT32(MF_MT_BLOCK_ALIGNMENT, dwChannels * 2);

    return pReader->SetCurrentMediaType(dwStreamIndex, nullptr, spType);
}

// ============================================================================
// Configuration
// ============================================================================

void MFSourceReaderBuilder::SetConfig(const SourceReaderBuilderConfig& config)
{
    m_config = config;
}

const SourceReaderBuilderConfig& MFSourceReaderBuilder::GetConfig() const
{
    return m_config;
}

// ============================================================================
// Codec capability queries
// ============================================================================

bool MFSourceReaderBuilder::IsH264Supported()
{
    return IsCodecAvailable(MFVideoFormat_H264);
}

bool MFSourceReaderBuilder::IsWMVSupported()
{
    return IsCodecAvailable(MFVideoFormat_WMVVC1);
}

bool MFSourceReaderBuilder::IsAACSupported()
{
    return IsCodecAvailable(MFAudioFormat_AAC);
}

bool MFSourceReaderBuilder::IsWMASupported()
{
    return IsCodecAvailable(MFAudioFormat_WMAudioV9);
}

bool MFSourceReaderBuilder::IsMP3Supported()
{
    return IsCodecAvailable(MFAudioFormat_MP3);
}

bool MFSourceReaderBuilder::IsHEVCSupported()
{
    return IsCodecAvailable(MFVideoFormat_HEVC);
}

// ============================================================================
// Protected helpers
// ============================================================================

HRESULT MFSourceReaderBuilder::ConfigureSourceReader(IMFSourceReader* pReader)
{
    if (!pReader)
        return E_POINTER;

    DWORD dwVideoStream = 0;
    DWORD dwAudioStream = 1;
    DWORD dwStreamCount = 0;

    HRESULT hr = EnumerateStreams(pReader, &dwVideoStream, &dwAudioStream, &dwStreamCount);
    if (FAILED(hr))
        return hr;

    if (m_config.uPreferredVideoWidth > 0 && m_config.uPreferredVideoHeight > 0)
    {
        SelectBestVideoType(pReader, dwVideoStream);
    }

    if (m_config.dwPreferredAudioSampleRate > 0)
    {
        SelectBestAudioType(pReader, dwAudioStream);
    }

    return S_OK;
}

HRESULT MFSourceReaderBuilder::SelectBestVideoType(IMFSourceReader* pReader, DWORD dwStreamIndex)
{
    if (!pReader)
        return E_POINTER;

    // Try to set a compatible video output type
    CComPtr<IMFMediaType> spDesired;
    HRESULT hr = MFCreateMediaType(&spDesired);
    if (FAILED(hr))
        return hr;

    spDesired->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    spDesired->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    spDesired->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

    if (m_config.uPreferredVideoWidth > 0 && m_config.uPreferredVideoHeight > 0)
    {
        MFSetAttributeSize(spDesired, MF_MT_FRAME_SIZE, m_config.uPreferredVideoWidth, m_config.uPreferredVideoHeight);
    }

    return pReader->SetCurrentMediaType(dwStreamIndex, nullptr, spDesired);
}

HRESULT MFSourceReaderBuilder::SelectBestAudioType(IMFSourceReader* pReader, DWORD dwStreamIndex)
{
    if (!pReader)
        return E_POINTER;

    CComPtr<IMFMediaType> spDesired;
    HRESULT hr = MFCreateMediaType(&spDesired);
    if (FAILED(hr))
        return hr;

    spDesired->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    spDesired->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    spDesired->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);

    if (m_config.dwPreferredAudioSampleRate > 0)
        spDesired->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_config.dwPreferredAudioSampleRate);

    if (m_config.dwPreferredAudioChannels > 0)
        spDesired->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_config.dwPreferredAudioChannels);

    return pReader->SetCurrentMediaType(dwStreamIndex, nullptr, spDesired);
}

HRESULT MFSourceReaderBuilder::CreateSourceReaderAttributes(IMFAttributes** ppAttributes)
{
    if (!ppAttributes)
        return E_POINTER;

    return MFCreateAttributes(ppAttributes, 2);
}

} // namespace HMRAVSource
