#include "pch.h"
#include "TimelineController.h"

TimelineController::TimelineController()
    : m_llCurrentPositionMs(0)
    , m_llDurationMs(0)
    , m_flZoomLevel(1.0f)
{
}

TimelineController::~TimelineController() {}

HRESULT TimelineController::SeekTo(LONGLONG llPositionMs)
{
    if (llPositionMs < 0)
        return E_INVALIDARG;

    m_llCurrentPositionMs = llPositionMs;
    return S_OK;
}

HRESULT TimelineController::SetDuration(LONGLONG llDurationMs)
{
    if (llDurationMs < 0)
        return E_INVALIDARG;

    m_llDurationMs = llDurationMs;
    if (m_llCurrentPositionMs > m_llDurationMs)
        m_llCurrentPositionMs = m_llDurationMs;

    return S_OK;
}

HRESULT TimelineController::SetZoomLevel(float flZoomLevel)
{
    if (flZoomLevel <= 0.0f)
        return E_INVALIDARG;

    m_flZoomLevel = flZoomLevel;
    return S_OK;
}

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
