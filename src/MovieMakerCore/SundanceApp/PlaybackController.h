#pragma once
#ifndef SUNDANCE_PLAYBACK_CONTROLLER_H
#define SUNDANCE_PLAYBACK_CONTROLLER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class PlaybackController
{
public:
    PlaybackController();
    ~PlaybackController();

    HRESULT Play();
    HRESULT Stop();
    HRESULT Pause();
    bool IsPlaying() const throw();
    bool IsPaused() const throw();

    HRESULT SeekTo(LONGLONG llPositionMs);
    HRESULT SetVolume(float flVolume);
    float GetVolume() const throw();
    LONGLONG GetCurrentPosition() const throw();

private:
    PlaybackController(const PlaybackController&);
    PlaybackController& operator=(const PlaybackController&);

    enum PlaybackState
    {
        State_Stopped = 0,
        State_Playing,
        State_Paused
    };

    PlaybackState m_state;
    LONGLONG      m_llPositionMs;
    float         m_flVolume;
};

#endif
