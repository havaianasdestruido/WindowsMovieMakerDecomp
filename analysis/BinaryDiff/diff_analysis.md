# Binary Diff Analysis: Original vs Reconstructed

**Date**: 2026-07-27
**Original Binaries**: `undecomp\Photo Gallery\MovieMaker.exe`, `MovieMakerCore.dll`
**Rebuilt Binaries**: `build\bin\Debug\MovieMaker.exe`, `build\bin\Debug\MovieMakerCore.dll`

---

## Executive Summary

The rebuilt binaries demonstrate **structural correctness** for the public API surface (the EXE→DLL entry point is preserved), but differ significantly in:
- Binary size (resources stripped, much less embedded code)
- Export hygiene (all C++ symbols leak from rebuilt; original exports only `MovieMakerMain`)
- CRT runtime (original uses MSVCR110/VS2012; rebuilt uses MSVCP140+VCRUNTIME140/VS2022)
- Missing system DLL dependencies (DirectX, DWM, UX, analytics, etc.)
- Missing `.rsrc` content (UI strings, dialogs, version info, icons)

---

## 1. MovieMaker.exe

### 1.1 File Size

| | Original | Rebuilt | Delta |
|---|---|---|---|
| File size | 122,048 bytes | 96,256 bytes | -21.1% |

The rebuilt is smaller mainly due to the missing `.rsrc` section (97,632 → 128 bytes).

### 1.2 Section Layout

#### Original (4 sections)
| Section | Virtual Size | Raw Size | Purpose |
|---------|-------------|----------|---------|
| `.text` | 4,848 | 5,120 | Thin launcher stub |
| `.data` | 920 | 512 | IAT, data |
| `.rsrc` | 97,632 | 97,792 | Icons, manifest, version, UI resources |
| `.reloc` | 1,402 | 1,536 | Base relocations |

#### Rebuilt (6 sections)
| Section | Virtual Size | Raw Size | Purpose |
|---------|-------------|----------|---------|
| `.text` | 56,556 | 56,832 | Main application code |
| `.rdata` | 27,290 | 27,648 | Read-only data, vtables, CRT |
| `.data` | 4,808 | 2,560 | IAT, initialized data |
| `.rsrc` | 128 | 512 | Stub (no real resources) |
| `.reloc` | 2,712 | 3,072 | Base relocations |
| `.fptable` | 4,188 | 4,608 | Function pointer table (CRT) |

**Key observation**: The original `.text` was a tiny 4,848-byte launcher; the rebuilt `.text` is 56,556 bytes because it inlines the CRT startup and the EXE logic that was previously linked statically.

### 1.3 Imports

#### Original
| DLL | Functions | Notes |
|-----|-----------|-------|
| KERNEL32.dll | 15 | Minimal: `SetDllDirectoryW`, `GetProcAddress`, `FreeLibrary`, etc. |
| MSVCR110.dll | 31 | VS2012 CRT (`_initterm`, `__dllonexit`, `_XcptFilter`, etc.) |
| WLXPhotoBase.dll | 1 | `?Delete@BasePrivate@@YAXPAX@Z` (memory deallocation) |
| *delay-load* MovieMakerCore.dll | 1 | `MovieMakerMain` (the sole entry point) |

#### Rebuilt
| DLL | Functions | Notes |
|-----|-----------|-------|
| KERNEL32.dll | 66 | Much heavier: heap, TLS, threading, file I/O |
| SHELL32.dll | 1 | `CommandLineToArgvW` |
| WLXPhotoBase.dll | 1 | `_WLXPhotoBase_Init@0` (**different symbol!**) |
| COMDLG32.dll | 2 | `GetOpenFileNameW`, `GetSaveFileNameW` |
| COMCTL32.dll | 1 | `InitCommonControlsEx` |
| MSVCP140.dll | 5 | VS2022 C++ runtime |
| VCRUNTIME140.dll | 17 | VS2022 CRT (`memcpy`, `memset`, `__CxxFrameHandler3`, etc.) |
| api-ms-win-crt-*.dll | 6 DLLs | UCRT shims (heap, string, stdio, etc.) |

#### Missing from rebuilt (originally imported)
- `MSVCR110.dll` — VS2012 CRT (replaced by VS2022 CRT)

#### Extra in rebuilt (not in original)
- `SHELL32.dll`, `COMDLG32.dll`, `COMCTL32.dll` — UI support
- `MSVCP140.dll`, `VCRUNTIME140.dll`, `api-ms-win-crt-*` — modern CRT

#### Critical difference
- **WLXPhotoBase.dll**: Original imports `?Delete@BasePrivate@@YAXPAX@Z`, rebuilt imports `_WLXPhotoBase_Init@0`. This is a different function entirely, suggesting the reconstructed source links against a different initialization path.
- **No delay-loaded MovieMakerCore.dll** in the rebuilt. The EXE apparently calls `MovieMakerMain` statically or via a different mechanism.

### 1.4 Exports

| | Original | Rebuilt |
|---|---|---|
| Exported functions | 0 | 0 |

Neither exports anything. Correct.

### 1.5 Resources

| | Original `.rsrc` | Rebuilt `.rsrc` |
|---|---|---|
| Virtual size | 97,632 | 128 |
| Content | Icons, manifest, version info, UI strings | Empty stub |

The rebuilt EXE has essentially **no embedded resources**. The original has a full set of Win32 resources (application icon, version information, embedded manifest, string tables).

---

## 2. MovieMakerCore.dll

### 2.1 File Size

| | Original | Rebuilt | Delta |
|---|---|---|---|
| File size | 10,583,240 bytes (10.1 MB) | 1,716,224 bytes (1.6 MB) | **-83.8%** |

The rebuilt is ~6x smaller. The major contributors to the difference:
- Resources: 4.0 MB → 0.5 KB (99.99% missing)
- Code: 5.5 MB → 1.0 MB (~82% less)
- Data: 228 KB → 31 KB (~86% less)

### 2.2 Section Layout

#### Original (4 sections)
| Section | Virtual Size | Purpose |
|---------|-------------|---------|
| `.text` | 5,719,124 (5.5 MB) | Compiled C++ code |
| `.data` | 234,140 | Mutable globals, vtables |
| `.rsrc` | 4,240,504 (4.0 MB) | Embedded resources (dialogs, strings, XML, etc.) |
| `.reloc` | 416,062 | Base relocations |

#### Rebuilt (5 sections)
| Section | Virtual Size | Purpose |
|---------|-------------|---------|
| `.text` | 1,013,466 (988 KB) | Compiled C++ code |
| `.rdata` | 625,222 (611 KB) | Read-only data, vtables, string literals |
| `.data` | 31,260 | Mutable globals |
| `.rsrc` | 480 | Stub |
| `.reloc` | 45,536 | Base relocations |

### 2.3 Exports

| | Original | Rebuilt |
|---|---|---|
| Exported functions | **1** | **4,224** |
| Export names | `MovieMakerMain` | ALL C++ mangled symbols + `MovieMakerMain` |
| Export module name | `MovieMakerCore.dll` | **`MOVIECORE.dll`** (wrong casing!) |

**Critical issues**:
1. The rebuilt exports **every C++ symbol** (4,224 functions). The original only exports `MovieMakerMain`. This is because the rebuilt project likely doesn't use a `.def` file or `__declspec(dllexport)` carefully. Only `MovieMakerMain` should be exported.
2. The export table header says the module name is `MOVIECORE.dll` instead of `MovieMakerCore.dll`. This is the name from the CMake target and must match the original filename for correct delay-load resolution.

### 2.4 Import DLL Comparison

#### Original (34 DLLs)
```
ADVAPI32.dll, d2d1.dll, d3d11.dll, d3d9.dll, D3DCOMPILER_46.dll,
DmxBici.dll, dwmapi.dll, DWrite.dll, dxva2.dll, ESENT.dll,
GDI32.dll, gdiplus.dll, KERNEL32.dll, MetadataSys.dll, MF.dll,
MFPlat.DLL, MSVCR110.dll, ole32.dll, OLEACC.dll, OLEAUT32.dll,
PROPSYS.dll, SHELL32.dll, SHLWAPI.dll, USER32.dll, UXCore.dll,
uxctl.dll, UxTheme.dll, VERSION.dll, WindowsCodecs.dll, WINMM.dll,
wlidcli.dll, WLXPhotoBase.dll, WLXPhotoSqm.dll, XmlLite.dll
```

#### Rebuilt (29 DLLs)
```
ADVAPI32.dll, api-ms-win-crt-convert-l1-1-0.dll,
api-ms-win-crt-heap-l1-1-0.dll, api-ms-win-crt-math-l1-1-0.dll,
api-ms-win-crt-runtime-l1-1-0.dll, api-ms-win-crt-stdio-l1-1-0.dll,
api-ms-win-crt-string-l1-1-0.dll, api-ms-win-crt-utility-l1-1-0.dll,
COMCTL32.dll, COMDLG32.dll, d2d1.dll, d3d11.dll, DWrite.dll,
GDI32.dll, gdiplus.dll, KERNEL32.dll, MF.dll, MFPlat.DLL,
MFReadWrite.dll, MSVCP140.dll, ole32.dll, OLEAUT32.dll,
SHELL32.dll, SHLWAPI.dll, USER32.dll, VCRUNTIME140.dll,
WININET.dll, WLXPhotoBase.dll, XmlLite.dll
```

#### Missing from rebuilt (18 DLLs not linked)
| DLL | Purpose | Impact |
|-----|---------|--------|
| `d3d9.dll` | Direct3D 9 | DX9 rendering pipeline broken |
| `D3DCOMPILER_46.dll` | HLSL shader compilation | Cinematic effects won't compile |
| `DmxBici.dll` | Analytics telemetry | SQM/telemetry non-functional |
| `dwmapi.dll` | Desktop Window Manager | Aero glass effects missing |
| `dxva2.dll` | DXVA2 video acceleration | HW video decode broken |
| `ESENT.dll` | Extensible Storage Engine | Photo library database broken |
| `MetadataSys.dll` | Internal metadata handling | EXIF/metadata reads fail |
| `MSVCR110.dll` | VS2012 CRT | Replaced by modern CRT |
| `OLEACC.dll` | Accessibility | UI Automation broken |
| `PROPSYS.dll` | Property System | Shell property reads fail |
| `UXCore.dll` | DirectUI framework | Entire UI framework missing |
| `uxctl.dll` | UX Controls | Ribbon/controls broken |
| `UxTheme.dll` | Visual styles/theming | Theme rendering broken |
| `VERSION.dll` | Version info queries | Version checking broken |
| `WindowsCodecs.dll` | WIC image codec | Image encoding/decoding broken |
| `WINMM.dll` | Multimedia (PlaySound) | Audio playback broken |
| `wlidcli.dll` | Windows Live ID | Sign-in/sharing broken |
| `WLXPhotoSqm.dll` | SQM telemetry | Usage reporting broken |

#### Extra in rebuilt (not in original, 10 DLLs)
| DLL | Purpose |
|-----|---------|
| `api-ms-win-crt-*.dll` (6) | Modern CRT shims |
| `COMCTL32.dll` | Common controls |
| `COMDLG32.dll` | Common dialogs |
| `MFReadWrite.dll` | Media Foundation read/write |
| `MSVCP140.dll` | VS2022 C++ runtime |
| `VCRUNTIME140.dll` | VS2022 CRT |
| `WININET.dll` | WinINet internet functions |

### 2.5 Individual Import Functions (Selected)

#### KERNEL32 — Original has 0, Rebuilt has ~65
The original MovieMakerCore.dll does **not** directly import KERNEL32 at all (it was linked with `/DELAYLOAD` or statically linked CRT that wrapped kernel calls). The rebuilt imports many kernel functions directly.

#### GDI32 — Original has 18, Rebuilt has 7
Original imports extensive GDI (region manipulation, DIB sections, brushes, `GetLayout`/`SetLayout`). Rebuilt only has basic: `DeleteDC`, `CreateDIBSection`, `SetTextColor`, `SetBkMode`, `SelectObject`, `CreateFontW`, `DeleteObject`.

#### ole32.dll — Original has 22, Rebuilt has 15
Missing from rebuilt: `PropVariantCopy`, `CoAllowSetForegroundWindow`, `OleIsCurrentClipboard`, `OleGetClipboard`, `OleSetClipboard`, `CoSetProxyBlanket`, `OleInitialize`, `OleUninitialize`, `CoInitializeEx`, `StgOpenStorage`, `CoGetObject`, `ReleaseStgMedium`

#### USER32 — Original has ~37, Rebuilt has ~38
Roughly equivalent in count, but different functions. Original has DirectUI-related calls; rebuilt has more standard Win32 window management.

#### gdiplus.dll — Original has 25, Rebuilt has 12
Original has extensive GDI+ (fonts, rendering transforms, region clipping, drawing). Rebuilt has basic bitmap operations only.

### 2.6 Linker Metadata

| Property | Original | Rebuilt |
|----------|----------|---------|
| Machine | x86 (0x14C) | x86 (0x14C) |
| Linker version | 11.00 | 14.44 |
| OS version | 6.02 (Win8) | 6.00 (Win6) |
| Image version | 6.02 | 16.04 |
| Subsystem | 2 (GUI) | 2 (GUI) |
| DLL characteristics | Dynamic base, NX compatible | Dynamic base, NX compatible, **CFG** |
| Image base | 0x10000000 | 0x10000000 |
| Size of image | 0xA22000 (10.1 MB) | 0x1A7000 (1.6 MB) |
| Checksum | 0xA23F3E | 0 (not computed) |
| Timestamp | Tue Apr 1 2014 | Thu Jul 23 2026 |

---

## 3. Summary of Gaps

### Critical (runtime-breaking)
1. **18 missing DLL imports** in MovieMakerCore.dll — Direct3D 9, DirectUI (UXCore), ESENT database, accessibility, theming, multimedia, WIC, etc.
2. **Massive resource gap** — Original has 4.0 MB of embedded resources (UI definitions, strings, XML templates); rebuilt has ~0
3. **Export over-exporting** — 4,224 symbols exported vs intended 1; must use a `.def` file
4. **Export module name wrong** — `MOVIECORE.dll` vs `MovieMakerCore.dll`

### Significant (functional)
5. **EXE→DLL linkage** — Original uses delay-loaded `MovieMakerCore.dll` imported by name `MovieMakerMain`; rebuilt uses static linking
6. **WLXPhotoBase.dll function mismatch** — `_WLXPhotoBase_Init@0` vs `?Delete@BasePrivate@@YAXPAX@Z`
7. **CRT mismatch** — VS2012 (MSVCR110) vs VS2022 (MSVCP140/VCRUNTIME140) — ABI incompatible
8. **Missing 8 GDI+ functions**, 7 ole32 functions, 11 GDI32 functions

### Cosmetic/Build
9. Extra sections in rebuilt (`.rdata`, `.fptable`) not present in original
10. Different linker version (11.00 vs 14.44)
11. Control Flow Guard (CFG) enabled in rebuilt but not original
12. No checksum in rebuilt

---

## 4. Recommendations

1. **Add a `.def` file** for MovieMakerCore.dll to export only `MovieMakerMain`
2. **Fix the DLL name** in the export table (CMake target name)
3. **Add delay-loaded import** of MovieMakerCore.dll from MovieMaker.exe (use `/DELAYLOAD:MovieMakerCore.dll`)
4. **Add missing system DLL stubs** or link against the actual DLLs (d3d9, D3DCOMPILER_46, dxva2, UXCore, ESENT, etc.)
5. **Import the correct WLXPhotoBase.dll symbol** (`?Delete@BasePrivate@@YAXPAX@Z`)
6. **Port or create resource files** (icons, manifests, version info, UI definitions)
7. **Match the original section layout** (4 sections in MovieMakerCore.dll: `.text`, `.data`, `.rsrc`, `.reloc`)

---

## 5. Export Count Verification

```
Original MovieMakerCore.dll exports:
  ordinal 1: MovieMakerMain (the only export)

Rebuilt MovieMakerCore.dll exports:
  ordinal 1: MovieMakerMain ✓
  ordinal 2..4224: ALL internal C++ symbols ✗ (must be removed)
```
