# WLXVAFilt.dll - Static Analysis

## Overview
**WLXVAFilt.dll** is a **DirectShow filter registration DLL** providing video/audio acquisition filters for Windows Photo Gallery (Windows Essentials 2012). It registers **8 custom DirectShow filters** used for video capture, encoding, format conversion, and transcoding pipelines. The DLL acts as a self-registering COM in-process server, using `IFilterMapper2` to register its filters with the DirectShow filter graph infrastructure.

**PDB:** `WLXVAFILT.pdb` (GUID: `{D4910ADE-5301-4BE0-8C76-EFEBA715FE1F}`)

**Note:** The export directory header string says "shotb.dll" - this is the original internal module name (likely "shot board" / capture board). The FileDescription in the version resource confirms: *"Photo Gallery Video Acquisition Filters"*.

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Linker | MSVC 11.00 (VS2012) |
| Timestamp | 2014-04-01 01:27:55 UTC |
| Image Base | 0x10000000 |
| Size of Image | 0x24000 (144 KB) |
| Entry Point | 0x1A662 |
| Subsystem | Windows CUI (3) |
| DLL Characteristics | Dynamic base, NX compatible |
| Stack Reserve | 0x40000 (256 KB) |
| Heap Reserve | 0x100000 (1 MB) |

## Sections
| Name | VirtAddr | VirtSize | RawSize | Flags |
|------|----------|----------|---------|-------|
| .text | 0x1000 | 0x1B832 | 0x1BA00 | Code, Execute Read |
| .data | 0x1D000 | 0x1690 | 0xE00 | Initialized Data, Read Write |
| .rsrc | 0x1F000 | 0x9F0 | 0xA00 | Initialized Data, Read Only |
| .reloc | 0x20000 | 0x30EE | 0x3200 | Initialized Data, Discardable, Read Only |

## Exports (Standard COM In-Process Server)
| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | 0x13091 | `DllCanUnloadNow` |
| 2 | 1 | 0x130AF | `DllGetClassObject` |
| 3 | 2 | 0xA80D | `DllRegisterServer` |
| 4 | 3 | 0xA81A | `DllUnregisterServer` |

### DllCanUnloadNow (RVA 0x13091)
Checks two global counters at `0x1001E294` and `0x1001E2A0`. Returns `S_OK` (1) if either has outstanding references (can't unload), `S_FALSE` (0) if both are zero.

### DllGetClassObject (RVA 0x130AF)
- Compares input CLSID against two known CLSIDs at `0x10003B00` and `0x10003B10`
- If matched, allocates a `CClassFactory` object (via `0x1001A7E6`, which wraps `CoTaskMemAlloc`)
- If not matched, iterates a CLSID-to-creator table at `0x1001D408` (count at `0x1001D4A8`, entries 0x14 bytes each)
- Returns `E_NOINTERFACE` (0x80004002) if no CLSID matches, `CLASS_E_CLASSNOTAVAILABLE` (0x80040111) if table iteration fails

### DllRegisterServer / DllUnregisterServer (RVA 0xA80D / 0xA81A)
Both are thin wrappers calling the shared registration function at `0x10015CE2` with argument 1 (register) or 0 (unregister):
```asm
DllRegisterServer:
    push 1
    call 10015CE2
    ret

DllUnregisterServer:
    push 0
    call 10015CE2
    ret
```

The registration function at `0x10015CE2`:
1. Gets module filename via `GetModuleFileNameW`
2. Constructs path for filter self-registration
3. Creates `IFilterMapper2` instance via `CoCreateInstance` (referenced interface GUIDs at `0x10003008`/`0x10003098` and `0x10002FF8`/`0x10002F38`)
4. Iterates filter registration table at `0x1001D418` (count at `0x1001D4A8`, 0x14 bytes per entry)
5. For each filter, calls register function at `0x10015E68` (register) or `0x10015AEE` (unregister)

## Imports
### MSVCR110.dll (Visual C++ 2012 Runtime)
C++ runtime: `??2@YAPAXI@Z` (operator new), `??3@YAXPAX@Z` (operator delete), `??_U@YAPAXI@Z` (operator new[]), `??_V@YAXPAX@Z` (operator delete[]), `??1type_info@@UAE@XZ`, `_except_handler4_common`, `__CxxFrameHandler3`, `__CppXcptFilter`, `_initterm_e`, `_initterm`, `_malloc_crt`, `_calloc_crt`, `_recalloc`, `malloc`, `free`, `_amsg_exit`, `_purecall`, `memset`, `memcpy`, `memcpy_s`, `memmove`, `memmove_s`, `memcmp`, `wcsstr`, `wcsncpy_s`, `_vsnwprintf`, `_wtoi`, `_lock`, `_unlock`, `_onexit`, `__dllonexit`, `__crtTerminateProcess`, `__crtUnhandledException`

### KERNEL32.dll
Thread management (`CreateThread`, `GetCurrentThread`, `SetThreadPriority`, `WaitForSingleObject`, `WaitForMultipleObjects`), synchronization (`CreateEventW`, `CreateSemaphoreW`, `SetEvent`, `ResetEvent`, `InterlockedIncrement/Decrement/Exchange`, `InitializeCriticalSectionAndSpinCount`, `EnterCriticalSection`, `LeaveCriticalSection`), file I/O (`CreateFileW`, `ReadFile`, `WriteFile`, `SetFilePointerEx`, `SetEndOfFile`, `DeleteFileW`, `GetFileSize`, `GetFileType`), I/O completion ports (`CreateIoCompletionPort`, `PostQueuedCompletionStatus`, `GetQueuedCompletionStatus`), module management (`LoadLibraryExW`, `LoadLibraryW`, `GetProcAddress`, `FreeLibrary`, `GetModuleHandleW`, `GetModuleFileNameW/A`), memory (`GlobalMemoryStatus`), path operations (`GetTempPathW`, `GetTempFileNameW`, `GetFullPathNameW`, `GetDiskFreeSpaceW`), resource loading (`FindResourceW`, `LoadResource`, `SizeofResource`), misc (`Sleep`, `RaiseException`, `DisableThreadLibraryCalls`, `GetVersionExW`, `GetTickCount`, `GetTickCount64`, `lstrlenW`, `lstrlenA`, `lstrcmpW`, `lstrcmpiW`, `MultiByteToWideChar`)

### USER32.dll
COM message pump: `RegisterWindowMessageW`, `PeekMessageW`, `DispatchMessageW`, `PostThreadMessageW`, `MsgWaitForMultipleObjects`, `GetQueueStatus`, `CharNextW`, `DestroyWindow`, `LoadStringW`

### ADVAPI32.dll
Registry operations: `RegCreateKeyExW`, `RegCreateKeyW`, `RegOpenKeyExW`, `RegCloseKey`, `RegSetValueExW`, `RegSetValueW`, `RegQueryInfoKeyW`, `RegDeleteKeyW`, `RegDeleteValueW`, `RegEnumKeyExW`

### ole32.dll
COM infrastructure: `CoCreateInstance`, `CoInitialize`, `CoUninitialize`, `CoFreeUnusedLibraries`, `CoTaskMemAlloc`, `CoTaskMemRealloc`, `CoTaskMemFree`, `OleFlushClipboard`, `OleIsCurrentClipboard`, `StringFromGUID2`

### OLEAUT32.dll
Ordinal 277 only (likely `SysFreeString`)

### WINMM.dll
Multimedia timers: `timeBeginPeriod`, `timeEndPeriod`, `timeSetEvent`, `timeKillEvent` — used for precise timing in audio/video capture pipelines.

## Registered DirectShow Filters (8 total)

| # | Filter Name | RTTI Class | Role |
|---|------------|------------|------|
| 1 | **Record Queue** | `CRecordQueue` | Queues recorded media samples between capture and file writer |
| 2 | **WMT Switch Filter** | `CSwitchFilter` | Switches between multiple input streams (implements `IWMTSwitchFilterConfig`) |
| 3 | **WMT Virtual Renderer** | `CWMTVirtualRendererFilter` | Renders video to memory (virtual framebuffer), implements `IWMTVirtualRenderer` |
| 4 | **WMT Virtual Source** | `CWMTVirtualSourceFilter` | Sources video from memory (virtual capture), implements `IWMTVirtualSource` |
| 5 | **WMT DV Extract Filter** | `CDVExtractFilter` | Extracts audio/video from DV tapes, implements `IWMTDVExtractFilterSetup`, `IWMTDVInformation` |
| 6 | **WM VIH2 Fix** | `CWMTVIH2Fix` | Fixes VIDEOINFOHEADER2 structures, implements `IWMTVIH2FixSetup` |
| 7 | **WMT Sample Info Filter** | `CWMTSampleInformationFilter` | Injects/accesses sample metadata, implements `IWMTSampleInformationFilter` |
| 8 | **Capture File Writer** | `CFileWriterFilter` | Writes captured streams to ASF/file containers |

## RTTI Class Hierarchy (64 classes)

### Base Infrastructure
| Class | Description |
|-------|-------------|
| `CUnknown` | IUnknown base implementation (COM reference counting) |
| `CBaseObject` | Base class for all WMT objects |
| `CBuffer` | Media sample buffer |
| `CMemBuffer` | Memory-backed buffer |
| `CCritSec` | Critical section wrapper |

### File I/O
| Class | Description |
|-------|-------------|
| `CFileData` | Abstract file data accessor |
| `CMemFileData` | Memory-mapped file data |
| `CSampleFileData` | Sample-based file data reader |
| `CFileBuffer` | File-backed buffer |
| `CFileIo` | Asynchronous file I/O with completion port |
| `CSyncFileIo` | Synchronous file I/O wrapper |
| `CFwIStream` | IStream implementation for file writer |

### Queue System
| Class | Description |
|-------|-------------|
| `CRecordQueue` | Records media samples in a queue |
| `CRecordQueueOutputPin` | Output pin for record queue |
| `CRecordQueueInputPin` | Input pin for record queue |
| `CQueueMediaSample` | Queued media sample object |
| `CQueueCommand` | Queued command base |
| `CSampleCommand` | Sample command |
| `CNewSegmentCommand` | New segment command |
| `CEosCommand` | End-of-stream command |

### DirectShow Base Classes
| Class | Description |
|-------|-------------|
| `CBaseFilter` | DirectShow base filter |
| `CBasePin` | Base pin |
| `CBaseOutputPin` | Base output pin |
| `CBaseInputPin` | Base input pin |
| `CEnumPins` | Pin enumerator |
| `CEnumMediaTypes` | Media type enumerator |
| `IPersistStream` | Persistence interface |

### Transform Filters
| Class | Description |
|-------|-------------|
| `CTransformFilter` | Base transform filter |
| `CTransInPlaceFilter` | In-place transform filter |
| `CWMTTransInPlaceFilter` | WMT in-place transform |
| `CTransformInputPin` | Transform input pin |
| `CTransformOutputPin` | Transform output pin |
| `CTransInPlaceInputPin` | TransInPlace input pin |
| `CTransInPlaceOutputPin` | TransInPlace output pin |

### Virtual Source (Memory Capture)
| Class | Description |
|-------|-------------|
| `CWMTVirtualSourceFilter` | Virtual source filter |
| `CSource` | Source filter base |
| `CSourceStream` | Source output pin |
| `CSourceSeeking` | Seeking implementation |
| `CAMThread` | Worker thread for streaming |

### Virtual Renderer (Memory Rendering)
| Class | Description |
|-------|-------------|
| `CWMTVirtualRendererFilter` | Virtual renderer filter |
| `CBaseRenderer` | Renderer base |
| `CRendererInputPin` | Renderer input pin |
| `CVirtualRendererPosPassThru` | Position passthru for virtual renderer |
| `CPosPassThru` | Position passthru base |
| `CMediaPosition` | Media position implementation |

### DV / Format Specific
| Class | Description |
|-------|-------------|
| `CDVExtractFilter` | DV tape extraction (dvsd/dvhd/dvsl/dv25/dv50/dvh1) |
| `CWMTVIH2Fix` | VIDEOINFOHEADER2 structure fixup |
| `CWMTSampleInformationFilter` | Sample metadata injector |

### File Writer
| Class | Description |
|-------|-------------|
| `CBaseWriterFilter` | Base ASF/file writer |
| `CBaseWriterInput` | Writer input pin |
| `CFileWriterFilter` | Concrete file writer |
| `CSyncFileIo` | Synchronous I/O for writer |

### Graph Utilities
| Class | Description |
|-------|-------------|
| `CWMTGraphUtil` | Filter graph utilities |
| `CWMTMediaTypeUtil` | Media type manipulation utilities |
| `CSelectionMgr` | Stream selection manager |
| `CFireGun` | Event firing helper |

### ATL/WTL Module System
| Class | Description |
|-------|-------------|
| `CAtlModule` (ATL) | ATL module base |
| `CComModule` (ATL) | ATL COM module |
| `CRegObject` (ATL) | ATL registry object |
| `CAppModule` (WTL) | WTL application module |
| `CServerAppModule` (WTL) | WTL server application module |
| `CWMModule` | WMT module (extends WTL) |
| `CClassFactory` | COM class factory |

## Media Types & Formats

### DV Format FOURCCs
Standard DV video format identifiers defined in the .text section:
| FOURCC | Description |
|--------|-------------|
| `dvsd` | DV Standard Definition (NTSC 29.97fps, 25Mbps) |
| `dvhd` | DV High Definition |
| `dvsl` | DV Standard Definition (PAL 25fps, 25Mbps) |
| `dv25` | DV 25Mbps generic |
| `dv50` | DV 50Mbps |
| `dvh1` | DV High Definition variant |

### DRM Headers (Windows Media DRM)
The DLL handles DRM metadata attributes:
- `IsDRM`, `IsDRMCached`
- `BaseLAURL` (License Acquisition URL)
- `DRMHeader.KID` (Key ID)
- `DRMHeader.LAINFO` (License info)
- `DRMHeader.CID` (Content ID)
- `DRMHeader.SECURITYVERSION`
- `DRMHeader.ContentDistributor`
- `DRMHeader.SubscriptionContentID`

## Registry Structure

### COM Server Registration
```
HKCR\CLSID\{filter-CLSID}
  ForceRemove "WMT <FilterName>"
    InprocServer32 = "WLXVAFilt.dll"
      ThreadingModel = "Both"
```

### Filter Category Registration
Uses `IFilterMapper2` to register filters under:
- Hardware categories (video/audio capture devices)
- Component Categories
- FileType categories (file extensions)
- Interface categories
- Module / Module_Raw categories
- "Pass Through" category

## Property Pages

### Effect Page
- Bin &Y, Bin &U, Bin &V (color channel controls)
- Scale
- Duration
- Tooltip: *"The bins refer to color precision. Higher is more precise. 4,4,4 is typical. Must be a multiple of 2."*

### Timing Page
- Start, Length
- Tooltip: *"Duration is typically 5, scale is typically 5. Don't mess with these values unless you know what you're doing."*

## String References
| String | Purpose |
|--------|---------|
| `shotb.dll` | Internal module name (visible in export directory) |
| `WLXVAFILT.DLL` | Filename referenced in registration |
| `AMUnblock` | DirectShow thread unblock notification |
| `control.tlb` | Type library reference |
| `Module_Raw` | Filter category name |
| `Pass Through` | Filter category name |
| `Output filename` | File writer property |

## Version Information
| Field | Value |
|-------|-------|
| CompanyName | Microsoft Corporation |
| FileDescription | Photo Gallery Video Acquisition Filters |
| FileVersion | 16.4.3528.0331_ship.client.main.w5m4 (ship) |
| InternalName | WLXVAFilt |
| LegalCopyright | 2012 Microsoft Corporation. All rights reserved. |
| OriginalFilename | WLXVAFilt.dll |
| ProductName | Photo Gallery |
| ProductVersion | 16.4.3528.0331 |

## Dynamic Analysis Results

| # | Test | Result | Notes |
|---|------|--------|-------|
| 1 | `DllCanUnloadNow` | `S_OK` (0x00000000) | No outstanding references |
| 2 | `DllCanUnloadNow` (2nd call) | `S_OK` (0x00000000) | Idempotent |
| 3 | `DllGetClassObject` (empty CLSID, null ppv) | SEH `0xC0000005` | Access violation writing to null — expected |
| 4 | `DllGetClassObject` (empty CLSID, valid ppv) | `CLASS_E_CLASSNOTAVAILABLE` (0x80040111) | No object for unknown CLSID |
| 5 | `DllGetClassObject` (zeroed CLSID, valid ppv) | `CLASS_E_CLASSNOTAVAILABLE` (0x80040111) | Correctly rejected |
| 6 | `DllGetClassObject` (IClassFactory IID, valid ppv) | `CLASS_E_CLASSNOTAVAILABLE` (0x80040111) | Correctly rejected |
| 7 | `DllRegisterServer` | Found at `0x70CFA80D` | Skipped — would register 8 DirectShow filters |
| 8 | `DllUnregisterServer` | Found at `0x70CFA81A` | Skipped — would unregister filters |

**Module loads** at `0x70CF0000`. All 4 exports resolved and callable. Version info: 1852 bytes.

**Interpretation**: `DllGetClassObject` correctly rejects all unknown CLSIDs with `CLASS_E_CLASSNOTAVAILABLE`. The actual filter CLSIDs are stored in a relocation-dependent table at `0x1001D408` (runtime address after base relocation). The `CClassFactory` at `0x10012EF1` allocates filter objects via `CoTaskMemAlloc` and sets vtable pointers — the filter objects are created only when the correct CLSID is provided.

## Key Architecture Notes

1. **DirectShow Filter DLL** — This is not a general COM server. It registers DirectShow filters (not COM classes) via `IFilterMapper2::RegisterFilter`. Each filter has a CLSID, friendly name, and filter category memberships.

2. **Memory-Mapped Video Pipeline** — The Virtual Source + Virtual Renderer pair enables memory-mapped video transfer between processes without actual hardware, used for video preview in Photo Gallery.

3. **DV Tape Support** — The DV Extract Filter handles DV camcorder tape capture, supporting all standard DV format variants (NTSC/PAL/HD).

4. **ASF File Writing** — The File Writer filter writes captured video/audio to ASF (Advanced Systems Format) files, with full DRM metadata support.

5. **WTL Module System** — Uses WTL's `CServerAppModule` and `CWMModule` for module lifecycle management, supporting COM EXE/DLL server patterns.

6. **I/O Completion Ports** — `CFileIo` uses Windows I/O completion ports for asynchronous file read/write, enabling high-throughput video capture without blocking the streaming threads.

7. **Multimedia Timers** — `WINMM.dll` time functions provide sub-millisecond precision for A/V sync in capture pipelines.
