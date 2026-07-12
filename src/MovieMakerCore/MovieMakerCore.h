/*
 * MovieMakerCore.h
 *
 * Main public header for MovieMakerCore.dll.
 * Provides the single exported entry point for the Windows Live Movie Maker
 * core engine. This DLL contains the entire Sundance application framework,
 * including UI, rendering pipeline, and project management.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef MOVIECORE_H
#define MOVIECORE_H

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

// ============================================================================
// Export / import macros
// ============================================================================
#ifdef MOVIECORE_EXPORTS
    #define MOVIECORE_API __declspec(dllexport)
#else
    #define MOVIECORE_API __declspec(dllimport)
#endif

// ============================================================================
// Include project-wide common definitions and utility types
// ============================================================================
#include "common.h"
#include "exports.h"

// ============================================================================
// Sundance codename definitions
// ============================================================================
#define SUNDANCE_VERSION_MAJOR      16
#define SUNDANCE_VERSION_MINOR      4
#define SUNDANCE_VERSION_BUILD      3528
#define SUNDANCE_VERSION_REVISION   331

#define SUNDANCE_VERSION_STRING     L"16.4.3528.331"
#define SUNDANCE_PRODUCT_NAME       L"Windows Live Movie Maker"
#define SUNDANCE_DLL_NAME           L"MovieMakerCore.dll"

// ============================================================================
// Application exit codes
// ============================================================================
#define SUNDANCE_EXIT_SUCCESS           0
#define SUNDANCE_EXIT_INIT_FAILED       1
#define SUNDANCE_EXIT_SINGLE_INSTANCE   2
#define SUNDANCE_EXIT_APP_ERROR         3
#define SUNDANCE_EXIT_NO_RENDER         4

// ============================================================================
// Forward declarations (Sundance framework classes)
// ============================================================================
namespace Sundance
{
    class ApplicationOptions;
    class AppMain;
    class CommandLineParser;
    class ResourceManager;
    class ThemeManager;
    class ThumbnailCache;
    class ProjectManager;
    class TimelineController;
    class ExportController;
    class ImportController;
    class PlaybackController;
    class UndoManager;
    class AutoSaveManager;
    class SqmLogger;
    class TelemetrySession;
}

// ============================================================================
// MovieMakerMain -- the single DLL export
// ============================================================================
// This is the sole entry point exported by MovieMakerCore.dll (ordinal @1).
// The launcher (MovieMaker.exe) loads this DLL dynamically and calls this
// function after bootstrapping WLXPhotoBase.
//
// Parameters:
//   argc - Number of command-line arguments (including executable name)
//   argv - Array of wide-character command-line argument strings
//
// Returns:
//   Application exit code (SUNDANCE_EXIT_* values)
//
extern "C"
{
    MOVIECORE_API int __cdecl MovieMakerMain(int argc, wchar_t** argv);
}

#endif // MOVIECORE_H
