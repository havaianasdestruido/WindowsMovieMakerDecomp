# Imaging.dll — Static Analysis

## Overview

**Imaging.dll** is a large (~7.4 MB) ATL COM in-proc server implementing the GPU-accelerated image processing engine for Windows Live Photo Gallery 2012 (Windows Live Essentials 16.4.3528.0331). It provides a comprehensive image effects pipeline with Direct3D 10.1 hardware acceleration, OpenMP parallel processing, ICC/ICM color management, and RAW camera processing via a Capture One-derived pipeline. The internal name is "Expression Imaging".

---

## PE Structure

| Field | Value |
|---|---|
| **File** | `Imaging.dll` |
| **Arch** | x86 (14C) |
| **Subsystem** | Windows GUI (2) |
| **Linker** | Visual Studio 2012 (11.00) |
| **Entry Point** | `0x57E0E` |
| **Image Base** | `0x10000000` |
| **Image Size** | `0x76E000` (7,790,592 bytes) |
| **Code Size** | `0x4CCA00` (4,966,912 bytes) |
| **OS Target** | 6.02 (Windows 8) |
| **Image Version** | 6.02 |
| **Timestamp** | Tue Apr 1 01:27:22 2014 |
| **Checksum** | `0x770860` |
| **DLL Characteristics** | `0x140` — Dynamic Base, NX Compatible |
| **Debug PDB** | `Imaging.pdb` `{D0FF972B-5DB4-4DBE-B44B-540EF1F6F70B}` |
| **SEH Handler Table** | 1,803 entries (`0x70B`) |

### Sections

| Section | Virtual Size | Virtual Address | Raw Size | Flags |
|---------|-------------|----------------|----------|-------|
| `.text` | `0x4CC874` | `0x10001000` | `0x4CCA00` | Code, Execute Read |
| `.data` | `0x19B6C` | `0x104CE000` | `0x18000` | Initialized Data, Read Write |
| `.rsrc` | `0x1D6540` | `0x104E8000` | `0x1D6600` | Initialized Data, Read Only |
| `.reloc` | `0xAEFCE` | `0x106BF000` | `0xAF000` | Initialized Data, Discardable, Read Only |

### Notable

- **Massive `.rsrc` section** (~1.8 MB) — contains embedded D3D effect FX shaders and XML graph definitions
- **1,803 SEH handler entries** — indicates extensive C++ exception usage across the codebase
- **Load Config** with Security Cookie at `0x104CE8BC` — standard GS buffer protection
- **No delay-load imports** — all dependencies are statically bound

---

## Export Table

Standard ATL COM server exports (4 functions):

| Ordinal | Hint | Name |
|---------|------|------|
| 1 | 0 | `DllCanUnloadNow` |
| 2 | 1 | `DllGetClassObject` |
| 3 | 2 | `DllRegisterServer` |
| 4 | 3 | `DllUnregisterServer` |

**No custom exports.** Pure COM in-proc server — all functionality accessed via COM class instantiation.

---

## Import Table

### MSVCR110.dll — Visual C++ 2012 Runtime
| Category | Functions |
|----------|-----------|
| Memory | `malloc`, `free`, `calloc`, `recalloc`, `_aligned_malloc`, `_aligned_free`, `_malloc_crt`, `_calloc_crt` |
| String | `wcschr`, `wcscat_s`, `wcscpy_s`, `wcsncpy_s`, `wcspbrk`, `wcsstr`, `wcscspn`, `wcsspn`, `wcsnlen`, `_wcsicmp`, `_wcslwr_s`, `wmemcpy_s` |
| Math (SSE2) | `__libm_sse2_pow`, `__libm_sse2_powf`, `__libm_sse2_log`, `__libm_sse2_logf`, `__libm_sse2_cos`, `__libm_sse2_cosf`, `__libm_sse2_sin`, `__libm_sse2_sinf`, `__libm_sse2_asinf`, `__libm_sse2_atan`, `__libm_sse2_expf`, `ceil`, `_libm_sse2_exp_precise`, `_libm_sse2_cos_precise`, `_libm_sse2_log_precise`, `_libm_sse2_sqrt_precise`, `_libm_sse2_pow_precise`, `_libm_sse2_sin_precise` |
| CRT Init | `_initterm`, `_initterm_e`, `_amsg_exit`, `__CppXcptFilter`, `__CxxFrameHandler3` |
| C++ | `??2@YAPAXI@Z` (operator new), `??_V@YAXPAX@Z` (operator delete[]), `??3@YAXPAX@Z` (operator delete), `_purecall`, `?terminate@@YAXXZ`, `??1type_info@@UAE@XZ`, `_CxxThrowException` |
| Math/Conversion | `iswdigit`, `_wtol`, `_wtoi`, `_finite`, `_isnan`, `_control87`, `_clearfp` |
| Thread | `_beginthreadex`, `_lock`, `_unlock`, `__dllonexit`, `_onexit` |
| Format | `vswprintf_s`, `_vscwprintf`, `_vsnwprintf` |
| Comparison | `_mbscmp` |
| Random | `rand`, `srand` |

### KERNEL32.dll — Core OS Services
| Category | Functions |
|----------|-----------|
| Memory | `HeapAlloc`, `HeapReAlloc`, `HeapFree`, `HeapSize`, `HeapDestroy`, `GetProcessHeap`, `VirtualAlloc`, `VirtualFree`, `GlobalMemoryStatusEx`, `GetLargePageMinimum` |
| PE Loading | `LoadLibraryExW`, `LoadLibraryW`, `FreeLibrary`, `GetModuleHandleA`, `GetModuleHandleW`, `GetModuleHandleExW`, `GetModuleFileNameW`, `GetProcAddress` |
| Resources | `FindResourceW`, `FindResourceExW`, `LoadResource`, `LockResource`, `SizeofResource` |
| File I/O | `CreateFileW`, `DeleteFileW`, `GetFileAttributesW`, `GetFullPathNameW`, `GetTempPathW`, `GetTempFileNameW` |
| Memory-Mapped | `CreateFileMappingW`, `MapViewOfFile`, `UnmapViewOfFile` |
| String/Encoding | `MultiByteToWideChar`, `WideCharToMultiByte`, `lstrlenW`, `lstrcmpiW`, `CompareStringW`, `ExpandEnvironmentStringsW` |
| Synchronization | `EnterCriticalSection`, `LeaveCriticalSection`, `TryEnterCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `DeleteCriticalSection`, `InterlockedIncrement`, `InterlockedDecrement` |
| Thread | `GetCurrentProcessId`, `GetCurrentThreadId`, `SwitchToThread`, `Sleep`, `WaitForSingleObject`, `WaitForSingleObjectEx` |
| Process | `GetCurrentProcess`, `CreateMutexW`, `ReleaseMutex`, `CreateEventW`, `SetEvent`, `ResetEvent`, `ResumeThread` |
| Error/Debug | `GetLastError`, `SetLastError`, `RaiseException`, `IsDebuggerPresent`, `DebugBreak`, `OutputDebugStringA`, `OutputDebugStringW` |
| System Info | `IsProcessorFeaturePresent`, `GetSystemInfo`, `GetVersion`, `GetVersionExW`, `GetSystemTimeAsFileTime`, `GetTickCount64`, `QueryPerformanceCounter`, `QueryPerformanceFrequency` |
| Security | `EncodePointer`, `DecodePointer`, `DisableThreadLibraryCalls` |

### USER32.dll — Window/Monitor Management
| Function | Purpose |
|----------|---------|
| `GetDC`, `ReleaseDC` | Device context acquisition for GDI rendering |
| `GetClientRect`, `GetWindowRect`, `ClientToScreen`, `ScreenToClient` | Window coordinate transforms |
| `GetWindowLongW`, `SetWindowLongW`, `GetClassLongW` | Window property access |
| `GetSystemMetrics` | Screen metrics |
| `EnumDisplayMonitors`, `GetMonitorInfoW` | Multi-monitor enumeration |
| `GetUpdateRgn`, `InvalidateRect`, `InvalidateRgn`, `ValidateRect` | Repaint management |
| `GetScrollBarInfo` | Scrollbar state |
| `IsWindow` | Window validity check |
| `CharNextW`, `CharLowerBuffW` | String cursor operations |

### ADVAPI32.dll — Registry + WPP Tracing
| Category | Functions |
|----------|-----------|
| Registry | `RegOpenKeyExW`, `RegQueryValueExW`, `RegCreateKeyExW`, `RegDeleteKeyW`, `RegDeleteValueW`, `RegEnumKeyExW`, `RegQueryInfoKeyW`, `RegSetValueExW`, `RegCloseKey` |
| Token | `OpenProcessToken`, `AdjustTokenPrivileges`, `LookupPrivilegeValueW` |
| WPP Tracing | `TraceMessage`, `RegisterTraceGuidsW`, `UnregisterTraceGuids`, `GetTraceLoggerHandle`, `GetTraceEnableLevel`, `GetTraceEnableFlags` |

### ole32.dll — COM Foundation
| Function | Purpose |
|----------|---------|
| `CoInitializeEx`, `CoUninitialize` | COM runtime init |
| `CoCreateInstance` | Create COM objects |
| `CoCreateFreeThreadedMarshaler` | Free-threaded marshaling |
| `CoSetProxyBlanket` | Security proxy configuration |
| `CoTaskMemAlloc`, `CoTaskMemRealloc`, `CoTaskMemFree` | COM task memory |
| `StringFromCLSID`, `StringFromGUID2`, `CLSIDFromString` | GUID string conversion |
| `CreateStreamOnHGlobal` | IStream from HGLOBAL |
| `ProgIDFromCLSID` | ProgID lookup |
| `PropVariantClear` | PROPVARIANT cleanup |

### OLEAUT32.dll — Automation
Standard OLE Automation ordinals: `SysAllocString` (2), `SysFreeString` (3), `SysStringLen` (6), `VariantInit` (7), `VariantClear` (8), `VariantCopy` (9), `SafeArrayCreate` (10), `SafeArrayDestroy` (12), `SysAllocStringLen` (15), `SysReAllocString` (16), `SysReAllocStringLen` (19), `SystemTimeToVariantTime` (20), `VariantTimeToSystemTime` (21), `VarI4FromStr` (22), `VarR8FromStr` (24), `OleCreateFontIndirect` (40), `OleCreatePictureIndirect` (84), `SystemTimeToVariantTime` (146), `VariantTimeToSystemTime` (147), `SafeArrayCopy` (149), `SafeArrayCopyData` (150), `SystemTimeToVariantTime` (161), `SafeArrayCreateVector` (162), `SafeArrayRedim` (163), `SystemTimeToVariantTime` (176), `SystemTimeToVariantTime` (186), `VarDateFromStr` (277).

### WLXPhotoBase.dll — Base Framework
| Function | Purpose |
|----------|---------|
| `Base::Exception::operator HRESULT()` | Exception to HRESULT conversion |
| `Base::String::GetBaseStringManager` | String resource manager |
| `Base::OS::IsVistaOrGreater` | OS version check |
| `Base::Exception::Exception(const Exception&)` | Copy constructor |
| `Base::Exception::~Exception` | Destructor |
| `BasePrivate::New` | Memory allocation |
| `BasePrivate::Delete` | Memory deallocation |
| `Base::BaseAtlThrow` | ATL exception throw |
| `Base::ThrowLastError` | Throw with GetLastError |
| `Base::Throw` | Throw with HRESULT |

### GDI32.dll — GDI Rendering
| Function | Purpose |
|----------|---------|
| `CreateDIBitmap`, `CreateDCW`, `CreateCompatibleDC`, `DeleteDC` | DC management |
| `CreateSolidBrush`, `CreateRectRgn`, `CreateRectRgnIndirect`, `SetRectRgn` | GDI object creation |
| `SelectObject`, `DeleteObject`, `GetStockObject` | GDI object management |
| `BitBlt` | Bit-block transfer |
| `FillRgn`, `CombineRgn`, `RectInRegion`, `GetRgnBox`, `GetRegionData` | Region operations |
| `GetDeviceCaps` | Device capability query |
| `GetICMProfileW` | ICM color profile query |

### VCOMP110.DLL — OpenMP Runtime (Visual C++ 2012)
| Function | Purpose |
|----------|---------|
| `omp_set_dynamic` | Dynamic thread enablement |
| `omp_get_max_threads` | Max thread count query |
| `omp_get_thread_num` | Current thread ID |
| `_vcomp_for_static_simple_init` | Static for-loop init |
| `_vcomp_for_static_end` | Static for-loop cleanup |
| `_vcomp_fork` | Parallel region fork |

### SHLWAPI.dll — Shell Path Utilities
| Function | Purpose |
|----------|---------|
| `PathIsRelativeW` | Path type detection |
| `SHCreateStreamOnFileW` | IStream from file path |
| `PathAddBackslashW` | Path normalization |
| `PathAppendW` | Path concatenation |
| `PathFindExtensionW` | Extension extraction |
| `PathRemoveFileSpecW` | Directory extraction |

### mscms.dll — ICC/ICM Color Management
| Function | Purpose |
|----------|---------|
| `OpenColorProfileW`, `CloseColorProfile` | Profile handle management |
| `GetColorProfileFromHandle` | Profile data extraction |
| `GetColorProfileHeader`, `GetColorProfileElement` | Profile structure access |
| `IsColorProfileValid` | Profile validation |
| `GetColorDirectoryW` | System ICC directory |
| `CreateMultiProfileTransform` | Multi-profile transform creation |
| `TranslateBitmapBits` | Pixel format conversion via ICC |
| `DeleteColorTransform` | Transform cleanup |

### XmlLite.dll — XML Parsing
| Function | Purpose |
|----------|---------|
| `CreateXmlReader` | XML reader creation (for effect graph XML parsing) |

### d3d10_1.dll — Direct3D 10.1
| Function | Purpose |
|----------|---------|
| `D3D10CreateDevice1` | D3D10.1 device creation (hardware or WARP) |
| `D3D10CreateEffectFromMemory` | D3D effect compilation from embedded FX |

### dxgi.dll — DXGI
| Function | Purpose |
|----------|---------|
| `CreateDXGIFactory` | DXGI factory for adapter enumeration |

---

## COM Class Registration

### Infrastructure CLSIDs

| GUID | Class | Description |
|------|-------|-------------|
| `CLSID_EffectGraph` | VCoEffectGraph | Effect processing graph container |
| `CLSID_EffectGraphCopier` | VCoEffectGraphCopier | Graph deep-copy utility |
| `CLSID_EffectHost` | VCoEffectHost | Effect hosting environment |
| `CLSID_EffectImpl` | VCoEffectImpl | Base effect implementation |
| `CLSID_EffectInfo` | VCoEffectInfo | Effect metadata descriptor |
| `CLSID_EffectRegistrar` | VCoEffectRegistrar | Effect registration/discovery |
| `CLSID_CompositeEffectImpl` | VCompositeEffectImpl | Composite effect base |
| `CLSID_GlobalResourceManager` | VGlobalResourceManager | Global resource (texture/shader) cache |
| `CLSID_ImagingDeviceManager` | VImagingDeviceManager | GPU device lifecycle management |
| `CLSID_ImagingEngine` | VImagingEngine | Main imaging engine |
| `CLSID_TraceManager` | VTraceManager | WPP tracing manager |
| `CLSID_LargeAllocMemoryManager` | VLargeAllocMemoryManager | Large-block memory manager |
| `CLSID_D3DImageImpl` | VD3DImageImpl | D3D image wrapper |
| `CLSID_EnumEffect` | VEnumEffect | Effect enumerator |
| `CLSID_EffectCollection` | VEffectCollection | Effect collection |
| `CLSID_EffectRenderer` | VEffectRenderer | Effect rendering engine |
| `CLSID_EffectBitmapRenderer` | VEffectBitmapRenderer | Bitmap output renderer |
| `CLSID_ForegroundGraphicCollection` | VForegroundGraphicCollection | Overlay graphics |
| `CLSID_ImageBufferCollection` | VImageBufferCollection | Image buffer pool |
| `CLSID_RectCollection` | VRectCollection | Rectangle collection |
| `CLSID_TileCollection` | VTileCollection | Tiling support |
| `CLSID_TileGenerator` | VTileGenerator | Mipmap/tile generation |
| `CLSID_View` | VCoView | View/port renderer |

### DaVinci RAW Processing (Internal, GUID_NULL)

| Class | Description |
|-------|-------------|
| `VDaVinciFilter` | RAW processing filter |
| `VDaVinciGraph` | RAW processing graph |
| `VDaVinciProperty` | RAW processing property |

---

## Embedded Resources — D3D Effect Pipeline

### FX Shader Files (HLSL)
| Resource | Purpose |
|----------|---------|
| `ApplyCurveEffect.fx` | Tone curve application |
| `BackgroundEffect.fx` | Background rendering |
| `Base3dLutEffect.fx` | 3D LUT color grading |
| `BaseCurveEffect.fx` | Base tone curve |
| `BlendEffect.fx` | Alpha/composite blending |
| `ChannelMixerEffect.fx` | Channel mixing |
| `ColorFillEffect.fx` | Solid color fill |
| `ColorManagementEffect.fx` | ICC/ICM color management |
| `ColorSpaceConversionEffect.fx` | Color space transforms (RGB/YIQ/HSV) |
| `Convolve1DEffect.fx` | 1D convolution (blur/sharpen) |
| `CreativeMaskEffect.fx` | Creative masking |
| `CurveSourceEffect.fx` | Curve data source |
| `DenoiserRenderEffect.fx` | Noise reduction rendering |
| `GaussianBlurEffect.fx` | Gaussian blur |
| `HSFormulaEffect.fx` | Hue/saturation formula |
| `InvertEffect.fx` | Color inversion |
| `OutOfBoundsEffect.fx` | Edge/tile mode |
| `SaturationEffect.fx` | Saturation adjustment |
| `TransformEffect.fx` | Affine transform |
| `UnsharpMaskV2Effect.fx` | Unsharp mask sharpening |
| `CopyTextureEffect.fx` | Texture copy utility |
| `Dither565Effect.fx` | 565 dithering |
| `ChromaDenoiseEffect.fx` | Chroma noise reduction |
| `CaptureOneRawColorEffect.fx` | RAW color processing |
| `CaptureOneEstimateGEffect.fx` | Green channel estimation |
| `CaptureOneEstimateRBEffect.fx` | Red/Blue channel estimation |
| `CaptureOneEdgeDetectEffect.fx` | Edge detection |
| `CaptureOneSigmaFilterEffect.fx` | Sigma noise filter |
| `CaptureOneGDenoiseEffect.fx` | Green channel denoise |
| `CaptureOneRBDenoiseEffect.fx` | Red/Blue denoise |
| `CaptureOneThresholdEffect.fx` | Threshold operation |
| `CaptureOneAntiColorAliasingEffect.fx` | Anti-aliasing |
| `CaptureOneBandingSuppressionEffect.fx` | Banding suppression |
| `CaptureOneSaturationEffect.fx` | RAW saturation |
| `CaptureOneHotPixelEffect.fx` | Hot pixel removal |
| `CaptureOneGGCalibrationEffect.fx` | Green-green calibration |
| `CaptureOneGaussFilterEffect.fx` | Gaussian filter |
| `GreenGreenPostprocessEffect.fx` | Green-green postprocessing |

### XML Graph Definitions
Over 80 XML effect graph definitions describing the processing pipeline topology, including:
- `DefaultViewGraph.xml` — main view rendering graph
- `DefaultSimpleViewGraph.xml` — simplified view
- `ProcessedSourceEffect_graph.xml` — source processing chain
- `ProcessedPipelineEffect_graph.xml` — full pipeline
- `CaptureOneRawSceneEffect_graph.xml` — RAW scene pipeline
- `InverseRawSceneEffect_graph.xml` — inverse RAW pipeline
- `CompositeSceneEffect_graph.xml` — composite scene
- `CaptureOnePostProcessEffect_graph.xml` — post-processing chain

---

## RTTI / Class Hierarchy

### Effect Classes (ATL CComAggObject wrapped)

| Category | Classes |
|----------|---------|
| **RAW Processing** | CaptureOneRawSceneEffect, CaptureOnePostProcessEffect, InverseRawSceneEffect, CaptureOneRawColorEffect, CaptureOneEstimateGEffect, CaptureOneEstimateRBEffect, CaptureOneDemosaicEffect, CaptureOneBandingSuppressionEffect, CaptureOneAntiColorAliasingEffect, CaptureOneThresholdEffect, CaptureOneHotPixelEffect, CaptureOneGDenoiseEffect, CaptureOneRBDenoiseEffect, CaptureOneSigmaFilterEffect, CaptureOneGaussFilterEffect, CaptureOneGGCalibrationEffect, CaptureOneGammaEffect, CaptureOneLevelsEffect, CaptureOneExposureEffect, CaptureOneBrightnessEffect, CaptureOneContrastEffect, CaptureOneSaturationEffect, CaptureOneEdgeDetectEffect, BayerBinningEffect, RawContrastEffect, RawPipelineHistogramEffect, PreprocessRawEffect, GreenGreenCalibrationEffect, GreenGreenPostprocessEffect |
| **Color Processing** | ColorManagementEffect, ColorSpaceConversionEffect, ColorEngineEffect, ColorExposureEffect, ColorFillEffect, ColorTemperatureEffect |
| **Tone Curves** | ApplyCurveEffect, BaseCurveEffect, CurveEffect, CurveSourceEffect, MultiCurveEffect |
| **Levels** | LevelsRGBEffect, LevelsYIQEffect |
| **Adjustments** | ExposureEffect, BrightnessEffect, ContrastEffect, SaturationEffect, HighlightsShadows (HS)Effect, HSFormulaEffect |
| **Spatial** | GaussianBlurEffect, GaussianBlurLargeRadiusEffect, Convolve1DEffect, UnsharpMaskV2Effect |
| **Transform** | TransformEffect, RotateEffect, CropEffect, StraighteningEffect, MipMapEffect |
| **Compositing** | BlendEffect, ChannelMixerEffect, CreativeMaskEffect, CompositeSceneEffect |
| **Retouching** | BlemishRemovalEffect, BlemishRemovalMaskEffect, RedEyeEffect, RedEyeMaskEffect |
| **Rendering** | ViewEffect, ZoomHelperEffect, BackgroundEffect, ImageEffect, ImageBufferSourceEffect |
| **Infrastructure** | CacheEffect, CacheGridEffect, NoOpEffect, OutOfBoundsEffect, ProxyEffect, MultiplexerEffect, GlobalResourceManager, ImagingDeviceManager, TraceManager, LargeAllocMemoryManager |

### Utility Classes

| Class | Description |
|-------|-------------|
| `type_info` | Standard C++ RTTI |
| `CComModule` (ATL) | ATL module |
| `CAtlModuleT<CComModule>` | ATL module template |
| `CAtlModule` | ATL base module |

### Interface Support

| GUID | Interface |
|------|-----------|
| `{32CB3B90-5576-4E93-B824-8CBC3C546FF2}` | Main connection point |
| `{1087DBCC-C4D9-4780-BA29-5ABF5C6CEA8A}` | Secondary connection point |

---

## String Analysis

### Effect Property Names (Unicode, embedded in code)
Key property strings for the effect pipeline:

| Property | Effect |
|----------|--------|
| `EnableRawSceneEffect` | RAW pipeline toggle |
| `EnableColorProfile` | ICC profile toggle |
| `EnableGamma` / `EnableFilmCurve` | Tone mapping toggles |
| `EnablePostFilmCurveContrast` | Post-curve contrast |
| `EnableColorManagement` | Color management |
| `EnableCacheBelowHighlightsShadows` | Cache optimization |
| `EnableCacheBelowSaturation` | Cache optimization |
| `EnableCacheBelowMultiCurve` | Cache optimization |
| `EnableCacheBelowSharpen` | Cache optimization |
| `EnableCacheBelowRedEye` | Cache optimization |
| `PostProcessingEnabled` | Post-processing toggle |
| `SceneProcessingEnabled` | Scene processing toggle |
| `MonitorColorProfile` | Monitor ICC profile path |
| `CameraColorEngineProfile` | Camera color engine |
| `CameraICCProfile` | Camera ICC profile |
| `UseColorEngineModel` | Color engine mode |
| `SharpeningRadius` / `SharpeningAmount` / `SharpeningThreshold` | Sharpen parameters |
| `Recovery` / `FillLight` / `ColorBalance` | RAW adjustments |
| `WhiteBalanceAutoFix` | WB auto-correction |
| `WaveletDenoiserEnabled` | Wavelet denoiser |
| `DenoiserAutoFix` | Auto denoise |
| `ImageQuality` | Output quality |
| `StreamType` | Image stream type |

### Format Strings

| Pattern | Use |
|---------|-----|
| `MachineConfig, "..."` | Configuration serialization |
| `%s::%s[%s] = %s; ` | Property debug output |
| `/xmp/MSImagingV%d:%s` | XMP custom namespace |
| `/ifd/xmp/MSImagingV%d:%s` | IFD XMP path |

### System Query Strings
| Query | Purpose |
|-------|---------|
| `ROOT\CIMV2` | WMI namespace |
| `Win32_Processor` | CPU info |
| `Win32_VideoController` | GPU info |
| `Win32_OperatingSystem` | OS info |
| `Win32_ComputerSystem` | System info |
| `Win32_DesktopMonitor` | Monitor info |

### Registry Keys
| Key | Purpose |
|-----|---------|
| `HKEY_CURRENT_USER` | User settings |
| `HKEY_USERS` | User hive access |

---

## Analysis Summary

### Architecture

1. **COM Registration**: Registers 23 COM classes — one of the richest ATL COM servers in the Photo Gallery suite
2. **GPU Pipeline**: Uses Direct3D 10.1 (`D3D10CreateDevice1`) for hardware-accelerated image effects; falls back to WARP software renderer
3. **Effect Graph**: XML-defined processing graphs compiled into D3D 10 effect objects (`D3D10CreateEffectFromMemory`)
4. **Capture One RAW**: Licensed/derived Capture One RAW processing pipeline — 20+ RAW-specific effects
5. **OpenMP Parallelism**: VCOMP110 runtime for CPU-side parallel processing (denoising, curve evaluation)
6. **Color Management**: Full ICC/ICM pipeline via `mscms.dll` — camera profiles, monitor profiles, output profiles
7. **Memory Management**: Custom `LargeAllocMemoryManager` + `GlobalResourceManager` for GPU texture caching
8. **WMI Hardware Detection**: Queries CPU, GPU, monitor, OS properties for adaptive quality selection

### Key Design Points

- **Expression Imaging heritage** — the "Expression" internal name and Capture One pipeline indicate this was originally Microsoft Expression's imaging engine, repurposed for Photo Gallery
- **23 COM classes** — each effect is a separate COM object enabling graph-based composition
- **GPU + CPU hybrid** — D3D effects for GPU-accelerated operations, OpenMP for CPU-bound operations (denoising, wavelets)
- **30+ embedded HLSL shaders** — the entire effect pipeline is shader-based
- **WPP tracing** — production-grade telemetry via `TraceMessage` and `RegisterTraceGuidsW`
- **No delay-load imports** — all dependencies are statically linked, indicating a critical-path DLL
- **Multi-monitor awareness** — `EnumDisplayMonitors` + `GetMonitorInfoW` for per-monitor color management
- **Hardware detection** — WMI queries for CPU cores, GPU memory, monitor characteristics drive adaptive quality

### Dependency Chain

```
WLXPhotoGallery.exe
  └─ Imaging.dll
       ├─ WLXPhotoBase.dll (runtime framework)
       ├─ MSVCR110.dll (CRT)
       ├─ d3d10_1.dll → dxgi.dll (GPU rendering)
       ├─ mscms.dll (ICC color management)
       ├─ VCOMP110.DLL (OpenMP parallelism)
       ├─ XmlLite.dll (effect graph parsing)
       ├─ OLEAUT32.dll (COM automation)
       └─ GDI32.dll (bitmap output)
```

---

## Runtime Test Results

Test harness: `tests/OtherDlls/test_regres_imaging_metadatasys.cpp`

| Export | Result |
|--------|--------|
| `DllCanUnloadNow` | `S_OK` (0x00000000) — no outstanding references |
| `DllGetClassObject` (empty CLSID) | SEH exception `0xC0000005` (ACCESS_VIOLATION) — null-check crash in class factory |
| `DllGetClassObject` (EffectGraph CLSID) | `S_OK` — IUnknown obtained at `0x006B9440` |
| `DllRegisterServer` | Found at `0x70D074D0` (skipped — would modify registry) |
| `DllUnregisterServer` | Found at `0x70D074F0` (skipped — would modify registry) |
