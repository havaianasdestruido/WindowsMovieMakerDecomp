// ImageThumbnail.cpp - WIC thumbnail generation implementation

#include "pch.h"
#include "ImageThumbnail.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

ImageThumbnail::ImageThumbnail()
    : m_dwFrameCount(0)
    , m_dwSelectedFrame(0)
    , m_uThumbWidth(0)
    , m_uThumbHeight(0)
    , m_fHasEmbeddedThumbnail(false)
    , m_fInitialized(false)
{
}

ImageThumbnail::~ImageThumbnail()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT ImageThumbnail::Initialize()
{
    HRESULT hr = CreateWicFactory();
    if (FAILED(hr))
        return hr;

    m_fInitialized = true;
    return S_OK;
}

HRESULT ImageThumbnail::Shutdown()
{
    m_spThumbnailBitmap = nullptr;
    m_spBitmapSource = nullptr;
    m_spDecoder = nullptr;
    m_spWicFactory = nullptr;
    m_fInitialized = false;
    m_fHasEmbeddedThumbnail = false;
    m_dwFrameCount = 0;
    m_uThumbWidth = 0;
    m_uThumbHeight = 0;

    return S_OK;
}

// ============================================================================
// Thumbnail generation
// ============================================================================

HRESULT ImageThumbnail::GenerateThumbnail(LPCWSTR pszImagePath, const ImageThumbnailDesc& desc)
{
    if (!pszImagePath)
        return E_POINTER;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    m_spThumbnailBitmap = nullptr;
    m_spBitmapSource = nullptr;
    m_fHasEmbeddedThumbnail = false;

    HRESULT hr = OpenImageFile(pszImagePath);
    if (FAILED(hr))
        return hr;

    if (desc.fUseThumbnail)
    {
        hr = TryGetEmbeddedThumbnail();
        if (SUCCEEDED(hr) && m_fHasEmbeddedThumbnail)
        {
            hr = ScaleImage(desc.uMaxWidth, desc.uMaxHeight, desc.fPreserveAspectRatio);
            return hr;
        }
    }

    // Use the main image and scale it
    hr = ScaleImage(desc.uMaxWidth, desc.uMaxHeight, desc.fPreserveAspectRatio);
    return hr;
}

HRESULT ImageThumbnail::GenerateThumbnailFromStream(IStream* pStream, const ImageThumbnailDesc& desc)
{
    if (!pStream)
        return E_POINTER;

    if (!m_fInitialized || !m_spWicFactory)
        return E_UNEXPECTED;

    m_spThumbnailBitmap = nullptr;
    m_spBitmapSource = nullptr;
    m_fHasEmbeddedThumbnail = false;

    HRESULT hr = m_spWicFactory->CreateDecoderFromStream(
        pStream,
        nullptr,
        WICDecodeMetadataCacheOnLoad,
        &m_spDecoder);

    if (FAILED(hr))
        return hr;

    CComPtr<IWICBitmapFrameDecode> spFrame;
    hr = m_spDecoder->GetFrame(0, &spFrame);
    if (FAILED(hr))
        return hr;
    m_spBitmapSource = spFrame;

    UINT uFrameCount = 0;
    m_spDecoder->GetFrameCount(&uFrameCount);
    m_dwFrameCount = uFrameCount;

    return ScaleImage(desc.uMaxWidth, desc.uMaxHeight, desc.fPreserveAspectRatio);
}

// ============================================================================
// Embedded thumbnail
// ============================================================================

HRESULT ImageThumbnail::GetEmbeddedThumbnail(LPCWSTR pszImagePath)
{
    if (!pszImagePath)
        return E_POINTER;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    HRESULT hr = OpenImageFile(pszImagePath);
    if (FAILED(hr))
        return hr;

    return TryGetEmbeddedThumbnail();
}

bool ImageThumbnail::HasEmbeddedThumbnail() const throw()
{
    return m_fHasEmbeddedThumbnail;
}

// ============================================================================
// Output access
// ============================================================================

HRESULT ImageThumbnail::GetBitmap(HBITMAP* phBitmap, HPALETTE* phPalette)
{
    if (!phBitmap)
        return E_POINTER;

    *phBitmap = nullptr;
    if (phPalette) *phPalette = nullptr;

    if (!m_spThumbnailBitmap)
        return E_UNEXPECTED;

    UINT uWidth = 0, uHeight = 0;
    HRESULT hr = m_spThumbnailBitmap->GetSize(&uWidth, &uHeight);
    if (FAILED(hr))
        return hr;

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(uWidth);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(uHeight);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    BYTE* pPixels = nullptr;
    HBITMAP hBitmap = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, (void**)&pPixels, nullptr, 0);
    if (!hBitmap)
        return HRESULT_FROM_WIN32(GetLastError());

    WICPixelFormatGUID guidFormat;
    hr = m_spThumbnailBitmap->GetPixelFormat(&guidFormat);
    if (FAILED(hr))
    {
        DeleteObject(hBitmap);
        return hr;
    }

    if (guidFormat != GUID_WICPixelFormat32bppBGRA)
    {
        CComPtr<IWICFormatConverter> spConverter;
        hr = m_spWicFactory->CreateFormatConverter(&spConverter);
        if (FAILED(hr))
        {
            DeleteObject(hBitmap);
            return hr;
        }

        hr = spConverter->Initialize(
            m_spThumbnailBitmap,
            GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
        {
            DeleteObject(hBitmap);
            return hr;
        }

        hr = spConverter->CopyPixels(nullptr, uWidth * 4, uWidth * uHeight * 4, pPixels);
    }
    else
    {
        hr = m_spThumbnailBitmap->CopyPixels(nullptr, uWidth * 4, uWidth * uHeight * 4, pPixels);
    }

    if (FAILED(hr))
    {
        DeleteObject(hBitmap);
        return hr;
    }

    *phBitmap = hBitmap;
    return S_OK;
}

HRESULT ImageThumbnail::GetBitmapData(BITMAPINFO* pBitmapInfo, BYTE** ppData)
{
    if (!pBitmapInfo || !ppData)
        return E_POINTER;

    *ppData = nullptr;
    ZeroMemory(pBitmapInfo, sizeof(BITMAPINFO));

    if (!m_spThumbnailBitmap)
        return E_UNEXPECTED;

    UINT uWidth = 0, uHeight = 0;
    HRESULT hr = m_spThumbnailBitmap->GetSize(&uWidth, &uHeight);
    if (FAILED(hr))
        return hr;

    WICPixelFormatGUID pixelFormat;
    hr = m_spThumbnailBitmap->GetPixelFormat(&pixelFormat);
    if (FAILED(hr))
        return hr;

    pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pBitmapInfo->bmiHeader.biWidth = static_cast<LONG>(uWidth);
    pBitmapInfo->bmiHeader.biHeight = -(static_cast<LONG>(uHeight));
    pBitmapInfo->bmiHeader.biPlanes = 1;
    pBitmapInfo->bmiHeader.biBitCount = 32;
    pBitmapInfo->bmiHeader.biCompression = BI_RGB;

    DWORD cbStride = uWidth * 4;
    DWORD cbSize = cbStride * uHeight;
    BYTE* pPixels = new (std::nothrow) BYTE[cbSize];
    if (!pPixels)
        return E_OUTOFMEMORY;

    if (pixelFormat != GUID_WICPixelFormat32bppBGRA)
    {
        CComPtr<IWICFormatConverter> spConverter;
        hr = m_spWicFactory->CreateFormatConverter(&spConverter);
        if (FAILED(hr))
        {
            delete[] pPixels;
            return hr;
        }

        hr = spConverter->Initialize(
            m_spThumbnailBitmap,
            GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom);
        if (SUCCEEDED(hr))
            hr = spConverter->CopyPixels(nullptr, cbStride, cbSize, pPixels);
    }
    else
    {
        hr = m_spThumbnailBitmap->CopyPixels(nullptr, cbStride, cbSize, pPixels);
    }

    if (FAILED(hr))
    {
        delete[] pPixels;
        return hr;
    }

    *ppData = pPixels;
    return S_OK;
}

HRESULT ImageThumbnail::SaveThumbnail(LPCWSTR pszOutputPath, UINT uQuality)
{
    if (!pszOutputPath)
        return E_POINTER;

    if (!m_spThumbnailBitmap || !m_spWicFactory)
        return E_UNEXPECTED;

    CComPtr<IWICBitmapEncoder> spEncoder;
    HRESULT hr = m_spWicFactory->CreateEncoder(
        GUID_ContainerFormatPng,
        nullptr,
        &spEncoder);

    if (FAILED(hr))
        return hr;

    CComPtr<IStream> spStream;
    hr = SHCreateStreamOnFileW(pszOutputPath, STGM_CREATE | STGM_WRITE, &spStream);
    if (FAILED(hr))
        return hr;

    hr = spEncoder->Initialize(spStream, WICBitmapEncoderNoCache);
    if (FAILED(hr))
        return hr;

    CComPtr<IWICBitmapFrameEncode> spFrameEncode;
    CComPtr<IPropertyBag2> spPropertyBag;
    hr = spEncoder->CreateNewFrame(&spFrameEncode, &spPropertyBag);
    if (FAILED(hr))
        return hr;

    if (spPropertyBag)
    {
        PROPBAG2 var = {};
        var.pstrName = const_cast<LPOLESTR>(L"ImageQuality");
        var.vt = VT_R4;
        VARIANT varValue;
        VariantInit(&varValue);
        varValue.vt = VT_R4;
        varValue.fltVal = static_cast<float>(uQuality) / 100.0f;
        spPropertyBag->Write(1, &var, &varValue);
        VariantClear(&varValue);
    }

    hr = spFrameEncode->Initialize(spPropertyBag);
    if (SUCCEEDED(hr))
        hr = spFrameEncode->WriteSource(m_spThumbnailBitmap, nullptr);
    if (SUCCEEDED(hr))
        hr = spFrameEncode->Commit();
    if (SUCCEEDED(hr))
        hr = spEncoder->Commit();

    return hr;
}

// ============================================================================
// Size query
// ============================================================================

UINT ImageThumbnail::GetThumbnailWidth() const throw()
{
    return m_uThumbWidth;
}

UINT ImageThumbnail::GetThumbnailHeight() const throw()
{
    return m_uThumbHeight;
}

// ============================================================================
// Format detection
// ============================================================================

bool ImageThumbnail::IsSupportedImageFormat(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return false;

    LPCWSTR pszExtension = PathFindExtensionW(pszFilePath);
    return IsSupportedExtension(pszExtension);
}

bool ImageThumbnail::IsSupportedExtension(LPCWSTR pszExtension)
{
    if (!pszExtension)
        return false;

    if (_wcsicmp(pszExtension, L".jpg") == 0 ||
        _wcsicmp(pszExtension, L".jpeg") == 0 ||
        _wcsicmp(pszExtension, L".png") == 0 ||
        _wcsicmp(pszExtension, L".bmp") == 0 ||
        _wcsicmp(pszExtension, L".gif") == 0 ||
        _wcsicmp(pszExtension, L".tiff") == 0 ||
        _wcsicmp(pszExtension, L".tif") == 0 ||
        _wcsicmp(pszExtension, L".ico") == 0 ||
        _wcsicmp(pszExtension, L".wdp") == 0 ||
        _wcsicmp(pszExtension, L".dds") == 0)
    {
        return true;
    }

    return false;
}

// ============================================================================
// Source image info
// ============================================================================

HRESULT ImageThumbnail::GetSourceImageSize(LPCWSTR pszImagePath, UINT* puWidth, UINT* puHeight)
{
    if (!pszImagePath || !puWidth || !puHeight)
        return E_POINTER;

    *puWidth = 0;
    *puHeight = 0;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    CComPtr<IWICBitmapDecoder> spDecoder;
    HRESULT hr = m_spWicFactory->CreateDecoderFromFilename(
        pszImagePath,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &spDecoder);

    if (FAILED(hr))
        return hr;

    CComPtr<IWICBitmapFrameDecode> spFrame;
    hr = spDecoder->GetFrame(0, &spFrame);
    if (FAILED(hr))
        return hr;

    return spFrame->GetSize(puWidth, puHeight);
}

HRESULT ImageThumbnail::GetSourceImageFormat(LPCWSTR pszImagePath, GUID* pContainerFormat)
{
    if (!pszImagePath || !pContainerFormat)
        return E_POINTER;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    CComPtr<IWICBitmapDecoder> spDecoder;
    HRESULT hr = m_spWicFactory->CreateDecoderFromFilename(
        pszImagePath,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &spDecoder);

    if (FAILED(hr))
        return hr;

    return spDecoder->GetContainerFormat(pContainerFormat);
}

// ============================================================================
// Multi-frame support
// ============================================================================

DWORD ImageThumbnail::GetFrameCount() const throw()
{
    return m_dwFrameCount;
}

HRESULT ImageThumbnail::SelectFrame(DWORD dwFrameIndex)
{
    if (!m_fInitialized)
        return E_UNEXPECTED;

    if (!m_spDecoder)
        return E_UNEXPECTED;

    if (dwFrameIndex >= m_dwFrameCount)
        return E_INVALIDARG;

    CComPtr<IWICBitmapFrameDecode> spFrame;
    HRESULT hr = m_spDecoder->GetFrame(dwFrameIndex, &spFrame);
    if (FAILED(hr))
        return hr;

    m_spBitmapSource = spFrame;
    m_dwSelectedFrame = dwFrameIndex;
    m_spThumbnailBitmap = nullptr;

    return S_OK;
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT ImageThumbnail::CreateWicFactory()
{
    return CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_spWicFactory));
}

HRESULT ImageThumbnail::OpenImageFile(LPCWSTR pszImagePath)
{
    if (!pszImagePath)
        return E_POINTER;

    m_spDecoder = nullptr;
    m_spBitmapSource = nullptr;
    m_dwFrameCount = 0;

    HRESULT hr = m_spWicFactory->CreateDecoderFromFilename(
        pszImagePath,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &m_spDecoder);

    if (FAILED(hr))
        return hr;

    CComPtr<IWICBitmapFrameDecode> spFrame2;
    hr = m_spDecoder->GetFrame(0, &spFrame2);
    if (FAILED(hr))
        return hr;
    m_spBitmapSource = spFrame2;

    UINT uFrameCount = 0;
    m_spDecoder->GetFrameCount(&uFrameCount);
    m_dwFrameCount = uFrameCount;

    return S_OK;
}

HRESULT ImageThumbnail::TryGetEmbeddedThumbnail()
{
    if (!m_spDecoder)
        return E_UNEXPECTED;

    // Try to get embedded thumbnail via WIC metadata reader
    CComPtr<IWICMetadataQueryReader> spQueryReader;
    HRESULT hr = m_spDecoder->GetMetadataQueryReader(&spQueryReader);
    if (FAILED(hr))
        return hr;

    // Check for /app1/ifd/exif/{uint16} thumbnail
    PROPVARIANT var;
    PropVariantInit(&var);
    hr = spQueryReader->GetMetadataByName(L"/app1/ifd/{uint16=513}", &var);
    if (SUCCEEDED(hr))
    {
        PropVariantClear(&var);
        // Thumbnail exists in metadata - extract it
        m_fHasEmbeddedThumbnail = true;

        CComPtr<IWICBitmapFrameDecode> spThumbFrame;
        hr = m_spDecoder->GetFrame(0, &spThumbFrame);
        if (SUCCEEDED(hr))
            m_spBitmapSource = spThumbFrame;
    }

    return hr;
}

HRESULT ImageThumbnail::ScaleImage(UINT uMaxWidth, UINT uMaxHeight, bool fPreserveAspect)
{
    if (!m_spBitmapSource)
        return E_UNEXPECTED;

    if (uMaxWidth == 0 || uMaxHeight == 0)
        return E_INVALIDARG;

    UINT uOrigWidth = 0, uOrigHeight = 0;
    HRESULT hr = m_spBitmapSource->GetSize(&uOrigWidth, &uOrigHeight);
    if (FAILED(hr))
        return hr;

    if (uOrigWidth == 0 || uOrigHeight == 0)
        return E_FAIL;

    UINT uNewWidth = uOrigWidth;
    UINT uNewHeight = uOrigHeight;

    if (uOrigWidth > uMaxWidth || uOrigHeight > uMaxHeight)
    {
        if (fPreserveAspect)
        {
            double dblAspect = static_cast<double>(uOrigWidth) / static_cast<double>(uOrigHeight);
            uNewWidth = uMaxWidth;
            uNewHeight = static_cast<UINT>(uMaxWidth / dblAspect);

            if (uNewHeight > uMaxHeight)
            {
                uNewHeight = uMaxHeight;
                uNewWidth = static_cast<UINT>(uMaxHeight * dblAspect);
            }
        }
        else
        {
            uNewWidth = uMaxWidth;
            uNewHeight = uMaxHeight;
        }
    }

    // Create the scaled bitmap using WIC
    CComPtr<IWICBitmapScaler> spScaler;
    hr = m_spWicFactory->CreateBitmapScaler(&spScaler);
    if (FAILED(hr))
        return hr;

    hr = spScaler->Initialize(m_spBitmapSource, uNewWidth, uNewHeight, WICBitmapInterpolationModeFant);
    if (FAILED(hr))
        return hr;

    m_spThumbnailBitmap.Release();
    hr = m_spWicFactory.p->CreateBitmapFromSource(spScaler, WICBitmapCacheOnLoad, &m_spThumbnailBitmap);
    if (FAILED(hr))
        return hr;

    m_uThumbWidth = uNewWidth;
    m_uThumbHeight = uNewHeight;

    return S_OK;
}

HRESULT ImageThumbnail::ConvertToHBITMAP(HBITMAP* phBitmap, HPALETTE* phPalette)
{
    if (!phBitmap)
        return E_POINTER;

    *phBitmap = nullptr;
    if (phPalette) *phPalette = nullptr;

    if (!m_spThumbnailBitmap || !m_spWicFactory)
        return E_UNEXPECTED;

    CComPtr<IWICBitmapSource> spSource;
    WICPixelFormatGUID guidFormat;
    HRESULT hr = m_spThumbnailBitmap->GetPixelFormat(&guidFormat);
    if (FAILED(hr))
        return hr;

    if (guidFormat != GUID_WICPixelFormat32bppBGRA)
    {
        CComPtr<IWICFormatConverter> spConverter;
        hr = m_spWicFactory->CreateFormatConverter(&spConverter);
        if (FAILED(hr))
            return hr;

        hr = spConverter->Initialize(
            m_spThumbnailBitmap,
            GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
            return hr;

        spSource = spConverter;
    }
    else
    {
        spSource = m_spThumbnailBitmap;
    }

    UINT uWidth = 0, uHeight = 0;
    hr = spSource->GetSize(&uWidth, &uHeight);
    if (FAILED(hr))
        return hr;

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(uWidth);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(uHeight);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    BYTE* pPixels = nullptr;
    HBITMAP hBitmap = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, (void**)&pPixels, nullptr, 0);
    if (!hBitmap)
        return HRESULT_FROM_WIN32(GetLastError());

    hr = spSource->CopyPixels(nullptr, uWidth * 4, uWidth * uHeight * 4, pPixels);
    if (FAILED(hr))
    {
        DeleteObject(hBitmap);
        return hr;
    }

    *phBitmap = hBitmap;
    return S_OK;
}

bool ImageThumbnail::IsWicContainerFormat(const GUID& /*guidContainer*/)
{
    return true;
}

} // namespace HMRAVSource
