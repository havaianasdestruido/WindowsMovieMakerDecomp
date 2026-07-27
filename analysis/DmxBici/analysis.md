# DmxBici.dll - BICI Telemetry Module Analysis

## 1. Binary Overview

| Property | Value |
|----------|-------|
| **File** | DmxBici.dll |
| **Description** | Photo Gallery Bici (BICI telemetry wrapper) |
| **Type** | DLL (PE32, x86) |
| **Image Base** | 0x10000000 |
| **Entry Point** | 0x10001EE3 |
| **Subsystem** | Windows GUI (2) |
| **Linker Version** | 11.00 (MSVC 11.0 / VS2012) |
| **OS Version** | 6.02 (Windows 8) |
| **Timestamp** | 0x533A4039 (Tue Apr 1 01:27:37 2014) |
| **DLL Characteristics** | Dynamic base (ASLR), NX compatible (DEP) |
| **Stack Reserve/Commit** | 0x40000 / 0x1000 |
| **Heap Reserve/Commit** | 0x100000 / 0x1000 |
| **Checksum** | 0x17EB4 |
| **PDB GUID** | 217C185E-8E8D-49F1-B8B4-84D7FDD51CD8 |
| **PDB Name** | DmxBici.pdb |
| **Total Size of Image** | 0x9000 (36,864 bytes) |
| **File Version** | 16.4.3528.0331_ship.client.main.w5m4 (ship) |
| **Product Version** | 16.4.3528.0331 |
| **Copyright** | (c) 2012 Microsoft Corporation. All rights reserved. |
| **Company** | Microsoft Corporation |
| **Internal Name** | DmxBici |
| **Original Filename** | DmxBici.dll |
| **Product Name** | Photo Gallery |

## 2. Section Layout

| Section | Virtual Address | Virtual Size | Raw Size | Flags |
|---------|----------------|-------------|----------|-------|
| **.text** | 0x1000 | 0x4008 (16,392) | 0x4200 (16,896) | Code, Execute Read |
| **.data** | 0x6000 | 0x42C (1,068) | 0x200 (512) | Initialized Data, Read Write |
| **.rsrc** | 0x7000 | 0x3D0 (976) | 0x400 (1,024) | Initialized Data, Read Only |
| **.reloc** | 0x8000 | 0x7B4 (1,972) | 0x800 (2,048) | Initialized Data, Discardable, Read Only |

**Key observations:**
- Tiny DLL (~36 KB total) — pure thin wrapper around internal BICI engine.
- The `.text` section is ~16 KB — all 19 exported functions are very short thunks.
- The `.data` section is ~1 KB — contains vtable pointers, RTTI data, and the internal function name dispatch table.
- The `.rsrc` section is ~1 KB — version info only (no UI resources).

## 3. Export Table (19 exports — all from BiciWrapper)

All exports are **static methods** of the `BiciWrapper` class (mangled as `?FuncName@BiciWrapper@@YG...Z`). The `YG` calling convention = `__stdcall` with C linkage.

### Export Summary

| Ord | Hint | RVA | Exported Name | Demangled Signature | Return | Test Result |
|-----|------|-----|---------------|---------------------|--------|-------------|
| 1 | 0 | 0x1A48 | `?AddStringToDataPoint@BiciWrapper@@YG_NKKPB_W@Z` | `bool __stdcall AddStringToDataPoint(DWORD key, DWORD subKey, const wchar_t* value)` | BOOL | FALSE |
| 2 | 1 | 0x19A6 | `?AddToAverage@BiciWrapper@@YG_NKK@Z` | `bool __stdcall AddToAverage(DWORD key, DWORD value)` | BOOL | FALSE |
| 3 | 2 | 0x1A2C | `?AddToDataPoint@BiciWrapper@@YG_NKKK@Z` | `bool __stdcall AddToDataPoint(DWORD key, DWORD subKey, DWORD value)` | BOOL | FALSE |
| 4 | 3 | 0x1A01 | `?AddToStream@BiciWrapper@@YGXKPBVTuple@1@@Z` | `void __stdcall AddToStream(DWORD key, const BiciWrapper::Tuple* tuple)` | void | SEH* |
| 5 | 4 | 0x1865 | `?EndExperience@BiciWrapper@@YGJXZ` | `long __stdcall EndExperience()` | LONG | 1 |
| 6 | 5 | 0x198D | `?Increment@BiciWrapper@@YG_NKK@Z` | `bool __stdcall Increment(DWORD key, DWORD value)` | BOOL | TRUE |
| 7 | 6 | 0x1929 | `?Set@BiciWrapper@@YG_NKK@Z` | `bool __stdcall Set(DWORD key, DWORD value)` | BOOL | TRUE |
| 8 | 7 | 0x18F4 | `?SetAnid@BiciWrapper@@YGJPB_W@Z` | `long __stdcall SetAnid(const wchar_t* anid)` | LONG | 0x80004005 |
| 9 | 8 | 0x1974 | `?SetIfMax@BiciWrapper@@YG_NKK@Z` | `bool __stdcall SetIfMax(DWORD key, DWORD value)` | BOOL | TRUE |
| 10 | 9 | 0x195B | `?SetIfMin@BiciWrapper@@YG_NKK@Z` | `bool __stdcall SetIfMin(DWORD key, DWORD value)` | BOOL | TRUE |
| 11 | A | 0x1942 | `?SetString@BiciWrapper@@YG_NKPB_W@Z` | `bool __stdcall SetString(DWORD key, const wchar_t* value)` | BOOL | FALSE |
| 12 | B | 0x1848 | `?StartExperience@BiciWrapper@@YGJW4BiciStartupId@1@@Z` | `long __stdcall StartExperience(BiciStartupId id)` | LONG | 1 |
| 13 | C | 0x1837 | `?StartExperience@BiciWrapper@@YGJXZ` | `long __stdcall StartExperience()` [overload] | LONG | 0 |
| 14 | D | 0x19EB | `?TimerAccumulate@BiciWrapper@@YG_NK@Z` | `bool __stdcall TimerAccumulate(DWORD timerId)` | BOOL | FALSE |
| 15 | E | 0x19D5 | `?TimerRecord@BiciWrapper@@YG_NK@Z` | `bool __stdcall TimerRecord(DWORD timerId)` | BOOL | FALSE |
| 16 | F | 0x19BF | `?TimerStart@BiciWrapper@@YG_NK@Z` | `bool __stdcall TimerStart(DWORD timerId)` | BOOL | TRUE |
| 17 | 10 | 0x18A5 | `?TransferExperienceToApp@BiciWrapper@@YG_NPAPA_W@Z` | `bool __stdcall TransferExperienceToApp(wchar_t*** outNames)` | BOOL | TRUE |
| 18 | 11 | 0x18DC | `?TransferExperienceToAppId@BiciWrapper@@YG_NK@Z` | `bool __stdcall TransferExperienceToAppId(DWORD appId)` | BOOL | TRUE |
| 19 | 12 | 0x190C | `?TransferExperienceToWeb@BiciWrapper@@YG_NPB_WPAPA_W@Z` | `bool __stdcall TransferExperienceToWeb(const wchar_t* url, wchar_t*** outParams)` | BOOL | FALSE |

\* AddToStream threw SEH 0xE06D7363 (C++ exception) because the Tuple parameter was NULL.

### Export Categories

| Category | Exports | Description |
|----------|---------|-------------|
| **Experience Lifecycle** | `StartExperience` (x2), `EndExperience` | Begin/end a telemetry experience session |
| **Data Point Setters** | `Set`, `SetString`, `SetIfMax`, `SetIfMin`, `SetAnid` | Set data point values with various semantics |
| **Data Point Accumulators** | `Increment`, `AddToAverage`, `AddToDataPoint`, `AddStringToDataPoint` | Accumulate values into data points |
| **Stream** | `AddToStream` | Append a Tuple to a data stream |
| **Timer** | `TimerStart`, `TimerRecord`, `TimerAccumulate` | Timing measurement operations |
| **Transfer/Upload** | `TransferExperienceToApp`, `TransferExperienceToAppId`, `TransferExperienceToWeb` | Move experience data to target apps or web endpoints |

## 4. Import Table

| DLL | Functions | Purpose |
|-----|-----------|---------|
| **MSVCR110.dll** | 26 | C runtime: `memcpy_s`, `memmove_s`, `wcsnlen`, `wcsrchr`, `_wcsicmp`, `__CxxFrameHandler3`, `_amsg_exit`, `free`, `_malloc_crt`, `_initterm`, `_initterm_e`, `?terminate@@YAXXZ`, `??1type_info@@UAE@XZ`, `_lock`, `_unlock`, `_calloc_crt`, `__dllonexit`, `_onexit`, `__clean_type_info_names_internal`, `_except_handler4_common`, `_crt_debugger_hook`, `__crtUnhandledException`, `__crtTerminateProcess` |
| **KERNEL32.dll** | 22 | `InterlockedIncrement`, `GetLastError`, `EnterCriticalSection`, `LeaveCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `DeleteCriticalSection`, `FreeLibrary`, `GetProcAddress`, `LocalAlloc`, `LocalFree`, `LoadLibraryW`, `IsProcessorFeaturePresent`, `IsDebuggerPresent`, `DisableThreadLibraryCalls`, `GetTickCount64`, `GetSystemTimeAsFileTime`, `GetCurrentThreadId`, `QueryPerformanceCounter`, `DecodePointer`, `EncodePointer`, `InterlockedDecrement`, `RaiseException`, `GetModuleFileNameW` |
| **OLEAUT32.dll** | 1 | Ordinal 6 (`SysFreeString`) |
| **WLXPhotoBase.dll** | 6 | `Base::Exception::~Exception`, `Base::Throw`, `Base::String::GetBaseStringManager`, `ATL::BaseAtlThrow`, `BasePrivate::Delete`, `Base::ThrowLastError` |
| **WLXPhotoSqm.dll** | 1 | `Sqm::Set(DWORD, DWORD)` — Software Quality Metrics data point |
| **SHELL32.dll** | 1 | `SHGetKnownFolderPath` — known folder resolution |
| **ole32.dll** | 1 | `CoTaskMemFree` — COM task memory |
| **SHLWAPI.dll** | 1 | `PathFileExistsW` — file existence check |

### Key Import Analysis

- **WLXPhotoBase.dll** — Framework dependency. Provides ATL-based string management, exception handling, and memory management. This is the "base layer" DLL for all Photo Gallery components.
- **WLXPhotoSqm.dll** — SQM (Software Quality Metrics / Microsoft telemetry). The single `Sqm::Set` import indicates DmxBici bridges to the older SQM telemetry system for backward compatibility.
- **KERNEL32 — Dynamic loading**: `LoadLibraryW`/`GetProcAddress`/`FreeLibrary` indicate DmxBici dynamically loads additional DLLs at runtime (likely the actual BICI data collection engine `WLBici.dll`).
- **KERNEL32 — Timing**: `GetTickCount64`, `QueryPerformanceCounter`, `GetSystemTimeAsFileTime` — high-precision timing for timer exports.
- **SHELL32 — Path resolution**: `SHGetKnownFolderPath` resolves the `%LOCALAPPDATA%` or `%PROGRAMDATA%` path for BICI data files.
- **SHLWAPI — File check**: `PathFileExistsW` checks for BICI configuration/data files.
- **MSVCR110 — CRT**: VS2012 C runtime. Wide string operations (`wcsnlen`, `wcsrchr`, `_wcsicmp`) for processing metric names and URLs.

## 5. RTTI Classes

| Class | Source | Purpose |
|-------|--------|---------|
| `type_info` | C++ CRT | Standard MSVC RTTI type_info (vtable at 0x10006000) |
| `Base::Exception` | WLXPhotoBase.dll | WLXPhotoBase exception class (vtable at 0x10006020) |

No BICI-specific RTTI classes are exposed — `BiciWrapper` is entirely static with no vtable or RTTI. The actual BICI engine classes live inside a dynamically loaded DLL.

## 6. Internal Function Name Table

The `.data` section contains a **null-terminated string table** of internal BICI function names. These are not direct exports but rather the internal API that `BiciWrapper` methods dispatch to:

### Data Point Operations
| Internal Name | Corresponding Export | Purpose |
|---------------|---------------------|---------|
| `Set` | `Set` | Set a DWORD data point |
| `SetString` | `SetString` | Set a string data point |
| `SetIfMin` | `SetIfMin` | Set if value < current |
| `SetIfMax` | `SetIfMax` | Set if value > current |
| `Increment` | `Increment` | Increment a counter |
| `AddToAverage` | `AddToAverage` | Add value to running average |
| `AddToDataPoint` | `AddToDataPoint` | Add to composite data point |
| `AddStringToDataPoint` | `AddStringToDataPoint` | Add string to composite data point |
| `MergeFlowId` | *(internal)* | Merge flow identifiers between experiences |

### Experience Lifecycle
| Internal Name | Corresponding Export | Purpose |
|---------------|---------------------|---------|
| `StartExperience` | `StartExperience` | Begin experience session |
| `EndExperience` | `EndExperience` | End experience session |
| `TransferExperienceToWeb` | `TransferExperienceToWeb` | Upload experience data to web |
| `TransferExperienceToApp` | `TransferExperienceToApp` | Transfer data to another app |
| `TransferExperienceToAppId` | `TransferExperienceToAppId` | Transfer by app ID |

### Timer Operations
| Internal Name | Corresponding Export | Purpose |
|---------------|---------------------|---------|
| `TimerStart` | `TimerStart` | Begin timing a named interval |
| `TimerRecord` | `TimerRecord` | Record elapsed time |
| `TimerAccumulate` | `TimerAccumulate` | Accumulate time across calls |

### Configuration (NOT exported — internal only)
| Internal Name | Purpose |
|---------------|---------|
| `SetFlags` | Configure telemetry flags |
| `SetDataFilePath` | Set the path for local data storage |
| `SetUploadInterval` | Configure upload frequency |
| `SetDataExpiration` | Configure data retention period |
| `SetSamplingForExperience` | Set sampling rate per experience |
| `SetSamplingForDataPoint` | Set sampling rate per data point |
| `GetDataFilePattern` | Get the file naming pattern |
| `UploadData` | Trigger immediate data upload |

## 7. Embedded Strings

### File Path References
From the `.data` section:
- `\\?\UNC\...` — UNC path prefix for long path support
- `\\?\Volume{...}` — Volume path prefix
- `Windows Live\Shared\WLBici.dll` — The actual BICI engine DLL (loaded dynamically)

### Target Executables (from .data references)
- `WLXPGSS.scr` — Photo Gallery screensaver
- `MovieMaker.exe` — Windows Movie Maker main executable

### Signature URLs (from certificate/authenticode data)
- `http://crl.microsoft.com/pki/crl/products/MicrosoftTimeStampPCA.crl`
- `http://www.microsoft.com/pki/certs/MicrosoftTimeStampPCA.crt`
- `http://crl.microsoft.com/pki/crl/products/MicCodSigPCA_08-31-2010.crl`
- `http://www.microsoft.com/pki/certs/MicCodSigPCA_08-31-2010.crt`
- `http://crl.microsoft.com/pki/crl/products/microsoftrootcert.crl`
- `http://www.microsoft.com/pki/certs/MicrosoftRootCert.crt`
- `http://www.microsoft.com/pkiops/crl/MicCodSigPCA2011_2011-07-08.crl`
- `http://www.microsoft.com/pkiops/certs/MicCodSigPCA2011_2011-07-08.crt`
- `http://crl.microsoft.com/pki/crl/products/MicRooCerAut2011_2011_03_22.crl`
- `http://www.microsoft.com/pki/certs/MicRooCerAut2011_2011_03_22.crt`
- `http://www.microsoft.com/pkiops/docs/primarycps.htm`
- `http://crl.microsoft.com/pki/crl/products/MicRooCerAut_2010-06-23.crl`
- `http://www.microsoft.com/pki/certs/MicRooCerAut_2010-06-23.crt`
- `http://www.microsoft.com/PKI/docs/CPS/default.htm`
- `http://crl.microsoft.com/pki/crl/products/MicTimStaPCA_2010-07-01.crl`
- `http://www.microsoft.com/pki/certs/MicTimStaPCA_2010-07-01.crt`

All URLs are Microsoft PKI/certificate infrastructure — no BICI upload endpoints found. The actual telemetry upload endpoint is configured at runtime or in the `WLBici.dll` engine.

## 8. Architecture & Data Flow

```
MovieMakerCore.dll (consumer)
       |
       | 5 imports used
       v
DmxBici.dll (thin wrapper - THIS DLL)
       |
       | static BiciWrapper methods
       | dynamic LoadLibrary("WLBici.dll")
       v
WLBici.dll (actual BICI engine - not present)
       |
       | Local file storage
       | Periodic upload
       v
Microsoft BICI telemetry servers
```

### MovieMakerCore.dll Import Usage (5 functions)

Based on the task description, MovieMakerCore.dll imports 5 functions from DmxBici.dll. The most likely candidates based on the telemetry workflow:

1. `StartExperience` — Begin a user experience session
2. `EndExperience` — End the session and finalize data
3. `Set` — Record integer data points
4. `SetString` — Record string data points
5. `TimerStart`/`TimerRecord` — Performance timing

### Internal Dispatch Mechanism

DmxBici.dll uses `LoadLibraryW`/`GetProcAddress` to dynamically load `WLBici.dll` at runtime (from `Windows Live\Shared\`). The function name strings in `.data` are the names resolved from WLBici.dll's export table. Each `BiciWrapper` exported method:

1. Checks if the BICI engine is loaded
2. Looks up the corresponding function in WLBici.dll
3. Forwards the call with the provided parameters
4. Returns success/failure

This indirection allows DmxBici.dll to:
- Function as a stable API surface even if WLBici.dll changes
- Gracefully degrade if WLBici.dll is absent (return FALSE/0)
- Use `PathFileExistsW` to check for WLBici.dll before loading

## 9. Runtime Behavior (from test probing)

| Observation | Detail |
|-------------|--------|
| **Load base** | 0x71900000 (ASLR-relocated from default 0x10000000) |
| **Export count verified** | 19 functions |
| **Experience lifecycle** | `StartExperience()` returns 0 (S_OK); `StartExperience(0)` returns 1 (S_FALSE — enum 0 may be invalid); `EndExperience()` returns 1 |
| **Set operations** | `Set`, `SetIfMax`, `SetIfMin`, `Increment` all return TRUE — data point writes succeed |
| **Timer operations** | `TimerStart` returns TRUE; `TimerRecord`/`TimerAccumulate` return FALSE — timer not started or not initialized |
| **Transfer** | `TransferExperienceToApp` and `TransferExperienceToAppId` return TRUE; `TransferExperienceToWeb` returns FALSE |
| **SetAnid** | Returns E_FAIL (0x80004005) — likely requires prior experience initialization |
| **AddToStream** | Throws C++ exception (0xE06D7363) with NULL tuple — validates input |
| **String operations** | `AddStringToDataPoint` and `SetString` return FALSE — may require initialized experience |

## 10. BiciWrapper API Reference

### Types

```cpp
// Opaque data stream element
struct BiciWrapper::Tuple;  // Forward-declared, internal structure

// Startup identifier enum
enum BiciStartupId {
    // Enum values are internal to WLBici.dll
    // Enum ordinal 0 tested: returns S_FALSE (likely invalid/uninitialized)
};
```

### Function Signatures

```cpp
extern "C" {
    // Experience lifecycle
    long  __stdcall StartExperience();                           // Start with default ID
    long  __stdcall StartExperience(BiciStartupId id);           // Start with specific ID
    long  __stdcall EndExperience();                             // End current experience

    // Data point setters
    bool  __stdcall Set(DWORD key, DWORD value);                 // Set integer value
    bool  __stdcall SetString(DWORD key, const wchar_t* value);  // Set string value
    bool  __stdcall SetIfMax(DWORD key, DWORD value);            // Set if > current
    bool  __stdcall SetIfMin(DWORD key, DWORD value);            // Set if < current
    long  __stdcall SetAnid(const wchar_t* anid);                // Set anonymous ID

    // Data point accumulators
    bool  __stdcall Increment(DWORD key, DWORD value);           // Add value to counter
    bool  __stdcall AddToAverage(DWORD key, DWORD value);        // Add to running average
    bool  __stdcall AddToDataPoint(DWORD key, DWORD subKey, DWORD value);  // Add to composite
    bool  __stdcall AddStringToDataPoint(DWORD key, DWORD subKey, const wchar_t* value);

    // Stream operations
    void  __stdcall AddToStream(DWORD key, const BiciWrapper::Tuple* tuple);

    // Timer operations
    bool  __stdcall TimerStart(DWORD timerId);                   // Start timer
    bool  __stdcall TimerRecord(DWORD timerId);                  // Record elapsed time
    bool  __stdcall TimerAccumulate(DWORD timerId);              // Accumulate time

    // Transfer/Upload
    bool  __stdcall TransferExperienceToApp(wchar_t*** outNames);
    bool  __stdcall TransferExperienceToAppId(DWORD appId);
    bool  __stdcall TransferExperienceToWeb(const wchar_t* url, wchar_t*** outParams);
}
```

## 11. Relationship to WLXPhotoSqm.dll

DmxBici.dll imports `Sqm::Set(DWORD, DWORD)` from WLXPhotoSqm.dll. This indicates:

- DmxBici.dll bridges the older **SQM** (Software Quality Metrics) telemetry system with the newer **BICI** system
- Some data points may be dual-reported to both SQM and BICI
- The SQM `Set` call is likely made inside the WLBici.dll engine for backward compatibility

## 12. Summary

DmxBici.dll is a **minimal static-link wrapper** (36 KB, 19 exports) that provides a flat C API for the Windows Live BICI telemetry framework. It does not contain the actual telemetry engine — it dynamically loads `WLBici.dll` from `Windows Live\Shared\` at runtime and dispatches all calls through function pointers resolved by name from the internal string table in `.data`. The DLL is consumed primarily by MovieMakerCore.dll for experience lifecycle tracking, data point collection, timing, and transfer operations.
