# WLMFReadWrite.dll Dynamic Analysis

## Test Harness

- **Source**: `tests/WLMFReadWrite/test_mfreadwrite.cpp`
- **Build**: x86 MSVC /EHsc /MDd, VC++ 2022 BuildTools 14.44
- **Execution**: LoadLibrary + GetProcAddress for all 7 exports, SEH-wrapped calls with NULL/empty parameters

## Export Verification (7/7 resolved)

| # | Export | RVA | Status |
|---|--------|-----|--------|
| 1 | `DllCanUnloadNow` | 0x85CA | OK |
| 2 | `DllGetClassObject` | 0x85DE | OK |
| 3 | `MFCreateSinkWriterFromMediaSink` | 0x82D7 | OK |
| 4 | `MFCreateSinkWriterFromURL` | 0x81A9 | OK |
| 5 | `MFCreateSourceReaderFromByteStream` | 0x8183 | OK |
| 6 | `MFCreateSourceReaderFromMediaSource` | 0x8183 | OK |
| 7 | `MFCreateSourceReaderFromURL` | 0x815D | OK |

## Dynamic Call Results

### DllCanUnloadNow
Both calls return `S_OK` (0x00000000) — DLL reports it can be unloaded. The implementation at 0x85CA is a simple 7-instruction function: checks a global refcount at `[10039CB0h]` and returns 1 (can unload) when zero, 0 (cannot) otherwise. No COM lifetime management observed during test.

### DllGetClassObject
Thunks to ATL helper at 0x8CFA. Creates `CMFReadWriteClassFactory` instances implementing `IMFReadWriteClassFactory` (extends `IClassFactory`). Uses `CComObjectCached<CComClassFactory>` for thread-safe cached instantiation.

### MFCreateSourceReaderFromURL
**Signature**: `(LPCWSTR pwszURL, IMFAttributes* pAttributes, IMFSourceReader** ppSourceReader)`

| Call | Result | Notes |
|------|--------|-------|
| `(NULL, NULL, &out)` | `E_INVALIDARG` (0x80070057) | URL is mandatory |
| `(L"", NULL, NULL)` | `E_POINTER` (0x80004003) | Output pointer mandatory |

**Assembly** (RVA 0x815D): Prologue, pushes `[ebp+10h]` (ppSourceReader), loads `[ebp+8]` (URL) into edx, pushes constant `0x10003374` (CLSID string for source reader class), pushes `[ebp+0Ch]` (attributes), loads `ecx = 0x10003294` (class table pointer), calls shared helper `CreateInternal@0x8045`. Uses a different internal path from the ByteStream/MediaSource variant — `0x8045` handles URL-based source resolution.

### MFCreateSourceReaderFromByteStream / MFCreateSourceReaderFromMediaSource
**Signature**: `(IMFByteStream* pByteStream/IMFMediaSource* pSource, IMFAttributes* pAttributes, IMFSourceReader** ppSourceReader)`

Both exports share **RVA 0x8183** — identical function body.

| Call | Result | Notes |
|------|--------|-------|
| `(NULL, NULL, &out)` | `E_INVALIDARG` (0x80070057) | Source stream mandatory |

**Assembly** (RVA 0x8183): Same parameter marshaling as FromURL but calls `CreateInternal@0x80D1` instead of `0x8045`. The shared helper at `0x80D1`:
1. Validates `edx` (source) is non-NULL → `E_INVALIDARG` on failure
2. Validates `edi` (output) is non-NULL → `E_POINTER` on failure
3. Calls `CoCreateInstance` with CLSID at `0x100031E4` / IID at `0x100031F4`
4. Calls vtable `[ecx+10h]` on the created object (5th function in vtable — likely `Initialize`)
5. Releases on failure via vtable `[ecx+8]` (Release)

### MFCreateSinkWriterFromURL
**Signature**: `(LPCWSTR pwszURL, IMFByteStream* pByteStream, IMFAttributes* pAttributes, IMFSinkWriter** ppSinkWriter)`

| Call | Result | Notes |
|------|--------|-------|
| `(NULL, NULL, NULL, &out)` | `E_INVALIDARG` (0x80070057) | URL mandatory |
| `(L"", NULL, NULL, NULL)` | `E_POINTER` (0x80004003) | Output pointer mandatory |

**Assembly** (RVA 0x81A9): Most complex export — 128 bytes of prologue, SECURE_COOKIE (`xor eax,ebp; mov [ebp-4],eax`), 0x20 bytes of locals. Logic:
1. Validates URL non-NULL → `E_INVALIDARG` at 0x828D
2. If attributes provided, calls `IMFAttributes::GetItem` (vtable `[eax+28h]`) with GUID at `0x10003344`
3. If GetItem succeeds, stores the source reader via `SetPtr@0x82FD` (smart pointer assignment with AddRef/Release)
4. Calls `MFCreateSourceResolver` (IAT entry `[1000115Ch]`) with `1` (source resolver flags)
5. Calls vtable `[eax+80h]` (128 bytes into vtable — likely `CreateSourceFromURL` or similar)
6. Calls `CMFSinkWriter::Initialize@0x17EC8` passing the resolved source
7. Calls vtable `[ecx+60h]` on attributes with GUID `0x10003344`
8. Falls through to `CreateInternal@0x80D1` which creates the sink writer COM object with profile manager

### MFCreateSinkWriterFromMediaSink
**Signature**: `(IMFMediaSink* pMediaSink, IMFAttributes* pAttributes, IMFSinkWriter** ppSinkWriter)`

| Call | Result | Notes |
|------|--------|-------|
| `(NULL, NULL, &out)` | `E_INVALIDARG` (0x80070057) | Media sink mandatory |

**Assembly** (RVA 0x82D7): Thin wrapper — 14 instructions. Pushes `[ebp+10h]` (ppSinkWriter), `[ebp+8]` (media sink) into edx, constant `0x10003364` (different CLSID string from source reader), `[ebp+0Ch]` (attributes), loads `ecx = 0x10003284` (sink writer class table), calls `CreateInternal@0x80D1`. `ret 0Ch` confirms 3 parameters (stdcall).

## Custom MFCreate* Shadowing Pattern

All 5 MFCreate* functions in this DLL are **not** the standard Windows MF APIs from `mfreadwrite.dll`. They are custom wrappers that:

1. **Validate parameters** with standard HRESULT error codes (`E_INVALIDARG`, `E_POINTER`)
2. **Delegate to ATL COM class factory** infrastructure (`CMFReadWriteClassFactory`)
3. **Call through shared internal helpers** (`CreateInternal@0x80D1` / `CreateInternal@0x8045`)
4. **Use `CoCreateInstance`** internally to instantiate `CMFSourceReader` / `CMFSinkWriter` COM objects
5. **Add WL-specific logic**: profile managers, async work queue integration, CxCode video processing

The shadowing means applications linking against this DLL get WLMFReadWrite's implementations instead of (or in addition to) the system `mfreadwrite.dll` APIs.

## Profile Manager Architecture

Profile managers are instantiated during `MFCreateSinkWriterFromURL`/`FromMediaSink` flows:

| Profile Manager | Class | Format |
|----------------|-------|--------|
| Generic | `CMFSinkWriterGenericProfileManager` | Fallback/default |
| MP3 | `CMFSinkWriterMP3ProfileManager` | MPEG Audio Layer III |
| 3GP | `CMFSinkWriter3GPProfileManager` | 3GPP Multimedia |
| MPEG4 | `CMFSinkWriterMPEG4ProfileManager` | MPEG-4 Part 14 |
| ASF | `CMFSinkWriterASFProfileManager` | Advanced Systems Format |

All inherit from `CMFSinkWriterProfileManager` (base) implementing `IMFSinkWriterProfileManager`. The ASF variant additionally uses `CPayloadExtensions` with `CTDynArray` for ASF payload extension metadata.

Format-specific profile names found in `.data` section:
- `MFCreate3GPMediaSink`
- `MFCreateMP3MediaSink`
- `MFCreateMPEG4MediaSink`

## Custom MFT: CxCodeVideoProcMFT

RTTI class hierarchy:
```
CMFTBase (IMFTransform)
  +-- CxCodeVideoProcMFT (IMFRealTimeClient)
        +-- CxCodeVideoProcMFTTypeHandler (CMFTTypeHandler)
        +-- CxCodeVideoProcMFTDataHandler (CMFTDataHandler)
```

Supporting classes:
- `CxCodeVideoProcThread` — dedicated work thread with CPU affinity mask support
- Thread callbacks: `OnWorkItemAsyncCallback`, `OnRegisterAsyncCallback`, `OnUnregisterAsyncCallback`, `OnSetAffinityMaskAsyncCallback`

The MFT supports these video format conversions (from string table in .data):
```
NV12, YV12, IYUV, I420, YUY2, UYVY, YVYU, NV11, AYUV
```

Format conversion string pairs (input→output mappings):
- `NV12→YUY2`, `NV12→IYUV`, `NV11→IYUV`, `AYUV→IYUV`
- `UYVY→IYUV`, `YUY2→IYUV`, `IYUV→NV12`, `AYUV→IYUV`

Error/debug strings: `MFT->ProcessOutput`, `MFT->ProcessInput`, `<NULL>`

## Async Work Queue Integration

MF work queue APIs imported from `MFPlat.DLL`:
- `MFAllocateWorkQueue` — creates dedicated work queues
- `MFPutWorkItem` — dispatches async callbacks
- `MFUnlockWorkQueue` — releases work queue
- `MFBeginRegisterWorkQueueWithMMCSS` / `MFEndRegisterWorkQueueWithMMCSS` — MMCSS QoS registration
- `MFBeginUnregisterWorkQueueWithMMCSS` / `MFEndUnregisterWorkQueueWithMMCSS` — MMCSS deregistration

Usage pattern:
1. Source reader streams allocate work queues per-stream
2. `OnReadSampleAsyncCallback`, `AsyncFlushAsyncCallback`, `GetWorkQueueThreadIdAsyncCallback` dispatch via work items
3. Sink writer transform thread (`TransformThreadProcAsyncCallback`) runs encoding on dedicated queue
4. CxCodeVideoProcThread registers with MMCSS for real-time scheduling

## COM/ATL Infrastructure

The DLL is an ATL in-process COM server:
- `CMFReadWriteClassFactory` → `CComObjectCached<CComClassFactory>` → `IClassFactory`
- `CMFModuleLock` manages DLL lifetime via `CComModule`
- `CLSID_MFReadWriteClassFactory` is the COM class ID
- `DllGetClassObject` thunk → `CComModule::GetClassObject` at 0x8CFA

## Error Code Summary

| HRESULT | Meaning | When Returned |
|---------|---------|---------------|
| `S_OK` (0x00000000) | Success | DllCanUnloadNow when refcount=0 |
| `E_INVALIDARG` (0x80070057) | Invalid argument | NULL required parameter |
| `E_POINTER` (0x80004003) | Null pointer | NULL output parameter |
| `E_OUTOFMEMORY` (0x8007000E) | Out of memory | Memory allocation failure (seen in code paths) |
| `E_FAIL` (0x80004005) | General failure | Internal operation failure |

## RTTI Class Count

**98 unique RTTI type descriptors** found in the binary, comprising:
- 26 COM interfaces (`IUnknown`-derived)
- 16 template instantiations (`CTPtrArray`, `CTSparseBlock`, `CTDynArray`, `MFDynamicArray`)
- 56 concrete C++ classes
- 16 ATL infrastructure classes
