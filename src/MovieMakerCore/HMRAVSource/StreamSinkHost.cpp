// StreamSinkHost.cpp - Stream sink host and audio stream sink implementation

#include "pch.h"
#include "StreamSinkHost.h"

namespace HMRAVSource
{

// ============================================================================
// AudioStreamSink
// ============================================================================

AudioStreamSink::AudioStreamSink()
    : m_dwStreamIndex(0)
    , m_state(StreamSinkStateIdle)
    , m_flVolume(1.0f)
    , m_fMute(false)
    , m_dwSamplesWritten(0)
    , m_llDurationWrittenHns(0)
{
}

AudioStreamSink::~AudioStreamSink()
{
    Shutdown();
}

HRESULT AudioStreamSink::Initialize(DWORD dwStreamIndex, const EncodeAudioParams& audioParams)
{
    m_dwStreamIndex = dwStreamIndex;
    m_audioParams = audioParams;
    m_state = StreamSinkStateReady;
    return S_OK;
}

HRESULT AudioStreamSink::Shutdown()
{
    m_spInputType = nullptr;
    m_spOutputType = nullptr;
    m_state = StreamSinkStateIdle;
    m_dwSamplesWritten = 0;
    m_llDurationWrittenHns = 0;
    return S_OK;
}

HRESULT AudioStreamSink::WriteSample(IMFSample* pSample)
{
    if (!pSample)
        return E_POINTER;

    if (m_state != StreamSinkStateReady && m_state != StreamSinkStateWriting)
        return E_UNEXPECTED;

    m_state = StreamSinkStateWriting;

    if (m_fMute)
        return S_FALSE;

    m_dwSamplesWritten++;

    LONGLONG llTimestamp = 0;
    if (SUCCEEDED(pSample->GetSampleTime(&llTimestamp)))
        m_llDurationWrittenHns = llTimestamp;

    return S_OK;
}

HRESULT AudioStreamSink::SetInputMediaType(IMFMediaType* pType)
{
    if (!pType)
        return E_POINTER;
    m_spInputType = pType;
    return S_OK;
}

HRESULT AudioStreamSink::SetOutputMediaType(IMFMediaType* pType)
{
    if (!pType)
        return E_POINTER;
    m_spOutputType = pType;
    return S_OK;
}

HRESULT AudioStreamSink::GetInputMediaType(IMFMediaType** ppType)
{
    if (!ppType)
        return E_POINTER;
    *ppType = m_spInputType;
    if (*ppType)
        (*ppType)->AddRef();
    return S_OK;
}

HRESULT AudioStreamSink::GetOutputMediaType(IMFMediaType** ppType)
{
    if (!ppType)
        return E_POINTER;
    *ppType = m_spOutputType;
    if (*ppType)
        (*ppType)->AddRef();
    return S_OK;
}

HRESULT AudioStreamSink::SetVolume(float flVolume)
{
    if (flVolume < 0.0f) flVolume = 0.0f;
    if (flVolume > 1.0f) flVolume = 1.0f;
    m_flVolume = flVolume;
    return S_OK;
}

float AudioStreamSink::GetVolume() const throw()
{
    return m_flVolume;
}

HRESULT AudioStreamSink::SetMute(bool fMute)
{
    m_fMute = fMute;
    return S_OK;
}

bool AudioStreamSink::IsMuted() const throw()
{
    return m_fMute;
}

StreamSinkState AudioStreamSink::GetState() const throw()
{
    return m_state;
}

DWORD AudioStreamSink::GetStreamIndex() const throw()
{
    return m_dwStreamIndex;
}

DWORD AudioStreamSink::GetSamplesWritten() const throw()
{
    return m_dwSamplesWritten;
}

LONGLONG AudioStreamSink::GetDurationWrittenHns() const throw()
{
    return m_llDurationWrittenHns;
}

DWORD AudioStreamSink::GetSampleRate() const throw()
{
    return m_audioParams.dwSampleRate;
}

DWORD AudioStreamSink::GetChannels() const throw()
{
    return m_audioParams.dwChannels;
}

DWORD AudioStreamSink::GetBitsPerSample() const throw()
{
    return m_audioParams.dwBitsPerSample;
}

HRESULT AudioStreamSink::ConvertAndScaleSample(IMFSample* pSample, IMFSample** ppConverted)
{
    if (!pSample || !ppConverted)
        return E_POINTER;

    *ppConverted = nullptr;

    // Volume scaling would be applied here if needed
    // For now, pass through unchanged
    *ppConverted = pSample;
    (*ppConverted)->AddRef();
    return S_OK;
}

// ============================================================================
// StreamSinkHost
// ============================================================================

StreamSinkHost::StreamSinkHost()
    : m_state(StreamSinkStateIdle)
    , m_dwVideoStreamIndex(0)
    , m_dwDefaultAudioIndex(0)
{
}

StreamSinkHost::~StreamSinkHost()
{
    Shutdown();
}

HRESULT StreamSinkHost::Initialize(LPCWSTR pszOutputPath, const EncodeProfile& profile)
{
    if (!pszOutputPath)
        return E_POINTER;

    if (m_state != StreamSinkStateIdle)
        return E_UNEXPECTED;

    m_strOutputPath = pszOutputPath;
    m_profile = profile;

    HRESULT hr = CreateSinkWriter();
    if (FAILED(hr))
        return hr;

    m_state = StreamSinkStateReady;
    return S_OK;
}

HRESULT StreamSinkHost::Shutdown()
{
    if (m_state == StreamSinkStateWriting)
        EndWriting();

    if (m_spSinkWriter)
    {
        m_spSinkWriter = nullptr;
    }

    for (SIZE_T i = 0; i < m_audioSinks.GetCount(); ++i)
    {
        m_audioSinks[i].Shutdown();
    }
    m_audioSinks.RemoveAll();

    m_state = StreamSinkStateIdle;
    m_stats = StreamSinkStats();
    return S_OK;
}

HRESULT StreamSinkHost::AddVideoStream(const EncodeVideoParams& videoParams)
{
    if (!m_spSinkWriter)
        return E_UNEXPECTED;

    CComPtr<IMFMediaType> spOutputType;
    HRESULT hr = MFCreateMediaType(&spOutputType);
    if (FAILED(hr))
        return hr;

    spOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    spOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

    switch (videoParams.codec)
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

    MFSetAttributeSize(spOutputType, MF_MT_FRAME_SIZE, videoParams.uWidth, videoParams.uHeight);
    MFSetAttributeRatio(spOutputType, MF_MT_FRAME_RATE,
        static_cast<UINT32>(videoParams.dblFrameRate * 100), 100);
    spOutputType->SetUINT32(MF_MT_AVG_BITRATE, videoParams.dwBitRate);

    if (videoParams.codec == VideoCodecH264)
    {
        spOutputType->SetUINT32(MF_MT_MPEG2_PROFILE, videoParams.uProfile);
        spOutputType->SetUINT32(MF_MT_MPEG2_LEVEL, videoParams.uLevel);
    }

    hr = m_spSinkWriter->AddStream(spOutputType, &m_dwVideoStreamIndex);
    return hr;
}

HRESULT StreamSinkHost::AddAudioStream(const EncodeAudioParams& audioParams)
{
    if (!m_spSinkWriter)
        return E_UNEXPECTED;

    DWORD dwStreamIndex = 0;

    CComPtr<IMFMediaType> spOutputType;
    HRESULT hr = MFCreateMediaType(&spOutputType);
    if (FAILED(hr))
        return hr;

    spOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);

    switch (audioParams.codec)
    {
    case AudioCodecAAC:
        spOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
        break;
    case AudioCodecWMA:
        spOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_WMAudioV9);
        break;
    default:
        spOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
        break;
    }

    spOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, audioParams.dwSampleRate);
    spOutputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audioParams.dwChannels);
    spOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, audioParams.dwBitsPerSample);
    spOutputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECTION, audioParams.dwBitRate / 8);

    hr = m_spSinkWriter->AddStream(spOutputType, &dwStreamIndex);
    if (FAILED(hr))
        return hr;

    AudioStreamSink audioSink;
    hr = audioSink.Initialize(dwStreamIndex, audioParams);
    if (FAILED(hr))
        return hr;

    audioSink.SetOutputMediaType(spOutputType);
    m_audioSinks.Add(audioSink);

    if (m_audioSinks.GetCount() == 1)
        m_dwDefaultAudioIndex = 0;

    return S_OK;
}

HRESULT StreamSinkHost::RemoveStream(DWORD /*dwStreamIndex*/)
{
    return S_OK;
}

HRESULT StreamSinkHost::BeginWriting()
{
    if (m_state != StreamSinkStateReady)
        return E_UNEXPECTED;

    HRESULT hr = m_spSinkWriter->BeginWriting();
    if (SUCCEEDED(hr))
    {
        m_state = StreamSinkStateWriting;
        m_stats = StreamSinkStats();
    }

    return hr;
}

HRESULT StreamSinkHost::WriteVideoSample(IMFSample* pSample)
{
    if (!pSample)
        return E_POINTER;

    if (m_state != StreamSinkStateWriting)
        return E_UNEXPECTED;

    HRESULT hr = m_spSinkWriter->WriteSample(m_dwVideoStreamIndex, pSample);
    if (SUCCEEDED(hr))
    {
        UpdateStats(pSample, true);
        FireProgress();
    }

    return hr;
}

HRESULT StreamSinkHost::WriteAudioSample(IMFSample* pSample, DWORD dwStreamIndex)
{
    if (!pSample)
        return E_POINTER;

    if (m_state != StreamSinkStateWriting)
        return E_UNEXPECTED;

    HRESULT hr = m_spSinkWriter->WriteSample(dwStreamIndex, pSample);
    if (SUCCEEDED(hr))
        UpdateStats(pSample, false);

    return hr;
}

HRESULT StreamSinkHost::WriteAudioSampleToDefault(IMFSample* pSample)
{
    if (m_audioSinks.GetCount() == 0)
        return E_UNEXPECTED;

    AudioStreamSink& defaultSink = m_audioSinks[m_dwDefaultAudioIndex];
    return WriteAudioSample(pSample, defaultSink.GetStreamIndex());
}

HRESULT StreamSinkHost::EndWriting()
{
    if (m_state != StreamSinkStateWriting)
        return E_UNEXPECTED;

    m_state = StreamSinkStateDraining;

    HRESULT hr = m_spSinkWriter->Finalize();
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
// Stream access
// ============================================================================

DWORD StreamSinkHost::GetStreamCount() const throw()
{
    return static_cast<DWORD>(m_audioSinks.GetCount()) + 1; // +1 for video
}

AudioStreamSink* StreamSinkHost::GetAudioSink(DWORD dwStreamIndex)
{
    if (dwStreamIndex >= m_audioSinks.GetCount())
        return nullptr;

    return &m_audioSinks[dwStreamIndex];
}

DWORD StreamSinkHost::GetDefaultAudioStreamIndex() const throw()
{
    return m_dwDefaultAudioIndex;
}

DWORD StreamSinkHost::GetVideoStreamIndex() const throw()
{
    return m_dwVideoStreamIndex;
}

// ============================================================================
// State
// ============================================================================

StreamSinkState StreamSinkHost::GetState() const throw()
{
    return m_state;
}

bool StreamSinkHost::IsWriting() const throw()
{
    return m_state == StreamSinkStateWriting;
}

// ============================================================================
// Statistics
// ============================================================================

const StreamSinkStats& StreamSinkHost::GetStats() const
{
    return m_stats;
}

void StreamSinkHost::ResetStats()
{
    m_stats = StreamSinkStats();
}

// ============================================================================
// Format overrides
// ============================================================================

HRESULT StreamSinkHost::SetInputVideoType(IMFMediaType* /*pType*/)
{
    return S_OK;
}

HRESULT StreamSinkHost::SetInputAudioType(DWORD dwStreamIndex, IMFMediaType* pType)
{
    AudioStreamSink* pSink = GetAudioSink(dwStreamIndex);
    if (!pSink)
        return E_INVALIDARG;

    return pSink->SetInputMediaType(pType);
}

HRESULT StreamSinkHost::SetOutputVideoType(IMFMediaType* /*pType*/)
{
    return S_OK;
}

HRESULT StreamSinkHost::SetOutputAudioType(DWORD dwStreamIndex, IMFMediaType* pType)
{
    AudioStreamSink* pSink = GetAudioSink(dwStreamIndex);
    if (!pSink)
        return E_INVALIDARG;

    return pSink->SetOutputMediaType(pType);
}

// ============================================================================
// Output
// ============================================================================

ATL::CString StreamSinkHost::GetOutputPath() const
{
    return m_strOutputPath;
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT StreamSinkHost::CreateSinkWriter()
{
    if (m_strOutputPath.IsEmpty())
        return E_INVALIDARG;

    return MFCreateSinkWriterFromURL(
        m_strOutputPath.GetString(),
        nullptr,
        nullptr,
        &m_spSinkWriter);
}

HRESULT StreamSinkHost::NegotiateVideoType()
{
    return S_OK;
}

HRESULT StreamSinkHost::NegotiateAudioType(AudioStreamSink& /*audioSink*/)
{
    return S_OK;
}

void StreamSinkHost::UpdateStats(IMFSample* pSample, bool fVideo)
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
        m_stats.llDurationWrittenHns = llSampleTime;

    if (fVideo)
        m_stats.dwFramesWritten++;
    else
        m_stats.dwAudioSamplesWritten++;
}

void StreamSinkHost::FireProgress()
{
    if (m_progressCb)
        m_progressCb(m_stats);
}

} // namespace HMRAVSource
