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

private:
    TimelineController(const TimelineController&);
    TimelineController& operator=(const TimelineController&);
};

#endif
