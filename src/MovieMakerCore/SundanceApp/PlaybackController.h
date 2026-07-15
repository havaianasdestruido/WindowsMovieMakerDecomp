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

private:
    PlaybackController(const PlaybackController&);
    PlaybackController& operator=(const PlaybackController&);
};

#endif
