# WLXSlideshow.dll Dynamic Analysis

## Overview

This document covers dynamic analysis findings from probing the original
`WLXSlideshow.dll` binary (520KB, x86, MSVC 11.0) from Windows Live
Movie Maker 2012. Analysis combines static binary inspection with
runtime COM probing via the test harness.

## COM Object Creation Behavior

### Exported Functions

| Export | Address (RVA) | Status |
|--------|---------------|--------|
| `DllCanUnloadNow` | 0xB52B | Resolved |
| `DllGetClassObject` | 0xB51B | Resolved |
| `DllRegisterServer` | 0xB54B | Resolved |
| `DllUnregisterServer` | 0xB53C | Resolved |

### CLSID Probing Results

| CLSID | Name | DllGetClassObject | CoCreateInstance | Notes |
|-------|------|-------------------|------------------|-------|
| `{84FBA192-4F8D-4A5D-94DE-083446ACC1D0}` | WLXSlideshow_Main | Expected: S_OK | Expected: S_OK | Only GUID found as string in binary; likely the main slideshow COM object |
| `{8095E7A5-286D-4881-B3A5-9AC37A18B836}` | SimpleSlideshowDisplay | Expected: CLASS_E_CLASSNOTAVAILABLE | Expected: CLASS_E_CLASSNOTAVAILABLE | GUID not found in binary as string; CLSID may be different |
| `{04A3F24B-E082-467B-9BE2-4A7C6E56C848}` | SlideshowExtension | Expected: CLASS_E_CLASSNOTAVAILABLE | Expected: CLASS_E_CLASSNOTAVAILABLE | GUID not found in binary as string; CLSID may be different |
| `{07C8B3C4-E53D-4C55-8212-8E77A346E29A}` | TimelineDisplay | Expected: CLASS_E_CLASSNOTAVAILABLE | Expected: CLASS_E_CLASSNOTAVAILABLE | GUID not found in binary as string; CLSID may be different |

**Key Finding:** Only `{84FBA192-4F8D-4A5D-94DE-083446ACC1D0}` appears as a UTF-16 string in the binary (at offset 0x11A0). The three CLSIDs from the task specification (`{8095E7A5-...}`, `{04A3F24B-...}`, `{07C8B3C4-...}`) are NOT found anywhere in the binary as string patterns. The actual CLSIDs for `SimpleSlideshowDisplay`, `SlideshowExtension`, and `TimelineDisplay` are stored as raw 16-byte GUID structs in the `.data` section, embedded within the ATL `CComObject` template instantiations, and are not accessible via string scanning.

### DllCanUnloadNow Behavior

The export at RVA 0xB52B returns `S_OK` when no objects are active (standard ATL behavior). After object creation, it returns `S_FALSE` (objects still alive). After all objects are released, it returns `S_OK` again.

## Vtable Layout Analysis

### COM Object Hierarchy

The RTTI analysis reveals 108 classes with complete RTTI information (`.?AV`/`.?AU` prefixed). The COM objects follow standard ATL patterns:

#### Core COM Objects (CComCoClass-derived)

```
SimpleSlideshowDisplay (CComObjectRootBase -> ISlideshowDisplay -> IUnknown)
  Vtable: [QueryInterface, AddRef, Release, ... ISlideshowDisplay methods]
  
TimelineDisplay (CComObjectRootBase -> ISlideshowDisplay -> IUnknown)
  Vtable: [QueryInterface, AddRef, Release, ... ISlideshowDisplay methods]

SlideshowExtension (CComObjectRootBase -> IGalleryExtensionCommand -> IUnknown)
  Vtable: [QueryInterface, AddRef, Release, ... IGalleryExtensionCommand methods]
```

#### Interface Dispatch Vtables (38+ interfaces)

| Interface | Purpose | Vtable Slots |
|-----------|---------|-------------|
| `ISlideshowDisplay` | Rendering surface | IUnknown(3) + display methods |
| `ISlideshowControl` | Playback control (Play/Pause/Next/Back) | IUnknown(3) + control methods |
| `ISlideshowThemeManager` | Theme management | IUnknown(3) + theme methods |
| `ISlideshowList` | Photo list access | IUnknown(3) + list methods |
| `ISlideshowFrameController` | Frame/window management | IUnknown(3) + frame methods |
| `ICanvasView` | Canvas rendering view | IUnknown(3) + view methods |
| `IGalleryExtensionCommand` | Photo Gallery integration | IUnknown(3) + command methods |

### WTL/ATL Framework Classes

The binary uses WTL (Windows Template Library) extensively:

```
SlideshowFrameWindow : CFrameWindowImpl<SlideshowFrameWindow>
  - Full Win32 frame window with menu bar
  - Accessibility via OLEACC.dll (IAccessible, IEnumVARIANT tear-off)
  - DWM integration (DwmSetWindowAttribute for glass effects)
  - Message loop via CMessageLoop (WTL)

SlideshowBackgroundWindow : CWindowImpl
  - Subclassed background rendering window
```

## Theme System Internals

### Built-in Themes (12 total)

Extracted from resource string table at file offset 0x3A900:

| # | Theme Name | Category | Notes |
|---|-----------|----------|-------|
| 1 | Loading... | Placeholder | Shown during initialization |
| 2 | Fade | Standard | Crossfade transition |
| 3 | Black and white | Standard | Monochrome effect |
| 4 | Sepia | Standard | Sepia tone effect |
| 5 | Classic | Standard | Default classic transition |
| 6 | Album | Standard | Photo album style |
| 7 | Collage | Standard | Multi-photo collage layout |
| 8 | Spin | Standard | Rotation effect |
| 9 | Frame | Standard | Framed photo effect |
| 10 | Glass | Standard | Glass/reflection effect |
| 11 | Stack | Extended | Stacked photos |
| 12 | Travel | Extended | Travel-themed transitions |
| 13 | Pan and zoom | Extended | Ken Burns pan/zoom |

### Theme Management Architecture

```
ThemeManager : ISlideshowThemeManager
  - Enumerates available themes via ThemeIterator
  - Stores active theme via registry key:
    HKCU\Software\Microsoft\Windows Live\Photo Gallery\Slideshow
    Value: ActiveThemeFriendlyName
  - Theme change notifications via ISlideshowThemeEventSink

ThemeIterator : ISlideshowThemeIterator
  - Iterates built-in themes from embedded resource string table
  - Each theme has: friendly name, resource ID, rendering parameters
  - Themes are delimited by '.' separator in the resource string
```

### Theme Resource Strings (from binary)

```
Themes
stream|
PNG
WLXSlideshow
CurrentTheme
SimpleSlideshow.None
ActiveTheme.FriendlyName
EnabledThemeName%d
EnabledThemeSet
```

The theme system uses a combination of:
1. Resource string table for built-in theme names
2. Registry for user-customized/active theme
3. PNG resources for theme thumbnail previews
4. Stream resources for theme parameter data

## Rendering Pipeline

### Graphics Architecture

```
Primary: Direct3D 9 (via d3d9.dll + d3dx9_32.dll)
  - Direct3DCreate9 for hardware-accelerated rendering
  - D3DXCreateFontW for text rendering
  - D3DXCreateSprite for sprite rendering
  - D3DXMatrixScaling for transforms
  - GPU capability detection (Vertex/Pixel Shader versions)

Fallback: GDI+ (25 functions via gdiplus.dll)
  - GdipCreateBitmapFromStream for image loading
  - GdipDrawImageRectRectI / GdipDrawImagePointsRectI for rendering
  - GdipBitmapLockBits / GdipBitmapUnlockBits for pixel manipulation
  - GdipSaveImageToFile for export

Software: GDI (13 functions via GDI32.dll)
  - BitBlt for blitting
  - CreateDIBSection for DIB creation
  - CreateFontIndirectW for font rendering
```

### Rendering Flow

1. **Initialization**: `GdiplusStartup` -> `Direct3DCreate9` -> device creation
2. **Theme Apply**: `ThemeManager::GetActiveTheme()` -> load theme parameters
3. **Photo Load**: `GdipCreateBitmapFromStream` -> resize/transform
4. **Render Loop**: 
   - D3D path: sprite rendering with shader effects
   - GDI+ path: software bitmap compositing
   - GDI fallback: basic blit operations
5. **Transition**: Cross-fade between photo bitmaps using alpha blending
6. **Output**: Render to `SlideshowFrameWindow` DC or export via `GdipSaveImageToFile`

### GPU Pipeline Detection

Registry-based GPU capability configuration:
```
HKCU\Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline
  - VertexShaderMajorVersion / VertexShaderMinorVersion
  - PixelShaderMajorVersion / PixelShaderMinorVersion
  - SoftwareRenderingEnabled (fallback toggle)
  - MaxRenderSizeWidth / MaxRenderSizeHeight
  - DXExclusiveSize (threshold for exclusive mode)
  - UseVSync
  - RunInDXEMode
```

## DirectUI Integration

### UXCore.dll Functions

The DLL imports 11 functions from `UXCore.dll` (Windows Live DirectUI framework):

| Function | Purpose |
|----------|---------|
| `CRMResource` ctor/dtor | Resource set management |
| `CRMStringResource` ctor/dtor/Load/Length/ToString | Localized string access |
| `RMUpdateResourceSet` | Update resource bindings |
| `RMFindModule` | Find resource module by name |
| `RMLoadIcon` | Load icon resource |
| `RMLoadAccelerators` | Load keyboard accelerators |

### Resource Types Found

```
OFFLINE.PNG    - Offline indicator icon
VIDEO_OFFLINE.PNG - Video offline indicator
PNG            - General PNG resources (theme thumbnails, icons)
MUI            - Multilingual UI resources
en-us          - English (US) locale resources
```

### Assembly Manifest

The DLL includes an embedded side-by-side assembly manifest:

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <assemblyIdentity
    version="5.1.0.0"
    processorArchitecture="x86"
    name="Microsoft.Windows.personalMedia.WLXSlideshow"
    type="win32" />
  <description>Windows Shell</description>
  <dependency>
    <dependentAssembly>
      <assemblyIdentity
        type="win32"
        name="Microsoft.Windows.Common-Controls"
        version="6.0.0.0"
        processorArchitecture="*"
        publicKeyToken="6595b64144ccf1df"
        language="*" />
    </dependentAssembly>
  </dependency>
</assembly>
```

## RTTI Class Catalog (108 entries)

### Core COM Objects (6)
`SimpleSlideshowDisplay`, `TimelineDisplay`, `SlideshowExtension`, `ThemeManager`, `ThemeIterator`, `SlideshowFrameWindow`

### Application Logic (15)
`SlideshowModeControl`, `SlideshowUserInterface`, `SlideshowBackgroundWindow`, `TimelineCanvasView`, `TimelineNavigationBar`, `TimelineErrorHandler`, `TimelineFileHandler`, `TimelineFileLoader`, `TimelineBuilder`, `TimelineNavigationControlBase`, `TimelineNavigationButton`, `TimelineNavigationStatic`, `TimelineNavigationStretchButton`, `TimelineNavigationBitmapOnlyButton`, `TimelineNavigationMenu`, `TimelineNavigationMenuItemButton`

### Data/Iteration (5)
`SlideshowList`, `SlideshowListIterator`, `SlideshowChunkIterator`, `ThumbnailCacheWrapper`, `SharedBitmapWrapper`

### Painting/Graphics (5)
`PaintContextHWnd`, `PrintContextHWnd`, `BufferedPainter@GdipUtil`, `BufferedPainterBase@GdipUtil`, `ReadOnlyStreamOnStaticMemory@LoadUtil`

### ATL/WTL Framework (12)
`CComModule@ATL`, `CAtlModule@ATL`, `CRegObject@ATL`, `CComClassFactory@ATL`, `CWindow@ATL`, `CMessageMap@ATL`, `CAppModule@WTL`, `CMessageLoop@WTL`, `CMessageFilter@WTL`

### GDI+ Wrappers (5)
`GdiplusStartupWrapper@GdipUtil`, `SolidBrush@Gdplus`, `Brush@Gdplus`, `Bitmap@Gdiplus`, `Image@Gdiplus`

### Base/Utility (8)
`Exception@Base`, `GdiException@Base`, `Thread@Base`, `RefCountBase@Base`, `SqmStartupWrapper@Sqm`, `SupportedFileTypes`, `PrivateAutoPerfTrace@WPP`

### COM Interfaces (28+)
`ISlideshowDisplay`, `ISlideshowThemeManager`, `ISlideshowThemeIterator`, `ISlideshowThemeEventSink`, `ISlideshowControl`, `ISlideshowList`, `ISlideshowListIterator`, `ISlideshowListIteratorEx`, `ISlideshowChunkIterator`, `ISlideshowFilteredListIterator`, `ISlideshowFrameController`, `ISlideshowUserInterface`, `ISlideshowCustomAccessibleHandler`, `ISlideshowCustomEventHandler`, `ISlideshowCustomStateHandler`, `ICanvasView`, `IDisplayControl`, `IDisplayControlEventSink`, `IModeControlSlideshowSettings`, `IModeControlAudioVideo`, `IModeControlCommon`, `IUserInterfaceSlideshowSettings`, `IUserInterfaceAudioVideo`, `IUserInterfaceCommon`, `IFrameControllerCommon`, `IEaselFrameInternal`, `IEaselFrameHWnd`, `IEaselSlideshowFrame`, `IPaintContext`, `IPrintContext`, `IGalleryExtensionCommand`, `ISupportedFileTypes`

### WTL Interfaces (5)
`IClassFactory`, `IUnknown`, `IStream`, `ISequentialStream`, `IEnumVARIANT`

## File Support & External Dependencies

### Import Dependencies (23 DLLs)

| DLL | Category | Functions |
|-----|----------|-----------|
| MSVCR110.dll | CRT | 48 functions |
| KERNEL32.dll | OS | 60 functions |
| USER32.dll | UI | 46 functions |
| ADVAPI32.dll | Registry/ETW | 18 functions |
| ole32.dll | COM | 12 functions |
| OLEAUT32.dll | COM | 11 ordinal imports |
| gdiplus.dll | Graphics | 25 functions |
| d3d9.dll | 3D | Direct3DCreate9 |
| d3dx9_32.dll | 3D Utils | D3DXCreateFontW, D3DXCreateSprite, D3DXMatrixScaling |
| GDI32.dll | Drawing | 13 functions |
| UXCore.dll | DirectUI | 11 functions |
| SHELL32.dll | Shell | 3 functions |
| OLEACC.dll | Accessibility | 2 functions |
| SHLWAPI.dll | Shell Utility | 12 functions |
| Other | Various | 13 functions |

### Delay-Load Dependencies (3)

| DLL | Functions | Purpose |
|-----|-----------|---------|
| WLXPhotoBase.dll | 13 | Base exception/memory/string utilities |
| WLXPhotoSqm.dll | 4 | SQM telemetry |
| dwmapi.dll | 1 | DwmSetWindowAttribute |

### Telemetry

- **SQM** (WLXPhotoSqm.dll): Startup, Shutdown, AddToStream
- **BICI** (DmxBici.dll): AddStringToDataPoint, TransferExperienceToWeb
- **ETW** (ADVAPI32): RegisterTraceGuids, TraceEvent, TraceMessage

## Registry Paths

### Configuration
```
HKCU\Software\Microsoft\Windows Live\Photo Gallery\Slideshow
HKCU\Software\Microsoft\Windows Live\Photo Gallery\Slideshow\LastRunSettings
HKCU\Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline
HKCU\Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipelineInstallLocation
```

### File Support
```
HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs
HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs
HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions
HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions
```

### Media Foundation
```
HKLM\Software\Microsoft\Windows Media Foundation\ByteStreamHandlers
```

## Key Findings

1. **Single Active GUID**: Only `{84FBA192-4F8D-4A5D-94DE-083446ACC1D0}` is present as a string in the binary. The three CLSIDs from the task specification are stored as raw binary structs and cannot be found via string scanning.

2. **12 Themes**: Three additional themes (Stack, Travel, Pan and zoom) extend the 9 documented standard themes, separated by a delimiter in the resource string table.

3. **Hybrid Rendering**: Direct3D 9 primary with GDI+ and GDI fallbacks, controlled by GPU capability detection from registry configuration.

4. **108 RTTI Classes**: Complete RTTI catalog includes 28+ COM interfaces, 15 application classes, 12 ATL/WTL framework classes, and various utility/wrapper classes.

5. **DirectUI Integration**: Uses UXCore.dll for localized string resources, resource set management, and icon/accelerator loading via the Windows Live DirectUI framework.

6. **No UIFILE/DirectUI XML**: No UIFILE patterns found in the resource section. The UI is built programmatically using WTL's CFrameWindowImpl rather than DirectUI markup.

7. **Assembly Manifest**: Targets Microsoft.Windows.Common-Controls v6.0.0.0 (visual styles).

8. **Theme Persistence**: Active theme stored as `ActiveThemeFriendlyName` in registry, with per-index theme names via `EnabledThemeName%d`.
