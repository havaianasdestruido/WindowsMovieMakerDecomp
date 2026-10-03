---
sidebar_position: 4
title: MovieMakerPreviewClient.dll
description: The embedded preview-pane communication interface — window lifecycle, seek-and-render, transport, snapshots.
---

# MovieMakerPreviewClient.dll

| | |
|---|---|
| **Source** | `src/MovieMakerPreviewClient/` |
| **CMake target** | `MovieMakerPreviewClient` |
| **Type** | Win32 COM DLL (28 KB / 5.9 KB code in the original) |
| **Family** | [Application targets](/modules/application) |

## At a glance

Provides the **preview window communication interface** used by the main application to
interact with an embedded preview pane. The public header
(`MovieMakerPreviewClient.h`) documents its scope:

- Preview window **creation and lifecycle**
- **Frame rendering requests** (seek + render a single frame)
- **Transport control** (play / pause / stop / seek)
- **Resize and aspect-ratio handling**
- **Snapshot capture** from the preview surface

## Public surface

The standard COM quartet:

```text
DllCanUnloadNow     DllGetClassObject
DllRegisterServer   DllUnregisterServer
```

## Implementation notes

- The client renders via **GDI+** (unlike the D3D11 in-process preview presenter — this
  DLL serves the simpler embedded-pane contract).
- Built with the same header conventions as the original (`STRICT`, `WIN32_LEAN_AND_MEAN`,
  `NOMINMAX` guards; `WINVER 0x0602`).
- GDI+ drawing notes: `Gdiplus::Graphics::DrawImage` takes `Gdiplus::Image*`, not
  `const Bitmap*` (Gotcha #21 in `ROADMAP.md`).

## Testing

- Contract coverage in `tests/mmr-python` (COM activation + transport semantics).
- `tests/MovieMakerPreviewClient/` — per-module C++ test directory.

## Analysis artifacts

`analysis/MovieMakerPreviewClient/` (PDB GUID
`{ABE6F6C4-02E6-47EA-B7FF-6A44A32264DD}`).
