# Word-for-Word Thinking Process
## Windows Live Movie Maker 2012 - Source Code Recreation Project

---

## Phase 1: Initial Analysis & Planning

**Starting Point:** The user provided Windows Live Movie Maker 2012 binaries in `undecomp/` and asked for a 1:1 source code recreation. The binaries were extracted from the original Microsoft Live Essentials 2012 setup (`wlsetup-all.exe`).

**First thought:** This is a reverse-engineering-to-source-code project. I need to analyze the PE binaries thoroughly before writing any code. The key files are:
- `MovieMaker.exe` - the launcher (~122KB)
- `MovieMakerCore.dll` - the main codebase (~10.5MB, single export: `MovieMakerMain`)
- 17 supporting DLLs (WLXPhotoBase.dll, DuiDirect.dll, etc.)

**Analysis approach:** I extracted RTTI class names, ASCII strings, UTF-16 strings, imports, exports, and PE structure from all binaries. Found 1360 unique RTTI type names across 48 namespaces, 5721 ASCII strings, and 2560 UTF-16 strings. The project codename is "Sundance" (confirmed by RTTI). Version is 16.4.3528.0331. Built with MSVC 11.0 (VS2012), targeting Windows 6.2+ (Win8+).

**Key insight:** MovieMaker.exe is just a thin launcher. It delay-loads MovieMakerCore.dll and calls `MovieMakerMain`. All the real code is in MovieMakerCore.dll. The 17 supporting DLLs are utility libraries (WLXPhotoBase, DuiDirect, etc.) that MovieMakerCore links against.

**Architecture decision:** The project should be structured as 18 CMake targets - one for MovieMakerCore.dll, one for MovieMakerLang.dll, one for MovieMaker.exe, and 16 supporting libraries.

---

## Phase 2: Setting Up Build System

**Build system choice:** CMake with Visual Studio 17 2022 generator. Initially tried NMake Makefiles but switched because the user has VS 2022 Build Tools installed. The architecture must be Win32 (x86) since the original binaries are 32-bit.

**Git setup:** Created `.gitignore` to exclude `undecomp/`, `intro/`, `readme.txt`, and all binaries. The source code goes in `src/`. Analysis output goes in `analysis/`.

**ATL/WTL dependency discovery:** The codebase heavily uses ATL (Active Template Library) and WTL (Windows Template Library). ATL comes with the VS Build Tools (needed to install the "C++ ATL" component via `vs_installer.exe modify`). WTL is not included with VS - downloaded WTL 10 headers from NuGet and placed them at `src/WTL/`.

**Important lesson:** WTL 10 headers were obtained from the NuGet package `WTLCSPkg`. The `atlapp.h` header from WTL requires `extern CAppModule _Module;` to be declared before including WTL control headers.

---

## Phase 3: Source Code Reconstruction - Round 1

**Starting with the skeleton:** Created all 18 CMakeLists.txt files and the basic source tree. The top-level `CMakeLists.txt` sets up:
- Win32 architecture
- MSVC runtime: MultiThreadedDLL
- Target version: Windows 6.2
- C++ standard: 14 (matching MSVC 11.0 capabilities)

**MovieMaker.exe reconstruction:** This is the simplest target. It's just a launcher that calls `MovieMakerMain` from MovieMakerCore.dll. Created:
- `src/MovieMaker/main.cpp` - WinMain entry point, loads MovieMakerCore.dll, calls MovieMakerMain
- Resource files for the icon

**Supporting DLLs:** Created stub implementations for all 17 DLLs based on their RTTI class names and string analysis:
- WLXPhotoBase - Foundation library (exceptions, memory, OS detection, GDI+, smart pointers, containers)
- DuiDirect - DirectUI rendering framework
- MovieMakerLang - Localization/language resources
- DmxBici - Analytics telemetry
- MetadataSys - Media metadata handling
- WLXPhotoSqm - Service Quality Management telemetry
- wlidcli - Windows Live ID client
- And 10 more...

**MovieMakerCore.dll structure:** This is the bulk of the work. Based on RTTI analysis, identified these major subsystems:
1. **SundanceApp** - Application framework (main, data context, command line, auto-save, clipboard, media browser)
2. **StoryboardManager** - Project management (movie project, extents, templates, timeline tracks)
3. **HMREngine** - 3D rendering engine (DX11, X3D scene graph, pattern mesh, text rendering)
4. **HMRAVSource** - Media pipeline (video/audio sources, encoding, transcoding, capture)
5. **UI** - Ribbon, preview, dialogs
6. **Serialization** - Project file I/O
7. **Theme** - Visual themes, text composition
8. **Background** - Background processing, auto-fit, conductor
9. **Transport** - Playback transport, command bin

---

## Phase 4: Source Code Reconstruction - Round 2 (Deep Implementation)

**SundanceApp:** The main application entry point that initializes the WTL message loop, creates the main window, and manages the application lifecycle. Key RTTI classes: `CSundanceApp`, `SundanceAppDataContext`, `CCommandLineParser`, `CAutoSaveManager`, `CClipboardManager`, `CMediaBrowser`.

**StoryboardManager:** The project model that manages the movie timeline. Contains the movie project structure, extent calculations, template definitions, and timeline tracks. Uses serialization for .wlmp file format.

**HMREngine (the big one):** This is the 3D rendering engine based on DirectX 11. It implements a scene graph with X3D-inspired node types:
- `X3DFieldNode` base class with typed field system (SFBool, SFVec3f, MFNode, etc.)
- Grouping nodes (Group, Transform, Switch, Billboard)
- Appearance nodes (Material, ImageTexture, Appearance)
- Geometry nodes (Shape, Box, Sphere, Cylinder, Cone, Plane, Mesh)
- Sensor nodes (TouchSensor, TimeSensor, ProximitySensor)
- Layer nodes (Layer2D, Layer3D)
- PatternMesh for video transition effects

**Key design pattern:** The HMREngine uses COM heavily. All X3D nodes inherit from `CComObjectRootEx` and implement `CComObjectRootEx`. The field system uses `CComObject<T>` for reference counting.

**D3DX11 compatibility:** The original code used D3DX11 (the Effect Framework), which was deprecated after DirectX SDK. Created `d3dx11compat.h` to provide compatible type definitions (`ID3DX11Effect`, `ID3DX11EffectTechnique`, `ID3DX11EffectPass`, etc.) using raw D3D11 interfaces.

**Math library (X3DMath.h):** Built a complete math library on top of DirectXMath:
- `Vec2`, `Vec3`, `Vec4` - vector types inheriting from XMFLOAT2/3/4
- `Rotation4f` - quaternion rotation
- `Matrix3f`, `Matrix4f` - matrix types inheriting from XMFLOAT3X3/XMFLOAT4X4
- `Rgb`, `Rgba` - color types
- `Frustum` - view frustum culling

**HMRAVSource (media pipeline):** Implements video/audio source management using Media Foundation:
- `AVSource`, `MFSource`, `DShowSource` - media source abstractions
- `EncodeProfile`, `StreamSink`, `TranscodeManager` - encoding pipeline
- `VideoCapture`, `AudioCapture`, `AVCaptureSession` - capture support
- `TextureInterop` - DX11 texture sharing with Media Foundation
- `Waveform`, `AudioOutput`, `AudioQueue`, `Mixer` - audio pipeline

**UI Ribbon:** Uses Windows Ribbon Framework (via `uiribbon.h`). Implements the ribbon application model with command handlers.

**Preview:** `PreviewPresenter` and `PreviewDataContext` manage the video preview window.

**Serialization:** Full project file I/O system for .wlmp format with `SerializationContext`, `SerializationReader`, `SerializationWriter`.

---

## Phase 5: Build Fixes - Round 1

**First build attempt:** CMake configured successfully, but compilation produced many errors.

**Error: `mfsession.h` not found** - The header doesn't exist in modern Windows SDK. Replaced with `mfidl.h` (which contains the same interfaces).

**Error: `imagingfactory.h` not found** - Doesn't exist. Removed the include; `IWICImagingFactory` is defined in `wincodec.h`.

**Error: `d3dx11.h` not found** - D3DX11 was removed from Windows SDK. Created `d3dx11compat.h` stub header.

**Error: GDI+ Status enum values not found** (`UnsupportedImageFormat`, `UnsupportedPixelFormat`, etc.) - These don't exist in the Windows 10 SDK's GDI+. The original code used custom enum values. Fixed by using standard GDI+ status codes or HRESULT equivalents.

**Error: `E_OVERFLOW` redefinition** - Changed to `HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW)`.

**Error: `CAtlArray` copy constructor private** - Modern ATL made the copy constructor private. Changed to use `Copy()` method instead.

**Error: `BaseAtlThrow` redefinition** - ATL 14+ (in VS 2022) provides this natively. Removed custom definition.

**Error: `ATL::CCRTHeap::Instance()` not found** - Changed to `new ATL::CAtlStringMgr` for string manager creation.

---

## Phase 6: Build Fixes - Round 2 (Current Session)

**Error: `structMovieMakerException` (missing space)** - Typo in `common.h` line 176. Fixed to `struct MovieMakerException`.

**Error: `<xstring>` not found** - Not a standard header in MSVC 19.44. Changed to `<string>` in `HMREngine.h`.

**Error: `mfmp2t.h` not found** - Removed from `pch.h`. Not available in all SDK versions.

**Error: `ATL::IAtlStringMgr` not a member of ATL** - `IAtlStringMgr` is defined in `<atlstr.h>`, not `<atlbase.h>`. Added `#include <atlstr.h>` to `WLXPhotoBase.h`.

**Error: `BaseAtlThrow` already defined** - ATL 14+ provides it. Removed custom definitions from both `WLXPhotoBase.h` and `WLXPhotoBase.cpp` and `BaseTypes.h`.

**Error: `GetBaseStringManager` return type mismatch** - The header declared it as `ATL::IAtlStringMgr*` but the implementation matched differently. Fixed the header declaration.

**Error: `winmm.h` not found (ROOT CAUSE of most MovieMakerCore errors)** - The Windows SDK 10.0.26100.0 does NOT include `winmm.h`. The multimedia header was renamed/moved. Changed to `#include <mmsystem.h>` in both `common.h` and `pch.h`.

**Error: `X3DFieldTypes.h` - Vec2, Vec3, Vec4, Rotation4f, Rgb, Rgba undeclared** - This was the CASCADING error from `winmm.h` failure. When `pch.h` fails to compile (because of `winmm.h`), the entire precompiled header fails, meaning ALL subsequent includes from pch.h (windows.h, atlbase.h, d3d11.h, DirectXMath.h, etc.) are NOT available. This caused:
- `DirectXMath.h` to not compile → `Vec2` not defined
- `X3DFieldNode` to not inherit from `CComObjectRootEx` → COM map errors
- All `SingleFieldBase<T>` specializations to fail

**Error: X3DFieldTypes.h `m_value` undeclared on `CComObject<SFVec2f>*`** - `CComObject<T>` does NOT expose `T`'s members through `->`. The `Clone()` method does `CComObject<SFVec2f>* p; ... p->m_value = m_value;` but `p->m_value` doesn't work because `CComObject` wraps the object. Need to use `p->m_pInstance->m_value` or restructure the Clone implementation.

**Current state after winmm.h fix:** The `winmm.h → mmsystem.h` fix resolved the PCH compilation. But now a NEW error pattern emerged: files that DO include pch.h compile fine, but files that DON'T include pch.h (PatternMeshFactory.cpp, TextRenderPipeline.cpp) fail because they can't find `namespace` keyword - this means X3DMath.h fails to compile without the PCH context.

**The real cascading chain:**
1. `winmm.h` not found → pch.h fails → everything fails (FIXED with `mmsystem.h`)
2. .cpp files without `#include "pch.h"` → X3DMath.h fails → Vec2 undeclared → X3DFieldTypes.h fails (NOT YET FIXED)

---

## Current Build State

**What compiles successfully:**
- MovieMakerLang.dll
- Most MovieMakerCore .cpp files (those that include pch.h)
- All supporting DLLs
- MovieMaker.exe

**What fails:**
- PatternMeshFactory.cpp, PatternMesh.cpp, TextRenderPipeline.cpp - missing `#include "pch.h"`
- ImageThumbnail.cpp - IWICImagingFactory template error at line 511
- HMREngine.h - syntax error at line 113 (HMRError enum issue)
- Shaders.h - Matrix4f not found (cascading from X3DMath.h failure)
- X3DFieldTypes.h - Vec2 undeclared (cascading), m_value access pattern wrong
- WLXPhotoBase - IAtlStringMgr (should be fixed by atlstr.h addition, verify)

**Git status:** 7 commits, all source files created, recent fixes (winmm.h, mmsystem.h, atlstr.h, struct space fix) not yet committed.

---

## Key Technical Decisions Made

1. **CMake over raw vcxproj** - Easier cross-platform setup, though we only target MSVC
2. **WTL 10 from NuGet** - Not bundled with VS, must be obtained separately
3. **d3dx11compat.h stub** - Rather than removing all D3DX11 usage, create compatibility layer
4. **DirectXMath-based math library** - Build X3DMath.h on DirectXMath rather than raw float arrays
5. **COM-based X3D field system** - Match the original architecture using ATL CComObject
6. **MultiThreadedDLL runtime** - Match the original binary's runtime linking

---

## Files Created (375 total across 18 targets)

### Top-level
- `CMakeLists.txt` - Master build configuration
- `.gitignore` - Excludes binaries, undecomp, intro

### MovieMaker.exe (launcher)
- `src/MovieMaker/main.cpp`
- `src/MovieMaker/CMakeLists.txt`
- `src/MovieMaker/MovieMaker.exe.manifest`
- `src/MovieMaker/MovieMaker.rc`

### MovieMakerCore.dll (main codebase - ~350 files)
- `src/MovieMakerCore/CMakeLists.txt`
- `src/MovieMakerCore/pch.h`, `pch.cpp`
- `src/MovieMakerCore/dllmain.cpp`
- `src/MovieMakerCore/MovieMakerCore.cpp`, `.h`, `.def`
- `src/common.h`
- `src/exports.h`
- SundanceApp/ (6 files)
- StoryboardManager/ (18+ files across subdirectories)
- HMREngine/ (30+ files across DXResources, X3DNodeImpls, PatternMesh, TextRender)
- HMRAVSource/ (25+ files including Audio/)
- UI/ (Ribbon/)
- Preview/ (2 files)
- DataStructs/ (2 files)

### Supporting DLLs (16 targets)
- WLXPhotoBase/ (BaseTypes.h, WLXPhotoBase.h/.cpp, etc.)
- MovieMakerLang/
- DuiDirect/
- DmxBici/
- MetadataSys/
- WLXPhotoSqm/
- wlidcli/
- And 9 more...

### External Dependencies
- `src/WTL/` - WTL 10 headers (20 files from NuGet)

---

## User Preferences & Constraints

1. **No frequent admin prompts** - User goes AFK; minimize UAC prompts
2. **Windows-only build** - This is a Windows application
3. **1:1 recreation** - Match the original binary behavior as closely as possible
4. **No comments unless asked** - Code style preference
5. **Git commits** - Only commit when explicitly asked
6. **Concise responses** - Keep terminal output short
