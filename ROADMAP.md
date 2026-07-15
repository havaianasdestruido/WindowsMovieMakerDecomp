# Project Summary & Resume Roadmap
## Windows Live Movie Maker 2012 - Source Code Recreation

> **Last updated:** Current session
> **Build command:** `"C:\Program Files\CMake\bin\cmake.exe" --build build --config Debug`
> **Project root:** `C:\Users\mcmco\Desktop\WMMR`

---

## What This Project Is

Recreating the full source code of Windows Live Movie Maker 2012 (`MovieMakerCore.dll` + 17 supporting DLLs + launcher EXE) from binary analysis. The originals are in `undecomp/` (gitignored). The reconstructed source is in `src/`.

**Key facts:**
- Original binary: MSVC 11.0 (VS2012), C++14, ATL/WTL, targets Windows 6.2+
- Build environment: MSVC 19.44 (VS2022 Build Tools), SDK 10.0.26100.0
- CMake generator: `Visual Studio 17 2022`, architecture: **Win32** (x86)
- Project codename: **Sundance**, version **16.4.3528.0331**
- Single DLL export: `MovieMakerMain` from MovieMakerCore.dll
- 1360 RTTI classes across 48 namespaces
- 375 source files, 18 CMake targets

---

## Current Build State

### What Compiles Successfully
- MovieMakerLang.dll (output: `build/bin/Debug/MovieMakerLang.dll`)
- MovieMaker.exe (output: `build/bin/Debug/MovieMaker.exe`)
- All 16 supporting DLLs (WLXPhotoBase, DuiDirect, DmxBici, MetadataSys, etc.)
- Most MovieMakerCore .cpp files (approximately 80 out of ~100)

### What Fails (Root Causes Identified)

There are **3 distinct root causes** remaining. Once these are fixed, the build should succeed.

---

### Fix 1: Add `#include "pch.h"` to .cpp files missing it (HIGH PRIORITY)

**Problem:** Several .cpp files in HMREngine and HMRAVSource subdirectories do NOT include `pch.h` as their first include. Without the PCH, headers like `DirectXMath.h` fail to compile, causing cascading errors (Vec2 undeclared, namespace syntax errors, etc.).

**Files that need `#include "pch.h"` added as line 1:**
- `src/MovieMakerCore/HMREngine/PatternMesh/PatternMeshFactory.cpp`
- `src/MovieMakerCore/HMREngine/PatternMesh/PatternMesh.cpp`
- `src/MovieMakerCore/HMREngine/TextRender/TextRenderPipeline.cpp`
- `src/MovieMakerCore/HMRAVSource/ImageThumbnail.cpp`

**Check all .cpp files:** Run this to find any others:
```powershell
Get-ChildItem "src\MovieMakerCore" -Recurse -Filter "*.cpp" | ForEach-Object {
    $first = (Get-Content $_.FullName -First 1).Trim()
    if ($first -ne '#include "pch.h"' -and $_.Name -ne 'pch.cpp') {
        Write-Output $_.FullName
    }
}
```

### Fix 2: HMREngine.h line 113 syntax error

**Problem:** `src/MovieMakerCore/HMREngine/HMREngine.h` line 113 has a syntax error: `error C2059: syntax error: '('`. The HMRError enum likely uses C++11 `= 0` initializers inside an enum, which should work in C++14, but there may be a missing semicolon or a conflicting macro.

**Action:** Read `HMREngine.h` lines 100-130 and fix the enum definition. Look for:
- Missing semicolons after enum values
- Macros expanding to unexpected tokens
- The `= 0` assignments conflicting with something

### Fix 3: X3DFieldTypes.h `m_value` access through CComObject

**Problem:** In `src/MovieMakerCore/HMREngine/X3DDFieldTypes.h`, the `Clone()` methods do:
```cpp
CComObject<SFVec2f>* p;
CComObject<SFVec2f>::CreateInstance(&p);
p->m_value = m_value;  // ERROR: m_value not accessible through CComObject
```

`CComObject<T>` wraps `T` but does NOT expose `T`'s members through `->`. You need to use the `CComObject<T>::m_pInstance` member or restructure.

**Fix options (choose one):**
1. Change `p->m_value = m_value;` to use a different approach
2. Restructure Clone to not use CComObject
3. Add a `SetValue()` method on the derived classes

**Affected classes:** SFVec2f, SFVec3f, SFVec4f, SFRotation, SFColor, SFColorRGBA, SFMatrix3f, SFMatrix4f, SFNode, and all MF* (MultipleField) types.

---

## Step-by-Step Resume Plan

### Step 1: Fix pch.h includes (15 min)
1. Open each .cpp file listed in Fix 1
2. Add `#include "pch.h"` as the VERY FIRST line (before any other includes or #pragma once)
3. For files that already have `#include "pch.h"` elsewhere, move it to line 1
4. Verify with the PowerShell script above that no .cpp files are missing pch.h

### Step 2: Fix HMREngine.h enum (5 min)
1. Read `src/MovieMakerCore/HMREngine/HMREngine.h` lines 100-130
2. The HMRError enum likely looks like:
   ```cpp
   enum HMRError {
       Success = 0,
       DeviceLost = 1,
       // etc.
   };
   ```
3. Check if there's a problematic macro or syntax issue
4. Fix and verify

### Step 3: Fix X3DFieldTypes.h Clone pattern (30 min)
1. Read `src/MovieMakerCore/HMREngine/X3DFieldTypes.h`
2. Every `Clone()` method that does `p->m_value = m_value;` needs to be fixed
3. The pattern should be changed to something like:
   ```cpp
   HRESULT Clone(X3DFieldNode** pp) const override
   {
       CComObject<SFVec2f>* p;
       CComObject<SFVec2f>::CreateInstance(&p);
       p->SetValue(m_value);  // Use SetValue instead of direct access
       p->AddRef();
       *pp = p;
       return S_OK;
   }
   ```
   Or restructure to not use CComObject at all.

### Step 4: Rebuild & Iterate (30 min)
1. Run: `"C:\Program Files\CMake\bin\cmake.exe" --build build --config Debug 2>&1`
2. Check for remaining errors
3. Fix any remaining issues (likely just cascading errors from the fixes above)
4. Repeat until build succeeds

### Step 5: Commit (when user asks)
1. Run `git status` and `git diff`
2. Stage all new/modified files
3. Commit with descriptive message

---

## Detailed File Map

### Source Structure
```
src/
  common.h                          # Shared PCH-like header (all Windows SDK, ATL, GDI+)
  exports.h                         # DLL export/import macros
  WTL/                              # WTL 10 headers (20 files from NuGet)
  
  MovieMaker/                       # Launcher EXE target
    main.cpp                        # WinMain entry, loads MovieMakerCore.dll
    CMakeLists.txt
    
  MovieMakerCore/                   # Main DLL target (~350 files)
    pch.h                           # Master precompiled header (Windows SDK + ATL + WTL + D3D + MF)
    pch.cpp                         # PCH compilation unit
    dllmain.cpp                     # DLL entry point
    MovieMakerCore.cpp/.h/.def      # Core exports
    SundanceApp/                    # Application framework (6 files)
    StoryboardManager/              # Project model (18+ files)
      Theme/                        # Visual themes (4 files)
      MovieEffect/                  # Movie effects (1 file)
      Serialization/                # .wlmp file I/O (9 files)
      MediaItems/                   # Media item management (1 file)
      Background/                   # Background processing (4 files)
      Transport/                    # Playback transport (2 files)
    HMREngine/                      # 3D rendering engine (30+ files)
      DXResources/                  # DX11 resource management (7 files)
      X3DNodeImpls/                 # X3D node implementations (5 files)
      PatternMesh/                  # Video transition mesh (2 files)
      TextRender/                   # Text rendering (1 file)
    HMRAVSource/                    # Media pipeline (25+ files)
      Audio/                        # Audio processing (7 files)
    UI/Ribbon/                      # Windows Ribbon (2 files)
    Preview/                        # Video preview (2 files)
    DataStructs/                    # Utility containers (2 files)
    
  WLXPhotoBase/                     # Foundation library target
    BaseTypes.h                     # Core type definitions
    WLXPhotoBase.h/.cpp             # Main implementation
    
  MovieMakerLang/                   # Localization target
  DuiDirect/                        # DirectUI target
  DmxBici/                          # Analytics target
  MetadataSys/                      # Metadata target
  WLXPhotoSqm/                      # SQM telemetry target
  wlidcli/                          # Live ID target
  (+ 9 more supporting DLL targets)
```

### External Dependencies
```
src/WTL/                            # WTL 10 headers
  atlapp.h, atlcrack.h, atlctrls.h, atlddx.h, atldlgs.h,
  atlframe.h, atlmisc.h, atlprint.h, atlscry.h, atlsplit.h, etc.

System (via VS 2022 Build Tools + SDK 10.0.26100.0):
  ATL:  C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\atlmfc\include\
  SDK:  C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\
```

### Key Compatibility Headers
- `src/MovieMakerCore/HMREngine/d3dx11compat.h` - D3DX11 Effect Framework stub (replaces removed d3dx11.h)
- `src/MovieMakerCore/HMREngine/X3DMath.h` - Math library on DirectXMath (Vec2/3/4, Matrix3/4, Rotation4f, etc.)
- `src/MovieMakerCore/HMREngine/X3DFieldTypes.h` - X3D field type system (COM-based)

---

## Known Gotchas & Notes

1. **`winmm.h` does NOT exist in SDK 10.0.26100.0** - Use `mmsystem.h` instead. This was fixed in both `common.h` and `pch.h`.

2. **`IAtlStringMgr` is in `<atlstr.h>` not `<atlbase.h>`** - WLXPhotoBase.h needs `#include <atlstr.h>`.

3. **ATL 14+ (VS 2022) defines `BaseAtlThrow` natively** - Don't provide custom definitions.

4. **`CAtlArray` copy constructor is private in modern ATL** - Use `.Copy()` method instead.

5. **`CComObject<T>` does NOT expose T's members through `->`** - Need `m_pInstance` or alternative access.

6. **D3DX11 (d3dx11.h) is completely removed from modern SDKs** - Must use `d3dx11compat.h` stub or rewrite effect system.

7. **GDI+ `Status` enum has fewer values in Windows 10 SDK** - Custom values like `UnsupportedImageFormat` don't exist; use standard GDI+ codes.

8. **WTL is NOT bundled with Visual Studio** - Must be obtained separately (NuGet package or manual download).

9. **All .cpp files MUST include pch.h as first line** - Without it, the PCH context is lost and most headers fail.

10. **The build uses x86 (Win32) architecture** - The original binaries are 32-bit. CMake must be configured with `-A Win32`.

---

## Git History
```
edb4713 add Background/Transport/Legacy/Publishing/External + HMRAVSource helpers
0b00926 add Preview, Serialization, Theme, Ribbon, Audio, DXResources, X3DNodeImpls, PatternMesh
1f64d2d add HMRAVSource media pipeline, DataStructs, and resource IDs
ac3851a reconstruct HMREngine, UI behaviors, and all supporting DLLs
728636b reconstruct MovieMakerCore.dll framework, SundanceApp, and StoryboardManager
d246f1  add analysis scripts and initial source reconstruction
d92f6e2 initial: project setup with gitignore and PE analysis script
```

**Recent uncommitted changes:**
- `winmm.h` -> `mmsystem.h` in common.h and pch.h
- Added `#include <atlstr.h>` to WLXPhotoBase.h
- Removed `#include <mfmp2t.h>` from pch.h
- Fixed `struct MovieMakerException` space in common.h
- Created `d3dx11compat.h` D3DX11 compatibility header
- Added `#include "../../Resources/ResourceIds.h"` to EffectResourceDX.cpp
- Added `IDR_COMMON_EFFECT` to ResourceIds.h
- Fixed WLXPhotoBase.h/cpp IAtlStringMgr and BaseAtlThrow issues
- Created `src/MovieMakerCore/pch.cpp` and `src/MovieMakerPreviewClient/dllmain.cpp`

---

## What Success Looks Like

When the build succeeds, we should have:
- `build/bin/Debug/MovieMaker.exe` (launcher)
- `build/bin/Debug/MovieMakerCore.dll` (main engine)
- `build/bin/Debug/MovieMakerLang.dll` (localization)
- 16 supporting DLLs in `build/bin/Debug/`
- All 18 CMake targets compile with 0 errors

After that, the next phase would be:
1. Verify all DLL imports resolve (no missing functions)
2. Test the launcher loads correctly
3. Compare RTTI information between original and rebuilt DLLs
4. Iterate on data structures and string tables to match originals
