/*
 * HMRAVSource.h
 *
 * Root header for the HMRAVSource namespace within MovieMakerCore.dll.
 * Manages audio/video source handling, Media Foundation pipeline, DirectShow
 * integration, encoding profiles, stream sinks, capture sessions, and
 * transcode orchestration.
 *
 * RTTI analysis identifies 39 classes in this namespace. Key subsystems:
 *   - Media Foundation source/sink pipeline
 *   - DirectShow legacy source support
 *   - Encoding profile management (resolutions, bitrates, codecs)
 *   - Video/audio capture (webcam, narration)
 *   - Transcode orchestration
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef HMRAVSOURCE_H
#define HMRAVSOURCE_H

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
#include <shlwapi.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlsync.h>

// Media Foundation
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mfobjects.h>
#include <mfsession.h>
#include <mfmediaengine.h>

// DirectShow (legacy)
#include <dshow.h>

#include "WLXPhotoBase.h"
#include "../MovieMakerCore.h"

// ============================================================================
// Export / import macros
// ============================================================================
#ifdef MOVIECORE_EXPORTS
    #define AVSOURCE_API __declspec(dllexport)
#else
    #define AVSOURCE_API __declspec(dllimport)
#endif

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4505) // unreferenced local function has been removed
#pragma warning(disable: 4512) // assignment operator could not be generated

// ============================================================================
// HMRAVSource namespace
// ============================================================================
// The HMRAVSource namespace contains all classes responsible for audio/video
// source management, Media Foundation pipeline construction, encoding profile
// selection, stream sink handling, and transcode orchestration used by
// Windows Live Movie Maker.
//
namespace HMRAVSource
{

// ============================================================================
// Forward declarations - base source types
// ============================================================================
class AVSource;
class MFSource;
class DShowSource;

// ============================================================================
// Forward declarations - encoding / output
// ============================================================================
class EncodeProfile;
class StreamSink;
class TranscodeManager;

// ============================================================================
// Forward declarations - capture
// ============================================================================
class VideoCapture;
class AudioCapture;
class AVCaptureSession;
class AudioCaptureSession;

// ============================================================================
// Forward declarations - cached data
// ============================================================================
class CachedWFSection;

// ============================================================================
// Forward declarations - internal pipeline
// ============================================================================
class SourceReaderCallback;
class MFTransformChain;
class StreamDescriptor;
class SampleContext;

// ============================================================================
// Enums
// ============================================================================

// Source types
enum AVSourceType
{
    AVSourceTypeUnknown     = 0,
    AVSourceTypeFile        = 1,
    AVSourceTypeStream      = 2,
    AVSourceTypeCapture     = 3,
    AVSourceTypeTranscode   = 4
};

// Media types
enum AVMediaType
{
    AVMediaTypeUnknown  = 0,
    AVMediaTypeVideo    = 1,
    AVMediaTypeAudio    = 2,
    AVMediaTypeImage    = 3
};

// Source states
enum AVSourceState
{
    AVSourceStateIdle       = 0,
    AVSourceStateOpening    = 1,
    AVSourceStateOpen       = 2,
    AVSourceStateStarted    = 3,
    AVSourceStatePaused     = 4,
    AVSourceStateStopped    = 5,
    AVSourceStateError      = 6,
    AVSourceStateClosed     = 7
};

// Encoding profiles
enum EncodeProfileId
{
    EncodeProfileCustom         = 0,
    EncodeProfileDefault        = 1,
    EncodeProfileWLP            = 2,    // Windows Live Photo standard
    EncodeProfileWLHD           = 3,    // Windows Live HD
    EncodeProfileWLX            = 4,    // Windows Live extended
    EncodeProfileMP4_H264       = 5,
    EncodeProfileWMV9           = 6,
    EncodeProfileMP4_H264_720p  = 7,
    EncodeProfileMP4_H264_1080p = 8
};

// Video codec types
enum VideoCodec
{
    VideoCodecUnknown   = 0,
    VideoCodecH264      = 1,
    VideoCodecWMV9      = 2,
    VideoCodecMPEG4     = 3,
    VideoCodecH265      = 4
};

// Audio codec types
enum AudioCodec
{
    AudioCodecUnknown       = 0,
    AudioCodecAAC           = 1,
    AudioCodecWMA           = 2,
    AudioCodecMP3           = 3,
    AudioCodecPCM           = 4
};

// Stream sink states
enum StreamSinkState
{
    StreamSinkStateIdle     = 0,
    StreamSinkStateReady    = 1,
    StreamSinkStateWriting  = 2,
    StreamSinkStateDraining = 3,
    StreamSinkStateDone     = 4,
    StreamSinkStateError    = 5
};

// Capture states
enum CaptureState
{
    CaptureStateIdle        = 0,
    CaptureStatePreviewing  = 1,
    CaptureStateCapturing   = 2,
    CaptureStatePaused      = 3,
    CaptureStateError       = 4
};

// ============================================================================
// HMRAVSource initialization
// ============================================================================

// Initializes the HMRAVSource subsystem. Must be called before any other
// HMRAVSource functions. Sets up Media Foundation and COM.
AVSOURCE_API HRESULT HMRAVSourceInitialize();

// Shuts down the HMRAVSource subsystem. Releases all global resources.
AVSOURCE_API void HMRAVSourceShutdown();

// Returns true if HMRAVSource has been successfully initialized.
AVSOURCE_API bool HMRAVSourceIsInitialized();

} // namespace HMRAVSource

// ============================================================================
// Include sub-headers
// ============================================================================
#include "AVSource.h"
#include "MFSource.h"
#include "DShowSource.h"
#include "EncodeProfile.h"
#include "StreamSink.h"
#include "TranscodeManager.h"
#include "VideoCapture.h"
#include "AudioCapture.h"
#include "AVCaptureSession.h"
#include "CachedWFSection.h"

#pragma warning(pop)

#endif // HMRAVSOURCE_H
