# PhotoViewerShimx64.dll - Static Analysis

## Overview
**PhotoViewerShimx64.dll** is the **64-bit (x64) version** of the PhotoViewerShim COM in-process server. Like its x86 counterpart, it implements OLE Drag-and-Drop target shims for Windows Photo Gallery, but targeting 64-bit processes. It is an ATL-based COM DLL that registers drop targets allowing files dragged from Windows Explorer to be handled by Photo Gallery components.

**PDB:** `PhotoViewerShim.pdb` (GUID: `{D15C59F9-BA59-4081-B04C-7365AD9AE382}`)

**Note:** The export directory header string says "DropTargetShim.dll" - the original module name. Same as the x86 version.

**File Size:** 61,632 bytes (vs 32 KB for x86 - ~2x larger due to x64 pointer size)

## PE Header
| Field | Value |
|-------|-------|
| Machine | x64 (0x8664) |
| Linker | MSVC 11.00 (VS2012) |
| Timestamp | 2012-03-28 16:51:00 UTC |
| Image Base | 0x0000000180000000 |
| Size of Image | 0xF000 (60 KB) |
| Entry Point | 0x7F90 |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | High Entropy VA, Dynamic base, NX compatible |
| Large Address Aware | Yes (>2GB) |

**Notable**: This DLL was built in **2012**, 2 years earlier than the other DLLs (2014). This suggests the 64-bit shim was finalized earlier, likely for the Windows 8 / Photo Gallery 2012 release.

## Sections
| Name | VirtAddr | VirtSize | RawSize | Flags |
|------|----------|----------|---------|-------|
| .text | 0x1000 | 0x93EB | 0x9400 | Code, Execute Read |
| .data | 0xB000 | 0xE10 | 0x800 | Initialized Data, Read Write |
| .pdata | 0xC000 | 0x540 | 0x600 | Initialized Data, Read Only |
| .rsrc | 0xD000 | 0x6A0 | 0x800 | Initialized Data, Read Only |
| .reloc | 0xE000 | 0x30A | 0x400 | Discardable, Read Only |

**Note**: Has a `.pdata` section (exception data) not present in the x86 version - required for x64 structured exception handling.

## Exports (Standard COM In-Process Server)
| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | 0x3660 | `DllCanUnloadNow` |
| 2 | 1 | 0x3674 | `DllGetClassObject` |
| 3 | 2 | 0x37A4 | `DllRegisterServer` |
| 4 | 3 | 0x3860 | `DllUnregisterServer` |

**Identical** to the x86 version - four standard COM DLL exports.

## Imports

### MSVCR110.dll (Visual C++ 2012 Runtime)
Full CRT: `_purecall`, `operator new/delete/new[]/delete[]`, `malloc`, `free`, `_recalloc`, `memset`, `memcpy_s`, `wcsstr`, `wcscat_s`, `wcscpy_s`, `wcsncpy_s`, `_CxxThrowException`, `__CxxFrameHandler3`, `__C_specific_handler`, `terminate`, `?_type_info_dtor_internal_method@type_info@@QEAAXXZ` (type_info destructor), `_initterm`, `_initterm_e`, `_malloc_crt`, `_calloc_crt`, `_lock`, `_unlock`, `__dllonexit`, `_onexit`, `_crt_debugger_hook`, `__crtUnhandledException`, `__crtTerminateProcess`, `__crtCaptureCurrentContext`, `__crtCapturePreviousContext`, `__clean_type_info_names_internal`, `_amsg_exit`, `__CppXcptFilter`.

### KERNEL32.dll
Full set: `GetTickCount64`, `GetSystemTimeAsFileTime`, `GetCurrentThreadId`, `QueryPerformanceCounter`, `IsProcessorFeaturePresent`, `IsDebuggerPresent`, `CreateEventW`, `CloseHandle`, `LoadLibraryExW`, `FindResourceW`, `LoadResource`, `SizeofResource`, `lstrcmpiW`, `FreeLibrary`, `GetModuleFileNameW`, `GetModuleHandleW`, `GetProcAddress`, `InitializeCriticalSectionAndSpinCount`, `GetLastError`, `LeaveCriticalSection`, `EnterCriticalSection`, `EncodePointer`, `DecodePointer`, `RaiseException`, `DeleteCriticalSection`, `GetThreadLocale`, `SetThreadLocale`, `MultiByteToWideChar`.

### ADVAPI32.dll (Registry)
`RegDeleteValueW`, `RegCreateKeyExW`, `RegSetValueExW`, `RegEnumKeyExW`, `RegOpenKeyExW`, `RegQueryInfoKeyW`, `RegCloseKey`, `RegDeleteKeyW`.

**Note**: Unlike the x86 version, there are **no transacted registry operations** (`RegOpenKeyTransactedW` etc.) - these were only added in the later x86 build.

### ole32.dll (COM Infrastructure)
`CoTaskMemAlloc`, `CoCreateGuid`, `StringFromCLSID`, `PropVariantClear`, `CoAllowSetForegroundWindow`, `CoTaskMemFree`, `CoTaskMemRealloc`, `StringFromGUID2`, `CoCreateInstance`.

### OLEAUT32.dll
Ordinal 277 only (`SafeArrayUnaccessData`).

### USER32.dll (Window Message Loop)
`CharNextW`, `PostQuitMessage`, `DispatchMessageW`, `TranslateMessage`, `PeekMessageW`, `MsgWaitForMultipleObjects`.

**Note**: No `MsgWaitForMultipleObjectsEx` (the Ex variant used in x86 version) - slightly simpler message loop.

## RTTI Class Hierarchy

Identical class hierarchy to x86 version:

### ATL Infrastructure Classes
| Class | Description |
|-------|-------------|
| `CAtlException<ATL>` | ATL exception class |
| `CAtlModule<ATL>` | ATL module base |
| `_ATL_MODULE70<ATL>` | ATL module v70 structure |
| `CAtlDllModuleT<DropTargetShimModule>` | DLL module template |
| `CAtlModuleT<DropTargetShimModule>` | Module template specialization |
| `CAtlValidateModuleConfiguration<1, DropTargetShimModule>` | Config validator |
| `CRegObject<ATL>` | Registry object |
| `CComClassFactory<ATL>` | COM class factory |
| `CComObjectRootEx<CComMultiThreadModel<ATL>>` | Multi-threaded COM root |
| `CComObjectRootBase<ATL>` | COM root base |
| `CComObjectCached<CComClassFactory<ATL>>` | Cached class factory |

### Core Module
| Class | Description |
|-------|-------------|
| `DropTargetShimModule` | Main ATL module class |

### COM Interfaces
| Interface | Description |
|-----------|-------------|
| `IUnknown` | Base COM interface |
| `IRegistrarBase` | ATL registration interface |
| `IClassFactory` | COM class factory |
| `IDropTarget` | OLE drag-and-drop target |
| `IObjectWithSite` | Object with site (container) reference |

### Drop Target Shim Classes
| Class | Description | CLSID |
|-------|-------------|-------|
| `ViewerDropTargetShim` | Drop target for Photo Viewer | `CLSID_PhotoViewerInprocShim` |
| `EditorDropTargetShim` | Drop target for Photo Editor | (separate CLSID) |
| `ViewerAutoplayDropTargetShim` | AutoPlay drop target for Viewer | (separate CLSID) |
| `ImportAutoplayDropTargetShim` | AutoPlay drop target for Import | (separate CLSID) |

All wrapped as `CComObject<>` instances.

### COM Coclass Association
```
CComCoClass<ViewerDropTargetShim, &CLSID_PhotoViewerInprocShim>
```

## Registered COM Objects
```reg
HKCR\CLSID
    ForceRemove '{00f346cb-35a4-465b-8b8f-65a29dbab1f6}' = s 'Live Shell Viewer Extension'
    {
        InprocServer32 = s '%MODULE%'
        {
            val ThreadingModel = s 'Apartment'
        }
    }
    ForceRemove '{00f3712a-ca79-45b4-9e4d-d7891e7f8b9d}' = s 'Live Shell Editor Extension'
    {
        InprocServer32 = s '%MODULE%'
        {
            val ThreadingModel = s 'Apartment'
        }
    }
```

**Same CLSIDs as the x86 version** - the 32-bit and 64-bit shims register under the same CLSIDs with Wow6432Node redirection handling the architecture distinction.

## Digital Signatures
Signed by Microsoft Corporation:
1. Microsoft Code Signing PCA 2011 (certificate chain)
2. Microsoft Time-Stamp PCA 2010
3. Microsoft Root Certificate Authority 2011

nCipher DSE ESN: `C0F4-3086-DEF8`
Time-stamped: 2014-04-01 04:34:17 UTC

**Note**: Even though the code was compiled in 2012, it was re-signed with a new timestamp in 2014 - indicating it was included in the 2014 release wave (Windows Essentials 2014).

## x86 vs x64 Comparison

| Property | x86 (PhotoViewerShim.dll) | x64 (PhotoViewerShimx64.dll) |
|----------|---------------------------|-------------------------------|
| Machine | x86 (0x14C) | x64 (0x8664) |
| Code Size | ~32 KB | ~60 KB |
| Compile Date | 2014-04-01 | 2012-03-28 |
| PDB GUID | `{DDB3B186-F89C-482C-83A7-9FDD8F0F52AE}` | `{D15C59F9-BA59-4081-B04C-7365AD9AE382}` |
| .pdata Section | No | Yes (required for x64) |
| High Entropy VA | No | Yes (x64 security) |
| Transacted Registry | Yes | No |
| COM Interfaces | Identical | Identical |
| Registered CLSIDs | Identical | Identical |
| RTTI Classes | Identical | Identical |

## Architecture
```
Windows Explorer (64-bit Shell)
  |
  | OLE Drag-and-Drop (IDropTarget)
  v
PhotoViewerShimx64.dll (COM InProc Server, 64-bit)
  |
  |-- ViewerDropTargetShim (IDropTarget)
  |-- EditorDropTargetShim (IDropTarget)
  |-- ViewerAutoplayDropTargetShim (IDropTarget)
  |-- ImportAutoplayDropTargetShim (IDropTarget)
  |
  v
CoCreateInstance / SendMessage
  |
  v
Windows Live Photo Gallery 64-bit (or photoviewer.dll x64)
```

## How It Works
1. **Registration**: `DllRegisterServer` writes CLSID entries (same as x86)
2. **Wow64 Redirection**: 64-bit Explorer loads the x64 shim from System32; 32-bit Explorer loads the x86 shim from SysWOW64
3. **Activation**: Shell queries the registered CLSID for the appropriate architecture
4. **Factory**: `DllGetClassObject` creates `CComClassFactory` for the requested CLSID
5. **Instantiation**: Factory creates the appropriate shim class
6. **Drop Handling**: `IDropTarget::Drop()` forwards data to Photo Gallery
7. **Message Pump**: USER32 message functions handle OLE drag-drop message loop

## Key Observations
1. **64-bit twin** of PhotoViewerShim.dll - identical functionality, x64 binary
2. **Same CLSIDs** as x86 version - Wow6432Node handles architecture dispatch
3. **Compiled 2 years earlier** (2012 vs 2014) but re-signed for the 2014 release
4. **~2x larger** than x86 version (62 KB vs 32 KB) due to x64 pointer size and alignment
5. **High Entropy VA** enabled - x64 security feature for ASLR
6. **.pdata section** present - x64 exception handling unwind data
7. **No transacted registry** operations - simpler registry code than x86 version
8. **Original name**: Export directory says "DropTargetShim.dll"
9. **Pure COM forwarding shim** - no actual photo processing
10. **Same RTTI hierarchy** as x86 - DropTargetShimModule, ViewerDropTargetShim, EditorDropTargetShim, etc.
11. **ATL-based** (Active Template Library v7.0)
12. **MSVC 2012** toolchain (linker 11.00) - same as all other binaries
13. **Apartment-threaded COM** despite using `CComMultiThreadModel`
