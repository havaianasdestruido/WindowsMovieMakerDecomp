# WLXPhotoVoyager.dll - Static Analysis

## PE Characteristics

| Property | Value |
|----------|-------|
| **File Type** | DLL (Dynamic Link Library) |
| **Machine** | x86 (0x14C) - 32-bit |
| **Linker Version** | 11.00 (MSVC 11.0 / VS2012) |
| **Image Base** | 0x10000000 |
| **Subsystem** | Windows GUI (2) |
| **DLL Characteristics** | Dynamic base, NX compatible |
| **Target OS** | Windows 6.2+ (Win8+) |
| **Entry Point** | 0x100260A7 |
| **Size of Image** | 0x132000 (1.2 MB) |
| **Stack Reserve** | 0x40000 (256 KB) |
| **Heap Reserve** | 0x100000 (1 MB) |
| **Timestamp** | Tue Apr 1 01:27:15 2014 (0x533A4023) |
| **PDB GUID** | {38163FEC-2565-4047-941F-66C59C1B1AD9} |
| **PDB File** | WLXPhotoVoyager.pdb |

## Section Analysis

| Section | Virtual Addr | Virtual Size | Raw Size | Flags | Purpose |
|---------|-------------|-------------|----------|-------|---------|
| .text | 0x1000 | 0x2988C | 0x29A00 | Code, Execute Read | Compiled C++ code |
| .data | 0x2B000 | 0x1578 | 0x1000 | Initialized Data, RW | Global/static variables, vtables, RTTI type names |
| .rsrc | 0x2D000 | 0x1014F8 | 0x101600 | Initialized Data, RO | Embedded XML theme definitions, D3D shaders, images, version info |
| .reloc | 0x12F000 | 0x2BF2 | 0x2C00 | Initialized Data, Discardable, RO | Base relocations (ASLR) |

**Notes:**
- .rsrc section is enormous (0x1014F8 = ~1 MB) — dominates the binary, contains all Voyager theme XML definitions, compiled D3D9 pixel/vertex shaders, and bitmap resources
- .text section (0x2988C = ~165 KB) contains all compiled code
- Total virtual image size: ~1.2 MB — medium-sized DLL

## Export Table (4 functions)

All exports are standard COM DLL exports. Ordinal base = 1.

| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | 0x16B1B | `DllCanUnloadNow` |
| 2 | 1 | 0x16B0B | `DllGetClassObject` |
| 3 | 2 | 0x16B3B | `DllRegisterServer` |
| 4 | 3 | 0x16B2C | `DllUnregisterServer` |

This is a pure COM in-process server — all four standard COM self-registration exports are present. There are no custom non-COM exports (unlike wlxclip.dll which had `VMGGetClipCreateFunctions`).

## Import Table

### MSVCR110.dll (C Runtime)
Standard CRT imports: memory allocation (`malloc`, `free`, `calloc`, `_recalloc`), string operations (`wcsncpy_s`, `wcscat_s`, `wcscpy_s`, `wcsstr`, `wcschr`, `wcsnlen`, `wmemcpy_s`), exception handling (`_CxxThrowException`, `__CxxFrameHandler3`, `_except_handler4_common`), COM support (`type_info`), process control (`_amsg_exit`, `__crtTerminateProcess`, `__crtUnhandledException`).

### KERNEL32.dll
Core Win32 APIs: resource loading (`FindResourceExW`, `FindResourceW`, `LoadResource`, `LoadLibraryW`, `LoadLibraryExW`, `SizeofResource`, `LockResource`), memory/encoding (`EncodePointer`, `DecodePointer`), threading (`EnterCriticalSection`, `LeaveCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `DeleteCriticalSection`, `DisableThreadLibraryCalls`, `GetCurrentThreadId`, `GetCurrentProcessId`, `InterlockedIncrement/Decrement`), diagnostics (`GetTickCount64`, `QueryPerformanceCounter`, `IsDebuggerPresent`, `IsProcessorFeaturePresent`, `RaiseException`), string (`ExpandEnvironmentStringsW`, `MultiByteToWideChar`, `lstrcmpiW`), module (`GetModuleHandleW`, `GetModuleFileNameW`, `GetProcAddress`, `FreeLibrary`).

### USER32.dll
Minimal: `DestroyWindow`, `CharNextW`

### ADVAPI32.dll
Registry operations (`RegOpenKeyExW`, `RegCreateKeyExW`, `RegCloseKey`, `RegQueryValueExW`, `RegSetValueExW`, `RegDeleteValueW`, `RegDeleteKeyW`, `RegEnumKeyExW`, `RegQueryInfoKeyW`) and ETW tracing (`GetTraceLoggerHandle`, `GetTraceEnableFlags`, `GetTraceEnableLevel`, `TraceEvent`, `RegisterTraceGuidsW`, `UnregisterTraceGuids`).

### ole32.dll
COM infrastructure: `CoCreateInstance`, `CoTaskMemAlloc/Free/Realloc`, `PropVariantClear`, `StringFromGUID2`.

### OLEAUT32.dll
OLE Automation ordinals (typical COM server): `SysFreeString` (1), `SysAllocStringLen` (2), `SysStringLen` (6), `VariantInit` (8), `VariantClear` (9), `SafeArrayCreate` (147), `SafeArrayDestroy` (150), `SystemTimeToVariantTime` (277).

### WLXPhotoBase.dll
Base framework imports: `BasePrivate::Delete`, `BasePrivate::New`, `Base::Throw`, `Base::Exception::~Exception`, `ATL::BaseAtlThrow`, `String::Base::GetBaseStringManager`.

### d3dx9_32.dll
Direct3D 9 Extensions: `D3DXCreateEffect`, `D3DXCreateEffectPool`, `D3DXMatrixDecompose`, `D3DXMatrixLookAtLH`, `D3DXMatrixPerspectiveFovLH`, `D3DXMatrixShadow`, `D3DXMatrixTransformation`, `D3DXPlaneFromPoints`, `D3DXQuaternionRotationAxis`, `D3DXQuaternionSlerp`.

### SHLWAPI.dll
`PathRemoveFileSpecW`

## RTTI / Class Hierarchy

Extracted 46 RTTI type names from the .data section. The class hierarchy reveals:

### Core Theme Classes (C++ implementation)

| Class | Role |
|-------|------|
| `VoyagerThemeBase` | Abstract base for all Voyager themes |
| `VoyagerThemeAlbum` | "Album" theme — COM co-class |
| `VoyagerThemeCollage` | "Collage" theme — COM co-class |
| `VoyagerThemeFlip` | "Flip" theme — COM co-class |
| `VoyagerThemeFrame` | "Frame" theme — COM co-class |
| `VoyagerThemeGlass` | "Glass" theme — COM co-class |
| `VoyagerThemeSnapshots` | "Snapshots" theme — COM co-class |
| `VoyagerThemeTravel` | "Travel" theme — COM co-class |
| `VoyagerTransform` | COM co-class implementing slideshow transitions |

### Interface Classes

| Class | Role |
|-------|------|
| `ISlideshowTheme` | Abstract interface for all slideshow themes |
| `IMediaNode` | Media node interface (photo/video in slideshow) |
| `IServiceProvider` | Service provider interface |
| `$IServiceProviderImpl<VoyagerTransform>` | ATL implementation of IServiceProvider for VoyagerTransform |
| `IClassFactory` | Standard COM class factory |

### ATL/COM Infrastructure

| Class | Role |
|-------|------|
| `$CComObject<VoyagerTransform>` | ATL CComObject wrapper |
| `$CComObject<VoyagerThemeTravel>` | ATL CComObject wrapper |
| `$CComObject<VoyagerThemeSnapshots>` | ATL CComObject wrapper |
| `$CComObject<VoyagerThemeGlass>` | ATL CComObject wrapper |
| `$CComObject<VoyagerThemeFrame>` | ATL CComObject wrapper |
| `$CComObject<VoyagerThemeFlip>` | ATL CComObject wrapper |
| `$CComObject<VoyagerThemeCollage>` | ATL CComObject wrapper |
| `$CComObject<VoyagerThemeAlbum>` | ATL CComObject wrapper |
| `$CComCoClass<VoyagerTransform>` | ATL co-class base |
| `$CComCoClass<VoyagerThemeTravel>` | ATL co-class base |
| `$CComCoClass<VoyagerThemeSnapshots>` | ATL co-class base |
| `$CComCoClass<VoyagerThemeGlass>` | ATL co-class base |
| `$CComCoClass<VoyagerThemeFrame>` | ATL co-class base |
| `$CComCoClass<VoyagerThemeFlip>` | ATL co-class base |
| `$CComCoClass<VoyagerThemeCollage>` | ATL co-class base |
| `$CComCoClass<VoyagerThemeAlbum>` | ATL co-class base |
| `$CComObjectNoLock<$CComClassFactory>` | ATL class factory |
| `$CComObjectRootEx<$CComMultiThreadModel>` | ATL root object |
| `CComClassFactory` | ATL class factory |
| `CComModule` / `CAtlModule` / `$CAtlModuleT<CComModule>` | ATL module |
| `CAppModule` (WTL) | WTL application module |
| `CRegObject` | ATL registry object |
| `Exception@Base` | Base framework exception class |

## COM Registration (from embedded REGINST resource)

### Registered ProgIDs and CLSIDs

| Theme | CLSID | ProgID | FriendlyName Resource |
|-------|-------|--------|----------------------|
| Album | `{C84CFE1B-89DC-40E7-83BF-CB821255F9EC}` | `Microsoft.Photos.Slideshow.VoyagerThemeAlbum.1` | `%MODULE%,1121` |
| Collage | `{AEE6C573-A192-4AF3-B62B-A4E6848533D3}` | `Microsoft.Photos.Slideshow.VoyagerThemeCollage.1` | `%MODULE%,1122` |
| Flip | `{653E52D8-D033-469A-8BB5-9C1A164416D5}` | `Microsoft.Photos.Slideshow.VoyagerThemeFlip.1` | `%MODULE%,1123` |
| Frame | `{B4E10BE6-A2CE-4BEF-9D80-99995CB3C162}` | `Microsoft.Photos.Slideshow.VoyagerThemeFrame.1` | `%MODULE%,1125` |
| Glass | `{5515D2B5-6825-409E-B377-544708C9DD06}` | `Microsoft.Photos.Slideshow.VoyagerThemeGlass.1` | `%MODULE%,1126` |
| Snapshots | `{D5561752-E5A7-46E7-B768-D945E144CA78}` | `Microsoft.Photos.Slideshow.VoyagerThemeSnapshots.1` | `%MODULE%,1127` |
| Travel | `{CC4F1166-CE12-41F7-85E2-AE4744D9381B}` | `Microsoft.Photos.Slideshow.VoyagerThemeTravel.1` | `%MODULE%,1128` |
| Transform | `{E48325CB-1EFC-425E-9CD9-47EF51BECD55}` | `Microsoft.Photos.Slideshow.VoyagerTransform.1` | N/A (transform, not a theme) |

All theme classes are registered under:
```
HKCR\Microsoft.Photos.Slideshow.VoyagerTheme*.1
HKCR\Microsoft.Photos.Slideshow.VoyagerTheme*
HKCR\CLSID\{...}\InprocServer32 = %MODULE%
HKCR\CLSID\{...}\InprocServer32\ThreadingModel = "Apartment"
HKLM\Software\Microsoft\Windows Photo Gallery\Slideshow\Themes\{...}
```

All themes require shader model 2.0:
- `VertexShaderMajorVersion = 2`, `VertexShaderMinorVersion = 0`
- `PixelShaderMajorVersion = 2`, `PixelShaderMinorVersion = 0`

## Embedded Resources

### Voyager Theme XML Definitions

The .rsrc section contains multiple VoyagerTheme XML documents defining slideshow animation paths. Each theme (Album, Collage, Flip, Frame, Glass, Snapshots, Travel) contains:

1. **PhodeoLayoutPages** — 2D quad positions (TopLeft, TopRight, BottomLeft, BottomRight X/Y/Z coordinates) for photo placement on a pseudo-3D surface
2. **PhodeoMotion** — Keyframe-based animation with time/alpha for transitions
3. **Scenes** — Layout, Motion, and TransitionLayout scene types with camera positions
4. **Paths** — Named paths defining photo traversal through scenes
5. **Groups** — Photo groups with orientation (NORMAL, FLIP_HORIZONTAL), background files, border settings
6. **Transitions** — Fade transitions (`TFX\Fade` GUID)

Scene types observed:
- `Layout` — static photo placement
- `Motion` — animated movement between keyframes
- `TransitionLayout` — layout transition between two states

### D3D9 Compiled Shaders

Embedded D3DX9 compiled shaders (vertex shader `vs_2_0` and pixel shader `ps_2_0`) with parameters:
- `s_mProjection`, `s_mView`, `s_mShadow` — matrix constants
- `s_rgmBones` — bone transforms (skeletal animation for photo warping)
- `s_unBoneDepth`, `s_flBoneAlpha` — bone depth/alpha blending
- `s_flCrossFadeAlpha`, `s_flBorderWidthPercent`, `s_flBorderHeightPercent` — crossfade and border effects
- `s_v4BorderAdjust` — border adjustment vector

Shader resource names: `InputTexture0`, `LinearSampler`, `Shadow`, `Border`, `Slide`, `Background`, `BorderCrossFade`, `SlideCrossFade`, `BackgroundCrossFade`

### Additional Resources
- `GetTFXCreateFunctions` — TFX (transition) factory function name
- `GetPipelineCreateFunctions` — Pipeline factory function name

## Functional Summary

WLXPhotoVoyager.dll is a **COM in-process server** implementing the "Voyager" family of slideshow themes for Windows Photo Gallery. It provides 7 visual themes:

1. **Album** — Standard album layout with reverse-L camera path
2. **Collage** — Multi-photo pile-on effect
3. **Flip** — Horizontal flip animation
4. **Frame** — Framed photo presentation
5. **Glass** — Glass overlay effect
6. **Snapshots** — Snapshot-style presentation
7. **Travel** — Travel-themed with background image (`Travelthemebackground.jpg`)

Plus a shared **VoyagerTransform** component handling D3D9-based transitions between photos.

### Architecture
- Uses ATL (Active Template Library) for COM infrastructure
- Uses WTL (Windows Template Library) `CAppModule` for module management
- D3DX9 for GPU-accelerated photo rendering with custom vertex/pixel shaders
- Embedded XML theme definitions for animation path specification
- Depends on `WLXPhotoBase.dll` for base framework (exception handling, memory management, string management)
- ETW tracing support via ADVAPI32 TraceEvent APIs

### Key Integration Points
- `WLXPhotoBase.dll` — Base framework dependency
- `d3dx9_32.dll` — Direct3D 9 Extensions for shader-based rendering
- `WLXSlideshow.dll` (likely consumer) — loads these themes via COM `CoCreateInstance`
- `GetTFXCreateFunctions` / `GetPipelineCreateFunctions` — exported factory functions referenced in resources (used by the pipeline framework)

## Dynamic Analysis Results

### Export Probing

| Export | Status | Result |
|--------|--------|--------|
| `DllCanUnloadNow` | Returns `S_OK` (0x00000000) | No outstanding COM references at load time |
| `DllGetClassObject` | SEH 0xC0000005 (Access Violation) | Called with empty CLSID — expected crash; validates COM factory path |
| `DllRegisterServer` | Skipped | Would write to HKCR and HKLM registry |
| `DllUnregisterServer` | Skipped | Would remove registry entries |

### CoCreateInstance Probing

All 8 theme CLSIDs returned `0x80040154` (`REGDB_E_CLASSNOTREG`) — the CLSIDs are not registered in this analysis environment since the DLL was not `regsvr32`'d. This is expected behavior: the COM server requires registration to create instances via `CoCreateInstance`.

### DLL Load Behavior
- DLL loads successfully at base address `0x711C0000` (ASLR relocated from image base `0x10000000`)
- `LoadLibraryExW` with `LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR` resolves the dependency chain correctly
- No implicit load-time failures — all dependency DLLs (WLXPhotoBase.dll, d3dx9_32.dll, etc.) resolve
