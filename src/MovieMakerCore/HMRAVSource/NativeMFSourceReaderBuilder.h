/*
 * NativeMFSourceReaderBuilder.h
 *
 * NativeMFSourceReaderBuilder - Creates native Media Foundation source
 * readers for standard file types using MFCreateSourceReaderFromURL.
 * Handles standard MF codec formats without DirectShow bridging.
 *
 * RTTI: ?AVNativeMFSourceReaderBuilder@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef NATIVEMFSOURCEREADERBUILDER_H
#define NATIVEMFSOURCEREADERBUILDER_H

#include "HMRAVSource.h"
#include "MFSourceReaderBuilder.h"

namespace HMRAVSource
{

// ============================================================================
// NativeMFSourceReaderBuilder
// ============================================================================
// Creates a native MF source reader for files that are supported by the
// built-in Media Foundation codecs (H.264, AAC, MP3, WMV, etc.). This is
// the primary path for most standard media files.
//
class AVSOURCE_API NativeMFSourceReaderBuilder
{
public:
    NativeMFSourceReaderBuilder();
    ~NativeMFSourceReaderBuilder();

    // Lifecycle
    HRESULT Initialize();
    HRESULT Shutdown();

    // Source reader creation
    HRESULT CreateSourceReader(LPCWSTR pszFilePath, IMFSourceReader** ppReader);
    HRESULT CreateSourceReaderWithAttributes(
        LPCWSTR pszFilePath,
        IMFAttributes* pAttributes,
        IMFSourceReader** ppReader);

    // Source reader from byte stream
    HRESULT CreateSourceReaderFromByteStream(
        IMFByteStream* pByteStream,
        LPCWSTR pszUrl,
        IMFSourceReader** ppReader);

    // Format support query
    static bool IsNativeFormatSupported(LPCWSTR pszFilePath);
    static bool CanOpenWithNativeReader(LPCWSTR pszFilePath);

    // Media type configuration
    HRESULT SetVideoOutputType(IMFSourceReader* pReader, IMFMediaType* pType);
    HRESULT SetAudioOutputType(IMFSourceReader* pReader, IMFMediaType* pType);
    HRESULT SetDefaultVideoOutputType(IMFSourceReader* pReader, UINT uWidth, UINT uHeight);
    HRESULT SetDefaultAudioOutputType(IMFSourceReader* pReader, DWORD dwSampleRate, DWORD dwChannels);

    // Source information
    static HRESULT GetSourceDuration(IMFSourceReader* pReader, LONGLONG* pllDuration);
    static HRESULT GetStreamCount(IMFSourceReader* pReader, DWORD* pdwCount);
    static HRESULT GetNativeMediaType(IMFSourceReader* pReader, DWORD dwStreamIndex, IMFMediaType** ppType);

private:
    bool    m_fInitialized;

    static bool IsStandardExtension(LPCWSTR pszExtension);
    static HRESULT CreateDefaultAttributes(IMFAttributes** ppAttributes);
};

} // namespace HMRAVSource

#endif // NATIVEMFSOURCEREADERBUILDER_H
