# wlxclip.dll — Reverse Engineering Analysis

## Overview
| Property | Value |
|----------|-------|
| **File** | `undecomp\Photo Gallery\wlxclip.dll` |
| **PDB** | `WLXCLIP.pdb` (GUID `FAAEEF1A-315F-49A7-89F2-0902F3319358`) |
| **Machine** | x86 (PE32) |
| **Timestamp** | Tue Apr 1 01:27:45 2014 |
| **Subsystem** | Windows CUI (3) |
| **Image Base** | `0x10000000` |
| **Image Size** | `0x46000` (286 KB) |
| **Code Size** | `0x31600` (~198 KB) |
| **Linker** | MSVC 11.00 (VS2012) |
| **DLL Characteristics** | Dynamic base, NX compatible |

## Exports (6)
| Ordinal | Hint | RVA | Name | Purpose |
|---------|------|-----|------|---------|
| 1 | 4 | `0xA9D1` | **VMGGetClipCreateFunctions** | Returns function pointers for clip creation (returns HRESULT error `0x80004005` when called with no context) |
| 2 | 0 | `0xAE71` | **DllCanUnloadNow** | Standard COM — returns `S_OK` (0x00000000) when called |
| 3 | 1 | `0xAE82` | **DllGetClassObject** | Standard COM class factory — requires valid CLSID |
| 4 | 2 | `0xAF56` | **DllRegisterServer** | Self-registration |
| 5 | 3 | `0xB003` | **DllUnregisterServer** | Self-unregistration |
| 6 | 5 | `0xB0C4` | **VMGShellThumbnailGeneratorW** | Shell thumbnail generator — accepts source/target paths, returned without crash on test args |

## DLL Imports (14 libraries)
| Library | Key Functions | Role |
|---------|--------------|------|
| **MSVCR110.dll** | malloc, free, memcpy, etc. | CRT runtime |
| **SHELL32.dll** | SHParseDisplayName, ShellExecuteW, SHFileOperationW, SHCreateShellItem, SHBindToParent, CommandLineToArgvW | Shell namespace operations, file operations |
| **KERNEL32.dll** | CreateFileW, GlobalAlloc/Lock/Unlock, OpenFileMappingW, MapViewOfFile, GetTempPathW, CreateThread, etc. | File I/O, memory mapping, temp files, threading |
| **USER32.dll** | GetSystemMetrics, MessageBoxW, **RegisterClipboardFormatW** | Clipboard format registration |
| **ADVAPI32.dll** | RegCreateKeyExW, RegQueryValueExW, RegSetValueExW, RegDeleteValueW | Registry (COM registration) |
| **GDI32.dll** | DeleteObject, GetObjectW | GDI object management |
| **ole32.dll** | OleInitialize, CoCreateInstance, **OleGetClipboard**, **OleSetClipboard**, CreateStreamOnHGlobal, ReleaseStgMedium, CoTaskMemAlloc/Free | OLE clipboard operations, COM |
| **OLEAUT32.dll** | Ordinals 2,4,6,7,8,9,12,149,150,313 | VARIANT, SysAllocString, etc. |
| **SHLWAPI.dll** | PathFindFileNameW, PathAppendW, PathFindExtensionW, SHDeleteKeyW | Path utilities |
| **gdiplus.dll** | GdipCreateBitmapFromHBITMAP, GdipBitmapLockBits, GdipSaveImageToStream, GdipImageRotateFlip, GdipCreateBitmapFromFile, GdipGetImageEncoders | GDI+ image encoding/manipulation |
| **WMVCore.DLL** | **WMCreateReader**, **WMCreateEditor** | Windows Media (ASF/WMV) reading/editing |
| **RPCRT4.dll** | UuidCreate | UUID generation |
| **VERSION.dll** | GetFileVersionInfoSizeW, GetFileVersionInfoW, VerQueryValueW | Version info |
| **urlmon.dll** | CreateFormatEnumerator | Format enumeration for media |
| **msi.dll** | Ordinal 90 | MSI product info |

## RTTI Class Hierarchy (64 classes)

### Core COM Infrastructure
- `CComObjectRootBase@ATL@@` — ATL base
- `CComObjectRootEx<CComSingleThreadModel>` — STA
- `CComObjectRootEx<CComMultiThreadModel>` — MTA
- `CUnknown`, `INonDelegatingUnknown`, `CBaseObject`
- `CCritSec` — Critical section wrapper

### Clipboard/OLE Container System
- **`CVMGClipContainer`** — Primary OLE item container (CLSID: `CLSID_VMGIOleItemContainer`)
  - Implements: `IOleItemContainer`, `IOleContainer`, `IParseDisplayName`
  - Contains `CVMGPixCommands` (CLSID: `CLSID_VMGPixCommands`) — pixel command interface (`IVMGPixCommands`)
  - Implements `IPersistFile`, `IPersist`, `IVMGEditSupport`

### Media Clip Management
- **`CVMGClip`** — Individual media clip
  - `CVMGClipTimeBlock` — Time block within a clip
  - Implements: `IVMGClip`, `IVMGClipInternal`, `IVMGClipAutoMoviedata`
  - Implements: `IExtractImage`, `IPropertyStore`, `IShellItem2`, `IShellItem`
- **`CVMGClipCollection`** — Collection of clips
  - `CIEnumClips<CVMGClipCollection>` — Enumerator
  - Implements: `IVMGClipCollection`, `IVMGClipCollectionInternal`, `IVMGAsyncErrors`
- **`CPropertyStoreSampleStatus`** — Property store for sample status

### Media Source/File Layer
- **`CVMGSourceManager`** — Media source manager (`IVMGSourceManager`)
- **`CVMGMediaSourceFile`** — Media file abstraction
  - Implements: `IVMGMediaSourceFile`, `IVMGMediaSourceFileInternal`, `IVMGMediaFileProperties`
- **`CVMGUserData`**, **`CVMGMediaPlayerData`** — User/player data containers
- **`CVMGTask`** — Task abstraction
- `IEnumUnknown` — COM enumerator

### Drag/Drop and OLE Data Transfer
- `IDataObject` — OLE data object (clipboard format)
- `IDropSource` — OLE drag-drop source

### Media Processing / DirectShow Filters
- **`CMpeg2Filter`** — MPEG-2 decoder filter
  - `CStaticObject<CMpeg2Filter>` — Static singleton
  - Implements: `IAMGraphBuilderCallback`, `IBaseFilter`, `IMediaFilter`, `IAMovieSetup`
- **`CVMGFilterSafetyControl`** — Filter safety (`IVMGFilterSafetyControl`)
- `IAMGraphBuilderCallback` — Graph builder callback

### DirectShow Pins / Buffers
- `CVMuxInputPin` — Multiplex input pin
- `CRenderedInputPin` → `CBaseInputPin` → `CBasePin` → `IPin`, `IQualityControl`, `IMemInputPin`
- `CMediaBuffer` — Media buffer (`IMediaBuffer`)
- `CStillImageSampleINSSBuffer` — Still image wrapped as NSS buffer
- `CVMGMediaSampleINSSBuffer` — Video clip sample as NSS buffer

### Media Playback / Seeking
- `CMediaPosition` → `IDispatch`, `IMediaSeeking`, `IMediaPosition`
- `CPosPassThru`, `CRendererPosPassThru` — Position pass-through
- `CEnumPins`, `CEnumMediaTypes` — DirectShow enumerators

### Scene Detection / ToC
- **`CVMGInlineSceneDetection`** — Inline scene detection (`IVMGInlineSceneDetection`)
- **`CVMGTocHelper`** — Table of contents helper (`IVMGTocHelper`)
- **`CVMGSampleSinker`** — Sample sink (`IVMGSampleSink`, `IVMGSampleProcessor`, `IVMGSampleStatus`)
- **`CVMGMinimalProgressIndicator`** — Progress indicator (`IVMGProgressIndicator`)

### Image / File Helpers
- **`CVMGWICImageHelper`** — WIC-based image handling
- **`CVMGStillImageHelper`** — Still image extraction
- **`CVMGAVIHelper`** — AVI file handling
- **`CVMGWMHelper`** — Windows Media (ASF/WMV) handling
  - Implements: `IWMReaderCallback`, `IWMStatusCallback`
- **`CVMGWMMetaDataHelper`** — Windows Media metadata
- **`CGetFrameHelper`** — Frame grabber (`IVMGSampleGrabber`)

## Clipboard Operations

The DLL's primary purpose is **clipboard operations for media clips**:

1. **`OleGetClipboard`** / **`OleSetClipboard`** — Standard OLE clipboard access
2. **`RegisterClipboardFormatW`** — Registers custom clipboard formats
3. **`IDataObject`** / **`IDropSource`** — Full drag-drop support
4. **`CreateStreamOnHGlobal`** — Memory-based clipboard data streams
5. **`CreateFormatEnumerator`** (urlmon) — Clipboard format enumeration
6. **`IVMGClipContainer`** as `IOleItemContainer` — Items available via OLE enumeration

## Shell Thumbnail Generation

**`VMGShellThumbnailGeneratorW`** accepts:
- Source path (media file)
- Output path (thumbnail image)
- Uses GDI+ for image manipulation (`GdipCreateBitmapFromFile`, `GdipImageRotateFlip`, `GdipSaveImageToStream`)

## Video Format Support

Extracted strings show pixel format conversion tables:
- `I420`, `IYUV`, `YV12`, `NV12`, `YUY2`, `UYVY`, `YVYU`, `YVU9`, `P411`, `Y41P`, `Y41T`, `Y42T`
- `WICConvertBitmapSource` — WIC format conversion
- Registry key: `"Vertical Flip Packed YUV"` mode for WMV encoding

## Test Harness Results
| Export | Status | Notes |
|--------|--------|-------|
| DllCanUnloadNow | **OK** → S_OK | DLL reports it can unload |
| DllGetClassObject | SEH `0xC0000005` | Expected — no COM init, null CLSID |
| DllRegisterServer | Skipped | Would write registry entries |
| DllUnregisterServer | Skipped | Would remove registry entries |
| VMGGetClipCreateFunctions | **OK** → `0x80004005` | Returns E_FAIL without context |
| VMGShellThumbnailGeneratorW | **OK** | No crash with test args |
