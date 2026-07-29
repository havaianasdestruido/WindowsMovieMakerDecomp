# COM Apartment Threading Model Audit

**Date:** 2026-07-29
**Scope:** All native DLLs in `src/`
**Reference:** `analysis/CROSS_BINARY_MASTER.md` Section G (Thread Model)

---

## Summary Table

| DLL | Current COM Init | Analysis Target | Match? | Fix |
|-----|-----------------|-----------------|--------|-----|
| **MovieMakerCore.dll** | `COINIT_MULTITHREADED` (primary), `COINIT_APARTMENTTHREADED` (fallback) | MTA (`CComMultiThreadModel`) | ✅ | None needed |
| **UXCore.dll** | `COINIT_APARTMENTTHREADED` | MTA (per cross-binary analysis) | ❌ | Fixed → `COINIT_MULTITHREADED` |
| **uxctl.dll** | `COINIT_APARTMENTTHREADED` | MTA (consistent with UXCore) | ❌ | Fixed → `COINIT_MULTITHREADED` |
| **WLXFaceRecognition.dll** | `COINIT_APARTMENTTHREADED \| COINIT_DISABLE_OLE1DDE` | MTA (Section G: "MTA, Background detection threads") | ❌ | Fixed → `COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE` |
| **WLXSlideshow.dll** | *None (missing)* | MTA (loads COM objects, used by MTA host) | ❌ | Fixed → Added `COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE` |
| **WLXMediaPublishSubscribe.dll** | `COINIT_APARTMENTTHREADED \| COINIT_DISABLE_OLE1DDE` | MTA (Section G: "MTA, Grinder scheduler workers") | ❌ | Fixed → `COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE` |
| **WLXPhotoCinematic.dll** | *None (missing)* | MTA (hosts 2 COM objects, loaded into MTA host) | ❌ | Fixed → Added `COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE` |
| **WLXVideoTrim.dll** | `COINIT_APARTMENTTHREADED \| COINIT_DISABLE_OLE1DDE` | Free/Apartment per CLSID | ⚠️ | No change (DirectShow filters need STA) |
| **WLXMovieLibrary.dll** | `COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE` | MTA (`CComMultiThreadModel`) | ✅ | None needed |
| **WLXPipeline.dll** | `COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE` | MTA | ✅ | None needed |
| **WLXPipetran.dll** | `COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE` | MTA | ✅ | None needed |
| **WLXMP4Parser.dll** | *None (missing)* | MTA (DirectShow + MF, used by MTA host) | ⚠️ | No change (loaded via COM, no DllMain COM usage) |
| **WLMFDS.dll** | `COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE` | MTA | ✅ | None needed |
| **WLMFReadWrite.dll** | `COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE` | MTA | ✅ | None needed |
| **WLXPhotoSqm.dll** | `COINIT_MULTITHREADED` | MTA | ✅ | None needed |
| **MetadataSys.dll** | `COINIT_MULTITHREADED` | MTA | ✅ | None needed |
| **DmxBici.dll** | `COINIT_MULTITHREADED` | MTA | ✅ | None needed |
| **wlidcli.dll** | `COINIT_MULTITHREADED` | MTA | ✅ | None needed |
| **MovieMakerPreviewClient.dll** | *None (no COM operations)* | N/A (utility EXE) | ✅ | None needed |
| **WLXCodecHost.exe** | `COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE` (EXE) | MTA | ✅ | None needed |

---

## Detailed Findings

### 1. UXCore.dll — FIXED
- **File:** `src/UXCore/dllmain.cpp:39`
- **Current:** `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)`
- **Assessment:** UXCore is the DirectUI framework DLL loaded by MovieMakerCore. MovieMakerCore uses MTA. Running UXCore with STA while the host uses MTA causes cross-apartment marshaling for any COM objects created between the two, degrading performance and potentially causing deadlocks.
- **Fix:** Changed to `CoInitializeEx(NULL, COINIT_MULTITHREADED)`
- **Also added** `COINIT_DISABLE_OLE1DDE` for consistency with peer DLLs.

### 2. uxctl.dll — FIXED
- **File:** `src/uxctl/dllmain.cpp:39`
- **Current:** `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)`
- **Assessment:** Companion to UXCore; same apartment model required.
- **Fix:** Changed to `CoInitializeEx(NULL, COINIT_MULTITHREADED)`

### 3. WLXFaceRecognition.dll — FIXED
- **File:** `src/WLXFaceRecognition/dllmain.cpp:30`
- **Current:** `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)`
- **Assessment:** CROSS_BINARY_MASTER.md Section G identifies WLXFaceRecognition as **MTA** ("Background detection threads, CRITICAL_SECTION, Events"). The STA initialization is incorrect for a DLL that spawns worker threads for face detection.
- **Fix:** Changed to `COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE`

### 4. WLXSlideshow.dll — FIXED
- **File:** `src/WLXSlideshow/dllmain.cpp`
- **Current:** No COM initialization at all
- **Assessment:** WLXSlideshow hosts COM objects and is loaded by MovieMakerCore (MTA). Missing COM initialization could cause `CO_E_NOTINITIALIZED` errors if any COM operations occur during DLL_PROCESS_ATTACH.
- **Fix:** Added `CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE)` at DLL_PROCESS_ATTACH and `CoUninitialize()` at DLL_PROCESS_DETACH.

### 5. WLXMediaPublishSubscribe.dll — FIXED
- **File:** `src/WLXMediaPublishSubscribe/dllmain.cpp:34`
- **Current:** `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)`
- **Assessment:** CROSS_BINARY_MASTER.md Section G identifies WLXMediaPublishSubscribe as **MTA** ("Grinder scheduler workers, Via IGrinderTaskScheduler"). It hosts 9+ COM objects and uses background worker threads for publishing operations.
- **Fix:** Changed to `COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE`

### 6. WLXPhotoCinematic.dll — FIXED
- **File:** `src/WLXPhotoCinematic/dllmain.cpp`
- **Current:** No COM initialization at all
- **Assessment:** WLXPhotoCinematic hosts 2 COM objects (CinematicFullScreen, PanZoomTransform) registered with ThreadingModel=Apartment. It is loaded by MovieMakerCore (MTA). Missing COM init could cause failures in COM-dependent operations during DLL init. The 2 COM objects use the Apartment model per CLSID registration, which is compatible with MTA as long as the objects handle cross-apartment marshaling.
- **Fix:** Added `CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE)` at DLL_PROCESS_ATTACH and `CoUninitialize()` at DLL_PROCESS_DETACH.

### 7. WLXVideoTrim.dll — NO CHANGE
- **File:** `src/WLXVideoTrim/dllmain.cpp:31`
- **Current:** `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)`
- **Assessment:** WLXVideoTrim hosts DirectShow filters that require STA for filter graph operations. The analysis confirms "Single-threaded per operation" (Section G). This is correct for DirectShow filters.

### 8. WLXMP4Parser.dll — NO CHANGE
- **File:** `src/WLXMP4Parser/dllmain.cpp`
- **Current:** No COM initialization
- **Assessment:** This DLL is loaded by COM (CoCreateInstance) when a DirectShow/Media Foundation graph needs MP4 parsing. COM initialization is handled by the caller's thread. No COM operations are performed during DllMain.

---

## Cross-Binary Impact

The mismatches fixed above (items 1-6) all involved DLLs using STA while the primary host (MovieMakerCore) uses MTA. This inconsistency would cause:

1. **Cross-apartment marshaling overhead** — COM must create proxies/stubs when passing interfaces between STA and MTA
2. **Potential deadlocks** — STA-to-MTA calls require the STA thread to pump messages; if it's busy in DllMain, deadlock occurs
3. **CoInitializeEx failure** — If a thread calls CoInitializeEx with conflicting models, it returns RPC_E_CHANGED_MODE

All fixes move the DLLs from STA → MTA, consistent with the CROSS_BINARY_MASTER.md analysis and the host application's threading model.

---

## Files Edited

| # | File | Change |
|---|------|--------|
| 1 | `src/UXCore/dllmain.cpp` | `COINIT_APARTMENTTHREADED` → `COINIT_MULTITHREADED`, added `COINIT_DISABLE_OLE1DDE` |
| 2 | `src/uxctl/dllmain.cpp` | `COINIT_APARTMENTTHREADED` → `COINIT_MULTITHREADED` |
| 3 | `src/WLXFaceRecognition/dllmain.cpp` | `COINIT_APARTMENTTHREADED` → `COINIT_MULTITHREADED` |
| 4 | `src/WLXSlideshow/dllmain.cpp` | Added COM init (`COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE`) + CoUninitialize |
| 5 | `src/WLXMediaPublishSubscribe/dllmain.cpp` | `COINIT_APARTMENTTHREADED` → `COINIT_MULTITHREADED` |
| 6 | `src/WLXPhotoCinematic/dllmain.cpp` | Added COM init (`COINIT_MULTITHREADED \| COINIT_DISABLE_OLE1DDE`) + CoUninitialize |
