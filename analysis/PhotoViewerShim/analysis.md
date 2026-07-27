# PhotoViewerShim.dll - Static Analysis

## Overview
**PhotoViewerShim.dll** is a **COM in-process server** that implements **OLE Drag-and-Drop target shims** for Windows Photo Gallery. It is a thin ATL-based COM DLL that registers drop targets allowing files dragged from Windows Explorer to be handled by Photo Gallery components. The "shim" layer intercepts OLE `IDropTarget` calls and routes them to the appropriate Photo Gallery COM objects.

**PDB:** `PhotoViewerShim.pdb` (GUID: `{DDB3B186-F89C-482C-83A7-9FDD8F0F52AE}`)

**Note:** The export directory header string says "DropTargetShim.dll" - this is the original module name before renaming.

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Linker | MSVC 11.00 (VS2012) |
| Timestamp | 2014-04-01 01:28:31 UTC |
| Image Base | 0x10000000 |
| Size of Image | 0xC000 (48 KB) |
| Entry Point | 0x6BC9 |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | Dynamic base, NX compatible |

## Sections
| Name | VirtAddr | VirtSize | RawSize | Flags |
|------|----------|----------|---------|-------|
| .text | 0x1000 | 0x7BAB | 0x7C00 | Code, Execute Read |
| .data | 0x9000 | 0x954 | 0x600 | Initialized Data, Read Write |
| .rsrc | 0xA000 | 0x658 | 0x800 | Initialized Data, Read Only |
| .reloc | 0xB000 | 0xB02 | 0xC00 | Discardable, Read Only |

## Exports (Standard COM In-Process Server)
| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | 0x3595 | `DllCanUnloadNow` |
| 2 | 1 | 0x35AE | `DllGetClassObject` |
| 3 | 2 | 0x35F8 | `DllRegisterServer` |
| 4 | 3 | 0x360A | `DllUnregisterServer` |

These are the four standard COM DLL exports required for an in-process COM server:
- **DllCanUnloadNow**: Returns `S_OK` if the DLL's COM server can be safely unloaded (ref count == 0)
- **DllGetClassObject**: Returns the class factory for a given CLSID
- **DllRegisterServer**: Self-registers the COM objects in the Windows registry
- **DllUnregisterServer**: Removes COM registrations from the registry

## Imports
### MSVCR110.dll (Visual C++ 2012 Runtime)
Standard CRT: `_except_handler4_common`, `__clean_type_info_names_internal`, `operator delete`, `__crtTerminateProcess`, `__crtUnhandledException`, `_crt_debugger_hook`, `_onexit`, `__dllonexit`, `_calloc_crt`, `operator delete`, `_lock`, `_unlock`, `terminate`, `_initterm_e`, `_initterm`, `_malloc_crt`, `_amsg_exit`, `__CppXcptFilter`, `memset`, `__CxxFrameHandler3`, `operator new[]`, `_recalloc`, `malloc`, `wcsstr`, `memcpy_s`, `operator new`, `_purecall`, `_CxxThrowException`, `free`, `wcsncpy_s`, `wcscpy_s`, `wcscat_s`, `operator delete[]`

### KERNEL32.dll
`GetTickCount64`, `GetSystemTimeAsFileTime`, `GetCurrentThreadId`, `QueryPerformanceCounter`, `IsProcessorFeaturePresent`, `IsDebuggerPresent`, `MultiByteToWideChar`, `FindResourceW`, `lstrcmpiW`, `SizeofResource`, `LoadResource`, `LoadLibraryExW`, `FreeLibrary`, `CreateEventW`, `SetThreadLocale`, `GetThreadLocale`, `GetProcAddress`, `GetModuleHandleW`, `GetModuleFileNameW`, `InterlockedDecrement`, `InterlockedIncrement`, `DeleteCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `LeaveCriticalSection`, `EncodePointer`, `DecodePointer`, `RaiseException`, `GetLastError`, `EnterCriticalSection`, `CloseHandle`

### ADVAPI32.dll (Registry)
`RegSetValueExW`, `RegEnumKeyExW`, `RegDeleteValueW`, `RegCreateKeyExW`, `RegQueryInfoKeyW`, `RegOpenKeyExW`, `RegDeleteKeyW`, `RegCloseKey`

### ole32.dll (COM Infrastructure)
`CoAllowSetForegroundWindow`, `CoTaskMemFree`, `CoTaskMemRealloc`, `CoTaskMemAlloc`, `PropVariantClear`, `CoCreateGuid`, `StringFromCLSID`, `StringFromGUID2`, `CoCreateInstance`

### OLEAUT32.dll (OLE Automation, by ordinal)
Ordinals 7, 161, 2, 6, 186, 277, 163 (SysFreeString, SysAllocStringLen, SysStringLen, SysAllocString, VariantClear, SafeArrayAccessData, SysReAllocString)

### USER32.dll (Window Message Loop)
`DispatchMessageW`, `PeekMessageW`, `PostQuitMessage`, `MsgWaitForMultipleObjects`, `TranslateMessage`, `CharNextW`

### Additional Registry/TypeLib Imports
`RegOpenKeyTransactedW`, `RegDeleteKeyTransactedW`, `RegDeleteKeyExW`, `UnRegisterTypeLibForUser`, `RegisterTypeLibForUser`, `RegCreateKeyTransactedW`

## RTTI Class Hierarchy

The DLL contains rich RTTI information revealing the complete class hierarchy:

### ATL Infrastructure Classes
| Class | Description |
|-------|-------------|
| `CAtlException<ATL>` | ATL exception class |
| `CAtlModule<ATL>` | ATL module base |
| `_ATL_MODULE70<ATL>` | ATL module v70 structure |
| `CAtlDllModuleT<DropTargetShimModule>` | DLL module template |
| `CAtlModuleT<DropTargetShimModule>` | Module template specialization |
| `CAtlValidateModuleConfiguration<1, DropTargetShimModule>` | Module config validator |
| `CRegObject<ATL>` | Registry object |
| `CComClassFactory<ATL>` | COM class factory |
| `CComObjectRootEx<CComMultiThreadModel<ATL>>` | Multi-threaded COM root |
| `CComObjectRootBase<ATL>` | COM root base |
| `CComObjectCached<CComClassFactory<ATL>>` | Cached class factory |

### Core Module Class
| Class | Description |
|-------|-------------|
| `DropTargetShimModule` | Main ATL module class managing all COM objects |

### COM Interfaces Implemented
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
| `ViewerDropTargetShim` | Drop target for Photo Viewer | InprocServer32 |
| `EditorDropTargetShim` | Drop target for Photo Editor | (separate CLSID) |
| `ViewerAutoplayDropTargetShim` | AutoPlay drop target for Viewer | (separate CLSID) |
| `ImportAutoplayDropTargetShim` | AutoPlay drop target for Import | (separate CLSID) |

All four are `CComObject<>` implementations wrapping the respective shim classes.

### COM Coclass Association
```
CComCoClass<ViewerDropTargetShim, &CLSID_PhotoViewerInprocShim>
```
The `ViewerDropTargetShim` is directly associated with `CLSID_PhotoViewerInprocShim`.

## Registered COM Objects (Registry Script in Resources)

The DLL contains an embedded REGSCRIPT (ATL registry script) for self-registration:

```reg
HKCR
    NoRemove CLSID
    {
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
    }
```

| CLSID | Description | Threading |
|-------|-------------|-----------|
| `{00f346cb-35a4-465b-8b8f-65a29dbab1f6}` | Live Shell Viewer Extension | Apartment |
| `{00f3712a-ca79-45b4-9e4d-d7891e7f8b9d}` | Live Shell Editor Extension | Apartment |

Both are registered as `InprocServer32` with Apartment threading model.

## Architecture
```
Windows Explorer (Shell)
  |
  | OLE Drag-and-Drop (IDropTarget)
  v
PhotoViewerShim.dll (COM InProc Server)
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
Windows Live Photo Gallery (wlarp.exe / photoviewer.dll)
```

## How It Works
1. **Registration**: `DllRegisterServer` writes the CLSID entries to `HKCR\CLSID`
2. **Activation**: When a user drags files onto a Photo Gallery drop target, the Shell queries the registered CLSID
3. **Factory**: `DllGetClassObject` creates a `CComClassFactory` for the requested CLSID
4. **Instantiation**: The factory creates the appropriate `ViewerDropTargetShim` / `EditorDropTargetShim` etc.
5. **Drop Handling**: The shim implements `IDropTarget` (DragEnter, DragOver, DragLeave, Drop) and forwards the data to the actual Photo Gallery component
6. **Site**: Uses `IObjectWithSite` to get a reference to its hosting container
7. **Message Pump**: The USER32 imports (PeekMessage, TranslateMessage, DispatchMessage, MsgWaitForMultipleObjects) suggest the shim processes window messages during drag operations (OLE requires a message loop during drag-drop)

## Key Observations
1. **Pure COM forwarding shim**: The DLL does no actual photo processing - it's a routing layer
2. **ATL-based**: Uses Active Template Library v7.0 for lightweight COM infrastructure
3. **Multi-threaded COM**: Uses `CComMultiThreadModel` but registers with `Apartment` threading
4. **Registry-heavy**: Extensive use of ADVAPI32 for COM registration, plus transacted registry operations
5. **Message loop**: Imports USER32 message functions for OLE drag-drop message pumping
6. **Four drop targets**: Viewer, Editor, Viewer AutoPlay, Import AutoPlay - covering all drag-drop scenarios
7. **Original name**: Export directory says "DropTargetShim.dll" - renamed to PhotoViewerShim.dll
8. **Two registered CLSIDs**: Viewer Extension and Editor Extension
9. **No delay imports**: All imports resolved at load time
10. **MSVC 2012**: Built with Visual Studio 2012 (MSVC 11.0)
11. **6.5x larger than NPWLPG**: ~32KB code vs ~4KB, reflecting ATL/COM infrastructure overhead
