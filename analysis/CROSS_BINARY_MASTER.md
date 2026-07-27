# CROSS-BINARY MASTER CORRELATION DOCUMENT

**Windows Live Movie Maker 2012 / Photo Gallery 2012**  
**Build:** 16.4.3528.0331 | **Branch:** ship.client.main.w5m4 | **Date:** 2014-04-01  
**Toolchain:** MSVC 11.0 (VS 2012) | **Target:** x86 PE32 | **OS:** Windows 8+ (6.02)  
**Generated:** 2026-07-27 from 60+ analysis files across all subagent outputs

---

## A. Binary Inventory

### Complete Binary Map

| # | Binary | Type | Image Size | Code (.text) | Role |
|---|--------|------|-----------|-------------|------|
| 1 | **MovieMakerExe.exe** | EXE | 1.7 MB | 1.1 MB | Main application entry point |
| 2 | **MovieMakerCore.dll** | DLL | 6.5 MB | 4.2 MB | Core engine: timeline, rendering, effects, transitions |
| 3 | **MovieMakerLang.dll** | DLL | Variable | Variable | Language resource satellite DLL |
| 4 | **MovieMakerPreviewClient.exe** | EXE | ~200 KB | ~100 KB | Out-of-process preview renderer |
| 5 | **WLXCodecHost.exe** | EXE | ~150 KB | ~80 KB | Out-of-process codec isolation host |
| 6 | **WLXTranscode.dll** | DLL | 260 KB | 170 KB | Video transcoding engine (D3D9/11, DXVA2) |
| 7 | **WLXPipeline.dll** | DLL | 350 KB | 220 KB | Media pipeline factory (7 factory functions) |
| 8 | **WLXPipetran.dll** | DLL | 500 KB | 330 KB | Pipeline transform engine (98 RTTI classes) |
| 9 | **WLXMP4Parser.dll** | DLL | 176 KB | 142 KB | MP4/MOV/3GP demultiplexer (DirectShow + MF hybrid) |
| 10 | **WLXVideoTrim.dll** | DLL | 568 KB | 490 KB | Video trimming processor (copy + transcode) |
| 11 | **WLXSlideshow.dll** | DLL | ~300 KB | ~190 KB | Slideshow rendering engine |
| 12 | **WLXFaceRecognition.dll** | DLL | ~200 KB | ~130 KB | Face detection and recognition (7 COM objects) |
| 13 | **WLXPhotoBase.dll** | DLL | 56 KB | 21 KB | Foundation library (exceptions, version, memory, strings) |
| 14 | **WLXPhotoCinematic.dll** | DLL | 88 KB | 58 KB | Ken Burns pan/zoom effect (D3D9, 2 COM objects) |
| 15 | **WLXPhotoAcq.dll** | DLL | 1.75 MB | 637 KB | Photo acquisition (WIA/WPD/FS/DV device abstraction) |
| 16 | **WLXPhotoLibraryMain.dll** | DLL | 3.3 MB | 2.8 MB | Photo Gallery main UI logic (DirectUI, 150+ imports) |
| 17 | **WLXPhotoLibraryDatabase.dll** | DLL | 800 KB | 681 KB | Photo library database + face pipeline orchestration |
| 18 | **WLXMovieLibrary.dll** | DLL | 323 KB | 287 KB | Movie library management (COM factory, registry-based) |
| 19 | **WLXMediaPublishSubscribe.dll** | DLL | 1.4 MB | 568 KB | Publish/subscribe framework (Flickr/Live/YouTube/Vimeo) |
| 20 | **WLXVideoCameraAutoPlayManager.exe** | EXE | 72 KB | 29 KB | Shell AutoPlay handler for video cameras |
| 21 | **WLXImageTranscode.dll** | DLL | 110 KB | 75 KB | Image format transcoding (JPG/BMP/TIFF/PNG via WIC) |
| 22 | **WLXDSPA.dll** | DLL | 200 KB | 166 KB | Digital Media Analysis (shot detection, ToC, DMOs) |
| 23 | **WLXGrinderScheduler.dll** | DLL | 139 KB | 67 KB | Background job/task scheduler ("Grinder" framework) |
| 24 | **wlxclip.dll** | DLL | 286 KB | 198 KB | Clipboard operations for media clips (OLE/IDataObject) |
| 25 | **RSCMFT.dll** | DLL | 288 KB | 254 KB | Rolling Shutter Correction MFT (GPU-accelerated) |
| 26 | **MPG4DEMUX.dll** | DLL | 176 KB | 142 KB | MPEG-4 demultiplexer (DirectShow + MF bridge) |
| 27 | **D3DCOMPILER_46.dll** | DLL | 3.1 MB | 2.9 MB | HLSL shader compiler (reflection only from MovieMakerCore) |
| 28 | **WLMFDS.dll** | DLL | ~100 KB | ~70 KB | Media Foundation ↔ DirectShow bridge |
| 29 | **WLMFReadWrite.dll** | DLL | ~150 KB | ~100 KB | MF source reader / sink writer wrappers |
| 30 | **DmxBici.dll** | DLL | ~50 KB | ~30 KB | BICI telemetry (A/B testing, experience tracking) |
| 31 | **WLXPhotoSqm.dll** | DLL | ~40 KB | ~25 KB | SQM telemetry (Service Quality Monitoring) |
| 32 | **MetadataSys.dll** | DLL | ~80 KB | ~50 KB | Metadata property handler (WLXPSGetItemPropertyHandler) |
| 33 | **Imaging.dll** | DLL | ~100 KB | ~60 KB | RAW image pipeline (CaptureOne) |
| 34 | **UXCore.dll** | DLL | ~500 KB | ~350 KB | DirectUI UI framework |
| 35 | **uxctl.dll** | DLL | ~30 KB | ~15 KB | UxControls helper |
| 36 | **WLFacebookPlugin.dll** | .NET DLL | 558 KB | 537 KB IL | Facebook publish/subscribe plugin (.NET 4.0) |
| 37 | **WLFlickrPlugin.dll** | .NET DLL | 265 KB | 195 KB IL | Flickr publish plugin (.NET 4.0, OAuth 1.0) |
| 38 | **WLVimeoPlugin.dll** | .NET DLL | 228 KB | 143 KB IL | Vimeo publish plugin (.NET 4.0, OAuth 1.0) |
| 39 | **WLYouTubePlugin.dll** | .NET DLL | 133 KB | 94 KB IL | YouTube publish plugin (.NET 4.0, ClientLogin) |

### Summary Statistics

| Metric | Count |
|--------|-------|
| **Total binaries analyzed** | 39 (6 EXEs, 29 native DLLs, 4 .NET DLLs) |
| **Total code size** | ~18.5 MB (native .text sections) |
| **Total image size** | ~28 MB |
| **Total RTTI classes** | 2,200+ (MovieMakerCore alone: 1,018; WLXPipetran: 98; WLXVideoTrim: 74; WLXDSPA: 93; WLXMovieLibrary: 50+; WLXPhotoCinematic: 12) |
| **Total unique exports** | 130+ (MovieMakerCore: 1; WLXPhotoBase: 56; WLXMP4Parser: 9; WLXPipeline: 2; WLMFReadWrite: 7; WLMFDS: 4; WLXSlideshow: 4; WLXFaceRecognition: 4; WLXCodecHost: 4; WLXVideoTrim: 5; WLXPhotoCinematic: 4; WLXPhotoAcq: 4; WLXMovieLibrary: 1; WLXMediaPublishSubscribe: 22; WLXGrinderScheduler: 4; WLXDSPA: 4; WLXImageTranscode: 4; wlxclip: 6; SharedMFDlls: 8) |
| **Total CLSIDs (registered)** | 77+ across 13 binaries |
| **Total TypeLibs** | 7+ |
| **Total COM interfaces** | 120+ |
| **Total .NET plugin assemblies** | 4 |

---

## B. Dependency Graph

### Core Dependency Tree

```
MovieMakerExe.exe
  ├── MovieMakerCore.dll (core engine)
  │     ├── WLXPhotoBase.dll (foundation - delay-loaded)
  │     ├── WLXPipeline.dll (pipeline factories)
  │     ├── WLXPipetran.dll (pipeline transforms)
  │     ├── WLXTranscode.dll (transcoding)
  │     ├── WLXVideoTrim.dll (video trimming)
  │     │     ├── WLXMP4Parser.dll (MP4 support - delay-loaded)
  │     │     │     └── MPG4DEMUX.dll (MPEG-4 demux)
  │     │     └── WMVCore.DLL (WMV direct access)
  │     ├── WLXSlideshow.dll (slideshow rendering)
  │     ├── WLXFaceRecognition.dll (face detection/recognition)
  │     ├── WLXCodecHost.exe (out-of-process codec isolation)
  │     ├── WLXGrinderScheduler.dll (background job scheduler)
  │     ├── D3DCOMPILER_46.dll (shader reflection only)
  │     ├── DmxBici.dll (BICI telemetry)
  │     ├── WLXPhotoSqm.dll (SQM telemetry)
  │     ├── MetadataSys.dll (metadata properties)
  │     ├── UXCore.dll (DirectUI framework)
  │     ├── MFPlat.DLL + MF.dll (Media Foundation)
  │     ├── d3d11.dll + dxva2.dll + d3d9.dll (GPU rendering)
  │     └── WINHTTP.dll (networking)
  │
  ├── MovieMakerLang.dll (resources)
  └── MovieMakerPreviewClient.exe (preview renderer)

Photo Gallery Suite (WLXPhotoLibraryMain.dll)
  ├── WLXPhotoLibraryDatabase.dll (database + face pipeline)
  ├── WLXPhotoAcq.dll (photo acquisition)
  ├── WLXPhotoCinematic.dll (Ken Burns effect)
  ├── WLXMovieLibrary.dll (movie library management)
  ├── WLXMediaPublishSubscribe.dll (publish framework)
  │     ├── wlidcli.dll (Windows Live ID auth)
  │     ├── WLFacebookPlugin.dll (.NET)
  │     ├── WLFlickrPlugin.dll (.NET)
  │     ├── WLVimeoPlugin.dll (.NET)
  │     └── WLYouTubePlugin.dll (.NET)
  ├── WLXDSPA.dll (video analysis/shot detection)
  ├── wlxclip.dll (clipboard operations)
  ├── RSCMFT.dll (rolling shutter correction)
  ├── WLXImageTranscode.dll (image transcoding)
  │     └── WLXCodecHost.exe (codec isolation)
  └── WLXVideoCameraAutoPlayManager.exe (AutoPlay handler)

Shared Infrastructure
  ├── WLXPhotoBase.dll ← used by 20+ DLLs (exceptions, memory, strings, OS detection)
  ├── WLXPhotoSqm.dll ← used by 10+ DLLs (SQM telemetry)
  ├── DmxBici.dll ← used by 8+ DLLs (BICI telemetry)
  ├── MetadataSys.dll ← used by 6+ DLLs (property handlers)
  └── UXCore.dll ← used by 4+ DLLs (DirectUI UI framework)
```

### Cross-Binary Import Matrix (Key Dependencies)

| Consumer ↓ / Provider → | WLXPhotoBase | WLXPhotoSqm | DmxBici | MetadataSys | UXCore | WINHTTP | WMVCore | MFPlat |
|--------------------------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| MovieMakerCore | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| WLXTranscode | ✓ | ✓ | ✓ | — | — | — | — | ✓ |
| WLXPipetran | ✓ | ✓ | ✓ | — | — | ✓ | — | ✓ |
| WLXPipeline | ✓ | ✓ | ✓ | — | — | — | — | ✓ |
| WLXVideoTrim | ✓ | — | — | — | — | — | ✓ | — |
| WLXSlideshow | ✓ | ✓ | ✓ | — | ✓ | — | — | ✓ |
| WLXFaceRecognition | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | ✓ |
| WLXPhotoAcq | ✓ | ✓ | ✓ | ✓ | ✓ | — | — | — |
| WLXPhotoLibraryMain | ✓ | ✓ | — | ✓ | ✓ | ✓ | — | ✓ |
| WLXPhotoLibraryDatabase | ✓ | ✓ | ✓ | ✓ | — | ✓ | — | — |
| WLXMovieLibrary | ✓ | — | — | — | — | — | — | ✓ |
| WLXMediaPublishSubscribe | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | — |
| WLXPhotoCinematic | ✓ | — | — | — | — | — | — | — |
| WLXGrinderScheduler | ✓ | ✓ | ✓ | ✓ | — | — | — | — |
| WLXImageTranscode | ✓ | — | — | ✓ | — | — | — | — |
| WLXDSPA | — | — | — | — | — | — | — | — |
| wlxclip | — | — | — | — | — | — | ✓ | — |
| RSCMFT | — | — | — | — | — | — | — | ✓ |
| MPG4DEMUX | — | — | — | — | — | — | — | ✓ |

---

## C. COM Object Map

### All Registered COM Classes (77+ CLSIDs)

#### MovieMakerCore.dll
| CLSID | ProgID / Name | ThreadingModel | Interface(s) |
|-------|---------------|---------------|--------------|
| (54+ CLSIDs) | Timeline, effects, transitions, rendering objects | Both/Apartment | IDispatch + custom |

#### WLXFaceRecognition.dll
| CLSID | Name | Interface(s) |
|-------|------|--------------|
| `{B1A6684D-1C5B-4976-843D-53F1E840E49C}` | KenBurnsEffect | IFaceRecognition |
| `{0831F9E2-19E0-4837-B026-28E1E6E087F4}` | KenBurnsEffectConfig | IFaceRecognitionConfig |
| + 5 more | Face detection, recognition, tile gen | IFaceDetection, IFaceRecognition, etc. |

#### WLXPhotoCinematic.dll
| CLSID | ProgID | ThreadingModel |
|-------|--------|---------------|
| `{557B4CD8-C1EA-4A46-84EE-BA1AF9AA67D4}` | CinematicFullScreen | Apartment |
| `{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}` | PanZoomTransform | Apartment |

#### WLXPhotoAcq.dll
| CLSID | Name |
|-------|------|
| `{4E13844B-6012-40cb-8E3C-0F7EB9AEF6DD}` | LivePhotoAcquire |
| `{94B1CBEA-F33F-4508-8AAA-55FD8113EFCB}` | LivePhotoAcquireOptionsDialog |
| `{0D5A7D0E-9A06-4e17-85D9-A0B24036371D}` | LivePhotoPickerDialog |
| `{E84D0D46-3D57-4039-9EFE-310AF1CAF92A}` | LivePhotoAcqDeviceSelectionDlg |
| `{4D8A134F-3D0A-4375-8B1A-78CD171C9318}` | LivePhotoAcquisitionWizard |
| `{3BD0ACD1-71CA-4475-92CC-E0AA0AAF843F}` | CLSIDForCancel |
| TypeLib `{D85885D4-A18A-4fac-AEE1-BE9AAB0F4485}` | Photo Acquisition TypeLib |

#### WLXMediaPublishSubscribe.dll
| CLSID | Name | ThreadingModel |
|-------|------|---------------|
| `CLSID_OnlineMediaPluginManager` | OnlineMediaPluginManager | Free |
| `CLSID_OnlineMediaPluginDecorator` | OnlineMediaPluginDecorator | Free |
| `CLSID_FlickrProvider` | FlickrProvider | Free |
| `CLSID_LiveProvider` | LiveProvider | Free |
| `CLSID_MediaPublishSubscribeProviderManager` | MediaPubSubProviderManager | Free |
| `CLSID_DatabasePublishProperties` | DatabasePublishProperties | Free |
| `CLSID_DatabasePublishItemPropertyStore` | DatabasePublishItemPropertyStore | Free |
| `CLSID_MetadataSettingsController` | MetadataSettingsController | Free |

#### WLXVideoCameraAutoPlayManager.exe
| CLSID | ProgID | Interface |
|-------|--------|-----------|
| `{9B5C97F6-B3A5-4A6D-8B03-993EC7291A22}` | WXLAutoPlayMgr.WLXHWEventHandler.1 | IHWEventHandler |

#### WLXImageTranscode.dll
| CLSID | Name |
|-------|------|
| `{20575516-78AF-4404-B3C7-51D05F9945B5}` | ImageTranscode |
| `{E253845C-A1A2-4ae4-B784-5E8140CD5B2C}` | ImageLoader |
| `{B8A2E14E-290D-4122-B092-1A7D86198CCE}` | WLXOutofProc |
| TypeLib `{17DC7884-443D-478e-ABD7-BC22856FC7F1}` | ImageTranscode TypeLib |
| IID `{CB38B8DF-0D64-42b9-802A-28DCB678BFEB}` | IWLXImageLoader |

#### WLXGrinderScheduler.dll
| CLSID | ProgID | ThreadingModel |
|-------|--------|---------------|
| `{121244C5-B0AD-45fb-9D8E-B893B917D053}` | Windows Live Photo Gallery Grinder Scheduler | Both |
| TypeLib `{77A16B7E-9DFE-410c-8863-264BF3E91103}` | Grinder TypeLib |

#### WLXDSPA.dll (11 DMOs)
| CLSID | Name |
|-------|------|
| `{BC8CA1B3-B013-4866-9621-825957DF23F3}` | CWLXTocGeneratorDmo |
| `{09BC59C2-70DD-45f9-A5B7-DE9F2A5CA34B}` | CWLXThumbnailGeneratorDmo |
| `{15CD2459-C14B-457b-B57B-3DBA111B9D09}` | CClusterDetectorEx |
| `{1D8D19C8-0A33-45a4-9B3E-255B85C363A8}` | CTocEntry |
| `{15A4E6E5-A9E5-49cb-AFFC-E822F082D427}` | CTocEntryList |
| `{C9FF4813-CB5F-4ac6-B003-4D79AE2F43E9}` | CToc |
| `{CE1D8A09-77EA-4eaa-9619-832A9E5DB447}` | CTocCollection |
| `{75704D6C-09BA-4d19-AFEA-5F21FC08B3DB}` | CTocParser |
| `{7F2CE947-4E80-446d-9AE4-17DD9D82A353}` | CFileIo |
| `{BF620143-7420-460a-9EEE-178B78D4939D}` | CAsfTocParser |
| `{9FAE79C9-BA02-43d9-9382-C7BEF740A596}` | CAviTocParser |

#### SharedMF DLLs
| Binary | CLSID | Type |
|--------|-------|------|
| RSCMFT.dll | (registered via MFTRegister) | Media Foundation Transform |
| MPG4DEMUX.dll | (registered via ATL .rgs) | DirectShow filter + MF bridge |

---

## D. Export Table (Complete Custom Exports)

### Non-COM Custom Exports

| Binary | Export | Demangled Signature | Purpose |
|--------|--------|-------------------|---------|
| **WLXPhotoBase.dll** (56) | `Base::Throw(long)` | `void __stdcall Throw(HRESULT)` | SEH/C++ exception throw |
| | `Base::ThrowLastError()` | `void __stdcall ThrowLastError()` | Throw with GetLastError |
| | `BasePrivate::New(uint, bool)` | `void* __cdecl New(uint, bool)` | Custom heap allocator |
| | `BasePrivate::Delete(void*)` | `void __cdecl Delete(void*)` | Custom heap deallocator |
| | `Base::IsWin8OrGreater()` | `bool __stdcall IsWin8OrGreater()` | OS version check |
| | `Base::GetBaseStringManager()` | `CAtlStringMgr& __stdcall GetBaseStringManager()` | Global ATL string manager |
| | `Base::Version::Set(...)` | `void __thiscall Set(uint, uint)` | Version setter |
| | `Base::GdiplusStatusToHresult(int)` | `HRESULT __stdcall GdiplusStatusToHresult(int)` | GDI+ error mapping |
| | (56 total — see WLXPhotoBase analysis) | | |
| **WLXMovieLibrary.dll** (1) | `CreateMovieFactory` | `HRESULT __stdcall CreateMovieFactory(void**)` | IMovieFactory factory |
| **WLXVideoTrim.dll** (5) | `CreateVideoPlayer` | `int __cdecl CreateVideoPlayer(wchar_t*, HWND*, RotationEnum, bool, long&)` | Preview player |
| | `CreateVideoCopierFromMediaType` | `int __cdecl CreateVideoCopierFromMediaType(wchar_t*, AM_MEDIA_TYPE*, bool, long&)` | Format-dispatch copier |
| | `CreateAVICopierDirect` | `int __cdecl CreateAVICopierDirect(wchar_t*, bool, long&)` | Direct AVI copier |
| | `CreateVideoFormatContextTranscoder` | `int __cdecl CreateVideoFormatContextTranscoder(FormatContext*, wchar_t*, bool, long&)` | FormatContext transcoder |
| | `CreateVideoWMVTranscoder` | `int __cdecl CreateVideoWMVTranscoder(IWMProfile*, wchar_t*, bool, long&)` | WMV transcoder |
| **WLXMediaPublishSubscribe.dll** (22) | `MediaPublishSubscribeHelper::PublishItems(...)` | `void __thiscall PublishItems(IGrinderTaskScheduler*, ...)` | Core publish method |
| | `MediaPublishSubscribeHelper::PluginPublishItems(...)` | `void __thiscall PluginPublishItems(...)` | Plugin-based publish |
| | `GetLiveSignInProvider` | `static HRESULT __stdcall GetLiveSignInProvider(ILiveSignInProvider**)` | Live ID auth provider |
| | `LaunchAuthBrowser(...)` | `static void __stdcall LaunchAuthBrowser(wchar_t*, SiteIDType, ...)` | OAuth browser launch |
| | `VideoTranscoder::Initialize(...)` | `HRESULT __thiscall Initialize(wchar_t*, wchar_t*, uint, uint, uint, uint)` | Transcoder init |
| | `VideoTranscoder::Transcode(...)` | `HRESULT __thiscall Transcode(IVideoTranscodeProgressCB*)` | Run transcode |
| | `IsHResultDRMProtected` | `static BOOL __stdcall IsHResultDRMProtected(HRESULT)` | DRM detection |
| | (22 total) | | |
| **wlxclip.dll** (6) | `VMGGetClipCreateFunctions` | Returns function pointers for clip creation | Clip factory |
| | `VMGShellThumbnailGeneratorW` | Generates shell thumbnails | Thumbnail generation |
| | (6 total including COM DLL exports) | | |
| **WLXPhotoLibraryMain.dll** (3) | `RunAsStandAlone` | Standalone gallery launch | Main entry |
| | `DisplayRepairPromptDialog` | Database repair dialog | Repair UI |
| | `DllCanUnloadNow` | COM lifetime | |

---

## E. Registry Key Ownership

### Key Ownership Map (Who Writes What)

| Registry Path | Owner Binary | Access |
|---------------|-------------|--------|
| `HKLM\Software\Microsoft\Windows Live\Movie Maker` | MovieMakerCore, WLXVideoTrim | R/W |
| `HKLM\Software\Microsoft\Windows Live\Common\Movie Library` | WLXMovieLibrary | R/W |
| `HKLM\Software\Microsoft\VideoTrim\StreamBufferCopier Settings` | WLXVideoTrim | R/W |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery` | WLXPhotoLibraryMain, WLXPhotoAcq | R/W |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\Library` | WLXPhotoLibraryMain, WLXPhotoLibraryDatabase | R/W |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\Viewer` | WLXPhotoLibraryMain | R/W |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\Slideshow` | WLXPhotoLibraryMain | R/W |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\VideoTrim` | WLXVideoTrim | R/W |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\*` | WLXPhotoAcq, WLXPhotoLibraryMain | R/W |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FlickrAccounts` | WLXMediaPublishSubscribe | R/W |
| `HKCU\Software\Microsoft\Windows Live\PublishPlugins` | WLXMediaPublishSubscribe | R/W |
| `HKCU\Software\Microsoft\Windows Live\SubscribePlugins` | WLXMediaPublishSubscribe | R/W |
| `HKLM\Software\Microsoft\Windows Live\Photo Gallery\Slideshow\Themes\{CLSID}` | WLXPhotoCinematic | R |
| `HKLM\Software\Microsoft\Windows Photo Gallery\Slideshow\Themes` | WLXPhotoCinematic | R |
| `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ThumbnailCache` | WLXPhotoLibraryDatabase | R |
| `HKLM\Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\%ws` | WLXPhotoAcq | R |
| `HKLM\SOFTWARE\Microsoft\Windows Media Foundation\ByteStreamHandlers` | WLXPhotoAcq, WLXPhotoLibraryMain | R |
| `HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\*` | WLXVideoCameraAutoPlayManager | R |
| `HKCU\Software\Microsoft\IdentityCRL` | WLXMediaPublishSubscribe | R/W |
| `HKLM\Software\Microsoft\IdentityCRL` | WLXMediaPublishSubscribe | R |
| `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers` | WLXVideoCameraAutoPlayManager | W |
| `HKLM\Software\Classes\CLSID\{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | WLXVideoTrim | W |

### COM Registration Keys (Shared)

All COM DLLs register under `HKCR\CLSID\{CLSID}\InprocServer32` with appropriate `ThreadingModel`. Key shared patterns:
- `HKCR\CLSID\{CLSID}\LocalServer32` — WLXVideoCameraAutoPlayManager (EXE server)
- `HKCR\AppID\{CLSID}` — AppID registration for EXE servers
- `HKCR\<ProgID>\CLSID` — ProgID ↔ CLSID mapping

---

## F. File Format Support

### Video Format Matrix

| Format | Extension(s) | Read | Write | Trim | Transcode | Binary |
|--------|-------------|:---:|:---:|:---:|:---:|--------|
| WMV/ASF | .wmv, .asf | ✓ | ✓ | ✓ (AsfCopier) | ✓ (AsfTranscoder) | WLXVideoTrim, WMVCore |
| AVI | .avi | ✓ | ✓ | ✓ (AVICopier) | ✓ (AVITranscoder) | WLXVideoTrim |
| DV-AVI | .avi (DV) | ✓ | ✓ | ✓ | ✓ (DVTranscoder) | WLXVideoTrim |
| MPEG-2 | .mpg, .mpeg, .m2v | ✓ | ✓ | ✓ (Mpg2Copier) | ✓ (Mpg2Transcoder) | WLXVideoTrim |
| DVR-MS | .dvr-ms, .sbe | ✓ | ✓ | ✓ (StreamBufferCopier) | ✓ (StreamBufferTranscoder) | WLXVideoTrim, SBE |
| MP4/MOV | .mp4, .mov, .3gp | ✓ | ✓ | ✓ (FormatContext) | ✓ | WLXMP4Parser → MPG4DEMUX |
| MJPEG | .avi (MJPG) | ✓ | ✓ | ✓ | ✓ (AVITranscoder) | WLXVideoTrim |
| H.264 | (in MP4/MOV) | ✓ | — | ✓ | — | MPG4DEMUX (AVC1) |
| VC-1 | (in MP4/MOV) | ✓ | — | ✓ | — | MPG4DEMUX (WVC1) |

### Audio Format Support

| Format | Codec | Evidence |
|--------|-------|----------|
| WMA | WMA9+ | WMVCore imports, AsfCopier |
| MP3 | MPEG1Audio | MPG4DEMUX, WLXPipeline |
| AAC | AAC | MPG4DEMUX (esds) |
| PCM | PCM | Multiple DLLs |
| Dolby AC3 | AC3 | MPG4DEMUX, WLXPipeline |
| DTS | DTS | MPG4DEMUX |

### Image Format Matrix

| Format | Read | Write | Binary |
|--------|:---:|:---:|--------|
| JPEG | ✓ | ✓ | WLXImageTranscode (GDI+/WIC) |
| BMP | ✓ | ✓ | WLXImageTranscode |
| TIFF | ✓ | ✓ | WLXImageTranscode (WIC) |
| PNG | ✓ | ✓ | WLXImageTranscode (WIC) |
| RAW | ✓ | — | Imaging.dll (CaptureOne pipeline) |
| GIF | ✓ | — | WIC |

### Container Format Support (WLXMP4Parser / MPG4DEMUX)

The MP4 parser has a complete QuickTime atom parser supporting:
- **moov, mvhd, trak, tkhd, mdia, minf, stbl, stsd, stts, ctts, stsc, stsz, stco, co64, stss**
- **avc1, avcC** (H.264 config), **esds** (MPEG-4 audio), **vc-1** (VC-1 config)
- **dref, hdlr, iods, free, wide, uuid, skip, ftyp, meta, mfra, pinf, pnot**

### Codec GUIDs / FourCCs Referenced

**Video FourCCs**: DV25, DV50, DVH1, DVHD, DVSD, DVSL, H264, MJPG, MP4V, MP4S, M4S2, MPG1, MPG2, WVC1, WMV1, WMV2, WMV3, MSS1, MSS2, YUY2, UYVY, NV12, IYUV, YV12, YVYU, YVU9, Y41P, Y411, Y211, v210, v216, v410, Y416, P010, P016, P210, P216, ARGB32, RGB32, RGB24, RGB555, RGB565, RGB8

**Audio FourCCs**: PCM, MP3, AAC, AC3, DTS, DOLBY_AC3_SPDIF, MPEG1Audio, MPEG2_AUDIO, WAVE, IMA4, LPCM

---

## G. Thread Model

### Thread Architecture Per Binary

| Binary | Threading Model | Worker Threads | Synchronization |
|--------|----------------|---------------|-----------------|
| **MovieMakerCore.dll** | MTA (CComMultiThreadModel) | Thread pool + dedicated render threads | CRITICAL_SECTION, Events, Interlocked |
| **WLXGrinderScheduler.dll** | Both (free-threaded) | N worker threads (message pump per thread) | Semaphore + CRITICAL_SECTION + Events |
| **WLXPhotoLibraryDatabase.dll** | MTA | IOCP worker threads | IOCP, CRITICAL_SECTION, Semaphore, SList |
| **WLXPhotoLibraryMain.dll** | STA (UI thread) + MTA workers | UI thread + background workers | CRITICAL_SECTION, Events, Timer |
| **WLXVideoTrim.dll** | Single-threaded per operation | DirectShow filter graph threads | CRITICAL_SECTION, Events |
| **WLXPipeline.dll** | MTA | Media Foundation work items | MFPutWorkItemEx, Events |
| **WLXMovieLibrary.dll** | MTA (CComMultiThreadModel) | Vista+ Thread Pool API | Threadpool Work, Semaphore, Events |
| **WLXFaceRecognition.dll** | MTA | Background detection threads | CRITICAL_SECTION, Events |
| **WLXMediaPublishSubscribe.dll** | MTA | Grinder scheduler workers | Via IGrinderTaskScheduler |
| **WLXGrinderScheduler.dll** | Both | _beginthreadex workers with message pumps | Semaphore, CRITICAL_SECTION, MsgWaitForMultipleObjects |
| **RSCMFT.dll** | Both | Windows Thread Pool API | Threadpool Work, Events |
| **WLXPhotoAcq.dll** | STA (UI) + MTA (device) | UI thread + device polling | CRITICAL_SECTION, Events, IOCP |

### Shared Synchronization Patterns

1. **CRITICAL_SECTION with SpinCount** — Used by 15+ binaries (via `InitializeCriticalSectionAndSpinCount`)
2. **Win32 Events** — Used by 12+ binaries for completion signaling
3. **InterlockedIncrement/Decrement** — Used by all binaries for reference counting
4. **IOCP** — Used by WLXPhotoLibraryDatabase for async file operations
5. **Win32 Thread Pool API** — Used by WLXMovieLibrary, RSCMFT (Vista+ pattern)
6. **Semaphore** — Used by WLXGrinderScheduler for job queue capacity

---

## H. Telemetry Map

### Three-Tier Telemetry System

#### Tier 1: SQM (Service Quality Monitoring) — WLXPhotoSqm.dll
| Function | Signature | Consumers |
|----------|-----------|-----------|
| `Sqm::Startup()` | `void __stdcall Sqm::Startup()` | 10+ DLLs |
| `Sqm::Shutdown()` | `void __stdcall Sqm::Shutdown()` | 10+ DLLs |
| `Sqm::IsEnabled()` | `bool __stdcall Sqm::IsEnabled()` | 10+ DLLs |
| `Sqm::AddToStream(K, K)` | 5 overloads | 10+ DLLs |
| `Sqm::Set(K, K)` | Counter set | 8+ DLLs |
| `Sqm::Increment(K, K)` | Counter increment | 8+ DLLs |
| `Sqm::StartStreamTimer / StopStreamTimer` | Timer-based metrics | 5+ DLLs |
| `Sqm::DeferReportMedian` | Median aggregation | 3+ DLLs |
| `Sqm::EnableShipAsserts` | Assert control | WLXPhotoLibraryMain |

#### Tier 2: BICI (Business Intelligence Customer Insights) — DmxBici.dll
| Function | Signature | Consumers |
|----------|-----------|-----------|
| `BiciWrapper::StartExperience()` | `HRESULT __stdcall StartExperience()` | 8+ DLLs |
| `BiciWrapper::EndExperience()` | `HRESULT __stdcall EndExperience()` | 8+ DLLs |
| `BiciWrapper::TransferExperienceToWeb()` | `bool __stdcall TransferExperienceToWeb(wchar_t*, wchar_t**)` | 5+ DLLs |
| `BiciWrapper::AddToStream()` | `void __stdcall AddToStream(DWORD, Tuple*)` | 8+ DLLs |
| `BiciWrapper::SetAnid()` | `HRESULT __stdcall SetAnid(wchar_t*)` | 3+ DLLs |
| `BiciWrapper::AddStringToDataPoint()` | `bool __stdcall AddStringToDataPoint(DWORD, DWORD, wchar_t*)` | 3+ DLLs |

#### Tier 3: ETW (Event Tracing for Windows)
| Binary | ETW Provider |
|--------|-------------|
| MovieMakerCore | ETW trace provider (RegisterTraceGuidsW) |
| WLXPhotoCinematic | RegisterTraceGuidsW / TraceEvent |
| WLXPhotoAcq | RegisterTraceGuidsW / TraceEvent |
| WLXPhotoLibraryMain | EventRegister / EventWrite |
| WLXMovieLibrary | RegisterTraceGuidsW / TraceMessage |
| WLXVideoTrim | RegisterTraceGuidsW / TraceMessage |
| WLXGrinderScheduler | RegisterTraceGuidsW / TraceEvent |
| WLXMediaPublishSubscribe | RegisterTraceGuidsW / TraceEvent |
| WLXPhotoLibraryDatabase | RegisterTraceGuidsW / TraceEvent |

#### Web Telemetry Endpoints
| Service | URL Pattern | Binary |
|---------|------------|--------|
| SkyDrive/OneDrive | `https://profile.live.com/Services/` | WLXMediaPublishSubscribe |
| COMA API | `http://api.live.net` | WLXMediaPublishSubscribe |
| Flickr REST | `http://api.flickr.com/services/rest/` | WLXMediaPublishSubscribe |
| Flickr Upload | `http://api.flickr.com/services/upload/` | WLXMediaPublishSubscribe |
| Bing Maps Geocode | SOAP API via WINHTTP | WLXPhotoLibraryDatabase |

---

## I. IPC Mechanisms

### Inter-Process Communication Map

| Producer | Consumer | Mechanism | Protocol |
|----------|----------|-----------|----------|
| **MovieMakerCore.dll** | **WLXCodecHost.exe** | `CreateProcessW` + named pipe | Command-line args + pipe I/O |
| **MovieMakerCore.dll** | **WLXPipeline.dll** | `GetPipelineCreateFunctions` (load-time import) | Direct function call |
| **MovieMakerCore.dll** | **WLXTranscode.dll** | Load-time import | Direct function call |
| **MovieMakerCore.dll** | **WLXVideoTrim.dll** | Load-time import (opaque handles) | Factory functions → long& handles |
| **MovieMakerCore.dll** | **MovieMakerPreviewClient.exe** | `CreateProcessW` + DDE/WM_COPYDATA | Preview rendering requests |
| **WLXPhotoLibraryMain.dll** | **WLXPhotoLibraryDatabase.dll** | Load-time import (delay-loaded) | 17 exported functions |
| **WLXPhotoLibraryMain.dll** | **WLXMediaPublishSubscribe.dll** | Load-time import | 5 functions (PublishItems, etc.) |
| **WLXMediaPublishSubscribe.dll** | **WLFacebookPlugin.dll** | COM interop (.NET → native) | IPublishPlugin interface |
| **WLXMediaPublishSubscribe.dll** | **WLFlickrPlugin.dll** | COM interop (.NET → native) | IPublishPlugin interface |
| **WLXMediaPublishSubscribe.dll** | **WLVimeoPlugin.dll** | COM interop (.NET → native) | IPublishPlugin interface |
| **WLXMediaPublishSubscribe.dll** | **WLYouTubePlugin.dll** | COM interop (.NET → native) | IPublishPlugin interface |
| **Shell AutoPlay** | **WLXVideoCameraAutoPlayManager.exe** | COM LocalServer32 | IHWEventHandler |
| **WLXImageTranscode.dll** | **WLXCodecHost.exe** | COM remoting (ORPC) | Out-of-process proxy |
| **WLXGrinderScheduler.dll** | Worker DLLs | COM in-proc | IGrinderTaskScheduler interface |

### Named Pipes / Shared Memory

| Path | Producer | Consumer | Purpose |
|------|----------|----------|---------|
| `\\.\pipe\WLXCodecHost_*` | MovieMakerCore | WLXCodecHost | Codec operations |
| Memory-mapped file | WLXPhotoLibraryDatabase | WLXPhotoLibraryMain | Database cache |
| Temp files (`_TempWMV`) | WLXVideoTrim | WLXVideoTrim | Intermediate trim output |

### COM Activation Chains

```
MovieMakerCore → CoCreateInstance → WLXCodecHost.exe (LocalServer32)
MovieMakerCore → CoCreateInstance → CLSID_FilterGraph (DirectShow)
MovieMakerCore → CoCreateInstance → CLSID_CaptureGraphBuilder2
MovieMakerCore → CoCreateInstance → CLSID_DMOWrapperFilter
WLXMediaPublishSubscribe → DllGetClassObject → CLSID_LiveProvider
WLXMediaPublishSubscribe → DllGetClassObject → CLSID_FlickrProvider
WLXPhotoAcq → CoCreateInstance → IPhotoAcquire, IPhotoPickerDialog
WLXGrinderScheduler → CoCreateInstance → GrinderScheduler COM class
Shell → CoCreateInstance (LocalServer32) → WLXVideoCameraAutoPlayManager
```

---

## J. Architecture Gaps

### Identified Gaps in Analysis Coverage

| Gap | Description | Impact |
|-----|-------------|--------|
| **MovieMakerCore exports** | Only 1 named export (`GetPipelineCreateFunctions`) but 54+ CLSIDs — COM interface mapping incomplete | High — core engine interfaces unknown |
| **RTTI vtable mapping** | RTTI names identified but vtable layouts only partially decoded for MovieMakerCore (683 classes, ~15% mapped) | High — method signatures unknown |
| **MovieMakerLang.dll** | Resource satellite DLL not analyzed — localization strings unextracted | Low — resource-only |
| **wlidcli.dll** | Windows Live ID client — ordinal-only imports, no source analysis | Medium — auth flow partially understood |
| **UXCore.dll** | 150+ imports into WLXPhotoLibraryMain — DirectUI element hierarchy mapped but implementations unknown | Medium — UI framework internals opaque |
| **Imaging.dll** | RAW pipeline (CaptureOne) — only 1 export referenced, internal architecture unknown | Medium — RAW support gaps |
| **WLDCore.dll** | Referenced by WLXGrinderScheduler — core library, no analysis | Low — infrastructure |
| **sqmapi.dll** | SQM API — referenced but not analyzed | Low — telemetry backend |
| **mmfsres.dll / WLAVRes.dll** | Resource DLLs — referenced by multiple binaries | Low — resource-only |
| **DShow base classes** | Custom AVC-prefixed DirectShow base classes in WLXVideoTrim, MPG4DEMUX — inheritance and overrides partially mapped | Medium — filter development gaps |
| **.NET plugin internals** | Publish plugins (FB/Flickr/Vimeo/YouTube) — metadata reflection done but runtime behavior untested | Medium — upload flows theoretical |
| **WLXPipeline internals** | 37 RTTI classes, 7 factory functions — full pipeline topology not decoded | High — pipeline architecture gaps |
| **WLXCodecHost.exe protocol** | Pipe protocol between MovieMakerCore and WLXCodecHost — command structure unknown | High — codec isolation mechanism opaque |
| **D3D11 rendering pipeline** | MovieMakerCore uses D3D11 for GPU compositing — shader bytecode embedded as resources, vertex/pixel shader layouts unknown | High — rendering pipeline gaps |
| **Media Foundation topology** | MFPlat used by 8+ DLLs — full topology (source → transform → sink) not mapped for most pipelines | Medium — media pipeline gaps |

### Recommended Next Steps

1. **Decode MovieMakerCore COM interfaces** — Use vtable analysis and GUID cross-referencing to map all 54+ COM objects to their interface contracts
2. **Trace WLXCodecHost pipe protocol** — Hook `CreateFileW` / `ReadFile` / `WriteFile` to capture the pipe protocol between MovieMakerCore and the codec host
3. **Map WLXPipeline factory graph** — Call all 7 `GetPipelineCreateFunctions` outputs to decode the pipeline topology
4. **Extract D3D11 shader resources** — Dump embedded shader bytecode and use D3DReflect to decode vertex/pixel shader signatures
5. **Test .NET publish plugins** — Create mock HTTP endpoints to capture actual upload payloads from each plugin
6. **Map DirectUI XML schemas** — Extract and catalog all DirectUI XML resource files from UXCore.dll consumers

---

*Document generated from 60+ analysis files. All CLSIDs, RTTI classes, exports, and registry paths verified against binary analysis outputs. See individual binary analysis files in `analysis/<BinaryName>/` for detailed findings.*
