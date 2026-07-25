# MovieMaker.exe - Static Binary Analysis

## Overview

| Property | Value |
|----------|-------|
| **File** | MovieMaker.exe |
| **File Type** | PE32 Executable (x86) |
| **Image Base** | 0x00400000 |
| **Entry Point** | 0x0040152F |
| **Subsystem** | Windows GUI (2) |
| **Linker Version** | 11.00 (MSVC) |
| **OS Version** | 6.02 (Windows 8) |
| **Image Version** | 6.02 |
| **Subsystem Version** | 6.00 |
| **Size of Image** | 0x1D000 (118,784 bytes) |
| **Size of Headers** | 0x400 (1,024 bytes) |
| **Checksum** | 0x2B644 |
| **Time Date Stamp** | Tue Apr 1 01:27:07 2014 (0x533A401B) |
| **Machine** | 0x14C (x86 / Intel 386) |
| **Characteristics** | 0x122 - Executable, Large (>2GB) addresses, 32-bit word machine |
| **Stack Reserve** | 0x40000 (256 KB) |
| **Stack Commit** | 0x2000 (8 KB) |
| **Heap Reserve** | 0x100000 (1 MB) |
| **Heap Commit** | 0x1000 (4 KB) |

### DLL Characteristics (0x8140)
- Dynamic Base (ASLR) ✓
- NX Compatible (DEP) ✓
- Terminal Server Aware ✓
- High Entropy VA (64-bit ASLR) ✗ (not set)
- Force Integrity Check ✗
- No SEH ✗ (SEH is present)
- NO_BIND ✗
- AppContainer ✗
- **Not** a WDM Driver
- Guard CF ✗

### Assembly Identity (from Manifest)
- **Name**: `Microsoft.Windows.personalMedia.MovieMaker`
- **Version**: 5.1.0.0
- **Architecture**: x86
- **Description**: Microsoft Windows Live Movie Maker
- **Execution Level**: asInvoker (no elevation required)
- **DPI Aware**: Yes
- **Common Controls**: 6.0.0.0
- **Copyright**: (c) Microsoft Corporation

---

## Section Layout

| Section | Virtual Addr | Virtual Size | Raw Size | Flags | Description |
|---------|-------------|-------------|----------|-------|-------------|
| `.text` | 0x1000 | 0x12F0 | 0x1400 | Code, Execute Read | Code section (very small - 4,848 bytes) |
| `.data` | 0x3000 | 0x398 | 0x200 | Initialized Data, Read Write | Read-write data |
| `.rsrc` | 0x4000 | 0x17D60 | 0x17E00 | Initialized Data, Read Only | Resources (97,632 bytes - largest section!) |
| `.reloc` | 0x1C000 | 0x57A | 0x600 | Initialized Data, Discardable, Read Only | Base relocations |

**Key observation**: The `.text` section is extremely small (only ~4.8KB of actual code). The `.rsrc` section is massive (~97KB), containing embedded images/resources. This means MovieMaker.exe is essentially a thin launcher/stub that delegates all real work to DLLs.

---

## Exported Functions

**None.** This is an executable, not a DLL. The export directory is empty.

However, the binary references one key function name:
- **`MovieMakerMain`** - Imported from `MovieMakerCore.dll` (delay-loaded)

---

## Imports

### KERNEL32.dll (15 functions)

| Function | Purpose |
|----------|---------|
| `SetDllDirectoryW` | Sets DLL search path (security: prevents DLL hijacking) |
| `GetProcAddress` | Runtime function resolution (used for delay-loading) |
| `FreeLibrary` | Unload DLLs |
| `InterlockedExchange` | Thread-safe variable exchange |
| `GetLastError` | Get last error code |
| `RaiseException` | Raise structured exception |
| `IsProcessorFeaturePresent` | CPU feature detection |
| `IsDebuggerPresent` | Debug detection |
| `DecodePointer` | Pointer obfuscation decode (security) |
| `GetTickCount64` | 64-bit millisecond timer |
| `GetSystemTimeAsFileTime` | High-resolution time |
| `GetCurrentThreadId` | Thread ID |
| `QueryPerformanceCounter` | High-resolution performance counter |
| `EncodePointer` | Pointer obfuscation encode (security) |
| `LoadLibraryExA` | Load DLL with flags (ANSI) |

### MSVCR110.dll (26 functions)

This is the **Visual C++ 2012 Runtime** (MSVC 11.0).

| Function | Purpose |
|----------|---------|
| `_commode` | Console mode flag |
| `?terminate@@YAXXZ` | C++ terminate handler |
| `__crtSetUnhandledExceptionFilter` | CRT exception filter |
| `_lock` | CRT internal lock |
| `_unlock` | CRT internal unlock |
| `_calloc_crt` | CRT memory allocation |
| `__dllonexit` | DLL exit handler registration |
| `_onexit` | Exit handler registration |
| `_invoke_watson` | Watson crash reporting |
| `_fmode` | File mode flag |
| `_except_handler4_common` | SEH4 exception handler |
| `_crt_debugger_hook` | CRT debug hook |
| `__crtUnhandledException` | Unhandled exception in CRT |
| `__crtTerminateProcess` | Process termination |
| `_XcptFilter` | Exception filter |
| `__crtGetShowWindowMode` | Window show mode |
| `_controlfp_s` | Floating-point control |
| `_amsg_exit` | Abort with message |
| `__wgetmainargs` | Wide-char main args |
| `_wcmdln` | Wide-char command line |
| `_initterm` | C++ initialization table |
| `_initterm_e` | C++ initialization table (error) |
| `__setusermatherr` | User math error handler |
| `_configthreadlocale` | Thread locale config |
| `_cexit` | C exit cleanup |
| `_exit` / `exit` | Process exit |
| `__set_app_type` | Set app type (GUI) |

### WLXPhotoBase.dll (1 function)

| Function | Purpose |
|----------|---------|
| `?Delete@BasePrivate@@YAXPAX@Z` | Memory deallocation via BasePrivate::Delete |

This is a **C++ mangled name** that resolves to:
```cpp
void __cdecl BasePrivate::Delete(void*)
```
This is the base memory management function from WLXPhotoBase.dll.

### MovieMakerCore.dll (Delay-Loaded, 1 function)

| Function | Purpose |
|----------|---------|
| `MovieMakerMain` | **THE main entry point** - this is the real application logic |

**Delay-load characteristics**: `0x00000001` (reserved, standard delay load). The HMODULE is stored at `0x00403378`.

---

## Dependency Analysis

### Direct Dependencies (loaded at startup)
1. **KERNEL32.dll** - Windows kernel API (process, thread, memory, file I/O, security)
2. **MSVCR110.dll** - Visual C++ 2012 Runtime (CRT initialization, exception handling)
3. **WLXPhotoBase.dll** - Windows Live Photo Base library (memory management, base types)

### Delay-Loaded Dependencies (loaded on demand)
4. **MovieMakerCore.dll** - Core MovieMaker functionality

### Implications
- The EXE is a **minimal launcher stub** that:
  1. Initializes the CRT
  2. Calls `SetDllDirectoryW` for DLL search path security
  3. Loads `WLXPhotoBase.dll` (for `BasePrivate::Delete`)
  4. On first use, delay-loads `MovieMakerCore.dll` and calls `MovieMakerMain`
  5. All real UI, project management, rendering, and export logic lives in MovieMakerCore.dll

---

## Entry Point Analysis

**Entry Point RVA**: 0x152F (at file offset 0x192F in .text section)

The entry point is the standard MSVC CRT startup code (`__tmainCRTStartup` or similar). It:
1. Initializes the CRT (`_initterm` / `_initterm_e`)
2. Sets up exception handling (`_except_handler4_common`)
3. Calls `__wgetmainargs` to parse command line
4. Calls `__set_app_type(MAPI_APP)` for GUI application
5. Calls the actual WinMain / wWinMain
6. Calls `exit()` on return

The actual WinMain logic (in .text) is minimal and likely:
1. Calls `SetDllDirectoryW(L"")` to prevent DLL hijacking
2. Calls `MovieMakerMain()` from MovieMakerCore.dll

---

## Debug Information

| Type | Value |
|------|-------|
| **PDB GUID** | {47558454-9C62-4123-96E9-91A66E8F4D87} |
| **PDB Age** | 1 |
| **PDB Name** | MovieMaker.pdb |
| **Build Date** | Tue Apr 1 01:27:07 2014 |
| **Security Features** | /GS=27 (buffer security checks enabled), Pre-VC++ 11.00=0 |

---

## COM GUIDs Found

### In Binary Signatures (X.509 Certificates)
These are certificate authority identifiers, not application COM objects:

| GUID | Context |
|------|---------|
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Microsoft Code Signing PCA (OID in cert) |
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Microsoft Code Signing PCA 2011 (OID in cert) |

**No application-level COM CLSIDs or IIDs are embedded in this binary.** This is consistent with the EXE being a thin launcher - all COM objects are in the DLLs (MovieMakerCore.dll, WLXPhotoBase.dll, etc.).

---

## Code Signing Certificates (Embedded)

The binary contains 4 embedded Authenticode timestamp certificates:

1. **Microsoft Time-Stamp PCA** (2013-2014) - nCipher DSE ESN:B8EC-30A4-7144
2. **Microsoft Code Signing PCA** (2013-2014) - Microsoft Corporation
3. **Microsoft Code Signing PCA 2011** (2013-2014) - Microsoft Corporation
4. **Microsoft Time-Stamp PCA 2010** (2013-2014) - Microsoft Time-Stamp Service

**Signature timestamp**: 2014-04-01 04:34:18 UTC

---

## Meaningful Strings

### Product Identity
- `Microsoft.Windows.personalMedia.MovieMaker` (assembly identity)
- `Microsoft Windows Live Movie Maker` (description)
- `Movie Maker` (file description in VS_VERSION_INFO)
- `MovieMakerMain` (exported function name from MovieMakerCore.dll)

### DLL Names
- `MovieMakerCore.dll` (delay-loaded)
- `WLXPhotoBase.dll` (direct import)
- `KERNEL32.dll`
- `MSVCR110.dll`

### Debug Symbols
- `MovieMaker.pdb` (PDB name)

### Version Information (VS_VERSION_INFO)
- **CompanyName**: Microsoft Corporation
- **FileDescription**: Movie Maker
- **FileVersion**: 16.4.3528.0331_ship.client.main.w5m4 (ship)
- **InternalName**: Movie Maker
- **LegalCopyright**: (c) 2012 Microsoft Corporation. All rights reserved.
- **OriginalFilename**: MovieMaker.EXE
- **ProductName**: Movie Maker
- **ProductVersion**: 16.4.3528.0331
- **Translation**: 0x0409 (English - United States)

### Embedded PNG Image
- There is a PNG image embedded in the resources (detected by `IHDR`, `IDAT`, `IEND` markers) - likely the application icon or splash image.

### Build Configuration
- `16.4.3528.0331_ship.client.main.w5m4 (ship)` - Ship build, client main branch, codename w5m4

### File Paths (in certificate data)
- `http://crl.microsoft.com/pki/crl/products/MicrosoftTimeStampPCA.crl`
- `http://www.microsoft.com/pki/certs/MicrosoftTimeStampPCA.crt`
- `http://crl.microsoft.com/pki/crl/products/MicCodSigPCA_08-31-2010.crl`
- `http://crl.microsoft.com/pki/certs/MicCodSigPCA_08-31-2010.crt`
- `http://crl.microsoft.com/pki/crl/products/microsoftrootcert.crl`
- `http://www.microsoft.com/pki/certs/MicrosoftRootCert.crt`
- `http://www.microsoft.com/pkiops/crl/MicCodSigPCA2011_2011-07-08.crl`
- `http://www.microsoft.com/pkiops/certs/MicCodSigPCA2011_2011-07-08.crt`
- `http://crl.microsoft.com/pki/crl/products/MicRooCerAut2011_2011_03_22.crl`
- `http://www.microsoft.com/pki/certs/MicRooCerAut2011_2011_03_22.crt`
- `http://www.microsoft.com/pkiops/docs/primarycps.htm`
- `http://www.microsoft.com`
- `http://crl.microsoft.com/pki/crl/products/MicRooCerAut_2010-06-23.crl`
- `http://www.microsoft.com/pki/certs/MicRooCerAut_2010-06-23.crt`
- `http://www.microsoft.com/PKI/docs/CPS/default.htm`
- `http://crl.microsoft.com/pki/crl/products/MicTimStaPCA_2010-07-01.crl`
- `http://www.microsoft.com/pki/certs/MicTimStaPCA_2010-07-01.crt`

### Certificate Subjects
- `Microsoft Root Certificate Authority`
- `Microsoft Root Certificate Authority 2010`
- `Microsoft Root Certificate Authority 2011`
- `Microsoft Code Signing PCA`
- `Microsoft Code Signing PCA 2011`
- `Microsoft Time-Stamp PCA`
- `Microsoft Time-Stamp PCA 2010`
- `Microsoft Time-Stamp Service`
- `Microsoft Time Source Master Clock`
- `nCipher DSE ESN:B8EC-30A4-7144`
- `nCipher DSE ESN:F528-3777-8A761`
- `nCipher NTS ESN:B027-C6F8-1D881`

### Window Class / Registry / Error Strings
**None found.** The EXE contains no window class names, registry keys, error messages, or user-facing strings. All such strings are in MovieMakerCore.dll.

---

## Resource Table Analysis

The `.rsrc` section (97,632 bytes) contains:

### RT_VERSION (9)
- **VS_FIXEDFILEINFO**:
  - Signature: 0xFEEF04BD
  - StrucVersion: 0x10000 (1.0)
  - FileVersionMS: 0x10004 (16.4)
  - FileVersionLS: 0xDF80D03 (3528.331)
  - ProductVersionMS: 0x10004 (16.4)
  - ProductVersionLS: 0xDF80D03 (3528.331)
  - FileFlagsMask: 0x3F
  - FileFlags: 0x0 (none - not debug, not prerelease)
  - FileOS: 0x40004 (Win32 NT)
  - FileType: 0x1 (Application)
  - FileSubtype: 0x0
  - FileDateMS: 0x0
  - FileDateLS: 0x0

### RT_MANIFEST
- Single manifest: `<assembly>` with identity `Microsoft.Windows.personalMedia.MovieMaker` version 5.1.0.0

### PNG Images
- Large PNG image(s) embedded - detected by IHDR/IDAT/IEND markers
- These are application icons and/or branding images

---

## Architecture Summary

```
MovieMaker.exe (Launcher Stub, ~54KB)
    |
    +-- KERNEL32.dll (OS kernel, always loaded)
    |     - Process/thread management
    |     - File I/O
    |     - Memory management
    |     - Security (SetDllDirectoryW for DLL hijack prevention)
    |
    +-- MSVCR110.dll (VC++ 2012 Runtime)
    |     - CRT initialization (_initterm, _initterm_e)
    |     - Exception handling (_except_handler4_common)
    |     - Command line parsing (__wgetmainargs)
    |     - Exit handling (_cexit, exit)
    |     - Crash reporting (_invoke_watson)
    |
    +-- WLXPhotoBase.dll (Direct Load)
    |     - BasePrivate::Delete(void*) - memory deallocation
    |     - Base types and utilities shared across WLX DLLs
    |
    +-- MovieMakerCore.dll (Delay Load) [LAZY LOADED]
          - MovieMakerMain() - THE entry point to all MovieMaker functionality
          - Contains all UI, project model, timeline, rendering, export logic
```

### Key Architectural Insight

MovieMaker.exe is a **thin launcher/stub** with only ~4.8KB of actual code. Its sole purpose is to:
1. Initialize the CRT and process environment
2. Set secure DLL search paths
3. Load WLXPhotoBase.dll for memory management primitives
4. Call `MovieMakerMain()` from MovieMakerCore.dll

**All application logic** - the UI (WPF/ATL ribbon), project model, timeline, media processing, effects, transitions, rendering, export, and file I/O - resides in MovieMakerCore.dll and its transitive dependencies (WLXPipeline.dll, WLMFReadWrite.dll, etc.).

This means for decompilation purposes, **MovieMakerCore.dll is the primary target** and MovieMaker.exe requires only a minimal `WinMain` wrapper implementation.

---

## Reconstructing the EXE Source Code

Based on the analysis, the reconstructed `main.cpp` or `WinMain.cpp` would look approximately like:

```cpp
#include <windows.h>

// From WLXPhotoBase.dll
extern "C" void __cdecl BasePrivate_Delete(void* p);

// From MovieMakerCore.dll (delay-loaded)
extern "C" int __cdecl MovieMakerMain();

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    SetDllDirectoryW(L"");
    int result = MovieMakerMain();
    return result;
}
```

The CRT setup, exception handling, and DLL delay-loading infrastructure are all handled by the MSVC 2012 linker and runtime - no custom code needed in the EXE.
