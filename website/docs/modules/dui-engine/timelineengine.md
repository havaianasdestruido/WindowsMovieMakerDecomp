---
sidebar_position: 5
title: TimelineEngine (static library)
description: Timeline clip arithmetic — add, move, trim with snapping and overlap detection, plus the UI dispatcher.
---

# TimelineEngine

| | |
|---|---|
| **Source** | `src/Timeline/` (5 files) |
| **CMake target** | `TimelineEngine` (static library) |
| **Type** | Static lib (linked into consumers) |
| **Family** | [DirectUI engine layer](../dui-engine) |

## At a glance

The engine-layer timeline: clip placement with **snapping** and **overlap detection**,
plus a `TimelineDispatcher` that mediates between the timeline model, the media catalog,
and the playback engine for UI clients.

## Public surface

**`Clip.h`** — the clip record:

```cpp
struct Clip {
    std::wstring mediaId;   // reference to MediaCatalog entry
    double start;           // start time on timeline (seconds)
    double duration;        // length (seconds)
    int track;              // track index (0 = video, >0 = audio etc.)
};
```

**`TimelineEngine.h`** — the model:

```cpp
class TimelineEngine {
public:
    bool AddClip(const Clip& clip);
    bool RemoveClip(const std::wstring& mediaId, double start);
    bool MoveClip(const std::wstring& mediaId, double oldStart, double newStart, int newTrack);
    bool TrimClip(const std::wstring& mediaId, double start, double newDuration);
    const std::vector<Clip>& GetClips() const;
private:
    double Snap(double time) const;      // 0.5 s grid
    bool Overlaps(const Clip& a, const Clip& b) const;
};
```

**`TimelineDispatcher.h`** — the `DirectUI::TimelineDispatcher` that connects the engine
to `PlaybackEngine` and `MediaCatalog` for consumers.

## Implementation notes

- Snapping rounds to a **0.5-second grid** (`Snap`), matching the coarse-drag behavior of
  the engine-layer demo apps. (The full editor's magnetic timeline lives in
  MovieMakerCore's `Extents` arithmetic.)
- `Overlaps` guards `AddClip`/`MoveClip` — clips cannot collide on a track.
- The file trio mirrors the
  [ProjectManager](./projectmanager.md)/[MediaCatalog](./mediacatalog.md) layering: model
  in the engine, UI binding via the dispatcher.

## Testing

Built in the CI matrix as `TimelineEngine`; exercised by engine-layer apps
(`apps/pipeline-graph` and friends).

## Analysis artifacts

Engine-layer reconstruction; the in-engine equivalent analysis lives in
`analysis/MovieMakerCore/`.
