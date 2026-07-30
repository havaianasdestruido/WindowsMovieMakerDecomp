#include "TimelineEngine.h"

bool TimelineEngine::AddClip(const Clip& clip) {
    Clip snapped = clip;
    snapped.start = Snap(snapped.start);
    // check overlap on same track
    for (const auto& existing : m_clips) {
        if (existing.track != snapped.track) continue;
        if (Overlaps(existing, snapped)) return false;
    }
    m_clips.push_back(snapped);
    return true;
}

bool TimelineEngine::RemoveClip(const std::wstring& mediaId, double start) {
    double snappedStart = Snap(start);
    for (auto it = m_clips.begin(); it != m_clips.end(); ++it) {
        if (it->mediaId == mediaId && it->start == snappedStart) {
            m_clips.erase(it);
            return true;
        }
    }
    return false;
}

bool TimelineEngine::MoveClip(const std::wstring& mediaId, double oldStart, double newStart, int newTrack) {
    double oldSnapped = Snap(oldStart);
    double newSnapped = Snap(newStart);
    for (auto& clip : m_clips) {
        if (clip.mediaId == mediaId && clip.start == oldSnapped) {
            Clip test = clip;
            test.start = newSnapped;
            test.track = newTrack;
            // check overlap with others
            for (const auto& other : m_clips) {
                if (&other == &clip) continue;
                if (other.track != test.track) continue;
                if (Overlaps(other, test)) return false;
            }
            clip.start = newSnapped;
            clip.track = newTrack;
            return true;
        }
    }
    return false;
}

bool TimelineEngine::TrimClip(const std::wstring& mediaId, double start, double newDuration) {
    double snappedStart = Snap(start);
    if (newDuration <= 0) return false;
    for (auto& clip : m_clips) {
        if (clip.mediaId == mediaId && clip.start == snappedStart) {
            Clip test = clip;
            test.duration = newDuration;
            // check overlap on same track
            for (const auto& other : m_clips) {
                if (&other == &clip) continue;
                if (other.track != test.track) continue;
                if (Overlaps(other, test)) return false;
            }
            clip.duration = newDuration;
            return true;
        }
    }
    return false;
}

bool TimelineEngine::Overlaps(const Clip& a, const Clip& b) const {
    double aEnd = a.start + a.duration;
    double bEnd = b.start + b.duration;
    return !(aEnd <= b.start || bEnd <= a.start);
}
