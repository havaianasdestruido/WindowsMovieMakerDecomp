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

    std::map<std::wstring, HBITMAP> m_cache;
};

#endif
