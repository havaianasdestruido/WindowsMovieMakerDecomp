# Windows Live Movie Maker 2012 — Complete Architecture Synthesis

**Product:** Windows Live Movie Maker 2012 (codename "Sundance")  
**Version:** 16.4.3528.0331 (ship.client.main.w5m4)  
**Build Date:** 2014-04-01  
**Compiler:** MSVC 11.0 (VS 2012), x86 (32-bit)  
**Target OS:** Windows 8+ (6.02)  
**Framework:** ATL/WTL 11.0, DirectUI, COM, DirectX 9/11, Media Foundation, DirectShow  
**Source Tree:** `e:\bt\1105173\client\personalmedia\`  

---

## 1. Layered Architecture Diagram

```
================================================================================
 LAYER 0: EXECUTABLE LAUNCHER
================================================================================
 MovieMaker.exe  (54 KB, thin stub)      WLXCodecHost.exe    WLXTranscode.exe
   Entry: wWinMain → SetDllDirectoryW()  (codec isolation)   (MF transcode)
          → MovieMakerMain() (delay-load)
   Direct deps: KERNEL32 | MSVCR110 | WLXPhotoBase.dll
================================================================================
 LAYER 1: CORE APPLICATION — MovieMakerCore.dll  (10.1 MB, 125+ classes)
================================================================================
 +-- SundanceAppMain, CommandLineParser, MediaBrowser
 +-- AutoSaveManager, TemplateTable, ClipboardManager
 +-- SundanceMainElementBehavior, AMPMainWindowBehavior
 +-- 40+ UI Behaviors (timeline, preview, effects, dialogs)
 +-- 15 Timeline classes (TimelineBehavior, DragDrop, Items, Tracks...)
 +-- 12 Dialog/Host classes (CDUIDialog, CFramelessHost...)
 +-- 20 Data/Model classes (AppDataContext, ComplexProperty, SFTime...)
 +-- 12 Media/Encoding classes (TrimBehavior, ProfileHandling, Narration...)
 +-- 8 Publishing classes (PublishManager, PublishJob, BackgroundWorker...)
 +-- 6 Text/Title classes (TextBoxBehavior, RichEdit, Caret...)
 +-- 5 Ribbon classes (RibbonApp, CategoryItem, List...)
 +-- 10 Site/Factory classes (CMRUSite, CGenericSite, BehaviorFactory...)
 +-- ESENT database (28 Jet functions) for local storage
 DirectUI (UXCore.dll): 180+ imported classes
================================================================================
 LAYER 2: PUBLISHING & AUTH — WLXMediaPublishSubscribe.dll  (1.4 MB)
================================================================================
 +-- MediaPublishSubscribeHelper, VideoTranscoder
 +-- LiveProvider, FlickrProvider (plus plugin ecosystem)
 +-- Grinder Task Scheduler (background job framework)
 +-- WinHTTP upload engine, wlidcli.dll (Live ID auth)
 +-- DPAPI credential protection
================================================================================
 LAYER 3: MEDIA LIBRARY — WLXMovieLibrary.dll  (316 KB)
================================================================================
 +-- MovieFactory, Movie, MovieBuilder
 +-- HMRAVSource (AVSource, AVSourceFactory, AsyncSourceResolver)
 +-- DXVA2 video processing (D3D9 + D3D11 paths)
 +-- WaveOut audio renderer (legacy WINMM)
 +-- MF Platform source readers (Native + DShow bridge)
 +-- DRM support via CryptAPI
================================================================================
 LAYER 4: SLIDESHOW — WLXSlideshow.dll  (520 KB) + WLXPhotoCinematic.dll  (88 KB)
================================================================================
 +-- 3 COM objects: SimpleSlideshowDisplay, SlideshowExtension, TimelineDisplay
 +-- 9 built-in themes (Fade, Sepia, Black & White, Classic, Album, Collage, Spin, Frame, Glass)
 +-- 38+ COM interfaces (ISlideshowControl, ISlideshowDisplay, IThemeManager...)
 +-- 156 RTTI classes (SlideshowFrameWindow, NavigationBar, CanvasView...)
 +-- D3D9 primary rendering + GDI+ fallback
 +-- CinematicKenBurns.dll: Ken Burns pan/zoom (2 COM objects, D3DX9 camera math)
================================================================================
 LAYER 5: VIDEO PROCESSING — WLXVideoTrim.dll  (568 KB)
================================================================================
 +-- DirectShow filter graph engine (not MF)
 +-- 5 factory exports (AVICopier, VideoCopierFromMT, Transcoder, Player, WMVTranscoder)
 +-- 5 copier types (Asf, AVI, Mpg2, StreamBuffer, base VideoCopier)
 +-- 5 transcoder types (Asf, AVI, DV, Mpg2, StreamBuffer)
 +-- 4 custom DShow filters (AudioRepackage, CopyProgressInfo, VideoRotation, YUY2ToI420)
 +-- Movely WLXMP4Parser.dll for MP4/MOV/3GP support
================================================================================
 LAYER 6: PIPELINE (FX/TRANSITIONS) — WLXPipeline.dll + WLXPipetran.dll
================================================================================
 +-- WLXPipeline: D3D9 rendering pipeline, DDRAW, DMO support (msdmo.dll), GDI+, WinMM timers
 +-- WLXPipetran: Pipeline transport with D3DX9 mesh/quaternion/camera math, GDI+ text, image lists
 +-- No WLXPhotoBase dependency — standalone pipeline infrastructure
================================================================================
 LAYER 7: MEDIA FOUNDATION BRIDGE — WLMFDS.dll + WLMFReadWrite.dll
================================================================================
 +-- WLMFDS: MF↔DirectShow bridge, MF source resolution, AVRT for AV thread priority
 +-- WLMFReadWrite: MF read/write helper, MFT enumeration, work queues, EVR interop
================================================================================
 LAYER 8: CODEC ISOLATION — WLXCodecHost.exe + WLXTranscode.exe
================================================================================
 +-- WLXCodecHost.exe: Out-of-process EXE (~32 KB code) hosting codecs for isolation
       Dependencies: WLXPhotoBase, MetadataSys, GDI+, PROPSYS
 +-- WLXTranscode.exe: Standalone EXE for MF-based transcoding
       Dependencies: MF, MFPlat, D3D9, D3D11, DXVA2, CryptAPI, WLXPhotoBase
================================================================================
 LAYER 9: PARSERS — WLXMP4Parser.dll
================================================================================
 +-- MP4/MOV/3GP container parser, GDI+ thumbnail extraction, WinMM timers, message loop
================================================================================
 LAYER 10: FACE RECOGNITION — WLXFaceRecognition.dll  (4.5 MB, MSRA origin)
================================================================================
 +-- 7 COM objects (Detection, Pipeline, Region, RegionSet, RepImpl, ImageData, ImageManager)
 +-- 4.4 MB embedded cascade/neural network models
 +-- 11 pose categories, 5 feature extractors (LBP, PCA, Texton, RPTexton, RPTextonPCA)
 +-- Standalone — not directly used by MovieMakerCore (Photo Gallery feature)
================================================================================
 LAYER 11: FOUNDATION — WLXPhotoBase.dll  (56 KB, 56 exports)
================================================================================
 +-- Exception handling (13 exports): Base::Exception, Throw, ThrowLastError
 +-- Version management (11 exports): Base::Version (parse, compare, format)
 +-- Error reporting (13 exports): WER, SQM, ship asserts, fault reporting
 +-- Memory management (4 exports): BasePrivate::New/Delete, VerifyPtr, leak tracking
 +-- String management (4 exports): ATL CAtlStringMgr, BaseAtlThrow
 +-- OS/CPU detection (6 exports): IsVista/Win7/Win8OrGreater, GetProcessorCaps/Count
 +-- Module version (2 exports): GetModuleAddresses, GetModuleVersion
 Dependencies: MSVCR110 | KERNEL32 | PSAPI | wer.dll | VERSION | SHLWAPI | ole32 (delay)
================================================================================
 LAYER 12: TELEMETRY INFRASTRUCTURE
================================================================================
 +-- WLXPhotoSqm.dll: SQM (Service Quality Monitoring) — counters, timers, streams
 +-- DmxBici.dll: BICI (Business Intelligence Customer Insights) — A/B testing, experiments
 +-- wer.dll: Windows Error Reporting — dump generation, fault submission
 +-- ADVAPI32 ETW: RegisterTraceGuidsW / TraceEvent in every DLL
 +-- Custom HTTP error reporting: http://g.live.com?version=...&ErrorCode=...
================================================================================
```

---

## 2. Data Flow: Media File Lifecycle

```
                    SOURCE (File, Webcam, Capture)
                           |
                           v
              +---------------------------+
              |  Import Phase             |
              |  MovieMakerCore.dll       |
              |  CommandLineMediaFileList |
              |  / ChunkMediaFileList     |
              +---------------------------+
                           |
                WLXMovieLibrary.dll
                (registry-based library)
                           |
                           v
              +---------------------------+
              |  Project Model Phase      |
              |  MovieMakerCore.dll       |
              |  Project → DataStr XML    |
              |  Timeline: Track, Items,  |
              |  Effects, Transitions     |
              |  (.wlmp file format)      |
              +---------------------------+
                           |
                           v
              +---------------------------+
              |  Preview Phase            |
              |  MovieMakerCore.dll       |
              |  + WLXMovieLibrary.dll    |
              |  MF Platform (MFPlat.dll) |
              |  → MF Source Resolver     |
              |  → DXVA2 decode (D3D9/11) |
              |  → EVR / D3D11 render     |
              |  GDI+ thumbnail fallback  |
              +---------------------------+
                           |
              +---------------------------+
              |  Editing Phase            |
              |  Trimming: WLXVideoTrim   |
              |  (DirectShow filter graph)|
              |  Effects: WLXPipeline     |
              |  (D3D9+DDRAW transitions) |
              |  Titles: MovieMakerCore   |
              |  (RichEdit, GDI+/DWrite)  |
              |  Slideshow: WLXSlideshow  |
              |  + WLXPhotoCinematic      |
              |  (D3D9 sprite renderer)   |
              +---------------------------+
                           |
                           v
              +---------------------------+
              |  Export Phase             |
              |  Formats: .wmv, .mp4,     |
              |  .m4a, .wma               |
              |  WLXTranscode.exe (MF)    |
              |  WLXVideoTrim.dll (DShow) |
              |  UserEncodeProfile*       |
              |  + WLMFReadWrite.dll      |
              +---------------------------+
                           |
                           v
              +---------------------------+
              |  Publishing Phase         |
              |  WLXMediaPublishSubscribe |
              |  → VideoTranscoder        |
              |  → Grinder Scheduler      |
              |  → Provider (Flickr/Live) |
              |  → WinHTTP upload         |
              |  → wlidcli.dll auth       |
              +---------------------------+
```

---

## 3. Component Dependency Chain & Startup Sequence

### 3.1 DLL Load Order

```
MovieMaker.exe launch
  │
  ├── KERNEL32.dll        (always loaded by OS)
  ├── MSVCR110.dll        (CRT init, exception handling)
  ├── WLXPhotoBase.dll    (direct import: BasePrivate::Delete)
  │
  └── [delay-load] MovieMakerCore.dll
        │
        ├── WLXPhotoBase.dll      (14 imports: exception, version, OS detection, memory, string)
        ├── WLXPhotoSqm.dll       (13 imports: SQM telemetry startup/shutdown)
        ├── DmxBici.dll           (5 imports: BICI telemetry)
        ├── UXCore.dll            (180+ imports: DirectUI framework)
        ├── wlidcli.dll           (7 ordinals: Windows Live ID)
        ├── uxctl.dll             (3: UX controls)
        ├── MetadataSys.dll       (1: property handler)
        ├── MFPlat.DLL            (18: MF platform startup)
        ├── MF.dll                (2: MFGetService)
        ├── ESENT.dll             (28: Jet database for local storage)
        ├── gdiplus.dll           (24: GDI+ 2D rendering)
        ├── d3d11.dll             (1: D3D11CreateDevice)
        ├── d3d9.dll              (2: Direct3DCreate9/Ex)
        ├── d2d1.dll              (1 ordinal: D2D1CreateFactory)
        ├── DWrite.dll            (1: DWriteCreateFactory)
        ├── dxva2.dll             (2: DXVA2 device manager + video service)
        ├── dwmapi.dll            (1: DwmExtendFrameIntoClientArea)
        ├── UxTheme.dll           (3: visual styles)
        ├── D3DCOMPILER_46.dll    (2: shader compilation)
        ├── WindowsCodecs.dll     (1: WICConvertBitmapSource)
        ├── XmlLite.dll           (4: XML reader/writer)
        ├── PROPSYS.dll           (8: property system)
        ├── OLEACC.dll            (2: accessibility)
        ├── VERSION.dll           (3: version info)
        ├── WINMM.dll             (1: PlaySoundW)
        │
        ├── [on-demand] WLXMovieLibrary.dll
        │     └── MFPlat.DLL, MF.dll, WINMM (waveOut), d3d9, d3d11, dxva2, PROPSYS
        │
        ├── [on-demand] WLXVideoTrim.dll
        │     ├── WMVCore.DLL         (Windows Media Format SDK)
        │     └── [delay] WLXMP4Parser.dll
        │
        ├── [on-demand] WLXPipeline.dll
        │     ├── d3d9.dll, d3dx9_32.dll, DDRAW.dll, msdmo.dll
        │     ├── WINMM.dll (timers), gdiplus.dll
        │     └── (no WLXPhotoBase)
        │
        ├── [on-demand] WLXPipetran.dll
        │     ├── d3dx9_32.dll (mesh/transform)
        │     ├── gdiplus.dll, COMCTL32.dll, PSAPI.DLL
        │     └── (no WLXPhotoBase)
        │
        ├── [on-demand] WLXSlideshow.dll
        │     ├── UXCore.dll, WLXPhotoBase (delay), WLXPhotoSqm (delay), DmxBici
        │     ├── d3d9, d3dx9_32, gdiplus, GDI32, OLEACC
        │     └── WTSAPI32, msi.dll, dwmapi (delay)
        │
        ├── [on-demand] WLXPhotoCinematic.dll
        │     ├── d3dx9_32.dll (camera math)
        │     ├── WLXPhotoBase (delay), ADVAPI32 (ETW+reg)
        │     └── (no MF — self-contained D3D9)
        │
        ├── [on-demand] WLXMediaPublishSubscribe.dll
        │     ├── WLXPhotoSqm, DmxBici, wlidcli
        │     ├── WINHTTP, WININET, CRYPT32, gdiplus
        │     ├── UXCore, UxTheme
        │     ├── [delay] WLXPhotoBase, MetadataSys, msi
        │     └── (no MF — transcode via separate VideoTranscoder helper)
        │
        └── [on-demand] WLXFaceRecognition.dll
              └── WLXPhotoSqm, WLXPhotoBase, WindowsCodecs
              └── (not directly used by MovieMaker - Photo Gallery feature)
```

### 3.2 MovieMakerCore.dll Initialization Sequence (from DllMain / MovieMakerMain)

```
1. CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)
2. ATL _Module.Init(ObjectMap, hInstance, &LIBID)
3. _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE)
4. GdiplusStartup()
5. MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET)
6. UXCore / DirectUI initialization
   → RMInitialize()
   → DUI: Create UI elements, load layouts
7. D3D11CreateDevice() — hardware rendering device
8. D2D1CreateFactory() — Direct2D factory
9. DWriteCreateFactory() — DirectWrite factory
10. WIC factory creation (IWICImagingFactory)
11. SundanceAppMain construction
    → CommandLineParser (load .wlmp, media files from CLI)
    → MediaBrowser (scan library)
    → AutoSaveManager (recovery check)
    → Create main window (WindowsLiveMovieMakerMainWindowClass)
    → Register Ribbon (RibbonApp, categories, items)
    → Load project (.wlmp XML via XmlLite)
    → Build timeline (Track → Items → Effects/Transitions)
    → Enter message loop
```

### 3.3 Shutdown Sequence (from DLL_PROCESS_DETACH)

```
1. _Module.RevokeClassObjects() + _Module.Term()
2. UXCore Uninit (RMTerminate)
3. MFShutdown()
4. GdiplusShutdown()
5. Release D3D11 device/context
6. Release D2D1 factory
7. Release DWrite factory
8. Release WIC factory
9. CoUninitialize()
```

---

## 4. COM Object Map

| DLL | CLSID | ProgID / Name | Threading | Purpose |
|-----|-------|---------------|-----------|---------|
| **WLXPhotoCinematic** | `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}` | `Microsoft.Photos.Slideshow.CinematicFullScreen1.1` | Apartment | Full-screen Ken Burns renderer |
| **WLXPhotoCinematic** | `{5409AB48-D8D3-40e6-A1EB-23489DC422DE}` | `Microsoft.Photos.Slideshow.CinematicTransform.1` | Apartment | Ken Burns camera transform |
| **WLXSlideshow** | `{84FBA192-4F8D-4a5d-94DE-083446ACC1D0}` (likely) | `CLSID_SimpleSlideshowDisplay` | — | Slideshow rendering surface |
| **WLXSlideshow** | — | `CLSID_SlideshowExtension` | — | Slideshow extension point |
| **WLXSlideshow** | — | `CLSID_TimelineDisplay` | — | Timeline display widget |
| **WLXVideoTrim** | `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | — | — | Video trim processor (self-reg) |
| **WLXFaceRecognition** | `{EF401225-1260-4716-A842-7D180DC14C1E}` | `CLSID_FaceRecognitionPipeline` | — | Face recognition pipeline |
| **WLXFaceRecognition** | `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` (likely) | `CLSID_FaceDetection` | — | Face detection engine |
| **WLXFaceRecognition** | `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` (likely) | `CLSID_ImageManager` | — | Image loading/management |
| **WLXFaceRecognition** | `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` (likely) | `CLSID_FaceRegion` / `CLSID_FaceRegionSet` | — | Face region / region set |
| **WLXFaceRecognition** | — | `CLSID_FaceRepImpl` | — | Face feature representation |
| **WLXFaceRecognition** | — | `CLSID_ImageData` | — | Image data wrapper |
| **WLXMovieLibrary** | — | `IMovieFactory` (via `CreateMovieFactory` export) | — | Movie library factory |
| **MovieMakerCore** | `{CE8B9537-708C-4784-9DD4-127B635DD348}` (likely APPID) | `Microsoft\Live\MovieMaker` | — | Application COM APPID |
| **WLXMediaPublishSubscribe** | — | `CLSID_LiveProvider` | — | Windows Live/SkyDrive publishing |
| **WLXMediaPublishSubscribe** | — | `CLSID_FlickrProvider` | — | Flickr photo publishing |
| **WLXMediaPublishSubscribe** | — | `CLSID_MediaPluginManager` | — | Plugin manager |
| **WLXMediaPublishSubscribe** | — | `CLSID_MediaPublishSubscribeProviderManager` | — | Provider manager factory |
| **WLXMediaPublishSubscribe** | — | `CLSID_MetadataManager` | — | Metadata manager |
| **WLXMediaPublishSubscribe** | — | `CLSID_LivePlugin` / `CLSID_LivePluginT` | — | Plugin templates |

### External COM Objects Used (via CoCreateInstance)

| CLSID | Where Used | Source |
|-------|-----------|--------|
| `CLSID_FilterGraph` | WLXVideoTrim | DirectShow |
| `CLSID_CaptureGraphBuilder2` | WLXVideoTrim | DirectShow |
| `CLSID_FileWriter` | WLXVideoTrim | DirectShow |
| `CLSID_EnhancedVideoRenderer` | WLXVideoTrim | EVR |
| `CLSID_VideoMixingRenderer9` | WLXVideoTrim | VMR-9 |
| `CLSID_MFSourceFilter` | WLXVideoTrim | MF bridge |
| `CLSID_MFMPEG2Demultiplexer` | WLXVideoTrim | MF MPEG-2 demux |
| `CLSID_StreamBufferSink` / `Config` / `ComposeRecording` | WLXVideoTrim | Stream Buffer Engine |
| `CLSID_DMOWrapperFilter` | WLXVideoTrim | DMO → DShow bridge |

### Key COM Interfaces (by component)

| Interface | Implemented By | Used By |
|-----------|---------------|---------|
| `ISlideshowControl` | WLXSlideshow | MovieMakerCore |
| `ISlideshowDisplay` | WLXSlideshow | MovieMakerCore |
| `ISlideshowThemeManager` | WLXSlideshow | MovieMakerCore |
| `IPublishSubscribeProvider` | WLXMediaPublishSubscribe | Plugin system |
| `ILiveSignInProvider` | WLXMediaPublishSubscribe (wlidcli) | Auth flow |
| `IGrinderTaskScheduler` | WLXMediaPublishSubscribe | Background jobs |
| `IMovieFactory` / `IMovie` | WLXMovieLibrary | MovieMakerCore |
| `IFaceDetection` / `IFaceRecognitionPipeline` | WLXFaceRecognition | Photo Gallery |
| `IMFSourceReaderCallback` | WLXMovieLibrary | MF async |
| `IMFAsyncCallback` | WLXMovieLibrary | MF work items |

### Transition DLL References (from WLXPhotoCinematic embedded XML)

| GUID | Reference |
|------|-----------|
| `{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}` | `TransitionsAndEffects` DLL (likely WLXPipeline) |
| `{7371ADEE-C195-427F-B0EC-3CCC13725665}` | `TransitionsAndEffects` DLL (likely WLXPipetran) |

---

## 5. Pipeline Stages: Complete Media Processing Pipeline

### 5.1 Source → Sink Pipeline (Export/Transcode)

```
[SOURCE]
  │
  ├── Local file (.wmv, .avi, .mp4, .mpg, .dvr-ms, .mov, .3gp, .wtv...)
  ├── Webcam (AVCaptureSession, AVCaptureCore)
  ├── Audio capture (AudioCaptureSession, NarrationUI)
  └── Photo stills (via WLXPhotoBase → GDI+ / WIC)
        │
        v
[STAGE 1: SOURCE RESOLUTION]
  │
  ├── NativeMFSourceReaderBuilder (MF source resolver)
  ├── DShowMFSourceReaderBuilder (DirectShow → MF bridge via WLMFDS)
  └── AsyncSourceResolver (for remote/non-seekable sources)
        │
        v
[STAGE 2: DEMULTIPLEXING]
  │
  ├── MF Source → MF Demux (MP4, WMV, etc.)
  ├── DirectShow Filter Graph → Splitter (AVI, MPEG-2)
  ├── WLXMP4Parser → AddMP4SourceFilter (MP4/MOV/3GP custom source)
  └── Stream Buffer Engine → DVR-MS parser
        │
        v
[STAGE 3: DECODING]
  │
  ├── Media Foundation → MFTs (hardware DXVA2 accelerated)
  ├── DirectShow → DShow filters (software)
  ├── WMVCore → WMV decoder (WMCreateReader)
  └── WLXCodecHost.exe (out-of-process for codec isolation)
        │
        v
[STAGE 4: VIDEO PROCESSING]
  │
  ├── WLXPipeline (D3D9 effects + transitions)
  │     ├── D3DX9 shaders, DDRAW fallback
  │     └── DMO transforms via msdmo.dll
  ├── WLXPipetran (transport + D3DX9 mesh transforms)
  │     ├── Quaternion rotation, matrix transforms
  │     └── GDI+ text overlay
  ├── WLXVideoTrim (trimming engine)
  │     ├── Copier path (re-mux, no decode)
  │     │     └── AsfCopier, AVICopier, Mpg2Copier, StreamBufferCopier
  │     └── Transcoder path (decode + re-encode)
  │           └── AsfTranscoder, AVITranscoder, DVTranscoder, Mpg2Transcoder, StreamBufferTranscoder
  ├── WLXSlideshow (still images → video)
  │     ├── D3D9 sprite composition (primary)
  │     └── GDI+ buffered painter (fallback)
  ├── WLXPhotoCinematic (Ken Burns pan/zoom)
  │     └── D3DX9 camera matrix → textured quad animation
  └── Custom DShow transform filters:
        ├── VideoRotationFilter (CW/CCW rotation)
        ├── AudioRepackageFilter (format conversion)
        ├── YUY2ToI420Filter (color space)
        └── CopyProgressInfoFilter (progress tracking)
        │
        v
[STAGE 5: ENCODING / MUXING]
  │
  ├── MF Transcode API (via WLXTranscode.exe)
  │     ├── MFCreateTranscodeProfile
  │     ├── MFCreateTranscodeTopology
  │     └── MF Media Session (topology-based)
  ├── DirectShow multiplexing
  │     ├── DVMux (DV-AVI)
  │     └── ASF Writer (WMV)
  ├── WMVCore: WMCreateWriter (WMV direct encoding)
  └── UserEncodeProfile system (.wlvs profiles, recommended settings)
        │
        v
[STAGE 6: PREVIEW]
  │
  ├── InlinePreviewLayoutBehavior (embedded preview pane)
  ├── FullscreenLayoutBehavior (fullscreen playback)
  ├── Options:
  │     ├── EVR (Enhanced Video Renderer) — primary
  │     ├── VMR-9/VMR-7 — fallback
  │     ├── Direct3D 9/11 swap chain
  │     └── WaveOut audio (WINMM legacy, not WASAPI)
  └── Thumbnail capture: VideoPlayer::TakePicture() → BMP via EVR/VMR GetCurrentImage
        │
        v
[STAGE 7: PUBLISH / EXPORT SINK]
  │
  ├── Local file export (.wmv/.mp4)
  ├── WLXMediaPublishSubscribe (online upload)
  │     ├── VideoTranscoder → analysis (bandwidth, duration, size)
  │     ├── Grinder scheduler → async job queue
  │     ├── LiveProvider → WinHTTP upload → SkyDrive/OneDrive
  │     └── FlickrProvider → Flickr API upload
  └── DRM detection: IsHResultDRMProtected()
```

### 5.2 Slideshow-Specific Pipeline

```
[Photo Source]
      │
      v
[Thumbnail Cache] ← ThumbCachePopulater (async)
      │
      v
[SlideshowList / ChunkIterator]
      │
      v
[Theme Selection] (9 built-in themes)
  ├── WLXPhotoCinematic (Ken Burns FX)
  │     └── D3DX9 camera → LookAtLH + PerspectiveFovLH
  └── WLXPipeline transitions (fade, spin, etc.)
      │
      v
[Rendering]
  ├── D3D9 sprite (D3DXCreateSprite) — primary path
  ├── GDI+ (GdipDrawImageRectRect) — software fallback
  └── GPU caps checked from registry (shader versions)
      │
      v
[Display] → SlideshowFrameWindow (WTL frameless)
  └── DWM integration (DwmSetWindowAttribute)
```

---

## 6. Telemetry Architecture

### 6.1 Three-Tier Telemetry System

```
                      ┌─────────────────────────┐
                      │  Custom HTTP Reporting   │
                      │  http://g.live.com       │
                      │  http://g.live-int.com   │
                      │  (internal/test)         │
                      └───────────┬─────────────┘
                                  │
                      Error URL template:
                      %s?version=%u.%u.%04u.%04u&ErrorCode=%x
                                  │
                    Used by: MovieMakerCore.dll
                    ─────────────────────────────────────
                    │                                   │
              ┌─────┴──────┐                    ┌───────┴──────┐
              │  SQM       │                    │  BICI        │
              │  WLXPhoto  │                    │  DmxBici.dll │
              │  Sqm.dll   │                    │              │
              └─────┬──────┘                    └───────┬──────┘
                    │                                   │
        Start, Increment,                     StartExperience,
        AddToStream (4 ovrls),                EndExperience,
        AddToStreamTimer (2 ovrls),           TransferExperienceToWeb,
        IsEnabled, Set, Shutdown              AddToStream, SetAnid,
                                              SetString,
                                              AddStringToDataPoint
                    │                                   │
                    └───────────┬───────────────────────┘
                                │
                    ┌───────────┴───────────┐
                    │  ETW (Event Tracing   │
                    │  for Windows)         │
                    │                      │
                    │  ADVAPI32.dll:        │
                    │  RegisterTraceGuidsW  │
                    │  UnregisterTraceGuids │
                    │  TraceEvent           │
                    │  TraceMessage         │
                    │  GetTraceEnableFlags  │
                    │  GetTraceEnableLevel  │
                    │  GetTraceLoggerHandle │
                    └──────────────────────┘
                                │
                    ┌───────────┴───────────┐
                    │  WER (Windows Error   │
                    │  Reporting)           │
                    │                      │
                    │  wer.dll:            │
                    │  WerReportCreate      │
                    │  WerReportAddDump     │
                    │  WerReportSetParameter│
                    │  WerReportSubmit      │
                    │  WerReportCloseHandle │
                    └──────────────────────┘
```

### 6.2 Telemetry by Component

| Component | SQM | BICI | ETW | WER | Custom HTTP |
|-----------|-----|------|-----|-----|-------------|
| WLXPhotoBase | ✔ ReportsForSqm | — | — | ✔ ReportsForWer | — |
| MovieMakerCore | ✔ (13 calls) | ✔ (5 calls) | ✔ | — | ✔ g.live.com |
| WLXSlideshow | ✔ (SqmStartupWrapper, AddToStream) | ✔ (AddStringToDataPoint, TransferExperienceToWeb) | ✔ | — | — |
| WLXFaceRecognition | ✔ (4 calls) | — | ✔ | — | — |
| WLXMediaPublishSubscribe | ✔ (11 calls) | ✔ (7 calls) | — | — | — |
| WLXMovieLibrary | — | — | ✔ (RegisterTraceGuids) | — | — |
| WLXVideoTrim | — | — | ✔ (RegisterTraceGuids) | — | — |
| WLXPipeline | — | — | ✔ (via ADVAPI32) | — | — |
| WLXPipetran | — | — | — | — | — |
| WLXPhotoCinematic | — | — | ✔ (RegisterTraceGuids, TraceEvent) | — | — |
| WLMFDS | — | — | ✔ | — | — |
| WLMFReadWrite | — | — | ✔ | — | — |
| WLXTranscode | — | — | ✔ | — | — |
| WLXCodecHost | — | — | — | — | — |

### 6.3 SQM Telemetry Points (from WLXPhotoSqm.dll exports)

```
Sqm::Startup()            → Initialize SQM session
Sqm::Shutdown()           → Flush and close
Sqm::AddToStream(DWORD, DWORD)          → Simple counter
Sqm::AddToStream(DWORD, PCWSTR)         → Named string metric
Sqm::AddToStream(DWORD, DWORD, DWORD)   → Tagged counter
Sqm::AddToStream(DWORD, DWORD, PCWSTR)  → Tagged string
Sqm::AddToStreamTimer(DWORD, DWORD, TUPLE)     → Timing event
Sqm::AddToStreamTimer(DWORD, DWORD, DWORD)     → Simple timer
Sqm::IsEnabled()          → Check if SQM is active
Sqm::Set(DWORD, DWORD)     → Set value
```

### 6.4 Error Reporting Data Fields (from WLXPhotoBase)

| Field | Description |
|-------|-------------|
| `AppName` | Application name |
| `AppVersion` | Application version |
| `AppTimeStamp` | App build timestamp |
| `ModName` | Module name (faulting DLL) |
| `ModVersion` | Module version |
| `ModTimeStamp` | Module timestamp |
| `Offset` | Code offset of fault |
| `HResultError` | Error code |

---

## 7. Threading Model

### 7.1 COM Apartment Model

| DLL | COM Model | CoInitialize Call |
|-----|-----------|-------------------|
| MovieMaker.exe | — (no COM init) | — |
| MovieMakerCore.dll | **Apartment** (main thread) | `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)` |
| WLXSlideshow | Apartment | Via AXCore |
| WLXPhotoCinematic | **Apartment** (per CLSID registration) | Via AXCore |
| WLXVideoTrim | Free/Apartment (per CLSID) | `CoInitialize` in filter graph |
| WLXFaceRecognition | Apartment (per RGS) | Via COM |
| WLXMediaPublishSubscribe | Apartment | Via AXCore |
| WLXMovieLibrary | Apartment (CComMultiThreadModel) | Via COM |
| WLXPipeline | Apartment | `CoInitialize` on pipeline thread |
| WLXPipetran | Apartment | Via COM |
| WLXTranscode.exe | Apartment | `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)` |
| WLXCodecHost.exe | Apartment | `CoInitialize` |

### 7.2 Thread Types

| Thread Type | Created By | Purpose |
|-------------|-----------|---------|
| **Main UI Thread** | MovieMakerCore.dll | Window message pump, DirectUI rendering, COM STA |
| **Worker Threads** | MovieMakerCore.dll (`CreateThread`) | Background tasks, timeline processing |
| **Thread Pool** | WLXMovieLibrary.dll | Vista+ `CreateThreadpool` / `CreateThreadpoolWork` for async operations |
| **MF Work Queues** | MFPlat.DLL / WLMFReadWrite | `MFPutWorkItem`, `MFPutWorkItemEx`, `MFAllocateWorkQueue` |
| **DirectShow Streaming Threads** | WLXVideoTrim.dll | Filter graph worker threads |
| **AV Playback Thread** | WLMFDS.dll | `AvSetMmThreadCharacteristics` ("Audio", "Capture" or "Playback") |
| **MMCSS Threads** | WLMFReadWrite.dll | `MFBeginRegisterWorkQueueWithMMCSS` for multimedia scheduling |
| **WaveOut Callback** | WLXMovieLibrary.dll | `waveOutOpen` callback thread for audio playback |
| **Timer Threads** | WLXPipeline / WLXMP4Parser | `timeSetEvent` for multimedia timers |

### 7.3 Synchronization Primitives

| Primitive | Used In |
|-----------|---------|
| `InitializeCriticalSectionAndSpinCount` | All DLLs (primary sync) |
| `InitializeCriticalSectionEx` | WLXPipetran (Win7+ new API) |
| `InterlockedIncrement/Decrement/Exchange/ExchangeAdd` | All DLLs (ref counts, atomics) |
| `InterlockedCompareExchange64` | WLMFDS (64-bit atomic) |
| `CreateEventW / SetEvent / ResetEvent / WaitForSingleObject` | All DLLs (thread signaling) |
| `CreateSemaphoreW / ReleaseSemaphore` | WLXPipeline, WLMFDS |
| `CreateThread / Sleep` | Various (explicit threading) |
| `InitializeConditionVariable` / `WakeConditionVariable` / `SleepConditionVariableCS` | WLXPipeline (Win7+ cond vars) |
| `MsgWaitForMultipleObjects` | WLXPipeline, WLXMP4Parser (message + wait) |
| `WaitForMultipleObjectsEx` | WLMFReadWrite (alertable wait) |
| `CreateThreadpoolWait` | WLMFDS (Vista+ TP) |
| `WaitForThreadpoolWaitCallbacks` | WLMFDS |

---

## 8. Registry Surface

### 8.1 Application Settings

| Key | Values | Used By |
|-----|--------|---------|
| `HKCU\Software\Microsoft\Windows Live\Movie Maker` | `Post`, `Suppressed`, `Recent`, `RecentWLVS` | MovieMakerCore |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery` | General settings | MovieMakerCore, WLXSlideshow |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\Slideshow` | `PhodeosPerChunk`, `PlaybackFramesPerSecond`, `UseVSync`, `RunInDXEMode`, `DXExclusiveSize`, `MaxRenderSizeWidth/Height`, `DesktopResolutionWidth/Height`, `SoftwareRenderingEnabled` | WLXSlideshow |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\Slideshow\LastRunSettings` | Last playback state | WLXSlideshow |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline` | Pipeline install location | WLXSlideshow |
| `HKCU\Software\Microsoft\Windows Live\Common\Movie Library` | Registry-based movie database | WLXMovieLibrary |
| `HKCU\Software\Microsoft\Windows Live\Common` | `SuiteLanguage`, `InstalledLanguages` | MovieMakerCore |
| `HKCU\Software\Microsoft\Windows Live\Installer` | `ShortCatalogTimeStamp`, `ProductStatus`, `TOUVersion` | MovieMakerCore |
| `HKCU\Software\Microsoft\Windows Live\Environment\PhotoGallery` | Environment settings | MovieMakerCore, WLXSlideshow |
| `HKCU\Software\Microsoft\Homer` | `Tracing` | MovieMakerCore |
| `HKCU\Software\Microsoft\MSRA\FaceRecognition` | `UseIterative`, model paths | WLXFaceRecognition |
| `HKCU\Software\Microsoft\VideoTrim\StreamBufferCopier Settings` | Stream buffer config | WLXVideoTrim |

### 8.2 File Support Configuration

| Key | Values | Used By |
|-----|--------|---------|
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs` | Codec download URLs | MovieMakerCore, WLXSlideshow |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs` | Raw codec URLs | MovieMakerCore, WLXSlideshow |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions` | Supported raw extensions | MovieMakerCore, WLXSlideshow |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions` | QT extensions | MovieMakerCore, WLXSlideshow |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes` | Disabled file types | MovieMakerCore, WLXSlideshow |
| `HKLM\Software\Microsoft\Windows Media Foundation\ByteStreamHandlers` | MF byte stream handlers | MovieMakerCore |

### 8.3 COM Registration

| Key | Used By |
|-----|---------|
| `HKCR\CLSID\{...}` | All COM DLLs via DllRegisterServer |
| `HKCR\AppID\{CE8B9537-...}` (MovieMakerCore APPID) | MovieMakerCore |
| `HKLM\Software\Microsoft\Windows Photo Gallery\Slideshow\Themes` | WLXPhotoCinematic |
| `HKCR\CLSID\{8095E7A5-...}\InprocServer32` | WLXVideoTrim |

### 8.4 Configuration Keys (from GPU Pipeline / Slideshow)

| Key | Type | Description |
|-----|------|-------------|
| `VertexShaderMajorVersion` / `VertexShaderMinorVersion` | DWORD | GPU vertex shader capabilities |
| `PixelShaderMajorVersion` / `PixelShaderMinorVersion` | DWORD | GPU pixel shader capabilities |
| `ActiveThemeFriendlyName` | SZ | Current slideshow theme name |
| `CurrentThemeSimpleSlideshowNone` | DWORD | "None" theme sentinel |
| `EnabledThemeSet` | DWORD | Whether theme set is enabled |
| `EnabledThemeName%d` | SZ | Per-index theme name |

### 8.5 Local App Data

| Path | Purpose |
|------|---------|
| `%LOCALAPPDATA%\Microsoft\Windows Live Movie Maker` | Project auto-save, user data |
| `%LOCALAPPDATA%\Microsoft\Windows Live Movie Maker\AutoSave.wlmp` | Auto-save project |
| `%LOCALAPPDATA%\Microsoft\Windows Live Movie Maker\AutoSaveValidate.wlmp` | Validation save |

---

## 9. File Format Support

### 9.1 Input Formats (Source → Decode)

| Format | Extension | Pipeline Path | Codec Support |
|--------|-----------|-------------|--------------|
| **WMV / ASF** | `.wmv`, `.asf` | MF / WMVCore / AsfCopier/Transcoder | WMV1/2/3, VC-1 (WVC1, MSS1/2) |
| **AVI** | `.avi` | DirectShow / AVICopier/Transcoder | Various via installed codecs |
| **DV-AVI** | `.avi` (DV) | DirectShow / DVTranscoder | DV25/50/SD/HD/SL |
| **MPEG-2** | `.mpg`, `.mpeg`, `.m2v`, `.mpv`, `.mpv2`, `.mp2v` | DShow / Mpg2Copier/Transcoder | MPEG-2 video |
| **DVR-MS** | `.dvr-ms`, `.sbe` | Stream Buffer / StreamBufferCopier | MPEG-2 + Stream Buffer |
| **MP4 / MOV / 3GP** | `.mp4`, `.mov`, `.m4v`, `.3gp`, `.3g2`, `.3gp2`, `.3gpp`, `.mqv` | WLXMP4Parser delay-load | H.264/MPEG-4 |
| **Windows Recorded TV** | `.wtv` | MF source | MPEG-2 / H.264 |
| **VOB** | `.vob` | DVD MPEG-2 | MPEG-2 |
| **MOD** | `.mod` | MOD format | MPEG-2 |
| **Images** | `.jpg`, `.png`, `.gif`, `.bmp`, `.tiff`, `.dib`, `.ico`, `.rle`, plus raw camera formats | GDI+/WIC | All WIC codecs |

### 9.2 Output Formats (Export → Sink)

| Format | Extension | Pipeline Path |
|--------|-----------|-------------|
| **WMV** | `.wmv` | MF Transcode / WMVCore / AsfTranscoder |
| **MP4** | `.mp4` | MF Transcode (H.264 + AAC) |
| **M4A** | `.m4a` | Audio-only MP4 |
| **WMA** | `.wma` | Audio-only WMA |

### 9.3 Project / Profile Formats

| Format | Extension | Description |
|--------|-----------|-------------|
| **Movie Maker Project** | `.wlmp` | XML project file (Sundance XML schema) |
| **Video Profile** | `.wlvs` | User encoding profile (XML) |
| **Exclusion List** | `ExclusionList.xml` | Library file exclusions |
| **Clipboard Format** | `WLPGMovieMaker_SelectedSet_16.4.3528.0331` | Internal clipboard transfer |
| **Clipboard Format** | `WLPGMovieMaker_SerializedProject_16.4.3528.0331` | Project copy/paste |

### 9.4 Media Foundation Format Constants (from WLXMovieLibrary)

**Video:**
- H.264 (`MFVideoFormat_H264`), MPEG-4 (`MFVideoFormat_MP4V`, `MP4S`, `M4S2`)
- WMV1/2/3, VC-1 (`WVC1`), MSS1/MSS2
- MPEG-1/2 (`MFVideoFormat_MPG1`, `MPG2`)
- DV25/50/SD/HD/SL
- NV12, IYUV, YUY2, UYVY, YV12, YVYU (YUV planar/packed)
- P010, P016, P210, P216 (10/16-bit YUV)
- ARGB32, RGB32/24/555/565/8
- v210, v216, v410, Y416 (high-end video)

**Audio:**
- PCM, Float, MP3, AAC, MPEG, DTS, Dolby AC3 SPDIF

---

## 10. Gaps Identified in Reconstruction

### 10.1 Critical Gaps

| Gap | Component | Impact | Details |
|-----|-----------|--------|---------|
| **ESENT (Jet) database** | MovieMakerCore.dll | Missing persistent storage for MRU, project index, thumbnail cache | 28 Jet functions imported, no ESENT usage in reconstructed source |
| **WLXPhotoSqm.dll stubs** | Telemetry | No SQM infrastructure in reconstruction | 13+ functions across multiple DLLs; needed for any telemetry |
| **DmxBici.dll stubs** | Telemetry | No BICI A/B testing | 5–7 functions across MovieMakerCore, WLXSlideshow, WLXMediaPublishSubscribe |
| **owlidcli.dll integration** | Auth/Identity | No Windows Live ID authentication | 7–18 ordinal imports in MovieMakerCore and MediaPublishSubscribe |
| **WLXPipeline.dll analysis** | Effects/Transitions | No analysis done — core rendering pipeline | D3D9+DDRAW+DMO pipeline, 300+ KB code |
| **WLXPipetran.dll analysis** | Transport/FX | No analysis done — pipeline transport | D3DX9 mesh/quaternion transforms, GDI+ text, 295 KB code |
| **WLXTranscode.exe analysis** | MF Export | No analysis done — separate transcode EXE | MF Transcode API, DXVA2, D3D9/11 |
| **WLXCodecHost.exe analysis** | Codec Isolation | No analysis done — out-of-process codec hosting | COM out-of-proc server, codec sandboxing |
| **WLMFDS.dll analysis** | MF↔DShow Bridge | No analysis done — critical bridge | 336 KB code, AVRT, stream buffer interop |
| **WLMFReadWrite.dll analysis** | MF R/W Helper | No analysis done — MFT, work queues | 219 KB code, EVR, work queue MMCSS |
| **WLXMP4Parser.dll analysis** | MP4 Parser | Only guessed exports (AddMP4SourceFilter, IsMP4FilePlayable) | 148 KB code, no detailed analysis |
| **WLXPhotoSqm.dll itself** | Telemetry DLL | Not analyzed or stubbed | Required by 5+ DLLs |
| **UXCore.dll** | UI Framework | No source reconstruction | 180+ imports in MovieMakerCore alone |
| **uxctl.dll** | Controls | No reconstruction | 3 imports in MovieMakerCore |

### 10.2 Secondary Gaps

| Gap | Description |
|-----|-------------|
| **COM GUID registry scripts** | No `.rgs` files reconstructed for any COM DLL |
| **Type libraries (.tlb)** | FaceRecognition has an embedded TLB; others may have runtime-only registration |
| **Resource sections (.rsrc)** | MovieMakerCore has 4.07 MB of resources (DirectUI layouts, icons, strings) — mostly undocumented |
| **WLMP project format** | XML schema only partially reverse-engineered |
| **WLVS profile format** | Video profile XML format not documented |
| **WLXPhotoLibraryDuiResourcesLocalized.dll** | Referenced resource DLL — not analyzed |
| **WLAVRes.dll** | AV resource DLL referenced by WLXVideoTrim, WLXMovieLibrary — not analyzed |
| **MovieMakerLang.dll** | Language resources — not analyzed |
| **MovieMakerPreviewClient.dll** | Preview client — not analyzed |
| **Published plugins** | WLFacebookPlugin, WLFlickrPlugin, WLVimeoPlugin, WLYouTubePlugin — not analyzed |
| **MetadataSys.dll** | Metadata property handler — not analyzed |
| **WLXPhotoSqm.dll** | SQM DLL itself — not analyzed |
| **DmxBici.dll** | BICI telemetry DLL — not analyzed |
| **DirectUI layout details** | The DirectUI element hierarchy is embedded in .rsrc and not documented |
| **Transition/effect XML schemas** | Embedded XML templates not fully extracted |
| **Clipboard format details** | Serialized binary formats not documented |
| **PanZoom theme XML structure** | Only partially extracted from WLXPhotoCinematic |
| **Build environment** | Build tree: `e:\bt\1105173\client\personalmedia\` — tools, scripts, project files unknown |

### 10.3 ELF/PDB Symbol Recovery Opportunities

| PDB File | GUID | Status |
|----------|------|--------|
| MovieMaker.pdb | `{47558454-9C62-4123-96E9-91A66E8F4D87}` | Not available |
| MovieMakerCore.pdb | `{D5217874-B614-477C-B45B-E0CE638C6496}` | Not available |
| WLXPhotoBase.pdb | `{0674DC61-4F42-4D44-AD50-155EB0251FA5}` | Not available |
| WLXVideoTrim.pdb | `{73CFF58F-A97D-4232-883C-397BD1DF5009}` | Not available |
| WLXSlideshow.pdb | `{6547A44F-AABE-4CFA-9675-0113F4BD19E9}` | Not available |
| WLXPhotoCinematic.pdb | `{0DB69AAE-1EE5-4822-95E2-F4B6520E7091}` | Not available |
| WLXMovieLibrary.pdb | `{CBF27DEC-DD3B-4A76-A5DF-9EECCC486627}` | Not available |
| WLXFaceRecognition.pdb | `{2FEBA5E0-6376-46CD-9ADA-4A79335C37BD}` | Not available |
| WLXMediaPublishSubscribe.pdb | `{17F284FA-930A-4DA2-9649-93B296009330}` | Not available |

### 10.4 Known Good: Correctly Reconstructed Components

| Component | Status |
|-----------|--------|
| MovieMaker.exe entry point (minimal WinMain + SetDllDirectoryW + MovieMakerMain) | ✅ Confirmed correct |
| MovieMakerCore.dll single export (MovieMakerMain, __cdecl) | ✅ Confirmed correct |
| WLXPhotoBase.dll core (Exception, Throw, New/Delete, string manager, OS detection) | ✅ Correct (but missing Version class, WER/SQM reporting) |
| WLXPhotoBase.dll export count (56 functions) | ✅ Confirmed |
| WLXSlideshow.dll COM DLL pattern (4 standard exports) | ✅ Confirmed |
| WLXVideoTrim.dll 5 factory exports (AVICopier, CopierFromMT, Transcoder, Player, WMVTranscoder) | ✅ Confirmed |
| WLXFaceRecognition.dll 7 COM objects, MSRA origin | ✅ Confirmed |
| D3D9 rendering pipeline for WLXSlideshow & WLXPhotoCinematic | ✅ Confirmed |
| WLXPhotoCinematic.dll Ken Burns D3DX9 camera matrix architecture | ✅ Confirmed |
| WLXMediaPublishSubscribe 22 exports, Live+Flickr providers | ✅ Confirmed |
| WLXMovieLibrary registry-based (not SQL), 1 export (CreateMovieFactory) | ✅ Confirmed |

---

## Appendix A: Cross-DLL Dependency Matrix

```
                 ┌─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
                 │  MM.exe  MMCore  PSPub  MvLib  SShow  Cinem  Trim  Pipe  Pptrn  MFDS  MFRW  MP4  Face  Codec  Tran  PBase │
├─────────────────┼─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│ KERNEL32        │  ●      ●      ●      ●      ●     ●     ●     ●     ●     ●     ●     ●    ●     ●     ●     ●   │
│ MSVCR110        │  ●      ●      ●      ●      ●     ●     ●     ●     ●     ●     ●     ●    ●     ●     ●     ●   │
│ USER32          │         ●      ●             ●     ●     ●     ●     ●     ●     ●     ●    ●           ●     ●    │
│ GDI32           │         ●      ●             ●            ●     ●     ●                          ●           ●    │
│ ADVAPI32        │         ●      ●      ●      ●     ●     ●     ●           ●     ●          ●           ●     ●    │
│ ole32           │         ●      ●      ●      ●     ●     ●     ●     ●     ●     ●     ●    ●     ●     ●     ●    │
│ OLEAUT32        │         ●      ●             ●     ●     ●     ●     ●     ●           ●    ●           ●     ●    │
│ shell32         │         ●      ●      ●      ●                   ●                                           ●    │
│ shlwapi         │         ●      ●             ●            ●            ●           ●     ●                     ●    │
│ WLXPhotoBase    │  ●      ●      ●      ●      ●     ●     ●                              ●     ●     ●    ●     ●   │
│ WLXPhotoSqm     │         ●      ●                    ●                         ●    ●                ●              │
│ DmxBici         │         ●      ●             ●                                                                     │
│ UXCore          │         ●      ●                    ●                                                               │
│ uxctl           │         ●                                                                                           │
│ wlidcli         │         ●      ●                                                                                   │
│ gdiplus         │         ●      ●             ●            ●     ●     ●               ●    ●           ●           │
│ d3d9            │         ●             ●      ●     ●     ●     ●     ●                                            │
│ d3d11           │         ●             ●                                                                 ●     ●    │
│ d2d1            │         ●                                                                                           │
│ DWrite          │         ●                                                                                           │
│ dxva2           │         ●             ●                                                                   ●     ●    │
│ dwmapi          │         ●                    ●                                                                      │
│ UxTheme         │         ●      ●                                                                                   │
│ D3DCOMPILER_46  │         ●                                                                                           │
│ WindowsCodecs   │         ●                                                                            ●              │
│ XmlLite         │         ●                                                                                           │
│ PROPSYS         │         ●      ●      ●                                                                   ●        │
│ OLEACC          │         ●      ●             ●                                                                      │
│ VERSION         │         ●      ●                                               ●           ●     ●          ●        │
│ WINMM           │         ●             ●                           ●                   ●     ●                       │
│ ESENT           │         ●                                                                                           │
│ MFPlat.DLL      │         ●             ●      ●                                ●     ●                              │
│ MF.dll          │         ●             ●                                ●     ●           ●               ●     ●    │
│ WMVCore         │                                    ●                                                               │
│ d3dx9_32        │                                           ●     ●     ●     ●                                      │
│ DDRAW           │                                                      ●                                              │
│ msdmo           │                                                      ●                                              │
│ COMCTL32        │                                                                  ●                                   │
│ WINHTTP         │                  ●                                                                                   │
│ WININET         │                  ●                                                                                   │
│ CRYPT32         │                  ●                                                                                   │
│ PSAPI           │                                                            ●               ●                       │
│ WTSAPI32        │                                         ●                                                           │
│ msi             │                  ●                    ●                                                             │
│ EVR             │                                                                      ●     ●                        │
│ AVRT            │                                                                      ●                              │
│ MetadataSys     │         ●                    ●                                                          ●          │
└─────────────────┴─────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

**Legend:** MM.exe=MovieMaker, MMCore=MovieMakerCore, PSPub=MediaPublishSubscribe, MvLib=MovieLibrary, SShow=Slideshow, Cinem=PhotoCinematic, Trim=VideoTrim, Pipe=WLXPipeline, Pptrn=WLXPipetran, MFDS=WLMFDS, MFRW=WLMFReadWrite, MP4=WLXMP4Parser, Face=FaceRecognition, Codec=WLXCodecHost, Tran=WLXTranscode, PBase=WLXPhotoBase

---

## Appendix B: Source File Inventory by DLL

| DLL | Estimated Source Files | Key Source Path |
|-----|----------------------|-----------------|
| MovieMakerCore | 200+ .cpp/.h (all app logic) | `personalmedia\moviemaker\dev\` |
| WLXVideoTrim | 25+ .cpp/.h | `personalmedia\videotrim\dev\` |
| WLXMovieLibrary | 40+ .cpp/.h | `personalmedia\movielibrary\dev\` |
| WLXMediaPublishSubscribe | 60+ .cpp/.h | `personalmedia\publish\dev\` |
| WLXSlideshow | 80+ .cpp/.h | `personalmedia\slideshow\dev\` |
| WLXPhotoCinematic | 15+ .cpp/.h | `personalmedia\cinematic\dev\` |
| WLXFaceRecognition | 60+ .cpp/.h (MSRA) | MSRA face recognition team |
| WLXPhotoBase | ~10 .cpp/.h | `personalmedia\photobase\dev\` |

---

*Synthesis generated from all available DLL analysis files, import tables, string extractions, RTTI inventories, COM registrations, and cross-DLL dependency analysis. WLXPipeline, WLXPipetran, WLXTranscode, WLXCodecHost, WLMFDS, WLMFReadWrite, and WLXMP4Parser remain the primary components requiring further analysis.*
