// StreamSink.cpp - Media Foundation stream sink implementation

#include "pch.h"
#include "StreamSink.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

StreamSink::StreamSink()
    : m_state(StreamSinkStateIdle)
    , m_dwVideoStreamIndex(0)
    , m_dwAudioStreamIndex(1)
    , m_llTotalDurationHns(0)
{
}

StreamSink::~StreamSink()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT StreamSink::Initialize(const StreamSinkDesc& desc)
{
    if (m_state != StreamSinkStateIdle)
        return E_UNEXPECTED;

    m_desc = desc;
    m_dwVideoStreamIndex = desc.dwVideoStreamIndex;
    m_dwAudioStreamIndex = desc.dwAudioStreamIndex;

    // Create the sink writer
    HRESULT hr = MFCreateSinkWriterFromURL(
        desc.strOutputPath.GetString(),
        nullptr,
        nullptr,
        &m_spSinkWriter);

    if (FAILED(hr))
        return hr;

    m_state = StreamSinkStateReady;
    return S_OK;
}

HRESULT StreamSink::Shutdown()
{
    if (m_state == StreamSinkStateWriting)
        EndWriting();

    if (m_spSinkWriter)
    {
        m_spSinkWriter = nullptr;
    }

    m_state = StreamSinkStateIdle;
    m_stats = StreamSinkStats();
    return S_OK;
}

// ============================================================================
// Writing
// ============================================================================

HRESULT StreamSink::BeginWriting()
{
    if (m_state != StreamSinkStateReady)
        return E_UNEXPECTED;

    HRESULT hr = S_OK;

    if (m_desc.fHasVideo)
    {
        hr = ConfigureVideoStream();
        if (FAILED(hr))
            return hr;
    }

    if (m_desc.fHasAudio)
    {
        hr = ConfigureAudioStream();
        if (FAILED(hr))
            return hr;
    }

    hr = m_spSinkWriter->BeginWriting();
    if (FAILED(hr))
        return hr;

    m_state = StreamSinkStateWriting;
    m_stats = StreamSinkStats();

    return S_OK;
}

HRESULT StreamSink::WriteVideoSample(IMFSample* pSample)
{
    if (!pSample)
        return E_POINTER;

    if (m_state != StreamSinkStateWriting)
        return E_UNEXPECTED;

    LONGLONG llTimestamp = 0;
    pSample->GetSampleTime(&llTimestamp);

    HRESULT hr = m_spSinkWriter->WriteSample(m_dwVideoStreamIndex, pSample);
    if (SUCCEEDED(hr))
    {
        UpdateStats(pSample, true);
        FireProgress();
    }

    return hr;
}

HRESULT StreamSink::WriteAudioSample(IMFSample* pSample)
{
    if (!pSample)
        return E_POINTER;

    if (m_state != StreamSinkStateWriting)
        return E_UNEXPECTED;

    HRESULT hr = m_spSinkWriter->WriteSample(m_dwAudioStreamIndex, pSample);
    if (SUCCEEDED(hr))
    {
        UpdateStats(pSample, false);
    }

    return hr;
}

HRESULT StreamSink::EndWriting()
{
    if (m_state != StreamSinkStateWriting)
        return E_UNEXPECTED;

    m_state = StreamSinkStateDraining;

    HRESULT hr = FinalizeWriter();
    if (SUCCEEDED(hr))
    {
        m_stats.fComplete = true;
        m_state = StreamSinkStateDone;
        FireProgress();
    }
    else
    {
        m_state = StreamSinkStateError;
    }

    return hr;
}

// ============================================================================
// Format
// ============================================================================

HRESULT StreamSink::SetInputVideoType(IMFMediaType* pType)
{
    if (!pType)
        return E_POINTER;
    m_spInputVideoType = pType;
    return S_OK;
}

HRESULT StreamSink::SetInputAudioType(IMFMediaType* pType)
{
    if (!pType)
        return E_POINTER;
    m_spInputAudioType = pType;
    return S_OK;
}

HRESULT StreamSink::SetOutputVideoType(IMFMediaType* pType)
{
    if (!pType)
        return E_POINTER;
    m_spOutputVideoType = pType;
    return S_OK;
}

HRESULT StreamSink::SetOutputAudioType(IMFMediaType* pType)
{
    if (!pType)
        return E_POINTER;
    m_spOutputAudioType = pType;
    return S_OK;
}

// ============================================================================
// Query
// ============================================================================

StreamSinkState StreamSink::GetState() const throw()
{
    return m_state;
}

const StreamSinkStats& StreamSink::GetStats() const
{
    return m_stats;
}

// ============================================================================
// Flush
// ============================================================================

HRESULT StreamSink::Flush()
{
    if (m_state != StreamSinkStateWriting)
        return E_UNEXPECTED;

    HRESULT hr = m_spSinkWriter->Flush(m_dwVideoStreamIndex);
    if (SUCCEEDED(hr) && m_desc.fHasAudio)
        hr = m_spSinkWriter->Flush(m_dwAudioStreamIndex);

    return hr;
}

// ============================================================================
// Output file
// ============================================================================

ATL::CString StreamSink::GetOutputPath() const
{
    return m_desc.strOutputPath;
}

// ============================================================================
// Duration tracking
// ============================================================================

void StreamSink::SetTotalDurationHns(LONGLONG llDuration) throw()
{
    m_llTotalDurationHns = llDuration;
}

LONGLONG StreamSink::GetTotalDurationHns() const throw()
{
    return m_llTotalDurationHns;
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT StreamSink::ConfigureVideoStream()
{
    if (!m_spSinkWriter)
        return E_UNEXPECTED;

    const EncodeVideoParams& video = m_desc.profile.GetVideoParams();

    CComPtr<IMFMediaType> spOutputType;
    HRESULT hr = MFCreateMediaType(&spOutputType);
    if (FAILED(hr))
        return hr;

    spOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    spOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

    switch (video.codec)
    {
    case VideoCodecH264:
        spOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
        break;
    case VideoCodecWMV9:
        spOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_WMVVC1);
        break;
    default:
        spOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
        break;
    }

    MFSetAttributeSize(spOutputType, MF_MT_FRAME_SIZE, video.uWidth, video.uHeight);
    MFSetAttributeRatio(spOutputType, MF_MT_FRAME_RATE,
        static_cast<UINT32>(video.dblFrameRate * 100), 100);
    spOutputType->SetUINT32(MF_MT_AVG_BITRATE, video.dwBitRate);

    if (video.codec == VideoCodecH264)
    {
        spOutputType->SetUINT32(MF_MT_MPEG2_PROFILE, video.uProfile);
        spOutputType->SetUINT32(MF_MT_MPEG2_LEVEL, video.uLevel);
    }

    // Create input type (NV12 raw frames)
    CComPtr<IMFMediaType> spInputType;
    hr = MFCreateMediaType(&spInputType);
    if (FAILED(hr))
        return hr;

    spInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    spInputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    spInputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(spInputType, MF_MT_FRAME_SIZE, video.uWidth, video.uHeight);
    MFSetAttributeRatio(spInputType, MF_MT_FRAME_RATE,
        static_cast<UINT32>(video.dblFrameRate * 100), 100);

    hr = m_spSinkWriter->AddStream(spOutputType, &m_dwVideoStreamIndex);
    if (FAILED(hr))
        return hr;

    hr = m_spSinkWriter->SetInputMediaType(m_dwVideoStreamIndex, spInputType, nullptr);
    return hr;
}

HRESULT StreamSink::ConfigureAudioStream()
{
    if (!m_spSinkWriter)
        return E_UNEXPECTED;

    const EncodeAudioParams& audio = m_desc.profile.GetAudioParams();

    CComPtr<IMFMediaType> spOutputType;
    HRESULT hr = MFCreateMediaType(&spOutputType);
    if (FAILED(hr))
        return hr;

    spOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);

    switch (audio.codec)
    {
    case AudioCodecAAC:
        spOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
        spOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
        break;
    case AudioCodecWMA:
        spOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_WMAudioV9);
        spOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
        break;
    default:
        spOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
        spOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
        break;
    }

    spOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, audio.dwSampleRate);
    spOutputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audio.dwChannels);
    spOutputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECTION,
        audio.dwBitRate > 0 ? audio.dwBitRate / 8 : audio.dwSampleRate * audio.dwChannels * 2);

    // Input type (PCM)
    CComPtr<IMFMediaType> spInputType;
    hr = MFCreateMediaType(&spInputType);
    if (FAILED(hr))
        return hr;

    spInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    spInputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    spInputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    spInputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, audio.dwSampleRate);
    spInputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audio.dwChannels);
    spInputType->SetUINT32(MF_MT_BLOCK_ALIGNMENT, audio.dwChannels * 2);

    hr = m_spSinkWriter->AddStream(spOutputType, &m_dwAudioStreamIndex);
    if (FAILED(hr))
        return hr;

    hr = m_spSinkWriter->SetInputMediaType(m_dwAudioStreamIndex, spInputType, nullptr);
    return hr;
}

HRESULT StreamSink::FinalizeWriter()
{
    if (!m_spSinkWriter)
        return E_UNEXPECTED;

    return m_spSinkWriter->Finalize();
}

void StreamSink::UpdateStats(IMFSample* pSample, bool fVideo)
{
    if (!pSample)
        return;

    DWORD cbTotal = 0;
    DWORD cBuffers = 0;
    pSample->GetBufferCount(&cBuffers);
    for (DWORD i = 0; i < cBuffers; ++i)
    {
        CComPtr<IMFMediaBuffer> spBuffer;
        if (SUCCEEDED(pSample->GetBufferByIndex(i, &spBuffer)))
        {
            DWORD cbCurrentLength = 0;
            spBuffer->GetCurrentLength(&cbCurrentLength);
            cbTotal += cbCurrentLength;
        }
    }

    m_stats.llTotalBytesWritten += cbTotal;

    LONGLONG llSampleTime = 0;
    if (SUCCEEDED(pSample->GetSampleTime(&llSampleTime)))
    {
        m_stats.llDurationWrittenHns = llSampleTime;
    }

    if (fVideo)
        m_stats.dwFramesWritten++;
    else
        m_stats.dwAudioSamplesWritten++;
}

void StreamSink::FireProgress()
{
    if (m_progressCb)
        m_progressCb(m_stats);
}

} // namespace HMRAVSource
