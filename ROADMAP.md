# Project Summary & Resume Roadmap
## Windows Live Movie Maker 2012 - Source Code Recreation

> **Last updated:** July 15, 2026
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

---

## Current Build State - ALL 18 TARGETS BUILD CLEAN

| Target | Type | Output |
|--------|------|--------|
| **MovieMaker.exe** | Launcher EXE | `build/bin/Debug/MovieMaker.exe` |
| **MovieMakerCore.dll** | Main engine DLL | `build/bin/Debug/MovieMakerCore.dll` (1,027 KB) |
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

## What's Next (Post-Build Phase)

1. Verify DLL import/export tables match original binaries
2. Test MovieMaker.exe launches (will fail at runtime without real implementations)
3. Compare RTTI class layouts between original and rebuilt DLLs
4. Incrementally replace stub implementations with real code
5. Improve semantic fidelity of reconstructed source

---

## Source Structure

### MovieMaker.exe (launcher)
- `src/MovieMaker/main.cpp` - WinMain entry, delay-loads MovieMakerCore.dll
- `src/MovieMaker/moviemaker.rc` - Resource script
- `src/MovieMaker/CMakeLists.txt`

### MovieMakerCore.dll (~350 files)
```
src/MovieMakerCore/
  pch.h / pch.cpp                 # PCH + SDK compat section
  dllmain.cpp                     # DLL entry (extern "C" MovieCore_Initialize/Shutdown)
  MovieMakerCore.cpp              # Stub exports
  exports.h / common.h
  SundanceApp/                    # Application framework (7 controller stubs + UI)
  StoryboardManager/              # Project model
    MovieProject.h/.cpp           # Core data model
    TimelineTrack.h/.cpp          # Track management
    StoryboardManager.h/.cpp      # Top-level manager
    Templates.h/.cpp              # Transition/effect templates
    Theme/                        # Visual themes
    Serialization/                # .wlmp file I/O
    Background/                   # Background processing
    Transport/                    # Playback transport
  HMREngine/                      # 3D rendering engine (X3D/VRML-based)
    DXResources/                  # DX11 resource management
    X3DNodes.cpp/.h               # X3D node implementations
    X3DMath.cpp/.h                # Math (quaternion SLERP, etc.)
    d3dx11compat.h/.cpp           # D3DX11 stub (removed from Win10 SDK)
    Shaders.cpp/.h
    X3DNodeImpls/
  HMRAVSource/                    # Media Foundation AV pipeline
    HMRAVSourceTypes.h            # Shared enums (breaks circular include)
    dxva2stubs.cpp                # DXVA2 stubs (removed from Win10 SDK)
    Audio/                        # Audio processing
  UI/Ribbon/                      # Windows Ribbon framework
  Preview/                        # Video preview
  Publishing/                     # Video publishing
  DataStructs/                    # Utility containers
```

### Supporting DLLs (12 targets)
- WLXPhotoBase, WLXPipeline, WLXVideoTrim
- WLXMovieLibrary (D3D9 video processor), WLXPipetran (transitions)
- WLXSlideshow, WLXTranscode, MovieMakerPreviewClient (GDI+)
- WLMFDS (MF/DS bridge), WLMFReadWrite (MF read/write)
- WLXMediaPublishSubscribe, WLXPhotoCinematic (Ken Burns)
- WLXMP4Parser, WLXFaceRecognition, WLXCodecHost

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

## Known Gotchas (21 lessons learned)

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

---

## Git History
```
d4ef776 All 18 CMake targets build clean: 0 compile errors, 0 link errors
edb4713 add Background/Transport/Legacy/Publishing/External + HMRAVSource helpers
0b00926 add Preview, Serialization, Theme, Ribbon, Audio, DXResources, X3DNodeImpls, PatternMesh
1f64d2d add HMRAVSource media pipeline, DataStructs, and resource IDs
ac3851a reconstruct HMREngine, UI behaviors, and all supporting DLLs
728636b reconstruct MovieMakerCore.dll framework, SundanceApp, and StoryboardManager
d246f1  add analysis scripts and initial source reconstruction
d92f6e2 initial: project setup with gitignore and PE analysis script
```
