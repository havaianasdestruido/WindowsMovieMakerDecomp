# WLMFDS.dll Dynamic Analysis

## Overview

This document covers dynamic analysis findings from probing the original
`WLMFDS.dll` binary (417KB, x86, MSVC 11.0) from Windows Live Movie Maker
2012. Analysis combines static binary inspection with runtime COM probing
via the test harness.

## Exported Functions

| Export | Address (RVA) | Runtime Status |
|--------|---------------|----------------|
| `DllCanUnloadNow` | 0x122BA | Returns `S_OK` (no live objects) |
| `DllGetClassObject` | 0x122D5 | Returns `CLASS_E_CLASSNOTAVAILABLE` for CLSID_NULL and unregistered CLSIDs |
| `DllRegisterServer` | 0x1228A | Skipped (requires admin) |
| `DllUnregisterServer` | 0x122A2 | Skipped (requires admin) |

All 4 exports resolve successfully from `undecomp\Shared\WLMFDS.dll` via
`LoadLibrary`/`GetProcAddress`.

## CLSID_DShowSourceResolver Probe

The `CLSID_DShowSourceResolver` GUID is embedded as raw bytes in the `.data`
section within the ATL `CComCoClass` template -- not as a printable string.
Since the DLL is not registered on the analysis system, `DllGetClassObject`
returns `0x80040111` (`CLASS_E_CLASSNOTAVAILABLE`).

The CLSID is declared in RTTI as:
```
.?AV?$CComCoClass@VCDShowSourceResolver@@$1?CLSID_DShowSourceResolver@@3U_GUID@@B@ATL@@
```
This confirms `CLSID_DShowSourceResolver` is a `const GUID __declspec(selectany)`
global exported via the ATL object map. Registration via `DllRegisterServer`
would install it under `HKCR\CLSID\{...}`.

## MF Source Resolver Probe

`MFCreateSourceResolver` (imported from `MF.dll`) returns a valid
`IMFSourceResolver*` pointer -- confirming the system's Media Foundation
stack is operational. This is the function WLMFDS.dll itself imports to
bridge DShow filter graphs into the MF pipeline.

## RTTI Class Hierarchy (160+ classes)

### Core AVC Framework (Audio Video Components)

The DLL implements a layered architecture built on the **AVC** (Audio Video
Component) class hierarchy:

#### Base Classes
```
CUnknown (IUnknown base)
  +-- CBaseObject (ref counting, debug trace)
      +-- CBaseFilter (IMediaFilter -> IBaseFilter)
      |   +-- CBaseInputPin
      |   +-- CBaseOutputPin
      |   |   +-- CBasePin (IPin, IQualityControl)
      |   +-- CMFSourceFilter (CMFSourceMPEG2TSPin)
      |   +-- CMPEG2Demultiplexer (CBDAMPEG2Demux)
      |   +-- CMPEG2ProgramController
      |   +-- CMPEG2TransportController
      |   +-- CMPEG2PushProgramController
      |   +-- CMPEG2PushClock
      |   +-- CMPEG2PullTransportController
      |   +-- CDSSampleSinkFilter
      +-- CMediaSample (IMediaSample, IMediaSample2)
```

#### DShow Source -> MF Bridge Layer
```
CDShowSource (IMFMediaSource + IAMFilterMiscFlags)
  +-- CDShowStream (IMFMediaStream)
  |   +-- CDSSampleSink
  +-- CDShowSourceResolver (IMFSourceResolver + IClassFactory)
  |   +-- CResolverResult
  +-- CDShowSourceOpQueue
  |   +-- CDShowSourceOp
  +-- CDShowEOSManager
  +-- CDShowESEvent / CDShowESEventEx
  +-- CBufferSourceManager
      +-- CBufferSource
```

#### MF Source Filter Layer
```
CMFSourceFilter (IBaseFilter -> IMFPsiFilterConfig)
  +-- CMFSourceOutputPin
  |   +-- CReadRequest
  |   +-- Async callbacks:
  |       +-- OnAsyncReadAsyncCallback
  |       +-- OnSyncReadAsyncCallback
  |       +-- OnEventAsyncCallback
  +-- CMFSourceMPEG2TSPin
      +-- Async callbacks:
      |   +-- OnBufferedReadAsyncCallback
      |   +-- OnSyncReadAsyncCallback
      |   +-- OnEventAsyncCallback
      +-- CMFSourceBufferingSupport
```

#### MPEG-2 Demultiplexer Engine (largest component, ~60 classes)
```
CMPEG2Demultiplexer : IBaseFilter
  +-- CMPEG2DemuxInputPin : IMemInputPin
  +-- CMPEG2DemuxOutputPin : IEnumPIDMap, IEnumStreamIdMap
  +-- CMpeg2DemuxMediaSeekingCOM : IMediaSeeking
  |
  +-- Stream Analysis:
  |   +-- CMpeg2ProgramStreamAnalyzer
  |   +-- CMpeg2ProgramStreamSniffer
  |   +-- CMpeg2StreamAnalyzer
  |   +-- CMpeg2TransportStreamSniffer
  |   +-- CMpeg2PullTransportStreamAnalyzer
  |   +-- CMpeg2GenericTSPayload
  |
  +-- PSI/PAT/PMT Parsing:
  |   +-- CMpeg2PSIParse / CMpeg2VersionFilteredPSIParse
  |   +-- CMpeg2NonVersionedPSIParse
  |   +-- CMpeg2PATSectionParser
  |   +-- CMpeg2PMTSectionParser
  |   +-- CMpeg2PBDAParser
  |   +-- CMpeg2PBDAPSISectionParser
  |   +-- CMpeg2PBDAPESStreamParser
  |   +-- CMPEG2PSISectionBufferSource
  |   +-- CVersionMap
  |
  +-- PES Stream Parsing:
  |   +-- CMpeg2PESStreamParser
  |   +-- CMpeg2PEStreamTime
  |   +-- CMpeg2SCRParser
  |   +-- CMpeg2PCRParser
  |   +-- CMpeg2TSPassThroughParser
  |
  +-- Program/Stream Mapping:
  |   +-- CProgramStreamMapper / CStreamMapper
  |   +-- CTransportStreamMapper
  |   +-- CProgramStreamMapContext / CStreamMapContext
  |   +-- CTransportStreamMapContext
  |   +-- CEnumStreamMapBase / CDBDADemuxEnumPIDMap / CDBDADemuxEnumStreamIdMap
  |
  +-- Content Management:
  |   +-- CTSContentManager / CPSContentManager
  |   +-- CTSProgram
  |   +-- CMpeg2StreamContentManager
  |   +-- CStreamParser
  |
  +-- Clock/PCR:
  |   +-- CMPEG2PushClock
  |   +-- CMPEG2Controller
  |   +-- CMpeg2Time / CMpeg2PCRParser
  |   +-- CPCRValue / CPCRRecordBufferSource
  |   +-- CIPTSConvert / CMpeg2PEStreamTime
  |
  +-- Media Samples:
      +-- CMediaSample / CMediaSampleCopyBuffer
      +-- CMediaSampleWrapper / CMediaSampleWrapperPool
      +-- CScratchMediaSample
      +-- CDemuxIMediaSample / CDemuxIMediaSamplePool
      +-- CAttributeList
```

#### Pin/Memory Infrastructure
```
CMemAllocator : CBaseAllocator : IMemAllocator
CMp2PullPin (async read for pull-mode sources)
CAsyncIO@BufferWriter
CRenderedInputPin
```

#### Sample Sink (DShow -> MF sample delivery)
```
CDSSampleSinkFilter : IBaseFilter
  +-- CDSSampleSinkInputPin : IMemInputPin
      +-- CDSSampleSink (sample queuing)
```

#### MacroVision / Copy Protection
```
CMacroVision
CMpeg2MacroVision@CMPEG2ProgramController
CMpeg2MacroVisionParser
```

#### ETW Tracing Infrastructure (Mpeg2DemuxTrace namespace)
```
CeHomeETWDemultiplexer, CeHomeETWProvider
CErrorEvent / CErrorEvents / CFatalErrorEvent / CCriticalErrorEvent
CFilterStateEvent / CInbandSpanningEvent
CSampleInEvent / CSampleOutEvent / CStreamMapEvent / CTransPropsEvent
CPBDATagEvent / CUDCRTagEvent
CMpeg2ErrorEvent / CMpeg2ExpiredProgramEvent / CMpeg2ExpiredStreamEvent
CMpeg2NewPATEvent / CMpeg2NewPMTEvent / CMpeg2NewProgramEvent / CMpeg2NewStreamEvent
CMpeg2PCREvent / CMpeg2PTSEvent
```

#### Async Callback Classes (for MF async bridge)
```
OnDShowWrapperStartAsyncCallback@CDShowSource
OnDShowWrapperPauseAsyncCallback@CDShowSource
OnDShowWrapperStopAsyncCallback@CDShowSource
OnFireEOPAsyncCallback@CDShowSource
OnObjectCreatedAsyncCallback@CMFSourceFilter
OnByteStreamCreatedAsyncCallback@CDShowSourceResolver
OnCreateObjectFromByteStreamAsyncCallback@CDShowSourceResolver
OnAsyncReadAsyncCallback@CMFSourceOutputPin
OnSyncReadAsyncCallback@CMFSourceOutputPin
OnBufferedReadAsyncCallback@CMFSourceMPEG2TSPin
OnSyncReadAsyncCallback@CMFSourceMPEG2TSPin
OnEventAsyncCallback@CMFSourceMPEG2TSPin
OnEventAsyncCallback@CMFSourceOutputPin
ProcessMarshalledOperationsAsyncCallback@COpQueue
```

#### ATL/COM Framework
```
CComModule@ATL / CAtlModule@ATL / CComClassFactory@ATL / CComObjectRootBase@ATL
CRegObject@ATL / CClassFactory : IClassFactory
```

#### Utility/Helper
```
CCritSec / CEnumPins / CEnumMediaTypes / CEnumKey / CEnumValue
CMFObjectPerfTrace / CAMThread / COpQueue / CAsyncResult
CFilterPolice / CPersistStream / CTAGPacket
CPsiParserFilter / CPsiParserInputPin
```

## COM Interfaces Implemented (from RTTI)

### Media Foundation Interfaces
| Interface | Implementing Class |
|-----------|-------------------|
| `IMFMediaSource` | `CDShowSource` |
| `IMFMediaStream` | `CDShowStream` |
| `IMFMediaEventGenerator` | `CDShowSource`, `CDShowStream` |
| `IMFSource` | `CDShowSource` |
| `IMFSourceResolver` | `CDShowSourceResolver` |
| `IMFRateControl` | `CDShowSource` |
| `IMFRateSupport` | `CDShowSource` |
| `IMFGetService` | `CDShowSource` |
| `IMFAsyncCallback` | Multiple async callback classes |
| `IMFAsyncResult` | `CAsyncResult` |
| `IMFPsiFilterConfig` | `CMFSourceFilter` |
| `IMFSourceFilterConfig` | (undocumented) |

### DirectShow Interfaces
| Interface | Implementing Class |
|-----------|-------------------|
| `IBaseFilter` | `CBaseFilter`, `CMPEG2Demultiplexer`, `CMFSourceFilter`, `CDSSampleSinkFilter` |
| `IMediaFilter` | `CBaseFilter` |
| `IMemInputPin` | `CMPEG2DemuxInputPin`, `CDSSampleSinkInputPin` |
| `IMemAllocator` / `IMemAllocatorCallbackTemp` | `CMemAllocator` |
| `IMediaSample` / `IMediaSample2` | `CMediaSample` |
| `IMediaSeeking` | `CMpeg2DemuxMediaSeekingCOM` |
| `IReferenceClock` | `CMPEG2PushClock` |
| `IPin` / `IQualityControl` | `CBasePin` |
| `IAMFilterMiscFlags` | `CDShowSource` |
| `IAMPushSource` | (interface vtable present) |
| `IAMGraphBuilderCallback` | (interface vtable present) |
| `IAMLatency` | (interface vtable present) |
| `IAsyncReader` | (interface vtable present) |
| `IFileSourceFilter` | (interface vtable present) |

### MPEG-2 Demultiplexer Interfaces
| Interface | Implementing Class |
|-----------|-------------------|
| `IMpeg2Demultiplexer` | `CMPEG2Demultiplexer` |
| `IMpeg2DemultiplexerModeConfig` | `CMPEG2Demultiplexer` |
| `IMpeg2DemultiplexerTesting` | `CMPEG2Demultiplexer` |
| `IMPEG2PIDMap` | `CMPEG2DemuxOutputPin` |
| `IMPEG2StreamIdMap` | `CMPEG2DemuxOutputPin` |
| `IMpeg2TSPsiParser` | `CPsiParserFilter` |
| `IDemuxRender` | `CDemuxRender` |

### BDA (Broadcast Driver Architecture) Interfaces
| Interface | Notes |
|-----------|-------|
| `IBDA_DeviceControl` | BDA device control |
| `IBDA_SignalProperties` | BDA signal tuning |
| `IBDA_Topology` | BDA topology |

### Other COM Interfaces
| Interface | Notes |
|-----------|-------|
| `ICodecAPI` | Codec parameter control |
| `IAttributeGet` / `IAttributeSet` | Attribute bag |
| `IInbandEvent` | Inband signaling events |
| `ISectionEventCallback` | PSI section callbacks |
| `IProgramInfo` | Program info |
| `ICAVStreamNotify` | Stream notification |
| `CIInputStreamEvent` | Input stream events |
| `ICPCRValueNotify` | PCR value notification |
| `IReaderSetPosition` | Reader position control |
| `IPersist` / `IPersistStream` | Persistent state |
| `ISpecifyPropertyPages` | Property page UI |
| `IRegistrarBase` | ATL registrar |

## DShow CLSID References (150 unique)

### Source/Reader Filters (18)
| CLSID | Filter | Purpose |
|-------|--------|---------|
| `CLSID_AsyncReader` | Async File Reader | Async byte-stream reading |
| `CLSID_URLReader` | URL Reader | HTTP/URL source |
| `CLSID_FileSource` | File Source (Async) | Local file source |
| `CLSID_WMAsfReader` | WM ASF Reader | ASF/WMV/WMA reading |
| `CLSID_AviReader` | AVI Reader | AVI file reading |
| `CLSID_AviSplitter` | AVI Splitter | AVI demux |
| `CLSID_AVICo` | AVI Compressor | AVI codec |
| `CLSID_AVIDoc` | AVI Document | AVI document |
| `CLSID_MOVReader` | QuickTime Reader | MOV/MP4 reading |
| `CLSID_QTDec` | QuickTime Decoder | QuickTime decoding |
| `CLSID_QuickTimeParser` | QuickTime Parser | QuickTime parsing |
| `CLSID_MPEG1Doc` | MPEG-1 Document | MPEG-1 System Stream |
| `CLSID_MPEG1Splitter` | MPEG-1 Splitter | MPEG-1 demux |
| `CLSID_MPEG1PacketPlayer` | MPEG-1 Packet Player | MPEG-1 playback |
| `CLSID_MPEG2Demultiplexer` | MPEG-2 Demultiplexer | MPEG-2 TS/PS demux |
| `CLSID_MPEG2Demultiplexer_NoClock` | MPEG-2 Demux (NoClock) | No-clock variant |
| `CLSID_MMSPLITTER` | MMS Splitter | MMS stream demux |
| `CLSID_DVSplitter` | DV Splitter | DV format demux |

### Encoder/Codec Filters (11)
| CLSID | Filter | Purpose |
|-------|--------|---------|
| `CLSID_CMpegAudioCodec` | MPEG Audio Codec | MP1/MP2/MP3 encode/decode |
| `CLSID_CMpegVideoCodec` | MPEG Video Codec | MPEG-1/2 video codec |
| `CLSID_DVVideoCodec` | DV Video Codec | DV decode |
| `CLSID_DVVideoEnc` | DV Video Encoder | DV encode |
| `CLSID_ACMWrapper` | ACM Wrapper | Audio compression |
| `CLSID_AVIDec` | AVI Decoder | AVI decompression |
| `CLSID_MjpegDec` | MJPEG Decoder | Motion JPEG decode |
| `CLSID_MJPGEnc` | MJPEG Encoder | Motion JPEG encode |
| `CLSID_IVideoEncoderProxy` | Video Encoder Proxy | Encoder abstraction |
| `CLSID_IVideoEncoderCodecAPIProxy` | CodecAPI Encoder Proxy | CodecAPI encoder |
| `CLSID_ICodecAPIProxy` | CodecAPI Proxy | CodecAPI abstraction |

### Renderer/Display Filters (16)
| CLSID | Filter | Purpose |
|-------|--------|---------|
| `CLSID_VideoRenderer` | Video Renderer | Legacy video renderer |
| `CLSID_VideoRendererDefault` | Default Video Renderer | System default |
| `CLSID_VideoMixingRenderer` | Video Mixing Renderer | VMR-7 |
| `CLSID_VideoMixingRenderer9` | Video Mixing Renderer 9 | VMR-9 |
| `CLSID_EnhancedVideoRenderer` | Enhanced Video Renderer | EVR |
| `CLSID_AllocPresenter` | Alloc Presenter | EVR presenter |
| `CLSID_AllocPresenterDDXclMode` | Alloc Presenter DDX | EVR DDX mode |
| `CLSID_EVRTearlessWindowPresenter9` | EVR Tearless Presenter | Tear-free EVR |
| `CLSID_EVRPlaybackPipelineOptimizer` | EVR Pipeline Optimizer | EVR optimization |
| `CLSID_MFVideoMixer9` | MF Video Mixer 9 | MF video mixer |
| `CLSID_MFVideoPresenter9` | MF Video Presenter 9 | MF video presenter |
| `CLSID_AudioRender` | Audio Renderer | DirectSound audio |
| `CLSID_DSoundRender` | DirectSound Renderer | DirectSound output |
| `CLSID_OverlayMixer` | Overlay Mixer | Overlay compositing |
| `CLSID_ModexRenderer` | Modex Renderer | Modex rendering |
| `CLSID_TextRender` | Text Renderer | Text overlay |

### TV/Tuner/Network Filters (14)
| CLSID | Filter | Purpose |
|-------|--------|---------|
| `CLSID_ATSCNetworkProvider` | ATSC Network Provider | ATSC tuning |
| `CLSID_DVBCNetworkProvider` | DVB-C Network Provider | Cable tuning |
| `CLSID_DVBSNetworkProvider` | DVB-S Network Provider | Satellite tuning |
| `CLSID_DVBTNetworkProvider` | DVB-T Network Provider | Terrestrial tuning |
| `CLSID_NetworkProvider` | Network Provider | Generic tuning |
| `CLSID_DShowTVEFilter` | DShow TV Filter | TV integration |
| `CLSID_TVEFilterTuneProperties` | TVE Tune Properties | TV tuning props |
| `CLSID_TVEFilterCCProperties` | TVE CC Properties | Closed captioning |
| `CLSID_TVEFilterStatsProperties` | TVE Stats Properties | TV statistics |
| `CLSID_TVTunerFilterPropertyPage` | TV Tuner Property Page | Tuner UI |
| `CLSID_CrossbarFilterPropertyPage` | Crossbar Property Page | Crossbar UI |
| `CLSID_AnalogVideoDecoderPropertyPage` | Analog Video Decoder Page | Decoder UI |
| `CLSID_TVAudioFilterPropertyPage` | TV Audio Property Page | Audio UI |
| `CLSID_ATSCNetworkPropertyPage` | ATSC Network Page | ATSC UI |

### Capture Filters (11)
| CLSID | Filter | Purpose |
|-------|--------|---------|
| `CLSID_VfwCapture` | VFW Capture | Video capture |
| `CLSID_CaptureGraphBuilder` | Capture Graph Builder | Capture pipeline |
| `CLSID_CaptureGraphBuilder2` | Capture Graph Builder 2 | Enhanced builder |
| `CLSID_AudioInputDeviceCategory` | Audio Input Device | Microphone |
| `CLSID_AudioInputMixerProperties` | Audio Mixer Props | Mixer UI |
| `CLSID_VideoInputDeviceCategory` | Video Input Device | Camera |
| `CLSID_CameraControlPropertyPage` | Camera Control Page | Camera UI |
| `CLSID_VideoProcAmpPropertyPage` | Video Proc Amp Page | Proc amp UI |
| `CLSID_VideoStreamConfigPropertyPage` | Stream Config Page | Config UI |
| `CLSID_AudioRecord` | Audio Record | Audio capture |
| `CLSID_CaptureProperties` | Capture Properties | Capture config |

### Stream Buffer Engine (SBE) (9)
| CLSID | Filter | Purpose |
|-------|--------|---------|
| `CLSID_SBE2Sink` | SBE2 Sink | Buffering sink |
| `CLSID_SBE2File` | SBE2 File | Buffered file |
| `CLSID_StreamBufferSink` | Stream Buffer Sink | DVR sink |
| `CLSID_StreamBufferSource` | Stream Buffer Source | DVR source |
| `CLSID_StreamBufferComposeRecording` | SBE Compose Recording | Multi-recording |
| `CLSID_StreamBufferConfig` | Stream Buffer Config | SBE config |
| `CLSID_StreamBufferPropertyHandler` | SBE Property Handler | Properties |
| `CLSID_StreamBufferRecordingAttributes` | SBE Recording Attrs | Recording metadata |
| `CLSID_StreamBufferThumbnailHandler` | SBE Thumbnail Handler | Thumbnails |

### Filter Graph Infrastructure (21)
| CLSID | Filter | Purpose |
|-------|--------|---------|
| `CLSID_FilterGraph` | Filter Graph | Standard graph builder |
| `CLSID_FilterGraphNoThread` | Filter Graph (No Thread) | Single-threaded |
| `CLSID_FilterGraphPrivateThread` | Filter Graph (Private Thread) | Isolated thread |
| `CLSID_ProtoFilterGraph` | Proto Filter Graph | Template graph |
| `CLSID_FGControl` | FG Control | Graph control |
| `CLSID_FilterMapper` | Filter Mapper | Legacy mapper |
| `CLSID_FilterMapper2` | Filter Mapper 2 | Enhanced mapper |
| `CLSID_SystemClock` | System Clock | Reference clock |
| `CLSID_MemoryAllocator` | Memory Allocator | Sample allocator |
| `CLSID_MediaPropertyBag` | Media Property Bag | Property storage |
| `CLSID_SeekingPassThru` | Seeking Pass Thru | Seeking delegation |
| `CLSID_SmartTee` | Smart Tee | Split output |
| `CLSID_InfTee` | Infinite Tee | Broadcast split |
| `CLSID_PersistMonikerPID` | Persist Moniker PID | Moniker persist |
| `CLSID_QualityProperties` | Quality Properties | Quality control |
| `CLSID_PerformanceProperties` | Performance Properties | Perf monitoring |
| `CLSID_DirectDrawProperties` | DirectDraw Properties | DD config |
| `CLSID_Colour` | Colour Space | Color conversion |
| `CLSID_Dither` | Dither | Dithering |
| `CLSID_ModexProperties` | Modex Properties | Modex config |
| `CLSID_DirectShowPluginControl` | DirectShow Plugin Control | Plugin mgmt |

### Category Enumeration (14)
| CLSID | Category | Purpose |
|-------|----------|---------|
| `CLSID_LegacyAmFilterCategory` | Legacy AM Filters | Filter enumeration |
| `CLSID_DeviceControlCategory` | Device Control | Device enumeration |
| `CLSID_ActiveMovieCategories` | ActiveMovie Categories | Category root |
| `CLSID_SystemDeviceEnum` | System Device Enumerator | Device COM |
| `CLSID_CDeviceMoniker` | Device Moniker | Device binding |
| `CLSID_DVDHWDecodersCategory` | DVD HW Decoders | Hardware decoders |
| `CLSID_AudioCompressorCategory` | Audio Compressors | Audio codec enum |
| `CLSID_VideoCompressorCategory` | Video Compressors | Video codec enum |
| `CLSID_MediaEncoderCategory` | Media Encoder | Encoder enum |
| `CLSID_MediaMultiplexerCategory` | Media Multiplexer | Muxer enum |
| `CLSID_MidiRendererCategory` | MIDI Renderer | MIDI output enum |
| `CLSID_TransmitCategory` | Transmit | Network transmit |
| `CLSID_VideoPortManager` | Video Port Manager | VP enumeration |
| `CLSID_AudioRendererCategory` | Audio Renderer | Audio output enum |

### DVD Navigation, Writers, Captioning, Other
| CLSID | Filter | Purpose |
|-------|--------|---------|
| `CLSID_DVDNavigator` | DVD Navigator | DVD navigation |
| `CLSID_DvdGraphBuilder` | DVD Graph Builder | DVD pipeline |
| `CLSID_DVDecPropertiesPage` | DV Decoder Properties | DV decode UI |
| `CLSID_DVEncPropertiesPage` | DV Encoder Properties | DV encode UI |
| `CLSID_AviDest` | AVI Destination | AVI muxing |
| `CLSID_AviMuxProptyPage` | AVI Mux Properties | Mux UI |
| `CLSID_AviMuxProptyPage1` | AVI Mux Properties 1 | Mux UI (alt) |
| `CLSID_WMAsfWriter` | WM ASF Writer | ASF/WMV writing |
| `CLSID_FileWriter` | File Writer | File sink |
| `CLSID_AVIMIDIRender` | AVI MIDI Render | MIDI output |
| `CLSID_Line21Decoder` | Line 21 Decoder | CC decode |
| `CLSID_Line21Decoder2` | Line 21 Decoder 2 | CC decode v2 |
| `CLSID_CaptionsFilter` | Captions Filter | Caption rendering |
| `CLSID_CCAFilter` | CC/A Filter | Closed captioning |
| `CLSID_DtvCcFilter` | DTV CC Filter | Digital TV CC |
| `CLSID_SubtitlesFilter` | Subtitles Filter | Subtitle rendering |
| `CLSID_WSTDecoder` | WST Decoder | Teletext/WST |
| `CLSID_WstDecoderPropertyPage` | WST Decoder Properties | WST UI |
| `CLSID_DVMux` | DV Mux | DV format mux |
| `CLSID_DVMuxPropertyPage` | DV Mux Properties | Mux UI |
| `CLSID_VBISurfaces` | VBI Surfaces | VBI output |
| `CLSID_VPObject` | Video Port Object | VP management |
| `CLSID_VPVBIObject` | VP VBI Object | VP VBI management |
| `CLSID_DShowSourceResolver` | CDShowSourceResolver | Custom DShow->MF resolver |

## MEDIASUBTYPE Constants (154 unique)

### Video Pixel Formats

#### RGB Formats (10)
`MEDIASUBTYPE_RGB1`, `MEDIASUBTYPE_RGB4`, `MEDIASUBTYPE_RGB8`, `MEDIASUBTYPE_RGB555`,
`MEDIASUBTYPE_RGB565`, `MEDIASUBTYPE_RGB24`, `MEDIASUBTYPE_RGB32`,
`MEDIASUBTYPE_ARGB1555`, `MEDIASUBTYPE_ARGB32`, `MEDIASUBTYPE_ARGB4444`

#### D3D Render Target Variants (10)
`MEDIASUBTYPE_RGB16_D3D_DX7_RT`, `MEDIASUBTYPE_RGB16_D3D_DX9_RT`,
`MEDIASUBTYPE_RGB32_D3D_DX7_RT`, `MEDIASUBTYPE_RGB32_D3D_DX9_RT`,
`MEDIASUBTYPE_ARGB1555_D3D_DX7_RT`, `MEDIASUBTYPE_ARGB1555_D3D_DX9_RT`,
`MEDIASUBTYPE_ARGB32_D3D_DX7_RT`, `MEDIASUBTYPE_ARGB32_D3D_DX9_RT`,
`MEDIASUBTYPE_ARGB4444_D3D_DX7_RT`, `MEDIASUBTYPE_ARGB4444_D3D_DX9_RT`

#### YUV Packed Formats (19)
`MEDIASUBTYPE_YUY2`, `MEDIASUBTYPE_YUYV`, `MEDIASUBTYPE_YVYU`, `MEDIASUBTYPE_UYVY`,
`MEDIASUBTYPE_IYUV`, `MEDIASUBTYPE_YV12`, `MEDIASUBTYPE_YVU9`, `MEDIASUBTYPE_IF09`,
`MEDIASUBTYPE_AYUV`, `MEDIASUBTYPE_CLJR`, `MEDIASUBTYPE_CLPL`, `MEDIASUBTYPE_CPLA`,
`MEDIASUBTYPE_CFCC`, `MEDIASUBTYPE_IMC1`, `MEDIASUBTYPE_IMC2`, `MEDIASUBTYPE_IMC3`,
`MEDIASUBTYPE_IMC4`, `MEDIASUBTYPE_IA44`, `MEDIASUBTYPE_AI44`

#### YUV Planar Formats (15)
`MEDIASUBTYPE_NV12`, `MEDIASUBTYPE_NV24`, `MEDIASUBTYPE_P010`, `MEDIASUBTYPE_P016`,
`MEDIASUBTYPE_P208`, `MEDIASUBTYPE_P210`, `MEDIASUBTYPE_P216`, `MEDIASUBTYPE_P408`,
`MEDIASUBTYPE_Y210`, `MEDIASUBTYPE_Y211`, `MEDIASUBTYPE_Y216`, `MEDIASUBTYPE_Y411`,
`MEDIASUBTYPE_Y41P`, `MEDIASUBTYPE_S340`, `MEDIASUBTYPE_S342`

#### Compressed Video (8)
`MEDIASUBTYPE_MJPG`, `MEDIASUBTYPE_IJPG`, `MEDIASUBTYPE_TVMJ`, `MEDIASUBTYPE_VPVideo`,
`MEDIASUBTYPE_VPVBI`, `MEDIASUBTYPE_Plum`, `MEDIASUBTYPE_WAKE`, `MEDIASUBTYPE_Overlay`

#### MPEG Video (7)
`MEDIASUBTYPE_MPEG1Video`, `MEDIASUBTYPE_MPEG1VideoCD`, `MEDIASUBTYPE_MPEG2_VIDEO`,
`MEDIASUBTYPE_H264`, `MEDIASUBTYPE_420O`, `MEDIASUBTYPE_A2B10G10R10`, `MEDIASUBTYPE_A2R10G10B10`

#### DV Formats (8)
`MEDIASUBTYPE_dv25`, `MEDIASUBTYPE_dv50`, `MEDIASUBTYPE_DVSD`, `MEDIASUBTYPE_dvh1`,
`MEDIASUBTYPE_dvhd`, `MEDIASUBTYPE_dvsl`, `MEDIASUBTYPE_DVCS`, `MEDIASUBTYPE_MDVF`

#### QuickTime (5)
`MEDIASUBTYPE_QTJpeg`, `MEDIASUBTYPE_QTMovie`, `MEDIASUBTYPE_QTRle`,
`MEDIASUBTYPE_QTRpza`, `MEDIASUBTYPE_QTSmc`

### Audio Formats (15)
`MEDIASUBTYPE_PCM`, `MEDIASUBTYPE_PCMAudio_Obsolete`, `MEDIASUBTYPE_IEEE_FLOAT`,
`MEDIASUBTYPE_DOLBY_AC3`, `MEDIASUBTYPE_DOLBY_AC3_SPDIF`, `MEDIASUBTYPE_DTS`,
`MEDIASUBTYPE_SDDS`, `MEDIASUBTYPE_SPDIF_TAG_241h`, `MEDIASUBTYPE_MPEG1Audio`,
`MEDIASUBTYPE_MPEG1AudioPayload`, `MEDIASUBTYPE_MPEG2_AUDIO`, `MEDIASUBTYPE_DRM_Audio`,
`MEDIASUBTYPE_AIFF`, `MEDIASUBTYPE_AU`, `MEDIASUBTYPE_WAVE`

### Container/Format Types (16)
`MEDIASUBTYPE_Asf`, `MEDIASUBTYPE_Avi`, `MEDIASUBTYPE_MPEG1Packet`,
`MEDIASUBTYPE_MPEG1Payload`, `MEDIASUBTYPE_MPEG1System`, `MEDIASUBTYPE_MPEG2_PROGRAM`,
`MEDIASUBTYPE_MPEG2_TRANSPORT`, `MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE`,
`MEDIASUBTYPE_MPEG2_UDCR_TRANSPORT`, `MEDIASUBTYPE_MPEG2_WMDRM_TRANSPORT`,
`MEDIASUBTYPE_MPEG2_PBDA_TRANSPORT_RAW`, `MEDIASUBTYPE_MPEG2_PBDA_TRANSPORT_PROCESSED`,
`MEDIASUBTYPE_MPEG2_VERSIONED_TABLES`, `MEDIASUBTYPE_MPEG2DATA`,
`MEDIASUBTYPE_DssAudio`, `MEDIASUBTYPE_DssVideo`

### DVD/Navigation (8)
`MEDIASUBTYPE_DVD_SUBPICTURE`, `MEDIASUBTYPE_DVD_LPCM_AUDIO`,
`MEDIASUBTYPE_DVD_NAVIGATION_PCI`, `MEDIASUBTYPE_DVD_NAVIGATION_DSI`,
`MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER`, `MEDIASUBTYPE_VBI`,
`MEDIASUBTYPE_VPS`, `MEDIASUBTYPE_WSS`

### Analog Video Standards (16)
`MEDIASUBTYPE_AnalogVideo_NTSC_M`, `MEDIASUBTYPE_AnalogVideo_PAL_B`,
`MEDIASUBTYPE_AnalogVideo_PAL_D`, `MEDIASUBTYPE_AnalogVideo_PAL_G`,
`MEDIASUBTYPE_AnalogVideo_PAL_H`, `MEDIASUBTYPE_AnalogVideo_PAL_I`,
`MEDIASUBTYPE_AnalogVideo_PAL_M`, `MEDIASUBTYPE_AnalogVideo_PAL_N`,
`MEDIASUBTYPE_AnalogVideo_PAL_N_COMBO`, `MEDIASUBTYPE_AnalogVideo_SECAM_B`,
`MEDIASUBTYPE_AnalogVideo_SECAM_D`, `MEDIASUBTYPE_AnalogVideo_SECAM_G`,
`MEDIASUBTYPE_AnalogVideo_SECAM_H`, `MEDIASUBTYPE_AnalogVideo_SECAM_K`,
`MEDIASUBTYPE_AnalogVideo_SECAM_K1`, `MEDIASUBTYPE_AnalogVideo_SECAM_L`

### Captioning/Subtitle/Broadcast (20)
`MEDIASUBTYPE_Line21_BytePair`, `MEDIASUBTYPE_Line21_GOPPacket`,
`MEDIASUBTYPE_Line21_VBIRawData`, `MEDIASUBTYPE_DtvCcData`,
`MEDIASUBTYPE_DVB_SUBTITLES`, `MEDIASUBTYPE_ISDB_CAPTIONS`,
`MEDIASUBTYPE_ISDB_SUPERIMPOSE`, `MEDIASUBTYPE_708_608Data`,
`MEDIASUBTYPE_TELETEXT`, `MEDIASUBTYPE_CC_CONTAINER`, `MEDIASUBTYPE_XDS`,
`MEDIASUBTYPE_ATSC_SI`, `MEDIASUBTYPE_DVB_SI`, `MEDIASUBTYPE_ISDB_SI`,
`MEDIASUBTYPE_TIF_SI`, `MEDIASUBTYPE_RAW_SPORT`, `MEDIASUBTYPE_None`

## MEDIATYPE Constants (25 unique)
`MEDIATYPE_Video`, `MEDIATYPE_Audio`, `MEDIATYPE_Stream`, `MEDIATYPE_Interleaved`,
`MEDIATYPE_Text`, `MEDIATYPE_ScriptCommand`, `MEDIATYPE_File`, `MEDIATYPE_AUXLine21Data`,
`MEDIATYPE_AUXTeletextPage`, `MEDIATYPE_Timecode`, `MEDIATYPE_Midi`,
`MEDIATYPE_AnalogVideo`, `MEDIATYPE_AnalogAudio`, `MEDIATYPE_VBI`,
`MEDIATYPE_DTVCCData`, `MEDIATYPE_MSTVCaption`, `MEDIATYPE_CC_CONTAINER`,
`MEDIATYPE_MPEG1SystemStream`, `MEDIATYPE_MPEG2_PACK`, `MEDIATYPE_MPEG2_PES`,
`MEDIATYPE_MPEG2_SECTIONS`, `MEDIATYPE_DVD_NAVIGATION`, `MEDIATYPE_DVD_ENCRYPTED_PACK`,
`MEDIATYPE_LMRT`, `MEDIATYPE_URL_STREAM`

## FORMAT Constants (28 unique)
`FORMAT_None`, `FORMAT_SAMPLE`, `FORMAT_FIELD`, `FORMAT_FRAME`, `FORMAT_MEDIA_TIME`,
`FORMAT_BYTE`, `FORMAT_VideoInfo`, `FORMAT_VIDEOINFO2`, `FORMAT_MPEGVideo`,
`FORMAT_MPEG2Video`, `FORMAT_DvInfo`, `FORMAT_Image`, `FORMAT_JPEGImage`,
`FORMAT_AnalogVideo`, `FORMAT_525WSS`, `FORMAT_WaveFormatEx`, `FORMAT_MPEG2Audio`,
`FORMAT_DolbyAC3`, `FORMAT_DVD_LPCMAudio`, `FORMAT_MPEGStreams`, `FORMAT_CC_CONTAINER`,
`FORMAT_CAPTIONED_MPEG2VIDEO`, `FORMAT_CAPTIONED_H264VIDEO`, `FORMAT_UVCH264Video`,
`FORMAT_ATSC`, `FORMAT_DVB`, `FORMAT_DIRECTV`, `FORMAT_ECHOSTAR`

## AM_KSCATEGORY Constants (11)
`AM_KSCATEGORY_AUDIO`, `AM_KSCATEGORY_CAPTURE`, `AM_KSCATEGORY_CROSSBAR`,
`AM_KSCATEGORY_DATACOMPRESSOR`, `AM_KSCATEGORY_RENDER`, `AM_KSCATEGORY_SPLITTER`,
`AM_KSCATEGORY_TVAUDIO`, `AM_KSCATEGORY_TVTUNER`, `AM_KSCATEGORY_VBICODEC`,
`AM_KSCATEGORY_VBICODEC_MI`, `AM_KSCATEGORY_VIDEO`

## AM_KSPROPSETID Constants (9)
`AM_KSPROPSETID_AC3`, `AM_KSPROPSETID_CopyProt`, `AM_KSPROPSETID_DVD_RateChange`,
`AM_KSPROPSETID_DvdKaraoke`, `AM_KSPROPSETID_DvdSubPic`, `AM_KSPROPSETID_FrameStep`,
`AM_KSPROPSETID_MPEG4_MediaType_Attributes`, `AM_KSPROPSETID_TSRateChange`, `AMPROPSETID_Pin`

## PIN_CATEGORY Constants (12)
`PIN_CATEGORY_CAPTURE`, `PIN_CATEGORY_PREVIEW`, `PIN_CATEGORY_VIDEOPORT`,
`PIN_CATEGORY_VIDEOPORT_VBI`, `PIN_CATEGORY_VBI`, `PIN_CATEGORY_CC`,
`PIN_CATEGORY_NABTS`, `PIN_CATEGORY_EDS`, `PIN_CATEGORY_TELETEXT`,
`PIN_CATEGORY_TIMECODE`, `PIN_CATEGORY_ANALOGVIDEOIN`, `PIN_CATEGORY_STILL`

## IID Constants (27)
`IID_IKsControl`, `IID_IKsDataTypeHandler`, `IID_IKsInterfaceHandler`,
`IID_IKsPin`, `IID_IKsPinFactory`, `IID_IAMDirectSound`, `IID_IAMLine21Decoder`,
`IID_IAMWstDecoder`, `IID_IBaseVideoMixer`, `IID_IDDVideoPortContainer`,
`IID_IDirectDrawKernel`, `IID_IDirectDrawSurfaceKernel`, `IID_IDirectDrawVideo`,
`IID_IFullScreenVideo`, `IID_IFullScreenVideoEx`, `IID_IMixerPinConfig`,
`IID_IMixerPinConfig2`, `IID_IMpegAudioDecoder`, `IID_IQualProp`,
`IID_IVPConfig`, `IID_IVPControl`, `IID_IVPNotify`, `IID_IVPNotify2`,
`IID_IVPObject`, `IID_IVPVBIConfig`, `IID_IVPVBINotify`, `IID_IVPVBIObject`

## DSATTRIB Constants (9)
`DSATTRIB_CAPTURE_STREAMTIME`, `DSATTRIB_CC_CONTAINER_INFO`,
`DSATTRIB_DSHOW_STREAM_DESC`, `DSATTRIB_OptionalVideoAttributes`,
`DSATTRIB_PBDATAG_ATTRIBUTE`, `DSATTRIB_PicSampleSeq`,
`DSATTRIB_SAMPLE_LIVE_STREAM_TIME`, `DSATTRIB_TRANSPORT_PROPERTIES`,
`DSATTRIB_UDCRTag`

## EVRConfig Constants (10)
`EVRConfig_AllowBatching`, `EVRConfig_ForceBatching`, `EVRConfig_AllowDropToBob`,
`EVRConfig_ForceBob`, `EVRConfig_AllowDropToHalfInterlace`, `EVRConfig_ForceHalfInterlace`,
`EVRConfig_AllowDropToThrottle`, `EVRConfig_ForceThrottle`, `EVRConfig_AllowScaling`,
`EVRConfig_ForceScaling`

## CODECAPI/UUID Constants (3)
`CODECAPI_AVDecMmcssClass`, `UUID_UdriTagTables`, `UUID_WMDRMTagTables`

## Filter Graph Construction Topology

Based on the CLSIDs, RTTI classes, and import analysis, WLMFDS.dll constructs
these filter graph topologies:

### 1. File Source -> Demux -> MF Source
```
CLSID_FileSource / CLSID_AsyncReader / CLSID_URLReader
  -> [parser/splitter based on file extension]
    -> CDShowSource (IMFMediaSource) wraps the filter graph
      -> CDShowStream (IMFMediaStream) wraps each output pin
```

### 2. MPEG-2 Transport Stream (Primary use case)
```
CLSID_FileSource (byte stream)
  -> CMPEG2Demultiplexer (built-in, CLSID_MPEG2Demultiplexer)
    -> CMPEG2DemuxOutputPin (per PID/stream)
      -> CMFSourceMPEG2TSPin (MF bridge pin)
        -> CMFSourceFilter (IMFMediaSource wrapper)
          -> CMFSourceOutputPin -> IMFMediaStream
```

### 3. WMV/ASF Source
```
CLSID_WMAsfReader -> ASF demux -> CDShowSource -> IMFMediaSource
```

### 4. AVI Source
```
CLSID_AviSplitter / CLSID_AviReader -> AVI demux -> CDShowSource -> IMFMediaSource
```

### 5. QuickTime Source
```
CLSID_MOVReader / CLSID_QTDec / CLSID_QuickTimeParser -> MOV demux -> CDShowSource -> IMFMediaSource
```

## Media Type Conversion Matrix

The bridge converts between DShow `AM_MEDIA_TYPE` and MF `IMFMediaType`
using `MFCreateMediaTypeFromRepresentation`:

| DShow MEDIASUBTYPE | MF Format |
|--------------------|-----------|
| `MEDIASUBTYPE_H264` | `MFVideoFormat_H264` |
| `MEDIASUBTYPE_MPEG2_VIDEO` | `MFVideoFormat_MPG2` |
| `MEDIASUBTYPE_MPEG1Video` | `MFVideoFormat_MPG1` |
| `MEDIASUBTYPE_NV12` | `MFVideoFormat_NV12` |
| `MEDIASUBTYPE_YUY2` | `MFVideoFormat_YUY2` |
| `MEDIASUBTYPE_RGB32` | `MFVideoFormat_RGB32` |
| `MEDIASUBTYPE_ARGB32` | `MFVideoFormat_ARGB32` |
| `MEDIASUBTYPE_PCM` | `MFAudioFormat_PCM` |
| `MEDIASUBTYPE_IEEE_FLOAT` | `MFAudioFormat_Float` |
| `MEDIASUBTYPE_MPEG2_AUDIO` | `MFAudioFormat_MP3` / AAC |
| `MEDIASUBTYPE_DOLBY_AC3` | `MFAudioFormat_Dolby_AC3` |
| `MEDIASUBTYPE_DTS` | `MFAudioFormat_DTS` |

## Import Dependencies (14 DLLs, ~185 functions)

| DLL | Category | Functions |
|-----|----------|-----------|
| MSVCR110.dll | VC++ 2012 CRT | 42 (memory, string, CRT init, exception) |
| KERNEL32.dll | OS Core | 53 (memory, sync, thread, file, time) |
| ADVAPI32.dll | Registry/Security/ETW | 24 (registry, ACL, ETW tracing) |
| ole32.dll | COM Core | 14 (COM lifecycle, GUID, memory) |
| OLEAUT32.dll | COM Automation | 4 ordinals (SysString, VARIANT) |
| MFPlat.DLL | Media Foundation Platform | 17 (samples, buffers, events, types) |
| MF.dll | Media Foundation | 1 (MFCreateSourceResolver) |
| WINMM.dll | Multimedia Timers | 6 (timer events for clock) |
| AVRT.dll | Multimedia Scheduling | 2 (MMCSS thread priority) |
| EVR.dll | Enhanced Video Renderer | 1 (MFCopyImage) |
| SHELL32.dll | Shell | 1 (SHCreateItemFromParsingName) |
| SHLWAPI.dll | Shell Utility | 1 (PathIsRelativeW) |
| VERSION.dll | Version Info | 3 (file version queries) |
| USER32.dll | UI | 1 (CharNextW) |

## Key Findings

1. **160+ RTTI classes** spanning the full DShow-to-MF bridge stack
2. **150 unique CLSID string references** for DShow filter graph construction
3. **154 MEDIASUBTYPE constants** covering every major media format
4. **CLSID_DShowSourceResolver** is the single COM-creatable object exported by this DLL
5. **MPEG-2 demux is the largest component** (~60 classes) with full PAT/PMT/PES/PSI parsing
6. **14 async callback classes** implement the MF async bridge pattern for DShow sample delivery
7. **ETW tracing** is deeply integrated with 20+ event classes in the Mpeg2DemuxTrace namespace
8. **MacroVision copy protection** support is built into the MPEG-2 program controller
9. **AVRT MMCSS** thread scheduling ensures low-latency audio/video processing
10. **Stream Buffer Engine (SBE)** integration provides DVR/timeshifting capability
