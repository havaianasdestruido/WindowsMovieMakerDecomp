# WLXPipetran.dll Dynamic Analysis

## Test Harness Results

**Date:** 2026-07-26  
**Harness:** `tests/WLXPipetran/test_pipetran.cpp`  
**Compile:** MSVC x86 `/EHsc /MDd`

### Load & Export Resolution

| Field | Value |
|-------|-------|
| LoadLibrary base | `0x71F60000` (ASLR-relocated from `0x10000000`) |
| EntryPoint | `0xA07E` (matches dumpbin) |
| Sections | 4 sections verified (`.text`, `.data`, `.rsrc`, `.reloc`) |
| Export | `GetTFXCreateFunctions` at RVA `0x9DD5`, resolved to `0x71F69DD5` |
| Return value | `0x80004005` (`E_FAIL`) — expected: requires D3D9 device + COM context |

The function is a COM factory that creates `ITransformFXCollection` instances. Without a valid D3D9 render context and proper COM initialization, it returns `E_FAIL`. This confirms the DLL loads, resolves its single export, and executes without crashing.

### Section Verification

| Section | VA | VirtSize | RawSize | Flags | Hex |
|---------|------|----------|---------|-------|-----|
| `.text` | `0x1000` | `0x49305` | `0x49400` | `0x60000020` | Code, Execute+Read |
| `.data` | `0x4B000` | `0x20C4` | `0x1600` | `0xC0000040` | Init Data, Read+Write |
| `.rsrc` | `0x4E000` | `0x5D670` | `0x5D800` | `0x40000040` | Init Data, Read Only |
| `.reloc` | `0xAC000` | `0x7E2C` | `0x8000` | `0x42000040` | Discardable, Read Only |

---

## RTTI Class Inventory (98 unique types)

### Core Infrastructure (11 classes)
| Class | Role |
|-------|------|
| `CTransformFXCollection` | Main factory/collection — owns all effects |
| `CTransformFX` | Single transform effect instance |
| `CMediaTransform` | Media transform base (with `IServiceProvider`) |
| `CDvdMediaTransform` | DVD-specific media transform |
| `CCreateFunction` | Static factory object (`CStaticObject` pattern) |
| `CIdentityObjectImpl` | GUID identity tracking |
| `CComObjectRootBase` | ATL COM root |
| `GdiplusBase` | GDI+ base class |
| `Bitmap`, `Image`, `SolidBrush`, `Brush` | GDI+ wrapper types |

### COM Interfaces (10)
| Interface | Purpose |
|-----------|---------|
| `ITransformFXCollection` | Collection of all TFX effects |
| `ITransformFX` | Individual effect interface |
| `IPersistTransformFX` | Serialization/persistence |
| `IMediaNode` | Media graph node |
| `IControlOutputSize` | Output dimension control |
| `IServiceProvider` | ATL service locator |
| `IDvdMenuInput` | DVD menu input handling |
| `IDvdMenuButton` | DVD menu button |
| `IDvdMenuCompositor` | DVD menu composition |
| `IIdentityObject` | Identity tracking |
| `ITokenHandler` | Callback/token handler |

### Animation Classes (22)
| Class | Type |
|-------|------|
| `CAnimation` | Base animation |
| `CCameraSetupAnimation` | Camera setup |
| `C2DProjectionAnimation` | 2D projection transform |
| `CMeshAnimation` | D3DX mesh deformation |
| `CPlaneQuadAnimation` | Single quad plane |
| `CWipePlaneQuadAnimation` | Wipe transition quad |
| `CDissolvePlaneQuadAnimation` | Dissolve transition quad |
| `COutGoingDissolvePlaneQuadAnimation` | Outgoing dissolve |
| `CInComingDissolvePlaneQuadAnimation` | Incoming dissolve |
| `CDissolveMaskAnimation` | Mask-based dissolve |
| `CPixelateAnimation` | Pixelation effect |
| `CWipeAnimation` | Wipe transition |
| `CTitleAnimation` | Title text animation |
| `CTitleRootAnimation` | Title root container |
| `CTitleTwoLinesAnimation` | Two-line title |
| `CCreditAnimation` | Credit text animation |
| `CFXAnimation` | Effects animation |
| `CFXAnimationPanZoom` | Pan+zoom effect |
| `CFXAgeAnimation` | Film aging effect |
| `CVideoBoxAnimation` | Video box layout |
| `CGradientBackgroundAnimation` | Gradient background |
| `CBitmapBoxAnimation` | Bitmap box layout |
| `CParagraphAnimation` | Paragraph text |
| `CBannerAnimation` | Banner text |
| `CTextLine` | Single text line |

### DVD Menu Classes (7)
| Class | Role |
|-------|------|
| `CDvdMenuInput` | Menu input processing |
| `CDvdMenuButton` | Menu button |
| `CDvdMenuCompositor` | Menu compositing |
| `CDvdFade` | DVD fade transition |
| `CDvdBaseAnimation` | DVD animation base |
| `CDvdAnimation` | DVD animation |
| `CDvdAnimationWithVideo` | DVD animation with video layer |
| `CDvdVideoWallAnimation` | DVD video wall |
| `CDvdVideoWallGridAnimation` | DVD video wall grid |

### Pattern Mesh Classes (28)
| # | Class | .x Mesh File | Display Name |
|---|-------|-------------|--------------|
| 0 | `CPatternMesh` | *(base class)* | — |
| 1 | `CRectanglePatternMesh` | `Rectangle.x` | Rectangle |
| 2 | `CStarPatternMesh` | `Star.x` | Star |
| 3 | `CStarsPatternMesh` | `Stars.x` | Stars |
| 4 | `CDiamondPatternMesh` | `Diamond.x` | Diamond |
| 5 | `CSwipePatternMesh` | `Swipe.x` | Swipe |
| 6 | `CSplitPatternMesh` | `Split.x` | Split |
| 7 | `CInsetPatternMesh` | `Inset.x` | Inset |
| 8 | `CIrisPatternMesh` | `Iris.x` | Iris |
| 9 | `CDiagonalBoxPatternMesh` | `DiagonalBox.x` | Diagonal Box |
| 10 | `CDiagonalCrossPatternMesh` | `DiagonalCross.x` | Diagonal Cross |
| 11 | `CDiagonalDownPatternMesh` | `DiagonalDown.x` | Diagonal Down |
| 12 | `CCirclePatternMesh` | `Circle.x` | Circle |
| 13 | `CCirclesPatternMesh` | `Circles.x` | Circles |
| 14 | `CRadarPatternMesh` | `Radar.x` | Radar |
| 15 | `CFillVPatternMesh` | `FillV.x` | Fill V |
| 16 | `CBowTiePatternMesh` | `BowTie.x` | Bow Tie |
| 17 | `CZigzagPatternMesh` | `Zigzag.x` | Zigzag |
| 18 | `CCheckerboardPatternMesh` | `Checkerboard.x` | Checkerboard |
| 19 | `CWheelPatternMesh` | `Wheel.x` | Wheel |
| 20 | `CFanAndSweepPatternMesh` | — | Fan And Sweep |
| 21 | `CEyePatternMesh` | `Eye.x` | Eye |
| 22 | `CKeyholePatternMesh` | `Keyhole.x` | Keyhole |
| 23 | `CHeartPatternMesh` | `Heart.x` | Heart |
| 24 | `CSweepOutPatternMesh` | `SweepOut.x` | Sweep Out |
| 25 | `CSweepUpPatternMesh` | `SweepUp.x` | Sweep Up |
| 26 | `CSweepInPatternMesh` | `SweepIn.x` | Sweep In |
| 27 | `CFanUpPatternMesh` | `FanUp.x` | Fan Up |
| 28 | `CFanOutPatternMesh` | `FanOut.x` | Fan Out |
| 29 | `CFanInPatternMesh` | `FanIn.x` | Fan In |

### 3D / Particle / Visual Burn (8)
| Class | Role |
|-------|------|
| `CSimple3DTransform` | 3D transform helper |
| `CSimple3DBase` | 3D base class |
| `CSimple3DParticle` | Particle system particle |
| `CDXUTMesh` | DXUT mesh wrapper |
| `CAlphaMap` | Alpha mask processing |
| `CVBKeyBase` | Visual burn keying base |
| `CVBFXKey` | Visual burn FX key |
| `CVBLineKey` | Visual burn line key |
| `CVBLintKey` | Visual burn lint key |
| `CVBSplotchKey` | Visual burn splotch key |

---

## HLSL Shader (Embedded in .data Section)

### TX_Noise — Procedural Noise Texture Generator
**Offset:** `0x54D4` (file) / `.data` section  
**Purpose:** Generates RGBA noise texture used for film grain, aging, and dissolving effects

```hlsl
void TX_Noise(in float2 vTexCoord : POSITION, out float4 f4Color : COLOR0) {
    float4 f4Noise;
    f4Noise.r = noise((vTexCoord) * 650011);
    f4Noise.g = noise((vTexCoord) * 300073);
    f4Noise.b = noise((vTexCoord) * 1200007);
    f4Noise.a = noise((vTexCoord) * 301331);
    f4Color = f4Noise;
}
```

**Key observations:**
- Uses D3DX intrinsic `noise()` function (not standard HLSL — requires `D3DXCreateTextureShader`)
- Large prime multipliers for per-channel pseudo-random decorrelation
- Compiled at runtime via `D3DXCreateTextureShader` (imported from d3dx9_32.dll)
- Output texture used as `NoiseTexture` semantic input for other effects

### External FX File: `Parity.fx`
All 28+ pattern mesh transitions reference `Parity.fx` via `FXFile` parameter. This is the shared D3DX9 effect file that handles:
- Pattern mesh rendering (alpha masking)
- Dissolve blending between input textures
- Camera/projection transforms

---

## Embedded Effect Techniques (18 unique)

Extracted from inline XML `RCDATA` resource definitions:

| Technique | Category | Notes |
|-----------|----------|-------|
| `Fade` | Transition | Multiple variants (FadeStart, Alpha0/1) |
| `Blur` | Post-process | Directional blur (BlurSize param) |
| `Brightness` | Post-process | Brightness adjustment |
| `Grayscale` | Post-process | Grayscale conversion |
| `Sepia` | Post-process | Sepia tone |
| `Posterize` | Post-process | Color reduction (PosterizeLevels) |
| `HueCycle` | Post-process | Hue rotation animation |
| `Pixelate` | Post-process | Pixel block effect |
| `Ripple` | Post-process | Ripple distortion |
| `Sharpen` | Post-process | Sharpening filter |
| `EdgeDetect` | Post-process | Edge detection |
| `Kuwahara` | Post-process | Kuwahara filter (artistic) |
| `FilmGrain` | Post-process | Film grain noise |
| `FilmAgeOld` | Post-process | Old film effect |
| `FilmAgeOlder` | Post-process | Older film effect |
| `FilmAgeOldest` | Post-process | Oldest film effect |
| `PanZoom` | Animation | Pan and zoom (15+ variants) |
| `Rotate` | Animation | Rotation effect |

### Resizing Techniques
- `KeepVerticalConstant`
- `KeepHorizontalConstant`
- `NoResize`

---

## Transition Configuration Parameters (90 unique XML params)

### Pattern Mesh Transition
| Parameter | Type | Description |
|-----------|------|-------------|
| `Mesh` | string | Pattern name (27 values: "Rectangle", "Heart", "Star", etc.) |
| `DissolveType` | string | Dissolve2D, HorizontalBars, Rough2D, VerticalBars |
| `FXFile` | string | Always "Parity.fx" |
| `Technique` | string | Rendering technique name |
| `FadeStartA` | float | Fade start alpha |
| `ClipProgress` | float | Clipping progress (0-1) |
| `XProgressStart` | float | X-axis progress start |
| `ZProgressStart` | float | Z-axis progress start |
| `EntranceDuration` | float | Duration of entrance animation |
| `ExitDuration` | float | Duration of exit animation |
| `ShowAFront/ABack/BFront/BBack` | bool | Layer visibility |
| `VertexVelocity` | float | Vertex animation speed |
| `MoveSpeedA` | float | Translation speed |
| `RotateCenterA` | float | Rotation center |
| `RotateA` | float | Rotation amount |
| `ScaleA` | float | Scale factor |
| `ZScaleAStart` | float | Z-scale start value |
| `PageCurlA` | float | Page curl amount |
| `RevolveCamera` | bool | Camera revolve mode |
| `CameraZoom` | float | Camera zoom level |

### Particle System
| Parameter | Type | Description |
|-----------|------|-------------|
| `MaxParticles` | int | Maximum particle count |
| `ScatterDirection` | float | Particle scatter direction |
| `SpinDirection` | float | Particle spin direction |
| `ParticleSpeed` | float | Particle movement speed |
| `ParticleSpinSpeed` | float | Particle spin speed |
| `PulseMode` | int | Pulse animation mode |

### Post-Processing
| Parameter | Type | Description |
|-----------|------|-------------|
| `BlurSize` | float | Blur kernel size |
| `PosterizeLevels` | int | Number of color levels |
| `GrayscaleOn` | bool | Grayscale toggle |
| `FadedEdgesOn` | bool | Vignette/edge fade |
| `NoiseFrequency` | float | Noise texture frequency |
| `SplotchFrequency` | float | Splotch pattern frequency |
| `NoiseLineFrequency` | float | Line noise frequency |
| `LintFrequency` | float | Lint artifact frequency |
| `ShakeFactor` | float | Camera shake amplitude |
| `PixelSize` | float | Pixelation block size |
| `TexturePerturbation` | float | Texture distortion amount |
| `GenerateNoiseTexture` | bool | Enable noise texture gen |

### Text / Title
| Parameter | Type | Description |
|-----------|------|-------------|
| `FontSize` | int | Text font size |
| `BannerType` | string | Explosion, HorizontalFlood, LeftFlood, UpperFlood |
| `ForceFrontColor1` | color | Text color |
| `ForceCenterAlignment` | bool | Center text |
| `IsOneLine` | bool | Single line mode |
| `MaxCharacterCount` | int | Character limit |
| `ShadowOutline` | bool | Text shadow |
| `VerticalAlignment` | string | None, Left, Right, Localized |

### Entrance/Exit Effects
| Parameter | Values |
|-----------|--------|
| `EntranceEffect` | EffectFade, EffectFadeEllipse, EffectFadeWipe, EffectOutlineExplode, EffectRotate |
| `ExitEffect` | EffectFade, EffectFadeEllipse, EffectFadeWipe |
| `MainEffect` | EffectBounceWipe, EffectFlashing, EffectInverse, EffectMirror, EffectMulti, EffectPerspective, EffectTypeWriter |

---

## D3DX9 Integration

### Imported Functions (26 from d3dx9_32.dll)

**Mesh Loading:**
- `D3DXLoadMeshFromXW` — Load .x mesh from file
- `D3DXLoadMeshFromXResource` — Load .x mesh from embedded resource
- `D3DXCreateMeshFVF` — Create empty FVF mesh

**Matrix Math (12):**
- `D3DXMatrixMultiply`, `D3DXMatrixLookAtLH`
- `D3DXMatrixPerspectiveFovLH`, `D3DXMatrixOrthoLH`, `D3DXMatrixPerspectiveLH`
- `D3DXMatrixTranslation`, `D3DXMatrixScaling`
- `D3DXMatrixRotationX`, `D3DXMatrixRotationY`, `D3DXMatrixRotationZ`, `D3DXMatrixRotationYawPitchRoll`
- `D3DXMatrixReflect`, `D3DXMatrixTransformation`, `D3DXMatrixAffineTransformation`

**Quaternion (3):**
- `D3DXQuaternionRotationMatrix`, `D3DXQuaternionRotationAxis`, `D3DXQuaternionRotationYawPitchRoll`

**Vector/Plane (3):**
- `D3DXVec2CatmullRom` — Catmull-Rom spline interpolation
- `D3DXVec3TransformNormal` — Normal vector transform
- `D3DXPlaneIntersectLine` — Plane-line intersection

**Shader/Texture (3):**
- `D3DXCompileShader` — Runtime HLSL compilation
- `D3DXCreateTextureShader` — Create texture from shader (TX_Noise)
- `D3DXFillTextureTX` — Fill texture via TX function

### Embedded 3D Meshes (35 .x files)

Resource section contains 35 DirectX mesh files loaded via `D3DXLoadMeshFromXResource`:

| Mesh | Used By |
|------|---------|
| `PlaneQuad.x` | Base quad for all 2D transitions |
| `DissolveMask.x` | Dissolve mask geometry |
| `RuntimeAlphaMap.x` | Runtime alpha map generation |
| `GradientBackground.x` | Gradient background rendering |
| `Banner.x` / `ExplosionBanner.x` | Text banner animation |
| `DvdFade.x` | DVD fade transition |
| 27 pattern meshes | One per CPatternMesh subclass (see table above) |

### Rendering Pipeline

```
1. Input: Two video frames (A, B) + optional overlay textures
2. Pattern Mesh Selection: CPatternMesh subclass generates alpha mask geometry
3. Parity.fx Effect:
   a. Apply matrix transforms (camera, projection, rotation, scale)
   b. Render pattern mesh as alpha mask
   c. Dissolve blend input textures using mask
   d. Apply post-processing (blur, noise, etc.)
4. GDI+ Compositing Layer:
   a. Text overlays (titles, credits, banners)
   b. DVD menu buttons and navigation
   c. UI elements and system colors
5. Output: Composited frame to downstream pipeline
```

---

## Resource Embedding Strategy

### .rsrc Section (382 KB)

| Resource Type | Content | Approx Size |
|--------------|---------|-------------|
| `RCDATA` | 35 .x mesh files (binary) | ~200 KB |
| `RCDATA` | XML effect definitions (90+ param sets) | ~120 KB |
| `RCDATA` | HLSL shader source (TX_Noise + Parity.fx refs) | ~2 KB |
| `RCDATA` | 140+ CSS color names (wide strings) | ~15 KB |
| `RCDATA` | Media file extension filters (.wmv, .avi, etc.) | ~1 KB |
| `RCDATA` | DLL name references (WLXPipeline.dll, WLXPipeTran.dll) | ~1 KB |

### .data Section (8.2 KB)
| Content | Offset |
|---------|--------|
| TX_Noise HLSL shader source | `0x54D4` |
| COM interface IIDs (2 GUIDs) | `0x19C0-0x1A00` |
| RTTI class names (98 types) | Throughout |
| CSS color name table | Throughout |
| DLL/effect name strings | Throughout |

### File Format Support
Detected media extensions in resource strings:
`.wmv`, `.avi`, `.mpg`, `.asf`, `.dvr-ms`, `.mpe`, `.png`, `.xml`

---

## DVD Authoring Subsystem

### Menu Architecture
- `CDvdMenuInput` → Processes user input (button clicks, navigation)
- `CDvdMenuButton` → Individual clickable button with token system
- `CDvdMenuCompositor` → Composites menu layers (background, buttons, subpictures)

### Token Types (from embedded strings)
- `SceneButtonTFXToken` — Scene selection button
- `NavigationButtonTFXToken` — Navigation button
- `SubpictureButtonTFXToken` — Subpicture overlay button
- `TextTFXToken` — Text element

### Menu Properties
- `IsBold`, `IsItalic`, `FrontColor1` — Text styling
- `Menus`, `Font`, `ButtonLocations`, `Graph` — Menu structure
- `MenuStartTime`, `MenuEndTime` — Time-based menu display
- `InputStartOffset`, `SourceStartOffset` — Stream offset management
- `VideoRectInInput`, `VideoRectInOutput` — Video rect mapping
- `Opacity`, `ShadowOutline` — Visual properties

---

## GDI+ Text Rendering Stack

### Font Pipeline
```
GdipCreateFontFamilyFromName → GdipIsStyleAvailable → GdipCreateFont
→ GdipCreateStringFormat → GdipSetStringFormatFlags/Align/LineAlign
→ GdipAddPathString → GdipFillPath / GdipDrawString → GdipMeasureString
```

### Path Drawing
```
GdipCreatePath → GdipAddPathRectangle / GdipAddPathString
→ GdipCreatePen2 / GdipCreateSolidFill
→ GdipDrawPath / GdipFillPath
→ GdipDeletePath / GdipDeletePen / GdipDeleteBrush
```

### Image Compositing
```
GdipCreateBitmapFromFile/FromResource/FromScan0
→ GdipGetImageGraphicsContext → GdipDrawImageRectI
→ GdipSetCompositingMode / GdipSetInterpolationMode
→ GdipGraphicsClear / GdipSetClipRectI
```

---

## DLL Dependency Map

```
WLXPipetran.dll
├── MSVCR110.dll (48 functions) — CRT, SSE2 math
├── KERNEL32.dll (55 functions) — Core OS
├── GDI32.dll (5 functions) — Font/GDI objects
├── USER32.dll (8 functions) — Window DC, cursor, colors
├── ADVAPI32.dll (3 functions) — Registry config
├── ole32.dll (8 functions) — COM activation
├── OLEAUT32.dll (13 ordinals) — Automation
├── SHLWAPI.dll (5 functions) — Path utilities
├── COMCTL32.dll (3 functions) — ImageList
├── PSAPI.DLL (1 function) — Module path query
├── gdiplus.dll (50 functions) — Full GDI+ stack
└── d3dx9_32.dll (26 functions) — D3DX9 mesh/shader/matrix
```

**Runtime dependency:** `WLXPipeline.dll` (referenced in resources, loaded dynamically)
