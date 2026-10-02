---
sidebar_position: 5
title: Glossary
description: Project jargon — codenames, module prefixes, and Windows Live terminology.
---

# Glossary

## Project & product names

| Term | Meaning |
|---|---|
| **WMMR** | The project's internal shorthand for this Windows Movie Maker Reconstruction |
| **Sundance** | Windows Live Movie Maker's internal Microsoft codename — leaked into runtime objects (mutex name, RTTI, class prefixes like `CSundanceApp`) |
| **16.4.3528.0331** | The exact product version being recreated (`.ship.client.main.w5m4` branch build) |
| **Windows Live Essentials 2012** | The suite that shipped Movie Maker 2012 (source of the `wlsetup-all.exe` installer the binaries came from) |

## Module prefixes

| Prefix | Meaning | Example |
|---|---|---|
| `WLX` | Windows Live eXperience (shared WL components) | `WLXPhotoBase`, `WLXPipeline` |
| `WL` | Windows Live (narrow) | `WLFacebookPlugin` |
| `WLMF` | Windows Live Media Foundation | `WLMFDS`, `WLMFReadWrite` |
| `MF` | Media Foundation | `MFReader_Open` |
| `Dmx` | Data management extension (telemetry host) | `DmxBici` |
| `Bici` | Microsoft's BI telemetry client | `BiciWrapper::StartExperience` |
| `Sqm` | Service Quality Management (CEIP telemetry) | `WLXPhotoSqm` |
| `HMR` | The engine's internal prefix | `HMREngine`, `HMRAVSource` |
| `TFX` | Transitions/effects | `GetTFXCreateFunctions` |
| `PSA` | The authentication store subsystem | `PSAStub`, `PSAAuthenticationStore` |
| `DUI` | DirectUI — Microsoft's markup-driven UI framework | `DuiDirect`, `.duxt` resources |
| `UIFILE` | DirectUI markup resource type | `RT_UIFILE_RIBBON` |

## Technical terms

| Term | Meaning |
|---|---|
| **RTTI** | Run-Time Type Information — the class names recovered from binaries (1360 unique) |
| **Export parity** | A DLL exporting the exact names/ordinals/decorations as the reference binary |
| **Contract** | An executable assertion of observable behavior in `tests/mmr-python` |
| **Parity-stub build** | The `build_clean/` DLLs — export-parity but `E_NOTIMPL` behavior; never a behavioral oracle |
| **Quirk** | An original bug/behavior deliberately preserved (23 cataloged) |
| **`E_NOTIMPL`** | `0x80004001` — "not implemented"; sometimes original behavior, sometimes a gap — see the [TODO audit](../testing/todo-audit.md) |
| **`S_FALSE`** | `0x00000001` — success-but-no-op; **intentional** COM idiom throughout |
| **VEH / SEH** | Vectored / Structured Exception Handling — see [Quirk #1](../methodology/quirks.md#1-double-exception-protection-pattern-veh--seh) |
| **WARP** | Windows Advanced Rasterization Platform — software D3D11 fallback |
| **EVR** | Enhanced Video Renderer (DirectShow) |
| **DXVA2** | DirectX Video Acceleration 2.0 |
| **X3D** | The ISO scene-graph standard HMREngine's node system is modeled on |
| **Ken Burns** | Pan/zoom effect over still photos (WLXPhotoCinematic) |
| **`.wlmp`** | Windows Live Movie Project — the project file format |
| **Ordinal** | Export table number; some consumers bind by ordinal, so they're part of parity |
| **Decoration** | The compiler-suffixed export name (`_Name@4` for `__stdcall`) |
| **Gitlink** | The parent repo's pointer to a submodule commit |
