# Shared Import Patterns Analysis

## 1. Universal Baseline (every binary)

Every binary imports **KERNEL32.dll** (process/thread/memory management) and **MSVCR110.dll** (Visual C++ 2012 runtime). This is the irreducible common denominator.

## 2. The "Base Library" Pattern (WLXPhotoBase.dll consumers)

**Members:** MovieMaker.exe, MovieMakerCore.dll, WLXSlideshow.dll, WLXVideoTrim.dll, WLXFaceRecognition.dll, WLXPhotoCinematic.dll, WLXMediaPublishSubscribe.dll, WLXCodecHost.exe, WLXTranscode.exe, WLXMovieLibrary.dll

**Shared API surface from WLXPhotoBase.dll:**
- `?Delete@BasePrivate@@YAXPAX@Z` — memory deallocation (all 10 consumers)
- `?New@BasePrivate@@YAPAXI_N@Z` — memory allocation (9 of 10, all except MovieMaker.exe)
- `??1Exception@Base@@UAE@XZ` — exception destructor (9 of 10)
- `?Throw@Base@@YGXJ@Z` — exception throwing (9 of 10)
- `?BaseAtlThrow@ATL@@YGXJ@Z` — ATL exception bridge (9 of 10)
- `?ThrowLastError@Base@@YGXXZ` — Win32 error→exception (7 of 10)
- `?GetBaseStringManager@String@Base@@...` — ATL string manager (7 of 10)
- `?IsWin7OrGreater@OS@Base@@YG_NXZ` — OS version check (7 of 10)
- `?IsWin8OrGreater@OS@Base@@YG_NXZ` — OS version check (6 of 10)

**Interpretation:** WLXPhotoBase.dll serves as a shared foundational library providing memory management, exception handling, string support, and OS version detection. All application-level DLLs depend on it.

## 3. The COM/OLE Pattern (ole32.dll + OLEAUT32.dll)

**Members:** MovieMakerCore.dll, WLXSlideshow.dll, WLXVideoTrim.dll, WLXFaceRecognition.dll, WLXPhotoCinematic.dll, WLXPipeline.dll, WLXPipetran.dll, WLXMediaPublishSubscribe.dll, WLXCodecHost.exe, WLXTranscode.exe, WLMFDS.dll, WLMFReadWrite.dll, WLXMovieLibrary.dll, WLXMP4Parser.dll, WLXPhotoBase.dll

**Shared functions:**
- `CoTaskMemAlloc` / `CoTaskMemFree` / `CoTaskMemRealloc` — universal COM memory
- `CoInitialize` / `CoUninitialize` — COM apartment initialization
- `CoCreateInstance` — COM object instantiation

**Interpretation:** The entire application is COM-based. Every module that does anything beyond trivial operations uses COM.

## 4. The Media Foundation Pipeline

**Members:** MovieMakerCore.dll, WLXTranscode.exe, WLMFDS.dll, WLMFReadWrite.dll, WLXMovieLibrary.dll

**Shared MFPlat.DLL functions:**
- `MFCreateAttributes` — attribute store creation
- `MFCreateMediaType` — media type creation
- `MFCreateMemoryBuffer` / `MFCreateSample` — media buffer/sample creation
- `MFCreateSourceResolver` — source resolution
- `MFCreateCollection` — object collection
- `MFPutWorkItemEx` / `MFInvokeCallback` — async work scheduling
- `MFLockPlatform` / `MFUnlockPlatform` — platform lifecycle
- `MFCreateAlignedMemoryBuffer` — aligned memory allocation

**MF.dll functions:**
- `MFGetService` — service querying (MovieMakerCore, WLXTranscode, WLMFReadWrite, WLXMovieLibrary)
- `MFCreateSourceResolver` — (WLMFDS, WLMFReadWrite)
- `MFCreateTranscodeProfile/Topology` — (WLXTranscode only)
- `MFCreatePresentationClock` — (WLMFReadWrite only)
- `MFCreateASFMediaSink` — (WLMFReadWrite only)

**Interpretation:** Media Foundation is the core media processing framework. The pipeline split is: MovieMakerCore owns playback/preview, WLXTranscode owns transcoding, WLMFDS/WLMFReadWrite own internal streaming/source resolution, WLXMovieLibrary owns library management.

## 5. The DirectX Rendering Stack

### 5a. Direct3D 11 + DXVA (hardware video decoding)

**Members:** MovieMakerCore.dll, WLXTranscode.exe, WLXMovieLibrary.dll

**Shared imports:**
- `d3d11.dll` → `D3D11CreateDevice`
- `dxva2.dll` → `DXVA2CreateDirect3DDeviceManager9`, `DXVA2CreateVideoService`

**Interpretation:** These three modules share the modern D3D11/DXVA video acceleration path for hardware-accelerated video decoding and rendering.

### 5b. Direct3D 9 (legacy fallback)

**Members:** MovieMakerCore.dll, WLXSlideshow.dll, WLXPipeline.dll, WLXTranscode.exe, WLXMovieLibrary.dll

**Shared imports:**
- `d3d9.dll` → `Direct3DCreate9` or `Direct3DCreate9Ex`

**Interpretation:** D3D9 is used as a fallback/compatibility rendering path, also used in slideshow and pipeline rendering.

### 5c. Direct2D/DirectWrite (UI rendering)

**Members:** MovieMakerCore.dll only

**Unique imports:**
- `d2d1.dll` (ordinal 1 — `D2D1CreateFactory`)
- `DWrite.dll` → `DWriteCreateFactory`
- `D3DCOMPILER_46.dll` → `D3DGetInputSignatureBlob`, `D3DReflect`

**Interpretation:** The Direct2D/DirectWrite modern text and UI rendering stack is exclusive to MovieMakerCore.dll, which hosts the DirectUI framework via UXCore.dll.

## 6. The d3dx9_32.dll 3D Effects Subsystem

**Members:** WLXSlideshow.dll, WLXPipeline.dll, WLXPipetran.dll, WLXPhotoCinematic.dll

**Shared imports:**
- `D3DXMatrixScaling` — used by WLXSlideshow, WLXPipeline, WLXPipetran
- `D3DXMatrixTranslation` — WLXPipeline, WLXPipetran
- `D3DXMatrixMultiply` — WLXPipeline, WLXPipetran
- `D3DXMatrixLookAtLH` — WLXPipetran, WLXPhotoCinematic
- `D3DXMatrixPerspectiveFovLH` — WLXPipetran, WLXPhotoCinematic

**WLXPipetran unique:** `D3DXCreateMeshFVF`, `D3DXLoadMeshFromXResource`, `D3DXLoadMeshFromXW`, `D3DXCompileShader`, `D3DXCreateTextureShader`, `D3DXFillTextureTX` — 3D mesh loading and shader compilation for transition effects.

**WLXSlideshow unique:** `D3DXCreateFontW`, `D3DXCreateSprite` — 2D sprite-based slideshow rendering.

**Interpretation:** The D3DX9 utility library powers the 3D transition effects (pipetran = pipeline transitions), slideshow rendering, and cinematic effects. WLXPipetran has the most sophisticated usage including mesh loading and shader compilation.

## 7. The Telemetry Group (SQM + BICI)

**Members:** MovieMakerCore.dll, WLXSlideshow.dll, WLXFaceRecognition.dll, WLXMediaPublishSubscribe.dll, WLXPipeline.dll

**WLXPhotoSqm.dll shared functions:**
- `?Startup@Sqm@@YGXXZ` / `?Shutdown@Sqm@@YGXXZ` — lifecycle
- `?AddToStream@Sqm@@YGXKK@Z` — basic event logging
- Various `AddToStream` overloads with timers/tuples

**DmxBici.dll shared functions:**
- `?AddStringToDataPoint@BiciWrapper@@YG_NKKPB_W@Z`
- `?StartExperience@BiciWrapper@@YGJW4BiciStartupId@1@@Z`
- `?EndExperience@BiciWrapper@@YGJXZ`
- `?TransferExperienceToWeb@BiciWrapper@@YG_NPB_WPAPA_W@Z`

**Interpretation:** SQM (Service Quality Monitoring) and BICI (Binary Instrumentation and Customer Insights) form the telemetry infrastructure. MovieMakerCore owns the most comprehensive telemetry integration, while other modules report specific feature usage.

## 8. The SHLWAPI Path Utilities Group

**Members:** MovieMakerCore.dll, WLXPhotoBase.dll, WLXSlideshow.dll, WLXVideoTrim.dll, WLXPhotoCinematic.dll, WLXPipeline.dll, WLXPipetran.dll, WLXMediaPublishSubscribe.dll, WLXTranscode.exe, WLMFDS.dll, WLXMovieLibrary.dll

**Common functions:**
- `PathAddBackslashW` / `PathRemoveBackslashW` — path normalization
- `PathAppendW` — path concatenation
- `PathFileExistsW` — file existence check
- `PathFindExtensionW` / `PathFindFileNameW` / `PathRemoveFileSpecW` — path decomposition
- `StrCmpIW` — case-insensitive string comparison

**Interpretation:** File path manipulation is ubiquitous. Every module that accesses files uses SHLWAPI for safe path handling.

## 9. The Windows Shell Integration Group

**Members:** MovieMakerCore.dll, WLXSlideshow.dll, WLXVideoTrim.dll, WLXMediaPublishSubscribe.dll, WLXCodecHost.exe, WLXTranscode.exe, WLMFDS.dll, WLXMovieLibrary.dll

**Common functions:**
- `SHCreateItemFromParsingName` — shell item creation (most common)
- `ShellExecuteW` / `ShellExecuteExW` — launching external processes
- `SHGetPropertyStoreFromParsingName` — property store access

**Interpretation:** Shell integration is spread across the application for file system interaction, property access, and launching external handlers.

## 10. The GDI+ Imaging Group

**Members:** MovieMakerCore.dll, WLXSlideshow.dll, WLXPipeline.dll, WLXPipetran.dll, WLXMediaPublishSubscribe.dll, WLXCodecHost.exe, WLXMP4Parser.dll

**Common functions:**
- `GdiplusStartup` / `GdiplusShutdown` — lifecycle
- `GdipAlloc` / `GdipFree` — memory
- `GdipDisposeImage` / `GdipCloneImage` — image management
- `GdipGetImageWidth` / `GdipGetImageHeight` — dimension queries
- `GdipCreateBitmapFromStream` / `GdipCreateBitmapFromScan0` — bitmap creation

**WLXMediaPublishSubscribe has the most extensive GDI+ usage:** text rendering (GdipDrawString, GdipMeasureString), string format manipulation, matrix transforms, world transforms.

**WLXPipetran uses GDI+ for:** text rendering (GdipDrawString, fonts), path operations (GdipAddPathString, GdipDrawPath), matrix transforms.

**Interpretation:** GDI+ is the primary 2D imaging library, used for image decoding, thumbnail generation, and text rendering across the pipeline.

## 11. The Windows Audio/Wave Group

**Members:** MovieMakerCore.dll, WLXPipeline.dll, WLXMovieLibrary.dll, WLXMP4Parser.dll, WLMFDS.dll

**Shared functions:**
- `timeBeginPeriod` / `timeEndPeriod` — timer resolution (all)
- `timeGetTime` — timer reading (WLXPipeline, WLMFDS)
- `timeSetEvent` / `timeKillEvent` — multimedia timers (WLXPipeline, WLXMP4Parser, WLMFDS)

**WLXMovieLibrary unique:** `waveOutOpen`, `waveOutPrepareHeader`, `waveOutWrite`, `waveOutClose` — legacy wave audio output.

## 12. The Authentication/Online Services Group

**Members:** WLXMediaPublishSubscribe.dll, MovieMakerCore.dll

**wlidcli.dll (Windows Live ID):**
- Used by both MovieMakerCore and WLXMediaPublishSubscribe

**WINHTTP.dll / WININET.dll / CRYPT32.dll:**
- Exclusive to WLXMediaPublishSubscribe.dll

**Interpretation:** WLXMediaPublishSubscribe is the networking/online component, handling HTTP requests, URL caching, data protection, and authentication. MovieMakerCore only uses wlidcli for Live ID integration.

## 13. The Database Group

**Members:** MovieMakerCore.dll only

**Unique imports:** ESENT.dll (Extensible Storage Engine) — `JetInit2`, `JetBeginTransaction`, `JetCommitTransaction`, `JetCreateDatabaseW`, `JetOpenTableW`, `JetRetrieveColumn`, `JetSetColumn`, etc. (31 Jet* functions)

**Interpretation:** MovieMakerCore is the sole database consumer, using ESENT as the embedded database engine for media library metadata storage and indexing.

## 14. The Shared (Non-Photo Gallery) DLLs

| DLL | Location | Consumers | Role |
|-----|----------|-----------|------|
| WLMFDS.dll | Shared\ | WLXTranscode.exe, WLXMovieLibrary.dll (inferred) | Media Foundation Data Source / streaming |
| WLMFReadWrite.dll | Shared\ | WLXTranscode.exe (inferred) | Media Foundation ASF read/write |
| WLXMovieLibrary.dll | Shared\ | MovieMakerCore.dll (inferred) | Media library management, audio playback |

These shared DLLs form the reusable "backend" infrastructure, while the Photo Gallery DLLs are the "frontend" modules.

## 15. Summary of Key Architectural Insights

1. **WLXPhotoBase.dll is the universal base** — all 10 application binaries depend on it for memory, exceptions, and OS detection
2. **MovieMakerCore.dll is the monolith** — it imports from 30+ distinct DLLs including Direct2D, Direct3D 11, ESENT, Media Foundation, and DirectUI
3. **WLXPipetran.dll has the richest 3D capabilities** — it compiles shaders, loads 3D meshes from X files, and performs extensive matrix math
4. **WLXMediaPublishSubscribe.dll is the networking hub** — it's the only module with HTTP, URL caching, and data protection capabilities
5. **WLXVideoTrim.dll bridges two codec worlds** — it uses both Windows Media (WMVCore) and MP4 (WLXMP4Parser) codec stacks
6. **WLXTranscode.exe is the self-contained transcoder** — it has its own MF pipeline + DXVA acceleration, separate from MovieMakerCore
7. **Telemetry is pervasive** — SQM and BICI are embedded in most modules for usage tracking
8. **No single binary has all capabilities** — the architecture is modular, with clear separation of rendering (MovieMakerCore), effects (WLXPipetran/Slideshow), transcoding (WLXTranscode), and networking (WLXMediaPublishSubscribe)
