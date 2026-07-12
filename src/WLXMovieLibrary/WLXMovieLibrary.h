/*
 * WLXMovieLibrary.h
 *
 * Public header for WLXMovieLibrary.dll.
 * Provides movie library management and Media Foundation integration for
 * Windows Live Photo Gallery / Movie Maker 2012.
 *
 * Responsibilities:
 *   - Enumerate video files from the system library
 *   - Query media properties (duration, resolution, codec, bitrate)
 *   - Manage thumbnail generation and caching for video files
 *   - Media Foundation session management for decoding
 *   - Hardware-accelerated video processing via D3D9/D3D11/DXVA2
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLXMOVIELIBRARY_H
#define WLXMOVIELIBRARY_H

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
#include <mfapi.h>
#include <mfidl.h>
#include <gdiplus.h>

#ifdef WLXMOVIELIBRARY_EXPORTS
    #define WLXML_API __declspec(dllexport)
#else
    #define WLXML_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace MovieLibrary
{
    class LibraryManager;
    class MediaInfo;
    class ThumbnailCache;
    class MFSessionManager;
    class VideoProcessor;
}

// ============================================================================
// Media info structure
// ============================================================================
struct MovieMediaInfo
{
    WCHAR       wszFilePath[MAX_PATH];
    LONGLONG    llDuration;         // in 100ns units
    UINT32      uWidth;
    UINT32      uHeight;
    UINT32      uFrameRate;         // frames per second
    UINT32      uBitrate;           // bits per second
    UINT32      uVideoCodecFourCC;
    UINT32      uAudioCodecFourCC;
    BOOL        bHasVideo;
    BOOL        bHasAudio;
    FILETIME    ftLastModified;

    MovieMediaInfo()
        : llDuration(0)
        , uWidth(0)
        , uHeight(0)
        , uFrameRate(0)
        , uBitrate(0)
        , uVideoCodecFourCC(0)
        , uAudioCodecFourCC(0)
        , bHasVideo(FALSE)
        , bHasAudio(FALSE)
    {
        ZeroMemory(wszFilePath, sizeof(wszFilePath));
        ftLastModified.dwLowDateTime = 0;
        ftLastModified.dwHighDateTime = 0;
    }
};

// ============================================================================
// Exported function (1 export)
// ============================================================================
extern "C"
{
    // Returns the singleton LibraryManager instance.
    // The manager handles enumeration, thumbnail caching, and MF sessions.
    WLXML_API void* __cdecl MovieLibrary_GetInstance();
}

#endif // WLXMOVIELIBRARY_H
