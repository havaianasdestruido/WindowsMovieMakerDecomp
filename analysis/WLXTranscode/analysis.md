# WLXTranscode.exe — Static Analysis

## Overview

**WLXTranscode.exe** is the Media Foundation-based transcoding engine for Windows Live Movie Maker 2012 (Windows Live Essentials 16.4.3528.0331). It is a PE32 console (CUI) executable that orchestrates hardware-accelerated and software video/audio transcoding via the Media Foundation pipeline. Despite the `.exe` extension, the VERSIONINFO identifies it as `WLXTranscoder.dll` — it was likely built as a DLL but deployed as a standalone tool.

---

## PE Structure

| Field | Value |
|---|---|
| **File** | `WLXTranscode.exe` |
| **Arch** | x86 (14C) |
| **Subsystem** | Windows CUI (3) |
| **Linker** | Visual Studio 2012 (11.00) |
| **Entry Point** | `0x0043F206` |
| **Image Base** | `0x00400000` |
| **Image Size** | `0x4C000` (311,296 bytes) |
| **OS Target** | 6.02 (Windows 8) |
| **Image Version** | 6.02 |
| **Subsystem Version** | 6.00 |
| **Timestamp** | Tue Apr 1 01:26:35 2014 |
| **Checksum** | `0x56540` |
| **DLL Characteristics** | `0x8140` — Dynamic Base, NX Compatible, Terminal Server Aware |
| **Debug PDB** | `WLXTranscode.pdb` `{3C6BCAEB-AF8F-4C79-9830-EB27BFFDD447}` |

### Sections

| Section | Virtual Size | Virtual Address | Raw Size | Flags |
|---------|-------------|----------------|----------|-------|
| `.text` | `0x43BF8` | `0x00401000` | `0x43C00` | Code, Execute Read |
| `.data` | `0x140C` | `0x00445000` | `0x1000` | Initialized Data, Read Write |
| `.rsrc` | `0x6F0` | `0x00447000` | `0x800` | Initialized Data, Read Only |
| `.reloc` | `0x37B4` | `0x00448000` | `0x3800` | Initialized Data, Discardable, Read Only |

### Directories

| Directory | RVA | Size |
|-----------|-----|------|
| Export | `0` | `0` |
| Import | `0x439B8` | `0x140` |
| Resource | `0x47000` | `0x6F0` |
| Base Reloc | `0x48000` | `0x2C64` |
| Debug | `0x1380` | `0x38` |
| Load Config | `0x89D0` | `0x40` |
| IAT | `0x1000` | `0x30C` |

---

## Export Table

**No exports.** The export directory is empty (RVA = 0, size = 0). Unlike the other WLX DLLs, this binary does not expose any DLL exports — it is invoked as a standalone process.

---

## Import Table

### ADVAPI32.dll — ETW Tracing & Cryptography
| Function | Description |
|----------|-------------|
| `RegisterTraceGuidsW` | ETW provider registration |
| `UnregisterTraceGuids` | ETW provider unregistration |
| `TraceMessage` | ETW trace message |
| `GetTraceLoggerHandle` | ETW logger handle |
| `GetTraceEnableLevel` | ETW enable level |
| `GetTraceEnableFlags` | ETW enable flags |
| `TraceEvent` | ETW event tracing |
| `CryptAcquireContextW` | Crypto API context |
| `CryptReleaseContext` | Release crypto context |
| `CryptCreateHash` | Hash creation |
| `CryptDestroyHash` | Destroy hash object |
| `CryptHashData` | Hash data |
| `CryptSignHashW` | Sign hash (digital signatures) |
| `CryptImportKey` | Import crypto key |
| `CryptDestroyKey` | Destroy key |
| `RegQueryValueExW` | Registry query |
| `RegOpenKeyExW` | Registry open |
| `RegCloseKey` | Registry close |

### KERNEL32.dll — Core OS Services
Synchronization: `CreateSemaphoreW`, `ReleaseSemaphore`, `CreateEventW`, `SetEvent`, `ResetEvent`, `WaitForMultipleObjects`, `WaitForSingleObject`, `Sleep`

Memory: `LocalFree`

PE Loading: `LoadLibraryExW`, `FreeLibrary`, `GetModuleHandleA`, `GetModuleHandleW`, `GetModuleFileNameW`, `GetProcAddress`

String/Encoding: `WideCharToMultiByte`, `MultiByteToWideChar`, `lstrlenW`

Error Handling: `FormatMessageW`, `RaiseException`, `GetLastError`

File I/O: `DeleteFileW`, `GetFileAttributesW`, `GetFileAttributesExW`

Resources: `FindResourceExW`, `FindResourceW`, `LoadResource`, `LockResource`, `SizeofResource`

CRT/Security: `IsDebuggerPresent`, `IsProcessorFeaturePresent`, `DecodePointer`, `EncodePointer`, `GetTickCount64`, `GetTickCount`, `QueryPerformanceCounter`, `GetSystemTimeAsFileTime`, `GetCurrentThreadId`

Threading: `EnterCriticalSection`, `LeaveCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `DeleteCriticalSection`

Other: `SetThreadExecutionState` (prevent sleep), `SetDllDirectoryW`, `GetVersion`, `InterlockedIncrement`, `InterlockedDecrement`, `FormatMessageW`

### MSVCR110.dll — Visual C++ 2012 Runtime
Standard CRT: `malloc`, `free`, `calloc`, `memcmp`, `memcpy`, `memcpy_s`, `memmove_s`, `memset`, `wcslen`, `wcsnlen`, `wcstoul`, `wmemcpy_s`, `swscanf_s`, `vswprintf_s`, `fprintf`, `fflush`, `tolower`, `isdigit`, `_wcsicmp`, `_wtof`, `_wfullpath`

CRT startup/exit: `_XcptFilter`, `__CxxFrameHandler3`, `_CxxThrowException`, `__set_app_type`, `__wgetmainargs`, `_cexit`, `_exit`, `exit`, `_amsg_exit`, `__setusermatherr`, `__dllonexit`, `_onexit`, `__initterm`, `__initterm_e`, `__winitenv`, `_configthreadlocale`, `__crtSetUnhandledExceptionFilter`, `__crtTerminateProcess`, `__crtUnhandledException`, `_crt_debugger_hook`, `_except_handler4_common`, `_controlfp_s`, `_invoke_watson`, `_purecall`, `_recalloc`, `_vscwprintf`, `__iob_func`, `_lock`, `_unlock`, `_calloc_crt`, `_commode`, `_fmode`

### ole32.dll — COM Foundation
| Function | Purpose |
|----------|---------|
| `CoInitializeEx` | Initialize COM |
| `CoUninitialize` | Uninitialize COM |
| `CoCreateInstance` | Create COM object |
| `CoTaskMemAlloc` | COM memory allocation |
| `CoTaskMemFree` | COM memory free |
| `CLSIDFromString` | CLSID string→GUID |
| `StringFromCLSID` | CLSID GUID→string |
| `PropVariantClear` | Clear PROPVARIANT |

### OLEAUT32.dll — Automation (ordinal imports)
Ordinals 2, 4, 6, 9 — likely `SysAllocString`, `SysFreeString`, `VariantClear`, `SysStringLen`.

### WLXPhotoBase.dll — Windows Live Base Runtime
| Symbol | Description |
|--------|-------------|
| `BaseAtlThrow@ATL@@YGXJ@Z` | ATL exception throw |
| `BasePrivate::New` | Custom memory allocation |
| `BasePrivate::Delete` | Custom memory deallocation |
| `Base::Throw` | Exception throw |
| `Base::ThrowLastError` | Throw last error |
| `Base::BException::operator J` | Get exception HRESULT |
| `Base::Exception::~Exception` | Exception destructor |
| `OS::Base::IsWin8OrGreater` | Windows 8 version check |
| `OS::Base::IsWin7OrGreater` | Windows 7 version check |
| `String::Base::GetBaseStringManager` | ATL CAtlStringMgr access |

### d3d9.dll — Direct3D 9
- `Direct3DCreate9Ex` — Create D3D9 device for DXVA

### d3d11.dll — Direct3D 11
- `D3D11CreateDevice` — Create D3D11 device for DXVA

### dxva2.dll — DirectX Video Acceleration 2
- `DXVA2CreateVideoService` — Create DXVA2 video service (hardware decode)
- `DXVA2CreateDirect3DDeviceManager9` — Create D3D9 device manager for DXVA

### PROPSYS.dll — Property System
- `PSCreateMemoryPropertyStore` — In-memory property store

### SHLWAPI.dll — Shell Path Utilities
- `PathAppendW`, `PathAddBackslashW`, `PathRemoveBackslashW`, `PathRemoveFileSpecW` — Path manipulation
- `PathFileExistsW` — File existence check
- `PathFindFileNameW`, `PathFindExtensionW` — Path parsing

### MF.dll — Media Foundation Pipeline
| Function | Purpose |
|----------|---------|
| `MFCreateTranscodeProfile` | Create transcode profile object |
| `MFCreateTranscodeTopology` | Create transcode topology |
| `MFCreateTopologyNode` | Create topology node |
| `MFCreateMediaSession` | Create Media Session |
| `MFTranscodeGetAudioOutputAvailableTypes` | Enumerate audio output types |
| `MFGetService` | Get service from MF object |

### MFPlat.DLL — Media Foundation Platform
| Function | Purpose |
|----------|---------|
| `MFStartup` / `MFShutdown` | MF platform init/shutdown |
| `MFLockPlatform` / `MFUnlockPlatform` | Platform lock |
| `MFCreateAttributes` | Create attribute store |
| `MFCreateCollection` | Create collection |
| `MFCreateMediaType` | Create media type |
| `MFCreateSample` | Create media sample |
| `MFCreateMemoryBuffer` | Create memory buffer |
| `MFCreateSourceResolver` | Create source resolver |
| `MFPutWorkItemEx` | Queue async work item |
| `MFInvokeCallback` | Invoke async callback |
| `MFGetPluginControl` | Get plugin control |
| `MFFrameRateToAverageTimePerFrame` | Frame rate conversion |
| `MFAverageTimePerFrameToFrameRate` | Frame rate conversion |

### SHELL32.dll
- `SHGetPropertyStoreFromParsingName` — Get property store from file path

### USER32.dll
- `GetDesktopWindow` — Get desktop HWND
- `OffsetRect` — Rect manipulation
- `SetRect` — Rect initialization

---

## COM / ATL Class Hierarchy (from RTTI / mangled names)

Classes are organized under two namespaces: `HMRAVSource` and `HMRTranscode`.

### HMRAVSource Namespace (Source/Buffer/Processing Layer)
| Class | Role |
|-------|------|
| `AVSourceFactory` | Factory for creating AV source instances |
| `AVSourceProxy` | Proxy for cross-thread/cross-process source access |
| `AVSource` | Core AV source (file → Media Foundation source reader) |
| `DShowMFSourceReaderBuilder` | Builds source reader from DirectShow filters |
| `NativeMFSourceReaderBuilder` | Builds source reader from native MF source |
| `AsyncSourceResolver` | Asynchronously resolves media source URLs |
| `MFByteStreamOnStreamAsyncResult` | Async result for byte stream wrapping |
| `AVSink` | Sink object for AV output |
| `XVideoProc` | Software-based video processing (colorspace, scaling) |
| `DXVA2VideoProc` | DXVA2 hardware-accelerated video processing |
| `AuthCredentials` | Authentication credentials for protected content |
| `MFByteStreamOnByteStream` | MF byte stream wrapper |

### HMRTranscode Namespace (Transcoding Layer)
| Class | Role |
|-------|------|
| `CodecMetadataParser` | Parses codec metadata from source |
| `MFTranscodeJob` | **Main transcoding job controller** — manages the MF transcode topology lifecycle |

### Infrastructure
- `CComObjectRootBase` (ATL)
- `CComObject<...>` (ATL COM object wrappers)
- `CAtlStringMgr` (ATL string manager from WLXPhotoBase)

---

## Video Codec FOURCCs / MF Subtypes

Found in `.text` as Media Foundation format GUID payloads:

| FOURCC | Codec |
|--------|-------|
| `H264` | H.264 / AVC |
| `WVC1` | VC-1 (SMPTE) |
| `WMV3` | Windows Media Video 9 |
| `WMV2` | Windows Media Video 8 |
| `WMV1` | Windows Media Video 7 |
| `MP43` | MPEG-4 Part 2 (Microsoft variant) |
| `MP4V` | MPEG-4 Part 2 (standard) |
| `MP4S` | MPEG-4 Part 2 Simple Profile |

### Additional video format strings (MFVideoFormat / FOURCC)
| String | Likely Format |
|--------|---------------|
| `HNKd` | H.264 (alternate subtype GUID) |
| `P210` | YUV 4:2:2 10-bit |
| `v410` | YUV 4:4:4 10-bit |
| `auds` | Audio (AUDIO) |
| `P010` | YUV 4:2:0 10-bit |
| `Y216` | YUV 4:2:2 16-bit |
| `Y210` | YUV 4:2:2 10-bit (packed) |

---

## Resource / Manifest

### Assembly Identity
```xml
<assemblyIdentity
  version="5.1.0.0"
  processorArchitecture="x86"
  name="Microsoft.Windows.personalMedia.WLXTranscode"
  type="win32" />
<description>Microsoft Windows Live Movie Maker Transcode</description>
```

### VERSIONINFO
| Field | Value |
|-------|-------|
| FileVersion | `16.4.3528.0331_ship.client.main.w5m4 (ship)` |
| ProductVersion | `16.4.3528.0331` |
| CompanyName | Microsoft Corporation |
| FileDescription | Movie Maker Transcoder |
| InternalName | WLXTranscoder |
| OriginalFilename | WLXTranscoder.dll |
| ProductName | Photo Gallery |
| LegalCopyright | © 2012 Microsoft Corporation. All rights reserved. |

> **Note:** The VERSIONINFO calls it `WLXTranscoder.dll` and the manifest calls it `WLXTranscode.exe`. This binary was internally a DLL (`DllMain`/`TranscodeJob` COM object) but deployed as a standalone EXE tool.

### TrustInfo
- `requestedExecutionLevel: asInvoker`
- `uiAccess: false`

---

## ETW Tracing

The binary registers ETW trace providers (`RegisterTraceGuidsW`/`TraceEvent`) for diagnostics. The GUIDs for these providers are embedded in the `.text` section but not easily extractable via static analysis of the raw hex (GUID structures are interleaved with code).

---

## Analysis Summary

### Architecture
1. **Startup:** Calls `CoInitializeEx`, `MFStartup`, initializes ATL via WLXPhotoBase
2. **Source Resolution:** Uses `MFCreateSourceResolver` / async resolver to open input files (via `SHGetPropertyStoreFromParsingName` + `PSCreateMemoryPropertyStore` for metadata)
3. **Profile Configuration:** Creates a transcode profile via `MFCreateTranscodeProfile`, configures audio via `MFTranscodeGetAudioOutputAvailableTypes`
4. **Topology Building:** Constructs MF topology via `MFCreateTranscodeTopology` / `MFCreateTopologyNode`
5. **Media Session:** Creates `MFCreateMediaSession`, queues the topology, runs the session with async work items (`MFPutWorkItemEx`, `MFInvokeCallback`)
6. **Video Processing Path:**
   - **Hardware path:** DXVA2 (`DXVA2CreateVideoService`, `DXVA2CreateDirect3DDeviceManager9`) + D3D11 `D3D11CreateDevice`
   - **Software path:** `XVideoProc` (custom software video processor)
   - **Selection:** Based on `IsWin8OrGreater`/`IsWin7OrGreater` OS checks
7. **Output:** Transcodes to H.264, VC-1, WMV3, or MPEG-4 depending on profile
8. **Shutdown:** `MFShutdown`, `CoUninitialize`

### Key Design Points
- **No export table** — invoked directly, not as a COM server
- **All COM objects are private** — registered only via CoCreateInstance internally (no CoRegisterClassObject)
- **Two video processing paths:** Software (XVideoProc) and Hardware DXVA2 (DXVA2VideoProc) with D3D9/D3D11 interop
- **ETW tracing** throughout for performance diagnostics
- **Cryptography imports** (`CryptSignHashW`, `CryptHashData`) suggest profile signing or content authentication (possibly for playback rights in WLMM)
- **Thread safety:** Critical sections, semaphores, events, work queues
- **ATL-based** via WLXPhotoBase for memory management and exception handling
- **Codec coverage:** H.264, VC-1, WMV (all versions), MPEG-4 Part 2 (Simple/Advanced profiles)
