---
sidebar_position: 9
title: Support Subsystems
description: Add-in contract, external stubs, utility containers, and resource IDs.
---

# Support Subsystems

Beyond the major subsystems, `MovieMakerCore` carries four small support directories.

## AddIn (`AddIn/`)

| File | Role |
|---|---|
| `SundanceAddInContract.h` | The add-in API contract (interfaces an add-in implements) |
| `SundanceAddInManager.h` | Loads add-in DLLs, manages lifetimes |

**Current state:** the add-in host **loads DLLs but the plugin API is inert** — a
documented stub category ([Stub Design](../../methodology/stub-design.md#stub-categories)).
Add-ins register, initialize, and shut down; no plugin commands are dispatched yet.

## External (`External/`)

Inert-but-linkable stubs for external surfaces MovieMakerCore imports but that do not
exist on modern Windows:

| Stub | Replaces |
|---|---|
| `PSAStub.cpp/.h` | The PSA authentication store — re-implemented **securely** with DPAPI (see [Security](../../reference/security.md)) |
| `DuiUtilStub.cpp/.h` | DuiDirect utility functions |
| `UIControlsStub.cpp/.h` | External UI controls |
| `DragDropStub.cpp/.h` | Drag/drop format enumeration (partial — see [TODO audit](../../testing/todo-audit.md)) |

These keep the engine linkable and runnable — the delay-load survival logic in the
launcher ensures their absence at runtime is also non-fatal.

## DataStructs (`DataStructs/`)

Utility containers used across the engine: `IntSet`, `StringSet`, and the shared
`DataStructs.h` definitions. The original used a **custom STL-like container library**
here rather than the CRT STL — preserved
([Quirk #17](../../methodology/quirks.md#17-custom-stl-like-container-library)).

## Resources (`Resources/`)

| File | Role |
|---|---|
| `ResourceIds.h` | Engine resource ID assignments (dialogs, images, UIFILEs) |
| `StringTableIds.h` | String table IDs (localizable strings — shipped in `MovieMakerLang.dll`) |

Command/resource IDs are **non-contiguous**, matching the original's organic growth
([Quirk #10](../../methodology/quirks.md#10-non-contiguous-command-ids)).

## Global singletons

The engine follows the original's **global singleton architecture** — key managers are
process-wide singletons reachable from static accessors
([Quirk #15](../../methodology/quirks.md#15-global-singleton-architecture)). This is why
threading notes in the codebase repeatedly stress which singleton methods may be called
from background workers.
