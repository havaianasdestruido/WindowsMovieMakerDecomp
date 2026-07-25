# WLMFReadWrite.dll Analysis

## Overview
Media Foundation Read/Write DLL from Windows Live Essentials 2012 (Photo Gallery / Movie Maker). Implements Source Reader and Sink Writer wrappers — the MF-level abstraction for reading from and writing to media files and byte streams. PE32 x86, linker v11.0 (MSVC 2012), image base 0x10000000, ASLR+NX.

| Field | Value |
|-------|-------|
| **File** | WLMFReadWrite.dll |
| **Description** | WLX Media Foundation ReadWrite |
| **Image** | 252 KB (0x3F000) |
| **Build** | `16.4.3528.0331_ship.client.main.w5m4 (ship)` |
| **PDB** | `WLMFReadWrite.pdb` GUID `{54E10C67-67A5-4F8A-8567-1D47C28B2189}` age 1 |
| **Timestamp** | PE: 2014-04-01 01:28:04, Export: 2014-04-01 01:16:51 |
| **Source path** | (not in PDB) |
| **Company** | Microsoft Corporation |
| **Product** | Photo Gallery |
| **InternalName** | wlmfreadwrite |

## PE Headers

### File Header
- **Machine**: x86 (0x14C)
- **Number of sections**: 4
- **Characteristics**: Executable, 32-bit, DLL

### Optional Header
| Field | Value |
|-------|-------|
| Magic | PE32 (0x10B) |
| Linker | 11.00 |
| Entry Point | 0x1003542B |
| Image Base | 0x10000000 |
| Image Size | 0x3F000 (252 KB) |
| Section Alignment | 0x1000 |
| File Alignment | 0x200 |
| OS Version | 6.02 (Win8) |
| Subsystem Ver | 6.00 |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | Dynamic Base (ASLR), NX Compatible |
| Stack Reserve/Commit | 0x40000 / 0x1000 |
| Heap Reserve/Commit | 0x100000 / 0x1000 |

### Directories
| Directory | RVA | Size |
|-----------|-----|------|
| Export | 0x37280 | 0x13F |
| Import | 0x363DC | 0xDC |
| Resource | 0x3A000 | 0x400 |
| Certificates | 0x3C000 | 0x3EC8 |
| Base Relocations | 0x3B000 | 0x2B84 |
| Debug | 0x12B0 | 0x38 |
| Load Config | 0x5AC0 | 0x40 |
| IAT | 0x1000 | 0x274 |

### Section Layout
| Section | VA | VSize | RawSize | Flags |
|---------|------|--------|---------|-------|
| `.text` | 0x1000 | 0x363BF (217 KB) | 0x36400 | Code, Execute+Read |
| `.data` | 0x38000 | 0x1D6C (7.5 KB) | 0x1800 | Init Data, Read+Write |
| `.rsrc` | 0x3A000 | 0x400 (1 KB) | 0x400 | Init Data, Read Only |
| `.reloc` | 0x3B000 | 0x3B2A (14.8 KB) | 0x3C00 | Init Data, Discardable, Read Only |

### Debug Info
- PDB: `WLMFReadWrite.pdb`, GUID `{54E10C67-67A5-4F8A-8567-1D47C28B2189}`
- /GS buffer security: 59 functions protected
- Pre-VC++ 11.00 count: 13, C/C++ count: 59

## Export Table (7 functions)

| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | 0x000085CA | `DllCanUnloadNow` |
| 2 | 1 | 0x000085DE | `DllGetClassObject` |
| 3 | 2 | 0x000082D7 | `MFCreateSinkWriterFromMediaSink` |
| 4 | 3 | 0x000081A9 | `MFCreateSinkWriterFromURL` |
| 5 | 4 | 0x00008183 | `MFCreateSourceReaderFromByteStream` |
| 6 | 5 | 0x00008183 | `MFCreateSourceReaderFromMediaSource` |
| 7 | 6 | 0x0000815D | `MFCreateSourceReaderFromURL` |

**Notes:**
- Functions 5 and 6 share the same RVA (0x8183) — `MFCreateSourceReaderFromMediaSource` is an alias for `MFCreateSourceReaderFromByteStream`. They are likely the same function with different parameter handling internally.
- These are **not** standard COM DLL exports — this DLL ships custom `MFCreate*` API functions instead of the standard `DllRegisterServer`/`DllUnregisterServer`.
- The `MFCreate*` functions shadow the standard Media Foundation API functions of the same names, wrapping them with additional Windows Live-specific logic (telemetry, custom source/sink resolution, etc.).

## Import Table

### MSVCR110.dll (VC++ 2012 CRT) — 36 imports
Memory: `memmove`, `memcpy`, `memcmp`, `memset`, `memcpy_s`, `malloc`, `free`, `_calloc_crt`, `_malloc_crt`, `_recalloc`
String: `_wcsicmp`, `wcsrchr`, `_ultoa`, `_gcvt_s`, `_vsnprintf`, `_vsnwprintf`, `wcsstr`, `wcsncpy_s`
Exception: `?terminate@@YAXXZ`, `_except_handler4_common`, `__CppXcptFilter`, `__crtTerminateProcess`, `__crtUnhandledException`, `_crt_debugger_hook`
Init/Term: `_initterm`, `_initterm_e`, `_onexit`, `__dllonexit`, `_amsg_exit`, `__clean_type_info_names_internal`, `_purecall`
Threading: `_lock`, `_unlock`
RTTI: `??1type_info@@UAE@XZ` (type_info destructor)

### KERNEL32.dll — 47 imports
**Module/Loading:** `GetModuleHandleExW/A`, `GetModuleHandleW`, `GetModuleFileNameW`, `LoadLibraryExW`, `LoadLibraryW`, `FreeLibrary`, `GetProcAddress`, `DisableThreadLibraryCalls`, `FindResourceW`
**Process/Thread:** `GetCurrentProcess`, `GetCurrentThread`, `GetCurrentThreadId`, `DuplicateHandle`, `GetLogicalProcessorInformation`, `SetThreadAffinityMask`, `WaitForMultipleObjects(Ex)`, `WaitForSingleObject(Ex)`, `CreateSemaphoreW`, `CreateEventW`, `SetEvent`, `ReleaseSemaphore`, `Sleep`
**Memory/Heap:** `HeapAlloc`, `HeapFree`, `HeapSize`, `GetProcessHeap`
**Synchronization:** `InitializeCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `EnterCriticalSection`, `LeaveCriticalSection`, `DeleteCriticalSection`, `InterlockedExchange/Increment/Decrement`
**Error/Debug:** `RaiseException`, `SetLastError`, `GetLastError`, `OutputDebugStringA`, `IsDebuggerPresent`
**System:** `GetSystemInfo`, `GetSystemTimeAsFileTime`, `GetTickCount64`, `GetVersion`, `IsProcessorFeaturePresent`, `QueryPerformanceCounter`
**String:** `lstrcmpiW`
**Encoding:** `MultiByteToWideChar`, `WideCharToMultiByte`
**Pointer:** `DecodePointer`, `EncodePointer`
**Other:** `SizeofResource`, `LoadResource`, `GetFileAttributesW`

### USER32.dll — 1 import
- `CharNextW` (character iteration)

### ADVAPI32.dll — 16 imports
**Registry (9):** `RegCreateKeyExW`, `RegOpenKeyExW`, `RegCloseKey`, `RegSetValueExW`, `RegQueryValueExW`, `RegQueryInfoKeyW`, `RegEnumKeyExW`, `RegDeleteKeyW`, `RegDeleteValueW`
**ETW Tracing (5):** `RegisterTraceGuidsW`, `UnregisterTraceGuids`, `GetTraceEnableFlags/Level/LoggerHandle`, `TraceMessage`

### ole32.dll — 7 imports
- `CoTaskMemAlloc`, `CoTaskMemFree`, `CoTaskMemRealloc` (COM memory)
- `CoCreateInstance` (COM instantiation)
- `StringFromGUID2` (GUID formatting)
- `PropVariantClear`, `PropVariantCopy` (PROPVARIANT management)

### OLEAUT32.dll — 3 ordinal imports
- Ordinal 9 (`VariantClear`), ordinal 8 (`VariantInit`), ordinal 277 (`SysFreeString`)

### MFPlat.DLL — 19 imports
**Work Queues:** `MFAllocateWorkQueue`, `MFPutWorkItem`, `MFUnlockWorkQueue`, `MFBeginRegisterWorkQueueWithMMCSS`, `MFBeginUnregisterWorkQueueWithMMCSS`, `MFEndRegisterWorkQueueWithMMCSS`, `MFEndUnregisterWorkQueueWithMMCSS`
**Media Types/Buffers:** `MFCreateMediaType`, `MFCreateMemoryBuffer`, `MFCreateSample`, `MFCreateAttributes`, `MFCreateMediaEvent`
**Other:** `MFTEnum` (enumerate MFTs), `MFHeapAlloc`, `MFHeapFree`, `MFCreateFile`, `MFCreateSystemTimeSource`, `MFGetSystemTime`, `MFCalculateImageSize`

### MF.dll — 5 imports
- `MFCreatePresentationClock` (clock for timing)
- `MFCreateASFProfile` (ASF profile creation)
- `MFCreateSourceResolver` (source URL/byte stream resolver)
- `MFCreateASFMediaSink` (ASF file sink)
- `MFGetService` (service interface query)

### EVR.dll — 3 imports
- `MFGetStrideForBitmapInfoHeader` (stride calculation)
- `MFCreateVideoSampleAllocator` (video sample pool)
- `MFCopyImage` (image copy with stride)

### PROPSYS.dll — 2 imports
- `PSCreateMemoryPropertyStore` (property store creation)
- `PropVariantToVariant` (PROPVARIANT → VARIANT conversion)

## COM / RTTI Classes

### Class Factory
- `CMFReadWriteClassFactory` — custom ATL COM class factory wrapped via `CComObject<CMFReadWriteClassFactory>` and `CComObjectCached<CComClassFactory@ATL>`
- Interface: `IMFReadWriteClassFactory` (extends `IClassFactory`)

### ATL Infrastructure
- `CComModule@ATL`, `CAtlModuleT<CComModule@ATL>@ATL`, `CAtlModule@ATL`
- `_ATL_MODULE70@ATL`
- `CRegObject@ATL` (registry object)
- `CComObjectRootEx<CComMultiThreadModelNoCS@ATL>@ATL` (non-threaded), `CComObjectRootEx<CComMultiThreadModel@ATL>@ATL` (multi-threaded)

### Source Reader Subsystem (CMFSourceReader)
- `CMFSourceReader` — main reader class
- `CMFSourceReaderManager@CMFSourceReader` — manages reader state/operations
- `CMFSourceReaderStream` — per-stream reader
- `CMFSourceReaderEventQueue` — event queue for async operations
- `CMFSourceReaderAppCallback` — application callback (with `OnEvent`, `OnFlush`, `OnReadSample` subclasses)
- Async state: `AsyncState`, `AsyncState_ReadSample`, `AsyncState_SetCurrentPosition`
- Stream callbacks: `AsyncFlushAsyncCallback`, `OnReadSampleAsyncCallback`, `GetWorkQueueThreadIdAsyncCallback`
- Interfaces: `IMFSourceReader`, `IMFSourceReaderManager`, `IMFSourceReaderStream`, `IMFSourceReaderEventQueue`, `IMFSourceReaderErrorSink`, `IMFAsyncCallback`

### Sink Writer Subsystem (CMFSinkWriter)
- `CMFSinkWriter` — main writer class
- `CMFSinkWriterManager@CMFSinkWriter` — manages writer operations
- `CMFSinkWriterStream` — per-stream writer
- `CMFSinkWriterAppCallback` — application callback (with `OnFinalize`, `OnMarker` subclasses)
- `CMFSinkWriterMarkerContext` — marker tracking
- Async callbacks: `AsyncFinalizeAsyncCallback`, `InvokeAppCallbackAsyncCallback`, `OnFinalizeAsyncCallback`, `OnMediaSinkEventAsyncCallback`, `OnStreamSinkEventAsyncCallback`, `TransformThreadProcAsyncCallback`
- Interfaces: `IMFSinkWriter`, `IMFSinkWriterManager`, `IMFSinkWriterStream`

### Profile Management
- `CMFSinkWriterProfileManager` (base)
- `CMFSinkWriterGenericProfileManager`, `CMFSinkWriterMP3ProfileManager`, `CMFSinkWriter3GPProfileManager`, `CMFSinkWriterMPEG4ProfileManager`, `CMFSinkWriterASFProfileManager`
- Interface: `IMFSinkWriterProfileManager`
- ASF profile uses `CPayloadExtensions@CMFSinkWriterASFProfileManager` with `CTDynArray<...>`

### Media Foundation Transform (MFT) Pipeline
- `CMFReadWriteTransform` — transform execution
- `CMFReadWriteTransformChain` — transform chain
- `CMFReadWriteAsyncTransform` — async transform base
- `CMFReadWriteTransformActivate` — activation object (`CMFAttributesImpl<IMFActivate, CWin32AttributeLock>`)
- Interfaces: `IMFReadWriteTransform`, `IMFReadWriteTransformChain`, `IMFTransform`, `IMFActivate`

### Codec Video Processing (CxCode)
- `CxCodeVideoProcMFT` — MFT for video processing
- `CxCodeVideoProcMFTTypeHandler` — media type negotiation
- `CxCodeVideoProcMFTDataHandler` — data handling
- `CxCodeVideoProcThread` — work thread with affinity mask support
- Thread callbacks: `OnWorkItemAsyncCallback`, `OnRegisterAsyncCallback`, `OnUnregisterAsyncCallback`, `OnSetAffinityMaskAsyncCallback`
- Interface: `IMFRealTimeClient`

### MFT Support Infrastructure
- `CMFTBase` — base MFT class
- `CMFTTypeHandler`, `CMFTDataHandler` — type/data management
- `CMFTSimpleTypeHandler`, `CMFTSimpleDataHandler` — simple implementations

### Media Foundation Types
- `CMFRatelessTimeSource` — rateless time source (interfaces: `IMFRatelessTimeSource`, `IMFPresentationTimeSource`, `IMFClock`, `IMFClockStateSink`)
- `CMFMediaBufferAllocator` — pooled buffer allocator
- `CMFMemoryBuffer` — memory-backed media buffer (interface: `IMFMediaBuffer`)
- `CPolledBuffer@CMFMediaBufferAllocator` — tracked buffer with allocation state

### Template Collections
- `CTPtrArray<T>` — pointer array (used for `IMFSourceReaderStream`, `IMFMediaEvent`, `IMFSinkWriterStream`, `COutputStreamInfo@CMFReadWriteTransform`)
- `CTSparseBlock<T>` — sparse block array (used for same types)
- `CTDynArray<T>` — dynamic array (used for `CPayloadExtensions`, `UTransformState`)
- `MFDynamicArray<K>` — generic dynamic array

## Strings & Registry

### Version Info
- **FileDescription**: WLX Media Foundation ReadWrite
- **FileVersion**: 16.4.3528.0331_ship.client.main.w5m4 (ship)
- **InternalName**: wlmfreadwrite
- **OriginalFilename**: wlmfreadwrite.dll
- **ProductName**: Photo Gallery
- **LegalCopyright**: (c) 2012 Microsoft Corporation. All rights reserved.
- **Language**: 0x0409 (English - US)

### RTTI Type Info
The DLL contains RTTI for ~70+ C++ classes. Key hierarchy reveals:
- ATL template chain: `CComObjectRootEx` → `CComCoClass` → specific class factories
- Source Reader hierarchy: `CBaseUnknown` → `CMFSourceReader*` classes
- Sink Writer hierarchy: `CBaseUnknown` → `CMFSinkWriter*` classes
- MFT pipeline: `CMFTBase` → `CxCodeVideoProcMFT`, `CMFReadWriteTransform`, etc.
- `type_info` for all RTTI classes found in `.data` section

### Registry
While no explicit registry path strings were found in high-value strings, ADVAPI32 registry imports suggest self-registration under `HKEY_CLASSES_ROOT\CLSID\{...}` via ATL `CRegObject`. The `DllGetClassObject` creates `CMFReadWriteClassFactory` instances, suggesting an in-process COM server pattern.

## Architecture Summary

### Design Pattern
WLMFReadWrite.dll is a **Media Foundation wrapper library** implementing the Source Reader and Sink Writer APIs with additional Windows Live-specific logic. It does NOT reimplement MF primitives — it delegates to `MFPlat.DLL` and `MF.dll` for platform functionality.

### Source Reader Flow
1. `MFCreateSourceReaderFromURL` / `FromByteStream` / `FromMediaSource`
2. Creates `CMFSourceReader` which uses `MFCreateSourceResolver` to resolve the URL/media source
3. Wraps the resolved MF source in `CMFSourceReaderStream` instances (per stream)
4. Async operations use MF work queues (`MFAllocateWorkQueue` + `MFPutWorkItem`)
5. Events queued via `CMFSourceReaderEventQueue` with `OnEvent/OnFlush/OnReadSample` callbacks

### Sink Writer Flow
1. `MFCreateSinkWriterFromURL` / `FromMediaSink`
2. Creates `CMFSinkWriter` which manages `CMFSinkWriterStream` per output stream
3. Profile managers negotiate output format (Generic/MP3/3GP/MPEG4/ASF)
4. For ASF: creates via `MFCreateASFMediaSink` + `MFCreateASFProfile`
5. Transform chain (`CMFReadWriteTransformChain`) processes media samples through MFTs
6. Video processing via `CxCodeVideoProcMFT` (single-pass MFT with thread affinity)

### Key Differences from Standard MF API
1. These `MFCreate*` functions are **not** the system MF APIs (which live in `mfreadwrite.dll` / `mfplat.dll`) — they are custom wrappers with additional Windows Live-specific functionality
2. Adds profile management (3GP, MPEG4, MP3, ASF profiles) — standard MF uses media type negotiation
3. Includes custom video processing MFT (`CxCodeVideoProcMFT`) — color conversion / scaling
4. All operations are async via MF work queues
5. ETW tracing via ADVAPI32 for diagnostics
6. ATL COM class factory pattern for object creation

### Dependencies
- **MFPlat.DLL**: Platform (work queues, media types, buffers, samples, attributes)
- **MF.dll**: Presentation clock, ASF profile/sink, source resolver, service queries
- **EVR.dll**: Video stride, sample allocator, image copy (video rendering support)
- **PROPSYS.dll**: Property stores (metadata, attributes)
- **ole32.dll**: COM memory, CoCreateInstance, GUID strings
- **ADVAPI32.dll**: Registry (self-registration) + ETW tracing
- **MSVCR110.dll**: CRT (memory, string, exceptions, RTTI)
- **KERNEL32.dll**: OS platform (process, thread, sync, modules, heap, system info)
