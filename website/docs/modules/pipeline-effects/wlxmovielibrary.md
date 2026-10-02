---
sidebar_position: 6
title: WLXMovieLibrary.dll
description: The movie library — CreateMovieFactory plus a D3D9-based video processor, 20+ RTTI classes.
---

# WLXMovieLibrary.dll

| | |
|---|---|
| **Source** | `src/WLXMovieLibrary/` |
| **CMake target** | `WLXMovieLibrary` |
| **Type** | Win32 COM DLL (324 KB / 281 KB code, 20+ RTTI classes in the original) |
| **Family** | [Pipeline & effects targets](/docs/modules/pipeline-effects) |

## At a glance

The **movie library**: manages a library of movie items (metadata, thumbnails, video
processing) for the browsing surfaces. The reference binary exposes a single meaningful
export — `CreateMovieFactory` — and a notable **D3D9-based video processor** among its
RTTI classes (Movie Maker 2012 still used D3D9 paths for certain video processing, even
as the main engine moved to D3D11).

## Public surface

From `WLXMovieLibrary.def`:

```text
CreateMovieFactory @1
DllCanUnloadNow     DllGetClassObject
DllRegisterServer  DllUnregisterServer
```

## Implementation notes

- `CreateMovieFactory` is the entry to the library object model; factory behavior is
  contract-pinned.
- The D3D9 video processor surfaces the older `d3d9.lib` link (present in the common
  system libs of the root CMake file) — kept because the original used it.
- Movie browser features in the main app's `MediaBrowser` talk to this library for
  library-level queries (aggregated movie items).

## Testing

- `tests/WLXMovieLibrary/` — per-module C++ tests.
- Contract coverage for the factory in `tests/mmr-python`.
- Exercise app: `apps/movielibrary` (submodule).

## Analysis artifacts

`analysis/WLXMovieLibrary/` (PDB GUID `{CBF27DEC-DD3B-4A76-A5DF-9EECCC486627}`).
