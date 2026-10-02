---
sidebar_position: 7
title: PlaybackEngine (static library)
description: Media Foundation playback — open, play, pause, seek, and per-frame updates onto a GPURenderer.
---

# PlaybackEngine

| | |
|---|---|
| **Source** | `src/Playback/` |
| **CMake target** | `PlaybackEngine` (static library) |
| **Type** | Static lib (linked into consumers) |
| **Family** | [DirectUI engine layer](/docs/modules/dui-engine) |

## At a glance

A compact **Media Foundation** playback engine in the `DirectUI` namespace: opens a media
file, plays/pauses/stops/seeks, and pushes frames to a
[GPURenderer](./gpurenderer.md) for presentation. The engine-layer counterpart of
MovieMakerCore's HMRAVSource + Preview stack.

## Public surface (header `Playback.h`)

```cpp
namespace DirectUI {

class GPURenderer;

class __declspec(dllexport) PlaybackEngine {
public:
    explicit PlaybackEngine(GPURenderer* pRenderer);
    ~PlaybackEngine();

    HRESULT OpenFile(const std::wstring& path);
    HRESULT Play();
    HRESULT Pause();
    HRESULT Stop();
    HRESULT Seek(double seconds);
    HRESULT UpdateFrame();
    // ...
};

} // namespace DirectUI
```

## Implementation notes

- Media Foundation based: includes `mfapi.h`, `mfobjects.h`, `mfidl.h`, `mfreadwrite.h`
  (source-reader style playback), plus `d3d11.h` for the texture handoff.
- Construction takes a `GPURenderer*` — frames decoded by MF are handed to the renderer's
  `DrawVideoFrame` path. `UpdateFrame()` pumps one frame (used for scrubbing/paused
  seek display).
- Being a static library it has no exports/ordinals; it appears in the CI matrix as
  `PlaybackEngine`.

## Testing

Built in the CI matrix; exercised by engine-layer apps (e.g. `apps/pipeline-graph`).

## Analysis artifacts

Engine-layer reconstruction; the original's in-engine playback analysis lives under
`analysis/MovieMakerCore/` and `analysis/SharedMFDlls/`.
