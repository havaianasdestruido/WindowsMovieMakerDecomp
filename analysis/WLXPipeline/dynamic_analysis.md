# WLXPipeline.dll Dynamic Analysis

**File**: `undecomp/Photo Gallery/WLXPipeline.dll`  
**Version**: `16.4.3528.0331_ship.client.main.w5m4 (ship)`  
**PDB**: `WLXPipeline.pdb` `{911D33AE-5E96-461A-B0D1-1ECCB116070F}`  
**Build**: MSVC 11.00 (VS 2012), PE32 x86, 728 KB  
**Date**: Tue Apr 1 01:27:09 2014

---

## 1. Exported Functions (2)

| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 1 | 0x2314C | `GetPipelineCreateFunctions` |
| 2 | 0 | 0x23443 | `DllRegisterServer` |

### 1.1 GetPipelineCreateFunctions — Disassembly-Confirmed Signature

```c
HRESULT __stdcall GetPipelineCreateFunctions(
    DWORD dwVersion,      // [ebp+8]  — must be 1
    DWORD dwStructSize,   // [ebp+0Ch] — must be 0x1C (28 = 7 × sizeof(DWORD))
    void* pOutBuffer      // [ebp+10h] — output: array of 7 DWORD function pointers
);
```

**Validation logic** (disassembled from `0x1002314C`):
1. If `pOutBuffer == NULL` → return `E_POINTER (0x80004003)`
2. If `dwVersion != 1` → return `E_FAIL (0x80004005)`
3. If `dwStructSize != 0x1C` → return `E_FAIL (0x80004005)`
4. Fill `pOutBuffer[0..6]` with 7 hardcoded function pointers (absolute RVAs from `.text`)
5. Return `S_OK (0)`

**Confirmed runtime behavior**:
```
fn(1, 0x1C, NULL)           → E_POINTER   ✓
fn(0, 0x1C, valid_ptr)     → E_FAIL      ✓ (version check)
fn(1, 0x10, valid_ptr)     → E_FAIL      ✓ (size check)
fn(1, 0x1C, &buffer[7])    → S_OK        ✓ (fills 7 function pointers)
```

### 1.2 DllRegisterServer

Standard COM self-registration. Would register `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` → `WLXPipeline.dll`. Not executed in test harness to avoid registry modification.

---

## 2. Pipeline Factory Functions (7)

The 7 function pointers returned by `GetPipelineCreateFunctions` are all `__stdcall` with signature:

```c
HRESULT __stdcall CreatePipelineNode(void* pParam, DWORD* pdwFlags);
```

- First param: context pointer (NULL for most, required for func[4])
- Second param: output DWORD, flags or type identifier
- Returns: COM-like object with vtable in WLXPipeline.dll

### 2.1 Factory Function Table

| Index | RVA | VA (dynamic) | Behavior | Notes |
|-------|-----|-------------|----------|-------|
| 0 | 0x4F3CE | Dynamic | Creates node, 8+ vtable entries | Large vtable, likely a pipeline node (CNode-derived) |
| 1 | 0x29A86 | Dynamic | Creates node, 6 vtable entries | Smaller vtable, possibly a simpler node/transform |
| 2 | 0x280C5 | Dynamic | Creates node, 8+ vtable entries | Similar profile to func[0] |
| 3 | 0x2813F | Dynamic | Creates node, 8+ vtable entries | Similar profile to func[2] |
| 4 | 0x2819D | Dynamic | **Requires non-NULL first param** | Returns E_POINTER if pParam==NULL; likely a context-dependent factory |
| 5 | 0x2DCB8 | Dynamic | Creates node, 8+ vtable entries | Unique vtable layout; may be pipeline timeline/graph object |
| 6 | 0x244CC | Dynamic | Creates node, 8+ vtable entries | vtable[0]==vtable[1] (same function); unusual pattern |

### 2.2 Factory Function vtable Analysis

**func[0]** — vtable at 0x709B6F9C:
```
[0] 0x709FED82  QueryInterface
[1] 0x709F1CEB  AddRef (shared with other vtables)
[2] 0x709FF021  Release
[3] 0x709FE1F6  [interface method 1]
[4] 0x709FE2AD  [interface method 2]
[5] 0x709FD7AA  [interface method 3]
[6] 0x709FD849  [interface method 4]
[7] 0x709FE394  [interface method 5]
```

**func[1]** — vtable at 0x709B25B0:
```
[0] 0x709D9171  QueryInterface
[1] 0x709D858D  AddRef
[2] 0x709D947D  Release
[3] 0x709D85A8  [method]
[4] 0x709D9A1A  [method]
[5] 0x709D86A2  [method]
[6] 0x70695043  ← MSVCR110.dll (purecall/destructor)
[7] 0x6E696C65  ← "nile" (ASCII string artifact, likely data pointer)
```

**func[6]** — vtable at 0x709B1BFC:
```
[0] 0x709D4635  QI
[1] 0x709D4635  ← Same as [0] (unusual: QI mapped to same entry)
[2] 0x709D42F4  Release
[3] 0x709D4437  [method]
[4] 0x709F8903  [method]
[5] 0x709D461D  [method]
[6] 0x709D450B  [method]
[7] 0x709D456B  [method]
```

---

## 3. D3D9 Proxy Layer — Complete vtable Mapping

### 3.1 Proxy Class Hierarchy

The DLL implements a complete **Direct3D9 proxy layer** using a layered ATL template hierarchy:

```
CComObjectRootEx<CComMultiThreadModel>
  └─ CATLIdenityObject (identity tracking)
       └─ CPipelineObject (pipeline integration)
            └─ CNonControlledObject (ref-counting policy 1)

CComControlledObject<CComMultiThreadModel>
  (ref-counting policy 2)

Both policies wrap:
  └─ CBaseD3DProxyBase (common proxy base)
       └─ CBaseD3DProxy<Interface, RefPolicy>
            └─ CProxy_<Interface, RefPolicy>
                 └─ CUnknownImpl<CProxy<Interface, RefPolicy>, RefPolicy>
```

### 3.2 Intercepted D3D9/DXVA Interfaces (17 unique)

Each proxy class wraps a specific Direct3D 9 or DirectX Video Acceleration interface:

| # | Proxy Class | D3D9 Interface | Purpose |
|---|------------|----------------|---------|
| 1 | `CProxy_IDirect3DDevice9Ex` | `IDirect3DDevice9Ex` | GPU device (root proxy) |
| 2 | `CProxy_IDirect3DTexture9` | `IDirect3DTexture9` | 2D texture resource |
| 3 | `CProxy_IDirect3DBaseTexture9` | `IDirect3DBaseTexture9` | Base texture (parent of texture types) |
| 4 | `CProxy_IDirect3DCubeTexture9` | `IDirect3DCubeTexture9` | Cube map texture |
| 5 | `CProxy_IDirect3DVolumeTexture9` | `IDirect3DVolumeTexture9` | 3D volume texture |
| 6 | `CProxy_IDirect3DSurface9` | `IDirect3DSurface9` | Render target / depth surface |
| 7 | `CProxy_IDirect3DVolume9` | `IDirect3DVolume9` | 3D volume data |
| 8 | `CProxy_IDirect3DIndexBuffer9` | `IDirect3DIndexBuffer9` | Index buffer |
| 9 | `CProxy_IDirect3DVertexBuffer9` | `IDirect3DVertexBuffer9` | Vertex buffer |
| 10 | `CProxy_IDirect3DPixelShader9` | `IDirect3DPixelShader9` | Pixel shader |
| 11 | `CProxy_IDirect3DVertexShader9` | `IDirect3DVertexShader9` | Vertex shader |
| 12 | `CProxy_IDirect3DVertexDeclaration9` | `IDirect3DVertexDeclaration9` | Vertex input layout |
| 13 | `CProxy_IDirect3DStateBlock9` | `IDirect3DStateBlock9` | Render state snapshot |
| 14 | `CProxy_IDirect3DSwapChain9` | `IDirect3DSwapChain9` | Swap chain (presentation) |
| 15 | `CProxy_IDirect3DQuery9` | `IDirect3DQuery9` | Occlusion/fence queries |
| 16 | `CProxy_IDirectXVideoProcessorService` | `IDirectXVideoProcessorService` | DXVA2 video processor service |
| 17 | `CProxy_IDirectXVideoProcessor` | `IDirectXVideoProcessor` | DXVA2 video processor |

**Additional DXVA proxy**: `CProxy_IDirectXVideoAccelerationService` wraps `IDirectXVideoAccelerationService` but inherits from `CProxy_IDirectXVideoProcessorService` — indicating the acceleration service is a sub-interface of the processor service.

### 3.3 Proxy Instantiation Patterns

Each D3D9 proxy has **two instantiation variants** based on COM threading model:

| Variant | Ref-Counting Policy | Base Class | Use Case |
|---------|---------------------|------------|----------|
| NonControlled | `CNonControlledObject` | `CATLIdenityObject + CPipelineObject` | Internal pipeline objects (no external COM clients) |
| Controlled | `CComControlledObject` | `CComMultiThreadModel` | Exposed to external COM clients |

**Exception**: `CProxy_IDirect3DDevice9Ex` only has the NonControlled variant (with `CPipelineObject`), because the device proxy is the pipeline's internal root — it's never exposed as a standalone COM object.

### 3.4 Proxy Management Classes

| Class | Role |
|-------|------|
| `CD3DProxyDevice` | Master D3D9 device proxy manager; creates all sub-proxies |
| `CBaseD3DProxyBase` | Common base for all proxy classes |
| `CDrawLock@CD3DProxyDevice` | RAII lock for drawing operations |
| `CSurfaceManager` | Video surface lifecycle management |
| `CDeviceManager` | D3D9 device creation and lifecycle |

### 3.5 Dynamic D3D9/DXVA Loading

| DLL | Function | Loaded Via |
|-----|----------|-----------|
| `d3d9.dll` | `Direct3DCreate9Ex` | Static import |
| `d3dx9_32.dll` | `D3DXMatrixTranslation`, `D3DXMatrixScaling`, `D3DXMatrixMultiply`, `D3DXCreateEffectFromFileW` | Static import |
| `dxva2.dll` | `DXVA2CreateVideoService` | Dynamic (`LoadLibraryW`/`GetProcAddress`) at runtime |
| `DDRAW.dll` | `DirectDrawCreateEx`, `DirectDrawEnumerateExA` | Static import |
| `WindowsCodecs.dll` | `WICConvertBitmapSource` | Dynamic (`LoadLibraryW`/`GetProcAddress`) at runtime |

**Runtime verification**: Both `dxva2.dll` and `WindowsCodecs.dll` load successfully and export the expected functions on a standard Windows 10/11 system.

---

## 4. Pipeline Stage Architecture

### 4.1 Core Pipeline Classes (RTTI-extracted)

| Class | Role | DirectShow Base |
|-------|------|-----------------|
| `CNode` | Pipeline node base class | — |
| `CPipelineObject` | ATL COM pipeline object wrapper | — |
| `CPipelineStream` | Pipeline data stream | — |
| `CPipelineStreamBuilder` | Constructs pipeline stream graphs | — |
| `CPipelineTimeline` | Timeline management | — |
| `CPipelineItems` | Pipeline item collection | — |
| `CPipelineSourceItem` | Source item descriptor | — |
| `CPipelineTokenItem` | Token/item for pipeline operations | — |
| `CPipelineStreamItem` | Stream item descriptor | — |
| `CQueueManager` | Pipeline queue management | — |
| `CQueueProxy` | Queue proxy wrapper | — |
| `CSyncManager` | Synchronization manager | — |
| `CNodeConnection` | Inter-node connection | — |
| `CTokenManager` | Token lifecycle management | — |
| `CPipelineDbgMgr` | Debug/diagnostic manager | — |

### 4.2 Source Nodes

| Class | Source Type | Pin Name |
|-------|-----------|----------|
| `CBaseSource` | Abstract source base | — |
| `CSourceCreator` | Creates source nodes | — |
| `CFileSourceWrapper` | File-based source | — |
| `CMediaFileSource` | Media file reader | — |
| `CDShowSource` | DirectShow source wrapper | — |
| `CDShowVideoStream` | DirectShow video stream | — |
| `CImageSource` | Image source base | — |
| `CGDIImageSource` | GDI+ image source | — |
| `CWICImageSource` | WIC image source | — |
| `CSolidColorSource` | Solid color generator | `Pipeline\SolidColor` |
| `CSilenceAudioSource` | Silence audio generator | `Pipeline\SilenceAudio` |
| `CErrorSource` | Error/placeholder source | — |
| `CVid2SysMem` | Video-to-system-memory transfer | — |

### 4.3 Processing Nodes

| Class | Processing Stage | Parameters |
|-------|-----------------|------------|
| `CDeInterlacer` | Deinterlacing | `Deinterlace:`, `DisableDXVAHWDeinterlacer`, `Force InverseTelecinePattern` |
| `CFrameEat` | Frame dropping/throttling | `FrameEat`, `FrameEat In`, `FrameEat Out` |
| `CDVAspectRatio` | DV aspect ratio correction | `DVAspectRatio`, `DVAspectRatio In`, `DVAspectRatio Out` |
| `CFrameRateConvert` | Frame rate conversion | — |
| `CInterlacer` | Interlacing filter | — |
| `CInterlacePresenter` | Interlace presentation | — |
| `CEffect` | Effect base class | — |
| `CEffectNode` | Effect processing node | — |
| `CVideoEffect` | Video effect application | — |
| `CImageDrawer` | Image → surface rendering | `Image Drawer Pin`, `Image Drawer Sample`, `Image Drawer Allocator` |
| `CAudioMixerNode` | Audio mixing | `Pipeline\AudioMixer`, `Volume`, `MixingMode` |
| `CAudioRepackager` | Audio repacking/format conversion | — |
| `CMFAudioMixerDMO` | MF audio mixer DMO wrapper | — |
| `CMFTimeCompressionDMO` | Time compression DMO | — |

### 4.4 Renderer Nodes

| Class | Renderer | Pin |
|-------|----------|-----|
| `CBaseRenderer` | Base renderer | — |
| `CBaseVideoRenderer` | Video renderer base | — |
| `CBaseAudioRenderer` | Audio renderer base | `Base Audio Renderer` |
| `CProgressivePresentor` | Progressive scan presenter | — |
| `CDShowPresentor` | DirectShow presenter | — |
| `CVMRPresentor` | VMR (Video Mixing Renderer) presenter | — |
| `CImageSink` | Image output sink | `Image Sink` |

### 4.5 Transform / Effect System

| Class | Role |
|-------|------|
| `CTransformFilter` | DirectShow transform filter base |
| `CTransInPlaceFilter` | In-place transform filter |
| `CTransformInputPin` / `CTransformOutputPin` | Transform I/O pins |
| `CTransformProperties` | Transform property bag |
| `CTransformProperty` | Individual property |
| `CTransformPropertyPoint` | Point property |

**Known transform parameter names** (from Unicode strings):
- `TFX\Fade` — Fade transition effect
- `Scrunch` — Resize/fit transform
- `Opacity` — Alpha/opacity control
- `Color` — Color correction
- `Translation` — Translation transform
- `XForm In` / `XForm Out` — Transform input/output pins

### 4.6 Graph Infrastructure

| Class | Role |
|-------|------|
| `CGraph` | DirectShow filter graph base |
| `CGraphImpl<stcNodeInfo>` | Templated graph implementation |
| `CGraphImpl<stcVideoNodeInfo>` | Video-specific graph |
| `CVideoGraph` | Video processing graph |
| `CAudioGraph` | Audio processing graph |
| `CTNodeCollection` | Node collection within graph |
| `CNodeCollection` | Generic node collection |
| `CFilterGraphAlive` | Filter graph lifetime monitor |
| `CMediaPosition` | Media position tracking |
| `CPreviewControl` | Preview rendering control |

### 4.7 Buffer Management

| Class | Role |
|-------|------|
| `CBaseAllocator` | DirectShow allocator base |
| `CProxyAllocator` | VMR surface allocator proxy (`IVMRSurfaceAllocator9`) |
| `CMediaBuffer` | Standard media buffer |
| `CAudioBuffer` | Audio data buffer |
| `CResampledAudioBuffer` | Resampled audio buffer |
| `CDShowAudioBuffer` | DirectShow audio buffer |
| `CImageBufferProvider` | Image buffer provider |
| `CFileSourceBufferProvider` | File source buffer provider |
| `CAudioBufferManager` | Audio buffer pool manager |
| `CSurfaceManager` | GPU surface management |
| `CSurface` | Video surface wrapper |
| `SurfaceBitmap` | GDI+ bitmap surface |

---

## 5. Audio Pipeline Details

### 5.1 Audio Classes

| Class | Role |
|-------|------|
| `CAudioMixerNode` | Audio mixing pipeline node |
| `CAudioMixerNodeProps` | Audio mixer properties (Volume, MixingMode) |
| `CAudioGraph` | Audio processing graph |
| `CAudioMediaStream` | Audio media stream |
| `CAudioBuffer` | Audio sample buffer |
| `CResampledAudioBuffer` | Resampled audio sample |
| `CDShowAudioBuffer` | DirectShow audio sample |
| `CAudioBufferManager` | Audio buffer pool |
| `CAudioRepackager` | Audio format repacking |
| `CAudioFrameGrabber` | Audio frame extraction |
| `CBaseAudioRenderer` | Audio renderer base |
| `CBaseAudioRendererPin` | Audio renderer input pin |
| `CMFAudioMixerDMO` | MF Audio Mixer DMO wrapper |
| `CMultiFlexStreamsAudioBaseDMO` | Multi-stream audio DMO base |
| `CWMDSP1in1outDMO` | WM DSP 1-in/1-out DMO base |

### 5.2 Audio DMO (DirectX Media Object) Architecture

```
IMediaObject (base)
  └─ CMFBaseDMO<CMFAudioMixerDMO, 8, 0>
       └─ CMultiFlexStreamsAudioBaseDMO<CMFAudioMixerDMO, 8, 0>
            └─ CWMDSPDMOWMILoggingBase
                 └─ CMFAudioMixerDMO
```

The audio mixer uses the Windows Media DSP DMO framework for hardware-accelerated audio mixing.

### 5.3 Audio Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `Volume` | Float | Audio volume level |
| `MixingMode` | DWORD | Audio mixing mode selection |
| `InputCount` | DWORD | Number of audio inputs |

---

## 6. GDI+ / WIC Integration

### 6.1 GDI+ Functions Used

| Category | Functions |
|----------|-----------|
| Bitmap creation | `GdipCreateBitmapFromFile`, `FromStream`, `FromScan0`, `FromGdiDib`, `FromHBITMAP` |
| Bitmap conversion | `GdipCreateHBITMAPFromBitmap`, `GdipCreateBitmapFromHBITMAP` |
| Drawing | `GdipDrawImageRectI`, `GdipDrawImageRectRectI` |
| Graphics | `GdipGetImageGraphicsContext`, `GdipDeleteGraphics`, `GdipFlush` |
| Image info | `GdipGetImageWidth`, `Height`, `PixelFormat` |
| Metadata | `GdipGetPropertyItem`, `GdipGetPropertyItemSize` |
| Multi-frame | `GdipImageSelectActiveFrame`, `GetFrameCount`, `DimensionsList` |
| Encoding | `GdipGetImageEncoders`, `GetImageEncodersSize`, `GdipSaveImageToFile` |
| Memory | `GdipAlloc`, `GdipFree`, `GdipCloneImage`, `GdipDisposeImage` |
| Lifecycle | `GdiplusStartup`, `GdiplusShutdown` |

### 6.2 WIC (Windows Imaging Component)

- `WICConvertBitmapSource` — loaded dynamically from `WindowsCodecs.dll`
- Used for bitmap format conversion between pixel formats

### 6.3 Image Format Support

From Unicode string constants:
```
.bmp, .dib, .gif, .jpg, .jpeg, .jfif, .jpe, .png, .tif, .tiff, .emf, .wmf
```

### 6.4 Supported Media Formats

From Unicode string constants:
```
Video: .avi, .mpg, .mpe, .asf, .wmv, .dvr-ms
Audio: .wma, .mp3, .wav
Image: .bmp, .dib, .gif, .jpg, .jpeg, .jfif, .jpe, .png, .tif, .tiff, .emf, .wmf
```

---

## 7. Custom Factory Pattern

### 7.1 Architecture

This DLL is **not** standard ATL COM. It uses a custom factory pattern:

```
Export: GetPipelineCreateFunctions(1, 0x1C, buffer)
  → Returns 7 factory function pointers
    → Each factory creates a specific pipeline node type
```

The `CCreateFunction` / `CStaticObject<CCreateFunction>` RTTI entries indicate a static singleton factory registry pattern. The `GetPipelineCreateFunctions` export returns hardcoded function pointers (not dynamic lookups).

### 7.2 Missing Standard COM Exports

| Standard Export | Present | Purpose |
|----------------|---------|---------|
| `DllGetClassObject` | **No** | Standard COM class factory |
| `DllCanUnloadNow` | **No** | Reference counting check |
| `DllUnregisterServer` | **No** | Unregistration |

Instead, the DLL provides:
- `GetPipelineCreateFunctions` — custom factory (7 pipeline node creators)
- `DllRegisterServer` — COM registration only (for CLSID_VideoTrim)

### 7.3 CLSID Registration

The CLSID `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` (CLSID_VideoTrim) is registered to `wlxclip.dll`, not `WLXPipeline.dll`. This means `WLXPipeline.dll` uses this CLSID internally to create video trim objects via `CoCreateInstance`, delegating to `wlxclip.dll`.

**Runtime registry state**:
```
CLSID\{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}\InprocServer32
  → C:\Program Files (x86)\Windows Live\Photo Gallery\wlxclip.dll
  → ThreadingModel: Apartment
```

---

## 8. Pipeline Configuration

### 8.1 Registry Keys

| Key Path | Purpose |
|----------|---------|
| `Software\Microsoft\GPUPipeline` | GPU pipeline settings |
| `Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline` | WLX-specific GPU pipeline config |

Both keys were **not present** at test time (no WLMM installation).

### 8.2 Configuration Parameters

| Parameter | Source | Description |
|-----------|--------|-------------|
| `DXVAEnabled` | Registry | Enable DXVA hardware acceleration |
| `DXVAEnableForMM` | Registry | Enable DXVA for Movie Maker |
| `DisableDXVAHWDeinterlacer` | Registry/pipeline | Disable DXVA hardware deinterlacer |
| `Force InverseTelecinePattern` | Registry/pipeline | Force IVTC pattern |
| `MPEG2SeekOffset` | Pipeline | MPEG-2 seek offset adjustment |
| `MPEG2PrerollOffset` | Pipeline | MPEG-2 preroll offset |
| `CPU Downgrade Level` | Pipeline | CPU fallback level |

### 8.3 Resource Loading

The DLL loads resources via:
- `FindResourceW` / `FindResourceExW` — resource lookup
- `LoadResource` / `LockResource` / `SizeofResource` — resource access
- `D3DXCreateEffectFromFileW` — shader/effect file loading from `TFX\Fade` path

---

## 9. Cross-DLL Integration

### 9.1 Pipeline.dll Dependency

WLXPipeline.dll references `Pipeline.dll` (string at offset 0x96FD9) and uses `GetProcAddress` at runtime to look up:
- `GetPipelineCreateFunctions` from Pipeline.dll
- `VMGGetClipCreateFunctions` from Pipeline.dll (Video Mixing Graph clip creation)

`Pipeline.dll` is a **separate binary** (not included in this distribution). It appears to be the core pipeline framework that WLXPipeline.dll extends.

### 9.2 ETW (Event Tracing for Windows)

The DLL registers ETW providers via:
- `RegisterTraceGuidsW` / `UnregisterTraceGuids`
- `TraceEvent`, `GetTraceEnableFlags`, `GetTraceEnableLevel`, `GetTraceLoggerHandle`

Used for pipeline performance diagnostics.

### 9.3 COM/OLE Infrastructure

| Function | Purpose |
|----------|---------|
| `CoCreateInstance` | Create COM objects (VideoTrim, etc.) |
| `CoInitialize(Ex)` / `CoUninitialize` | COM initialization |
| `CoTaskMemAlloc` / `CoTaskMemFree` | COM task memory |
| `PropVariantClear` / `PropVariantCopy` | Property storage |
| `OLEAUT32` ordinals | VARIANT operations (SysAllocString, VariantInit, etc.) |

---

## 10. Complete RTTI Class Inventory (180+ classes)

### D3D9 Proxies (17)
`CProxy_IDirect3DDevice9Ex`, `CProxy_IDirect3DTexture9`, `CProxy_IDirect3DBaseTexture9`, `CProxy_IDirect3DCubeTexture9`, `CProxy_IDirect3DVolumeTexture9`, `CProxy_IDirect3DSurface9`, `CProxy_IDirect3DVolume9`, `CProxy_IDirect3DIndexBuffer9`, `CProxy_IDirect3DVertexBuffer9`, `CProxy_IDirect3DPixelShader9`, `CProxy_IDirect3DVertexShader9`, `CProxy_IDirect3DVertexDeclaration9`, `CProxy_IDirect3DStateBlock9`, `CProxy_IDirect3DSwapChain9`, `CProxy_IDirect3DQuery9`, `CProxy_IDirectXVideoProcessorService`, `CProxy_IDirectXVideoProcessor`

### Pipeline Core (20)
`CNode`, `CPipelineObject`, `CPipelineStream`, `CPipelineStreamBuilder`, `CPipelineTimeline`, `CPipelineItems`, `CPipelineSourceItem`, `CPipelineTokenItem`, `CPipelineStreamItem`, `CQueueManager`, `CQueueProxy`, `CSyncManager`, `CNodeConnection`, `CTokenManager`, `CPipelineDbgMgr`, `CNodeCollection`, `CTNodeCollection`, `CCreateFunction`, `CStaticObject`, `CDeviceManager`

### Sources (13)
`CBaseSource`, `CSourceCreator`, `CFileSourceWrapper`, `CMediaFileSource`, `CDShowSource`, `CDShowVideoStream`, `CImageSource`, `CGDIImageSource`, `CWICImageSource`, `CSolidColorSource`, `CSilenceAudioSource`, `CErrorSource`, `CVid2SysMem`

### Processing (18)
`CDeInterlacer`, `CFrameEat`, `CDVAspectRatio`, `CFrameRateConvert`, `CInterlacer`, `CInterlacePresenter`, `CEffect`, `CEffectNode`, `CVideoEffect`, `CImageDrawer`, `CImageDrawerAllocator`, `CImageDrawerPin`, `CImageDrawerSample`, `CAudioMixerNode`, `CAudioRepackager`, `CMFAudioMixerDMO`, `CMFTimeCompressionDMO`, `CTransformFilter`

### Renderers (8)
`CBaseRenderer`, `CBaseVideoRenderer`, `CBaseAudioRenderer`, `CBaseAudioRendererPin`, `CProgressivePresentor`, `CDShowPresentor`, `CVMRPresentor`, `CImageSink`

### Graph (10)
`CGraph`, `CGraphImpl`, `CVideoGraph`, `CAudioGraph`, `CFilterGraphAlive`, `CMediaPosition`, `CPreviewControl`, `CThumbnailControl`, `CPosPassThru`, `CRendererPosPassThru`

### Buffers (13)
`CBaseAllocator`, `CProxyAllocator`, `CMediaBuffer`, `CAudioBuffer`, `CResampledAudioBuffer`, `CDShowAudioBuffer`, `CImageBufferProvider`, `CFileSourceBufferProvider`, `CAudioBufferManager`, `CSurfaceManager`, `CSurface`, `SurfaceBitmap`, `CMediaSample`

### DirectShow Base Classes (15)
`CBaseObject`, `CBaseFilter`, `CBasePin`, `CBaseInputPin`, `CBaseOutputPin`, `CSource`, `CSourceStream`, `CStreamImpl`, `CEnumPins`, `CEnumMediaTypes`, `CByteStreamImpl`, `CResStream`, `CVideoMediaStream`, `CASyncMediaStream`, `CMediaStream`

### Miscellaneous (15+)
`CEvent`, `CCritSec`, `CUnknownLock`, `CTraceUnknownLock`, `CMonitorInfo`, `CPropertyBagImpl`, `CIdentityObjectImpl`, `CWMDSPBaseObject`, `CWMDSPComBase`, `CWMDSPPropImpl`, `CWMDSP1in1outDMO`, `CSampleQueue`, `CSegment`, `CSegmentList`, `CTimeline`

### Interfaces (IIDs)
`IID_IPipelineSourceResolver`, `IID_IPipelineSourceError`, `IID_IPreviewGraphEvent`

---

## 11. Test Harness Output

### Compilation

```
cl /EHsc /MDd test_pipeline.cpp /Fe:test_pipeline.exe /link ole32.lib oleaut32.lib advapi32.lib psapi.lib
```

### Runtime Results

```
GetPipelineCreateFunctions(1, 0x1C, &buf[7]) = S_OK
  → 7 factory function pointers retrieved
  → 6/7 create objects with vtables in WLXPipeline.dll
  → 1/4 requires non-NULL context parameter

Pipeline.dll: NOT FOUND (error 126) — separate binary dependency
dxva2.dll:    LOADED → DXVA2CreateVideoService at 0x71074290
WindowsCodecs.dll: LOADED → WICConvertBitmapSource at 0x715305E0

CLSID_VideoTrim: Registered to wlxclip.dll (Apartment threading)
GPUPipeline:     Not present
WLXGPUPipeline:  Not present
```
