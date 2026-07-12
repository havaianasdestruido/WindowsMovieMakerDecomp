/*
 * DShowSource.h
 *
 * DShowSource - DirectShow source wrapper for legacy media formats.
 * Wraps IGraphBuilder / IMediaControl / IMediaSeeking for file playback
 * and sample capture via DirectShow filters.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef DSOWSOURCE_H
#define DSOWSOURCE_H

#include "AVSource.h"
#include <dshow.h>

namespace HMRAVSource
{

// ============================================================================
// DShowSource
// ============================================================================
// DirectShow source wrapper. Creates a render filter graph for media files
// that are not well-supported by Media Foundation (e.g., certain legacy
// codecs). Provides sample reading via IMediaSample interfaces.
//
class DShowSource : public AVSource
{
public:
    DShowSource();
    virtual ~DShowSource();

    // AVSource overrides
    HRESULT Open(const AVSourceDesc& desc) override;
    HRESULT Close() override;

    HRESULT SetPositionHns(LONGLONG llPosition) override;

    HRESULT Start() override;
    HRESULT Stop() override;
    HRESULT Pause() override;
    HRESULT Resume() override;

    HRESULT ReadSample(IMFSample** ppSample, DWORD dwStreamIndex) override;
    HRESULT Flush() override;

    // DShow-specific
    HRESULT GetGraphBuilder(IGraphBuilder** ppGraph);
    HRESULT GetMediaControl(IMediaControl** ppControl);
    HRESULT GetMediaSeeking(IMediaSeeking** ppSeeking);
    HRESULT GetMediaEvent(IMediaEventEx** ppEvent);

    // Filter graph
    HRESULT BuildFilterGraph(LPCWSTR pszFilePath);
    HRESULT RenderFile(LPCWSTR pszFilePath);
    HRESULT GetVideoWindowHandle(HWND* phWnd);
    HRESULT SetVideoWindowHandle(HWND hWnd);

    // DShow event pump
    HRESULT CheckDShowEvent(long* pEventCode, LONG_PTR* pParam1, LONG_PTR* pParam2);

private:
    CComPtr<IGraphBuilder>      m_spGraph;
    CComPtr<IMediaControl>      m_spControl;
    CComPtr<IMediaSeeking>      m_spSeeking;
    CComPtr<IMediaEventEx>      m_spEvent;
    CComPtr<IVideoWindow>       m_spVideoWindow;
    CComPtr<IBasicVideo>        m_spBasicVideo;
    CComPtr<IBasicAudio>        m_spBasicAudio;

    // Sample grabbing
    CComPtr<ISampleGrabber>     m_spSampleGrabber;
    CComPtr<IBaseFilter>        m_spGrabberFilter;

    HWND    m_hVideoWnd;
    bool    m_fGraphBuilt;

    HRESULT CreateSampleGrabber();
    HRESULT ConnectGrabberToGraph();
    HRESULT QueryVideoDimensions();
    HRESULT ConfigureDefaultAudio();
};

} // namespace HMRAVSource

#endif // DSOWSOURCE_H
