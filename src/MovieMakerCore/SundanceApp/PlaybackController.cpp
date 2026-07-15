#include "pch.h"
#include "PlaybackController.h"

PlaybackController::PlaybackController() {}
PlaybackController::~PlaybackController() {}
HRESULT PlaybackController::Play() { return E_NOTIMPL; }
HRESULT PlaybackController::Stop() { return E_NOTIMPL; }
HRESULT PlaybackController::Pause() { return E_NOTIMPL; }
bool PlaybackController::IsPlaying() const throw() { return false; }
