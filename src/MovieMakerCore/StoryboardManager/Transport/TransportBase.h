/*
 * TransportBase.h
 *
 * Transport classes for timeline playback control and render/export.
 *
 * TransportBase: Base transport for timeline playback control.
 * MovieTransport: Movie playback transport (play, pause, seek, rate).
 * RenderTransport: Render/export transport.
 *
 * RTTI: ?AVTransportBase@@, ?AVMovieTransport@@, ?AVRenderTransport@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef TRANSPORTBASE_H
#define TRANSPORTBASE_H

#include "../../pch.h"

// ============================================================================
// TransportState enum
// ============================================================================
enum TransportState
{
    TransportStateStopped    = 0,
    TransportStatePlaying    = 1,
    TransportStatePaused     = 2,
    TransportStateSeeking    = 3,
    TransportStateTransitioning = 4,
    TransportStateError      = 5
};

// ============================================================================
// TransportSeekFlags
// ============================================================================
enum TransportSeekFlags
{
    TransportSeekFlagNone        = 0x0000,
    TransportSeekFlagAbsolute    = 0x0001,
    TransportSeekFlagRelative    = 0x0002,
    TransportSeekFlagKeyFrame    = 0x0004,
    TransportSeekFlagDisableEVR  = 0x0008
};

// ============================================================================
// TransportBase
// ============================================================================
// Base class for timeline playback transport. Provides the core interface
// for play, pause, seek, and rate control operations shared by both
// preview playback (MovieTransport) and export (RenderTransport).
//
class TransportBase
{
public:
    TransportBase();
    virtual ~TransportBase();

    // -- Playback control --
    virtual HRESULT Play() = 0;
    virtual HRESULT Pause() = 0;
    virtual HRESULT Stop() = 0;
    virtual HRESULT Seek(LONGLONG llPositionHns, DWORD dwFlags = TransportSeekFlagAbsolute) = 0;

    // -- Rate control --
    virtual HRESULT SetRate(double dblRate);
    double GetRate() const throw();

    // -- Position --
    virtual HRESULT GetCurrentPosition(LONGLONG* pllPositionHns) const;
    virtual HRESULT GetDuration(LONGLONG* pllDurationHns) const;

    // -- State --
    TransportState GetState() const throw();
    bool IsPlaying() const throw();
    bool IsPaused() const throw();
    bool IsStopped() const throw();

    // -- Volume --
    virtual HRESULT SetVolume(float flVolume);
    float GetVolume() const throw();

    // -- Event callbacks --
    void SetStateChangeCallback(std::function<void(TransportState, TransportState)> fn);
    void SetPositionChangeCallback(std::function<void(LONGLONG)> fn);
    void SetErrorCallback(std::function<void(HRESULT)> fn);

protected:
    void SetState(TransportState state);
    void FireStateChange(TransportState oldState, TransportState newState);
    void FirePositionChange(LONGLONG llPosition);
    void FireError(HRESULT hr);

    TransportState  m_state;
    double          m_dblRate;
    float           m_flVolume;
    LONGLONG        m_llCurrentPositionHns;
    LONGLONG        m_llDurationHns;

    std::function<void(TransportState, TransportState)> m_fnStateChange;
    std::function<void(LONGLONG)>                      m_fnPositionChange;
    std::function<void(HRESULT)>                       m_fnError;

private:
    TransportBase(const TransportBase&);
    TransportBase& operator=(const TransportBase&);
};

// ============================================================================
// MovieTransport
// ============================================================================
// Preview playback transport for the timeline. Manages the Media Foundation
// playback session, Direct3D 11 rendering surface, and audio output.
// Supports play, pause, seek, variable-rate playback, and A-B looping.
//
class MovieTransport : public TransportBase
{
public:
    MovieTransport();
    ~MovieTransport();

    // -- Playback control --
    HRESULT Play() override;
    HRESULT Pause() override;
    HRESULT Stop() override;
    HRESULT Seek(LONGLONG llPositionHns, DWORD dwFlags = TransportSeekFlagAbsolute) override;

    // -- Rate control --
    HRESULT SetRate(double dblRate) override;

    // -- Loop region --
    void SetLoopRegion(LONGLONG llStartHns, LONGLONG llEndHns);
    void ClearLoopRegion();
    bool HasLoopRegion() const throw();
    LONGLONG GetLoopStartHns() const throw();
    LONGLONG GetLoopEndHns() const throw();

    // -- Frame stepping --
    HRESULT StepForward();
    HRESULT StepBackward();
    void SetFrameStepSize(LONGLONG llStepHns);
    LONGLONG GetFrameStepSize() const throw();

    // -- Source management --
    HRESULT OpenMedia(LPCWSTR pszFilePath);
    HRESULT CloseMedia();
    bool IsMediaOpen() const throw();

private:
    bool        m_bMediaOpen;
    bool        m_bLoopEnabled;
    LONGLONG    m_llLoopStartHns;
    LONGLONG    m_llLoopEndHns;
    LONGLONG    m_llFrameStepHns;

    // Media Foundation session
    ATL::CComPtr<IMFMediaSession>   m_spSession;
    ATL::CComPtr<IMFMediaSource>    m_spSource;
    ATL::CComPtr<IMFTopology>       m_spTopology;
};

// ============================================================================
// RenderTransport
// ============================================================================
// Export/render transport. Operates in a headless mode to encode the project
// timeline to an output file. Uses the Media Foundation transcode pipeline
// with configurable encoding profiles and progress reporting.
//
class RenderTransport : public TransportBase
{
public:
    RenderTransport();
    ~RenderTransport();

    // -- Playback control --
    HRESULT Play() override;
    HRESULT Pause() override;
    HRESULT Stop() override;
    HRESULT Seek(LONGLONG llPositionHns, DWORD dwFlags = TransportSeekFlagAbsolute) override;

    // -- Render configuration --
    HRESULT SetOutputPath(LPCWSTR pszPath);
    ATL::CString GetOutputPath() const;

    HRESULT SetProfile(DWORD dwProfileIndex);
    DWORD GetProfile() const throw();

    void SetOutputWidth(UINT uWidth);
    UINT GetOutputWidth() const throw();

    void SetOutputHeight(UINT uHeight);
    UINT GetOutputHeight() const throw();

    void SetFrameRate(DWORD dwFrameRateNumerator, DWORD dwFrameRateDenominator);
    DWORD GetFrameRateNumerator() const throw();
    DWORD GetFrameRateDenominator() const throw();

    void SetBitRate(DWORD dwBitRateKbps);
    DWORD GetBitRate() const throw();

    // -- Progress --
    void SetProgressCallback(std::function<void(float)> fn);
    float GetRenderProgress() const throw();

    // -- Abort --
    void RequestAbort();
    bool IsAbortRequested() const throw();

    // -- Result --
    HRESULT GetRenderResult() const throw();

private:
    ATL::CString m_strOutputPath;
    DWORD        m_dwProfileIndex;
    UINT         m_uOutputWidth;
    UINT         m_uOutputHeight;
    DWORD        m_dwFrameRateNumerator;
    DWORD        m_dwFrameRateDenominator;
    DWORD        m_dwBitRateKbps;
    float        m_flRenderProgress;
    bool         m_bAbortRequested;
    HRESULT      m_hrRenderResult;

    std::function<void(float)> m_fnProgress;
};

#endif // TRANSPORTBASE_H
