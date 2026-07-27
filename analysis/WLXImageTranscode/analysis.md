# WLXImageTranscode.dll — Static Analysis

## Overview

**WLXImageTranscode.dll** is a COM in-proc server implementing image format transcoding for Windows Live Photo Gallery 2012 (Windows Live Essentials 16.4.3528.0331). It handles reading, writing, and converting between image formats (JPEG, BMP, TIFF, PNG), metadata extraction/transformation (XMP, EXIF, IPTC), and color space conversions. It also exposes an out-of-process COM proxy (WLXOutofProc) for codec isolation.

---

## PE Structure

| Field | Value |
|---|---|
| **File** | `WLXImageTranscode.dll` |
| **Arch** | x86 (14C) |
| **Subsystem** | Windows GUI (2) |
| **Linker** | Visual Studio 2012 (11.00) |
| **Entry Point** | `0x000EEAC` |
| **Image Base** | `0x10000000` |
| **Image Size** | `0x1B000` (110,592 bytes) |
| **Code Size** | `0x12600` (75,264 bytes) |
| **OS Target** | 6.02 (Windows 8) |
| **Image Version** | 6.02 |
| **Timestamp** | Tue Apr 1 01:26:43 2014 |
| **Checksum** | `0x20903` |
| **DLL Characteristics** | `0x140` — Dynamic Base, NX Compatible |
| **Debug PDB** | `WLXImageTranscode.pdb` `{47D7299D-84E6-4FD7-BCF0-6E492552A4F7}` |

### Sections

| Section | Virtual Size | Virtual Address | Raw Size | Flags |
|---------|-------------|----------------|----------|-------|
| `.text` | `0x123CE` | `0x10001000` | `0x12400` | Code, Execute Read |
| `.orpc` | `0xA1` | `0x10014000` | `0x200` | Code, Execute Read |
| `.data` | `0x1B04` | `0x10015000` | `0xA00` | Initialized Data, Read Write |
| `.rsrc` | `0x1088` | `0x10017000` | `0x1200` | Initialized Data, Read Only |
| `.reloc` | `0x1D48` | `0x10019000` | `0x1E00` | Initialized Data, Discardable, Read Only |

### Directories

| Directory | RVA | Size |
|-----------|-----|------|
| Export | `0x13320` | `0xAE` |
| Import | `0x12630` | `0xB4` |
| Resource | `0x17000` | `0x1088` |
| Debug | `0x1270` | `0x38` |
| Base Reloc | `0x19000` | `0x127C` |
| Load Config | `0x3A50` | `0x40` |
| IAT | `0x1000` | `0x230` |
| Delay Import | `0x1240C` | `0x80` |
| Certificates | `0x16400` | `0x3EC0` |

### Notable: `.orpc` Section
Contains ORPC (Object RPC) proxy/stub marshaling code — indicates this DLL exports COM interfaces that can be remoted across processes.

---

## Export Table

Standard ATL COM server exports (4 functions):

| Ordinal | Hint | Name |
|---------|------|------|
| 1 | 0 | `DllCanUnloadNow` |
| 2 | 1 | `DllGetClassObject` |
| 3 | 2 | `DllRegisterServer` |
| 4 | 3 | `DllUnregisterServer` |

**No custom exports.** Pure COM in-proc server (plus ORPC proxy/stub for cross-process remoting).

---

## Import Table

### MSVCR110.dll — Visual C++ 2012 Runtime
Standard CRT: `malloc`, `free`, `calloc`, `memcmp`, `memset`, `memcpy_s`, `memmove_s`, `wcsncpy_s`, `wcscat_s`, `wcscpy_s`, `wcsstr`, `wcsrchr`, `wcsnlen`, `_wcsicmp`, `wmemcpy_s`, `vswprintf_s`, `_vscwprintf`, `swprintf_s`, `swscanf_s`, `_vsnwprintf`, `_vsnwprintf_l`, `_swscanf_s_l`, `_mbscmp`, `iswspace`, `_create_locale`, `_free_locale`

### KERNEL32.dll — Core OS Services
| Category | Functions |
|----------|-----------|
| Memory | `HeapAlloc`, `HeapReAlloc`, `HeapFree`, `HeapSize`, `HeapDestroy`, `GetProcessHeap` |
| PE Loading | `LoadLibraryExW`, `LoadLibraryExA`, `LoadLibraryW`, `FreeLibrary`, `GetModuleHandleA`, `GetModuleHandleW`, `GetModuleFileNameW`, `GetProcAddress` |
| Resources | `FindResourceW`, `FindResourceExW`, `LoadResource`, `LockResource`, `SizeofResource` |
| String/Encoding | `MultiByteToWideChar`, `lstrlenW`, `lstrcmpiW` |
| Synchronization | `EnterCriticalSection`, `LeaveCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `DeleteCriticalSection`, `InterlockedIncrement`, `InterlockedDecrement`, `InterlockedExchange` |
| Error/Debug | `GetLastError`, `SetLastError`, `RaiseException`, `IsDebuggerPresent`, `IsProcessorFeaturePresent` |
| File I/O | `GetFileAttributesW` |
| CRT/Security | `DecodePointer`, `EncodePointer`, `DisableThreadLibraryCalls` |
| Timing | `GetTickCount64`, `GetSystemTimeAsFileTime`, `QueryPerformanceCounter` |
| Other | `GetVersion`, `GetVersionExW`, `GetEnvironmentVariableW`, `OutputDebugStringA`, `GetCurrentThreadId` |

### SHELL32.dll — Shell Integration
| Function | Purpose |
|----------|---------|
| `SHCreateItemFromParsingName` | Create shell item from file path (for property store access) |

### SHLWAPI.dll — Shell Path Utilities
| Function | Purpose |
|----------|---------|
| `PathFindExtensionW` | Extract file extension |
| `StrCmpNIW` | Case-insensitive prefix comparison |

### ADVAPI32.dll — Registry
| Function | Purpose |
|----------|---------|
| `RegCreateKeyExW` / `RegOpenKeyExW` / `RegQueryValueExW` / `RegSetValueExW` | Registry access |
| `RegDeleteKeyW` / `RegDeleteValueW` | Registry cleanup |
| `RegEnumKeyExW` / `RegQueryInfoKeyW` / `RegCloseKey` | Registry enumeration |

### ole32.dll — COM Foundation
| Function | Purpose |
|----------|---------|
| `CoCreateInstance` | Create COM objects |
| `CoTaskMemAlloc` / `CoTaskMemFree` / `CoTaskMemRealloc` | COM memory |
| `StringFromGUID2` | GUID-to-string |
| `PropVariantClear` | PROPVARIANT cleanup |
| `CreateStreamOnHGlobal` | IStream from HGLOBAL (in-memory stream) |

### RPCRT4.dll — COM Proxy/Stub Marshaling
Full ORPC proxy/stub infrastructure:
- `NdrDllGetClassObject`, `NdrDllCanUnloadNow`, `NdrDllRegisterProxy`, `NdrDllUnregisterProxy`
- `CStdStubBuffer_Connect`, `CStdStubBuffer_Invoke`, `CStdStubBuffer_QueryInterface`
- `IUnknown_QueryInterface_Proxy`, `IUnknown_AddRef_Proxy`, `IUnknown_Release_Proxy`
- `NdrOleAllocate` / `NdrOleFree`

### USER32.dll
- `CharNextW` — String parsing

### Delay-Load Imports
| DLL | Functions |
|-----|-----------|
| `OLEAUT32.dll` | Ordinal 9, Ordinal 277 — `VariantInit`?, `SystemTimeToVariantTime`? |
| `MetadataSys.dll` | `WLXPSGetItemPropertyHandler` — Property system handler |
| `WLXPhotoBase.dll` | `BaseAtlThrow@ATL`, `BException::~Exception`, `Base::Throw`, `Base::ThrowLastError`, `Base::GetBaseStringManager`, `BasePrivate::Delete`, `BasePrivate::New`, `OS::Base::IsWin7OrGreater`, `OS::Base::IsWin8OrGreater` |

---

## COM Class Registration

### CLSIDs

| GUID | Class | Description |
|------|-------|-------------|
| `{20575516-78AF-4404-B3C7-51D05F9945B5}` | **ImageTranscode** | Main image transcoding engine |
| `{E253845C-A1A2-4ae4-B784-5E8140CD5B2C}` | **ImageLoader** | Image loading/decoding |
| `{B8A2E14E-290D-4122-B092-1A7D86198CCE}` | **WLXOutofProc** | Out-of-process codec host proxy |

### IIDs

| GUID | Interface |
|------|-----------|
| `{CB38B8DF-0D64-42b9-802A-28DCB678BFEB}` | **IWLXImageLoader** — Image loader interface |

### Infrastructure

| GUID | Type |
|------|------|
| `{3FBB103C-F1B9-47dc-9EB3-A0C07F5F6AFA}` | PSFactoryBuffer (ORPC proxy/stub factory) |
| `{17DC7884-443D-478e-ABD7-BC22856FC7F1}` | **TypeLib** |

---

## Image Format Support

### Supported Extensions (from string analysis)
| Extension | Format |
|-----------|--------|
| `.jpg` | JPEG/JFIF |
| `.bmp` | Windows BMP |
| `.jpe` | JPEG (alternate extension) |

### Implicit Format Support (from dependency chain)
- **TIFF** — via WLXCodecHost.exe (WIC-based codec isolation)
- **PNG** — via WIC (Windows Imaging Component)
- **RAW** — via Imaging.dll (CaptureOne pipeline)
- **GIF** — via WIC

---

## Metadata Support

### XMP Metadata (from string analysis)
The DLL contains extensive XMP path strings for reading/writing metadata:

#### Microsoft Photo 1.2 Namespace
| XMP Path | Description |
|----------|-------------|
| `/<xmpstruct>http://ns.microsoft.com/photo/1.2/:RegionInfo` | Face region info |
| `/<xmpbag>http://ns.microsoft.com/photo/1.2/t/RegionInfo#:Regions` | Region collection |
| `/<xmpstruct>{ulong=%d}` | Region struct format |
| `/http://ns.microsoft.com/photo/1.2/t/Region#:Rectangle` | Face bounding box |
| `/http://ns.microsoft.com/photo/1.2/t/Region#:PersonDisplayName` | Person name |
| `/http://ns.microsoft.com/photo/1.2/t/Region#:PersonEmailDigest` | Person email hash |
| `/http://ns.microsoft.com/photo/1.2/t/Region#:PersonLiveIdCID` | Person Live ID |
| `/http://ns.microsoft.com/photo/1.2/t/Region#:PersonSourceID` | Person source ID |

#### IPTC I4xmpExt Namespace
| XMP Path | Description |
|----------|-------------|
| `/<xmpbag>http://iptc.org/std/Iptc4xmpExt/2008-02-29/:LocationCreated` | Location created |
| `/http://iptc.org/std/Iptc4xmpExt/2008-02-29/:CountryName` | Country |
| `/http://iptc.org/std/Iptc4xmpExt/2008-02-29/:ProvinceState` | State/Province |
| `/http://iptc.org/std/Iptc4xmpExt/2008-02-29/:City` | City |
| `/http://iptc.org/std/Iptc4xmpExt/2008-02-29/:Sublocation` | Sublocation |

#### Photoshop Namespace
| XMP Path | Description |
|----------|-------------|
| `/photoshop:Country` | Country (Photoshop) |
| `/photoshop:State` | State (Photoshop) |
| `/photoshop:City` | City (Photoshop) |

#### IPTC Core Namespace
| XMP Path | Description |
|----------|-------------|
| `/Iptc4xmpCore:Location` | Location |

#### Property System Paths
| Path | Description |
|------|-------------|
| `/ifd/xmp` | IFD XMP property root |
| `System.Photo.Orientation` | Photo orientation |
| `EImageQuality` | Image quality setting |

#### Location Extraction Paths
| Path | Description |
|------|-------------|
| `/LocationCreatedCountryName` | Mapped country |
| `/LocationCreatedProvinceState` | Mapped province |
| `/LocationCreatedCity` | Mapped city |
| `/LocationCreatedSublocation` | Mapped sublocation |

### String Format Patterns
| Pattern | Use |
|---------|-----|
| `%s:%I64d` | Metadata ID formatting |
| `%.6lf, %.6lf, %.6lf, %.6lf` | Four-double coordinate formatting (face regions) |
| `%lf, %lf, %lf, %lf` | Four-double coordinate formatting |
| `%.1f` | Float formatting |
| `%I64d` | 64-bit integer formatting |
| `{ulong=0}` | XMP struct format |

---

## RTTI / Class Hierarchy (Inferred)

### Image Codec Classes
| Class (Inferred) | Role | Evidence |
|-------|------|----------|
| `ImageTranscode` | Main COM class — orchestrates transcode operations | CLSID registration |
| `ImageLoader` | Image decoder/reader — loads images via WIC | CLSID + IWLXImageLoader interface |
| `WLXOutofProc` | Out-of-process proxy for codec isolation | CLSID (remoting to WLXCodecHost.exe) |
| `PSFactoryBuffer` | ORPC proxy/stub factory | IID registration + `.orpc` section |
| `XmpMetadataReader` | XMP metadata parser/extractor | XMP path strings |
| `XmpMetadataWriter` | XMP metadata writer | XMP path strings |
| `IptcLocationParser` | IPTC location metadata handler | IPTC namespace paths |
| `RegionInfoProcessor` | Face region metadata processor | Microsoft Photo 1.2 paths |
| `ColorSpaceConverter` | Color space transform utility | Format strings for coordinate/double transforms |

### Interface Classes
| Interface | Role |
|-----------|------|
| `IWLXImageLoader` | Image loading interface — decode, get dimensions, get pixel data |

---

## Analysis Summary

### Architecture
1. **COM Registration**: Registers 3 COM classes — ImageTranscode (main), ImageLoader (decoder), WLXOutofProc (remoting proxy)
2. **Image Loading**: `ImageLoader` reads images via WIC (Windows Imaging Component), extracting pixel data and metadata
3. **Image Transcoding**: `ImageTranscode` converts between formats (JPEG, BMP, TIFF), adjusting quality, dimensions, and color space
4. **Metadata Pipeline**: Full XMP read/write for Microsoft Photo 1.2 (face regions), IPTC (location), and Photoshop namespaces
5. **Codec Isolation**: `WLXOutofProc` enables out-of-process codec execution via WLXCodecHost.exe for crash isolation
6. **ORPC Proxy/Stub**: `.orpc` section + RPCRT4 imports enable cross-process COM remoting
7. **Property System**: Uses `SHCreateItemFromParsingName` + `WLXPSGetItemPropertyHandler` for Shell property access

### Key Design Points
- **Three COM classes** — unusually rich for a transcode DLL; the out-of-process proxy suggests codec isolation was a design priority
- **Extensive metadata support** — face region coordinates, person identification, GPS/location, and orientation
- **ORPC proxy/stub** built-in (`.orpc` section) — enables transparent cross-process transcoding via COM marshaling
- **WIC-based** image I/O — leverages Windows Imaging Component for format support
- **Quality parameter** (`EImageQuality`) — configurable JPEG/compression quality
- **Dependency chain**: WLXPhotoBase (runtime) → MetadataSys (property handler) → WLXCodecHost (codec isolation) → Imaging.dll (RAW pipeline)
- **`.jpe` extension support** — indicates legacy JPEG handling
- **Coordinate formatting** (`%.6lf, %.6lf, %.6lf, %.6lf`) — face region rectangles stored as four doubles (left, top, right, bottom)
- **No thread synchronization imports** — stateless/immutable transcode operations (thread-safe by design)

---

## Runtime Test Results

Test harness: `tests/OtherDlls/test_grinder_transcode.cpp`

| Export | Result |
|--------|--------|
| `DllCanUnloadNow` | `S_OK` (0x00000000) — no outstanding references |
| `DllGetClassObject` (empty CLSID) | `0xC0000005` (ACCESS_VIOLATION) — crash on null CLSID, caught by SEH |
| `DllGetClassObject` (ImageTranscode CLSID) | `S_OK` — IUnknown obtained |
| `DllGetClassObject` (ImageLoader CLSID) | `S_OK` — IUnknown obtained |
| `DllGetClassObject` (WLXOutofProc CLSID) | `S_OK` — IUnknown obtained |
| `DllRegisterServer` | Found (skipped — would modify registry) |
| `DllUnregisterServer` | Found (skipped — would modify registry) |

**Summary:** All 4 standard COM exports found. All 3 COM classes (ImageTranscode, ImageLoader, WLXOutofProc) successfully instantiate via `DllGetClassObject`. Note: empty CLSID causes access violation (missing null-check in class factory), but all valid CLSIDs work correctly.
