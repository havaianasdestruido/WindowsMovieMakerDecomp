---
sidebar_position: 4
title: MetadataSys.dll
description: The Windows property handler — WLXPSGetItemPropertyHandler over the shell property system.
---

# MetadataSys.dll

| | |
|---|---|
| **Source** | `src/MetadataSys/` |
| **CMake target** | `MetadataSys` |
| **Type** | Win32 COM DLL |
| **Family** | [Platform services targets](/docs/modules/platform) |

## At a glance

The **property handler** DLL: exposes media metadata through the Windows property
system (`propsys`) so Explorer and other apps can read/write the metadata Movie Maker
cares about (title, rating, tags, ...).

## Public surface

From `MetadataSys.def`:

```text
WLXPSGetItemPropertyHandler @1
DllCanUnloadNow            @2
DllGetClassObject          @3
DllRegisterServer          @4
DllUnregisterServer        @5
```

## Implementation notes

- `WLXPSGetItemPropertyHandler` is the primary API entry (the "WLXPS" prefix = Windows
  Live eXPerience property system). It returns the handler for a given item's metadata.
- Links `propsys` (shell property system) — one of the few modules with a shell-system
  dependency beyond the common set.
- **Metadata writes and thumbnail fallback behavior** are a current reconstruction
  priority (see the [TODO audit](../../testing/todo-audit.md)) — reads are real, some
  write/fallback paths remain guarded stubs.
- Registry/property-store writes are real but **guarded** — the "Registry ops" category
  from [Stub Design](../../methodology/stub-design.md#stub-categories).

## Testing

- Contract coverage for the handler entry + COM quartet in `tests/mmr-python`.
- Exercise app: `apps/metadata-editor` (submodule).

## Analysis artifacts

`analysis/MetadataSys/`, `analysis/RegRes/`.
