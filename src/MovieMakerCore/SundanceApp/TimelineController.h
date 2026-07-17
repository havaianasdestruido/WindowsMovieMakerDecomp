#pragma once
#ifndef SUNDANCE_TIMELINE_CONTROLLER_H
#define SUNDANCE_TIMELINE_CONTROLLER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class TimelineController
{
public:
    TimelineController();
    ~TimelineController();

    HRESULT SeekTo(LONGLONG llPositionMs);
    HRESULT SetDuration(LONGLONG llDurationMs);
    HRESULT SetZoomLevel(float flZoomLevel);
    LONGLONG GetCurrentPosition() const throw();
    LONGLONG GetDuration() const throw();
    float GetZoomLevel() const throw();

private:
    TimelineController(const TimelineController&);
    TimelineController& operator=(const TimelineController&);

    LONGLONG m_llCurrentPositionMs;
    LONGLONG m_llDurationMs;
    float    m_flZoomLevel;
};

#endif
