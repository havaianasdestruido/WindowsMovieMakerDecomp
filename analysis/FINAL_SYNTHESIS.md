# FINAL SYNTHESIS — Windows Live Movie Maker 2012 Architecture

**Product:** Windows Live Movie Maker 2012 (codename "Sundance")  
**Version:** 16.4.3528.0331_ship.client.main.w5m4  
**Build Date:** 2014-04-01  
**Compiler:** MSVC 11.0 (VS 2012), x86 (32-bit)  
**Target OS:** Windows 8+ (6.02)  
**Source Tree:** `e:\bt\1105173\client\personalmedia\`

---

## 1. Complete Binary Inventory

### 1.1 EXEs

| Binary | Size | Code | Key Exports | RTTI Classes | PDB GUID |
|--------|------|------|-------------|--------------|----------|
| MovieMaker.exe | 54 KB | 4.8 KB | None (thin launcher) | 0 | `{47558454-9C62-4123-96E9-91A66E8F4D87}` |
| WLXCodecHost.exe | ~32 KB | ~20 KB | 4 COM standard | 0 | — |
| WLXTranscode.exe | ~200 KB | ~120 KB | 4 COM standard + MFTranscode | ~15 | — |
| WLXVideoAcquireWizard.exe | — | — | 2 COM classes | — | — |
| WLXVideoCameraAutoPlayManager.exe | — | — | 1 COM class | — | `{9b5c8343-bdee-475d-9d3b-3715c6b8972e}` |
| WLXQuickTimeControlHost.exe | — | — | 2 COM classes | — | — |

### 1.2 DLLs — Core Application

| Binary | Size | Code | Key Exports | RTTI Classes | PDB GUID |
|--------|------|------|-------------|--------------|----------|
| MovieMakerCore.dll | 10.1 MB | 5.46 MB | 1 (`MovieMakerMain`) | 125+ | `{D5217874-B614-477C-B45B-E0CE638C6496}` |
| MovieMakerLang.dll | 216 KB | 0 (resource-only) | 0 | 0 | — |
| MovieMakerPreviewClient.dll | 28 KB | 5.9 KB | 4 COM standard | 0 | `{ABE6F6C4-02E6-47EA-B7FF-6A44A32264DD}` |

### 1.3 DLLs — Foundation & Utilities

| Binary | Size | Code | Key Exports | RTTI Classes | PDB GUID |
|--------|------|------|-------------|--------------|----------|
| WLXPhotoBase.dll | 56 KB | 21 KB | 56 (Base namespace) | 0 | `{0674DC61-4F42-4D44-AD50-155EB0251FA5}` |
| WLXMovieLibrary.dll | 324 KB | 281 KB | 1 (`CreateMovieFactory`) | 20+ | `{CBF27DEC-DD3B-4A76-A5DF-9EECCC486627}` |

### 1.4 DLLs — Media Foundation

| Binary | Size | Code | Key Exports | RTTI Classes | PDB GUID |
|--------|------|------|-------------|--------------|----------|
| WLMFDS.dll | 427 KB | 345 KB | 4 COM standard | 28+ | `{41188442-2579-4939-8E66-6697FE148922}` |
| WLMFReadWrite.dll | 252 KB | 217 KB | 7 (5 MF API + 2 COM) | 70+ | `{54E10C67-67A5-4F8A-8567-1D47C28B2189}` |
| WLXMP4Parser.dll | 184 KB | 147 KB | 9 (4 API + 4 COM + DllMain) | 50+ | `{A4577D2C-5400-4F17-91C0-06B5EA1BE4DE}` |

### 1.5 DLLs — Rendering & Effects

| Binary | Size | Code | Key Exports | RTTI Classes | PDB GUID |
|--------|------|------|-------------|--------------|----------|
| WLXSlideshow.dll | 520 KB | 255 KB | 4 COM standard | 156 | `{6547A44F-AABE-4CFA-9675-0113F4BD19E9}` |
| WLXPhotoCinematic.dll | 88 KB | 58 KB | 4 COM standard | 12+ | `{0DB69AAE-1EE5-4822-95E2-F4B6520E7091}` |
| WLXVideoTrim.dll | 568 KB | — | 5 (factory functions) | 57 | `{73CFF58F-A97D-4232-883C-397BD1DF5009}` |
| WLXPipeline.dll | 728 KB | 605 KB | 2 (`GetPipelineCreateFunctions` + `DllRegisterServer`) | 40+ | `{911D33AE-5E96-461A-B0D1-1ECCB116070F}` |
| WLXPipetran.dll | 737 KB | 299 KB | 1 (`GetTFXCreateFunctions`) | 80+ | `{89765767-4CF9-44E2-8351-95650D5D12B1}` |
| WLXGrinderScheduler.dll | — | — | 4 COM standard | — | — |
| WLXImageTranscode.dll | — | — | 3 COM classes | — | — |

### 1.6 DLLs — Publishing & Auth

| Binary | Size | Code | Key Exports | RTTI Classes | PDB GUID |
|--------|------|------|-------------|--------------|----------|
| WLXMediaPublishSubscribe.dll | 1.4 MB | 568 KB | 22 (4 COM + 18 app) | 40+ | `{17F284FA-930A-4DA2-9649-93B296009330}` |

### 1.7 DLLs — Photo Gallery Support

| Binary | Size | Code | Key Exports | RTTI Classes | PDB GUID |
|--------|------|------|-------------|--------------|----------|
| WLXFaceRecognition.dll | 4.5 MB | ~200 KB | 4 COM standard | 40+ | `{2FEBA5E0-6376-46CD-9ADA-4A79335C37BD}` |
| WLXPhotoAcq.dll | — | — | 5 COM classes | — | — |
| WLXPhotoClassic.dll | — | — | 5 COM classes | — | — |
| WLXPhotoVoyager.dll | — | — | 8 COM classes | — | — |
| WLXPhotoViewer.dll | — | — | — | — | — |
| WLXPhotoLibraryMain.dll | — | — | — | — | — |
| WLXPhotoLibraryDatabase.dll | — | — | — | — | — |
| WLXPhotoSqm.dll | — | — | SQM API | — | — |
| WLXQuickTimeShellExt.dll | — | — | 2 COM classes | — | — |
| WLXDSPA.dll | — | — | 6+ COM classes | — | — |
| WLXSendMail.dll | — | — | — | — | — |

### 1.8 Shared DLLs (Photo Gallery infrastructure)

| Binary | Purpose |
|--------|---------|
| SharedMFDlls (RSCMFT + MPG4DEMUX) | Rolling shutter correction + MPEG-4 demux MFTs |
| Imaging.dll | WIC image effects engine (~65 CLSIDs) |
| PublishPluginsInterop.dll | .NET interop wrapper for publish plugins |
| SubscribePluginsInterop.dll | .NET interop wrapper for subscribe plugins |
| PhotoViewerShim.dll | Shell viewer/editor extension shim |
| AlbumDownloadProtocolHandler.dll | Album download protocol handler |
| WLAVRes.dll | AV resource DLL (8 COM classes, threading=both) |

### 1.9 .NET Publish Plugins

| Binary | Purpose |
|--------|---------|
| WLFacebookPlugin.dll | Facebook upload |
| WLFlickrPlugin.dll | Flickr upload |
| WLVimeoPlugin.dll | Vimeo upload |
| WLYouTubePlugin.dll | YouTube upload |

---

## 2. Corrected Dependency Graph

### 2.1 Load-Time Dependencies

```
MovieMaker.exe
  ├── KERNEL32.dll (always)
  ├── MSVCR110.dll (CRT)
  ├── WLXPhotoBase.dll (direct: BasePrivate::Delete)
  └── [delay] MovieMakerCore.dll
        ├── WLXPhotoBase.dll (14 imports)
        ├── WLXPhotoSqm.dll (13 imports)
        ├── DmxBici.dll (5 imports)
        ├── UXCore.dll (180+ imports: DirectUI)
        ├── wlidcli.dll (7 ordinals: Live ID)
        ├── uxctl.dll (3 imports)
        ├── MetadataSys.dll (1 import)
        ├── MFPlat.DLL (18 imports)
        ├── MF.dll (2 imports)
        ├── ESENT.dll (28 Jet functions)
        ├── gdiplus.dll (24 imports)
        ├── d3d11.dll (1: D3D11CreateDevice)
        ├── d3d9.dll (2: Direct3DCreate9/Ex)
        ├── d2d1.dll (1 ordinal)
        ├── DWrite.dll (1: DWriteCreateFactory)
        ├── dxva2.dll (2 imports)
        ├── dwmapi.dll (1 import)
        ├── UxTheme.dll (3 imports)
        ├── D3DCOMPILER_46.dll (2 imports)
        ├── WindowsCodecs.dll (1 import)
        ├── XmlLite.dll (4 imports)
        ├── PROPSYS.dll (8 imports)
        ├── OLEACC.dll (2 imports)
        ├── VERSION.dll (3 imports)
        ├── WINMM.dll (1 import)
        ├── [on-demand] WLXMovieLibrary.dll
        ├── [on-demand] WLXVideoTrim.dll → [delay] WLXMP4Parser.dll
        ├── [on-demand] WLXPipeline.dll (no WLXPhotoBase)
        ├── [on-demand] WLXPipetran.dll (no WLXPhotoBase)
        ├── [on-demand] WLXSlideshow.dll → [delay] WLXPhotoBase, WLXPhotoSqm
        ├── [on-demand] WLXPhotoCinematic.dll → [delay] WLXPhotoBase
        ├── [on-demand] WLXMediaPublishSubscribe.dll → [delay] WLXPhotoBase, MetadataSys
        └── [on-demand] WLXFaceRecognition.dll
```

### 2.2 Inter-DLL Dependencies (verified via import tables)

| DLL | Depends On (WLX) |
|-----|------------------|
| MovieMaker.exe | WLXPhotoBase |
| MovieMakerCore.dll | WLXPhotoBase, WLXPhotoSqm, DmxBici, UXCore |
| WLXMovieLibrary.dll | WLXPhotoBase (delay) |
| WLXSlideshow.dll | WLXPhotoBase (delay), WLXPhotoSqm (delay), DmxBici |
| WLXVideoTrim.dll | WLXPhotoBase, WLXMP4Parser (delay) |
| WLXPhotoCinematic.dll | WLXPhotoBase (delay) |
| WLXMediaPublishSubscribe.dll | WLXPhotoBase (delay), WLXPhotoSqm, DmxBici, MetadataSys (delay) |
| WLXFaceRecognition.dll | WLXPhotoBase, WLXPhotoSqm |
| WLXPipeline.dll | **None** (standalone, no WLX dependencies) |
| WLXPipetran.dll | **None** (standalone, no WLX dependencies) |
| WLMFDS.dll | **None** (shared infrastructure) |
| WLMFReadWrite.dll | **None** (shared infrastructure) |
| WLXMP4Parser.dll | **None** (standalone) |
| WLXCodecHost.exe | WLXPhotoBase |
| WLXTranscode.exe | WLXPhotoBase |

**Key finding:** WLXPipeline.dll and WLXPipetran.dll are completely standalone with zero WLXPhotoBase dependency — they use only system APIs and D3D9/GDI+.

---

## 3. COM Object Map (Verified)

### 3.1 Core COM Objects

| Binary | CLSID | Name | Threading | Source |
|--------|-------|------|-----------|--------|
| **WLXSlideshow** | `{84FBA192-4F8D-4a5d-94DE-083446ACC1D0}` | CLSID_SimpleSlideshowDisplay | — | .rgs |
| **WLXSlideshow** | — | CLSID_SlideshowExtension | — | RTTI |
| **WLXSlideshow** | — | CLSID_TimelineDisplay | — | RTTI |
| **WLXPhotoCinematic** | `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}` | CinematicFullScreen1.1 | Apartment | .rgs |
| **WLXPhotoCinematic** | `{5409AB48-D8D3-40e6-A1EB-23489DC422DE}` | CinematicTransform.1 | Apartment | .rgs |
| **WLXVideoTrim** | `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | CLSID_VideoTrim | — | Cross-DLL shared |
| **WLXFaceRecognition** | `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | CLSID_FaceRecognitionPipeline | Apartment | .rgs ✅ |
| **WLXFaceRecognition** | `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | CLSID_FaceDetection | Apartment | .rgs ✅ |
| **WLXFaceRecognition** | `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` | CLSID_ImageManager | Apartment | .rgs ✅ |
| **MovieMakerCore** | `{CE8B9537-708C-4784-9DD4-127B635DD348}` | APPID | — | Binary |

### 3.2 Publishing COM Objects (WLXMediaPublishSubscribe.dll)

| CLSID | Name | Threading |
|-------|------|-----------|
| `{5F4019FA-375D-4D74-9922-EFCD9D920A0D}` | LiveProvider | Apartment |
| `{197608E2-D42D-43a5-927E-1C67FC041431}` | FlickrProvider | Apartment |
| `{43DBAB44-8EDC-4FB0-B305-F117ECB15305}` | ProviderManager | Apartment |
| `{8ab5eea6-7a20-417d-b6d1-d40c9804b2cc}` | OnlineMediaPluginManager | Apartment |
| `{54c41e30-bfd8-44e5-91a2-ce038e242817}` | PluginDecorator | Apartment |
| `{66A5A6CA-0D84-44cb-BEC4-04B3B888E4B2}` | PluginManagerHelper | Apartment |
| `{C9C25561-A401-43b4-BC22-79FBA2A00042}` | InternetCacheManager | Apartment |
| `{CE5E100A-6CE8-4d67-8870-245DFA3499AB}` | PublishPluginHelper | Apartment |
| `{DA69067E-3959-47ca-A58D-2300786168CD}` | MetadataSettingsController | Apartment |

### 3.3 Photo Gallery Theme COM Objects

| Binary | CLSID | Name |
|--------|-------|------|
| **WLXPhotoClassic** | `{773AFF18-2083-47C1-9EA9-A5DA346A0122}` | Classic Theme |
| **WLXPhotoClassic** | `{854E43AC-E1FD-46f2-8DD3-EE4C7A1844B6}` | Classic Transform |
| **WLXPhotoClassic** | `{B9087BDF-F0F8-4454-A7D1-F6242E1654F8}` | Black & White Theme |
| **WLXPhotoClassic** | `{F91A0A3F-3E4E-4273-88CC-6664834ACA6F}` | Sepia Theme |
| **WLXPhotoClassic** | `{71ED30A7-499A-4F61-84F8-10CDEC657FE0}` | Basic Theme |
| **WLXPhotoVoyager** | `{C84CFE1B-89DC-40e7-83BF-CB821255F9EC}` | Album Theme |
| **WLXPhotoVoyager** | `{AEE6C573-A192-4af3-B62B-A4E6848533D3}` | Collage Theme |
| **WLXPhotoVoyager** | `{653E52D8-D033-469a-8BB5-9C1A164416D5}` | Flip Theme |
| **WLXPhotoVoyager** | `{B4E10BE6-A2CE-4bef-9D80-99995CB3C162}` | Frame Theme |
| **WLXPhotoVoyager** | `{5515D2B5-6825-409e-B377-544708C9DD06}` | Glass Theme |
| **WLXPhotoVoyager** | `{D5561752-E5A7-46e7-B768-D945E144CA78}` | Snapshots Theme |
| **WLXPhotoVoyager** | `{CC4F1166-CE12-41f7-85E2-AE4744D9381B}` | Travel Theme |
| **WLXPhotoVoyager** | `{E48325CB-1EFC-425e-9CD9-47EF51BECD55}` | Voyager Transform |

### 3.4 Infrastructure COM Objects

| Binary | CLSID | Name |
|--------|-------|------|
| **WLXGrinderScheduler** | `{121244C5-B0AD-45fb-9D8E-B893B917D053}` | GrinderScheduler |
| **WLXImageTranscode** | `{20575516-78AF-4404-B3C7-51D05F9945B5}` | ImageTranscode |
| **WLXImageTranscode** | `{E253845C-A1A2-4ae4-B784-5E8140CD5B2C}` | ImageLoader |
| **WLXImageTranscode** | `{B8A2E14E-290D-4122-B092-1A7D86198CCE}` | WLXOutofProc |
| **WLXCodecHost** | `{E30A45E6-1916-4659-95EE-035E62DB9AB0}` | Codec Host |
| **WLXQuickTimeControlHost** | `{B9AD19CB-FA75-4B29-B4A4-86C7E9616390}` | QuickTimePlayerHost |
| **WLXQuickTimeControlHost** | `{AE3A66BB-85FE-49B8-BF7B-4DB4E0005091}` | QuickTimeMovieThumbnail |
| **WLXVideoAcquireWizard** | `{5abe6468-4a2a-403c-892d-06e1fc31097f}` | MSLive Capture Wizard |
| **WLXVideoAcquireWizard** | `{5ab23fca-6040-4012-8fea-8da67f5806a7}` | MSLive Auto Capture |
| **WLXVideoCameraAutoPlayManager** | `{9b5c97f6-b3a5-4a6d-8b03-993ec7291a22}` | WLXHWEventHandler |
| **WLXQuickTimeShellExt** | `{007EFBDF-8A5D-4930-97CC-A4B437CBA777}` | MovieThumbnail |
| **WLXDSPA** | `{BC8CA1B3-B013-4866-9621-825957DF23F3}` | CWLXTocGeneratorDmo |
| **WLXDSPA** | `{09BC59C2-70DD-45f9-A5B7-DE9F2A5CA34B}` | CWLXThumbnailGeneratorDmo |
| **WLXDSPA** | `{15CD2459-C14B-457b-B57B-3DBA111B9D09}` | CClusterDetectorEx |
| **WLXDSPA** | `{75704D6C-09BA-4d19-AFEA-5F21FC08B3DB}` | CTocParser |
| **WLXDSPA** | `{BF620143-7420-460a-9EEE-178B78D4939D}` | CAsfTocParser |
| **WLXDSPA** | `{9FAE79C9-BA02-43d9-9382-C7BEF740A596}` | CAviTocParser |
| **WLXPhotoAcq** | `{4E13844B-6012-40cb-8E3C-0F7EB9AEF6DD}` | LivePhotoAcquire |
| **WLXPhotoAcq** | `{94B1CBEA-F33F-4508-8AAA-55FD8113EFCB}` | LivePhotoAcquireOptionsDialog |
| **WLXPhotoAcq** | `{0D5A7D0E-9A06-4e17-85D9-A0B24036371D}` | LivePhotoPickerDialog |
| **WLXPhotoAcq** | `{E84D0D46-3D57-4039-9EFE-310AF1CAF92A}` | LivePhotoAcqDeviceSelectionDlg |
| **WLXPhotoAcq** | `{4D8A134F-3D0A-4375-8B1A-78CD171C9318}` | LivePhotoAcquisitionWizard |

### 3.5 Cross-DLL Shared CLSID

| CLSID | Defined In | Referenced In |
|-------|-----------|---------------|
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | WLXVideoTrim (defines) | WLXPipeline, WLXMediaPublishSubscribe |

### 3.6 TypeLib GUIDs

| TypeLib | DLL | CLSID Count |
|---------|-----|-------------|
| `{AEE505D4-908A-4BFC-9A7E-31AF955C15BF}` | Imaging.dll | ~65 |
| `{7B996FF4-FFF2-4573-9728-C1A612BD8592}` | WLXMediaPublishSubscribe.dll | 8 |
| `{D85885D4-A18A-4fac-AEE1-BE9AAB0F4485}` | WLXPhotoAcq.dll | 5 |
| `{17DC7884-443D-478e-ABD7-BC22856FC7F1}` | WLXImageTranscode.dll | 3 |
| `{C6D340BB-0CEA-4923-8082-51036E472379}` | WLXQuickTimeControlHost.exe | 2 |
| `{5ab7792c-0f76-4003-aa47-5f075165d4de}` | WLXVideoAcquireWizard.exe | 2 |
| `{9b5c8343-bdee-475d-9d3b-3715c6b8972e}` | WLXVideoCameraAutoPlayManager.exe | 1 |
| `{77A16B7E-9DFE-410c-8863-264BF3E91103}` | WLXGrinderScheduler.dll | 1 |
| `{60E1FA84-4F2F-417C-AEE4-7681A960D09E}` | AlbumDownloadProtocolHandler.dll | 1 |

---

## 4. Registry Key Ownership

### 4.1 MovieMakerCore.dll — Primary Owner

```
Software\Microsoft\Windows Live\Movie Maker
Software\Microsoft\Windows Live\Movie Maker\Post
Software\Microsoft\Windows Live\Movie Maker\Recent
Software\Microsoft\Windows Live\Movie Maker\RecentWLVS
Software\Microsoft\Windows Live\Movie Maker\Suppressed
Software\Microsoft\Windows Live\Photo Gallery
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions
Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes
Software\Microsoft\Windows Live\Common
SOFTWARE\Microsoft\Windows Live\Common\Movie Library
Software\Microsoft\Windows Live\Environment\PhotoGallery
Software\Microsoft\Windows Live\Installer
Software\Microsoft\Windows Live\Installer\ProductStatus
Software\Microsoft\Windows Media Foundation\ByteStreamHandlers
Software\Microsoft\Homer
Microsoft\Live\MovieMaker
Microsoft\Windows Live\Video Profiles
```

### 4.2 WLXSlideshow.dll — Slideshow Settings

```
Software\Microsoft\Windows Live\Photo Gallery\Slideshow
Software\Microsoft\Windows Live\Photo Gallery\Slideshow\LastRunSettings
Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline
```

### 4.3 WLXVideoTrim.dll — Trimming Settings

```
SOFTWARE\Microsoft\VideoTrim\StreamBufferCopier Settings
Software\Microsoft\Windows Live\Photo Gallery\VideoTrim
```

### 4.4 WLXFaceRecognition.dll — Face Recognition

```
Software\Microsoft\MSRA\FaceRecognition
Software\Microsoft\MSRA\FaceRecognition\Iterative
```

### 4.5 WLXMediaPublishSubscribe.dll — Publishing

```
Software\Microsoft\Windows Live\Photo Gallery\PublishPlugins
Software\Microsoft\Windows Live\Photo Gallery\SubscribePlugins
Software\Microsoft\Windows Live\Photo Gallery\FlickrAccounts
Software\Microsoft\Windows Live\Photo Gallery\PubSubProviders
Microsoft\WindowsLive\PublishPlugins\PersistentData
```

### 4.6 GPU Pipeline (shared across 5 DLLs)

```
Software\Microsoft\GPUPipeline                          (WLXPipeline, wlxclip)
Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline  (WLXPipeline, WLXPipetran, WLXPhotoCinematic, WLXPhotoClassic, WLXPhotoVoyager, WLXSlideshow)
```

### 4.7 Telemetry (WLXPhotoSqm.dll)

```
Microsoft\Windows Live Client Album Viewer\SqmApi
Microsoft\Windows Live Movie Maker\SqmApi
Microsoft\Windows Live Photo Gallery\SqmApi
Microsoft\Windows Live\SqmApi
Software\Microsoft\Windows Live\Ship Asserts
Software\Microsoft\Windows Live\Ship Asserts\Response Table
Software\Microsoft\Windows Live\Movie Maker\DebugSQM
Software\Microsoft\Windows Live\Photo Gallery\Slideshow\DebugSQM
Software\Microsoft\Windows Live\Photo Gallery\Library\DebugSQM
Software\Microsoft\Windows Live\Client Album Viewer\DebugSQM
```

### 4.8 Shared Configuration (read by 8+ DLLs)

```
Software\Microsoft\Windows Live\Photo Gallery            (central hub)
Software\Microsoft\Windows Live\Common                   (suite-wide)
Software\Microsoft\SQMClient                              (SQM infrastructure)
Software\Microsoft\IdentityCRL                           (auth/identity)
```

---

## 5. File Format Support Matrix

### 5.1 Input Formats

| Format | Extensions | Pipeline | Key DLLs |
|--------|-----------|----------|----------|
| **WMV/ASF** | `.wmv`, `.asf` | MF/WMVCore/AsfCopier | WLXVideoTrim, MovieMakerCore |
| **AVI** | `.avi` | DirectShow/AVICopier | WLXVideoTrim, WLXSlideshow |
| **DV-AVI** | `.avi` (DV) | DirectShow/DVTranscoder | WLXVideoTrim |
| **MPEG-2** | `.mpg`, `.mpeg`, `.m2v` | DShow/Mpg2Copier | WLXVideoTrim |
| **DVR-MS** | `.dvr-ms`, `.sbe` | StreamBuffer | WLXVideoTrim |
| **MP4/MOV/3GP** | `.mp4`, `.mov`, `.m4v`, `.3gp`, `.3g2`, `.3gp2`, `.3gpp`, `.mqv` | WLXMP4Parser (delay) | WLXVideoTrim, MovieMakerCore |
| **Windows Recorded TV** | `.wtv` | MF source | MovieMakerCore |
| **VOB** | `.vob` | DVD MPEG-2 | WLXVideoTrim |
| **MOD** | `.mod` | MOD format | WLXVideoTrim |
| **QuickTime** | `.mov`, `.qt` | WLXQuickTimeControlHost | MovieMakerCore |
| **Images** | `.jpg`, `.png`, `.gif`, `.bmp`, `.tiff`, `.dib`, `.ico`, `.rle` | GDI+/WIC | All components |
| **Raw Camera** | `.cr2`, `.nef`, `.dng`, `.arw`, etc. | WLXImageTranscode | MovieMakerCore |

### 5.2 Output Formats

| Format | Extension | Pipeline | Key DLLs |
|--------|-----------|----------|----------|
| **WMV** | `.wmv` | MF Transcode/WMVCore | WLXTranscode, WLXVideoTrim |
| **MP4** | `.mp4` | MF Transcode (H.264+AAC) | WLXTranscode, WLXVideoTrim |
| **M4A** | `.m4a` | Audio-only MP4 | WLXTranscode |
| **WMA** | `.wma` | Audio-only WMA | WLXTranscode |

### 5.3 Project/Profile Formats

| Format | Extension | Description |
|--------|-----------|-------------|
| **Movie Maker Project** | `.wlmp` | XML project file |
| **Video Profile** | `.wlvs` | User encoding profile (XML) |
| **Template** | `.wlmx` | Transition/effect templates (191 files) |
| **Script** | `.wlms` | Script files (10 files) |
| **Exclusion List** | `ExclusionList.xml` | Library file exclusions |
| **Clipboard** | `WLPGMovieMaker_SelectedSet_16.4.3528.0331` | Internal clipboard |
| **Clipboard** | `WLPGMovieMaker_SerializedProject_16.4.3528.0331` | Project copy/paste |

### 5.4 MP4 Atom/Codec Support (WLXMP4Parser)

**Container atoms:** `moov`, `trak`, `mdia`, `minf`, `stbl`, `dinf`, `edts`, `udta`, `meta`, `ilst`, `mvex`, `moof`, `traf`, `sinf`, `schi`

**Video codecs:** AVC/H.264 (`avc1`, `avc2`), MPEG-4 (`mp4v`), H.263 (`s263`), VC-1 (`vc-1`), WMV3, VP8 (`vp80`), VP9 (`vp90`), AV1 (`av01`), Motion JPEG (`jpeg`, `mjp2`), PNG

**Audio codecs:** AAC (`mp4a`), PCM (`raw `, `twos`, `sowt`), IMA ADPCM (`ima4`), MACE 3/6, QDesign Music 2 (`QDM2`), Qualcomm PureVoice (`Qclp`), Dolby AC3 (`ac-3`), Dolby AC3+ (`ec-3`), DTS, Apple Lossless (`alac`), Opus, FLAC (`fLaC`), WMA Pro (`wmap`)

**DRM:** PlayReady/Widevine (`pssh`), track encryption (`tenc`, `sinf`, `frma`, `schm`, `schi`)

**Metadata:** Apple/iTunes (`ilst`), Windows Media (`CMSMetadata*`)

---

## 6. Thread Model Summary

### 6.1 COM Apartment Model

| DLL | COM Model | Notes |
|-----|-----------|-------|
| MovieMakerCore.dll | **Apartment** (main thread) | `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)` |
| WLXSlideshow | Apartment | Via AXCore |
| WLXPhotoCinematic | **Apartment** | Per CLSID registration |
| WLXVideoTrim | Free/Apartment | Per CLSID |
| WLXFaceRecognition | Apartment | Per RGS |
| WLXMediaPublishSubscribe | Apartment | Via AXCore |
| WLXMovieLibrary | Apartment | `CComMultiThreadModel` |
| WLXPipeline | Apartment | `CoInitialize` on pipeline thread |
| WLXPipetran | Apartment | Via COM |
| WLXTranscode.exe | Apartment | `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)` |
| WLXCodecHost.exe | Apartment | `CoInitialize` |

### 6.2 Thread Types

| Thread Type | Created By | Purpose |
|-------------|-----------|---------|
| Main UI Thread | MovieMakerCore | Window message pump, DirectUI rendering |
| Worker Threads | MovieMakerCore (`CreateThread`) | Background tasks, timeline processing |
| Thread Pool | WLXMovieLibrary | Vista+ `CreateThreadpool`/`CreateThreadpoolWork` |
| MF Work Queues | MFPlat/WLMFReadWrite | `MFPutWorkItem`, `MFPutWorkItemEx` |
| DirectShow Streaming | WLXVideoTrim | Filter graph worker threads |
| AV Playback | WLMFDS | `AvSetMmThreadCharacteristics` (MMCSS) |
| MMCSS Threads | WLMFReadWrite | `MFBeginRegisterWorkQueueWithMMCSS` |
| WaveOut Callback | WLXMovieLibrary | `waveOutOpen` callback thread |
| Timer Threads | WLXPipeline/WLXMP4Parser | `timeSetEvent` multimedia timers |

### 6.3 Synchronization Primitives

| Primitive | Used In |
|-----------|---------|
| `InitializeCriticalSectionAndSpinCount` | All DLLs |
| `InitializeCriticalSectionEx` | WLXPipetran |
| `InterlockedIncrement/Decrement/Exchange` | All DLLs |
| `InterlockedCompareExchange64` | WLMFDS |
| `CreateEventW/SetEvent/WaitForSingleObject` | All DLLs |
| `CreateSemaphoreW/ReleaseSemaphore` | WLXPipeline, WLMFDS |
| `InitializeConditionVariable/WakeConditionVariable` | WLXPipeline |
| `MsgWaitForMultipleObjects` | WLXPipeline, WLXMP4Parser |
| `WaitForMultipleObjectsEx` | WLMFReadWrite |
| `CreateThreadpoolWait` | WLMFDS |

---

## 7. Architecture Gaps

### 7.1 Primary Gaps

| Gap | Component | Impact | Status |
|-----|-----------|--------|--------|
| **ESENT (Jet) database** | MovieMakerCore | Missing persistent storage for MRU, project index, thumbnail cache | 28 Jet functions imported, no reconstruction |
| **WLXPhotoSqm.dll** | Telemetry | No SQM infrastructure | 13+ functions across multiple DLLs |
| **DmxBici.dll** | Telemetry | No BICI A/B testing | 5–7 functions across Core, Slideshow, MediaPublish |
| **wlidcli.dll** | Auth/Identity | No Windows Live ID authentication | 7–18 ordinals in Core and MediaPublish |
| **UXCore.dll** | UI Framework | No source reconstruction | 180+ imports in MovieMakerCore |
| **uxctl.dll** | Controls | No reconstruction | 3 imports in MovieMakerCore |
| **MetadataSys.dll** | Metadata | Not analyzed | Property handler for Core and MediaPublish |

### 7.2 Secondary Gaps

| Gap | Status |
|-----|--------|
| WLMP project XML schema (partial) | Partial |
| WLVS profile format | Not started |
| DirectUI layout hierarchy | Not started |
| Transition/effect XML schemas | Partial |
| Clipboard format details | Not started |
| Build environment/tools | Not started |

### 7.3 PDB Symbol Recovery Opportunities

| PDB | GUID | Status |
|-----|------|--------|
| MovieMaker.pdb | `{47558454-9C62-4123-96E9-91A66E8F4D87}` | Not available |
| MovieMakerCore.pdb | `{D5217874-B614-477C-B45B-E0CE638C6496}` | Not available |
| WLXPhotoBase.pdb | `{0674DC61-4F42-4D44-AD50-155EB0251FA5}` | Not available |
| WLXVideoTrim.pdb | `{73CFF58F-A97D-4232-883C-397BD1DF5009}` | Not available |
| WLXSlideshow.pdb | `{6547A44F-AABE-4CFA-9675-0113F4BD19E9}` | Not available |
| WLXPhotoCinematic.pdb | `{0DB69AAE-1EE5-4822-95E2-F4B6520E7091}` | Not available |
| WLXMovieLibrary.pdb | `{CBF27DEC-DD3B-4A76-A5DF-9EECCC486627}` | Not available |
| WLXFaceRecognition.pdb | `{2FEBA5E0-6376-46CD-9ADA-4A79335C37BD}` | Not available |
| WLXMediaPublishSubscribe.pdb | `{17F284FA-930A-4DA2-9649-93B296009330}` | Not available |
| WLMFDS.pdb | `{41188442-2579-4939-8E66-6697FE148922}` | Not available |
| WLMFReadWrite.pdb | `{54E10C67-67A5-4F8A-8567-1D47C28B2189}` | Not available |
| WLXMP4Parser.pdb | `{A4577D2C-5400-4F17-91C0-06B5EA1BE4DE}` | Not available |
| WLXPipeline.pdb | `{911D33AE-5E96-461A-B0D1-1ECCB116070F}` | Not available |
| WLXPipeTran.pdb | `{89765767-4CF9-44E2-8351-95650D5D12B1}` | Not available |
| MovieMakerPreviewClient.pdb | `{ABE6F6C4-02E6-47EA-B7FF-6A44A32264DD}` | Not available |

---

## 8. Key Corrections Applied

### 8.1 CLSID Corrections

| GUID | Previous Label | Corrected Label | Source |
|------|---------------|-----------------|--------|
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | ~~CLSID_FaceRegion/FaceRegionSet~~ | **CLSID_FaceRecognitionPipeline** | .rgs confirmed |
| `{EF401225-1260-4716-A842-7D180DC14C1E}` | ~~CLSID_FaceRecognitionPipeline~~ | **TypeLib IID** | .rgs confirmed |
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | ~~`{CC1A9149-0E73-4EBB-93D3-F52A83D36935}`~~ | **CLSID_FaceDetection** | .rgs confirmed |

### 8.2 Export Corrections

| DLL | Previous Claim | Verified | Source |
|-----|---------------|----------|--------|
| WLXPipeline | 1 export (GetPipelineCreateFunctions) | **2 exports** (GetPipelineCreateFunctions + DllRegisterServer) | dumpbin /EXPORTS |
| WLXPipetran | — | **1 export** (GetTFXCreateFunctions) | dumpbin /EXPORTS |
| WLMFReadWrite | 5 exports | **7 exports** (5 MF API + 2 COM) | dumpbin /EXPORTS |
| WLXMP4Parser | — | **9 exports** (4 API + 4 COM + DllMain) | dumpbin /EXPORTS |

### 8.3 Architecture Corrections

| Area | Previous | Corrected |
|------|----------|-----------|
| WLXPipeline | Assumed ATL COM | **Not ATL COM** — custom factory pattern (`GetPipelineCreateFunctions`) |
| WLXPipetran | — | **Standalone** — no WLXPhotoBase dependency |
| WLMFDS COM CLSID | Unknown | **CLSID_DShowSourceResolver** (via RTTI `CComCoClass` template param) |
| WLXMovieLibrary registry | Assumed SQL/ESENT | **Registry-based** — `SOFTWARE\Microsoft\Windows Live\Common\Movie Library` |
| WLXPhotoBase exports | Claimed File/Thread/FindFile classes | **None exist** — only exception, version, error reporting, memory, string, OS detection |

---

*Synthesis generated from 51 analysis files across all DLL/EXE binaries. Last updated 2026-07-27.*
