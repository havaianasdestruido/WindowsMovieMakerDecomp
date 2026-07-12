/*
 * StreamSinkHelper.h
 *
 * StreamSinkHelper - Utility functions for stream sink setup, format
 * negotiation, and media type configuration. Provides helper methods
 * for creating input/output media types and negotiating compatible
 * formats between source and sink.
 *
 * RTTI: ?AVStreamSinkHelper@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef STREAMSINKHELPER_H
#define STREAMSINKHELPER_H

#include "HMRAVSource.h"
#include "EncodeProfile.h"

namespace HMRAVSource
{

// ============================================================================
// StreamSinkHelper
// ============================================================================
// Static utility class providing helper functions for stream sink setup.
// Creates media types for video and audio streams, negotiates formats
// between source readers and sink writers, and provides codec detection.
//
class AVSOURCE_API StreamSinkHelper
{
public:
    StreamSinkHelper();
    ~StreamSinkHelper();

    // Video media type creation
    static HRESULT CreateVideoMediaType(
        const EncodeVideoParams& params,
        IMFMediaType** ppMediaType);

    static HRESULT CreateVideoInputType(
        UINT uWidth,
        UINT uHeight,
        double dblFrameRate,
        IMFMediaType** ppMediaType);

    static HRESULT CreateVideoOutputType(
        const EncodeVideoParams& params,
        IMFMediaType** ppMediaType);

    // Audio media type creation
    static HRESULT CreateAudioMediaType(
        const EncodeAudioParams& params,
        IMFMediaType** ppMediaType);

    static HRESULT CreateAudioInputType(
        DWORD dwSampleRate,
        DWORD dwChannels,
        DWORD dwBitsPerSample,
        IMFMediaType** ppMediaType);

    static HRESULT CreateAudioOutputType(
        const EncodeAudioParams& params,
        IMFMediaType** ppMediaType);

    // Format negotiation
    static HRESULT NegotiateMediaType(
        IMFSourceReader* pReader,
        DWORD dwStreamIndex,
        IMFMediaType* pDesiredOutputType,
        IMFMediaType** ppAcceptedInputType);

    static HRESULT FindBestOutputType(
        IMFMediaType* pInputType,
        REFGUID guidMajorType,
        IMFMediaType** ppBestOutputType);

    // Codec helpers
    static HRESULT GetVideoSubtypeFromCodec(VideoCodec codec, GUID* pSubtype);
    static HRESULT GetAudioSubtypeFromCodec(AudioCodec codec, GUID* pSubtype);
    static VideoCodec GetCodecFromVideoSubtype(REFGUID guidSubtype);
    static AudioCodec GetCodecFromAudioSubtype(REFGUID guidSubtype);

    // Stream index helpers
    static bool IsVideoStream(IMFMediaType* pType);
    static bool IsAudioStream(IMFMediaType* pType);
    static HRESULT GetStreamType(IMFSourceReader* pReader, DWORD dwStreamIndex, AVMediaType* pMediaType);

    // Frame rate helpers
    static HRESULT SetFrameRate(IMFMediaType* pType, UINT32 numerator, UINT32 denominator);
    static HRESULT GetFrameRate(IMFMediaType* pType, UINT32* pNumerator, UINT32* pDenominator);
    static HRESULT SetFrameRateAsDouble(IMFMediaType* pType, double dblFps);
    static HRESULT GetFrameRateAsDouble(IMFMediaType* pType, double* pFps);

    // Resolution helpers
    static HRESULT SetResolution(IMFMediaType* pType, UINT uWidth, UINT uHeight);
    static HRESULT GetResolution(IMFMediaType* pType, UINT* puWidth, UINT* puHeight);

private:
    static HRESULT CreateBaseVideoType(IMFMediaType** ppType);
    static HRESULT CreateBaseAudioType(IMFMediaType** ppType);
};

} // namespace HMRAVSource

#endif // STREAMSINKHELPER_H
