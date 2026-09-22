#include "MediaCatalog.h"
#include <objbase.h>
#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

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

HBITMAP MediaCatalog::CreateThumbnail(const std::wstring& /*path*/) {
    // TODO(reconstruction): Decode and scale the source image once thumbnail behavior is recovered.
    // Current behavior intentionally supplies a deterministic non-null fallback for callers.
    return CreateBitmap(1, 1, 1, 0, nullptr);
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
