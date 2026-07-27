# WLXPhotoLibraryDatabase.dll Analysis

## Overview

**File:** WLXPhotoLibraryDatabase.dll (Windows Live Photo Gallery 2012 — Photo Library Database)
**Role:** Photo library database operations — object storage, face detection/reognition tasks, thumbnail management, geocoding, file scanning
**Image size:** 0xC8000 (~800 KB)
**PDB:** `WLXPhotoLibraryDatabase.pdb` `{85E2F423-D889-4BE0-9A1F-A61FCCEF7C0C}`
**Build:** 16.4.3528.0331 (timestamp: 2014-04-01 01:24:42)
**Real DLL name:** `WLXPhotoLibraryDatabase.dll`

## PE Structure

| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Subsystem | Windows GUI (2) |
| Linker | MSVC 11.00 |
| Image Base | 0x10000000 |
| Image Size | 0xC8000 |
| Entry Point | 0x10089F4F |
| OS/Image Version | 6.02 / 6.02 |
| DLL Characteristics | Dynamic Base (ASLR), NX Compatible |
| Stack reserve/commit | 256 KB / 4 KB |
| Heap reserve/commit | 1 MB / 4 KB |
| Checksum | 0xD378C |

### Sections

| Section | VA | VirtSize | RawSize | Characteristics |
|---------|------|----------|---------|-----------------|
| `.text` | 0x1000 | 0xAE448 (681 KB) | 0xAE600 | Code, Execute Read |
| `.data` | 0xB0000 | 0x5490 (21 KB) | 0x4800 | Init Data, Read Write |
| `.rsrc` | 0xB6000 | 0x400 (1 KB) | 0x400 | Init Data, Read Only |
| `.reloc` | 0xB7000 | 0x10024 (64 KB) | 0x10200 | Init Data, Discardable, Read Only |

**Key observation**: Code-heavy DLL — `.text` section is 681 KB. Minimal resources (1 KB) — all logic is in code. Large relocation table (64 KB) suggests many cross-module references.

## Exports (17 functions)

| Ordinal | Name | RVA | Purpose |
|---------|------|-----|---------|
| 11 | `?CreateDBThumbnailBroker@@YGJABU_GUID@@0PAPAX@Z` | 0x245A2 | Create thumbnail broker for DB |
| 12 | `?CreateDBThumbnailSupplier@@YGJABU_GUID@@0PAPAX@Z` | 0x24492 | Create thumbnail supplier for DB |
| 13 | `?EnsureLocationTaskStarted@@YGJPAPAUIUnknown@@@Z` | 0x3FBD3 | Start geocoding location task |
| 14 | `?GetFaceThumbnailCache@@YGJPAVIObjectDatabase@@ABU_GUID@@PAPAX@Z` | 0x34D61 | Get face thumbnail cache |
| 15 | `?PeekDatabase@@YGJABU_GUID@@PA_N@Z` | 0x1CDAD | Check if database is open |
| 16 | `?ScheduleFaceThumbnailGenerationForObject@@YGJPAUIUnknown@@K@Z` | 0x29E6E | Schedule face thumb gen for object |
| 17 | `?ScheduleFaceThumbnailGenerationForRegion@@YGJPAUIUnknown@@K@Z` | 0x29DE7 | Schedule face thumb gen for region |
| 18 | `?UpdateFaceDetectionForObject@@YGJPAUIUnknown@@KPAUISharedBitmap@@_N@Z` | 0x29D5A | Update face detection for object |
| 3 | `EnsureFaceDataTaskStarted` | 0x29CF5 | Start face data extraction task |
| 4 | `EnsureFaceRecognitionTaskStarted` | 0x2D485 | Start face recognition task |
| 7 | `EnsureFaceTileTaskStarted` | 0x34FD9 | Start face tile generation task |
| 1 | `GetDatabase` | 0x1CD55 | Get/create object database |
| 2 | `GetDatabasePropertyStore` | 0x2063C | Get property store from DB |
| 8 | `SetRegionAsDefaultFaceTile` | 0x3503E | Set region as default face tile |
| 9 | `UpdateFaceGrouping` | 0x2D611 | Update face grouping/clustering |
| 6 | `UpdateFaceRecognition` | 0x2D55F | Update face recognition |
| 5 | `UpdateFaceRecognitionForObjects` | 0x2D5B9 | Update recognition for objects |

## Import Table

### CRT / Runtime
- **MSVCR110.dll** (72 functions): CRT core — memory (malloc/free/calloc/realloc/aligned_malloc/aligned_free/memcpy/memcmp/memset/memmove/memcpy_s/memmove_s), SSE2 math (cos/sin/exp/log/pow), string (wcschr/wcscat_s/wcscpy_s/wcsncpy_s/wcsstr/wcsrchr/wcscspn/wcsspn/wcsnlen/wcslen/wcsncmp/wcstok_s/wcstoul/wcsstr/_wcsicmp/_wcsnicmp/_wcslwr_s/towlower/towupper/iswspace/iswupper/swprintf_s/swscanf_s/_vsnwprintf/_vsnwprintf_l/_vscwprintf/vswprintf_s), exceptions (CxxThrowException/CxxFrameHandler3/terminate), qsort/bsearch, locale (_create_locale/_free_locale), threading (_beginthreadex), string conversion (_swscanf_s_l/_vsnprintf)

### OS
- **KERNEL32.dll** (108 functions): File I/O (CreateFileW/ReadFile/WriteFile/CopyFileW/MoveFileExW/DeleteFileW/FindFirstFileExW/FindNextFileW/FindClose/SetFileAttributesW/GetFileAttributesW/GetFileAttributesExW/GetFileTime/SetFileTime/BackupRead/BackupSeek), directory (CreateDirectoryW/GetFullPathNameW/GetLongPathNameW), volume (GetDriveTypeW/GetVolumeInformationW/GetLogicalDriveStringsW/GetDiskFreeSpaceExW), module (LoadLibraryW/LoadLibraryExW/FreeLibrary/GetModuleHandleW/GetModuleHandleA/GetModuleFileNameW/GetProcAddress), resources (FindResourceW/FindResourceExW/LoadResource/LockResource/SizeofResource), sync (CreateEventW/CreateSemaphoreW/CreateMutexW/CreateIoCompletionPort/GetQueuedCompletionStatus/PostQueuedCompletionStatus/WaitForSingleObject/CRITICAL_SECTION), threading (CreateThread/GetCurrentThreadId/GetCurrentProcessId/SetThreadPriority/GetThreadPriority/GetCurrentThread/GetExitCodeThread), memory (LocalAlloc/LocalFree/HeapAlloc/HeapFree/HeapReAlloc/HeapSize/HeapDestroy/VirtualAlloc/VirtualFree/GlobalFree), time (GetSystemTime/GetSystemTimeAsFileTime/FileTimeToSystemTime/GetLocalTime/GetTickCount/GetTickCount64/QueryPerformanceCounter/QueryPerformanceFrequency/SystemTimeToFileTime/SystemTimeTzSpecificLocalTime/TzSpecificLocalTimeToSystemTime/SetWaitableTimer/CreateWaitableTimerW), encoding (MultiByteToWideChar/WideCharToMultiByte), process (CreateFileMappingW/MapViewOfFile/UnmapViewOfFile), ASLR (EncodePointer/DecodePointer), debug (IsDebuggerPresent/RaiseException/OutputDebugStringA/FlushInstructionCache), environment (GetEnvironmentVariableW), temp (GetTempPathW/GetTempFileNameW), thread locale (GetThreadUILanguage/GetUserDefaultLCID), version (GetVersion/GetVersionExW), drive type (GetDriveTypeW), error (GetLastError/SetLastError), misc (DisableThreadLibraryCalls/SetErrorMode/IsProcessorFeaturePresent/Sleep/InterlockedIncrement/InterlockedDecrement/InterlockedExchange/InterlockedCompareExchange/InitializeSListHead/InterlockedPopEntrySList/InterlockedPushEntrySList/CompareStringW/CompareFileTime)
- **USER32.dll** (45 functions): Window management (CreateWindowExW/DestroyWindow/RegisterClassExW/RegisterClassW/GetClassInfoExW/UnregisterClassW/DefWindowProcW/CallWindowProcW), message loop (GetMessageW/TranslateMessage/DispatchMessageW/PeekMessageW/PostMessageW/PostQuitMessage/PostThreadMessageW/MsgWaitForMultipleObjects/MsgWaitForMultipleObjectsEx/CallMsgFilterW), UI (SetWindowTextW/GetWindowTextW/MessageBoxW/FindWindowExW/SendMessageW/SendNotifyMessageW), timer (SetTimer/KillTimer), DC (GetDC/ReleaseDC), rect (IntersectRect/InflateRect/SetRect/CopyRect/IsRectEmpty), misc (CharNextW/CharLowerBuffW/GetWindowLongW/SetWindowLongW/IsWindow/GetClassInfoExW/LoadCursorW/RegisterWindowMessageW)
- **ADVAPI32.dll** (30 functions): Registry (RegCreateKeyExW/RegOpenKeyExW/RegSetValueExW/RegQueryValueExW/RegDeleteKeyW/RegDeleteValueW/RegEnumKeyExW/RegEnumValueW/RegQueryInfoKeyW/RegOpenCurrentUser), cryptography (CryptAcquireContextW/CryptCreateHash/CryptHashData/CryptGetHashParam/CryptDestroyHash/CryptReleaseContext), ETW tracing (RegisterTraceGuidsW/TraceEvent/EventRegister/EventUnregister/EventWrite/GetTraceLoggerHandle/GetTraceEnableLevel/GetTraceEnableFlags/UnregisterTraceGuids), security (GetNamedSecurityInfoW/SetNamedSecurityInfoW), file encryption (EncryptFileW/DecryptFileW)
- **GDI32.dll** (8 functions): Bitmap (CreateDIBSection/CreateCompatibleDC/GetDIBits/GetDeviceCaps/GetObjectW), DC (DeleteDC/SelectObject/DeleteObject)

### COM
- **ole32.dll** (14 functions): COM core (CoInitialize/CoUninitialize/CoCreateInstance/CoGetClassObject/CoInitializeSecurity/CoGetMalloc), GUID (CoCreateGuid/CLSIDFromString/IIDFromString/StringFromGUID2), property (PropVariantClear/PropVariantCopy), memory (CoTaskMemAlloc/CoTaskMemRealloc/CoTaskMemFree), context (CreateBindCtx)
- **OLEAUT32.dll** (22 ordinal-only): Ordinals 2,4,6,7,8,9,10,12,15,16,17,19,20,21,22,27,77,150,184,185,277,411 — COM automation (SysAllocString/SysFreeString/SysStringLen/VariantInit/VariantClear/VariantCopy/VariantChangeType/etc.)
- **PROPSYS.dll** (4 functions): Property system (InitPropVariantFromStringAsVector/PSCoerceToCanonicalValue/PropVariantToVariant/PSGetPropertyKeyFromName)

### Network
- **WINHTTP.dll** (14 functions): HTTP client (WinHttpOpen/WinHttpConnect/WinHttpOpenRequest/WinHttpSendRequest/WinHttpReceiveResponse/WinHttpReadData/WinHttpQueryDataAvailable/WinHttpQueryHeaders/WinHttpSetOption/WinHttpSetStatusCallback/WinHttpAddRequestHeaders/WinHttpCloseHandle/WinHttpGetIEProxyConfigForCurrentUser/WinHttpGetProxyForUrl)

### Shell / UI
- **SHLWAPI.dll** (28 functions): Path utilities (PathFileExistsW/PathFindFileNameW/PathFindExtensionW/PathIsRelativeW/PathIsRootW/PathAppendW/PathAddBackslashW/PathRemoveBackslashW/PathRemoveFileSpecW/PathStripToRootW/PathStripPathW/PathIsDirectoryW/PathIsFileSpecW/PathIsNetworkPathW), string (StrCmpIW/StrStrIW/StrRChrW/StrRetToBSTR/SHStrDupW), registry (SHRegGetValueW/SHRegOpenUSKeyW/SHRegGetUSValueW/SHRegEnumUSValueW/SHRegCloseUSKey/SHOpenRegStream2W), stream (SHCreateStreamOnFileW/SHCreateStreamOnFileEx), misc (AssocGetPerceivedType/SHEnumKeyExW/PathIsUNCW)
- **gdiplus.dll** (11 functions): GDI+ bitmap (GdipCreateBitmapFromScan0/GdipCloneBitmapAreaI/GdipBitmapLockBits/GdipBitmapUnlockBits/GdipCreateHBITMAPFromBitmap/GdipCloneImage/GdipDisposeImage/GdipGetImageWidth/GdipGetImageHeight/GdipGetImagePixelFormat/GdipAlloc/GdipFree)

### Windows Live DLLs
- **WLXPhotoBase.dll** (13 functions): `Base::Exception` (ctor/copy/dtor/operator), `Base::Throw/ThrowLastError`, `Base::IsOutOfMemoryError`, `Base::IsWin7OrGreater/IsWin8OrGreater`, `Base::GdiplusStatusToHresult`, `Base::GetBaseStringManager`, `BasePrivate::New/Delete`, `ATL::BaseAtlThrow`
- **WLXPhotoSqm.dll** (9 functions): `Sqm::Startup/Shutdown/IsEnabled`, `Sqm::AddToStream` (3 overloads), `Sqm::Set`, `Sqm::DeferReportMedian`, `Sqm::StartTimer/PauseTimer`
- **DmxBici.dll** (1 function): `BiciWrapper::AddToStream` — telemetry

### Version
- **VERSION.dll** (3 functions): File version (GetFileVersionInfoW/GetFileVersionInfoSizeW/VerQueryValueW)

### Delay-Loaded
- **SHELL32.dll** (6 functions): Shell (SHCreateItemFromParsingName/SHGetSpecialFolderPathW/SHBindToParent/SHParseDisplayName/SHChangeNotify, ordinal 16)
- **MetadataSys.dll** (1 function): `WLXPSGetItemPropertyHandler`
- **MPR.dll** (1 function): `WNetGetUniversalNameW` — network path resolution

## Database Architecture

### Object Database Schema

The DLL implements a custom object database for photo library storage. From string analysis, the schema is:

#### Tables

| Table | Columns | Purpose |
|-------|---------|---------|
| `tblObject` | ObjectID, PathID, Filename, FileVolumeID, FilePathID, FileType, FileSize, ObjectSig, ThumbnailMoniker | Core photo/video file metadata |
| `tblPath` | PathID, VolumeIDAndPath, RootPaths, Path, ParentPathID | File path storage with volume tracking |
| `tblPerson` | PersonID, SourceAndObjectID, PersonUsageCount | Person/face identity |
| `tblRegion` | RegionID, ObjectID, PersonID, RegionLeft, RegionTop, RegionWidth, RegionHeight, RegionMatchStatus | Face region rectangles |
| `tblPublishedObject` | PublishedObjectID, ObjectID, RemoteObjectID, DateObjectModified | Online publishing state |
| `RecycleTable` | UniqueFileID | Deleted items tracking |

#### Indexes

- `TopRegionMatchAndObject` — optimized face match queries
- `PersonIDTopMatchedPersonID` — person matching
- `MatchedPersonID1-5` — multi-match face tracking
- `IgnoredMatchRegionID`, `IgnoredMatchPersonID` — ignored matches
- `WordObjectID`, `TableID`, `LabelObjectID`, `LocationObjectID` — text search indexes

#### Schema Metadata Queries

```sql
-- Standard catalog queries found in strings:
SELECT * FROM TABLE_CATALOG
SELECT * FROM TABLE_SCHEMA
SELECT * FROM TABLE_NAME
SELECT * FROM INDEX_CATALOG
SELECT * FROM INDEX_SCHEMA
SELECT * FROM INDEX_NAME
```

### Property System

The database integrates with the Windows Property System:

| Property | Type | Purpose |
|----------|------|---------|
| `System.Photo.DateTaken` | Date | Photo taken date |
| `System.Image.HorizontalSize` | UINT32 | Image width |
| `System.Image.VerticalSize` | UINT32 | Image height |
| `System.Photo.CameraManufacturer` | String | Camera maker |
| `System.Photo.CameraModel` | String | Camera model |
| `System.Photo.ExposureTime` | Double | Shutter speed |
| `System.Photo.FocalLength` | Double | Lens focal length |
| `System.Photo.FNumber` | Double | Aperture |
| `System.Photo.ISOSpeed` | UINT32 | ISO rating |
| `System.Photo.Aperture` | Double | Aperture value |
| `System.Photo.ShutterSpeed` | Double | Shutter speed value |
| `System.Image.ImageID` | String | Unique image ID |
| `System.Photo.Orientation` | UINT16 | EXIF orientation |

### Microsoft Photo Metadata (XMP)

#### Face/Region Tags
- `http://ns.microsoft.com/photo/1.2/` namespace
- `RegionInfo` → `Region` → `Rectangle`, `PersonDisplayName`, `PersonEmailDigest`, `PersonLiveIdCID`, `PersonSourceID`

#### Location Tags
- IPTC location: `LocationCreatedCountryName`, `LocationCreatedProvinceState`, `LocationCreatedCity`, `LocationCreatedSublocation`
- `Microsoft.Photos.LocationTag` — location metadata tag
- `Microsoft.Photos.PeopleRegion` — people region tag

### Thumbnail Management

| Class | Purpose |
|-------|---------|
| `ObjectDatabase` | Core database operations |
| `FileScanner` | File system scanning |
| `Metadata` | EXIF/XMP metadata reading |
| `ThumbnailMoniker` | Thumbnail storage/retrieval |
| `FaceThumbnailDatabase` | Face-specific thumbnail cache |
| `FaceThumbs.fd1` | Face thumbnail data file |
| `FaceExemplars.ed1` | Face exemplar data file |

### Face Detection/Recognition Pipeline

The DLL orchestrates face detection and recognition tasks:

| Function | Purpose |
|----------|---------|
| `EnsureFaceDataTaskStarted` | Initialize face data extraction pipeline |
| `EnsureFaceRecognitionTaskStarted` | Initialize face recognition pipeline |
| `EnsureFaceTileTaskStarted` | Initialize face tile generation |
| `UpdateFaceDetectionForObject` | Run face detection on single object |
| `UpdateFaceRecognition` | Run face recognition on detected regions |
| `UpdateFaceRecognitionForObjects` | Batch face recognition |
| `UpdateFaceGrouping` | Cluster/group recognized faces by person |
| `SetRegionAsDefaultFaceTile` | Set default face thumbnail |
| `ScheduleFaceThumbnailGenerationForObject` | Queue face thumbnail generation |
| `ScheduleFaceThumbnailGenerationForRegion` | Queue region-specific thumbnail |

### Task Messages

From string constants:
- `FaceRecognitionTaskMessages_GroupingStopped/Started`
- `FaceRecognitionTaskMessages_FaceRecoStopped/Started`
- `FaceRecognitionTaskMessages_UnadviseFaceRecoStatus/AdviseFaceRecoStatus`
- `FaceRecognitionTaskMessages_SignalImportAddToScope`
- `FaceRecognitionTaskMessages_PendingExemplarCount`

### Performance Metrics (SQM)

- `Total face reco run time: %f for %d regions`
- `Total representation load time: %f`
- `Total aggregate recognition job execution time: %f`
- `Average reco time per region: %f`
- `Average load time per region: %f`
- `Average match write time per region: %f`

### Geocoding (Bing Maps)

Full Bing Maps SOAP API integration for reverse geocoding:

#### Search API
```xml
<Search xmlns="http://dev.virtualearth.net/webservices/v1/search/contracts">
  <Credentials><ApplicationId>%s</ApplicationId></Credentials>
  <Culture>%s</Culture>
  <Query>%s</Query>
</Search>
```

#### Geocode API
```xml
<Geocode xmlns="http://dev.virtualearth.net/webservices/v1/geocode/contracts">
  <Credentials><ApplicationId>%s</ApplicationId></Credentials>
  <Latitude>%.3f</Latitude>
  <Longitude>%.3f</Longitude>
</Geocode>
```

#### Reverse Geocode
Converts lat/long to `CountryRegion`, city, state/province for location tagging.

### Thumbnail Cache

Registry-based cache configuration:
- `SOFTWARE\Microsoft\Windows\CurrentVersion\ThumbnailCache`
- `MaxCacheFileSize256` — 256px thumbnail cache
- `MaxCacheFileSize256SlowReclaim` — slow-reclaim variant
- `MaxCacheFileSize1024` — 1024px thumbnail cache

## Registry Paths

| Registry Path | Purpose |
|---------------|---------|
| `\Microsoft\Windows Live Photo Gallery` | Main library path |
| `Software\Microsoft\Windows Live\Photo Gallery\Library` | Library settings |
| `Software\Microsoft\Windows Live\Photo Gallery\Library\MDSyncInFlight\Active` | MDSync active flag |
| `Software\Microsoft\Windows Photo Gallery\Library` | Legacy gallery paths |
| `DBPath_W4.0`, `DBPath_W3.0`, `DBPath` | Database file paths |
| `EnableFaceDetection` | Face detection toggle |
| `FaceRecognitionThreshold` | Face recognition threshold |
| `SOFTWARE\Microsoft\Windows\CurrentVersion\ThumbnailCache` | Thumbnail cache config |
| `SYSTEM\CurrentControlSet\Control\ContentIndex\Language` | Content indexing |
| `Software\Microsoft\Windows Live\Photo Gallery\Viewer` | Viewer settings |
| `Software\Microsoft\Windows Live\Photo Gallery\FileSupport\*` | File type support |

## Dependencies Summary

| DLL | Functions | Role |
|-----|-----------|------|
| MSVCR110.dll | 72 | CRT core |
| KERNEL32.dll | 108 | OS kernel |
| USER32.dll | 45 | Window/message |
| ADVAPI32.dll | 30 | Registry + ETW + Crypto |
| GDI32.dll | 8 | Graphics |
| ole32.dll | 14 | COM core |
| OLEAUT32.dll | 22 ordinals | COM automation |
| PROPSYS.dll | 4 | Property system |
| WINHTTP.dll | 14 | HTTP (Bing Maps) |
| SHLWAPI.dll | 28 | Path/string utilities |
| gdiplus.dll | 11 | GDI+ bitmap |
| WLXPhotoBase.dll | 13 | Memory/exception base |
| WLXPhotoSqm.dll | 9 | SQM telemetry |
| DmxBici.dll | 1 | Telemetry |
| VERSION.dll | 3 | File version info |
| SHELL32.dll (delay) | 6 | Shell integration |
| MetadataSys.dll (delay) | 1 | Metadata properties |
| MPR.dll (delay) | 1 | Network path resolution |

## Architecture Summary

WLXPhotoLibraryDatabase.dll is the database and face processing backbone of Windows Live Photo Gallery 2012. At 681 KB of code (800 KB total), it implements a custom object database with 6 tables for photo/video metadata, person/face tracking, path management, and published object tracking. The DLL handles the complete face detection→recognition→grouping pipeline, including thumbnail generation, exemplar caching, and performance metrics. It integrates with Bing Maps for reverse geocoding of GPS-tagged photos and uses WINHTTP for SOAP API calls. The DLL exports 17 functions — mostly task management and database access — providing the storage layer that WLXPhotoLibraryMain.dll consumes. It connects to the property system via PROPSYS.dll and reads EXIF/XMP metadata via MetadataSys.dll.
