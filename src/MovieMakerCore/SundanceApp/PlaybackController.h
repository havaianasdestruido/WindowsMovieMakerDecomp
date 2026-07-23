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

    HRESULT Initialize();
    void    Shutdown();

    HRESULT OpenFile(LPCWSTR pszFilePath);
    HRESULT Close();

    HRESULT Play();
    HRESULT Stop();
    HRESULT Pause();
    bool IsPlaying() const throw();
    bool IsPaused() const throw();

    HRESULT SeekTo(LONGLONG llPositionMs);
    HRESULT SeekRelative(LONGLONG llOffsetMs);
    HRESULT SetVolume(float flVolume);
    HRESULT SetMute(bool bMute);
    float GetVolume() const throw();
    bool IsMuted() const throw();
    LONGLONG GetCurrentPosition() const throw();
    LONGLONG GetDuration() const throw();

    HRESULT OnSessionEvent(MediaEventType met, HRESULT hrStatus);

private:
    PlaybackController(const PlaybackController&);
    PlaybackController& operator=(const PlaybackController&);

    HRESULT CreateMediaSource(LPCWSTR pszFilePath);
    HRESULT CreateSession();
    HRESULT StartPlaybackFromPosition(LONGLONG llPositionHns);
    HRESULT ShutdownSession();
    LONGLONG GetCurrentPositionHns() const;
    HRESULT UpdateDuration();

    enum PlaybackState
    {
        State_Stopped = 0,
        State_Playing,
        State_Paused
    };

    PlaybackState m_state;
    LONGLONG      m_llPositionMs;
    LONGLONG      m_llDurationMs;
    float         m_flVolume;
    bool          m_bMuted;

    CComPtr<IMFMediaSession>    m_spSession;
    CComPtr<IMFMediaSource>     m_spSource;
    CComPtr<IMFSimpleAudioVolume> m_spVolume;
};

#endif
