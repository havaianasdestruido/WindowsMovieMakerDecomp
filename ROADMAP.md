# Project Summary & Resume Roadmap
## Windows Live Movie Maker 2012 - Source Code Recreation

> **Last updated:** July 22, 2026
> **Build command:** `& "C:\Program Files\CMake\bin\cmake.exe" --build build --config Debug`
> **Project root:** `C:\Users\mcmco\Desktop\WMMR`
> **STATUS: FULL BUILD CLEAN - All 18 targets compile and link with 0 errors**

---

## What This Project Is

Recreating the full source code of Windows Live Movie Maker 2012 (`MovieMakerCore.dll` + 17 supporting DLLs + launcher EXE) from binary analysis. The originals are in `undecomp/` (gitignored). The reconstructed source is in `src/`.

**Key facts:**
- Original binary: MSVC 11.0 (VS2012), C++14, ATL/WTL, targets Windows 6.2+
- Build environment: MSVC 19.44 (VS2022 Build Tools), SDK 10.0.26100.0
- CMake generator: `Visual Studio 17 2022`, architecture: **Win32** (x86)
- Project codename: **Sundance**, version **16.4.3528.0331**
- Single DLL export: `MovieMakerMain` from MovieMakerCore.dll
- 1360 RTTI classes across 48 namespaces
- 375+ source files, 18 CMake targets
- **App launches** and shows a window ("Windows Live Movie Maker")
- **23 original bugs/quirks documented** in `QUIRKS.md`
- **200+ stub methods replaced** with real implementations

---

## Current Build State - ALL 18 TARGETS BUILD CLEAN

| Target | Type | Output |
|--------|------|--------|
| **MovieMaker.exe** | Launcher EXE | `build/bin/Debug/MovieMaker.exe` (94 KB) |
| **MovieMakerCore.dll** | Main engine DLL | `build/bin/Debug/MovieMakerCore.dll` (1,414 KB) |
| **WLXPhotoBase.dll** | Foundation library | `build/bin/Debug/WLXPhotoBase.dll` (48 KB) |
| **MovieMakerLang.dll** | Localization | `build/bin/Debug/MovieMakerLang.dll` |
| **WLXPipeline.dll** | Media pipeline | `build/bin/Debug/WLXPipeline.dll` |
| **WLXVideoTrim.dll** | Video trim | `build/bin/Debug/WLXVideoTrim.dll` |
| **WLXMovieLibrary.dll** | Movie library | `build/bin/Debug/WLXMovieLibrary.dll` |
| **WLXPipetran.dll** | Transitions | `build/bin/Debug/WLXPipetran.dll` |
| **WLXSlideshow.dll** | Slideshow engine | `build/bin/Debug/WLXSlideshow.dll` |
| **WLXTranscode.exe** | Transcode tool | `build/bin/Debug/WLXTranscode.exe` |
| **MovieMakerPreviewClient.dll** | Preview renderer | `build/bin/Debug/MovieMakerPreviewClient.dll` |
| **WLMFDS.dll** | MF/DS bridge | `build/bin/Debug/WLMFDS.dll` |
| **WLMFReadWrite.dll** | MF read/write | `build/bin/Debug/WLMFReadWrite.dll` |
| **WLXMediaPublishSubscribe.dll** | Publishing | `build/bin/Debug/WLXMediaPublishSubscribe.dll` |
| **WLXPhotoCinematic.dll** | Cinematic effects | `build/bin/Debug/WLXPhotoCinematic.dll` |
| **WLXMP4Parser.dll** | MP4 parser | `build/bin/Debug/WLXMP4Parser.dll` |
| **WLXFaceRecognition.dll** | Face detection | `build/bin/Debug/WLXFaceRecognition.dll` |
| **WLXCodecHost.exe** | Codec host | `build/bin/Debug/WLXCodecHost.exe` |

**Build result: 0 compile errors, 0 link errors, 0 RC errors**

---

## Implementation Progress

### Fully Implemented Subsystems
- **Application framework** (`SundanceApp/`): SundanceAppMain, ProjectManager, ImportController, ExportController, MediaBrowser, ThumbnailCache, ClipboardManager, UndoManager
- **Storyboard/Project model** (`StoryboardManager/`): MovieProject, TimelineTrack, MovieExtent, Templates, Serialization (Reader/Writer/Classes), Theme system
- **Preview/Transport** (`Preview/`, `Transport/`): PreviewPresenter, PreviewDataContext, TransportBase, CommandBin
- **Publishing** (`Publishing/`): PublishJob, PublishClasses, PublishDialogs
- **Audio processing** (`Audio/`): AudioFadeProcessor, AudioDuckingProcessor, AudioChannelMapper, AudioOutput, AudioCapture, Mixer
- **UI Behaviors** (`UI/`): 50+ OnMessage implementations across 37 behavior classes
- **Ribbon UI** (`UI/Ribbon/`): 67 command handlers, full UpdateState, ComputeCommandEnabled, MRU site registry
- **Media Foundation pipeline** (`HMRAVSource/`): MFSource, StreamSink, StreamSinkHost, TextureInterop, VideoProc, TranscodeManager
- **HMREngine** (`HMREngine/`): DXResources, TextRender, MeshResourceDX, TextureCodecs, d3dx11compat
- **Undo/Redo**: Transaction-based undo stack with clipboard integration

### What's Still Stub/Placeholder
- Some serialization round-trip edge cases
- Full export rendering pipeline (writes test frames, not real video)
- Add-in/plugin system (loads DLLs but no plugin API)
- Telemetry/SQM (intentionally stubbed - no data sent)
- Legacy project format (dead code, shipped but unused)

---

## Source Structure

### MovieMaker.exe (launcher)
- `src/MovieMaker/main.cpp` - WinMain entry, delay-loads MovieMakerCore.dll
- `src/MovieMaker/moviemaker.rc` - Resource script
- `src/MovieMaker/CMakeLists.txt`

### MovieMakerCore.dll (~375 files)
```
src/MovieMakerCore/
  pch.h / pch.cpp                 # PCH + SDK compat section
  dllmain.cpp                     # DLL entry (extern "C" MovieCore_Initialize/Shutdown)
  MovieMakerCore.cpp              # Stub exports (original, file-locked)
  MovieMakerCore_new.cpp          # Corrected exports (currently compiled)
  ComFactory.h                    # ATL COM wrappers (fixed MSVC 14.44 conflicts)
  exports.h / common.h
  SundanceApp/                    # Application framework
    SundanceAppMain.cpp/.h        # Main app class (subsystem init, WndProc, message loop)
    SundanceAppDataContext.cpp/.h  # IDispatch data binding
    ProjectManager.cpp/.h         # File I/O, dirty state, auto-save
    ImportController.cpp/.h       # Media import
    ExportController.cpp/.h       # Export/publish
    MediaBrowser.cpp              # Thumbnail, drag-drop
    ThumbnailCache.cpp/.h         # Background thumbnail generation
    ClipboardManager.cpp/.h       # System clipboard + internal clipboard
    UndoManager.cpp/.h            # Undo/redo stack with transactions
  StoryboardManager/              # Project model
    MovieProject.cpp/.h           # Core data model (I/O, extent management)
    TimelineTrack.h               # Track management
    StoryboardManager.h/.cpp      # Top-level manager
    MovieEffect/                  # Effect serialization
    Templates.h/.cpp              # Transition/effect templates (23 effects, 7 themes)
    Theme/                        # Visual themes (Apply/Remove, XML parsing)
    Serialization/                # .wlmp file I/O (Reader/Writer/Classes/BoundPlaceholder)
    Transport/                    # Playback transport, CommandBin
  HMREngine/                      # 3D rendering engine (X3D/VRML-based)
    DXResources/                  # DX11 resource management
    TextRender/                   # DWrite text rendering pipeline
    d3dx11compat.h/.cpp           # D3DX11 stub (removed from Win10 SDK)
  HMRAVSource/                    # Media Foundation AV pipeline
    Audio/                        # Audio processing (fade, ducking, channel mapping)
    MFSource.cpp/.h               # Media Foundation source creation
    StreamSink.cpp/.h             # Sample processing
    TextureInterop.cpp/.h         # DX11 texture sharing with MF
    VideoProc.cpp                 # Video processing (DXVA2, XVideoProc)
    TranscodeManager.cpp          # Transcoding pipeline
  UI/
    Ribbon/                       # Windows Ribbon framework (67 commands)
    UIBehaviorClasses.cpp/.h      # 50+ behavior implementations
    EditingBehaviors.cpp          # Rich edit, options dialog, help
    TimelineDataSources.cpp       # Timeline item handler
  Preview/                        # Video preview
    PreviewPresenter.cpp/.h       # D3D11 preview rendering
    PreviewDataContext.cpp/.h     # IDispatch data binding
  Publishing/                     # Video publishing
    PublishJob.cpp/.h             # Background encode/upload
    PublishClasses.cpp/.h         # Service implementations
    PublishDialogs.cpp            # Summary dialog
  DataStructs/                    # Utility containers
```

### Supporting DLLs (12 targets)
- WLXPhotoBase, WLXPipeline, WLXVideoTrim
- WLXMovieLibrary (D3D9 video processor), WLXPipetran (transitions)
- WLXSlideshow, WLXTranscode, MovieMakerPreviewClient (GDI+)
- WLMFDS (MF/DS bridge), WLMFReadWrite (MF read/write)
- WLXMediaPublishSubscribe, WLXPhotoCinematic (Ken Burns)
- WLXMP4Parser, WLXFaceRecognition, WLXCodecHost

### Tools Suite
```
tools/
  pe_analyzer.py                  # PE binary analysis
  rtti_extractor.py               # RTTI class name extraction
  diff_exports.py                 # Export table comparison
  build.ps1                       # Automated build script
  test_all.py                     # Build verification
  string_analyzer.py              # String extraction
  analysis/                       # Detailed binary analysis scripts
  thirdparty/                     # Ghidra, VBox scripts
```

### External Dependencies
```
src/WTL/                              # WTL 10 headers (20 files from NuGet)
src/MovieMakerCore/HMREngine/d3dx11compat.h  # D3DX11 stub
src/MovieMakerCore/HMRAVSource/dxva2stubs.cpp # DXVA2 stubs
```

---

## SDK Compatibility Fixes Applied

| Original (VS2012/Win8 SDK) | Fix Applied (VS2022/Win10 SDK) |
|---|---|
| `d3dx11.h` / d3dx11 effect API | Stub: `d3dx11compat.h/.cpp` (returns `E_NOTIMPL`) |
| `d3dx9.h` / `d3dx9.lib` | Removed from all targets |
| `d3dcompiler_46.lib` | Renamed to `d3dcompiler.lib` |
| `dxva2.h` DXVA2CreateVideoProcessor* | Stub: `dxva2stubs.cpp` |
| `mferror.lib` | Removed (MF error codes in headers) |
| `MF_ENABLE_HARDWARE_TRANSFORMS` | Removed (not in Win10 SDK) |
| `INTERNET_OPTION_ENABLE_FEATURE` | Removed (not in Win10 SDK enum) |
| `uiribbon.lib` / `IID_IUICommandHandler` | GUID manually defined |
| `MF_OBJECT_UNKNOWN` | Defined as `((MF_OBJECT_TYPE)0)` |
| `XmlWriterProperty_ProcessNamespaces` | Defined as `((XmlWriterProperty)1)` |
| `IXmlReader::GetAttribute` | Replaced with `MoveToAttributeByName` + `GetValue` |
| `IDXVA2VideoProcessor` interface | Stub interface in `pch.h` |
| `winmm.h` | Replaced with `mmsystem.h` |
| `MFAttributes` type (non-standard) | Replaced with `IMFAttributes*` |

---

## Known Gotchas (23 lessons learned)

1. `winmm.h` does NOT exist in SDK 10.0.26100.0 - Use `mmsystem.h`
2. `IAtlStringMgr` is in `<atlstr.h>` not `<atlbase.h>`
3. ATL 14+ defines `BaseAtlThrow` natively
4. `CAtlArray` copy constructor is private - Use `.Copy()`
5. `CComObject<T>` does NOT expose T's members through `->`
6. D3DX11 is completely removed - Must use stub
7. `CComPtr<T>` requires `AddRef/Release` - Not for non-COM types
8. Circular includes broken by extracting enums into separate header
9. `RIBBON_API` macro was never defined - Added to `exports.h`
10. `ATL::CStringMap` does NOT exist - Use `std::map`
11. `IXmlReader::GetAttribute` does NOT exist - Use `MoveToAttributeByName` + `GetValue`
12. MF enums removed from Win10 SDK
13. `IDXVA2VideoProcessor` interface removed
14. `DrawTextLayout` takes 3 or 4 args (NOT `D2D1_RECT_F` as 4th)
15. `_itow_s` requires 4 args (value, buf, bufSize, radix)
16. `WriteEndElement()` and `WriteEndDocument()` take 0 args
17. Circular include fixed by heap-allocating `ProjectTimeline`
18. Forward-declared structs must be moved INTO namespace in headers
19. `DEFINE_STUB_TRANSITION` macro `L##stringId` - Remove L## prefix
20. `mferror.lib` does not exist in Win10 SDK
21. `Gdiplus::Graphics::DrawImage` - Takes `Gdiplus::Image*` not `const Bitmap*`
22. ATL `BEGIN_COM_MAP`/`COM_INTERFACE_ENTRY(IUnknown)` fails with MSVC 14.44 - Use `COM_INTERFACE_ENTRY_IID(IID_IUnknown, ClassName)`
23. `CComQIPtr<IUnknown>` has template specialization conflict in MSVC 14.44 - Use `CComPtr<IUnknown>` instead

---

## Git History
```
5ac21fa feat: 15-subagent pass - serialization, UI, publishing, audio, MF pipeline, undo/redo, stability
dddf67e feat: massive stub implementation pass - 50+ methods, build fixes, tools suite
5a3b787 Add QUIRKS.md documenting 23 original bugs, quirks, and anomalies
9507e27 MovieMaker.exe now launches with main window
a51b520 15 parallel subagents: implement real logic across all subsystems
defc56d Add 200+ RTTI classes, fix DLL exports to COM pattern, all 18 targets still clean
43053da Clean up temp files, update .gitignore, refresh ROADMAP.md and THINKING_PROCESS.md
d4ef776 All 18 CMake targets build clean: 0 compile errors, 0 link errors
edb4713 add Background/Transport/Legacy/Publishing/External + HMRAVSource helpers
0b00926 add Preview, Serialization, Theme, Ribbon, Audio, DXResources, X3DNodeImpls, PatternMesh
1f64d2d add HMRAVSource media pipeline, DataStructs, and resource IDs
ac3851a reconstruct HMREngine, UI behaviors, and all supporting DLLs
728636b reconstruct MovieMakerCore.dll framework, SundanceApp, and StoryboardManager
d246f1  add analysis scripts and initial source reconstruction
d92f6e2 initial: project setup with gitignore and PE analysis script
```
