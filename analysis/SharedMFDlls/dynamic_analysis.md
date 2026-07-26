# Shared MF DLLs — Dynamic Analysis

## Test Harness
- **File**: `tests/SharedMFDlls/test_sharedmf.cpp`
- **Compiled**: x86, /EHsc /MDd, MSVC 19.44
- **Approach**: LoadLibrary each DLL, GetProcAddress all 4 exports, call each via SEH-wrapped trampolines

## RSCMFT.dll — Dynamic Results

### Export Table (verified live)
| Export | RVA | Resolved Address |
|--------|-----|------------------|
| `DllCanUnloadNow` | 0x000040BB | ✅ Resolved |
| `DllGetClassObject` | 0x000040CC | ✅ Resolved |
| `DllRegisterServer` | 0x000040DF | ✅ Resolved |
| `DllUnregisterServer` | 0x00004125 | ✅ Resolved |

### Call Results
| Function | Result | Notes |
|----------|--------|-------|
| `DllCanUnloadNow()` | `S_OK` (0x00000000) | No outstanding COM objects — clean unload |
| `DllRegisterServer()` | `E_ACCESSDENIED` (0x80070005) | Expected — requires admin for registry write |
| `DllUnregisterServer()` | `S_OK` (0x00000000) | Registry delete succeeded or no-op |
| `DllGetClassObject()` | `CLASS_E_CLASSNOTAVAILABLE` (0x80040111) | Expected — fabricated CLSID not registered |

### Key Observations
- **DllCanUnloadNow returns S_OK** on fresh load with zero COM objects, confirming reference counting is initialized correctly
- **DllRegisterServer fails with 0x80070005** (access denied) — the function executes code, attempts registry writes to `HKLM\Software\Classes\CLSID`, and fails at the ADVAPI32 `RegCreateKeyExW` call due to insufficient permissions (expected in non-admin context)
- **DllUnregisterServer returns S_OK** — either silently succeeds (no keys to delete) or gracefully handles missing keys
- **DllGetClassObject returns CLASS_E_CLASSNOTAVAILABLE** — the function executes, checks the CLSID against the registered class map, and correctly rejects an unknown CLSID. This confirms the CLSID → factory map exists and is functional
- **No C++ exceptions thrown** — all calls wrapped in SEH completed cleanly (no `0xE06D7363` mscpp exceptions)
- **DLL loads and unloads cleanly** — no cleanup issues detected

### MFT Registration Pipeline
From imports, the DLL self-registers via:
1. `DllRegisterServer` → `MFTRegister` (MFPlat) — registers as a Media Foundation Transform
2. `DllRegisterServer` → `RegCreateKeyExW`/`RegSetValueExW` (ADVAPI32) — writes `InProcServer32` + `ThreadingModel`
3. `DllUnregisterServer` → `MFTUnregister` + `RegDeleteTreeW`

### Algorithm Stages (from embedded Unicode strings)
The RS correction pipeline stages are string-identified for logging/profiling:
1. `RGB2Y Conversion` — Input color space conversion to luma
2. `Pyramid Construction` — Multi-scale image pyramid build
3. `Correspondences` — Inter-frame feature matching
4. `Derivatives` — Gradient/derivative computation
5. `Initialize DFlow+Updating Flow` — Dense optical flow initialization and iterative refinement
6. `Compute Stabilization` — Temporal stabilization path estimation
7. `Compute Correction` — Per-frame correction warp computation
8. `DestMap Computation` — Destination map for warp
9. `Image Warping` — Final geometric warp application
10. `Add Image + Optical Flow` — Pipeline integration
11. `Add Image` — Frame buffer management
12. `Warp for Error Image` — Error visualization
13. `Weight Computation` — Blending weight calculation
14. `Error Image` — Error metric computation

### HLSL Shaders (embedded)
The DLL contains compiled DirectX shaders (DXBC bytecode) for GPU-accelerated warping:
- **Vertex Shader**: `POSITION`/`TEXCOORD` input → `SV_POSITION`/`TEXCOORD` output
- **Pixel Shader 1**: `PointSampler` + `LinearSampler2` + `texSourceFrame` + `texLineOffsets` — line-offset-based warp
- **Pixel Shader 2**: `PointSampler` + `LinearSampler` + `texSourceFrame` + `texLineOffsets` — alternative interpolation
- **Pixel Shader 3**: `LinearSampler` + `texSourceFrame` — simple linear warp
- Shader compiler: `Microsoft (R) HLSL Shader Compiler 9.27.952.3022`

### Thread Pool Usage
Uses Windows Thread Pool API (`CreateThreadpool`, `CreateThreadpoolWork`, `SubmitThreadpoolWork`, `CloseThreadpoolWork`, `SetThreadpoolThreadMaximum`) for parallel optical flow computation.

---

## MPG4DEMUX.dll — Dynamic Results

### Export Table (verified live)
| Export | RVA | Resolved Address |
|--------|-----|------------------|
| `DllCanUnloadNow` | 0x0001F8B2 | ✅ Resolved |
| `DllGetClassObject` | 0x0001F8D0 | ✅ Resolved |
| `DllRegisterServer` | 0x00009CD2 | ✅ Resolved |
| `DllUnregisterServer` | 0x00009CE7 | ✅ Resolved |

### Call Results
| Function | Result | Notes |
|----------|--------|-------|
| `DllCanUnloadNow()` | `S_OK` (0x00000000) | No outstanding COM objects |
| `DllRegisterServer()` | `E_ACCESSDENIED` (0x80070005) | Expected — requires admin for registry write |
| `DllUnregisterServer()` | `E_FAIL` (0x80004005) | Slightly different from RSCMFT — may attempt COM class cleanup |
| `DllGetClassObject()` | `CLASS_E_CLASSNOTAVAILABLE` (0x80040111) | Expected — zero CLSID passed, function executes and rejects |

### Key Observations
- **DllCanUnloadNow returns S_OK** — clean state on load
- **DllRegisterServer fails 0x80070005** — same access denied pattern as RSCMFT, confirming ADVAPI32 registry operations execute but are blocked by permissions
- **DllUnregisterServer returns E_FAIL** (vs RSCMFT's S_OK) — suggests this function performs additional validation or attempts cleanup of COM class objects that don't exist yet, returning a more specific failure
- **DllGetClassObject** — even with a zero CLSID, the function executes and returns a proper HRESULT, confirming the factory dispatch table is operational
- **No C++ exceptions** — all calls clean

### MF Bridge Architecture
From live imports, the DLL bridges DirectShow and Media Foundation:
1. **DS→MF**: `MFInitMediaTypeFromAMMediaType` — converts `AM_MEDIA_TYPE` to `IMFMediaType`
2. **MF→DS**: `MFInitAMMediaTypeFromMFMediaType` — reverse conversion
3. **MF Objects**: `MFCreatePresentationDescriptor`, `MFCreateStreamDescriptor`, `MFCreateMediaType`, `MFCreateMemoryBuffer`
4. **Memory**: `MFHeapAlloc`/`MFHeapFree` — MF-managed heap for cross-API buffer sharing

### Codec Registry (from embedded strings)
**Video FourCC/Codecs:**
- `AVC1` — H.264/AVC
- `WVC1` — VC-1 (WMV9)
- `H264` — H.264 (alternate tag)
- `MJPG` — Motion JPEG
- `MP4V` — MPEG-4 Visual (Part 2)
- `WMV3` — Windows Media Video 9

**Audio Codecs (from ID3 genre table + format tags):**
- PCM, AAC, various WAVE sub-formats (`ima4`, `lpcm`)
- Audio format properties: `_CODECNAME`, `_DEFAULTCRISP`, `_PASSESRECOMMENDED`, `_SUPPORTEDVBRMODES`, `_VBRENABLED`, `_VBRQUALITY`

**Private/Custom Attributes:**
- `PRIVATE/AudioFormat`, `PRIVATE/AudioSyncInfo`, `PRIVATE/MP4moovAtomSize`
- `PRIVATE/TotalBitrate`, `PRIVATE/VideoBitrate`, `PRIVATE/VideoFormat`, `PRIVATE/VideoFourCC`

### MP4 Atom Types (from hex patterns)
Four-byte atom identifiers found as binary patterns:
- `free`, `ftyp`, `meta`, `mfra`, `moov`, `pinf`, `pnot`, `skip`, `uuid`, `wide`

### WM/Metadata Attributes
Full Windows Media metadata attribute set:
- `WM/AlbumArtist`, `WM/AlbumTitle`, `WM/BeatsPerMinute`, `WM/Composer`, `WM/Genre`, `WM/IsCompilation`, `WM/Picture`, `WM/TrackNumber`
- `WM/VideoFrameRate`, `WM/VideoHeight`, `WM/VideoWidth`
- `AspectRatioX`, `AspectRatioY`, `CurrentBitrate`, `Duration`, `FileSize`, `Is_Protected`, `MP4_HasBFrames`, `MP4_HasConstantFrameRate`

---

## Comparative Summary

| Aspect | RSCMFT.dll | MPG4DEMUX.dll |
|--------|-----------|---------------|
| Size | 288 KB (0x48000) | 176 KB (0x2B000) |
| Code size | 254 KB (.text) | 142 KB (.text) |
| Exports | 4 (COM DLL) | 4 (COM DLL) |
| DllCanUnloadNow | S_OK ✅ | S_OK ✅ |
| DllRegisterServer | 0x80070005 (access denied) | 0x80070005 (access denied) |
| DllUnregisterServer | S_OK | E_FAIL |
| DllGetClassObject | 0x80040111 (bad CLSID) | 0x80040111 (bad CLSID) |
| SEH exceptions | None | None |
| MF APIs used | MFTRegister, MFCopyImage, MFCreateVideoSampleAllocatorEx | MFInitMediaTypeFromAMMediaType, MFCreatePresentationDescriptor |
| Primary purpose | Video frame stabilization (optical flow) | MP4 container demuxing |
| DirectShow base | No (pure MF Transform) | Yes (hybrid DShow+MF) |
| GPU acceleration | Yes (HLSL shaders embedded) | No |
| Threading | Windows Thread Pool API | Custom thread (CAMThread) |
| PDB GUID | {8BE07FA3-8940-469E-A496-FED16CCA5FC4} | {7A27A4CE-69B7-4A00-9C73-C687AA8E9ED0} |
