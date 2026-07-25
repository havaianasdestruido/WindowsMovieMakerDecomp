# WLXPipetran.dll Analysis

## Overview

Pipeline transport/transform DLL from Windows Live Movie Maker 2012 (Photo Gallery suite). DirectX 9 + GDI+ effect/compositing engine providing transitions, animations, DVD menu compositing, and pixel shader effects.

| Field | Value |
|-------|-------|
| **File** | WLXPipetran.dll |
| **Image** | 737,280 bytes (0xB4000) |
| **Type** | PE32 x86, DLL |
| **Linker** | MSVC 11.00 (VS 2012) |
| **OS/Subsystem Ver** | 6.02 / 6.00 (Windows 8) |
| **Subsystem** | Windows CUI (3) |
| **Image Base** | 0x10000000 |
| **Entry Point** | 0x1000A07E |
| **Timestamp (PE)** | 2014-04-01 01:28:06 UTC |
| **Timestamp (Export)** | 2014-04-01 01:17:19 UTC |
| **DLL Characteristics** | Dynamic Base (ASLR), NX Compatible |
| **Checksum** | 0xB8F5F |
| **PDB** | `WLXPipeTran.pdb`, GUID `{89765767-4CF9-44E2-8351-95650D5D12B1}` |
| **Build** | 16.4.3528.0331 (ship build) |
| **Code signing** | Microsoft Code Signing PCA 2010/2011, multiple timestamps |

## PE Sections

| Section | VA | VirtSize | RawSize | Flags |
|---------|------|----------|---------|-------|
| `.text` | 0x10001000 | 0x49305 (299 KB) | 0x49400 (299 KB) | Code, Execute+Read |
| `.data` | 0x1004B000 | 0x20C4 (8.2 KB) | 0x1600 (5.5 KB) | Init Data, Read+Write |
| `.rsrc` | 0x1004E000 | 0x5D670 (382 KB) | 0x5D800 (383 KB) | Init Data, Read Only |
| `.reloc` | 0x100AC000 | 0x7E2C (31.5 KB) | 0x8000 (32 KB) | Init Data, Discardable, Read Only |

## Export Table

| Ordinal | RVA | Name |
|---------|------|------|
| 1 | 0x9DD5 | `GetTFXCreateFunctions` |

Single export — factory function that returns `ITransformFXCollection` interface pointer. All other functionality accessed via COM vtables.

## Import Table (12 DLLs, ~200+ functions)

### MSVCR110.dll (48 functions)
VC++ 2012 CRT: memory (malloc, free, calloc, _recalloc), string ops (wcschr, wcsstr, wcsnlen, wcsncpy_s, wcspbrk, wcsrchr, _wcsicmp, wmemcpy_s), memcpy/memset/memmove, math (floor, ceil, rand/srand, iswspace), SSE2 precise math trig (_libm_sse2_sin/cos/tan/atan/log/sqrt_precise, _CIatan2), C++ exception/terminate/purecall, CRT init (_initterm, _initterm_e, _dllonexit, _except_handler4_common)

### KERNEL32.dll (55 functions)
Module loading (LoadLibraryExW/FreeLibrary/GetModuleHandleW/ExW/GetProcAddress), resources (FindResourceExW/FindResourceW/LoadResource/LockResource/FreeResource/SizeofResource), file (GetFileAttributesW, FindFirstFileExW/FindNextFileW/FindClose), process/thread (GetCurrentProcess/CreateThread/GetCurrentThreadId/G etModuleFileNameW), heap (HeapAlloc/Free/ReAlloc/Destroy/Size/GetProcessHeap), sync (InitializeCriticalSectionEx/Enter/Leave/DeleteCriticalSection, WaitForSingleObject, InterlockedIncrement/Decrement), error (RaiseException, SetErrorMode, GetLastError), locale (GetUserDefaultUILanguage, EnumUILanguagesW, GetLocaleInfoW), time (QueryPerformanceCounter, GetSystemTimeAsFileTime, GetTickCount64), string (WideCharToMultiByte, MultiByteToWideChar, lstrlenW, lstrcmpiW), CRT helpers (EncodePointer/DecodePointer, ExpandEnvironmentStringsW), other (IsDebuggerPresent, IsProcessorFeaturePresent, SetErrorMode, MulDiv)

### GDI32.dll (5 functions)
GetStockObject, SelectObject, GetTextFaceW, DeleteObject, GetObjectW — font and GDI object management.

### USER32.dll (8 functions)
GetDC/ReleaseDC, LoadCursorW, LoadImageW, InflateRect, GetSysColor, SetCursor, CharNextW — window DC, cursor, system colors.

### ADVAPI32.dll (3 functions)
RegCloseKey, RegOpenKeyExW, RegQueryValueExW — minimal registry access for configuration.

### ole32.dll (8 functions)
CoCreateInstance, CLSIDFromString, CoInitialize/CoUninitialize, CoTaskMemAlloc/Free, PropVariantClear/Copy — COM activation, memory, and PROPVARIANT handling.

### OLEAUT32.dll (13 ordinals)
Variant/SafeArray/BSTR automation support.

### SHLWAPI.dll (5 functions)
PathFileExistsW, PathAddBackslashW, PathIsRelativeW, PathFindFileNameW, PathRemoveFileSpecW — path manipulation and file existence checks.

### COMCTL32.dll (3 functions)
ImageList_Create, ImageList_Destroy, ImageList_Add — image list for toolbar/UI elements.

### PSAPI.DLL (1 function)
GetModuleFileNameExW — get module path from process handle (for locating resources/external files).

### gdiplus.dll (50 functions)
Full GDI+ usage: bitmap loading (GdipCreateBitmapFromFile/FromScan0/FromResource, GdipCloneImage/DisposeImage), bitmap-to-graphics (GdipCreateFromHWND, GdipGetImageGraphicsContext), drawing (GdipDrawImageRectI, GdipDrawPath, GdipDrawString, GdipFillPath), font/string (GdipCreateFont, GdipCreateFontFamilyFromName, GdipCreateStringFormat, GdipSetStringFormat*, GdipMeasureString, GdipIsStyleAvailable), path/matrix (GdipCreate/DeleteMatrix/Path, GdipAddPathRectangle/String, GdipGetPathWorldBounds, GdipTransformMatrixPoints, GdipTranslate/Rotate/ScaleWorldTransform, GdipGetWorldTransform), pen/brush (GdipCreatePen2, GdipCreateSolidFill, GdipCloneBrush, GdipSetPenLineJoin), graphics state (GdipSetCompositingMode/SmoothingMode/TextRenderingHint/InterpolationMode/PixelOffsetMode/PageUnit/ ClipRect, GdipGraphicsClear), memory (GdipAlloc/GdipFree)

### d3dx9_32.dll (26 functions)
DirectX 9 utility library: mesh loading (D3DXLoadMeshFromXW, D3DXLoadMeshFromXResource, D3DXCreateMeshFVF), matrix math (D3DXMatrixMultiply/LookAtLH/PerspectiveFovLH/OrthoLH/PerspectiveLH/Transformation/Translation/Scaling/RotationX/Y/Z/YawPitchRoll/Reflect/AffineTransformation), quaternion (D3DXQuaternionRotationMatrix/Axis/YawPitchRoll), vector (D3DXVec2CatmullRom, D3DXVec3TransformNormal), plane (D3DXPlaneIntersectLine), shader (D3DXCompileShader, D3DXCreateTextureShader, D3DXFillTextureTX)

## COM Interfaces (from RTTI mangled names)

### Core Interfaces
- `ITransformFXCollection` — collection of transform effects
- `ITransformFX` — individual transform effect
- `IPersistTransformFX` — persistence for transform effects
- `IMediaNode` — media processing node
- `IControlOutputSize` — output size control
- `IServiceProvider` — service provider
- `IUIUnknown` — base IUnknown

### DVD Menu Interfaces
- `IDvdMenuInput` — DVD menu input handling
- `IDvdMenuButton` — DVD menu button
- `IDvdMenuCompositor` — DVD menu composition

### Supporting
- `IIdentityObject` — identity tracking
- `ITokenHandler` — token/callback handler
- `UIUnknown` — base ATL unknown

## ATL C++ Classes (from RTTI names)

### Core Infrastructure
- `CTransformFXCollection` — main transform collection object
- `CTransformFX` — single transform effect
- `CMediaTransform` — media transform (with IServiceProviderImpl)
- `CDvdMediaTransform` — DVD-specific media transform
- `CCreateFunction` — factory creation function (CStaticObject)
- `CIdentityObjectImpl` — identity tracking

### DVD Menu
- `CDvdMenuInput` / `CDvdMenuButton` / `CDvdMenuCompositor`
- `CDvdFade` / `CDvdBaseAnimation` / `CDvdAnimation` / `CDvdAnimationWithVideo`
- `CDvdVideoWallAnimation` / `CDvdVideoWallGridAnimation`

### Animation Classes
- `CAnimation` / `CCameraSetupAnimation` / `C2DProjectionAnimation` / `CMeshAnimation`
- `CPlaneQuadAnimation` / `CWipePlaneQuadAnimation` / `CDissolvePlaneQuadAnimation`
- `COutGoingDissolvePlaneQuadAnimation` / `CInComingDissolvePlaneQuadAnimation`
- `CDissolveMaskAnimation` / `CPixelateAnimation` / `CWipeAnimation`
- `CTitleAnimation` / `CTitleRootAnimation` / `CTitleTwoLinesAnimation` / `CCreditAnimation`
- `CFXAnimation` / `CFXAnimationPanZoom` / `CFXAgeAnimation`
- `CVideoBoxAnimation` / `CGradientBackgroundAnimation` / `CBitmapBoxAnimation`

### Transition Pattern Meshes (28 classes)
- `CPatternMesh` (base) + 27 concrete patterns:
  - `CRectanglePatternMesh`, `CStarPatternMesh`, `CStarsPatternMesh`
  - `CDiamondPatternMesh`, `CSwipePatternMesh`, `CSplitPatternMesh`
  - `CInsetPatternMesh`, `CIrisPatternMesh`, `CDiagonalBoxPatternMesh`
  - `CDiagonalCrossPatternMesh`, `CCirclePatternMesh`, `CCirclesPatternMesh`
  - `CRadarPatternMesh`, `CDiagonalDownPatternMesh`, `CFillVPatternMesh`
  - `CBowTiePatternMesh`, `CZigzagPatternMesh`, `CCheckerboardPatternMesh`
  - `CWheelPatternMesh`, `CFanAndSweepPatternMesh`, `CEyePatternMesh`
  - `CKeyholePatternMesh`, `CHeartPatternMesh`, `CSweepOutPatternMesh`
  - `CSweepUpPatternMesh`, `CSweepInPatternMesh`, `CFanUpPatternMesh`
  - `CFanOutPatternMesh`, `CFanInPatternMesh`

### Visual Burn/Keying
- `CVBKeyBase` / `CVBFXKey` / `CVBLineKey` / `CVBLintKey` / `CVBSplotchKey`
- Template: `CVBTransIDKey<T>`

### 3D / Particles
- `CSimple3DTransform` / `CSimple3DBase` / `CSimple3DParticle`
- `CDXUTMesh` — DXUT mesh helper
- `CAlphaMap` — alpha map processing

### Text Animation
- `CParagraphAnimation` / `CBannerAnimation` / `CTextLine`

### Collection/Template Infrastructure
- `CCollectionImpl<CTransformFX>`
- `CSimpleArray<CTransformFX*>`

## Key Strings & Parameters

### Effect Parameters
- `BlurSize`, `BlurAxis` — directional blur
- `GenerateNoiseTexture` — noise texture generation flag
- `ShowAFront/ABack/BFront/BBack` — multi-layer visibility
- `DebugCamera`, `WireframeA/B` — debug rendering
- `RevolveCamera`, `CameraZoom`, `RotationDegree` — camera control
- `PageCurlA`, `FadeStartA` — transition parameters
- `VertexVelocity`, `MoveSpeedA`, `RotateCenterA`, `RotateA`, `ScaleA`, `ZScaleAStart` — animation
- `ScatterDirection`, `SpinDirection`, `MaxParticles`, `PulseMode` — particle system
- `XProgressStart`, `ZProgressStart`, `ClipProgress` — progress control
- `Semantics`, `TextureViewport` — rendering semantics
- `Shatter`, `OutALittle` — effect types
- `lowerright/upperright/lowerleft/upperleft`, `smaller/bigger` — alignment anchors

### Post-Processing
- `PosterizeLevels`, `GrayscaleOn`, `FadedEdgesOn`
- `NoiseFrequency`, `SplotchFrequency`, `NoiseLineFrequency`, `LintFrequency`
- `ShakeFactor`
- `PixelSize`, `TexturePerturbation`

### Texture Semantics
- `InputTexture0`, `InputTexture1`, `LastTexture`, `NoiseTexture`
- `VideoTexture`, `MaskTexture`, `OverlayTexture`, `UnderlayTexture`
- `OverlayColor`, `VideoZoom`, `MasterAlpha`, `ZoomOut`
- `VideoCellSize`, `VideoGridLineWidth`
- `VideoRectInInput`, `VideoRectInOutput`

### CSS Colors (embedded, 140+ names)
Full CSS named color palette from `aliceblue` to `yellowgreen`, plus system colors (`activeborder`, `activecaption`, etc.)

### Inline HLSL Pixel Shader
```hlsl
// TX_Noise — generated noise texture
void TX_Noise(in float2 vTexCoord : POSITION, out float4 f4Color : COLOR0) {
    float4 f4Noise;
    f4Noise.r = noise((vTexCoord)*650011); 
    f4Noise.g = noise((vTexCoord)*300073); 
    f4Noise.b = noise((vTexCoord)*1200007); 
    f4Noise.a = noise((vTexCoord)*301331); 
    f4Color = f4Noise;
}
```

### Process types (embedded XML `RCDATA`)
- `Technique` — likely defines pixel shader techniques

## Architecture

1. **Transform/Effect Engine**: Core DLL for applying visual transforms (transitions, animations, effects) to video frames. Single `GetTFXCreateFunctions` entry point.

2. **Transition System**: 28+ wipe/pattern mesh classes implement different transition geometries (circle, heart, star, checkerboard, wheel, iris, etc.)

3. **Animation Framework**: Hierarchical animation classes for camera movement, 2D/3D projection, mesh deformation, dissolve/wipe transitions, title/credit text animation, DVD-style menus.

4. **DVD Authoring**: CDvdMenuInput/Button/Compositor classes support DVD menu creation with buttons, compositing, and animations.

5. **Pixel Shader Effects**: Compiles D3DX pixel shaders (including TX_Noise), supports blur, posterize, grayscale, noise, shake, pixelate, splotch effects.

6. **Text Rendering**: GDI+ for string measurement and drawing, plus CParagraphAnimation/CBannerAnimation for animated text overlays.

7. **Particle System**: CSplitPatternMesh, CStarsPatternMesh, and parameters like MaxParticles/ScatterDirection/SpinDirection suggest particle-based transitions.

8. **Dual Graphics Stack**: GDI+ (2D bitmap/font) + Direct3D 9 via D3DX (3D mesh, shader, matrix) — GDI+ renders the UI/composition layer, D3D9 renders 3D transform effects.

9. **ATL COM**: Standard ATL COM pattern with CComObjectRootEx (multi-threaded model), CUnknownImpl, and CComModule infrastructure.

10. **No Media Foundation**: No MF imports — this DLL is purely D3D9/GDI+ based and operates outside the MF pipeline.

## Registry Dependencies
- ADVAPI32: RegOpenKeyExW/RegQueryValueExW/RegCloseKey — minimal config lookup

## Debug Info
- PDB file: `WLXPipeTran.pdb`
- PDB GUID: `{89765767-4CF9-44E2-8351-95650D5D12B1}`
- /GS: 90 functions protected
- Pre-VC++ 11.00: 2 count, C/C++: 90 count
- Guard: unreported (no CFG)

## Digital Signature
- Microsoft Code Signing PCA 2010 and PCA 2011
- Multiple timestamps (Microsoft Time-Stamp Service)
- Embedded certificates in Certificates Directory at RVA 0xB0600
