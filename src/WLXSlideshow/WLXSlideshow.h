/*
 * WLXSlideshow.h
 *
 * Public header for WLXSlideshow.dll.
 * Provides slideshow generation capabilities for Windows Live Movie Maker 2012.
 *
 * Manages:
 *   - Photo sequencing with automatic timing
 *   - Pan/zoom (Ken Burns) animations
 *   - Transition insertion between slides
 *   - Audio track synchronization
 *   - Slideshow template management (e.g., "Noel", "Contemporary", "Classic")
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLXSLIDESHOW_H
#define WLXSLIDESHOW_H

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

#ifdef WLXSLIDESHOW_EXPORTS
    #define WLXSLD_API __declspec(dllexport)
#else
    #define WLXSLD_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace Slideshow
{
    class SlideshowEngine;
    class SlideSequence;
    class TransitionPlacer;
    class AudioSync;
    struct SlideInfo;
    struct SlideshowConfig;
}

// ============================================================================
// Slide info
// ============================================================================
struct SlideshowSlideInfo
{
    WCHAR       wszImagePath[MAX_PATH];
    LONGLONG    llDuration;         // in 100ns units
    UINT32      uPanStartX;         // Ken Burns pan start (percent)
    UINT32      uPanStartY;
    UINT32      uPanEndX;
    UINT32      uPanEndY;
    UINT32      uZoomStart;         // zoom start (percent, 100 = no zoom)
    UINT32      uZoomEnd;
    WCHAR       wszTransitionId[64];

    SlideshowSlideInfo()
        : llDuration(30000000LL)    // 3 seconds default
        , uPanStartX(0)
        , uPanStartY(0)
        , uPanEndX(100)
        , uPanEndY(100)
        , uZoomStart(100)
        , uZoomEnd(100)
    {
        ZeroMemory(wszImagePath, sizeof(wszImagePath));
        ZeroMemory(wszTransitionId, sizeof(wszTransitionId));
    }
};

// ============================================================================
// Slideshow configuration
// ============================================================================
struct SlideshowConfig
{
    UINT32      uSlideDurationMs;   // default slide duration in ms
    UINT32      uTransitionDurationMs;
    WCHAR       wszTemplateId[64];
    WCHAR       wszAudioPath[MAX_PATH];
    BOOL        bAutoSyncToAudio;
    BOOL        bRandomizeTransitions;

    SlideshowConfig()
        : uSlideDurationMs(5000)
        , uTransitionDurationMs(1000)
        , bAutoSyncToAudio(FALSE)
        , bRandomizeTransitions(FALSE)
    {
        ZeroMemory(wszTemplateId, sizeof(wszTemplateId));
        ZeroMemory(wszAudioPath, sizeof(wszAudioPath));
    }
};

// ============================================================================
// Exported functions (4 exports)
// ============================================================================
extern "C"
{
    // Creates a slideshow engine instance.
    WLXSLD_API HANDLE __stdcall Slideshow_Create();

    // Destroys the slideshow engine.
    WLXSLD_API void __stdcall Slideshow_Destroy(HANDLE hSlideshow);

    // Generates a slideshow from the given slides and configuration.
    // Returns the total duration in 100ns units.
    WLXSLD_API HRESULT __stdcall Slideshow_Generate(HANDLE hSlideshow,
        const SlideshowSlideInfo* pSlides, UINT32 uSlideCount,
        const SlideshowConfig* pConfig, LONGLONG* pTotalDuration);

    // Retrieves available slideshow template IDs.
    WLXSLD_API HRESULT __stdcall Slideshow_EnumerateTemplates(WCHAR* pTemplateIds,
        UINT32* pCount);
}

#endif // WLXSLIDESHOW_H
