/*
 * DShowMFSourceReaderBuilder.h
 *
 * DShowMFSourceReaderBuilder - Bridges DirectShow filters to the MF source
 * reader interface. For legacy formats only supported via DirectShow, this
 * builder creates a DShow filter graph and wraps it as an MF source reader.
 *
 * RTTI: ?AVDShowMFSourceReaderBuilder@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef DSOWMFSOURCEREADERBUILDER_H
#define DSOWMFSOURCEREADERBUILDER_H

#include "HMRAVSource.h"
#include "MFSourceReaderBuilder.h"

namespace HMRAVSource
{

// ============================================================================
// DShowMFSourceReaderBuilder
// ============================================================================
// Creates an MF-compatible source reader that internally uses a DirectShow
// filter graph for formats not natively supported by Media Foundation.
// The DShow graph captures frames and wraps them as MF samples.
//
class AVSOURCE_API DShowMFSourceReaderBuilder
{
public:
    DShowMFSourceReaderBuilder();
    ~DShowMFSourceReaderBuilder();

    // Lifecycle
    HRESULT Initialize();
    HRESULT Shutdown();

    // Source reader creation (wraps DShow in MF-compatible interface)
    HRESULT CreateSourceReader(LPCWSTR pszFilePath, IMFSourceReader** ppReader);
    HRESULT CreateSourceReaderWithCallback(
        LPCWSTR pszFilePath,
        IMFSourceReaderCallback* pCallback,
        IMFSourceReader** ppReader);

    // Format support
    static bool IsDShowRequired(LPCWSTR pszFilePath);
    static bool IsDShowOnlyFormat(LPCWSTR pszFilePath);

    // DShow-specific configuration
    HRESULT SetVideoWindow(HWND hWnd);
    HRESULT EnableDeinterlace(bool fEnable);
    HRESULT SetPreferredVideoFormat(const GUID* pSubtype);

    // Source info from DShow
    HRESULT GetVideoDimensions(LPCWSTR pszFilePath, DWORD* pdwWidth, DWORD* pdwHeight);
    HRESULT GetFrameRate(LPCWSTR pszFilePath, double* pdblFrameRate);
    HRESULT GetDuration(LPCWSTR pszFilePath, LONGLONG* pllDurationHns);

    // Filter graph access
    HRESULT GetFilterGraph(IGraphBuilder** ppGraph);
    HRESULT GetMediaControl(IMediaControl** ppControl);
    HRESULT GetMediaEvent(IMediaEventEx** ppEvent);

private:
    bool                m_fInitialized;
    HWND                m_hVideoWnd;
    bool                m_fDeinterlace;
    GUID                m_guidPreferredSubtype;

    CComPtr<IGraphBuilder>  m_spGraph;
    CComPtr<IMediaControl>  m_spControl;
    CComPtr<IMediaEventEx>  m_spEvent;
    CComPtr<IMediaSeeking>  m_spSeeking;

    HRESULT CreateFilterGraph(LPCWSTR pszFilePath);
    HRESULT AddVideoRenderer(IGraphBuilder* pGraph);
    HRESULT AddSampleGrabber(IGraphBuilder* pGraph);
    HRESULT ConnectFilters();
    static bool IsLegacyExtension(LPCWSTR pszExtension);
};

} // namespace HMRAVSource

#endif // DSOWMFSOURCEREADERBUILDER_H
