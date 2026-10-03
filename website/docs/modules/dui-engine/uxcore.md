---
sidebar_position: 1
title: UXCore.dll (DirectUI Core)
description: The reconstructed DirectUI core — Element tree, containers, controls, and the DUI parser.
---

# UXCore.dll

| | |
|---|---|
| **Source** | `src/UXCore/` (14 files) |
| **CMake target** | `UXCore` |
| **Type** | Win32 DLL |
| **Family** | [DirectUI engine layer](/modules/dui-engine) |

## At a glance

The reconstructed **DirectUI core**: the element system Movie Maker's UI is built on.
DirectUI ("DUI") is the markup-driven UI framework Microsoft used across Windows Live
applications — elements, not Win32 controls, with behaviors attached via class names.

The original shipped as the system `uxcore.dll` (not present on modern Windows); this
target reconstructs the subset the application depends on, in the `DirectUI` namespace.

## Public surface

Exported via `__declspec(dllexport)` on the classes (no `.def` file):

| Header | Contents |
|---|---|
| `Element.h` | The base `Element` class — the DUI element tree node |
| `Containers.h` | `HWNDElement` (root HWND hosting) and container elements |
| `Controls.h` | Native-feeling DUI controls |
| `Resources.cpp` | Resource/registry helpers (**`__cdecl`** — one of the two documented non-`__stdcall` exception families; see [Calling Conventions](../../architecture/calling-conventions.md)) |

## Implementation notes

- `HWNDElement` owns the root `HWND` and key-focus tracking — the bridge between Win32
  messages and the DUI element tree.
- A `CRMDUIParser` (referenced from `Containers.h`) parses DUI markup resources into the
  element tree.
- DirectUI `.duxt` resource loading is a **known partial area** — tracked as a top
  priority in the [TODO audit](../../testing/todo-audit.md).
- MovieMakerCore's UI layer consumes this core; the external surface it does not need is
  stubbed in `src/MovieMakerCore/External/` (see
  [Support Subsystems](../../architecture/moviemakercore/support-subsystems.md)).

## Testing

- Contracts in `tests/mmr-python` cover the exported entry points and decoration parity.
- `tests/OtherDlls/` includes UXCore-adjacent checks.

## Analysis artifacts

`analysis/SharedMFDlls/`, `analysis/Shared/`, and the UXCore sections of
`analysis/CrossDllImports/`.
