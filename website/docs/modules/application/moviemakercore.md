---
sidebar_position: 2
title: MovieMakerCore.dll (Engine)
description: The main engine — SundanceApp, storyboard model, HMREngine, HMRAVSource, Ribbon UI, serialization, publishing.
---

# MovieMakerCore.dll

| | |
|---|---|
| **Source** | `src/MovieMakerCore/` (349 files) |
| **CMake target** | `MovieMakerCore` |
| **Type** | Win32 DLL (the engine), ~1.4 MB |
| **Family** | [Application targets](../application) |

## At a glance

The heart of the product — the original binary is 10.1 MB with ~5.46 MB of code and 125+
RTTI classes. It houses the Sundance app framework, the storyboard/project model, the
HMREngine render core, the Media Foundation A/V pipeline, the full Ribbon UI,
serialization, publishing, preview, and transport.

## Public surface

Exactly **one export**:

```text
MovieMakerMain @1        ; __cdecl, undecorated
```

Everything else is internal. The DLL is loaded via `LoadLibrary`/`GetProcAddress`, never
`CoCreateInstance` — its CLSID is all zeros
([Quirk #3](../../methodology/quirks.md#3-null-clsid-for-moviemakercoredll)).

`MovieMakerCore.cpp` holds the original stub exports (file-locked in the original layout)
and `MovieMakerCore_new.cpp` the corrected export set that is actually compiled — a
reconstruction artifact documented in the git history.

## Subsystems

Full deep-dives live under [MovieMakerCore Internals](../../architecture/moviemakercore/):

| Subsystem | Directory |
|---|---|
| App framework (controllers, clipboard, undo, auto-save) | `SundanceApp/` |
| Project model (project, extents, tracks, templates, themes) | `StoryboardManager/` |
| Render core (D3D11/D2D/DWrite, X3D scene graph, pattern meshes) | `HMREngine/` |
| A/V pipeline (MF sources/sinks, transcoding, audio DSP, capture) | `HMRAVSource/` |
| UI (Ribbon with 67 commands, 37 behavior classes, timeline UI) | `UI/` |
| Preview presenter + data context | `Preview/` |
| Publishing (background jobs, service classes, dialogs) | `Publishing/` |
| `.wlmp` serialization | `StoryboardManager/Serialization/` |
| Add-in contract, external stubs, containers, resource IDs | `AddIn/`, `External/`, `DataStructs/`, `Resources/` |

## Key implementation files

| File | Role |
|---|---|
| `pch.h` / `pch.cpp` | Precompiled header + SDK compatibility section (includes the `IDXVA2VideoProcessor` stub interface) |
| `dllmain.cpp` | DLL entry: `MovieCore_Initialize`/`Shutdown`, COM/D3D11/WIC/MF init, single-instance mutex |
| `ComFactory.h` | ATL COM wrappers (with the MSVC 14.44 `COM_INTERFACE_ENTRY` fixes) |
| `exports.h` / `common.h` | Export macros, version macros (`WMMR_VERSION_*`), shared includes |

## Compatibility shims inside the engine

- `HMREngine/d3dx11compat.h/.cpp` — D3DX11 Effect Framework stub over raw D3D11.
- `HMRAVSource/dxva2stubs.cpp` — DXVA2 video-processor stubs for removed SDK APIs.
- See [SDK Compatibility](../../reference/sdk-compatibility.md) for the full list.

## Hardening (2026-09 pass)

Documented in `src/MovieMakerCore/README.md`: clipboard failure surfaces `E_FAIL`, ribbon
MRU registry failure surfaces the Win32 error, scene traversal depth-capped at 512,
X3DReader caps (512 depth / 100k nodes), DPAPI-protected PSA auth store,
in-memory-only `AuthProvider`. See [Security](../../reference/security.md).

## Testing

- CTest sanity check `sanity_MovieMakerCore_dll_exists`.
- Contracts across many surfaces live in `tests/mmr-python/wmmr/contracts/` (see
  [Contract Suite](../../testing/contract-suite.md)).
- `tests/MovieMakerCore/test_core.cpp` — legacy per-module C++ test.

## Analysis artifacts

`analysis/MovieMakerCore/` (RTTI, strings, imports; PDB GUID
`{D5217874-B614-477C-B45B-E0CE638C6496}`) plus `analysis/ThreadModel/`,
`analysis/COMGuids/`, `analysis/ArchitectureSynthesis/`.
