---
sidebar_position: 3
title: WLXSlideshow.dll
description: The slideshow generation engine — turning photo sets into animated slideshows with transitions.
---

# WLXSlideshow.dll

| | |
|---|---|
| **Source** | `src/WLXSlideshow/` |
| **CMake target** | `WLXSlideshow` |
| **Type** | Win32 COM DLL |
| **Family** | [Pipeline & effects targets](../pipeline-effects) |

## At a glance

The **slideshow generation engine** shared across Windows Live photo/video apps: takes an
ordered set of photos plus a style, and produces an animated slideshow (Ken Burns moves,
crossfades, timed transitions). In Movie Maker it powers the auto-movie/slideshow
creation flows.

## Public surface

The standard COM quartet (`WLXSlideshow.def`):

```text
DllCanUnloadNow     DllGetClassObject
DllRegisterServer  DllUnregisterServer
```

## Relationship to other modules

- **[WLXPhotoCinematic](./wlxphotocinematic.md)** computes the *cinematic moves* (Ken
  Burns pan/zoom analysis) that slideshows consume.
- **[WLXPipetran](./wlxpipetran.md)** supplies transition animations between slides.
- MovieMakerCore's `StoryboardManager/Templates` exposes slideshow themes (7 themes) at
  the editor level.

## Implementation notes

- COM class factory is real and contract-pinned.
- The reconstruction implements slideshow sequencing (durations, transition selection,
  effect parameters); rendering runs through the host application's pipeline.

## Testing

- `tests/WLXSlideshow/` — per-module C++ tests.
- Contract coverage for COM activation in `tests/mmr-python`.
- Exercise app: `apps/slideshow-studio` (submodule).

## Analysis artifacts

`analysis/WLXSlideshow/`.
