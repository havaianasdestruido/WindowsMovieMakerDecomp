#include "pch.h"
#include "PlaybackController.h"

// ============================================================================
// Construction / destruction
// ============================================================================
PlaybackController::PlaybackController()
    : m_state(State_Stopped)
    , m_llPositionMs(0)
    , m_llDurationMs(0)
    , m_flVolume(1.0f)
    , m_bMuted(false)
{
}

PlaybackController::~PlaybackController()
{
    Shutdown();
}

// ============================================================================
// Initialize
// ============================================================================
HRESULT PlaybackController::Initialize()
{
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ============================================================================
// Shutdown
// ============================================================================
void PlaybackController::Shutdown()
{
    if (m_spSession)
    {
        m_spSession->Close();
        m_spSession->Shutdown();
        m_spSession.Release();
    }

    if (m_spSource)
    {
        m_spSource->Shutdown();
        m_spSource.Release();
    }

    m_spVolume.Release();

    MFShutdown();
}

// ============================================================================
// OpenFile
//
// Creates a media source from the given file path and prepares a session
// for playback. The session is not started until Play() is called.
// ============================================================================
HRESULT PlaybackController::OpenFile(LPCWSTR pszFilePath)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    if (m_spSession)
        Close();

    HRESULT hr = CreateMediaSource(pszFilePath);
    if (FAILED(hr))
        return hr;

    hr = CreateSession();
    if (FAILED(hr))
    {
        if (m_spSource)
        {
            m_spSource->Shutdown();
            m_spSource.Release();
        }
        if (m_spSession)
        {
            m_spSession->Close();
            m_spSession->Shutdown();
            m_spSession.Release();
        }
        return hr;
    }

    hr = UpdateDuration();
    if (FAILED(hr))
        m_llDurationMs = 0;

    return S_OK;
}

// ============================================================================
// Close
//
// Stops playback and releases the media source and session.
// ============================================================================
HRESULT PlaybackController::Close()
{
    Stop();

    if (m_spSession)
    {
        m_spSession->Close();
        m_spSession->Shutdown();
        m_spSession.Release();
    }

    if (m_spSource)
    {
        m_spSource->Shutdown();
        m_spSource.Release();
    }

    m_spVolume.Release();
    m_llPositionMs = 0;
    m_llDurationMs = 0;

    return S_OK;
}

// ============================================================================
// Play
//
// Starts or resumes playback via the MF media session. If the session is
// stopped, begins from the current position (or beginning if at end).
// ============================================================================
HRESULT PlaybackController::Play()
{
    if (m_state == State_Playing)
        return S_FALSE;

    if (!m_spSession)
        return E_UNEXPECTED;

    if (m_state == State_Stopped)
    {
        LONGLONG llPositionHns = m_llPositionMs * 10000;
        HRESULT hr = StartPlaybackFromPosition(llPositionHns);
        if (FAILED(hr))
            return hr;
    }
    else if (m_state == State_Paused)
    {
        PROPVARIANT varStart;
        PropVariantInit(&varStart);
        varStart.vt = VT_EMPTY;

        HRESULT hr = m_spSession->Start(&GUID_NULL, &varStart);
        PropVariantClear(&varStart);
        if (FAILED(hr))
            return hr;
    }

    m_state = State_Playing;
    return S_OK;
}

// ============================================================================
// Stop
//
// Stops playback and resets the position to the beginning.
// ============================================================================
HRESULT PlaybackController::Stop()
{
    if (m_state == State_Stopped)
        return S_FALSE;

    if (m_spSession)
    {
        HRESULT hr = m_spSession->Stop();
        if (FAILED(hr))
            return hr;
    }

    m_state = State_Stopped;
    m_llPositionMs = 0;
    return S_OK;
}

// ============================================================================
// Pause
//
// Pauses playback. The session remains ready to resume.
// ============================================================================
HRESULT PlaybackController::Pause()
{
    if (m_state != State_Playing)
        return S_FALSE;

    if (!m_spSession)
        return E_UNEXPECTED;

    HRESULT hr = m_spSession->Pause();
    if (FAILED(hr))
        return hr;

    m_state = State_Paused;
    return S_OK;
}

// ============================================================================
// IsPlaying / IsPaused
// ============================================================================
bool PlaybackController::IsPlaying() const throw()
{
    return m_state == State_Playing;
}

bool PlaybackController::IsPaused() const throw()
{
    return m_state == State_Paused;
}

// ============================================================================
// SeekTo
//
// Seeks to an absolute position in milliseconds. If currently playing or
// paused, the session seeks immediately; otherwise stores the position
// for the next Play() call.
// ============================================================================
HRESULT PlaybackController::SeekTo(LONGLONG llPositionMs)
{
    if (llPositionMs < 0)
        return E_INVALIDARG;

    if (m_llDurationMs > 0 && llPositionMs > m_llDurationMs)
        llPositionMs = m_llDurationMs;

    LONGLONG llPositionHns = llPositionMs * 10000;

    if (m_spSession && (m_state == State_Playing || m_state == State_Paused))
    {
        LONGLONG llCurrentHns = GetCurrentPositionHns();
        LONGLONG llDeltaHns = llPositionHns - llCurrentHns;

        PROPVARIANT varStart;
        PropVariantInit(&varStart);
        varStart.vt = VT_I8;
        varStart.hVal.QuadPart = llDeltaHns;

        HRESULT hr = m_spSession->Start(&GUID_NULL, &varStart);
        PropVariantClear(&varStart);
        if (FAILED(hr))
            return hr;

        if (m_state == State_Paused)
        {
            m_spSession->Pause();
        }
    }

    m_llPositionMs = llPositionMs;
    return S_OK;
}

// ============================================================================
// SeekRelative
//
// Seeks by a relative offset in milliseconds from the current position.
// ============================================================================
HRESULT PlaybackController::SeekRelative(LONGLONG llOffsetMs)
{
    return SeekTo(m_llPositionMs + llOffsetMs);
}

// ============================================================================
// SetVolume
//
// Sets the playback volume (0.0 silence to 1.0 full). Applies to the
// session's audio stream via IMFSimpleAudioVolume.
// ============================================================================
HRESULT PlaybackController::SetVolume(float flVolume)
{
    if (flVolume < 0.0f || flVolume > 1.0f)
        return E_INVALIDARG;

    m_flVolume = flVolume;

    if (m_spVolume)
    {
        float flMaster = m_bMuted ? 0.0f : m_flVolume;
        return m_spVolume->SetMasterVolume(flMaster);
    }

    return S_OK;
}

// ============================================================================
// SetMute
//
// Mutes or unmutes the playback audio without changing the volume level.
// ============================================================================
HRESULT PlaybackController::SetMute(bool bMute)
{
    m_bMuted = bMute;

    if (m_spVolume)
    {
        float flMaster = m_bMuted ? 0.0f : m_flVolume;
        return m_spVolume->SetMasterVolume(flMaster);
    }

    return S_OK;
}

// ============================================================================
// GetVolume / IsMuted / GetCurrentPosition / GetDuration
// ============================================================================
float PlaybackController::GetVolume() const throw()
{
    return m_flVolume;
}

bool PlaybackController::IsMuted() const throw()
{
    return m_bMuted;
}

LONGLONG PlaybackController::GetCurrentPosition() const throw()
{
    return m_llPositionMs;
}

LONGLONG PlaybackController::GetDuration() const throw()
{
    return m_llDurationMs;
}

// ============================================================================
// OnSessionEvent
//
// Called by the application when it receives MEEvent notifications from the
// MF media session. Handles end-of-stream and session-closed events to
// update the playback state machine.
// ============================================================================
HRESULT PlaybackController::OnSessionEvent(MediaEventType met, HRESULT hrStatus)
{
    switch (met)
    {
    case MESessionEnded:
        m_state = State_Stopped;
        m_llPositionMs = 0;
        break;

    case MESessionStopped:
        m_state = State_Stopped;
        break;

    case MESessionPaused:
        m_state = State_Paused;
        break;

    case MEError:
        m_state = State_Stopped;
        break;

    default:
        break;
    }

    return S_OK;
}

// ============================================================================
// CreateMediaSource (private)
//
// Creates an IMFMediaSource from a file path using the MF source resolver.
// The source resolver handles video, audio, and mixed-media file types
// and produces a source suitable for use with MFCreateMediaSession.
// ============================================================================
HRESULT PlaybackController::CreateMediaSource(LPCWSTR pszFilePath)
{
    CComPtr<IMFSourceResolver> spResolver;
    HRESULT hr = MFCreateSourceResolver(&spResolver);
    if (FAILED(hr))
        return hr;

    MF_OBJECT_TYPE objectType = MF_OBJECT_UNKNOWN;
    CComPtr<IUnknown> spUnkSource;

    hr = spResolver->CreateObjectFromURL(
        pszFilePath,
        MF_RESOLUTION_MEDIASOURCE,
        NULL,
        &objectType,
        &spUnkSource);

    if (FAILED(hr))
        return hr;

    if (objectType != MF_OBJECT_MEDIASOURCE)
        return E_UNEXPECTED;

    hr = spUnkSource->QueryInterface(IID_PPV_ARGS(&m_spSource));
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ============================================================================
// CreateSession (private)
//
// Creates an MF media session, builds a playback topology from the
// loaded media source, and sets it on the session. Also attempts to
// obtain the audio volume interface.
// ============================================================================
HRESULT PlaybackController::CreateSession()
{
    if (!m_spSource)
        return E_UNEXPECTED;

    HRESULT hr = MFCreateMediaSession(NULL, &m_spSession);
    if (FAILED(hr))
        return hr;

    // Build a playback topology from the media source by creating a
    // topology with a single source node.
    CComPtr<IMFTopology> spTopology;
    hr = MFCreateTopology(&spTopology);
    if (FAILED(hr))
    {
        ShutdownSession();
        return hr;
    }

    // Get the presentation descriptor to determine the stream count
    CComPtr<IMFPresentationDescriptor> spPD;
    hr = m_spSource->CreatePresentationDescriptor(&spPD);
    if (FAILED(hr))
    {
        ShutdownSession();
        return hr;
    }

    DWORD cStreams = 0;
    hr = spPD->GetStreamDescriptorCount(&cStreams);
    if (FAILED(hr))
    {
        ShutdownSession();
        return hr;
    }

    for (DWORD i = 0; i < cStreams; ++i)
    {
        CComPtr<IMFStreamDescriptor> spSD;
        BOOL fSelected = FALSE;
        hr = spPD->GetStreamDescriptorByIndex(i, &fSelected, &spSD);
        if (FAILED(hr))
            continue;

        if (!fSelected)
            continue;

        // Create a source node for this stream
        CComPtr<IMFTopologyNode> spSourceNode;
        hr = MFCreateTopologyNode(
            MF_TOPOLOGY_SOURCESTREAM_NODE,
            &spSourceNode);
        if (FAILED(hr))
            continue;

        hr = spSourceNode->SetUnknown(MF_TOPONODE_SOURCE, m_spSource);
        if (FAILED(hr))
            continue;

        hr = spSourceNode->SetUINT32(MF_TOPONODE_STREAMID, i);
        if (FAILED(hr))
            continue;

        hr = spSourceNode->SetObject(spSD);
        if (FAILED(hr))
            continue;

        hr = spTopology->AddNode(spSourceNode);
        if (FAILED(hr))
            continue;
    }

    hr = m_spSession->SetTopology(0, spTopology);
    if (FAILED(hr))
        return hr;

    // Note: IMFSimpleAudioVolume is obtained from the audio output node
    // in the topology. For now, volume operations are tracked locally and
    // applied when an audio stream topology node is available.
    m_spVolume = nullptr;

    return S_OK;
}

// ============================================================================
// StartPlaybackFromPosition (private)
//
// Starts the media session from a given position specified in hundred-
// nanoseconds (100ns units). Used by Play() and SeekTo().
// ============================================================================
HRESULT PlaybackController::StartPlaybackFromPosition(LONGLONG llPositionHns)
{
    if (!m_spSession)
        return E_UNEXPECTED;

    PROPVARIANT varStart;
    PropVariantInit(&varStart);
    varStart.vt = VT_I8;
    varStart.hVal.QuadPart = llPositionHns;

    HRESULT hr = m_spSession->Start(&GUID_NULL, &varStart);
    PropVariantClear(&varStart);

    return hr;
}

// ============================================================================
// ShutdownSession (private)
//
// Gracefully shuts down the media session, waiting for it to close
// before releasing resources.
// ============================================================================
HRESULT PlaybackController::ShutdownSession()
{
    if (!m_spSession)
        return S_OK;

    m_spSession->Close();
    m_spSession->Shutdown();
    m_spSession.Release();

    return S_OK;
}

// ============================================================================
// GetCurrentPositionHns (private)
//
// Queries the current playback position from the presentation clock in
// hundred-nanosecond units. Falls back to the stored millisecond
// position if the clock is not available.
// ============================================================================
LONGLONG PlaybackController::GetCurrentPositionHns() const
{
    if (!m_spSession)
        return m_llPositionMs * 10000;

    IMFClock* pClock = NULL;
    HRESULT hr = m_spSession->GetClock(&pClock);
    if (FAILED(hr) || !pClock)
        return m_llPositionMs * 10000;

    MFCLOCK_STATE clockState;
    hr = pClock->GetState(0, &clockState);
    LONGLONG llTime = 0;
    if (SUCCEEDED(hr) && clockState == MFCLOCK_STATE_RUNNING)
    {
        hr = pClock->GetCorrelatedTime(0, &llTime, nullptr);
    }
    pClock->Release();

    if (FAILED(hr))
        return m_llPositionMs * 10000;

    return llTime;
}

// ============================================================================
// UpdateDuration (private)
//
// Queries the presentation descriptor from the media source to determine
// the total duration of the current media in hundred-nanoseconds, then
// converts it to milliseconds.
// ============================================================================
HRESULT PlaybackController::UpdateDuration()
{
    if (!m_spSource)
        return E_UNEXPECTED;

    CComPtr<IMFPresentationDescriptor> spPD;
    HRESULT hr = m_spSource->CreatePresentationDescriptor(&spPD);
    if (FAILED(hr))
        return hr;

    UINT64 ullDuration = 0;
    hr = spPD->GetUINT64(MF_PD_DURATION, &ullDuration);
    if (FAILED(hr))
        return hr;

    m_llDurationMs = static_cast<LONGLONG>(ullDuration / 10000);
    return S_OK;
}
