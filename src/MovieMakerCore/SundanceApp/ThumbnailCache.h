#pragma once
#ifndef SUNDANCE_THUMBNAIL_CACHE_H
#define SUNDANCE_THUMBNAIL_CACHE_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class ThumbnailCache
{
public:
    ThumbnailCache();
    ~ThumbnailCache();

    Gdiplus::Bitmap* GetThumbnail(LPCWSTR pszPath);
    HRESULT AddThumbnail(LPCWSTR pszPath, Gdiplus::Bitmap* pBitmap);
    HRESULT RemoveThumbnail(LPCWSTR pszPath);
    void Clear();
    void InvalidateAll();
    size_t GetCount() const throw();

private:
    ThumbnailCache(const ThumbnailCache&);
    ThumbnailCache& operator=(const ThumbnailCache&);

    std::map<std::wstring, Gdiplus::Bitmap*> m_cache;
};

#endif
