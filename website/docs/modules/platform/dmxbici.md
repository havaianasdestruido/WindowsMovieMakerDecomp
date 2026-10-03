---
sidebar_position: 7
title: DmxBici.dll (Telemetry, inert)
description: The BiciWrapper analytics surface — experience timers, data points, and streams that never transmit.
---

# DmxBici.dll

| | |
|---|---|
| **Source** | `src/DmxBici/` |
| **CMake target** | `DmxBici` |
| **Type** | Win32 DLL (**inert telemetry**) |
| **Family** | [Platform services targets](../platform) |

## At a glance

The **Bici analytics wrapper** ("Bici" = Microsoft's BI telemetry client; "Dmx" = the
data-management extension that hosted it). The original wrapped experience tracking —
start/end experience, data points, timers — for Microsoft's telemetry pipeline.

As with [WLXPhotoSqm](./wlxphotosqm.md), the reconstruction keeps the **full export
surface but makes it inert** — nothing is transmitted. Return behavior follows each
export's signature: the `HRESULT` (`LONG`) exports such as `SetAnid` and
`StartExperience` return `S_OK`; the `BOOL` exports such as `Set`, `TimerStart`, and
`TransferExperienceToApp`/`TransferExperienceToWeb` return `TRUE`; the `void`
`AddToStream` is a no-op.

## Public surface

C++-mangled `BiciWrapper::` exports aliased to flat names (`DmxBici.def`):

```text
?AddStringToDataPoint@BiciWrapper@@YG_NKKPB_W@Z = _BiciWrapper_AddStringToDataPoint@12 @1
?AddToAverage@BiciWrapper@@YG_NKK@Z             = _BiciWrapper_AddToAverage@8          @2
?AddToDataPoint@BiciWrapper@@YG_NKKK@Z          = _BiciWrapper_AddToDataPoint@12       @3
?AddToStream@BiciWrapper@@YGXKPBVTuple@1@@Z     = _BiciWrapper_AddToStream@8           @4
?EndExperience@BiciWrapper@@YGJXZ               = _BiciWrapper_EndExperience@0         @5
?Increment@BiciWrapper@@YG_NKK@Z                = _BiciWrapper_Increment@8             @6
?Set@BiciWrapper@@YG_NKK@Z                      = _BiciWrapper_Set@8                   @7
?SetAnid@BiciWrapper@@YGJPB_W@Z                 = _BiciWrapper_SetAnid@4               @8
?SetIfMax@BiciWrapper@@YG_NKK@Z                 = _BiciWrapper_SetIfMax@8              @9
?SetIfMin@BiciWrapper@@YG_NKK@Z                 = _BiciWrapper_SetIfMin@8              @10
?SetString@BiciWrapper@@YG_NKPB_W@Z             = _BiciWrapper_SetString@8             @11
?StartExperience@BiciWrapper@@YGJW4BiciStartupId@1@@Z = _BiciWrapper_StartExperienceWithId@4 @12
?StartExperience@BiciWrapper@@YGJXZ             = _BiciWrapper_StartExperience@0       @13
?TimerAccumulate@BiciWrapper@@YG_NK@Z           = _BiciWrapper_TimerAccumulate@4      @14
?TimerRecord@BiciWrapper@@YG_NK@Z               = _BiciWrapper_TimerRecord@4          @15
?TimerStart@BiciWrapper@@YG_NK@Z                = _BiciWrapper_TimerStart@4           @16
?TransferExperienceToApp@BiciWrapper@@YG_NPAPA_W@Z    = _BiciWrapper_TransferExperienceToApp@4 @17
?TransferExperienceToAppId@BiciWrapper@@YG_NK@Z       = _BiciWrapper_TransferExperienceToAppId@4 @18
```

## Implementation notes

- The API surface is **experience-oriented**: `StartExperience`/`EndExperience` bracket a
  user session; `TimerStart/Record/Accumulate` time operations; `AddToStream/Tuple`
  batch metrics; `TransferExperienceToApp(Id)` hands the session to the host app.
- Note the two `StartExperience` overloads (with and without `BiciStartupId`) — both
  mangled forms are exported, as in the original.
- Inert-but-safe is mandatory for this category: return `S_OK`/`TRUE`, keep internal
  state, never open a socket, never write telemetry files.

## Testing

- Contract coverage pins inert behavior + export parity in `tests/mmr-python`.
- `tests/DmxBici/` — per-module C++ tests.

## Analysis artifacts

`analysis/DmxBici/`.
