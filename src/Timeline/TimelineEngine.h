#pragma once
#include <vector>
#include <cmath>
#include "Clip.h"

class TimelineEngine {
public:
    TimelineEngine() = default;
    ~TimelineEngine() = default;
    bool AddClip(const Clip& clip);
    bool RemoveClip(const std::wstring& mediaId, double start);
    bool MoveClip(const std::wstring& mediaId, double oldStart, double newStart, int newTrack);
    bool TrimClip(const std::wstring& mediaId, double start, double newDuration);
    const std::vector<Clip>& GetClips() const { return m_clips; }
private:
    std::vector<Clip> m_clips;
    double Snap(double time) const { const double step = 0.5; return std::round(time/step)*step; }
    bool Overlaps(const Clip& a, const Clip& b) const;
};
