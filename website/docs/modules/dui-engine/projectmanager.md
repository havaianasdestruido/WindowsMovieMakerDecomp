---
sidebar_position: 4
title: ProjectManager (static library)
description: The engine-layer project model — create, load, save over a simple media/clip/timeline schema.
---

# ProjectManager

| | |
|---|---|
| **Source** | `src/Project/` |
| **CMake target** | `ProjectManager` (static library) |
| **Type** | Static lib (linked into consumers) |
| **Family** | [DirectUI engine layer](/modules/dui-engine) |

## At a glance

The engine-layer project model: a minimal `Project` schema with media references and
clips, plus a manager for create/load/save. Distinct from MovieMakerCore's
`MovieProject` (which implements the full `.wlmp` model) — this is the simplified model
used by the engine-layer targets and their demo apps.

## Public surface (header `Project.h`)

```cpp
struct MediaRef { std::wstring id; std::wstring path; };

struct Clip {
    std::wstring mediaId;
    double start;
    double duration;
    int track;
};

struct Project {
    std::wstring title;
    std::vector<MediaRef> media;
    std::vector<Clip> timeline;
};

class ProjectManager {
public:
    bool CreateNew(const std::wstring& title);
    bool Load(const std::wstring& filePath);
    bool Save(const std::wstring& filePath) const;
    const Project& GetProject() const;
};
```

## Implementation notes

- Clips reference media by `mediaId` — same reference pattern as
  [MediaCatalog](./mediacatalog.md).
- Load/Save is a simple file format sufficient for the engine-layer apps; the full
  `.wlmp` XML format belongs to
  [MovieMakerCore serialization](../../architecture/moviemakercore/serialization.md).
- No exports — static library, CI target name `ProjectManager`.

## Testing

Built in the CI matrix; exercised by engine-layer apps.

## Analysis artifacts

Engine-layer reconstruction; no dedicated original binary. See
`analysis/MovieMakerCore/` for the in-engine equivalent.
