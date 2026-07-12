/*
 * ImageThumbnail.h
 *
 * ImageThumbnail - Thumbnail generation for images using Windows Imaging
 * Component (WIC). Creates thumbnail bitmaps from image files for use
 * in the media browser and storyboard.
 *
 * RTTI: ?AVImageThumbnail@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef IMAGETHUMBNAIL_H
#define IMAGETHUMBNAIL_H

#include "HMRAVSource.h"
#include <wincodec.h>

namespace HMRAVSource
{

// ============================================================================
// ImageThumbnailDesc
// ============================================================================
struct ImageThumbnailDesc
{
    UINT    uMaxWidth;
    UINT    uMaxHeight;
    bool    fPreserveAspectRatio;
    bool    fUseThumbnail;

    ImageThumbnailDesc()
        : uMaxWidth(160)
        , uMaxHeight(120)
        , fPreserveAspectRatio(true)
        , fUseThumbnail(true)
    {
    }
};

// ============================================================================
// ImageThumbnail
// ============================================================================
// Generates thumbnail bitmaps from image files using WIC. Supports
// reading embedded thumbnails from EXIF/Metadata, or creating new
// thumbnails by scaling down the source image.
//
class AVSOURCE_API ImageThumbnail
{
public:
    ImageThumbnail();
    ~ImageThumbnail();

    // Lifecycle
    HRESULT Initialize();
    HRESULT Shutdown();

    // Thumbnail generation
    HRESULT GenerateThumbnail(LPCWSTR pszImagePath, const ImageThumbnailDesc& desc);
    HRESULT GenerateThumbnailFromStream(IStream* pStream, const ImageThumbnailDesc& desc);

    // Embedded thumbnail
    HRESULT GetEmbeddedThumbnail(LPCWSTR pszImagePath);
    bool HasEmbeddedThumbnail() const throw();

    // Output access
    HRESULT GetBitmap(HBITMAP* phBitmap, HPALETTE* phPalette);
    HRESULT GetBitmapData(BITMAPINFO* pBitmapInfo, BYTE** ppData);
    HRESULT SaveThumbnail(LPCWSTR pszOutputPath, UINT uQuality = 90);

    // Size query
    UINT GetThumbnailWidth() const throw();
    UINT GetThumbnailHeight() const throw();

    // Format detection
    static bool IsSupportedImageFormat(LPCWSTR pszFilePath);
    static bool IsSupportedExtension(LPCWSTR pszExtension);

    // Source image info
    HRESULT GetSourceImageSize(LPCWSTR pszImagePath, UINT* puWidth, UINT* puHeight);
    HRESULT GetSourceImageFormat(LPCWSTR pszImagePath, GUID* pContainerFormat);

    // Multi-frame support
    DWORD GetFrameCount() const throw();
    HRESULT SelectFrame(DWORD dwFrameIndex);

private:
    CComPtr<IWICImagingFactory>     m_spWicFactory;
    CComPtr<IWICBitmapDecoder>      m_spDecoder;
    CComPtr<IWICBitmapSource>       m_spBitmapSource;
    CComPtr<IWICBitmap>             m_spThumbnailBitmap;

    DWORD                           m_dwFrameCount;
    DWORD                           m_dwSelectedFrame;
    UINT                            m_uThumbWidth;
    UINT                            m_uThumbHeight;
    bool                            m_fHasEmbeddedThumbnail;
    bool                            m_fInitialized;

    HRESULT CreateWicFactory();
    HRESULT OpenImageFile(LPCWSTR pszImagePath);
    HRESULT TryGetEmbeddedThumbnail();
    HRESULT ScaleImage(UINT uMaxWidth, UINT uMaxHeight, bool fPreserveAspect);
    HRESULT ConvertToHBITMAP(HBITMAP* phBitmap, HPALETTE* phPalette);
    static bool IsWicContainerFormat(const GUID& guidContainer);
};

} // namespace HMRAVSource

#endif // IMAGETHUMBNAIL_H
