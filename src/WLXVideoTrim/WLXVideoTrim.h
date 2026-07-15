/*
 * WLXVideoTrim.h
 *
 * Public header for WLXVideoTrim.dll.
 * Provides video trimming functionality for Windows Live Movie Maker 2012.
 * Supports trimming operations on AVI, ASF, DV, MPEG2, MP4, and WMV containers
 * using DirectShow-based transcoding pipelines.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLXVIDEOTRIM_H
#define WLXVIDEOTRIM_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WINVER        0x0602
#define _WIN32_WINNT  0x0602

#include <windows.h>
#include <objbase.h>
#include <gdiplus.h>

#ifdef WLXVIDEOTRIM_EXPORTS
    #define WLXVT_API __declspec(dllexport)
#else
    #define WLXVT_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace VideoTrim
{
    class TrimmerEngine;
    class TrimJob;
    class DirectShowGraph;
    class ASFTrimmer;
    class AVITrimmer;
    class MP4Trimmer;
}

// ============================================================================
// Trim status codes
// ============================================================================
enum TrimStatus
{
    TrimStatus_Idle       = 0,
    TrimStatus_Initializing = 1,
    TrimStatus_Running    = 2,
    TrimStatus_Paused     = 3,
    TrimStatus_Complete   = 4,
    TrimStatus_Error      = 5,
};

// ============================================================================
// Trim parameters
// ============================================================================
struct TrimParams
{
    WCHAR   wszInputPath[MAX_PATH];
    WCHAR   wszOutputPath[MAX_PATH];
    LONGLONG llStartTime;       // in 100ns units
    LONGLONG llEndTime;         // in 100ns units
    UINT32  uBitrate;
    BOOL    bCopyAudio;
    BOOL    bCopyVideo;
    UINT32  uOutputWidth;
    UINT32  uOutputHeight;

    TrimParams()
        : llStartTime(0)
        , llEndTime(0)
        , uBitrate(5000000)
        , bCopyAudio(TRUE)
        , bCopyVideo(TRUE)
        , uOutputWidth(0)
        , uOutputHeight(0)
    {
        ZeroMemory(wszInputPath, sizeof(wszInputPath));
        ZeroMemory(wszOutputPath, sizeof(wszOutputPath));
    }
};

// ============================================================================
// Trim progress callback
// ============================================================================
typedef void (CALLBACK* PFN_TRIM_PROGRESS)(void* pUserData, UINT uPercentComplete, LONGLONG llCurrentTime);
typedef void (CALLBACK* PFN_TRIM_COMPLETE)(void* pUserData, HRESULT hrResult);

// ============================================================================
// Exported functions -- DirectShow filter factory functions
// ============================================================================
extern "C"
{
    WLXVT_API HRESULT __stdcall CreateAVICopierDirect(IUnknown** ppUnknown);
    WLXVT_API HRESULT __stdcall CreateVideoCopierFromMediaType(const GUID* pMediaType, IUnknown** ppUnknown);
    WLXVT_API HRESULT __stdcall CreateVideoFormatContextTranscoder(IUnknown** ppUnknown);
    WLXVT_API HRESULT __stdcall CreateVideoPlayer(IUnknown** ppUnknown);
    WLXVT_API HRESULT __stdcall CreateVideoWMVTranscoder(IUnknown** ppUnknown);
}

#endif // WLXVIDEOTRIM_H
