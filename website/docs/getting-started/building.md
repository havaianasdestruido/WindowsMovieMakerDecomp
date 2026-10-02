---
sidebar_position: 3
title: Building
description: Configure and build all 29 CMake targets of the WMMR reconstruction.
---

# Building

## Configure

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
```

Key points:

- The generator **must** be `Visual Studio 17 2022` with the **Win32** architecture.
- `CMAKE_RUNTIME_OUTPUT_DIRECTORY` is set to `build/bin`, so all binaries land in
  `build/bin/<Config>/` regardless of which module produced them.
- Common compile definitions are set globally: `UNICODE`/`_UNICODE`, `WINVER=0x0601`,
  `NOMINMAX`, `WIN32_LEAN_AND_MEAN`, `_CRT_SECURE_NO_WARNINGS`, plus the version macros
  (`VERSION_MAJOR=16`, `VERSION_MINOR=4`, `VERSION_BUILD=3528`, `VERSION_REVISION=331`).

## Build

```powershell
# Everything (Debug)
cmake --build build --config Debug

# Everything (Release)
cmake --build build --config Release

# A single target
cmake --build build --config Debug --target MovieMakerCore --parallel
```

Or use the wrapper script:

```powershell
.\tools\build.ps1                      # Full Debug build
.\tools\build.ps1 -Config Release      # Release
.\tools\build.ps1 -Target MovieMakerCore   # Single target
.\tools\build.ps1 -Clean               # Clean build
.\tools\build.ps1 -Analyze             # Build + run binary analysis
```

## Build outputs

All 29 targets write to `build/bin/Debug/`. The headline outputs:

| Output | Target type | Role |
|---|---|---|
| `MovieMaker.exe` | Launcher EXE (~94 KB) | Thin WinMain launcher; delay-loads `MovieMakerCore.dll` |
| `MovieMakerCore.dll` | Engine DLL (~1.4 MB) | The entire application: app framework, project model, rendering, UI |
| `WLXPhotoBase.dll` | Foundation DLL | Shared base library used by all WLX components |
| `MovieMakerLang.dll` | Resource-only DLL | Localization strings, UIFILEs, icons |
| `WLXPipeline.dll`, `WLXPipetran.dll`, `WLXVideoTrim.dll`, `WLXMovieLibrary.dll`, `WLXSlideshow.dll`, `WLXPhotoCinematic.dll`, `WLXMediaPublishSubscribe.dll`, `WLXFaceRecognition.dll`, `WLXMP4Parser.dll`, `WLMFDS.dll`, `WLMFReadWrite.dll`, `MovieMakerPreviewClient.dll` | Supporting DLLs | Media pipeline, effects, publishing, parsing |
| `WLXTranscode.exe`, `WLXCodecHost.exe` | Supporting EXEs | Transcode tool and out-of-process codec host |
| `UXCore.dll`, `uxctl.dll`, `GPURenderer.dll`, `MediaCatalog.lib`, `ProjectManager.lib`, `TimelineEngine.lib`, `PlaybackEngine.lib`, `DmxBici.dll`, `MetadataSys.dll`, `wlidcli.dll`, `WLXPhotoSqm.dll` | Engine-layer & platform targets | DirectUI engine layer, telemetry, metadata, identity |

The full table with per-module documentation is in the [Module Reference](../modules/overview.md).

## What a clean build looks like

A successful full build reports **0 compile errors, 0 link errors, 0 RC errors**, e.g.:

```text
MovieMakerCore.vcxproj -> build\bin\Debug\MovieMakerCore.dll
MovieMaker.vcxproj -> build\bin\Debug\MovieMaker.exe
...
Build succeeded.
    0 Warning(s)
    0 Error(s)
```

:::warning Don't panic about `build_clean`

A second directory, `build_clean/` (gitignored), holds the **reference/parity-stub DLLs**
used for export comparison. Those return `E_NOTIMPL` from many entry points by design —
they are an export-parity source only, never a behavioral oracle. If you find yourself
testing against `build_clean\bin\Debug`, you are testing the wrong binaries.

:::

## CI

GitHub Actions (`.github/workflows/build-projects.yml`) builds every target on a
`windows-2022` runner matrix — the runner is pinned because the project requires the
VS2022 Win32 toolchain. Documentation deployment is handled separately by
[docs deployment](overview.md) (see `.github/workflows/docs.yml`).
