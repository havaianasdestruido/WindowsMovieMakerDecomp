# WLXCodecHost.exe — Static Analysis

**File:** `WLXCodecHost.exe`
**Description:** Windows Live Photo Gallery Codec Host
**Binary path:** `undecomp\Photo Gallery\WLXCodecHost.exe`

---

## PE Structure

| Field | Value |
|-------|-------|
| Architecture | x86 (32-bit) |
| PE type | PE32 (magic `0x10B`) |
| Linker version | 11.00 |
| Entry point | `0x00406D6C` |
| Image base | `0x00400000` |
| Image size | `0xD000` (53,248 bytes) |
| Subsystem | Windows GUI (2) |
| OS version | 6.02 (Windows 8) |
| Image version | 6.02 |
| Subsystem version | 6.00 |
| Checksum | `0x1DC77` |
| Timestamp | `0x533A3FED` — Tue Apr 1 01:26:21 2014 |

### DLL Characteristics
- Dynamic base (ASLR)
- NX compatible (DEP)
- Terminal Server Aware

### Sections

| Section | Virtual Size | Virtual Address | Raw Size | Characteristics |
|---------|-------------|----------------|----------|-----------------|
| `.text` | `0x7D3A` | `0x00401000` | `0x7E00` | Code, Execute Read |
| `.data` | `0x0A6C` | `0x00409000` | `0x0600` | Init Data, Read Write |
| `.rsrc` | `0x09D8` | `0x0040A000` | `0x0A00` | Init Data, Read Only |
| `.reloc` | `0x13DE` | `0x0040B000` | `0x1400` | Init Data, Discardable, Read Only |

### Directories
- **Import Directory:** RVA `0x80F0`, size `0x104`
- **Resource Directory:** RVA `0xA000`, size `0x9D8`
- **Certificate Directory:** RVA `0xA600`, size `0x3EC0` (Authenticode signature)
- **Base Relocation Directory:** RVA `0xB000`, size `0x7EC`
- **Debug Directory:** RVA `0x1250`, size `0x38`
- **Load Config Directory:** RVA `0x21C8`, size `0x40`
- **IAT Directory:** RVA `0x1000`, size `0x220`
- **No Export Directory** — this is an executable, not a DLL.
- **No COM Descriptor Directory** — no .NET metadata.

---

## Export Table

**No exports.** WLXCodecHost.exe is an EXE and does not export any symbols. This is expected for a codec host process.

---

## Import Table

### ADVAPI32.dll — Registry access (8 imports)
| Ordinal | Name |
|---------|------|
| 258 | `RegCloseKey` |
| 261 | `RegCreateKeyExW` |
| 267 | `RegDeleteKeyW` |
| 270 | `RegDeleteValueW` |
| 277 | `RegEnumKeyExW` |
| 289 | `RegOpenKeyExW` |
| 290 | `RegQueryInfoKeyW` |
| 2A6 | `RegSetValueExW` |

### KERNEL32.dll — Process/thread/memory management (38 imports)
Notable: `SetPriorityClass`, `CreateThread`, `GetModuleFileNameW`, `GetCommandLineW`, `LoadLibraryExW`, `FindResourceW`, `LoadResource`, `SizeofResource`, `HeapSetInformation`, `IsDebuggerPresent`, `IsProcessorFeaturePresent`, `GetTickCount64`, `QueryPerformanceCounter`, `MultiByteToWideChar`, `lstrcmpiW`

### USER32.dll — Message loop (6 imports)
`GetMessageW`, `DispatchMessageW`, `TranslateMessage`, `PostThreadMessageW`, `CharNextW`, `CharUpperW`

### MSVCR110.dll — Visual C++ 2012 runtime (38 imports)
Full CRT: `_onexit`, `__CxxFrameHandler3`, `_initterm`, `_initterm_e`, `__wgetmainargs`, `_XcptFilter`, `__set_app_type`, `_crt_debugger_hook`, `__crtUnhandledException`, `__crtTerminateProcess`, `memset`, `memcpy_s`, `malloc`, `free`, `wcscat_s`, `wcscpy_s`, `wcsncpy_s`, `wcsstr`, `_purecall`, `_except_handler4_common`, etc.

### ole32.dll — COM infrastructure (13 imports)
| Ordinal | Name |
|---------|------|
| 10 | `CoAddRefServerProcess` |
| 6B | `CoReleaseServerProcess` |
| 4E | `CoInitialize` |
| 7D | `CoUninitialize` |
| 78 | `CoTaskMemAlloc` |
| 7A | `CoTaskMemRealloc` |
| 79 | `CoTaskMemFree` |
| 1B5 | `StringFromGUID2` |
| 19 | `CoCreateInstance` |
| 63 | `CoRegisterClassObject` |
| 6F | `CoRevokeClassObject` |
| 6C | `CoResumeClassObjects` |
| 181 | `PropVariantClear` |

### OLEAUT32.dll — By ordinal only (7 ordinal imports)
Ordinals: 2, 6, 7, 161, 163, 186, 277

Likely corresponding to `SysAllocString`, `SysFreeString`, `SysStringLen`, `VariantClear`, `SafeArrayCreate`, `SafeArrayDestroy`, etc.

### gdiplus.dll — GDI+ initialization (2 imports)
- `GdiplusStartup`
- `GdiplusShutdown`

### WLXPhotoBase.dll — WLXPhoto shared base (5 imports)
| Ordinal | Name |
|---------|------|
| 34 | `?Throw@Base@@YGXJ@Z` — Base::Throw |
| 2B | `?New@BasePrivate@@YAPAXI_N@Z` — BasePrivate::New |
| 6 | `??1Exception@Base@@UAE@XZ` — Base::Exception::~Exception |
| 29 | `?IsWin7OrGreater@OS@Base@@YG_NXZ` — Base::OS::IsWin7OrGreater |
| 14 | `?Delete@BasePrivate@@YAXPAX@Z` — BasePrivate::Delete |
| 11 | `?BaseAtlThrow@ATL@@YGXJ@Z` — ATL::BaseAtlThrow |

### MetadataSys.dll — Property handler registration (1 import)
| Ordinal | Name |
|---------|------|
| 4 | `WLXPSGetItemPropertyHandler` |

### GDI32.dll — Bitmap resources (2 imports)
- `CreateDIBSection`
- `DeleteObject`

### SHELL32.dll — Shell item creation (1 import)
- `SHCreateItemFromParsingName`

### PROPSYS.dll — Property system (1 import)
- `PSCoerceToCanonicalValue`

---

## COM GUIDs (Extracted from `rgs` resource / RTTI)

### CLSID_CodecHost
```
{E30A45E6-1916-4659-95EE-035E62DB9AB0}
```

### ProgIDs
- `Microsoft.WLXCodecHost.CodecHost.1`
- `Microsoft.WLXCodecHost.CodecHost`

### C++ RTTI Types
The binary contains ATL-based COM infrastructure with the following type hierarchy:

```
IClassFactory         (interface)
  └─ CComClassFactory (ATL)

ICodecHost            (interface — custom)
  └─ CodecHost        (implementation)

CComObjectRootEx<CComMultiThreadModelNoCS>
  └─ CComObject<CodecHost>

CAtlExeModuleT<CodecHostModule>
  └─ CAtlModuleT<CodecHostModule>
```

Key RTTI names found:
- `.?AVCodecHostModule@@` — `CodecHostModule` (EXE module class)
- `.?AVCodecHost@@` — `CodecHost` (main COM object)
- `.?AUICodecHost@@` — `ICodecHost` (custom interface)
- `.?AUCAtlModule@ATL@@` — `ATL::CAtlModule`
- `.?AV?$CAtlExeModuleT@VCodecHostModule@@@ATL@@` — `ATL::CAtlExeModuleT<CodecHostModule>`
- `.?AV?$CComObject@VCodecHost@@@ATL@@` — `ATL::CComObject<CodecHost>`
- `.?AV?$CComCoClass@VCodecHost@@$1?CLSID_CodecHost@@3U_GUID@@B@ATL@@` — `ATL::CComCoClass<CodecHost, &CLSID_CodecHost>`
- `.?AVGdiplusStartupWrapper@GdipUtil@@` — `GdipUtil::GdiplusStartupWrapper`

---

## Key Strings (ASCII/Unicode)

### Embedded manifest resource
```xml
<assembly xmlns='urn:schemas-microsoft-com:com:asm.v1' manifestVersion='1.0'>
  <assemblyIdentity
    version="5.1.0.0"
    processorArchitecture="x86"
    name="Microsoft.Windows.personalMedia.WLXCodecHost"
    type="win32" />
  <description>Windows Live Photo Gallery Codec Host</description>
  <requestedExecutionLevel level="asInvoker" uiAccess="false"/>
</assembly>
```

### COM Registration (rgs — registry script)
```
HKCR
  Microsoft.WLXCodecHost.CodecHost.1
    CLSID = {E30A45E6-1916-4659-95EE-035E62DB9AB0}
  Microsoft.WLXCodecHost.CodecHost
    CLSID = {E30A45E6-1916-4659-95EE-035E62DB9AB0}
    CurVer = Microsoft.WLXCodecHost.CodecHost.1
  CLSID
    {E30A45E6-1916-4659-95EE-035E62DB9AB0}
      ProgID = Microsoft.WLXCodecHost.CodecHost.1
      VersionIndependentProgID = Microsoft.WLXCodecHost.CodecHost
      LocalServer32 = %MODULE%
      AppID = %APPID%
```

### Additional function strings (likely for COM registration/unregistration)
- `RegisterTypeLibForUser`
- `UnRegisterTypeLibForUser`

---

## Authenticode Signature

- Signed by **Microsoft Corporation**
- Timestamp: **2014-04-01 00:21:28Z** (primary), 2013-03-27 — 2014-06-27 window
- Certificate chain:
  - Microsoft Root Certificate Authority
  - Microsoft Root Certificate Authority 2010
  - Microsoft Code Signing PCA / Microsoft Code Signing PCA 2011
  - Microsoft Time-Stamp PCA 2010
- ESN (nCipher): `F528-3777-8A76`, `B8EC-30A4-7144` (time-stamp HSMs)
- PDB: `WLXCodecHost.pdb`, GUID `{90EC3586-C320-47E2-A466-071F0ABA3923}`, age 1

---

## Architecture Summary

WLXCodecHost.exe is an **ATL-based out-of-process COM server** (LocalServer32) that acts as a **sandboxed codec host** for Windows Live Photo Gallery. Key design points:

1. **OUT-OF-PROCESS SANDBOX**: Runs as a standalone EXE, separate from the main WLX processes. Codecs execute in this isolated process for crash isolation and security.

2. **COM ACTIVATION**: Registered as LocalServer32 — WLX apps activate `CLSID_CodecHost` (`{E30A45E6-1916-4659-95EE-035E62DB9AB0}`) via `CoCreateInstance`. The host manages its lifetime via `CoAddRefServerProcess`/`CoReleaseServerProcess`.

3. **IMAGE DECODING PIPELINE**: Imports GDI+ (`GdiplusStartup`/`GdiplusShutdown`) for image decoding, `CreateDIBSection` for DIB management, and `SHCreateItemFromParsingName` for shell item parsing (drag-and-drop / file open).

4. **PROPERTY SYSTEM**: Uses `PSCoerceToCanonicalValue` (PROPSYS.dll) and `WLXPSGetItemPropertyHandler` (MetadataSys.dll) to handle photo metadata via the Windows Property System.

5. **REGISTRY ISOLATION**: Manages its own registry keys (ADVAPI32 imports) — likely for per-user codec registration and sandboxed settings independent of the parent process.

6. **CRT**: Statically links Visual C++ 2012 runtime (MSVCR110.dll) functions. Uses SEH (`__CxxFrameHandler3`, `_XcptFilter`) for structured exception handling critical for a sandbox process that must not crash the caller.

7. **THREADING**: Single-threaded message pump (`GetMessageW`/`DispatchMessageW`) with `CreateThread` usage, `PostThreadMessageW` for cross-thread communication.

8. **WLXPHOTOBASE DEPENDENCY**: Uses `BasePrivate::New`/`Delete` for heap allocation, `Base::Throw` for exception handling, `OS::IsWin7OrGreater` for OS version detection — consistent with Windows 8 targeting (OS version 6.02).
