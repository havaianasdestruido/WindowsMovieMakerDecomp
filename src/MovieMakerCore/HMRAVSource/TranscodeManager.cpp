// TranscodeManager.cpp - Transcode orchestration implementation

#include "pch.h"
#include "TranscodeManager.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

TranscodeManager::TranscodeManager()
    : m_spSource(nullptr)
    , m_spSink(nullptr)
    , m_state(TranscodeStateIdle)
    , m_fTranscoding(false)
    , m_fPaused(false)
{
}

TranscodeManager::~TranscodeManager()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT TranscodeManager::Initialize()
{
    return S_OK;
}

HRESULT TranscodeManager::Shutdown()
{
    if (m_fTranscoding)
        CancelTranscode();

    if (m_spSource)
    {
        m_spSource->Release();
        m_spSource = nullptr;
    }

    if (m_spSink)
    {
        delete m_spSink;
        m_spSink = nullptr;
    }

    m_spVideoTransform = nullptr;
    m_spAudioTransform = nullptr;

    return S_OK;
}

// ============================================================================
// Transcode control
// ============================================================================

HRESULT TranscodeManager::BeginTranscode(const TranscodeParams& params)
{
    if (m_fTranscoding)
        return E_UNEXPECTED;

    m_params = params;
    m_progress = TranscodeProgress();
    m_fPaused = false;

    HRESULT hr = CreateSource();
    if (FAILED(hr))
    {
        FireError(hr);
        return hr;
    }

    hr = CreateSink();
    if (FAILED(hr))
    {
        FireError(hr);
        return hr;
    }

    AVSourceDesc srcDesc;
    srcDesc.strFilePath = params.strInputPath;
    srcDesc.fEnableVideo = params.fIncludeVideo;
    srcDesc.fEnableAudio = params.fIncludeAudio;

    hr = m_spSource->Open(srcDesc);
    if (FAILED(hr))
    {
        FireError(hr);
        return hr;
    }

    hr = ConfigureTransforms();
    if (FAILED(hr))
    {
        FireError(hr);
        return hr;
    }

    // Configure duration bounds
    if (params.llEndHns > 0)
        m_progress.llInputDurationHns = params.llEndHns - params.llStartHns;
    else
        m_progress.llInputDurationHns = m_spSource->GetDurationHns() - params.llStartHns;

    if (params.llStartHns > 0)
    {
        hr = m_spSource->SetPositionHns(params.llStartHns);
        if (FAILED(hr))
        {
            FireError(hr);
            return hr;
        }
    }

    hr = m_spSink->BeginWriting();
    if (FAILED(hr))
    {
        FireError(hr);
        return hr;
    }

    m_fTranscoding = true;
    m_progress.fComplete = false;
    m_progress.fCancelled = false;

    return S_OK;
}

HRESULT TranscodeManager::TranscodeFrame()
{
    if (!m_fTranscoding || m_fPaused)
        return E_UNEXPECTED;

    return ProcessNextFrame();
}

HRESULT TranscodeManager::EndTranscode()
{
    if (!m_fTranscoding)
        return E_UNEXPECTED;

    HRESULT hr = m_spSink->EndWriting();

    m_fTranscoding = false;
    m_progress.fComplete = true;

    FireComplete(hr);

    if (m_spSource)
    {
        m_spSource->Close();
    }

    return hr;
}

void TranscodeManager::CancelTranscode()
{
    if (!m_fTranscoding)
        return;

    m_fTranscoding = false;
    m_progress.fCancelled = true;

    if (m_spSink)
        m_spSink->Shutdown();

    if (m_spSource)
        m_spSource->Close();

    m_spVideoTransform = nullptr;
    m_spAudioTransform = nullptr;
}

// ============================================================================
// Status
// ============================================================================

TranscodeProgress TranscodeManager::GetProgress() const
{
    return m_progress;
}

bool TranscodeManager::IsTranscoding() const throw()
{
    return m_fTranscoding;
}

// ============================================================================
// Multi-frame encode
// ============================================================================

HRESULT TranscodeManager::TranscodeAllFrames()
{
    if (!m_fTranscoding)
        return E_UNEXPECTED;

    HRESULT hr = S_OK;

    while (m_fTranscoding && !m_progress.fCancelled)
    {
        hr = ProcessNextFrame();
        if (hr == MF_E_END_OF_STREAM)
        {
            hr = S_OK;
            break;
        }
        if (FAILED(hr))
            break;
    }

    if (SUCCEEDED(hr) && !m_progress.fCancelled)
    {
        hr = EndTranscode();
    }

    return hr;
}

HRESULT TranscodeManager::ProcessNextFrame()
{
    if (!m_fTranscoding)
        return E_UNEXPECTED;

    if (m_fPaused)
        return S_FALSE;

    // Read video sample
    CComPtr<IMFSample> spVideoSample;
    HRESULT hr = m_spSource->ReadSample(&spVideoSample, 0);

    if (hr == MF_E_END_OF_STREAM)
    {
        return MF_E_END_OF_STREAM;
    }

    if (FAILED(hr))
        return hr;

    if (spVideoSample)
    {
        hr = ProcessVideoFrame(spVideoSample);
        if (FAILED(hr))
            return hr;
    }

    // Read audio sample
    CComPtr<IMFSample> spAudioSample;
    hr = m_spSource->ReadSample(&spAudioSample, 1);

    if (SUCCEEDED(hr) && spAudioSample)
    {
        ProcessAudioSample(spAudioSample);
    }

    // Update progress
    LONGLONG llCurrentPos = m_spSource->GetPositionHns();
    UpdateProgress(llCurrentPos);

    return S_OK;
}

// ============================================================================
// Pause / Resume
// ============================================================================

HRESULT TranscodeManager::PauseTranscode()
{
    if (!m_fTranscoding || m_fPaused)
        return E_UNEXPECTED;

    m_fPaused = true;
    return S_OK;
}

HRESULT TranscodeManager::ResumeTranscode()
{
    if (!m_fTranscoding || !m_fPaused)
        return E_UNEXPECTED;

    m_fPaused = false;
    return S_OK;
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT TranscodeManager::CreateSource()
{
    MFSource* pSource = new (std::nothrow) MFSource();
    if (!pSource)
        return E_OUTOFMEMORY;

    m_spSource = pSource;
    return S_OK;
}

HRESULT TranscodeManager::CreateSink()
{
    m_spSink = new (std::nothrow) StreamSink();
    if (!m_spSink)
        return E_OUTOFMEMORY;

    StreamSinkDesc desc;
    desc.strOutputPath = m_params.strOutputPath;
    desc.profile = m_params.profile;
    desc.fHasVideo = m_params.fIncludeVideo;
    desc.fHasAudio = m_params.fIncludeAudio;

    return m_spSink->Initialize(desc);
}

HRESULT TranscodeManager::ConfigureTransforms()
{
    if (!m_spSink)
        return E_UNEXPECTED;

    if (m_params.fIncludeVideo && m_spVideoTransform)
    {
        CComPtr<IMFMediaType> spVideoInputType;
        HRESULT hr = MFCreateMediaType(&spVideoInputType);
        if (SUCCEEDED(hr))
        {
            spVideoInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            spVideoInputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
            spVideoInputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

            const EncodeVideoParams& video = m_params.profile.GetVideoParams();
            MFSetAttributeSize(spVideoInputType, MF_MT_FRAME_SIZE, video.uWidth, video.uHeight);
            MFSetAttributeRatio(spVideoInputType, MF_MT_FRAME_RATE,
                static_cast<UINT32>(video.dblFrameRate * 100), 100);

            m_spVideoTransform->SetInputType(0, spVideoInputType, 0);
        }

        CComPtr<IMFMediaType> spVideoOutputType;
        hr = MFCreateMediaType(&spVideoOutputType);
        if (SUCCEEDED(hr))
        {
            spVideoOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);

            const EncodeVideoParams& video = m_params.profile.GetVideoParams();
            switch (video.codec)
            {
            case VideoCodecH264:
                spVideoOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
                break;
            case VideoCodecWMV9:
                spVideoOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_WMVVC1);
                break;
            default:
                spVideoOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
                break;
            }

            spVideoOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
            MFSetAttributeSize(spVideoOutputType, MF_MT_FRAME_SIZE, video.uWidth, video.uHeight);
            MFSetAttributeRatio(spVideoOutputType, MF_MT_FRAME_RATE,
                static_cast<UINT32>(video.dblFrameRate * 100), 100);
            spVideoOutputType->SetUINT32(MF_MT_AVG_BITRATE, video.dwBitRate);

            if (video.codec == VideoCodecH264)
            {
                spVideoOutputType->SetUINT32(MF_MT_MPEG2_PROFILE, video.uProfile);
                spVideoOutputType->SetUINT32(MF_MT_MPEG2_LEVEL, video.uLevel);
            }

            m_spVideoTransform->SetOutputType(0, spVideoOutputType, 0);
        }

        m_spVideoTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
        m_spVideoTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    }

    if (m_params.fIncludeAudio && m_spAudioTransform)
    {
        CComPtr<IMFMediaType> spAudioInputType;
        HRESULT hr = MFCreateMediaType(&spAudioInputType);
        if (SUCCEEDED(hr))
        {
            spAudioInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
            spAudioInputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
            spAudioInputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);

            const EncodeAudioParams& audio = m_params.profile.GetAudioParams();
            spAudioInputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, audio.dwSampleRate);
            spAudioInputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audio.dwChannels);
            spAudioInputType->SetUINT32(MF_MT_BLOCK_ALIGNMENT, audio.dwChannels * 2);

            m_spAudioTransform->SetInputType(0, spAudioInputType, 0);
        }

        CComPtr<IMFMediaType> spAudioOutputType;
        hr = MFCreateMediaType(&spAudioOutputType);
        if (SUCCEEDED(hr))
        {
            spAudioOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);

            const EncodeAudioParams& audio = m_params.profile.GetAudioParams();
            switch (audio.codec)
            {
            case AudioCodecAAC:
                spAudioOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
                break;
            case AudioCodecWMA:
                spAudioOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_WMAudioV9);
                break;
            default:
                spAudioOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
                break;
            }

            spAudioOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, audio.dwSampleRate);
            spAudioOutputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audio.dwChannels);
            spAudioOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
            spAudioOutputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECTION, audio.dwBitRate / 8);
            spAudioOutputType->SetUINT32(MF_MT_BLOCK_ALIGNMENT, audio.dwChannels * 2);

            m_spAudioTransform->SetOutputType(0, spAudioOutputType, 0);
        }

        m_spAudioTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
        m_spAudioTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    }

    return S_OK;
}

HRESULT TranscodeManager::ProcessVideoFrame(IMFSample* pSample)
{
    if (!pSample || !m_spSink)
        return E_POINTER;

    if (!m_spVideoTransform)
        return m_spSink->WriteVideoSample(pSample);

    HRESULT hr = m_spVideoTransform->ProcessInput(0, pSample, 0);
    if (FAILED(hr))
        return hr;

    MFT_OUTPUT_STREAM_INFO streamInfo = {};
    hr = m_spVideoTransform->GetOutputStreamInfo(0, &streamInfo);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFSample> spOutputSample;
    hr = MFCreateSample(&spOutputSample);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFMediaBuffer> spBuffer;
    hr = MFCreateMemoryBuffer(streamInfo.cbSize, &spBuffer);
    if (FAILED(hr))
        return hr;

    hr = spOutputSample->AddBuffer(spBuffer);
    if (FAILED(hr))
        return hr;

    MFT_OUTPUT_DATA_BUFFER outputData = {};
    outputData.dwStreamID = 0;
    outputData.pSample = spOutputSample;

    DWORD dwStatus = 0;
    hr = m_spVideoTransform->ProcessOutput(0, 1, &outputData, &dwStatus);
    if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
        return S_OK;
    if (FAILED(hr))
        return hr;

    return m_spSink->WriteVideoSample(outputData.pSample);
}

HRESULT TranscodeManager::ProcessAudioSample(IMFSample* pSample)
{
    if (!pSample || !m_spSink)
        return E_POINTER;

    if (!m_spAudioTransform)
        return m_spSink->WriteAudioSample(pSample);

    HRESULT hr = m_spAudioTransform->ProcessInput(0, pSample, 0);
    if (FAILED(hr))
        return hr;

    MFT_OUTPUT_STREAM_INFO streamInfo = {};
    hr = m_spAudioTransform->GetOutputStreamInfo(0, &streamInfo);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFSample> spOutputSample;
    hr = MFCreateSample(&spOutputSample);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFMediaBuffer> spBuffer;
    hr = MFCreateMemoryBuffer(streamInfo.cbSize, &spBuffer);
    if (FAILED(hr))
        return hr;

    hr = spOutputSample->AddBuffer(spBuffer);
    if (FAILED(hr))
        return hr;

    MFT_OUTPUT_DATA_BUFFER outputData = {};
    outputData.dwStreamID = 0;
    outputData.pSample = spOutputSample;

    DWORD dwStatus = 0;
    hr = m_spAudioTransform->ProcessOutput(0, 1, &outputData, &dwStatus);
    if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
        return S_OK;
    if (FAILED(hr))
        return hr;

    return m_spSink->WriteAudioSample(outputData.pSample);
}

void TranscodeManager::UpdateProgress(LONGLONG llPosition)
{
    m_progress.llCurrentPositionHns = llPosition;

    if (m_progress.llInputDurationHns > 0)
    {
        double dblRatio = static_cast<double>(llPosition - m_params.llStartHns) /
                          static_cast<double>(m_progress.llInputDurationHns);
        if (dblRatio < 0.0) dblRatio = 0.0;
        if (dblRatio > 1.0) dblRatio = 1.0;
        m_progress.fPercentComplete = static_cast<float>(dblRatio * 100.0);
    }

    m_progress.dwFramesEncoded++;

    if (m_progressCb)
        m_progressCb(m_progress);
}

void TranscodeManager::FireError(HRESULT hr)
{
    m_progress.fError = true;
    m_progress.hrError = hr;

    if (m_errorCb)
        m_errorCb(hr, L"Transcode error occurred");
}

void TranscodeManager::FireComplete(HRESULT hr)
{
    if (m_completeCb)
        m_completeCb(hr, m_params.strOutputPath.GetString());
}

} // namespace HMRAVSource
