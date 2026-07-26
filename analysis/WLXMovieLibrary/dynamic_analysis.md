# WLXMovieLibrary.dll — Dynamic Analysis

## Build Info
- **Timestamp**: 2014-04-01 01:26:24 UTC
- **PDB**: `{CBF27DEC-DD3B-4A76-A5DF-9EECCC486627}` → WLXMovieLibrary.pdb
- **Linker**: MSVC 11.0 (VS2012), /GS security cookies enabled
- **Code size**: 287 KB (.text), 5.5 KB (.data), 1 KB (.rsrc)
- **Image**: 316 KB total, ASLR+DEP, x86 PE32

## CreateMovieFactory — Confirmed Signature

### Prototype
```c
HRESULT __stdcall CreateMovieFactory(void** ppObject);
```

### Calling Convention
- **stdcall** (callee cleans stack)
- Hotpatch preamble: `MOV EDI,EDI / PUSH EBP / MOV EBP,ESP`
- Security cookie check at entry

### Behavior (from disassembly)
1. Security cookie validation
2. Calls `WLXPhotoBase!Base::GetBaseStringManager()` to get ATL string manager
3. If string manager is NULL, throws `E_FAIL` via `WLXPhotoBase!Base::Throw()`
4. Calls vtable method on string manager to get `CAtlStringMgr`
5. Constructs two string objects (WM metadata key/value pair)
6. Validates input pointer `[ebp+8]` — returns `E_POINTER (0x80004003)` if NULL
7. Calls internal function (likely `MovieFactory::Create`) with the output pointer
8. Returns `S_OK (0x00000000)` on success, or error HRESULT

### Arguments
| Param | Type | Description |
|-------|------|-------------|
| `[ebp+8]` | `void**` | Output pointer to created object. Must not be NULL. |

### Return Value
| HRESULT | Meaning |
|---------|---------|
| `0x00000000` (S_OK) | Success — object created |
| `0x80004003` (E_POINTER) | NULL output pointer |
| `0x80004005` (E_FAIL) | Internal creation failure |
| Other negative | Internal error from creation function |

## Dynamic Test Results (2026-07-26)

### Call Probing
| Call Pattern | HRESULT | Object Ptr | Notes |
|--------------|---------|------------|-------|
| `CreateMovieFactory(NULL)` | `0x80004003` (E_POINTER) | — | Correctly rejects NULL |
| `CreateMovieFactory(&ptr)` | `0x00000000` (S_OK) | `0x010D2C80` | **Object created** |
| `CreateMovieFactory(&ptr, 0)` | `0x00000000` (S_OK) | `0x010E91C0` | 2nd arg ignored |
| `CreateMovieFactory(&ptr, 0, 0)` | `0x00000000` (S_OK) | `0x010E9080` | 3rd arg ignored |

### Export Resolution
| Export | Present | Address |
|--------|---------|---------|
| `CreateMovieFactory` | Yes | `0xADD8` (ordinal 1) |
| `DllCanUnloadNow` | **No** | — |
| `DllGetClassObject` | **No** | — |
| `DllRegisterServer` | **No** | — |
| `DllUnregisterServer` | **No** | — |

**Key finding**: This DLL does NOT export standard ATL COM DLL functions (`DllGetClassObject`, `DllCanUnloadNow`, etc.). The only entry point is `CreateMovieFactory`. The DLL is NOT a self-registering COM server — it is loaded programmatically and creates objects through its exported factory function.

## IMovieFactory Vtable

The returned object exposes a vtable with 8 entries at `0x71FD5F7C`:

| Slot | Address | Likely Function |
|------|---------|-----------------|
| [0] | `0x71FE4D32` | QueryInterface |
| [1] | `0x71FE4CE3` | AddRef |
| [2] | `0x71FE4CFE` | Release |
| [3] | `0x71FE4997` | IMovieFactory method 1 (CreateMovie?) |
| [4] | `0x71FE49CD` | IMovieFactory method 2 |
| [5] | `0x71FE4A9F` | IMovieFactory method 3 |
| [6] | `0x71FE4AEC` | IMovieFactory method 4 |
| [7] | `0x71FE4D53` | IMovieFactory method 5 |

Release() works correctly — object is freed when refcount reaches 0.

## Delay-Load Dependencies

WLXMovieLibrary.dll uses delay-loaded imports for:

| DLL | Functions | Purpose |
|-----|-----------|---------|
| **WLXPhotoBase.dll** | 10 | Base utilities: memory (`New`, `Delete`), exceptions (`Throw`, `ThrowLastError`), OS version checks (`IsWin7OrGreater`, `IsWin8OrGreater`), string manager |
| **SHLWAPI.dll** | 7 | Path utilities: `PathAddBackslashW`, `PathAppendW`, `PathFileExistsW`, `PathFindFileNameW`, etc. |
| **SHELL32.dll** | 1 | `SHGetPropertyStoreFromParsingName` — file property access |

**Critical**: `WLXPhotoBase.dll` must be in the DLL search path. Without it, `CreateMovieFactory` crashes immediately on the first delay-loaded call. The DLL is not in `undecomp\Shared/` and must be sourced from the Photo Gallery installation.

## Disassembly: CreateMovieFactory First 256 Bytes

```
1000ADD8: mov edi,edi              ; hotpatch nop
1000ADDA: push ebp
1000ADDB: mov ebp,esp
1000ADDD: and esp,0FFFFFFF8h      ; 8-byte align
1000ADE0: sub esp,0Ch              ; locals
1000ADE3: push ebx / esi / edi    ; save regs
1000ADE6: mov eax,[10048170h]     ; security cookie
1000ADEB: mov ebx,10048170h       ; cookie addr
1000ADF0: cmp eax,ebx
1000ADF2: je 1000AE0D             ; skip if unchanged
1000ADF4: test [eax+1Ch],20h      ; /GS check
1000ADF8: je 1000AE0D
1000ADFA-1000AE08: security_cookie_report
1000AE0D: call [delay!WLXPhotoBase]   ; string manager init
1000AE13: mov ecx,eax
1000AE15: test ecx,ecx
1000AE17: jne 1000AE24
1000AE19: push 80004005h               ; E_FAIL
1000AE1E: call [delay!Base::Throw]
1000AE24: mov eax,[ecx]                ; vtable
1000AE26: call [eax+0Ch]              ; string manager method
1000AE29: add eax,10h                  ; offset into result
1000AE2C-1000AE82: build 2 CAtlString objects
1000AE87-1000AEBC: additional setup calls
1000AEC1: lea ecx,[edi-10h]
1000AEC4: call 1000B84C               ; cleanup string objects
1000AECB: mov esi,[esp+10h]           ; load result ptr
1000AECF: mov ecx,[ebp+8]            ; ← arg1 (output pointer)
1000AED2: test ecx,ecx
1000AED4: jne 1000AEDD
1000AED6: mov edi,80004003h          ; E_POINTER
1000AEDB: jmp 1000AEEA
1000AEDD: call 100145B7              ; create object → result
1000AEE2: mov edi,eax                ; save HRESULT
1000AEE4: test edi,edi
1000AEE6: js 1000AEEA                ; if error, skip zeroing
1000AEE8: xor edi,edi                ; edi = S_OK
1000AEEA: call 1000B84C              ; cleanup
1000AEF2: mov eax,edi                ; return HRESULT
1000AEF4-1000AEF7: pop regs, restore frame
```

## RTTI Classes (Complete Inventory)

### ATL Infrastructure
| RTTI | Class |
|------|-------|
| `.?AVCMovieDllModule@@` | ATL DLL module |
| `.?AV?$CAtlModuleT@VCMovieDllModule@@@ATL@@` | ATL module template |
| `.?AV?$CAtlDllModuleT@VCMovieDllModule@@@ATL@@` | ATL DLL module template |
| `.?AU?$CAtlValidateModuleConfiguration@$00VCMovieDllModule@@@ATL@@` | Module config |
| `.?AV?$CComObjectRootEx@VCComMultiThreadModel@ATL@@@ATL@@` | COM root (thread-safe) |
| `.?AVCComObjectRootBase@ATL@@` | COM root base |

### Core Classes
| RTTI | Class | Description |
|------|-------|-------------|
| `.?AVMovieFactory@@` | `MovieFactory` | Primary factory (created by export) |
| `.?AVMovie@@` | `Movie` | Single media file representation |
| `.?AVMovieBuilder@@` | `MovieBuilder` | Builder for constructing Movie objects |
| `.?AVSystemClock@@` | `SystemClock` | System clock wrapper |
| `.?AVThreadPool@@` | `ThreadPool` | Thread pool abstraction |

### ATL COM Wrappers
| RTTI | Class |
|------|-------|
| `.?AV?$CComObject@VMovieFactory@@@ATL@@` | CComObject<MovieFactory> |
| `.?AV?$CComObject@VMovie@@@ATL@@` | CComObject<Movie> |
| `.?AV?$CComObject@VMovieBuilder@@@ATL@@` | CComObject<MovieBuilder> |
| `.?AV?$CComObject@VSystemClock@@@ATL@@` | CComObject<SystemClock> |
| `.?AV?$CComObject@VWaveAudioRenderer@@@ATL@@` | CComObject<WaveAudioRenderer> |
| `.?AV?$CComObject@VAVSourceFactory@HMRAVSource@@@ATL@@` | CComObject<AVSourceFactory> |
| `.?AV?$CComObject@VAVSource@HMRAVSource@@@ATL@@` | CComObject<AVSource> |
| `.?AV?$CComObject@VAVSourceProxy@HMRAVSource@@@ATL@@` | CComObject<AVSourceProxy> |
| `.?AV?$CComObject@VAVSink@HMRAVSource@@@ATL@@` | CComObject<AVSink> |
| `.?AV?$CComObject@VAsyncSourceResolver@HMRAVSource@@@ATL@@` | CComObject<AsyncSourceResolver> |
| `.?AV?$CComObject@VDShowMFSourceReaderBuilder@HMRAVSource@@@ATL@@` | CComObject<DShowMFSourceReaderBuilder> |
| `.?AV?$CComObject@VNativeMFSourceReaderBuilder@HMRAVSource@@@ATL@@` | CComObject<NativeMFSourceReaderBuilder> |
| `.?AV?$CComObject@VXVideoProc@HMRAVSource@@@ATL@@` | CComObject<XVideoProc> |
| `.?AV?$CComObject@VDXVA2VideoProc@HMRAVSource@@@ATL@@` | CComObject<DXVA2VideoProc> |
| `.?AV?$CComObject@VSyncVideoSampleSource@HMRAVSource@@@ATL@@` | CComObject<SyncVideoSampleSource> |
| `.?AV?$CComObject@VMFByteStreamOnStream@HMRAVSource@@@ATL@@` | CComObject<MFByteStreamOnStream> |
| `.?AV?$CComObject@VMFByteStreamOnStreamAsyncResult@MFByteStreamOnStream@HMRAVSource@@@ATL@@` | CComObject<MFByteStreamOnStreamAsyncResult> |
| `.?AV?$CComObject@VAuthProvider@HMRAVSource@@@ATL@@` | CComObject<AuthProvider> |
| `.?AV?$CComObject@VAuthCredentials@HMRAVSource@@@ATL@@` | CComObject<AuthCredentials> |

### HMRAVSource Namespace (Audio/Video)
| RTTI | Class | Description |
|------|-------|-------------|
| `.?AVAVSourceFactory@HMRAVSource@@` | `AVSourceFactory` | Creates AV source objects |
| `.?AVAVSource@HMRAVSource@@` | `AVSource` | Media source wrapper |
| `.?AVAVSourceProxy@HMRAVSource@@` | `AVSourceProxy` | Source proxy for async access |
| `.?AVAVSink@HMRAVSource@@` | `AVSink` | Media sink |
| `.?AVStreamSinkHelper@HMRAVSource@@` | `StreamSinkHelper` | Stream sink helper |
| `.?AVMFRateControlHelper@HMRAVSource@@` | `MFRateControlHelper` | Playback rate control |
| `.?AVStreamSinkHost@HMRAVSource@@` | `StreamSinkHost` | Hosts stream sinks |
| `.?AVAsyncSourceResolver@HMRAVSource@@` | `AsyncSourceResolver` | Async source resolution |
| `.?AVMFSourceReaderBuilder@HMRAVSource@@` | `MFSourceReaderBuilder` | Base source reader builder |
| `.?AVNativeMFSourceReaderBuilder@HMRAVSource@@` | `NativeMFSourceReaderBuilder` | Native MF source reader |
| `.?AVDShowMFSourceReaderBuilder@HMRAVSource@@` | `DShowMFSourceReaderBuilder` | DShow bridge source reader |
| `.?AVXVideoProc@HMRAVSource@@` | `XVideoProc` | Software video processor |
| `.?AVDXVA2VideoProc@HMRAVSource@@` | `DXVA2VideoProc` | DXVA2 hardware video processor |
| `.?AVSyncVideoSampleSource@HMRAVSource@@` | `SyncVideoSampleSource` | Synchronized video sample source |
| `.?AVTextureInterOp@HMRAVSource@@` | `TextureInterOp` | Base texture interop |
| `.?AVTextureInterOpDX9@HMRAVSource@@` | `TextureInterOpDX9` | D3D9 texture interop |
| `.?AVTextureInterOpDX11@HMRAVSource@@` | `TextureInterOpDX11` | D3D11 texture interop |
| `.?AVMFByteStreamOnStream@HMRAVSource@@` | `MFByteStreamOnStream` | MF byte stream wrapper |
| `.?AVMFAsyncResult@HMRAVSource@@` | `MFAsyncResult` | MF async result |
| `.?AVAuthProvider@HMRAVSource@@` | `AuthProvider` | DRM auth provider |
| `.?AVAuthCredentials@HMRAVSource@@` | `AuthCredentials` | DRM credentials |

### Audio
| RTTI | Class | Description |
|------|-------|-------------|
| `.?AVWaveAudioRenderer@@` | `WaveAudioRenderer` | WaveOut audio renderer |
| `.?AVWaveDevice@@` | `WaveDevice` | WaveOut device wrapper |
| `.?AVAudioFormat@@` | `AudioFormat` | Audio format descriptor |
| `.?AVPcmFormat@@` | `PcmFormat` | PCM format descriptor |

### Transcode
| RTTI | Class | Description |
|------|-------|-------------|
| `.?AVTranscodeMetadataParser@HMRTranscode@@` | `TranscodeMetadataParser` | Transcode metadata parser |

### COM Interfaces
| RTTI | Interface | Description |
|------|-----------|-------------|
| `.?AUIMovieFactory@@` | `IMovieFactory` | Factory interface |
| `.?AUIMovie@@` | `IMovie` | Movie object interface |
| `.?AUIMovieBuilder@@` | `IMovieBuilder` | Movie builder interface |
| `.?AUIAudioRenderer@@` | `IAudioRenderer` | Audio renderer interface |
| `.?AUIAVClock@@` | `IAVClock` | AV clock interface |
| `.?AUIAVTickTimeSource@HMRAVSource@@` | `IAVTickTimeSource` | Tick time source |
| `.?AUIAVStreamSink@HMRAVSource@@` | `IAVStreamSink` | Stream sink interface |
| `.?AUIAVSourceFactory@HMRAVSource@@` | `IAVSourceFactory` | Source factory interface |
| `.?AUIAVSourceFactoryInternal@HMRAVSource@@` | `IAVSourceFactoryInternal` | Internal source factory |
| `.?AUIAVSource@HMRAVSource@@` | `IAVSource` | Source interface |
| `.?AUIAVSink@HMRAVSource@@` | `IAVSink` | Sink interface |
| `.?AUIAVSampleSource@HMRAVSource@@` | `IAVSampleSource` | Sample source interface |
| `.?AUIAVProcessor@HMRAVSource@@` | `IAVProcessor` | Processor interface |
| `.?AUIVideoProcessor@HMRAVSource@@` | `IVideoProcessor` | Video processor interface |
| `.?AUIVideoSampleSource@HMRAVSource@@` | `IVideoSampleSource` | Video sample source interface |
| `.?AUIMFByteStream@@` | `IMFByteStream` | MF byte stream interface |
| `.?AUIMFAsyncResult@@` | `IMFAsyncResult` | MF async result interface |
| `.?AUIMFAsyncCallback@@` | `IMFAsyncCallback` | MF async callback interface |
| `.?AUIMFSourceReaderCallback@@` | `IMFSourceReaderCallback` | MF source reader callback |
| `.?AUVideoBuffer@HMRAVSource@@` | `VideoBuffer` | Video buffer struct |
| `.?AUtagMFASYNCRESULT@@` | `tagMFASYNCRESULT` | MFAsyncResult struct |
| `.?AUIServiceProvider@@` | `IServiceProvider` | COM service provider |
| `.?AUIValidateBinding@@` | `IValidateBinding` | DRM binding validation |
| `.?AURefCountBaseMultiThreaded@Base@@` | `RefCountBaseMultiThreaded` | Thread-safe ref counting |

### Other
| RTTI | Class | Description |
|------|-------|-------------|
| `.?AVtype_info@@` | `type_info` | C++ RTTI type_info |
| `.?AVException@Base@@` | `Base::Exception` | WLXPhotoBase exception |
| `.?AVOnReadAsyncCallback@MFByteStreamOnStream@HMRAVSource@@` | `OnReadAsyncCallback` | Async read callback |
| `.?AVOnWriteAsyncCallback@MFByteStreamOnStream@HMRAVSource@@` | `OnWriteAsyncCallback` | Async write callback |
| `.?AVMFByteStreamOnStreamAsyncResult@MFByteStreamOnStream@HMRAVSource@@` | `MFByteStreamOnStreamAsyncResult` | Byte stream async result |

## Architecture Summary

```
CreateMovieFactory (export)
    │
    ▼
┌──────────────────────┐
│ MovieFactory         │ ← IMovieFactory COM interface
│ (ATL CComObject)     │
└──────┬───────────────┘
       │ creates
       ▼
┌──────────────────────┐
│ Movie                │ ← IMovie (media file representation)
│ (ATL CComObject)     │
└──────┬───────────────┘
       │ builds via
       ▼
┌──────────────────────┐
│ MovieBuilder         │ ← IMovieBuilder (builder pattern)
│ (ATL CComObject)     │
└──────────────────────┘

HMRAVSource namespace (internal):
┌──────────────────────┐
│ AVSourceFactory      │ ← Creates AV sources
│ AVSource             │ ← Media source wrapper
│ AVSourceProxy        │ ← Async proxy
│ DShowMFSourceReaderBuilder │ ← DShow → MF bridge
│ NativeMFSourceReaderBuilder │ ← Native MF reader
│ AsyncSourceResolver  │ ← Async source resolution
└──────────────────────┘

Video Processing:
┌──────────────────────┐
│ XVideoProc           │ ← Software video processing
│ DXVA2VideoProc       │ ← Hardware (DXVA2) processing
│ TextureInterOpDX9    │ ← D3D9 texture sharing
│ TextureInterOpDX11   │ ← D3D11 texture sharing
└──────────────────────┘

Audio:
┌──────────────────────┐
│ WaveAudioRenderer    │ ← WaveOut audio output
│ WaveDevice           │ ← WaveOut device
└──────────────────────┘

DRM:
┌──────────────────────┐
│ AuthProvider         │ ← CryptAPI-based auth
│ AuthCredentials      │ ← DRM credentials
│ IValidateBinding     │ ← Binding validation
└──────────────────────┘
```

## Key Findings

1. **Single export factory**: `CreateMovieFactory` is the sole entry point. No standard ATL COM exports.
2. **WLXPhotoBase.dll is critical**: Delay-loaded dependency that must be present for any functionality.
3. **Registry-based library**: Uses `HKCU\SOFTWARE\Microsoft\Windows Live\Common\Movie Library` — no SQL database.
4. **DXVA2 + D3D9/D3D11**: Hardware-accelerated video processing via both Direct3D 9 and 11 paths.
5. **DShow bridge**: `DShowMFSourceReaderBuilder` bridges DirectShow filters into Media Foundation pipeline.
6. **WaveOut audio**: Uses legacy WaveOut API (not WASAPI) for audio rendering.
7. **CryptAPI DRM**: Uses `CryptAcquireContextW`, `CryptSignHashW` for content protection validation.
8. **Thread pool**: Uses Windows Vista+ thread pool API for async operations.
9. **ETW tracing**: Full Event Tracing for Windows instrumentation.
10. **WM metadata**: Extensive Windows Media metadata property support (50+ properties).

## Source Files
- `tests/WLXMovieLibrary/test_library.cpp` — Test harness (LoadLibrary, export probing, signature detection, call probing)
- `tests/WLXMovieLibrary/test_library.exe` — Compiled test binary
- `analysis/WLXMovieLibrary/analysis.md` — Comprehensive static analysis
- `analysis/WLXMovieLibrary/dynamic_analysis.md` — This file (dynamic test results)
