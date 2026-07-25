# WLXMovieLibrary.dll Analysis

## Overview
Windows Live Movie Maker 2012 Movie Library management DLL. PE32 x86, linker v11.0 (VS2012), image base 0x10000000, ASLR+DEP. Timestamp: 2014-04-01 01:26:24 UTC. Build: 16.4.3528.0331_ship.client.main.w5m4. PDB GUID: `{CBF27DEC-DD3B-4A76-A5DF-9EECCC486627}`, PDB: `WLXMovieLibrary.pdb`.

**Purpose**: Manages the "Photo Gallery Movie Library" — a collection of media files (video/audio) available to Movie Maker. Provides a COM factory and COM objects for enumerating, indexing, and accessing media files. The library stores its configuration in the registry (not SQL).

## PE Structure
| Section | VA | VirtualSize | RawSize | Purpose |
|---------|-------|-------------|---------|---------|
| `.text` | 0x1000 | 0x46419 (287,769) | 0x46600 | Code |
| `.data` | 0x48000 | 0x1658 (5,720) | 0x1200 | R/W initialized data (vtables, globals) |
| `.rsrc` | 0x4A000 | 0x400 (1,024) | 0x400 | Resources |
| `.reloc` | 0x4B000 | 0x3CB0 (15,536) | 0x3E00 | Base relocations |

**Total image size**: 0x4F000 (323,584 bytes)

## Exports (1 function)
| Ordinal | Name | RVA | Convention |
|---------|------|-----|------------|
| 1 | `CreateMovieFactory` | 0xADD8 | `__stdcall` (COM factory pattern) |

This is the **only** export. The function creates an `IMovieFactory` COM interface. All other functionality is accessed through COM vtables.

## Import Table

### Direct Imports (13 DLLs)

**MSVCR110.dll** (35 functions) — CRT core:
- Memory: `malloc`, `calloc`, `free`, `_recalloc`, `_calloc_crt`, `_malloc_crt`
- String/memory: `memcpy`, `memcpy_s`, `memmove_s`, `memset`, `memcmp`, `wcslen`, `wcsnlen`, `wmemcpy_s`, `_wcsicmp`, `vswprintf_s`, `swscanf_s`, `_vscwprintf`
- Exception: `_CxxThrowException`, `__CxxFrameHandler3`, `__CppXcptFilter`, `_purecall`, `terminate`, `__clean_type_info_names_internal`
- CRT init: `_initterm`, `_initterm_e`, `_amsg_exit`, `_onexit`, `__dllonexit`, `_lock`, `_unlock`, `_except_handler4_common`, `_crt_debugger_hook`, `__crtTerminateProcess`, `__crtUnhandledException`

**KERNEL32.dll** (63 functions) — OS kernel:
- Thread pool (Win7+): `CreateThreadpool`, `CreateThreadpoolWork`, `SubmitThreadpoolWork`, `CloseThreadpoolWork`, `WaitForThreadpoolWorkCallbacks`, `CreateThreadpoolCleanupGroup`, `CloseThreadpoolCleanupGroup`, etc.
- Synchronization: `CreateEventW`, `SetEvent`, `ResetEvent`, `CreateSemaphoreW`, `ReleaseSemaphore`, `WaitForSingleObject`, `WaitForMultipleObjects`, `EnterCriticalSection`, `LeaveCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `DeleteCriticalSection`
- Module loading: `LoadLibraryExW`, `LoadLibraryExA`, `FreeLibrary`, `GetModuleHandleW`, `GetModuleHandleA`, `GetModuleFileNameW`, `SetDllDirectoryW`
- Process/thread: `GetCurrentThreadId`, `DisableThreadLibraryCalls`, `Sleep`, `IsDebuggerPresent`, `IsProcessorFeaturePresent`
- Other: `GetLastError`, `GetProcAddress`, `GetVersion`, `GetFileAttributesW`, `MultiByteToWideChar`, `WideCharToMultiByte`, `FormatMessageW`, `LocalFree`, `lstrlenW`, `GetSystemTimeAsFileTime`, `GetTickCount64`, `QueryPerformanceCounter`, `QueryPerformanceFrequency`, `EncodePointer`, `DecodePointer`, `RaiseException`
- Resources: `FindResourceW`, `FindResourceExW`, `LoadResource`, `LockResource`, `SizeofResource`, `FreeLibrary`
- Pointer protection: `EncodePointer`, `DecodePointer`, `InterlockedIncrement`, `InterlockedDecrement`, `InterlockedExchange`

**ADVAPI32.dll** (16 functions) — Security/registry:
- Crypto: `CryptAcquireContextW`, `CryptCreateHash`, `CryptHashData`, `CryptSignHashW`, `CryptDestroyHash`, `CryptDestroyKey`, `CryptImportKey`, `CryptReleaseContext`
- Registry: `RegOpenKeyExW`, `RegQueryValueExW`, `RegCloseKey`
- ETW tracing: `RegisterTraceGuidsW`, `UnregisterTraceGuids`, `GetTraceEnableFlags`, `GetTraceEnableLevel`, `GetTraceLoggerHandle`, `TraceMessage`, `TraceEvent`

**ole32.dll** (6 functions) — COM core:
- `CoTaskMemAlloc`, `CoTaskMemFree`, `CoCreateInstance`, `CLSIDFromString`, `StringFromCLSID`, `PropVariantClear`

**OLEAUT32.dll** (4 ordinals: 2, 4, 6, 9) — Automation:
- Likely `SysAllocString`, `SysFreeString`, `VariantClear`, `VariantCopy` or similar

**MFPlat.DLL** (13 functions) — Media Foundation platform:
- `MFStartup`, `MFShutdown`, `MFLockPlatform`, `MFUnlockPlatform`
- `MFCreateMediaType`, `MFCreateAttributes`, `MFCreateCollection`, `MFCreateSample`, `MFCreateMemoryBuffer`
- `MFCreateSourceResolver`, `MFGetPluginControl`
- `MFInvokeCallback`, `MFPutWorkItemEx`

**WINMM.dll** (11 functions) — Audio wave output:
- `waveOutOpen`, `waveOutClose`, `waveOutPrepareHeader`, `waveOutUnprepareHeader`, `waveOutWrite`, `waveOutReset`, `waveOutPause`, `waveOutRestart`, `waveOutGetPosition`
- `timeBeginPeriod`, `timeEndPeriod`

**USER32.dll** (3 functions):
- `SetRect`, `OffsetRect`, `GetDesktopWindow`

**d3d9.dll** (1 function):
- `Direct3DCreate9Ex`

**d3d11.dll** (1 function):
- `D3D11CreateDevice`

**dxva2.dll** (2 functions):
- `DXVA2CreateVideoService`, `DXVA2CreateDirect3DDeviceManager9`

**PROPSYS.dll** (1 function):
- `PSCreateMemoryPropertyStore`

**MF.dll** (1 function):
- `MFGetService`

### Delay-Load Imports (3 DLLs)

**SHLWAPI.dll** (8 functions):
- Path utilities: `PathAddBackslashW`, `PathRemoveBackslashW`, `PathAppendW`, `PathRemoveFileSpecW`, `PathFileExistsW`, `PathFindFileNameW`, `PathFindExtensionW`

**WLXPhotoBase.dll** (10 functions):
- `BasePrivate::New`, `BasePrivate::Delete` — memory management
- `Base::Throw`, `Base::ThrowLastError`, `Base::Exception::operator HRESULT`
- `Base::IsWin7OrGreater`, `Base::IsWin8OrGreater`
- `BaseAtlThrow` (ATL exception shim)
- `String::Base::GetBaseStringManager` — string manager
- `Base::Exception::~Exception`

**SHELL32.dll** (1 function):
- `SHGetPropertyStoreFromParsingName` — get property store from file path

## COM Architecture

### Interfaces (from RTTI)
| Interface | Description |
|-----------|-------------|
| `IMovieFactory` | Primary factory interface (created by export) |
| `IMovie` | Movie/media file object |
| `IMovieBuilder` | Builder for constructing movie objects |
| `IMovieCollection` | (likely) Collection of movies |
| `IAudioRenderer` | Audio rendering sink |
| `IAVClock` | AV clock interface |
| `IAVTickTimeSource` | Tick time source |
| `IAVStreamSink` | AV stream sink |
| `IMFSourceReaderCallback` | MF source reader callback |
| `IMFAsyncCallback` | MF async callback |
| `IServiceProvider` | Service provider |
| `IValidateBinding` | DRM/binding validation |

### Internal Classes (from RTTI)
**ATL Infrastructure:**
- `CAtlModule` -> `CAtlModuleT<CMovieDllModule>` -> `CAtlDllModuleT<CMovieDllModule>` (ATL COM DLL module)
- `CMovieDllModule` — custom module class
- `CAtlValidateModuleConfiguration` — module config validation
- `CComObjectRootEx<CComMultiThreadModel>` — thread-safe COM base
- `CComObject<V>` wrappers for: `MovieFactory`, `Movie`, `MovieBuilder`, `WaveAudioRenderer`, `SystemClock`, and all HMRAVSource classes

**Core Library Classes:**
- `MovieFactory` — Creates movie objects and manages library state
- `Movie` — Represents a single media file in the library
- `MovieBuilder` — Builder pattern for constructing movies
- `SystemClock` — System clock wrapper
- `ThreadPool` — Thread pool abstraction

**AV Source (HMRAVSource namespace):**
- `AVSourceFactory`, `AVSource`, `AVSourceProxy` — Media source management
- `AVSink` — Media sink
- `StreamSinkHelper`, `MFRateControlHelper` — Stream helpers
- `XVideoProc`, `DXVA2VideoProc` — Video processing (DXVA2 via d3d9/d3d11)
- `SyncVideoSampleSource` — Video sample synchronization
- `TextureInterOpDX9`, `TextureInterOpDX11` — DirectX interop for textures
- `MFByteStreamOnStream` — MF byte stream wrapper
- `AsyncSourceResolver`, `DShowMFSourceReaderBuilder`, `NativeMFSourceReaderBuilder` — Source readers
- `AuthProvider`, `AuthCredentials` — DRM/authentication

**Audio:**
- `WaveAudioRenderer`, `WaveDevice` — WaveOut audio rendering
- `AudioFormat`, `PcmFormat` — Audio format descriptors

**Transcode:**
- `TranscodeMetadataParser` — Metadata parsing for transcoding

**Base:**
- `RefCountBaseMultiThreaded` — Thread-safe ref counting (from WLXPhotoBase)

## Registry Paths
- `SOFTWARE\Microsoft\Windows Live\Common\Movie Library` — **Primary database/config key** (the library is registry-based, not SQL!)
- `SOFTWARE\Microsoft\Windows Live\Movie Maker` — Movie Maker settings

## COM GUIDs
No COM CLSID/IID GUIDs found stored as string literals in the binary. All COM interfaces are registered at runtime via ATL's object map and resolved via `CoCreateInstance` with CLSIDs from the registry (registered by `DllRegisterServer`). Two GUIDs found are code-signing certificate identifiers only:
- `2860b52e-c4a3-454d-bc1e-32c5add17e90` — Code signing subject identifier
- `4faf0b71-ad37-4aa3-a671-76bc052344ad` — Code signing subject identifier

## Key Strings

### Configuration/Paths
- `SOFTWARE\Microsoft\Windows Live\Common\Movie Library` — library registry key
- `SOFTWARE\Microsoft\Windows Live\Movie Maker` — app settings
- `Photo Gallery Movie Library` — library display name
- `Photo Gallery` — product name
- `ExclusionList.xml` — file exclusion list (loaded from library path)
- `WLAVRes.dll` — AV resource DLL reference

### Media Metadata (Windows Media namespaces)
- `WM/AlbumArtist`, `WM/AlbumTitle`, `WM/Author`, `WM/Title`, `WM/Year`, `WM/Genre`, `WM/Composer`, `WM/Director`, `WM/Producer`, `WM/Publisher`, `WM/Writer`, `WM/Language`
- `WM/Category`, `WM/Mood`, `WM/SubTitle`, `WM/SubTitleDescription`
- `WM/ParentalRating`, `WM/ParentalRatingReason`
- `WM/MediaClassPrimaryID`, `WM/MediaClassSecondaryID`
- `WM/WMCollectionGroupID`, `WM/WMCollectionID`, `WM/WMContentID`
- `WM/IsProtected`, `WM/SharedUserRating`, `WM/BeatsPerMinute`
- `WM/EncodingTime`, `WM/EncodingSettings`, `WM/EncodedBy`
- `WM/ToolName`, `WM/Provider`, `WM/ProviderRating`, `WM/ProviderStyle`
- `WM/ContentDistributor`, `WM/ContentGroupDescription`
- And ~50+ more WM/* metadata properties

### Media Foundation Format Identifiers
- `MFMediaType_Audio`, `MFMediaType_Video`, `MFMediaType_Image`, `MFMediaType_Binary`, `MFMediaType_HTML`, `MFMediaType_Script`, `MFMediaType_SAMI`, `MFMediaType_Protected`
- `MFAudioFormat_PCM`, `MFAudioFormat_Float`, `MFAudioFormat_MP3`, `MFAudioFormat_AAC`, `MFAudioFormat_MPEG`, `MFAudioFormat_DTS`, `MFAudioFormat_Dolby_AC3_SPDIF`
- `MFVideoFormat_NV12`, `MFVideoFormat_IYUV`, `MFVideoFormat_YUY2`, `MFVideoFormat_UYVY`, `MFVideoFormat_YV12`, `MFVideoFormat_YVYU`
- `MFVideoFormat_H264`, `MFVideoFormat_WMV1`, `MFVideoFormat_WMV2`, `MFVideoFormat_WMV3`, `MFVideoFormat_WVC1`, `MFVideoFormat_MSS1`, `MFVideoFormat_MSS2`
- `MFVideoFormat_MPG1`, `MFVideoFormat_MPG2`, `MFVideoFormat_MP4V`, `MFVideoFormat_MP4S`, `MFVideoFormat_M4S2`
- `MFVideoFormat_DV25`, `MFVideoFormat_DV50`, `MFVideoFormat_DVH1`, `MFVideoFormat_DVHD`, `MFVideoFormat_DVSD`, `MFVideoFormat_DVSL`
- `MFVideoFormat_ARGB32`, `MFVideoFormat_RGB32`, `MFVideoFormat_RGB24`, `MFVideoFormat_RGB555`, `MFVideoFormat_RGB565`, `MFVideoFormat_RGB8`
- `MFVideoFormat_P010`, `MFVideoFormat_P016`, `MFVideoFormat_P210`, `MFVideoFormat_P216`
- `MFVideoFormat_v210`, `MFVideoFormat_v216`, `MFVideoFormat_v410`, `MFVideoFormat_Y416`

### MF Attributes
- ~80+ `MF_MT_*` attributes (frame rate, frame size, pixel aspect, interlace mode, audio format descriptors, etc.)
- ~100+ `MF_E_*` error codes

### DXVA Modes
- `DXVA_ModeMPEG2_A`, `DXVA_ModeMPEG2_B`, `DXVA_ModeMPEG2_C`, `DXVA_ModeMPEG2_D`

### File Extension
- `.wtv` — Windows Recorded TV format (supported as a source)

## Library Architecture Notes
1. **No SQL database**: The movie library does not use SQLite, ESENT/Jet, or any SQL. It is managed through the registry key `SOFTWARE\Microsoft\Windows Live\Common\Movie Library`, file system metadata (via `SHGetPropertyStoreFromParsingName`), and `ExclusionList.xml` for exclusions.
2. **ATL COM DLL pattern**: This is a standard ATL COM server with `DllMain`, `DllGetClassObject`, `DllCanUnloadNow`, `DllRegisterServer`, `DllUnregisterServer` (all auto-generated by ATL).
3. **Thread pool usage**: Uses the Windows Vista+ thread pool API (`CreateThreadpool`, etc.) for asynchronous operations.
4. **Crypto usage**: `CryptAcquireContextW`, `CryptSignHashW` — likely used for DRM/content protection validation.
5. **ETW tracing**: Full Event Tracing for Windows instrumentation (`RegisterTraceGuidsW`, etc.).
6. **DirectX VA**: Hardware-accelerated video processing via DXVA2 with both Direct3D 9 and Direct3D 11 paths.
7. **Audio playback**: WaveOut API for audio rendering.
8. **Media Foundation core**: Uses MF for media type handling, source resolution, and sample management.
