# WLXVideoTrim.dll — Dynamic Analysis & Export Probing

## Export Table (Corrected)

The actual exports differ from the initial assumptions. The DLL has **no** `GetTrimInterfaces`, `GetTrimProgress`, `DllGetClassObject`, `DllCanUnloadNow`, or `DllRegisterServer` exports.

| # | Export Name | Decorated (undecorated from dumpbin) |
|---|-------------|---------------------------------------|
| 1 | `CreateAVICopierDirect` | `int __cdecl CreateAVICopierDirect(unsigned short const * const, bool, long &)` |
| 2 | `CreateVideoCopierFromMediaType` | `int __cdecl CreateVideoCopierFromMediaType(unsigned short const * const, struct AM_MEDIA_TYPE *, bool, long &)` |
| 3 | `CreateVideoFormatContextTranscoder` | `int __cdecl CreateVideoFormatContextTranscoder(class FormatContext *, unsigned short const * const, bool, long &)` |
| 4 | `CreateVideoPlayer` | `int __cdecl CreateVideoPlayer(unsigned short const * const, struct HWND__ *, enum RotationEnum, bool, long &)` |
| 5 | `CreateVideoWMVTranscoder` | `int __cdecl CreateVideoWMVTranscoder(struct IWMProfile *, unsigned short const * const, bool, long &)` |

### Common Signature Pattern

All 5 exports are factory functions returning `HRESULT` (as `int`). They share a common pattern:
- First parameter: `const wchar_t*` — source file path (wide string)
- Middle parameters: format-specific context (AM_MEDIA_TYPE, FormatContext, IWMProfile, HWND, RotationEnum)
- Penultimate parameter: `bool` — likely a "start immediately" or "validate only" flag
- Last parameter: `long&` — receives an opaque handle/cookie to the created object

The returned `long` handle is used by the host (MovieMakerCore.dll) to subsequently call into the DLL's internal methods for progress polling, seeking, and lifecycle management — all through indirect vtable or message-based dispatch (not through further DLL exports).

## RTTI Class Inventory (74 classes)

Extracted from `.?AV` and `.?AU` RTTI patterns in the binary's `.rdata` section.

### Core Framework Classes (AV prefix — concrete, C++ vtable)

| Category | Classes |
|----------|---------|
| **Filter Graph Engine** | `AVFilterGraph`, `AVFilterHelper`, `AVScript` |
| **Copiers (re-mux)** | `AVVideoCopier`, `AVAsfCopier`, `AVAVICopier`, `AVMpg2Copier`, `AVStreamBufferCopier` |
| **Transcoders (re-encode)** | `AVVideoTranscoder`, `AVAsfTranscoder`, `AVAVITranscoder`, `AVMpg2Transcoder`, `AVStreamBufferTranscoder`, `AVDVTranscoder` |
| **Workers / Helpers** | `AVAsfCopyWorker`, `AVSyncReader`, `AVVideoCopierWrapper`, `AVVideoPlayerWrapper` |
| **Player** | `AVVideoPlayer` |
| **Custom DShow Filters** | `AVAudioRepackageFilter`, `AVCopyProgressInfoFilter`, `AVVideoRotationFilter`, `AVYUY2ToI420Filter` |
| **DShow Base Classes** | `AVCBaseFilter`, `AVCBasePin`, `AVCBaseInputPin`, `AVCBaseOutputPin`, `AVCBaseObject`, `AVCTransformFilter`, `AVCTransformInputPin`, `AVCTransformOutputPin`, `AVCTransInPlaceFilter`, `AVCTransInPlaceInputPin`, `AVCTransInPlaceOutputPin`, `AVCEnumMediaTypes`, `AVCEnumPins`, `AVCUnknown` |
| **Diagnostics** | `AVEventTracingProvider`, `AVAsfSampleTrace`, `AVVideoTrimTracing` |
| **ATL/COM Glue** | `AVCAtlModule@ATL@@`, `AVCComModule@ATL@@`, `AVCComObjectRootBase@ATL@@`, `AVCRegObject@ATL@@` |
| **Other** | `AVException@Base@@`, `AVRefCountBase@Base@@`, `AVtype_info@@` |
| **Nested (HMRAVSource)** | `AVAuthCredentials@HMRAVSource@@`, `AVAuthProvider@HMRAVSource@@` |

### Interface Classes (AU prefix — abstract/virtual)

| Category | Interfaces |
|----------|------------|
| **COM Interfaces** | `AUIUnknown@@`, `AUIBaseFilter@@`, `AUIMediaFilter@@`, `AUIPin@@`, `AUIEnumPins@@`, `AUIEnumMediaTypes@@`, `AUIMemInputPin@@`, `AUIMediaSeeking@@`, `AUIQualityControl@@`, `AUIPersist@@`, `AUIRegistrarBase@@`, `AUIServiceProvider@@`, `AUIValidateBinding@@`, `AUINonDelegatingUnknown@@`, `AUIAMovieSetup@@` |
| **Copier / Transcoder Interfaces** | `AUIVideoCopier@@`, `AUIVideoTrimVideoPlayer@@`, `AUIAsfCopyWorker@@`, `AUISyncReaderCallback@@` |
| **WM Interfaces** | `AUIWMReaderCallback@@`, `AUIWMReaderCallbackAdvanced@@`, `AUIWMStatusCallback@@` |
| **ATL** | `AU_ATL_MODULE70@ATL@@` |

## COM Interfaces Used (from RTTI + imports)

| Interface | Purpose |
|-----------|---------|
| `IVideoCopier` | Video copy operation control |
| `ITranscoder` | Transcoding operation control |
| `ICopier` | Base copier interface |
| `ICopierDirect` | Direct AVI copy interface |
| `IVideoTrimVideoPlayer` | Video preview/playback |
| `IAsfCopyWorker` | ASF/WMV copy worker |
| `ISyncReaderCallback` | Synchronous reader callback |
| `IWMReaderCallback` | WM Reader event callback |
| `IWMReaderCallbackAdvanced` | WM Reader advanced callback |
| `IWMStatusCallback` | WM status notifications |

## String Evidence from Binary

### Custom Error Codes
- `AVS_E_DECODER_FAILURE`
- `AVS_E_INVALID_DURATION_FILE`
- `AVS_E_NON_SEEKABLE_FILE`
- `AVS_E_SAMPLE_OVERSEEK`
- `AVS_E_UNSUPPORTED_FILE_TYPE`

### Filter Graph Strings
- `FilterGraph %08x pid %08x` — graph instance identification with PID
- `[Temporal Compression]` / `[NOT] Temporal Compression` — stream analysis
- `FilterGraph::RenderAllPins() failed.` — graph build failure

### AsfCopyWorker Debug Messages
- `AsfCopyWorker::Copy -- [finished].`
- `AsfCopyWorker::OnSample -- Reader Callback: Received a decompressed sample.`
- `AsfCopyWorker::OnStatus -- [WMT_EOF] {SetEvent}`
- `AsfCopyWorker::CopyMarker -- [%d] -- %s -- [Seconds: %0.5f]`
- `AsfCopyWorker::CopyAttribute -- Stream Number [%d] -- Attributes Count [%d]`
- `AsfCopyWorker::CopyCodecInfo -- Codec [%d] -- %s -- %s`
- `AsfCopyWorker::ForceKeyFrameInsertion.`

### File Format Strings
- `.TempWMV` — temp file suffix for WMV operations
- `-- VideoCopier Temp Filename : %s`
- `-- Mpg2Copier Profile Filename : %s`
- `-- StreamBufferCopier Profile Path : %s`

### Format-Related Subtype Constants
Over 200 `ASUBTYPE_*` entries referencing standard DirectShow media subtypes:
- Video: H264, MJPG, YUY2, NV12, UYVY, IYUV, YV12, DV, MPEG2_VIDEO, RGB32, ARGB32, etc.
- Audio: PCM, MPEG1Audio, MPEG2_AUDIO, DOLBY_AC3, DTS, WAVE, etc.
- MPEG: MPEG1Video, MPEG2_TRANSPORT, MPEG2_PROGRAM, etc.

### DShow Category Strings
- `AM_KSCATEGORY_AUDIO`, `AM_KSCATEGORY_VIDEO`, `AM_KSCATEGORY_RENDER`
- `AM_KSCATEGORY_SPLITTER`, `AM_KSCATEGORY_DATACOMPRESSOR`
- `ActiveMovieCategories` — standard filter enumeration
- `AudioRender`, `AudioRendererCategory` — audio output selection

## Registry Access

| Key Pattern | Purpose |
|-------------|---------|
| `SOFTWARE\Microsoft\VideoTrim\StreamBufferCopier Settings` | DVR-MS copier configuration |
| `SOFTWARE\Microsoft\Windows Live\Common\Movie Library` | Shared WLMM settings |
| `SOFTWARE\Microsoft\Windows Live\Movie Maker` | Movie Maker settings |
| `Software\Microsoft\Windows Live\Photo Gallery\VideoTrim` | Video trim preferences |
| `CLSID\{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}\InprocServer32` | DLL self-registration |

## DLL Dependencies

### Load-time Imports

| DLL | Key Functions |
|-----|---------------|
| MSVCR110.dll | VC++ 2012 CRT — 35 imports (memory, string, thread, exception) |
| KERNEL32.dll | 39 imports — file I/O, threading, sync, module loading |
| USER32.dll | 9 imports — message pump, painting (EVR/VMR preview) |
| ADVAPI32.dll | 22 imports — registry, ETW tracing, crypto |
| ole32.dll | 9 imports — COM initialization, monikers |
| OLEAUT32.dll | 3 ordinals — COM automation |
| WLXPhotoBase.dll | 10 imports — Base::Throw, memory management, OS version detection |
| SHELL32.dll | 1 ordinal |
| SHLWAPI.dll | 11 imports — path manipulation, stream creation |
| WMVCore.DLL | 6 imports — WMCreateReader, WMCreateWriter, WMCreateProfileManager, etc. |

### Delay-Load Import

| DLL | Functions |
|-----|-----------|
| WLXMP4Parser.dll | `AddMP4SourceFilter`, `IsMP4FilePlayable` — MP4/MOV support (loaded on demand) |

## Test Harness

A test harness is provided at `tests/WLXVideoTrim/test_trim.cpp` that:
1. Loads the DLL via `LoadLibraryW`
2. Resolves all 5 exports via `GetProcAddress`
3. Calls each export with NULL/zero and with dummy file paths
4. Logs HRESULT return values and handle outputs
5. Frees the DLL

### Compile & Run

```batch
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
cl /EHsc /Fe:test_trim.exe test_trim.cpp /link ole32.lib user32.lib
test_trim.exe
```

## Key Architectural Observations

1. **No standard COM server exports** — This DLL is not a self-contained COM server. It does not export `DllGetClassObject`, `DllCanUnloadNow`, or `DllRegisterServer`. Instead, it exposes raw factory functions. The host application (MovieMakerCore.dll) loads it directly and manages object lifetimes.

2. **Opaque handle pattern** — All exports return a `long&` handle (cookie). The host never receives a COM interface pointer directly. Instead, it passes this handle back into the DLL through other (unexported) dispatch mechanisms, likely through function pointers or vtables obtained via the handle.

3. **DirectShow + WMVCore hybrid** — Video processing uses DirectShow filter graphs for most formats, but WMV/ASF uses WMVCore.DLL directly for sample-accurate copy without re-encoding. MP4 support bridges through a custom WLXMP4Parser delay-loaded DLL.

4. **Format dispatch at export level** — `CreateVideoCopierFromMediaType` is the factory dispatcher: given an `AM_MEDIA_TYPE`, it selects between `AsfCopier`, `AVICopier`, `Mpg2Copier`, or `StreamBufferCopier`.

5. **Single-instance DLL registration** — The DLL registers one COM class (`{8095E7A5-...}`) for external use, but its primary interface is through the 5 exported factory functions used internally by MovieMakerCore.dll.
