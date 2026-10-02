---
sidebar_position: 5
title: WLXCodecHost.exe
description: The out-of-process COM codec host — isolating codec bugs from the editor process.
---

# WLXCodecHost.exe

| | |
|---|---|
| **Source** | `src/WLXCodecHost/` |
| **CMake target** | `WLXCodecHost` |
| **Type** | Win32 EXE (~32 KB / ~20 KB code in the original) |
| **Family** | [Media Foundation targets](/docs/modules/media-foundation) |

## At a glance

An **out-of-process COM codec host**: hosts media codecs in a surrogate process so a
misbehaving third-party codec cannot take down the editor. The reference binary exposes
the 4 COM standard entries and nothing else — no RTTI classes (pure host shell).

## Role

COM activation (via `CoCreateInstance` with `CLSCTX_LOCAL_SERVER`) hands codec work to
this process. MovieMakerCore's `WLXCodecHostPS` proxy/stub pair
(`analysis/WLXCodecHostPS/`) marshals the interfaces across the process boundary.

## Implementation notes

- Minimal by design: a message loop plus COM registration — the original has no
  application UI and no exports beyond the COM quartet.
- The reconstruction is a single source file (`WLXCodecHost.cpp`) implementing the
  local-server registration and class factory.

## Testing

- `tests/WLXCodecHost/` and `tests/WLXCodecHostPS/` — per-module test directories.
- Built in the CI matrix.

## Analysis artifacts

`analysis/WLXCodecHost/` and `analysis/WLXCodecHostPS/` (proxy/stub marshaling analysis).
