/*
 * MovieMakerPreviewClient.h
 *
 * Public header for MovieMakerPreviewClient.dll.
 * Provides the preview window communication interface used by the main
 * Movie Maker application to interact with the embedded preview pane.
 *
 * The preview client manages:
 *   - Preview window creation and lifecycle
 *   - Frame rendering requests (seek + render single frame)
 *   - Transport control (play/pause/stop/seek)
 *   - Resize and aspect ratio handling
 *   - Snapshot capture from the preview surface
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef MOVIEPREVIEWCLIENT_H
#define MOVIEPREVIEWCLIENT_H

#ifndef STRICT
#define STRICT
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WINVER        0x0602
#define _WIN32_WINNT  0x0602
#define _WIN32_IE     0x0800

#include <windows.h>
#include <objbase.h>
#include <gdiplus.h>
#include <unknwn.h>

#ifdef MOVIEPREVIEWCLIENT_EXPORTS
    #define MPC_API __declspec(dllexport)
#else
    #define MPC_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace Preview
{
    class PreviewClient;
    class PreviewWindow;
    class PreviewRenderer;
    struct PreviewState;
    struct PreviewConfig;
}

// ============================================================================
// Preview transport state
// ============================================================================
enum PreviewTransportState
{
    PreviewTransportStopped = 0,
    PreviewTransportPlaying = 1,
    PreviewTransportPaused  = 2,
    PreviewTransportSeeking = 3,
};

// ============================================================================
// Preview configuration
// ============================================================================
struct PreviewConfig
{
    HWND    hParentWnd;
    RECT    rcPreview;
    UINT    uFrameWidth;
    UINT    uFrameHeight;
    double  dAspectRatio;
    BOOL    bHardwareAccel;
    PreviewConfig()
        : hParentWnd(NULL)
        , uFrameWidth(640)
        , uFrameHeight(480)
        , dAspectRatio(4.0 / 3.0)
        , bHardwareAccel(TRUE)
    {
        SetRectEmpty(&rcPreview);
    }
};

// ============================================================================
// Exported functions (4 exports)
// ============================================================================
extern "C"
{
    // Creates and initializes the preview client.
    // Returns an opaque handle to the client instance.
    MPC_API HANDLE __stdcall PreviewClient_Create(const PreviewConfig* pConfig);

    // Destroys the preview client and releases all resources.
    MPC_API void __stdcall PreviewClient_Destroy(HANDLE hClient);

    // Sends a command to the preview client.
    //   uCommand: command ID (see PreviewCommand enum below)
    //   lParam:   command-specific data
    // Returns S_OK on success.
    MPC_API HRESULT __stdcall PreviewClient_Command(HANDLE hClient, UINT uCommand, LPARAM lParam);

    // Retrieves the current state of the preview client.
    //   pState: receives the current PreviewTransportState
    // Returns S_OK on success.
    MPC_API HRESULT __stdcall PreviewClient_GetState(HANDLE hClient, UINT* pState);
}

// ============================================================================
// Preview command IDs (used with PreviewClient_Command)
// ============================================================================
enum PreviewCommand
{
    PreviewCmd_LoadFile       = 0x0001,
    PreviewCmd_CloseFile     = 0x0002,
    PreviewCmd_Play           = 0x0003,
    PreviewCmd_Pause          = 0x0004,
    PreviewCmd_Stop           = 0x0005,
    PreviewCmd_Seek           = 0x0006,
    PreviewCmd_RenderFrame    = 0x0007,
    PreviewCmd_Snapshot       = 0x0008,
    PreviewCmd_Resize         = 0x0009,
    PreviewCmd_SetVolume      = 0x000A,
    PreviewCmd_GetPosition    = 0x000B,
    PreviewCmd_GetDuration    = 0x000C,
};

#endif // MOVIEPREVIEWCLIENT_H
