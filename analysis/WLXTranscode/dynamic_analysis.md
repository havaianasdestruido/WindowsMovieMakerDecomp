# WLXTranscode.exe — Dynamic & Deep Static Analysis

## 1. RTTI Class Inventory (52 types)

### Concrete Classes (AV — C++ vtable implementations)

#### HMRAVSource Namespace (Source/Buffer/Processing Layer)

| Class | Category | Purpose |
|-------|----------|---------|
| `AVSourceFactory` | Factory | Factory for creating AV source instances |
| `AVSourceProxy` | Proxy | Cross-thread/cross-process source proxy |
| `AVSource` | Core | Core AV source — file to MF source reader |
| `AVSink` | Sink | Output sink for transcoded media |
| `DShowMFSourceReaderBuilder` | Builder | Builds MF source reader from DirectShow filters |
| `NativeMFSourceReaderBuilder` | Builder | Builds MF source reader from native MF source |
| `MFSourceReaderBuilder` | Builder | Abstract base for source reader construction |
| `AsyncSourceResolver` | Resolver | Asynchronously resolves media source URLs |
| `MFAsyncResult` | Async | MF async operation result wrapper |
| `MFByteStreamOnStream` | Stream | MF byte stream wrapping IStream |
| `MFByteStreamOnStreamAsyncResult` | Async | Async result for byte stream operations |
| `OnReadAsyncCallback` | Callback | Async read callback for byte stream |
| `OnWriteAsyncCallback` | Callback | Async write callback for byte stream |
| `MFRateControlHelper` | Helper | MF rate control (playback speed) |
| `XVideoProc` | Video Proc | Software video processing (colorspace, scaling) |
| `DXVA2VideoProc` | Video Proc | DXVA2 hardware-accelerated video processing |
| `TextureInterOp` | GPU | Base texture interop (D3D ↔ MF samples) |
| `TextureInterOpDX9` | GPU | Direct3D 9 texture interop |
| `TextureInterOpDX11` | GPU | Direct3D 11 texture interop |
| `VideoBuffer` | Buffer | Video sample buffer management |
| `SyncVideoSampleSource` | Source | Synchronous video sample source |
| `StreamSinkHelper` | Sink Helper | Stream sink helper for output |
| `StreamSinkHost` | Sink Host | Hosts stream sinks for media session |
| `AuthCredentials` | Security | Authentication credentials for protected content |
| `AuthProvider` | Security | Auth provider for DRM/protected content |

#### HMRTranscode Namespace (Transcoding Layer)

| Class | Category | Purpose |
|-------|----------|---------|
| `MFTranscodeJob` | Core | **Main transcoding job controller** — manages MF transcode topology lifecycle |
| `TranscodeMetadataParser` | Metadata | Parses codec metadata from source for transcode decisions |

### Interface Types (AU — abstract vtable interfaces)

| Interface | Purpose |
|-----------|---------|
| `IAVSource` | AV source interface — media access abstraction |
| `IAVSourceFactory` | Factory interface for creating AV sources |
| `IAVSourceFactoryInternal` | Internal factory interface |
| `IAVSink` | AV sink interface — output abstraction |
| `IAVStreamSink` | Stream sink interface |
| `IAVSampleSource` | Sample source interface |
| `IVideoSampleSource` | Video-specific sample source |
| `IAVProcessor` | AV processing interface |
| `IVideoProcessor` | Video processing interface (software + hardware) |
| `IValidateBinding` | DRM binding validation interface |
| `IServiceProvider` | COM service provider |
| `IUnknown` | Standard COM |
| `IMFAsyncCallback` | MF async callback interface |
| `IMFAsyncResult` | MF async result interface |
| `IMFByteStream` | MF byte stream interface |
| `IMFSourceReaderCallback` | MF source reader callback |
| `_ATL_MODULE70` | ATL module v7 |

### Infrastructure Classes

| Class | Purpose |
|-------|---------|
| `CATLModule` | ATL module instance |
| `CComObjectRootBase` | ATL COM object root |
| `RefCountBaseMultiThreaded` | Thread-safe reference counting base |
| `Exception` | `Base::Exception` — structured exception |
| `type_info` | C++ RTTI metadata |

---

## 2. Command-Line Arguments

### Complete Argument Set

Found as string table entries in `.data` section at offset ~2950:

| Argument | Hypothesized Purpose | Evidence |
|----------|---------------------|----------|
| `-mw` | Movie Writer — output file path | First in sequence, likely primary output param |
| `-mh` | Movie Header — metadata header file | Follows `-mw`, before format params |
| `-mfps` | Movie FPS — target frame rate | Format-specific float parameter |
| `-mvbr` | Movie Video Bit Rate — target video bitrate | Integer bitrate value |
| `-q` | Quiet / Query mode | Short flag, no value |
| `-mkfs` | Movie Key Frame Size — keyframe interval | GOP size configuration |
| `-fsp` | File Source Path — input file path | Source file specification |
| `-rd` | Raw Data — raw data mode flag | Possibly raw stream passthrough |
| `-vsm` | Video Stream Map — video stream selection | Stream mapping/index |
| `-mc` | Movie Codec — output codec selection | Codec FOURCC or profile ID |
| `-tf` | Transcode Format — output container format | Container format specification |
| `-pph` | Pre-Process Header — header pre-processing | Header parsing flag |
| `-ms` | Media Source — media source specification | Found in different context (file format table at offset 14800) |

### Expected Invocation Pattern

```
WLXTranscode.exe -fsp <input.wmv> -mw <output.mp4> -mc H264 -mvbr 8000000 -mfps 30 -mkfs 30 -tf mp4 -mh <metadata> -pph
```

### Launching Context

MovieMakerCore.dll launches WLXTranscode.exe as a **child process** (not COM activation). The transcoding job parameters are serialized to command-line arguments. The process communicates results via:
- Exit code (HRESULT encoding)
- Output file creation
- Possibly stderr/stdout for progress

---

## 3. IPC Mechanism

### Primary: Command-Line + Process Monitoring

```
MovieMakerCore.dll
  → CreateProcessW("WLXTranscode.exe", "-fsp ... -mw ... -mc ...", ...)
  → WaitForSingleObject(hProcess) or APC callback
  → GetExitCodeProcess(hProcess, &dwExitCode)
  → HRESULT = (HRESULT)dwExitCode
```

### Secondary: COM (Internal)

- `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)` — COM for MF API
- `CoCreateInstance` — for MF objects (source resolver, session, etc.)
- `CLSIDFromString` / `StringFromCLSID` — CLSID manipulation for MF services
- `PropVariantClear` — PROPVARIANT cleanup for MF attributes

### No Named Pipes / Shared Memory

No evidence of IPC beyond COM remoting and process exit codes.

---

## 4. Media Foundation Transcode Pipeline

### Initialization Sequence

1. `MFStartup(MF_VERSION, MFSTARTUP_LITE)` — initialize MF platform
2. `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)` — COM for MF
3. `MFLockPlatform` — lock MF platform DLLs in memory
4. `MFCreateAttributes(&pAttributes, 0)` — create attribute store

### Source Resolution

1. `MFCreateSourceResolver` — create source resolver
2. `MFCreateSourceResolver::CreateObjectFromURL` — resolve input file
3. `SHGetPropertyStoreFromParsingName` — get property store for metadata
4. `PSCreateMemoryPropertyStore` — in-memory property store for custom metadata

### Transcode Profile Configuration

1. `MFCreateTranscodeProfile` — create transcode profile
2. `MFTranscodeGetAudioOutputAvailableTypes` — enumerate audio output types
3. `MFCreateMediaType` — configure audio/video output types
4. Profile attributes: `MF_MT_MAJOR_TYPE`, `MF_MT_SUBTYPE`, `MF_MT_FRAME_SIZE`, `MF_MT_FRAME_RATE`, `MF_MT_AVG_BITRATE`

### Topology Construction

1. `MFCreateTranscodeTopology` — create transcode topology from source + profile
2. `MFCreateTopologyNode` — create custom topology nodes (for DXVA2, etc.)
3. `MFGetService` — query MF services from topology objects

### Session Execution

1. `MFCreateMediaSession` — create MF media session
2. `IMFMediaSession::SetTopology` — set transcode topology
3. `IMFMediaSession::Start(NULL)` — begin transcoding
4. `MFPutWorkItemEx` — queue async work items for progress polling
5. `MFInvokeCallback` — invoke completion callbacks
6. `MFShutdown` — cleanup

---

## 5. Hardware Acceleration Stack

### DXVA2 Path (Primary for HD Content)

| API | DLL | Purpose |
|-----|-----|---------|
| `Direct3DCreate9Ex` | d3d9.dll | Create D3D9Ex device for DXVA |
| `D3D11CreateDevice` | d3d11.dll | Create D3D11 device (alternate path) |
| `DXVA2CreateDirect3DDeviceManager9` | dxva2.dll | Create D3D9 device manager for DXVA |
| `DXVA2CreateVideoService` | dxva2.dll | Create DXVA2 video decode/processing service |

### Texture Interop Classes

| Class | Purpose |
|-------|---------|
| `TextureInterOp` | Base class for GPU ↔ CPU texture transfer |
| `TextureInterOpDX9` | Direct3D 9 texture interop (DXVA2 path) |
| `TextureInterOpDX11` | Direct3D 11 texture interop (MFT path) |
| `DXVA2VideoProc` | DXVA2 hardware video processor (deinterlace, scale, colorspace) |
| `XVideoProc` | Software video processor (fallback) |

### DXVA2 Decode Modes Referenced

- `DXVA_ModeMPEG2_A` — MPEG-2 A-profile
- `DXVA_ModeMPEG2_B` — MPEG-2 B-profile
- `DXVA_ModeMPEG2_C` — MPEG-2 C-profile
- `DXVA_ModeMPEG2_D` — MPEG-2 D-profile

### OS-Dependent Path Selection

```cpp
if (Base::OS::IsWin8OrGreater()) {
    // D3D11 + MFTransform (hardware MFT) path
    D3D11CreateDevice(...);
} else if (Base::OS::IsWin7OrGreater()) {
    // D3D9 + DXVA2 path
    Direct3DCreate9Ex(...);
    DXVA2CreateVideoService(...);
} else {
    // Software-only XVideoProc fallback
}
```

---

## 6. Video/Audio Format Support

### Video Codecs (FOURCCs → MF Subtypes)

| FOURCC | MF Format String | Codec | Direction |
|--------|-----------------|-------|-----------|
| `H264` | `MFVideoFormat_H264` | H.264/AVC | Encode + Decode |
| `WVC1` | `MFVideoFormat_WVC1` | VC-1 (SMPTE) | Decode only |
| `WMV3` | `MFVideoFormat_WMV3` | WMV 9 | Decode only |
| `WMV2` | `MFVideoFormat_WMV2` | WMV 8 | Decode only |
| `WMV1` | `MFVideoFormat_WMV1` | WMV 7 | Decode only |
| `MP43` | `MFVideoFormat_MP43` | MPEG-4 (MS variant) | Decode only |
| `MP4V` | `MFVideoFormat_MP4V` | MPEG-4 Part 2 | Decode only |
| `MP4S` | `MFVideoFormat_MP4S` | MPEG-4 Simple Profile | Decode only |

### Additional Video Formats (from MF format strings)

| Format | Description |
|--------|-------------|
| `MFVideoFormat_MPG1` | MPEG-1 |
| `MFVideoFormat_MPG2` | MPEG-2 |
| `MFVideoFormat_MJPG` | Motion JPEG |
| `MFVideoFormat_DV25/DV50/DVC/DVHD/DVSD/DVSL/DVH1` | DV family |
| `MFVideoFormat_I420/IYUV/NV12/P010/P016` | YUV planar formats |
| `MFVideoFormat_RGB24/RGB32/RGB555/RGB565/RGB8` | RGB packed formats |
| `MFVideoFormat_YUY2/UYVY/YV12/YVU9/YVYU` | YUV packed formats |
| `MFVideoFormat_ARGB32` | ARGB 32-bit |
| `MFVideoFormat_AYUV` | AYUV 4:4:4 |

### Audio Formats

| Format | Description |
|--------|-------------|
| `MFAudioFormat_PCM` | PCM |
| `MFAudioFormat_Float` | 32-bit float |
| `MFAudioFormat_MP3` | MP3 |
| `MFAudioFormat_AAC` | AAC |
| `MFAudioFormat_Dolby_AC3_SPDIF` | Dolby AC3 (SPDIF) |
| `MFAudioFormat_DTS` | DTS |
| `MFAudioFormat_WMAudioV8/WMAudioV9/WMAudio_Lossless` | WMA family |
| `MFAudioFormat_ADPCM/DVI_ADPCM` | ADPCM variants |
| `MFAudioFormat_MPEG` | MPEG audio |
| `MFAudioFormat_MULAW/MSP1/DRM` | Legacy formats |

### Container Formats (from file extensions)

- `.wmv` — Windows Media Video (ASF)
- `.mp4` — MP4/MOV
- `.m4a` — MP4 Audio
- `.wma` — Windows Media Audio
- `.wtv` — Windows TV (DVR)
- `.dvr-ms` — DVR Media Stream

---

## 7. WM/ Metadata Property Map

62 Windows Media metadata properties mapped for transcode passthrough:

| Property | Purpose |
|----------|---------|
| `WM/Title`, `WM/Author`, `WM/Description` | Basic metadata |
| `WM/AlbumArtist`, `WM/AlbumTitle`, `WM/Genre` | Music metadata |
| `WM/TrackNumber`, `WM/Year`, `WM/Lyrics` | Track info |
| `WM/WMCollectionID`, `WM/WMContentID` | WM content identification |
| `WM/WMRVContentProtected`, `WM/WMRVContentExpirationDate` | DRM metadata |
| `WM/EncodingSettings`, `WM/EncodingTime` | Encoding parameters |
| `WM/ToolName` | Identifies encoding tool |
| `WM/Picture` | Embedded album art |
| `WM/SharedUserRating` | User rating |

---

## 8. ETW Tracing Infrastructure

### ETW Provider Registration

```c
RegisterTraceGuidsW(
    &WLXTranscodeGuid,      // Provider GUID (embedded in binary)
    &WLXTranscodeGuid,      // Registration handle
    WLXTranscodeTraceGuids, // Trace GUID array
    count,                  // Number of GUIDs
    NULL, NULL, NULL, NULL, // Callbacks
    &RegHandle              // Output registration handle
);
```

### Trace Events

| API | Purpose |
|-----|---------|
| `TraceMessage` | Emit structured trace message |
| `TraceEvent` | Emit trace event |
| `GetTraceLoggerHandle` | Get trace session handle |
| `GetTraceEnableLevel` | Query trace enable level |
| `GetTraceEnableFlags` | Query trace enable flags |
| `UnregisterTraceGuids` | Unregister provider |

---

## 9. Cryptography

### Crypto Operations

| API | Purpose |
|-----|---------|
| `CryptAcquireContextW` | Acquire crypto context (MS_ENH_RSA_AES_PROV) |
| `CryptCreateHash` | Create hash object (SHA-256 or MD5) |
| `CryptHashData` | Hash data for content verification |
| `CryptSignHashW` | Sign hash (profile authentication) |
| `CryptImportKey` | Import encryption key |
| `CryptDestroyKey` | Release key |
| `CryptDestroyHash` | Release hash |
| `CryptReleaseContext` | Release crypto context |

### Purpose

Likely used for **transcode profile authentication** — verifying that the `.wlvs` profile file has not been tampered with, and **content integrity** — generating checksums of transcode output for validation.

---

## 10. Error Handling

### Custom Error Codes (from string table)

| Error | Meaning |
|-------|---------|
| `AVS_E_DECODER_FAILURE` | Decoder failed to initialize or process samples |
| `AVS_E_INVALID_DURATION_FILE` | Invalid file duration metadata |
| `AVS_E_NON_SEEKABLE_FILE` | File does not support seeking |
| `AVS_E_SAMPLE_OVERSEEK` | Sample position exceeds seek range |
| `AVS_E_UNSUPPORTED_FILE_TYPE` | File format not supported |

### MF Error Codes (comprehensive set)

130+ `MF_E_*` error codes embedded for error reporting, including:
- `MF_E_TOPO_CODEC_NOT_FOUND` — no codec for topology node
- `MF_E_INVALID_FILE_FORMAT` — corrupt input
- `MF_E_HW_MFT_FAILED_START_STREAMING` — DXVA hardware MFT failed
- `MF_E_DRM_UNSUPPORTED` — DRM not supported
- `MF_E_UNSUPPORTED_FORMAT` — format not supported

### HRESULT Formatting

The binary includes `FormatMessageW` for human-readable HRESULT conversion:
- `<Failed: 0x%08X>` — generic failure format
- `<IUnknown*>` — unknown COM object
- `<Too Long>` — string truncation
- `<NULL>` — null value representation
- `<BLOB>` — binary data representation

---

## 11. Process Configuration

### Console Subsystem

WLXTranscode.exe uses **Windows CUI (console)** subsystem (3), unusual for a media processing tool. This is because:
- CUI allows redirecting stdout/stderr for diagnostics
- No window creation overhead
- Simpler for child process spawning from MovieMakerCore.dll

### Thread Execution State

`SetThreadExecutionState` — prevents system sleep during transcoding:
```c
SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
// ... transcoding ...
SetThreadExecutionState(ES_CONTINUOUS);  // allow sleep again
```

### DLL Directory Hardening

`SetDllDirectoryW("")` — removes current directory from DLL search path (security best practice).

---

## 12. File Format Detection Table

Found at offset ~14800 in `.data`, a file format association table mapping extensions to codec capabilities:

| Extension | Codec Flags |
|-----------|------------|
| `.wtv` | Windows TV recording format |
| `.dvr-ms` | DVR media stream |
| `.wmv` | Windows Media Video |
| `.mp4` | MPEG-4 Part 14 |
| `.m4a` | MPEG-4 Audio |
| `.wma` | Windows Media Audio |

---

## 13. Registry Keys Accessed

| Key | Purpose |
|-----|---------|
| `SOFTWARE\Microsoft\Windows Live\Movie Maker` | Movie Maker settings/configuration |
| `SOFTWARE\Microsoft\Windows Live\Common\Movie Library` | Shared movie library metadata |

---

## 14. Key Findings

1. **Standalone transcoder** — Unlike other WLX DLLs, WLXTranscode.exe is a standalone process with no exports, activated via command-line arguments. The VERSIONINFO `WLXTranscoder.dll` name confirms it was originally a DLL, later converted to an EXE for process isolation.

2. **Full MF pipeline** — Implements the complete Media Foundation transcode pipeline: source resolution → profile configuration → topology construction → session execution → output.

3. **Hardware acceleration** — Dual GPU path: DXVA2 (D3D9) for Win7 and D3D11 MFT for Win8+, with software XVideoProc fallback.

4. **Comprehensive codec support** — 8+ video codecs (H.264, VC-1, WMV 7/8/9, MPEG-4 variants), 15+ audio formats, 6+ container formats.

5. **No COM server** — Unlike WLXCodecHost.exe, this binary does not register as a COM server. It is a process launched by MovieMakerCore.dll with command-line arguments and monitored via exit code.

6. **Crypto and ETW** — Production-grade telemetry (ETW) and content authentication (CryptoAPI) for diagnostics and profile integrity.

7. **62 metadata properties** — Complete WM/ metadata passthrough ensures all media properties survive the transcode process.

8. **DXVA2 mode table** — Full MPEG-2 DXVA decode mode enumeration suggests legacy support for MPEG-2 content from DVD/DVR sources.
