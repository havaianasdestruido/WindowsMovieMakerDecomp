# Shared Media Foundation DLLs Analysis

## RSCMFT.dll — Rolling Shutter Correction MFT

### Overview
- **File Description**: "Rolling Shutter Correction MFT" (from VERSIONINFO string resource)
- **Product**: Photo Gallery
- **Version**: 16.4.3528.0331 (ship) — `16.4.3528.0331_ship.client.main.w5m4`
- **Image Size**: 288 KB (0x48000)
- **PE32 x86**, Linker v11.0 (MSVC 2012), ASLR+NX
- **Timestamp**: 2014-04-01 01:28:13 UTC (PE header), 01:17:26 UTC (export table)
- **PDB**: `{8BE07FA3-8940-469E-A496-FED16CCA5FC4}`, 1, `RSCMFT.pdb`
- **Entry Point**: 0x1003C472

### Sections
| Section | VA | VirtSize | RawSize | Flags | Purpose |
|---------|------|----------|---------|-------|---------|
| `.text` | 0x1000 | 0x3F7A3 | 0x3F800 | Code/Execute/Read | ~254 KB code |
| `.data` | 0x41000 | 0x2980 | 0x800 | InitData/Read/Write | ~10.5 KB globals, vtables |
| `.rsrc` | 0x44000 | 0x3E8 | 0x400 | InitData/Read | ~1 KB version/resources |
| `.reloc` | 0x45000 | 0x29F0 | 0x2A00 | InitData/Discardable/Read | ~10.5 KB relocations |

### Exports (4 — Standard COM DLL)
| Ordinal | RVA | Name |
|---------|-----|------|
| 1 | 0x000040BB | `DllCanUnloadNow` |
| 2 | 0x000040CC | `DllGetClassObject` |
| 3 | 0x000040DF | `DllRegisterServer` |
| 4 | 0x00004125 | `DllUnregisterServer` |

### Imports

**MSVCR110.dll** (37 functions): CRT core — `memcpy`, `memmove`, `memset`, `qsort`, `ceil`, `floor`, `free`, `_malloc_crt`, `_calloc_crt`, `_purecall`, C++ exception handling (`__CxxFrameHandler3`, `__CppXcptFilter`, `?terminate@@YAXXZ`), string (`wcsncpy_s`, `wcscpy_s`, `_wcsicmp`, `_vsnwprintf`), math (`_CIatan2`, `_libm_sse2_cos/pow/sin/sqrt_precise`), CRT init (`_initterm`, `_initterm_e`, `_amsg_exit`), RTTI (`__clean_type_info_names_internal`), locking (`_lock`, `_unlock`)

**KERNEL32.dll** (42 functions): Thread pool (`CreateThreadpool`, `CreateThreadpoolWork`, `CloseThreadpool`, `CloseThreadpoolWork`, `QueueUserWorkItem`), sync (`CreateMutexW`, `CreateEventW`, `SetEvent`, `ResetEvent`, `ReleaseMutex`, `WaitForSingleObject`, `WaitForSingleObjectEx`, `InitializeCriticalSectionAndSpinCount`, `EnterCriticalSection`, `LeaveCriticalSection`, `DeleteCriticalSection`), file I/O (`CreateFileW`, `ReadFileEx`, `WriteFileEx`, `SetFilePointerEx`, `GetFileSizeEx`, `SetEndOfFile`, `DeleteFileW`, `GetTempFileNameW`, `GetTempPathW`), memory (`HeapAlloc`, `HeapFree`, `GetProcessHeap`), module (`GetModuleHandleW`, `GetProcAddress`, `GetModuleFileNameW`), threading (`CreateThread`, `GetCurrentThreadId`, `Sleep`, `SwitchToThread`), interlocked (`InterlockedIncrement/Decrement/Exchange/CompareExchange`), time (`QueryPerformanceCounter`, `QueryPerformanceFrequency`, `GetSystemTimeAsFileTime`, `GetTickCount64`), OS info (`GetSystemInfo`, `IsProcessorFeaturePresent`, `IsDebuggerPresent`), PE (`EncodePointer`, `DecodePointer`), `DisableThreadLibraryCalls`, `GetLastError`

**ADVAPI32.dll** (4 functions): Registry — `RegCloseKey`, `RegDeleteTreeW`, `RegCreateKeyExW`, `RegSetValueExW`

**MFPlat.DLL** (7 functions — Media Foundation platform):
- `MFTRegister` / `MFTUnregister` — Register/unregister MFT with MF
- `MFGetStrideForBitmapInfoHeader` — Calculate bitmap stride
- `MFCreateVideoSampleAllocatorEx` — Create video sample allocator
- `MFCreateMediaType` — Create media type
- `MFCopyImage` — Copy image data
- `MFCreateAttributes` — Create attribute store

**ole32.dll** (1 function): `StringFromGUID2` — GUID to string conversion

**SHLWAPI.dll** (1 ordinal): ordinal 219 — likely `SHRegGetValueW` or path utility

### COM/MFT Registration
The DLL registers as a Media Foundation Transform (MFT) via `MFTRegister`/`MFTUnregister`. No CLSID string literals are embedded in the binary — the CLSID is defined in the ATL module's `.rgs` file (not embedded as string resource). Registry keys under `InProcServer32` with `ThreadingModel` are referenced in the resource version info.

### Internal Classes (from ASCII strings / RTTI)
The DLL uses a custom AVC (Audio Video Components?) class hierarchy:

**Core MFT:**
- `AVCRSC_MFT` — Main Rolling Shutter Correction MFT
- `AVCRSC_HS` — Rolling Shutter Correction half-size/downscale path
- `AVCCustomWarpTransformUV` / `AVCCustomWarpTransformY` — Per-plane geometric warping transforms
- `AVCRSCorrection` — Correction algorithm core

**Factory:**
- `AVCClassFactory` — COM class factory
- `AVCErrorBase` — Error handling base

**Image Processing Pipeline:**
- `AVCPyramid`, `AVCLevelImg`, `AVCImg` — Multi-scale image pyramid representation
- `CCompositeImg`, `CTypedImg`, `CTypedPyramid`, `CImageCache`, `CImgReaderWriter` — Image composite/cache types
- `AVCFileBlobStore`, `AVIBlobStore`, `AVIBlobCallback`, `AVCICBlobCallback` — Blob storage for image data
- `AVITaskState`, `AVCTaskStatus`, `AVCTaskStatusDefault`, `AVCTaskStatusEvent`, `AVCTaskStatusPoll` — Task state machine
- `AVIAddressGenerator`, `AVC3x3TransformAddressGen` — Address generation for transforms
- `AVCTransformGraphNode`, `AVCTransformGraphNoSrcNode`, `AVCTransformTaskTraverse`, `AVCTransformTaskStatus` — Transform graph traversal
- `AVIImageReader`, `AVIImageWriter`, `AVIImageReaderWriter`, `AVIImageTransform` — I/O abstraction

**Algorithm Stages (from Unicode strings):**
1. `Correspondences` — Feature matching between frames
2. `Derivatives` — Gradient computation
3. `Initialize DFlow` — Dense optical flow initialization
4. `Compute Stabilization` — Stabilization path estimation
5. `Compute Correction` — Per-frame warp computation
6. `Image Warping` — Final warp application
7. `DestMap Computation` — Destination mapping
8. `Error Image` — Error metrics

**Sampling:**
- `LinearSampler`, `LinearSampler2` — Interpolation methods
- `LumaType` — Luminance processing

**Memory/Threading:**
- `CTaskManager`, `AVCTaskProgress` — Thread pool task management
- `CVec2` — 2D vector math
- `AVCTextureTriplet` — Texture triplet (likely GPU interop)

### Algorithm Summary
The MFT corrects rolling shutter distortion in video. It:
1. Computes dense correspondences between frames (optical flow)
2. Estimates stabilization path (smooth camera motion)
3. Computes per-frame correction warp (custom warp transform on Y and UV planes separately)
4. Uses image pyramids for multi-scale processing
5. Likely runs on threadpool tasks for performance

The `_libm_sse2_*` math functions and `_CIatan2` indicate SSE2-optimized math, likely for the optical flow and warp computation.

### Certificate GUIDs (code signing, not COM)
- `4faf0b71-ad37-4aa3-a671-76bc052344ad`
- `2860b52e-c4a3-454d-bc1e-32c5add17e90`

---

## MPG4DEMUX.dll — MPEG-4 Demultiplexer

### Overview
- **File Description**: "MPEG-4 Demultiplexer" (from VERSIONINFO — truncated in resource to "MPEG")
- **Product**: Photo Gallery
- **Version**: 16.4.3528.0331 (ship) — `16.4.3528.0331_ship.client.main.w5m4`
- **Image Size**: 176 KB (0x2B000)
- **PE32 x86**, Linker v11.0 (MSVC 2012), ASLR+NX
- **Timestamp**: 2014-04-01 01:28:15 UTC (PE header), 01:28:12 UTC (export table)
- **PDB**: `{7A27A4CE-69B7-4A00-9C73-C687AA8E9ED0}`, 1, `Mpg4Demux.pdb`
- **Entry Point**: 0x10022641

### Sections
| Section | VA | VirtSize | RawSize | Flags | Purpose |
|---------|------|----------|---------|-------|---------|
| `.text` | 0x1000 | 0x22D46 | 0x22E00 | Code/Execute/Read | ~142 KB code |
| `.data` | 0x24000 | 0x1718 | 0x1400 | InitData/Read/Write | ~5.6 KB globals |
| `.rsrc` | 0x26000 | 0x3E0 | 0x400 | InitData/Read | ~1 KB version/resources |
| `.reloc` | 0x27000 | 0x394E | 0x3A00 | InitData/Discardable/Read | ~14.5 KB relocations |

### Exports (4 — Standard COM DLL)
| Ordinal | RVA | Name |
|---------|-----|------|
| 1 | 0x0001F8B2 | `DllCanUnloadNow` |
| 2 | 0x0001F8D0 | `DllGetClassObject` |
| 3 | 0x00009CD2 | `DllRegisterServer` |
| 4 | 0x00009CE7 | `DllUnregisterServer` |

### Imports

**MSVCR110.dll** (28 functions): CRT core — `memcpy`, `memcmp`, `memset`, `free`, `_malloc_crt`, `_calloc_crt`, `_purecall`, C++ exception handling, string (`_vsnwprintf`, `_itow_s`, `_ultow_s`, `wcscat_s`, `swscanf_s`, `wcstol`, `wcstoul`, `_wcsicmp`), CRT init

**KERNEL32.dll** (31 functions): Module loading (`GetModuleHandleW`, `GetProcAddress`, `GetModuleFileNameA`, `FreeLibrary`), threading (`CreateThread`, `GetCurrentThread`, `GetCurrentThreadId`, `SetThreadPriority`), sync (`CreateEventW`, `SetEvent`, `ResetEvent`, `WaitForSingleObject`, `InitializeCriticalSection`, `EnterCriticalSection`, `LeaveCriticalSection`, `DeleteCriticalSection`, `InterlockedIncrement/Decrement/Exchange`), OS info (`GetVersionExW`, `GetTickCount64`, `GetSystemTimeAsFileTime`, `IsProcessorFeaturePresent`, `IsDebuggerPresent`, `QueryPerformanceCounter`), string (`lstrcmpW`, `lstrlenW`, `lstrlenA`), conversion (`WideCharToMultiByte`, `MultiByteToWideChar`), PE (`EncodePointer`, `DecodePointer`), `DisableThreadLibraryCalls`, `GetLastError`

**USER32.dll** (1 function): `SetRect` — Rectangle manipulation

**ADVAPI32.dll** (8 functions): Registry — `RegOpenKeyExW`, `RegEnumKeyExW`, `RegCreateKeyW`, `RegSetValueW`, `RegSetValueExW`, `RegCloseKey`, `RegDeleteKeyW`; Tracing — `TraceMessage` (ETW)

**ole32.dll** (10 functions): COM core — `CoInitialize`, `CoUninitialize`, `CoCreateInstance`, `CoFreeUnusedLibraries`, `CoGetMalloc`, `CoTaskMemAlloc`, `CoTaskMemFree`, `CLSIDFromString`, `StringFromGUID2`, `PropVariantClear`, `PropVariantCopy`

**OLEAUT32.dll** (4 ordinals): ordinals 2, 4, 12, 185 — likely `SysAllocString`, `SysFreeString`, `VariantInit`, `VariantClear`

**MFPlat.DLL** (9 functions — Media Foundation platform):
- `MFCreatePresentationDescriptor` — Create presentation descriptor
- `MFCreateMemoryBuffer` — Create MF memory buffer
- `MFCreateMediaType` — Create media type
- `MFCreateStreamDescriptor` — Create stream descriptor
- `MFInitMediaTypeFromAMMediaType` — Convert DirectShow media type to MF
- `MFInitAMMediaTypeFromMFMediaType` — Convert MF media type to DirectShow
- `MFHeapAlloc` / `MFHeapFree` — MF heap allocator
- `MFllMulDiv` — 64-bit multiplication/division utility

**PROPSYS.dll** (1 function): `PropVariantChangeType` — Property system variant conversion

### COM Registration
The DLL uses `CoCreateInstance`, `CLSIDFromString`, and `StringFromGUID2` for COM object creation. Registry keys include `CLSID`, `InProcServer32`, and `ThreadingModel` references. The CLSID is defined in the ATL module's `.rgs` registry script (not embedded as a string literal).

### Internal Classes — MPEG-4 Demux Core
**Main filter:**
- `AVCMPEG4Demultiplexer` — Main demux filter
- `AVCMPEG4DemuxInputPin` — Input pin (reads MP4 file)
- `AVCMPEG4DemuxOutputPin` / `CMPEG4DemuxOutputPin` — Output pins (video/audio streams)

### Internal Classes — QuickTime/MP4 Atom Parser
The demuxer has a **full QuickTime/MP4 container parser** built in:

**Movie/Container:**
- `AVCQTMovie`, `AVCQTTrack`, `AVCQTMedia` — Top-level QT structures
- `AVCQTMovieEditor` — Movie editing support
- `AVCQTUserData` — User metadata

**Atoms (boxes):**
- `AVCQTAtom`, `AVCQTFullAtom` — Base atom types
- `AVCQTChunkOffsetAtom`, `AVCQTChunkLargeOffsetAtom` — `stco`/`co64`
- `AVCQTSampleSizeAtom` — `stsz`
- `AVCQTSampleToChunkMapAtom` — `stsc`
- `AVCQTTimeToSampleMapAtom` — `stts`
- `AVCQTCompositionTimeToSampleMapAtom` — `ctts`
- `AVCQTSyncSampleAtom` — `stss`
- `AVCQTSampleTable` — Overall sample table
- `AVCQTDataRefInfoAtom` — `dref`
- `AVCQTFreeSpaceAtom` — `free`
- `AVCQTTrackHeaderAtom` — `tkhd`
- `AVCQTMediaInformationAtom` — `minf`
- `AVCQTHandlerAtom` — `hdlr`
- `AVCQTInitialObjectDescriptorAtom` — `iods`
- `AVCQTESDSAtom` — `esds` (MPEG-4 decoder config)
- `AVCQTDataBuffer` — Atom data buffer
- `AVCQTMetadataAtom`, `AVCQTMetadataCollection`, `AVCQTMetadataRecord` — Metadata atoms

**Media Handlers:**
- `AVCQTMediaHandler` / `AVCQTVideoMediaHandler` / `AVCQTMediaInformationAtom` — Media type routing
- `AVCQTAudioStreamHandler` / `AVCQTCompressedAudioStreamHandler` / `AVCQTUncompressedAudioStreamHandler` — Audio track handling
- `AVCQTMediaHandler` / `AVCQTVideoMediaHandler` — Video track handling

**Sample Descriptions:**
- `AVCMP4VideoSampleDescription` / `AVCMP4AudioSampleDescription` — MP4 codec configs
- `AVCAVCVideoSampleDescription` — AVC/H.264 config (`avcC`/`avc1`)
- `AVCVC1VideoSampleDescription` — VC-1 config
- `AVCMJPEGVideoSampleDescription` — MJPEG config
- `AVCWaveAudioSampleDescription` — WAVE format audio (`ima4`/`lpcm`)
- `AVCQTAudioSampleDescription` / `AVCQTVideoSampleDescription` — Generic QT descriptions
- `AVCQTLegacyAudioSampleDescription` — Legacy QT audio (`QDMA`-era)
- `AVCQTCompressedAudioStreamHandler` / `AVCQTUncompressedAudioStreamHandler` — Audio format-handling classes

**MP4 Descriptors:**
- `AVCMP4Descriptor`, `AVCMP4DecoderConfigDescriptor` — MPEG-4 system descriptors (from `esds`)
- `AVCMP4MetadataAtom`, `AVCMP4MetadataHandler`, `AVCMP4StreamHandler`, `AVIMP4StreamHandler` — MP4-specific metadata

### Internal Classes — DirectShow Base Classes
Standard DirectShow base class pattern with AVC prefix:
- `AVCBaseFilter`, `AVCBasePin`, `AVCBaseOutputPin`, `AVCBaseObject`, `AVCBaseUnknown`
- `AUIBaseFilter`, `AUIMediaFilter`, `AUIPin`, `AUIEnumPins`, `AUIEnumMediaTypes`, `AUIEnumAtoms`
- `AUIAMovieSetup`, `AUIMediaSeeking`, `AUIPersist`, `AUIQualityControl`
- `AVCAggUnknown` — Aggregated unknown (COM tear-off)
- `AVCAMThread` / `AVCOutputPinThread` — Threading for output pins
- `AVCEnumAtoms`, `AVCEnumMediaTypes`, `AVCEnumPins` — Enumerator implementations
- `AVCMSMetadataAtom`, `AVCMSMetadataCollection`, `AVCMSMetadataRecord` — CMS metadata support
- `AVCMFQTDataBuffer` — MF-based QT data buffer
- `AVCMetadataRecord` — Metadata record base

### Architecture Summary
This is a **DirectShow + Media Foundation hybrid** demultiplexer for MP4 files:

1. **Input**: Reads MP4/MOV/3GP files via DirectShow input pin (`AVCMPEG4DemuxInputPin`)
2. **Parsing**: Full QuickTime atom parser (`AVCQT*` hierarchy) navigates the MP4 box structure
3. **Sample Extraction**: `AVCQTSampleTable` maps chunks → samples → timestamps
4. **Codec Config**: Extracts codec-specific configuration (`avcC` for H.264, `esds` for AAC/MPEG-4 audio, VC-1, MJPEG)
5. **Output**: Splits into video and audio streams via `AVCMPEG4DemuxOutputPin`
6. **MF Bridge**: Uses MF platform (`MFPlat.DLL`) to create presentation/stream descriptors and convert between DirectShow (`AM_MEDIA_TYPE`) and Media Foundation (`IMFMediaType`) formats
7. **MF Heap**: Uses `MFHeapAlloc`/`MFHeapFree` for buffer management (interop with MF pipeline)

### Certificate GUIDs (code signing, not COM)
- `4faf0b71-ad37-4aa3-a671-76bc052344ad`
- `2860b52e-c4a3-454d-bc1e-32c5add17e90`
