#pragma once
#ifndef SUNDANCE_TIMELINE_CONTROLLER_H
#define SUNDANCE_TIMELINE_CONTROLLER_H

#include "../pch.h"
#include "../MovieMakerCore.h"
#include <mutex>

class TimelineController
{
public:
    TimelineController();
    ~TimelineController();

    HRESULT SeekTo(LONGLONG llPositionMs);
    HRESULT SeekRelative(LONGLONG llOffsetMs);
    HRESULT StepForward(LONGLONG llStepMs = 1000);
    HRESULT StepBackward(LONGLONG llStepMs = 1000);
    HRESULT GoToStart();
    HRESULT GoToEnd();

    HRESULT SetDuration(LONGLONG llDurationMs);
    HRESULT SetZoomLevel(float flZoomLevel);
    HRESULT ZoomIn(float flZoomFactor = 1.25f);
    HRESULT ZoomOut(float flZoomFactor = 0.8f);
    HRESULT ResetZoom();
    HRESULT SetZoomRange(float flMinZoom, float flMaxZoom);

    LONGLONG GetCurrentPosition() const throw();
    LONGLONG GetDuration() const throw();
    float GetZoomLevel() const throw();
    LONGLONG GetVisibleStartMs() const throw();
    LONGLONG GetVisibleEndMs() const throw();
    LONGLONG GetVisibleRangeMs() const throw();
    float PositionToNormalized(LONGLONG llPositionMs) const throw();
    LONGLONG NormalizedToPosition(float flNormalized) const throw();

    HRESULT SetTrackCount(size_t cTracks);
    size_t  GetTrackCount() const throw();
    HRESULT SetSelectedTrack(size_t nIndex);
    size_t  GetSelectedTrack() const throw();

private:
    TimelineController(const TimelineController&);
    TimelineController& operator=(const TimelineController&);

    LONGLONG m_llCurrentPositionMs;
    LONGLONG m_llDurationMs;
    float    m_flZoomLevel;
    float    m_flMinZoom;
    float    m_flMaxZoom;
    size_t   m_cTracks;
    size_t   m_nSelectedTrack;
    mutable std::mutex m_mutex;
};

#endif
