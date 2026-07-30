#pragma once
#include <string>

struct Clip {
    std::wstring mediaId;   // reference to MediaCatalog entry
    double start;           // start time on timeline (seconds)
    double duration;        // length (seconds)
    int track;               // track index (0 = video, >0 = audio etc.)
};
