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
// MP4 box types (common ISOBMFF box identifiers)
// ============================================================================
#define MP4_BOX_FTYP    0x74797066  // 'ftyp'
#define MP4_BOX_MOOV    0x766F6F6D  // 'moov'
#define MP4_BOX_MVHD    0x6468766D  // 'mvhd'
#define MP4_BOX_TRAK    0x6B617274  // 'trak'
#define MP4_BOX_MDIA    0x6169646D  // 'mdia'
#define MP4_BOX_STBL    0x6C627473  // 'stbl'
#define MP4_BOX_MDAT    0x7461646D  // 'mdat'
#define MP4_BOX_EDTS    0x73746465  // 'edts'
#define MP4_BOX_ELST    0x74736C65  // 'elst'
#define MP4_BOX_UDTA    0x61746475  // 'udta'
#define MP4_BOX_CHAP    0x70616863  // 'chap'
#define MP4_BOX_ILST    0x74736C69  // 'ilst'

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
