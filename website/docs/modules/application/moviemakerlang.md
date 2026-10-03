---
sidebar_position: 3
title: MovieMakerLang.dll (Resources)
description: The resource-only localization DLL — string tables, UIFILE markup, icons, and the version manifest.
---

# MovieMakerLang.dll

| | |
|---|---|
| **Source** | `src/MovieMakerLang/` |
| **CMake target** | `MovieMakerLang` |
| **Type** | Resource-only DLL (216 KB in the original, **zero code**) |
| **Family** | [Application targets](../application) |

## At a glance

A pure resource DLL: **0 exports, 0 code bytes** in the reference binary. All localizable
and layout resources for the application ship here, which is why the engine can be updated
without touching resources.

## Resource map

From `MovieMakerLangResource.h`:

| Range | Contents |
|---|---|
| `IDR_VERSION` (1), `IDR_MANIFEST` (2) | Version info + manifest |
| `IDI_MAIN_ICON` (100), `IDI_SMALL_ICON` (101) | Application icons |
| `IDC_APPSTARTING` / `IDC_WAIT` (200–201) | Cursors |
| `IDB_SPLASH`, `IDB_TOOLBAR`, `IDB_TOOLBAR_DARK`, `IDB_WELCOME_BANNER` (300–303) | Bitmaps |
| `RT_UIFILE_RIBBON` (10000) | **Ribbon markup** (consumed by the Windows Ribbon Framework — see [UI subsystem](../../architecture/moviemakercore/ui.md)) |
| `RT_UIFILE_MAIN` (10001) | Main window DirectUI markup |
| `RT_UIFILE_DIALOGS` (10002) | Dialog markup |

String table IDs referenced by the engine live in
`src/MovieMakerCore/Resources/StringTableIds.h`.

## Implementation notes

- The reconstruction reproduces the resource **layout and IDs**; bitmaps/icons are
  placeholders drawn for the same resource slots.
- The "SkyDrive" branding appears in string tables as in the original — a historical
  anachronism kept on purpose
  ([Quirk #7](../../methodology/quirks.md#7-skydrive-branding-historical-anachronism)).
- Being resource-only, the DLL has no COM registration and no `.def` file.

## Testing

- Contract suite verifies loadability and the presence of the UIFILE resources
  (`tests/mmr-python/wmmr/contracts/`).

## Analysis artifacts

`analysis/MovieMakerLang/` and `analysis/RegRes/` (registry + resource extraction across
binaries).
