---
sidebar_position: 3
title: Preserved Quirks (23)
description: The original bugs, quirks, and anomalies deliberately reproduced — the faithful-reconstruction checklist.
---

# Preserved Quirks

This is the condensed index of all 23 quirks documented in
[`QUIRKS.md`](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/blob/main/QUIRKS.md).
These are **not bugs in our recreation** — they are faithful reproductions of the
original binary's behavior. If you're building a "remix" with fixes, use this (and
`QUIRKS.md`) as the checklist.

:::warning Do not "fix" these

Recreating, keeping, and extending the codebase to match these behaviors is **correct
and expected**. Only change them when an explicit remix/fix pass is requested.

:::

## Exception & crash handling

### 1. Double-Exception Protection Pattern (VEH + SEH)

`src/MovieMaker/main.cpp:14-30` — The launcher registers a Vectored Exception Handler
catching C++ exception code `0xe06d7363` and subcodes `0x19930520` (`__CT typeName`),
`0x19930521` (bad_typeid), `0x19930522` (bad_cast), plus the undocumented `0x01994000`.
The VEH returns `EXCEPTION_EXECUTE_HANDLER`, then the outer `__except` catches the
unwind — silently swallowing RTTI failures that should probably be fatal.

### 8. Exception Objects Use Process Heap (Not CRT Heap)

`src/WLXPhotoBase/WLXPhotoBase.cpp:179-199` — `Exception::operator new` allocates from
`GetProcessHeap()` and raises `EXCEPTION_NONCONTINUABLE_EXCEPTION` on failure instead of
throwing `std::bad_alloc`. Mixing heap allocators can crash if CRT frees process-heap
memory.

### 21. HMREngine Error Handling Silently Swallows Failures

HMREngine logs-and-continues on non-fatal failures — failures disappear rather than
propagate.

### 22. Null Pointer Dereference in Transport Controls

A reachable null-deref path in the transport controls (specific seek-while-stopped
sequence). Reproduced faithfully; surrounding behavior is contract-pinned.

## Identity & branding

### 2. "Sundance" Codename Leaked Into Runtime Objects

`MovieMakerCore.cpp:427-432` — the single-instance mutex is named
`Global\WindowsLiveMovieMaker_Sundance_SingleInstance` (visible across all Terminal
Services sessions; the internal codename persists in a system object). Any program
creating this mutex blocks Movie Maker from launching.

### 7. "SkyDrive" Branding (Historical Anachronism)

`UIBehaviorClasses.h` — `PublishDialogSkyDriveBehavior` targets "SkyDrive", renamed to
OneDrive in 2014. The publish dialog targets a service that no longer exists under this
name.

## COM & GUID oddities

### 3. Null CLSID for MovieMakerCore.dll

`src/common.h:163` — `CLSID_MovieMakerCore` is all zeros. The DLL is loaded via
`LoadLibrary`/`GetProcAddress`, so it's never used — but COM-registering it would
register a null GUID.

### 4. Deterministic Pseudo-GUIDs for COM Objects

`ComFactory.h:33-47` — hand-crafted sequential placeholder GUIDs
(`{A1B2C3D4-E5F6-...}`) for reconstructed COM classes; the originals were never
extracted. They will conflict with any real COM registration.

### 16. IsolationAwareTaskDialog

`External/UIControlsStub.h` — RTTI `?AVIsolationAwareTaskDialog@@` references a task
dialog type aware of manifest-based side-by-side isolation that doesn't exist in modern
SDKs.

## Rendering & UI

### 6. Removed Win10 Ribbon SDK APIs

`UI/Ribbon/RibbonApp.cpp` — the original used `IUIFramework::RegisterUICommand`,
`UI_VIEWVERB_EXECUTE`, and `UI_PKEY_*` keys removed from the Windows 10 SDK;
`IID_IUICommandHandler` had to be defined manually. The Ribbon integration silently
fails where the APIs are missing — as the original would today.

### 9. D3D11 Single-Threaded Flag for a Video Editor

`dllmain.cpp` — D3D11 device creation passes the single-threaded flag (no
`D3D11_CREATE_DEVICE_THREAD_SAFE`) despite the app being multi-threaded: a latent
race the original shipped with.

### 12. Vista Basic Ribbon Workaround

`src/WTL/atlribbon.h:2215` — `SetWindowRgn(NULL, TRUE)` works around a Vista Basic
theme rendering bug; a no-op on modern Windows that wastes a call.

### 13. 118 Dummy Texture Codec Classes

`HMREngine/DXResources/TextureCodecs.h` — ~118 D3DX11 texture codec classes existing
solely to generate unique RTTI entries; `Encode`/`Decode` return `E_NOTIMPL`. Binary
bloat with no functional purpose.

### 17. Custom STL-like Container Library

`DataStructs/IntSet.h` — a custom STL-like container library (`IntSet`, `StringSet`)
instead of CRT STL, duplicating container functionality with different semantics.

## Architecture choices

### 10. Non-Contiguous Command IDs

`MovieMakerCore.cpp:201-213` — command IDs with large gaps
(`ID_APP_IMPORT 0xE101`, `ID_APP_UNDO 0xE12B`, `ID_APP_DELETE 0xE200`, ...),
likely IDs of removed features never reclaimed.

### 11. Delay-Loaded DLLs That No Longer Exist

`WLXMediaPublishSubscribe.cpp:11-15` — `WLXPhotoSqm.dll`, `DmxBici.dll`, `wlidcli.dll`,
`uxcore.dll` are delay-loaded but not shipped on modern Windows; without delay-load
exception handling the app would crash at startup.

### 14. Local Classes Nested Inside Methods (RTTI)

`SundanceAppMain.cpp:39-44` — RTTI `@?1??` names reveal C++03-style "lambda objects"
(local classes inside member functions) — lambdas didn't exist in the original's
toolchain.

### 15. Global Singleton Architecture

`dllmain.cpp:66`, `SundanceAppMain.cpp:50` — `CAppModule _Module` and
`g_pSundanceAppMain` wire the entire app through file-scope singletons with no
dependency injection: hard to unit-test, hidden coupling.

### 18. 92 Transition/Effect Animation Classes

`src/WLXPipetran/WLXPipetran.h:4-7` — 92 RTTI transition/effect classes
(`StarWipeTransition`, `BowTieWipeTransition`, `ClockWipeTransition`,
`PixelateTransition`, ...), of which **22 were never fully implemented** in the original
— shipped as stubs.

### 20. WARP Fallback as Default Rendering Path

`dllmain.cpp:200-243` — initialization tries hardware D3D11, then falls back to WARP
with feature levels 11.1 → 9.1. Without hardware support the entire pipeline runs in
software — extremely slow for video editing.

## Legacy & leaks

### 19. Legacy Project Format Support

`UI/Legacy/LegacyProject.cpp` etc. — full backward compatibility with an older project
format (`LegacyExtent`, `LegacyTextExtent`, `ComplexProperty`, `SingleProperty`,
`TransformProperty`) — dead code in the 2012 version, but still compiled and shipped.

### 23. Memory Leak in Serialization Writer

The `.wlmp` serialization writer leaks memory on an error path — reproduced faithfully
(the contract suite documents the non-leaking paths around it).

### 5. HMREngine Redefines Windows SDK Error Macros

`HMREngine/HMREngine.h:111-182` — the engine temporarily redefines SDK error macros and
restores them. Never add includes in that span; it breaks the restore logic.

---

## How to use this list

- **Contributors:** treat these as pinned behavior. If a change alters one of them,
  that's a remix decision, not a bug fix.
- **Remixers:** each quirk above is a candidate fix — read the full entry in `QUIRKS.md`
  (file/line references included) before changing it, and check the contract suite for
  pinned behavior around it.
