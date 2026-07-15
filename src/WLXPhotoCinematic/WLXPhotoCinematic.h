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
// Forward declarations
// ============================================================================
namespace PhotoCinematic
{
    class CinematicEngine;
    class KenBurnsEffect;
    class EasingFunction;
    struct FrameOutput;

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
}

// ============================================================================
// Exported functions -- standard COM DLL entry points
// (DllCanUnloadNow, DllGetClassObject, DllRegisterServer, DllUnregisterServer
//  are defined in WLXPhotoCinematic.cpp but not declared here to avoid
//  conflict with combaseapi.h declarations)
// ============================================================================

#endif // WLXPHOTOCINEMATIC_H
