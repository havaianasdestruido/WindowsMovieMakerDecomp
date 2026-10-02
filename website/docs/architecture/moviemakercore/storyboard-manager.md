---
sidebar_position: 2
title: StoryboardManager (Project Model)
description: The movie data model — projects, tracks, extents, templates, themes, and effects.
---

# StoryboardManager

**Location:** `src/MovieMakerCore/StoryboardManager/`

StoryboardManager is the **data model** of a movie: everything the user edits lives here,
and everything else (UI, transport, rendering, serialization) is a view over it.

## Tree

```text
StoryboardManager/
  StoryboardManager.cpp/.h          # top-level manager/facade
  MovieProject.cpp/.h               # the core data model (I/O, extent management)
  MovieExtent.cpp/.h                # a clip on the timeline (media ref + in/out + effects)
  Extents.cpp/.h                    # extent arithmetic (layout, overlap, ripple)
  TimelineTrack.cpp/.h              # track management (0 = video, >0 = audio)
  Templates.cpp/.h                  # transition/effect templates (23 effects, 7 themes)
  MovieEffect/                      # effect serialization
  Theme/                            # visual themes (Apply/Remove, XML patching)
  Serialization/                    # .wlmp file I/O (see Serialization page)
  Transport/                        # playback transport + CommandBin
  Background/                       # background processing
  MediaItems/                       # imported media item registry
  StoryboardManagerExceptions.cpp/.h# model-level exceptions
```

## Model invariants

- A **`MovieProject`** owns ordered **`TimelineTrack`**s. Track 0 is the primary video
  track; audio-only tracks sit above.
- Each track contains **`MovieExtent`**s — a media reference plus in/out points, speed,
  and attached effects/transitions.
- **`Extents`** implements the placement arithmetic: overlap resolution, ripple moves,
  trim-induced reflow. UI never computes layout itself.
- **`Templates`** define the catalog: **23 effects and 7 themes**. Template application is
  undoable and serialized into the project.
- **`Theme`** implements Apply/Remove by patching the project's effect graph via XML —
  the same mechanism the original used.

## Templates, effects, and the transition bridge

An effect on an extent references a template; rendering resolves the template to a
transition/effect animation from **WLXPipetran** (the 92 transition/effect animation
classes — [Quirk #18](../../methodology/quirks.md#18-92-transitioneffect-animation-classes)).
The `DEFINE_STUB_TRANSITION` macro family wires fallbacks where an animation is not yet
reconstructed (note the historical `L##stringId` macro bug — Gotcha #19 in
`ROADMAP.md`).

## Exceptions

`StoryboardManagerExceptions` carries model-level failures (corrupt project, impossible
edit) as typed exceptions that surface through `ProjectManager` as HRESULTs. They allocate
on the **process heap**, not the CRT heap — preserved original behavior
([Quirk #8](../../methodology/quirks.md#8-exception-objects-use-process-heap-not-crt-heap)).

## Circular-include lessons

Two structural notes from the reconstruction (also in `ROADMAP.md` gotchas):

- Circular includes between `MovieProject` and track/extent headers were broken by
  extracting shared enums into a separate header.
- `ProjectTimeline` is heap-allocated inside its owner specifically to break an include
  cycle (Gotcha #17).

## Related analysis

- `analysis/MovieMakerCore/` — RTTI class list driving the model shape
- `tests/mmr-python` — serialization round-trip and template contracts
