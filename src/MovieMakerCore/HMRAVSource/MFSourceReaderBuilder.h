/*
 * MFSourceReaderBuilder.h
 *
 * MFSourceReaderBuilder - Base source reader builder with codec detection.
 * Provides common functionality for creating and configuring MF source
 * readers, including codec enumeration and media type selection.
 *
 * RTTI: ?AVMFSourceReaderBuilder@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef MFSOURCEREADERBUILDER_H
#define MFSOURCEREADERBUILDER_H

#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// SourceReaderBuilderConfig
// ============================================================================
struct SourceReaderBuilderConfig
{
    DWORD   dwVideoStreamIndex;
    DWORD   dwAudioStreamIndex;
    UINT    uPreferredVideoWidth;
    UINT    uPreferredVideoHeight;
    DWORD   dwPreferredAudioSampleRate;
    DWORD   dwPreferredAudioChannels;
    bool    fEnableAsyncReading;
    bool    fEnableHardwareAcceleration;
    DWORD   dwAsyncCallbackQueue;

    SourceReaderBuilderConfig()
        : dwVideoStreamIndex(0)
        , dwAudioStreamIndex(1)
        , uPreferredVideoWidth(0)
        , uPreferredVideoHeight(0)
        , dwPreferredAudioSampleRate(0)
        , dwPreferredAudioChannels(0)
        , fEnableAsyncReading(false)
        , fEnableHardwareAcceleration(true)
        , dwAsyncCallbackQueue(0)
    {
    }
};

// ============================================================================
// CodecInfo
// ============================================================================
struct CodecInfo
{
    GUID    guidSubtype;
    ATL::CString strFriendlyName;
    bool    fIsHardwareCodec;
    DWORD   dwInputStreams;
    DWORD   dwOutputStreams;
};

// ============================================================================
// MFSourceReaderBuilder
// ============================================================================
// Base class for MF source reader construction and configuration.
// Provides codec detection, media type enumeration, and stream
// selection utilities used by both native and DShow-based builders.
//
class AVSOURCE_API MFSourceReaderBuilder
{
public:
    MFSourceReaderBuilder();
    virtual ~MFSourceReaderBuilder();

    // Lifecycle
    HRESULT Initialize();
    HRESULT Shutdown();

    // Source reader creation
    virtual HRESULT CreateSourceReader(LPCWSTR pszFilePath, IMFSourceReader** ppReader);
    HRESULT CreateSourceReaderFromFile(LPCWSTR pszFilePath, const SourceReaderBuilderConfig& config, IMFSourceReader** ppReader);

    // Codec detection
    static HRESULT EnumerateVideoCodecs(ATL::CAtlArray<CodecInfo>& codecs);
    static HRESULT EnumerateAudioCodecs(ATL::CAtlArray<CodecInfo>& codecs);
    static bool IsCodecAvailable(REFGUID guidSubtype);
    static HRESULT GetCodecFriendlyName(REFGUID guidSubtype, ATL::CString* pstrName);

    // Stream enumeration
    static HRESULT EnumerateStreams(IMFSourceReader* pReader, DWORD* pdwVideoStream, DWORD* pdwAudioStream, DWORD* pdwStreamCount);
    static HRESULT GetStreamInfo(IMFSourceReader* pReader, DWORD dwStreamIndex, AVMediaType* pMediaType, IMFMediaType** ppNativeType);

    // Format negotiation
    HRESULT NegotiateVideoFormat(IMFSourceReader* pReader, DWORD dwStreamIndex, UINT uWidth, UINT uHeight);
    HRESULT NegotiateAudioFormat(IMFSourceReader* pReader, DWORD dwStreamIndex, DWORD dwSampleRate, DWORD dwChannels);

    // Configuration
    void SetConfig(const SourceReaderBuilderConfig& config);
    const SourceReaderBuilderConfig& GetConfig() const;

    // Codec capability queries
    static bool IsH264Supported();
    static bool IsWMVSupported();
    static bool IsAACSupported();
    static bool IsWMASupported();
    static bool IsMP3Supported();
    static bool IsHEVCSupported();

protected:
    SourceReaderBuilderConfig   m_config;
    bool                        m_fInitialized;

    HRESULT ConfigureSourceReader(IMFSourceReader* pReader);
    HRESULT SelectBestVideoType(IMFSourceReader* pReader, DWORD dwStreamIndex);
    HRESULT SelectBestAudioType(IMFSourceReader* pReader, DWORD dwStreamIndex);
    static HRESULT CreateSourceReaderAttributes(IMFAttributes** ppAttributes);
};

} // namespace HMRAVSource

#endif // MFSOURCEREADERBUILDER_H
