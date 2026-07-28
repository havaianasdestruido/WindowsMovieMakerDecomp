# WLXPhotoViewer.dll — "Easel" Photo Viewer

## Overview
- **File**: `undecomp\Photo Gallery\WLXPhotoViewer.dll`
- **Size**: 1.4 MB
- **Architecture**: PE32 x86 (32-bit)
- **Compiler**: MSVC (Visual Studio 2012/2013, MSVCR110.dll)
- **Timestamp**: Tue Apr 1 01:25:13 2014
- **PDB**: `WLXPhotoViewer.pdb` (GUID: `{10656E54-E0D2-4B9C-BC3D-8E3F67E7F8B8}`)
- **Internal codename**: **Easel** (export section header says "exports for Easel.dll")

## Exports (6)
| Ordinal | Hint | Name | Signature |
|---------|------|------|-----------|
| 1 | 0 | `DllCanUnloadNow` | `HRESULT __stdcall()` |
| 2 | 1 | `DllGetClassObject` | `HRESULT __stdcall(REFCLSID, REFIID, LPVOID*)` |
| 3 | 2 | `DllRegisterServer` | `HRESULT __stdcall()` |
| 4 | 3 | `DllUnregisterServer` | `HRESULT __stdcall()` |
| 5 | 4 | `LaunchViewer` | `void __stdcall(int argc, const wchar_t** argv)` |
| 6 | 5 | `LaunchViewerComServer` | `void __stdcall(int argc, const wchar_t** argv)` |

Standard 4 COM DLL exports + 2 non-COM launch entry points. `LaunchViewer` launches the viewer directly; `LaunchViewerComServer` launches via COM activation.

## RTTI Classes (308)

### Photo Editing Actions (22)
| Class | Purpose |
|-------|---------|
| `AdjustColorAction` | Color adjustment |
| `AdjustExposureAction` | Exposure adjustment |
| `AutoColorAction` | Auto color fix |
| `AutoExposureAction` | Auto exposure fix |
| `AutoFixAction` | Auto fix (combined) |
| `BlackAndWhiteAction` | B&W conversion |
| `BlackPointAction` | Black point adjustment |
| `BlemishRemovalAddAction` | Blemish removal |
| `BrightnessAction` | Brightness adjustment |
| `ColorTemperatureAction` | Color temperature |
| `ColorTintAction` | Color tint |
| `ContrastAction` | Contrast adjustment |
| `CropAction` | Crop operation |
| `DenoiseAction` | Noise reduction |
| `FloatPhotoAction` | Floating-point photo op |
| `HighlightsAction` | Highlights adjustment |
| `MaskAction` | Mask editing |
| `RedEyeAddAction` | Red-eye removal |
| `SaturationAction` | Saturation adjustment |
| `ShadowsAction` | Shadows adjustment |
| `SharpenAction` | Sharpening |
| `StraightenAction` | Straighten rotation |
| `WhitePointAction` | White point adjustment |

### Document/Viewer Architecture
| Class | Purpose |
|-------|---------|
| `PhotoDocument` | Main photo document |
| `PhotoDocumentFrame` | Document frame window |
| `PhotoDocumentLoadWorker` | Async photo loading |
| `PhotoDocumentSaveWorker` | Async photo saving |
| `PhotoDocumentSaveAsWorker` | Save As operation |
| `PhotoEditedState` | Tracks editing state |
| `PhotoEditedStateAutoFixWorker` | Auto-fix background worker |
| `PhotoEditedStateBlemishWorker` | Blemish background worker |
| `PhotoEditedStateDenoiseWorker` | Denoise background worker |
| `PhotoEditedStateMaskActionWorker` | Mask background worker |
| `PhotoEditedStateRedEyeWorker` | Red-eye background worker |
| `VideoDocument` | Video document |
| `VideoEditTaskPane` | Video editing pane |

### UI Framework — DirectUI / UXCore
| Class | Purpose |
|-------|---------|
| `HWNDElement@DirectUI` | HWND-backed DirectUI element |
| `NativeHWNDHost@DirectUI` | Native HWND hosting |
| `FillLayout@DirectUI` | Fill layout |
| `VirtualListView` | Virtual list view |
| `Button@DirectUI` | DirectUI button |
| `HcdpiParser@DuiUtil` | DPI-aware parser |
| `HcdpiResourceCache@DuiUtil` | DPI resource cache |

### Task Pane System
| Class | Purpose |
|-------|---------|
| `TaskControl` | Generic task control |
| `TaskPaneBase` | Base task pane |
| `TaskButtonBase` / `TaskButtonEx` | Task buttons |
| `TaskSlider` / `TaskSliderCtrl` | Slider controls |
| `TaskCollectionControl` | Collection control |
| `PhotoEditTaskPane` | Photo edit pane |
| `VideoEditTaskPane` | Video edit pane |
| `PhotoTranscodeTaskPane` | Transcode pane |

### Easel/Phodeo System
| Class | Purpose |
|-------|---------|
| `ShellEasel` | Main Easel COM shell handler |
| `ShellPhodeo` | Photo slideshow |
| `ShellVideoPhodeo` | Video slideshow |
| `ShellImagePhodeo` | Image slideshow |
| `EaselCropUI` | Crop UI overlay |
| `EaselPhodeoEventSink` | Phodeo event proxy |

### People/Face
| Class | Purpose |
|-------|---------|
| `PeopleUI` | People tagging UI |
| `PersonRegionCreatorUI` | Face region creator |
| `PersonRegionUI` | Face region display |
| `ContactPickerWindow` | Contact picker |
| `ContactPickerListView` | Contact list view |

### Codec/Stream
| Class | Purpose |
|-------|---------|
| `CachedStream` / `StreamCache` | Cached file streams |
| `SimpleCodecEncDecCallbackCore` | Codec encode/decode |
| `FindStreamHelper` | Stream helper |
| `SafeSave` | Atomic save |
| `SmartBackupRead` | Backup read |
| `SupportedFileTypes` | File type registry |

### Modularity / Storyboard
| Class | Purpose |
|-------|---------|
| `CommandBar@ModularWindow` | Modular command bar |
| `ContextMenu@ModularWindow` | Context menu |
| `LightweightFrameWindow` | Lightweight frame |
| `ModBeginElement` / `ModEndElement` | Storyboard elements |
| `SerializationContext` / `SerializationWriter` | Storyboard serialization |

### Other Notable
| Class | Purpose |
|-------|---------|
| `WMPVideoPlayer` | Windows Media Player integration |
| `QuickTimeMoviePlayer` | QuickTime integration |
| `VideoTrimMoviePlayer` | Video trimming |
| `HistogramWorker` | Histogram computation |
| `OriginalImageManager` | Original image cache |
| `ThumbnailCacheWrapper` | Thumbnail cache |
| `SelectionScanner` | Selection scanning |
| `PhotoFuseBehaviorFactory` / `PhotoFuseWindow` | Photo fusion |
| `TweakerClient` / `TweakerDisplay` | Settings tweaker |
| `WordBreaker` / `WordGroup` / `WordwheelWindow` | Text search |
| `ProductStatusChecker` | Product status |
| `ViewerLocalServer` | Local COM server |

## Key Dependencies
- **WLXPhotoBase.dll** — Base library (exceptions, memory, strings)
- **UXCore.dll** — DirectUI framework, visual elements
- **DmxBici.dll** — Telemetry (BICI experiences)
- **gdiplus.dll** — Graphics rendering (GDI+)
- **d3d9.dll** — Direct3D 9 (delay-loaded)
- **WLXPhotoSqm.dll** — SQM telemetry (delay-loaded)
- **dwmapi.dll** — Desktop Window Manager (delay-loaded)
- **WindowsCodecs.dll** — WIC codecs (delay-loaded)
- **MetadataSys.dll** — Metadata handling (delay-loaded)
- **XmlLite.dll** — XML parsing (delay-loaded)
- **msi.dll** — Windows Installer queries (delay-loaded)

## COM Registration
- Registry key: `CLSID_PhotoViewer` (value found at RTTI offset)
- COM server with `DllRegisterServer` / `DllUnregisterServer`
- `ViewerLocalServer` class implements local server activation
- `AutoplayLocalServer` for autoplay integration

## PE Layout
| Section | VA Range | Size | Purpose |
|---------|----------|------|---------|
| `.text` | 0x10001000–0x1010F288 | 10E400h | Code (688 KB) |
| `.data` | 0x10110000–0x10124953 | 13600h | Initialized data (77 KB) |
| `.rsrc` | 0x10125000–0x1013BFF7 | 17000h | Resources (92 KB) |
| `.reloc` | 0x1013C000–0x101599F5 | 1DA00h | Relocations (118 KB) |

## String Constants Found
- Photo editing ops: `AutoFixed`, `Straightened`, `Gamma`, `CustomAdjusted`, `3DRotate`
- UI strings: `RectAction`, `Striaghten`
- Metadata: `Microsoft.Windows.personalMedia.WLXPhotoViewer`
