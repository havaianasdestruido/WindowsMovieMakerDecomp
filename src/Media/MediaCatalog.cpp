#include "MediaCatalog.h"
#include <objbase.h>
#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <wincodec.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace
{

// Decode the first frame of an image file with WIC and scale it to fit inside
// the thumbnail box (preserving aspect ratio). Returns a 32bpp top-down DIB
// section, or nullptr when the file cannot be decoded or is unreasonably
// large. The output is always bounded by the thumbnail box.
HBITMAP DecodeThumbnailWic(const std::wstring& path, UINT uMaxWidth, UINT uMaxHeight)
{
    const UINT kMaxSourceDimension = 16384;

    ComPtr<IWICImagingFactory> wicFactory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory));
    if (FAILED(hr))
        return nullptr;

    ComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr))
        return nullptr;

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr))
        return nullptr;

    UINT uSrcWidth = 0;
    UINT uSrcHeight = 0;
    hr = frame->GetSize(&uSrcWidth, &uSrcHeight);
    if (FAILED(hr))
        return nullptr;

    // Bound the decode: reject empty or absurdly large sources outright.
    if (uSrcWidth == 0 || uSrcHeight == 0 ||
        uSrcWidth > kMaxSourceDimension || uSrcHeight > kMaxSourceDimension)
        return nullptr;

    // Scale down when the source exceeds the box, and also when it is merely
    // small in each dimension but enormous in pixel count.
    const bool fScale =
        (uSrcWidth > uMaxWidth || uSrcHeight > uMaxHeight) ||
        (static_cast<UINT64>(uSrcWidth) * static_cast<UINT64>(uSrcHeight) >
            64ull * 1024ull * 1024ull);

    UINT uThumbWidth = uSrcWidth;
    UINT uThumbHeight = uSrcHeight;

    IWICBitmapSource* pSource = frame.Get();

    if (fScale)
    {
        double dblAspect = static_cast<double>(uSrcWidth) / static_cast<double>(uSrcHeight);
        uThumbWidth = uMaxWidth;
        uThumbHeight = static_cast<UINT>(uMaxWidth / dblAspect);
        if (uThumbHeight > uMaxHeight)
        {
            uThumbHeight = uMaxHeight;
            uThumbWidth = static_cast<UINT>(uMaxHeight * dblAspect);
        }
        if (uThumbWidth == 0)
            uThumbWidth = 1;
        if (uThumbHeight == 0)
            uThumbHeight = 1;

        ComPtr<IWICBitmapScaler> scaler;
        hr = wicFactory->CreateBitmapScaler(&scaler);
        if (SUCCEEDED(hr))
            hr = scaler->Initialize(frame.Get(), uThumbWidth, uThumbHeight,
                WICBitmapInterpolationModeFant);
        if (FAILED(hr))
            return nullptr;

        pSource = scaler.Get();
    }

    // Convert to 32bppBGRA so the pixel copy into the DIB section is a straight
    // row copy regardless of the source pixel format.
    WICPixelFormatGUID guidFormat = GUID_WICPixelFormatUndefined;
    pSource->GetPixelFormat(&guidFormat);

    ComPtr<IWICFormatConverter> converter;
    if (guidFormat != GUID_WICPixelFormat32bppBGRA)
    {
        hr = wicFactory->CreateFormatConverter(&converter);
        if (SUCCEEDED(hr))
            hr = converter->Initialize(pSource, GUID_WICPixelFormat32bppBGRA,
                WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
            return nullptr;

        pSource = converter.Get();
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(uThumbWidth);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(uThumbHeight); // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    BYTE* pPixels = nullptr;
    HBITMAP hBitmap = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS,
        reinterpret_cast<void**>(&pPixels), nullptr, 0);
    if (!hBitmap)
        return nullptr;

    hr = pSource->CopyPixels(nullptr, uThumbWidth * 4, uThumbWidth * uThumbHeight * 4, pPixels);
    if (FAILED(hr))
    {
        DeleteObject(hBitmap);
        return nullptr;
    }

    return hBitmap;
}

} // namespace


MediaCatalog::MediaCatalog() {
    MFStartup(MF_VERSION);
}

MediaCatalog::~MediaCatalog() {
    for (auto &kv : m_items) {
        if (kv.second.thumbnail) {
            DeleteObject(kv.second.thumbnail);
        }
    }
    MFShutdown();
}

std::wstring MediaCatalog::GenerateId() {
    GUID guid;
    CoCreateGuid(&guid);
    wchar_t buffer[64];
    StringFromGUID2(guid, buffer, 64);
    return std::wstring(buffer);
}

HBITMAP MediaCatalog::CreateThumbnail(const std::wstring& path) {
    // Decode and scale the source image with WIC into a 160x120 thumbnail box.
    // The catalog can run on threads that have not initialized COM, so guard
    // the WIC factory creation with a per-call apartment init.
    const UINT kMaxThumbWidth = 160;
    const UINT kMaxThumbHeight = 120;

    HRESULT hrCoInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hrCoInit))
        return CreateBitmap(1, 1, 1, 0, nullptr);

    HBITMAP hThumbnail = DecodeThumbnailWic(path, kMaxThumbWidth, kMaxThumbHeight);

    // Only uninitialize what this call initialized (S_OK, not S_FALSE).
    if (hrCoInit == S_OK)
        CoUninitialize();

    // Deterministic non-null fallback for callers when the image cannot be
    // decoded (missing file, unsupported format, or oversized source).
    return hThumbnail ? hThumbnail : CreateBitmap(1, 1, 1, 0, nullptr);
}

std::vector<std::wstring> MediaCatalog::ImportFiles(const std::vector<std::wstring>& paths) {
    std::vector<std::wstring> ids;
    for (const auto& p : paths) {
        MediaItem item;
        item.id = GenerateId();
        item.path = p;
        // Open source reader
        ComPtr<IMFSourceReader> reader;
        HRESULT hr = MFCreateSourceReaderFromURL(p.c_str(), nullptr, &reader);
        if (SUCCEEDED(hr) && reader) {
            // Get media source
            ComPtr<IMFMediaSource> mediaSource;
            reader->GetServiceForStream(MF_SOURCE_READER_MEDIASOURCE, GUID_NULL, IID_PPV_ARGS(&mediaSource));
            if (mediaSource) {
                ComPtr<IMFPresentationDescriptor> pd;
                if (SUCCEEDED(mediaSource->CreatePresentationDescriptor(&pd))) {
                    UINT64 dur = 0;
                    if (SUCCEEDED(pd->GetUINT64(MF_PD_DURATION, &dur))) {
                        item.duration = static_cast<double>(dur) / 10000000.0; // 100-ns to seconds
                    }
                }
            }
            // Get video dimensions from first stream (index 0)
            ComPtr<IMFMediaType> type;
            if (SUCCEEDED(reader->GetNativeMediaType(0, 0, &type))) {
                UINT32 w = 0, h = 0;
                MFGetAttributeSize(type.Get(), MF_MT_FRAME_SIZE, &w, &h);
                item.width = static_cast<int>(w);
                item.height = static_cast<int>(h);
            }
        }
        item.thumbnail = CreateThumbnail(p);
        m_items[item.id] = item;
        ids.push_back(item.id);
    }
    return ids;
}

const MediaItem* MediaCatalog::GetItem(const std::wstring& id) const {
    auto it = m_items.find(id);
    return it != m_items.end() ? &it->second : nullptr;
}
