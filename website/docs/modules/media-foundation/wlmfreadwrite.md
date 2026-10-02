---
sidebar_position: 2
title: WLMFReadWrite.dll (MF Reader/Writer)
description: Flat C exports over the Media Foundation source reader and sink writer — open, read frames, write, finalize.
---

# WLMFReadWrite.dll

| | |
|---|---|
| **Source** | `src/WLMFReadWrite/` |
| **CMake target** | `WLMFReadWrite` |
| **Type** | Win32 DLL (252 KB / 217 KB code, 70+ RTTI classes in the original) |
| **Family** | [Media Foundation targets](/docs/modules/media-foundation) |

## At a glance

A **flat C API over Media Foundation read/write**: the original exposes 7 exports — 5 MF
reader/writer functions plus 2 COM standard entries. This is the workhorse DLL for
codecs/transcode tooling that wants MF without the full object model.

## Public surface

From `WLMFReadWrite.def`:

```text
DllCanUnloadNow               DllGetClassObject
MFReader_Close@4          @3
MFReader_GetProperties@8  @4
MFReader_Open@4           @5
MFReader_ReadFrame@24     @6
MFWriter_Create@8         @7
MFWriter_Finalize@4       @8
MFWriter_WriteFrame@20    @9
```

All `__stdcall` with byte-count decorations.

## API shape

| Function | Purpose |
|---|---|
| `MFReader_Open` / `MFReader_Close` | Open a media source / release it |
| `MFReader_GetProperties` | Query stream properties (duration, formats, frame count) |
| `MFReader_ReadFrame` | Read one frame (24 bytes of params — buffer, stride, timestamps) |
| `MFWriter_Create` | Create a writer for an output profile |
| `MFWriter_WriteFrame` | Write one frame |
| `MFWriter_Finalize` | Finalize the output (flush + index + close) |

## Implementation notes

- Wraps `IMFSourceReader` / `IMFSinkWriter` from `mfreadwrite.h`.
- Contract-pinned: HRESULTs for open/read/write on sample media are asserted by
  `tests/mmr-python` (the parity-stub build's `E_NOTIMPL`s are *not* the oracle here).

## Testing

- `tests/WLMFReadWrite/` — per-module C++ tests.
- Contract coverage in `tests/mmr-python`.

## Analysis artifacts

`analysis/WLMFReadWrite/` (PDB GUID `{54E10C67-67A5-4F8A-8567-1D47C28B2189}`).
