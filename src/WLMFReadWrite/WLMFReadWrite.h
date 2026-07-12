/*
 * WLMFReadWrite.h
 *
 * Public header for WLMFReadWrite.dll.
 * Provides Media Foundation read/write capabilities for
 * Windows Live Movie Maker 2012.
 *
 * Wraps MFSourceReader and MFSinkWriter with convenience APIs
 * for video file enumeration, thumbnail extraction, and format conversion.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLMFREADWRITE_H
#define WLMFREADWRITE_H

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
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#ifdef WLMFREADWRITE_EXPORTS
    #define WLMFRW_API __declspec(dllexport)
#else
    #define WLMFRW_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace MFReadWrite
{
    class ReaderManager;
    class WriterManager;
    class ThumbnailExtractor;
    struct MediaProperties;
}

// ============================================================================
// Media properties structure
// ============================================================================
struct WLMediaProperties
{
    WCHAR       wszFileType[32];
    UINT32      uVideoWidth;
    UINT32      uVideoHeight;
    UINT32      uFrameRateNumerator;
    UINT32      uFrameRateDenominator;
    UINT32      uVideoBitrate;
    UINT32      uAudioBitrate;
    UINT32      uAudioSampleRate;
    UINT32      uAudioChannels;
    LONGLONG    llDuration;         // 100ns units
    GUID        guidVideoSubtype;
    GUID        guidAudioSubtype;
    BOOL        bHasVideo;
    BOOL        bHasAudio;

    WLMediaProperties()
        : uVideoWidth(0)
        , uVideoHeight(0)
        , uFrameRateNumerator(0)
        , uFrameRateDenominator(1)
        , uVideoBitrate(0)
        , uAudioBitrate(0)
        , uAudioSampleRate(0)
        , uAudioChannels(0)
        , llDuration(0)
        , bHasVideo(FALSE)
        , bHasAudio(FALSE)
    {
        ZeroMemory(wszFileType, sizeof(wszFileType));
        ZeroMemory(&guidVideoSubtype, sizeof(GUID));
        ZeroMemory(&guidAudioSubtype, sizeof(GUID));
    }
};

// ============================================================================
// Exported functions (7 exports)
// ============================================================================
extern "C"
{
    // Creates a source reader for the specified file.
    WLMFRW_API HANDLE __stdcall MFReader_Open(LPCWSTR pszFilePath);

    // Closes the source reader.
    WLMFRW_API void __stdcall MFReader_Close(HANDLE hReader);

    // Queries media properties.
    WLMFRW_API HRESULT __stdcall MFReader_GetProperties(HANDLE hReader, WLMediaProperties* pProps);

    // Reads a decoded frame at the specified time.
    WLMFRW_API HRESULT __stdcall MFReader_ReadFrame(HANDLE hReader, LONGLONG llTimeMs,
        BYTE* pBuffer, UINT32 cbBuffer, UINT32* pcbRead);

    // Creates a sink writer for the specified output file.
    WLMFRW_API HANDLE __stdcall MFWriter_Create(LPCWSTR pszOutputPath, const WLMediaProperties* pProps);

    // Writes an encoded frame.
    WLMFRW_API HRESULT __stdcall MFWriter_WriteFrame(HANDLE hWriter, const BYTE* pData, UINT32 cbData, LONGLONG llTimeMs);

    // Finalizes the output file.
    WLMFRW_API HRESULT __stdcall MFWriter_Finalize(HANDLE hWriter);
}

#endif // WLMFREADWRITE_H
