#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <windows.h>

struct MediaItem {
    std::wstring id;
    std::wstring path;
    double duration = 0.0;
    int width = 0;
    int height = 0;
    HBITMAP thumbnail = nullptr;
};

class MediaCatalog {
public:
    MediaCatalog();
    ~MediaCatalog();
    std::vector<std::wstring> ImportFiles(const std::vector<std::wstring>& paths);
    const MediaItem* GetItem(const std::wstring& id) const;
    const std::unordered_map<std::wstring, MediaItem>& GetAll() const { return m_items; }
private:
    std::unordered_map<std::wstring, MediaItem> m_items;
    std::wstring GenerateId();
    HBITMAP CreateThumbnail(const std::wstring& path);
};
