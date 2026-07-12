// AVCaptureSession.cpp - Capture session management implementation

#include "pch.h"
#include "AVCaptureSession.h"

namespace HMRAVSource
{

// ============================================================================
// AVCaptureSession
// ============================================================================

AVCaptureSession::AVCaptureSession()
    : m_pVideoCapture(nullptr)
    , m_pAudioCapture(nullptr)
    , m_state(CaptureStateIdle)
    , m_llStartTimeHns(0)
    , m_llMaxDurationHns(0)
    , m_fActive(false)
{
}

AVCaptureSession::~AVCaptureSession()
{
    Shutdown();
}

HRESULT AVCaptureSession::Initialize(const AVCaptureSessionDesc& desc)
{
    m_desc = desc;
    m_llMaxDurationHns = desc.llMaxDurationHns;

    if (desc.fCaptureVideo)
    {
        m_pVideoCapture = new (std::nothrow) VideoCapture();
        if (!m_pVideoCapture)
            return E_OUTOFMEMORY;

        HRESULT hr = m_pVideoCapture->Initialize(desc.videoDevice);
        if (FAILED(hr))
            return hr;

        m_pVideoCapture->SetConfig(desc.videoConfig);
    }

    if (desc.fCaptureAudio)
    {
        m_pAudioCapture = new (std::nothrow) AudioCapture();
        if (!m_pAudioCapture)
            return E_OUTOFMEMORY;

        HRESULT hr = m_pAudioCapture->Initialize(desc.audioDevice);
        if (FAILED(hr))
            return hr;

        m_pAudioCapture->SetConfig(desc.audioConfig);
    }

    m_state = CaptureStateIdle;
    return S_OK;
}

HRESULT AVCaptureSession::Shutdown()
{
    if (m_fActive)
        StopSession();

    if (m_pVideoCapture)
    {
        m_pVideoCapture->Shutdown();
        delete m_pVideoCapture;
        m_pVideoCapture = nullptr;
    }

    if (m_pAudioCapture)
    {
        m_pAudioCapture->Shutdown();
        delete m_pAudioCapture;
        m_pAudioCapture = nullptr;
    }

    m_state = CaptureStateIdle;
    return S_OK;
}

HRESULT AVCaptureSession::StartSession()
{
    if (m_fActive)
        return S_FALSE;

    m_stats = AVCaptureSessionStats();
    m_llStartTimeHns = 0;
    m_fActive = true;
    m_state = CaptureStateCapturing;

    if (m_pVideoCapture)
        m_pVideoCapture->StartPreview(nullptr);

    if (m_pAudioCapture)
        m_pAudioCapture->StartCapture();

    m_stats.fActive = true;
    NotifyStats();

    return S_OK;
}

HRESULT AVCaptureSession::StopSession()
{
    if (!m_fActive)
        return S_FALSE;

    if (m_pAudioCapture)
        m_pAudioCapture->StopCapture();

    if (m_pVideoCapture)
    {
        m_pVideoCapture->StopPreview();
    }

    m_fActive = false;
    m_state = CaptureStateIdle;
    m_stats.fActive = false;

    NotifyStats();
    return S_OK;
}

HRESULT AVCaptureSession::PauseSession()
{
    if (!m_fActive)
        return E_UNEXPECTED;

    if (m_pAudioCapture)
        m_pAudioCapture->PauseCapture();

    m_state = CaptureStatePaused;
    return S_OK;
}

HRESULT AVCaptureSession::ResumeSession()
{
    if (m_state != CaptureStatePaused)
        return E_UNEXPECTED;

    if (m_pAudioCapture)
        m_pAudioCapture->ResumeCapture();

    m_state = CaptureStateCapturing;
    return S_OK;
}

CaptureState AVCaptureSession::GetState() const throw()
{
    return m_state;
}

bool AVCaptureSession::IsActive() const throw()
{
    return m_fActive;
}

AVCaptureSessionStats AVCaptureSession::GetStats() const
{
    return m_stats;
}

LONGLONG AVCaptureSession::GetSessionDurationHns() const throw()
{
    return m_stats.llElapsedHns;
}

void AVCaptureSession::SetMaxDurationHns(LONGLONG llMax) throw()
{
    m_llMaxDurationHns = llMax;
}

ATL::CString AVCaptureSession::GetOutputPath() const
{
    return m_desc.strOutputPath;
}

void AVCaptureSession::UpdateStats()
{
    m_stats.llElapsedHns += 10000000; // 1 second per tick (approximation)
    CheckMaxDuration();
}

void AVCaptureSession::CheckMaxDuration()
{
    if (m_llMaxDurationHns > 0 && m_stats.llElapsedHns >= m_llMaxDurationHns)
    {
        StopSession();

        if (m_completeCb)
            m_completeCb(S_OK);
    }
}

void AVCaptureSession::NotifyStats()
{
    if (m_statsCb)
        m_statsCb(m_stats);
}

// ============================================================================
// AudioCaptureSession
// ============================================================================

AudioCaptureSession::AudioCaptureSession()
    : m_pAudioCapture(nullptr)
    , m_state(CaptureStateIdle)
    , m_fActive(false)
    , m_llStartTimeHns(0)
{
}

AudioCaptureSession::~AudioCaptureSession()
{
    Shutdown();
}

HRESULT AudioCaptureSession::Initialize(
    const AudioCaptureDeviceInfo& deviceInfo,
    const AudioCaptureConfig& config,
    LPCWSTR pszOutputPath)
{
    m_config = config;
    m_strOutputPath = pszOutputPath ? pszOutputPath : L"";

    m_pAudioCapture = new (std::nothrow) AudioCapture();
    if (!m_pAudioCapture)
        return E_OUTOFMEMORY;

    HRESULT hr = m_pAudioCapture->Initialize(deviceInfo);
    if (FAILED(hr))
        return hr;

    m_pAudioCapture->SetConfig(config);

    m_state = CaptureStateIdle;
    return S_OK;
}

HRESULT AudioCaptureSession::Shutdown()
{
    if (m_fActive)
        StopSession();

    if (m_pAudioCapture)
    {
        m_pAudioCapture->Shutdown();
        delete m_pAudioCapture;
        m_pAudioCapture = nullptr;
    }

    m_state = CaptureStateIdle;
    return S_OK;
}

HRESULT AudioCaptureSession::StartSession()
{
    if (m_fActive)
        return S_FALSE;

    if (!m_pAudioCapture)
        return E_UNEXPECTED;

    m_fActive = true;
    m_state = CaptureStateCapturing;
    m_llStartTimeHns = 0;

    return m_pAudioCapture->StartCapture();
}

HRESULT AudioCaptureSession::StopSession()
{
    if (!m_fActive)
        return S_FALSE;

    if (m_pAudioCapture)
        m_pAudioCapture->StopCapture();

    m_fActive = false;
    m_state = CaptureStateIdle;

    if (m_completeCb)
        m_completeCb(S_OK, m_strOutputPath.GetString());

    return S_OK;
}

HRESULT AudioCaptureSession::PauseSession()
{
    if (!m_fActive || m_state != CaptureStateCapturing)
        return E_UNEXPECTED;

    if (m_pAudioCapture)
        m_pAudioCapture->PauseCapture();

    m_state = CaptureStatePaused;
    return S_OK;
}

HRESULT AudioCaptureSession::ResumeSession()
{
    if (m_state != CaptureStatePaused)
        return E_UNEXPECTED;

    if (m_pAudioCapture)
        m_pAudioCapture->ResumeCapture();

    m_state = CaptureStateCapturing;
    return S_OK;
}

CaptureState AudioCaptureSession::GetState() const throw()
{
    return m_state;
}

bool AudioCaptureSession::IsActive() const throw()
{
    return m_fActive;
}

LONGLONG AudioCaptureSession::GetSessionDurationHns() const throw()
{
    return 0;
}

} // namespace HMRAVSource
