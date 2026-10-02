---
sidebar_position: 3
title: Code Conventions
description: Language, headers, namespaces, CMake patterns, and error-handling rules used across the reconstruction.
---

# Code Conventions

The repo has **no lint/format tooling by design** — match the existing style by eye. This
page is the by-eye reference. (The canonical agent-facing version lives in `AGENTS.md`.)

## Language & idiom

- **C++14**, MSVC 11-era idiom dominates: ATL (`CAppModule`), WTL, `HRESULT` returns,
  PascalCase methods, Hungarian-ish members (`m_` / `g_` / `s_` prefixes).
- **Do not modernize wholesale** — the code deliberately reads like 2012.

## Headers

- `#pragma once` everywhere; include guard additionally in `common.h`.
- `src/common.h` is the de-facto precompiled set (Windows/COM/GDI+/DX/Media
  Foundation + C++ std).
- `src/MovieMakerCore/pch.h` for the engine.

## Namespaces & version macros

- `WMMR` for shared reconstruction utilities: `WMMR::ComPtr` (= `CComPtr`),
  `WMMR::MovieMakerException`, `WMMR::CoInitializer`, `WMMR::GdiplusInit`,
  `WMMR::Succeeded/Failed`.
- Most module code uses the original ATL/global style (no namespace) — matching the
  binaries.
- Version macros from `src/common.h` (`WMMR_VERSION_*`), never hardcoded.

## CMake pattern (per module)

`src/WLXVideoTrim/CMakeLists.txt` is the template:

```cmake
project(<Name> LANGUAGES CXX)
add_library(${TARGET} SHARED <sources> <Name>.def)
set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME "<Name>"          # no PREFIX/SUFFIX munging
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
target_include_directories(${TARGET} PRIVATE src src/WTL)
target_compile_definitions(...)
target_link_libraries(${TARGET} PRIVATE <explicit system libs>)
```

Exceptions: the launcher (`src/MovieMaker`) uses the **static** runtime and delay-loads
`MovieMakerCore.dll`; helper object libs use `$<TARGET_OBJECTS:...>`.

## Export surface rules

- Most modules export via a `.def` file (`Name @ordinal`); DmxBici / WLXPhotoBase /
  WLXPhotoSqm list C++ mangled names; UXCore uses `__declspec(dllexport)`.
- **Every DLL must match the reference export names, ordinals, and decorations** —
  [export parity is mandatory](../methodology/stub-design.md#ground-rules). Run
  `tools/diff_exports.py` after touching exports.

## Error handling

- `HRESULT` from every COM/factory/export surface; check with `SUCCEEDED`
  (`WMMR::Succeeded/Failed` for readability).
- **`S_FALSE` is intentional** for true "no-op / already-correct" outcomes — do not
  change it arbitrarily.
- Throw `WMMR::MovieMakerException` (carries its HRESULT) or `ATLASSERT` for
  programmer errors; `Base::Throw`/`ThrowLastError` in WLXPhotoBase land.
- Module-specific HRESULT spaces exist (e.g. WLXVideoTrim's `FACILITY_WLXVIDEOTRIM` /
  `AVS_E_*` codes) — match them.
- Respect the [HMREngine macro-redefinition zone](../architecture/moviemakercore/hmr-engine.md#d3dx11-compatibility-zone).
- Never swallow the VEH pattern or the delay-load survival logic.

## Comments

Match the existing "recreation notes" style — reference the original binary where
relevant (RTTI name, export ordinal, quirk number). No gratuitous comments.

## Submodules & vendoring

- Submodule output is never vendored into the parent; submodule changes are committed in
  the submodule, plus a gitlink bump here.
- `src/WTL/` is vendored third-party — keep pristine.
- System libs only — no third-party runtime dependencies.
