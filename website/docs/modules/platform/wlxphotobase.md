---
sidebar_position: 1
title: WLXPhotoBase.dll (Foundation)
description: The shared foundation library — Base namespace exceptions, memory, OS checks, GDI+ helpers, 56 exports.
---

# WLXPhotoBase.dll

| | |
|---|---|
| **Source** | `src/WLXPhotoBase/` |
| **CMake target** | `WLXPhotoBase` |
| **Type** | Win32 DLL (56 KB / 21 KB code in the original) |
| **Family** | [Platform services targets](/docs/modules/platform) |

## At a glance

The **shared foundation library** for all WLX (Windows Live ...) components. Everything
else links it. Contents per the public header: base exception handling, memory
management, OS detection, GDI+ integration, smart pointers, containers, and common
utilities — exported as **56 C++ mangled symbols in the `Base` namespace**.

## Public surface

C++-mangled exports (from `WLXPhotoBase.def`), the `Base::` namespace surface. Because
they are mangled, parity means matching the **exact mangled names** — the `.def` file
lists them verbatim. Key families:

- `Base::Exception` + `Throw` / `ThrowLastError` — the error backbone
- Memory & string management helpers
- OS version/capability checks
- GDI+ init and drawing helpers
- Smart pointer and container utilities

## Implementation notes

- `BaseTypes.h` carries the shared value types; `WLXPhotoBase.cpp` the implementations.
- **First target in the build order** (the root `CMakeLists.txt` adds it first) —
  everything may depend on it, it depends on nothing but the OS.
- Callers use `Base::Throw`/`ThrowLastError` for errors that cross module boundaries in
  WLXPhotoBase land (vs. `WMMR::MovieMakerException` in shared reconstruction code).

## Testing

- Contract coverage in `tests/mmr-python` (mangled-name parity + behavior of the
  exception/memory surface).
- `tests/SharedMFDlls/` covers shared-lib interactions.

## Analysis artifacts

`analysis/WLXPhotoBase/` (PDB GUID `{0674DC61-4F42-4D44-AD50-155EB0251FA5}`).
