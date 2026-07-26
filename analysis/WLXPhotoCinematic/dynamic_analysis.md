# WLXPhotoCinematic.dll - Dynamic Analysis

## Test Harness
- `tests/WLXPhotoCinematic/test_cinematic.cpp` - compiled with MSVC 2022 x86 (`/EHsc /MDd /GS-`)
- Loads DLL via `LoadLibraryW`, calls `DllGetClassObject` directly, probes COM interfaces

---

## Key Findings

### 1. Real CLSIDs (Object Map) vs. RGS Script CLSIDs

**The RGS scripts in the resource section do NOT match the actual object map CLSIDs.**

| Purpose | CLSID | Status |
|---------|-------|--------|
| RGS CinematicFullScreen | `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}` | Not registered, CLASS_E_CLASSNOTAVAILABLE |
| RGS CinematicTransform | `{5409AB48-D8D3-40E6-A1EB-23489DC422DE}` | Not registered, CLASS_E_CLASSNOTAVAILABLE |
| KenBurnsEffect | `{B1A6684D-1C5B-4976-843D-53F1E840E49C}` | Not in this DLL, CLASS_E_CLASSNOTAVAILABLE |
| KenBurnsEffectConfig | `{0831F9E2-19E0-4837-B026-28E1E6E087F4}` | Not in this DLL, CLASS_E_CLASSNOTAVAILABLE |
| **Actual Entry 0** | `{557B4CD8-C1EA-4A46-84EE-BA1AF9AA67D4}` | **S_OK — CinematicFullScreen** |
| **Actual Entry 1** | `{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}` | **S_OK — PanZoomTransform** |

### 2. COM Object Instantiation Results

#### Entry 0 — CinematicFullScreen
```
DllGetClassObject: S_OK
CreateInstance(IUnknown): S_OK @ 0x0106FDF0
QI(IUnknown):     S_OK (same=1, identity IID)
QI(Entry0_IID):   S_OK (same=1, responds to {70E8E77F-...})
QI(IServiceProvider): E_NOINTERFACE
QI(ISlideshowTheme): E_NOINTERFACE
```
- **IUnknown only** — no domain-specific interfaces exposed
- 13 vtable slots [00-12], slot [12] is destructor variant
- Vtable @ VA `0x1000164C`

#### Entry 1 — PanZoomTransform (the Ken Burns effect engine)
```
DllGetClassObject: S_OK
CreateInstance(IUnknown): S_OK @ 0x0105EF90
QI(IUnknown):      S_OK (same=1, identity IID)
QI(Entry1_IID):    S_OK (same=1, responds to {C2DB9835-...})
QI(Entry1_Second): S_OK (same=0, ptr=0x0105EF94 = this+4)
    Sub-vtable @ 0x72301610:
      [00] 0x723038FE  [01] 0x723038EF  [02] 0x723038E0
      [03] 0x723033B6  [04] 0x72302A40
      [05-14] cascade into main vtable (offset-adjusted thunks)
QI(IServiceProvider): E_NOINTERFACE (not via this IID)
```
- Main vtable has 20+ slots, slots [03-09] are custom methods
- **Sub-interface at `this+4`** responds to `{6D5140C1-7436-11CE-8034-00AA006009FA}` (IServiceProvider by IID)
- 5 custom sub-interface methods, then cascading into main vtable
- Vtable @ VA `0x10001624`

### 3. Object Map Layout (ATL _ATL_OBJMAP_ENTRY32)

Static array at VA `0x10010348` (.data), set into `[10010FB8]` during DllMain.

```
struct _ATL_OBJMAP_ENTRY32 {  // 0x24 bytes per entry
    DWORD pclsid;             // +0x00: ptr to CLSID in .text
    DWORD pfnCreateInstance;  // +0x04: creator function
    DWORD m_pCF;              // +0x08: cached class factory (0x100032C1 = CComClassFactory)
    DWORD dwRegister;         // +0x0C: registration cookie
    DWORD reserved1;          // +0x10
    DWORD reserved2;          // +0x14
    DWORD pfnGetClassObject;  // +0x18
    DWORD pfnGetClassObject2; // +0x1C
    DWORD pfnInit;            // +0x20: called during DllRegisterServer
};
```

### 4. IID Table Format (QI Dispatch)

Generic QI handler at `0x100065D4` uses 12-byte entries:

```
struct IID_ENTRY {     // 0x0C bytes
    DWORD pIID;        // +0x00: ptr to IID in .text (NULL = end sentinel)
    DWORD vtblOffset;  // +0x04: offset from 'this' pointer to sub-interface
    DWORD pfnHandler;  // +0x08: creator/dispatch function (1 = IUnknown-style direct)
};
```

#### Entry 0 IID table at VA `0x10001D3C`:
| # | IID | vtblOffset | Description |
|---|-----|-----------|-------------|
| 0 | `{70E8E77F-8721-46B6-B746-335E-BF857704}` | 0x00 | IUnknown identity |

#### Entry 1 IID table at VA `0x100018A8`:
| # | IID | vtblOffset | Description |
|---|-----|-----------|-------------|
| 0 | `{C2DB9835-1146-4B10-A7BB-73619125B026}` | 0x00 | IUnknown identity |
| 1 | `{6D5140C1-7436-11CE-8034-00AA006009FA}` | 0x04 | IServiceProvider sub-interface at `this+4` |

---

## D3DX9 Camera Math Parameters

### D3DXMatrixPerspectiveFovLH — Projection Matrix

**Call site:** `0x10009B72` via IAT `[0x100011CC]`

```asm
; Function at 0x100099XX builds the projection matrix
10009B25: mov dword ptr [esp+0Ch], 41200000h  ; zf = 10.0f (far plane)
10009B2D: and dword ptr [esp+8], 0            ; zn = 0.0f (near plane)
10009B64: movss [esp+4], xmm1                ; aspect = height/width
10009B6A: mov dword [esp], 3F490FDBh         ; fovy = PI/4 = 0.7854 (45 degrees)
10009B71: push eax                            ; pOut = [esi+0x80]
10009B72: call D3DXMatrixPerspectiveFovLH
```

| Parameter | Value | Notes |
|-----------|-------|-------|
| `pOut` | `[esi+0x80]` | Output 4x4 matrix stored in object at offset 0x80 |
| `fovy` | **PI/4 (45°)** | `0x3F490FDB` = 0.7854 radians |
| `aspect` | **computed** | Image height / image width (from `[ebp-24h]` / `[ebp-20h]`) |
| `zn` | **0.0** | Near clip plane |
| `zf` | **10.0** | `0x41200000` — far clip plane |

### D3DXMatrixLookAtLH — View Matrix

**Call site:** `0x1000B6D6` via IAT `[0x100011D0]`

```asm
; Function at 0x1000B67D builds the view matrix
1000B6A2: lea eax, [ecx+28h]           ; pEye = &this[0x28] (animated camera position)
1000B6A6: movss xmm0, [eax]            ; x-component of eye
1000B6AD: movss [esp+8], xmm0          ; store eye.z
1000B6B3: movss xmm0, [ecx+2Ch]        ; y-component at this+0x2C
1000B6B8: lea ecx, [esp+14h]           ; pUp on stack
1000B6BC: push ecx                      ; arg4: pUp
1000B6BD: lea ecx, [esp+0Ch]           ;
1000B6C1: push ecx                      ; arg3: pAt
1000B6C2: push eax                      ; arg2: pEye = this+0x28
1000B6C7: push eax                      ; arg1: pOut
1000B6C8: mov [esp+28h], 3F800000h     ; up.z = 1.0f
1000B6D6: call D3DXMatrixLookAtLH
```

| Parameter | Value | Notes |
|-----------|-------|-------|
| `pOut` | Stack `[esp+2Ch]` | Output view matrix |
| `pEye` | `this+0x28` | Camera position (animated during effect) |
| `pAt` | Stack parameter | Look-at target point |
| `pUp` | Stack, z=1.0 | Up vector has z=1.0 (vertical) |

After building the view matrix, the result is passed to:
```asm
1000B6E6: call [eax+0B0h]  ; SetTransform or similar D3D9 call with matrix
```

### Float Constants Table (Ken Burns effect parameters)

Located in `.text` section at `0x10001814`:

| Address | Hex | Float | Purpose |
|---------|-----|-------|---------|
| `0x1814` | `3D4CCCCD` | **0.05** | PanZoom amount (pan speed factor) |
| `0x1818` | `3E4CCCCD` | **0.2** | Zoom amount (20% magnification) |
| `0x181C` | `3F000000` | **0.5** | Midpoint / interpolation factor |
| `0x1820` | `3F19999A` | **0.6** | Aspect ratio threshold |
| `0x1824` | `3F800000` | **1.0** | Identity scale / unit factor |
| `0x1828` | `3FCCCCCD` | **1.6** | Max aspect ratio or scale factor |

### Range / Probability Constants

| Address | Hex | Float | Purpose |
|---------|-----|-------|---------|
| `0x1840` | `00000000` | **0.0** | Range start: XPortrait (aspect=0) |
| `0x1844` | `40000000` | **2.0** | Range boundary: Panorama |
| `0x1848` | `40400000` | **3.0** | Range boundary: XPanorama end sentinel |
| `0x184C` | `40C00000` | **6.0** | Maximum aspect ratio |
| `0x1850` | `42C80000` | **100.0** | PanZoomProb (probability = 100%) |
| `0x1854` | `437F0000` | **255.0** | Byte-max as float (clamping) |
| `0x1858` | `447A0000` | **1000.0** | Scale factor / timing constant |
| `0x185C` | `BF800000` | **-1.0** | Reverse direction / default invalid |

---

## Embedded PanZoom Theme XML

Complete Ken Burns parameter configuration embedded as an ASCII XML resource:

```xml
<PanZoomTheme>
  <Global HalfCrossFade="1" PanZoomProb="100.0"/>
  <Ranges>
    <Range ClassName="XPortrait"  Start="0"     End="0.5" />
    <Range ClassName="Portrait"   Start="0.5"   End="1.0" />
    <Range ClassName="Landscape"  Start="1.0"   End="2.0" />
    <Range ClassName="Panorama"   Start="2.0"   End="3.5" />
    <Range ClassName="XPanorama"  Start="3.5"   End="inf" />
  </Ranges>
  <Classes>
    <XPortrait><FullScreen>
      <Slow   Zoom="0.2" PanZoom="0.05" EffectLength="60"/>
      <Medium Zoom="0.2" PanZoom="0.05" EffectLength="40"/>
      <Fast   Zoom="0.2" PanZoom="0.05" EffectLength="20"/>
    </FullScreen></XPortrait>
    <Portrait><FullScreen>
      <Slow   Zoom="0.2" PanZoom="0.05" EffectLength="18"/>
      <Medium Zoom="0.2" PanZoom="0.05" EffectLength="14"/>
      <Fast   Zoom="0.2" PanZoom="0.05" EffectLength="8"/>
    </FullScreen></Portrait>
    <Landscape><FullScreen>
      <Slow   Zoom="0.1" PanZoom="0.05" EffectLength="15"/>
      <Medium Zoom="0.1" PanZoom="0.05" EffectLength="12"/>
      <Fast   Zoom="0.1" PanZoom="0.05" EffectLength="6"/>
    </FullScreen></Landscape>
    <Panorama><FullScreen>
      <Slow   Zoom="0.2" PanZoom="0.05" EffectLength="50"/>
      <Medium Zoom="0.2" PanZoom="0.05" EffectLength="35"/>
      <Fast   Zoom="0.2" PanZoom="0.05" EffectLength="20"/>
    </FullScreen></Panorama>
    <XPanorama><FullScreen>
      <Slow   Zoom="0.2" PanZoom="0.05" EffectLength="90"/>
      <Medium Zoom="0.2" PanZoom="0.05" EffectLength="75"/>
      <Fast   Zoom="0.2" PanZoom="0.05" EffectLength="40"/>
    </FullScreen></XPanorama>
  </Classes>
</PanZoomTheme>
```

### Parameter Interpretation

| Parameter | Meaning | Value Range |
|-----------|---------|-------------|
| `Zoom` | Total zoom magnification over effect duration (10% or 20%) | 0.1 - 0.2 |
| `PanZoom` | Lateral pan amount (constant across all classes) | 0.05 |
| `EffectLength` | Duration in seconds at 30fps | 6 - 90 |
| `HalfCrossFade` | Cross-fade occurs during half the transition | 1 (enabled) |
| `PanZoomProb` | Probability that pan/zoom is applied | 100.0% |

### Image Classification by Aspect Ratio

| Class | Aspect Range | Description | Zoom | Duration Range |
|-------|-------------|-------------|------|----------------|
| XPortrait | 0 - 0.5 | Ultra-tall portrait | 20% | 20-60s |
| Portrait | 0.5 - 1.0 | Standard portrait | 20% | 8-18s |
| Landscape | 1.0 - 2.0 | Standard landscape | **10%** | 6-15s |
| Panorama | 2.0 - 3.5 | Wide panorama | 20% | 20-50s |
| XPanorama | 3.5+ | Ultra-wide panorama | 20% | 40-90s |

**Key insight**: Landscape images get half the zoom of all other classes (10% vs 20%), while ultra-wide panoramas get the longest effect durations (up to 90 seconds).

---

## D3DX9 Integration Details

### Imported Functions
- `D3DXMatrixPerspectiveFovLH` (ordinal 0xD1) — builds 4x4 perspective projection matrix
- `D3DXMatrixLookAtLH` (ordinal 0xC9) — builds 4x4 left-handed view matrix

### Camera Model (Ken Burns Effect)

The Ken Burns effect simulates camera movement across a still 2D image:

1. **Texture mapping**: Photo is applied as a texture on a full-screen quad
2. **View matrix** (`D3DXMatrixLookAtLH`): Camera position (`this+0x28`, `this+0x2C`) animates over time, creating the pan effect
3. **Projection matrix** (`D3DXMatrixPerspectiveFovLH`): Fixed 45° FOV with aspect ratio computed from image dimensions; the zoom is achieved by moving the camera closer along the view axis
4. **Combined matrix**: View+Projection passed to D3D9 via `[eax+0xB0h]` vtable call (likely `IDirect3DDevice9::SetTransform`)

### Animation Interpolation

- Camera position at object offset `+0x28` (x) and `+0x2C` (y) are animated per-frame
- Float constants at `0x1814-0x1828` serve as interpolation parameters
- The `EffectLength` parameter controls animation duration
- Linear interpolation between start/end camera positions based on time elapsed

---

## Vtable Summary

### Entry 0 (CinematicFullScreen) Vtable @ VA 0x1000164C

| Slot | RVA | Function |
|------|-----|----------|
| 00 | 0x371E | QueryInterface (IID table at 0x10001D3C) |
| 01 | 0x36CF | AddRef |
| 02 | 0x36EA | Release |
| 03 | 0x6C35 | Unknown method |
| 04 | 0x6CB3 | Unknown method |
| 05 | 0x6CF7 | Unknown method |
| 06 | 0x6D44 | Unknown method |
| 07 | 0x6D79 | Unknown method |
| 08 | 0x6D9F | Unknown method |
| 09 | 0x6EA6 | Unknown method |
| 10 | 0x6E85 | Unknown method |
| 11 | 0x6F02 | Unknown method |
| 12 | 0x386C | Release (destructor variant) |

**Interface**: IUnknown only. No IServiceProvider, no ISlideshowTheme.

### Entry 1 (PanZoomTransform) Main Vtable @ VA 0x10001624

| Slot | RVA | Function |
|------|-----|----------|
| 00 | 0x384B | QueryInterface (IID table at 0x100018A8) |
| 01 | 0x37FC | AddRef |
| 02 | 0x3817 | Release |
| 03 | 0x8C9E | Custom method 1 (likely IMediaNode::SetSource) |
| 04 | 0x8CF2 | Custom method 2 (likely IMediaNode::Process) |
| 05 | 0x9CB6 | Custom method 3 (render/transform) |
| 06 | 0x9BA8 | Custom method 4 (setup/init) |
| 07 | 0x99DB | Custom method 5 (parameter get/set) |
| 08 | 0x38B9 | Cleanup |
| 09 | 0x2A54 | IServiceProvider::QueryService |
| 10-19 | various | Cascaded from CinematicFullScreen (slots 03-11) |

### Entry 1 Sub-interface Vtable @ VA 0x10001610

Returned via QI for `{6D5140C1-7436-11CE-8034-00AA006009FA}` at `this+4`:

| Slot | RVA | Function |
|------|-----|----------|
| 00 | 0x38FE | Unknown method (sub-interface specific) |
| 01 | 0x38EF | Unknown method |
| 02 | 0x38E0 | Unknown method |
| 03 | 0x33B6 | Unknown method |
| 04 | 0x2A40 | Unknown method |
| 05-14 | various | Cascade into main vtable (offset-adjusted thunks) |

---

## RTTI Classes

All C++ classes identified via RTTI typeinfo names:

| Class | Role |
|-------|------|
| `PanZoomTransform` | Main Ken Burns camera controller COM object |
| `CinematicFullScreen` | Full-screen rendering COM object |
| `PanZoomTheme` | Loads/parses PanZoomTheme XML configuration |
| `ImageClass` | Classifies images by aspect ratio into the 5 categories |
| `FullScreenLandscape` | Handles landscape orientation effects |
| `FullScreenPortrait` | Handles portrait orientation effects |
| `FullScreenPanorama` | Handles panorama orientation effects |
| `FullScreenXPanorama` | Handles extreme panorama orientation effects |
| `FullScreenXPortrait` | Handles extreme portrait orientation effects |
| `ISlideshowTheme` | Interface for slideshow theme plugins |
| `IServiceProvider` | COM service provider interface |
| `IMediaNode` | Media pipeline node interface |

---

## Embedded XML Resources

### CinematicTheme.xml
Resource name: `CINEMATICTHEME.XML`, type: `RT_XML`

### TransitionsAndEffects.xml
```xml
<TransitionsAndEffects Version="2.8">
  <Transitions>
    <TransitionDLL guid="{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}">
      <Transition name="Cinematic" iconid="2" guid="Cinematic"/>
    </TransitionDLL>
    <TransitionDLL guid="{7371ADEE-C195-427F-B0EC-3CCC13725665}">
      <Transition name="Classic" iconid="2" guid="Classic"/>
    </TransitionDLL>
  </Transitions>
</TransitionsAndEffects>
```

---

## Registry Paths

Registered under:
- `HKLM\Software\Microsoft\Windows Photo Gallery\Slideshow\Themes\{B1CACF91-...}` — CinematicFullScreen
- `HKLM\Software\Microsoft\Windows Photo Gallery\Slideshow\Themes\{5409AB48-...}` — CinematicTransform

Both register with `ThreadingModel=Apartment` via `InprocServer32`.

---

## DllMain Initialization Chain

```
DllMain (0x1000BE24)
  -> DLL_PROCESS_ATTACH
  -> 0x1000C576: security cookie init (RDTSC-based)
  -> 0x1000BE4C: SEH-protected main init
    -> 0x1000BC2A: thread pool / lock manager (lock cmpxchg at 0x10011028)
    -> 0x100031A4: ATL module init
      -> 0x1000411D: Set object map [10010FB8] = 0x10010348
        -> Call pfnInit (0x10004574) for each entry
      -> 0x10003128: Register entries
```

## DllGetClassObject Code Flow

```
DllGetClassObject (0x1000327A -> 0x10003F38)
  1. Read [10010FB8] (object map pointer)
  2. If NULL -> CO_E_NOT_INITIALIZED
  3. Iterate 0x24-byte entries:
     a. Check entry.pclsid != NULL
     b. Check entry.m_pCF != 0
     c. Compare CLSID
     d. Match: create CComClassFactory via entry.pfnCreateInstance
  4. No match: CLASS_E_CLASSNOTAVAILABLE (0x80040111)
```

## DllCanUnloadNow

Returns **0 (FALSE)** — objects still in use after test (AddRef from our test).

## DllRegisterServer / DllUnregisterServer

- `DllRegisterServer` (0x32AA): Returns `E_ACCESSDENIED` without admin rights
- `DllUnregisterServer` (0x329B): Returns `S_OK` (deletes registry keys)
