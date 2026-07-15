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

    void InvalidateAll();

private:
    ThumbnailCache(const ThumbnailCache&);
    ThumbnailCache& operator=(const ThumbnailCache&);
};

#endif
