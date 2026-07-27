# RegRes.dll — Static Analysis

## Overview

**RegRes.dll** is a pure resource-only DLL (no code, no exports, no imports) providing UI string tables, version information, and localized resources for Windows Live Photo Gallery 2012 (Windows Live Essentials 16.4.3528.0331). It is a satellite resource assembly — loaded by the application solely via `FindResourceExW`/`LoadResource` to obtain localized UI strings.

---

## PE Structure

| Field | Value |
|---|---|
| **File** | `RegRes.dll` |
| **Arch** | x86 (14C) |
| **Subsystem** | Windows GUI (2) |
| **Linker** | Visual Studio 2012 (11.00) |
| **Entry Point** | `0x00000000` (none) |
| **Image Base** | `0x10000000` |
| **Image Size** | `0x2000` (8,192 bytes) |
| **Code Size** | `0x0` (zero — no code) |
| **OS Target** | 6.02 (Windows 8) |
| **Image Version** | 6.02 |
| **Timestamp** | Tue Apr 1 01:28:29 2014 |
| **Checksum** | `0x8186` |
| **DLL Characteristics** | `0x540` — Dynamic Base, NX Compatible, No SEH |
| **Debug PDB** | None |

### Sections

| Section | Virtual Size | Virtual Address | Raw Size | Flags |
|---------|-------------|----------------|----------|-------|
| `.rsrc` | `0x8B8` | `0x10001000` | `0xA00` | Initialized Data, Read Only |

### Notable

- **Single `.rsrc` section** — no `.text`, no `.data`, no `.reloc`. This is a pure resource satellite DLL.
- **No entry point** — `DllMain` is never called; the DLL exists only to hold resources.
- **No imports** — zero dependency chain; loaded solely for `LoadResource`/`LockResource`.
- **No exports** — not a COM server; not a function provider.

---

## Export Table

**None.** This DLL has no export directory.

---

## Import Table

**None.** This DLL has no import directory.

---

## Resource Contents

### VS_VERSION_INFO

| Field | Value |
|---|---|
| **CompanyName** | Microsoft Corporation |
| **FileDescription** | WLX Registry Resource |
| **InternalName** | RegRes |
| **LegalCopyright** | Microsoft Corporation. All rights reserved. |
| **OriginalFilename** | RegRes.dll |
| **ProductName** | Photo Gallery |
| **FileVersion** | 16.4.3528.0331 |
| **ProductVersion** | 16.4.3528.0331 |

### String Table (Unicode)

The resource section contains a string table with the following categories:

#### Application Description Strings
| String | Purpose |
|--------|---------|
| `Photo Gallery` | Application product name |
| `Import pictures and videos` | Import feature description |
| `View pictures` | View feature description |
| `Import video` | Video import feature |
| `Photo Gallery is the built-in picture viewer...` | Extended description (installer/app registration) |

#### Context Menu / Toolbar Actions
| String | Purpose |
|--------|---------|
| `Pre&view` | Preview menu item |
| `Rotate &right 90` | Rotate right 90° |
| `Rotate &left 90` | Rotate left 90° |
| `&Print` | Print action |
| `Set as Desktop &Background` | Set as wallpaper |

#### Image Format Labels
| String | Format |
|--------|--------|
| `JPEG Image` | JPEG/JFIF |
| `PNG Image` | PNG |
| `Bitmap Image` | BMP |
| `GIF Image` | GIF |
| `TIFF Image` | TIFF |
| `HD Photo` | HD Photo/JPEG XR |
| `Camera Raw Image` | Camera RAW formats |
| `Icon` | ICO |

#### Localization
| String | Purpose |
|--------|---------|
| `en-us` | English (US) locale identifier |

---

## RTTI / Class Hierarchy

**None.** No code section, no RTTI data.

---

## Analysis Summary

### Architecture
1. **Resource Satellite DLL** — standard Windows pattern for UI string externalization
2. **Zero dependencies** — no imports, no code, no entry point
3. **Zero exports** — not loadable as a function provider; only useful via `LoadLibrary` + `FindResource`
4. **English (US) strings only** — other locale variants would be separate DLLs (e.g., `RegRes.deu.dll`)

### Design Pattern
- Windows Live Essentials used satellite resource DLLs for localization
- The host application (WLXPhotoGallery.exe) loads `RegRes.dll` via `LoadLibraryEx` with `LOAD_LIBRARY_AS_DATAFILE` or equivalent
- Strings are extracted via `LoadStringW` or resource table parsing
- Each locale gets its own copy (e.g., `RegRes.dll` = en-us, `RegRes.deu.dll` = German, etc.)

### Role in Photo Gallery Ecosystem
- Provides **UI labels** for image format display (what the user sees in the gallery)
- Provides **context menu text** for image operations (rotate, print, set as background)
- Provides **installer registration strings** for file type associations
- Referenced by `WLXPhotoGallery.exe` and potentially `WLXPhotoBase.dll` for file type descriptions

---

## Runtime Test Results

Test harness: `tests/OtherDlls/test_regres_imaging_metadatasys.cpp`

| Test | Result |
|------|--------|
| `LoadLibrary` | Success — base `0x001F0000` |
| `GetProcAddress("DllCanUnloadNow")` | NULL — not found (expected for resource-only DLL) |
| `GetProcAddress("DllGetClassObject")` | NULL — not found (expected for resource-only DLL) |
| `FindResourceW(RT_VERSION)` | Success — VS_VERSION_INFO found |
| `LoadResource` + `LockResource` | Success — version info struct readable |
