# sqmapi.dll - Windows SQM API Analysis

## Metadata
- **PDB**: `sqmapi.pdb` (GUID: `{DFD6718A-41F5-49AC-A5FC-DAD456158944}`)
- **Timestamp**: Mon Jul 13 22:11:00 2009
- **Linker**: 9.00
- **OS Version**: 6.01 (Windows 7)
- **Subsystem**: Windows GUI
- **Base**: `0x6C110000`, Image size: `0x33000`
- **Machine**: x86 (0x14C)
- **DLL Characteristics**: Dynamic base, NX compatible

## Purpose
Core SQM (Software Quality Metrics) client API. Implements Microsoft's CEIP (Customer Experience Improvement Program) telemetry infrastructure used across Windows Live and Windows components. Handles data collection, session management, upload scheduling, and manifest management.

## Exports (59 functions)

### Session Lifecycle
| Function | Description |
|---|---|
| `SqmStartSession` | Begin a telemetry collection session |
| `SqmEndSession` | End session, finalize data |
| `SqmFlushSession` | Flush session data to disk |
| `SqmGetSession` | Retrieve active session handle |
| `SqmGetSessionStartTime` | Query session start timestamp |

### Data Collection (Set/Get/Stream)
| Function | Description |
|---|---|
| `SqmSet` | Set a data point value (generic) |
| `SqmSetDWord64` | Set 64-bit DWORD data point |
| `SqmSetString` | Set string data point |
| `SqmSetBool` | Set boolean data point |
| `SqmSetBits` | Set bitmask data point |
| `SqmSetIfMax` | Set if value exceeds current max |
| `SqmSetIfMin` | Set if value is below current min |
| `SqmIncrement` | Increment counter |
| `SqmAddToAverage` | Add value to running average |
| `SqmAddToStream` | Add raw data to stream |
| `SqmAddToStreamDWord` | Add DWORD to stream |
| `SqmAddToStreamDWord64` | Add DWORD64 to stream |
| `SqmAddToStreamString` | Add string to stream |
| `SqmAddToStreamV` | Variadic stream add |
| `SqmGetFlags` | Get session flags |
| `SqmSetFlags` | Set session flags |
| `SqmClearFlags` | Clear session flags |

### Timer Functions
| Function | Description |
|---|---|
| `SqmTimerStart` | Start a named timer |
| `SqmTimerRecord` | Record timer value |
| `SqmTimerAccumulate` | Accumulate timer duration |
| `SqmTimerAddToAverage` | Add timer to average calculation |

### Identity & Configuration
| Function | Description |
|---|---|
| `SqmSetAppId` | Set application ID |
| `SqmSetAppVersion` | Set application version |
| `SqmGetMachineId` | Retrieve machine GUID |
| `SqmSetMachineId` | Set machine GUID |
| `SqmGetUserId` | Retrieve user ID |
| `SqmSetUserId` | Set user ID |
| `SqmCreateNewId` | Generate new telemetry ID |
| `SqmGetEnabled` | Check if SQM is enabled |
| `SqmSetEnabled` | Enable/disable SQM |
| `SqmIsWindowsOptedIn` | Check Windows CEIP opt-in |

### Escalation (Advanced Telemetry)
| Function | Description |
|---|---|
| `SqmLoadEscalationManifest` | Load escalation rules manifest |
| `SqmUnloadEscalationManifest` | Unload manifest |
| `SqmGetEscalationRuleStatus` | Check escalation rule status |
| `SqmSetEscalationInfo` | Set escalation metadata |
| `SqmCheckEscalationAddToStreamDWord` | Escalation-conditional DWORD add |
| `SqmCheckEscalationAddToStreamDWord64` | Escalation-conditional DWORD64 add |
| `SqmCheckEscalationAddToStreamString` | Escalation-conditional string add |
| `SqmCheckEscalationSetDWord` | Escalation-conditional DWORD set |
| `SqmCheckEscalationSetDWord64` | Escalation-conditional DWORD64 set |
| `SqmCheckEscalationSetString` | Escalation-conditional string set |

### Upload & Sync
| Function | Description |
|---|---|
| `SqmStartUpload` | Initiate data upload |
| `SqmWaitForUploadComplete` | Block until upload finishes |
| `SqmGetInstrumentationProperty` | Get instrumentation property |

### Shared IDs
| Function | Description |
|---|---|
| `SqmReadSharedMachineId` | Read machine ID from shared memory |
| `SqmWriteSharedMachineId` | Write machine ID to shared memory |
| `SqmReadSharedUserId` | Read user ID from shared memory |
| `SqmWriteSharedUserId` | Write user ID to shared memory |

### Sysprep & Maintenance
| Function | Description |
|---|---|
| `SqmSysprepCleanup` | Cleanup during sysprep generalize |
| `SqmSysprepGeneralize` | Generalize for sysprep |
| `SqmSysprepSpecialize` | Specialize after sysprep |
| `SqmUnattendedSetup` | Handle unattended setup |
| `SqmCleanup` | General cleanup |

## Upload Endpoints
- **Primary**: `https://sqm.microsoft.com/sqm/%s/manifests/Sqm%d.bin`
- **HTTP variant**: `http%s://%s/%s` (template)
- **Partner URL**: `%s/?Partner=%s`

## Registry Keys
| Key | Purpose |
|---|---|
| `Software\Microsoft\SQMClient` | Main SQM client config |
| `Software\Microsoft\SQMClient\%s\AdaptiveSqm\ManifestInfo` | Adaptive manifest info |
| `Software\Microsoft\SQMClient\%s\AdaptiveSqm\Throttling` | Adaptive throttling |
| `Software\Microsoft\SQMClient\UploadUrlMap` | Upload URL mapping |
| `Software\Microsoft\SQMClient\Windows` | Windows SQM config |
| `Software\Microsoft\SQMClient\Windows\AdaptiveSQM\ManifestInfo` | Adaptive SQM manifest |
| `Software\Microsoft\SQMClient\Windows\AdaptiveSQM\Throttling` | Adaptive SQM throttling |
| `Software\Microsoft\SQMClient\Windows\CommonDatapoints` | Common datapoints |
| `Software\Microsoft\SQMClient\Windows\DisabledProcesses` | Disabled processes |
| `Software\Microsoft\SQMClient\Windows\DisabledSessions` | Disabled sessions |
| `Software\Microsoft\SQMClient\Windows\ServerSync` | Server sync state |
| `Software\Microsoft\SQMClient\Windows\Uploader\PendingUpload` | Pending uploads |
| `Software\Microsoft\SQMClient\Windows\Users` | Per-user data |
| `Software\Microsoft\Windows NT\CurrentVersion\UnattendSettings\SQMClient` | Unattend |
| `Software\Policies\Microsoft\SQMClient` | Group Policy |
| `Software\Policies\Microsoft\SQMClient\Windows` | Group Policy (Windows) |
| `SOFTWARE\Microsoft\Reliability Analysis\RAC` | Reliability Analysis |

## File Paths
| Path | Purpose |
|---|---|
| `%LOCALAPPDATA%\Microsoft\Windows\Sqm\Manifest` | Local manifest storage |
| `%LOCALAPPDATA%\Microsoft\Windows\Sqm\Sessions` | Session data files |
| `%LOCALAPPDATA%\Microsoft\Windows\Sqm\Upload` | Upload staging |
| `%LOCALAPPDATA%\Local\SqmData_%s` | Per-user SQM data |
| `%LOCALAPPDATA%\Local\SqmLock_%s` | Per-user SQM lock file |
| `%ProgramData%\Microsoft\Windows\SoftwareQualityMetricsClient` | Shared SQM data |
| `%ProgramData%\LogFiles\SQM` | SQM log files |
| `%s\Sqm%d.bin` | Binary session files |
| `*.psqm` | Protected SQM files |
| `.cab.tmp` | Temporary cab for upload |

## Metric/Config Strings
| String | Purpose |
|---|---|
| `AdaptiveSqmManifest` | Adaptive manifest identifier |
| `ASqmManifest` | SQM manifest type |
| `CEIPEnable` | CEIP enable flag |
| `CEIPEnabled` | CEIP enabled state |
| `CEIPSampledIn` | Sampling inclusion flag |
| `CEIPSamplingRangeHigh` | Sampling range upper bound |
| `CEIPSamplingRangeLow` | Sampling range lower bound |
| `CopyUploadedFilesToFolder` | Upload destination override |
| `CorporateSQMURL` | Corporate SQM endpoint override |
| `DisabledSessions` | Disabled sessions list |
| `DisableManifestDownload` | Disable manifest download flag |
| `DisableOptinExperience` | Disable opt-in UI |
| `IsTest` | Test mode flag |
| `MachineId` | Machine identifier |
| `MachineThrottling` | Throttling configuration |
| `MaxUploadFileSize` | Max upload size limit |
| `Manifest` / `ManifestVersion` | Manifest data/version |
| `MSFTInternal` | Internal Microsoft flag |
| `RacSampleNumber` | RAC sample number |
| `Sampling` / `SamplingInterval` | Sampling configuration |
| `ServerSync` / `ServerSyncToken` | Server sync state |
| `StudyId` | Study identifier |
| `TestManifestMode` | Test manifest mode |
| `TestManifestUrl` | Test manifest URL |
| `ThrottlingExceptionSessionList` | Throttle exceptions |
| `ThrottlingInterval` | Throttle interval |
| `UploadDisableFlag` | Upload disable flag |
| `Upload Completion` | Upload completion event |

## Dependencies
- **Direct**: msvcrt.dll, ADVAPI32.dll, USER32.dll, KERNEL32.dll, ntdll.dll
- **Delay-loaded**: ole32.dll (CoCreateGuid, StringFromGUID2, CoInitialize/Uninitialize), OLEAUT32.dll (Sys* allocators), WINHTTP.dll (full WinHTTP stack)

## Security Descriptors
- `D:(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GR;;;WD)` - Standard: SYSTEM+BA full, Everyone read
- `D:(A;OICI;GA;;;S-1-5-80-2970612574-78537857-698502321-558674196-1451644582)(A;OICI;GR;;;AU)` - NT SERVICE\SqmService
- `O:%sD:(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)` - Owner-referenced
- `O:%sD:(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GA;;;%s)` - Dynamic owner SID
- `(A;OICI;GRGWSD;;;LS)` - Local Service access

## Key Internal Strings
- `MSQM` / `SQMA` / `SQMC` - Magic signatures for data validation
- `CHttpRequest async operation` / `CHttpRequest Waiting for WinHttp to complete` - Async upload tracking
- `SqmLogger*.etl.*` - ETL log file pattern
- `Upload Completion or Terminate` / `Upload Thread Exit` - Thread synchronization events
