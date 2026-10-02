---
sidebar_position: 3
title: MediaCatalog (static library)
description: Media item registry — import, metadata, and thumbnail generation for engine-layer apps.
---

# MediaCatalog

| | |
|---|---|
| **Source** | `src/Media/` |
| **CMake target** | `MediaCatalog` (static library) |
| **Type** | Static lib (linked into consumers) |
| **Family** | [DirectUI engine layer](/docs/modules/dui-engine) |

## At a glance

A compact media-item registry in the `DirectUI`-adjacent engine layer: import files,
track their metadata, and generate thumbnails. Consumed by the engine-layer applications
(GPURenderer demo apps, pipeline-graph, etc.) rather than by MovieMakerCore itself —
which has its own `MediaBrowser` + `ThumbnailCache`.

## Public surface (header `MediaCatalog.h`)

```cpp
struct MediaItem {
    std::wstring id;
    std::wstring path;
    double duration = 0.0;
    int width  = 0;
    int height = 0;
    HBITMAP thumbnail = nullptr;
};

class MediaCatalog {
public:
    std::vector<std::wstring> ImportFiles(const std::vector<std::wstring>& paths);
    const MediaItem* GetItem(const std::wstring& id) const;
    const std::unordered_map<std::wstring, MediaItem>& GetAll() const;
private:
    std::unordered_map<std::wstring, MediaItem> m_items;
    std::wstring GenerateId();
    HBITMAP CreateThumbnail(const std::wstring& path);
};
```

## Implementation notes

- IDs are generated internally (`GenerateId`) — callers refer to items by ID.
- Thumbnails are `HBITMAP`s created via GDI+ (`CreateThumbnail`), keeping the dependency
  footprint to Windows APIs only.
- Being a static library, it has no exports or DLL entry points; it participates in the
  CI matrix as target `MediaCatalog`.

## Testing

Built as part of the CI matrix (`build-projects.yml`); exercised by the engine-layer
apps under `apps/`.

## Analysis artifacts

This is reconstructed-first code (engine layer), so no 1:1 original binary; see
`analysis/MovieMakerCore/` for the equivalent in-engine surfaces.
