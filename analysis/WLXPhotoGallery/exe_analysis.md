# WLXPhotoGallery.exe — Static Analysis

## 1. PE Header Summary

| Field | Value |
|---|---|
| Machine | x86 (0x14C) |
| Magic | PE32 (0x10B) |
| Linker Version | 11.00 |
| Timestamp | Tue Apr 1 01:26:18 2014 (0x533A3FEA) |
| Entry Point | 0x0040168F (.text) |
| Image Base | 0x00400000 |
| Size of Image | 0x22000 (139,264 bytes) |
| File Size | ~143,560 bytes |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | Dynamic base (ASLR), NX compatible (DEP), Terminal Server Aware |
| Stack Reserve / Commit | 0x40000 / 0x2000 |
| Heap Reserve / Commit | 0x100000 / 0x1000 |
| Sections | .text (0x7000), .data (0x1000), .rsrc (0x18000), .reloc (0x1000) |
| Debug PDB | WLXPhotoGallery.pdb ({D5847618-47E4-454A-AB7D-80D90A1FA7CC}) |
| Security Cookie | 0x004080C8 |
| SEH Handler Count | 9 |

## 2. Entry Point Disassembly

The entry point at `0x0040168F` performs the following:

```asm
0040168F: call    004019F6            ; CRT initialization (verifies MZ/PE headers, sets up _except_handler4)
00401694: push    14h                 ; SEH frame size
00401696: push    4016B0h             ; SEH handler descriptor
0040169B: call    004015B8            ; __security_init_cookie + SEH frame registration
004016A0: and     dword ptr [ebp-1Ch], 0  ; clear local var (exception state)
004016A4: call    00406123            ; Main application logic / WinMain
004016A9: jmp     00401E62            ; CRT cleanup / exit path
```

### Startup Chain
1. **0x00401865** (`VerifyModuleIntegrity`): Validates MZ signature at 0x400000, PE signature, and PE32 magic.
2. **0x004019F6** (`__security_init_cookie`): Initializes the GS stack cookie from `GetSystemTimeAsFileTime`.
3. **0x004015B8** (`__except_handler4_prolog`): Registers SEH frame with `/GS` security cookie XOR.
4. **0x00406123**: Main application entry — likely resolves COM, loads resources, and launches the Photo Gallery UI.

### Notable Internal Functions

| Address | Function |
|---|---|
| 0x0040126C | `Base::Delete` — heap free wrapper with flag check |
| 0x004012CC | CRT initializer loop (`_initterm` equivalent) |
| 0x0040133F | Memory allocation wrapper (flags 0x2/0x1 → HeapAlloc with/without zeroing) |
| 0x00401432 | Heap manager init (sets up vtable at 0x4080F8, calls HeapCreate) |
| 0x0040152A | Registry/config open routine (creates handle from name + flag) |
| 0x00401564 | Wide string copy with bounds checking (MAX_PATH = 0x104 chars) |
| 0x00401664 | Boolean wrapper around 0x4015FD (SEH-based try block) |
| 0x00401679 | COM server initialization (calls 0x4017F1 → 0x4017D8 → CoInitializeEx) |
| 0x004017F1 | COM apartment setup (sets vtable, calls CoInitializeEx via 0x401058) |
| 0x004018FC | Heap creation (commit=0x10000, reserve=0x30000) |
| 0x00401944 | Module path resolution (GetModuleFileNameW → backslash search → LoadLibraryExW) |
| 0x00401951 | Full path builder with security cookie check (MAX_PATH buffer) |

### Application Manifest Activation (SxS without manifest)

The EXE uses **activation context API** to load common controls v6 without an embedded manifest:

```asm
; 0x4016E0 — manifest activation check
push    401754h             ; string "Kernel32.dll"
push    401744h             ; string "CreateActCtxW"
call    GetProcAddress       ; resolve dynamically
; ...
call    ActivateActCtx       ; activate visual styles context
```

Manifest-related strings found in .text section:
- `"Kernel32.dll"`, `"Comctl32.dll"`
- `"CreateActCtxW"`, `"FindActCtxSectionStringW"`, `"ActivateActCtx"`, `"DeactivateActCtx"`
- `"QueryActCtxW"`, `"GetModuleHandleExW"`

## 3. Imports (Static Load)

### ADVAPI32.dll — Registry & ETW Tracing
| Import | Purpose |
|---|---|
| `TraceEvent` | ETW event emission |
| `GetTraceLoggerHandle` | ETW logger acquisition |
| `GetTraceEnableLevel` | ETW trace level query |
| `GetTraceEnableFlags` | ETW trace flags query |
| `RegisterTraceGuidsW` | ETW provider registration |
| `UnregisterTraceGuids` | ETW provider cleanup |
| `RegOpenKeyExW` | Registry key open |
| `RegOpenCurrentUser` | Open HKCU with user token |
| `RegSetValueExW` | Registry value write |
| `RegCloseKey` | Registry key close |
| `RegQueryValueExW` | Registry value read |

### KERNEL32.dll — Core OS Services
| Import | Purpose |
|---|---|
| `GetCurrentThreadId` / `GetCurrentProcessId` | Process/thread identity |
| `GetSystemTime` / `GetSystemTimeAsFileTime` | Time queries |
| `HeapDestroy` / `HeapAlloc` / `HeapReAlloc` / `HeapFree` / `HeapSize` / `GetProcessHeap` | Heap management |
| `EnterCriticalSection` / `LeaveCriticalSection` / `InitializeCriticalSectionEx` / `DeleteCriticalSection` | Thread synchronization |
| `CompareStringW` | Locale-aware string comparison |
| `GetThreadUILanguage` | UI language detection |
| `LoadLibraryExA` / `LoadLibraryA` / `LoadLibraryExW` / `LoadLibraryW` / `FreeLibrary` / `GetProcAddress` | Dynamic DLL loading |
| `FindResourceExW` / `FindResourceW` / `LoadResource` / `LockResource` / `SizeofResource` | PE resource access |
| `FormatMessageW` | Error message formatting |
| `GetModuleFileNameW` / `GetModuleHandleA` / `GetModuleHandleW` | Module introspection |
| `RaiseException` | Structured exception raise |
| `SetEnvironmentVariableW` / `GetEnvironmentVariableW` | Environment manipulation |
| `EncodePointer` / `DecodePointer` | Pointer obfuscation (security) |
| `IsDebuggerPresent` / `IsProcessorFeaturePresent` | Debug/feature detection |
| `QueryPerformanceCounter` | High-resolution timer |
| `InterlockedExchange` | Atomic operations |
| `MultiByteToWideChar` | String encoding conversion |
| `OutputDebugStringA` | Debug output |
| `lstrlenW` | Wide string length |

### MSVCR110.dll — Visual C++ 2012 Runtime
| Import | Purpose |
|---|---|
| `_CxxThrowException` | C++ exception throw |
| `__CxxFrameHandler3` | C++ exception frame handler |
| `_except_handler4_common` | `/GS` SEH4 handler |
| `_initterm` / `_initterm_e` | CRT initialization table walk |
| `??1type_info@@UAE@XZ` | `type_info` destructor (RTTI) |
| `?terminate@@YAXXZ` | `std::terminate` |
| `bsearch` / `free` / `memcpy_s` / `memmove_s` / `memset` | C runtime memory/string ops |
| `wcsnlen` / `wcsrchr` / `wcsspn` / `wcscspn` / `wcsncpy_s` / `wmemcpy_s` / `iswspace` / `_wcsicmp` | Wide string utilities |
| `__wgetmainargs` / `_wcmdln` | Wide-character argv setup |
| `__set_app_type` / `__setusermatherr` / `_configthreadlocale` | CRT configuration |
| `_onexit` / `__dllonexit` / `_cexit` / `_exit` / `exit` | Exit/cleanup |
| `_lock` / `_unlock` | CRT internal locks |
| `_commode` / `_fmode` | CRT mode globals |
| `_XcptFilter` / `__crtSetUnhandledExceptionFilter` | Exception filtering |
| `_crt_debugger_hook` / `_invoke_watson` | Debug CRT hooks |

### OLEAUT32.dll
| Ordinal | Purpose |
|---|---|
| 6 | `SysAllocString` |
| 7 | `SysFreeString` |

### COMCTL32.dll
| Import | Purpose |
|---|---|
| `InitCommonControlsEx` | Common controls v6 init |

### USER32.dll
| Import | Purpose |
|---|---|
| `GetActiveWindow` | Active window query |
| `LoadIconW` | Application icon load |
| `GetProcessDefaultLayout` | Default layout detection |

### ole32.dll
| Import | Purpose |
|---|---|
| `CoTaskMemAlloc` / `CoTaskMemFree` | COM task memory allocator |

### SHLWAPI.dll
| Import | Purpose |
|---|---|
| `StrRChrW` | Reverse character search |
| `StrStrIW` | Case-insensitive substring |
| `StrCmpIW` | Case-insensitive compare |

## 4. Delay-Loaded Imports

### SHELL32.dll (delay-loaded)
| Import | Purpose |
|---|---|
| `ShellExecuteExW` | Shell action execution |
| `ShellExecuteW` | Shell action (simplified) |
| `CommandLineToArgvW` | Command-line parsing |

## 5. DLL Dependencies (First-Party)

| DLL | Mangled Export | Demangled |
|---|---|---|
| **WLXPhotoBase.dll** | `?Delete@BasePrivate@@YAXPAX@Z` | `BasePrivate::Delete(void*)` |
| | `?Throw@Base@@YGXJ@Z` | `Base::Throw(long)` |
| | `??1Exception@Base@@UAE@XZ` | `Base::Exception::~Exception()` |
| | `?GetBaseStringManager@String@Base@@SGAAVCAtlStringMgr@ATL@@XZ` | `Base::String::GetBaseStringManager() → ATL::CAtlStringMgr&` |
| | `?BaseAtlThrow@ATL@@YGXJ@Z` | `ATL::BaseAtlThrow(long)` |
| **WLXPhotoSqm.dll** | `?Startup@Sqm@@YGXXZ` | `Sqm::Startup()` |
| | `?AddToStream@Sqm@@YGXKK@Z` | `Sqm::AddToStream(unsigned long, unsigned long)` |
| | `?Shutdown@Sqm@@YGXXZ` | `Sqm::Shutdown()` |
| **DmxBici.dll** | `?TransferExperienceToWeb@BiciWrapper@@YG_NPB_WPAPA_W@Z` | `BiciWrapper::TransferExperienceToWeb(wchar_t const*, wchar_t**&) → bool` |
| | `?AddStringToDataPoint@BiciWrapper@@YG_NKKPB_W@Z` | `BiciWrapper::AddStringToDataPoint(unsigned long, unsigned long, wchar_t const*) → bool` |

## 6. RTTI

The binary imports `??1type_info@@UAE@XZ` (`type_info::~type_info()`) from MSVCR110.dll, confirming C++ RTTI is enabled. RTTI class name strings are minimal in this thin launcher; the actual class hierarchy resides in WLXPhotoLibraryMain.dll and WLXPhotoBase.dll.

A single RTTI reference was found in the binary:
```
?aX??5
```
This appears to be a partial/mangled RTTI name near the end of the .text section.

## 7. String Constants

### Version Information (Resource)
| Field | Value |
|---|---|
| Company | Microsoft Corporation |
| Product Name | Photo Gallery |
| Internal Name | WLXPhotoGallery |
| Original Filename | WLXPhotoGallery.exe |
| File Description | Photo Gallery |
| File Version | 16.4.3528.0331 |
| Build String | 16.4.3528.0331_ship.client.main.w5m4 (ship) |

### URLs
- `http://g.live-int.com` — internal/update endpoint
- `http://g.live.com` — public endpoint
- `%s/%s%s/%u` — URL format template

### API/Feature Strings
- `RegisterApplicationRestart` — crash recovery registration
- `RunAsStandAlone` — standalone launch mode flag
- `LaunchViewerComServer` — COM server entry point for Photo Viewer
- `LaunchViewer` — direct viewer launch
- `InitializeEx` — extended initialization (COMCTL32)
- `Unicows.dll` — legacy Unicode layer (pre-XP compatibility)
- `Kernel32.dll`, `Comctl32.dll` — manifest activation context targets
- `dmxmm` — registry key (possibly DRM/media metadata)

### DLL References (in .rsrc)
- `MetadataSys.dll`, `WLXVideoTrim.dll`, `DmxBici.dll`, `UXCore.dll`, `sqmapi.dll`

### Locale Support (90+ locales)
Full i18n coverage including: am-ET, ar-sa, as-IN, az-Latn-AZ, bn-BD, bs-Latn-BA, cs-cz, cy-GB, da-dk, de-de, el-gr, en-us, es-es, et-ee, eu-es, fa-IR, fi-fi, fil-PH, ga-IE, gl-ES, gu-in, ha-Latn-NG, hi-in, hr-hr, hu-hu, hy-AM, id-id, ig-NG, is-IS, it-it, iu-Latn-CA, ka-GE, kk-KZ, km-KH, kn-in, ko-kr, kok-IN, lb-LU, lt-lt, lv-lv, mi-NZ, mk-MK, ml-IN, mn-MN, mr-in, ms-my, mt-MT, ne-NP, nl-nl, nn-NO, nso-ZA, pa-IN, pl-pl, quz-PE, ru-ru, si-LK, sk-sk, sl-si, sq-AL, sr-cyrl-ba, sw-KE, ta-in, te-in, th-th, tk-TM, tn-ZA, tr-tr, tt-RU, uk-ua, ur-PK, uz-Latn-UZ, zh-ZA, yo-NG, zu-ZA, pt-pt, zh-tw, and special locales (ar-ploc-sa, en-locr-us, prs-af, ku-Arab, pa-Arab, qut-Latn, sr-Cyrl-BA).

## 8. Architecture Summary

**WLXPhotoGallery.exe is a thin 32-bit launcher/shim** (~143 KB). It:

1. **Initializes the CRT** (MSVCR110.dll — Visual C++ 2012 runtime) with `/GS` stack cookies and SEH4 exception handling.
2. **Sets up a COM apartment** via `CoInitializeEx` for OLE/COM interop.
3. **Activates visual styles** dynamically using the SxS activation context API (`CreateActCtxW` / `ActivateActCtx`) rather than an embedded manifest, targeting `Comctl32.dll` v6.
4. **Loads first-party DLLs** at startup:
   - `WLXPhotoBase.dll` — base framework (ATL-based string management, exception hierarchy, memory management)
   - `WLXPhotoSqm.dll` — SQM telemetry (usage tracking)
   - `DmxBici.dll` — BICI telemetry (experience transfer to web)
5. **Delegates all UI and business logic** to `WLXPhotoLibraryMain.dll` and other companion DLLs.
6. **Supports crash recovery** via `RegisterApplicationRestart`.
7. **Has a standalone launch mode** (`RunAsStandAlone` flag).
8. **Can launch the Photo Viewer** via COM server activation (`LaunchViewerComServer`, `LaunchViewer`).
9. **ETW tracing** is integrated via ADVAPI32 for diagnostic logging.
10. **Registry operations** read/write configuration under HKCU (via `RegOpenCurrentUser`).

The `.rsrc` section (96 KB — 69% of the binary) contains version info, locale strings for 90+ languages, and UI resources. The `.text` code section is only ~27 KB of actual executable code.

### Security Features
- ASLR (Dynamic base)
- DEP/NX (NX compatible)
- `/GS` stack buffer overrun detection (security cookie at 0x4080C8, 9 SEH handlers)
- Pointer encoding (`EncodePointer` / `DecodePointer`)
- `IsDebuggerPresent` check
- Terminal Server Aware (multi-session compatible)

## 9. Load Configuration

| Field | Value |
|---|---|
| Size | 0x48 |
| Dependent Load Flag | 0x0000 (default — follows process DEP setting) |
| Security Cookie | 0x004080C8 |
| Safe Exception Handler Table | 0x00402D58 (9 entries) |

### Safe Exception Handler Addresses
```
0x004062B9  0x004068D0  0x0040692C
0x0040697C  0x004069CC  0x00406A20
0x00406AA8  0x00406B10  0x00406B60
```

These correspond to the C++ exception handling frames scattered throughout the .text section — typical of `/EHa` or `/EHsc` compiled C++ code with structured exception handling.
