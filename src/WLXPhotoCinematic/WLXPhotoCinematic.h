/*
 * WLXPhotoCinematic.h
 *
 * Public header for WLXPhotoCinematic.dll.
 * Provides cinematic photo effects for Windows Live Movie Maker 2012,
 * primarily Ken Burns pan/zoom animations applied to still images.
 *
 * Ken Burns effect: slow pan and zoom on a still image to create the
 * illusion of motion. Parameters include start/end viewport rectangles
 * and easing curves for natural-looking motion.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLXPHOTOCINEMATIC_H
#define WLXPHOTOCINEMATIC_H

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

#ifdef WLXPHOTOCINEMATIC_EXPORTS
    #define WLXPCIN_API __declspec(dllexport)
#else
    #define WLXPCIN_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace PhotoCinematic
{
    class CinematicEngine;
    class KenBurnsEffect;
    class EasingFunction;
    struct CinematicParams;
    struct FrameOutput;
}

// ============================================================================
// Easing type
// ============================================================================
enum EasingType
{
    EasingType_Linear       = 0,
    EasingType_EaseIn       = 1,
    EasingType_EaseOut      = 2,
    EasingType_EaseInOut    = 3,
};

// ============================================================================
// Viewport rectangle (normalized 0.0 - 1.0 coordinates)
// ============================================================================
struct NormalizedRect
{
    DOUBLE  dLeft;
    DOUBLE  dTop;
    DOUBLE  dRight;
    DOUBLE  dBottom;

    NormalizedRect()
        : dLeft(0.0), dTop(0.0), dRight(1.0), dBottom(1.0)
    {}

    NormalizedRect(DOUBLE l, DOUBLE t, DOUBLE r, DOUBLE b)
        : dLeft(l), dTop(t), dRight(r), dBottom(b)
    {}
};

// ============================================================================
// Cinematic parameters
// ============================================================================
struct CinematicParams
{
    NormalizedRect  rcViewportStart;    // start viewport (region of image to show)
    NormalizedRect  rcViewportEnd;      // end viewport
    LONGLONG        llDuration;         // duration in 100ns units
    EasingType      eEasing;
    UINT32          uOutputWidth;       // output frame dimensions
    UINT32          uOutputHeight;

    CinematicParams()
        : llDuration(30000000LL)        // 3 seconds
        , eEasing(EasingType_EaseInOut)
        , uOutputWidth(1920)
        , uOutputHeight(1080)
    {}
};

// ============================================================================
// Exported functions (4 exports)
// ============================================================================
extern "C"
{
    // Creates a cinematic effects engine.
    WLXPCIN_API HANDLE __stdcall PhotoCinematic_Create();

    // Destroys the engine.
    WLXPCIN_API void __stdcall PhotoCinematic_Destroy(HANDLE hEngine);

    // Renders a single frame from a photo with Ken Burns effect.
    // dProgress: 0.0 (start) to 1.0 (end)
    // ppBitmap:  receives the rendered frame (caller must delete)
    WLXPCIN_API HRESULT __stdcall PhotoCinematic_RenderFrame(HANDLE hEngine,
        Gdiplus::Bitmap* pSourceImage, const CinematicParams* pParams,
        DOUBLE dProgress, Gdiplus::Bitmap** ppBitmap);

    // Renders the entire Ken Burns animation as a sequence of frames.
    // ppFrames:  receives an array of frame bitmaps (caller must delete each)
    // pFrameCount: receives the number of frames
    WLXPCIN_API HRESULT __stdcall PhotoCinematic_RenderAll(HANDLE hEngine,
        Gdiplus::Bitmap* pSourceImage, const CinematicParams* pParams,
        UINT32 uFrameRate, Gdiplus::Bitmap*** ppFrames, UINT32* pFrameCount);
}

#endif // WLXPHOTOCINEMATIC_H
