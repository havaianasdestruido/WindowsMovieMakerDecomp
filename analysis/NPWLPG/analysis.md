# NPWLPG.dll - Static Analysis

## Overview
**NPWLPG.dll** is a **NPAPI (Netscape Plugin Application Programming Interface) browser plugin** for Windows Live Photo Gallery (WLPG). It was designed to be loaded by browser NPAPI hosts (e.g., Firefox, IE inProtected Mode) to enable in-browser photo viewing/editing via the Windows Live Photo Gallery COM infrastructure.

**PDB:** `NPWLPG.pdb` (GUID: `{4E381E84-56D9-4D85-AAA4-E8B9EF7EBE14}`)

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Linker | MSVC 11.00 (VS2012) |
| Timestamp | 2014-04-01 01:28:30 UTC |
| Image Base | 0x10000000 |
| Size of Image | 0x5000 (20 KB) |
| Entry Point | 0x1453 |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | Dynamic base, NX compatible |

## Sections
| Name | VirtAddr | VirtSize | RawSize | Flags |
|------|----------|----------|---------|-------|
| .text | 0x1000 | 0xF1D | 0x1000 | Code, Execute Read |
| .data | 0x2000 | 0x358 | 0x200 | Initialized Data, Read Write |
| .rsrc | 0x3000 | 0x5A0 | 0x600 | Initialized Data, Read Only |
| .reloc | 0x4000 | 0x36A | 0x400 | Discardable, Read Only |

## Exports (NPAPI Plugin Interface)
| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | 0x11F5 | `NP_GetEntryPoints` |
| 2 | 1 | 0x11F5 | `NP_Initialize` |
| 3 | 2 | 0x1200 | `NP_Shutdown` |

All three are the minimum required NPAPI plugin exports:
- **NP_Initialize**: Called by the browser when loading the plugin. Returns `NPERR_NO_ERROR` (0) on success. At 0x11F5 both NP_GetEntryPoints and NP_Initialize resolve to the same thunk, suggesting a shared stub.
- **NP_GetEntryPoints**: Returns the plugin's function table to the browser.
- **NP_Shutdown**: Cleanup on unload.

The export name strings are stored at the end of the data section (0x1EE0-0x1F1C):
```
NPWLPG.dll\0NP_GetEntryPoints\0NP_Initialize\0NP_Shutdown
```

## Imports
### MSVCR110.dll (Visual C++ 2012 Runtime)
`_unlock`, `_calloc_crt`, `__dllonexit`, `_onexit`, `_lock`, `_except_handler4_common`, `_crt_debugger_hook`, `__crtUnhandledException`, `__crtTerminateProcess`, `_initterm_e`, `_initterm`, `_malloc_crt`, `free`, `_amsg_exit`, `__CppXcptFilter`, `__clean_type_info_names_internal`, `operator delete`

### KERNEL32.dll
`IsDebuggerPresent`, `GetTickCount64`, `GetSystemTimeAsFileTime`, `GetCurrentThreadId`, `QueryPerformanceCounter`, `DecodePointer`, `EncodePointer`, `DisableThreadLibraryCalls`, `IsProcessorFeaturePresent`

## Key Internal Functions

### PE Validation (0x1700)
Validates a loaded module as a valid PE32 executable:
1. Checks for MZ signature (0x5A4D) at offset 0
2. Reads `e_lfanew` at offset 0x3C
3. Verifies PE signature (0x4550)
4. Checks Optional Header magic for PE32 (0x10B)

Used to validate the browser process before loading NPAPI entry points.

### Initialization (0x1777)
SEH-protected initialization that:
1. Uses a critical section/lock object at `10002350h`
2. Calls through IAT entries (likely `InitializeCriticalSection` + `EncodePointer`)
3. Allocates and initializes internal state
4. Calls PE validation to verify the host browser

### NPAPI Thunks (0x1924, 0x1930)
Simple jump-through-indirect-address stubs:
```asm
10001924: jmp dword ptr ds:[10001050h]  ; -> NP_GetEntryPoints
10001930: jmp dword ptr ds:[1000104Ch]  ; -> NP_Initialize  
10001935: ...NP_Shutdown stub...
```

### Security Cookie (0x183B)
Standard MSVC GS cookie initialization using `GetTickCount64`, `GetCurrentThreadId`, `QueryPerformanceCounter`, and `GetSystemTimeAsFileTime` for entropy.

## Architecture
```
Browser (Firefox/IE)
  |
  v
NPWLPG.dll (NPAPI plugin)
  |
  |-- NP_Initialize / NP_GetEntryPoints / NP_Shutdown
  |
  v
Loads WLPG COM objects via CoCreateInstance
  |
  v
Windows Live Photo Gallery backend
```

## Key Observations
1. **Minimal NPAPI stub**: The actual plugin logic is extremely thin - it's essentially a bridge that delegates to WLPG COM components
2. **PE validation**: The plugin validates the host process PE before proceeding - a security measure
3. **No `NP_GetMimeDescription`**: Unlike many NPAPI plugins, this doesn't export MIME type registration - the plugin is likely registered via Windows registry/plugin scanning
4. **Very small**: Only 4KB of code, 2KB of data - confirms it's a thin shim
5. **MSVC 2012**: Built with Visual Studio 2012 (MSVC 11.0)
6. **No delay imports**: No delay-loaded DLLs
7. **No COM descriptor**: The DLL itself does not register as a COM server (no DllGetClassObject etc.)
8. **Debug hooks present**: `_crt_debugger_hook` import suggests debug build checks or error reporting
