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

    hr = m_spSource->Open(AVSourceDesc());
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

    if (m_params.fIncludeVideo)
    {
        UINT32 unFlags = MFT_ENUM_FLAG_SYNCMFT;
        if (m_params.fHardwareAcceleration)
            unFlags = MFT_ENUM_FLAG_HARDWARE | MFT_ENUM_FLAG_SYNCMFT;

        IMFActivate** ppActivates = nullptr;
        UINT32 cMFTs = 0;
        HRESULT hr = MFTEnumEx(
            MFT_CATEGORY_VIDEO_ENCODER,
            unFlags,
            nullptr,
            nullptr,
            &ppActivates,
            &cMFTs);

        if (SUCCEEDED(hr) && cMFTs > 0)
        {
            ppActivates[0]->QueryInterface(IID_PPV_ARGS(&m_spVideoTransform));
            for (UINT32 i = 0; i < cMFTs; ++i)
                ppActivates[i]->Release();
            CoTaskMemFree(ppActivates);
        }

        if (m_spVideoTransform)
        {
            CComPtr<IMFAttributes> spAttrs;
            if (SUCCEEDED(m_spVideoTransform->GetAttributes(&spAttrs)))
            {
                spAttrs->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
            }
        }
    }

    if (m_params.fIncludeAudio)
    {
        IMFActivate** ppActivates = nullptr;
        UINT32 cMFTs = 0;
        HRESULT hr = MFTEnumEx(
            MFT_CATEGORY_AUDIO_ENCODER,
            MFT_ENUM_FLAG_SYNCMFT,
            nullptr,
            nullptr,
            &ppActivates,
            &cMFTs);

        if (SUCCEEDED(hr) && cMFTs > 0)
        {
            ppActivates[0]->QueryInterface(IID_PPV_ARGS(&m_spAudioTransform));
            for (UINT32 i = 0; i < cMFTs; ++i)
                ppActivates[i]->Release();
            CoTaskMemFree(ppActivates);
        }
    }

    return S_OK;
}

HRESULT TranscodeManager::ProcessVideoFrame(IMFSample* pSample)
{
    if (!pSample || !m_spSink)
        return E_POINTER;

    return m_spSink->WriteVideoSample(pSample);
}

HRESULT TranscodeManager::ProcessAudioSample(IMFSample* pSample)
{
    if (!pSample || !m_spSink)
        return E_POINTER;

    return m_spSink->WriteAudioSample(pSample);
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
