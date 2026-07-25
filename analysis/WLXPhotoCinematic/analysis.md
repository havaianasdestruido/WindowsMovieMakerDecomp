# WLXPhotoCinematic.dll Analysis

## Overview

COM DLL implementing cinematic (Ken Burns / pan & zoom) photo effects for Windows Live Photo Gallery slideshow. PE32 x86, linker v11.0 (MSVC 2012), image base 0x10000000, ASLR+DEP. Timestamp: 2014-04-01. Build: 16.4.3528.0331 (same as other WLX DLLs). PDB: `WLXPhotoCinematic.pdb` GUID `{0DB69AAE-1EE5-4822-95E2-F4B6520E7091}`.

## PE Structure

| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Image Base | 0x10000000 |
| Image Size | 0x16000 (88 KB) |
| Linker | 11.00 |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | Dynamic base, NX compatible |
| Stack | 256KB reserve / 4KB commit |
| Heap | 1MB reserve / 4KB commit |
| Entry Point | 0x1000BE24 |

### Sections

| Section | VA | Virtual Size | Raw Size | Flags | Purpose |
|---------|-----|-------------|----------|-------|---------|
| `.text` | 0x1000 | 0xE8EE | 0xEA00 | Code, Execute Read | Code (~58 KB) |
| `.data` | 0x10000 | 0x103C | 0xC00 | Init Data, Read Write | Globals, vtables |
| `.rsrc` | 0x12000 | 0x18F8 | 0x1A00 | Init Data, Read Only | Resources (~6.4 KB) |
| `.reloc` | 0x14000 | 0x1632 | 0x1800 | Init Data, Discardable, Read Only | Base relocations |

## Export Table (4 functions)

| Ordinal | RVA | Name |
|---------|------|------|
| 1 | 0x0000328A | DllCanUnloadNow |
| 2 | 0x0000327A | DllGetClassObject |
| 3 | 0x000032AA | DllRegisterServer |
| 4 | 0x0000329B | DllUnregisterServer |

Standard COM DLL exports. No application-specific exports means all functionality is accessed via COM coclasses.

## Import Table

### Direct Imports (8 DLLs)

| DLL | Purpose | Key Imports |
|-----|---------|-------------|
| **MSVCR110.dll** (32) | CRT | malloc, free, memcpy, memset, wcscat_s, wcscpy_s, wcsstr, rand, srand, _time64, _wtof, C++ exception handling, _CxxFrameHandler3, _initterm |
| **KERNEL32.dll** (34) | OS kernel | LoadLibraryExW, GetProcAddress, GetModuleHandleW, GetModuleFileNameW, resources (FindResourceW, LoadResource, SizeofResource, LockResource), Interlocked ops, critical sections, system time, QueryPerformanceCounter |
| **USER32.dll** (2) | UI | DestroyWindow, CharNextW |
| **ADVAPI32.dll** (14) | Registry, ETW | RegCreateKeyExW, RegOpenKeyExW, RegQueryValueExW, RegSetValueExW, RegDeleteKeyW, RegDeleteValueW, RegisterTraceGuidsW, TraceEvent, UnregisterTraceGuids |
| **ole32.dll** (7) | COM | CoTaskMemAlloc/Free/Realloc, CoCreateInstance, PropVariantClear, StringFromCLSID, StringFromGUID2 |
| **OLEAUT32.dll** (10) | Automation | 10 ordinal-only imports (no named exports) |
| **SHLWAPI.dll** (2) | Path/string | PathRemoveFileSpecW, StrCmpW |
| **WLXPhotoBase.dll** (6) | Foundation | BasePrivate::New/Delete, Base::Throw, Exception dtor, BaseAtlThrow, GetBaseStringManager |
| **d3dx9_32.dll** (2) | Direct3D 9 math | D3DXMatrixPerspectiveFovLH, D3DXMatrixLookAtLH |

### Key DLL: d3dx9_32.dll

Imports DirectX 9 camera/projection matrix functions:
- `D3DXMatrixLookAtLH` — Builds a left-handed view matrix (camera position → look-at point)
- `D3DXMatrixPerspectiveFovLH` — Builds a left-handed perspective projection matrix (FOV-based)

This confirms the DLL uses Direct3D 9 for rendering the pan-and-zoom effect. The camera math directly implements the Ken Burns effect (simulated camera movement across a still image).

## COM Class Identifiers (CLSIDs)

Two COM coclasses are registered:

### CLSID 1: `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}`
- **ProgID**: `Microsoft.Photos.Slideshow.CinematicFullScreen1.1`
- **VersionIndependentProgID**: `Microsoft.Photos.Slideshow.CinematicFullScreen1`
- **FriendlyName**: "Cinematic - Full Screen"
- **ThreadingModel**: Apartment
- **InprocServer32**: `%MODULE%`

### CLSID 2: `{5409AB48-D8D3-40e6-A1EB-23489DC422DE}`
- **ProgID**: `Microsoft.Photos.Slideshow.CinematicTransform.1`
- **VersionIndependentProgID**: `Microsoft.Photos.Slideshow.CinematicTransform`
- **FriendlyName**: "Cinematic Theme Transform"
- **ThreadingModel**: Apartment
- **InprocServer32**: `%MODULE%`

### Transition DLL GUIDs (referenced in embedded XML, not CLSIDs in this DLL)

| GUID | Reference |
|------|-----------|
| `{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}` | Referenced as `TransitionDLL` guid in Embedded XML |
| `{7371ADEE-C195-427F-B0EC-3CCC13725665}` | Referenced as another `TransitionDLL` guid in Embedded XML |

These GUIDs likely correspond to transition effect DLLs (possibly `WLXSlideshow.dll` or `WLXPipeline.dll`).

## Embedded XML Resources (PanZoom Theme)

The DLL embeds a complete PanZoom theme definition as an XML resource. This is the **core data** driving the Ken Burns effect parameters.

### XML Structure: `/PanZoomTheme/Classes/`

Root: `<PanZoomTheme>` → `<Classes>`

Four aspect-ratio classes, each with three speed variants:

```
<Classes>
  <Portrait>
    <FullScreen>
      <Slow Zoom="0.2" PanZoom="0.05" EffectLength="18"/>
      <Medium Zoom="0.2" PanZoom="0.05" EffectLength="14"/>
      <Fast Zoom="0.2" PanZoom="0.05" EffectLength="8"/>
    </FullScreen>
  </Portrait>
  <Landscape>
    <FullScreen>
      <Slow Zoom="0.1" PanZoom="0.05" EffectLength="15"/>
      <Medium Zoom="0.1" PanZoom="0.05" EffectLength="12"/>
      <Fast Zoom="0.1" PanZoom="0.05" EffectLength="6"/>
    </FullScreen>
  </Landscape>
  <Panorama>
    <FullScreen>
      <Slow Zoom="0.2" PanZoom="0.05" EffectLength="50"/>
      <Medium Zoom="0.2" PanZoom="0.05" EffectLength="35"/>
      <Fast Zoom="0.2" PanZoom="0.05" EffectLength="20"/>
    </FullScreen>
  </Panorama>
  <XPanorama>
    <FullScreen>
      <Slow Zoom="0.2" PanZoom="0.05" EffectLength="90"/>
      <Medium Zoom="0.2" PanZoom="0.05" EffectLength="75"/>
      <Fast Zoom="0.2" PanZoom="0.05" EffectLength="40"/>
    </FullScreen>
  </XPanorama>
</Classes>
</PanZoomTheme>
```

### Parameter Interpretation

| Parameter | Meaning |
|-----------|---------|
| `Zoom` | Magnification factor applied over the effect (0.1 = 10% zoom-in, 0.2 = 20%) |
| `PanZoom` | Panning speed/amount (0.05 consistent across all variants) |
| `EffectLength` | Duration in frames (or seconds) at 30fps: Slow=6-90, Medium=6-75, Fast=6-40 |

### Class Characteristics

| Class | Zoom | PanZoom | Effect Length Range | Notes |
|-------|------|---------|-------------------|-------|
| Portrait | 0.2 (20%) | 0.05 | 8-18 (fastest) | Tall images, most aggressive zoom |
| Landscape | 0.1 (10%) | 0.05 | 6-15 | Wide images, gentler zoom |
| Panorama | 0.2 (20%) | 0.05 | 20-50 | Extra-wide, longer effect |
| XPanorama | 0.2 (20%) | 0.05 | 40-90 | Extreme wide, longest effect |

The PanZoom factor is constant (0.05) across all classes — panning speed is uniform. Zoom and duration vary to accommodate different aspect ratios.

### Additional Embedded XML

The resources also contain a `TransitionsAndEffects Version="2.8"` XML section with:
- Two `<TransitionDLL>` entries referencing the GUIDs above
- A `<Transition name="Cinematic" iconid="2" guid="Cinematic">` — note the GUID is a string literal `"Cinematic"`, not a UUID, suggesting it's a named reference resolved at runtime (likely matching to a transition definition in `WLXSlideshow.dll`)
- A `<Transition name="Classic" iconid="2" guid="Classic">`

## Registration Scripts (RGS)

The embedded `.rgs` files contain COM registration for both CLSIDs:
- HKCR registration with ProgID, CLSID, CurVer
- "NoRemove CLSID" with ForceRemove for both CLSIDs
- InprocServer32 with Apartment threading model
- HKLM registration under `Software\Microsoft\Windows Photo Gallery\Slideshow\Themes`

## Key Observations

1. **Pure COM DLL**: No application-specific named exports; all functionality via `DllGetClassObject` → IClassFactory → COM interfaces.

2. **Two distinct COM objects**: A "Full Screen" renderer and a "Transform" processor, suggesting the cinematic effect is split into (a) a full-screen rendering surface and (b) a transform that applies the pan/zoom matrix math.

3. **Direct3D 9 pipeline**: Uses `D3DXMatrixLookAtLH` and `D3DXMatrixPerspectiveFovLH` to construct view/projection matrices — the Ken Burns effect is a moving virtual camera over a 2D quad textured with the photo.

4. **Aspect-ratio-aware**: The XML defines different parameters for Portrait, Landscape, Panorama, and XPanorama images, adjusting zoom intensity and duration.

5. **WLXPhotoBase dependency**: Uses Base memory management (`New`/`Delete`) and string manager, consistent with all other WLX DLLs.

6. **ETW tracing**: Imports `RegisterTraceGuidsW`/`TraceEvent`/`UnregisterTraceGuids` from ADVAPI32, enabling performance/telemetry tracing during effect rendering.

7. **No direct MF/Media Foundation imports**: Unlike other pipeline DLLs, this one doesn't use Media Foundation — it's self-contained with D3D9.

8. **Gallery context**: Registry path `Windows Photo Gallery\Slideshow\Themes` indicates this DLL is a slideshow theme plug-in for Windows Photo Gallery, not Movie Maker. It's co-located in the Photo Gallery folder.
