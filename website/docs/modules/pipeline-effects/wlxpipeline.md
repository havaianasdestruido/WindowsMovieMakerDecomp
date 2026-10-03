---
sidebar_position: 1
title: WLXPipeline.dll
description: The media pipeline factory — GetPipelineCreateFunctions returning a six-pointer function table.
---

# WLXPipeline.dll

| | |
|---|---|
| **Source** | `src/WLXPipeline/` |
| **CMake target** | `WLXPipeline` |
| **Type** | Win32 DLL |
| **Family** | [Pipeline & effects targets](/modules/pipeline-effects) |

## At a glance

The **media processing pipeline factory**. Its single meaningful export,
`GetPipelineCreateFunctions`, hands callers a versioned function table for creating,
driving, and introspecting media pipeline stages.

## Public surface

From `WLXPipeline.def`:

```text
GetPipelineCreateFunctions @1
```

The returned table (from the module README):

| Field | Meaning |
|---|---|
| `uVersion` | Table version |
| `uStructSize` | Size of the struct (ABI check) |
| `pfnCreate` | Create a pipeline object |
| `pfnDestroy` | Destroy a pipeline object |
| `pfnProcess` | Process a media sample |
| `pfnGetInfo` | Query pipeline info |

## Contract coverage

`GetPipelineCreateFunctions` returns `S_OK` and a **real 6-pointer table**. Covered by
`tests/mmr-python/wmmr/contracts/wlxpipeline.py`.

:::note Reference-stub divergence

The reference parity build (`build_clean`) returns `E_NOTIMPL` from
`GetPipelineCreateFunctions` — our real table wins, and the contract pins it. This is
[Stub Design ground rule 3](../../methodology/stub-design.md#ground-rules) in action:
where the reference is stub-like, the real implementation is the oracle.

:::

## Implementation notes

- Classic module layout: `CMakeLists.txt`, `WLXPipeline.def`, `WLXPipeline.cpp/.h`,
  `dllmain.cpp`.
- `__stdcall` entry, decorated per the reference.

## Testing

- Contract: `tests/mmr-python/wmmr/contracts/wlxpipeline.py`.
- Exercise app: `apps/pipeline-graph` (submodule).

## Analysis artifacts

`analysis/WLXPipeline/`.
