---
sidebar_position: 5
title: UI (DirectUI, Ribbon & Behaviors)
description: The DirectUI integration, 67-command Ribbon, and the 37 behavior classes that wire markup to the model.
---

# UI Subsystem

**Location:** `src/MovieMakerCore/UI/`

Movie Maker 2012 is a **DirectUI** application: native Win32 controls are mostly absent;
instead, markup-driven element trees (UIFILE resources) are animated by behavior classes,
with the **Windows Ribbon Framework** providing the command surface.

## Tree

```text
UI/
  Ribbon/                    # Windows Ribbon framework integration
  UIBehaviorClasses.cpp/.h   # 50+ behavior implementations across 37 classes
  SundanceBehaviors.cpp/.h   # app-specific behaviors
  LayoutBehaviors.cpp/.h     # layout behaviors
  EditingBehaviors.cpp/.h    # rich edit, options dialog, help
  SpecialBehaviors.cpp/.h    # specialized behaviors
  TimelineBehavior.cpp/.h    # timeline surface behavior
  TimelineDataSources.cpp/.h # timeline item data sources
  SliderUI.cpp/.h            # custom slider
  ProgressUI.cpp/.h          # progress surfaces
  Dialogs.cpp/.h             # dialog hosting
  SundanceNativeHwndHost     # hosts native HWNDs inside DirectUI
  SundanceClipboardChainWindow # clipboard-viewer chain window
  SundanceUIComponents       # shared UI components
  UXBrush.cpp/.h             # drawing brushes
  DuiInterfaces.h            # DirectUI interface shims
  Legacy/                    # legacy (dead) UI paths — shipped but unused
  SundanceResourceIds.h      # resource ID assignments
```

## The Ribbon

`UI/Ribbon/` implements the ribbon application model on `uiribbon.h`:

- **67 command handlers**, one class per functional group, each implementing
  `IUICommandHandler::Execute` / `UpdateProperty`.
- Full **`UpdateState` / `ComputeCommandEnabled`** pass — command enablement is computed
  centrally from app state (selection, playback, project dirty state) rather than
  scattered.
- **MRU site registry** — the recent-projects list is a site registry persisted to the
  registry; its "Count" query failure path returns the Win32 error (hardening fix from
  the 2026-09 pass).
- The ribbon markup is `RT_UIFILE_RIBBON` (resource 10000) shipped in `MovieMakerLang.dll`.

Ribbon SDK quirks are preserved/documented: the Win10 SDK removed several ribbon APIs and
`uiribbon.lib` GUIDs had to be defined manually
([Quirk #6](../../methodology/quirks.md#6-removed-win10-ribbon-sdk-apis) and
[SDK Compatibility](../../reference/sdk-compatibility.md)); the Vista Basic workaround is
[Quirk #12](../../methodology/quirks.md#12-vista-basic-ribbon-workaround).

## Behavior classes

Behaviors are the DirectUI idiom: a behavior attaches to an element (by `class` attribute
in the UIFILE) and reacts to its events. The reconstruction carries **37 behavior classes
with 50+ `OnMessage` implementations**, covering:

- selection & drag on the storyboard/timeline (`TimelineBehavior`, `TimelineDataSources`)
- ribbon-adjacent widgets (`SliderUI`, `ProgressUI`)
- editing surfaces (`EditingBehaviors`: rich edit, options dialog, help)
- window choreography (`LayoutBehaviors`, `SundanceNativeHwndHost`)

## DirectUI itself

The DirectUI framework runtime is **not** part of this repo's engine — the original
shipped `uxcore.dll` / `uxctl.dll` / `DuiDirect.dll` as separate components. The
reconstruction provides:

- `src/UXCore/` — a reconstructed DirectUI core (`Element`, `HWNDElement`, containers,
  controls, parser — 14 files), consumed by MovieMakerCore's UI layer and by the engine
  layer targets (see [UXCore](../../modules/dui-engine/uxcore.md)).
- `src/uxctl/` — the `UxControlsInitProcess` / `UxControlsCreateObject` /
  `UxControlsUninitProcess` control-host entry points.
- `src/MovieMakerCore/External/` — inert stubs for the parts of the external DirectUI
  surface MovieMakerCore imports (`DuiUtilStub`, `UIControlsStub`, `DragDropStub`,
  `PSAStub`) so the engine links and runs even though the real externals are absent on
  modern Windows.

## Command IDs

Command IDs are **non-contiguous** in the original resource space — preserved faithfully
([Quirk #10](../../methodology/quirks.md#10-non-contiguous-command-ids)); see
`UI/SundanceResourceIds.h` and `Resources/ResourceIds.h`.
