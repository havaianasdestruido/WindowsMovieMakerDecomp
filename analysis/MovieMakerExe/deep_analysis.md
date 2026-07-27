# MovieMaker.exe — Deep Reverse Engineering Analysis

## Binary Overview

| Property | Value |
|----------|-------|
| File | `undecomp\Photo Gallery\MovieMaker.exe` |
| Size | 54KB (115,200 bytes on disk — 0x1D000 virtual) |
| Architecture | x86 (PE32, 0x14C) |
| Timestamp | Tue Apr 1 01:27:07 2014 (0x533A401B) |
| Linker | MSVC 11.00 (Visual Studio 2012) |
| Entry Point | 0x0040152F |
| Image Base | 0x00400000 |
| Subsystem | Windows GUI (2) |
| PDB | `MovieMaker.pdb` GUID `{47558454-9C62-4123-96E9-91A66E8F4D87}` |
| OS Version | 6.02 (Windows 8 targeting) |
| Subsystem Version | 6.00 (Windows Vista) |

---

## Section Layout

| Section | Virt Addr | Virt Size | Raw Size | Flags |
|---------|-----------|-----------|----------|-------|
| `.text` | 0x1000 | 0x12F0 | 0x1400 | Code, Execute Read |
| `.data` | 0x3000 | 0x398 | 0x200 | Initialized Data, Read Write |
| `.rsrc` | 0x4000 | 0x17D60 | 0x17E00 | Initialized Data, Read Only |
| `.reloc` | 0x1C000 | 0x57A | 0x600 | Initialized Data, Discardable, Read Only |

The `.rsrc` section is by far the largest (97KB), consuming ~80% of the binary. The actual code section is only ~5KB (0x12F0 bytes), confirming this is a **thin launcher/stub**.

---

## Startup Sequence — Full Disassembly Walkthrough

### Entry Point (0x0040152F)

```asm
0040152F: E8 E2 01 00 00     call  00401716       ; __security_init_cookie
00401534: E9 48 FE FF FF     jmp   00401381       ; __tmainCRTStartup
```

The entry point is just two instructions: call the security cookie initializer, then tail-jump to the CRT startup function.

### __security_init_cookie (0x00401716)

Generates the GS stack cookie value by XOR-ing:
- `GetSystemTimeAsFileTime` low/high parts
- `GetCurrentThreadId`
- `GetCurrentProcessId` (via `QueryPerformanceCounter`)

Stores the result at `__security_cookie` (0x00403018) and its complement at `__security_cookie_complement` (0x0040301C). If the default cookie value `0xBB40E64E` is detected (unused/uninitialized), a more random value is forced via `0x4711` OR pattern.

### __tmainCRTStartup (0x00401381)

This is the main CRT initialization function — the true "main" of the binary:

```
00401381: push  14h                  ; SEH frame size
00401383: push  401DE0h              ; SEH handler table
00401388: call  __ehahahahahahah     ; __except_handler4 prolog (0x00401950)
0040138D: and   [ebp-1Ch], 0         ; Zero out exception flag

; --- Detect /GS cookie support ---
00401391: call  IsProcessorFeaturePresent (wrapper at 0x004015B8)

; --- Thread lock: acquire __lock(0) via lock cmpxchg ---
0040139D: mov   eax, fs:[18h]        ; TEB.Self
004013A3: mov   edx, [eax+4]         ; TEB.ThreadId
004013A6-004013C6:
    lock cmpxchg [403384h], ecx      ; Atomic acquire of lock word
    ; First thread wins, others spin

; --- Check if _setdefaultprecision needed ---
004013CF: push  1Fh
004013D1: call  __setFPPrecision      ; wrapper → __setFPPrecision

; --- Call __setdefaultprecision ---
004013E8: push  4010D8h              ; Rdata pointer
004013ED: push  4010C8h              ; Rdata pointer
004013F2: call  __lock(0)            ; acquire

; --- Parse PDB/debug info from PE header ---
00401468: mov   eax, [40108Ch]       ; Load PE header pointer
00401475: ; Walk wide-char string table in PE, check for valid chars
; Detect if debugging: if IsDebuggerPresent → set flag

; --- Call __except_handler4 registered function ---
004014A2: push  400000h              ; Image base
004014A7: call  00401240             ; Validate PE/MZ header

; --- Call wWinMainCRTStartup (0x00401331) ---
; This sets up __wgetmainargs → wWinMain entry
```

### wWinMain / wmainCRTStartup (0x00401331)

```asm
00401331: push  4018E7h              ; __security_init_cookie
00401336: call  004018A8             ; __setdefaultprecision wrapper
0040133B: mov   eax, [403050h]       ; Load wWinMainCRTStartup data
00401347: push  [40304Ch]            ; __wcmdln (wide command line)
00401352: push  403038h              ; __wenvptr
00401357: push  403034h              ; __wshowcmd
0040135C: push  403030h              ; __wcmdshow
00401361: call  __wgetmainargs       ; CRT: parse wide command-line args
00401367: add   esp, 14h
0040136A: mov   [40303Ch], eax       ; Store result
0040136F: test  eax, eax
00401371: jns   40137B              ; If success, skip error
00401373: push  8                    ; Error code 8
00401375: call  _amsg_exit           ; Abort with fatal error
```

### The __except_handler4 Chain (0x004019AE)

The binary uses MSVC's `/GS` structured exception handling v4:

```asm
004019AE: push  ebp
004019AF: mov   ebp, esp
004019B1: push  [ebp+14h]            ; Exception record
004019B4: push  [ebp+10h]            ; Establisher frame
004019B7: push  [ebp+0Ch]            ; Context record
004019BA: push  [ebp+8]              ; Dispatcher context
004019BD: push  40153Eh              ; Handler function address
004019C2: push  403018h              ; __security_cookie
004019C7: call  00401B6A             ; _except_handler4_common (MSVCR110)
```

### The __security_check_cookie (0x0040153E)

```asm
0040153E: cmp   ecx, [__security_cookie]  ; Compare GS cookie
00401544: jne   00401548                   ; Cookie corrupted!
00401546: rep ret                           ; Normal return
00401548: jmp   __GSHandlerCheck_Epilog    ; Fast-fail / abort
```

### PE Validation (0x00401273)

The binary validates its own PE header before proceeding:

```asm
00401273: mov   eax, 5A4Dh         ; "MZ" magic
00401278: cmp   word ptr [400000h], ax
0040127F: je    00401285
00401281: xor   eax, eax           ; Return FALSE if no MZ
00401283: jmp   004012B9

00401285: mov   ecx, [40003Ch]     ; e_lfanew offset
0040128B: cmp   dword ptr [ecx+400000h], 4550h  ; "PE\0\0"
00401295: jne   00401281           ; Not a valid PE

00401297: mov   eax, 10Bh          ; PE32 magic
0040129C: cmp   word ptr [ecx+400018h], ax
004012A3: jne   00401281           ; Not PE32

004012A5: xor   eax, eax
004012A7: cmp   dword ptr [ecx+400074h], 0Eh  ; NumberOfRvaAndSizes
004012AE: jbe   004012B9           ; Too few data directories
004012B0: cmp   dword ptr [ecx+4000E8h], eax   ; Check security dir
004012B6: setne al                 ; Has certificate table?
```

### Module Validation / WER Registration (0x00401620)

```asm
00401620: push  ebp
00401623: push  -2                  ; EXCEPTION_CHAIN_END
00401625: push  401E00h             ; Unwind data
0040162A: push  4019AEh             ; Handler
0040162F: mov   eax, fs:[0]         ; Current SEH chain
00401635: push  eax
00401636: sub   esp, 8              ; Local variables
0040163C: mov   eax, [__security_cookie]
00401641: xor   [ebp-8], eax        ; Encode local with cookie
00401644: xor   eax, ebp
00401646: push  eax                 ; Set stack frame cookie
; ...
; Validates MZ/PE of caller's module
; Checks if section is executable
00401685: mov   eax, [eax+24h]      ; Section characteristics
00401688: shr   eax, 1Fh            ; Check IMAGE_SCN_MEM_EXECUTE bit
0040168B: not   eax
0040168D: and   eax, 1              ; Return 1 if executable section
```

### Heap Initialization (0x0040190C)

```asm
0040190C: push  esi
0040190D: push  30000h              ; dwMaximumSize = 192KB
00401912: push  10000h              ; dwInitialSize = 64KB
00401917: xor   esi, esi
00401919: push  esi                 ; lpHeapAttributes = NULL
0040191A: call  HeapCreate          ; Create default process heap
0040191F: add   esp, 0Ch
00401922: test  eax, eax
00401924: jne   00401928            ; Success
00401926: pop   esi
00401927: ret                       ; Heap create failed

00401928: push  esi                 ; 0 — debug flag
00401929: push  esi                 ; 0 — options
0040192A: push  esi                 ; 0 — initial bytes
0040192B: push  esi                 ; 0 — initial commits
0040192C: push  esi                 ; 0 — initial reserve
0040192D: call  HeapSetInformation   ; Enable heap checking
```

### Exception Filter (0x00401552)

Handles C++ exceptions with magic code `0xE06D7363` (`.msc` = Microsoft C++ Exception):

```asm
00401552: push  ebp
00401555: mov   eax, [eax]
0040155A: cmp   dword ptr [eax], 0E06D7363h   ; C++ exception magic
00401560: jne   00401587                       ; Not C++ exception
00401562: cmp   dword ptr [eax+10h], 3         ; ExceptionType == 3
00401566: jne   00401587
0040156B: cmp   eax, 19930520h                 ; MSVC exception version
00401570: je    0040158D                       ; Handle it
; Also handles 19930521h, 19930522h, 19940000h
0040158D: call  00401B16                       ; __crtUnhandledException
```

### Termination Handler (0x004019D6)

```asm
004019D6: push  ebp
004019D9: call  IsDebuggerPresent              ; via IAT
004019E1: mov   [403374h], eax                 ; Store debugger state
004019E6: call  __crtSetUnhandledExceptionFilter (0x00401B76)
004019EE: call  __crtTerminateProcess (0x00401B82)
004019F3: cmp   dword ptr [403374h], 0
004019FA: pop   ecx
004019FC: jne   00401A06                       ; Debugging — skip
004019FE: push  1
00401A00: call  __crtSetUnhandledExceptionFilter ; Reset handler
00401A06: push  0C0000409h                     ; STATUS_STACK_BUFFER_OVERRUN
00401A0B: call  __crtTerminateProcess
```

### Full Unhandled Exception Handler (0x00401A18)

This is the comprehensive crash reporter — it captures a full register/context dump:

```asm
00401A18: push  ebp
00401A1B: sub   esp, 324h                     ; 804 bytes of local storage
00401A21: push  17h                           ; ProcessorFeatureXMMI
00401A23: call  IsProcessorFeaturePresent     ; Check SSE2 support
00401A28: test  eax, eax
00401A2A: je    00401A31
00401A2C: push  2
00401A2E: pop   ecx
00401A2F: int   29h                           ; Fast-fail if SSE2 present

; Save full CPU context:
00401A31: mov   [403158h], eax                ; EAX
00401A36: mov   [403154h], ecx                ; ECX
00401A3C: mov   [403150h], edx                ; EDX
00401A42: mov   [40314Ch], ebx                ; EBX
00401A48: mov   [403148h], esi                ; ESI
00401A4E: mov   [403144h], edi                ; EDI
00401A54: mov   [403170h], ss                 ; SS
00401A5B: mov   [403164h], cs                 ; CS
00401A62: mov   [403140h], ds                 ; DS
00401A69: mov   [40313Ch], es                 ; ES
00401A70: mov   [403138h], fs                 ; FS
00401A77: mov   [403134h], gs                 ; GS
00401A7E: pushfd
00401A7F: pop   [403168h]                     ; EFLAGS
00401A85: mov   eax, [ebp]                    ; Saved EBP
00401A88: mov   [40315Ch], eax                ; Frame pointer
00401A8D: mov   eax, [ebp+4]                  ; Return address
00401A90: mov   [403160h], eax                ; Return address
00401A95: lea   eax, [ebp+8]                  ; Stack pointer
00401A98: mov   [40316Ch], eax                ; ESP at entry

; Configure crash report structures:
00401AAD: mov   [403064h], eax                ; Context record
00401AB7: mov   [403058h], 0C0000409h         ; STATUS_STACK_BUFFER_OVERRUN
00401AC1: mov   [40305Ch], 1                  ; ExceptionRecord.NumberParameters
00401ACB: mov   [403068h], 1                  ; ExceptionRecord.ExceptionFlags
00401ADB: mov   [40306Ch], 2                  ; EXCEPTION_NONCONTINUABLE

; Invoke __crtUnhandledException:
00401B05: push  401118h                       ; Exception record pointer
00401B0A: call  004019D6                       ; Unhandled exception filter
00401B0F: leave
00401B10: ret
```

---

## Delay-Load Import: MovieMakerCore.dll

The critical design pattern — **MovieMakerCore.dll is delay-loaded**, not statically linked:

```
Characteristics:       0x00000001 (RVA-based)
Address of HMODULE:    0x00403378 (stores loaded DLL handle)
IAT:                   0x00403020
Bound Import Table:    0x00401E98
Unload Import Table:   0x00000000 (no unload support)
Delayed Import Name:   MovieMakerMain (ordinal 0, name-based)
```

### MovieMakerMain Thunk (at 0x00401DD6)

```asm
00401DD6: jmp dword ptr ds:[00401018h]   ; IAT entry → delay-load resolver
```

When `MovieMakerMain` is first called, the delay-load helper (`__delayLoadHelper2`) will:
1. Call `LoadLibraryA("MovieMakerCore.dll")`
2. Call `GetProcAddress(handle, "MovieMakerMain")`
3. Patch the IAT slot at `[00401018h]`
4. Jump to the resolved address

---

## Static Imports

### KERNEL32.dll (14 functions)

| Function | Purpose |
|----------|---------|
| `SetDllDirectoryW` | Restrict DLL search path (security) |
| `GetProcAddress` | Dynamic symbol resolution |
| `FreeLibrary` | Unload DLLs |
| `InterlockedExchange` | Thread-safe variable swap |
| `GetLastError` | Win32 error code |
| `RaiseException` | Trigger structured exception |
| `IsProcessorFeaturePresent` | CPU capability detection |
| `IsDebuggerPresent` | Debug detection |
| `DecodePointer` | Pointer deobfuscation (safe SEH) |
| `GetTickCount64` | High-resolution timer |
| `GetSystemTimeAsFileTime` | Entropy for stack cookie |
| `GetCurrentThreadId` | Thread identification |
| `QueryPerformanceCounter` | High-perf counter for cookie |
| `EncodePointer` | Pointer obfuscation (safe SEH) |
| `LoadLibraryExA` | Load DLL with flags |

### MSVCR110.dll (28 functions — Visual C++ 2012 Runtime)

Key CRT functions: `_except_handler4_common`, `_XcptFilter`, `_initterm`, `_initterm_e`, `_amsg_exit`, `__wgetmainargs`, `_wcmdln`, `_fmode`, `_commode`, `exit`, `_exit`, `_cexit`, `_configthreadlocale`, `_controlfp_s`, `_calloc_crt`, `_lock`, `_unlock`, `__dllonexit`, `_onexit`, `_invoke_watson`, `_crt_debugger_hook`, `_crtGetShowWindowMode`, `__crtSetUnhandledExceptionFilter`, `__crtUnhandledException`, `__crtTerminateProcess`, `__setusermatherr`, `__set_app_type`, `?terminate@@YAXXZ`

### WLXPhotoBase.dll (1 function)

| Function | Mangled Name |
|----------|-------------|
| `Delete` | `?Delete@BasePrivate@@YAXPAX@Z` |

This is a Windows Live / Photo Gallery base library. The `Delete@BasePrivate` function is a memory deallocator — the only direct dependency on the WLX runtime.

---

## Security Features Analysis

### DEP (Data Execution Prevention)

**NX Compatible** — DLL characteristic bit 0x0100 is set. All data pages (`.data`, `.rsrc`) are non-executable. Code pages are Execute-Read only. DEP is fully enabled.

### ASLR (Address Space Layout Randomization)

**Dynamic Base** — DLL characteristic bit 0x0040 is set. The OS will randomize the image base at load time. The default image base of `0x00400000` is a relocation hint only.

Base Relocation Directory present at RVA 0x1C000 (0x178 bytes), confirming the binary supports ASLR relocation fixups.

### SEH (Structured Exception Handling)

**Full SEH v4 with `/GS` cookies**:
- Safe Exception Handler Table at RVA 0x1200, 1 entry at address 0x004019AE
- Uses `_except_handler4_common` from MSVCR110.dll
- 0x004019AE: Full handler that captures context, calls `__crtUnhandledException`
- Stack cookie validation via `__security_check_cookie` (0x0040153E)
- Pointer encoding/decoding via `EncodePointer`/`DecodePointer` for handler table obfuscation

### Stack Cookie (/GS)

**Enabled**: Security cookie at address 0x00403018.
- Initialized in `__security_init_cookie` (0x00401716) using `GetSystemTimeAsFileTime` + `GetCurrentThreadId` + `QueryPerformanceCounter`
- Complement stored at 0x0040301C for fast validation
- Verified before every function return via `__security_check_cookie`

### CFG (Control Flow Guard)

**Not present**. No `guard CF function table` or `guard address function table` entries in the load configuration. This is expected for a 2014 MSVC 11.0 (VS2012) build — CFG was introduced in VS2015.

### Other Security

- **Terminal Server Aware** — DLL characteristic 0x2000 set
- **SetDllDirectoryW("")** — Called early to remove CWD from DLL search path (DLL hijacking prevention)
- **`/sdl` (Security Development Lifecycle) checks**: Debug feature directory shows `/sdl=0` — not enabled, but `/GS=27` shows stack buffer overrun protection is active
- **Safe exception handler count**: 1 (minimum for single-exit-point design)
- **Int 29h fast-fail**: Used as an abort mechanism in the crash handler when SSE2 is available

---

## Application Manifest (Embedded in .rsrc)

```xml
<?xml version='1.0' encoding='UTF-8' standalone='yes'?>
<!-- Copyright (c) Microsoft Corporation -->
<assembly xmlns='urn:schemas-microsoft-com:asm.v1'
          manifestVersion='1.0'
          xmlns:asmv3="urn:schemas-microsoft-com:asm.v3">
  <assemblyIdentity
      version="5.1.0.0"
      processorArchitecture="x86"
      name="Microsoft.Windows.personalMedia.MovieMaker"
      type="win32" />
  <description>Microsoft Windows Live Movie Maker</description>

  <!-- DPI Awareness: Per-Monitor (legacy) -->
  <asmv3:application>
    <asmv3:windowsSettings
        xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings">
      <dpiAware>true</dpiAware>
    </asmv3:windowsSettings>
  </asmv3:application>

  <!-- Common Controls v6 dependency -->
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

  <!-- Execution Level: asInvoker (no elevation required) -->
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v2">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel
            level="asInvoker"
            uiAccess="false"/>
      </requestedPrivileges>
    </security>
  </trustInfo>
</assembly>
```

### Manifest Analysis

| Property | Value | Significance |
|----------|-------|-------------|
| `assemblyIdentity` | `Microsoft.Windows.personalMedia.MovieMaker` v5.1.0.0 | Windows Live Movie Maker identity |
| `processorArchitecture` | x86 | 32-bit only |
| `dpiAware` | `true` | System DPI awareness (legacy, not Per-Monitor v2) |
| `requestedExecutionLevel` | `asInvoker` | No UAC elevation required |
| `uiAccess` | `false` | No UI privilege isolation |
| Common Controls | v6.0.0.0 | Visual styles / themed controls enabled |
| **Missing** | `supportedOS` GUID | No Windows 8/10/11 compatibility declaration |
| **Missing** | `longPathAware` | No long path support |
| **Missing** | `heapType` | No segment heap declaration |

---

## Version Information

| Field | Value |
|-------|-------|
| `CompanyName` | Microsoft Corporation |
| `FileDescription` | Movie Maker |
| `FileVersion` | 16.4.3528.0331_ship.client.main.w5m4 (ship) |
| `InternalName` | Movie Maker |
| `LegalCopyright` | 2012 Microsoft Corporation. All rights reserved. |
| `OriginalFilename` | MovieMaker.EXE |
| `ProductName` | Movie Maker |
| `ProductVersion` | 16.4.3528.0331 |

Build identifier breakdown: `16.4` = Windows Essentials 2013 wave, `3528` = build number, `0331` = March 31 date stamp, `_ship.client.main.w5m4` = shipping client build of the Windows Live "Wave 5 Movie 4" component.

---

## String Extraction Summary

### DLL Names (loaded at runtime)

| DLL | Load Method | Purpose |
|-----|------------|---------|
| `KERNEL32.dll` | Static | Win32 core API |
| `MSVCR110.dll` | Static | C runtime (VS2012) |
| `WLXPhotoBase.dll` | Static | Windows Live Photo base |
| `MovieMakerCore.dll` | **Delay-load** | Actual movie-making engine |

### CRT / Internal Strings

| Address | String | Purpose |
|---------|--------|---------|
| `0x1120` | `MovieMakerCore.dll` | Delay-load DLL name |
| `0x1186` | `MovieMakerMain` | Exported entry point in core DLL |
| `0x1197` | `MovieMaker.pdb` | Debug symbol file reference |
| `0x15AA` | `?Delete@BasePrivate@@YAXPAX@Z` | WLXPhotoBase memory free |
| `0x1CB0` | Full XML manifest | Application manifest |

### Error / Debug Strings

| Address | String | Context |
|---------|--------|---------|
| `0x21AA` | `?Delete@BasePrivate@@YAXPAX@Z` | Delete operator override |
| `0x20F5` | `_unlock` | CRT lock release |
| `0x20EC` | `_lock` | CRT lock acquire |
| `0x20E7` | `_initterm_e` | CRT init with error handling |
| `0x20E2` | `_initterm` | CRT initialization table walk |
| `0x20B9` | `__crtSetUnhandledExceptionFilter` | WER integration |
| `0x20C2` | `__crtUnhandledException` | Crash reporting |
| `0x20C9` | `__crtTerminateProcess` | Fatal termination |

### Certificate Strings (Authenticode signature)

The `.rsrc` section contains two complete Authenticode certificate chains:

1. **Code Signing Certificate**: Microsoft Corporation, signed by "Microsoft Code Signing PCA 2011", issued by "Microsoft Root Certificate Authority 2011"
2. **Timestamp Certificate**: "Microsoft Time-Stamp PCA 2010", issued by "Microsoft Root Certificate Authority 2010", with nCipher DSE ESN:B8EC-30A4-71441

---

## Function Map

| Address | Function | Description |
|---------|----------|-------------|
| `0x00401000` | IAT start | Import Address Table |
| `0x00401118` | Exception record | `__crtUnhandledException` data |
| `0x00401197` | PDB string | `MovieMaker.pdb` |
| `0x0040122F` | PE validator | Validates MZ/PE/PE32 header |
| `0x00401240` | Image validation | Calls PE validator + stores result |
| `0x00401273` | `IsPE32Valid` | Checks MZ, PE\0\0, PE32, security dir |
| `0x00401331` | `wWinMainCRTStartup` | CRT wide-main entry, calls `__wgetmainargs` |
| `0x00401381` | `__tmainCRTStartup` | Main CRT initialization + exception setup |
| `0x0040152F` | **Entry Point** | Calls `__security_init_cookie` → jumps to `__tmainCRTStartup` |
| `0x0040153E` | `__security_check_cookie` | GS cookie verification |
| `0x00401552` | C++ exception filter | Handles `0xE06D7363` (`.msc`) exceptions |
| `0x00401598` | Exception registration | Registers `__except_handler4` |
| `0x004015AC` | `__delayLoadHelper` thunk | Jump to delay-load IAT resolver |
| `0x004015B8` | `IsProcessorFeaturePresent` thunk | Via IAT |
| `0x004015C4` | `_amsg_exit` thunk | Via IAT |
| `0x004015D0` | `FindRVAInSection` | Walks PE sections to map RVA to section |
| `0x00401620` | `ValidateModule` | MZ/PE check + executable section test |
| `0x004016E0` | `IsPEModule` | Validates MZ signature + PE32 header |
| `0x00401716` | `__security_init_cookie` | Initializes GS stack cookie |
| `0x004017B5` | `__noop_init` | Returns 0 (stub/no-op) |
| `0x004017BC` | `__initialize_heap` | `VirtualAlloc` + `HeapSetInformation` |
| `0x004017FE` | `__dll_init` | DLL initialization with SEH |
| `0x004018A8` | `__setdefaultprecision` | FPU precision setup |
| `0x004018C2` | `__initterm_e` | Walk `_initterm_e` table (with error checking) |
| `0x004018E7` | `__initterm` | Walk `_initterm` table |
| `0x0040190C` | `__heap_init` | `HeapCreate` + `HeapSetInformation` |
| `0x00401938` | `EncodePointer` thunk | Via IAT |
| `0x00401944` | `DecodePointer` thunk | Via IAT |
| `0x00401950` | `__ehprolog4` | SEH v4 prolog (cookie-protected) |
| `0x00401995` | `__ehepilog4` | SEH v4 epilog |
| `0x004019AE` | Safe SEH handler | Registered handler for all frames |
| `0x004019D6` | `__crt_exit_process` | `IsDebuggerPresent` + `TerminateProcess` |
| `0x00401A18` | `__unhandled_exception_filter` | Full crash context dump + WER |
| `0x00401B16` | `__crtUnhandledException` thunk | Via IAT |
| `0x00401B22` | `__crtTerminateProcess` thunk | Via IAT |
| `0x00401B2E` | `EnterCriticalSection` thunk | Via IAT |
| `0x00401B3A` | `LeaveCriticalSection` thunk | Via IAT |
| `0x00401B46` | `RaiseException` thunk | Via IAT |
| `0x00401B52` | `HeapSetInformation` thunk | Via IAT |
| `0x00401B5E` | `HeapCreate` thunk | Via IAT |
| `0x00401B6A` | `_except_handler4_common` thunk | Via IAT |
| `0x00401B76` | `__crtSetUnhandledExceptionFilter` thunk | Via IAT |
| `0x00401B82` | `__crtTerminateProcess` thunk | Via IAT |
| `0x00401B8E` | `RtlUnhandledExceptionFilter` thunk | Via IAT |
| `0x00401DD6` | `MovieMakerMain` | Delay-load thunk → resolves to MovieMakerCore.dll export |

---

## Architecture Summary

```
┌──────────────────────────────────────────────────┐
│                  MovieMaker.exe                   │
│                  54KB Thin Launcher               │
├──────────────────────────────────────────────────┤
│ Entry Point (0x40152F)                           │
│   ├─ __security_init_cookie (GS cookie init)     │
│   └─ __tmainCRTStartup                           │
│       ├─ SEH v4 frame setup (4 bytes cookie)     │
│       ├─ IsDebuggerPresent check                 │
│       ├─ Thread lock (lock cmpxchg)              │
│       ├─ __setdefaultprecision (FPU)             │
│       ├─ __wgetmainargs (CRT arg parsing)        │
│       ├─ __initterm / __initterm_e               │
│       ├─ HeapCreate (64KB init, 192KB max)       │
│       └─ HeapSetInformation                      │
├──────────────────────────────────────────────────┤
│  Static Links:                                   │
│    KERNEL32.dll (14 funcs)                       │
│    MSVCR110.dll  (28 funcs)                      │
│    WLXPhotoBase.dll (1 func: Delete)             │
│                                                  │
│  Delay-Load:                                     │
│    MovieMakerCore.dll → MovieMakerMain()         │
├──────────────────────────────────────────────────┤
│  Resources (97KB / 80% of binary):               │
│    ├─ Application manifest (DPI, asInvoker)      │
│    ├─ PNG icons/images                           │
│    ├─ Version info (16.4.3528.0331)              │
│    └─ Authenticode certificate (dual chain)      │
└──────────────────────────────────────────────────┘
```

---

## Key Findings

1. **Ultra-thin launcher pattern**: Only 5KB of actual code (.text section). The 97KB `.rsrc` section is dominated by resources and the Authenticode signature, not code.

2. **Single-purpose stub**: The entire code exists to: (a) initialize the CRT, (b) set up exception handling and security cookies, (c) delay-load `MovieMakerCore.dll`, and (d) call `MovieMakerMain()`.

3. **No wWinMain**: Despite being a GUI application, there is no `wWinMain` function in this binary. The CRT startup directly triggers the MovieMakerCore delay-load, which contains the actual application logic.

4. **Security hardening**: ASLR + DEP + GS cookies + SEH v4 + `SetDllDirectoryW("")` + `EncodePointer`/`DecodePointer`. Missing: CFG (not available in VS2012).

5. **No supportedOS manifest**: The manifest lacks `<supportedOS>` GUIDs, meaning Windows will run it in compatibility mode (Vista-level behavior by default).

6. **Single WLX dependency**: Only one function from the Windows Live ecosystem (`BasePrivate::Delete`), confirming this is a minimal shim.

7. **Crash diagnostics**: The binary includes a comprehensive crash handler that dumps all CPU registers, segment registers, EFLAGS, and stack pointers before invoking `__crtUnhandledException` (WER integration).

8. **Dual Authenticode signing**: Signed with both Microsoft Code Signing PCA 2011 and Microsoft Time-Stamp PCA 2010, with timestamp from nCipher DSE hardware security module.
