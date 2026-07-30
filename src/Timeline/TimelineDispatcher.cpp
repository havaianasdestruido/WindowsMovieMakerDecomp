#include "TimelineDispatcher.h"
#include "PlaybackEngine.h"
#include "MediaCatalog.h"

namespace DirectUI {

TimelineDispatcher::TimelineDispatcher(
    TimelineEngine* pTimeline,
    PlaybackEngine* pPlayback,
    MediaCatalog* pCatalog)
    : m_pTimeline(pTimeline)
    , m_pPlayback(pPlayback)
    , m_pCatalog(pCatalog)
{
}

TimelineDispatcher::~TimelineDispatcher() {
}

const Clip* TimelineDispatcher::FindClipAt(double seconds) const {
    if (!m_pTimeline) return nullptr;
    for (const auto& clip : m_pTimeline->GetClips()) {
        if (seconds >= clip.start && seconds < clip.start + clip.duration) {
            return &clip;
        }
    }
    return nullptr;
}

HRESULT TimelineDispatcher::SetCursorPosition(double seconds) {
    m_cursor = seconds;

    const Clip* pClip = FindClipAt(seconds);
    if (!pClip) {
        if (m_pPlayback && m_pPlayback->HasMedia()) {
            m_pPlayback->Pause();
        }
        m_activeMediaId.clear();
        return S_OK;
    }

    if (pClip->mediaId != m_activeMediaId) {
        if (m_pCatalog && m_pPlayback) {
            const MediaItem* pItem = m_pCatalog->GetItem(pClip->mediaId);
            if (pItem) {
                m_pPlayback->OpenFile(pItem->path);
            }
        }
        m_activeMediaId = pClip->mediaId;
    }

    double clipLocalTime = seconds - pClip->start;
    if (m_pPlayback) {
        m_pPlayback->Seek(clipLocalTime);
    }

    return S_OK;
}

HRESULT TimelineDispatcher::Tick(double deltaTime) {
    m_cursor += deltaTime;

    const Clip* pClip = FindClipAt(m_cursor);
    if (!pClip) {
        if (m_pPlayback && m_pPlayback->IsPlaying()) {
            m_pPlayback->Pause();
        }
        return S_FALSE;
    }

    if (pClip->mediaId != m_activeMediaId) {
        if (m_pCatalog && m_pPlayback) {
            const MediaItem* pItem = m_pCatalog->GetItem(pClip->mediaId);
            if (pItem) {
                m_pPlayback->OpenFile(pItem->path);
            }
        }
        m_activeMediaId = pClip->mediaId;
    }

    if (m_pPlayback) {
        m_pPlayback->UpdateFrame();
    }

    return S_OK;
}

} // namespace DirectUI
