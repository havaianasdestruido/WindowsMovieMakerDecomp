---
sidebar_position: 4
title: HMRAVSource (Media Foundation Pipeline)
description: The A/V pipeline — Media Foundation sources, sinks, transcoding, capture, and the audio DSP stack.
---

# HMRAVSource

**Location:** `src/MovieMakerCore/HMRAVSource/`

HMRAVSource is the audio/video pipeline built on **Media Foundation** (with a DirectShow
compatibility path), recovered from RTTI classes such as `AVSource`, `MFSource`,
`DShowSource`, `EncodeProfile`, `StreamSink`, `TranscodeManager`, `VideoCapture`.

## Tree (selected)

```text
HMRAVSource/
  HMRAVSource.cpp/.h                 # pipeline facade
  AVSource / AVSourceFactory / AVSourceProxy / AVSink
  MFSource.cpp/.h                    # Media Foundation source creation
  MFSourceReaderBuilder / NativeMFSourceReaderBuilder /
  DShowMFSourceReaderBuilder / AsyncSourceResolver    # reader strategies
  MFByteStreamOnStream               # IMFByteStream over an IStream
  StreamSink.cpp/.h                  # sample processing sink
  StreamSinkHost / StreamSinkHelper
  TextureInterop.cpp/.h              # DX11 texture sharing with MF
  VideoProc.cpp/.h                   # DXVA2 / XVideoProc conversion
  TranscodeManager.cpp               # transcoding pipeline
  TranscodeMetadata / EncodeProfile / SAXProfileBuilder  # profile/metadata
  SyncVideoSource / CachedWFSection / ImageThumbnail
  VideoCapture / AudioCapture / AVCaptureSession        # capture support
  AuthProvider.cpp/.h                # in-memory credential cache (never persisted)
  dxva2stubs.cpp                     # DXVA2 compatibility stubs
  Audio/                             # audio DSP: fade, ducking, channel mapping,
                                     #   AudioOutput, AudioQueue, Mixer, Waveform
```

## Source resolution strategy

Media resolution is strategy-based — a chain of reader builders:

```mermaid
flowchart LR
    REQ["Resolve media URL"] --> NATIVE["NativeMFSourceReaderBuilder<br/>(native MF)"]
    NATIVE -->|not supported| DSHOWB["DShowMFSourceReaderBuilder<br/>(DirectShow via WLMFDS bridge)"]
    DSHOWB -->|fallback| PROXY["AVSourceProxy<br/>(async resolve, AVSourceFactory)"]
    PROXY --> SRC["IMFSourceReader ready"]
```

`AsyncSourceResolver` performs the resolution off the UI thread.

## Rendering integration

- **TextureInterop** moves decoded frames from MF surfaces into D3D11 textures shared with
  HMREngine (and the preview presenter), handling format conversion through `VideoProc`
  (DXVA2 video processor where available, via `dxva2stubs.cpp` compatibility shims).
- **TranscodeManager** drives export encodes using `EncodeProfile` (built from
  service-specific profiles by `SAXProfileBuilder`), coordinating with `WLXTranscode.exe`
  / `WLXCodecHost.exe` for out-of-process work.

## Audio subsystem (`Audio/`)

| Component | Role |
|---|---|
| `AudioFadeProcessor` | fade-in/fade-out envelope application |
| `AudioDuckingProcessor` | background-music ducking under narration (classic Movie Maker feature) |
| `AudioChannelMapper` | channel remapping (stereo↔mono, 5.1 downmix) |
| `AudioOutput` / `AudioQueue` | device output and buffering |
| `Mixer` | multi-track mixing |
| `Waveform` | waveform extraction for timeline display |

## Credentials

`AuthProvider` caches auth tokens **in memory only** (cleared on shutdown); the persistent
store is the DPAPI-encrypted `PSAAuthenticationStore` (see
[Security](../../reference/security.md)). This split is deliberate: the original kept
tokens alive for the session, and the reconstruction refuses to persist them in
plaintext.

## Known limitations (documented stubs)

- The full export rendering pipeline writes **verified test frames, not real video** —
  the highest-priority remaining reconstruction work (see the
  [TODO audit](../../testing/todo-audit.md) and [Stub Design](../../methodology/stub-design.md#stub-categories)).
- Source-image thumbnails and per-transition rendering details are partial.
