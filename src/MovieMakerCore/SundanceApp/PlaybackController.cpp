#include "pch.h"
#include "PlaybackController.h"

PlaybackController::PlaybackController()
    : m_state(State_Stopped)
    , m_llPositionMs(0)
    , m_flVolume(1.0f)
{
}

PlaybackController::~PlaybackController() {}

HRESULT PlaybackController::Play()
{
    if (m_state == State_Playing)
        return S_FALSE;

    m_state = State_Playing;
    return S_OK;
}

HRESULT PlaybackController::Stop()
{
    if (m_state == State_Stopped)
        return S_FALSE;

    m_state = State_Stopped;
    m_llPositionMs = 0;
    return S_OK;
}

HRESULT PlaybackController::Pause()
{
    if (m_state != State_Playing)
        return S_FALSE;

    m_state = State_Paused;
    return S_OK;
}

bool PlaybackController::IsPlaying() const throw()
{
    return m_state == State_Playing;
}

bool PlaybackController::IsPaused() const throw()
{
    return m_state == State_Paused;
}

HRESULT PlaybackController::SeekTo(LONGLONG llPositionMs)
{
    if (llPositionMs < 0)
        return E_INVALIDARG;

    m_llPositionMs = llPositionMs;
    return S_OK;
}

HRESULT PlaybackController::SetVolume(float flVolume)
{
    if (flVolume < 0.0f || flVolume > 1.0f)
        return E_INVALIDARG;

    m_flVolume = flVolume;
    return S_OK;
}

float PlaybackController::GetVolume() const throw()
{
    return m_flVolume;
}

LONGLONG PlaybackController::GetCurrentPosition() const throw()
{
    return m_llPositionMs;
}
