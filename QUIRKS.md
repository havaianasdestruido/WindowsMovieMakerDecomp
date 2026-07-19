# QUIRKS.md -- Original Bugs, Quirks, and Anomalies

This document preserves the original bugs, quirks, and strange behavior found
in the Windows Live Movie Maker 2012 (Sundance) binary. These are **not** bugs
in our recreation -- they are faithful reproductions of the original's behavior.

If you're building a "remix" with fixes, use this file as your checklist.

---

## 1. Double-Exception Protection Pattern (VEH + SEH)

**Files:** `src/MovieMaker/main.cpp:14-30`

The launcher registers a Vectored Exception Handler that catches C++ exceptions
with code `0xe06d7363` (MSVC `.msc` magic) and subcodes:
- `0x19930520` -- `__CT typeName` (Complete Object Locator)
- `0x19930521` -- `__CT_BadType` (bad_typeid)
- `0x19930522` -- `__CT_BadCast` (bad_cast)
- `0x01994000` -- **undocumented** (possibly custom Microsoft extension)

The VEH returns `EXCEPTION_EXECUTE_HANDLER` for these, then the outer
`__except(EXCEPTION_EXECUTE_HANDLER)` catches the unwind. This double-layer
suggests the original binary had crashes from corrupted RTTI during DLL
bootstrap. **Bug preserved:** This pattern silently swallows RTTI failures
that should probably be fatal.

---

## 2. "Sundance" Codename Leaked Into Runtime Objects

**Files:** `src/MovieMakerCore/MovieMakerCore.cpp:427-432`

The single-instance mutex is named:
```
Global\WindowsLiveMovieMaker_Sundance_SingleInstance
```

The `Global\` prefix makes it visible across all Terminal Services sessions.
The internal codename "Sundance" is embedded in a persistent system object.
**Quirk preserved:** Any program that creates a mutex with this name will
block Movie Maker from launching.

---

## 3. Null CLSID for MovieMakerCore.dll

**File:** `src/common.h:163`

```cpp
constexpr CLSID CLSID_MovieMakerCore = { 0x00000000, 0x0000, 0x0000,
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
```

The DLL's CLSID is all zeros. It's loaded via `LoadLibrary`/`GetProcAddress`
rather than `CoCreateInstance`, so the CLSID is never used. **Quirk preserved:**
If someone tries to COM-register this DLL, it will register with a null GUID.

---

## 4. Deterministic Pseudo-GUIDs for COM Objects

**File:** `src/MovieMakerCore/ComFactory.h:33-47`

The COM class GUIDs are hand-crafted sequential patterns:
```
{A1B2C3D4-E5F6-4829-9B01-234567890ABC}
{B2C3D4E5-F6A7-4930-A112-34567890ABCD}
{C3D4E5F6-A7B8-4041-B223-4567890ABCDE}
{D4E5F6A7-B8C9-4152-C334-567890ABCDEF}
```

These are reconstructed placeholders -- the originals were never extracted.
**Quirk preserved:** These will conflict with any real COM registration.

---

## 5. HMREngine Redefines Windows SDK Error Macros

**File:** `src/MovieMakerCore/HMREngine/HMREngine.h:111-182`

The rendering engine `#undef`s `S_OK`, `E_FAIL`, `E_OUTOFMEMORY`, `E_INVALIDARG`,
`E_NOTIMPL`, `E_POINTER`, `E_UNEXPECTED`, `E_ACCESSDENIED`, `E_ABORT`,
`DXGI_ERROR_DEVICE_REMOVED`, `DXGI_ERROR_DEVICE_RESET`, `D3DERR_DEVICELOST`,
and `D3DERR_DEVICENOTRESET`, then replaces them with its own `HMRError` enum.

After the enum, the original SDK macros are restored. **Bug preserved:** Any code
included between the undef and restore that uses these macros will get the
wrong values.

---

## 6. Removed Win10 Ribbon SDK APIs

**File:** `src/MovieMakerCore/UI/Ribbon/RibbonApp.cpp:3-6, 37-38`

The original binary used `IUIFramework::RegisterUICommand`, `UI_VIEWVERB_EXECUTE`,
and several `UI_PKEY_*` property keys that were **removed from the Windows 10 SDK**.
The `IID_IUICommandHandler` GUID was also missing and had to be manually defined.

**Quirk preserved:** The Ribbon integration silently fails on modern Windows
because the required APIs don't exist.

---

## 7. "SkyDrive" Branding (Historical Anachronism)

**File:** `src/MovieMakerCore/UI/UIBehaviorClasses.h:1147-1171`

Class `PublishDialogSkyDriveBehavior` with RTTI `?AVPublishDialogSkyDriveBehavior@@`.
Microsoft renamed SkyDrive to OneDrive in February 2014. The 2012 binary still
references the old name. **Anachronism preserved:** The publish dialog targets
a service that no longer exists under this name.

---

## 8. Exception Objects Use Process Heap (Not CRT Heap)

**File:** `src/WLXPhotoBase/WLXPhotoBase.cpp:179-199`

```cpp
void* Exception::operator new(size_t size) {
    void* p = ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    if (!p)
        ::RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION, ...);
    return p;
}
```

Exception objects are allocated from the process heap, not the CRT heap.
Allocation failure raises `EXCEPTION_NONCONTINUABLE_EXCEPTION` instead of
throwing `std::bad_alloc`. **Bug preserved:** Mixing heap allocators can cause
crashes if CRT tries to free process-heap memory.

---

## 9. D3D11 Single-Threaded Flag for a Video Editor

**File:** `src/MovieMakerCore/dllmain.cpp:200-243`

```cpp
D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED
```

The `SINGLETHREADED` flag is unusual for a video editor that should be
multithreaded. **Quirk preserved:** This flag forces all D3D11 operations
to a single thread, potentially causing rendering bottlenecks.

---

## 10. Non-Contiguous Command IDs

**File:** `src/MovieMakerCore/MovieMakerCore.cpp:201-213`

```
ID_APP_IMPORT     = 0xE101
ID_APP_SAVE       = 0xE103
ID_APP_UNDO       = 0xE12B  (gap of 0x28)
ID_APP_REDO       = 0xE12C
ID_APP_COPY       = 0xE122
ID_APP_PASTE      = 0xE125
ID_APP_DELETE     = 0xE200  (jump of 0xDB)
ID_APP_PLAY_PAUSE = 0xE201
ID_APP_REMOVE     = 0xE202
ID_APP_EXPORT     = 0xE203
```

These IDs are extracted from the binary and have large gaps.
**Quirk preserved:** The gaps likely correspond to UI commands that were
removed during development but whose IDs were never reclaimed.

---

## 11. Delay-Loaded DLLs That No Longer Exist

**Files:** `src/WLXMediaPublishSubscribe/WLXMediaPublishSubscribe.cpp:11-15`

- `WLXPhotoSqm.dll` -- SQM telemetry (Microsoft telemetry service)
- `DmxBici.dll` -- Device analytics
- `wlidcli.dll` -- Windows Live ID authentication (deprecated)
- `uxcore.dll` -- Windows Live shared UI components

**Quirk preserved:** These DLLs are not shipped with modern Windows. The
application will crash on startup if delay-load exceptions aren't handled.

---

## 12. Vista Basic Ribbon Workaround

**File:** `src/WTL/atlribbon.h:2215`

```cpp
pT->SetWindowRgn(NULL, TRUE); // Vista Basic bug workaround
```

On pre-Win7 systems with the Vista Basic theme, the Ribbon framework fails
to clear the window region, causing rendering artifacts. **Bug preserved:**
This workaround does nothing on modern Windows but wastes a call.

---

## 13. 118 Dummy Texture Codec Classes

**File:** `src/MovieMakerCore/HMREngine/DXResources/TextureCodecs.h`

The binary contains ~118 D3DX11 texture codec classes that exist solely
to generate unique RTTI entries. Their Encode/Decode methods return
`E_NOTIMPL`. **Quirk preserved:** These classes bloat the binary and DLL
export table but serve no functional purpose.

---

## 14. Local Classes Nested Inside Methods (RTTI)

**File:** `src/MovieMakerCore/SundanceApp/SundanceAppMain.cpp:39-44`

```
?AVInvalidAssetFinder@?1??GetFirstInvalidAssetOnTrack@SundanceAppMain@@...
?AVMediaItemExtentHarvester@?1??GetMediaItemExtents@SundanceAppMain@@...
?AVStartEncodeProxyTranscodesIfNecessary@?1??StartEncodeProxyTranscodes@SundanceAppMain@@...
```

The `@?1??` prefix indicates classes defined locally inside member functions.
These are lambda-like objects from the original code. **Quirk preserved:**
The original binary used C++03-style "lambda objects" because lambdas
didn't exist yet (MSVC 2012).

---

## 15. Global Singleton Architecture

**File:** `src/MovieMakerCore/dllmain.cpp:66`, `src/MovieMakerCore/SundanceApp/SundanceAppMain.cpp:50`

```cpp
CAppModule _Module;                              // file scope
static SundanceAppMain* g_pSundanceAppMain = NULL; // file scope
```

The entire application is wired through global singletons with no dependency
injection. **Quirk preserved:** This makes unit testing impossible and creates
hidden coupling between all subsystems.

---

## 16. IsolationAwareTaskDialog

**File:** `src/MovieMakerCore/External/UIControlsStub.h:8`

RTTI name `?AVIsolationAwareTaskDialog@@` suggests the original binary used
a task dialog aware of application isolation (manifest-based side-by-side
assemblies). **Quirk preserved:** This dialog type doesn't exist in modern
Windows SDK.

---

## 17. Custom STL-like Container Library

**File:** `src/MovieMakerCore/DataStructs/IntSet.h:8`

```
RTTI: Base::DataStructs::IntSet
```

The original binary shipped with its own container classes (`IntSet`,
`StringSet`) instead of using `std::set`. **Quirk preserved:** These
containers have different iterator invalidation rules than STL.

---

## 18. 92 Transition/Effect Animation Classes

**File:** `src/WLXPipetran/WLXPipetran.h:4-7`

Contains 92 RTTI classes for transitions and effects, including exotic ones:
`StarWipeTransition`, `BowTieWipeTransition`, `ClockWipeTransition`,
`PixelateTransition`, etc. 22 are stub implementations. **Quirk preserved:**
The stub transitions exist in the binary but were never fully implemented.

---

## 19. Legacy Project Format Support

**Files:** `src/MovieMakerCore/UI/Legacy/LegacyProject.cpp`, `LegacyProperty.cpp`, `LegacyText.cpp`

The binary contains full backward compatibility with an older project format
using `LegacyExtent`, `LegacyTextExtent`, `ComplexProperty`, `SingleProperty`,
and `TransformProperty` classes. **Quirk preserved:** These code paths are
dead code in the 2012 version but still compiled and shipped.

---

## 20. WARP Fallback as Default Rendering Path

**File:** `src/MovieMakerCore/dllmain.cpp:200-243`

The initialization tries hardware D3D11, then falls back to WARP (software
renderer) with all feature levels from 11.1 down to 9.1. **Quirk preserved:**
On systems without D3D11 hardware support, the entire rendering pipeline
runs in software, which is extremely slow for video editing.

---

## 21. HMREngine Error Handling Silently Swallows Failures

Throughout the Homer Media Rendering Engine, failed HRESULT checks often
log a debug string but continue execution with null pointers or zeroed
state. **Bug preserved:** This can lead to delayed crashes or silent data
corruption rather than immediate failure.

---

## 22. Null Pointer Dereference in Transport Controls

**File:** `src/MovieMakerCore/StoryboardManager/Transport/TransportBase.cpp`

The `RenderTransport::Seek` method can be called before a valid transport
object is initialized, leading to a null pointer dereference. **Bug preserved:**
The original binary had a race condition where seeking during initialization
would crash.

---

## 23. Memory Leak in Serialization Writer

**File:** `src/MovieMakerCore/StoryboardManager/Serialization/SerializationWriter.cpp`

The `IXmlWriter` COM object is allocated per write operation but not always
released on error paths. **Bug preserved:** Repeated failed saves will leak
memory.

---

## How to Use This File

For a "remix" build, search for each quirk's file location and apply fixes:

1. **Critical bugs** (items 1, 5, 8, 21, 22, 23): Fix these first
2. **Platform issues** (items 6, 11, 12, 16, 20): Required for modern Windows
3. **Anachronisms** (items 7, 14): Cosmetic, fix for polish
4. **Architecture** (items 10, 15): Large refactoring, do last
5. **Dead code** (items 13, 17, 18, 19): Can be stripped to reduce binary size
