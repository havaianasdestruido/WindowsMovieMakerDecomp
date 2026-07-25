# WLXVideoTrim.dll Analysis

## Overview
Video trimming processor DLL from Windows Live Movie Maker 2012 (Photo Gallery suite). Handles **trimming operations** (copy & transcode) for all video formats supported by WLMM: WMV/ASF, AVI, DV-AVI, MPEG-2 (DVR-MS), MP4/MOV (via delay-load WLXMP4Parser.dll), and MJPG. Uses DirectShow filter graphs for all media processing.

- **File:** WLXVideoTrim.dll
- **Description:** "Photo Gallery Video Trim Processor"
- **Build:** `16.4.3528.0331_ship.client.main.w5m4 (ship)` — same branch as other WLX DLLs
- **Timestamp:** 2014-04-01 01:26:24 (PE header) / 01:14:53 (export table)
- **Image Size:** 0x8B000 (568 KB)
- **PDB:** `WLXVideoTrim.pdb`, GUID `{73CFF58F-A97D-4232-883C-397BD1DF5009}`, age 1
- **Source tree (from debug info):** `e:\bt\1105173\client\personalmedia\videotrim\dev\`

## PE Structure

| Section | VA | VirtSize | RawSize | Flags |
|---------|------|----------|---------|-------|
| `.text`  | 0x1000 | 0x79ECE | 0x7A000 | Code + Execute Read |
| `.data`  | 0x7B000 | 0x355C | 0x3000 | Init Data + Read Write |
| `.rsrc`  | 0x7F000 | 0x408 | 0x600 | Init Data + Read Only (VERSION info only) |
| `.reloc` | 0x80000 | 0xAE70 | 0xB000 | Init Data + Discardable + Read Only |

**Headers:**
- **Machine:** x86 (0x14C), **Subsystem:** Windows GUI (2)
- **Linker:** MSVC 11.00 (VS 2012)
- **Entry Point:** 0x1006343C
- **Image Base:** 0x10000000
- **DLL Characteristics:** Dynamic Base (ASLR), NX Compatible (DEP)
- **Stack:** 256KB reserve / 4KB commit
- **Heap:** 1MB reserve / 4KB commit

## Export Table (5 functions)

| Ordinal | RVA | Name | Undecorated Signature |
|---------|------|------|----------------------|
| 1 | 0x00026A3F | `CreateAVICopierDirect` | `int __cdecl CreateAVICopierDirect(unsigned short const * const, bool, long &)` |
| 2 | 0x0002701D | `CreateVideoCopierFromMediaType` | `int __cdecl CreateVideoCopierFromMediaType(unsigned short const * const, struct AM_MEDIA_TYPE *, bool, long &)` |
| 3 | 0x00027161 | `CreateVideoFormatContextTranscoder` | `int __cdecl CreateVideoFormatContextTranscoder(class FormatContext *, unsigned short const * const, bool, long &)` |
| 4 | 0x00026EE3 | `CreateVideoPlayer` | `int __cdecl CreateVideoPlayer(unsigned short const * const, struct HWND__ *, enum RotationEnum, bool, long &)` |
| 5 | 0x00026B7C | `CreateVideoWMVTranscoder` | `int __cdecl CreateVideoWMVTranscoder(struct IWMProfile *, unsigned short const * const, bool, long &)` |

All exports return `HRESULT` (`int`), take a wide string path, a `bool` (likely start/pause flag), and an output `long &` (receives the created object pointer as a cookie/handle). The caller receives an opaque long reference that is used internally to access the created object.

- **CreateAVICopierDirect** — Creates a copier for AVI files
- **CreateVideoCopierFromMediaType** — Factory that inspects AM_MEDIA_TYPE to select the right copier (AVI, WMV/ASF, DVR-MS/MPEG-2)
- **CreateVideoFormatContextTranscoder** — Creates transcoder from a FormatContext (used for MP4/MOV via delay-load WLXMP4Parser)
- **CreateVideoPlayer** — Creates a video player for preview with rotation support
- **CreateVideoWMVTranscoder** — Creates transcoder from an IWMProfile for WMV output

## Import Table

### Direct Imports (load-time)

| DLL | Functions | Purpose |
|-----|-----------|---------|
| **MSVCR110.dll** | 35 | VC++ 2012 CRT (memcpy, malloc, free, wcslen, wcschr, wcsstr, _beginthreadex, exception handling, etc.) |
| **KERNEL32.dll** | 39 | Process/thread, file I/O, module loading, sync (CriticalSection, Event), resources, temp files, encoding |
| **USER32.dll** | 9 | Window message pump (PeekMessage, DispatchMessage, PostMessage, PostQuitMessage), painting (BeginPaint, EndPaint, FillRect), CharNextW |
| **ADVAPI32.dll** | 22 | Registry (RegCreateKeyEx, RegOpenKeyEx, RegQueryValueEx, RegSetValueEx, etc.), ETW tracing (TraceEvent, TraceMessage, RegisterTraceGuids), Crypto (CryptAcquireContext, CryptHashData, etc.) |
| **ole32.dll** | 9 | COM (CoCreateInstance, CoInitialize, CoTaskMemAlloc/Free, CreateItemMoniker, GetRunningObjectTable) |
| **OLEAUT32.dll** | 3 ordinals | COM automation (ordinals 2, 6, 277 — likely VariantInit, SafeArrayCreate, etc.) |
| **WLXPhotoBase.dll** | 10 | Base support: `Base::Throw`, `Base::ThrowLastError`, `Base::Exception::operator HRESULT`, `Base::IsWin7OrGreater`, `Base::IsWin8OrGreater`, `BasePrivate::New`, `BasePrivate::Delete`, `Base::GetBaseStringManager` (ATL string mgr), `BaseAtlThrow` |
| **SHELL32.dll** | 1 ordinal | Ordinal 165 (likely SHCreateItemFromParsingName or SHGetKnownFolderPath) |
| **SHLWAPI.dll** | 11 | Path functions (PathAddExtension, PathRemoveExtension, PathAppend, PathFindExtension, PathFileExists, PathIsDirectory), SHCreateStreamOnFileW |
| **WMVCore.DLL** | 6 | Windows Media: `WMCreateReader`, `WMCreateWriter`, `WMCreateEditor`, `WMCreateProfileManager`, `WMCreateSyncReader`, `WMIsContentProtected` |

### Delay-Load Imports

| DLL | Functions | Purpose |
|-----|-----------|---------|
| **WLXMP4Parser.dll** | 2 | `AddMP4SourceFilter`, `IsMP4FilePlayable` — MP4/MOV support |

These are the only two functions needed from WLXMP4Parser. MP4 trimming is done by adding a custom MP4 source filter into the DirectShow graph.

## COM GUIDs / CLSIDs Used

The DLL references these COM classes via `CoCreateInstance` / `__uuidof` (resolved at build time to GUIDs embedded in the binary's .rdata section; GUID text strings are not stored):

| Identifier | Likely CLSID / Purpose |
|------------|----------------------|
| `CLSID_FilterGraph` | Standard DirectShow filter graph |
| `CLSID_CaptureGraphBuilder2` | DirectShow capture graph builder |
| `CLSID_FileWriter` | DirectShow file writer |
| `CLSID_DSoundRender` | DirectSound audio renderer |
| `CLSID_DMOWrapperFilter` | DMO Wrapper filter |
| `CLSID_DVVideoEnc` | DV video encoder |
| `CLSID_DVMux` | DV multiplexer |
| `CLSID_EnhancedVideoRenderer` | EVR (Enhanced Video Renderer) |
| `CLSID_VideoMixingRenderer9` | VMR-9 |
| `CLSID_VideoRendererDefault` | Legacy video renderer |
| `CLSID_MFMPEG2Demultiplexer` | MPEG-2 demux (Media Foundation) |
| `CLSID_Mpeg2VideoStreamAnalyzer` | MPEG-2 video stream analyzer |
| `CLSID_PsiParserFilter` | MPEG-2 PSI parser |
| `CLSID_MFSourceFilter` | Media Foundation source filter |
| `CLSID_StreamBufferSink` | Stream Buffer Sink (DVR-MS) |
| `CLSID_StreamBufferConfig` | Stream Buffer config |
| `CLSID_StreamBufferComposeRecording` | Stream Buffer recording composer |
| `CLSID_DTFilter` | TV rating decrypter |
| `CMPEG2EncoderDS` | MPEG-2 encoder/multiplexer |
| `CColorConvertDMO` | Color converter DMO |
| `CResamplerMediaObject` | Audio resampler DMO |

**COM Interfaces used:** `IBaseFilter`, `IMediaControl`, `IMediaEventEx`, `IMediaSeeking`, `IBasicAudio`, `IFilterGraph2`, `IWMHeaderInfo`, `IWMHeaderInfo3`, `IWMProfile`, `IWMReaderAdvanced`, `IWMReaderProfile`, `IWMWriter`, `IWMWriterAdvanced`, `IWMWriterAdvanced2`, `IDMOWrapperFilter`, `IMFGetService`, `IMFVideoProcessor`, `IMFVideoDisplayControl`, `IVMRDeinterlaceControl`, `IVMRDeinterlaceControl9`, `IServiceProvider`, `IStreamBufferSink`, `IStreamBufferRecordControl`, `IStreamBufferDataCounters`, `IStreamBufferInitialize`, `IStreamBufferRecordingAttribute`, `IValidateBinding`, `IDX`

**Explicit CLSID registry string found:** `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` (appears in `CLSID\{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}\InprocServer32` — likely one of this DLL's own COM classes for external use)

## Internal Class Architecture (from debug strings)

### Copier Classes (trim by re-encoding only the selected range)

| Class | File | Extends | Purpose |
|-------|------|---------|---------|
| `VideoCopier` | videocopier.cpp | Base class | Abstract copier; manages temp files, progress, range lists, AVI mux options |
| `AsfCopier` | asfcopier.cpp | VideoCopier | WMV/ASF copy via AsfCopyWorker (reader + writer directly) |
| `AVICopier` | avicopier.cpp | VideoCopier | AVI file copy (re-mux) |
| `Mpg2Copier` | mpg2copier.cpp | VideoCopier | MPEG-2 Program Stream copy via Stream Buffer |
| `StreamBufferCopier` | streambuffercopier.cpp | VideoCopier | DVR-MS (.dvr-ms/.sbe) copy via Stream Buffer |

### Transcoder Classes (trim + re-encode to a different format)

| Class | File | Purpose |
|-------|------|---------|
| `VideoTranscoder` | videotranscoder.cpp | Factory: creates format-specific transcoder based on file extension |
| `AsfTranscoder` | asftranscoder.cpp | WMV transcoding |
| `AVITranscoder` | avitranscoder.cpp | AVI transcoding (also MJPG) |
| `DVTranscoder` | dvtranscoder.cpp | DV-AVI transcoding |
| `Mpg2Transcoder` | mpg2transcoder.cpp | MPEG-2 transcoding |
| `StreamBufferTranscoder` | streambuffertranscoder.cpp | DVR-MS transcoding |

### Helper/Filter Classes

| Class | File | Purpose |
|-------|------|---------|
| `AsfCopyWorker` | asfcopyworker.cpp | Low-level WMV read/write with sample-level copying; marker/script/attribute copy; VBR quality; temp WMV handling |
| `FilterGraph` | filtergraph.cpp | DirectShow graph management: source filter creation, seeking, range lists, pin connection, audio encoder swapping, decrypter, rotation filter, progress filters |
| `RangeList` | rangelist.cpp | Manages trim ranges (start/end times) |
| `SyncReader` | syncreader.cpp | Synchronous reader for temp WMV sequential read |
| `CopyProgressInfoFilter` | copyprogressinfofilter.cpp | DirectShow transform filter that tracks copy progress and sends EOS events |
| `AudioRepackageFilter` | audiorepackagefilter.cpp | DirectShow transform filter for audio format repackaging |
| `VideoRotationFilter` | videorotationfilter.cpp | DirectShow transform filter for video rotation (clockwise/counter-clockwise) |
| `YUY2ToI420Filter` | yuy2toi420filter.cpp | DirectShow color space converter |
| `DShowHelper` | dshowhelper.h | Helper utilities for DirectShow operations |

### Player Class

| Class | File | Purpose |
|-------|------|---------|
| `VideoPlayer` | videoplayer.cpp | Preview player; manages EVR/VMR7/VMR9, audio renderer, playback events, seek, take-picture (thumbnail) |

## Source Files (from debug paths)

All under `e:\bt\1105173\client\personalmedia\videotrim\dev\`:

**DLL entry:**
- `dll\dllmain.cpp`

**Core video trim logic:**
- `videotrim\asfcopier.cpp`
- `videotrim\asfcopyworker.cpp`
- `videotrim\asftranscoder.cpp`
- `videotrim\avicopier.cpp`
- `videotrim\avitranscoder.cpp`
- `videotrim\dvtranscoder.cpp`
- `videotrim\mpg2copier.cpp`
- `videotrim\mpg2transcoder.cpp`
- `videotrim\streambuffercopier.cpp`
- `videotrim\streambuffertranscoder.cpp`
- `videotrim\videocopier.cpp`
- `videotrim\videoplayer.cpp`
- `videotrim\videotranscoder.cpp`
- `videotrim\filtergraph.cpp`
- `videotrim\dshowhelpers.cpp`
- `videotrim\rangelist.cpp`
- `videotrim\syncreader.cpp`

**DirectShow filters:**
- `videotrimfilters\audiorepackagefilter.cpp`
- `videotrimfilters\copyprogressinfofilter.cpp`
- `videotrimfilters\videorotationfilter.cpp`
- `videotrimfilters\yuy2toi420filter.cpp`
- `videotrimfilters\dshowhelper.h`

## Registry Paths Used

- `SOFTWARE\Microsoft\VideoTrim\StreamBufferCopier Settings`
- `SOFTWARE\Microsoft\Windows Live\Common\Movie Library`
- `SOFTWARE\Microsoft\Windows Live\Movie Maker`
- `Software\Microsoft\Windows Live\Photo Gallery\VideoTrim`
- `CLSID\{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}\InprocServer32`

## Supported File Extensions (from codec selection logic)

From `VideoCopier::CreateVideoCopierFromMediaType` and `VideoTranscoder::CreateObject`:
- `.wmv` / `.asf` — ASF/WMV format → AsfCopier / AsfTranscoder
- `.avi` — AVI format → AVICopier / AVITranscoder (also DV-AVI → DVTranscoder)
- `.dvr-ms` / `.sbe` — DVR-MS (Microsoft Digital Video Recording) → StreamBufferCopier / StreamBufferTranscoder (MPEG-2)
- `.mpg` / `.mpeg` / `.m2v` — MPEG-2 Program Stream → Mpg2Copier / Mpg2Transcoder
- `.mp4` / `.mov` / `.3gp` / `.3g2` / `.3gp2` / `.3gpp` / `.mqv` — MP4/MOV/3GP → via WLXMP4Parser.dll (delay-load)
- MJPEG in AVI → AVITranscoder

## Key Video Processing Features

- **Trim by range:** Uses `RangeList` to specify start/end times within the source
- **Frame-accurate trimming:** Handles key frame alignment, sample-level copying for WMV
- **Rotation support:** `RotationEnum` parameter; uses `VideoRotationFilter` (clockwise/counter-clockwise)
- **Deinterlacing:** Configurable via WMVCore profile settings (`g_wszDeinterlaceMode`)
- **VBR quality:** `_VBRENABLED`, `_VBRQUALITY`, `_VBRAUTOMODE`, `_PASSESRECOMMENDED`, `_DEFAULTCRISP` settings
- **Copy vs Transcode:** "Copier" = re-mux/copy without re-encode (preserves original compression); "Transcoder" = full decode + re-encode
- **Progress reporting:** Custom `CopyProgressInfoFilter` inserted into the filter graph to track and report copy progress
- **Thumbnail capture:** `VideoPlayer::TakePicture` captures current frame as BMP via EVR/VMR `GetCurrentImage`
- **Temporal compression detection:** Debug messages indicate `[Temporal Compression]` / `[NOT] Temporal Compression`
- **Stream Buffer (DVR-MS):** Full support for Microsoft's Stream Buffer Engine (SBE) for live TV recording format

## Strings Summary

### Version Resource Strings
- CompanyName: `Microsoft Corporation`
- FileDescription: `Photo Gallery Video Trim Processor`
- FileVersion: `16.4.3528.0331_ship.client.main.w5m4 (ship)`
- InternalName: `WLXVideoTrim`
- OriginalFilename: `WLXVideoTrim.dll`
- ProductName: `Photo Gallery`
- ProductVersion: `16.4.3528.0331`
- LegalCopyright: `(c) 2012 Microsoft Corporation. All rights reserved.`

### Error Code Strings
- `AVS_E_DECODER_FAILURE`, `AVS_E_INVALID_DURATION_FILE`, `AVS_E_NON_SEEKABLE_FILE`, `AVS_E_SAMPLE_OVERSEEK`, `AVS_E_UNSUPPORTED_FILE_TYPE`
- Standard HRESULT strings: `E_FAIL`, `E_INVALIDARG`, `E_UNEXPECTED`, `S_OK`, `S_FALSE`, and many `MF_E_*` error codes

### Progress/Time Format Strings
- `%s [Seconds: %0.5f] [Progress: %0.5f]`
- `%s [Seconds: %0.5f] [Count: %d]`
- `%0.5f` (used extensively for time values in 100-ns unit conversion)
- `[Start: %0.5f] [Stop: %0.5f]` (segment notifications)
- `0x%08X` (debug formatting)
- `%08x` (filter graph handle formatting)

### Resource DLL References
- `WLAVRes.dll` — Audio/Video resource DLL
- `WLXPhotoLibraryDuiResourcesLocalized` — Photo Gallery DirectUI resources (referenced via Base)

## Key Design Observations

1. **Factory pattern:** The 5 exported functions are factory methods that return opaque `long&` handles. Internally these map to `VideoCopier`, `VideoTranscoder`, or `VideoPlayer` objects.

2. **DirectShow-centric:** Despite targeting Windows 8 era, the trim engine relies almost entirely on DirectShow filter graphs (not Media Foundation topology). MP4 support is bridged via WLXMP4Parser's custom source filter that feeds into DirectShow.

3. **WMVCore for WMV:** WMV trimming uses the Windows Media Format SDK directly (WMCreateReader/WMCreateWriter) for sample-accurate copy without decode/re-encode.

4. **Stream Buffer Engine for DVR-MS:** Microsoft's Stream Buffer Sink/Config/ComposeRecording classes are used for MPEG-2 DVR-MS format, providing frame-accurate trimming through stream buffer recording objects.

5. **No UI strings in this DLL:** The DLL has no UI thread or dialog resources — it is purely a processing engine. UI (trim bar, time display, playback controls) is handled by the host application (MovieMakerCore.dll).

6. **Rotation handled in-filter:** Video rotation is achieved by inserting a `VideoRotationFilter` transform filter into the graph (not by re-encoding with rotation metadata).

7. **Temp file workflow:** All trimming creates a temporary file (`_TempWMV` or temp file in a configured folder), which is then moved to the final destination on completion.
