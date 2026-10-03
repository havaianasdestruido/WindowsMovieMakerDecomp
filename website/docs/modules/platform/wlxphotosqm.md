---
sidebar_position: 6
title: WLXPhotoSqm.dll (Telemetry, inert)
description: The SQM/CEIP telemetry surface — full Sqm namespace exports that never send data off-machine.
---

# WLXPhotoSqm.dll

| | |
|---|---|
| **Source** | `src/WLXPhotoSqm/` |
| **CMake target** | `WLXPhotoSqm` |
| **Type** | Win32 DLL (**inert telemetry**) |
| **Family** | [Platform services targets](../platform) |

## At a glance

The **SQM (Service Quality Management / CEIP) telemetry** surface. The original DLL
exported a large `Sqm::` namespace API (streams, timers, averages, medians, deferred
reports) that Windows Live apps used to phone home quality metrics.

**In the reconstruction, telemetry is a documented stub category: the exports are inert
and no data ever leaves the machine** — see
[Stub Design](../../methodology/stub-design.md#stub-categories). Return behavior follows
each export's signature: the `void` setters and stream/timer functions are no-ops; the
`DWORD` query `Sqm_GetOptInState` returns `0`; the `BOOL` queries (`Sqm_IsEnabled`,
`Sqm_IsStreamTimerActive`, `Sqm_IsStreamTimerDataSet`) return `FALSE`.

## Public surface

C++-mangled `Sqm::` exports, aliased to stable undecorated names
(`WLXPhotoSqm.def`), including overloads distinguished by suffix:

```text
?AddToAverage@Sqm@@YGXKK@Z            = _Sqm_AddToAverage@8      @4
?AddToStream@Sqm@@YGXKK@Z             = _Sqm_AddToStream@8       @5
?AddToStream@Sqm@@YGXKKK@Z            = _Sqm_AddToStream3@12     @6
?AddToStream@Sqm@@YGXKKKPB_W@Z        = _Sqm_AddToStreamW@12     @9
?AddStreamTimerData@Sqm@@YGXKKPB_W@Z  = _Sqm_AddStreamTimerDataW@12 @3
?AbortStreamTimer@Sqm@@YGXKK@Z        = _Sqm_AbortStreamTimer@8  @1
?DeferAddToAverage@Sqm@@YGXKK@Z       = _Sqm_DeferAddToAverage@8 @12
... (full table in src/WLXPhotoSqm/WLXPhotoSqm.def)
```

## Implementation notes

- The **mangled-name alias pattern** preserves both the original C++ symbol and a stable
  flat name — matching the original's export table shape.
- Overload sets (`AddToStream` with 2/3/4 args, wide-string variants, tuple variants)
  must all exist with correct decorations — parity is checked by
  `tools/diff_exports.py`.
- Delay-load optional at runtime: if absent, callers' telemetry calls degrade to no-ops
  via the delay-load failure hooks.

## Testing

- Contract coverage pins the inert behavior (calls succeed, nothing is sent) in
  `tests/mmr-python`.

## Analysis artifacts

`analysis/WLXPhotoSqm/` and `analysis/E_NOTIMPL/` (the audit that separated intentional
`S_OK` stubs from genuine masking bugs).
