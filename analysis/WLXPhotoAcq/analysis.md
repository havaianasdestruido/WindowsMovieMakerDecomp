# WLXPhotoAcq.dll Analysis

## Overview

**File:** WLXPhotoAcq.dll (Windows Live Photo Gallery 2012 — Photo Acquisition)
**Role:** Photo acquisition COM DLL for importing photos/videos from cameras, scanners, WPD, and file systems
**Image size:** 0x1B6000 (~1.75 MB)
**PDB:** `WLXPhotoAcq.pdb` `{76FDE0E3-6E26-443A-9D2B-FC70B044BE6C}`
**Build:** 16.4.3528.0331 (timestamp: 2014-04-01 01:26:07)
**Real DLL name:** `PhotoAcq.dll` (exported as PhotoAcq.dll)

## PE Structure

| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Subsystem | Windows GUI (2) |
| Linker | MSVC 11.00 |
| Image Base | 0x10000000 |
| Image Size | 0x1B6000 |
| Entry Point | 0x100814B4 |
| OS/Image Version | 6.02 / 6.02 |
| DLL Characteristics | Dynamic Base (ASLR), NX Compatible |
| Stack reserve/commit | 256 KB / 4 KB |
| Heap reserve/commit | 1 MB / 4 KB |
| Checksum | 0x1B78E0 |

### Sections

| Section | VA | VirtSize | RawSize | Characteristics |
|---------|------|----------|---------|-----------------|
| `.text` | 0x1000 | 0x9F7C5 (637 KB) | 0x9F800 | Code, Execute Read |
| `.orpc` | 0xA1000 | 0x37C (892 B) | 0x400 | Code, Execute Read (ORPC stubs) |
| `.data` | 0xA2000 | 0x6D98 (27 KB) | 0x5E00 | Init Data, Read Write |
| `.rsrc` | 0xA9000 | 0xFDFE8 (1016 KB) | 0xFE000 | Init Data, Read Only |
| `.reloc` | 0x1A7000 | 0xE76C (58 KB) | 0xE800 | Init Data, Discardable, Read Only |

**Key observation**: `.rsrc` section is ~1 MB — contains dialog resources, string tables, icon bitmaps, XMP metadata templates, and COM registry scripts for the photo picker UI.

## Exports (4 functions — standard COM DLL)

| Ordinal | Name | RVA | Purpose |
|---------|------|-----|---------|
| 1 | `DllCanUnloadNow` | 0x1CDEA | COM unload check |
| 2 | `DllGetClassObject` | 0x1CE12 | COM class factory |
| 3 | `DllRegisterServer` | 0x1CE60 | COM self-registration |
| 4 | `DllUnregisterServer` | 0x1CECB | COM unregistration |

Standard ATL COM DLL exports. All photo acquisition logic is accessed through COM interfaces.

## Import Table

### CRT / Runtime
- **MSVCR110.dll** (80+ functions): CRT core — memory (malloc/free/calloc/realloc/memcpy/memset/memmove), SSE2 math (_libm_sse2_cos/sin/exp/log_precise), string (wcscat/wcscpy/wcsstr/swscanf/towlower/towupper), exceptions (CxxThrowException, CxxFrameHandler3), file I/O, qsort/bsearch, locale (_create_locale/_free_locale), threading (_beginthreadex)

### OS
- **KERNEL32.dll** (100+ functions): File I/O (CreateFileW/ReadFile/WriteFile/CopyFileExW/MoveFileExW), module loading (LoadLibraryExW/FreeLibrary/GetModuleHandle), resources (FindResourceW/LoadResource/LockResource), sync (CreateEventW/WaitForSingleObject/CRITICAL_SECTION), threading (CreateThread), volume (GetDriveTypeW/GetVolumeInformationW/GetLogicalDriveStringsW), device (DeviceIoControl), memory (VirtualAlloc/VirtualFree/Heap*), COMPLETION PORT (CreateIoCompletionPort/ReadDirectoryChangesW/GetQueuedCompletionStatus)
- **USER32.dll** (100+ functions): Window management (CreateWindowExW/DialogBoxParamW/DestroyWindow), UI (DrawTextW/FillRect/TrackPopupMenuEx), input (GetKeyState/SetFocus), device notifications (RegisterDeviceNotificationW/UnregisterDeviceNotificationW)
- **ADVAPI32.dll** (17 functions): Registry (RegCreateKeyExW/RegOpenKeyExW/RegSetValueExW/RegQueryValueExW/RegDeleteKeyW/RegEnumKeyExW), ETW tracing (RegisterTraceGuidsW/TraceEvent), security (EncryptFileW/DecryptFileW/GetNamedSecurityInfoW/SetNamedSecurityInfoW/TreeResetNamedSecurityInfoW)
- **SETUPAPI.dll** (5 functions): Device enumeration (SetupDiOpenDeviceInterfaceW/SetupDiGetDeviceInterfaceDetailW/SetupDiCreateDeviceInfoList/SetupDiDestroyDeviceInfoList/SetupDiOpenDevRegKey)

### COM
- **ole32.dll** (22 functions): COM core (CoCreateInstance/CoGetClassObject/CoInitializeEx), drag-drop (RegisterDragDrop/RevokeDragDrop), marshaling (HBITMAP_UserMarshal/Unmarshal/Size/Free, HWND_UserMarshal/Unmarshal/Size/Free, HICON_UserMarshal/Unmarshal/Size/Free), stream (CreateStreamOnHGlobal)
- **OLEAUT32.dll** (24 ordinal-only): COM automation helpers (SysAllocString, Variant*, etc.)
- **RPCRT4.dll** (17 functions): COM proxy/stub (CStdStubBuffer_*, NdrDllRegisterProxy/NdrDllUnregisterProxy/NdrDllGetClassObject)

### Shell / UI
- **SHELL32.dll** (22+ functions): Shell integration (SHBrowseForFolderW/SHOpenFolderAndSelectItems/SHGetPathFromIDListW/SHGetKnownFolderPath/ShellExecuteExW/SHCreateItemFromParsingName/SHCreateItemFromIDList)
- **SHLWAPI.dll** (35+ functions): Path utilities (PathFileExistsW/PathFindFileNameW/PathFindExtensionW/PathIsRelativeW/PathIsRootW/PathAppendW), string (StrCmpIW/StrStrW/StrRetToBufW), registry (SHRegGetValueW/SHRegOpenUSKeyW)
- **GDI32.dll** (50+ functions): Graphics (BitBlt/StretchBlt/CreateDIBSection/CreateFontIndirectW/CreateSolidBrush/SelectObject)
- **gdiplus.dll** (100+ functions): GDI+ rendering (GdipCreateBitmapFromHBITMAP/GdipDrawImageRectI/GdipMeasureString/GdipCreateFont)
- **UxTheme.dll** (7 functions): Visual styles (OpenThemeData/DrawThemeBackground/GetThemeColor/GetThemeFont/GetThemeMargins/GetThemePartSize/SetWindowTheme)
- **OLEACC.dll** (3 functions): Accessibility (AccessibleObjectFromWindow/LresultFromObject/CreateStdAccessibleProxyW)

### Windows Live DLLs
- **WLXPhotoBase.dll** (13 functions): `Base::Exception`, `Base::Throw/ThrowLastError`, `Base::Delete/New`, `Base::IsOutOfMemoryError`, `Base::IsWin7OrGreater/IsWin8OrGreater`, `Base::GdiplusStatusToHresult`, `Base::GetBaseStringManager`, `BasePrivate::New/Delete`, `ATL::BaseAtlThrow`
- **WLXPhotoSqm.dll** (4 functions): `Sqm::Startup/Shutdown/IsEnabled/AddToStream`
- **DmxBici.dll** (1 function): `BiciWrapper::TransferExperienceToWeb` — web telemetry
- **UXCore.dll** (8 functions): Resource manager (RMFindModule/RMLoadMenu/RMLoadImage/RMUpdateResourceSet), string resources (CRMStringResource::Load/Length/ToString)
- **MetadataSys.dll** (1 function): `WLXPSGetItemPropertyHandler` — metadata property access

### Device / WIA
- **STI.dll** (delay-loaded, 1 function): `StiCreateInstanceW` — Still Image API (WIA/scanner acquisition)
- **VERSION.dll** (3 functions): File version info (GetFileVersionInfoW/GetFileVersionInfoSizeW/VerQueryValueW)
- **WINMM.dll** (1 function): `PlaySoundW` — audio feedback on acquisition complete
- **msi.dll** (1 ordinal): MSI installer integration

## COM Architecture

### ATL Module
ATL COM DLL with `CAtlDllModuleT<PhotoAcquireModule>` pattern. Standard COM registration, class factory, object map infrastructure.

### COM Classes (8 COM-servable classes)

| ATL Class | CLSID Constant | Interface | Purpose |
|-----------|---------------|-----------|---------|
| `PhotoAcquire` | `CLSID_PhotoAcquire` | `IPhotoAcquire` | Main acquisition engine |
| `PhotoAcquireSettings` | `GUID_NULL` | `IPhotoAcquireSettings/Settings2` | Acquisition settings/configuration |
| `PhotoAcquireDeviceSelectionDialog` | `CLSID_PhotoAcquireDeviceSelectionDialog` | `IPhotoAcquireDeviceSelectionDialog` | Device selection UI |
| `PhotoAcquireOptionsDialog` | `CLSID_PhotoAcquireOptionsDialog` | `IPhotoAcquireOptionsDialog` | Advanced options UI |
| `PhotoPickerDialog` | `CLSID_PhotoPickerDialog` | `IPhotoPickerDialog` | Photo picker/browse UI |
| `SupportedFileTypes` | `GUID_NULL` | `ISupportedFileTypes` | File type enumeration |
| `LiveAcquisitionWizard` | `GUID_NULL` | `ILivePhotoAcquisitionWizard` | Wizard-driven acquisition |
| `PathnameFromTemplate` | `GUID_NULL` | `IPathnameFromTemplate` | Filename template resolution |

### COM GUIDs (7 — verified via .rgs)

| GUID | Purpose |
|------|---------|
| `{4E13844B-6012-40cb-8E3C-0F7EB9AEF6DD}` | LivePhotoAcquire |
| `{94B1CBEA-F33F-4508-8AAA-55FD8113EFCB}` | LivePhotoAcquireOptionsDialog |
| `{0D5A7D0E-9A06-4e17-85D9-A0B24036371D}` | LivePhotoPickerDialog |
| `{E84D0D46-3D57-4039-9EFE-310AF1CAF92A}` | LivePhotoAcqDeviceSelectionDlg |
| `{4D8A134F-3D0A-4375-8B1A-78CD171C9318}` | LivePhotoAcquisitionWizard |
| `{3BD0ACD1-71CA-4475-92CC-E0AA0AAF843F}` | CLSIDForCancel |
| TypeLib: `{D85885D4-A18A-4fac-AEE1-BE9AAB0F4485}` | Photo Acquisition Type Library |

### COM Interfaces (22 unique interfaces)

**Core Acquisition:**
- `IPhotoAcquire` — Main acquisition controller
- `IPhotoAcquireSource` / `IPhotoAcquireSource2` — Photo source (device)
- `IPhotoAcquireItem` — Individual item to acquire
- `IPhotoAcquireItemGetThumbnailEx` — Thumbnail retrieval
- `IPhotoAcquireSettings` / `IPhotoAcquireSettings2` — Configuration
- `IPhotoAcquirePlugin` — Import plugin interface
- `IPhotoAcquireDuplicateTrackingSession` — Duplicate detection
- `IPhotoAcquireFilenameToken` — Filename token resolution

**Progress/Callbacks:**
- `IPhotoAcquireProgressCB` / `IPhotoAcquireProgressCB2` — Progress reporting
- `IPhotoProgressActionCB` — Progress action callback
- `IPhotoProgressDialogCancelCB` — Cancel callback

**Device Selection:**
- `IPhotoAcquireDeviceSelectionDialog` — Device picker dialog
- `IPhotoAcquireOptionsDialog` — Options dialog
- `IUIDeviceEnumCallback` — Device enumeration callback

**Photo Picker:**
- `IPhotoPickerDialog` — Photo picker dialog
- `IPhotoPickerDialogSelectionCallback` — Selection change
- `IPhotoPickerDialogCopyCallback` — Copy progress
- `IPhotoPickerDialogEnumerationCallback` — Item enumeration
- `IPhotoPickerDialogAbortCallback` — Abort notification
- `IPhotoPickerClientCallback` — Client callback interface
- `IPhotoPickerClientEventCallback` — Client event
- `IPhotoPickerClientSelectionCallback` — Client selection
- `IPhotoPickerClientReclusterableCallback` — Reclustering
- `IPhotoPickerClientWorkerCallback` — Worker thread callback

**Live Acquisition:**
- `ILivePhotoAcquisitionWizard` — Wizard-driven import

**Utility:**
- `ICharacterReplacement` — Filename character replacement
- `IDirectoryCreatedCB` — Directory creation notification
- `ITokenLookup` — Token resolution for filename templates
- `IPathnameFromTemplate` — Template-based path generation
- `ISupportedFileTypes` — File type support
- `IActionProgress` — Action progress display

## Photo Device Abstraction Layer

The DLL implements a device abstraction hierarchy for different camera/device connection types:

| Class | Description |
|-------|-------------|
| `PhotoDeviceBase` | Abstract base for all device types |
| `PhotoDeviceWiaSti` | **WIA (Windows Image Acquisition)** — TWAIN-compatible scanners and cameras via STI.dll |
| `PhotoDeviceWpd` | **WPD (Windows Portable Devices)** — MTP/PTP cameras, smartphones, mass storage |
| `PhotoDeviceFileSystem` | **File System** — Direct folder import |
| `PhotoDeviceDigitalVideo` | **DV (IEEE 1394)** — Digital video cameras via DV capture |

### Device Detection Flow
1. `SetupDiOpenDeviceInterfaceW` → enumerate device interfaces
2. `SetupDiGetDeviceInterfaceDetailW` → get device path
3. Device path determines class: WPD (MTP/PTP), WIA/STI (TWAIN), FileSystem, or DV
4. `STI.dll` (delay-loaded) → `StiCreateInstanceW` for WIA scanner initialization
5. `DeviceIoControl` → direct device control for mass storage

## Photo Picker UI Architecture

Large ATL/WTL-based UI subsystem with thumbnail rendering:

### UI Classes
| Class | Description |
|-------|-------------|
| `PhotoPickerDialog` | Main picker dialog (ATL CDialogImpl) |
| `PhotoPickerList` | Photo list control |
| `PhotoPickerListView` | ListView-based photo list |
| `PhotoPickerEventView` | Event/group view |
| `PhotoPickerEventThumbView` | Event thumbnail view |
| `PhotoPickerItemThumbView` | Item thumbnail view |
| `PhotoPickerItemView` | Single item view |
| `PhotoPickerProgressWindow` | Transfer progress window |
| `PhotoPickerCustomSelectionWindow` | Custom selection UI |
| `PhotoPickerSimpleSelectionWindow` | Simple selection UI |
| `PhotoPickerTrackBar` | Time range track bar |
| `PhotoPickerAddTagsButton` | Tag button |
| `PhotoPickerInPlaceEdit` | In-place rename editing |
| `PhotoPickerNoEraseBkgndControlSubclass` | Background painting control |
| `PhotoPickerClientWorker` | Background worker thread window |

### Accessibility
Full UI Automation accessibility support via `AccPrivate::UIBase`:
- `AccessibleProxy` for ButtonEx, CueEdit, ListView
- `IDispatchImpl` for `IAccessible`
- `EnumVariantTearOff` for enumeration

### Custom Controls
| Class | Description |
|-------|-------------|
| `ButtonEx` | Extended button control |
| `CueEdit` | Edit control with placeholder text |
| `StaticTextEx` | Extended static text |
| `InPlaceEdit` | In-place editing control |
| `ListView` | Custom list view |
| `SpecialKeyGetter` | Keyboard input handling |

## Filename Template System

The DLL implements a sophisticated filename template system for organizing imported photos:

### Template Variables
- `$(MyPicturesFolder)` — User's Pictures folder
- `$(MyVideoFolder)` — User's Videos folder
- `$(OriginalFilename)` — Original file name from camera
- `$(OriginalExtension)` — Original file extension
- `$(RelativePathname)` — Relative path on device
- `$(GroupTag)` — Date/group tag
- `$(DateTimeTaken)` — Date/time photo was taken
- `$(FILE_EXTENSION)` — Output file extension
- `$(DATE_TAKEN_1)` — Date taken (formatted)
- `$(DATE_ACQUIRED)` — Date imported
- `$(DownloadedAlbums)` — Downloaded album paths
- `$(OwnerName)` — Owner name

### Template Examples
```
$(MyPicturesFolder)\$(GroupTag)\$(OriginalFilename).$(OriginalExtension)
$(MyPicturesFolder)\$(GroupTag)\$(RelativePathname[\])$(OriginalFilename).$(OriginalExtension)
$(MyPicturesFolder)\$(DownloadedAlbums[\])$(OwnerName[\])$(AlbumName)\$(OriginalFilename).$(OriginalExtension)
$(MyVideoFolder)\$(DateTimeTaken)$([ ]GroupTag).$(OriginalExtension)
Himalayas {DATE_TAKEN_1} 11.35.33 {FILE_EXTENSION}
Asia 001 {FILE_EXTENSION}
DSC_1234 {FILE_EXTENSION}
```

## XMP Metadata Support

The DLL reads/writes Microsoft Photo metadata (XMP namespace):
- `http://ns.microsoft.com/photo/1.1/` — Panoramic stitch properties
- `http://ns.microsoft.com/photo/1.2/` — Region/face tag properties

### Region Metadata (Face Tags)
- `RegionInfo` → `Region` → `PersonDisplayName`, `PersonEmailDigest`, `PersonLiveIdCID`, `PersonSourceID`, `Rectangle`

### Panoramic Stitch Metadata
- `PanoramicStitchCameraMotion`, `PanoramicStitchMapType`, `PanoramicStitchPhi0/1`, `PanoramicStitchTheta0/1`

## Codec Support

### Registered File Types
| Registry Path | Purpose |
|---------------|---------|
| `FileSupport\DownloadableCodecs` | Downloadable codec registrations |
| `FileSupport\DownloadableWLRawCodecs` | WL raw codec registrations |
| `FileSupport\QuickTimeMovieExtensions` | QuickTime MOV/MP4 support |
| `FileSupport\RawPhotoExtensions` | RAW photo format support |
| `SuppressFileTypes` | Files to hide from UI |
| `Media Type\Extensions` | Media type extensions |

### Codec Classes
| Class | Description |
|-------|-------------|
| `CodecUtil::SafeSave` | Atomic file save |
| `CodecUtil::SmartBackupRead` | Backup stream reader |
| `CodecUtil::FindStreamHelper` | Stream search |
| `CodecUtil::SimpleCodecEncDecCallbackCore` | Encryption/decryption |
| `CodecUtil::CodecEncDecCallbackCore` | Codec encode/decode |
| `CodecUtil::IFindStream` | Stream interface |
| `CodecUtil::PersonTagSerializer` | XMP person tag I/O |
| `LoadUtil::ReadOnlyStreamOnStaticMemory` | Memory stream |
| `GdipUtil::BufferedPainter` | Double-buffered GDI+ painting |
| `GdipUtil::GdiplusStartupWrapper` | GDI+ initialization |

## Data Structures

| Class | Description |
|-------|-------------|
| `DataStructs::GroupedDataSet` | Grouped photo data (by date) |
| `DataStructs::IntSet` | Integer set collection |
| `PicList::Colors` | Color scheme definitions |
| `PicList::ItemViewBase/ItemViewThumb` | Item view rendering |
| `PicList::ListViewDataSetGlobal` | Global list data |
| `PicList::ListViewDragDrop` | Drag-drop support |
| `Win7Library::LibraryManager` | Win7 library integration |

## Registry Paths

| Registry Path | Purpose |
|---------------|---------|
| `Software\Microsoft\Windows Live\Photo Acquisition` | Main settings |
| `Software\Microsoft\Windows Live\Photo Acquisition\CharacterReplacements` | Filename character replacements |
| `Software\Microsoft\Windows Live\Photo Gallery` | Gallery settings |
| `Software\Microsoft\Windows Live\Photo Gallery\FileSupport\*` | File type support |
| `Software\Microsoft\Windows Live\Photo Gallery\Library` | Library paths |
| `Software\Microsoft\Windows Live\Environment\PhotoGallery` | Environment paths |
| `Software\Microsoft\Windows\CurrentVersion\Photo Acquisition\Plugins` | Import plugins |
| `Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\%ws` | Property handlers |
| `Software\Microsoft\Windows Media Foundation\ByteStreamHandlers` | MF byte stream handlers |
| `Software\Microsoft\Windows NT\CurrentVersion\LanguagePack` | Language packs |
| `WiaDialogExtensionHandlers` | WIA dialog extensions |

## Import UI Strings

Key user-facing strings from the resource section:
- "Looking for photos and videos..."
- "Found %d photos and videos"
- "Importing item %1!u! of %2!u!"
- "Import &all new items now" / "Review, organize, and group items to import"
- "This device:" / "Choose a location to store your photos and videos:"
- "Import Options" / "Photo Import Settings"
- "Erase after importing" / "&Import to:" / "&Open Photo Gallery after importing files"
- "Import videos as &multiple files (not available for HD video)"
- "Digital Video Cameras" / "Scanners and Cameras" / "Other Devices"
- "PortableDeviceIsMassStorage"
- "The exact error is unknown. Make a note of this error code..."

## SQM Telemetry
- `Sqm::Startup/Shutdown/IsEnabled` — Lifecycle management
- `Sqm::AddToStream(K, K)` / `Sqm::AddToStream(K, PBVTuple)` — Usage metrics
- `BiciWrapper::TransferExperienceToWeb` — Web telemetry via DmxBici.dll

## Dependencies Summary

| DLL | Functions | Role |
|-----|-----------|------|
| MSVCR110.dll | 80+ | CRT core |
| KERNEL32.dll | 100+ | OS kernel |
| USER32.dll | 100+ | Window management |
| ADVAPI32.dll | 17 | Registry + ETW + Security |
| SETUPAPI.dll | 5 | Device enumeration |
| ole32.dll | 22 | COM core + marshaling |
| OLEAUT32.dll | 24 ordinals | COM automation |
| RPCRT4.dll | 17 | COM proxy/stub |
| SHELL32.dll | 22+ | Shell integration |
| SHLWAPI.dll | 35+ | Path/string utilities |
| GDI32.dll | 50+ | Graphics |
| gdiplus.dll | 100+ | GDI+ rendering |
| UxTheme.dll | 7 | Visual styles |
| OLEACC.dll | 3 | Accessibility |
| WLXPhotoBase.dll | 13 | Memory/exception base |
| WLXPhotoSqm.dll | 4 | SQM telemetry |
| DmxBici.dll | 1 | Web telemetry |
| UXCore.dll | 8 | Resource management |
| MetadataSys.dll | 1 | Metadata properties |
| STI.dll | 1 (delay) | WIA scanner acquisition |
| VERSION.dll | 3 | File version info |
| WINMM.dll | 1 | Audio feedback |
| msi.dll | 1 ordinal | MSI integration |

## Architecture Summary

WLXPhotoAcq.dll is the comprehensive photo acquisition engine for Windows Live Photo Gallery 2012. It implements a multi-device abstraction layer supporting four connection types: WIA/STI (scanners), WPD/MTP/PTP (cameras), file system (folder import), and IEEE 1394 DV (digital video). The DLL provides a complete photo picker UI with thumbnail views, tag editing, grouping by date, and configurable filename templates. It handles XMP metadata reading/writing for face tags and panoramic stitch properties, codec management for RAW/QuickTime formats, duplicate detection, and progress reporting. The entire subsystem is COM-based with 8 coclasses and 22+ interfaces, using ATL/WTL for the UI and GDI+ for rendering. At ~1.75 MB, roughly 60% of the binary is resource data (dialogs, strings, icons, XMP templates).
