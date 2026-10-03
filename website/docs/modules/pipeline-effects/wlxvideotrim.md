---
sidebar_position: 5
title: WLXVideoTrim.dll
description: DirectShow-based video trimming — AVI/ASF/DV/MPEG2/MP4/WMV copy pipelines with a custom HRESULT facility.
---

# WLXVideoTrim.dll

| | |
|---|---|
| **Source** | `src/WLXVideoTrim/` (the CMake-template module) |
| **CMake target** | `WLXVideoTrim` |
| **Type** | Win32 DLL (568 KB in the original) |
| **Family** | [Pipeline & effects targets](/modules/pipeline-effects) |

## At a glance

**Video trimming** without re-encoding where possible: cut points on AVI, ASF, DV,
MPEG2, MP4, and WMV containers using **DirectShow-based** copy/transcode pipelines. Used
by the quick-trim UI (and the `apps/video-trimmer` exercise app).

## Public surface

From `WLXVideoTrim.def` — five factories:

```text
CreateAVICopierDirect @1
CreateVideoCopierFromMediaType @2
CreateVideoFormatContextTranscoder @3
CreateVideoPlayer @4
CreateVideoWMVTranscoder @5
```

## Custom HRESULT facility

The module defines its own error space — `FACILITY_WLXVIDEOTRIM` with `AVS_E_*` codes.
Match them exactly when touching error paths; see the module README. This is one of the
reasons the module is the reference example for per-module conventions.

## Implementation notes

- **Real COM factories, contract-pinned** — this module is the canonical
  "COM factories" row in the [stub-category table](../../methodology/stub-design.md#stub-categories).
- `CreateAVICopierDirect` is one of the entry points where the *reference parity build*
  returns `E_NOTIMPL`; the reconstruction ships real implementations and the contract
  pins the real HRESULTs.
- The module's `CMakeLists.txt` is the **template** the AGENTS instructions point to for
  new modules: `project()`, `add_library(... SHARED)`, `OUTPUT_NAME` without
  `PREFIX`/`SUFFIX`, `MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"`, explicit includes and
  link libraries.

## Testing

- Contract coverage in `tests/mmr-python/wmmr/contracts/` (factory semantics + HRESULTs).
- Exercise app: `apps/video-trimmer` (submodule).

## Analysis artifacts

`analysis/WLXVideoTrim/`.
