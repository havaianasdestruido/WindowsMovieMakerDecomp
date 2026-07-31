#pragma once
#include <windows.h>
#include "TimelineEngine.h"
#include <string>
#include <vector>

namespace DirectUI {

class PlaybackEngine;

} // namespace DirectUI

class MediaCatalog;

namespace DirectUI {

class __declspec(dllexport) TimelineDispatcher {
public:
    TimelineDispatcher(TimelineEngine* pTimeline, PlaybackEngine* pPlayback, ::MediaCatalog* pCatalog);
    ~TimelineDispatcher();

    HRESULT SetCursorPosition(double seconds);
    HRESULT Tick(double deltaTime);
    double GetCursorPosition() const { return m_cursor; }

private:
    const Clip* FindClipAt(double seconds) const;

    TimelineEngine* m_pTimeline = nullptr;
    PlaybackEngine* m_pPlayback = nullptr;
    ::MediaCatalog* m_pCatalog = nullptr;
    double m_cursor = 0.0;
    std::wstring m_activeMediaId;
};

} // namespace DirectUI
