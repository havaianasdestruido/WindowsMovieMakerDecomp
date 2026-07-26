# Windows Live Movie Maker 2012 - DLL Architecture Knowledge Base

## Shared Analysis State
This file is continuously updated by all subagents. Each DLL analysis contributes to the aggregate picture.

## DLL Inventory

### Core Application DLLs (we have source code)
| DLL | Status | Purpose |
|-----|--------|---------|
| MovieMaker.exe | Source reconstructed | Main application entry point |
| MovieMakerCore.dll | Source reconstructed | Core project model, timeline, rendering |
| MovieMakerLang.dll | Stubs only | Language resources |
| MovieMakerPreviewClient.dll | Stubs only | Preview client |

### Media Foundation DLLs (original binaries only)
| DLL | Size | Purpose |
|-----|------|---------|
| WLMFDS.dll | ? | Media Foundation DirectShow bridge |
| WLMFReadWrite.dll | ? | Media Foundation Read/Write |
| WLXMP4Parser.dll | ? | MP4 container parser |

### Pipeline/Rendering DLLs
| DLL | Size | Purpose |
|-----|------|---------|
| WLXPipeline.dll | ? | Media processing pipeline |
| WLXPipetran.dll | ? | Pipeline transport/transform |
| WLXPhotoCinematic.dll | ? | Cinematic effects rendering |
| WLXSlideshow.dll | ? | Slideshow generation |
| WLXCodecHost.exe | ? | Codec hosting process |

### Photo/Base DLLs
| DLL | Size | Purpose |
|-----|------|---------|
| WLXPhotoBase.dll | 56 KB image (21 KB code) | Base types, exception handling, versioning, error reporting, memory/string management |
| WLXFaceRecognition.dll | ? | Face detection/recognition |
| WLXVideoTrim.dll | 568 KB | Video trimming engine (DirectShow) |
| WLXMovieLibrary.dll | ? | Movie library management |

### Publishing DLLs
| DLL | Size | Purpose |
|-----|------|---------|
| WLXMediaPublishSubscribe.dll | ? | Publish/subscribe framework |
| WLFacebookPlugin.dll | ? | Facebook upload plugin |
| WLFlickrPlugin.dll | ? | Flickr upload plugin |
| WLVimeoPlugin.dll | ? | Vimeo upload plugin |
| WLYouTubePlugin.dll | ? | YouTube upload plugin |

## COM GUIDs Discovered

Updated from binary ASCII/UTF-16 string extraction + .rgs registry script extraction across 23+ WLMM binaries. Full report: `analysis/COMGuids/guids.md`.

### CORRECTED CLSIDs (2026-07-26)

| GUID | Previous Label | Corrected Label | Source |
|------|---------------|-----------------|--------|
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | ~~CLSID_FaceRegion/FaceRegionSet~~ | **CLSID_FaceRecognitionPipeline** | .rgs confirmed |
| `{EF401225-1260-4716-A842-7D180DC14C1E}` | ~~CLSID_FaceRecognitionPipeline~~ | **TypeLib IID** | .rgs confirmed |

### Key CLSIDs Found in Binary Strings + .rgs (verified)

| GUID | Name | DLL(s) |
|------|------|--------|
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | CLSID_FaceRecognitionPipeline | WLXFaceRecognition.dll |
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | CLSID_FaceDetection | WLXFaceRecognition.dll |
| `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` | CLSID_ImageManager | WLXFaceRecognition.dll |
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | CLSID_VideoTrim | WLXVideoTrim, WLXPipeline, WLXMediaPublishSubscribe |
| `{84FBA192-4F8D-4a5d-94DE-083446ACC1D0}` | COM GUID (WLXSlideshow) | WLXSlideshow.dll |
| `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}` | CLSID_CinematicFullScreen | WLXPhotoCinematic.dll |
| `{5409AB48-D8D3-40e6-A1EB-23489DC422DE}` | CLSID_CinematicTransform | WLXPhotoCinematic.dll |
| `{E30A45E6-1916-4659-95EE-035E62DB9AB0}` | Codec Host | WLXCodecHost.exe |
| `{121244C5-B0AD-45fb-9D8E-B893B917D053}` | GrinderScheduler | WLXGrinderScheduler.dll |

### WLXMediaPublishSubscribe Interfaces (confirmed by .rgs)

| GUID | Confirmed Name |
|------|----------------|
| `{5F4019FA-375D-4D74-9922-EFCD9D920A0D}` | Live Publish and Subscribe Provider Interface |
| `{197608E2-D42D-43a5-927E-1C67FC041431}` | Flickr Publish and Subscribe Provider Interface |
| `{43DBAB44-8EDC-4FB0-B305-F117ECB15305}` | Publish and Subscribe Provider Manager Interface |
| `{8ab5eea6-7a20-417d-b6d1-d40c9804b2cc}` | Online Media Plugin Manager Interface |
| `{54c41e30-bfd8-44e5-91a2-ce038e242817}` | Plugin Decorator Interface |
| `{66A5A6CA-0D84-44cb-BEC4-04B3B888E4B2}` | Publish Plugin Manager Helper |
| `{C9C25561-A401-43b4-BC22-79FBA2A00042}` | Internet Cache Manager |
| `{CE5E100A-6CE8-4d67-8870-245DFA3499AB}` | Publish Plugin Helper |
| `{DA69067E-3959-47ca-A58D-2300786168CD}` | Metadata Settings Controller Interface |
| `{7B996FF4-FFF2-4573-9728-C1A612BD8592}` | TypeLib IID |

### Newly Discovered COM CLSIDs (from .rgs scan, 2026-07-26)

**WLXPhotoClassic.dll (5):** Classic, Classic Transform, Black&White, Sepia, Basic themes
**WLXPhotoVoyager.dll (9):** Album, Collage, Flip, Frame, Glass, Snapshots, Travel themes + Voyager Transform
**WLXPhotoAcq.dll (5):** LivePhotoAcquire, OptionsDialog, PickerDialog, DeviceSelectionDlg, AcquisitionWizard
**WLXImageTranscode.dll (3):** ImageTranscode, ImageLoader, WLXOutofProc
**WLXQuickTimeControlHost.exe (2):** QuickTimePlayerHost, QuickTimeMovieThumbnail
**WLXVideoAcquireWizard.exe (2):** MSLive Capture Wizard, Auto Capture
**WLXVideoCameraAutoPlayManager.exe (1):** WLXHWEventHandler
**WLXDSPA.dll (11):** CWLXTocGeneratorDmo, CWLXThumbnailGeneratorDmo, CClusterDetectorEx, CTocEntry, CTocEntryList, CToc, CTocCollection, CTocParser, CFileIo, CAsfTocParser, CAviTocParser
**WLAVRes.dll (8):** AV Resource DLL COM classes (all threading=both)
**Imaging.dll (~65):** WIC-based image effects engine + CaptureOne RAW pipeline

### Movie Maker App Identifiers

| GUID | Purpose | DLL |
|------|---------|-----|
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | App/module identifier | MovieMakerCore.dll |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Telemetry/error endpoint | MovieMakerCore.dll |
| `{9590CDCE-BFFC-45d5-85EA-493C65FB2C4F}` | Transition/effect template | MovieMakerCore.dll |

### Publishing Plugin GUIDs (WLXMediaPublishSubscribe.dll)
10 new GUIDs discovered: `{197608E2...}`, `{43DBAB44...}`, `{54c41e30...}`, `{5F4019FA...}`, `{66A5A6CA...}`, `{7B996FF4...}`, `{8ab5eea6...}`, `{C9C25561...}`, `{CE5E100A...}`, `{DA69067E...}` — likely CLSIDs/IIDs for LiveProvider, FlickrProvider, MediaPluginManager, MetadataManager, etc.

### Cross-DLL Shared CLSIDs
- `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` (CLSID_VideoTrim) referenced in **3 DLLs**: WLXVideoTrim (defines it), WLXPipeline (uses it), WLXMediaPublishSubscribe (uses it)

### TypeLib GUIDs (9 discovered)

| TypeLib | DLL | CLSIDs |
|---------|-----|--------|
| `{AEE505D4-908A-4BFC-9A7E-31AF955C15BF}` | Imaging.dll | ~65 |
| `{7B996FF4-FFF2-4573-9728-C1A612BD8592}` | WLXMediaPublishSubscribe.dll | 8 |
| `{D85885D4-A18A-4fac-AEE1-BE9AAB0F4485}` | WLXPhotoAcq.dll | 5 |
| `{17DC7884-443D-478e-ABD7-BC22856FC7F1}` | WLXImageTranscode.dll | 3 |
| `{C6D340BB-0CEA-4923-8082-51036E472379}` | WLXQuickTimeControlHost.exe | 2 |
| `{5ab7792c-0f76-4003-aa47-5f075165d4de}` | WLXVideoAcquireWizard.exe | 2 |
| `{9b5c8343-bdee-475d-9d3b-3715c6b8972e}` | WLXVideoCameraAutoPlayManager.exe | 1 |
| `{77A16B7E-9DFE-410c-8863-264BF3E91103}` | WLXGrinderScheduler.dll | 1 |
| `{60E1FA84-4F2F-417C-AEE4-7681A960D09E}` | AlbumDownloadProtocolHandler.dll | 1 |

### Certificate GUIDs (non-COM, present in ALL binaries)
- `{4faf0b71-ad37-4aa3-a671-76bc052344ad}`
- `{2860b52e-c4a3-454d-bc1e-32c5add17e90}`

### Known GUIDs NOT found via string scanning
These exist in the binaries as 16-byte GUID structs or in .rgs resources, not as string literals:
- CLSID_CinematicFullScreen `{B1CACF91-...}`, CLSID_CinematicTransform `{5409AB48-...}` (WLXPhotoCinematic)
- Transition DLL GUIDs `{0B1A232A-...}`, `{7371ADEE-...}` (embedded XML resources)
- All PDB GUIDs (stored as debug directory entries)

### Extraction Method
- ASCII scan: contiguous 7-bit printable strings
- UTF-16LE scan: wide-character strings (char + 0x00 pairs)
- Not scanned: 16-byte binary GUID structs (requires PE section analysis), embedded .rgs registry scripts

### Unknown GUIDs (need investigation)
8 GUIDs found in WLXMediaPublishSubscribe.dll with no known assignment: `{0DDA997F...}`, `{1812A500...}`, `{1D31145D...}`, `{66557ED9...}`, `{7D2A6ECD...}`, `{8BE133F2...}`, `{8DB100C7...}`, `{D78480B9...}`.

## Inter-DLL Dependencies
(Updated by Cross-DLL import subagent)

## Architecture Notes
- Codename: Sundance
- Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+)
- Uses ATL/WTL, DirectX 11, Media Foundation, COM, Windows Ribbon
- 32-bit (x86) binaries
- DLLs found in undecomp/Photo Gallery/

## WLXPhotoBase.dll Analysis

### Overview
Foundation library (56 KB image, 21 KB code) used by all WLX components. PE32 x86 DLL, linker v11.0, image base 0x10000000, ASLR+NX. Timestamp: Apr 1 2014. Build: 16.4.3528.0331_ship.client.main.w5m4. PDB: {0674DC61-4F42-4D44-AD50-155EB0251FA5}.

### Exports (56 functions)
All C++ mangled names in `Base` namespace. Key categories:
- **Exception handling (13)**: `Base::Exception` (wraps HRESULT, private ctor+Init, operator HRESULT, ThreadID, copy/assign/dtor), `Base::OutOfMemoryException` (subclass), `Base::Throw(long)`, `Base::ThrowLastError()`
- **Version management (11)**: `Base::Version` class - default/copy ctors, operator=, operator<, operator>, AsString, Set (3 overloads: VS_FIXEDFILEINFO, u16x4, u32x2), IsValid, Invalidate
- **Error reporting & asserts (13)**: EnableShipAsserts/DisableShipAsserts, NoAssertCount/IncrementNoAssertCount/DecrementNoAssertCount, GetAssertCallback, ReportError (BasePrivate), ReportFault, GetReportMetrics, GetReportsForSqm, GetReportsForWer, SetReportsForWer, s_nAssertsInhibited
- **Memory (4)**: BasePrivate::New(size_t, bool), BasePrivate::Delete(void*), VerifyPtr, EnableLeakTrackingAndSetSymbolPath
- **String (4)**: GetBaseStringManager, GetBaseStringComManager (ATL::CAtlStringMgr), BaseAtlThrow, BaseAtlThrowLastError
- **OS/CPU (6)**: IsVistaOrGreater, IsWin7OrGreater, IsWin8OrGreater, GetProcessorCount, GetProcessorCaps, GdiplusStatusToHresult
- **Module version (2)**: GetModuleAddresses, GetModuleVersion

### Imports
- MSVCR110.dll (CRT): memory, exception, threading, string, CRT init
- KERNEL32.dll: process/thread, synchronization, module loading, OS info, resources, WER registration
- PSAPI.DLL: GetModuleInformation
- wer.dll: WerReportCreate/AddDump/SetParameter/Submit/CloseHandle
- VERSION.dll: GetFileVersionInfoSizeW/GetFileVersionInfoW/VerQueryValueW
- SHLWAPI.dll: PathFindFileNameW
- ole32.dll (delay-load): CoTaskMemAlloc/Free/Realloc, CoGetMalloc

### No COM GUIDs
This DLL has no CLSID/IID/LIBID registrations. Two binary GUIDs found (4faf0b71-... and 2860b52e-...) are likely WER/telemetry correlation IDs, not COM.

### String Literals
- Error fields: `AppName`, `AppVersion`, `AppTimeStamp`, `ModName`, `ModVersion`, `ModTimeStamp`, `Offset`, `HResultError`
- Ship assert: `WindowsLiveShipAssert`
- Format strings: `%d.%d.%d.%d`, `%u.%u.%u.%u`, `%08x`
- Resource: `WLXPhotoLibraryDuiResourcesLocalized` (DirectUI resource DLL name)
- Legal: `$Windows Essential`, `Legal_policy_statement`, `Legal_Policy_Statement`

### Sections
- .text (0x1000, 21 KB code), .data (0x7000, 13 KB - vtables+globals), .tls (0xB000, 3 bytes), .rsrc (0xC000, 1 KB), .reloc (0xD000, 2.6 KB)

### Key Corrections to Reconstructed Source
Our `WLXPhotoBase.def` and `.cpp` claim exports for File, TempFile, Thread, FindFile, GdiException, and IntSet classes - **none of these exist in the actual binary**. The original DLL is focused on exception handling, versioning, error reporting, and memory/string management. The File/Thread/etc. classes are likely in a different DLL or are header-only templates.
Base namespace is `BasePrivate` (not `Base::Private`) for New/Delete/VerifyPtr.
The original DLL has a `Base::Version` class (11 exports) and full WER/SQM error reporting infrastructure that our reconstruction completely omits.

## MovieMaker.exe Analysis

### Summary
MovieMaker.exe is a **thin launcher/stub** (~54KB total, only ~4.8KB of actual code in .text). It contains no application logic itself - it initializes the CRT, sets secure DLL search paths, and calls `MovieMakerMain()` from `MovieMakerCore.dll` via delay-load.

### PE Headers
- **Machine**: x86 (0x14C), **Subsystem**: Windows GUI (2)
- **Entry Point**: 0x0040152F (CRT startup wrapper)
- **Image Base**: 0x00400000, **Image Size**: 0x1D000 (118,784 bytes)
- **Linker**: MSVC 11.00, **OS/Subsystem Ver**: 6.02/6.00 (Win8)
- **Timestamp**: 2014-04-01 01:27:07 UTC
- **DLL Characteristics**: Dynamic Base (ASLR), NX Compatible (DEP), Terminal Server Aware
- **Stack**: 256KB reserve / 8KB commit
- **Heap**: 1MB reserve / 4KB commit

### Section Layout
| Section | VA | VirtSize | RawSize | Purpose |
|---------|-------|----------|---------|---------|
| `.text` | 0x1000 | 0x12F0 | 0x1400 | Code (only 4,848 bytes!) |
| `.data` | 0x3000 | 0x398 | 0x200 | R/W initialized data |
| `.rsrc` | 0x4000 | 0x17D60 | 0x17E00 | Resources (97,632 bytes - largest!) |
| `.reloc` | 0x1C000 | 0x57A | 0x600 | Base relocations |

### Dependencies
**Direct (load-time):**
1. `KERNEL32.dll` (15 functions) - OS kernel: process/thread/memory/file, ASLR, DEP, security
2. `MSVCR110.dll` (26 functions) - VC++ 2012 CRT: initialization, exception handling, exit
3. `WLXPhotoBase.dll` (1 function) - `BasePrivate::Delete(void*)` for memory management

**Delay-loaded:**
4. `MovieMakerCore.dll` (1 function) - `MovieMakerMain()` - ALL real application logic

### Exports
None (EXE, not DLL).

### Key Function: MovieMakerMain
- Imported from `MovieMakerCore.dll` via delay-load mechanism
- This is the sole entry point from the EXE into application logic
- The delay-load HMODULE is stored at VA 0x00403378 in .data section
- All UI, project model, timeline, media processing, effects, rendering, and export logic lives in MovieMakerCore.dll

### Key Function: BasePrivate::Delete
- C++ mangled: `?Delete@BasePrivate@@YAXPAX@Z` = `void __cdecl BasePrivate::Delete(void*)`
- Imported from WLXPhotoBase.dll at load time
- Base memory deallocation used across all WLX DLLs

### Resources (RT_VERSION)
- **CompanyName**: Microsoft Corporation
- **FileDescription**: Movie Maker
- **FileVersion**: 16.4.3528.0331_ship.client.main.w5m4 (ship build)
- **InternalName**: Movie Maker
- **LegalCopyright**: (c) 2012 Microsoft Corporation. All rights reserved.
- **OriginalFilename**: MovieMaker.EXE
- **ProductName**: Movie Maker
- **ProductVersion**: 16.4.3528.0331
- **Language**: 0x0409 (English - US)
- Contains embedded PNG image(s) (app icon/branding)

### Manifest
- Assembly identity: `Microsoft.Windows.personalMedia.MovieMaker` v5.1.0.0
- DPI Aware: Yes
- Execution Level: asInvoker
- Depends on Common Controls 6.0.0.0

### Debug Info
- PDB: `MovieMaker.pdb` GUID {47558454-9C62-4123-96E9-91A66E8F4D87} age 1
- /GS buffer security: 27 functions protected
- Build: ship.client.main.w5m4 branch

### COM GUIDs
No application-level COM CLSIDs or IIDs in this binary. Only certificate authority OIDs (Microsoft Code Signing PCA GUIDs).

### Strings of Interest
- `MovieMakerMain` (key function name)
- `MovieMakerCore.dll` (delay-load target)
- `WLXPhotoBase.dll` (direct dependency)
- `MovieMaker.pdb` (debug symbols)
- `Microsoft.Windows.personalMedia.MovieMaker` (assembly name)
- Build string: `16.4.3528.0331_ship.client.main.w5m4 (ship)`

### Entry Point Reconstruction
The EXE's WinMain is approximately:
```cpp
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    SetDllDirectoryW(L"");
    int result = MovieMakerMain();
    return result;
}
```
CRT startup, exception handling, delay-load init all handled by MSVC 2012 linker infrastructure.

### Relevance for Decompilation
- MovieMaker.exe requires only a **minimal wrapper** implementation
- The real decompilation target is **MovieMakerCore.dll** (all application logic)
- WLXPhotoBase.dll provides shared base types and memory management
- The delay-load pattern means MovieMakerCore.dll can be unloaded independently

## WLXPhotoCinematic.dll Analysis

### Overview
COM DLL (88 KB image, ~58 KB code) implementing cinematic/Ken Burns pan & zoom photo effects for slideshow. PE32 x86, linker v11.0, image base 0x10000000, ASLR+DEP. Timestamp: 2014-04-01. PDB: `WLXPhotoCinematic.pdb` GUID `{0DB69AAE-1EE5-4822-95E2-F4B6520E7091}`. Build: 16.4.3528.0331 (ship).

### Sections
- `.text` (0x1000, 58 KB code), `.data` (0x10000, 4 KB globals), `.rsrc` (0x12000, 6.4 KB resources), `.reloc` (0x14000, 5.6 KB relocations)

### Exports (4 COM standard)
1. `DllCanUnloadNow` (0x328A)
2. `DllGetClassObject` (0x327A)
3. `DllRegisterServer` (0x32AA)
4. `DllUnregisterServer` (0x329B)

### CLSIDs
- `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}` — `Microsoft.Photos.Slideshow.CinematicFullScreen1.1`, FriendlyName: "Cinematic - Full Screen", Apartment threaded
- `{5409AB48-D8D3-40e6-A1EB-23489DC422DE}` — `Microsoft.Photos.Slideshow.CinematicTransform.1`, FriendlyName: "Cinematic Theme Transform", Apartment threaded

### Imports
- **d3dx9_32.dll**: `D3DXMatrixLookAtLH`, `D3DXMatrixPerspectiveFovLH` — Direct3D 9 camera matrix math
- **WLXPhotoBase.dll** (6): Base::New/Delete/Throw, Exception, BaseAtlThrow, GetBaseStringManager
- **MSVCR110.dll** (32): CRT (malloc/free/string/math/time), C++ exception handling
- **KERNEL32.dll** (34): Module loading, resources, sync, system time, Interlocked ops
- **ADVAPI32.dll** (14): Registry (Reg*), ETW (RegisterTraceGuidsW/TraceEvent)
- **ole32.dll** (7): CoTaskMem, CoCreateInstance, StringFromCLSID, PropVariantClear
- **OLEAUT32.dll** (10 ordinals): Automation/VARIANT support
- **SHLWAPI.dll** (2): PathRemoveFileSpecW, StrCmpW
- **USER32.dll** (2): DestroyWindow, CharNextW

### Embedded XML: PanZoomTheme
The DLL embeds an XML resource defining Ken Burns effect parameters per aspect ratio:
- **Portrait** (tall): Zoom=0.2, length 8-18 frames
- **Landscape** (wide): Zoom=0.1, length 6-15 frames
- **Panorama** (extra-wide): Zoom=0.2, length 20-50 frames
- **XPanorama** (extreme-wide): Zoom=0.2, length 40-90 frames
- All classes: PanZoom=0.05 (constant), three speeds (Slow/Medium/Fast)

Also embeds `TransitionsAndEffects Version="2.8"` XML referencing transition DLL GUIDs `{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}` and `{7371ADEE-C195-427F-B0EC-3CCC13725665}` with transition named "Cinematic" and "Classic".

### Key Observations
- Two COM objects: FullScreen (renderer) + Transform (matrix processor)
- Ken Burns effect = virtual D3D9 camera moving over a textured quad
- Aspect-ratio-aware with XML-defined parameters
- Registers under `Windows Photo Gallery\Slideshow\Themes` — it's a slideshow theme plug-in
- No Media Foundation imports — self-contained D3D9 rendering
- No application-specific named exports — pure COM dispatch

## MovieMakerCore.dll Analysis

### Overview
Monolithic DLL containing the entire Windows Live Movie Maker application logic. PE32 x86, linker v11.0, image base 0x10000000, ASLR+DEP. Timestamp: 2014-04-01 01:26:04 UTC. Build: 16.4.3528.0331_ship.client.main.w5m4. PDB: {D5217874-B614-477C-B45B-E0CE638C6496}. Total image size: 10,626,048 bytes (~10.1 MB).

### Section Layout
| Section | VA | VirtSize | RawSize | Purpose |
|---------|-------|----------|---------|---------|
| `.text` | 0x1000 | 5,719,636 | 5,719,808 | Code (5.46 MB!) |
| `.data` | 0x576000 | 234,140 | 188,928 | Globals, vtables, RTTI |
| `.rsrc` | 0x5B0000 | 4,240,504 | 4,240,896 | Resources (4.07 MB!) |
| `.reloc` | 0x9BC000 | 415,550 | 416,256 | ASLR relocations |

### Exports (1 function)
| Ordinal | Name | Convention | Signature |
|---------|------|-----------|-----------|
| 1 | `MovieMakerMain` | `__cdecl` | `int __cdecl MovieMakerMain(int argc, wchar_t** argv)` |

This is the ONLY export. All other functionality is accessed via COM vtables and internal class dispatch.

### Import Summary (~630+ functions from 31 DLLs)
**System DLLs:** KERNEL32 (141), USER32 (105), GDI32 (18), ADVAPI32 (25), SHELL32 (19), SHLWAPI (24), ole32 (23), OLEAUT32 (22 ordinals)
**Windows Live DLLs:** UXCore.dll (180+ DirectUI classes), WLXPhotoBase.dll (14), WLXPhotoSqm.dll (13), MetadataSys.dll (1), DmxBici.dll (5), wlidcli.dll (7), uxctl.dll (3)
**Media DLLs:** MF.dll (2), MFPlat.DLL (18)
**Graphics DLLs:** gdiplus.dll (24), d3d11.dll (1), d3d9.dll (2), d2d1.dll (1 ordinal), DWrite.dll (1), dxva2.dll (2), dwmapi.dll (1), UxTheme.dll (3), D3DCOMPILER_46.dll (2), WindowsCodecs.dll (1)
**Other:** VERSION.dll (3), WINMM.dll (1), XmlLite.dll (4), PROPSYS.dll (8), OLEACC.dll (2), ESENT.dll (28 - Jet database)

### COM GUIDs in Binary
| GUID | Purpose |
|------|---------|
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | App/module identifier (near `Microsoft\Live\MovieMaker`, `APPID`, `REGISTRY`) |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Telemetry/error reporting endpoint ID (near `//ItemSet/Item/ErrorCode`, `http://g.live.com`) |
| `{9590CDCE-BFFC-45d5-85EA-493C65FB2C4F}` | Transition/effect template GUID (near "Wheel", "Circles", "Heart" effect names) |

Certificate GUIDs (not app-related): `{4faf0b71-ad37-4aa3-a671-76bc052344ad}`, `{2860b52e-c4a3-454d-bc1e-32c5add17e90}`

### RTTI Classes (125 unique C++ classes)
**Application framework (5):** SundanceAppMain, CommandLineParser, AutoSaveManager, MediaBrowser, TemplateTable
**Timeline (15):** TimelineBaseBehavior, TimelineBehavior, TimelineDataSource, TimelineDragDrop, TimelineItemBehavior, TimelineItemInputBehavior, TimelineInstructionsBehavior, TimelineSelectionRootBehavior, TimelineVisualTrackItemBehavior, TimelineSecondaryTrackItemBehavior, TimelineTemplateSource, TimelineLayoutMode, TimelineExtentUIObject
**Text/Title (6):** TextBoxBehavior, LegacyTextExtent, LegacyParagraph, CFontSite, CColorPickerSite, CaretBehavior, CaretScrollBehavior, RichEditControlBehavior
**Media/Encoding (12):** AudioCaptureSession, AVCaptureCore, AVCaptureSession, WebcamElementBehavior, WebcamUI, NarrationUI, TrimBehavior, UserEncodeProfileBehavior/Dialog/Info/Recommended/WLVS, ChunkMediaFileList, CommandLineMediaFileList
**Publishing (8):** PublishManager, PublishJob, PublishBackgroundJob, PublishBackgroundWorker, PublishSummaryDialog, PublishProgressCallBack, PublishItemProperties, PublishItemPropertyStore
**UI Behaviors (40+):** SundanceMainElementBehavior, AboveHomerBehavior, HomerHeavyLayerBehavior, StandardLayerBehavior, DuiLayerBehaviorImpl, InlinePreviewLayoutBehavior, InlinePreviewSliderBehavior, FullscreenLayoutBehavior, ProjectWorkspaceLayoutBehavior, ResizeablePaneBehavior, AMPCommandBarBehavior, AMPMainWindowBehavior, MultipleEffectBehavior/Dialog, OptionsDialogBehavior, HelpBehavior, CaptureUIBehavior, PopUpSlider, ProgressBase/Dialog/StatusBar
**Dialog/Host (12):** CDUIDialog, CFramelessHost, CSundanceDialog, CMsgFilter, SundanceNativeHwndHost, DontShowPromptDialog, SundanceApplicationOptionsDialog
**Data/Model (20):** SundanceAppDataContext, PreviewDataContext, ComplexProperty, SingleProperty, TransformProperty, FilenameList, FilenameArrayList, LegacyExtent, LegacyTransform, LegacyProjectSupport, CachedWFSection, SFTime, SelectionRootImpl/Wrapper, SelectionIndex, ContactStore
**Sites (10):** CMRUSite, CSpinnerSite, CGenericSite, CGroupSite, CGallerySite, ToggleSite, CFontSite, CColorPickerSite, SundanceBehaviorFactory
**Ribbon (5):** RibbonApp, RibbonCategoryItem, RibbonCategoryList, RibbonList, RibbonListItem

### Registry Paths
- `Software\Microsoft\Windows Live\Movie Maker` (+ `\Post`, `\Suppressed`, `\Recent`, `\RecentWLVS`)
- `Software\Microsoft\Windows Live\Photo Gallery` (+ `\FileSupport\*`)
- `Software\Microsoft\Windows Live\Installer` (ShortCatalogTimeStamp, ProductStatus)
- `Software\Microsoft\Windows Live\Common` (SuiteLanguage, InstalledLanguages)
- `Software\Microsoft\Homer` (Tracing)
- `Software\Microsoft\Windows Media Foundation\ByteStreamHandlers`
- `Software\RegisteredApplications`

### Key File Formats
- `.wlmp` — Movie Maker project file (auto-save: `AutoSave.wlmp`, `AutoSaveValidate.wlmp`)
- `.wlvs` — Video profile format
- `%LOCALAPPDATA%\Microsoft\Windows Live Movie Maker` — Local data path

### XML Serialization Paths
- `//MovieMaker/Project/DataStr` — Project data structure
- `//TiEffectArr[@UID="%s"]/UID` — Effect array
- `//TiEffect[@UID="%s"]/TiEffectPtr` — Effect pointer
- `//TiTransition[@UID="%s"]/TTFrom` — Transition from
- `//TiTransition[@UID="%s"]/TiTransitionPtr` — Transition pointer
- `//Track[@TrackTyp="0"]` — Track type
- `//ItemSet/Item/ErrorCode` — Error reporting

### Window Classes
- `WindowsLiveMovieMakerMainWindowClass` — Main window
- `WindowLiveMovieMakerFwdCmdMapping` — Command forwarding

### Clipboard Formats
- `WLPGMovieMaker_SelectedSet_16.4.3528.0331`
- `WLPGMovieMaker_SerializedProject_16.4.3528.0331`

### Telemetry Endpoints
- `http://g.live.com` (production)
- `http://g.live-int.com` (internal/test)
- Error URL template: `%s?version=%u.%u.%04u.%04u&ErrorCode=%x`

### Version Info
- **FileVersion:** 16.4.3528.0331_ship.client.main.w5m4 (ship)
- **InternalName:** MovieMakerCore
- **LegalCopyright:** (c) 2012 Microsoft Corporation. All rights reserved.
- **ProductName:** Movie Maker
- **ProductVersion:** 16.4.3528.0331

### Key Corrections to Reconstructed Source
1. **Single export confirmed:** The original DLL exports only `MovieMakerMain`. The reconstructed `exports.h` defines 4 export macro families (`MOVIECORE_API`, `RIBBON_API`, `STORYBOARD_API`, `DATASTRUCT_API`) that are never used for actual exports.
2. **No externally-visible COM factories:** The reconstructed `ComFactory.h` creates 4 ATL COM wrappers with placeholder GUIDs. The original binary registers COM objects internally via ATL OBJECT_MAP and `_Module.RegisterClassObjects()` — no explicit COM factory exports.
3. **Placeholder GUIDs:** The reconstructed CLSIDs (`{A1B2C3D4-...}`, `{B2C3D4E5-...}`, etc.) are clearly dummy values not present in the original binary.
4. **125 RTTI classes exist** in the binary but are NOT exposed via COM registration in the export table — they're internal implementation classes accessed through vtable dispatch.
5. **Missing: ESENT (Jet) database usage** — The original binary imports 28 Jet functions for database operations, which the reconstructed source does not account for.
6. **Missing: BICI/SQM telemetry** — Three separate telemetry systems (WLXPhotoSqm, DmxBici, custom HTTP error reporting) are present in the binary.
7. **Missing: Extensive XML serialization** — The binary contains XPath-style query templates for project serialization not represented in the reconstructed source.
8. **Missing: wlidcli.dll (Windows Live ID)** — Authentication/identity integration not present in reconstructed source.

## WLXMovieLibrary.dll Analysis

### Overview
Movie Library management DLL (316 KB image, ~281 KB code). PE32 x86, linker v11.0, ASLR+DEP. Timestamp: 2014-04-01 01:26:24 UTC. PDB: `WLXMovieLibrary.pdb` `{CBF27DEC-DD3B-4A76-A5DF-9EECCC486627}`. Manages the "Photo Gallery Movie Library" — media collection for Movie Maker.

### Exports (1 function)
| Name | RVA | Convention |
|------|-----|------------|
| `CreateMovieFactory` | 0xADD8 | `__stdcall` |

Creates `IMovieFactory` interface. All other access via COM vtables.

### ATL COM Architecture
Standard ATL COM DLL with `CAtlDllModuleT<CMovieDllModule>`. Internal classes:
- **Factory**: `MovieFactory` (implements `IMovieFactory`)
- **Model**: `Movie` (`IMovie`), `MovieBuilder` (`IMovieBuilder`), `SystemClock`
- **AV Source (HMRAVSource)**: `AVSource`, `AVSourceFactory`, `AVSourceProxy`, `AVSink`, `StreamSinkHelper`, `MFRateControlHelper`, `XVideoProc`, `DXVA2VideoProc`, `SyncVideoSampleSource`, `TextureInterOpDX9/DX11`, `MFByteStreamOnStream`, `AsyncSourceResolver`, `DShowMFSourceReaderBuilder`, `NativeMFSourceReaderBuilder`, `AuthProvider`, `AuthCredentials`
- **Audio**: `WaveAudioRenderer`, `WaveDevice`, `AudioFormat`, `PcmFormat`
- **Transcode**: `TranscodeMetadataParser`

### Dependencies
- **MSVCR110.dll** (35): CRT core (memory, string, exceptions, init)
- **KERNEL32.dll** (63): Thread pool (Win7+), sync, modules, process, resources
- **ADVAPI32.dll** (16): Crypto (`Crypt*` — DRM), registry, ETW tracing
- **ole32.dll** (6): COM memory, `CoCreateInstance`, `CLSIDFromString`
- **MFPlat.DLL** (13): MF startup/shutdown, media types, source resolver, callbacks
- **WINMM.dll** (11): WaveOut audio playback
- **d3d9.dll** / **d3d11.dll** / **dxva2.dll**: DirectX video acceleration
- **PROPSYS.dll** (1): `PSCreateMemoryPropertyStore`
- **MF.dll** (1): `MFGetService`
- **WLXPhotoBase.dll** (10, delay): Memory, exception, string, version checks
- **SHLWAPI.dll** (8, delay): Path manipulation, file existence
- **SHELL32.dll** (1, delay): `SHGetPropertyStoreFromParsingName`

### Registry (No SQL Database)
The "library" is **registry-based**, not SQL. Key path:
- `SOFTWARE\Microsoft\Windows Live\Common\Movie Library`
- Also: `SOFTWARE\Microsoft\Windows Live\Movie Maker`

No SQL, ESENT, or Jet imports. No SQL strings. Library state persisted via registry and `ExclusionList.xml`.

### Key Strings
- `Photo Gallery Movie Library`, `Photo Gallery`
- `ExclusionList.xml` — file exclusion list
- `WLAVRes.dll` — resource DLL
- Extensive `WM/*` metadata properties (~50+ Windows Media namespace attributes)
- All `MF_E_*`, `MF_MT_*`, `MFAudioFormat_*`, `MFVideoFormat_*` constants
- `.wtv` — Windows Recorded TV format support
- `AVS_E_DECODER_FAILURE`, `AVS_E_INVALID_DURATION_FILE`, `AVS_E_NON_SEEKABLE_FILE` — custom HRESULTs

### No User-facing COM GUIDs
No CLSID/IID string literals found. These are defined in the ATL module's `.rgs` (registry script) files or registered at runtime. Two binary GUIDs found are code-signing certificate identifiers.

### Key Architecture Points
1. **No database engine** — uses registry + file metadata + XML for exclusion list
2. **ATL COM DLL** — standard pattern with one factory export
3. **Hardware accelerated** — DXVA2 with D3D9 and D3D11 paths
4. **DRM support** — CryptoAPI for content protection validation
5. **Media Foundation** — core playback pipeline uses MF platform
6. **WaveOut audio** — legacy WINMM audio (not WASAPI)
7. **ETW tracing** — full instrumentation via ADVAPI32 trace APIs

## WLXFaceRecognition.dll Analysis

### Overview
Face detection and recognition COM DLL from Windows Live Photo Gallery / Movie Maker 2012. PE32 x86, linker v11.0 (MSVC 2012), image base 0x10000000, ASLR+NX. Timestamp: 2014-04-01. PDB: `{2FEBA5E0-6376-46CD-9ADA-4A79335C37BD}`. Total image: ~4.5 MB. Origin: **Microsoft Research Asia (MSRA)** — registry path `Software\Microsoft\MSRA\FaceRecognition\`.

### Key Stats
- **4 exports**: `DllCanUnloadNow`, `DllGetClassObject`, `DllRegisterServer`, `DllUnregisterServer` (standard COM DLL)
- **7 COM objects**: `VFaceDetection`, `VFaceRecognitionPipeline`, `VFaceRegion`, `VFaceRegionSet`, `VFaceRepImpl`, `VImageData`, `VImageManager`
- **4.4 MB resources**: Cascade classifier data, neural network weights, trained models in `.rsrc`
- **6 dependencies**: MSVCR110, MSVCP110, KERNEL32, USER32, ADVAPI32, ole32, OLEAUT32, WindowsCodecs, WLXPhotoBase, WLXPhotoSqm

### Face Detection Pipeline
1. **Detection** — `AVFaceDetectionSoftCascade`, `AVPoly2Classifier`, `AVCNeuralNet`; 11 pose categories (frontal, half-profile, profile × ±30° rotations)
2. **Parts Localization** — `AVFacePartsDetectionNeuralNet` (eye/nose/mouth via neural net)
3. **Geometry Rectification** — `AVFaceGeomRectEyes`, `AVFacePhotoRectLBP/RawPCA`
4. **Feature Extraction** — LBP, PCA, Texton, RPTexton, RPTextonPCA
5. **Distance Matching** — LBP distance, PCA distance, RPTextonPCA distance
6. **Grouping** — `AVFaceGroupRankOrder` (rank-order clustering)
7. **Recognition** — Match thresholds: Default/Loose/Max/Moderate/Strict

### Cascade Model Files (embedded resources)
- `DetectNeuralNet.bin` — neural network model
- `background.bf` — background/boosting model
- `FaceRecognition.rcv` — RCV classifier model

### Key COM Identifiers
- `{EF401225-1260-4716-A842-7D180DC14C1E}` — CLSID_FaceRecognitionPipeline (confirmed in RGS)
- `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` — likely CLSID_FaceDetection
- `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` — likely CLSID_ImageManager
- `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` — likely CLSID_FaceRegion or FaceRegionSet

### SQM Telemetry
Tracks face detection usage via `WLXPhotoSqm.dll` counters/timers.

### C++ Internal Classes (~40+)
Top-level: `AVCFaceDetector`, `AVCFaceRecognizor`, `AVCFaceRepresentor`. Pipeline: `AVFaceRecognitionPipeline`. Feature extractors: `AVFaceFeatureExtractorLBP/RawPCA/Texton/RPTexton/RPTextonPCA`. Distance: `AVFaceDistanceLBP/RawPCA/RPTextonPCA`. Geometry: `AVFaceGeomRectEyes/RPTexton`, `AVFacePhotoRectLBP/RawPCA`. Parts: `AVFacePartsDetectionNeuralNet`. Grouping: `AVFaceGroupRankOrder`. Matching: `AVFaceMatchLookupIteration/LBP`. Classifiers: `AVClassifier`, `AVClassifierBase`, `AVPoly2Classifier`, `AVCNeuralNet`, `AVFaceDetectionSoftCascade`.

## WLXVideoTrim.dll Analysis

### Summary
Video trimming processor DLL (568 KB). 5 exported factory functions for creating copiers, transcoders, and a video player. DirectShow-based: all trimming uses filter graphs. Delay-loads WLXMP4Parser.dll for MP4/MOV support. No UI in this DLL — pure processing engine. See full analysis at `analysis/WLXVideoTrim/analysis.md`.

| Field | Value |
|-------|-------|
| **Image** | 568 KB (0x8B000) |
| **Exports** | 5 (CreateAVICopierDirect, CreateVideoCopierFromMediaType, CreateVideoFormatContextTranscoder, CreateVideoPlayer, CreateVideoWMVTranscoder) |
| **Dependencies** | MSVCR110, KERNEL32, USER32, ADVAPI32, ole32, OLEAUT32, WLXPhotoBase, SHELL32, SHLWAPI, WMVCore; delay: WLXMP4Parser |
| **Key CLSID** | `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` |
| **Build** | `16.4.3528.0331_ship.client.main.w5m4` |
| **PDB** | `{73CFF58F-A97D-4232-883C-397BD1DF5009}` |
| **Source path** | `e:\bt\1105173\client\personalmedia\videotrim\dev\` |

### Copier/Transcoder Architecture
- **Copiers** (trim by re-mux): VideoCopier → AsfCopier, AVICopier, Mpg2Copier, StreamBufferCopier
- **Transcoders** (trim by re-encode): VideoTranscoder → AsfTranscoder, AVITranscoder, DVTranscoder, Mpg2Transcoder, StreamBufferTranscoder
- **Player**: VideoPlayer (EVR/VMR7/VMR9, thumbnail capture, rotation)
- **Custom filters**: AudioRepackageFilter, CopyProgressInfoFilter, VideoRotationFilter, YUY2ToI420Filter

### Supported Formats
WMV/ASF, AVI, DV-AVI, MPEG-2/DVR-MS (.dvr-ms/.sbe), MP4/MOV/3GP (via WLXMP4Parser), MJPG

## WLXSlideshow.dll Analysis

### Overview
Slideshow generation and playback control DLL (~520 KB image, ~255 KB code). PE32 x86, linker v11.0, image base 0x10000000, ASLR+NX. Timestamp: 2014-04-01 01:14:01 (exports), 2014-04-01 01:26:23 (PE header). PDB: `{6547A44F-AABE-4CFA-9675-0113F4BD19E9}`. Build: 16.4.3528.0331_ship.client.main.w5m4.

### Exports (4 COM standard)
| Ordinal | RVA | Name |
|---------|-----|------|
| 1 | 0x0000B52B | `DllCanUnloadNow` |
| 2 | 0x0000B51B | `DllGetClassObject` |
| 3 | 0x0000B54B | `DllRegisterServer` |
| 4 | 0x0000B53C | `DllUnregisterServer` |

### Sections
| Section | VA | VirtSize | RawSize | Purpose |
|---------|-------|----------|---------|---------|
| `.text` | 0x1000 | 0x3EE89 | 0x3F000 | Code (255 KB) |
| `.data` | 0x40000 | 0x414C | 0x3800 | R/W globals, vtables |
| `.rsrc` | 0x45000 | 0x32AD0 | 0x32C00 | Resources (203 KB — themes, strings, icons) |
| `.reloc` | 0x78000 | 0x6654 | 0x6800 | ASLR relocations |

### Dependencies (18 DLLs, ~240+ functions)
**System:**
- **MSVCR110.dll** (48): CRT core — memory, strings, format, exceptions, threading, RTTI
- **KERNEL32.dll** (60): Process/thread/memory, sync, modules, registry-like, time, PE
- **USER32.dll** (46): Windows/messages/menus/drawing/input/monitors/accessibility
- **GDI32.dll** (13): BitBlt, DIB, fonts, regions, DC management
- **ADVAPI32.dll** (18): Registry (15), ETW tracing (5)
- **SHELL32.dll** (3): ShellExecute, SHCreateItemFromParsingName
- **SHLWAPI.dll** (12): Registry helpers (USKey), paths, strings
- **ole32.dll** (12): COM core, memory, CLSID/string conversion
- **OLEAUT32.dll** (11 ordinals): Variant, BSTR, SafeArray management
- **WTSAPI32.dll** (2): Session notifications
- **VERSION.dll** (3): File version info
- **PSAPI.DLL** (1): Process memory info
- **OLEACC.dll** (2): UI Automation accessibility
- **msi.dll** (1 ordinal): Windows Installer
- **dwmapi.dll** (delay-load): DWM window attribute

**Graphics:**
- **gdiplus.dll** (25): Bitmap loading/manipulation, drawing, encoding
- **d3d9.dll** (1): Direct3DCreate9
- **d3dx9_32.dll** (3): D3DXCreateFontW, D3DXCreateSprite, D3DXMatrixScaling

**Windows Live:**
- **UXCore.dll** (11): DirectUI resource strings, icons, accelerators
- **WLXPhotoBase.dll** (13, delay): Exception handling, memory, string, version
- **WLXPhotoSqm.dll** (4, delay): SQM telemetry startup/shutdown/add
- **DmxBici.dll** (2): BICI telemetry — AddStringToDataPoint, TransferExperienceToWeb

### COM Objects (registered)
Three COM classes registered via ATL OBJECT_MAP:
- `CLSID_SimpleSlideshowDisplay` — slideshow rendering surface
- `CLSID_SlideshowExtension` — slideshow extension point
- `CLSID_TimelineDisplay` — timeline display widget

COM GUID found in binary: `{84FBA192-4F8D-4a5d-94DE-083446ACC1D0}`

### COM Interfaces (from RTTI, 38+)
- `ISlideshowControl` — playback control
- `ISlideshowDisplay` — rendering surface
- `ISlideshowList` / `ISlideshowListIterator` / `ISlideshowListIteratorEx` — slide list
- `ISlideshowChunkIterator` / `ISlideshowFilteredListIterator` — chunked iteration
- `ISlideshowThemeManager` / `ISlideshowThemeIterator` / `ISlideshowThemeEventSink` — themes
- `ISlideshowUserInterface` / `ISlideshowFrameController` — UI integration
- `IModeControlSlideshowSettings` / `IUserInterfaceSlideshowSettings` — settings
- `IEaselSlideshowFrame` — "Easel" canvas display concept
- `IDisplayControl` / `IDisplayControlEventSink` — display control events
- `ICanvasView` / `IPaintContext` / `IPrintContext` — painting pipeline

### RTTI Classes (156 total)
- **Core slideshow objects (16):** SimpleSlideshowDisplay, SlideshowExtension, SlideshowFrameWindow, SlideshowChunkIterator, SlideshowList, SlideshowListIterator, SlideshowModeControl, SlideshowUserInterface, ThemeManager, ThemeIterator, TimelineDisplay, TimelineCanvasView, TimelineNavigationBar, PaintContextHWnd, PrintContextHWnd, SupportedFileTypes
- **Graphics (8):** BufferedPainter, BufferedPainterBase, SharedBitmapWrapper, ThumbnailCacheWrapper, ThumbCachePopulater, GdiplusStartupWrapper, SqmStartupWrapper, Thread@Base
- **Timeline navigation (9):** TimelineNavigationButton/Menu/MenuItemButton/Static/StretchButton/BitmapOnlyButton/ControlBase, TimelineErrorHandler, TimelineFileHandler, TimelineFileLoader, TimelineBuilder
- **ATL/WTL infrastructure (20+):** CComModule, CAtlModuleT, CFrameWindowImpl, CWindowImpl variants, CComObject variants, tear-off objects, accessibility proxies

### Built-in Themes/Effects (9 names, from resource string)
```
Loading...  Fade  Black and white  Sepia  Classic  Album  Collage  Spin  Frame  Glass
```

These are the selectable slideshow visual styles. "Loading..." is a placeholder/loading state. The other 8 are distinct rendering themes.

### Configuration & Registry
**Slideshow-specific keys** under `Software\Microsoft\Windows Live\Photo Gallery`:
- `\Slideshow` — main settings
- `\Slideshow\LastRunSettings` — persist last playback settings
- `PhodeosPerChunk`, `PlaybackFramesPerSecond` — playback tuning
- `UseVSync`, `RunInDXEMode`, `DXExclusiveSize` — DirectX config
- `MaxRenderSizeWidth/Height`, `DesktopResolutionWidth/Height` — rendering resolution
- `VertexShaderMajorVersion/MinorVersion`, `PixelShaderMajorVersion/MinorVersion` — GPU caps
- `SoftwareRenderingEnabled` — GDI fallback toggle
- `ActiveThemeFriendlyName`, `CurrentThemeSimpleSlideshowNone`, `EnabledThemeSet`, `EnabledThemeName%d` — theme tracking

### UI Strings (localized via UXCore)
- Window title: `Photo Gallery Slide Show`
- Controls: `&Play`, `P&ause`, `&Next`, `&Back`, `&Exit`, `S&huffle`, `&Loop`
- Speeds: `Slide Show speed - &Slow / &Medium / &Fast`

### Graphics Architecture (Hybrid D3D9 + GDI+)
1. **Primary: Direct3D 9** — Hardware-accelerated via d3d9.dll + d3dx9_32.dll (sprite + font rendering)
2. **Fallback: GDI+** — Software rendering via gdiplus.dll (25 functions) for bitmap loading, manipulation, encoding
3. **GDI** — Legacy GDI operations (BitBlt, DIB sections, regions)
4. **DWM** — Desktop Window Manager integration (glass effects via DwmSetWindowAttribute)
5. **GPU caps detected** via registry-configured shader versions, resolution caps

### Function Categories
1. **COM registration** — DllCanUnloadNow/GetClassObject/RegisterServer/UnregisterServer
2. **Playback control** — Play/Pause/Next/Back, speed settings, shuffle/loop
3. **Rendering** — D3D9 sprite composition, GDI+ bitmap processing, buffered painter
4. **Themes** — 9 built-in visual styles with theme manager/iterator
5. **Slide list management** — Chunked list iteration for parallel processing
6. **Registry persistence** — Last-run settings, GPU pipeline config, theme preference
7. **DirectUI integration** — UXCore for localized resource strings
8. **Telemetry** — SQM (counters) + BICI (data points) + ETW tracing
9. **Thumbnail cache** — Shared bitmap cache with async population
10. **Accessibility** — UI Automation support via OLEACC

## WLXMediaPublishSubscribe.dll Analysis

### Overview
Publish/subscribe framework DLL (~1.4 MB image, 568 KB code). PE32 x86, linker v11.0, image base 0x10000000, ASLR+DEP. Timestamp: 2014-04-01. PDB: `{17F284FA-930A-4DA2-9649-93B296009330}`. Build: 16.4.3528.0331_ship.client.main.w5m4. See full analysis at `analysis/WLXMediaPublishSubscribe/analysis.md`.

### Exports (22 functions)
| Category | Count | Details |
|----------|-------|---------|
| COM Infrastructure | 4 | DllCanUnloadNow/GetClassObject/RegisterServer/UnregisterServer |
| MediaPublishSubscribeHelper | 9 | ctor/dtor, Initialize (GUID), PublishItems, PluginPublishItems, CreateContainerAndPublishItems, GetLiveSignInProvider, GetMediaPublishSubscribeProviderManager, LaunchAuthBrowser (2 overloads) |
| VideoTranscoder | 6 | ctor/dtor, Initialize (I/O paths+flags), GetInputFileAverageBandwidth/Duration/DisplaySize, Transcode (progress callback) |
| Utility | 1 | IsHResultDRMProtected (static) |

### Dependencies (20 DLLs, ~400+ functions)
- **MSVCR110.dll** (50+), **KERNEL32.dll** (100+), **USER32.dll** (70+), **GDI32.dll** (20+), **ADVAPI32.dll** (20+)
- **SHELL32.dll** (7), **SHLWAPI.dll** (22+), **ole32.dll** (15+), **OLEAUT32.dll** (24+)
- **WINHTTP.dll** (16) — HTTP upload/API calls with proxy detection
- **WININET.dll** (5) — URL cache (thumbnail caching)
- **CRYPT32.dll** (2) — DPAPI credential/token protection
- **PROPSYS.dll** (3) — Windows property system
- **gdiplus.dll** (40+), **UxTheme.dll** (5), **UXCore.dll** (12)
- **OLEACC.dll** (2), **VERSION.dll** (3)

**Windows Live:**
- **WLXPhotoSqm.dll** (11) — SQM telemetry counters/timers
- **DmxBici.dll** (7) — BICI telemetry A/B testing
- **wlidcli.dll** (18+ ordinals) — Windows Live ID authentication

**Delay-load:**
- **WLXPhotoBase.dll** (10) — exception, memory, string, GDI+
- **MetadataSys.dll** (1) — metadata property handler
- **msi.dll** (2) — Windows Installer

### COM Objects & Interfaces
**CLSIDs referenced:** CLSID_MediaPluginManager, CLSID_MediaPluginDecorator, CLSID_FlickrProvider, CLSID_LiveProvider, CLSID_MediaPublishSubscribeProviderManager, CLSID_MetadataManager, CLSID_LivePlugin(T)

**Key interfaces:** IMediaPublishSubscribeProviderManager, IMediaPublishSubscribeItemSet, ILiveSignInProvider, ILiveProvider/ILiveProviderPrivate, IPublishSubscribeProvider, IPubSubProviderManager, IGrinderTaskScheduler, IGrinderJobEventSink, IVideoTranscodeProgressCB, IServiceProvider

### Provider Ecosystem
- **FlickrProvider** — Flickr photo publishing
- **LiveProvider** — Windows Live / SkyDrive (OneDrive) publishing
- **LivePlugin / LivePluginT** — Templatized plugin pattern
- **PublishTaskProvider** — Task-based publishing workers
- **DatabasePublishItemProperties / DatabasePublishProvider** — Local DB-backed publish settings

### Architecture
1. **Plugin architecture** via MediaPluginManager/Decorator pattern; providers implement IPublishSubscribeProvider
2. **Background jobs** via "Grinder" task scheduler (IGrinderTaskScheduler + IGrinderJobEventSink)
3. **HTTP upload** via WinHTTP (16 functions incl. proxy detection via WinHttpGetIEProxyConfigForCurrentUser)
4. **Authentication** via wlidcli.dll (Windows Live ID) with browser-based OAuth flow (LaunchAuthBrowser with SiteIDType enum)
5. **Video transcoding** pre-publish (VideoTranscoder with bandwidth/duration/size analysis)
6. **DRM detection** (IsHResultDRMProtected)
7. **Dual telemetry**: SQM (WLXPhotoSqm) + BICI (DmxBici)
8. **Credential protection** via DPAPI (CryptProtectData/CryptUnprotectData)

### Key Strings
- `WLXMediaPublishSubscribe`, `MVTranscoder`
- `FlickrProvider`, `FlickrMoniker`, `FlickrMonikerAttachment`
- `LiveProvider`, `LiveProviderPrivate`, `LivePlugin`
- `MediaPublishSubscribeProvider`, `PublishTaskProvider`
- `OnlineMediaPlugin`, `MediaPluginInlineManager`
- `DatabasePublishItemProperties`, `DatabasePublishProvider`, `BasePublishProperties`

### SQM Telemetry Counters
Tracks publishing operations via 11 SQM functions: Start, Increment, AddToStream (4 overloads), AddToStreamTimer (2 overloads), IsEnabled, Set, Shutdown. BICI tracks A/B experimentation data.

## MovieMakerLang.dll Analysis

### Overview
**Pure resource-only satellite DLL** (216 KB) containing all English (US) localization resources for Windows Live Movie Maker 2012. Single `.rsrc` section only — no code, no exports, no imports, no entry point.

### PE Headers
- **Machine**: x86 (0x14C), **Subsystem**: Windows GUI (2)
- **Entry Point**: 0x0000 (none)
- **Image Base**: 0x10000000, **Image Size**: 0x36000 (216 KB)
- **Linker**: MSVC 11.00, **OS Ver**: 6.02 (Win8)
- **Timestamp**: 2014-04-01 01:28:09 UTC
- **DLL Characteristics**: Dynamic Base (ASLR), NX Compatible
- **Digital signature**: Present (Certificates Directory at RVA 0x35200, 0x3EC8 bytes)

### Section Layout
| Section | VA | VirtSize | RawSize | Flags |
|---------|-----|----------|---------|-------|
| `.rsrc` | 0x1000 | 0x34F28 | 0x35000 | Read-only initialized data |

### Exports
**None** — export directory empty (RVA=0, size=0).

### Imports
**None** — import directory empty (RVA=0, size=0). No dependencies.

### COM GUIDS Found
- `{1D31145D-AEFE-48B0-B48A-513E5D413B44}` — plugin/service identifier
- `{66557ED9-C5F8-4815-A8CC-D157272CFFCE}` — plugin/service identifier
- `{8BE133F2-9F23-4344-B5A1-A49BDD09FC0F}` — publish error context
- `{0DDA997F-E7FA-404B-B3D3-F1610807FB66}` — binary GUID
- `{1812A500-BA1E-41EE-B3A5-5D7B6FCA7393}` — binary GUID

### Version Info
- **InternalName**: MovieMakerLang, **OriginalFilename**: MovieMakerLang.DLL
- **ProductName**: Movie Maker, **FileVersion**: 16.4.3528.0331 (ship)
- **LegalCopyright**: (c) 2012 Microsoft Corporation. All rights reserved.
- **Language**: 0x0409 (en-US)

### Resource Types
Contains RT_STRING (UI strings, ~600+ unique strings), RT_DIALOG, RT_MENU, RT_ACCELERATOR, RT_VERSION, RT_GROUP_ICON/RT_ICON, custom UIFILE (DirectUI XML), and embedded PNG images (created with Adobe Photoshop CS5).

### Key Content
- **DirectUI UIFILE** resources for the SUNDANCE ribbon and chrome (loaded by UXCore.dll)
- **All UI strings**: menu items, toolbar tooltips, dialog buttons, error messages (~200+ error strings covering codecs, DRM, permissions, project corruption, webcam, publishing)
- **Transition/effect names**: 100+ named transitions (Cinematic, Contemporary, Fade, Dissolve, Wipe, Slide, Zoom, Circle, Diamond, Heart, Star, Wheel, Page Curl, etc.) and pan/zoom directions
- **Publishing targets**: OneDrive, Facebook, YouTube, Flickr, Vimeo, Blip.TV, MySpace, DailyMotion
- **Device profiles**: Windows Phone, Apple iPhone/iPad, Android Phone/Tablet, Zune HD, Feature Phone, plus Windows 7/8 HD profiles
- **Credit templates**: DIRECTED BY, STARRING, FILMED ON LOCATION, SOUNDTRACK with placeholder text
- **Help URL**: `http://g.msn.com/5meen_us/122`
- **Font**: Segoe UI throughout

### Architecture Notes
- Canonical MUI satellite DLL pattern: loaded via LoadLibrary/FindResource/LoadString
- No COM registration, no self-registration exports
- Reconstructed source needs a `.rc` file with all strings mirrored from this DLL
- PNG resources can be extracted and reused as-is

## MovieMakerPreviewClient.dll Analysis

### Overview
COM proxy/stub DLL for the timeline/storyboard preview system (~28 KB image, ~5.9 KB code). PE32 x86, linker v11.0, image base 0x10000000, ASLR+NX. Timestamp: 2014-04-01. PDB: `{ABE6F6C4-02E6-47EA-B7FF-6A44A32264DD}`. Build: `16.4.3528.0331_ship.client.main.w5m4 (ship)`. Description: "Photo Gallery Preview Client". See full analysis at `analysis/MovieMakerPreviewClient/analysis.md`.

### Sections
| Section | VA | VirtSize | Purpose |
|---------|-------|----------|---------|
| `.text` | 0x1000 | 0x1704 | Code + CRT init (5.9 KB) |
| `.orpc` | 0x3000 | 0x41 | MIDL-gen'd proxy stub |
| `.data` | 0x4000 | 0x3B8 | Globals, IID table, vtables |
| `.rsrc` | 0x5000 | 0x420 | VERSION_INFO resource |
| `.reloc` | 0x6000 | 0x466 | ASLR relocations |

### Exports (4 COM standard)
| Ordinal | RVA | Name |
|---------|------|------|
| 1 | 0x14E1 | `DllCanUnloadNow` |
| 2 | 0x14A5 | `DllGetClassObject` |
| 3 | 0x1535 | `DllRegisterServer` |
| 4 | 0x1561 | `DllUnregisterServer` |

### COM Interface & GUIDs
- **`IPreviewClientStatusCallback`** — sole custom COM interface (6 methods: 3 IUnknown + 3 custom)
- **Interface IID**: `{DBFFDF24-FBB1-42D1-719A-EC305FBF765F}` (found at .data 0x10001210)
- Uses `HWND` parameters (ole32 `HWND_User*` marshaling)
- Uses `BSTR` parameters (OLEAUT32 ordinals 283-286 = SysAllocString/Free/Len/ByteLen)
- MIDL-generated proxy/stub with 19 RPCRT4.dll imports

### Dependencies (5 DLLs, ~59 functions)
- **MSVCR110.dll** (18): CRT init, memory, C++ exception, operator delete
- **KERNEL32.dll** (9): Thread, process, time, ASLR/DEP helpers
- **ole32.dll** (4): HWND_UserMarshal/Unmarshal/Free/Size
- **OLEAUT32.dll** (4 ordinals): SysAllocString, SysFreeString, SysStringLen, SysStringByteLen
- **RPCRT4.dll** (19): CStdStubBuffer*, IUnknown_*Proxy, NdrOle*, NdrDll*

### Key Design Points
1. **Minimal MIDL-generated proxy/stub DLL** — no application logic
2. **Dual role**: COM server (DllGetClassObject) + proxy/stub marshaling (NdrDllGetClassObject)
3. **HWND cross-apartment passing** via ole32 custom marshaling helpers
4. **No WLX dependencies** — fully self-contained, depends only on system DLLs
5. **6 methods** on IPreviewClientStatusCallback — reports preview status back to the host
6. Interface connects the **timeline/storyboard preview** (likely in MovieMakerCore.dll) with a **status sink** (the host application)

## Shared MF DLLs Analysis

### RSCMFT.dll — Rolling Shutter Correction MFT
**Location**: `undecomp/Shared/RSCMFT.dll`
**FileDescription**: "Rolling Shutter Correction MFT" (correcting earlier assumption of "Rich Scale/Color MFT")

| Field | Value |
|-------|-------|
| **Image** | 288 KB (0x48000), PE32 x86, linker v11.0, ASLR+NX |
| **Build** | `16.4.3528.0331_ship.client.main.w5m4` |
| **PDB** | `{8BE07FA3-8940-469E-A496-FED16CCA5FC4}`, RSCMFT.pdb |
| **Exports** | 4 standard COM |
| **MF imports** | MFTRegister, MFTUnregister, MFCreateMediaType, MFCreateVideoSampleAllocatorEx, MFCreateAttributes, MFCopyImage, MFGetStrideForBitmapInfoHeader |
| **CRT** | MSVCR110 — SSE2 math (_libm_sse2_pow/sin/cos/sqrt_precise, _CIatan2) |

**Internal classes**: `AVCRSC_MFT`, `AVCRSC_HS`, `AVCCustomWarpTransformUV/Y`, `AVCRSCorrection`, `AVCPyramid`, `AVCClassFactory`, `AVCFileBlobStore`, `CVec2`, `CTaskManager`

**Algorithm** (from Unicode strings): `Correspondences` → `Derivatives` → `Initialize DFlow` → `Compute Stabilization` → `Compute Correction` → `Image Warping` → `DestMap Computation`. Uses dense optical flow (DFlow) pipeline with image pyramids and per-plane custom warps (Y + UV).

**Sections**: .text (254 KB), .data (10.5 KB), .rsrc (1 KB), .reloc (10.5 KB)
**Dependencies**: MSVCR110, KERNEL32 (42 — thread pool!), ADVAPI32 (4 reg), MFPlat (7), ole32 (StringFromGUID2), SHLWAPI (1 ordinal)

### MPG4DEMUX.dll — MPEG-4 Demultiplexer
**Location**: `undecomp/Shared/MPG4DEMUX.dll`

| Field | Value |
|-------|-------|
| **Image** | 176 KB (0x2B000), PE32 x86, linker v11.0, ASLR+NX |
| **Build** | Same ship build |
| **PDB** | `{7A27A4CE-69B7-4A00-9C73-C687AA8E9ED0}`, Mpg4Demux.pdb |
| **Exports** | 4 standard COM |
| **MF imports** | MFCreatePresentationDescriptor, MFCreateMemoryBuffer, MFCreateMediaType, MFCreateStreamDescriptor, MFInitMediaTypeFromAMMediaType, MFInitAMMediaTypeFromMFMediaType, MFHeapAlloc/Free, MFllMulDiv |
| **COM** | CoCreateInstance, CLSIDFromString, CoInitialize, CoTaskMemAlloc/Free, PropVariantClear/Copy |
| **DShow** | Full DirectShow base class pattern (AVCBaseFilter, AVCBasePin, etc.) |

**Internal classes** (~50+): Full QuickTime/MP4 atom parser (`AVCQTMovie`, `AVCQTTrack`, `AVCQTChunkOffsetAtom`, `AVCQTSampleToChunkMapAtom`, `AVCQTTimeToSampleMapAtom`, etc.), sample descriptions for AVC/H.264, VC-1, MJPEG, AAC/PCM audio, codec config from `esds`/`avcC` boxes

**Architecture**: DirectShow filter graph + MF bridge — parses MP4 atoms natively, produces MF presentation/stream descriptors, converts between `AM_MEDIA_TYPE` and `IMFMediaType`

**Sections**: .text (142 KB), .data (5.6 KB), .rsrc (1 KB), .reloc (14.5 KB)
**Dependencies**: MSVCR110, KERNEL32 (31), USER32 (SetRect), ADVAPI32 (8 + TraceMessage ETW), ole32 (10 + PropVariant), OLEAUT32 (4 ordinals), MFPlat (9), PROPSYS (PropVariantChangeType)

### Shared Observations
- Both DLLs share Microsoft code signing cert GUIDs `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` and `{2860b52e-c4a3-454d-bc1e-32c5add17e90}`
- Both use the AVC (Audio Video Components) C++ class hierarchy — shared internal framework across the "Shared" MF DLLs
- Both are ATL COM DLLs with standard 4-export pattern
- Both built with MSVC 2012 (v11.0), targeting Windows 8+ (6.02 subsystem)
- Neither embeds CLSID/IID string literals — COM registration via ATL `.rgs` resources

## WLXMP4Parser.dll Analysis

### Overview
MP4 container parser/source filter DLL for Windows Live Movie Maker 2012 (~184 KB). PE32 x86, linker v11.0 (MSVC 2012), image base 0x10000000, ASLR+NX. Timestamp: 2014-04-01. PDB: `WLXMP4Parser.pdb` `{A4577D2C-5400-4F17-91C0-06B5EA1BE4DE}`. Build: 16.4.3528.0331_ship.client.main.w5m4. Description: "WLX MP4 Parser Filter".

### Exports (9 functions)
| Ordinal | Name | RVA | Purpose |
|---------|------|-----|---------|
| 1 | `AddMP4SourceFilter` | 0x1541E | Add MP4 source filter to graph |
| 2 | `BuildMP4FilterGraph` | 0x15132 | Build custom filter graph |
| 3 | `BuildMP4PlayBack` | 0x1533A | Build playback graph |
| 4 | `IsMP4FilePlayable` | 0x14F4E | Pre-check MP4 playability |
| 5 | `DllCanUnloadNow` | 0x1D16A | COM |
| 6 | `DllGetClassObject` | 0x1D1A3 | COM |
| 7 | `DllMain` | 0x86B6 | Entry |
| 8 | `DllRegisterServer` | 0x85F7 | COM reg |
| 9 | `DllUnregisterServer` | 0x8604 | COM unreg |

4 non-COM exports form the primary public API for WLMM.

### Dependencies (9 DLLs)
- **MSVCR110.dll** (33) — CRT: memcpy/memset/memcmp, wcscat_s, wcstol/wcstoul, _wcsicmp, wcsrchr, _vsnwprintf, operator new/delete, C++ exception handling (__CxxFrameHandler3, _except_handler4_common), type_info, _purecall, _calloc_crt, _malloc_crt, free, _initterm(_e)
- **KERNEL32.dll** (41) — Threading: CreateThread, Sleep, GetCurrentThread(Id), Set/GetThreadPriority, WaitFor*Object, CreateEventW, Set/ResetEvent, Interlocked*, Enter/Leave/DeleteCriticalSection. Files: SetFilePointer, SetEndOfFile, WriteFile. Modules: GetModuleHandleW, GetProcAddress, LoadLibraryW, FreeLibrary, GetModuleFileNameW/A. Time: QueryPerformanceCounter, GetTickCount(64), GetSystemTimeAsFileTime. Misc: lstrcmpW, lstrlenW/A, MultiByteToWideChar, WideCharToMultiByte, GetVersionExW, IsDebuggerPresent, IsProcessorFeaturePresent, DebugBreak, Encode/DecodePointer, DisableThreadLibraryCalls
- **gdiplus.dll** (15) — Thumbnail bitmap: GdipCreateBitmapFromScan0/GdiDib, GdipCloneImage, GdipDrawImageRectI, GdipGetImageWidth/Height, GdipGetImageGraphicsContext, GdipSetInterpolationMode, GdipCreateHBITMAPFromBitmap, GdipDisposeImage, GdipDeleteGraphics, GdiplusStartup/Shutdown
- **USER32.dll** (7) — Message pump: DispatchMessageW, PeekMessageW, PostThreadMessageW, GetQueueStatus, MsgWaitForMultipleObjects, SetRect, RegisterWindowMessageW
- **ADVAPI32.dll** (7) — Filter registration: RegCreateKeyW, RegOpenKeyExW, RegDeleteKeyW, RegEnumKeyExW, RegSetValueW, RegSetValueExW, RegCloseKey
- **ole32.dll** (9) — COM: CoInitialize/Uninitialize/FreeUnusedLibraries, CoCreateInstance, CoTaskMemAlloc/Free, PropVariantCopy/Clear, StringFromGUID2
- **OLEAUT32.dll** (4 ordinals) — BSTR/VARIANT support (ordinals 2, 4, 184, 185)
- **WINMM.dll** (4) — Multimedia timers: timeSetEvent, timeKillEvent, timeBeginPeriod, timeEndPeriod

### Sections
| Section | VA | VirtSize | RawSize | Purpose |
|---------|-------|----------|---------|---------|
| .text | 0x1000 | 0x24A1D | 0x24C00 | Code (150 KB) |
| .data | 0x26000 | 0x18B4 | 0x1600 | R/W globals, vtables, RTTI |
| .rsrc | 0x28000 | 0x3E8 | 0x400 | Resources (1 KB) |
| .reloc | 0x29000 | 0x4B48 | 0x4C00 | ASLR relocations (19 KB) |

### RTTI Class Hierarchy
**Core Parser:** CMP4Parser, CMP4InputPin, CMP4OutputPin, CMP4DecodeRendererFilter, CMP4DShowGraph, CMP4StreamHandler / IMP4StreamHandler
**QuickTime/ISO Atoms:** CQTMovie, CQTTrack, CQTMedia, CQTAtom, CQTFullAtom, CQTUserData, CQTMediaHandler/CQTVideoMediaHandler/CQTAudioMediaHandler, CQTMediaInformationAtom, CQTDataRefInfoAtom, CQTSampleTable, CQTSampleDescription/CQTVideoSampleDescription/CQTAudioSampleDescription/CQTLegacyAudioSampleDescription/CVC1VideoSampleDescription, CQTTimeToSampleMapAtom, CQTSampleToChunkMapAtom, CQTSampleSizeAtom, CQTChunkOffsetAtom, CQTChunkLargeOffsetAtom, CQTSyncSampleAtom, CQTCompositionTimeToSampleMapAtom, CQTESDSAtom
**Metadata:** CMP4MetadataHandler, CQTMetadataAtom, CQTAppleMetadataAtom, CAppleMetadataCollection/Record, CMSMetadataAtom/Collection/Record, CMP4Descriptor
**Thumbnailing:** MP4MovieThumbnail, CMP4BitmapGenerator, CYUVTORGB (software YUV->RGB)
**DirectShow Base:** CUnknown, CBaseObject, CBaseFilter, CBasePin (input/output), CTransformFilter, CBaseRenderer, CMediaPosition, CPosPassThru, CRendererPosPassThru
**Utility:** CCritSec, CAMThread, CEnumPins, CEnumMediaTypes, CEnumAtoms, CClassFactory, CQTDataBuffer

### MP4 Atom Types Parsed (FourCC)
**Containers:** moov, trak, mdia, minf, stbl, dinf, edts, udta, meta, ilst, mvex, moof, traf, sinf, schi
**Leaf - Movie:** ftyp, mvhd, mehd, free, skip, wide, pssh, sidx, ssix, prft, emsg
**Leaf - Track:** tkhd, mdhd, hdlr, vmhd, smhd, nmhd, elst, dref, trex
**Leaf - Sample Table:** stsd, stts, ctts, stsc, stsz, stco, co64, stss, stsh, padb, stdp, sdtp, sbgp, sgpd, saio, saiz
**Sample Desc FourCC:** avc1, avc2, mp4v, s263, mp4a, raw, twos, sowt, ima4, MAC3, MAC6, QDM2, Qclp, encv, enca, vc-1, wmv3, wmap, vp80, vp90, av01, jpeg, png, mjp2, tx3g, text, ac-3, ec-3, dts, alac, Opus, fLaC
**Metadata:** data, mean, name, ©too (iTunes keyspace)
**Fragment:** mfro, mfhd, tfhd, tfdt, trun, trex, leva, fecr, fpar, meco
**Protection:** pssh, tenc, sinf, frma, schm, schi
**Config:** avcC, btrt, pasp, esds, wave

### MPEG-4 Descriptors (via CMP4Descriptor)
Parses ISO 14496-1 descriptor chains from esds: ObjectDescriptor (1), InitialObjectDescriptor (2), ES_Descriptor (3), DecoderConfigDescriptor (4), DecoderSpecificInfo (5), SLConfigDescriptor (6), IPI_DescrPointer (10), IPMP_DescriptorPointer (11), LanguageDescriptor (14), QoS_Descriptor (15), RegistrationDescriptor (16), ExtensionDescriptor (21..254)

### Key Observations
1. **Thumbnail provider** - Implements IThumbnailProvider + IInitializeWithFile + IExtractImage + IPersistFile for Explorer MP4 thumbnail previews via GDI+
2. **Software YUV->RGB fallback** - CYUVTORGB for thumbnails without hardware decode
3. **VC-1 support** - CVC1VideoSampleDescription for VC-1/SMPTE video in MP4 containers
4. **Metadata** - Full Apple/iTunes (ilst/data/mean/name) + Microsoft (CMSMetadata*) metadata extraction
5. **DRM detection** - pssh, tenc, sinf atoms parsed (detection only, no decryption)
6. **DirectShow source filter** - At core it's a DShow source filter feeding MP4 demuxed streams
7. **WLXVideoTrim dependency** - Loaded delay-load by WLXVideoTrim.dll for MP4 trim support

## WLXFaceRecognition Test Harness

### Files
- `analysis/WLXFaceRecognition/harness.md` — full harness document with all 4 export signatures, 7 COM interface defs, per-export test harness functions, complete main.cpp, CMakeLists.txt for test project, and expected behaviors
- Copies original WLXFaceRecognition.dll, WLXPhotoBase.dll, WLXPhotoSqm.dll alongside test binary
- Requires x86 process, VS 2012 redist, admin rights for registration tests

## Publish Plugins Analysis

### Critical Distinction
All four publishing plugins (WLFacebookPlugin, WLFlickrPlugin, WLVimeoPlugin, WLYouTubePlugin) are **.NET Framework 4.0 managed assemblies**, NOT native C++/ATL COM DLLs like every other WLX/WL DLL in the suite. This is a unique category within the Windows Live Movie Maker 2012 codebase.

### PE Structure (All Four Identical Pattern)
| Feature | Value |
|---------|-------|
| **Type** | PE32 x86 .NET assembly |
| **Linker** | MSVC 11.00 (VS 2012) |
| **Subsystem** | Windows CUI (3) |
| **Import** | Only `mscoree.dll` / `_CorDllMain` |
| **Exports** | None (native) — COM via .NET interop |
| **Sections** | .text (IL+metadata), .rsrc (resources), .reloc |
| **CLR Header** | COM Descriptor at RVA 0x2008, size 0x48 |
| **Characteristics** | 0x8540: ASLR + DEP + No SEH + Terminal Server Aware |
| **Version** | 16.4.3528.331, "Windows Live (R)", Microsoft Corp. |
| **Target** | `.NETFramework,Version=v4.0` |

### File Sizes
| DLL | Size | Largest aspect |
|-----|------|----------------|
| WLFacebookPlugin.dll | 558 KB | Photo + video album management + friends |
| WLFlickrPlugin.dll | 265 KB | Photo sets + OAuth |
| WLVimeoPlugin.dll | 228 KB | Video upload wizard + TOS/privacy text |
| WLYouTubePlugin.dll | 133 KB | Most validation errors, Wizard UI |

### Plugin Interface (Common)
All implement `IPublishPlugin` (from `WLXMediaPublishSubscribe.dll`) via .NET COM interop. Facebook additionally implements `ISubscribePlugin`. The native framework discovers plugins via COM and calls `CreatePlugin()` → `Initialize()` → `ShowConfigurationSettings()` → `PublishItem()`.

### Authentication Comparison
| Plugin | Method | Key Fields |
|--------|--------|-----------|
| **Facebook** | Custom REST API key/secret + session | APIKey, Secret, SessionKey, SessionSecret |
| **Flickr** | OAuth 1.0 | consumerKey, consumerSecret, Token, TokenSecret |
| **Vimeo** | OAuth 1.0 | consumerKey, consumerSecret, Token, TokenSecret |
| **YouTube** | Google ClientLogin + Developer Key | clientId, DeveloperKey, Token |

### Upload Mechanism (Common)
Chunked HTTP upload via `System.Net.HttpWebRequest`/`HttpWebResponse` with `UploadChunk()` in all four. Errors handled via plugin-specific exception classes (`FacebookException`, `FlickrApiException`, `VimeoApiException`, `YouTubeApiException`). Session data persisted as XML.

### Key Class Names
- `FacebookPlugin`, `FacebookPublishPlugin`, `FacebookSubscribePlugin`, `FacebookService`, `FacebookParser`, `FacebookException`, `IFacebook`
- `FlickrPlugin`, `FlickrPublishPlugin`, `FlickrApi`, `FlickrApiException`
- `VimeoPlugin`, `VimeoPublishPlugin`, `VimeoApi`, `VimeoApiException`
- `YouTubePlugin`, `YouTubePublishPlugin`, `YouTubeApi`, `YouTubeApiException`, `YouTubeConfigureForm`

### YouTube COM GUID
`860ED1B9-FDFE-4930-A609-77B12B8B5610` — likely `CLSID_YouTubePlugin`.

### URLs Found (ASCII strings in binary)
- `http://download.live.com/` (Facebook plugin — Windows Live download)
- `https://www.google.com/accounts/DisplayUnlockCaptcha` (YouTube — CAPTCHA challenge)
- `FACEBOOK_REST_URL`, `FACEBOOK_VIDEO_REST_URL` (Facebook — API endpoint constants, exact values not in ASCII)
- Certificate revocation lists from `http://crl.microsoft.com/` and `http://www.microsoft.com/pki/` (Authenticode signatures)

### Key Resource Strings
- Facebook: Permissions (offline_access, photo_upload, video_upload), friend visibility, album visibility
- Flickr: Photo permissions (Public/Friends/Family/Private), upload limits
- Vimeo: TOS text, privacy info, video resolution warnings, download privacy
- YouTube: Extensive validation errors (10+ types), category system, tags/keywords, safety settings

### See Full Analysis
`analysis/PublishPlugins/analysis.md`

## WLXSlideshow Test Harness

Location: `analysis/WLXSlideshow/harness.md`

### Overview
C++ test harness exercising all 4 COM exports (`DllCanUnloadNow`, `DllGetClassObject`, `DllRegisterServer`, `DllUnregisterServer`) via LoadLibrary + GetProcAddress with SEH exception guarding.

### Test Cases (14 total)

| TC | Export | Input | Expected | Notes |
|----|--------|-------|----------|-------|
| 1 | DllCanUnloadNow | () | S_OK | Stub always returns S_OK |
| 2 | DllCanUnloadNow | () x5 | S_OK | Idempotent, no refcount |
| 3 | DllGetClassObject | Known CLSID + IID_IUnknown | CLASS_E_CLASSNOTAVAILABLE | CLSID not registered |
| 4 | DllGetClassObject | CLSID_NULL | CLASS_E_CLASSNOTAVAILABLE | Null CLSID |
| 5 | DllGetClassObject | NULL ppv | E_POINTER or AV | Boundary — SEH guarded |
| 6 | DllGetClassObject | Zeroed CLSID | CLASS_E_CLASSNOTAVAILABLE | Invalid CLSID |
| 7 | DllGetClassObject | Zeroed IID | CLASS_E_CLASSNOTAVAILABLE | Invalid IID |
| 8 | DllRegisterServer | () | S_OK | Stub, no registry writes |
| 9 | DllRegisterServer | () x2 | S_OK | Idempotent |
| 10 | DllUnregisterServer | () | S_OK | Stub |
| 11 | DllUnregisterServer | () x2 | S_OK | Idempotent |
| 12 | LoadLibrary | "WLXSlideshow.dll" | Valid HMODULE | GDI+ init in DllMain |
| 13 | GetProcAddress | 4 export names | Non-NULL | All 4 in .def |
| 14 | FreeLibrary | HMODULE | TRUE | Clean unload |

### Harness Structure
- `SafeCall_*` wrappers with `__try/__except` for each export
- Test result counting (pass/fail) with printf logging
- `CoInitializeEx`/`CoUninitialize` around COM-sensitive tests
- CMakeLists.txt copies DLL to test output dir via POST_BUILD

### Original Binary Deviations
- Original `DllGetClassObject` returns real class factories for 3 CLSIDs after ATL init
- Original `DllRegisterServer` writes COM registry keys + slideshow settings
- Original `DllUnregisterServer` removes those keys
- Reconstructed source stubs all four exports; harness validates current behavior

## WLXCodecHost.exe Analysis

### Overview
ATL-based out-of-process COM LocalServer32 that serves as a **sandboxed codec host** for Windows Live Photo Gallery. Codecs are isolated in this separate process for crash isolation, security, and memory management.

### PE Metadata
| Field | Value |
|-------|-------|
| Binary | `WLXCodecHost.exe` |
| Arch | x86 (32-bit) |
| Subsystem | Windows GUI |
| Linker | VS 2012 (11.00) |
| Entry | `0x00406D6C` |
| Image base | `0x00400000` |
| Size | 53,248 bytes (`0xD000`) |
| OS target | 6.02 (Windows 8) |
| Timestamp | 2014-04-01 |
| Signed | Microsoft Authenticode |

### COM Identity
- **CLSID:** `{E30A45E6-1916-4659-95EE-035E62DB9AB0}`
- **ProgID:** `Microsoft.WLXCodecHost.CodecHost.1`
- **Interface:** `ICodecHost` (custom, undocumented)

### Key Dependencies
| DLL | Purpose |
|-----|---------|
| `ole32.dll` | COM infrastructure (CoRegisterClassObject, etc.) |
| `gdiplus.dll` | Image decoding |
| `GDI32.dll` | `CreateDIBSection` — DIB management |
| `SHELL32.dll` | `SHCreateItemFromParsingName` — shell parsing |
| `PROPSYS.dll` | `PSCoerceToCanonicalValue` — property coercion |
| `MetadataSys.dll` | `WLXPSGetItemPropertyHandler` — photo metadata property handler |
| `WLXPhotoBase.dll` | Memory allocation, exception handling, OS version detection |
| `ADVAPI32.dll` | Registry sandboxing (RegCreateKeyExW, RegDeleteKeyW, etc.) |
| `MSVCR110.dll` | Visual C++ 2012 runtime |

### Architecture Notes
- **OOP sandbox**: Isolated process with `SetPriorityClass` control, SEH (`__CxxFrameHandler3`, `_XcptFilter`) for fault tolerance.
- **Message pump**: UI thread with `GetMessageW`/`DispatchMessageW`; cross-thread via `PostThreadMessageW`.
- **Lifetime**: Managed via `CoAddRefServerProcess`/`CoReleaseServerProcess` + `CoResumeClassObjects`.
- **RTTI chain**: `CAtlExeModuleT<CodecHostModule>` → `CodecHost` (implements `ICodecHost`) via `CComObject<CodecHost>`.
- **See full analysis:** `analysis/WLXCodecHost/analysis.md`


## WLMFReadWrite.dll Analysis

### Overview
Media Foundation Read/Write wrapper DLL (252 KB) from Windows Live Essentials 2012. Implements Source Reader and Sink Writer APIs with custom Windows Live-specific logic (profile management, async work queues, ETW tracing, video processing MFT). Not a reimplementation � delegates to MFPlat.DLL and MF.dll. PE32 x86, linker v11.0, ASLR+NX. Build: 16.4.3528.0331_ship.client.main.w5m4. PDB: WLMFReadWrite.pdb {54E10C67-67A5-4F8A-8567-1D47C28B2189}.

### Exports (7)
| Ordinal | Name |
|---------|------|
| 1 | DllCanUnloadNow |
| 2 | DllGetClassObject |
| 3 | MFCreateSinkWriterFromMediaSink |
| 4 | MFCreateSinkWriterFromURL |
| 5 | MFCreateSourceReaderFromByteStream |
| 6 | MFCreateSourceReaderFromMediaSource (alias, same RVA as #5) |
| 7 | MFCreateSourceReaderFromURL |

These shadow the standard MF API functions - they are **not** the system APIs (which are in mfreadwrite.dll/mfplat.dll).

### Dependencies
- **MFPlat.DLL** (19): Work queues, media types, buffers, samples, MFTEnum, attributes
- **MF.dll** (5): Presentation clock, ASF profile/sink, source resolver, MFGetService
- **EVR.dll** (3): Video stride, sample allocator, MFCopyImage
- **PROPSYS.dll** (2): PSCreateMemoryPropertyStore, PropVariantToVariant
- **OLEAUT32.dll** (3 ordinals): VariantClear/Init, SysFreeString
- **ole32.dll** (7): CoTaskMem, CoCreateInstance, StringFromGUID2, PropVariant
- **ADVAPI32.dll** (16): Registry (9) + ETW tracing (7)
- **USER32.dll** (1): CharNextW
- **KERNEL32.dll** (47): Process, modules, heap, sync, system info
- **MSVCR110.dll** (36): CRT memory, string, exceptions, RTTI

### RTTI Classes (~70+)
- **Factory**: CMFReadWriteClassFactory (ATL CComObject + CComObjectCached)
- **Source Reader**: CMFSourceReader, CMFSourceReaderManager, CMFSourceReaderStream, CMFSourceReaderEventQueue, AppCallback variants
- **Sink Writer**: CMFSinkWriter, CMFSinkWriterManager, CMFSinkWriterStream, AppCallback variants, MarkerContext
- **Profile Mgmt**: CMFSinkWriterProfileManager + Generic/MP3/3GP/MPEG4/ASF subclass variants
- **MFT Pipeline**: CMFReadWriteTransform, CMFReadWriteTransformChain, CMFReadWriteTransformActivate, CMFReadWriteAsyncTransform
- **Video Proc**: CxCodeVideoProcMFT, CxCodeVideoProcTypeHandler/DataHandler, CxCodeVideoProcThread
- **Support**: CMFMediaBufferAllocator, CMFMemoryBuffer, CMFRatelessTimeSource, CMFTBase, CMFTTypeHandler/DataHandler

### Sections
- .text (0x1000, 217 KB code), .data (0x38000, 7.5 KB), .rsrc (0x3A000, 1 KB), .reloc (0x3B000, 14.8 KB)

### Key Architecture
1. Custom MFCreate* functions shadow standard MF API with WL-specific enhancements
2. All operations async via MF work queues (MFAllocateWorkQueue + MFPutWorkItem)
3. Profile managers for 3GP/MPEG4/MP3/ASF output container negotiation
4. Custom video processing MFT (CxCodeVideoProcMFT) for color conversion/scaling
5. ATL COM factory pattern via CComObjectCached<CComClassFactory>
6. Full ETW instrumentation via ADVAPI32 trace APIs
7. Async callbacks with separate objects per operation type (OnReadSample, OnEvent, OnFlush, OnMarker, OnFinalize)

### Full Analysis
analysis/WLMFReadWrite/analysis.md

## WLXPipetran.dll Analysis

### Overview
Pipeline transport/transform DLL for Windows Live Movie Maker 2012. DirectX 9 + GDI+ effect/compositing engine providing transitions, animations, DVD menu compositing, and pixel shader effects. Single export `GetTFXCreateFunctions` factory. PE32 x86, linker v11.0, ASLR+NX. Timestamp: 2014-04-01. PDB: `WLXPipeTran.pdb` `{89765767-4CF9-44E2-8351-95650D5D12B1}`. Build: 16.4.3528.0331 (ship). Image: 720 KB, code: 299 KB.

### Sections
| Section | VA | VirtSize | RawSize | Purpose |
|---------|------|----------|---------|---------|
| `.text` | 0x10001000 | 0x49305 (299 KB) | 0x49400 | Code, Execute+Read |
| `.data` | 0x1004B000 | 0x20C4 (8.2 KB) | 0x1600 | R/W globals, vtables |
| `.rsrc` | 0x1004E000 | 0x5D670 (382 KB) | 0x5D800 | Resources (shaders, patterns) |
| `.reloc` | 0x100AC000 | 0x7E2C (31.5 KB) | 0x8000 | ASLR relocations |

### Exports (1)
| Ordinal | RVA | Name |
|---------|------|------|
| 1 | 0x9DD5 | `GetTFXCreateFunctions` |

Returns `ITransformFXCollection` — all other functionality via COM vtable dispatch.

### Dependencies (12 DLLs, ~200 functions)
- **MSVCR110.dll** (48): CRT, SSE2 precise math trig, C++ exception, memory
- **KERNEL32.dll** (55): Process, modules, resources, heap, sync, locale, time
- **USER32.dll** (8): DC, cursors, system colors, rectangles, CharNext
- **GDI32.dll** (5): Stock objects, text, SelectObject, DeleteObject
- **ADVAPI32.dll** (3): Registry open/query/close
- **ole32.dll** (8): CoCreateInstance, CLSIDFromString, PropVariant, CoTaskMem, CoInitialize
- **OLEAUT32.dll** (13 ordinals): Variant/SafeArray/BSTR
- **SHLWAPI.dll** (5): Path manipulation, file existence
- **COMCTL32.dll** (3): ImageList_Create/Destroy/Add
- **PSAPI.DLL** (1): GetModuleFileNameExW
- **gdiplus.dll** (50): Bitmap loading/drawing, font/string, path/matrix, pen/brush
- **d3dx9_32.dll** (26): D3DX mesh, matrix/quaternion math, shader compilation, texture fill

### Architecture
1. **Transform Engine**: Single factory export returns `ITransformFXCollection` — effects accessed via COM
2. **Transition System**: 28+ pattern mesh classes (circle, heart, star, checkerboard, wheel, iris, diamond, zigzag, etc.)
3. **Animation Framework**: Camera, 2D/3D projection, mesh, dissolve/wipe, title/credit text, DVD menu animations
4. **DVD Authoring**: CDvdMenuInput/Button/Compositor classes for DVD-style menu compositing
5. **Pixel Shader**: D3DX shader compilation with inline TX_Noise HLSL (noise texture generator)
6. **Text Rendering**: GDI+ (text drawing + paragraph/banner/title animation classes)
7. **Dual Stack**: GDI+ (2D/layout) + Direct3D 9 via D3DX (3D mesh/matrix/shaders)
8. **No Media Foundation**: Pure D3D9/GDI+ — operates outside MF pipeline

### Key RTTI Classes (~150+ total)
- **Core**: CTransformFXCollection, CTransformFX, CMediaTransform, CDvdMediaTransform, CCreateFunction
- **Transitions**: 28 CPatternMesh subclasses (Rectangle, Star, Circle, Heart, Wheel, Iris, Checkerboard, Fan, Sweep, etc.)
- **DVD**: CDvdMenuInput, CDvdMenuButton, CDvdMenuCompositor, CDvdFade, CDvdVideoWallAnimation
- **Animation**: CAnimation, CCameraSetupAnimation, CWipeAnimation, CDissolveAnimation, CPixelateAnimation, CPanZoom, CTitles, CCredits, CBanner
- **Effects**: CVBFXKey, CVBLineKey, CVBLintKey, CVBSplotchKey, CFXAgeAnimation
- **3D**: CSimple3DTransform/Base/Particle, CDXUTMesh, CAlphaMap
- **Text**: CParagraphAnimation, CBannerAnimation, CTextLine

### Key Strings
- Effect params: `BlurSize/Axis`, `Shatter`, `PosterizeLevels`, `GrayscaleOn`, `NoiseFrequency`, `ShakeFactor`, `RotationDegree`, `PixelSize`, `TexturePerturbation`
- Texture semantics: `InputTexture0/1`, `VideoTexture`, `MaskTexture`, `OverlayTexture`, `NoiseTexture`, `Projection`
- Camera: `CameraZoom`, `TextureViewport`, `RevolveCamera`, `DebugCamera`
- CSS color names (140+) + system color names
- Inline shader: `TX_Noise` HLSL procedural noise generator

### COM GUIDs
No CLSID/IID string literals found; these are defined in .rgs resources or registered via ATL OBJECT_MAP.

### Full Analysis
`analysis/WLXPipetran/analysis.md`

## WLMFDS.dll Analysis

### Overview
Media Foundation DirectShow bridge DLL (~417 KB, 345 KB code). PE32 x86, ATL 7.0 COM DLL, linker v11.0 (VS 2012), ASLR+DEP. Timestamp: 2014-04-01. Build: 16.4.3528.0331_ship.client.main.w5m4. PDB: `WLMFDS.pdb` `{41188442-2579-4939-8E66-6697FE148922}`.

### Exports (4 — standard ATL COM)
`DllCanUnloadNow`, `DllGetClassObject`, `DllRegisterServer`, `DllUnregisterServer`

### Dependencies (14 DLLs)
- **MSVCR110.dll** (42): CRT memory, strings, exceptions, RTTI
- **KERNEL32.dll** (53): Thread pool, sync, memory-mapped files, modules, file I/O, Interlocked ops
- **ADVAPI32.dll** (24): Registry (13), ETW tracing (6), security descriptors (5)
- **ole32.dll** (14): COM core, CoCreateInstance, CLSID/string conversion, PropVariant
- **OLEAUT32.dll** (4 ordinals): Variant/automation support
- **MFPlat.DLL** (17): MF samples, streams, descriptors, buffers, work queues, async callbacks
- **MF.dll** (1): `MFCreateSourceResolver`
- **WINMM.dll** (6): Multimedia timers (timeSetEvent, timeGetTime, etc.)
- **AVRT.dll** (2): MMCSS thread scheduling
- **EVR.dll** (1): `MFCopyImage`
- **SHELL32.dll** (1): `SHCreateItemFromParsingName`
- **SHLWAPI.dll** (1): `PathIsRelativeW`
- **VERSION.dll** (3): File version info
- **USER32.dll** (1): `CharNextW`

### Implemented Interfaces (from RTTI)
- **MF**: `IMFMediaSource`, `IMFSource`, `IMFMediaStream`, `IMFAsyncCallback`, `IMFRateControl`, `IMFRateSupport`, `IMFGetService`
- **DShow**: `IMemInputPin`, `IMemAllocator`, `IMediaFilter`, `IMediaSample`, `IMediaSeeking`, `IMediaPosition`, `IAMLatency`
- **MPEG-2**: `IMpeg2Demultiplexer`, `IMPEG2Stream`, `IMpeg2TS`
- **Codec**: `ICodecAPI`

### Internal AVC Classes
`AVCBaseFilter`, `AVCBasePin`, `CEnumPins`, `CKey`, `CPCRValue`

### Architecture
Wraps DirectShow filter graphs as Media Foundation sources. Builds a DShow filter graph internally, exposes each output pin as an `IMFMediaStream`, and the graph as an `IMFMediaSource`. Bridges rate control (seeking, speed), async delivery, and media type negotiation between the two APIs. References ~100+ DShow CLSID names (source, transform, render, capture, TV, DVD), ~100+ MEDIASUBTYPE_* names, and all major DShow category/property constants.

### Full Analysis
analysis/WLMFDS/analysis.md

## WLXPipeline.dll Analysis

### Overview
Media processing pipeline DLL (728 KB image, 605 KB code) for Windows Live Movie Maker 2012. PE32 x86, linker v11.0, image base 0x10000000, ASLR+NX. Timestamp: 2014-04-01. PDB: `WLXPipeline.pdb` `{911D33AE-5E96-461A-B0D1-1ECCB116070F}`. Build: 16.4.3528.0331_ship.client.main.w5m4.

### Exports (2 — custom, not ATL COM)
| Ordinal | RVA | Name |
|---------|------|------|
| 1 | 0x0002314C | `GetPipelineCreateFunctions` |
| 2 | 0x00023443 | `DllRegisterServer` |

**Notable**: No `DllGetClassObject`/`DllCanUnloadNow` — uses custom factory export pattern (shared with WLXPipetran.dll's `GetTFXCreateFunctions` and WLXVideoTrim.dll's 5 factory exports).

### Sections
| Section | VA | VirtSize | RawSize | Purpose |
|---------|------|----------|---------|---------|
| `.text` | 0x1000 | 0x96C06 | 0x96E00 | Code (605 KB) |
| `.data` | 0x98000 | 0x9B24 | 0x9000 | R/W globals, vtables |
| `.rsrc` | 0xA2000 | 0x3F0 | 0x400 | Resources (1 KB) |
| `.reloc` | 0xA3000 | 0xEA08 | 0xEC00 | ASLR relocations (58 KB) |

### Dependencies (14 DLLs, ~200 functions)
- **MSVCR110** (52): CRT, SSE2 math, memory, strings, C++ exception
- **KERNEL32** (48): Process, heap, sync (CRITICAL_SECTION, ConditionVariable, Semaphore), thread, time, modules
- **ADVAPI32** (12): Registry (6) + ETW tracing (6 — Register/Unregister/GetEnableFlags/Level/Handle, TraceEvent)
- **USER32** (19): Display/monitor enumeration, window/message, desktop
- **GDI32** (3): SetBkColor, DeleteObject, ExtTextOutW
- **SHLWAPI** (2): PathRemoveFileSpecW, PathFindExtensionW
- **ole32** (8): COM core, CoCreateInstance, PropVariant, CoTaskMem
- **OLEAUT32** (9 ordinals): Variant/automation
- **gdiplus** (29): Bitmap I/O, image metadata, frame selection, encoding, GDI+ init
- **d3d9** (1): Direct3DCreate9Ex
- **d3dx9_32** (4): Matrix math (translation/scaling/multiply) + D3DXCreateEffectFromFileW
- **DDRAW** (2): DirectDrawCreateEx, DirectDrawEnumerateExA
- **WINMM** (5): Multimedia timers (timeBegin/End/Get/Set/Kill)
- **msdmo** (3): DirectShow Media Object helpers (MoInitMediaType/FreeMediaType/CopyMediaType)

### COM CLSID
- `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` — CLSID_VideoTrim (shared with WLXVideoTrim.dll, WLXMediaPublishSubscribe.dll)

### Pipeline Stage Names (from Unicode strings)
- `Photo Gallery Video Pipeline` — description
- `Pipeline\AudioMixer`, `Pipeline\SilenceAudio`, `Pipeline\SolidColor` — stage paths
- `Timeline Source Stream Seeking` — timeline seeking
- `Image Drawer Allocator` — image buffer allocator
- Video transforms: `Deinterlace:`, `FrameEat`, `DVAspectRatio`, `XForm In/Out`
- Effect params: `Volume`, `Opacity`, `Color`, `MixingMode`, `Scrunch`, `Translation`
- DXVA flags: `DXVAEnabled`, `DXVAEnableForMM`, `DisableDXVAHWDeinterlacer`, `Force InverseTelecinePattern`

### Pipeline RTTI Classes
- **Core**: CNode, CEffect, CEffectNode, CQueueManager, CSyncManager, CPipelineItems, CPipelineStream, CPipelineStreamBuilder, CSourceCreator, CFileSourceWrapper, CMediaStream, CBaseSource, CBaseAudioRenderer
- **Audio**: CAudioMixerNode, CAudioBuffer, CResampledAudioBuffer, CAudioMediaStream, CSilenceAudioSource, CMediaStream
- **Video**: CSolidColorSource, CDShowVideoStream, CDVAspectRatio, CSurface
- **D3D9 Proxy Layer** (14 classes): CProxy_IDirect3DDevice9Ex/Texture9/CubeTexture9/VolumeTexture9/Surface9/Volume9/IndexBuffer9/VertexBuffer9/PixelShader9/VertexShader9/VertexDeclaration9/StateBlock9/SwapChain9/Resource9/Query9 + CProxy_IDirectXVideoAccelerationService/VideoProcessor/VideoProcessorService

### Registry Paths
- `Software\Microsoft\GPUPipeline`
- `Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline`

### Key Observations
1. **Not a standard ATL COM DLL** — uses `GetPipelineCreateFunctions` factory export (like WLXPipetran's `GetTFXCreateFunctions`)
2. **D3D9 proxy layer** intercepts all 14+ Direct3D 9 interfaces for GPU pipeline resource tracking; includes DXVA2 video acceleration proxying
3. **DirectShow integration** through msdmo.dll (media type helpers) and CDShowVideoStream wrapper
4. **Audio + video pipeline** — mixing, silence gen, solid color gen, frame throttling, deinterlacing, inverse telecine
5. **Image processing** via gdiplus.dll (29 functions) for bitmap loading/manipulation/encoding
6. **GPU pipeline config** stored in `WLXGPUPipeline` registry key
7. **ETW tracing** via ADVAPI32 for performance instrumentation (shared pattern with WLXPhotoCinematic, WLMFReadWrite)
8. See full analysis at `analysis/WLXPipeline/analysis.md`

## WLXTranscode.exe Analysis

### Overview
PE32 x86 console (CUI) executable serving as the **Media Foundation transcoding engine** for Windows Live Movie Maker 2012. Despite the `.exe` extension, VERSIONINFO identifies it as `WLXTranscoder.dll` — built as a DLL but deployed as a standalone tool. Orchestrates hardware (DXVA2/D3D9/D3D11) and software video transcoding via the MF pipeline.

### PE Metadata
| Field | Value |
|-------|-------|
| Binary | `WLXTranscode.exe` |
| Arch | x86 (32-bit) |
| Subsystem | Windows CUI |
| Linker | VS 2012 (11.00) |
| Entry | `0x0043F206` |
| Image base | `0x00400000` |
| Size | 311,296 bytes (`0x4C000`) |
| OS target | 6.02 (Windows 8) |
| Timestamp | 2014-04-01 |
| PDB | `WLXTranscode.pdb` `{3C6BCAEB-AF8F-4C79-9830-EB27BFFDD447}` |

### Assembly & Version Info
- **Assemblies:** `Microsoft.Windows.personalMedia.WLXTranscode` (v5.1.0.0)
- **FileVersion:** `16.4.3528.0331_ship.client.main.w5m4`
- **InternalName:** `WLXTranscoder`
- **OriginalFilename:** `WLXTranscoder.dll`

### Dependencies
| DLL | Purpose |
|-----|---------|
| `MF.dll` | Transcode profile, topology, media session creation |
| `MFPlat.DLL` | Platform startup, attributes, media types, async work queues |
| `dxva2.dll` | DXVA2CreateVideoService, DXVA2CreateDirect3DDeviceManager9 |
| `d3d9.dll` | Direct3DCreate9Ex (DXVA interop) |
| `d3d11.dll` | D3D11CreateDevice (DXVA interop) |
| `WLXPhotoBase.dll` | ATL memory allocation, exceptions, OS version checks |
| `PROPSYS.dll` | PSCreateMemoryPropertyStore |
| `SHLWAPI.dll` | Path manipulation utilities |
| `SHELL32.dll` | SHGetPropertyStoreFromParsingName |
| `OLEAUT32.dll` | Automation string types (ordinal imports) |
| `ADVAPI32.dll` | ETW tracing, cryptography (hashing/signing), registry |
| `MSVCR110.dll` | Visual C++ 2012 runtime |

### Exports
**None** — no export directory. Invoked directly, not as a COM LocalServer32.

### COM / ATL Classes (RTTI)
- **HMRAVSource:** `AVSourceFactory`, `AVSourceProxy`, `AVSource`, `DShowMFSourceReaderBuilder`, `NativeMFSourceReaderBuilder`, `AsyncSourceResolver`, `MFByteStreamOnStreamAsyncResult`, `AVSink`, `XVideoProc`, `DXVA2VideoProc`, `AuthCredentials`
- **HMRTranscode:** `CodecMetadataParser`, `MFTranscodeJob` (main transcode job controller)
- All objects are `CComObject<...>` ATL wrappers

### Video Codecs Supported (FOURCCs in .text)
- **H264** (H.264/AVC) — primary output codec
- **WVC1** (VC-1)
- **WMV3/2/1** (Windows Media Video 9/8/7)
- **MP43/MP4V/MP4S** (MPEG-4 Part 2 variants)
- Additional raw formats: P210, v410, P010, Y216, Y210

### Architecture
1. **Init:** `CoInitializeEx` + `MFStartup` + ATL setup via WLXPhotoBase
2. **Source:** `MFCreateSourceResolver` / `AsyncSourceResolver` → opens input file
3. **Profile:** `MFCreateTranscodeProfile` + `MFTranscodeGetAudioOutputAvailableTypes`
4. **Topology:** `MFCreateTranscodeTopology` / `MFCreateTopologyNode`
5. **Session:** `MFCreateMediaSession` → async work items → transcode
6. **Video Processing:** Dual path — software `XVideoProc` or hardware `DXVA2VideoProc` (D3D9/D3D11/DXVA2)
7. **Tracing:** Full ETW instrumentation for diagnostics
8. **Shutdown:** `MFShutdown` + `CoUninitialize`
- **See full analysis:** `analysis/WLXTranscode/analysis.md`
