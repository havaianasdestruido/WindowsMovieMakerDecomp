# WLXMP4Parser.dll — Static Analysis

**File:** `WLXMP4Parser.dll` (Windows Live Movie Maker 2012, Photo Gallery)
**Version:** 16.4.3528.0331
**Description:** WLX MP4 Parser Filter
**Date:** 2014-04-01
**Arch:** x86 (32-bit)
**Image Base:** `0x10000000`
**Size:** ~184 KB (`0x2E000`)
**Linker:** MSVC 2012 (VC11) / v110
**Subsystem:** Windows GUI
**Checksum:** `0x3D0C6`
**PDB:** `WLXMP4Parser.pdb` (GUID: `{A4577D2C-5400-4F17-91C0-06B5EA1BE4DE}`)

---

## PE Structure

| Section | Virtual Address | Virtual Size | Raw Size | Flags |
|---------|----------------|--------------|----------|-------|
| `.text` | `0x10001000` | `0x24A1D` | `0x24C00` | Code, Execute Read |
| `.data` | `0x10026000` | `0x18B4` | `0x1600` | Init Data, Read Write |
| `.rsrc` | `0x10028000` | `0x3E8` | `0x400` | Init Data, Read Only |
| `.reloc` | `0x10029000` | `0x4B48` | `0x4C00` | Init Data, Discardable, Read Only |

**Security:** `/GS` (buffer security check, 42 functions covered). ASLR (Dynamic Base) and NX compatible. No `/sdl`.

---

## Export Table (9 functions)

| Ordinal | Name | RVA |
|---------|------|-----|
| 1 | `AddMP4SourceFilter` | `0x1001541E` |
| 2 | `BuildMP4FilterGraph` | `0x10015132` |
| 3 | `BuildMP4PlayBack` | `0x1001533A` |
| 4 | `IsMP4FilePlayable` | `0x10014F4E` |
| 5 | `DllCanUnloadNow` | `0x1001D16A` |
| 6 | `DllGetClassObject` | `0x1001D1A3` |
| 7 | `DllMain` | `0x100086B6` |
| 8 | `DllRegisterServer` | `0x100085F7` |
| 9 | `DllUnregisterServer` | `0x10008604` |

The 4 non-COM exports (`AddMP4SourceFilter`, `BuildMP4FilterGraph`, `BuildMP4PlayBack`, `IsMP4FilePlayable`) form the primary public API for WLMM to parse and play MP4 files. `BuildMP4PlayBack` likely constructs the full playback graph, `BuildMP4FilterGraph` a custom graph, `AddMP4SourceFilter` adds just the source filter, and `IsMP4FilePlayable` pre-checks playability.

---

## Import Table

### MSVCR110.dll (CRT — Microsoft VC++ 2012 Redistributable)
C++ runtime: `malloc`, `free`, `memcpy`, `memset`, `memcmp`, `wcscat_s`, `wcstol`, `wcstoul`, `_wcsicmp`, `wcsrchr`, `_vsnwprintf`, `_calloc_crt`, `_initterm(_e)`, `_onexit`, `__CxxFrameHandler3`, `__CppXcptFilter`, `_except_handler4_common`, `?terminate@@YAXXZ`, `?2@YAPAXI@Z` (operator new), `?3@YAXPAX@Z` (operator delete), `?U@YAPAXI@Z` (operator new[]), `?V@YAXPAX@Z` (operator delete[]), `?1type_info@@UAE@XZ` (type_info destructor), `_purecall`.

### KERNEL32.dll
Threading: `CreateThread`, `Sleep`, `GetCurrentThread(Id)`, `SetThreadPriority`, `GetThreadPriority`, `WaitForMultipleObjects`, `WaitForSingleObject`, `CloseHandle`, `CreateEventW`, `SetEvent`, `ResetEvent`. Sync: `InitializeCriticalSection`, `EnterCriticalSection`, `LeaveCriticalSection`, `DeleteCriticalSection`. Memory/Files: `SetFilePointer`, `SetEndOfFile`, `WriteFile`. String: `lstrcmpW`, `lstrlenW/A`. Modules: `GetModuleHandleW`, `GetProcAddress`, `LoadLibraryW`, `FreeLibrary`, `GetModuleFileNameW/A`. Misc: `QueryPerformanceCounter`, `GetTickCount(64)`, `GetSystemTimeAsFileTime`, `GetVersionExW`, `IsDebuggerPresent`, `IsProcessorFeaturePresent`, `MultiByteToWideChar`, `WideCharToMultiByte`, `MulDiv`, `InterlockedExchange/Increment/Decrement`, `EncodePointer`, `DecodePointer`, `DebugBreak`, `DisableThreadLibraryCalls`.

### ole32.dll
`CoCreateInstance`, `CoInitialize`, `CoUninitialize`, `CoFreeUnusedLibraries`, `CoTaskMemAlloc`, `CoTaskMemFree`, `PropVariantCopy`, `PropVariantClear`, `StringFromGUID2`.

### OLEAUT32.dll (by ordinal)
Ordinals 2, 4, 184, 185 — likely `SysAllocString`, `SysFreeString`, `VariantClear`, `VariantCopy`.

### USER32.dll
`DispatchMessageW`, `PeekMessageW`, `PostThreadMessageW`, `GetQueueStatus`, `MsgWaitForMultipleObjects`, `SetRect`, `RegisterWindowMessageW`. Message pumping for COM apartment/window interaction.

### ADVAPI32.dll
Registry operations: `RegCreateKeyW`, `RegOpenKeyExW`, `RegDeleteKeyW`, `RegEnumKeyExW`, `RegSetValueW`, `RegSetValueExW`, `RegCloseKey`. Used for filter registration (DllRegisterServer/DllUnregisterServer).

### gdiplus.dll
`GdiplusStartup`, `GdiplusShutdown`, `GdipAlloc`, `GdipFree`, `GdipCloneImage`, `GdipDisposeImage`, `GdipDeleteGraphics`, `GdipCreateBitmapFromScan0`, `GdipCreateBitmapFromGdiDib`, `GdipCreateHBITMAPFromBitmap`, `GdipDrawImageRectI`, `GdipGetImageWidth`, `GdipGetImageHeight`, `GdipGetImageGraphicsContext`, `GdipSetInterpolationMode`. Used for thumbnail generation.

### WINMM.dll
`timeBeginPeriod`, `timeEndPeriod`, `timeSetEvent`, `timeKillEvent`. Multimedia timers.

---

## C++ Class Hierarchy (RTTI strings)

### Core Parser
- `CMP4Parser` — Main DirectShow source filter
- `CMP4InputPin` — Input pin (for demux)
- `CMP4OutputPin` — Output pin (video/audio streams)
- `CMP4DecodeRendererFilter` — Decoder/renderer filter
- `CMP4DShowGraph` — Graph builder helper
- `CMP4StreamHandler` / `IMP4StreamHandler` — Stream handler interface/impl

### QuickTime/ISO Atom Model
- `CQTMovie` — Root movie (`moov`)
- `CQTTrack` — Track (`trak`)
- `CQTMedia` — Media (`mdia`)
- `CQTAtom` — Base atom
- `CQTFullAtom` — Atom with version+flags
- `CQTUserData` — User data atoms
- `CQTMediaHandler` — Abstract media handler
- `CQTVideoMediaHandler` — Video track handler
- `CQTAudioMediaHandler` — Audio track handler
- `CQTMediaInformationAtom` — Media info (`minf`)
- `CQTDataRefInfoAtom` — Data reference (`dref`)
- `CQTSampleTable` — Sample table (`stbl`)
- `CQTSampleDescription` — Sample description (`stsd`)
- `CQTVideoSampleDescription` — Video sample desc
- `CQTAudioSampleDescription` — Audio sample desc
- `CQTLegacyAudioSampleDescription` — Legacy audio (e.g. uncompressed)
- `CVC1VideoSampleDescription` — VC-1 video sample desc
- `CQTTimeToSampleMapAtom` — `stts`
- `CQTSampleToChunkMapAtom` — `stsc`
- `CQTSampleSizeAtom` — `stsz`
- `CQTChunkOffsetAtom` — `stco`
- `CQTChunkLargeOffsetAtom` — `co64`
- `CQTSyncSampleAtom` — `stss`
- `CQTCompositionTimeToSampleMapAtom` — `ctts`
- `CQTESDSAtom` — `esds` (MPEG-4 descriptors)
- `CQTDataBuffer` — Raw data buffer

### Metadata
- `CMP4MetadataHandler` — Metadata handler
- `CQTMetadataAtom` — Metadata atom (`meta`)
- `CQTAppleMetadataAtom` — Apple/iTunes metadata (`ilst`)
- `CAppleMetadataCollection` / `CAppleMetadataRecord` — Apple metadata items
- `CMSMetadataAtom` / `CMSMetadataCollection` / `CMSMetadataRecord` — Windows Media metadata
- `CMP4Descriptor` — MPEG-4 descriptor parsing (Object Descriptor, DecoderConfig, etc.)

### Thumbnailing
- `MP4MovieThumbnail` / `IMP4MovieThumbnail` — Thumbnail provider
- `CMP4BitmapGenerator` — Bitmap generation from video frames
- `CYUVTORGB` — YUV-to-RGB conversion (software)

### Utility / Base Classes (from BaseClasses/STRMBASE)
- `CUnknown`, `CBaseObject`, `CBaseFilter`, `CBasePin`, `CBaseInputPin`, `CBaseOutputPin`
- `CTransformFilter`, `CTransformInputPin`, `CTransformOutputPin`
- `CRendererInputPin`, `CBaseRenderer`
- `CMediaPosition`, `CPosPassThru`, `CRendererPosPassThru`
- `CAMThread` — Worker thread
- `CCritSec` — Critical section wrapper
- `CEnumPins`, `CEnumMediaTypes`, `CEnumAtoms`
- `CClassFactory`, `IClassFactory`

### Interfaces
- `IMP4StreamHandler`, `IMP4DecodeRendererFilter`, `IMP4MovieThumbnail`
- `IQTMovie`, `IQTAtom`, `IQTChunkOffsetAtom`, `IQTMediaHandler`, `IQTMetadata`, `IEnumAtoms`
- `IThumbnailProvider`, `IInitializeWithFile`, `IExtractImage`, `IPersistFile`
- Standard DirectShow: `IPin`, `IMemInputPin`, `IMediaFilter`, `IMediaSeeking`, `IMediaPosition`, `IBaseFilter`, `IQualityControl`, `IReferenceClock`, `IDispatch`, `IUnknown`, `IPersist`, `IAMovieSetup`

---

## MP4 Atom Types (FourCC Codes) Parsed

### Container Atoms
`moov`, `trak`, `mdia`, `minf`, `stbl`, `dinf`, `edts`, `udta`, `meta`, `ilst`, `mvex`, `moof`, `traf`, `sinf`, `schi`

### Leaf Atoms — Movie Level
`ftyp` (file type), `mvhd` (movie header), `mehd` (movie extends header), `free`, `skip`, `wide`, `pssh`, `sidx`, `ssix`, `prft`, `emsg`

### Leaf Atoms — Track Level
`tkhd` (track header), `mdhd` (media header), `hdlr` (handler), `vmhd` (video media header), `smhd` (sound media header), `nmhd` (null media header), `elst` (edit list), `dref` (data reference)

### Leaf Atoms — Sample Table
`stsd` (sample descriptions), `stts` (time-to-sample), `ctts` (composition time-to-sample), `stsc` (sample-to-chunk), `stsz` (sample sizes), `stco` (chunk offset 32-bit), `co64` (chunk offset 64-bit), `stss` (sync samples), `stsh` (shadow sync), `padb` (padding bits), `stdp` (degradation priority), `sdtp` (independent/disposable), `sbgp` (sample-to-group), `sgpd` (sample group description), `saio` (auxiliary info offset), `saiz` (auxiliary info size)

### Sample Description Codes
- `avc1` (AVC/H.264 video), `avc2` (AVC/H.264 with in-band SPS/PPS)
- `mp4v` (MPEG-4 video), `s263` (H.263 video)
- `mp4a` (MPEG-4 audio / AAC), `raw ` (uncompressed audio), `twos` (two's complement PCM), `sowt` (signed little-endian PCM), `ima4` (IMA ADPCM), `MAC3` (MACE 3), `MAC6` (MACE 6), `QDM2` (QDesign Music 2), `Qclp` (Qualcomm PureVoice)
- `encv` (encrypted video), `enca` (encrypted audio)
- `vc-1` (SMPTE VC-1 video), `wmv3` (WMV3), `wmap` (WMA Pro)
- `vp80` (VP8), `vp90` (VP9)
- `av01` (AV1)
- `jpeg` (Motion JPEG), `png ` (PNG), `mjp2` (Motion JPEG 2000)
- `tx3g` (timed text 3GPP), `text` (QuickTime text)
- `ac-3` (Dolby Digital), `ec-3` (Dolby Digital+), `dts ` (DTS), `dtsc`/`dtsh`/`dtsl`/`dtse` (DTS variants)
- `alac` (Apple Lossless), `Opus`, `fLaC` (FLAC)

### Metadata Atoms
`data` (data container), `mean` (meaning), `name` (name), `\xA9too` (iTunes metadata keyspace)

### Fragment / Streaming Atoms
`mfro`, `mfhd`, `tfhd`, `tfdt`, `trun`, `trex`, `leva`, `fecr`, `fpar`, `meco`

### Protection
`pssh` (PlayReady/Widevine), `tenc` (track encryption), `sinf`, `frma` (original format), `schm` (scheme type), `schi`

---

## MPEG-4 Descriptors (via CMP4Descriptor)

The `CMP4Descriptor` class handles ISO 14496-1 (MPEG-4 Systems) descriptor chains found in the `esds` atom:

- **ObjectDescriptor** (tag 1) — top-level descriptor
- **InitialObjectDescriptor** (tag 2)
- **ES_Descriptor** (tag 3) — elementary stream descriptor (stream priority, stream dependency, URL, etc.)
- **DecoderConfigDescriptor** (tag 4) — object type indication (e.g. 0x40 = AAC, 0x21 = H.264), buffer size, max bitrate, avg bitrate
- **DecoderSpecificInfo** (tag 5) — codec configuration blob (e.g. AVC config record, AudioSpecificConfig)
- **SLConfigDescriptor** (tag 6) — sync layer configuration
- **IPI_DescrPointer** (tag 10)
- **IPMP_DescriptorPointer** (tag 11)
- **LanguageDescriptor** (tag 14)
- **QoS_Descriptor** (tag 15)
- **RegistrationDescriptor** (tag 16)
- **ExtensionDescriptor** (tag 21..254)

---

## Key Observations

1. **Thumbnail Provider:** Implements `IThumbnailProvider` + `IInitializeWithFile` + `IExtractImage` + `IPersistFile`, meaning this DLL serves as a Windows thumbnail handler for MP4 files in Explorer. Uses GDI+ for rendering.

2. **Software Decode Fallback:** `CYUVTORGB` class indicates software YUV-to-RGB conversion, used when hardware acceleration is unavailable (e.g. for thumbnails).

3. **VC-1 Support:** `CVC1VideoSampleDescription` shows the parser also handles VC-1 (SMPTE) video in MP4 containers—used by WMM for certain formats.

4. **Metadata Extraction:** Full Apple/iTunes metadata (ilst/data/mean/name) and Microsoft metadata (CMSMetadata*) extraction, used for WMM project metadata reading.

5. **DRM/Encryption:** `pssh`, `tenc`, `sinf`, `frma`, `schm`, `schi` atom support for PlayReady and other encryption schemes, though the parser likely only detects/extracts rather than decrypts.

6. **DirectShow Filter Registration:** Standard COM self-registration (`DllRegisterServer`/`DllUnregisterServer`) writes DirectShow filter registry entries under a generic CLSID.

7. **CRT Dependency:** Statically linked to VC 2012 (MSVCR110.dll), uses `/GS` for 42 functions.
