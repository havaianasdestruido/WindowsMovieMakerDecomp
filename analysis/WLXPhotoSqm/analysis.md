# WLXPhotoSqm.dll - Static Analysis

## PE Characteristics

| Property | Value |
|----------|-------|
| **File Type** | DLL (Dynamic Link Library) |
| **Machine** | x86 (0x14C) - 32-bit |
| **Linker Version** | 11.00 (MSVC 11.0 / VS2012) |
| **Image Base** | 0x10000000 |
| **Subsystem** | Windows GUI (2) |
| **DLL Characteristics** | Dynamic base, NX compatible |
| **Target OS** | Windows 6.2+ (Win8+) |
| **Entry Point** | 0x10010E7D |
| **Size of Image** | 0x20000 (128 KB) |
| **Stack Reserve** | 0x40000 (256 KB) |
| **Heap Reserve** | 0x100000 (1 MB) |
| **Timestamp** | Tue Apr 1 01:26:36 2014 (0x533A3FFC) |
| **PDB GUID** | {972BE92B-E2FA-404D-9A47-96FA92B41CCF} |
| **PDB File** | WLXPhotoSqm.pdb |

### Version Information (Resource Section)

| Property | Value |
|----------|-------|
| CompanyName | Microsoft Corporation |
| FileDescription | Photo Sqm Library |
| FileVersion | 16.4.3528.0331_ship.client.main.w5m4 (ship) |
| InternalName | WLXPhotoSqm |
| LegalCopyright | 2012 Microsoft Corporation. All rights reserved. |
| OriginalFilename | WLXPhotoSqm.dll |
| ProductName | Photo Gallery |
| ProductVersion | 16.4.3528.0331 |

## Section Analysis

| Section | Virtual Addr | Virtual Size | Raw Size | Flags | Purpose |
|---------|-------------|-------------|----------|-------|---------|
| .text | 0x1000 | 0x1847F | 0x18600 | Code, Execute Read | Compiled C++ code (~97 KB) |
| .data | 0x1A000 | 0x14DC | 0x0A00 | Initialized Data, RW | Global/static variables, vtables |
| .rsrc | 0x1C000 | 0x03D8 | 0x0400 | Initialized Data, RO | Version info, certificate resources |
| .reloc | 0x1D000 | 0x2656 | 0x2800 | Initialized Data, Discardable, RO | Base relocations (ASLR) |

**Notes:**
- .text section is large (97 KB) - substantial telemetry logic including HTTP client, WMI queries, config cab download
- Total virtual image size: 128 KB - mid-sized telemetry module
- .rsrc contains Microsoft Authenticode signatures and timestamp certificates

## Complete Export Table (44 functions)

All exports belong to the `Sqm` class with `__stdcall` calling convention (`__cdecl` not used).

### Lifecycle (3)

| Ordinal | Name | Signature |
|---------|------|-----------|
| 43 | `?Startup@Sqm@@YGXXZ` | `void Sqm::Startup(void)` |
| 42 | `?Startup@Sqm@@YGXW4SqmDmxAppId@1@@Z` | `void Sqm::Startup(SqmDmxAppId)` |
| 40 | `?Shutdown@Sqm@@YGXXZ` | `void Sqm::Shutdown(void)` |

### Configuration (6)

| Ordinal | Name | Signature |
|---------|------|-----------|
| 34 | `?SetAppStatusReportingMode@Sqm@@YGX_N@Z` | `void Sqm::SetAppStatusReportingMode(BOOL)` |
| 38 | `?SetOptInPreference@Sqm@@YGX_N@Z` | `void Sqm::SetOptInPreference(BOOL)` |
| 35 | `?SetApplicationMode@Sqm@@YGXK@Z` | `void Sqm::SetApplicationMode(DWORD)` |
| 33 | `?SetAppDefinedValue@Sqm@@YGXK@Z` | `void Sqm::SetAppDefinedValue(DWORD)` |
| 20 | `?EnableShipAsserts@Sqm@@YGXXZ` | `void Sqm::EnableShipAsserts(void)` |
| 24 | `?InitializeUserExecutedActionReporting@Sqm@@YGXKK@Z` | `void Sqm::InitializeUserExecutedActionReporting(DWORD,DWORD)` |

### Query (2)

| Ordinal | Name | Signature |
|---------|------|-----------|
| 21 | `?GetOptInState@Sqm@@YG?AW4OptInState@1@XZ` | `OptInState Sqm::GetOptInState(void)` |
| 23 | `?IsEnabled@Sqm@@YG_NXZ` | `BOOL Sqm::IsEnabled(void)` |

### Data Collection - Basic Setters (5)

| Ordinal | Name | Signature |
|---------|------|-----------|
| 31 | `?Set@Sqm@@YGXKK@Z` | `void Sqm::Set(DWORD id, DWORD value)` |
| 32 | `?Set@Sqm@@YGXKPB_W@Z` | `void Sqm::Set(DWORD id, const wchar_t* value)` |
| 37 | `?SetIfMin@Sqm@@YGXKK@Z` | `void Sqm::SetIfMin(DWORD id, DWORD value)` |
| 36 | `?SetIfMax@Sqm@@YGXKK@Z` | `void Sqm::SetIfMax(DWORD id, DWORD value)` |
| 22 | `?Increment@Sqm@@YGXKK@Z` | `void Sqm::Increment(DWORD id, DWORD increment)` |

### Data Collection - Streams (5)

| Ordinal | Name | Signature |
|---------|------|-----------|
| 5 | `?AddToStream@Sqm@@YGXKK@Z` | `void Sqm::AddToStream(DWORD id, DWORD value)` |
| 6 | `?AddToStream@Sqm@@YGXKKK@Z` | `void Sqm::AddToStream(DWORD id, DWORD a, DWORD b)` |
| 7 | `?AddToStream@Sqm@@YGXKKKK@Z` | `void Sqm::AddToStream(DWORD id, DWORD a, DWORD b, DWORD c)` |
| 8 | `?AddToStream@Sqm@@YGXKPBVTuple@1@@Z` | `void Sqm::AddToStream(DWORD id, const Tuple* tuple)` |
| 9 | `?AddToStream@Sqm@@YGXKPB_W@Z` | `void Sqm::AddToStream(DWORD id, const wchar_t* value)` |

### Data Collection - Averages (1)

| Ordinal | Name | Signature |
|---------|------|-----------|
| 4 | `?AddToAverage@Sqm@@YGXKK@Z` | `void Sqm::AddToAverage(DWORD id, DWORD value)` |

### Deferred Operations (8)

Deferred operations buffer metric computations and apply them at session end.

| Ordinal | Name | Signature |
|---------|------|-----------|
| 19 | `?DeferSetIfMin@Sqm@@YGXKK@Z` | `void Sqm::DeferSetIfMin(DWORD id, DWORD value)` |
| 18 | `?DeferSetIfMax@Sqm@@YGXKK@Z` | `void Sqm::DeferSetIfMax(DWORD id, DWORD value)` |
| 12 | `?DeferAddToAverage@Sqm@@YGXKK@Z` | `void Sqm::DeferAddToAverage(DWORD id, DWORD value)` |
| 13 | `?DeferAddToMedian@Sqm@@YGXKK@Z` | `void Sqm::DeferAddToMedian(DWORD id, DWORD value)` |
| 17 | `?DeferReportMin@Sqm@@YGXK@Z` | `void Sqm::DeferReportMin(DWORD id)` |
| 15 | `?DeferReportMax@Sqm@@YGXK@Z` | `void Sqm::DeferReportMax(DWORD id)` |
| 14 | `?DeferReportAverage@Sqm@@YGXK@Z` | `void Sqm::DeferReportAverage(DWORD id)` |
| 16 | `?DeferReportMedian@Sqm@@YGXK@Z` | `void Sqm::DeferReportMedian(DWORD id)` |

### Timers (2)

| Ordinal | Name | Signature |
|---------|------|-----------|
| 41 | `?StartTimer@Sqm@@YGXK@Z` | `void Sqm::StartTimer(DWORD id)` |
| 27 | `?PauseTimer@Sqm@@YGXK@Z` | `void Sqm::PauseTimer(DWORD id)` |

### Stream Timers (9)

Stream timers associate timed data with stream entries.

| Ordinal | Name | Signature |
|---------|------|-----------|
| 40 | `?StartStreamTimer@Sqm@@YGXKKK@Z` | `void Sqm::StartStreamTimer(DWORD a, DWORD b, DWORD c)` |
| 44 | `?StopStreamTimer@Sqm@@YGXKK@Z` | `void Sqm::StopStreamTimer(DWORD a, DWORD b)` |
| 1 | `?AbortStreamTimer@Sqm@@YGXKK@Z` | `void Sqm::AbortStreamTimer(DWORD a, DWORD b)` |
| 25 | `?IsStreamTimerActive@Sqm@@YG_NKK@Z` | `BOOL Sqm::IsStreamTimerActive(DWORD a, DWORD b)` |
| 26 | `?IsStreamTimerDataSet@Sqm@@YG_NKK@Z` | `BOOL Sqm::IsStreamTimerDataSet(DWORD a, DWORD b)` |
| 2 | `?AddStreamTimerData@Sqm@@YGXKKK@Z` | `void Sqm::AddStreamTimerData(DWORD a, DWORD b, DWORD c)` |
| 3 | `?AddStreamTimerData@Sqm@@YGXKKPB_W@Z` | `void Sqm::AddStreamTimerData(DWORD a, DWORD b, const wchar_t* c)` |
| 10 | `?AddToStreamTimer@Sqm@@YGXKKKPBVTuple@1@@Z` | `void Sqm::AddToStreamTimer(DWORD a, DWORD b, DWORD c, const Tuple* tuple)` |
| 11 | `?AddToStreamTimer@Sqm@@YGXKKPBVTuple@1@@Z` | `void Sqm::AddToStreamTimer(DWORD a, DWORD b, const Tuple* tuple)` |

### User Action Reporting (3)

| Ordinal | Name | Signature |
|---------|------|-----------|
| 30 | `?ReportUserExecutedAction@Sqm@@YGXKK@Z` | `void Sqm::ReportUserExecutedAction(DWORD a, DWORD b)` |
| 29 | `?ReportAppLaunchStatus@Sqm@@YGX_N@Z` | `void Sqm::ReportAppLaunchStatus(BOOL success)` |
| 28 | `?ReportAppCloseStatus@Sqm@@YGX_N@Z` | `void Sqm::ReportAppCloseStatus(BOOL success)` |

## Delay-Loaded Imports

### sqmapi.dll (27 functions) - Core SQM Engine

This is the **Windows SQM API** - the actual telemetry data collection engine. WLXPhotoSqm.dll is a **wrapper/facade** around this API.

| Function | Purpose |
|----------|---------|
| `SqmSetAppVersion` | Set application version string |
| `SqmSetAppId` | Set application identifier |
| `SqmSetMachineId` | Set machine UUID |
| `SqmGetMachineId` | Get machine UUID |
| `SqmSetUserId` | Set user identifier |
| `SqmGetUserId` | Get user identifier |
| `SqmCreateNewId` | Generate new session ID |
| `SqmReadSharedMachineId` | Read shared machine ID from registry |
| `SqmWriteSharedMachineId` | Write shared machine ID to registry |
| `SqmReadSharedUserId` | Read shared user ID from registry |
| `SqmWriteSharedUserId` | Write shared user ID to registry |
| `SqmSet` | Set counter value |
| `SqmSetString` | Set string counter value |
| `SqmSetIfMin` | Set if value is minimum |
| `SqmSetIfMax` | Set if value is maximum |
| `SqmIncrement` | Increment counter |
| `SqmAddToStreamDWord` | Add DWORD to stream |
| `SqmAddToStreamString` | Add string to stream |
| `SqmAddToAverage` | Add to running average |
| `SqmSetEnabled` | Enable/disable SQM |
| `SqmGetEnabled` | Query SQM enabled state |
| `SqmGetSession` | Get current session handle |
| `SqmStartSession` | Start new SQM session |
| `SqmEndSession` | End current SQM session |
| `SqmStartUpload` | Initiate data upload |
| `SqmWaitForUploadComplete` | Block until upload finishes |

### VERSION.dll (3 functions) - Version Info
- `GetFileVersionInfoW`, `GetFileVersionInfoSizeW`, `VerQueryValueW`

### UXCore.dll (8 ordinal imports)
- Ordinals 1-7, 15: UXCore helper functions (ordinals only, no named exports)

### WLXPhotoBase.dll (14 functions) - Base Library
- `Base::EnableShipAsserts`, `Base::DisableShipAsserts`
- `Base::GetReportsForWer` / `Base::SetReportsForWer` (WER crash reporting)
- `Base::GetReportsForSqm` (SQM-specific crash reporting)
- `Base::GetModuleVersion`, `Base::GetReportMetrics`
- `Base::Throw`, `Base::ThrowLastError`, `Base::BaseAtlThrow`
- `Base::GetBaseStringManager`, `BasePrivate::New`, `BasePrivate::Delete`

## Static Imports (Direct)

| DLL | Functions | Purpose |
|-----|-----------|---------|
| **WINHTTP.dll** | `WinHttpOpen`, `WinHttpConnect`, `WinHttpOpenRequest`, `WinHttpSendRequest`, `WinHttpReceiveResponse`, `WinHttpReadData`, `WinHttpQueryHeaders`, `WinHttpQueryDataAvailable`, `WinHttpSetOption`, `WinHttpSetStatusCallback`, `WinHttpGetProxyForUrl`, `WinHttpGetIEProxyConfigForCurrentUser`, `WinHttpCrackUrl`, `WinHttpCloseHandle` | HTTP client for SQM upload |
| **KERNEL32.dll** | 47 functions | Process/thread/memory/file/time management |
| **ADVAPI32.dll** | `RegOpenCurrentUser`, `RegCloseKey`, `RegOpenKeyExW`, `RegQueryValueExW`, `RegCreateKeyExW`, `RegSetValueExW`, `RegEnumKeyExW`, `RegDeleteKeyW`, `TraceMessage` | Registry + WMI tracing |
| **ole32.dll** | `CoInitialize`, `CoUninitialize`, `CoCreateInstance`, `CoInitializeSecurity`, `CoSetProxyBlanket`, `CoTaskMemAlloc/Free`, `StringFromCLSID` | COM for WMI queries |
| **SHELL32.dll** | `SHGetSpecialFolderPathW` | Known folder paths |
| **RPCRT4.dll** | `UuidCreate` | UUID generation |
| **SHLWAPI.dll** | `PathAppendW`, `PathFindFileNameW`, `PathIsDirectoryW`, `PathIsRelativeW`, `PathRemoveFileSpecW`, `PathRemoveExtensionW`, `StrCmpIW`, `StrRChrW`, `PathFindExtensionW`, `PathAddBackslashW` | Path manipulation |
| **USER32.dll** | `PeekMessageW`, `DispatchMessageW`, `TranslateMessage`, `MsgWaitForMultipleObjects`, `EnumDisplayDevicesW`, `EnumDisplaySettingsW`, `PostQuitMessage`, `CallMsgFilterW`, `MsgWaitForMultipleObjectsEx` | Message pump for thread sync |
| **MSVCR110.dll** | 30+ CRT functions | C runtime (malloc, strings, exceptions, threading) |

## RTTI (Run-Time Type Information) Classes

| Class | Namespace | Purpose |
|-------|-----------|---------|
| `Sqm` | (exported) | Main SQM telemetry facade - 44 exported methods |
| `DmxCommonDatapointsReporter` | `SqmPrivate` | Internal reporter for common DMX data points |
| `SqmShipAssert` | `SqmPrivate` | Ship assert handler for error reporting |
| `Exception` | `Base` | Exception base class |
| `Thread` | `Base` | Thread wrapper class |
| `ConfigurationCabDownload` | `ConfigurationUtil` | Downloads config cab files from live servers |
| `CabFileExtractor` | `ConfigurationUtilPrivate` | Extracts downloaded cab archives (RefCounted) |
| `RefCountBaseT<CabFileExtractor>` | `Base` | Reference-counted base for cab extractor |
| `CCabFileManager` | (global) | Manages cab file lifecycle |
| `TempFile` | `Base` | Temporary file management |
| `File` | `Base` | File I/O abstraction |
| `WinHttpRequest` | (global) | HTTP request wrapper around WINHTTP API |
| `SmartProxyInfo` | (global) | RAII wrapper for WINHTTP_PROXY_INFO |
| `SmartIEProxyConfig` | (global) | RAII wrapper for IE proxy config |
| `RefCountBaseMultiThreaded` | `Base` | Thread-safe reference counting base |

## Key Data Strings

### SQM Upload Infrastructure
```
http://sqm.microsoft.com/sqm/WindowsLive/sqmserver.dll   ← Primary SQM upload endpoint
http://g.live-int.com                                      ← Ship assert (internal/preview builds)
http://g.live.com                                         ← Ship assert (release builds)
%s/%s%s/%u                                                ← URL path format pattern
```

### SQM File System
```
SqmData%d_%s.sqm                                          ← Named SQM data files
SqmData*.sqm                                              ← SQM data file glob
SqmSessionData-                                           ← Session data prefix
SqmSessionData-NoOptIn-                                   ← Session data (no opt-in)
-%02d.sqm                                                  ← Session file suffix
```

### Registry Keys
```
Microsoft\Windows Live\SqmApi                              ← SQM API configuration
Software\Microsoft\Windows Live\Common                     ← Common WL settings
Software\Microsoft\Windows Live\Environment\PhotoGallery   ← Photo Gallery environment
Software\Microsoft\Windows Live\Photo Gallery\Library\DebugSQM     ← Debug SQM (Photo Gallery)
Software\Microsoft\Windows Live\Movie Maker\DebugSQM              ← Debug SQM (Movie Maker)
Software\Microsoft\Windows Live\Photo Gallery\Slideshow\DebugSQM  ← Debug SQM (Slideshow)
Software\Microsoft\Windows Live\Client Album Viewer\DebugSQM      ← Debug SQM (Album Viewer)
Software\Microsoft\Windows Live\Ship Asserts               ← Ship assert config
Software\Microsoft\Windows Live\Ship Asserts\Response Table ← Error response table
Software\Microsoft\Internet Explorer                       ← IE proxy settings
Software\Microsoft\IdentityCRL                             ← Identity/credential settings
```

### Application Identifiers
```
WLXPGSS.scr                  ← Photo Gallery Slideshow screensaver
MovieMaker.exe                ← Movie Maker
WindowsLivePhotoViewer.exe    ← Photo Viewer
```

### WMI Queries
```
ROOT\CIMV2
SELECT * FROM Win32_VideoController      → VideoProcessor, AdapterRAM
SELECT * FROM Win32_ComputerSystem       → TotalPhysicalMemory
SELECT * FROM Win32_Processor            → Revision, CurrentClockSpeed
```

### Ship Assert Configuration
```
ShipAssertSettings.xml                                              ← Local XML config
/WindowsLiveShipAssertConfig/ErrorResponseTable/Feature[@name="DmxShipAssert"]/Entry  ← XPath query
Parameters, AppName, AppVersion, AppTimeStamp, ModName, ModVersion, ModTimeStamp, Offset, HResultError  ← Error report fields
```

### Configuration Cab Download
```
WindowsLiveExperienceConfigurationCabDownload   ← Cab download key name
NextDownloadDateUTC                             ← Scheduling
DownloadInterval                                ← Interval between downloads
DisableDownload                                 ← Opt-out flag
NewParser                                       ← Parser version
```

### Machine Identification
```
D79251FD-B6BD-496c-A417-3F7FE486821C  ← App GUID
```

### Locale/Language Support (extensive list)
```
SuiteLanguage, InstalledLanguages, UserLanguage
sr-cyrl-cs, sr-latn-cs, ar-ploc-sa, ja-ploc-jp, az-latn-az, bs-latn-ba,
fil-ph, ha-latn-ng, iu-latn-ca, nso-za, quz-pe, uz-latn-uz, en-locr-us,
prs-af, ca-ES-valencia, chr-Cher, gd-Latn, ku-Arab, pa-Arab, qut-Latn,
sd-Arab, sr-Cyrl-BA, tg-Cyrl, ug-Arab, az-Latn-AZ, bs-Latn-BA, fil-PH,
ha-Latn-NG, iu-Latn-CA, kok-IN, nso-ZA, quz-PE, sr-cyrl-ba, uz-Latn-UZ
```

### Digital Signature
```
Microsoft Corporation - Signed: Tue Apr 1 01:11:25 2014
Microsoft Time-Stamp PCA
Microsoft Code Signing PCA
Microsoft Root Certificate Authority 2010 / 2011
MOPR1 (signing service)
nCipher DSE ESN:7D2E-3782-B0F7 / C0F4-3086-DEF8
```

## Architecture Summary

### Role in Windows Live Suite
WLXPhotoSqm.dll serves as the **SQM (Software Quality Metrics) telemetry facade** for Windows Live Photo Gallery and Movie Maker. It wraps the low-level `sqmapi.dll` API with a higher-level C++ class interface.

### SQM Data Flow
```
Application Code
    ↓
WLXPhotoSqm.dll (Sqm:: API)
    ↓                           ↓
sqmapi.dll                  WinHTTP.dll
    ↓                           ↓
SqmData*.sqm files    →    sqm.microsoft.com/sqm/WindowsLive/sqmserver.dll
    ↓
Registry: Microsoft\Windows Live\SqmApi
```

### Metric Collection Categories

1. **Application Lifecycle**: Launch status, close status, user actions
2. **Hardware Telemetry**: Video controller, CPU speed, RAM, processor info (via WMI)
3. **Feature Usage**: Stream counters, timers, averages, medians, min/max
4. **Deferred Aggregation**: Min/max/average/median computations deferred to session end
5. **Error Reporting**: Ship asserts with WER integration, XML-configured error responses
6. **Configuration**: Live cab downloads for dynamic config updates

### Consumer DLLs
- **MovieMakerCore.dll**: 13 imports (primary consumer)
- **Other WLX*.dll modules**: Various telemetry integration points

### Thread Safety
- Uses `InitializeCriticalSectionAndSpinCount` for thread synchronization
- `InterlockedIncrement`/`InterlockedDecrement` for atomic operations
- `InterlockedExchange` for atomic flag updates
- `_beginthreadex` for background thread creation
- `WaitForMultipleObjects` for thread synchronization

### Security Considerations
- Proxy-aware HTTP via `WinHttpGetProxyForUrl` + `WinHttpGetIEProxyConfigForCurrentUser`
- COM security via `CoInitializeSecurity` + `CoSetProxyBlanket`
- User opt-in preference via `SetOptInPreference` / `GetOptInState`
- Ship asserts gated by `EnableShipAsserts`/`DisableShipAsserts`
- Debug SQM modes per-application for development

## Test Harness

See `tests/OtherDlls/test_sqm.cpp` for the export probing test harness. The test:
- Resolves all 44 demangled export names via `GetProcAddress`
- Makes safe query-only live calls: `IsEnabled()`, `GetOptInState()`, `Startup()`, `Shutdown()`
- Validates function pointers and calling conventions
- Logs SEH exceptions for any failures

## Key Findings

1. **This is NOT the core SQM engine** - it is a Windows Live-specific facade wrapping `sqmapi.dll`
2. **Upload mechanism**: WinHTTP → `http://sqm.microsoft.com/sqm/WindowsLive/sqmserver.dll`
3. **Data persistence**: Local `.sqm` files in `%TEMP%` or per-user SQM directories
4. **Hardware fingerprinting**: Collects GPU, CPU, RAM info via WMI for device profiling
5. **Dynamic configuration**: Supports downloading config cabs from `live-int.com`/`live.com`
6. **Error telemetry**: Full ship assert infrastructure with XML-configured response tables
7. **User consent**: Respects opt-in/opt-out via registry, with per-app debug SQM overrides
8. **44 exports** span the complete telemetry lifecycle: init → collect → aggregate → upload → shutdown
