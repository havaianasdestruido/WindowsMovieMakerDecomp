---
sidebar_position: 3
title: WLXMP4Parser.dll
description: The MP4 container parser — bounds-checked box walker with capped trak counts, plus DirectShow graph helpers.
---

# WLXMP4Parser.dll

| | |
|---|---|
| **Source** | `src/WLXMP4Parser/` |
| **CMake target** | `WLXMP4Parser` |
| **Type** | Win32 DLL (184 KB / 147 KB code, 50+ RTTI classes in the original) |
| **Family** | [Media Foundation targets](../media-foundation) |

## At a glance

The **MP4 container parser**: walks ISO-BMFF boxes (`ftyp`, `moov`, `trak`, `stts`, ...)
and exposes DirectShow integration helpers for MP4 playback. 9 exports in the reference
binary: 4 API functions, 4 COM standard entries, plus the DLL entry.

## Public surface

From `WLXMP4Parser.def`:

```text
AddMP4SourceFilter@12   @1
BuildMP4FilterGraph@8   @2
BuildMP4PlayBack@8      @3
DllCanUnloadNow         @4
DllGetClassObject       @5
DllRegisterServer       @6
DllUnregisterServer     @7
IsMP4FilePlayable@4     @8
```

## Security-hardened parsing

This DLL parses **untrusted files from disk**, so it is the reference implementation for
the project's bounded-parser policy (see [Security](../../reference/security.md)):

- The **box walker is bounds-checked** — every box header is validated against the
  remaining buffer before descending.
- `MoovBox` **caps trak count at 1024** — a malicious file cannot explode track arrays.
- `ParseStts` accumulates sample counts in **64-bit before clamping** — no overflow
  during accumulation.

## Stub-design note

`BuildMP4FilterGraph` is one of the factory entry points where the *reference parity
build* returns `E_NOTIMPL` — the reconstruction ships the real implementation and the
contract pins the real HRESULT
([Stub Design rule 3](../../methodology/stub-design.md#ground-rules)).

## Testing

- `tests/WLXMP4Parser/` — per-module C++ tests.
- Contract coverage including parser bounds in `tests/mmr-python`.

## Analysis artifacts

`analysis/WLXMP4Parser/` (PDB GUID `{A4577D2C-5400-4F17-91C0-06B5EA1BE4DE}`).
