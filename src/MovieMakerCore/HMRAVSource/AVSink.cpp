// AVSink.cpp - Audio/video output sink implementation

#include "pch.h"
#include "AVSink.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

AVSink::AVSink()
    : m_state(AVSinkStateUninitialized)
    , m_hVideoWnd(nullptr)
    , m_flVolume(1.0f)
    , m_fEnableVideo(true)
    , m_fEnableAudio(true)
    , m_dwVideoWidth(0)
    , m_dwVideoHeight(0)
    , m_dwAudioSampleRate(44100)
    , m_dwAudioChannels(2)
    , m_dwAudioBitsPerSample(16)
    , m_dwVideoFramesProcessed(0)
    , m_dwAudioSamplesProcessed(0)
{
}

AVSink::~AVSink()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT AVSink::Initialize(const AVSinkDesc& desc)
{
    if (m_state != AVSinkStateUninitialized)
        return E_UNEXPECTED;

    m_hVideoWnd = desc.hVideoWnd;
    m_flVolume = desc.flVolume;
    m_fEnableVideo = desc.fEnableVideo;
    m_fEnableAudio = desc.fEnableAudio;
    m_dwVideoWidth = desc.dwVideoWidth;
    m_dwVideoHeight = desc.dwVideoHeight;
    m_dwAudioSampleRate = desc.dwAudioSampleRate;
    m_dwAudioChannels = desc.dwAudioChannels;
    m_dwAudioBitsPerSample = desc.dwAudioBitsPerSample;

    HRESULT hr = S_OK;

    if (m_fEnableVideo && m_hVideoWnd)
    {
        hr = CreateVideoRenderer();
        if (FAILED(hr))
            return hr;
    }

    if (m_fEnableAudio)
    {
        hr = CreateAudioRenderer();
        if (FAILED(hr))
            return hr;
    }

    m_state = AVSinkStateReady;
    return S_OK;
}

HRESULT AVSink::Shutdown()
{
    if (m_state == AVSinkStateUninitialized)
        return S_OK;

    m_spVideoDisplay = nullptr;
    m_spAudioRenderer = nullptr;

    m_dwVideoFramesProcessed = 0;
    m_dwAudioSamplesProcessed = 0;
    m_state = AVSinkStateUninitialized;

    return S_OK;
}

bool AVSink::IsInitialized() const throw()
{
    return m_state != AVSinkStateUninitialized;
}

// ============================================================================
// Sample reception
// ============================================================================

HRESULT AVSink::ProcessVideoSample(IMFSample* pSample)
{
    if (!pSample)
        return E_POINTER;

    if (m_state != AVSinkStateReady && m_state != AVSinkStateRunning)
        return E_UNEXPECTED;

    if (!m_fEnableVideo)
        return S_FALSE;

    HRESULT hr = ProcessVideoFrameInternal(pSample);
    if (SUCCEEDED(hr))
    {
        m_dwVideoFramesProcessed++;

        LONGLONG llTimestamp = 0;
        pSample->GetSampleTime(&llTimestamp);

        if (m_renderCb)
            m_renderCb(S_OK, llTimestamp);
    }

    return hr;
}

HRESULT AVSink::ProcessAudioSample(IMFSample* pSample)
{
    if (!pSample)
        return E_POINTER;

    if (m_state != AVSinkStateReady && m_state != AVSinkStateRunning)
        return E_UNEXPECTED;

    if (!m_fEnableAudio)
        return S_FALSE;

    HRESULT hr = ProcessAudioBufferInternal(pSample);
    if (SUCCEEDED(hr))
        m_dwAudioSamplesProcessed++;

    return hr;
}

// ============================================================================
// Flush
// ============================================================================

HRESULT AVSink::Flush()
{
    m_dwVideoFramesProcessed = 0;
    m_dwAudioSamplesProcessed = 0;
    return S_OK;
}

// ============================================================================
// State
// ============================================================================

AVSinkInternalState AVSink::GetState() const throw()
{
    return m_state;
}

// ============================================================================
// Volume
// ============================================================================

HRESULT AVSink::SetVolume(float flVolume)
{
    if (flVolume < 0.0f) flVolume = 0.0f;
    if (flVolume > 1.0f) flVolume = 1.0f;
    m_flVolume = flVolume;
    return S_OK;
}

float AVSink::GetVolume() const throw()
{
    return m_flVolume;
}

// ============================================================================
// Video window
// ============================================================================

HRESULT AVSink::SetVideoWindow(HWND hWnd)
{
    m_hVideoWnd = hWnd;

    if (m_spVideoDisplay)
        return m_spVideoDisplay->SetVideoWindow(hWnd);

    return S_OK;
}

HWND AVSink::GetVideoWindow() const throw()
{
    return m_hVideoWnd;
}

// ============================================================================
// Format changes
// ============================================================================

HRESULT AVSink::SetVideoFormat(DWORD dwWidth, DWORD dwHeight, const GUID* /*pSubtype*/)
{
    m_dwVideoWidth = dwWidth;
    m_dwVideoHeight = dwHeight;
    return ConfigureVideoRenderer();
}

HRESULT AVSink::SetAudioFormat(DWORD dwSampleRate, DWORD dwChannels, DWORD dwBitsPerSample)
{
    m_dwAudioSampleRate = dwSampleRate;
    m_dwAudioChannels = dwChannels;
    m_dwAudioBitsPerSample = dwBitsPerSample;
    return ConfigureAudioRenderer();
}

// ============================================================================
// Statistics
// ============================================================================

DWORD AVSink::GetVideoFramesProcessed() const throw()
{
    return m_dwVideoFramesProcessed;
}

DWORD AVSink::GetAudioSamplesProcessed() const throw()
{
    return m_dwAudioSamplesProcessed;
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT AVSink::CreateVideoRenderer()
{
    if (!m_hVideoWnd)
        return E_INVALIDARG;

    // Create the Enhanced Video Renderer (EVR) for video display
    HRESULT hr = CoCreateInstance(
        CLSID_EnhancedVideoRenderer,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_spVideoDisplay));

    if (FAILED(hr))
        return hr;

    hr = m_spVideoDisplay->SetVideoWindow(m_hVideoWnd);
    if (FAILED(hr))
        return hr;

    hr = m_spVideoDisplay->SetAspectRatioMode(MFVideoARMode_PreservePicture);
    return hr;
}

HRESULT AVSink::CreateAudioRenderer()
{
    // Audio rendering is handled by the audio output subsystem
    // This creates a placeholder for the audio renderer control
    return S_OK;
}

HRESULT AVSink::ConfigureVideoRenderer()
{
    if (!m_spVideoDisplay)
        return S_OK;

    MFVideoNormalizedRect nrcSource = { 0.0f, 0.0f, 1.0f, 1.0f };
    RECT rcDest = { 0, 0, static_cast<LONG>(m_dwVideoWidth), static_cast<LONG>(m_dwVideoHeight) };

    return m_spVideoDisplay->SetVideoPosition(&nrcSource, &rcDest);
}

HRESULT AVSink::ConfigureAudioRenderer()
{
    return S_OK;
}

HRESULT AVSink::ProcessVideoFrameInternal(IMFSample* pSample)
{
    if (!pSample || !m_spVideoDisplay)
        return S_FALSE;

    LONGLONG llDuration = 0;
    pSample->GetSampleDuration(&llDuration);

    LONGLONG llTimestamp = 0;
    pSample->GetSampleTime(&llTimestamp);

    CComPtr<IMFVideoPresenter> spPresenter;
    HRESULT hr = m_spVideoDisplay->QueryInterface(IID_PPV_ARGS(&spPresenter));
    if (FAILED(hr))
        return hr;

    return spPresenter->ProcessMessage((MFVP_MESSAGE_TYPE)MFVP_MESSAGE_DISPLAYSAMPLE, (ULONG_PTR)pSample);
}

HRESULT AVSink::ProcessAudioBufferInternal(IMFSample* pSample)
{
    if (!pSample)
        return E_POINTER;

    // Extract audio data from sample buffers and render
    DWORD cBuffers = 0;
    pSample->GetBufferCount(&cBuffers);
    for (DWORD i = 0; i < cBuffers; ++i)
    {
        CComPtr<IMFMediaBuffer> spBuffer;
        HRESULT hr = pSample->GetBufferByIndex(i, &spBuffer);
        if (FAILED(hr))
            continue;

        BYTE* pData = nullptr;
        DWORD cbLength = 0;
        hr = spBuffer->Lock(&pData, nullptr, &cbLength);
        if (SUCCEEDED(hr))
        {
            // Audio data is ready for rendering
            spBuffer->Unlock();
        }
    }

    return S_OK;
}

} // namespace HMRAVSource
