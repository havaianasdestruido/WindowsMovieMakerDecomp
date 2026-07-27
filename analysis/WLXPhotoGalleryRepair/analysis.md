# WLXPhotoGalleryRepair.exe - Reverse Engineering Analysis

## PE Header Summary

| Field | Value |
|---|---|
| Machine | 0x14C (x86) |
| Subsystem | 2 (Windows GUI) |
| Linker Version | 11.00 |
| Image Base | 0x00400000 |
| Entry Point | 0x00401F62 |
| Timestamp | Tue Apr 1 01:26:57 2014 |
| Sections | .text, .data, .rsrc, .reloc |
| DLL Characteristics | Dynamic base, NX compatible, Terminal Server Aware |
| Stack Reserve/Commit | 0x40000 / 0x2000 |
| Heap Reserve/Commit | 0x100000 / 0x1000 |

## PDB / Debug Info

- **PDB**: `WLXPhotoGalleryRepair.pdb`
- **GUID**: `{573D2BF0-1AAD-45F5-B067-E1F44BB387EB}`

## Resource Version Info

| Field | Value |
|---|---|
| CompanyName | Microsoft Corporation |
| FileDescription | **Photo Gallery Repair** |
| FileVersion | 16.4.3528.0331_ship.client.main.w5m4 (ship) |
| InternalName | WLXPhotoGalleryRepair |
| OriginalFilename | WLXPhotoGalleryRepair.dll |
| ProductName | Photo Gallery |
| ProductVersion | 16.4.3528.0331 |
| Copyright | 2012 Microsoft Corporation. All rights reserved. |

## Manifest

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!-- Copyright (c) Microsoft Corporation -->
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <assemblyIdentity
    name="Microsoft.Windows.personalMedia.WLXPhotoGalleryRepair"
    processorArchitecture="x86"
    version="5.1.0.0"
    type="win32" />
  <description>Windows Shell</description>
  <dependency>
    <dependentAssembly>
      <assemblyIdentity
        type="win32"
        name="Microsoft.Windows.Common-Controls"
        version="6.0.0.0"
        processorArchitecture="*"
        publicKeyToken="6595b64144ccf1df"
        language="*" />
    </dependentAssembly>
  </dependency>
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v2">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel level="asInvoker" uiAccess="false"/>
      </requestedPrivileges>
    </security>
  </trustInfo>
</assembly>
```

- Runs as **asInvoker** (no elevation required)
- Depends on **Common Controls v6** (visual styles)

## Import Analysis

### KERNEL32.dll (Core)
| Function | Purpose |
|---|---|
| `FreeLibrary` | Unload DLLs |
| `GetModuleHandleW` / `GetModuleHandleA` | Get DLL base addresses |
| `LoadLibraryW` / `LoadLibraryA` | Load DLLs at runtime |
| `GetFileAttributesW` | Check file existence/attributes |
| `GetModuleFileNameW` | Get own executable path |
| `GetProcAddress` | Resolve DLL exports dynamically |
| `InterlockedExchange` | Thread-safe global variable updates |
| `GetLastError` / `SetLastError` | Error handling |
| `HeapSetInformation` | Heap corruption detection |
| `OutputDebugStringA` | Debug logging |
| `GetVersion` | OS version detection |
| `IsDebuggerPresent` | Debugger detection |
| `GetTickCount64` | Timing |
| `GetSystemTimeAsFileTime` | Random seed / timing |
| `GetCurrentThreadId` | Thread identification |
| `QueryPerformanceCounter` | High-resolution timing |
| `EncodePointer` / `DecodePointer` | Pointer obfuscation (security) |
| `InitializeCriticalSectionEx` / `DeleteCriticalSection` | Thread synchronization |
| `IsProcessorFeaturePresent` | CPU feature detection |

### MSVCR110.dll (Visual C++ 2012 Runtime)
Standard CRT initialization: `_initterm`, `_initterm_e`, `_configthreadlocale`, `_commode`, `_fmode`, `_wcmdln`, `__wgetmainargs`, `_cexit`, exception handling (`_except_handler4_common`, `_XcptFilter`, `__crtSetUnhandledExceptionFilter`), memory (`free`, `memset`), and termination (`exit`, `_exit`, `?terminate@@YAXXZ`).

### ole32.dll
| Function | Purpose |
|---|---|
| `OleInitialize` | Initialize COM |
| `OleUninitialize` | Uninitialize COM |

### SHLWAPI.dll
| Function | Purpose |
|---|---|
| `PathAppendW` | Append path components |
| `PathRemoveFileSpecW` | Remove filename from path |

### WLXPhotoBase.dll (Windows Live Photo Base)
| Function | Purpose |
|---|---|
| `?Delete@BasePrivate@@YAXPAX@Z` | `BasePrivate::Delete(void*)` - memory deallocation |
| `?IsWin8OrGreater@OS@Base@@YG_NXZ` | `Base::OS::IsWin8OrGreater()` - OS version check |

### UXCore.dll (UX Core - Resource Management)
| Function | Purpose |
|---|---|
| `?RMFindModule@@YGPAUHINSTANCE__@@PBDK@Z` | `RMFindModule(char*, unsigned long)` - find a resource module |
| `?RMUpdateResourceSet@@YG_NPBDPB_WK11PAUHINSTANCE__@@@Z` | `RMUpdateResourceSet(char*, wchar_t const**, unsigned long, unsigned short, HINSTANCE**)` - update resource set |
| `UXCoreInitProcess` | Initialize UXCore process state |
| `UXCoreUnInitProcess` | Uninitialize UXCore process state |

## Runtime Function Imports (via Delayed/Dynamic Loading)

Strings reveal these additional Win32 APIs used via `GetProcAddress`:
- `CreateActCtxW`, `ActivateActCtx`, `DeactivateActCtx`, `FindActCtxSectionStringW`, `QueryActCtxW` - **Side-by-Side (SxS) Activation Context** API
- `GetModuleHandleExW` - extended module handle retrieval
- `InitCommonControls` from Comctl32.dll - common control initialization

## What Does Repair Do? (Logic Analysis)

### Core Purpose
WLXPhotoGalleryRepair.exe is a **Photo Gallery repair utility** for Windows Essentials / Windows Live Photo Gallery. Its assembly identity is `Microsoft.Windows.personalMedia.WLXPhotoGalleryRepair`.

### Execution Flow

1. **Entry Point** (`0x401F62`): Calls security cookie init (`__security_init_cookie` at `0x402276`) then jumps to CRT startup (`0x401DB4`).

2. **CRT Startup** (`0x401DB4`): Sets up SEH, calls `InterlockedExchange` for thread safety, then invokes the main repair logic.

3. **Initialization** (`0x401B14` - `wWinMain` equivalent):
   - Stores the hInstance parameter in a global (`[00403408h]`)
   - Calls `OleInitialize` to initialize COM
   - Calls `00401934` (the main repair orchestration function)
   - Initializes UXCore process (`UXCoreInitProcess`)
   - Registers with the **SxS Activation Context** system
   - Calls `00401A41` (path-based check/repair logic) if not already in activation context mode

4. **Repair Orchestration** (`0x401934`):
   - Calls `004018C2` which performs initialization via a **delayed/late-binding DLL loader** - loads `WLXPhotoLibraryMain.dll` dynamically
   - The function at `0x4018C2` checks several error codes (0x7F, 0x7E, 0x78) which correspond to specific known-broken states
   - If the system is already "committed" (`[00403418h]` != 0), it reuses cached state
   - Otherwise, it loads the library and invokes repair functions

5. **Library Loading via Delayed Binding** (`0x4015EF`):
   - This is the **core dynamic loader** - it resolves functions from `WLXPhotoLibraryMain.dll` by name using `GetProcAddress`
   - Cached in global variables (`[004033F0h]`, `[004033F4h]`, `[004033F8h]`, `[004033FCh]`, `[00403428h]`)
   - Two paths are tried: one for pre-Win8 and one for Win8+ (detected via `Base::OS::IsWin8OrGreater`)
   - The function at `0x4015EF` checks the OS version via `[00401034h]` (GetVersion) to determine which path to take

### String Constants (Repair Actions)

From the data section, these function name strings are resolved from `WLXPhotoLibraryMain.dll`:

| Address | String | Purpose |
|---|---|---|
| `0x401190h` | `CreateActCtxW` | Create activation context for SxS |
| `0x4011A0h` | `ActivateActCtx` | Activate context |
| `0x4011B0h` | `DeactivateActCtx` | Deactivate context |
| `0x4011C4h` | `GetModuleHandleExW` | Get extended module handle |
| `0x4011E0h` | `InitCommonControls` | Init common controls |
| `0x4011F0h` | (debug string) | `"IsolationAware function called after IsolationAwareCleanup"` |
| `0x401200h` | (debug string) | `"IsolationAware function called after IsolationAwareCleanup"` |

### File Operations

1. **Module Path Check** (`0x401A41`):
   - Gets own module path via `GetModuleFileNameW` (path buffer at `[ebp-204h]`, max 0x104 chars)
   - Calls `PathRemoveFileSpecW` to get the directory
   - Calls `PathAppendW` to append `"WLXPhotoLibraryMain.dll"` to the directory
   - Calls `GetProcAddress` on the resolved module to find the `"DisplayRepairPromptDialog"` export
   - If found, calls it and captures the return value (repair result)
   - If not found, returns `0x8000FFFF` (E_UNEXPECTED)
   - Always calls `FreeLibrary` on the loaded module after use

2. **DLL Path Construction**: The repair builds the path `%module_dir%\WLXPhotoLibraryMain.dll` (visible at `0x401294h` = "WLXPhotoLibraryMain.dll") and loads it from the same directory as the repair executable itself.

### Registry / Configuration Operations

The binary does **not directly access the registry**. All registry operations are delegated to `WLXPhotoLibraryMain.dll` through the dynamically resolved function pointers. The binary itself only:
- Loads `WLXPhotoLibraryMain.dll` from its own directory
- Resolves and calls exported functions
- Uses SxS activation contexts (which may internally read registry for side-by-side assembly lookups)

### Repair Dialog Display

The string at `0x4012C4h` is `"DisplayRepairPromptDialog"` - this is the key export called from `WLXPhotoLibraryMain.dll`. The repair flow is:
1. Load `WLXPhotoLibraryMain.dll`
2. Find `DisplayRepairPromptDialog` export
3. Call it to show the repair UI to the user
4. Return the result code

### Global State Machine

| Global Address | Purpose |
|---|---|
| `0x403020h` | Repair context handle (initialized to -1) |
| `0x403410h` | Delayed loader initialized flag |
| `0x403414h` | Repair context valid flag |
| `0x403418h` | "Committed" flag (repair already completed/irreversible) |
| `0x40341Ch` | Cached repair function pointer |
| `0x403420h` | Error flag (set if PE validation fails) |
| `0x403424h` | Cached DLL handle |
| `0x403428h` | Cached `GetModuleHandleExW` pointer |
| `0x40342Ch` | Thread lock (used with lock cmpxchg) |
| `0x403430h` | Activation context state (0=uninit, 1=first, 2=active) |
| `0x403434h` / `0x403438h` | Security token / saved token |

### PE Self-Validation

The binary validates its own PE structure at startup (`0x401CA6`):
- Checks MZ signature at image base
- Validates PE signature
- Confirms PE32 (0x10B) magic
- Checks data directory count >= 14
- Checks for presence of CLR header (determines if mixed-mode)

Additionally at `0x401C56`, it calls `CreateActCtxW` via the resolved activation context API to set up SxS context.

## RTTI (Run-Time Type Information)

No custom C++ RTTI structures were found in the binary. The binary is relatively simple C code with no visible vtable/RTTI patterns. The class hierarchies are:
- `Base::OS` - static utility class (method `IsWin8OrGreater()`)
- `BasePrivate` - memory management namespace (function `Delete(void*)`)

These are from the `WLXPhotoBase.dll` helper library, not defined in this binary.

## Digital Signature

The binary is **Microsoft-signed** with dual signatures:
1. **Code Signing**: Microsoft Corporation, signed by Microsoft Code Signing PCA 2011
2. **Time Stamping**: Microsoft Time-Stamp Service, via nCipher DSE
3. **Root Chain**: Microsoft Root Certificate Authority 2011 / Microsoft Root CA

## Security Features

- **GS Cookie** (`/GS`): Stack buffer overrun detection (27 functions protected)
- **ASLR** (`Dynamic base`): Image base randomization
- **DEP/NX** (`NX compatible`): Data execution prevention
- **SafeSEH** (`/sdl=0`): Not explicitly enabled
- **CFG**: Not present
- **EncodePointer/DecodePointer**: Heap pointer obfuscation used for function pointers
- **HeapSetInformation**: Heap corruption detection enabled

## Summary

WLXPhotoGalleryRepair.exe is a lightweight x86 GUI utility (24 KB) that:
1. Initializes COM and UXCore
2. Dynamically loads `WLXPhotoLibraryMain.dll` from its own directory
3. Resolves the `DisplayRepairPromptDialog` export
4. Calls it to show a repair prompt dialog to the user
5. Handles OS version differences (pre-Win8 vs Win8+) via `Base::OS::IsWin8OrGreater`
6. Uses SxS activation contexts for proper DLL isolation
7. Delegates all actual repair logic to `WLXPhotoLibraryMain.dll`

The binary itself performs **no file modification or registry writes** - it is purely a UI shim that loads the actual repair library and presents the repair dialog.
