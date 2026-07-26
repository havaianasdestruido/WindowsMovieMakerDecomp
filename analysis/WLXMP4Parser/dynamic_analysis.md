# WLXMP4Parser.dll — Dynamic Analysis Report

## Overview
- **DLL**: WLXMP4Parser.dll (Windows Photo Gallery MPEG-4 Atom Parser)
- **PDB**: WLXMP4Parser.pdb `{A4577D2C-5400-4F17-91C0-06B5EA1BE4DE}`
- **Date**: Tue Apr 1 01:28:23 2014
- **Architecture**: x86 (PE32), MSVC 11.00, MSVCR110.dll
- **Subsystem**: Windows GUI (DirectShow filter)
- **Image Base**: 0x10000000, Size: 0x2E000

## Export Table (9 functions)

| Ordinal | Name | RVA | Purpose |
|---------|------|-----|---------|
| 1 | `AddMP4SourceFilter` | 0x0001541E | Add MP4 source filter to DirectShow graph |
| 2 | `BuildMP4FilterGraph` | 0x00015132 | Build complete MP4 filter graph |
| 3 | `BuildMP4PlayBack` | 0x0001533A | Build MP4 playback pipeline |
| 4 | `IsMP4FilePlayable` | 0x00014F4E | Quick probe if MP4 file is playable |
| 5 | `DllCanUnloadNow` | 0x0001D16A | COM: check if DLL can unload |
| 6 | `DllGetClassObject` | 0x0001D1A3 | COM: class factory |
| 7 | `DllMain` | 0x000086B6 | DLL entry point |
| 8 | `DllRegisterServer` | 0x000085F7 | COM: self-registration |
| 9 | `DllUnregisterServer` | 0x00008604 | COM: unregistration |

### Export Analysis
- **AddMP4SourceFilter**: Creates a source filter for an MP4 file, adds to DirectShow graph
- **BuildMP4FilterGraph**: Full graph building — source → parser → decoder → renderer
- **BuildMP4PlayBack**: End-to-end playback pipeline construction
- **IsMP4FilePlayable**: Synchronous check (requires COM init; accesses file directly)
  - CRASH on invalid path (0xC0000005) — no NULL-path guard, requires CoInitialize

## Import Dependencies

| DLL | Key Functions | Purpose |
|-----|---------------|---------|
| MSVCR110.dll | memcpy, memset, _vsnwprintf, wcscat_s, wcstoul | CRT runtime |
| KERNEL32.dll | CreateThread, WaitForMultipleObjects, CriticalSection | Threading |
| gdiplus.dll | GdipCreateBitmapFromScan0, GdipCloneImage, GdipDrawImageRectI | Thumbnail generation |
| USER32.dll | DispatchMessageW, PeekMessageW, MsgWaitForMultipleObjects | Message pump |
| ADVAPI32.dll | RegCreateKeyW, RegOpenKeyExW, RegSetValueExW | Registry COM server |
| ole32.dll | CoCreateInstance, CoInitialize, PropVariantClear | COM infrastructure |
| OLEAUT32.dll | Ordinals 2,4,184,185 | OLE automation |
| WINMM.dll | timeSetEvent, timeKillEvent, timeBeginPeriod/EndPeriod | Multimedia timer |

## RTTI Class Hierarchy (57+ classes)

### AVCQT* Atom Parser Classes (35 classes)
These implement the core MP4/QuickTime atom tree parsing:

| Class | Atom Type | Description |
|-------|-----------|-------------|
| `CQTAtom` | Base | Base atom node (implements `IQTAtom`) |
| `CQTFullAtom` | Base | Base for atoms with version/flags |
| `CQTMovie` | moov | Movie atom (top-level container) |
| `CQTTrack` | trak | Track atom |
| `CQTMedia` | mdia | Media atom |
| `CQTMediaInformationAtom` | minf | Media information container |
| `CQTSampleTable` | stbl | Sample table atom |
| `CQTHandlerAtom` | hdlr | Handler reference atom |
| `CQTMetadataAtom` | meta/udta | Metadata container |
| `CQTAppleMetadataAtom` | illilst | Apple iTunes metadata |
| `CQTDataRefInfoAtom` | dref | Data reference atom |
| `CQTVideoSampleDescription` | vide | Video sample description |
| `CQTAudioSampleDescription` | soun | Audio sample description |
| `CQTSampleDescription` | stsd | Sample description |
| `CQTLegacyAudioSampleDescription` | soun (v0-2) | Legacy audio desc (pre-ESDS) |
| `CQTTimeToSampleMapAtom` | stts | Time-to-sample map |
| `CQTCompositionTimeToSampleMapAtom` | ctts | Composition time offsets |
| `CQTSampleToChunkMapAtom` | stsc | Sample-to-chunk map |
| `CQTSampleSizeAtom` | stsz | Sample size atom |
| `CQTChunkOffsetAtom` | stco | Chunk offset (32-bit) |
| `CQTChunkLargeOffsetAtom` | co64 | Chunk offset (64-bit) |
| `CQTSyncSampleAtom` | stss | Sync sample (keyframe) index |
| `CQTUserData` | udta | User data atom |
| `CQTMediaHandler` | hdlr (media) | Media handler interface |
| `CQTVideoMediaHandler` | hdlr (vide) | Video handler |
| `CQTAudioMediaHandler` | hdlr (soun) | Audio handler |
| `CQTAudioStreamHandler` | (audio) | Audio stream processing |
| `CQTUncompressedAudioStreamHandler` | (uncompressed) | PCM audio handler |
| `CQTESDSAtom` | esds | Elementary Stream Descriptor |
| `CQTMetadata` | (metadata) | Metadata interface |

### MP4 Filter Graph Classes (15 classes)
DirectShow filter pipeline:

| Class | Interface | Description |
|-------|-----------|-------------|
| `CMP4Parser` | — | Core MP4 parser engine |
| `CMP4DShowParserFilter` | IBaseFilter | DirectShow parser filter |
| `CMP4DShowGraph` | — | Filter graph builder |
| `CMP4DecodeRendererFilter` | IBaseFilter | Decoder renderer |
| `CMP4StreamHandler` | — | Stream sample delivery |
| `IMP4StreamHandler` | IMP4StreamHandler | Stream handler interface |
| `CMP4InputPin` | IMemInputPin | Input pin (receives samples) |
| `CMP4OutputPin` | IPin | Output pin (delivers samples) |
| `CMP4BitmapGenerator` | — | Thumbnail bitmap creation |
| `CMP4MovieThumbnail` | IExtractImage/IThumbnailProvider | Thumbnail extraction |
| `CMP4MetadataHandler` | IQTMetadata | Metadata reader |
| `CRendererInputPin` | IMemInputPin | Renderer input pin |
| `CRendererPosPassThru` | IQualityControl | Position pass-through |
| `CPosPassThru` | IMediaPosition | Media position control |
| `CBaseRenderer` | IMediaFilter | Base renderer filter |

### Supporting Infrastructure Classes (7+ classes)

| Class | Description |
|-------|-------------|
| `CBaseObject` | Base reference counting |
| `CBaseFilter` | DirectShow base filter |
| `CBasePin` / `CBaseInputPin` / `CBaseOutputPin` | Pin hierarchy |
| `CTransformFilter` / `CTransformInputPin` / `CTransformOutputPin` | Transform filter |
| `CEnumAtoms` | Atom enumeration (implements `IEnumAtoms`) |
| `CEnumMediaTypes` / `CEnumPins` | DirectShow enumerators |
| `CClassFactory` | COM class factory |
| `CCritSec` / `CAMThread` | Synchronization / threading |
| `CMediaPosition` | IMediaPosition implementation |
| `CYUVTORGB` | YUV→RGB color conversion |
| `CVC1VideoSampleDescription` | VC-1 video sample desc |
| `CUnknown` | IUnknown delegator |

### Metadata Classes (7 classes)

| Class | Description |
|-------|-------------|
| `CAppleMetadataCollection` | iTunes metadata collection |
| `CAppleMetadataRecord` | Single metadata record |
| `CMSMetadataAtom` | Microsoft metadata atom |
| `CMSMetadataCollection` | MS metadata collection |
| `CMSMetadataRecord` | MS metadata record |
| `CMetadataRecord` | Base metadata record |
| `CQTDataBuffer` | Data buffer for metadata |

## FourCC Atom Registry

### Container Atoms (parsed recursively)
`moov`, `trak`, `mdia`, `minf`, `stbl`, `edts`, `udta`, `meta`, `ilst`, `dinf`

### Full Atoms (version + flags header)
`mdhd`, `hdlr`, `elst`, `stsd`, `stts`, `ctts`, `stsc`, `stsz`, `stco`, `co64`, `stss`, `sdtp`, `sbgp`, `sgpd`, `subs`

### Media Atoms
`vide`, `soun`, `hint`, `alias`, `rsrc`, `cdsc`

### Codec Atoms (detected in binary)
`avc1`, `avcC`, `mp4a`, `esds`, `Wave`, `wave`, `twos`, `sowt`, `dOps`

### Metadata Atoms
`ilst`, `meta`, `udta`, `hnti`, `hinf`, `uuid`, `free`, `skip`, `mdat`, `ftyp`, `text`

### Embedded FourCC in RTTI (from binary data)
`H264`, `M4S2`, `WVC1`, `WMV3`, `MJPG`, `YUY2`

## Codec Support Matrix

| Codec | FourCC | Sample Description | Handler |
|-------|--------|-------------------|---------|
| H.264/AVC | avc1 | CQTVideoSampleDescription | vide |
| VC-1 | WMV3/WVC1 | CVC1VideoSampleDescription | vide |
| MJPEG | MJPG | CQTVideoSampleDescription | vide |
| AAC | mp4a/esds | CQTAudioSampleDescription | soun |
| PCM (twos) | twos | CQTLegacyAudioSampleDescription | soun |
| PCM (sowt) | sowt | CQTLegacyAudioSampleDescription | soun |
| PCM (LE 16-bit) | twos | CQTLegacyAudioSampleDescription | soun |
| Uncompressed | — | CQTUncompressedAudioStreamHandler | soun |

## Parser Architecture

### Atom Tree Traversal
```
CQTAtom (base)
  ├── CQTFullAtom (adds version/flags)
  │     ├── CQTMovie (moov)
  │     ├── CQTTrack (trak)
  │     ├── CQTMedia (mdia)
  │     ├── CQTMediaInformationAtom (minf)
  │     ├── CQTSampleTable (stbl)
  │     ├── CQTSampleDescription (stsd)
  │     ├── CQTTimeToSampleMapAtom (stts)
  │     ├── CQTCompositionTimeToSampleMapAtom (ctts)
  │     ├── CQTSampleToChunkMapAtom (stsc)
  │     ├── CQTSampleSizeAtom (stsz)
  │     ├── CQTChunkOffsetAtom (stco)
  │     ├── CQTChunkLargeOffsetAtom (co64)
  │     └── CQTSyncSampleAtom (stss)
  ├── CQTHandlerAtom (hdlr)
  ├── CQTDataRefInfoAtom (dref)
  ├── CQTVideoSampleDescription (vide)
  ├── CQTAudioSampleDescription (soun)
  ├── CQTESDSAtom (esds)
  └── CQTAppleMetadataAtom (ilst)
```

### Sample Delivery Pipeline
```
MP4 File → CMP4Parser (atom tree parse)
         → CMP4StreamHandler (sample lookup via stts/stsc/stsz/stco)
         → CMP4OutputPin (IMediaSample delivery)
         → [Decoder Filter]
         → CRendererInputPin → CBaseRenderer → Render
```

### Thread Model
- **CAMThread**: Worker thread base class
- **OutputPin ThreadProc**: Dedicated output pin thread with message pump
  - Uses PostThreadMessage + WaitForMultipleObjects
  - Messages: flush, end-of-stream, new sample
- **CRendererInputPin**: Renderer-side input with critical section synchronization

### Buffer Management
- **CQTDataBuffer**: Generic data buffer for atom payloads
- **CPosPassThru / CRendererPosPassThru**: Position tracking through filter chain
- **PushMediaSample**: Core sample delivery mechanism with timing

### Integer Overflow Protection
- Extensive overflow checking in atom length calculations:
  - "integer overflow trying to calculate atom length pre-children"
  - "integer overflow trying to calculate atom length post-children"
  - "integer overflow trying to calculate atom length (root/child type)"
  - "Integer overflow detected in Time to Sample Map atom"
  - "64-bit Integer overflow computing duration"

### Error Detection Messages (from binary strings)
- `Bad audio stsd length %S`
- `Bad dref atom length %S`
- `bad media handler atom length %S`
- `mvhd atom too small -- is %d bytes (%S)`
- `mvhd timescale is 0 (%S)`
- `mdhd v0/v1 timescale is 0 (%S)`
- `Can't save content; > 1000 atoms at top level`
- `Can't save content; multiple moov atoms found`
- `Sample count in ctts != sample count in stts`
- `no ES descriptor found in audio sample descriptor`
- `audio ES descriptor is absurdly huge: %d`
- `audio sample description has multiple WAVE atoms`
- `audio stsd refers to > 1 data reference`
- `Partial MP4 file (size %d, need %ld)`
- `rejecting MP4 file because of large movie atom`
- `MP4 not playable because is FairPlay protected`
- `compressed movie atom (%S)`
- `metadata handler type is %08X, not mdir`
- `dref atom has %d entries (we want only one)`
- `found multiple dinf atoms`
- `data reference isn't self-referenced`
- `Too small a buffer for sample %S`
- `vid stsd reports sdescSize %d, atom length %d`
- `Marking %S sample time %I64d as discon`
- `Stop pushing %S samples -- out of samples`

## Integration with WLXVideoTrim.dll
- WLXMP4Parser builds the filter graph; WLXVideoTrim likely provides trim/cut functionality
- Both use DirectShow COM infrastructure (IBaseFilter, IPin, IMediaSample)
- Shared pattern: CAMThread for async operations, CCritSec for synchronization
- Both import from MSVCR110.dll (MSVC 2012 runtime)

## Dynamic Test Results
- All 9 exports resolved successfully at runtime
- `IsMP4FilePlayable`: Access violation (0xC0000005) on invalid path — requires COM initialization
- COM exports (DllMain, DllCanUnloadNow, etc.) skipped (require COM runtime)
- High-level exports (AddMP4SourceFilter, BuildMP4FilterGraph, BuildMP4PlayBack) require IGraphBuilder
