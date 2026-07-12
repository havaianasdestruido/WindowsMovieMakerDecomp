/*
 * VideoCapture.h
 *
 * VideoCapture - Video capture source (webcam). Wraps Media Foundation
 * capture device APIs for live video preview and recording.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef VIDEOCAPTURE_H
#define VIDEOCAPTURE_H

#include "HMRAVSource.h"
#include "AVSource.h"
#include <mfapi.h>
#include <mfidl.h>

namespace HMRAVSource
{

// ============================================================================
// VideoCaptureDeviceInfo
// ============================================================================
struct VideoCaptureDeviceInfo
{
    ATL::CString    strDeviceName;
    ATL::CString    strDevicePath;
    UINT            uMaxWidth = 0;
    UINT            uMaxHeight = 0;
    double          dblMaxFrameRate = 0.0;
    bool            fHasPreview = true;
    bool            fHasCapture = true;
};

// ============================================================================
// VideoCaptureConfig
// ============================================================================
struct VideoCaptureConfig
{
    UINT    uRequestedWidth = 640;
    UINT    uRequestedHeight = 480;
    double  dblRequestedFrameRate = 30.0;
    bool    fEnablePreview = true;
    bool    fMirrorPreview = false;
};

// ============================================================================
// VideoCapture
// ============================================================================
// Manages video capture from a webcam using Media Foundation capture APIs.
// Provides device enumeration, preview rendering, and sample capture.
//
class AVSOURCE_API VideoCapture
{
public:
    VideoCapture();
    ~VideoCapture();

    // Device enumeration
    static HRESULT EnumDevices(ATL::CAtlArray<VideoCaptureDeviceInfo>& devices);
    static HRESULT GetDefaultDevice(VideoCaptureDeviceInfo* pInfo);

    // Lifecycle
    HRESULT Initialize(const VideoCaptureDeviceInfo& deviceInfo);
    HRESULT Shutdown();

    // Preview
    HRESULT StartPreview(HWND hWnd);
    HRESULT StopPreview();
    HRESULT IsPreviewing() const throw();

    // Capture
    HRESULT StartCapture(LPCWSTR pszOutputPath);
    HRESULT StopCapture();
    HRESULT IsCapturing() const throw();

    // Single frame capture
    HRESULT CaptureFrame(IMFSample** ppSample);

    // Configuration
    HRESULT SetConfig(const VideoCaptureConfig& config);
    VideoCaptureConfig GetConfig() const throw();

    // State
    CaptureState GetState() const throw();

    // Device info
    VideoCaptureDeviceInfo GetDeviceInfo() const;

    // Resolution
    HRESULT SetCaptureResolution(UINT uWidth, UINT uHeight);
    HRESULT GetCaptureResolution(UINT* puWidth, UINT* puHeight);

    // Frame callback
    using FrameCallback = std::function<void(IMFSample*)>;
    void SetFrameCallback(FrameCallback cb) { m_frameCb = cb; }

private:
    CComPtr<IMFMediaSource>         m_spMediaSource;
    CComPtr<IMFCaptureEngine>       m_spCaptureEngine;
    CComPtr<IMFCapturePreviewSink>  m_spPreviewSink;
    CComPtr<IMFCaptureRecordSink>   m_spRecordSink;

    VideoCaptureDeviceInfo           m_deviceInfo;
    VideoCaptureConfig               m_config;
    CaptureState                     m_state;
    HWND                             m_hPreviewWnd;
    bool                             m_fPreviewing;
    bool                             m_fCapturing;

    FrameCallback                    m_frameCb;

    HRESULT CreateCaptureEngine();
    HRESULT ConfigureCaptureEngine();
    HRESULT FindBestMediaType(CComPtr<IMFMediaType>& spType);
};

} // namespace HMRAVSource

#endif // VIDEOCAPTURE_H
