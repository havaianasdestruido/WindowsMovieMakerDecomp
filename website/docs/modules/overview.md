---
sidebar_position: 0
title: Module Reference Overview
description: The index of all 29 build targets and how to read their documentation pages.
---

# Module Reference Overview

Every CMake target in the reconstruction has its own page, grouped into five families:

| Family | Targets | Focus |
|---|---|---|
| [Application](application/) | 4 | The launcher, the engine, localization resources, preview client |
| [DirectUI engine layer](dui-engine/) | 7 | UXCore, uxctl, and the DirectUI-namespace engine libraries |
| [Media Foundation](media-foundation/) | 5 | MF bridge DLLs, MP4 parser, transcode/codec hosts |
| [Pipeline & effects](pipeline-effects/) | 6 | Media processing, transitions, slideshow, trim, movie library |
| [Platform services](platform/) | 7 | Foundation library, publishing, recognition, metadata, identity, telemetry |

## Full target table

| Target | Output | Type | Family | Page |
|---|---|---|---|---|
| `MovieMaker` | `MovieMaker.exe` | EXE (launcher) | Application | [MovieMaker.exe](application/moviemaker-exe.md) |
| `MovieMakerCore` | `MovieMakerCore.dll` | DLL (engine) | Application | [MovieMakerCore.dll](application/moviemakercore.md) |
| `MovieMakerLang` | `MovieMakerLang.dll` | DLL (resource-only) | Application | [MovieMakerLang.dll](application/moviemakerlang.md) |
| `MovieMakerPreviewClient` | `MovieMakerPreviewClient.dll` | DLL | Application | [MovieMakerPreviewClient.dll](application/moviemakerpreviewclient.md) |
| `UXCore` | `UXCore.dll` | DLL | DirectUI engine | [UXCore.dll](dui-engine/uxcore.md) |
| `uxctl` | `uxctl.dll` | DLL | DirectUI engine | [uxctl.dll](dui-engine/uxctl.md) |
| `MediaCatalog` | `MediaCatalog.lib` | static lib | DirectUI engine | [MediaCatalog](dui-engine/mediacatalog.md) |
| `ProjectManager` | `ProjectManager.lib` | static lib | DirectUI engine | [ProjectManager](dui-engine/projectmanager.md) |
| `TimelineEngine` | `TimelineEngine.lib` | static lib | DirectUI engine | [TimelineEngine](dui-engine/timelineengine.md) |
| `GPURenderer` | `GPURenderer.dll` | DLL | DirectUI engine | [GPURenderer.dll](dui-engine/gpurenderer.md) |
| `PlaybackEngine` | `PlaybackEngine.lib` | static lib | DirectUI engine | [PlaybackEngine](dui-engine/playbackengine.md) |
| `WLMFDS` | `WLMFDS.dll` | DLL | Media Foundation | [WLMFDS.dll](media-foundation/wlmfds.md) |
| `WLMFReadWrite` | `WLMFReadWrite.dll` | DLL | Media Foundation | [WLMFReadWrite.dll](media-foundation/wlmfreadwrite.md) |
| `WLXMP4Parser` | `WLXMP4Parser.dll` | DLL | Media Foundation | [WLXMP4Parser.dll](media-foundation/wlxmp4parser.md) |
| `WLXTranscode` | `WLXTranscode.exe` | EXE | Media Foundation | [WLXTranscode.exe](media-foundation/wlxtranscode.md) |
| `WLXCodecHost` | `WLXCodecHost.exe` | EXE | Media Foundation | [WLXCodecHost.exe](media-foundation/wlxcodechost.md) |
| `WLXPipeline` | `WLXPipeline.dll` | DLL | Pipeline & effects | [WLXPipeline.dll](pipeline-effects/wlxpipeline.md) |
| `WLXPipetran` | `WLXPipetran.dll` | DLL | Pipeline & effects | [WLXPipetran.dll](pipeline-effects/wlxpipetran.md) |
| `WLXSlideshow` | `WLXSlideshow.dll` | DLL | Pipeline & effects | [WLXSlideshow.dll](pipeline-effects/wlxslideshow.md) |
| `WLXPhotoCinematic` | `WLXPhotoCinematic.dll` | DLL | Pipeline & effects | [WLXPhotoCinematic.dll](pipeline-effects/wlxphotocinematic.md) |
| `WLXVideoTrim` | `WLXVideoTrim.dll` | DLL | Pipeline & effects | [WLXVideoTrim.dll](pipeline-effects/wlxvideotrim.md) |
| `WLXMovieLibrary` | `WLXMovieLibrary.dll` | DLL | Pipeline & effects | [WLXMovieLibrary.dll](pipeline-effects/wlxmovielibrary.md) |
| `WLXPhotoBase` | `WLXPhotoBase.dll` | DLL | Platform | [WLXPhotoBase.dll](platform/wlxphotobase.md) |
| `WLXMediaPublishSubscribe` | `WLXMediaPublishSubscribe.dll` | DLL | Platform | [WLXMediaPublishSubscribe.dll](platform/wlxmediapublishsubscribe.md) |
| `WLXFaceRecognition` | `WLXFaceRecognition.dll` | DLL | Platform | [WLXFaceRecognition.dll](platform/wlxfacerecognition.md) |
| `MetadataSys` | `MetadataSys.dll` | DLL | Platform | [MetadataSys.dll](platform/metadatasys.md) |
| `wlidcli` | `wlidcli.dll` | DLL | Platform | [wlidcli.dll](platform/wlidcli.md) |
| `WLXPhotoSqm` | `WLXPhotoSqm.dll` | DLL (inert telemetry) | Platform | [WLXPhotoSqm.dll](platform/wlxphotosqm.md) |
| `DmxBici` | `DmxBici.dll` | DLL (inert telemetry) | Platform | [DmxBici.dll](platform/dmxbici.md) |

## How to read a module page

Each page follows a consistent template:

1. **At a glance** — output name, type, family, and one-line purpose.
2. **Public surface** — exports (from the `.def` file where present), COM classes,
   calling-convention notes.
3. **Implementation notes** — what is real, what is a documented stub, and why.
4. **Testing** — contract coverage in `tests/mmr-python` and any module test binary.
5. **Analysis artifacts** — the matching `analysis/<Module>/` directory.

:::tip Source of truth for module facts

The per-module pages summarize the code. For authoritative detail, consult:

- the module's own `src/<Module>/README.md`,
- the corresponding `analysis/<Module>/` binary-analysis directory, and
- `tests/mmr-python/wmmr/contracts/` for pinned behavior.

:::
