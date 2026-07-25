# WLXSlideshow.dll Analysis

## Overview

**File:** WLXSlideshow.dll (Windows Live Movie Maker 2012)
**Role:** Slideshow generation, display, and playback control DLL
**Image size:** 0x7F000 (~520 KB)
**PDB:** `WLXSlideshow.pdb` `{6547A44F-AABE-4CFA-9675-0113F4BD19E9}`
**Build:** 16.4.3528.0331_ship.client.main.w5m4 (timestamp: 2014-04-01)

## PE Structure

| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Subsystem | Windows GUI (2) |
| Linker | MSVC 11.00 |
| Image Base | 0x10000000 |
| Image Size | 0x7F000 |
| Entry Point | 0x100338FF |
| OS/Image Version | 6.02 / 6.02 |
| DLL Characteristics | Dynamic Base (ASLR), NX Compatible |
| Stack reserve/commit | 256 KB / 4 KB |
| Heap reserve/commit | 1 MB / 4 KB |
| Checksum | 0x81FEB |

### Sections

| Section | VA | VirtSize | RawSize | Characteristics |
|---------|-------|----------|---------|-----------------|
| `.text` | 0x1000 | 0x3EE89 | 0x3F000 | Code, Execute Read |
| `.data` | 0x40000 | 0x414C | 0x3800 | Init Data, Read Write |
| `.rsrc` | 0x45000 | 0x32AD0 | 0x32C00 | Init Data, Read Only (203 KB) |
| `.reloc` | 0x78000 | 0x6654 | 0x6800 | Init Data, Discardable, Read Only |

### Directories

| Directory | RVA | Size |
|-----------|-----|------|
| Export | 0x3FDE0 | 0xA9 |
| Import | 0x3DDE8 | 0x190 |
| Resource | 0x45000 | 0x32AD0 |
| Base Reloc | 0x78000 | 0x466C |
| Debug | 0x15D0 | 0x38 |
| Load Config | 0x6B90 | 0x40 |
| IAT | 0x1000 | 0x594 |
| Delay Import | 0x3DA80 | 0x80 |
| Certificates | 0x7C000 | 0x3EC8 |

## Export Table (4 functions)

| Ordinal | RVA | Name |
|---------|-----|------|
| 1 | 0x0000B52B | `DllCanUnloadNow` |
| 2 | 0x0000B51B | `DllGetClassObject` |
| 3 | 0x0000B54B | `DllRegisterServer` |
| 4 | 0x0000B53C | `DllUnregisterServer` |

Standard COM DLL exports. The DLL is a COM in-process server that provides slideshow-related COM objects.

## COM GUIDs Discovered

### Registered COM Classes (by CLSID variable name in RTTI)
- `CLSID_SimpleSlideshowDisplay` — registered via ATL object map
- `CLSID_SlideshowExtension` — registered via ATL object map
- `CLSID_TimelineDisplay` — registered via ATL object map

### GUID Found in Binary
| GUID | Likely Purpose |
|------|----------------|
| `{84FBA192-4F8D-4a5d-94DE-083446ACC1D0}` | CLSID for one of the COM classes (SimpleSlideshowDisplay, SlideshowExtension, or TimelineDisplay) |

### COM Interface RTTI (38+ COM interfaces defined internally)
- `ISlideshowControl` — slideshow playback control
- `ISlideshowDisplay` — slideshow rendering surface
- `ISlideshowList` — list of slides/photos
- `ISlideshowListIterator` / `ISlideshowListIteratorEx` — iteration over slides
- `ISlideshowChunkIterator` — chunked iteration (for parallel processing)
- `ISlideshowFilteredListIterator` — filtered iteration
- `ISlideshowThemeManager` — theme management
- `ISlideshowThemeIterator` — theme iteration
- `ISlideshowThemeEventSink` — theme change notifications
- `ISlideshowUserInterface` — UI integration
- `ISlideshowFrameController` — frame/window control
- `ISlideshowCustomAccessibleHandler` — accessibility
- `ISlideshowCustomEventHandler` — custom events
- `ISlideshowCustomStateHandler` — custom state
- `IModeControlSlideshowSettings` — mode-specific settings
- `IUserInterfaceSlideshowSettings` — UI settings
- `IEaselSlideshowFrame` — "Easel" frame (likely the display canvas concept)
- `IDisplayControl` / `IDisplayControlEventSink` — display control
- `IModeControlAudioVideo` / `IUserInterfaceAudioVideo` — audio/video modes
- `ICanvasView` — canvas view
- `IPaintContext` / `IPaintContextHWnd` — paint context
- `IPrintContext` / `IPrintContextHWnd` — print context

## Import Table

### Direct Imports (load-time)

#### MSVCR110.dll (48 functions)
VC++ 2012 CRT: memory (`malloc`, `calloc`, `free`, `memcpy`, `memmove`, `memset`), string (`wcscpy_s`, `wcscat_s`, `wcsstr`, `wcschr`, `wcsncpy_s`, `wmemcpy_s`, `_wcsicmp`, `_wcsnicmp`, `iswspace`, `towupper`), format (`swprintf_s`, `vswprintf_s`, `_vsnwprintf`, `_vsnprintf`), CRT init (`_initterm`, `_initterm_e`, `_amsg_exit`), exception (`_CxxThrowException`, `__CxxFrameHandler3`, `__CppXcptFilter`, `terminate`), threading (`_beginthreadex`), RTTI (`??1type_info@@UAE@XZ`)

#### KERNEL32.dll (60 functions)
Process/thread (`CreateThread`, `GetCurrentProcess`, `GetCurrentThreadId`, `GetExitCodeThread`), synchronization (`CreateMutexW`, `CreateEventW`, `SetEvent`, `ResetEvent`, `WaitForSingleObject`, `Interlocked*`), memory (`HeapAlloc`, `HeapFree`, `VirtualAlloc`, `VirtualFree`, `GlobalMemoryStatusEx`), module (`LoadLibraryA/W/Ex`, `GetModuleHandleA/W`, `GetProcAddress`), files (`DeleteFileW`, `GetFileAttributesW`), time (`GetSystemTime`, `GetLocalTime`, `GetTickCount`, `GetTickCount64`, `SystemTimeToFileTime`, `QueryPerformanceCounter`), locale (`CompareStringW`, `GetThreadUILanguage`), error handling (`RaiseException`, `UnhandledException`), PE-specific (`EncodePointer`, `DecodePointer`)

#### USER32.dll (46 functions)
Window management (`CreateWindowExW`, `DestroyWindow`, `ShowWindow`, `SetWindowPos`, `SetWindowLongW`, `GetWindowLongW`, `GetWindowRect`, `GetClientRect`), message loop (`GetMessageW`, `PeekMessageW`, `DispatchMessageW`, `PostMessageW`, `PostQuitMessage`, `SendMessageW`), menus (`CreatePopupMenu`, `AppendMenuW`, `InsertMenuItemW`, `RemoveMenu`, `DestroyMenu`, `TrackPopupMenuEx`, `GetMenuItemCount`, `GetMenuItemInfoW`), drawing (`BeginPaint`, `EndPaint`, `DrawTextW`, `FillRect`, `InvalidateRect`, `UpdateWindow`), input (`SetCursor`, `GetCursor`, `ShowCursor`, `GetKeyState`, `ScreenToClient`), monitors (`EnumDisplayDevicesW`, `EnumDisplaySettingsW`, `GetMonitorInfoW`, `MonitorFromPoint`), accessibility (`NotifyWinEvent`)

#### ADVAPI32.dll (18 functions)
Registry (`RegOpenKeyExW`, `RegCreateKeyExW`, `RegQueryValueExW`, `RegSetValueExW`, `RegEnumKeyExW`, `RegEnumValueW`, `RegDeleteKeyW`, `RegDeleteValueW`, `RegCloseKey`, `RegQueryInfoKeyW`, `RegOpenCurrentUser`), ETW tracing (`TraceEvent`, `TraceMessage`, `RegisterTraceGuidsW`, `UnregisterTraceGuids`, `GetTraceEnableFlags/Level/LoggerHandle`)

#### ole32.dll (12 functions)
Core COM (`CoCreateInstance`, `CoGetClassObject`, `CoInitialize`, `CoUninitialize`, `CLSIDFromString`), memory (`CoTaskMemAlloc`, `CoTaskMemFree`, `CoTaskMemRealloc`), utility (`StringFromCLSID`, `StringFromGUID2`, `PropVariantClear`, `CreateStreamOnHGlobal`)

#### OLEAUT32.dll (11 ordinal imports)
Ordinals 2, 4, 6, 7, 8, 9, 10, 150, 161, 162, 277 — likely `SysAllocString`, `SysFreeString`, `VariantClear`, `VariantInit`, `SafeArray*` functions

#### gdiplus.dll (25 functions)
Image loading/manipulation (`GdipCreateBitmapFromStream`, `GdipCreateBitmapFromScan0`, `GdipCloneImage`, `GdipDisposeImage`), drawing (`GdipDrawImageRectRectI`, `GdipDrawImagePointsRectI`, `GdipFillRectangle`, `GdipFillRectangleI`), encoding/saving (`GdipGetImageEncoders`, `GdipGetImageEncodersSize`, `GdipSaveImageToFile`), attributes (`GdipCreateImageAttributes`, `GdipSetImageAttributesWrapMode`), pixel ops (`GdipBitmapLockBits`, `GdipBitmapUnlockBits`, `GdipGetImagePixelFormat`), startup (`GdiplusStartup`, `GdiplusShutdown`)

#### d3d9.dll (1 function)
`Direct3DCreate9` — Direct3D 9 initialization

#### d3dx9_32.dll (3 functions)
`D3DXCreateFontW` — font rendering
`D3DXCreateSprite` — sprite rendering
`D3DXMatrixScaling` — matrix scaling transform

#### GDI32.dll (13 functions)
`BitBlt`, `CreateCompatibleDC`, `CreateDIBSection`, `CreateFontIndirectW`, `CreateRectRgn`, `SelectObject`, `DeleteDC`, `DeleteObject`, `GetDeviceCaps`, `GetStockObject`, `SetTextColor`, `SetBkMode`, `ExtCreateRegion`

#### UXCore.dll (11 functions)
DirectUI resource management: `CRMResource` (constructor/destructor), `CRMStringResource` (constructor, Load, Length, ToString), plus `RMUpdateResourceSet`, `RMFindModule`, `RMLoadIcon`, `RMLoadAccelerators`

#### SHELL32.dll (3 functions)
`ShellExecuteW`, `ShellExecuteExW`, `SHCreateItemFromParsingName`

#### Other Direct Imports
- **OLEACC.dll** (2): `AccessibleObjectFromWindow`, `LresultFromObject` (UI Automation)
- **PSAPI.DLL** (1): `GetProcessMemoryInfo`
- **SHLWAPI.dll** (12): registry (`SHRegOpenUSKeyW`, `SHRegGetUSValueW`, `SHRegEnumUSValueW`, `SHRegCloseUSKey`), paths (`PathAddBackslashW`, `PathAppendW`, `PathRemoveFileSpecW`, `PathFileExistsW`, `PathFindExtensionW`), strings (`StrCmpIW`, `StrRChrW`, `SHStrDupW`)
- **WTSAPI32.dll** (2): `WTSRegisterSessionNotification`, `WTSUnRegisterSessionNotification`
- **VERSION.dll** (3): `GetFileVersionInfoSizeW`, `GetFileVersionInfoW`, `VerQueryValueW`
- **DmxBici.dll** (2): BICI telemetry — `AddStringToDataPoint`, `TransferExperienceToWeb`
- **msi.dll** (1): Ordinal 90 (likely `MsiGetComponentPathW` or similar)
- **dwmapi.dll** (1, delay-load): `DwmSetWindowAttribute`

### Delay-Load Imports

| DLL | Functions |
|-----|-----------|
| **WLXPhotoBase.dll** (13) | `Base::Exception` (ctor/dtor/copy), `Base::Throw(HRESULT)`, `Base::ThrowLastError()`, `BaseAtlThrow`, `BasePrivate::New/Delete`, `Base::GdiplusStatusToHresult`, `Base::IsWin7OrGreater`, `Base::IsWin8OrGreater`, `Base::IsOutOfMemoryError`, `String::GetBaseStringManager` |
| **WLXPhotoSqm.dll** (4) | `Sqm::Startup`, `Sqm::Shutdown`, `Sqm::AddToStream` (2 overloads) |
| **dwmapi.dll** (1) | `DwmSetWindowAttribute` |

## Resource Strings (UX Text)

### Window Title
```
Photo Gallery Slide Show
```

### Menu / Navigation Bar Commands
```
&Play   P&ause   &Next   &Back   &Exit
S&huffle   &Loop
```

### Speed Selection (Radio menu items)
```
Slide Show speed - &Slow
Slide Show speed - &Medium
Slide Show speed - &Fast
```

### Navigation bar
```
Slide show navigation bar
```

### Effect/Theme Names (in one resource string delimited by null/LF)
```
Loading...
Fade
Black and white
Sepia
Classic
Album
Collage
Spin
Frame
Glass
```

These are the built-in slideshow transition/theme styles. The slideshow offers:
- **Loading...** — placeholder/loading state
- **Fade** — crossfade transition
- **Black and white** — monochrome effect
- **Sepia** — sepia tone effect
- **Classic** — default classic transition
- **Album** — album/photo album style
- **Collage** — multi-photo collage layout
- **Spin** — rotation effect
- **Frame** — framed photo effect
- **Glass** — glass/reflection effect

### Resource Type Identifiers
```
resource|%s|#10|%s
stream|
```

## Registry Paths

### Slideshow-specific
```
Software\Microsoft\Windows Live\Photo Gallery\Slideshow
Software\Microsoft\Windows Live\Photo Gallery\Slideshow\LastRunSettings
```

### GPU Pipeline
```
Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline
Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipelineInstallLocation
```

### File Support Codecs
```
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions
Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes
```

### Media Foundation
```
Software\Microsoft\Windows Media Foundation\ByteStreamHandlers
```

### Other
```
Software\Microsoft\Windows Live\Common
Software\Microsoft\Windows Live\Environment\PhotoGallery
Software\Microsoft\Windows Live\Photo Gallery\Viewer
SOFTWARE\Microsoft\Windows\CurrentVersion\Media Center\Decoder
Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\%ws
```

## Configuration Keys (from binary strings)
- `PhodeosPerChunk` — batch size for parallel processing
- `PlaybackFramesPerSecond` — playback frame rate
- `DXExclusiveSize` — DirectX exclusive mode size threshold
- `UseVSync` — vertical sync toggle
- `RunInDXEMode` — DirectX exclusive mode toggle
- `MaxRenderSizeWidth` / `MaxRenderSizeHeight` — render resolution caps
- `DesktopResolutionWidth` / `DesktopResolutionHeight` — display resolution
- `ActiveThemeFriendlyName` — current active theme name
- `VertexShaderMajorVersion` / `VertexShaderMinorVersion` — GPU vertex shader version
- `PixelShaderMajorVersion` / `PixelShaderMinorVersion` — GPU pixel shader version
- `SoftwareRenderingEnabled` — software/GDI fallback toggle
- `CurrentThemeSimpleSlideshowNone` — special "None" theme sentinel
- `EnabledThemeSet` — theme set toggle
- `EnabledThemeName%d` — per-index theme name

## RTTI Classes (156 total)

### Core Slideshow COM Objects (with ATL `CComCoClass`)
| Class | Registered? |
|-------|------------|
| `SimpleSlideshowDisplay` | **Yes** (`CLSID_SimpleSlideshowDisplay`) |
| `SlideshowExtension` | **Yes** (`CLSID_SlideshowExtension`) |
| `TimelineDisplay` | **Yes** (`CLSID_TimelineDisplay`) |
| `SlideshowFrameWindow` | No (internal) |
| `SlideshowChunkIterator` | No |
| `SlideshowList` | No |
| `SlideshowListIterator` | No |
| `SlideshowModeControl` | No |
| `SlideshowUserInterface` | No |
| `ThemeIterator` | No |
| `ThemeManager` | No |
| `TimelineCanvasView` | No |
| `TimelineNavigationBar` | No |
| `PaintContextHWnd` | No |
| `PrintContextHWnd` | No |
| `SupportedFileTypes` | No |

### Application Logic Classes
- **Display/Rendering:** `SimpleSlideshowDisplay`, `SlideshowBackgroundWindow`, `SlideshowFrameWindow`, `TimelineDisplay`, `TimelineCanvasView`
- **Slideshow Control:** `SlideshowModeControl`, `SlideshowUserInterface`, `SlideshowExtension`
- **Data/Iteration:** `SlideshowList`, `SlideshowListIterator`, `SlideshowChunkIterator`
- **Themes:** `ThemeManager`, `ThemeIterator`
- **Timeline UI:** `TimelineNavigationBar`, `TimelineNavigationButton`, `TimelineNavigationMenu`, `TimelineNavigationMenuItemButton`, `TimelineNavigationStatic`, `TimelineNavigationStretchButton`, `TimelineNavigationBitmapOnlyButton`, `TimelineNavigationControlBase`, `TimelineErrorHandler`, `TimelineFileHandler`, `TimelineFileLoader`, `TimelineBuilder`
- **Painting/Graphics:** `PaintContextHWnd`, `PrintContextHWnd`, `BufferedPainter`, `BufferedPainterBase`, `SharedBitmapWrapper`, `ThumbnailCacheWrapper`, `ThumbCachePopulater`
- **File Support:** `SupportedFileTypes`
- **Accessibility:** `AccessibleProxy<SlideshowFrameWindow>`, `EnumVariantTearOff<SlideshowFrameWindow>`
- **Threading:** `Thread@Base`
- **SQM Telemetry:** `SqmStartupWrapper`
- **GDI+:** `GdiplusStartupWrapper`

## Function Categories

### 1. COM Infrastructure
- `DllCanUnloadNow`, `DllGetClassObject`, `DllRegisterServer`, `DllUnregisterServer`
- ATL `CComModule` / `CAtlModuleT` initialization
- COM object creation via `CComCoClass` / `CComObject` template instantiations

### 2. Slideshow Playback & Control
- Play/Pause/Next/Back/Exit command handling
- Speed control (Slow/Medium/Fast)
- Shuffle/Loop modes
- Frame controller (`IFrameControllerCommon`, `ISlideshowFrameController`)
- Mode control (`IModeControlSlideshowSettings`)

### 3. Slideshow Rendering Pipeline
- Direct3D 9 rendering (via `d3d9.dll` and `d3dx9_32.dll`)
- GDI+ bitmap processing (GdipDrawImage*, GdipBitmapLockBits, etc.)
- GPU pipeline support with shader version detection
- Software rendering fallback via GDI
- Buffered painter pattern (`BufferedPainter`, `BufferedPainterBase`)

### 4. Theme/Effect Engine
- 9 built-in themes: Fade, Black and white, Sepia, Classic, Album, Collage, Spin, Frame, Glass
- Theme manager/iterator pattern
- Theme event sink for change notifications
- Active theme persistence via registry

### 5. UI & Window Management
- WTL frame window (`CFrameWindowImpl<SlideshowFrameWindow>`)
- Timeline display window with canvas view
- Navigation bar with buttons, menus, stretch controls
- Subclassed background window

### 6. Photo List Management
- List/linked-list implementation for slides
- Chunked iteration for parallel processing
- Filtered iteration (subset selection)

### 7. Registry & Configuration
- Extensive registry access for settings persistence
- Last-run settings save/restore
- GPU pipeline install location
- Codec/file type support configuration

### 8. DirectUI Integration (UXCore)
- CRMStringResource for localized UI strings
- Resource set management

### 9. Telemetry
- **SQM** (WLXPhotoSqm): `Startup`, `Shutdown`, `AddToStream`
- **BICI** (DmxBici): `AddStringToDataPoint`, `TransferExperienceToWeb`
- **ETW Tracing** (ADVAPI32): `RegisterTraceGuids`, `TraceEvent`, `TraceMessage`

### 10. Thumbnail & Bitmap Cache
- `ThumbnailCacheWrapper` — cached bitmap storage
- `ThumbCachePopulater` — async thumbnail generation
- `SharedBitmapWrapper` — shared bitmap references

## Graphics Architecture Summary

The slideshow uses a hybrid rendering approach:

1. **Primary: Direct3D 9** — Hardware-accelerated rendering via `d3d9.dll` + `d3dx9_32.dll` sprite/font system
2. **Fallback: GDI+** — Software rendering via `gdiplus.dll` (25 functions) and `GDI32.dll` (13 functions)
3. **GPU Pipeline check** — Detects shader version support from registry config
4. **DX Exclusive Mode** — Configurable fullscreen exclusive mode

The `SlideshowFrameWindow` is a full WTL `CFrameWindowImpl` with accessibility support (UI Automation via `OLEACC.dll`), integrated with the DWM (desktop window manager) for glass/transparency effects.
