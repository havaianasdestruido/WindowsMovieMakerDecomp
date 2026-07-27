# MetadataSys.dll — Static Analysis

## Overview

**MetadataSys.dll** is a small (~44 KB) ATL COM in-proc server implementing the Windows Property System integration layer for photo metadata in Windows Live Photo Gallery 2012 (Windows Live Essentials 16.4.3528.0331). It provides a custom `IPropertyStore` implementation (`CSafePropertyStore`) that bridges EXIF/XMP/IPTC metadata to the Windows Shell property system, and exposes a key export `WLXPSGetItemPropertyHandler` used by other DLLs to access metadata properties.

---

## PE Structure

| Field | Value |
|---|---|
| **File** | `MetadataSys.dll` |
| **Arch** | x86 (14C) |
| **Subsystem** | Windows GUI (2) |
| **Linker** | Visual Studio 2012 (11.00) |
| **Entry Point** | `0x5A97` |
| **Image Base** | `0x10000000` |
| **Image Size** | `0xB000` (45,056 bytes) |
| **Code Size** | `0x6A00` (27,136 bytes) |
| **OS Target** | 6.02 (Windows 8) |
| **Image Version** | 6.02 |
| **Timestamp** | Tue Apr 1 01:28:22 2014 |
| **Checksum** | `0x1951F` |
| **DLL Characteristics** | `0x140` — Dynamic Base, NX Compatible |
| **Debug PDB** | `MetadataSys.pdb` `{E4825AAB-EBDD-4719-8B0D-2A0A863B7B79}` |

### Sections

| Section | Virtual Size | Virtual Address | Raw Size | Flags |
|---------|-------------|----------------|----------|-------|
| `.text` | `0x697E` | `0x10001000` | `0x6A00` | Code, Execute Read |
| `.data` | `0x7C0` | `0x10008000` | `0x200` | Initialized Data, Read Write |
| `.rsrc` | `0x3E8` | `0x10009000` | `0x400` | Initialized Data, Read Only |
| `.reloc` | `0xCA6` | `0x1000A000` | `0xE00` | Initialized Data, Discardable, Read Only |

### Directories

| Directory | RVA | Size |
|-----------|-----|------|
| Export | `0x78B0` | `0xCE` |
| Import | `0x6FD0` | `0xA0` |
| Resource | `0x9000` | `0x3E8` |
| Debug | `0x11A0` | `0x38` |
| Base Reloc | `0xA000` | `0x6BC` |
| Load Config | `0x19C8` | `0x40` |
| IAT | `0x1000` | `0x17C` |
| Delay Import | `0x6F50` | `0x40` |
| Certificates | `0x8200` | `0x3EC8` |

---

## Export Table

5 exports (standard COM + 1 custom):

| Ordinal | Hint | Name | Description |
|---------|------|------|-------------|
| 2 | 0 | `DllCanUnloadNow` | COM server reference counting |
| 3 | 1 | `DllGetClassObject` | COM class factory |
| 4 | 2 | `DllRegisterServer` | Self-registration |
| 5 | 3 | `DllUnregisterServer` | Self-unregistration |
| 1 | 4 | `WLXPSGetItemPropertyHandler` | **Custom export** — returns IPropertyStore for a given item |

### WLXPSGetItemPropertyHandler

The custom export `WLXPSGetItemPropertyHandler` (ordinal 1, RVA `0x4CA5`) is the primary entry point for metadata access. This function is delay-loaded by `WLXImageTranscode.dll` and provides a `IPropertyStore` wrapper around photo metadata. The function signature (inferred from usage in WLXImageTranscode) is:

```cpp
HRESULT WLXPSGetItemPropertyHandler(
    IUnknown* pItem,           // Shell item or similar
    DWORD dwAccessMode,        // Read/Write
    REFIID riid,               // IID_IPropertyStore
    void** ppv                 // Output: IPropertyStore*
);
```

---

## Import Table

### MSVCR110.dll — Visual C++ 2012 Runtime
| Category | Functions |
|----------|-----------|
| Memory | `malloc`, `free`, `_recalloc`, `_calloc_crt`, `_malloc_crt` |
| CRT Init | `_initterm`, `_initterm_e`, `_amsg_exit`, `__CppXcptFilter`, `__CxxFrameHandler3` |
| C++ | `operator new`, `operator delete`, `operator delete[]`, `_purecall`, `terminate`, `type_info::~type_info`, `_CxxThrowException` |
| String | `wcscat_s`, `wcscpy_s`, `wcsncpy_s`, `wcsstr` |
| Security | `_except_handler4_common`, `_crt_debugger_hook`, `__crtUnhandledException`, `__crtTerminateProcess`, `__clean_type_info_names_internal` |
| Thread | `_lock`, `_unlock`, `__dllonexit`, `_onexit` |

### KERNEL32.dll — Core OS Services
| Category | Functions |
|----------|-----------|
| Synchronization | `InterlockedIncrement`, `InterlockedDecrement`, `InterlockedExchange`, `EnterCriticalSection`, `LeaveCriticalSection`, `InitializeCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `DeleteCriticalSection` |
| PE Loading | `LoadLibraryExW`, `LoadLibraryExA`, `LoadLibraryW`, `FreeLibrary`, `GetModuleHandleA`, `GetModuleHandleW`, `GetModuleFileNameW`, `GetProcAddress` |
| Resources | `FindResourceW`, `LoadResource`, `SizeofResource` |
| String/Encoding | `MultiByteToWideChar`, `lstrcmpiW`, `CompareStringW` |
| Timing | `GetTickCount64`, `GetSystemTimeAsFileTime`, `QueryPerformanceCounter` |
| Error/Debug | `GetLastError`, `SetLastError`, `RaiseException`, `IsDebuggerPresent`, `OutputDebugStringA` |
| Misc | `GetVersion`, `GetVersionExW`, `GetFileAttributesW`, `EncodePointer`, `DecodePointer`, `DisableThreadLibraryCalls`, `IsProcessorFeaturePresent`, `GetCurrentThreadId` |

### USER32.dll
| Function | Purpose |
|----------|---------|
| `CharNextW` | String cursor advancement |

### ADVAPI32.dll — Registry Operations
| Function | Purpose |
|----------|---------|
| `RegOpenKeyExW` | Open registry key |
| `RegCreateKeyExW` | Create/open registry key |
| `RegSetValueExW` | Set registry value |
| `RegQueryValueExW` (via `RegQueryInfoKeyW`) | Query registry metadata |
| `RegDeleteKeyW` | Delete registry key |
| `RegDeleteValueW` | Delete registry value |
| `RegEnumKeyExW` | Enumerate subkeys |
| `RegQueryInfoKeyW` | Query key metadata |
| `RegCloseKey` | Close registry handle |

### ole32.dll — COM Foundation
| Function | Purpose |
|----------|---------|
| `CoCreateInstance` | Create COM objects |
| `CoTaskMemAlloc`, `CoTaskMemFree`, `CoTaskMemRealloc` | COM task memory |
| `StringFromGUID2` | GUID-to-string conversion |
| `PropVariantClear` | PROPVARIANT cleanup |
| `PropVariantCopy` | PROPVARIANT copy |

### SHLWAPI.dll
| Function | Purpose |
|----------|---------|
| `PathFindExtensionW` | File extension extraction |

### PROPSYS.dll — Windows Property System
| Function | Purpose |
|----------|---------|
| `PSCoerceToCanonicalValue` | Coerce property value to canonical type |
| `PSGetItemPropertyHandler` | Get property handler for shell item |

### OLEAUT32.dll (Delay-Loaded)
| Ordinal | Likely Function |
|---------|-----------------|
| 277 | `SystemTimeToVariantTime` |
| 186 | `VarDateFromStr` or similar |
| 2 | `SysAllocString` |
| 161 | `VariantTimeToSystemTime` |
| 163 | `SafeArrayRedim` |
| 7 | `VariantInit` |
| 6 | `SysStringLen` |

---

## COM Class Registration

### Data Section RTTI Classes

| Class | Description |
|-------|-------------|
| `CSafePropertyStore` | `IPropertyStore` implementation — wraps metadata access with thread safety |
| `CComModule` (ATL) | ATL module infrastructure |
| `CAtlModule` / `CAtlModuleT<CComModule>` | ATL module hierarchy |
| `CRegObject` (ATL) | ATL registry object for COM self-registration |
| `CAtlException` | ATL exception wrapper |
| `IRegistrarBase` | ATL registrar interface |
| `IUnknown` | Base COM interface |
| `type_info` | Standard C++ RTTI |

### Key Insight: CSafePropertyStore

The `CSafePropertyStore` class implements `IPropertyStore` (Windows Shell interface for property access). This is the bridge between:
- **Photo metadata** (EXIF, XMP, IPTC embedded in image files)
- **Windows Shell property system** (System.Photo.*, System.Photo.CameraModel, etc.)

The "Safe" prefix indicates thread-safe property access — critical for the multi-threaded imaging pipeline.

---

## Analysis Summary

### Architecture

1. **COM Registration**: Registers COM classes for property system integration
2. **Property Handler**: `WLXPSGetItemPropertyHandler` is the main entry point — creates a `CSafePropertyStore` wrapping photo metadata
3. **Windows Property System**: Uses `PSCoerceToCanonicalValue` to normalize metadata values and `PSGetItemPropertyHandler` for shell item access
4. **Registry Persistence**: Full ADVAPI32 registry API for reading/writing property values to the Windows registry
5. **PROPVIARIANT-based**: Uses `PropVariantClear`/`PropVariantCopy` for typed property values

### Key Design Points

- **Small and focused** — only 27 KB of code, serving as a thin adapter between photo metadata and Windows Shell properties
- **Custom export `WLXPSGetItemPropertyHandler`** — not a standard COM method; called directly by other DLLs (WLXImageTranscode delay-loads this)
- **Thread-safe property store** — `CSafePropertyStore` wrapping enables concurrent access from the imaging pipeline
- **Delay-loaded OLEAUT32** — VARIANT/SAFEARRAY operations are optional; the DLL can function for simple property reads without automation
- **ATL COM infrastructure** — uses `CComModule`, `CRegObject` for standard COM registration
- **No WMI, no GPU, no threads** — purely synchronous property access

### Dependency Chain

```
WLXPhotoGallery.exe
  └─ WLXImageTranscode.dll (delay-loads MetadataSys.dll)
       └─ MetadataSys.dll
            ├─ PROPSYS.dll (Windows Property System)
            ├─ ole32.dll (COM)
            ├─ ADVAPI32.dll (Registry)
            └─ MSVCR110.dll (CRT)
```

### Role in Photo Gallery Ecosystem

MetadataSys.dll acts as the **metadata translation layer**:
1. When a photo is opened, `WLXImageTranscode.dll` calls `WLXPSGetItemPropertyHandler` to get property access
2. `CSafePropertyStore` reads EXIF/XMP/IPTC data from the image file
3. Properties are exposed through the standard Windows `IPropertyStore` interface
4. The Gallery shell UI reads properties via `PSFormatForDisplay`, `PSCoerceToCanonicalValue`
5. Metadata changes (rating, tags, date) flow back through `IPropertyStore::SetValue`

---

## Runtime Test Results

Test harness: `tests/OtherDlls/test_regres_imaging_metadatasys.cpp`

| Export | Result |
|--------|--------|
| `DllCanUnloadNow` | `S_OK` (0x00000000) — no outstanding references |
| `DllGetClassObject` (empty CLSID) | SEH exception `0xC0000005` (ACCESS_VIOLATION) — null-check crash in class factory |
| `DllRegisterServer` | Found at `0x71411DF8` (skipped — would modify registry) |
| `DllUnregisterServer` | Found at `0x71411DE9` (skipped — would modify registry) |
| `WLXPSGetItemPropertyHandler` | Found at `0x71414CA5` — returns `0x80004003` (E_POINTER) with null args (expected) |
