# wldlog.dll - Windows Live Desktop Logging Analysis

## Metadata
- **PDB**: `wldlog.pdb` (GUID: `{E41DD69F-516C-4033-8713-5AD830814704}`)
- **Timestamp**: Tue Apr 1 01:28:22 2014
- **Linker**: 11.00
- **OS Version**: 6.02 (Windows 8)
- **Subsystem**: Windows GUI
- **Base**: `0x10000000`, Image size: `0xC000`
- **Machine**: x86 (0x14C)
- **DLL Characteristics**: Dynamic base, NX compatible
- **File Description**: Windows Live Desktop Logging

## Purpose
Lightweight logging infrastructure for the Windows Live desktop suite. Provides zone-based logging with configurable verbosity levels, retail/development logging modes, file-based log output, parallel logging support, and memory log buffers. Uses ETW (Event Tracing for Windows) trace providers and custom file sinks.

## Exports (23 functions)

### Initialization / Lifecycle
| Function | Description |
|---|---|
| `InitializeLogging` | Initialize logging subsystem |
| `UninitializeLogging` | Shutdown logging subsystem |
| `InitializeModule` | Initialize a logging module |
| `IsModuleInitialized` | Check module init state |
| `IsRetailLoggingEnabled` | Check retail logging mode |

### Logging Control
| Function | Description |
|---|---|
| `LogOutput` | Primary log output function |
| `EnableParallelLog` | Enable parallel (multi-threaded) logging |
| `DisableParallelLog` | Disable parallel logging |
| `SetZoneLevel` | Set verbosity level for a zone |
| `GetZoneLevel` | Get current zone verbosity level |
| `SetZoneLoggingEnabled` | Enable/disable logging for a zone |
| `SetZoneLoggingEnabled_Temporary` | Temporary zone logging override |
| `ZoneLoggingEnabled` | Check if zone logging is active |
| `ZoneFileLoggingEnabled` | Check if file logging is active for zone |

### Retail Logging
| Function | Description |
|---|---|
| `SetRetailLogging` | Configure retail logging mode |
| `RetailLoggingFlush` | Flush retail log buffers to disk |
| `RetailLoggingDirtyFlushLogs` | Force flush dirty log pages |
| `RetailLoggingGetMemoryLog` | Retrieve in-memory log buffer |
| `RetailLoggingGetPath` | Get retail log file path |

### COM Registration
| Function | Description |
|---|---|
| `DllCanUnloadNow` | COM unload check |
| `DllGetClassObject` | COM class factory |
| `DllRegisterServer` / `DllUnregisterServer` | COM registration (same RVA - shared stub) |

## Logging Architecture

### Zone-Based System
- `SetZoneLevel` / `GetZoneLevel` - Per-zone verbosity control
- `SetZoneLoggingEnabled` / `ZoneLoggingEnabled` - Per-zone enable/disable
- `ZoneFileLoggingEnabled` - Per-zone file output control
- `SetZoneLoggingEnabled_Temporary` - Transient overrides (debug sessions)

### Logging Modes
1. **Retail Logging**: Production-safe, filtered logging to files
2. **Parallel Logging**: Thread-safe multi-producer logging (lock-free SLList based)
3. **Memory Logging**: In-ring-buffer logging for post-mortem analysis
4. **File Logging**: Persistent file-based log output

### Log Format Strings
```
* Log opened: %1
* Log closed (%s) at %s
%04d-%02d-%02dT%02d:%02d:%02dZ  (ISO 8601 UTC timestamps)
```

## File Paths
| Path | Purpose |
|---|---|
| `LogFileName` (registry/config) | Log file destination |

## Registry Keys
| Key | Purpose |
|---|---|
| `Software\Microsoft\SQMClient` (inherited) | SQM integration |
| `USERDNSDOMAIN` (env var) | Domain info for log tagging |

## Configuration Strings
| String | Purpose |
|---|---|
| `corp.microsoft.com` | Internal domain reference |
| `LogFileName` | Log file name config key |
| `Legal_policy_statement` | Policy statement |
| `Windows Live Desktop Logging` | Product identifier |

## Dependencies
- **Direct**: MSVCR110.dll, KERNEL32.dll, ole32.dll (CoCreateInstance), ADVAPI32.dll (registry)
- **Notable**: No WINHTTP, no Cabinet - this is a pure logging module with no upload capability
- Uses `QueueUserWorkItem` for async flush operations
- Uses `DebugBreak` for assertion/debug support

## Key Implementation Details

### Thread Safety
- `InterlockedIncrement`/`InterlockedDecrement` - Reference counting
- `InterlockedExchange` - Atomic state updates
- `InterlockedPushEntrySList`/`InterlockedPopEntrySList`/`InterlockedFlushSList` - Lock-free log queue
- `InitializeSListHead`/`QueryDepthSList` - SLList management
- `InitializeCriticalSectionAndSpinCount` - Critical sections for non-SLList paths
- `CreateEventW`/`SetEvent`/`ResetEvent`/`WaitForSingleObject` - Event signaling for flush synchronization

### File I/O
- `CreateFileW`/`ReadFile`/`WriteFile`/`SetEndOfFile`/`SetFilePointer` - Direct file operations (not buffered I/O)
- `FindResourceW`/`FindResourceExW`/`LoadResource`/`LockResource` - Embedded resource loading

### Error Handling
- `FormatMessageA` - System error message formatting
- `GetLastError`/`SetLastError` - Windows error state
- `RaiseException` - Exception-based error signaling
- `DebugBreak` - Debugger breakpoint for debug builds

### String Handling
- `memcpy_s`/`memmove_s`/`wmemcpy_s` - Safe memory operations
- `_vscwprintf`/`_vsnwprintf` - Wide string formatting
- `_vscprintf`/`_vsnprintf` - Narrow string formatting
- `vsprintf_s` - Safe formatted output
- `_stricmp`/`wcsstr` - String comparison/search
- `_wgetenv_s`/`_wcslwr_s` - Environment string handling

## Comparison with Other Shared DLLs
| Feature | sqmapi | wlbici | wldcore | wldlog |
|---|---|---|---|---|
| Exports | 59 named | 30 named | 2 named + 270 ord | 23 named |
| Upload | WinHTTP | WinHTTP + Cabinet | N/A | None |
| COM | No | Yes | Yes | Yes (stub) |
| Registry | Heavy | Heavy | Moderate | Minimal |
| Logging | Debug strings | Debug strings | Debug strings | Primary purpose |
| Size (.text) | 0x2B400 | 0x22C00 | 0xE200 | 0x6200 |

## Shared Pattern: Microsoft Code Signing Certificates
All four DLLs contain embedded certificate chain references:
- `Microsoft Root Certificate Authority`
- `Microsoft Root Certificate Authority 2010`
- `Microsoft Root Certificate Authority 2011`
- `Microsoft Code Signing PCA` / `Microsoft Code Signing PCA 2011`
- `Microsoft Time-Stamp PCA` / `Microsoft Time-Stamp PCA 2010`
- CRL URLs: `http://crl.microsoft.com/pki/crl/products/...`
- CPS: `http://www.microsoft.com/pkiops/docs/primarycps.htm`

This confirms all four DLLs are Authenticode-signed Microsoft binaries.
