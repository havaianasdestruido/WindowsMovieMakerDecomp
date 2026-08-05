/*
 * WLXMP4Parser.h
 *
 * Public header for WLXMP4Parser.dll.
 * Provides MP4/ISOBMFF (ISO Base Media File Format) container parsing
 * for Windows Live Movie Maker 2012.
 *
 * Parses MP4 box structure (ftyp, moov, mdat, mvhd, trak, stbl, etc.)
 * for fast-start optimization, chapter extraction, and thumbnail retrieval
 * without needing to fully decode the file.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLXMP4PARSER_H
#define WLXMP4PARSER_H

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

struct IGraphBuilder;
struct IBaseFilter;

#ifdef WLXMP4PARSER_EXPORTS
    #define WLXMP4P_API __declspec(dllexport)
#else
    #define WLXMP4P_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace MP4Parser
{
    class MP4File;
    class BoxReader;
    class MoovBox;
    class TrakBox;
    class StblBox;

    struct MP4TrackInfo
    {
        UINT32      uTrackId;
        UINT32      uCodecFourCC;
        UINT32      uWidth;
        UINT32      uHeight;
        UINT32      uSampleCount;
        UINT32      uDuration;          // in timescale units
        UINT32      uTimeScale;
        UINT32      uBitrate;
        WCHAR       wszCodecName[64];
        BOOL        bIsVideo;
        BOOL        bIsAudio;

        MP4TrackInfo()
            : uTrackId(0)
            , uCodecFourCC(0)
            , uWidth(0)
            , uHeight(0)
            , uSampleCount(0)
            , uDuration(0)
            , uTimeScale(1)
            , uBitrate(0)
            , bIsVideo(FALSE)
            , bIsAudio(FALSE)
        {
            ZeroMemory(wszCodecName, sizeof(wszCodecName));
        }
    };

    struct MP4ChapterInfo
    {
        LONGLONG    llStartTime;        // in 100ns units
        LONGLONG    llEndTime;
        WCHAR       wszTitle[256];

        MP4ChapterInfo()
            : llStartTime(0)
            , llEndTime(0)
        {
            ZeroMemory(wszTitle, sizeof(wszTitle));
        }
    };
}

// ============================================================================
// MP4 box types (common ISOBMFF box identifiers, big-endian fourcc)
// ============================================================================
#define MP4_BOX_FTYP    0x66747970  // 'ftyp'
#define MP4_BOX_MOOV    0x6D6F6F76  // 'moov'
#define MP4_BOX_MVHD    0x6D766864  // 'mvhd'
#define MP4_BOX_TRAK    0x7472616B  // 'trak'
#define MP4_BOX_MDIA    0x6D646961  // 'mdia'
#define MP4_BOX_STBL    0x7374626C  // 'stbl'
#define MP4_BOX_MDAT    0x6D646174  // 'mdat'
#define MP4_BOX_EDTS    0x65647473  // 'edts'
#define MP4_BOX_ELST    0x656C7374  // 'elst'
#define MP4_BOX_UDTA    0x75647461  // 'udta'
#define MP4_BOX_CHAP    0x63686170  // 'chap'
#define MP4_BOX_ILST    0x696C7374  // 'ilst'

// ============================================================================
// Exported functions -- DirectShow filter factory functions + COM exports
// (DllCanUnloadNow, DllGetClassObject, DllRegisterServer, DllUnregisterServer
//  are defined in WLXMP4Parser.cpp but not declared here to avoid
//  conflict with combaseapi.h declarations)
// ============================================================================
extern "C"
{
    WLXMP4P_API HRESULT __stdcall AddMP4SourceFilter(LPCWSTR pszFilePath, IGraphBuilder* pGraph, IBaseFilter** ppFilter);
    WLXMP4P_API HRESULT __stdcall BuildMP4FilterGraph(LPCWSTR pszFilePath, IGraphBuilder** ppGraph);
    WLXMP4P_API HRESULT __stdcall BuildMP4PlayBack(LPCWSTR pszFilePath, IGraphBuilder* pGraph);
    WLXMP4P_API BOOL    __stdcall IsMP4FilePlayable(LPCWSTR pszFilePath);
}

#endif // WLXMP4PARSER_H
