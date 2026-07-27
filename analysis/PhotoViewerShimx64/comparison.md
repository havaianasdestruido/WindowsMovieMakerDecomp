# PhotoViewerShimx64.dll - x64 vs x86 Deep Comparison

## Executive Summary

**PhotoViewerShimx64.dll is a native x64 build** compiled from the same source code as the x86 `PhotoViewerShim.dll`. It is **not** a thunking layer, WoW64 bridge, or forwarding shim. Both binaries implement the same ATL-based COM in-process server providing OLE Drag-and-Drop target shims for Windows Photo Gallery, with identical CLSIDs, identical RTTI class hierarchies, and identical functionality — just compiled for different architectures.

---

## 1. PE Header Comparison

| Property | x86 (PhotoViewerShim.dll) | x64 (PhotoViewerShimx64.dll) |
|----------|--------------------------|------------------------------|
| Machine | 0x14C (x86) | 0x8664 (AMD64) |
| PE Format | PE32 (magic 0x10B) | PE32+ (magic 0x20B) |
| Linker | MSVC 11.00 | MSVC 11.00 |
| Timestamp | **2014-04-01 01:28:31 UTC** | **2012-03-28 16:51:00 UTC** |
| Image Base | 0x10000000 | 0x0000000180000000 |
| Size of Image | 0xC000 (48 KB) | 0xF000 (60 KB) |
| Entry Point | 0x6BC9 | 0x7F90 |
| Subsystem | Windows GUI (2) | Windows GUI (2) |
| DLL Characteristics | Dynamic base, NX compatible | **High Entropy VA**, Dynamic base, NX compatible |
| Size of Code | 0x7C00 (31,744 B) | 0x9400 (37,888 B) |
| Size of Init Data | 0x1E00 (7,680 B) | 0x2200 (8,704 B) |
| Stack Reserve | 0x40000 | 0x40000 |
| Certificates Dir | 0x3EC0 (16,064 B) | 0x3EC0 (16,064 B) |
| Exception Dir | **0x0000 (none)** | **0x540 (1,344 B)** |
| Load Config Dir | 0x38 bytes | **0x70 bytes** |

### Key Observations
- **x64 is 2 years older** (March 2012) than the x86 binary (April 2014)
- x64 has **High Entropy Virtual Addresses** (ASLR entropy) — a 64-bit security feature
- x64 has a **64-bit Exception Directory** (0x540 bytes) for native x64 structured exception handling
- x64 Load Config Directory is double the size (0x70 vs 0x38) — includes 64-bit security cookies and CFG
- Both have identical **Certificates Directory size** (0x3EC0) — same Authenticode signature blob size
- x64 code is ~19% larger due to wider pointers and x64 calling conventions

---

## 2. Section Comparison

### x86 — 4 sections
| Section | VirtAddr | VirtSize | RawSize |
|---------|----------|----------|---------|
| .text | 0x1000 | 0x7BAB | 0x7C00 |
| .data | 0x9000 | 0x954 | 0x600 |
| .rsrc | 0xA000 | 0x658 | 0x800 |
| .reloc | 0xB000 | 0xB02 | 0xC00 |

### x64 — 5 sections
| Section | VirtAddr | VirtSize | RawSize |
|---------|----------|----------|---------|
| .text | 0x1000 | 0x93EB | 0x9400 |
| .data | 0xB000 | 0xE10 | 0x800 |
| **.pdata** | **0xC000** | **0x540** | **0x600** |
| .rsrc | 0xD000 | 0x6A0 | 0x800 |
| .reloc | 0xE000 | 0x30A | 0x400 |

### Key Differences
- x64 adds a **.pdata section** (0x540 bytes) — contains the x64 Exception Directory (`RUNTIME_FUNCTION` entries mapping every function's unwind info)
- x64 .data section is 46% larger (0xE10 vs 0x954) — wider pointers in RTTI/vtable structures
- x64 .reloc is smaller (0x30A vs 0xB02) — x64 PE uses `IMAGE_BASE_RELOCATION` with 12-bit offsets, more efficient
- x64 .rsrc slightly larger (0x6A0 vs 0x658) — wide-char version strings differ

---

## 3. Export Comparison

| Property | x86 | x64 |
|----------|-----|-----|
| Export directory name | `DropTargetShim.dll` | `DropTargetShim.dll` |
| Number of functions | 4 | 4 |
| Number of names | 4 | 4 |
| Ordinal base | 1 | 1 |

| Ordinal | Hint | x86 RVA | x64 RVA | Name |
|---------|------|---------|---------|------|
| 1 | 0 | 0x3595 | 0x3660 | `DllCanUnloadNow` |
| 2 | 1 | 0x35AE | 0x3674 | `DllGetClassObject` |
| 3 | 2 | 0x35F8 | 0x37A4 | `DllRegisterServer` |
| 4 | 3 | 0x360A | 0x3860 | `DllUnregisterServer` |

**Identical exports, same ordinal scheme, same internal DLL name.** The export directory string "DropTargetShim.dll" is the original internal name for both binaries.

---

## 4. Import Comparison

### MSVCR110.dll (C Runtime)

| Feature | x86 | x64 |
|---------|-----|-----|
| Exception handler | `_except_handler4_common` | **`__C_specific_handler`** |
| Operator new | `??2@YAPAXI@Z` (void* __cdecl new(uint)) | `??2@YAPEAX_K@Z` (void* __operator new(size_t)) |
| Operator delete | `??3@YAXPAX@Z` (void __cdecl delete(void*)) | `??3@YAXPEAX@Z` (void __operator delete(void*)) |
| Operator new[] | `??_U@YAPAXI@Z` | `??_U@YAPEAX_K@Z` |
| Operator delete[] | `??_V@YAXPAX@Z` | `??_V@YAXPEAX@Z` |
| type_info dtor | `??1type_info@@UAE@XZ` | **`?_type_info_dtor_internal_method@type_info@@QEAAXXZ`** |
| SEH context | N/A | **`__crtCapturePreviousContext`**, **`__crtCaptureCurrentContext`** |

x64 mangled names use `PEAX` (pointer to void) instead of `PAX`, and `_K` (size_t=uint64) instead of `I` (uint32) — these are standard x64 name mangling differences.

### OLEAUT32.dll

| x86 Ordinals | x64 Ordinals |
|-------------|-------------|
| 7, 161, 2, 6, 186, 277, 163 | **277 only** |

x86 imports 7 OLEAUT32 functions (SysFreeString, SysAllocStringLen, SysStringLen, SysAllocString, VariantClear, SafeArrayAccessData, SysReAllocString). x64 imports only ordinal 277 (SafeArrayAccessData). The others are likely resolved via delay-load or the x64 runtime binds them differently.

### Other DLLs
All other imports (KERNEL32, ADVAPI32, ole32, USER32) are **functionally identical** between x86 and x64 — same API set, same semantics.

---

## 5. CLSID Verification

Both binaries contain an identical embedded ATL REGISTRY resource with **the same two CLSIDs**:

| CLSID | Description | ThreadingModel |
|-------|-------------|----------------|
| `{00f346cb-35a4-465b-8b8f-65a29dbab1f6}` | Live Shell Viewer Extension | Apartment |
| `{00f3712a-ca79-45b4-9e4d-d7891e7f8b9d}` | Live Shell Editor Extension | Apartment |

Both register as `InprocServer32` with `%MODULE%` placeholder (ATL substitutes the actual DLL path at registration time).

Both binaries reference `CLSID_PhotoViewerInprocShim` via RTTI in the `CComCoClass<ViewerDropTargetShim, &CLSID_PhotoViewerInprocShim>` template instantiation.

**CLSID match confirmed.** The x64 binary registers under the same COM GUIDs as x86 — only one can be active per bitness in a given registry hive.

---

## 6. RTTI Comparison

### Identical class hierarchy in both binaries:

| Class | x86 RTTI | x64 RTTI |
|-------|----------|----------|
| `type_info` | Present | Present |
| `CAtlModule<ATL>` | Present | Present |
| `_ATL_MODULE70<ATL>` | Present | Present |
| `CAtlDllModuleT<DropTargetShimModule>` | Present | Present |
| `CAtlModuleT<DropTargetShimModule>` | Present | Present |
| `CAtlValidateModuleConfiguration<1, DropTargetShimModule>` | Present | Present |
| `DropTargetShimModule` | Present | Present |
| `CAtlException<ATL>` | Present | Present |
| `CComObjectRootEx<CComMultiThreadModel<ATL>>` | Present | Present |
| `CComObjectRootBase<ATL>` | Present | Present |
| `CComClassFactory<ATL>` | Present | Present |
| `CComObjectCached<CComClassFactory<ATL>>` | Present | Present |
| `CRegObject<ATL>` | Present | Present |
| `IUnknown` | Present | Present |
| `IRegistrarBase` | Present | Present |
| `IClassFactory` | Present | Present |
| `IDropTarget` | Present | Present |
| `IObjectWithSite` | Present | Present |
| `ViewerDropTargetShim` | Present | Present |
| `EditorDropTargetShim` | Present | Present |
| `ViewerAutoplayDropTargetShim` | Present | Present |
| `ImportAutoplayDropTargetShim` | Present | Present |
| `CComCoClass<ViewerDropTargetShim, &CLSID_PhotoViewerInprocShim>` | Present | Present |
| `CComObject<ViewerDropTargetShim>` | Present | Present |
| `CComObject<EditorDropTargetShim>` | Present | Present |
| `CComObject<ViewerAutoplayDropTargetShim>` | Present | Present |
| `CComObject<ImportAutoplayDropTargetShim>` | Present | Present |

The RTTI data in `.data` sections confirms **identical source code compiled for both architectures**. The only difference is pointer widths in the RTTI structure records (x64 entries are larger due to 8-byte pointers).

---

## 7. PDB / Debug Info Comparison

| Property | x86 | x64 |
|----------|-----|-----|
| PDB Name | PhotoViewerShim.pdb | PhotoViewerShim.pdb |
| GUID | `{DDB3B186-F89C-482C-83A7-9FDD8F0F52AE}` | `{D15C59F9-BA59-4081-B04C-7365AD9AE382}` |
| Age | 1 | 1 |
| Format | RSDS | RSDS |

**Different PDB GUIDs** — confirms these were compiled as separate build invocations, not cross-compiled from the same session. Same PDB filename.

---

## 8. Version / Resource Comparison

| Property | x86 | x64 |
|----------|-----|-----|
| File Version | 16.4.3528.0331_ship | 16.4.0867.0325_working |
| Product Version | 16.4.3528.0331 | 16.4.0867.0325 |
| File Description | Photo Gallery | **Windows Live Photo Gallery** |
| Product Name | Photo Gallery | **Windows Live Photo Gallery** |
| Internal Name | DropTargetShim | DropTargetShim |
| Original Filename | DropTargetShim.dll | DropTargetShim.dll |
| Legal Copyright | 2012 Microsoft Corporation | 2012 Microsoft Corporation |
| VS_VERSION_INFO | 0x04BD04EF | 0x04BD04EF |

- x86 is from build branch `w5m4` (ship milestone 3528)
- x64 is from build branch `personalMedia` (working milestone 0867)
- Both are the same copyright year (2012), but x86 was built later (2014) — likely a recompile for a post-release update
- x64 description includes "Windows Live" branding; x86 has a shorter "Photo Gallery" string

---

## 9. Architecture Verdict

### Evidence this is a native x64 build (NOT a thunking layer):

1. **Machine type 0x8664** — native AMD64, not x86 or WOW64
2. **PE32+ format** — the 64-bit PE format, not PE32
3. **x64 name mangling** — `PEAX`, `_K` suffixes throughout CRT imports
4. **.pdata section** — contains `RUNTIME_FUNCTION` entries for x64 exception unwinding
5. **Exception Directory** (0x540 bytes) — x64-specific structured exception handling data
6. **`__C_specific_handler`** — x64 SEH handler, not x86 `_except_handler4_common`
7. **`__crtCapturePreviousContext`/`__crtCaptureCurrentContext`** — x64-only CRT functions for context capture
8. **High Entropy VA** flag — 64-bit ASLR feature
9. **64-bit Load Config** — 0x70 bytes (doubled from x86's 0x38) for 64-bit security features
10. **Identical RTTI hierarchy** — same classes, same COM objects, same functionality
11. **Identical CLSIDs** — same two registered COM objects
12. **Same source, different build** — different PDB GUIDs, different timestamps, different version strings
13. **No WoW64 thunks** — no Wow64Transition references, no SYSWOW64 paths
14. **No forwarding** — all code is native, no forwarded exports

### What it is NOT:
- **Not a thunking layer**: No WoW64 translation or forwarding to 32-bit code
- **Not a stub**: Contains full COM infrastructure and all four drop target implementations
- **Not a wrapper**: No inter-process communication or marshaling code

---

## 10. File Size Comparison

| Metric | x86 | x64 |
|--------|-----|-----|
| File size | ~40 KB | ~49 KB |
| Code (.text) | 0x7C00 (31,744 B) | 0x9400 (37,888 B) |
| Data (.data) | 0x600 (1,536 B) | 0x800 (2,048 B) |
| Resources (.rsrc) | 0x800 (2,048 B) | 0x800 (2,048 B) |
| Relocations (.reloc) | 0xC00 (3,072 B) | 0x400 (1,024 B) |
| Exception data (.pdata) | 0 (N/A) | 0x600 (1,536 B) |
| Code bloat factor | 1.0x | **1.19x** |

The 19% code size increase is consistent with a straightforward x86→x64 recompilation: wider registers, 8-byte pointers, REX prefixes, and larger instruction encodings.

---

## 11. Build Provenance Summary

| Property | x86 | x64 |
|----------|-----|-----|
| Product | Windows Live Photo Gallery | Windows Live Photo Gallery |
| Build branch | w5m4 (ship) | personalMedia (working) |
| Build number | 3528 | 0867 |
| Build date | 2014-04-01 | 2012-03-28 |
| Compiler | MSVC 11.00 (VS2012) | MSVC 11.00 (VS2012) |
| Ship/client | ship | ship |
| Platform target | x86 | x64 |

The x64 binary appears to be from an **earlier development milestone** that shipped with the original Windows Live Photo Gallery 2012 release, while the x86 binary was recompiled for a later update (possibly a Windows 8.1 or 10 era update given the 2014 timestamp).
