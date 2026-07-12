/*
 * WLXPipetran.h
 *
 * Public header for WLXPipetran.dll.
 * Provides pipeline transform classes for transitions and effects animations
 * in Windows Live Movie Maker 2012. Contains 92 RTTI classes implementing
 * various transition types, visual effects, and animation transforms.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLXPIPETRAN_H
#define WLXPIPETRAN_H

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
#include <d3d9.h>

#ifdef WLXPIPETRAN_EXPORTS
    #define WLXPIPET_API __declspec(dllexport)
#else
    #define WLXPIPET_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations for transition/effect class hierarchy
// ============================================================================
namespace Pipetran
{
    class TransitionBase;
    class EffectBase;
    class TransformRegistry;

    // Transition types (cross-fade, wipe, slide, etc.)
    class CrossFadeTransition;
    class FadeToBlackTransition;
    class FadeToWhiteTransition;
    class DiagonalWipeTransition;
    class HorizontalWipeTransition;
    class VerticalWipeTransition;
    class CircleWipeTransition;
    class DiamondWipeTransition;
    class StarWipeTransition;
    class BowTieWipeTransition;
    class ClockWipeTransition;
    class PushLeftTransition;
    class PushRightTransition;
    class PushUpTransition;
    class PushDownTransition;
    class SlideLeftTransition;
    class SlideRightTransition;
    class RotateTransition;
    class FlipTransition;
    class SpinTransition;
    class ZoomTransition;
    class PixelateTransition;
    class DissolveTransition;
    class BlurTransition;
    class ShrinkTransition;
    class ExpandTransition;

    // Effect types (adjustments, overlays, etc.)
    class BrightnessEffect;
    class ContrastEffect;
    class SaturationEffect;
    class HueEffect;
    class SepiaEffect;
    class BlackAndWhiteEffect;
    class InvertEffect;
    class BlurEffect;
    class SharpenEffect;
    class EdgeDetectEffect;
    class EmbossEffect;
    class PixelateEffect;
    class MosaicEffect;
    class OldFilmEffect;
    class VignetteEffect;
    class GrayscaleEffect;
}

// ============================================================================
// Transform parameters
// ============================================================================
struct TransformParams
{
    DOUBLE      dProgress;          // 0.0 to 1.0 animation progress
    UINT32      uFrameWidth;
    UINT32      uFrameHeight;
    UINT64      uTimeMs;            // current time in milliseconds
    UINT64      uDurationMs;        // total transform duration
    DOUBLE      dParameter1;        // effect-specific parameter
    DOUBLE      dParameter2;
    DOUBLE      dParameter3;
    IDirect3DDevice9* pD3DDevice;   // optional hardware device

    TransformParams()
        : dProgress(0.0)
        , uFrameWidth(0)
        , uFrameHeight(0)
        , uTimeMs(0)
        , uDurationMs(0)
        , dParameter1(0.0)
        , dParameter2(0.0)
        , dParameter3(0.0)
        , pD3DDevice(NULL)
    {
    }
};

// ============================================================================
// Exported function (1 export)
// ============================================================================
extern "C"
{
    // Returns the global TransformRegistry singleton.
    // The registry manages creation and lifecycle of all transform objects.
    WLXPIPET_API void* __cdecl Pipetran_GetRegistry();
}

#endif // WLXPIPETRAN_H
