# WLXPhotoBase.dll - Static Analysis

## PE Characteristics

| Property | Value |
|----------|-------|
| **File Type** | DLL (Dynamic Link Library) |
| **Machine** | x86 (0x14C) - 32-bit |
| **Linker Version** | 11.00 (MSVC 11.0 / VS2012) |
| **Image Base** | 0x10000000 |
| **Subsystem** | Windows GUI (2) |
| **DLL Characteristics** | Dynamic base, NX compatible |
| **Target OS** | Windows 6.2+ (Win8+) |
| **Entry Point** | 0x10003CFF |
| **Size of Image** | 0xE000 (56 KB) |
| **Stack Reserve** | 0x40000 (256 KB) |
| **Heap Reserve** | 0x100000 (1 MB) |
| **Timestamp** | Tue Apr 1 01:28:14 2014 (0x533A405E) |
| **PDB GUID** | {0674DC61-4F42-4D44-AD50-155EB0251FA5} |
| **PDB File** | WLXPhotoBase.pdb |

### Version Information (Resource Section)

| Property | Value |
|----------|-------|
| CompanyName | Microsoft Corporation |
| FileDescription | Photo Base Library |
| FileVersion | 16.4.3528.0331_ship.client.main.w5m4 (ship) |
| InternalName | WLXPhotoBase |
| LegalCopyright | 2012 Microsoft Corporation. All rights reserved. |
| OriginalFilename | WLXPhotoBase.dll |
| ProductName | Photo Gallery |
| ProductVersion | 16.4.3528.0331 |

## Section Analysis

| Section | Virtual Addr | Virtual Size | Raw Size | Flags | Purpose |
|---------|-------------|-------------|----------|-------|---------|
| .text | 0x1000 | 0x5378 | 0x5400 | Code, Execute Read | Compiled C++ code |
| .data | 0x7000 | 0x3410 | 0x0400 | Initialized Data, RW | Global/static variables, vtables |
| .tls | 0xB000 | 0x0003 | 0x0200 | Initialized Data, RW | Thread-local storage |
| .rsrc | 0xC000 | 0x03E8 | 0x0400 | Initialized Data, RO | Version info, resources |
| .reloc | 0xD000 | 0x0A3C | 0x0C00 | Initialized Data, Discardable, RO | Base relocations (ASLR) |

**Notes:**
- .data section is large (0x3410 virtual) relative to raw (0x0400) - holds vtables, global data, and initialized statics
- .tls section indicates use of `__declspec(thread)` or thread-local storage callbacks
- Total virtual image size: 56 KB - this is a lightweight foundation library

## Complete Export Table (56 functions)

All exports use C++ decorated (mangled) names. Ordinal base = 1.

### Exception Handling

| Ordinal | RVA | Mangled Name | Demangled Name |
|---------|-----|-------------|----------------|
| 1 | 0x1970 | `??0Exception@Base@@IAE@J@Z` | `Base::Exception::Exception(long)` (private ctor) |
| 2 | 0x1B90 | `??0Exception@Base@@QAE@ABV01@@Z` | `Base::Exception::Exception(const Exception&)` (copy ctor) |
| 7 | 0x1939 | `??1Exception@Base@@UAE@XZ` | `Base::Exception::~Exception()` (virtual dtor) |
| 8 | 0x1939 | `??1OutOfMemoryException@Base@@UAE@XZ` | `Base::OutOfMemoryException::~OutOfMemoryException()` (virtual dtor) |
| 9 | 0x1895 | `??4Exception@Base@@QAEAAV01@ABV01@@Z` | `Base::Exception::operator=(const Exception&)` |
| 10 | 0x1895 | `??4OutOfMemoryException@Base@@QAEAAV01@ABV01@@Z` | `Base::OutOfMemoryException::operator=(const OutOfMemoryException&)` |
| 12 | 0x1992 | `??BException@Base@@QBEJXZ` | `Base::Exception::operator long() const` (implicit HRESULT conversion) |
| 15 | 0x1218 | `??_7Exception@Base@@6B@` | `Base::Exception::vtable` (vftable for typeinfo) |
| 16 | 0x1210 | `??_7OutOfMemoryException@Base@@6B@` | `Base::OutOfMemoryException::vtable` |
| 37 | 0x1B70 | `?Init@Exception@Base@@AAEXJ@Z` | `Base::Exception::Init(long)` (private) |
| 52 | 0x1BB5 | `?ThreadID@Exception@Base@@QBEKXZ` | `Base::Exception::ThreadID() const` |
| 53 | 0x1BBE | `?Throw@Base@@YGXJ@Z` | `Base::Throw(long)` (free function - throws HRESULT via SEH) |
| 54 | 0x1BD1 | `?ThrowLastError@Base@@YGXXZ` | `Base::ThrowLastError()` (free function) |

### OutOfMemoryException

| Ordinal | RVA | Mangled Name | Demangled Name |
|---------|-----|-------------|----------------|
| 3 | 0x1908 | `??0OutOfMemoryException@Base@@IAE@XZ` | `Base::OutOfMemoryException::OutOfMemoryException()` (private ctor) |
| 4 | 0x1945 | `??0OutOfMemoryException@Base@@QAE@ABV01@@Z` | `Base::OutOfMemoryException::OutOfMemoryException(const OutOfMemoryException&)` |

### Version

| Ordinal | RVA | Mangled Name | Demangled Name |
|---------|-----|-------------|----------------|
| 5 | 0x20D6 | `??0Version@Base@@QAE@ABV01@@Z` | `Base::Version::Version(const Version&)` (copy ctor) |
| 6 | 0x20CA | `??0Version@Base@@QAE@XZ` | `Base::Version::Version()` (default ctor) |
| 11 | 0x21B7 | `??4Version@Base@@QAEAAV01@ABV01@@Z` | `Base::Version::operator=(const Version&)` |
| 13 | 0x218F | `??MVersion@Base@@QBE_NABV01@@Z` | `Base::Version::operator<(const Version&) const` |
| 14 | 0x2167 | `??OVersion@Base@@QBE_NABV01@@Z` | `Base::Version::operator>(const Version&) const` |
| 17 | 0x21F1 | `?AsString@Version@Base@@QBEXPAVString@2@@Z` | `Base::Version::AsString(Base::String*) const` |
| 25 | 0x21E7 | `?Invalidate@Version@Base@@QAEXXZ` | `Base::Version::Invalidate()` |
| 27 | 0x21DE | `?IsValid@Version@Base@@QBE_NXZ` | `Base::Version::IsValid() const` |
| 48 | 0x20F9 | `?Set@Version@Base@@QAEXABUtagVS_FIXEDFILEINFO@@@Z` | `Base::Version::Set(const VS_FIXEDFILEINFO&)` |
| 49 | 0x2136 | `?Set@Version@Base@@QAEXGGGG@Z` | `Base::Version::Set(unsigned short, unsigned short, unsigned short, unsigned short)` |
| 50 | 0x2119 | `?Set@Version@Base@@QAEXKK@Z` | `Base::Version::Set(unsigned long, unsigned long)` |

### Error Handling & Assertions

| Ordinal | RVA | Mangled Name | Demangled Name |
|---------|-----|-------------|----------------|
| 20 | 0x1A52 | `?DecrementNoAssertCount@Base@@YGXXZ` | `Base::DecrementNoAssertCount()` |
| 22 | 0x36C6 | `?DisableShipAsserts@Base@@YGXXZ` | `Base::DisableShipAsserts()` |
| 24 | 0x1A6E | `?GetAssertCallback@Base@@YGAAP6G_NPBDH0@ZXZ` | `Base::GetAssertCallback()` |
| 33 | 0x358C | `?GetReportMetrics@Base@@YGXPAUReportMetrics@1@@Z` | `Base::GetReportMetrics(ReportMetrics*)` |
| 34 | 0x35DC | `?GetReportsForSqm@Base@@YGXPAUReportsForSqm@1@@Z` | `Base::GetReportsForSqm(ReportsForSqm*)` |
| 35 | 0x369C | `?GetReportsForWer@Base@@YGXPAUReportsForWer@1@@Z` | `Base::GetReportsForWer(ReportsForWer*)` |
| 36 | 0x1A41 | `?IncrementNoAssertCount@Base@@YGXXZ` | `Base::IncrementNoAssertCount()` |
| 39 | 0x1A79 | `?IsOutOfMemoryError@Base@@YG_NJ@Z` | `Base::IsOutOfMemoryError(long)` |
| 45 | 0x1A63 | `?NoAssertCount@Base@@YGHXZ` | `Base::NoAssertCount()` |
| 46 | 0x1AA9 | `?ReportError@BasePrivate@@YA_NPBDHPB_W0PAD0KH0@Z` | `BasePrivate::ReportError(const char*, int, const wchar_t*, unsigned long, char*, unsigned long, const char*)` |
| 47 | 0x370E | `?ReportFault@Base@@YGXJ@Z` | `Base::ReportFault(long)` |
| 51 | 0x3640 | `?SetReportsForWer@Base@@YGXPAUReportsForWer@1@@Z` | `Base::SetReportsForWer(ReportsForWer*)` |
| 56 | 0x7598 | `?s_nAssertsInhibited@AssertInhibitor@BasePrivate@@2HA` | `BasePrivate::AssertInhibitor::s_nAssertsInhibited` (static int) |

### String Management

| Ordinal | RVA | Mangled Name | Demangled Name |
|---------|-----|-------------|----------------|
| 18 | 0x1BBE | `?BaseAtlThrow@ATL@@YGXJ@Z` | `ATL::BaseAtlThrow(long)` (ATL error throw helper) |
| 19 | 0x1BED | `?BaseAtlThrowLastError@ATL@@YGXXZ` | `ATL::BaseAtlThrowLastError()` |
| 27 | 0x1EEA | `?GetBaseStringComManager@StringCom@Base@@SGAAVCAtlStringMgr@ATL@@XZ` | `Base::StringCom::GetBaseStringComManager()` - returns `ATL::CAtlStringMgr&` |
| 28 | 0x1EDF | `?GetBaseStringManager@String@Base@@SGAAVCAtlStringMgr@ATL@@XZ` | `Base::String::GetBaseStringManager()` - returns `ATL::CAtlStringMgr&` |

### Memory Management

| Ordinal | RVA | Mangled Name | Demangled Name |
|---------|-----|-------------|----------------|
| 21 | 0x1D4C | `?Delete@BasePrivate@@YAXPAX@Z` | `BasePrivate::Delete(void*)` |
| 23 | 0x199B | `?EnableLeakTrackingAndSetSymbolPath@Base@@YGX_N@Z` | `Base::EnableLeakTrackingAndSetSymbolPath(bool)` |
| 44 | 0x1CDF | `?New@BasePrivate@@YAPAXI_N@Z` | `BasePrivate::New(unsigned int, bool)` |
| 55 | 0x1AD5 | `?VerifyPtr@BasePrivate@@YA_NPBX@Z` | `BasePrivate::VerifyPtr(const void*)` |

### OS/CPU Detection

| Ordinal | RVA | Mangled Name | Demangled Name |
|---------|-----|-------------|----------------|
| 26 | 0x1C06 | `?GdiplusStatusToHresult@Base@@YGJH@Z` | `Base::GdiplusStatusToHresult(Gdiplus::Status)` |
| 28 | 0x1E20 | `?IsVistaOrGreater@OS@Base@@YG_NXZ` | `Base::OS::IsVistaOrGreater()` |
| 29 | 0x1E7F | `?IsWin7OrGreater@OS@Base@@YG_NXZ` | `Base::OS::IsWin7OrGreater()` |
| 30 | 0x1E20 | `?IsWin8OrGreater@OS@Base@@YG_NXZ` | `Base::OS::IsWin8OrGreater()` |
| 31 | 0x19DB | `?GetProcessorCaps@CPU@Base@@YGXAATCPUCaps@12@@Z` | `Base::CPU::GetProcessorCaps(TCPUCaps&)` |
| 32 | 0x1A25 | `?GetProcessorCount@CPU@Base@@YGHXZ` | `Base::CPU::GetProcessorCount()` |

### Module Version

| Ordinal | RVA | Mangled Name | Demangled Name |
|---------|-----|-------------|----------------|
| 29 | 0x3324 | `?GetModuleAddresses@Base@@YGJPAXPAUModuleAddresses@1@@Z` | `Base::GetModuleAddresses(void*, ModuleAddresses*)` |
| 30 | 0x33B2 | `?GetModuleVersion@Base@@YGJPAUHINSTANCE__@@PAUModuleVersion@1@@Z` | `Base::GetModuleVersion(HINSTANCE*, ModuleVersion*)` |

### Enable Ship Asserts

| Ordinal | RVA | Mangled Name | Demangled Name |
|---------|-----|-------------|----------------|
| 24 | 0x36B1 | `?EnableShipAsserts@Base@@YGXPAX0PAUModuleVersion@1@@Z` | `Base::EnableShipAsserts(void*, void*, ModuleVersion*)` |

## Import Table Analysis

### Direct Imports (7 DLLs)

#### MSVCR110.dll (Visual C++ 2012 Runtime)
- Memory: `malloc`, `free`, `realloc`, `memcpy`, `memset`, `memcpy_s`, `wmemcpy_s`, `_msize`, `_calloc_crt`, `_malloc_crt`
- Exception handling: `_CxxThrowException`, `__CxxFrameHandler3`, `??3@YAXPAX@Z` (operator delete), `??2@YAPAXI@Z` (operator new), `?terminate@@YAXXZ`
- String: `_wcsicmp`, `_vsnwprintf`, `_vscwprintf`, `vswprintf_s`
- CRT init: `_initterm`, `_initterm_e`, `_amsg_exit`, `_purecall`
- Threading: `_beginthreadex`
- Process: `__crtTerminateProcess`, `__crtUnhandledException`
- Debug: `_crt_debugger_hook`, `_except_handler4_common, `__CppXcptFilter`
- Locking: `_lock`, `_unlock`
- Other: `_onexit`, `__dllonexit`, `?_query_new_handler@@YAP6AHI@ZXZ`, `??1type_info@@UAE@XZ`, `__clean_type_info_names_internal`

#### KERNEL32.dll (Core Win32)
- Process/Thread: `GetCurrentProcess`, `GetCurrentThread`, `GetCurrentThreadId`, `GetExitCodeThread`, `SetThreadPriority`, `_beginthreadex`, `DisableThreadLibraryCalls`
- Synchronization: `InitializeCriticalSectionAndSpinCount`, `EnterCriticalSection`, `LeaveCriticalSection`, `DeleteCriticalSection`, `InterlockedIncrement`, `InterlockedDecrement`, `InterlockedExchange`, `SetEvent`, `WaitForSingleObject`
- Module: `GetModuleHandleW`, `GetModuleHandleExW`, `GetModuleFileNameW`, `LoadLibraryExA`, `LoadLibraryExW`, `FreeLibrary`, `GetProcAddress`
- Memory: `EncodePointer`, `DecodePointer`
- OS info: `GetVersionExW`, `IsDebuggerPresent`, `IsProcessorFeaturePresent`, `GetSystemInfo`, `GetTickCount64`, `GetSystemTimeAsFileTime`
- Handles: `CloseHandle`, `DuplicateHandle`
- Resources: `FindResourceW`, `LoadResource`, `LockResource`, `SizeofResource`
- Error: `GetLastError`, `RaiseException`
- WER: `WerRegisterMemoryBlock`, `WerUnregisterMemoryBlock`

#### PSAPI.DLL
- `GetModuleInformation` - for getting module base/size

#### wer.dll (Windows Error Reporting)
- `WerReportCreate`, `WerReportAddDump`, `WerReportSetParameter`, `WerReportSubmit`, `WerReportCloseHandle`

#### VERSION.dll
- `GetFileVersionInfoSizeW`, `GetFileVersionInfoW`, `VerQueryValueW`

#### SHLWAPI.dll
- `PathFindFileNameW` - shell path utility

### Delay-Loaded Imports (1 DLL)

#### ole32.dll
- `CoTaskMemAlloc`, `CoTaskMemFree`, `CoTaskMemRealloc`, `CoGetMalloc`
- Delay-loaded because these are only needed for COM memory allocation and the DLL can function without OLE being initialized

## COM GUIDs

**No COM GUIDs (CLSIDs, IIDs, LIBIDs) found.** This DLL does not register or implement any COM objects. It is a pure C++ library with decorated (mangled) exports.

### PDB GUID
- `{0674DC61-4F42-4D44-AD50-155EB0251FA5}` - debug symbol file identifier

### Binary GUIDs Found (in .text section, likely compile-time constants or debug metadata)
- `4faf0b71-ad37-4aa3-a671-76bc052344ad`
- `2860b52e-c4a3-454d-bc1e-32c5add17e90`

These appear to be embedded GUIDs (possibly for WER reporting, version tagging, or internal correlation IDs). They are not COM registration GUIDs.

## All String Literals

### Version Info Strings
- `%d.%d.%d.%d` - version format string
- `%u.%u.%u.%u` - version format string (unsigned)
- `%08x` - hex format string
- `16.4.3528.0331_ship.client.main.w5m4 (ship)` - full build string
- `16.4.3528.0331` - short version

### Error Reporting / Assert Strings
- `WindowsLiveShipAssert` - ship assert notification identifier
- `AppName` - report field name
- `AppVersion` - report field name
- `AppTimeStamp` - report field name
- `ModName` - report field name (module name)
- `ModVersion` - report field name (module version)
- `ModTimeStamp` - report field name
- `Offset` - report field name (code offset)
- `HResultError` - report field name (HRESULT value)

### Resource Strings
- `WLXPhotoLibraryDuiResourcesLocalized` - resource DLL name for DUI (DirectUI) resources
- ` !"#$%&'()*+,-./01234567` - font character mapping string

### Product/Version Info
- `Photo Base Library` - file description
- `WLXPhotoBase` - internal name
- `WLXPhotoBase.dll` - original filename
- `Microsoft Corporation` - company name
- `Photo Gallery` - product name
- ` 2012 Microsoft Corporation. All rights reserved.` - copyright

### Legal/Localization
- `$Windows Essential` - product branding prefix
- `Legal_policy_statement` - localization key
- `Legal_Policy_Statement` - localization key
- `SubC` - sub-component identifier

### Section Names (PE)
- `.text`, `.data`, `.tls`, `.rsrc`, `.reloc`

## Function Categories

### 1. Exception Handling (13 exports - 23%)
Core SEH-based exception system. `Base::Exception` wraps HRESULT values and can be thrown/caught across the WLX component boundary. `OutOfMemoryException` is a specialized subclass.

**Key functions:** Exception ctor/dtor/copy/assign, operator=HRESULT, Throw, ThrowLastError, Init, ThreadID

### 2. Version Management (11 exports - 20%)
`Base::Version` class for parsing, comparing, and formatting module/file versions. Supports VS_FIXEDFILEINFO, four-component (major.minor.build.rev), and two-component (major.minor) formats.

**Key functions:** Version ctor/copy/assign, operator<, operator>, AsString, Set (3 overloads), IsValid, Invalidate

### 3. Error Reporting & Assertions (13 exports - 23%)
Ship assert infrastructure, WER (Windows Error Reporting) integration, SQM (Microsoft telemetry) metrics, and fault reporting. The `NoAssertCount` mechanism suppresses asserts during expected error paths.

**Key functions:** EnableShipAsserts/DisableShipAsserts, IncrementNoAssertCount/DecrementNoAssertCount/NoAssertCount, GetAssertCallback, ReportError, ReportFault, GetReportMetrics, GetReportsForSqm, GetReportsForWer, SetReportsForWer

### 4. Memory Management (4 exports - 7%)
Custom heap allocators in the `BasePrivate` namespace. `New`/`Delete` provide debug-friendly allocation with optional zero-init. `VerifyPtr` validates pointer validity. `EnableLeakTrackingAndSetSymbolPath` activates CRT leak detection.

### 5. String Management (4 exports - 7%)
ATL CString manager factories. `GetBaseStringManager` provides the global `CAtlStringMgr` for all WLX components, ensuring all ATL strings share a common allocator. `BaseAtlThrow`/`BaseAtlThrowLastError` are ATL error helpers.

### 6. OS/CPU Detection (6 exports - 11%)
Windows version detection (Vista+, Win7+, Win8+) and CPU capabilities/processor count queries. Used by dependent DLLs to conditionally use features.

**Key functions:** IsVistaOrGreater, IsWin7OrGreater, IsWin8OrGreater, GetProcessorCaps, GetProcessorCount, GdiplusStatusToHresult

### 7. Module Version (2 exports - 4%)
Reads VERSION.DLL-based version resources from other loaded modules at runtime.

**Key functions:** GetModuleAddresses, GetModuleVersion

### 8. Static Data (1 export - 2%)
- `BasePrivate::AssertInhibitor::s_nAssertsInhibited` - global assert inhibition counter

## Relationship to Reconstructed Source Code

### What the Original DLL Actually Exports vs. What Our Source Claims

The original binary exports **56 functions**. Our reconstructed `WLXPhotoBase.def` (88 lines) claims to export additional classes not present in the actual binary:

| Claimed in .def | Present in Binary | Status |
|----------------|-------------------|--------|
| Base::Exception (core) | YES | Match - but different signatures |
| Base::Throw / ThrowLastError | YES | Match |
| Base::GdiplusStatusToHresult | YES | Match |
| Base::String::GetBaseStringManager | YES | Match |
| Base::OS::IsWin7OrGreater / IsWin8OrGreater | YES | Match - but binary also has IsVistaOrGreater |
| Base::Private::New / Delete | YES | Match |
| Base::CPU::GetProcessorCount | YES | Match |
| ATL::BaseAtlThrow | YES | Match |
| Base::File | **NO** | NOT in original binary |
| Base::TempFile | **NO** | NOT in original binary |
| Base::Thread | **NO** | NOT in original binary |
| Base::FindFile | **NO** | NOT in original binary |
| Base::GdiException | **NO** | NOT in original binary |
| Base::DataStructs::IntSet | **NO** | NOT in original binary |
| RefCountBase | **NO** | Template, not exported |

### Functions in Binary But Missing from Our Source

| Function | Purpose |
|----------|---------|
| `Base::OutOfMemoryException` | OOM exception subclass (ctor, dtor, copy, assign) |
| `Base::Version` | Version parsing/comparison class (11 functions) |
| `Base::DecrementNoAssertCount` | Assert suppression |
| `Base::DisableShipAsserts` / `EnableShipAsserts` | Assert control |
| `Base::GetAssertCallback` | Assert handler query |
| `Base::ReportError` | Error reporting (BasePrivate) |
| `Base::ReportFault` | WER fault reporting |
| `Base::GetReportMetrics` | SQM metrics |
| `Base::GetReportsForSqm` / `GetReportsForWer` / `SetReportsForWer` | Telemetry |
| `Base::IsOutOfMemoryError` | HRESULT OOM check |
| `Base::NoAssertCount` | Assert count query |
| `Base::VerifyPtr` | Pointer validation (BasePrivate) |
| `Base::EnableLeakTrackingAndSetSymbolPath` | CRT leak tracking |
| `Base::GetModuleAddresses` | Module base/size query |
| `Base::GetModuleVersion` | Module version reader |
| `Base::CPU::GetProcessorCaps` | CPU feature flags |
| `Base::OS::IsVistaOrGreater` | Vista detection |
| `BasePrivate::AssertInhibitor::s_nAssertsInhibited` | Static assert flag |
| `ATL::BaseAtlThrowLastError` | ATL error throw |
| `Base::StringCom::GetBaseStringComManager` | COM string manager |

### Key Observations

1. **Our .cpp file is overly ambitious** - it implements File, TempFile, Thread, FindFile, GdiException, and IntSet classes that do NOT appear in the original DLL's export table. These classes likely exist in a different DLL or are implemented inline/header-only.

2. **Missing critical classes** - The original DLL heavily features `Base::Version` (11 exports!) and the error reporting/assert infrastructure, neither of which are in our reconstructed source.

3. **Different Exception design** - The original has a private `Init(long)` constructor and a `ThreadID()` accessor that our reconstruction omits. The `operator HRESULT` is named `operator B` (the `??B` mangled prefix) suggesting it's `operator HRESULT()`.

4. **Namespace difference** - The original uses `BasePrivate` (not `Base::Private`) for `New`, `Delete`, `VerifyPtr`, `ReportError`. Our source uses `Base::Private`.

5. **BaseTypes.h is correct for templates** - The RefCountBase, PtrRef, Array, NoHeap template classes are header-only and wouldn't appear in exports. These match the binary's design.

6. **Version target mismatch** - Our header targets `_WIN32_WINNT 0x0602` (Win8) which matches. The binary confirms this via `IsVistaOrGreater`/`IsWin7OrGreater`/`IsWin8OrGreater`.

## Architecture Summary

WLXPhotoBase.dll is the **foundation library** for the entire Windows Live Photo Gallery / Movie Maker 2012 suite. It provides:

1. **Exception model** - Custom SEH-based exceptions wrapping HRESULT codes, used by all WLX DLLs for error propagation
2. **Version management** - Parsing and comparing semantic versions from PE resources
3. **Error reporting** - Integration with Windows Error Reporting (WER) and Microsoft telemetry (SQM)
4. **Assert infrastructure** - Ship-mode assert suppression with callback hooks
5. **Memory management** - Custom allocators with leak tracking support
6. **String management** - Global ATL CString allocator shared across all WLX components
7. **OS/CPU detection** - Runtime feature detection for conditional code paths

The library is intentionally lightweight (56 KB image, 21 KB code) and has no COM dependencies. It serves as the common base that all other WLX DLLs link against.
