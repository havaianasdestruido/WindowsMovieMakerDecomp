# WLXPhotoClassic.dll Analysis

## Overview

Classic photo viewer / slideshow theme DLL from Windows Live Photo Gallery 2012. Standard ATL COM in-process server implementing slideshow transition themes with GPU pipeline support via Direct3D 9.

| Field | Value |
|-------|-------|
| **File** | WLXPhotoClassic.dll |
| **Image** | ~92 KB (0x16000) |
| **Type** | PE32 x86, DLL |
| **Linker** | MSVC 11.00 (VS 2012) |
| **OS/Subsystem Ver** | 6.02 / 6.00 (Windows 8) |
| **Subsystem** | Windows GUI (2) |
| **Image Base** | 0x10000000 |
| **Entry Point** | 0x1000B3C1 |
| **Timestamp (PE)** | 2014-04-01 01:26:49 UTC |
| **Timestamp (Export)** | 2014-04-01 01:13:58 UTC |
| **DLL Characteristics** | Dynamic Base (ASLR), NX Compatible |
| **Checksum** | 0x19FBA |
| **PDB** | `WLXPhotoClassic.pdb`, GUID `{CB29A460-1C0D-4947-A59B-8CFC2260BBED}` |
| **Build** | 16.4.3528.0331 (ship) |
| **Code Signing** | Microsoft Code Signing PCA 2010/2011, multiple timestamps |

### Version Information

| Property | Value |
|----------|-------|
| CompanyName | Microsoft Corporation |
| FileDescription | (embedded, read as "Photo Gallery") |
| FileVersion | 16.4.3528.0331 |
| ProductName | Photo Gallery |
| OriginalFilename | WLXPhotoClassic.dll |

## PE Sections

| Section | VA | VirtSize | RawSize | Flags |
|---------|------|----------|---------|-------|
| `.text` | 0x1000 | 0xD77C (54 KB) | 0xD800 (54 KB) | Code, Execute+Read |
| `.data` | 0xF000 | 0x1058 (4.1 KB) | 0xC00 (3 KB) | Init Data, Read+Write |
| `.rsrc` | 0x11000 | 0x2048 (8 KB) | 0x2200 (8.5 KB) | Init Data, Read Only |
| `.reloc` | 0x14000 | 0x1628 (5.5 KB) | 0x1800 (6 KB) | Init Data, Discardable, RO |

## Export Table

4 exports — standard ATL COM DLL pattern, ordinal base 1:

| Ordinal | Hint | RVA | Name |
|---------|------|------|------|
| 1 | 0 | 0x30AB | `DllCanUnloadNow` |
| 2 | 1 | 0x309B | `DllGetClassObject` |
| 3 | 2 | 0x30CB | `DllRegisterServer` |
| 4 | 3 | 0x30BC | `DllUnregisterServer` |

No additional factory/creator exports — all functionality accessed through COM interfaces.

## Import Table (8 DLLs)

### MSVCR110.dll (31 functions)
C++ CRT: memory (`malloc`, `free`, `_calloc_crt`, `_recalloc`), string (`wcsncpy_s`, `wcscat_s`, `wcscpy_s`, `wcsstr`, `wcsnlen`, `wmemcpy_s`, `memmove_s`, `memset`, `memcpy_s`), CRT init (`_initterm`, `_initterm_e`, `_dllonexit`, `_onexit`), exception handling (`_except_handler4_common`, `__CppXcptFilter`, `__CxxFrameHandler3`), C++ runtime (`?terminate@@YAXXZ`, `??1type_info@@UAE@XZ`, `__clean_type_info_names_internal`), error (`_amsg_exit`, `_purecall`), conversion (`_itow_s`), debug (`_crt_debugger_hook`, `__crtTerminateProcess`, `__crtUnhandledException`).

### KERNEL32.dll (33 functions)
Module loading (`LoadLibraryW`, `LoadLibraryExW`, `FreeLibrary`, `GetModuleHandleW`, `GetProcAddress`), resources (`FindResourceExW`, `FindResourceW`, `LoadResource`, `LockResource`, `SizeofResource`), string (`ExpandEnvironmentStringsW`, `MultiByteToWideChar`, `lstrcmpiW`), threading (`DisableThreadLibraryCalls`, `GetCurrentThreadId`, `InitializeCriticalSectionAndSpinCount`, `EnterCriticalSection`, `LeaveCriticalSection`, `DeleteCriticalSection`, `InterlockedIncrement`, `InterlockedDecrement`), time (`GetSystemTimeAsFileTime`, `GetTickCount64`, `QueryPerformanceCounter`), process (`GetModuleFileNameW`, `MulDiv`, `RaiseException`), pointer safety (`EncodePointer`, `DecodePointer`), debug (`IsDebuggerPresent`, `IsProcessorFeaturePresent`), error (`GetLastError`).

### USER32.dll (2 functions)
Minimal — `DestroyWindow`, `CharNextW`.

### ADVAPI32.dll (9 functions)
Registry operations: `RegOpenKeyExW`, `RegCreateKeyExW`, `RegCloseKey`, `RegDeleteKeyW`, `RegDeleteValueW`, `RegSetValueExW`, `RegQueryValueExW`, `RegEnumKeyExW`, `RegQueryInfoKeyW`. Used for COM self-registration and configuration.

### ole32.dll (7 functions)
COM: `CoCreateInstance`, `CoTaskMemAlloc`, `CoTaskMemRealloc`, `CoTaskMemFree`, `StringFromCLSID`, `StringFromGUID2`, `PropVariantClear`.

### OLEAUT32.dll (1 function)
Ordinal 277 only — likely `SysFreeString` or `SysAllocString`.

### WLXPhotoBase.dll (6 functions)
Base framework imports (C++ mangled):

| Mangled Name | Demangled |
|-------------|-----------|
| `?Throw@Base@@YGXJ@Z` | `void __stdcall Base::Throw(long)` |
| `?BaseAtlThrow@ATL@@YGXJ@Z` | `void __stdcall ATL::BaseAtlThrow(long)` |
| `??1Exception@Base@@UAE@XZ` | `Base::Exception::~Exception()` |
| `?New@BasePrivate@@YAPAXI_N@Z` | `void* __cdecl BasePrivate::New(unsigned int, bool)` |
| `?GetBaseStringManager@String@Base@@SGAAVCAtlStringMgr@ATL@@XZ` | `ATL::CAtlStringMgr& __stdcall Base::String::GetBaseStringManager()` |
| `?Delete@BasePrivate@@YAXPAX@Z` | `void __cdecl BasePrivate::Delete(void*)` |

**Notable**: `Base::Throw` and `ATL::BaseAtlThrow` resolve to the **same address** (0x71901BBE at runtime) — they are aliased/thunked within WLXPhotoBase.dll. Both serve as SEH-based error throwing for the Base framework.

### d3dx9_32.dll (3 functions)
Direct3D 9 helper matrix operations for GPU pipeline rendering:

| Function | Purpose |
|----------|---------|
| `D3DXMatrixTranslation` | Translation matrices for transitions |
| `D3DXMatrixMultiply` | Matrix composition |
| `D3DXMatrixScaling` | Scale matrices for zoom/fade effects |

### SHLWAPI.dll (1 function)
`PathRemoveFileSpecW` — path manipulation for module-relative lookups.

## RTTI Type Descriptors (.data section)

### Internal Classes

| Mangled | Demangled | Role |
|---------|-----------|------|
| `.?AVClassicTheme@@` | `ClassicTheme` | Primary classic slideshow theme |
| `.?AVClassicThemeBase@@` | `ClassicThemeBase` | Abstract base for theme implementations |
| `.?AVClassicThemeBasic@@` | `ClassicThemeBasic` | Basic variant (no effects) |
| `.?AVClassicThemeBlackAndWhite@@` | `ClassicThemeBlackAndWhite` | B&W filter theme |
| `.?AVClassicThemeSepia@@` | `ClassicThemeSepia` | Sepia tone filter theme |
| `.?AVClassicTransform@@` | `ClassicTransform` | GPU transform/transition effect |

### Framework/ATL Classes

| Mangled | Demangled | Role |
|---------|-----------|------|
| `.?AVISlideshowTheme@@` | `ISlideshowTheme` | Interface — slideshow theme contract |
| `.?AUIClassFactory@@` | `IClassFactory` | Standard COM class factory |
| `.?AUIMediaNode@@` | `IMediaNode` | Pipeline media node interface |
| `.?AUIServiceProvider@@` | `IServiceProvider` | COM service provider |
| `.?AUIUnknown@@` | `IUnknown` | Base COM interface |
| `.?AUIRegistrarBase@@` | `IRegistrarBase` | ATL registrar base |
| `.?AVCAppModule@WTL@@` | `WTL::CAppModule` | WTL application module |
| `.?AV?$CComObjectNoLock@VCComClassFactory@ATL@@@ATL@@` | `ATL::CComObjectNoLock<ATL::CComClassFactory>` | ATL class factory instance |
| `.?AVCComClassFactory@ATL@@` | `ATL::CComClassFactory` | ATL class factory |
| `.?AVCRegObject@ATL@@` | `ATL::CRegObject` | ATL registry object (self-registration) |
| `.?AVException@Base@@` | `Base::Exception` | Base framework exception |
| `.?AVtype_info@@` | `type_info` | C++ RTTI |

### Class Hierarchy

```
ISlideshowTheme (interface)
  └── ClassicThemeBase
        ├── ClassicTheme
        ├── ClassicThemeBasic
        ├── ClassicThemeBlackAndWhite
        └── ClassicThemeSepia

ClassicTransform (separate — GPU pipeline transform)

IUnknown
  ├── IClassFactory
  │     └── CComClassFactory
  ├── IServiceProvider
  ├── IMediaNode
  └── IRegistrarBase
```

## Dynamic Probing Results

### DllCanUnloadNow
Returns `S_OK` (0x00000000) — no outstanding COM references, DLL can be safely unloaded.

### DllGetClassObject
- **NULL CLSID**: SEH exception `0xC0000005` (access violation) — expected null dereference on CLSID lookup.
- **All 5 extracted GUID candidates**: Return `0x80040111` (`CLASS_E_CLASSNOTAVAILABLE`) — none are the registered CLSID. The actual CLSID is embedded in ATL REGXML resources within the `.rsrc` section (binary XML format, not directly string-extractable).

### GetTFXCreateFunctions (WLXPipeTran.dll)
Returns `0x80004005` (E_FAIL) — expected, requires proper COM initialization and pipeline context.

### GetPipelineCreateFunctions (WLXPipeline.dll)
Returns `0x80004005` (E_FAIL) — expected, same reason.

## Embedded Resource Content

### REGXML (ATL Registry Script)
Binary XML resource at RVA 0x11000, contains ATL self-registration data:

- **Component Categories**: `\Implemented Categories`, `\Required Categories`
- **Registry Paths**: `Software\Microsoft\Windows Live\Photo Gallery\Slideshow`, `Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline`
- **Registry Functions Used**: `RegCreateKeyTransactedW`, `RegOpenKeyTransactedW`, `RegDeleteKeyTransactedW`, `RegDeleteKeyExW` — transactional registry API (supports KTM)

### TransitionsAndEffects XML
```xml
<TransitionsAndEffects Version="2.8">
  <TransitionDLL guid="{7371ADEE-C195-427F-B0EC-3CCC13725665}">
    <Transitions>
      <Transition name="Classic" iconid="2" guid="Classic">
```

### Version Info String Table
Key values from `VS_VERSION_INFO`:
- `Module` = "WLXPhotoClassic"
- `Module_Raw` = "WLXPhotoClassic.dll"
- `BasicMode`, `BlackAndWhite`, `Classic` — mode identifiers
- `DoCrossFade`, `FadeIn` — transition function names
- `InputCount`, `Volume`, `MixingMode` — audio mixer parameters
- `Hardware` — GPU pipeline capability flag
- `EnabledThemeSet` — theme set activation
- `Slideshow Theme - Classic` — description

### Pipeline Dependencies
String data references two additional DLLs loaded dynamically:
- `WLXPipeTran.dll` — `GetTFXCreateFunctions` (transform effects)
- `WLXPipeline.dll` — `GetPipelineCreateFunctions` (rendering pipeline)

## GUIDs

| GUID | Context |
|------|---------|
| `{7371ADEE-C195-427F-B0EC-3CCC13725665}` | TransitionDLL GUID (XML resource) |
| `{CB29A460-1C0D-4947-A59B-8CFC2260BBED}` | PDB GUID |

### Candidate CLSIDs (not confirmed — all returned CLASS_E_CLASSNOTAVAILABLE)

| GUID | Source |
|------|--------|
| `{773AFF18-2083-47C1-9EA9-A5DA346A0122}` | Binary scan |
| `{854E43AC-E1FD-46f2-8DD3-EE4C7A1844B6}` | Binary scan |
| `{B9087BDF-F0F8-4454-A7D1-F6242E1654F8}` | Binary scan |
| `{F91A0A3F-3E4E-4273-88CC-6664834ACA6F}` | Binary scan |
| `{71ED30A7-499A-4F61-84F8-10CDEC657FE0}` | Binary scan |

## Architecture Notes

WLXPhotoClassic.dll is a **slideshow transition theme provider** within the Windows Live Photo Gallery 2012 pipeline:

1. **Registration**: Self-registers via ATL as a COM server. The CLSID is defined in the embedded REGXML resource (requires REGUTIL.DLL or ATL `CRegObject` to parse).

2. **Class Factory**: Standard `DllGetClassObject` → `CComClassFactory` → creates instances of `ClassicThemeBase` and its derivatives.

3. **Theme System**: Implements `ISlideshowTheme` interface with 4 concrete themes (Classic, Basic, BlackAndWhite, Sepia). Each theme provides transition timing, cross-fade parameters, and visual filter settings.

4. **GPU Pipeline**: Uses `ClassicTransform` for D3DX9-based matrix transformations. Depends on `WLXPipeTran.dll` for transform effects and `WLXPipeline.dll` for the rendering pipeline. Both are loaded dynamically by the host (WLXSlideshow.exe).

5. **Configuration**: Reads theme settings from `HKCU\Software\Microsoft\Windows Live\Photo Gallery\Slideshow` and GPU pipeline settings from `HKCU\Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline`.

6. **Audio**: Version string table references `Pipeline\AudioMixer` and `Pipeline\SolidColor` — audio mixing and solid color background for transitions.

## Test Harness

File: `tests/OtherDlls/test_photo_classic.cpp`

Compiled output: `tests/OtherDlls/test_photo_classic.exe`

Test coverage:
- LoadLibraryEx with fallback paths for target DLL + dependencies
- DllCanUnloadNow call with SEH protection
- DllGetClassObject with NULL CLSID (expects SEH) and 5 CLSID candidates (all return CLASS_E_CLASSNOTAVAILABLE)
- DllRegisterServer/DllUnregisterServer pointer resolution (call skipped)
- WLXPhotoBase.dll: all 6 mangled import resolution verification
- WLXPipeTran.dll: GetTFXCreateFunctions call (returns E_FAIL without init)
- WLXPipeline.dll: GetPipelineCreateFunctions call (returns E_FAIL without init)
