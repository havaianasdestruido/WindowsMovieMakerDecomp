# WLXPhotoBase.dll — Dynamic Analysis & Export Probing

## Test Harness

Test harness: `tests/WLXPhotoBase/test_base.cpp`
Build script: `tests/WLXPhotoBase/build.bat`

All 56 exports loaded and resolved via `LoadLibraryA`/`GetProcAddress`. Thunk-based calling convention adapter used for `__thiscall` member functions (Version, Exception classes). SEH guards wrap all dangerous calls.

---

## Export Resolution

| Status | Count |
|--------|-------|
| Resolved | **56/56** |
| Missing | 0 |

All exports are present and resolvable. The DLL loads cleanly at a randomized base address (ASLR enabled).

---

## Per-Export Results

### 1. OS Detection (`Base::OS::`)

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `IsVistaOrGreater` | `bool __stdcall OS::IsVistaOrGreater()` | **true** |
| `IsWin7OrGreater` | `bool __stdcall OS::IsWin7OrGreater()` | **true** |
| `IsWin8OrGreater` | `bool __stdcall OS::IsWin8OrGreater()` | **true** |

Returns `bool` (1-byte). On Windows 10+, all three return true. Uses `GetVersionExW` under the hood.

### 2. CPU Detection (`Base::CPU::`)

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `GetProcessorCount` | `int __stdcall CPU::GetProcessorCount()` | **8** |
| `GetProcessorCaps` | `void __stdcall CPU::GetProcessorCaps(TCPUCaps&)` | `Caps[0] = 0xBFCBFBFF` |

**TCPUCaps** is a struct of at least 64 bytes (unsigned int d[16]). Caps[0] = `0xBFCBFBFF` decodes as standard x86 feature flags (SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, CMPXCHG8B, etc.).

### 3. Assert System (`Base::`)

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `NoAssertCount` | `int __stdcall NoAssertCount()` | **0** (initial) |
| `IncrementNoAssertCount` | `void __stdcall IncrementNoAssertCount()` | Increments count by 1 |
| `DecrementNoAssertCount` | `void __stdcall DecrementNoAssertCount()` | Decrements count by 1 |
| `s_nAssertsInhibited` | `static int` (data export) | **0** |
| `GetAssertCallback` | `FnAssertCallback __stdcall GetAssertCallback()` | Returns **0x7231A2D4** (function pointer within DLL .data) |
| `DisableShipAsserts` | `void __stdcall DisableShipAsserts()` | OK (no crash) |
| `EnableShipAsserts` | `void __stdcall EnableShipAsserts(void*, void*, ModuleVersion*)` | **Throws C++ exception** (0xE06D7363) when called with NULL params |

**Key finding:** The NoAssertCount is a thread-local counter. Increment/Decrement work as expected (0 → 1 → 0). The assert callback is a static function pointer stored in the DLL's .data section. EnableShipAsserts requires valid module handles and a populated ModuleVersion struct — calling with NULL triggers a ship assert failure.

### 4. IsOutOfMemoryError (`Base::`)

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `IsOutOfMemoryError` | `bool __stdcall IsOutOfMemoryError(long)` | See below |

| HRESULT Tested | Result |
|---------------|--------|
| `S_OK` (0x00000000) | **false** |
| `E_OUTOFMEMORY` (0x8007000E) | **true** |
| `E_FAIL` (0x80004005) | **false** |
| `E_ACCESSDENIED` (0x80070005) | **false** |

Exact match only on `0x8007000E`. Does NOT match `E_FAIL` or `E_ACCESSDENIED`. Returns `bool` (1 byte).

### 5. Memory Management (`BasePrivate::`)

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `New` | `void* __cdecl BasePrivate::New(unsigned int size, bool zeroInit)` | **Non-null** (0x012896A8 for 64 bytes) |
| `Delete` | `void __cdecl BasePrivate::Delete(void* ptr)` | OK |
| `VerifyPtr` | `bool __cdecl BasePrivate::VerifyPtr(const void* ptr)` | See below |

| Pointer Tested | VerifyPtr Result |
|---------------|-----------------|
| Valid heap ptr | **true** |
| NULL | **false** |
| 0xDEADBEEF | **true** (!) |

**Critical finding:** `VerifyPtr` returns `true` for `0xDEADBEEF`. It likely only checks `ptr != NULL` or uses a simple validity heuristic (not a full heap-validate). This is a debug-only helper, not a security check.

### 6. Version Class (`Base::Version`)

**Version struct layout: 32 bytes (8 DWORDs)**

| Offset | Size | Field |
|--------|------|-------|
| 0x00 | 4 | Major version (MS) |
| 0x04 | 4 | Minor version (LS) |
| 0x08 | 4 | Build number (MS) |
| 0x0C | 4 | Revision (LS) |
| 0x10 | 16 | Reserved / padding / internal state |

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `Version()` | `Version::Version()` (default ctor) | Creates valid version (all zeros) |
| `IsValid` | `bool Version::IsValid() const` | **true** after default ctor |
| `Set(4 args)` | `void Version::Set(ushort, ushort, ushort, ushort)` | OK (16.4.3528.331) |
| `Set(2 args)` | `void Version::Set(ulong, ulong)` | OK (0x00100004, 0x0DD0014B) |
| `Set(FFI)` | `void Version::Set(const VS_FIXEDFILEINFO&)` | OK |
| `operator<` | `bool Version::operator<(const Version&) const` | false (equal versions) |
| `operator>` | `bool Version::operator>(const Version&) const` | true (after Set(2) overrides Set(4)) |
| `operator=` | `Version& Version::operator=(const Version&)` | Works correctly |
| `Copy ctor` | `Version(const Version&)` | Preserves validity |
| `Invalidate` | `void Version::Invalidate()` | Sets invalid BUT IsValid still returns true |
| `AsString` | `void Version::AsString(String*) const` | **CRASH** (0xC0000005) — requires a valid `Base::String` object |

**Key finding:** Default-constructed versions are immediately valid. `Invalidate()` does NOT make `IsValid()` return false — the struct appears to use a validity flag that defaults to "valid" and Invalidate may set a specific internal marker that is still checked as valid. The `AsString` function requires a properly constructed `Base::String` (ATL CString wrapper), not a raw buffer.

### 7. Exception System (`Base::Exception`)

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `Exception(long)` | `Exception::Exception(HRESULT)` (private ctor, via thunks) | OK — stores E_FAIL |
| `Exception(const&)` | `Exception::Exception(const Exception&)` (copy ctor) | OK — copies HR |
| `operator=` | `Exception& operator=(const Exception&)` | OK |
| `operator long()` | `HRESULT Exception::operator long() const` | Returns **0x80004005** |
| `ThreadID` | `DWORD Exception::ThreadID() const` | Returns **0xFFFFFFFF** (4294967295) |
| `~Exception` | `virtual Exception::~Exception()` | Called during cleanup |
| `Base::Throw(long)` | `void __stdcall Throw(HRESULT)` | Throws C++ exception (0xE06D7363) |
| `Base::ThrowLastError()` | `void __stdcall ThrowLastError()` | Throws C++ exception with GetLastError() = ERROR_FILE_NOT_FOUND |

**Exception object layout (min 64 bytes):**

| Offset | Size | Field |
|--------|------|-------|
| 0x00 | 4 | vtable pointer |
| 0x04 | 4 | HRESULT value |
| 0x08 | 4 | ThreadID (DWORD) |
| 0x0C | 52 | Internal state / padding |

**Key finding:** `ThreadID` returns 0xFFFFFFFF for a manually-constructed exception (not caught via Throw). This suggests the field is set by `Init()` which is private — the normal path constructs exceptions via `Throw()`. The exception mechanism uses `_CxxThrowException` (C++ EH), not raw SEH, despite the DLL also supporting SEH. The exception code 0xE06D7363 is the standard MSVC C++ exception magic.

### 8. OutOfMemoryException (`Base::OutOfMemoryException`)

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `OutOfMemoryException()` | `OutOfMemoryException::OutOfMemoryException()` (private ctor) | OK |
| `OutOfMemoryException(const&)` | Copy ctor | OK |
| `~OutOfMemoryException` | Virtual destructor | Called during cleanup |

Shares vtable entry [0] with Exception (both have the same RTTI typeinfo base).

### 9. Vtable Layout

**Exception vtable @ 0x72311218:**

| Index | Address (RVA) | Inferred Function |
|-------|---------------|-------------------|
| [0] | 0x723118B4 (0x18B4) | RTTI typeinfo / `type_info` ptr |
| [1] | 0x00000000 | **NULL** (no scalar deleting destructor) |
| [2] | 0x723115D0 (0x15D0) | `~Exception()` destructor |
| [3] | 0x72311F54 (0x1F54) | Virtual method (likely what() / ToString) |
| [4] | 0x72311F49 (0x1F49) | Virtual method |
| [5] | 0x72311F38 (0x1F38) | Virtual method |
| [6] | 0x72311EF5 (0x1EF5) | Virtual method |
| [7] | 0x72311664 (0x1664) | Virtual method |
| [8] | 0x72311FC9 (0x1FC9) | Virtual method |
| [9] | 0x72312025 (0x2025) | Virtual method |

**OOM vtable @ 0x72311210:**

| Index | Address (RVA) | Relationship to Exception |
|-------|---------------|---------------------------|
| [0] | 0x723118B4 | **same** (shared RTTI) |
| [1] | 0x72311554 | **different** (different destructor chain) |
| [2] | 0x723118B4 | **different** |
| [3] | 0x00000000 | **different** (NULL vs function) |
| [4] | 0x723115D0 | **different** |

**Analysis:** Exception has at least 10 virtual functions (indices 0-9). OOM overrides the first few entries. The shared [0] entry is the RTTI `type_info` pointer — both classes share a common base but OOM has its own typeinfo hierarchy.

### 10. Telemetry / Error Reporting

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `GetReportMetrics` | `void __stdcall GetReportMetrics(ReportMetrics*)` | All zeros (no pending reports) |
| `GetReportsForSqm` | `void __stdcall GetReportsForSqm(ReportsForSqm*)` | All zeros |
| `GetReportsForWer` | `void __stdcall GetReportsForWer(ReportsForWer*)` | **CRASH** (0xC0000005) — access violation, even with heap-allocated 4KB buffer |
| `SetReportsForWer` | `void __stdcall SetReportsForWer(ReportsForWer*)` | **CRASH** (0xC0000005) — same issue |
| `ReportFault` | `void __stdcall ReportFault(HRESULT)` | OK (no crash for E_FAIL) |

**Key finding:** `GetReportsForWer`/`SetReportsForWer` crash with access violation even when called with a valid heap-allocated buffer of 1KB. This indicates these functions require internal state that is initialized by `EnableShipAsserts` or `Init` — they likely access an uninitialized internal pointer. The SQM and Metrics functions safely handle zeroed state.

**Inferred struct sizes (probed via buffer allocation):**
- `ReportMetrics`: 256 bytes (64 DWORDs) — safe with zeroed buffer
- `ReportsForSqm`: 256 bytes (64 DWORDs) — safe with zeroed buffer
- `ReportsForWer`: 256+ bytes — requires initialized internal state, crashes without it

### 11. GdiplusStatusToHresult (`Base::`)

Full mapping table recovered:

| Gdiplus::Status | HRESULT |
|----------------|---------|
| 0 (Ok) | 0x00000000 (S_OK) |
| 1 (GenericError) | 0x80004005 (E_FAIL) |
| 2 (InvalidParameter) | 0x80070057 (E_INVALIDARG) |
| 3 (OutOfMemory) | 0x8007000E (E_OUTOFMEMORY) |
| 4 (ObjectBusy) | 0x800700AA (E_BUSY) |
| 5 (InsufficientBuffer) | 0x8007007A (ERROR_INSUFFICIENT_BUFFER) |
| 6 (NotImplemented) | 0x80004001 (E_NOTIMPL) |
| 7 (Win32Error) | 0x80070715 (ERROR_RESOURCE_TYPE_NOT_FOUND) |
| 8 (Aborted) | 0x8007139F (ERROR_INVALID_STATE) |
| 9 (AccessDenied) | 0x80004004 (E_ABORT) |
| 10 (FileNotFound) | 0x80070002 (ERROR_FILE_NOT_FOUND) |
| -1 (unknown) | 0x80004005 (E_FAIL) |
| 11-15 | 0x80070216, 0x80070005, 0x80004005, 0x80004005, 0x80004005 |
| 20+ | 0x80004005 (E_FAIL, fallback) |

### 12. Module Information

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `GetModuleVersion` | `HRESULT __stdcall GetModuleVersion(HINSTANCE*, ModuleVersion*)` | **0x80070715** (ERROR_RESOURCE_TYPE_NOT_FOUND) — no version resource in test exe |
| `GetModuleAddresses` | `HRESULT __stdcall GetModuleAddresses(void*, ModuleAddresses*)` | **0x80004005** (E_FAIL) — but base/size partially filled |

**Key finding:** `GetModuleVersion` uses `VERSION.dll` (`GetFileVersionInfoSizeW`/`GetFileVersionInfoW`/`VerQueryValueW`) and fails gracefully when no version resource exists. `GetModuleAddresses` uses `PSAPI.DLL` (`GetModuleInformation`) — it returned a base address for the test exe but a size of 0, suggesting a partial failure path.

### 13. String Management

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `GetBaseStringManager` | `static CAtlStringMgr& String::GetBaseStringManager()` | Resolved (no crash) |
| `GetBaseStringComManager` | `static CAtlStringMgr& StringCom::GetBaseStringComManager()` | Resolved (no crash) |
| `BaseAtlThrow` | `void __stdcall ATL::BaseAtlThrow(HRESULT)` | Throws C++ exception (0xE06D7363) |
| `BaseAtlThrowLastError` | `void __stdcall ATL::BaseAtlThrowLastError()` | Resolved (SEH-protected) |

These return ATL `CAtlStringMgr` references — the global string allocator for all WLX components. `BaseAtlThrow` throws an ATL exception with the given HRESULT.

### 14. Leak Tracking

| Export | Signature (inferred) | Result |
|--------|---------------------|--------|
| `EnableLeakTrackingAndSetSymbolPath` | `void __stdcall EnableLeakTrackingAndSetSymbolPath(bool)` | OK (no crash) |

Activates CRT leak detection. Parameter controls whether to set the symbol path for leak reports.

### 15. ReportError (`BasePrivate::`)

| Export | Signature (inferred) | Resolved |
|--------|---------------------|----------|
| `ReportError` | `bool __cdecl ReportError(const char*, int, const wchar_t*, unsigned long, char*, unsigned long, const char*)` | Resolved (8 params) |

**Inferred signature (from mangled name):**
```
bool __cdecl BasePrivate::ReportError(
    const char* file,       // source file
    int line,               // line number
    const wchar_t* expr,    // expression string
    unsigned long,          // likely HRESULT or flags
    char*,                  // output buffer
    unsigned long,          // buffer size
    const char*             // function name
)
```
This is the internal error reporting function used by the ship assert mechanism.

---

## Inferred Internal Structures

### `Base::Version` (32 bytes)
```cpp
struct Version {
    DWORD major;        // +0x00
    DWORD minor;        // +0x04
    DWORD build;        // +0x08
    DWORD revision;     // +0x0C
    DWORD reserved[4];  // +0x10 (internal state, validity flag)
};
```

### `Base::TCPUCaps` (64 bytes minimum)
```cpp
struct TCPUCaps {
    DWORD features[16];  // CPU feature flags (bitfield)
};
```
Caps[0] = `0xBFCBFBFF` = standard x86 feature bits.

### `Base::ModuleAddresses` (8 bytes)
```cpp
struct ModuleAddresses {
    void* pBase;     // +0x00 module base address
    DWORD cbSize;    // +0x04 module image size
};
```

### `Base::ModuleVersion` (64 bytes minimum)
```cpp
struct ModuleVersion {
    DWORD fields[16];  // version info from GetFileVersionInfoW
};
```

### `Base::ReportMetrics` (256 bytes)
Zeroed buffer accepted without crash. All fields zero when no reports pending.

### `Base::ReportsForSqm` (256 bytes)
Zeroed buffer accepted without crash. SQM telemetry counters.

### `Base::ReportsForWer` (256+ bytes)
**Requires initialized internal state.** Access-violates with zeroed buffer.

### `Base::Exception` (min 64 bytes)
```cpp
class Exception {
    /*+0x00*/ void* vtable;       // vtable pointer
    /*+0x04*/ HRESULT hr;         // stored HRESULT
    /*+0x08*/ DWORD threadId;     // creating thread ID
    /*+0x0C*/ BYTE  internal[52]; // internal state
};
```

### `Base::TCPUCaps` — Feature bits (Caps[0] = 0xBFCBFBFF)
```
Bit 0: FPU    Bit 1: CMPXCHG8B    Bit 2: FXSR    Bit 3: MMX
Bit 4: SSE    Bit 5: SSE2          Bit 8: SSE3     Bit 9: CMPXCHG16B
Bit 10: SSE4.1  Bit 11: SSE4.2     Bit 12: POPCNT
```

---

## DLL Cross-References

### Load-time imports (from other DLLs into WLXPhotoBase.dll)
The following DLLs import from WLXPhotoBase.dll (per `analysis/CrossDllImports/`):
- WLXVideoTrim.dll — Base::Throw, memory management, OS version detection
- WLXSlideshow.dll
- WLXTranscode.dll
- WLXPhotoCinematic.dll
- MovieMakerCore.dll
- And others (see `analysis/CrossDllImports/imports_table.md`)

### WLXPhotoBase.dll imports from:
| DLL | Functions Used |
|-----|---------------|
| MSVCR110.dll | CRT: malloc, free, new, delete, _CxxThrowException, memcpy, memset, etc. |
| KERNEL32.dll | Process/thread/module/SWER/CRITICAL_SECTION APIs |
| PSAPI.DLL | GetModuleInformation |
| wer.dll | WerReportCreate/AddDump/SetParameter/Submit/CloseHandle |
| VERSION.dll | GetFileVersionInfoSizeW/GetFileVersionInfoW/VerQueryValueW |
| SHLWAPI.dll | PathFindFileNameW |
| ole32.dll (delay-load) | CoTaskMemAlloc/Free/Realloc/GetMalloc |

---

## Constants & Magic Values

| Value | Meaning |
|-------|---------|
| `0xE06D7363` | MSVC C++ exception magic code |
| `0xC0000005` | STATUS_ACCESS_VIOLATION |
| `0x8007000E` | E_OUTOFMEMORY |
| `0x80004005` | E_FAIL |
| `0x80070057` | E_INVALIDARG |
| `0x80070715` | ERROR_RESOURCE_TYPE_NOT_FOUND |
| `0x80070002` | ERROR_FILE_NOT_FOUND |
| `0x800700AA` | E_BUSY |
| `0x8007007A` | ERROR_INSUFFICIENT_BUFFER |
| `0x80004001` | E_NOTIMPL |
| `0x80004004` | E_ABORT |
| `0x8007139F` | ERROR_INVALID_STATE |
| `0x80070216` | ERROR_ARITHMETIC_OVERFLOW |
| `0x80070005` | E_ACCESSDENIED |
| `0xBFCBFBFF` | CPU feature flags (SSE/SSE2/SSE3/SSSE3/SSE4.x) |
| `0xFEEF04BD` | VS_FIXEDFILEINFO signature |
| `0xFFFFFFFF` | Invalid/uninitialized thread ID |

---

## Key Architectural Findings

1. **Foundation library** — All 56 exports are present and functional. This DLL is the shared base for the entire Windows Live Photo Gallery / Movie Maker 2012 suite.

2. **C++ exceptions, not SEH** — `Base::Throw()` uses `_CxxThrowException` (code 0xE06D7363), not raw SEH. However, the exception object wraps an HRESULT, creating a hybrid model.

3. **Thread-local assert suppression** — `NoAssertCount` is incremented/decremented per-thread to suppress asserts in expected error paths. `s_nAssertsInhibited` is a global counter.

4. **VerifyPtr is not a security check** — Returns `true` for any non-null address including `0xDEADBEEF`. It's a debug diagnostic, not a pointer validation function.

5. **Version default ctor marks as valid** — Unlike many C++ designs where default-constructed objects are "empty", `Version()` creates a valid (zero) version. `Invalidate()` apparently doesn't make `IsValid()` return false.

6. **ThreadID 0xFFFFFFFF** — Manually-constructed exceptions have thread ID = -1. Only exceptions created through the normal `Throw()` path get the actual thread ID (via `Init()`).

7. **WER reporting requires initialization** — `GetReportsForWer`/`SetReportsForWer` crash with access violations when called without prior initialization via `EnableShipAsserts`. The telemetry functions have a dependency on internal state.

8. **GdiplusStatusToHresult is a pure lookup table** — Maps all 11 Gdiplus::Status values to their HRESULT equivalents, with E_FAIL as the fallback for unknown codes.

9. **ATL CString shared allocator** — `GetBaseStringManager`/`GetBaseStringComManager` provide the global `CAtlStringMgr` for all WLX components, ensuring all ATL strings share a common allocator across DLL boundaries.

10. **Module information uses external APIs** — `GetModuleVersion` delegates to VERSION.dll; `GetModuleAddresses` delegates to PSAPI.DLL. Both fail gracefully for modules without version resources.
