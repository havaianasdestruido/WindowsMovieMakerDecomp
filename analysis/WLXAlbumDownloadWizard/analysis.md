# WLXAlbumDownloadWizard.exe + Resources.dll Analysis

## Overview

**Files:**
- `WLXAlbumDownloadWizard.exe` — Album download wizard executable (291,528 bytes)
- `WLXAlbumDownloadWizardResources.dll` — Resource-only satellite DLL (1,496,768 bytes / ~1.4 MB)

**Role:** Windows Live Photo Gallery 2012 — OneDrive (SkyDrive) album download wizard. Downloads photos/videos from OneDrive cloud albums to local storage.

**Build:** 16.4.3528.0331 (timestamp: 2014-04-01 01:26:31 EXE / 01:28:09 DLL)
**PDB:** `WLXAlbumDownloadWizard.pdb` `{D1F2EB28-4C81-4754-8839-5F96AA504D59}`
**Build tag:** `16.4.3528.0331_ship.client.main.w5m4 (ship)`

## Manifest

Embedded Win32 application manifest at RVA 0x2F2E0:
```xml
<assemblyIdentity version="5.1.0.0" processorArchitecture="x86"
  name="Microsoft.Windows.personalMedia.WLXAlbumDownloadWizard" type="win32"/>
<description>Windows Shell</description>
<!-- DPI-aware, Common Controls v6, asInvoker -->
```

Key manifest attributes:
- **DPI-aware:** `<dpiAware>true</dpiAware>`
- **Common Controls v6** dependency (visual styles)
- **Requested execution level:** `asInvoker` (no elevation required)

## WLXAlbumDownloadWizard.exe — PE Structure

| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Subsystem | Windows GUI (2) |
| Linker | MSVC 11.00 |
| Image Base | 0x00400000 |
| Image Size | 0x47000 (284 KB) |
| Entry Point | 0x20D97 → `__security_init_cookie` |
| OS/Image Version | 6.02 / 6.02 |
| DLL Characteristics | Dynamic Base (ASLR), NX Compatible, Terminal Server Aware |
| Stack reserve/commit | 256 KB / 8 KB |
| Heap reserve/commit | 1 MB / 4 KB |
| Checksum | 0x554AE |

### Sections

| Section | VA | VirtSize | RawSize | Characteristics |
|---------|------|----------|---------|-----------------|
| `.text` | 0x1000 | 0x2A234 (168 KB) | 0x2A400 | Code, Execute Read |
| `.data` | 0x2C000 | 0x24B0 (9 KB) | 0x1800 | Init Data, Read Write |
| `.rsrc` | 0x2F000 | 0x12B00 (74 KB) | 0x12C00 | Init Data, Read Only |
| `.reloc` | 0x42000 | 0x465E (18 KB) | 0x4800 | Init Data, Discardable, Read Only |

## WLXAlbumDownloadWizardResources.dll — PE Structure

| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Type | DLL (resource-only) |
| Linker | MSVC 11.00 |
| Image Base | 0x10000000 |
| Image Size | 0x16B000 (~1.4 MB) |
| Entry Point | 0 (no code) |
| OS/Image Version | 6.02 / 6.02 |
| DLL Characteristics | Dynamic Base (ASLR), NX Compatible |

### Sections

| Section | VA | VirtSize | RawSize | Characteristics |
|---------|------|----------|---------|-----------------|
| `.rsrc` | 0x1000 | 0x1695D0 (1.4 MB) | 0x169600 | Init Data, Read Only |

**The Resources DLL is 100% resource data** — no code, no imports, no exports, no relocation table. It exists solely to be loaded by the EXE's delay-loaded `UXCore.dll` resource management system (`RMFindModule`/`RMLoadImage`/`CRMStringResource`).

## Exports

**EXE:** No exports (standard GUI application)
**Resources DLL:** No exports (resource-only satellite assembly)

## Imports — WLXAlbumDownloadWizard.exe

### CRT / Runtime
- **MSVCR110.dll** (~65 functions): CRT core — memory (`malloc`/`free`/`calloc`/`_recalloc`/`memcpy`/`memcpy_s`/`memset`), string (`wcstok_s`/`wcsstr`/`wcsncpy_s`/`swprintf_s`/`_wcsicmp`/`_wcslwr_s`/`towupper`/`wcstol`), exceptions (`__CxxFrameHandler3`/`_CxxThrowException`/`terminate`), locale (`_create_locale`/`_free_locale`), threading (`_beginthreadex`), CRT init (`_initterm`/`_initterm_e`/`__wgetmainargs`/`_amsg_exit`)

### OS Kernel
- **KERNEL32.dll** (~100+ functions): File I/O (`CreateFileW`/`ReadFile`/`WriteFile`/`DeleteFileW`/`MoveFileExW`/`BackupRead`/`BackupSeek`), module management (`LoadLibraryExW`/`LoadLibraryW`/`FreeLibrary`/`GetModuleHandleW`/`GetModuleFileNameW`/`GetProcAddress`), resources (`FindResourceW`/`FindResourceExW`/`LoadResource`/`LockResource`/`SizeofResource`), memory (`VirtualAlloc`/`VirtualFree`/`HeapAlloc`/`HeapFree`/`HeapReAlloc`), threading (`CreateThread`/`CreateEventW`/`WaitForSingleObject`/`InitializeCriticalSection`/`InitializeSListHead`/`InterlockedCompareExchange`), COMPLETION PORT (`CreateIoCompletionPort`/`GetQueuedCompletionStatus`/`PostQueuedCompletionStatus`/`ReadDirectoryChangesW`), process (`CreateProcessW`/`IsWow64Process`), path (`GetFullPathNameW`/`GetTempPathW`/`GetTempFileNameW`), locale (`SetThreadLocale`/`SetThreadPreferredUILanguages`/`LocaleNameToLCID`), security (`HeapSetInformation`/`SetDllDirectoryW`), version (`GetVersion`/`GetVersionExW`)

### Window Management
- **USER32.dll** (~25 functions): Window (`CreateWindowExW`/`DestroyWindow`/`IsWindow`/`DefWindowProcW`/`CallWindowProcW`/`SetWindowLongW`/`GetWindowLongW`), messaging (`PeekMessageW`/`TranslateMessage`/`DispatchMessageW`/`SendMessageW`/`PostMessageW`/`PostQuitMessage`/`MsgWaitForMultipleObjects`), UI (`MessageBoxW`/`SetFocus`/`GetSystemMetrics`/`SetCursor`/`GetCursor`/`LoadCursorW`), timer (`SetTimer`/`KillTimer`), class (`RegisterClassExW`/`UnregisterClassW`/`GetClassInfoExW`), icons (`DestroyIcon`)

### GDI
- **GDI32.dll** (1 function): `DeleteObject`

### Shell / COM
- **SHELL32.dll** (~7 functions): `SHGetKnownFolderPath`/`CommandLineToArgvW`/`SHChangeNotify`/`SHCreateItemFromParsingName`/`SHCreateItemFromIDList`, Ordinal 162
- **ole32.dll** (~12 functions): COM core (`CoInitializeEx`/`CoInitialize`/`CoUninitialize`/`CoCreateInstance`/`CoGetClassObject`), memory (`CoTaskMemAlloc`/`CoTaskMemFree`/`CoTaskMemRealloc`), `CLSIDFromString`/`PropVariantClear`/`OleInitialize`/`OleUninitialize`
- **OLEAUT32.dll** (~9 ordinals): `SysAllocString`/`SysFreeString`/`SysStringLen`/etc.
- **gdiplus.dll** (~10 functions): GDI+ bitmap operations (`GdipCreateBitmapFromHBITMAP`/`GdipCreateHBITMAPFromBitmap`/`GdipCloneImage`/`GdipCloneBitmapAreaI`/`GdipGetImageWidth`/`GdipGetImageHeight`/`GdipGetImagePixelFormat`/`GdipDisposeImage`/`GdipAlloc`/`GdipFree`)

### Registry / Security
- **ADVAPI32.dll** (~22 functions): Registry (`RegOpenKeyExW`/`RegCreateKeyExW`/`RegCloseKey`/`RegQueryValueExW`/`RegSetValueExW`/`RegDeleteKeyW`/`RegDeleteValueW`/`RegEnumKeyExW`/`RegEnumValueW`/`RegQueryInfoKeyW`/`RegGetValueW`/`RegOpenCurrentUser`), ETW tracing (`RegisterTraceGuidsW`/`TraceEvent`/`GetTraceLoggerHandle`/`GetTraceEnableLevel`/`GetTraceEnableFlags`/`UnregisterTraceGuids`), security (`EncryptFileW`/`DecryptFileW`/`GetNamedSecurityInfoW`/`SetNamedSecurityInfoW`)

### Path / String Utilities
- **SHLWAPI.dll** (~20 functions): Path (`PathAppendW`/`PathFindFileNameW`/`PathFindExtensionW`/`PathIsRelativeW`/`PathRemoveFileSpecW`/`PathRemoveBackslashW`/`PathAddBackslashW`/`PathIsDirectoryEmptyW`), stream (`SHCreateStreamOnFileW`/`SHCreateStreamOnFileEx`), string (`StrStrW`/`StrStrIW`/`StrRChrW`/`SHStrDupW`), registry (`SHRegOpenUSKeyW`/`SHRegGetUSValueW`/`SHRegEnumUSValueW`/`SHRegCloseUSKey`), URL (`UrlUnescapeW`), type (`AssocGetPerceivedType`)

### Windows Live DLLs
- **WLXPhotoBase.dll** (10 functions): Memory (`BasePrivate::New`/`BasePrivate::Delete`), exceptions (`Base::Exception`/`Base::Throw`/`Base::ThrowLastError`/`Base::IsOutOfMemoryError`/`ATL::BaseAtlThrow`), OS checks (`Base::OS::IsWin7OrGreater`/`Base::OS::IsWin8OrGreater`), strings (`Base::String::GetBaseStringManager`)
- **WLXPhotoSqm.dll** (4 functions): Telemetry (`Sqm::Startup`/`Sqm::Shutdown`/`Sqm::EnableShipAsserts`/`Sqm::AddToStream`)
- **MetadataSys.dll** (1 function): `WLXPSGetItemPropertyHandler` — metadata property access

### Delay-Loaded
- **UXCore.dll** (12 functions): Resource management (`UXCoreInitProcess`/`UXCoreUnInitProcess`/`UXCoreInitThread`/`UXCoreUnInitThread`), CRM resource system (`RMUpdateResourceSet`/`RMFindModule`/`RMLoadImage`), string resources (`CRMStringResource::ToString`/`CRMStringResource::Length`/`CRMStringResource::Load`/`CRMStringResource::CRMStringResource`)

### Other
- **VERSION.dll** (3 functions): `GetFileVersionInfoSizeW`/`GetFileVersionInfoW`/`VerQueryValueW`
- **msi.dll** (1 ordinal): MSI installer integration

## RTTI Class Hierarchy (52 classes)

### Core Wizard
| RTTI Type | Purpose |
|-----------|---------|
| `AlbumDownloadWizard` | Main wizard ATL coclass |
| `?$CComObject@VAlbumDownloadWizard@@` | ATL COM object wrapper |
| `?$CComCoClass@VAlbumDownloadWizard@@$1?GUID_NULL@@` | ATL coclass registration |
| `AlbumDownloader` | Download engine |

### COM Infrastructure
| RTTI Type | Purpose |
|-----------|---------|
| `CComObjectRootBase@ATL` | ATL root |
| `?$CComObjectRootEx@VCComMultiThreadModel@ATL@@` | Multi-threaded ATL root |
| `CAppModule@WTL` | WTL application module |
| `CComModule@ATL` | ATL module base |
| `?$CAtlModuleT@VCComModule@ATL@@` | ATL module template |
| `CAtlModule@ATL` | ATL module |
| `CRegObject@ATL` | ATL registry object |
| `CAtlException@ATL` | ATL exception |
| `CWindow@ATL` | ATL window |
| `CMessageMap@ATL` | ATL message map |

### UI System
| RTTI Type | Purpose |
|-----------|---------|
| `?$CWindowImplRoot@VCWindow@ATL@@` | ATL window root |
| `?$CWindowImplBaseT@VCWindow@ATL@@V?$CWinTraits@$0FGAAAAAA@$0A@@2@@` | ATL window base traits |
| `?$CWindowImpl@VPhotoPickerClientWorker@@VCWindow@ATL@@...` | Photo picker client worker window |
| `PhotoPickerClientWorker` | Background photo picker worker |
| `DownloadWorker` | Download background worker |
| `?$SystemCursorT@$0HPAC@@UIBase@@` | System cursor wrapper |
| `SetAndRestoreCursor@UIBase` | Cursor save/restore |

### COM Callback Proxies
| RTTI Type | Purpose |
|-----------|---------|
| `?$PhotoPickerClientCallbackProxy@VAlbumDownloadWizard@@` | Photo picker callback proxy |
| `?$CComObject@V?$PhotoPickerClientCallbackProxy@VAlbumDownloadWizard@@@@` | ATL COM proxy object |
| `?$HandlerProxyBase@VAlbumDownloadWizard@@...` | Handler proxy base |

### Photo Picker / Album Content
| RTTI Type | Purpose |
|-----------|---------|
| `?$CComObject@VDirectoryCleanup@AlbumContentWriterPrivate@@` | ATL directory cleanup |
| `DirectoryCleanup@AlbumContentWriterPrivate` | Directory cleanup logic |
| `?$CComObject@VSimplePropertyBag@AlbumContentWriterPrivate@@` | ATL property bag |
| `SimplePropertyBag@AlbumContentWriterPrivate` | Property bag for content writing |
| `ItemWriter@AlbumContentWriterPrivate` | Individual item writer |
| `?$CComObject@VSupportedFileTypes@@` | ATL supported file types |
| `?$CComCoClass@VSupportedFileTypes@@` | Supported file types coclass |
| `SupportedFileTypes` | File type filtering |

### Codec / File I/O
| RTTI Type | Purpose |
|-----------|---------|
| `PersonTagSerializer@CodecUtil` | XMP person tag I/O |
| `?$XmpSerializerBase@...PersonTagRegionData...@@` | XMP region/face metadata serializer |
| `SimpleCodecEncDecCallbackCore@CodecUtil` | Encryption/decryption |
| `SmartBackupRead@CodecUtil` | Backup stream reader |
| `FindStreamHelper@CodecUtil` | Stream search helper |
| `SafeSave@CodecUtil` | Atomic file save |
| `File@Base` | Base file class |
| `TempFile@Base` | Temporary file |

### Base / System
| RTTI Type | Purpose |
|-----------|---------|
| `Exception@Base` | Base exception class |
| `Thread@Base` | Thread wrapper |
| `RefCountBaseMultiThreaded@Base` | Reference-counted base |
| `InvariantBase` | Invariant assertions |
| `Bitmap@Gdiplus` | GDI+ bitmap |
| `Image@Gdiplus` | GDI+ image |
| `GdiplusBase@Gdiplus` | GDI+ base |
| `PrivateAutoPerfTrace@WPP` | WPP performance tracing |
| `SqmStartupWrapper@Sqm` | SQM telemetry startup |
| `?$RefCountBaseSingleton@VLibraryManager@Win7Library@@` | Win7 library singleton |
| `LibraryManager@Win7Library` | Win7 library integration |

## Startup Flow

1. **Entry point** at RVA 0x20D97: `call __security_init_cookie` → `jmp __tmainCRTStartup`
2. **`__security_init_cookie`** (RVA 0x213F6): Generates stack buffer overrun protection cookie from timestamp, process/thread IDs, and performance counters. Cookie stored at `0x42C028`.
3. **`__tmainCRTStartup`** (RVA 0x20BE9): Standard MSVC CRT initialization — calls `_initterm_e`/`_initterm` (C++ initializers at 0x407B80), initializes locale, calls `wWinMain`.

## Key Functions

### Security Cookie (RVA 0x213F6)
```
004213F6: push ebp / mov ebp,esp / sub esp,14h
  ; Read cookie from [0042C028h]
  ; If default (0xBB40E64E) or NULL → generate from:
  ;   - QueryPerformanceCounter
  ;   - GetCurrentProcessId/ThreadId  
  ;   - SystemTimeAsFileTime
  ; Store at [0042C028h], inverse at [0042C02Ch]
```

### Registry Query Pattern (RVA 0x1F527+)
Opens `HKEY_CURRENT_USER\Software\Microsoft\Windows Live\Photo Gallery` with `RegOpenKeyExW` (0x80000002 = HKCU, access 0x20019 = READ_KEY), reads registry values for wizard configuration.

### File Type Detection (RVA 0x16FBB+)
Uses `AssocGetPerceivedType` (imported via SHLWAPI ordinal 158) to determine file types, with a lookup table of 111 (0x6F) entries at `0x42C278` (LCID table) and `0x42C438` (type GUIDs).

### Download Worker (RTTI: `DownloadWorker`)
Background thread using `CWindowImpl` pattern with completion port I/O for asynchronous album download operations.

### UXCore Resource Loading
Uses delay-loaded `UXCore.dll` functions: `RMFindModule` locates the resource DLL (`WLXAlbumDownloadWizardResources.dll`), `CRMStringResource::Load` loads localized strings from it. The EXE itself has minimal string resources; the bulk of UI text is in the satellite DLL.

## Registry Keys

| Path | Purpose |
|------|---------|
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery` | Main settings, download paths |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs` | Downloadable codec support |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs` | WL raw codec support |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions` | RAW format extensions |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions` | QuickTime support |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FileSupport\SuppressFileTypes` | Hidden file types |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\Viewer` | Viewer settings |
| `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\%ws` | Property handlers |
| `HKLM\SOFTWARE\Microsoft\Windows Media Foundation\ByteStreamHandlers` | MF byte stream handlers |
| `HKCU\Software\Microsoft\Windows Live\Installer\ProductStatus` | Product status |
| `HKCU\Software\Microsoft\Windows Live\Common` | Common WL settings |
| `HKCU\Software\Microsoft\IdentityCRL` | Identity/auth CRL |
| `HKCR\Software\RegisteredApplications` | COM app registration |
| `HKCR\Component Categories` | COM category registration |

## Filename Templates

The wizard reuses the WLXPhotoAcq filename template system for downloaded albums:

| Template | Source |
|----------|--------|
| `$(DownloadedAlbums[\])$(OwnerName[\])$(AlbumName)\$(OriginalFilename).$(OriginalExtension)` | Downloaded album path |
| `$(DownloadedAlbums[\])$(AlbumName)\$(OriginalFilename).$(OriginalExtension)` | Album-only path |
| `$(MyPicturesFolder)\$(GroupTag)\$(OriginalFilename).$(OriginalExtension)` | Camera import |
| `$(MyPicturesFolder)\$(GroupTag)\$(RelativePathname[\])$(OriginalFilename).$(OriginalExtension)` | Optical media |
| `$(MyVideoFolder)\$(DateTimeTaken)$([ ]GroupTag).$(OriginalExtension)` | Video camera |
| `$(GroupTag[ ])$(DateTaken) $(AcquisitionSequence).$(OriginalExtension)` | Date-based |
| `$(AlbumName).$(OriginalExtension)` | Album-only |

Template variables: `DownloadedAlbums`, `OwnerName`, `AlbumName`, `MyPicturesFolder`, `MyVideoFolder`, `GroupTag`, `DateTaken`, `DateTakenRange`, `DateTimeTaken`, `AcquisitionDate`, `AcquisitionSequence`, `OriginalFilename`, `OriginalExtension`, `RelativePathname`

## XMP Metadata Support

The wizard reads/writes Microsoft Photo metadata (XMP namespace `http://ns.microsoft.com/photo/1.2/`):

### Region/Face Metadata
- `RegionInfo` → `Region` → `PersonDisplayName`, `PersonEmailDigest`, `PersonLiveIdCID`, `PersonSourceID`, `Rectangle`

### EXIF Integration
- `/app1/ifd/` and `/ifd/exif` paths for EXIF data access
- Padding schema support for XMP/EXIF segments

## External DLL Dependencies (Loaded at Runtime)

| DLL | Purpose |
|-----|---------|
| `WLXPhotoBase.dll` | Memory management, exception handling, OS detection |
| `WLXPhotoSqm.dll` | SQM telemetry |
| `MetadataSys.dll` | Metadata property access |
| `UXCore.dll` (delay-loaded) | Resource management, localized string loading |
| `WLXMediaPublishSubscribe.dll` | Media publish/subscribe (referenced in strings) |
| `WLXPhotoLibraryDuiResources` | DUI resource data |
| `WLXPhotoLibraryDuiResourcesLocalized` | Localized DUI resources |
| `WLDCore.dll` | Windows Live core |
| `DmxBici.dll` | Web telemetry |
| `msidcrl40.dll` | MSIDCRL auth (from `\Microsoft Shared\Windows Live`) |
| `wlidcli.dll` | Windows Live ID client |
| `sqmapi.dll` | SQM API |
| `Comctl32.dll` | Common controls |
| `shell32.dll` | Shell operations |
| `MetadataSys.dll` | Metadata properties |

## UI Strings (Resource DLL)

### Wizard Pages
| String | Context |
|--------|---------|
| "Downloaded Albums" | Album picker title |
| "Select the files you want to download" | File selection instruction |
| "Downloading %1!u! of %2!u! items" | Download progress |
| "Locating photos and videos..." | Searching state |
| "Retrieving information" | Information retrieval state |
| "Downloading %s" | Download in progress with name |

### Source Labels
| String | Context |
|--------|---------|
| "From %1!s!" | Album source |
| "From %1!s!'s %2!s!" | Album source with owner |
| "OneDrive" / "OneDrive Photos" | Source service name |

### Error Messages
| String | Context |
|--------|---------|
| "You don't have permission to access the content." | Permission error |
| "Photo Gallery is unable to download this album because your Microsoft account doesn't have permission to view it. Would you like to retry with another email address?" | Account permission error |
| "The service is not available right now. Please try again later." | Service unavailable |
| "Photo Gallery is unable to complete this album download due to network errors. Please try again later." | Network error |
| "Some items in this album could not be downloaded due to network error. Would you like to retry the download?" | Partial download error |
| "There is not enough disk space to continue downloading. Free additional disk space and then try again." | Disk space error |
| "The folder you're trying to download to isn't available or doesn't exist. Please choose another folder." | Folder unavailable |
| "Sorry, something went wrong and the album couldn't be downloaded. Try again." | Generic failure |
| "Sorry, Photo Gallery couldn't save to the folder you selected. Please make sure that the folder is available and not marked Read-only." | Save permission error |
| "Sorry, we can't save to that location because the final path is too long." | Path too long error |
| "The folder you have selected doesn't contain any files that Photo Gallery supports." | Unsupported files |
| "Some files couldn't be downloaded." | Partial failure |
| "The files may have been deleted, changed recently or might be infected with a virus. The following files weren't downloaded: %1!s!" | Files missing/deleted |
| "A network error has occurred. Please check your network connection and try again." | Network connectivity |
| "If you cancel, the photos won't be downloaded. Are you sure you want to cancel?" | Cancel confirmation |
| "You have entered an invalid command" | Invalid CLI args |

### Display
| String | Context |
|--------|---------|
| "Only image and video files are displayed." | Filter notice |
| "Photo Gallery Download Wizard" | Window title |
| "Photo Gallery" | Product name |

### Localized in Resource DLL
The Resources DLL contains version info in standard `VS_VERSION_INFO` structure with `StringFileInfo` block `040904B0` (English US, Unicode).

## Download Architecture

### Worker Thread Pattern
```
AlbumDownloadWizard (main UI)
    ├── PhotoPickerClientWorker (ATL CWindowImpl, background worker)
    │       ├── Uses completion port I/O
    │       ├── File downloads via COM IStream
    │       └── Progress callbacks via PhotoPickerClientCallbackProxy
    ├── DownloadWorker (background download)
    │       ├── Creates download directory
    │       └── Writes album content via AlbumContentWriterPrivate
    └── AlbumDownloader (download engine)
            ├── ItemWriter (per-file writer)
            ├── SimplePropertyBag (metadata transfer)
            └── DirectoryCleanup (rollback on failure)
```

### Download Flow
1. User selects album from OneDrive via `PhotoPickerClientWorker`
2. `AlbumDownloader` receives file list
3. For each file: `ItemWriter` creates local file, `SimplePropertyBag` stores metadata
4. XMP person tags preserved via `PersonTagSerializer@CodecUtil`
5. `SmartBackupRead@CodecUtil` handles resume/retry
6. `DirectoryCleanup@AlbumContentWriterPrivate` rolls back partial downloads on failure
7. `SafeSave@CodecUtil` ensures atomic file writes
8. `SupportedFileTypes` filters for image/video only

### Authentication
- `msidcrl40.dll` (Windows Live ID CRL) for OAuth token management
- `wlidcli.dll` for Windows Live ID client operations
- `Software\Microsoft\IdentityCRL` for credential storage

## Language Support

The EXE contains 111 LCID entries in lookup tables at `0x42C278` and `0x42C438`, supporting the full WL 2012 locale set. The Resources DLL is loaded per-locale by `UXCore.dll`'s `RMFindModule` system.

## Telemetry

- **SQM (Software Quality Metrics):** `Sqm::Startup` → `Sqm::AddToStream` → `Sqm::Shutdown` lifecycle
- **WPP (Windows Performance Profiler):** `PrivateAutoPerfTrace@WPP` for ETW trace logging via `RegisterTraceGuidsW`/`TraceEvent`
- **Bici (Web telemetry):** Via `DmxBici.dll` for web experience metrics

## Architecture Summary

WLXAlbumDownloadWizard.exe is a Windows Live Photo Gallery 2012 component that provides a wizard UI for downloading photo/video albums from OneDrive (SkyDrive) to local storage. The EXE is a compact 284 KB PE32 GUI application built with ATL/WTL, while the 1.4 MB resource-only satellite DLL contains all localized UI strings, dialog resources, and version information.

The application uses an ATL COM architecture with `AlbumDownloadWizard` as the main coclass, implementing the `IPhotoPickerClient*` callback interfaces for the photo picker UI. Background downloads are handled by `DownloadWorker` and `PhotoPickerClientWorker` using Windows completion port I/O for asynchronous file operations.

Key features:
- OneDrive album browsing and file selection
- Asynchronous multi-file download with progress reporting
- XMP metadata preservation (face tags, person regions)
- Atomic file saves with rollback on failure
- DPI-aware Common Controls v6 UI
- Multi-locale support via UXCore satellite resource system
- SQM/WPP telemetry integration
- `asInvoker` execution (no elevation)
