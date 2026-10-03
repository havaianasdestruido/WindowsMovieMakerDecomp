---
sidebar_position: 1
title: MovieMaker.exe (Launcher)
description: The thin launcher executable — VEH handler, LoadLibrary, and the single __cdecl call into the engine.
---

# MovieMaker.exe

| | |
|---|---|
| **Source** | `src/MovieMaker/` |
| **CMake target** | `MovieMaker` |
| **Type** | Win32 EXE (launcher), ~94 KB |
| **Family** | [Application targets](../application) |

## At a glance

A deliberately tiny launcher. In the original binary it is 54 KB with ~4.8 KB of code and
**zero exports**; all real code lives in `MovieMakerCore.dll`. The launcher's only job:
load the engine and call one function.

## Public surface

None. `MovieMaker.exe` is not a library — it is the process entry.

## Startup sequence

```cpp
// src/MovieMaker/main.cpp (reconstructed shape)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    wchar_t dir[MAX_PATH]; GetModuleFileNameW(...); /* strip file */
    LoadLibraryExW(dir + L"WLXPhotoBase.dll", ...);       /* optional bootstrap */
    HMODULE hCore = LoadLibraryExW(dir + L"MovieMakerCore.dll", ...);
    auto main = (int(__cdecl*)(int, wchar_t**))GetProcAddress(hCore, "MovieMakerMain");
    PVOID veh = AddVectoredExceptionHandler(0, VexHandler);   // see Quirk #1 —
                                                              // registered only now
    int ret = 0;
    __try { ret = main(argc, argv); }
    __except (EXCEPTION_EXECUTE_HANDLER) { ret = 3; }
    if (veh) RemoveVectoredExceptionHandler(veh);
    return ret;
}
```

Notable preserved behaviors:

- **VEH + SEH double-exception layer** for MSVC C++ exception codes (`0xe06d7363`,
  subcodes `0x19930520/21/22`, and the undocumented `0x01994000`) —
  [Quirk #1](../../methodology/quirks.md#1-double-exception-protection-pattern-veh--seh).
- **Static CRT** linking and **delay-load** of `MovieMakerCore.dll` — the launcher can
  start (and fail gracefully) even if the engine DLL is missing.
- The original icon/resources are reproduced in `moviemaker.rc`.

## Build notes

- Uses the **static** MSVC runtime (`MultiThreaded`), unlike the DLLs
  (`MultiThreadedDLL`) — matching the original link setup.
- Links almost nothing: `kernel32`, `user32`, `shell32` — everything else happens inside
  the engine.

## Testing

- CTest sanity check `sanity_MovieMaker_exe_exists` verifies the output exists.
- `tools/test_all.py --launch` launches the exe, observes process state, window creation,
  and stability (see [Verification Tools](../../tools/build-tools.md)).
- `apps/moviemaker-launcher` (submodule) exercises the launcher contract.

## Analysis artifacts

`analysis/MovieMakerExe/` — PE structure, imports, strings of the original launcher
(PDB GUID `{47558454-9C62-4123-96E9-91A66E8F4D87}`).
