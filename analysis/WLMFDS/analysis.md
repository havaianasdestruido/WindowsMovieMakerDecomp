# WLMFDS.dll Analysis — Windows Live Media Foundation DirectShow Bridge

**File**: `undecomp/Shared/WLMFDS.dll`
**Size**: ~417 KB (427,208 bytes on disk)
**Description**: Media Foundation <-> DirectShow bridge DLL. Exposes DirectShow filters as Media Foundation sources/streams.

## PE Structure

| Field | Value |
|-------|-------|
| **Machine** | x86 (0x14C) |
| **Linker** | MSVC 11.00 (VS 2012) |
| **Image Base** | 0x10000000 |
| **Image Size** | 0x67000 (421,888 bytes) |
| **Entry Point** | 0x1004FDE2 |
| **Subsystem** | Windows GUI (2) |
| **OS / Subsystem Ver** | 6.02 / 6.00 (Windows 8) |
| **DLL Characteristics** | Dynamic Base (ASLR), NX Compatible |
| **Checksum** | 0x6E19B |
| **Timestamp (PE)** | 2014-04-01 01:28:01 UTC |
| **Timestamp (Exports)** | 2014-04-01 01:15:32 UTC |
| **Build** | 16.4.3528.0331_ship.client.main.w5m4 |
| **PDB** | `WLMFDS.pdb` `{41188442-2579-4939-8E66-6697FE148922}` age 1 |

### Sections

| Section | VA | VirtSize | RawSize | Flags | Purpose |
|---------|------|----------|---------|-------|---------|
| `.text` | 0x1000 | 0x56573 | 0x56600 | Code, Execute Read | 345 KB — all executable code |
| `.data` | 0x58000 | 0x5F5C | 0x5800 | Init'd Data, Read Write | ~24 KB — globals, vtables, RTTI |
| `.rsrc` | 0x5E000 | 0x3F8 | 0x400 | Init'd Data, Read Only | ~1 KB — version info |
| `.reloc` | 0x5F000 | 0x7FC0 | 0x8000 | Init'd Data, Discardable, Read Only | ~32 KB — ASLR relocations |

### Directories

| Directory | RVA | Size |
|-----------|------|------|
| Export | 0x574D0 | 0xA3 |
| Import | 0x5619C | 0x12C |
| Resource | 0x5E000 | 0x3F8 |
| Certificates | 0x64600 | 0x3EC8 |
| Base Relocations | 0x5F000 | 0x5A50 |
| Debug | 0x1380 | 0x38 |
| Load Config | 0x9A20 | 0x40 |
| IAT | 0x1000 | 0x354 |

### Debug Info

- **PDB**: WLMFDS.pdb, GUID `{41188442-2579-4939-8E66-6697FE148922}`, age 1
- **Pre-VC++ 11.00 features**: 13
- **/GS buffer security**: 75 functions protected
- **/sdl**: 0 (not enabled)
- **guardN**: unreported

## Export Table (4 functions — Standard COM DLL pattern)

| Ordinal | Hint | RVA | Name |
|---------|------|------|------|
| 1 | 0 | 0x122BA | `DllCanUnloadNow` |
| 2 | 1 | 0x122D5 | `DllGetClassObject` |
| 3 | 2 | 0x1228A | `DllRegisterServer` |
| 4 | 3 | 0x122A2 | `DllUnregisterServer` |

Standard ATL COM DLL exports. COM objects are registered via ATL OBJECT_MAP; no named factory exports.

## Import Table (14 DLLs, ~185+ functions)

### MSVCR110.dll (42) — VC++ 2012 CRT
Memory: `malloc`, `free`, `memcpy`, `memmove`, `memset`, `memcmp`, `memcpy_s`, `_calloc_crt`, `_malloc_crt`, `_recalloc`
String: `_vsnwprintf`, `_wcsicmp`, `_wtoi`, `swprintf_s`, `wcscat_s`, `wcscpy_s`, `wcsncmp`, `wcsncpy_s`, `wcsrchr`, `wcsstr`, `iswalpha`, `iswdigit`, `towlower`, `towupper`
CRT init/exception: `_initterm`, `_initterm_e`, `__CppXcptFilter`, `_amsg_exit`, `__crtUnhandledException`, `__crtTerminateProcess`, `_crt_debugger_hook`, `_except_handler_common`, `__dllonexit`, `_onexit`, `_purecall`, `?terminate@@YAXXZ`, `??1type_info@@UAE@XZ`, `??2@YAPAXI@Z`, `??3@YAXPAX@Z`, `__clean_type_info_names_internal`
Sync: `_lock`, `_unlock`
Other: `qsort`

### KERNEL32.dll (53) — OS Core
Memory/heap: `HeapAlloc`, `HeapFree`, `HeapReAlloc`, `HeapSize`, `GetProcessHeap`
Sync: `InitializeCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `EnterCriticalSection`, `LeaveCriticalSection`, `DeleteCriticalSection`, `CreateEventW`, `CreateSemaphoreW`, `CreateIoCompletionPort`, `CreateThreadpoolWait`, `SetThreadpoolWait`, `CloseThreadpoolWait`, `WaitForThreadpoolWaitCallbacks`, `WaitForSingleObject`, `WaitForSingleObjectEx`, `WaitForMultipleObjects`, `SetEvent`, `ResetEvent`, `ReleaseSemaphore`
Thread: `CreateThread`, `GetCurrentThread`, `GetCurrentThreadId`, `SetThreadPriority`, `GetThreadPriority`
IP/ASLR: `DecodePointer`, `EncodePointer`
Module: `LoadLibraryExW`, `FreeLibrary`, `GetModuleHandleW`, `GetModuleHandleExW`, `GetModuleFileNameA`, `GetModuleFileNameW`, `GetProcAddress`
File: `CreateFileW`, `WriteFile`
Memory-mapped: `CreateFileMappingW`, `OpenFileMappingW`, `MapViewOfFile`, `UnmapViewOfFile`
Sync/interlocked: `InterlockedIncrement`, `InterlockedDecrement`, `InterlockedExchange`, `InterlockedCompareExchange`, `InterlockedCompareExchange64`
Time: `GetTickCount`, `GetTickCount64`, `QueryPerformanceCounter`, `QueryPerformanceFrequency`, `GetSystemTimeAsFileTime`
Process: `RaiseException`, `SetLastError`, `GetLastError`
OS info: `GetSystemInfo`, `IsDebuggerPresent`, `IsProcessorFeaturePresent`, `GetVersionExW`, `GetFullPathNameW`, `MulDiv`
Resources: `FindResourceW`, `LoadResource`, `SizeofResource`
Other: `lstrcmpiW`, `lstrcmpW`, `lstrlenA`, `lstrlenW`, `DisableThreadLibraryCalls`, `Sleep`

### ADVAPI32.dll (24) — Registry, Security, ETW Tracing
Registry: `RegCreateKeyW`, `RegCreateKeyExW`, `RegOpenKeyExW`, `RegCloseKey`, `RegDeleteKeyW`, `RegDeleteValueW`, `RegEnumKeyExW`, `RegEnumValueW`, `RegGetValueW`, `RegQueryValueExW`, `RegSetValueW`, `RegSetValueExW`, `RegQueryInfoKeyW`
Security descriptors: `AllocateAndInitializeSid`, `FreeSid`, `BuildTrusteeWithSidW`, `InitializeSecurityDescriptor`, `SetSecurityDescriptorDacl`, `SetEntriesInAclW`
ETW: `RegisterTraceGuidsW`, `UnregisterTraceGuids`, `GetTraceEnableFlags`, `GetTraceEnableLevel`, `GetTraceLoggerHandle`, `TraceMessage`

### ole32.dll (14) — COM Core
`CoInitialize`, `CoUninitialize`, `CoFreeUnusedLibraries`, `CoCreateInstance`, `CoCreateGuid`, `CoGetMalloc`, `CoTaskMemAlloc`, `CoTaskMemFree`, `CoTaskMemRealloc`, `CLSIDFromString`, `StringFromCLSID`, `StringFromGUID2`, `PropVariantClear`, `PropVariantCopy`

### OLEAUT32.dll (4 ordinals)
Ordinals 4, 6, 9, 277 — COM automation/VARIANT support.

### MFPlat.DLL (17) — Media Foundation Platform
`MFCreateSample`, `MFCreateStreamDescriptor`, `MFCreatePresentationDescriptor`, `MFCreateMediaTypeFromRepresentation`, `MFCreateMemoryBuffer`, `MFCreateAlignedMemoryBuffer`, `MFCreateEventQueue`, `MFCreateAsyncResult`, `MFTEnum`, `MFPutWorkItem`, `MFPutWorkItemEx`, `MFInvokeCallback`, `MFLockPlatform`, `MFUnlockPlatform`, `MFHeapAlloc`, `MFHeapFree`, `CreatePropertyStore`

### MF.dll (1)
`MFCreateSourceResolver`

### WINMM.dll (6) — Multimedia Timers
`timeBeginPeriod`, `timeEndPeriod`, `timeGetDevCaps`, `timeGetTime`, `timeKillEvent`, `timeSetEvent`

### AVRT.dll (2) — Multimedia Thread Scheduling
`AvSetMmThreadCharacteristicsW`, `AvRevertMmThreadCharacteristics`

### EVR.dll (1) — Enhanced Video Renderer
`MFCopyImage`

### SHELL32.dll (1)
`SHCreateItemFromParsingName`

### SHLWAPI.dll (1)
`PathIsRelativeW`

### VERSION.dll (3)
`GetFileVersionInfoSizeW`, `GetFileVersionInfoW`, `VerQueryValueW`

### USER32.dll (1)
`CharNextW`

## ATL Framework (from mangled symbols)

The DLL uses ATL 7.0 (Visual Studio 2003-era ATL, carried forward):
- `_MODULE70@ATL@@` — ATL 7.0 module class
- `Module@ATL@@` — CAtlModule / CComModule base

This is the standard ATL COM DLL pattern: `CAtlDllModuleT<T>` with `DllMain`, object map, class factories.

## COM Interfaces Implemented (from RTTI/demangled symbols)

### Media Foundation Interfaces (IMF*)
- **`IMFMediaSource`** — primary MF source object (wraps a DirectShow filter graph as an MF source)
- **`IMFSource`** — IAMFilterGraph semantics in MF
- **`IMFMediaStream`** — individual stream from the wrapped DirectShow filter
- **`IMFAsyncCallback`** — async operation completion
- **`IMFAsyncResult`** — async result handling
- **`IMFRateControl`** — rate/playback speed control
- **`IMFRateSupport`** — rate capability query
- **`IMFGetService`** — service querying

### DirectShow Interfaces (from symbols)
- **`IMemInputPin`** — memory input pin (DirectShow data flow)
- **`IMemAllocator`** — memory allocator
- **`IMediaFilter`** — filter state (Stop/Pause/Run)
- **`IMediaSample`** — media sample
- **`IMediaSeeking`** — seeking/positioning
- **`IMediaPosition`** — position control
- **`IAMLatency`** — latency reporting

### MPEG-2 TS
- **`IMpeg2Demultiplexer`** — MPEG-2 demux interface
- **`IMPEG2Stream`** — MPEG-2 stream interface
- **`IMpeg2TS`** — MPEG-2 transport stream

### DirectShow Filter Graph
- **`IFilterGraph`** / **`IGraphBuilder`** — built internally to host DirectShow filters
- **`IReferenceClock`** — clock reference for sync
- **`ICodecAPI`** — codec configuration

### AVC Internal Classes
- **`AVCBaseFilter`** — DirectShow base filter class
- **`AVCBasePin`** — DirectShow base pin class
- **`CEnumPins`** — pin enumerator
- **`CKey`** — COM identity key
- **`CPCRValue`** — PCR clock reference value

## COM CLSID String References (DirectShow filter bridge targets)

The DLL references these DirectShow CLSIDs by **string name** (not by GUID), used for filter graph construction and device enumeration:

**Source/Reader filters:**
`CLSID_AsyncReader`, `CLSID_URLReader`, `CLSID_FileSource`, `CLSID_WMAsfReader`, `CLSID_AviReader`, `CLSID_AVICo`, `CLSID_AVIDoc`, `CLSID_AviSplitter`, `CLSID_MOVReader`, `CLSID_QTDec`, `CLSID_QuickTimeParser`, `CLSID_MPEG1Doc`, `CLSID_MPEG1Splitter`, `CLSID_MPEG2Demultiplexer`, `CLSID_MMSPLITTER`, `CLSID_DVSplitter`, `CLSID_DVMux`, `CLSID_MjpegDec`, `CLSID_MJPGEnc`

**Encoder/Codec filters:**
`CLSID_CMpegAudioCodec`, `CLSID_CMpegVideoCodec`, `CLSID_DVVideoCodec`, `CLSID_DVVideoEnc`, `CLSID_IVideoEncoderProxy`, `CLSID_IVideoEncoderCodecAPIProxy`, `CLSID_ICodecAPIProxy`

**Renderer/Display:**
`CLSID_VideoRenderer`, `CLSID_VideoRendererDefault`, `CLSID_VideoMixingRenderer`, `CLSID_VideoMixingRenderer9`, `CLSID_EnhancedVideoRenderer`, `CLSID_AllocPresenter`, `CLSID_AllocPresenterDDXclMode`, `CLSID_MFVideoMixer9`, `CLSID_MFVideoPresenter9`, `CLSID_EVRTearlessWindowPresenter9`, `CLSID_EVRPlaybackPipelineOptimizer`, `CLSID_AudioRender`, `CLSID_DSoundRender`, `CLSID_OverlayMixer`, `CLSID_ModexRenderer`, `CLSID_TextRender`, `CLSID_AudioRendererCategory`

**TV/Encoder:**
`CLSID_DShowTVEFilter`, `CLSID_ATSCNetworkProvider`, `CLSID_DVBCNetworkProvider`, `CLSID_DVBSNetworkProvider`, `CLSID_DVBTNetworkProvider`, `CLSID_NetworkProvider`, `CLSID_TVEFilterTuneProperties`, `CLSID_TVEFilterCCProperties`, `CLSID_TVEFilterStatsProperties`

**Capture:**
`CLSID_VfwCapture`, `CLSID_CaptureGraphBuilder`, `CLSID_CaptureGraphBuilder2`, `CLSID_AudioInputMixerProperties`, `CLSID_AudioInputDeviceCategory`, `CLSID_VideoInputDeviceCategory`, `CLSID_CameraControlPropertyPage`, `CLSID_VideoProcAmpPropertyPage`, `CLSID_CrossbarFilterPropertyPage`, `CLSID_TVTunerFilterPropertyPage`, `CLSID_AnalogVideoDecoderPropertyPage`

**Audio/Video decoding:**
`CLSID_ACMWrapper`, `CLSID_Line21Decoder`, `CLSID_Line21Decoder2`, `CLSID_CaptionsFilter`, `CLSID_CCAFilter`, `CLSID_DtvCcFilter`, `CLSID_SubtitlesFilter`, `CLSID_WSTDecoder`, `CLSID_VBISurfaces`, `CLSID_VPVBIObject`, `CLSID_VPObject`

**Stream Buffer / SBE:**
`CLSID_SBE2Sink`, `CLSID_SBE2File`, `CLSID_StreamBufferSink`, `CLSID_StreamBufferSource`, `CLSID_StreamBufferComposeRecording`, `CLSID_StreamBufferConfig`, `CLSID_StreamBufferPropertyHandler`, `CLSID_StreamBufferRecordingAttributes`, `CLSID_StreamBufferThumbnailHandler`

**Filter Graph infrastructure:**
`CLSID_FilterGraph`, `CLSID_FilterGraphNoThread`, `CLSID_FilterGraphPrivateThread`, `CLSID_ProtoFilterGraph`, `CLSID_FGControl`, `CLSID_FilterMapper`, `CLSID_FilterMapper2`, `CLSID_SystemClock`, `CLSID_MemoryAllocator`, `CLSID_MediaPropertyBag`, `CLSID_SeekingPassThru`, `CLSID_SmartTee`, `CLSID_InfTee`, `CLSID_PersistMonikerPID`, `CLSID_CaptureProperties`, `CLSID_QualityProperties`, `CLSID_PerformanceProperties`, `CLSID_DirectDrawProperties`

**Categories for enumeration:**
`CLSID_LegacyAmFilterCategory`, `CLSID_DeviceControlCategory`, `CLSID_ActiveMovieCategories`, `CLSID_SystemDeviceEnum`, `CLSID_CDeviceMoniker`, `CLSID_DVDHWDecodersCategory`, `CLSID_AudioCompressorCategory`, `CLSID_VideoCompressorCategory`, `CLSID_MediaEncoderCategory`, `CLSID_MediaMultiplexerCategory`, `CLSID_MidiRendererCategory`, `CLSID_TransmitCategory`, `CLSID_VideoPortManager`

**DVD Navigation:**
`CLSID_DVDNavigator`, `CLSID_DVDState`, `CLSID_DvdGraphBuilder`, `CLSID_DVDecPropertiesPage`, `CLSID_DVEncPropertiesPage`, `CLSID_DVMuxPropertyPage`

**DirectShow class managers:**
`CLSID_CAcmCoClassManager`, `CLSID_CIcmCoClassManager`, `CLSID_CQzFilterClassManager`, `CLSID_CVidCapClassManager`, `CLSID_CWaveinClassManager`, `CLSID_CWaveOutClassManager`, `CLSID_CMidiOutClassManager`

**Writer/Output:**
`CLSID_AviDest`, `CLSID_AviMuxProptyPage`, `CLSID_AviMuxProptyPage1`, `CLSID_WMAsfWriter`, `CLSID_FileWriter`, `CLSID_AVIDraw`, `CLSID_AVIMIDIRender`

**Other:**
`CLSID_Colour`, `CLSID_Dither`, `CLSID_OverlayMixer`, `CLSID_Mpeg2VideoStreamAnalyzer`

## DirectShow Media Types Referenced (as strings)

### MEDIATYPE_*
`MEDIATYPE_Video`, `MEDIATYPE_Audio`, `MEDIATYPE_Stream`, `MEDIATYPE_Interleaved`, `MEDIATYPE_Text`, `MEDIATYPE_ScriptCommand`, `MEDIATYPE_File`, `MEDIATYPE_AUXLine21Data`, `MEDIATYPE_AUXTeletextPage`, `MEDIATYPE_Timecode`, `MEDIATYPE_Midi`, `MEDIATYPE_AnalogVideo`, `MEDIATYPE_AnalogAudio`, `MEDIATYPE_VBI`, `MEDIATYPE_DTVCCData`, `MEDIATYPE_MSTVCaption`, `MEDIATYPE_CC_CONTAINER`, `MEDIATYPE_MPEG1SystemStream`, `MEDIATYPE_MPEG2_PACK`, `MEDIATYPE_MPEG2_PES`, `MEDIATYPE_MPEG2_SECTIONS`, `MEDIATYPE_DVD_NAVIGATION`, `MEDIATYPE_DVD_ENCRYPTED_PACK`, `MEDIATYPE_LMRT`, `MEDIATYPE_URL_STREAM`

### MEDIASUBTYPE_*
**Video formats:** `MEDIASUBTYPE_RGB1/4/8/24/32/555/565`, `MEDIASUBTYPE_ARGB32/1555/4444`, `MEDIASUBTYPE_AYUV`, `MEDIASUBTYPE_YUY2`, `MEDIASUBTYPE_YUYV`, `MEDIASUBTYPE_YVYU`, `MEDIASUBTYPE_YV12`, `MEDIASUBTYPE_YVU9`, `MEDIASUBTYPE_UYVY`, `MEDIASUBTYPE_IYUV`, `MEDIASUBTYPE_IF09`, `MEDIASUBTYPE_IJPG`, `MEDIASUBTYPE_IMC1-4`, `MEDIASUBTYPE_IA44`, `MEDIASUBTYPE_AI44`, `MEDIASUBTYPE_CLJR`, `MEDIASUBTYPE_CLPL`, `MEDIASUBTYPE_CPLA`, `MEDIASUBTYPE_CFCC`, `MEDIASUBTYPE_MDVF`, `MEDIASUBTYPE_NV12`, `MEDIASUBTYPE_NV24`, `MEDIASUBTYPE_P010/016/208/210/216/408`, `MEDIASUBTYPE_Y210/211/216`, `MEDIASUBTYPE_Y411`, `MEDIASUBTYPE_Y41P`, `MEDIASUBTYPE_S340/342`, `MEDIASUBTYPE_Plum`, `MEDIASUBTYPE_WAKE`, `MEDIASUBTYPE_H264`, `MEDIASUBTYPE_dv25/50/100`, `MEDIASUBTYPE_dvh1`, `MEDIASUBTYPE_dvhd`, `MEDIASUBTYPE_dvsl`, `MEDIASUBTYPE_DVCS`, `MEDIASUBTYPE_MJPG`, `MEDIASUBTYPE_TVMJ`, `MEDIASUBTYPE_VPVideo`, `MEDIASUBTYPE_VPVBI`, `MEDIASUBTYPE_Overlay`, `MEDIASUBTYPE_QTJpeg`, `MEDIASUBTYPE_QTMovie`, `MEDIASUBTYPE_QTRle`, `MEDIASUBTYPE_QTRpza`, `MEDIASUBTYPE_QTSmc`, `MEDIASUBTYPE_RAW_SPORT`

**Audio:** `MEDIASUBTYPE_PCM`, `MEDIASUBTYPE_PCMAudio_Obsolete`, `MEDIASUBTYPE_IEEE_FLOAT`, `MEDIASUBTYPE_DOLBY_AC3`, `MEDIASUBTYPE_DOLBY_AC3_SPDIF`, `MEDIASUBTYPE_DTS`, `MEDIASUBTYPE_SDDS`, `MEDIASUBTYPE_MPEG1Audio`, `MEDIASUBTYPE_MPEG1AudioPayload`, `MEDIASUBTYPE_MPEG2_AUDIO`, `MEDIASUBTYPE_DRM_Audio`, `MEDIASUBTYPE_SPDIF_TAG_241h`

**Analog/captioning:** `MEDIASUBTYPE_AnalogVideo_NTSC_M/PAL_B/D/G/H/I/M/N/N_COMBO/SECAM_B/D/G/H/K/K1/L`, `MEDIASUBTYPE_Line21_BytePair/GOPPacket/VBIRawData`, `MEDIASUBTYPE_DtvCcData`, `MEDIASUBTYPE_DVB_SUBTITLES`, `MEDIASUBTYPE_ISDB_CAPTIONS/SUPERIMPOSE`, `MEDIASUBTYPE_708_608Data`, `MEDIASUBTYPE_TELETEXT`, `MEDIASUBTYPE_VPS`, `MEDIASUBTYPE_WSS`, `MEDIASUBTYPE_XDS`, `MEDIASUBTYPE_CC_CONTAINER`, `MEDIASUBTYPE_VBI`

**MPEG:** `MEDIASUBTYPE_MPEG1Packet/Payload/System/Video/VideoCD`, `MEDIASUBTYPE_MPEG2_VIDEO/VIDEO`, `MEDIASUBTYPE_MPEG2_PROGRAM/TRANSPORT/TRANSPORT_STRIDE/UDCR_TRANSPORT/WMDRM_TRANSPORT/PBDA_TRANSPORT_RAW/PBDA_TRANSPORT_PROCESSED/VERSIONED_TABLES`, `MEDIASUBTYPE_MPEG2DATA`, `MEDIASUBTYPE_Asf`, `MEDIASUBTYPE_Avi`

**DVD:** `MEDIASUBTYPE_DVD_SUBPICTURE`, `MEDIASUBTYPE_DVD_LPCM_AUDIO`, `MEDIASUBTYPE_DVD_NAVIGATION_PCI/DSI/PROVIDER`, `MEDIASUBTYPE_ATSC_SI`, `MEDIASUBTYPE_DVB_SI`, `MEDIASUBTYPE_ISDB_SI`, `MEDIASUBTYPE_TIF_SI`, `MEDIASUBTYPE_DSS_Audio`, `MEDIASUBTYPE_DSS_Video`

**File/container:** `MEDIASUBTYPE_AIFF`, `MEDIASUBTYPE_AU`, `MEDIASUBTYPE_WAVE`, `MEDIASUBTYPE_None`

### FORMAT_*
`FORMAT_None`, `FORMAT_VideoInfo`, `FORMAT_VIDEOINFO2`, `FORMAT_WaveFormatEx`, `FORMAT_MPEGVideo`, `FORMAT_MPEGStreams`, `FORMAT_MPEG2Video`, `FORMAT_MPEG2Audio`, `FORMAT_DolbyAC3`, `FORMAT_DVD_LPCMAudio`, `FORMAT_DvInfo`, `FORMAT_AnalogVideo`, `FORMAT_Image`, `FORMAT_JPEGImage`, `FORMAT_CC_CONTAINER`, `FORMAT_CAPTIONED_MPEG2VIDEO`, `FORMAT_CAPTIONED_H264VIDEO`, `FORMAT_UVCH264Video`, `FORMAT_525WSS`

### AM_KSCATEGORY_*
`AM_KSCATEGORY_AUDIO`, `AM_KSCATEGORY_CAPTURE`, `AM_KSCATEGORY_CROSSBAR`, `AM_KSCATEGORY_DATACOMPRESSOR`, `AM_KSCATEGORY_RENDER`, `AM_KSCATEGORY_SPLITTER`, `AM_KSCATEGORY_TVAUDIO`, `AM_KSCATEGORY_TVTUNER`, `AM_KSCATEGORY_VBICODEC`, `AM_KSCATEGORY_VBICODEC_MI`, `AM_KSCATEGORY_VIDEO`

### AM_KSPROPSET_*
`AM_KSPROPSETID_AC3`, `AM_KSPROPSETID_CopyProt`, `AM_KSPROPSETID_DVD_RateChange`, `AM_KSPROPSETID_DvdKaraoke`, `AM_KSPROPSETID_DvdSubPic`, `AM_KSPROPSETID_FrameStep`, `AM_KSPROPSETID_MPEG4_MediaType_Attributes`, `AM_KSPROPSETID_TSRateChange`, `AMPROPSETID_Pin`

### PIN_CATEGORY_*
`PIN_CATEGORY_CAPTURE`, `PIN_CATEGORY_PREVIEW`, `PIN_CATEGORY_VIDEOPORT`, `PIN_CATEGORY_VIDEOPORT_VBI`, `PIN_CATEGORY_VBI`, `PIN_CATEGORY_CC`, `PIN_CATEGORY_NABTS`, `PIN_CATEGORY_EDS`, `PIN_CATEGORY_TELETEXT`, `PIN_CATEGORY_TIMECODE`, `PIN_CATEGORY_ANALOGVIDEOIN`, `PIN_CATEGORY_STILL`

### Other DirectShow constants
`AM_INTERFACESETID_Standard`, `TIME_FORMAT_NONE/BYTE/SAMPLE/FIELD/FRAME/MEDIA_TIME`, `LOOK_UPSTREAM_ONLY`, `LOOK_DOWNSTREAM_ONLY`, `EOS_SENT`, `EOS_PENDING`, `EOS_NO`, `SETRATE`, `PAUSE`, `PAUSED`, `START`, `STARTED`, `STOP`, `STOPPED`

**DSATTRIB_** (DirectShow sample attributes):
`DSATTRIB_CAPTURE_STREAMTIME`, `DSATTRIB_CC_CONTAINER_INFO`, `DSATTRIB_DSHOW_STREAM_DESC`, `DSATTRIB_OptionalVideoAttributes`, `DSATTRIB_PBDATAG_ATTRIBUTE`, `DSATTRIB_PicSampleSeq`, `DSATTRIB_SAMPLE_LIVE_STREAM_TIME`, `DSATTRIB_TRANSPORT_PROPERTIES`, `DSATTRIB_UDCRTag`

**EVR configuration:**
`EVRConfig_AllowBatching/ForceBatching`, `EVRConfig_AllowDropToBob/ForceBob`, `EVRConfig_AllowDropToHalfInterlace/ForceHalfInterlace`, `EVRConfig_AllowDropToThrottle/ForceThrottle`, `EVRConfig_AllowScaling/ForceScaling`

**CODECAPI_**: `CODECAPI_AVDecMmcssClass`

**UUID_** (DRM/table identifiers): `UUID_UdriTagTables`, `UUID_WMDRMTagTables`

**IID_** (DirectShow interfaces referenced):
`IID_IKsControl`, `IID_IKsDataTypeHandler`, `IID_IKsInterfaceHandler`, `IID_IKsPin`, `IID_IKsPinFactory`, `IID_IAMDirectSound`, `IID_IAMLine21Decoder`, `IID_IAMWstDecoder`, `IID_IBaseVideoMixer`, `IID_IDDVideoPortContainer`, `IID_IDirectDrawKernel/SurfaceKernel/Video`, `IID_IFullScreenVideo/Ex`, `IID_IMixerPinConfig/2`, `IID_IMpegAudioDecoder`, `IID_IQualProp`, `IID_IVPConfig/Control/Notify/Notify2/Object`, `IID_IVPVBIConfig/Notify/Object`

## Architecture: DirectShow → MF Bridge

WLMFDS.dll implements a **Media Foundation source** that wraps an entire DirectShow filter graph. The architecture is:

1. **DirectShow Filter Graph** is built internally using `IFilterGraph`/`IGraphBuilder` from quartz.dll (via `CoCreateInstance` with `CLSID_FilterGraph` or similar)
2. **Filter enumeration** uses `IFilterMapper2` / `ICreateDevEnum` to locate DirectShow filters by category
3. **Pin management** via `IEnumPins`/`IPin` interfaces on each filter
4. **Media type conversion** between `AM_MEDIA_TYPE` (DirectShow) and `IMFMediaType` (Media Foundation) using `MFCreateMediaTypeFromRepresentation`
5. **Stream presentation**: Each DirectShow output pin becomes an `IMFMediaStream`; the filter graph becomes an `IMFMediaSource`
6. **Rate control**: `IMFRateControl`/`IMFRateSupport` wrap DirectShow's `IMediaSeeking`/`IVideoFrameStep` for playback speed changes
7. **Seeking** is bridged from MF seeking calls to DShow `IMediaSeeking::SetPositions`
8. **Async model**: MF async callbacks (`IMFAsyncCallback`) wrap DShow's `IPin::Receive()` push model — samples from DShow are delivered via the memcpy path and then signaled as MF async results
9. **Multimedia timers** (WINMM `timeSetEvent` etc.) provide reference clock and scheduling
10. **AVRT.dll** (`AvSetMmThreadCharacteristicsW`) sets MMCSS (Multimedia Class Scheduler Service) for low-latency audio/video threads
11. **ETW Tracing** (ADVAPI32) provides instrumentation across the bridge
12. **Registry** is used for filter registration, codec enumeration, and plug-in discovery (DLL registration via `DllRegisterServer` for custom DShow filters)
13. **Stream Buffer Engine (SBE)** integration for timeshifting / DVR functionality

### Why this pattern?
Windows Live Essentials targeted Windows 7/8 which had Media Foundation as the preferred multimedia API. However, Movie Maker's pipeline was built on DirectShow (WMMR's VideoTrim, codecs, source readers). This bridge DLL allows MF-based components (like WLMFReadWrite.dll) to reuse existing DirectShow filters without porting them to the MF pipeline.

## CERTIFICATE GUIDs (non-COM, present in all WLMM binaries)
- `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` — Microsoft Code Signing PCA
- `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` — Microsoft Root Certificate Authority

## Key Strings of Interest
- `WLMFDS.pdb` — debug symbols
- `WLMFDS.DLL` — module name in import hints
- `DllCanUnloadNow` / `DllGetClassObject` / `DllRegisterServer` / `DllUnregisterServer` — export names
- `DirectShowPluginControl` — DirectShow plugin management
- `no operation set` — error/compatibility state string
- `NOTVALID` — validation state
- `UNKNOWN` — unknown state

## Summary

| Attribute | Value |
|-----------|-------|
| **Role** | DirectShow → Media Foundation bridge |
| **Size** | ~417 KB (345 KB code) |
| **Pattern** | ATL 7.0 COM DLL, 4 standard exports |
| **Framework** | AVC (Audio Video Components) class hierarchy |
| **CRT** | MSVCR110 (VS 2012) |
| **Build** | 16.4.3528.0331 (ship, w5m4 branch) |
| **Date** | 2014-04-01 |
| **Target OS** | Windows 8+ (6.02 subsystem) |
| **Key APIs** | MFPlat (17), ADVAPI32 (24), KERNEL32 (53), WINMM (6), AVRT (2) |
| **Bridge targets** | All major DirectShow filters (source, transform, render, capture, TV, DVD) |
