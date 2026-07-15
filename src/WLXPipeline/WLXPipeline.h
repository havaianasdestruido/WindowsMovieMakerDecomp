/*
 * WLXPipeline.h
 *
 * Public header for WLXPipeline.dll.
 * Provides the video processing pipeline for Windows Live Movie Maker 2012.
 * Manages frame processing, effects application, color space conversion,
 * and rendering through Direct3D 9, GDI+, and DirectDraw surfaces.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLXPIPELINE_H
#define WLXPIPELINE_H

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

#ifdef WLXPIPELINE_EXPORTS
    #define WLXPIPE_API __declspec(dllexport)
#else
    #define WLXPIPE_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace Pipeline
{
    class ProcessingPipeline;
    class FrameSource;
    class FrameSink;
    class EffectChain;
    class ColorConverter;
    class D3D9Surface;
}

// ============================================================================
// Pipeline configuration
// ============================================================================
struct PipelineConfig
{
    UINT32      uInputWidth;
    UINT32      uInputHeight;
    UINT32      uOutputWidth;
    UINT32      uOutputHeight;
    D3DFORMAT   d3dFormat;
    UINT32      uFrameRate;
    BOOL        bUseHardwareProcessing;

    PipelineConfig()
        : uInputWidth(0)
        , uInputHeight(0)
        , uOutputWidth(0)
        , uOutputHeight(0)
        , d3dFormat(D3DFMT_A8R8G8B8)
        , uFrameRate(30)
        , bUseHardwareProcessing(TRUE)
    {
    }
};

// ============================================================================
// Exported functions -- GetPipelineCreateFunctions + DllRegisterServer
// ============================================================================
extern "C"
{
    WLXPIPE_API HRESULT __stdcall GetPipelineCreateFunctions(void** ppFunctions, UINT32* pCount);
    WLXPIPE_API HRESULT __stdcall DllRegisterServer();
}

#endif // WLXPIPELINE_H
