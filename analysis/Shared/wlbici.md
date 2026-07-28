# wlbici.dll - Windows Live BICI Module Analysis

## Metadata
- **PDB**: `WLBici.pdb` (GUID: `{F1A3C900-C225-47BD-ADB1-75B0B4E45E81}`)
- **Timestamp**: Tue Apr 1 01:28:11 2014
- **Linker**: 11.00
- **OS Version**: 6.02 (Windows 8)
- **Subsystem**: Windows GUI
- **Base**: `0x10000000`, Image size: `0x2A000`
- **Machine**: x86 (0x14C)
- **DLL Characteristics**: Dynamic base, NX compatible
- **File Description**: Windows Live Client BICI Module

## Purpose
BICI (Behavioral Instrumentation Client Infrastructure) core module for Windows Live. Extends the SQM telemetry framework with "experience" lifecycle management, session transfer between apps, and enhanced data upload via both WinHTTP and Cabinet (FDI) compression. Manages telemetry persistence as `.sqm` binary files.

## Exports (30 functions)

### Experience Lifecycle
| Function | Description |
|---|---|
| `StartExperience` | Begin a named telemetry experience |
| `EndExperience` | End active experience, finalize data |
| `TransferExperienceToApp` | Transfer experience data to another app |
| `TransferExperienceToAppId` | Transfer experience data to app by ID |
| `TransferExperienceToWeb` | Transfer experience to web endpoint |

### Data Collection
| Function | Description |
|---|---|
| `Set` | Set data point value (generic) |
| `SetString` | Set string data point |
| `AddToDataPoint` | Add value to data point |
| `AddStringToDataPoint` | Add string to data point |
| `AddToAverage` | Add value to running average |
| `Increment` | Increment counter |
| `SetIfMax` | Set if value exceeds current max |
| `SetIfMin` | Set if value is below current min |
| `TimerStart` | Start named timer |
| `TimerAccumulate` | Accumulate timer duration |
| `TimerRecord` | Record timer value |
| `MergeFlowId` | Merge flow identifiers |

### Configuration
| Function | Description |
|---|---|
| `SetAnid` | Set ANID (Application Name ID) |
| `SetDataExpiration` | Set data expiration time |
| `SetDataFilePath` | Set data storage path |
| `SetFlags` | Set feature flags |
| `SetSamplingForDataPoint` | Set sampling rate for a data point |
| `SetSamplingForExperience` | Set sampling rate for experience |
| `SetUploadInterval` | Set upload frequency |

### Upload & Transport
| Function | Description |
|---|---|
| `UploadData` | Trigger data upload |
| `GetDataFilePattern` | Get file pattern for data files |

### COM Registration
| Function | Description |
|---|---|
| `DllCanUnloadNow` | COM unload check |
| `DllGetClassObject` | COM class factory |
| `DllRegisterServer` | COM self-registration |
| `DllUnregisterServer` | COM unregistration |

## Upload Endpoints
- **Primary (live.com)**: `http://ssw.live.com/uploaddata.aspx`
- **SQM manifest**: `https://sqm.microsoft.com/sqm/%s/manifests/Sqm%d.bin`
- **HTTP template**: `http%s://%s/%s`
- **Partner URL**: `%s/?Partner=%s`
- **Query string templates**: `%s?%s=%s&%s=%d` and `%s&%s=%s&%s=%d`
- **live-int** (internal staging): Referenced as environment marker

## Registry Keys
| Key | Purpose |
|---|---|
| `Software\Microsoft\SQMClient` | Main SQM client config |
| `Software\Microsoft\SQMClient\%s\AdaptiveSqm\ManifestInfo` | Adaptive manifest info |
| `Software\Microsoft\SQMClient\%s\AdaptiveSqm\Throttling` | Adaptive throttling |
| `Software\Microsoft\SQMClient\UploadUrlMap` | Upload URL mapping |
| `Software\Microsoft\SQMClient\Windows` | Windows SQM config |
| `Software\Microsoft\Windows Live\Common` | WL Common config |
| `Software\Microsoft\Windows Live\Common\Activations\%d` | Activation tracking |
| `Software\Policies\Microsoft\SQMClient` | Group Policy |
| `Software\Policies\Microsoft\SQMClient\Windows` | Group Policy (Windows) |
| `SOFTWARE\Microsoft\Reliability Analysis\RAC` | Reliability Analysis |

## File Paths
| Path | Purpose |
|---|---|
| `%LOCALAPPDATA%\Microsoft\Windows Live\Bici` | BICI data root |
| `%LOCALAPPDATA%\Local\SqmData_%s` | Per-user SQM data |
| `%LOCALAPPDATA%\Local\SqmLock_%s` | Per-user SQM lock |
| `%LOCALAPPDATA%\Local\WLBiciExperience%d` | Experience session files |
| `%LOCALAPPDATA%\Local\WLBiciSessionUpload` | Upload staging |
| `%LOCALAPPDATA%\Local\WLBiciStartupUpload` | Startup upload staging |
| `%LOCALAPPDATA%\Local\WLBiciTransfer%d` | Transfer session files |
| `%LOCALAPPDATA%\Local\WLBiciTransferMutex` | Transfer mutex |
| `%s\Sqm%d.bin` | Binary session files |
| `_%02d.sqm` / `_*.sqm` | SQM data files |
| `*.psqm` | Protected SQM files |
| `EmptyNoOpt.sqm` | Empty non-opt-in template |
| `EmptyOptIn.sqm` | Empty opt-in template |
| `*.cab.tmp` | Compressed upload archives |

## Metric/Config Strings
| String | Purpose |
|---|---|
| `AdaptiveSqmManifest` | Adaptive manifest identifier |
| `ASqmManifest` | SQM manifest type |
| `Bici%d` | BICI session naming |
| `BiciOptIn%d` | BICI opt-in state |
| `CEIPEnable` | CEIP enable flag |
| `CEIPSampledIn` | Sampling inclusion |
| `CEIPSamplingRangeHigh` / `CEIPSamplingRangeLow` | Sampling bounds |
| `CopyUploadedFilesToFolder` | Upload destination override |
| `CorporateSQMURL` | Corporate endpoint override |
| `EIDOverride` | External ID override |
| `Legal_policy_statement` | Policy statement reference |
| `MachineId` | Machine identifier |
| `MachineThrottling` | Throttling config |
| `Manufacturer` | OEM manufacturer |
| `OemDealId` | OEM deal identifier |
| `PCModel` | PC model identifier |
| `SuiteLanguage` | Language suite code |
| `WinOobeDate` / `WLOobeDate` | OOBE date tracking |
| `wlexpid` | Windows Live experience ID |
| `wlrefapp` | Reference application |

## Unique Features vs sqmapi.dll
1. **Cabinet.dll integration**: FDI-based .cab compression for upload archives (ordinals 10-14, 20, 22-23)
2. **InternetCrackUrlW**: URL parsing for custom endpoint handling
3. **Version.dll**: File version info extraction for telemetry
4. **COM/OLEAUT32**: Full COM registration (DllGetClassObject, DllRegisterServer)
5. **Experience model**: StartExperience/EndExperience lifecycle beyond SQM sessions
6. **Session transfer**: Cross-app telemetry data transfer mechanism

## Dependencies
- **Direct**: MSVCR110.dll, SHELL32.dll, SHLWAPI.dll, WININET.dll, WINHTTP.dll, Cabinet.dll, VERSION.dll, ntdll.dll, KERNEL32.dll, USER32.dll, ADVAPI32.dll, ole32.dll, OLEAUT32.dll
- **Key**: Full WinHTTP stack for upload, Cabinet FDI for compression, Shell APIs for known folder paths

## Security Descriptors
- `D:(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GR;;;WD)` - Standard
- `D:(A;OICI;GA;;;BA)(A;OICI;GA;;;SY)(A;OICI;GA;;;AU)` - Alternate
- `D:(A;OICI;GA;;;S-1-5-80-2970612574-78537857-698502321-558674196-1451644582)(A;OICI;GR;;;AU)` - SqmService
- `O:%sD:(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GA;;;%s)` - Dynamic SID
- `(A;OICI;GRGWSD;;;LS)` - Local Service access

## Key Internal Strings
- `MSQM` - SQM magic signature for data validation
- `CHttpRequest async operation` / `CHttpRequest Waiting for WinHttp to complete` - Async upload tracking
- `%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X` - GUID formatting
- `Upload Completion` / `Upload Completion or Terminate` / `Upload Thread Exit` - Thread sync events
- `*@%s[*].txt` - Log file pattern
- `CLSID\` / `\Implemented Categories` / `\Required Categories` - COM registration
