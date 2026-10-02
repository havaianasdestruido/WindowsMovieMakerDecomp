---
sidebar_position: 4
title: WLXPhotoCinematic.dll
description: Cinematic effects — Ken Burns pan/zoom computation with guarded COM registration.
---

# WLXPhotoCinematic.dll

| | |
|---|---|
| **Source** | `src/WLXPhotoCinematic/` |
| **CMake target** | `WLXPhotoCinematic` |
| **Type** | Win32 COM DLL |
| **Family** | [Pipeline & effects targets](/docs/modules/pipeline-effects) |

## At a glance

The **cinematic effects** module — best known for the **Ken Burns** effect: computing
smooth pan/zoom camera moves over still photos (which regions to visit, at what zoom,
over what duration). Consumed by slideshow and movie generation.

## Public surface

The standard COM quartet (`WLXPhotoCinematic.def`):

```text
DllCanUnloadNow     DllGetClassObject
DllRegisterServer  DllUnregisterServer
```

## Registration semantics (documented category)

`DllRegisterServer` performs **real registry writes** under
`HKEY_LOCAL_MACHINE\Software\Microsoft\Windows Photo Gallery\Slideshow\Themes\{CLSID}`
(the original binary's `.rgs` location), writing `InprocServer32` and
`ThreadingModel=Apartment` for the two servable CLSIDs
(`CLSID_CinematicFullScreen`, `CLSID_PanZoomTransform`). The result is
**elevation-dependent** — writing HKLM without admin rights fails (the original
returned `E_ACCESSDENIED` there as well), and the DLL reports that honestly. This is
the "Registry ops" row of the
[stub-category table](../../methodology/stub-design.md#stub-categories): real writes,
guarded, elevation-dependent results documented.

## Implementation notes

- Pan/zoom computation works on **downscaled analysis buffers** (bounded resource use —
  same policy as face recognition; see [Security](../../reference/security.md)).
- COM factory real and contract-pinned.

## Testing

- Contract coverage for registration/factory semantics in `tests/mmr-python` — including
  the elevation-dependent behavior.

## Analysis artifacts

`analysis/WLXPhotoCinematic/` and `analysis/RegRes/` (registry scripts).
