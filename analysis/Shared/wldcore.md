# wldcore.dll - Windows Live Client Core Analysis

## Metadata
- **PDB**: `WLDCore.pdb` (GUID: `{A9E26409-D395-4011-BDF8-BD32342B6BE2}`)
- **Timestamp**: Tue Apr 1 01:27:15 2014
- **Linker**: 11.00
- **OS Version**: 6.02 (Windows 8)
- **Subsystem**: Windows GUI
- **Base**: `0x10000000`, Image size: `0x13000`
- **Machine**: x86 (0x14C)
- **DLL Characteristics**: Dynamic base, NX compatible
- **File Description**: Windows Live Client Shared Platform Module

## Purpose
Core shared platform module for Windows Live suite. Provides SQM session management infrastructure (`CSqmSessionManager`), time utilities, window management primitives, and the `DCCreateObject` factory for Windows Live internal component creation. Acts as the shared runtime for all WL desktop applications.

## Exports (272 total - 2 named, 270 ordinal-only)

### Named Exports
| Function | Ordinal | Description |
|---|---|---|
| `DCCreateObject` | 3 | Windows Live internal object factory |
| `?AddMinutesToSystemTime@@YGJABU_SYSTEMTIME@@PAU1@K@Z` | 117 | Add minutes to SYSTEMTIME (C++ mangled) |

### Ordinal Exports (Key Ranges)
- **Ordinals 1-116**: Core infrastructure functions (session management, data operations, configuration)
- **Ordinals 200-272**: Extended functions (possibly v2 API additions)

The vast majority of exports are ordinal-only (no name), indicating an internal/undocumented API consumed only by sibling Windows Live DLLs.

## Known Internal Classes (from strings)

### CSqmSessionManager
Core SQM session manager class with full telemetry operations:
```
CSqmSessionManager::AddDataPointToAverage(%lu) - DP=%lu, Val=%lu
CSqmSessionManager::AddDWORDsToStream(%lu) - DP=%lu, cDWORDs=%lu
CSqmSessionManager::AddStringToStream(%lu) - DP=%lu, Val=%ls
CSqmSessionManager::EndSession(%lu) - File = %ls
CSqmSessionManager::IncrementDataPoint(%lu) - DP=%lu, Val=%lu
CSqmSessionManager::SetAppId(%lu) - Id=%lu
CSqmSessionManager::SetDataPoint(%lu) - DP=%lu, Val=%lu
CSqmSessionManager::SetDataPointIfMax(%lu) - DP=%lu, Val=%lu
CSqmSessionManager::SetDataPointIfMin(%lu) - DP=%lu, Val=%lu
CSqmSessionManager::SetDataPointString(%lu) - DP=%lu, Val=%ls
CSqmSessionManager::SetDataPointStringContent(%lu) - DP=%lu, Val=%ls
```

### CHttpRequest
HTTP upload request handler:
```
CHttpRequest async operation
CHttpRequest Waiting for WinHttp to complete
```

## File Paths
| Path | Purpose |
|---|---|
| `Windows Live\Shared\sqmapi.dll` | SQM API dependency path |
| `WLDCore.dll` | Self-reference |
| `sqmdata%02d.sqm` | SQM data files (numbered) |
| `sqmdata*.sqm` | SQM data file glob |
| `sqmnoopt%02d.sqm` | Non-opt-in SQM files |
| `sqmnoopt*.sqm` | Non-opt-in SQM file glob |

## Registry Keys
| Key | Purpose |
|---|---|
| `Software\Microsoft\Internet Explorer` | IE config (proxy detection) |
| `Software\Microsoft\Windows Live\Common` | WL Common settings |

## Configuration Strings
| String | Purpose |
|---|---|
| `SQM_NOOPT` | Non-opt-in SQM mode |
| `Version` | Version information |
| `%04d-%02d-%02dT%02d:%02d:%02d%lsZ` | ISO 8601 timestamp format |
| `%2.2u%2.2u%2.2u%2.2u` | Time formatting |
| `%s.%lu` | Naming template |

## Key Operational Strings (Debug/Logging)
| String | Purpose |
|---|---|
| `%hs > (%lu) - dwFullBinaryVersionMS=%lu, dwFullBinaryVersionLS=%lu` | Version info logging |
| `%hs > Failed to load SQM module (hr=0x%x).` | Module load error |
| `%hs > Failure (hr=0x%x)` | Generic failure |
| `%hs > hSession=%lu` | Session handle logging |
| `%hs > Loading SQM module from %ls` | Module loading |
| `%hs > Type %d -> Id %d` | Data type mapping |
| `%hs > Unable to get procedure address.` | GetProcAddress failure |
| `%hs > Unable to upload data because of initialization failure (hr=0x%x).` | Upload init failure |
| `%hs > You have called start session twice without calling EndSession -- ignoring this call to StartSession.` | Double-start guard |
| `y%hs > Setting SQM directory to %ls` | Directory configuration |
| `ATL:%p` | ATL allocation tracking |

## Dependencies
- **Direct**: MSVCR110.dll, KERNEL32.dll, SHELL32.dll (SHGetKnownFolderPath), USER32.dll (window management), ADVAPI32.dll (registry), ole32.dll (CoTaskMemFree)
- **Delay-loaded**: OLEAUT32.dll, WSOCK32.dll

## Notable Design Characteristics

### Heavy Use of Ordinals
272 total exports with only 2 named - this is a pure internal API. All sibling WL DLLs (wlcomm, wlmail, wlcsync, etc.) resolve these by ordinal number, making the API opaque to external consumers.

### Window Management
Imports `CreateWindowExW`, `RegisterClassExW`, `DefWindowProcW`, `PostMessageW`, `SetTimer`/`KillTimer`, `SetWindowLongW`/`GetWindowLongW` - indicating it manages hidden windows for message-based async operations (COM-like patterns).

### Time Utilities
Imports `TzSpecificLocalTimeToSystemTime`, `GetLocalTime`, `GetSystemTime` - dedicated timezone handling for timestamp generation.

### SLList-based Memory
Uses `InterlockedPushEntrySList`/`InterlockedPopEntrySList`/`InitializeSListHead` - lock-free singly-linked list for high-performance internal data structures.

### Resource Loading
Imports `FindResourceW`, `FindResourceExW`, `LoadResource`, `LockResource`, `SizeofResource` - loads embedded resources (likely version info, manifests).

## Security
No direct security descriptor strings found - this module defers to sqmapi.dll for file/registry ACL management.

## Comparison with sqmapi.dll
| Feature | sqmapi.dll | wldcore.dll |
|---|---|---|
| Export style | Named (59) | Ordinal-only (270/272) |
| Role | Telemetry API | Platform runtime |
| HTTP | WinHTTP (delay-loaded) | N/A (delegates to sqmapi) |
| Window mgmt | Minimal | Full (hidden windows) |
| COM factory | No | Yes (DCCreateObject) |
| C++ RTTI | Minimal | Present (class hierarchies) |
