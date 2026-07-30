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

    LONGLONG GetCurrentPosition() const;
    LONGLONG GetDuration() const;
    float GetZoomLevel() const;
    LONGLONG GetVisibleStartMs() const;
    LONGLONG GetVisibleEndMs() const;
    LONGLONG GetVisibleRangeMs() const;
    float PositionToNormalized(LONGLONG llPositionMs) const;
    LONGLONG NormalizedToPosition(float flNormalized) const;

    HRESULT SetTrackCount(size_t cTracks);
    size_t  GetTrackCount() const;
    HRESULT SetSelectedTrack(size_t nIndex);
    size_t  GetSelectedTrack() const;

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
    mutable std::recursive_mutex m_mutex;
};

#endif
