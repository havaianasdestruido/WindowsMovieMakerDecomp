#include "pch.h"
#include "TimelineController.h"

// ============================================================================
// Construction / destruction
// ============================================================================
TimelineController::TimelineController()
    : m_llCurrentPositionMs(0)
    , m_llDurationMs(0)
    , m_flZoomLevel(1.0f)
    , m_flMinZoom(0.1f)
    , m_flMaxZoom(100.0f)
    , m_cTracks(1)
    , m_nSelectedTrack(0)
{
}

TimelineController::~TimelineController() {}

// ============================================================================
// SeekTo
//
// Seeks to an absolute position on the timeline, clamping to the
// valid range [0, duration].
// ============================================================================
HRESULT TimelineController::SeekTo(LONGLONG llPositionMs)
{
    if (llPositionMs < 0)
        return E_INVALIDARG;

    m_llCurrentPositionMs = llPositionMs;

    if (m_llDurationMs > 0 && m_llCurrentPositionMs > m_llDurationMs)
        m_llCurrentPositionMs = m_llDurationMs;

    return S_OK;
}

// ============================================================================
// SeekRelative
//
// Seeks by a relative offset from the current position.
// ============================================================================
HRESULT TimelineController::SeekRelative(LONGLONG llOffsetMs)
{
    return SeekTo(m_llCurrentPositionMs + llOffsetMs);
}

// ============================================================================
// StepForward / StepBackward
//
// Moves the cursor by a fixed step (default 1 second) for frame-by-frame
// or chapter navigation.
// ============================================================================
HRESULT TimelineController::StepForward(LONGLONG llStepMs)
{
    if (llStepMs <= 0)
        return E_INVALIDARG;

    return SeekTo(m_llCurrentPositionMs + llStepMs);
}

HRESULT TimelineController::StepBackward(LONGLONG llStepMs)
{
    if (llStepMs <= 0)
        return E_INVALIDARG;

    return SeekTo(m_llCurrentPositionMs - llStepMs);
}

// ============================================================================
// GoToStart / GoToEnd
//
// Jumps the cursor to the beginning or end of the timeline.
// ============================================================================
HRESULT TimelineController::GoToStart()
{
    m_llCurrentPositionMs = 0;
    return S_OK;
}

HRESULT TimelineController::GoToEnd()
{
    m_llCurrentPositionMs = m_llDurationMs;
    return S_OK;
}

// ============================================================================
// SetDuration
//
// Sets the total timeline duration. If the current position exceeds the
// new duration, it is clamped.
// ============================================================================
HRESULT TimelineController::SetDuration(LONGLONG llDurationMs)
{
    if (llDurationMs < 0)
        return E_INVALIDARG;

    m_llDurationMs = llDurationMs;

    if (m_llCurrentPositionMs > m_llDurationMs)
        m_llCurrentPositionMs = m_llDurationMs;

    return S_OK;
}

// ============================================================================
// SetZoomLevel
//
// Sets the timeline zoom level to an absolute value, clamping to the
// configured min/max range.
// ============================================================================
HRESULT TimelineController::SetZoomLevel(float flZoomLevel)
{
    if (flZoomLevel <= 0.0f)
        return E_INVALIDARG;

    if (flZoomLevel < m_flMinZoom)
        flZoomLevel = m_flMinZoom;
    if (flZoomLevel > m_flMaxZoom)
        flZoomLevel = m_flMaxZoom;

    m_flZoomLevel = flZoomLevel;
    return S_OK;
}

// ============================================================================
// ZoomIn / ZoomOut
//
// Adjusts the zoom level by a multiplicative factor. ZoomIn increases
// magnification (shows less time, more detail). ZoomOut decreases it.
// ============================================================================
HRESULT TimelineController::ZoomIn(float flZoomFactor)
{
    if (flZoomFactor <= 1.0f)
        return E_INVALIDARG;

    return SetZoomLevel(m_flZoomLevel * flZoomFactor);
}

HRESULT TimelineController::ZoomOut(float flZoomFactor)
{
    if (flZoomFactor <= 0.0f || flZoomFactor >= 1.0f)
        return E_INVALIDARG;

    return SetZoomLevel(m_flZoomLevel * flZoomFactor);
}

// ============================================================================
// ResetZoom
//
// Resets the zoom level to 1.0x (100%).
// ============================================================================
HRESULT TimelineController::ResetZoom()
{
    m_flZoomLevel = 1.0f;
    return S_OK;
}

// ============================================================================
// SetZoomRange
//
// Configures the minimum and maximum zoom levels. Clamps current zoom
// to the new range if necessary.
// ============================================================================
HRESULT TimelineController::SetZoomRange(float flMinZoom, float flMaxZoom)
{
    if (flMinZoom <= 0.0f || flMaxZoom <= 0.0f)
        return E_INVALIDARG;
    if (flMinZoom > flMaxZoom)
        return E_INVALIDARG;

    m_flMinZoom = flMinZoom;
    m_flMaxZoom = flMaxZoom;

    if (m_flZoomLevel < m_flMinZoom)
        m_flZoomLevel = m_flMinZoom;
    if (m_flZoomLevel > m_flMaxZoom)
        m_flZoomLevel = m_flMaxZoom;

    return S_OK;
}

// ============================================================================
// GetCurrentPosition / GetDuration / GetZoomLevel
// ============================================================================
LONGLONG TimelineController::GetCurrentPosition() const throw()
{
    return m_llCurrentPositionMs;
}

LONGLONG TimelineController::GetDuration() const throw()
{
    return m_llDurationMs;
}

float TimelineController::GetZoomLevel() const throw()
{
    return m_flZoomLevel;
}

// ============================================================================
// GetVisibleStartMs / GetVisibleEndMs / GetVisibleRangeMs
//
// Returns the time range visible at the current zoom level and scroll
// position. At 1.0x zoom the full duration is visible; higher zoom
// levels show a narrower window centered on the cursor.
// ============================================================================
LONGLONG TimelineController::GetVisibleStartMs() const throw()
{
    if (m_flZoomLevel <= 1.0f || m_llDurationMs <= 0)
        return 0;

    LONGLONG llVisibleRange = static_cast<LONGLONG>(m_llDurationMs / m_flZoomLevel);
    LONGLONG llHalfVisible = llVisibleRange / 2;
    LONGLONG llStart = m_llCurrentPositionMs - llHalfVisible;

    if (llStart < 0)
        llStart = 0;

    return llStart;
}

LONGLONG TimelineController::GetVisibleEndMs() const throw()
{
    if (m_flZoomLevel <= 1.0f || m_llDurationMs <= 0)
        return m_llDurationMs;

    LONGLONG llVisibleRange = static_cast<LONGLONG>(m_llDurationMs / m_flZoomLevel);
    LONGLONG llEnd = GetVisibleStartMs() + llVisibleRange;

    if (llEnd > m_llDurationMs)
        llEnd = m_llDurationMs;

    return llEnd;
}

LONGLONG TimelineController::GetVisibleRangeMs() const throw()
{
    return GetVisibleEndMs() - GetVisibleStartMs();
}

// ============================================================================
// PositionToNormalized / NormalizedToPosition
//
// Converts between timeline positions and a normalized [0,1] value
// used for UI scroll-bar and scrubber positioning.
// ============================================================================
float TimelineController::PositionToNormalized(LONGLONG llPositionMs) const throw()
{
    if (m_llDurationMs <= 0)
        return 0.0f;

    float flResult = static_cast<float>(llPositionMs) / static_cast<float>(m_llDurationMs);

    if (flResult < 0.0f) flResult = 0.0f;
    if (flResult > 1.0f) flResult = 1.0f;

    return flResult;
}

LONGLONG TimelineController::NormalizedToPosition(float flNormalized) const throw()
{
    if (flNormalized < 0.0f) flNormalized = 0.0f;
    if (flNormalized > 1.0f) flNormalized = 1.0f;

    return static_cast<LONGLONG>(flNormalized * static_cast<float>(m_llDurationMs));
}

// ============================================================================
// SetTrackCount / GetTrackCount / SetSelectedTrack / GetSelectedTrack
//
// Manages the logical track count and the currently selected track index.
// ============================================================================
HRESULT TimelineController::SetTrackCount(size_t cTracks)
{
    if (cTracks == 0)
        return E_INVALIDARG;

    m_cTracks = cTracks;

    if (m_nSelectedTrack >= m_cTracks)
        m_nSelectedTrack = m_cTracks - 1;

    return S_OK;
}

size_t TimelineController::GetTrackCount() const throw()
{
    return m_cTracks;
}

HRESULT TimelineController::SetSelectedTrack(size_t nIndex)
{
    if (nIndex >= m_cTracks)
        return E_INVALIDARG;

    m_nSelectedTrack = nIndex;
    return S_OK;
}

size_t TimelineController::GetSelectedTrack() const throw()
{
    return m_nSelectedTrack;
}
