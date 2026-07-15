/*
 * HMRAVSourceTypes.h
 *
 * Enums and export macros for the HMRAVSource namespace.
 * Extracted to break circular include dependencies between
 * HMRAVSource.h and its sub-headers (AVSource.h, MFSource.h, etc.).
 */

#pragma once

#ifndef HMRAVSOURCETYPES_H
#define HMRAVSOURCETYPES_H

#include <windows.h>
#include <objbase.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mfobjects.h>

// ============================================================================
// Export / import macros
// ============================================================================
#ifdef MOVIECORE_EXPORTS
    #define AVSOURCE_API __declspec(dllexport)
#else
    #define AVSOURCE_API __declspec(dllimport)
#endif

namespace HMRAVSource
{

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
    EncodeProfileWLP            = 2,
    EncodeProfileWLHD           = 3,
    EncodeProfileWLX            = 4,
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

// Transcode states
enum TranscodeState
{
    TranscodeStateIdle      = 0,
    TranscodeStateOpening   = 1,
    TranscodeStateReady     = 2,
    TranscodeStateRunning   = 3,
    TranscodeStatePaused    = 4,
    TranscodeStateDone      = 5,
    TranscodeStateError     = 6,
    TranscodeStateCancelled = 7
};

} // namespace HMRAVSource

#endif // HMRAVSOURCETYPES_H
