# WLXPipeline.dll Analysis

**File**: `undecomp/Photo Gallery/WLXPipeline.dll`
**Description**: Photo Gallery Video Pipeline — media processing pipeline for Windows Live Movie Maker 2012
**Build**: `16.4.3528.0331_ship.client.main.w5m4 (ship)`
**PDB**: `WLXPipeline.pdb` `{911D33AE-5E96-461A-B0D1-1ECCB116070F}`

---

## PE Structure

| Field | Value |
|-------|-------|
| **Machine** | x86 (0x14C) |
| **Subsystem** | Windows CUI (3) |
| **Linker** | MSVC 11.00 (VS 2012) |
| **OS Version** | 6.02 (Windows 8) |
| **Image Version** | 6.02 |
| **Image Base** | 0x10000000 |
| **Image Size** | 0xB2000 (728 KB) |
| **Entry Point** | 0x10023AAB |
| **Checksum** | 0xBCF8A |
| **DLL Characteristics** | Dynamic Base (ASLR), NX Compatible |
| **Stack Reserve/Commit** | 0x40000 / 0x1000 (256 KB / 4 KB) |
| **Heap Reserve/Commit** | 0x100000 / 0x1000 (1 MB / 4 KB) |

### Timestamps

- **Export directory**: Tue Apr 1 01:16:34 2014 (0x533A3DA2)
- **PE header**: Tue Apr 1 01:27:09 2014 (0x533A401D)

### Section Layout

| Section | VA | Virtual Size | Raw Size | Flags | Purpose |
|---------|-----|-------------|----------|-------|---------|
| `.text` | 0x1000 | 0x96C06 | 0x96E00 | Code, Execute Read | Executable code (605 KB) |
| `.data` | 0x98000 | 0x9B24 | 0x9000 | Init Data, Read Write | Globals, vtables, RTTI (38 KB) |
| `.rsrc` | 0xA2000 | 0x3F0 | 0x400 | Init Data, Read Only | Resources (1 KB) |
| `.reloc` | 0xA3000 | 0xEA08 | 0xEC00 | Init Data, Discardable, Read Only | Base relocations (58 KB) |

### PE Directories

| Directory | RVA | Size |
|-----------|-----|------|
| Export | 0x97B90 | 0x76 |
| Import | 0x96664 | 0x12C |
| Resource | 0xA2000 | 0x3F0 |
| Certificates | 0xAF200 | 0x3EC0 |
| Base Relocation | 0xA3000 | 0xD5D0 |
| Debug | 0x1410 | 0x38 |
| Load Config | 0x112F0 | 0x40 |
| IAT | 0x1000 | 0x3B8 |

---

## Exports (2 functions)

| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 2 | 0 | 0x00023443 | `DllRegisterServer` |
| 1 | 1 | 0x0002314C | `GetPipelineCreateFunctions` |

**Key observations:**

- **`GetPipelineCreateFunctions`** — the primary factory entry point. Returns function pointers to create pipeline nodes/stages. This is the DLL's core purpose: constructing the media processing pipeline.
- **`DllRegisterServer`** — COM self-registration (registers the `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` CLSID if applicable, or registry settings for the pipeline).
- No `DllGetClassObject`/`DllCanUnloadNow` — this is **not** a standard ATL COM DLL. Uses a custom factory pattern instead of COM class factory.

---

## Imports (14 DLLs)

### MSVCR110.dll (CRT) — ~52 functions

CRT core: memory (malloc/free/calloc/realloc/recalloc), string (wcslwr_s, wcsicmp, wcsstr, wcschr, wcsnlen, wcscpy_s, wmemcpy_s, memmove_s), math (ceil, floor, sqrt, pow, cos, sin via SSE2), printf, qsort, rand, itow_s, vsnwprintf. C++ exception support: terminate, type_info destructor, purecall, CppXcptFilter, set_terminate, dllonexit. Locking: _lock/_unlock. Initialization: _initterm/_initterm_e.

### KERNEL32.dll — ~48 functions

Process/thread: GetModuleFileNameW, GetModuleHandleW, GetProcAddress, CreateThread, GetCurrentThread(Id), GetThreadPriority/SetThreadPriority. Memory: HeapAlloc/Free/ReAlloc/Size/Destroy, VirtualQuery. Synchronization: InitializeCriticalSection(AndSpinCount), Enter/Leave/DeleteCriticalSection, InterlockedIncrement/Decrement/Exchange/ExchangeAdd, CreateEventW/SetEvent/ResetEvent, CreateSemaphoreW/ReleaseSemaphore, InitializeConditionVariable, WakeConditionVariable, SleepConditionVariableCS, WaitForSingleObject/WaitForMultipleObjects, TryEnterCriticalSection. Time: QueryPerformanceCounter, GetSystemTimeAsFileTime, GetTickCount(64). Module: LoadLibraryW/LoadLibraryExW/FreeLibrary/FreeLibraryAndExitThread. Resources: FindResourceW/ExW, LoadResource, LockResource, SizeofResource. Error: GetLastError, RaiseException. Misc: DecodePointer/EncodePointer, IsDebuggerPresent, IsProcessorFeaturePresent, GetSystemInfo, GetVersionExW, GetFileAttributesW, ExpandEnvironmentStringsW, lstrcmpW/lstrcmpiW, SetThreadDesktop, MulDiv, Sleep.

### ADVAPI32.dll — 12 functions

Registry: RegCreateKeyExW, RegOpenKeyExW, RegOpenKeyW, RegQueryValueExW, RegSetValueExW, RegCloseKey. ETW tracing: RegisterTraceGuidsW, UnregisterTraceGuids, TraceEvent, GetTraceEnableFlags, GetTraceEnableLevel, GetTraceLoggerHandle.

### USER32.dll — 19 functions

Messages: GetMessageW, DispatchMessageW, PeekMessageW, PostThreadMessageW, MsgWaitForMultipleObjects, RegisterWindowMessageW. Display/Monitor: EnumDisplayDevicesW, EnumDisplayMonitors, GetMonitorInfoW, MonitorFromWindow. Desktop: EnumDesktopWindows, OpenDesktopW, SetThreadDesktop. Window: GetWindowRect, GetClientRect, MapWindowPoints, IntersectRect. Drawing: GetDC, ReleaseDC, LoadImageW. Misc: IsIconic, IsWindow, IsWindowVisible, CopyRect, EqualRect, GetSystemMetrics.

### GDI32.dll — 3 functions

SetBkColor, DeleteObject, ExtTextOutW.

### SHLWAPI.dll — 2 functions

PathRemoveFileSpecW, PathFindExtensionW.

### ole32.dll — 8 functions

COM core: CoCreateInstance, CoInitialize(Ex), CoUninitialize, CoFreeUnusedLibraries. Memory: CoTaskMemAlloc, CoTaskMemFree. Properties: PropVariantClear, PropVariantCopy.

### OLEAUT32.dll — 9 ordinal imports (ordinals 2, 4, 6, 8, 9, 10, 12, 149, 150)

Likely: SysAllocString (2), SysFreeString (4), VariantInit (6), VariantClear (8), VariantCopy (9), VariantCopyInd (10), VariantChangeType (12), SysStringLen (149), SysStringByteLen (150). Provides COM automation/VARIANT support.

### gdiplus.dll — 29 functions

Bitmap: GdipCreateBitmapFromFile/FromStream/FromScan0/FromGdiDib/FromHBITMAP, GdipCreateHBITMAPFromBitmap, GdipCloneImage, GdipDisposeImage, GdipSaveImageToFile. Drawing: GdipDrawImageRectI, GdipDrawImageRectRectI, GdipGetImageGraphicsContext, GdipDeleteGraphics, GdipFlush. Image metadata: GdipGetPropertyItem(Size), GdipGetImageWidth/Height, GdipGetImagePixelFormat. Encoders: GdipGetImageEncoders(Size), GdipImageSelectActiveFrame, GdipImageGetFrameCount/DimensionsList/Count. Memory: GdipAlloc, GdipFree. Startup: GdiplusStartup, GdiplusShutdown.

### d3d9.dll — 1 function

Direct3DCreate9Ex.

### d3dx9_32.dll — 4 functions

D3DXMatrixTranslation, D3DXMatrixScaling, D3DXMatrixMultiply, D3DXCreateEffectFromFileW.

### DDRAW.dll — 2 functions

DirectDrawCreateEx, DirectDrawEnumerateExA.

### WINMM.dll — 5 functions

timeGetTime, timeBeginPeriod, timeEndPeriod, timeSetEvent, timeKillEvent.

### msdmo.dll — 3 functions

MoInitMediaType, MoFreeMediaType, MoCopyMediaType.

---

### Import Categories Summary

| Category | DLLs | Purpose |
|----------|------|---------|
| CRT | MSVCR110.dll | Memory, strings, math, exceptions |
| OS Core | KERNEL32.dll | Process, memory, sync, threading, modules |
| COM | ole32 + OLEAUT32 | COM objects, VARIANTs, property storage |
| Registry/ETW | ADVAPI32.dll | Registry persistence, event tracing |
| GDI+ | gdiplus.dll | Bitmap loading, image processing, encoding |
| Direct3D 9 | d3d9.dll + d3dx9_32.dll | GPU rendering pipeline, 3D transforms, shaders |
| DirectDraw | DDRAW.dll | Legacy video overlay/surface management |
| Multimedia timers | WINMM.dll | Precise timer events |
| DirectShow Media Types | msdmo.dll | AM_MEDIA_TYPE manipulation |
| UI/Monitor | USER32.dll | Display enumeration, window/desktop management |
| Path helpers | SHLWAPI.dll | File path manipulation |

---

## COM GUID

| GUID | Name |
|------|------|
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | CLSID_VideoTrim (shared with WLXVideoTrim.dll and WLXMediaPublishSubscribe.dll) |

The only COM CLSID found as a string literal. This DLL uses the VideoTrim CLSID, likely to create video trim/copy/transcode objects for pipeline integration.

---

## Pipeline Stage Names (from Unicode strings)

| Name | Category |
|------|----------|
| `Photo Gallery Video Pipeline` | Pipeline description |
| `Pipeline\AudioMixer` | Audio mixing node |
| `Pipeline\SilenceAudio` | Silence audio source |
| `Pipeline\SolidColor` | Solid color video source |
| `AudioMixer` | Audio mixer |
| `Color` | Color transform/effect |
| `Image Drawer Allocator` | Image allocation/surface management |
| `Timeline Source Stream Seeking` | Timeline seeking support |
| `stream` | Generic stream |
| `Volume` | Volume control |
| `Translation` | Translation transform |
| `MixingMode` | Mixing mode configuration |
| `Scrunch` | Scrunch (likely a resize/fit transform) |
| `FrameEat` / `FrameEat Out` | Frame dropping/throttling |
| `Deinterlace:` / `Deinterlace: Out` | Deinterlacing |
| `DisableDXVAHWDeinterlacer` | DXVA deinterlacer disable flag |
| `DXVAEnabled` / `DXVAEnableForMM` | DXVA acceleration flags |
| `Force InverseTelecinePattern` | Inverse telecine (IVTC) control |
| `DVAspectRatio` / `DVAspectRatio In` / `DVAspectRatio Out` | Aspect ratio control |
| `XForm In` / `XForm Out` | Transform input/output pins |
| `Opacity` | Opacity/alpha control |

### Additional Pipeline Element Names (from ASCII class strings)

| Class | Role |
|-------|------|
| `CAudioMixerNode` | Audio mixing pipeline node |
| `CEffectNode` | Effect processing node |
| `CEffect` | Effect base class |
| `CNode` | Pipeline node base class |
| `CQueueManager` | Pipeline queue management |
| `CSyncManager` | Synchronization manager |
| `CPipelineItems` | Pipeline item collection |
| `CPipelineSourceItem` | Pipeline source item |
| `CPipelineStream` | Pipeline stream |
| `CPipelineStreamBuilder` | Pipeline stream builder |
| `CMediaStream` | Media stream |
| `CAudioBuffer` / `CResampledAudioBuffer` | Audio buffer management |
| `CAudioMediaStream` | Audio media stream |
| `CBaseSource` / `CSourceCreator` | Source base/caching |
| `CSilenceAudioSource` | Silence audio source |
| `CSolidColorSource` | Solid color source |
| `CFileSourceWrapper` | File source wrapper |
| `CBaseAudioRenderer` | Audio renderer base |
| `CDShowVideoStream` | DirectShow video stream wrapper |
| `CDVAspectRatio` | DV aspect ratio handler |
| `CSurface` | Video surface |
| `CProxy_IDirect3D*` (14 classes) | Direct3D 9 proxy wrappers for GPU pipeline |

---

## Registry Paths

| Path | Purpose |
|------|---------|
| `Software\Microsoft\GPUPipeline` | GPU pipeline settings |
| `Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline` | WLX GPU pipeline configuration |

---

## Direct3D 9 Proxy Architecture

The DLL contains **14 proxy classes** wrapping Direct3D 9 interfaces:

- `CProxy_IDirect3DDevice9Ex` — device proxy
- `CProxy_IDirect3DBaseTexture9` / `CProxy_IDirect3DTexture9` / `CProxy_IDirect3DCubeTexture9` / `CProxy_IDirect3DVolumeTexture9`
- `CProxy_IDirect3DSurface9` / `CProxy_IDirect3DVolume9`
- `CProxy_IDirect3DIndexBuffer9` / `CProxy_IDirect3DVertexBuffer9`
- `CProxy_IDirect3DPixelShader9` / `CProxy_IDirect3DVertexShader9` / `CProxy_IDirect3DVertexDeclaration9`
- `CProxy_IDirect3DStateBlock9` / `CProxy_IDirect3DSwapChain9`
- `CProxy_IDirect3DResource9` / `CProxy_IDirect3DQuery9`
- `CProxy_IDirectXVideoAccelerationService` / `CProxy_IDirectXVideoProcessor` / `CProxy_IDirectXVideoProcessorService`

This proxy layer intercepts D3D9 calls, likely for GPU pipeline instrumentation, resource tracking, or software fallback rendering. The presence of `DXVA2CreateVideoService` and DirectX VA proxies indicates hardware-accelerated video decoding integration.

---

## Key Architecture Points

1. **Custom factory export** — `GetPipelineCreateFunctions` (not standard COM DllGetClassObject)
2. **DirectShow integration** — Uses `msdmo.dll` (DirectX Media Object helpers) for media type manipulation
3. **GPU-accelerated pipeline** — Full Direct3D 9 + DXVA2 chain for hardware video processing
4. **Effect system** — `CEffect`/`CEffectNode` classes for applying transitions and visual effects
5. **Audio pipeline** — `CAudioMixerNode` for audio mixing, silence generation, resampling
6. **Source types** — File source wrapper, solid color generator, silence audio generator, image drawer
7. **Frame control** — Frame eating/throttling, deinterlacing, inverse telecine
8. **GFX interoperability** — `VMGGetClipCreateFunctions` import suggests integration with a Video Mixing Graph (VMG)
9. **Cross-DLL CLSID sharing** — Uses `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` (CLSID_VideoTrim) — see also WLXVideoTrim.dll and WLXMediaPublishSubscribe.dll
10. **No standard ATL COM pattern** — No DllGetClassObject/DllCanUnloadNow, no ATL module. Uses `GetPipelineCreateFunctions` + `VMGGetClipCreateFunctions` as factory entry points
