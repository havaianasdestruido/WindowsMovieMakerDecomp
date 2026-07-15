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

#include "HMRAVSourceTypes.h"

#include <shlwapi.h>
#include <atlsync.h>
#include <mfmediaengine.h>
#include <dshow.h>

#include "WLXPhotoBase.h"
#include "../MovieMakerCore.h"

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4505) // unreferenced local function has been removed
#pragma warning(disable: 4512) // assignment operator could not be generated

// ============================================================================
// HMRAVSource namespace
// ============================================================================
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
// HMRAVSource initialization
// ============================================================================

AVSOURCE_API HRESULT HMRAVSourceInitialize();
AVSOURCE_API void HMRAVSourceShutdown();
AVSOURCE_API bool HMRAVSourceIsInitialized();

} // namespace HMRAVSource

// ============================================================================
// Include sub-headers (each is self-contained)
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
