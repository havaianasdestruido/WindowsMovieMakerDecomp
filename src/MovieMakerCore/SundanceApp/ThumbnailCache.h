#pragma once
#ifndef SUNDANCE_THUMBNAIL_CACHE_H
#define SUNDANCE_THUMBNAIL_CACHE_H

#include "../pch.h"
#include "../MovieMakerCore.h"
#include <list>

class ThumbnailCache
{
public:
    ThumbnailCache();
    ~ThumbnailCache();

    HBITMAP GetThumbnail(LPCWSTR pszPath);
    HRESULT AddThumbnail(LPCWSTR pszPath, HBITMAP hBitmap);
    HRESULT RemoveThumbnail(LPCWSTR pszPath);
    void Clear();
    void InvalidateAll();
    size_t GetCount() const throw();

private:
    ThumbnailCache(const ThumbnailCache&);
    ThumbnailCache& operator=(const ThumbnailCache&);

    HBITMAP GenerateThumbnail(LPCWSTR pszPath);
    static HBITMAP CreateHBitmapFromFrame(BYTE* pbFrameData, UINT uWidth, UINT uHeight);
    void EvictOldest();
    bool IsFileStale(LPCWSTR pszPath, const FILETIME& cachedTime) const;

    struct CacheEntry
    {
        HBITMAP hBitmap;
        FILETIME lastWriteTime;
        std::list<std::wstring>::iterator lruIter;
    };

    static const size_t MAX_CACHE_SIZE = 200;
    mutable CRITICAL_SECTION m_cs;
    std::map<std::wstring, CacheEntry> m_cache;
    std::list<std::wstring> m_lruOrder;
};

#endif
