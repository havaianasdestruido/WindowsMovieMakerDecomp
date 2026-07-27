# WLXQuickTime Integration Components - Static Analysis

> **Date:** 2026-07-27
> **Source:** Windows Live Photo Gallery 16.4.3528 (ship, client.main.w5m4)
> **Copyright:** 2012 Microsoft Corporation

Three binaries provide QuickTime format support in Windows Live Photo Gallery: an ActiveX control host (EXE), a COM proxy/stub DLL, and a shell thumbnail extension DLL.

---

## 1. WLXQuickTimeControlHost.exe

**Path:** `undecomp\Photo Gallery\WLXQuickTimeControlHost.exe`
**Size:** 139,456 bytes | **Subsystem:** Windows GUI (PE32, x86)
**Timestamp:** Tue Apr 1 01:27:04 2014 (UTC)
**PDB:** `{6748E991-8444-4897-8D72-1FE63B110F50}` → `WLXQuickTimeControlHost.pdb`
**Entry Point:** `0x00412835`

### Purpose
Local-server ActiveX EXE that hosts the Apple QuickTime ActiveX control within Photo Gallery. Provides two COM classes for QuickTime playback and movie thumbnail extraction. Registers as a LocalServer32 (out-of-process COM) to isolate QuickTime's third-party code from the Photo Gallery process.

### Imports
| DLL | Notable Functions |
|---|---|
| ADVAPI32.dll | Full registry API (RegCreateKeyExW, RegOpenKeyExW, RegSetValueExW, RegDeleteKeyW, RegQueryInfoKeyW, RegEnumKeyExW), ETW tracing (RegisterTraceGuidsW, TraceEvent, GetTraceLoggerHandle) |
| KERNEL32.dll | LoadLibraryW/A, GetProcAddress, FreeLibrary, resource loading (FindResourceExW, LoadResource), GlobalAlloc/GlobalLock, HeapAlloc, CreateThread, CreateMutexA, GetShortPathNameW, FlushInstructionCache, VirtualAlloc/VirtualFree |
| GDI32.dll | CreateDIBSection, CreateCompatibleBitmap, BitBlt, SelectObject, GetStockObject (thumbnail rendering) |
| USER32.dll | CreateWindowExW, RegisterClassExW, DefWindowProcW, SetWindowLongW, GetMessageW, TranslateMessage, DispatchMessageW, MoveWindow, ShowWindow (host window management) |
| MSVCR110.dll | C++ runtime (VS2012), __CxxFrameHandler3, exception handling, wide string functions (wcsncpy_s, vswprintf_s) |
| ole32.dll | CoRegisterClassObject, CoGetClassObject, CoCreateInstance, CoInitialize, OleInitialize, CoAddRefServerProcess, CoReleaseServerProcess, CoResumeClassObjects, CLSIDFromString, CLSIDFromProgID, StringFromGUID2, CreateStreamOnHGlobal |
| OLEAUT32.dll | Ordinals 2,4,7,8,9,161,162,163,186,277,420 (SysAllocString, SysFreeString, SysStringLen, SysStringByteLen, VariantInit, etc.) |
| SHLWAPI.dll | PathFileExistsW |
| WLXPhotoBase.dll | `BasePrivate::New`, `BasePrivate::Delete`, `Base::ThrowLastError`, `Base::Throw`, `Base::Exception::~Exception`, `String::Base::GetBaseStringManager`, `ATL::BaseAtlThrow` |
| VERSION.dll | GetFileVersionInfoSizeA, GetFileVersionInfoA, VerQueryValueA |

### RTTI Classes
| Class | Description |
|---|---|
| `CWLXQuickTimeControlHostModule` | ATL module class, inherits `CAtlModule` |
| `QuickTimePlayerHost` | ActiveX control host for QuickTime movie playback |
| `QuickTimeMovieThumbnail` | Generates thumbnails from QuickTime movie files |
| `QuickTimeHostFrameWindow` | Frame window for hosting the QuickTime control |
| `QuickTimeSupportTracing` | ETW tracing provider for QuickTime support diagnostics |
| `IQuickTimePlayer` | Interface for the player host |
| `IQuickTimeMovieThumbnail` | Interface for the thumbnail generator |

### COM Registrations (Embedded .reg Script)

#### AppID
```
HKCR\AppID
  '%APPID%' = s 'WLXQuickTimeControlHost'
    'WLXQuickTimeControlHost.EXE'
      val AppID = s '%APPID%'
```

#### CLSID: QuickTimePlayerHost
```
HKCR\CLSID
  ForceRemove {B9AD19CB-FA75-4B29-B4A4-86C7E9616390} = s 'QuickTimePlayerHost Class'
    ProgID = s 'WLXQuickTimeControlHost.QuickTimePlayer.1'
    VersionIndependentProgID = s 'WLXQuickTimeControlHost.QuickTimePlay'
    CurVer = s 'WLXQuickTimeControlHost.QuickTimePlay.1'
    LocalServer32 = s '%MODULE%'
      val AppID = s '%APPID%'
    TypeLib = s '{C6D340BB-0CEA-4923-8082-51036E472379}'
```

#### CLSID: QuickTimeMovieThumbnail
```
HKCR\CLSID
  ForceRemove {AE3A66BB-85FE-49B8-BF7B-4DB4E0005091} = s 'QuickTimeMovieThumbnail Class'
    ProgID = s 'WLXQuickTimeControlHost.QuickTimeMovieThumbnail.1'
    VersionIndependentProgID = s 'WLXQuickTimeControlHost.QuickTimeMovieThumbnail'
    CurVer = s 'WLXQuickTimeControlHost.QuickTimeMovieThumbnail.1'
    LocalServer32 = s '%MODULE%'
      val AppID = s '%APPID%'
    TypeLib = s '{C6D340BB-0CEA-4923-8082-51036E472379}'
```

### Embedded Manifest
```xml
<!-- xmlns='urn:schemas-microsoft-com:asm.v1' manifestVersion='1.0' -->
<assemblyIdentity
    version="5.1.0.0"
    processorArchitecture="x86"
    name="WLXQuickTimeControlHost"
    type="win32" />
<!-- <description>Win32 QuickTime Control Host</description> -->
<!-- <trustInfo> (no explicit requestedExecutionLevel) -->
```

### QuickTime API Usage (Strings)
- **Registry:** `Software\Apple Computer, Inc.\QuickTime` — reads QuickTime installation path
- **Keys read:** `QTSysDir` (system directory), `QTExtDir` (extensions directory)
- **DLL exports called:** `QuickTime.qts` folder, `QTMLInit`, `TermMutex` (mutex: `TermMutex%lx`)
- **Dispatch:** `theQuickTimeDispatcher`, `_CallComponent`, `_CallComponentStorage`
- **COM:** `CLSIDFromProgID`, `CLSIDFromString`, `OleInitialize`

---

## 2. WLXQuickTimeControlHostPS.dll

**Path:** `undecomp\Photo Gallery\WLXQuickTimeControlHostPS.dll`
**Size:** 27,840 bytes | **Subsystem:** Windows CUI (PE32, x86)
**Timestamp:** Tue Apr 1 01:28:26 2014 (UTC)
**PDB:** `{35667A94-982A-4EAA-89AD-60CA227B0184}` → `WLXQuickTimeControlHostPS.pdb`
**Entry Point:** `0x10001B07`

### Purpose
Standard MIDL-generated COM proxy/stub DLL for the `IQuickTimePlayer` interface. Enables marshaling of `IQuickTimePlayer` calls between the Photo Gallery client process and the WLXQuickTimeControlHost.exe local server.

### Exports
| Ordinal | Name |
|---|---|
| 1 | `DllCanUnloadNow` |
| 2 | `DllGetClassObject` |
| 3 | `DllRegisterServer` |
| 4 | `DllUnregisterServer` |
| 5 | `GetProxyDllInfo` |

### Sections
| Section | Size | Purpose |
|---|---|---|
| `.text` | 0x1C00 | Code |
| `.orpc` | 0x200 | RPC interface data (NDR format strings, proxy/stub vtable layout) |
| `.data` | 0x200 | Initialized data |
| `.rsrc` | 0x600 | Resources (type library, interface info) |
| `.reloc` | 0x400 | Relocations |

### Imports
| DLL | Notable Functions |
|---|---|
| RPCRT4.dll | `CStdStubBuffer_*` (AddRef, Release, QueryInterface, Connect, Disconnect, Invoke, IsIIDSupported, CountRefs, DebugServerQueryInterface, DebugServerRelease), `NdrDllGetClassObject`, `NdrDllCanUnloadNow`, `NdrDllRegisterProxy`, `NdrDllUnregisterProxy`, `NdrOleAllocate`, `NdrOleFree`, `NdrCStdStubBuffer_Release`, `IUnknown_*_Proxy` |
| ole32.dll | `HBITMAP_UserFree/Unmarshal/Marshal/Size`, `HWND_UserFree/Unmarshal/Marshal/Size` |
| MSVCR110.dll | CRT initialization (_initterm, _initterm_e), memory (malloc, free, _calloc_crt) |
| KERNEL32.dll | `DisableThreadLibraryCalls`, `IsDebuggerPresent`, entropy (GetTickCount64, QueryPerformanceCounter, GetSystemTimeAsFileTime, GetCurrentThreadId) |

### Interface
- **IQuickTimePlayer** — referenced by ordinal in `.orpc` section
- Standard `CStdStubBuffer` vtable for COM interface marshaling

---

## 3. WLXQuickTimeShellExt.dll

**Path:** `undecomp\Photo Gallery\WLXQuickTimeShellExt.dll`
**Size:** 59,072 bytes | **Subsystem:** Windows GUI (PE32, x86)
**Timestamp:** Tue Apr 1 01:26:48 2014 (UTC)
**PDB:** `{F54E0511-6DF1-496E-8C0D-B1E48F8C0948}` → `WLXQuickTimeShellExt.pdb`
**Entry Point:** `0x10006FED`

### Purpose
In-process shell extension DLL that provides Windows Explorer thumbnail and image extraction support for QuickTime media files. Implements `IThumbnailProvider` and `IExtractImage` COM interfaces.

### Version Information
| Field | Value |
|---|---|
| Company | Microsoft Corporation |
| Description | Photo Gallery QuickTime Shell Extension |
| Product | Photo Gallery |
| ProductVersion | 16.4.3528.0331 |
| FileVersion | 16.4.3528.0331_ship.client.main.w5m4 (ship) |
| InternalName | WLXQuickTimeShellExt |
| Copyright | 2012 Microsoft Corporation. All rights reserved. |
| OriginalFilename | WLXQuickTimeShellExt.dll |
| ResourceName | QuickTimeSupport (translation string) |

### Exports
| Ordinal | Name |
|---|---|
| 1 | `DllCanUnloadNow` |
| 2 | `DllGetClassObject` |
| 3 | `DllRegisterServer` |
| 4 | `DllUnregisterServer` |

### Imports
| DLL | Notable Functions |
|---|---|
| MSVCR110.dll | C++ runtime, string functions (wcsncpy_s, wcsstr, wcscat_s, wcscpy_s, vswprintf_s) |
| KERNEL32.dll | LoadLibraryExW, GetProcAddress, FreeLibrary, GetModuleHandleW/FileNameW, resource loading, `SetThreadLocale`/`GetThreadLocale` |
| USER32.dll | CharNextW |
| ADVAPI32.dll | Full registry API (RegCreateKeyExW, RegOpenKeyExW, RegSetValueExW, RegDeleteKeyW), ETW tracing |
| GDI32.dll | DeleteObject |
| ole32.dll | CoTaskMemAlloc/Free/Realloc, CoCreateInstance, StringFromGUID2 |
| OLEAUT32.dll | Ordinals 2,6,161,186,277 |
| WLXPhotoBase.dll | `BasePrivate::New`, `BasePrivate::Delete`, `Base::Throw`, `ATL::BaseAtlThrow`, `String::Base::GetBaseStringManager`, `Base::Exception::~Exception` |

### RTTI Classes
| Class | Description |
|---|---|
| `MovieThumbnail` | Core thumbnail extraction class |
| `QuickTimeSupportModule` | ATL DLL module (`CAtlDllModuleT<QuickTimeSupportModule>`) |
| `QuickTimeSupportTracing` | ETW tracing provider |
| `CComObjectCached<CComClassFactory>` | Cached class factory for COM objects |

### COM Interfaces Implemented
| Interface | Purpose |
|---|---|
| `IThumbnailProvider` | Windows Vista+ thumbnail protocol (IThumbnailProvider::GetThumbnail) |
| `IExtractImage` | Legacy (XP-era) image extraction (IExtractImage::Extract) |
| `IPersistFile` | File path loading for the extractor |
| `IPersist` | Class identifier persistence |
| `IInitializeWithFile` | Win7+ file-based initializer for thumbnail providers |
| `IClassFactory` | Standard COM class factory |

### COM Registrations (Embedded .reg Script)

#### AppID
```
HKCR\AppID
  '%APPID%' = s 'QuickTimeShellExt'
    'WLXQuickTimeShellExt.DLL'
      val AppID = s '%APPID%'
```

#### CLSID: MovieThumbnail
```
HKCR\CLSID
  ForceRemove {007EFBDF-8A5D-4930-97CC-A4B437CBA777} = s 'MovieThumbnail Class'
    InprocServer32 = s '%MODULE%'
      val ThreadingModel = s 'Apartment'
      val AppID = s '%APPID%'
```

### Shell Extension Registrations

Two well-known shell handler CLSIDs are registered per file extension:

| Shell Handler CLSID | Interface | Description |
|---|---|---|
| `{E357FCCD-A995-4576-B01F-234630154E96}` | IThumbnailProvider | Thumbnail provider (Vista+) |
| `{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}` | IExtractImage | Image extractor (legacy) |

Both map to `{007EFBDF-8A5D-4930-97CC-A4B437CBA777}` (MovieThumbnail).

### Supported File Extensions
Each extension has `NoRemove shellex` with `ForceRemove` entries for both handlers:

| Extension | Thumbnail Handler | Extract Handler |
|---|---|---|
| `.mov` | `{E357FCCD-...}` → MovieThumbnail | `{BB2E617C-...}` → MovieThumbnail |
| `.qt` | `{E357FCCD-...}` → MovieThumbnail | `{BB2E617C-...}` → MovieThumbnail |
| `.mp4` | `{E357FCCD-...}` → MovieThumbnail | `{BB2E617C-...}` → MovieThumbnail |
| `.mqv` | `{E357FCCD-...}` → MovieThumbnail | `{BB2E617C-...}` → MovieThumbnail |
| `.3gp` | `{E357FCCD-...}` → MovieThumbnail | `{BB2E617C-...}` → MovieThumbnail |
| `.3gpp` | `{E357FCCD-...}` → MovieThumbnail | `{BB2E617C-...}` → MovieThumbnail |
| `.3g2` | `{E357FCCD-...}` → MovieThumbnail | `{BB2E617C-...}` → MovieThumbnail |
| `.3gp2` | `{E357FCCD-...}` → MovieThumbnail | `{BB2E617C-...}` → MovieThumbnail |

---

## Summary of CLSIDs

| CLSID | Component | Class |
|---|---|---|
| `{B9AD19CB-FA75-4B29-B4A4-86C7E9616390}` | ControlHost.exe | QuickTimePlayerHost |
| `{AE3A66BB-85FE-49B8-BF7B-4DB4E0005091}` | ControlHost.exe | QuickTimeMovieThumbnail |
| `{007EFBDF-8A5D-4930-97CC-A4B437CBA777}` | ShellExt.dll | MovieThumbnail |

### TypeLib
| TypeLib | Component |
|---|---|
| `{C6D340BB-0CEA-4923-8082-51036E472379}` | ControlHost.exe (both classes) |

### Known Shell Handler CLSIDs (Microsoft)
| CLSID | Interface |
|---|---|
| `{E357FCCD-A995-4576-B01F-234630154E96}` | IThumbnailProvider |
| `{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}` | IExtractImage |

---

## Architecture

```
Photo Gallery (WLXPhotoGallery.exe)
  |
  |-- WLXQuickTimeControlHostPS.dll (proxy/stub)
  |     |
  |     +-- [RPC marshaling via IQuickTimePlayer]
  |
  |-- WLXQuickTimeControlHost.exe (LocalServer32)
  |     |
  |     +-- Apple QuickTime ActiveX control (QuickTime.qts)
  |     +-- Reads: HKLM\Software\Apple Computer, Inc.\QuickTime
  |     +-- Classes: QuickTimePlayerHost, QuickTimeMovieThumbnail
  |
  +-- WLXQuickTimeShellExt.dll (InprocServer32, Apartment-threaded)
        |
        +-- Explorer thumbnails for .mov, .qt, .mp4, .mqv,
        |   .3gp, .3gpp, .3g2, .3gp2
        +-- Implements IThumbnailProvider + IExtractImage
```
