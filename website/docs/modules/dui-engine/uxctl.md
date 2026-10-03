---
sidebar_position: 2
title: uxctl.dll (DUI Control Host)
description: The DirectUI control-host entry points — init, object creation, and process teardown.
---

# uxctl.dll

| | |
|---|---|
| **Source** | `src/uxctl/` |
| **CMake target** | `uxctl` |
| **Type** | Win32 DLL |
| **Family** | [DirectUI engine layer](../dui-engine) |

## At a glance

A small control-host DLL for the DirectUI stack — the process-level init/uninit and
object-creation entry points that a DirectUI host calls before using controls. In the
original architecture it pairs with `uxcore.dll` (see
[UXCore](./uxcore.md)).

## Public surface

From `uxctl.def`:

```text
UxControlsInitProcess
UxControlsCreateObject
UxControlsUninitProcess
```

All `__stdcall`.

## Implementation notes

- `UxControlsInitProcess` sets up per-process control state; `UxControlsUninitProcess`
  tears it down; `UxControlsCreateObject` is the class-factory-style creation entry.
- Like `wlidcli`, this DLL is among the **delay-loaded optional** components on some
  original setups — the engine survives its absence
  ([Quirk #11](../../methodology/quirks.md#11-delay-loaded-dlls-that-no-longer-exist)).
- Links: `kernel32`, `user32`, `gdi32`, `advapi32`, `ole32`, ... (system libs only).

## Testing

- Export-parity and decoration contracts in `tests/mmr-python`.

## Analysis artifacts

`analysis/Shared/` and the cross-DLL import matrices in `analysis/CrossDllImports/`.
