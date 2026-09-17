# AGENTS.md — WindowsMovieMakerDecomp

Single canonical instruction file for AI coding agents working in this repo.
Read the whole file before editing anything. Do not treat other docs in this
repo as competing instruction sources; `CLAUDE.md` and `AI.md` point here.

## Project Overview

Recreation of **Microsoft Windows Live Movie Maker 2012** (codename Sundance,
version 16.4.3528.0331) as C++14 / ATL / WTL source, reconstructed from binary
analysis of the original MSVC-11 (VS2012) binaries.

- Original binaries live under `undecomp/` — **gitignored, copyrighted; never
  commit them and never un-ignore them.** The repo may reference them for
  analysis only.
- The reconstructed source lives in `src/`. Each module is behavior- and
  export-compatible with the reference DLL, validated by the contract suite.
- Load-bearing design rules are in `StubDesign.md` — read it before touching
  any DLL surface.
- Faithfully preserved original bugs are in `QUIRKS.md` — **do not "fix" them
  unless the user explicitly asks for a remix/fix pass.**

## Repository Structure

| Path | Contents |
|---|---|
| `src/` | Reconstructed source. 30 module dirs + vendored `WTL/`, 29 module `CMakeLists.txt`. `MovieMakerCore/` is the engine (349 files); most other modules are small (1–4 files). |
| `src/WTL/` | Vendored WTL 10 headers (from NuGet `WTLCSPkg`). Treat as third-party, not authored code. |
| `analysis/` | Gold-mine binary-analysis output, one directory per module (RTTI, exports, cross-DLL imports). Consult before writing real implementations. |
| `tests/` | Test harnesses. `mmr-python` (ctypes contract suite, submodule), `mmr-gui` (WinForms browser, submodule), `mmr-cli` (.NET console twin, submodule) — plus per-module classic C++ test dirs (e.g. `tests/MovieMakerCore/test_core.cpp`). |
| `apps/` | 10 consumer/exercise apps (submodules): photoviewer, transition-plugin, moviemaker-launcher, mediapublisher, video-trimmer, metadata-editor, slideshow-studio, face-tagger, movielibrary, pipeline-graph. |
| `tools/` | `build.ps1` (configure/build wrapper), `diff_exports.py` (export-parity gate), `pe_analyzer.py`, `rtti_extractor.py`, `string_analyzer.py`, `resource_extractor.cpp` (DUI resource tool, own CMake project), `test_all.py` (launches MovieMaker.exe). |
| `python32/` | **Gitignored**, embedded 32-bit CPython 3.11.9 used to run the contract suite. Do not add to git. |
| `build/` | Real build output (`bin/Debug/*.dll|*.exe`). Always the target for tests. |
| `build_clean/`, `build64/`, `out/`, `cool/`, `intro/` | Gitignored scratch/alternate builds. |

Documentation: `README.md` (overview), `ROADMAP.md` (build state + subsystem
inventory), `StubDesign.md` (stub/parity/security rules), `QUIRKS.md` (23 preserved bugs),
`THINKING_PROCESS.md` (reconstruction methodology).

## Architecture & Entry Points

**Application flow:** `MovieMaker.exe` (launcher, `src/MovieMaker/main.cpp`) →
`WinMain` → registers VEH handler → resolves own exe dir → `LoadLibrary`s
`MovieMakerCore.dll` → calls its single export **`MovieMakerMain`** (`__cdecl`).

- `MovieMakerCore.dll` — the engine: SundanceApp (controllers, browser,
  clipboard, undo), StoryboardManager (project model, serialization, transport),
  HMREngine (D3D11/D2D/DW render core), UI (DirectUI/Ribbon/panels), plus the
  D3D11/Media Foundation/WIC init in `dllmain.cpp`.
- Supporting DLLs (built from their own `src/*` dirs): `WLXPhotoBase`
  (foundation: `Base::Exception`, memory, OS checks, GDI+ helpers), `MovieMakerLang`
  (localization), `WLXPipeline`, `WLXVideoTrim`, `WLXMovieLibrary`, `WLXPipetran`
  (transitions), `WLXSlideshow`, `WLXTranscode.exe`, `WLXCodecHost.exe`,
  `MovieMakerPreviewClient`, `WLMFDS`, `WLMFReadWrite`, `WLXMediaPublishSubscribe`,
  `WLXPhotoCinematic`, `WLXPhotoSqm`, `WLXMP4Parser`, `WLXFaceRecognition`,
  `DmxBici` (telemetry), `MetadataSys` (property handler), `UXCore`, `uxctl`,
  `wlidcli` (identity), `GPURenderer.dll` (from `src/Renderer`).
- Delay-loaded in-process: `WLXPhotoSqm.dll`, `DmxBici.dll`, `wlidcli.dll`,
  `uxcore.dll` — not shipped on modern Windows; code must survive their absence.

**Calling convention rule (verified by tests):** every public entry point is
`__stdcall` (WINAPI) **except** `MovieMakerMain` (`__cdecl`) and the UXCore
resource/registry helpers from `Resources.cpp` (`__cdecl`). Match the reference
decoration exactly (`Name@N` vs undecorated); see `tests/mmr-python/README.md`.

## Build & Test

Requirements: VS2022 Build Tools with the C++ **ATL** component, CMake ≥ 3.20
(`CMakeLists.txt` declares 3.15 minimum), Windows SDK 10.0.26100.0, WTL 10 under
`src/WTL/`. Project is **MSVC-only and WIN32(x86)-only** — both are enforced with
`FATAL_ERROR`. Architecture is **Win32 (x86)**, generator `Visual Studio 17 2022`.

```powershell
# Build (Debug)
& "C:\Program Files\CMake\bin\cmake.exe" --build build --config Debug

# Contract suite (REQUIRED after any change) — must set WMMR_DLL_DIR!
$env:WMMR_DLL_DIR = "C:\Users\mcmco\Desktop\WMMR\build\bin\Debug"
python32\python.exe tests\mmr-python\run_tests.py        # expect PASS=116 FAIL=0

# CTest
ctest --test-dir build -C Debug --output-on-failure      # expect 2/2 pass

# GUI harness (C#, separate checkout via submodule)
#   dotnet build -warnaserror
#   mmr-gui.exe --selfcheck            # managed-only, no DLLs needed
#   mmr-gui.exe --selftest <binDir>    # 128 checks / 21 DLLs (needs x86 .NET runtime)
```

**Critical gotcha:** `tests/mmr-python/run_tests.py` **defaults the DLL dir to
`build_clean\bin\Debug`**, which contains the *reference/parity-stub* DLLs that
return `E_NOTIMPL`. Always set `WMMR_DLL_DIR` to the real build output
(`build\bin\Debug`) before running, or contracts fail / report stubs.

Verify changes with: build → contract suite → ctest. The suite is the regression
oracle; do not weaken it to make failures pass.

## Testing Strategy

- `tests/mmr-python` — ctypes contract suite over the real 32-bit DLLs. It
  asserts HRESULTs, GUIDs, return values, decoration parity, and observable side
  effects. Executable only by the gitignored 32-bit `python32` interpreter
  (64-bit Python cannot load 32-bit DLLs). Contracts live in
  `wmmr/contracts/*.py`; cross-DLL scenarios in `wmmr/workflows.py`.
  CLI: `run_tests.py [DLL-DIR] [--list|--filter|--skip|--retries|--junit|--json|--tap|--quiet]`.
- `tests/mmr-gui` — WinForms browser + `--selftest <binDir>` headless gate
  (128 checks across 21 DLLs), `--selfcheck` managed self-test. Requires
  `.NET 10`, `PlatformTarget=x86`, and `$env:DOTNET_ROOT_X86`.
- `tests/mmr-cli` — .NET 10 console twin of the GUI harness (no README).
- `tests/<Module>/test_core.cpp` — legacy per-module C++ exe tests.
- CTest — `tests` entry wired into the build; run via `ctest --test-dir build`.

**Guidance:** a new behavior fix (factory semantics, HRESULT change, file-format
detail, registry side effect) that could regress should get a contract in
`tests/mmr-python`. Changing a contract = a commit in the `mmr-python`
submodule **plus** a gitlink bump in the parent repo.

## Linting / Formatting / Type-checking

None are configured in this repo — there is no clang-format/clang-tidy/
.editorconfig/cppcheck config at the root. Match the existing style by eye
(MSVC era conventions, see below). Do not add new lint tooling unless asked.

## Dependency & Submodule Rules

- 13 git submodules under `github.com/havaianasdestruido/`: `tests/mmr-{cli,gui,python}`
  + 10 `apps/*`. Use `git submodule update --init --recursive` when cloning.
- Do not vendor submodule output; do not commit submodule source into the parent.
- `src/WTL/` is vendored third-party; keep it pristine.
- System libs only — no third-party runtime deps. Each module links explicit
  system import libs (kernel32, user32, shlwapi, ole32, gdiplus, mf, etc.); see
  per-module `CMakeLists.txt` for the exact list. Root `CMakeLists.txt` sets the
  common list for modules that have none.

## Code Conventions

- **Language:** C++14. MSVC 11-era idiom dominates (CAppModule/ATL, WTL, `HRESULT`
  returns, PascalCase methods). Preserve it; do not modernize wholesale.
- **Headers:** `#pragma once` + include guard in `common.h`; `common.h` is the
  de-facto precompiled set (Windows/COM/GDI+/DX/Media Foundation + C++ std).
  `src/MovieMakerCore/pch.h` for the engine.
- **Version macros:** in `src/common.h` — `WMMR_VERSION_MAJOR/MINOR/BUILD/REVISION`
  = 16/4/3528/331, `WMMR_VERSION_STRING "16.4.3528.331"`. Reference these, never
  hardcode elsewhere.
- **Namespace:** `WMMR` for shared utilities (`ComPtr = CComPtr`, `MovieMakerException`,
  `CoInitializer`, `GdiplusInit`). Most module code uses the original ATL/global style.
- **CMake per module** (see `src/WLXVideoTrim/CMakeLists.txt` as the template):
  `project()`, `add_library(... SHARED)`, `OUTPUT_NAME` without `PREFIX`/`SUFFIX`,
  `MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"` for DLLs, `$<TARGET_OBJECTS:...>`
  for helper object libs, include dirs `src` + `src/WTL`, explicit
  `target_include_directories`/`target_compile_definitions`/`target_link_libraries`.
  The launcher (`src/MovieMaker`) uses static runtime and delay-loads
  `MovieMakerCore.dll`.
- **Export surface:** every DLL must match the reference export names, ordinals,
  and decorations (parity is mandatory). Most modules do this via a `.def` file
  (`src/<Module>/<Module>.def`) listing `Name @ordinal` or C++ mangled names
  (DmxBici, WLXPhotoBase, WLXPhotoSqm). Some (UXCore) rely on
  `__declspec(dllexport)`. `tools/diff_exports.py` enforces parity — run it when
  adding/removing exports.
- **Stubs:** follow `StubDesign.md` categories. Telemetry/SQM and auth surfaces
  must be inert-but-safe; factory COM entries should be real and contract-pinned.
- **Comments:** match the existing "recreation notes" style referencing the
  original binary where relevant. Do not add gratuitous comments.

## Error Handling

- Return `HRESULT` from every COM/factory/export surface. Check with `SUCCEEDED`;
  the `WMMR::Succeeded/Failed` helpers are preferences for readability.
- `S_FALSE` is **intentionally used** for true "no-op / already-correct" outcomes.
  Do not change it arbitrarily. The two genuine masking cases were fixed
  (ClipboardManager → `E_FAIL`, RibbonSites registry query → Win32 error);
  see `StubDesign.md`.
- Throw `WMMR::MovieMakerException` (carries its own HRESULT) or `ATLASSERT` for
  programmer errors; `Base::Throw`/`ThrowLastError` in WLXPhotoBase land.
- Module-specific HRESULTs exist (e.g. `WLXVideoTrim` uses a custom
  `FACILITY_WLXVIDEOTRIM` / `AVS_E_*` set) — match them; see the module READMEs.
- Respect the HMREngine macro redefinition warning zone in
  `src/MovieMakerCore/HMREngine/HMREngine.h` — it temporarily redefines SDK error
  macros and restores them; do not add includes in that span.
- Never swallow the VEH pattern (`src/MovieMaker/main.cpp`) or the delay-load
  survival logic; removing them breaks faithful behavior.

## Security Requirements

- **Untrusted inputs (files from disk) must be bounded** — loaders must cap
  resource consumption. Precedent (keep these, extend the same way):
  - `WLXMP4Parser`: bounds-checked box walker; `MoovBox` caps traks at 1024;
    `ParseStts` accumulates sample counts in 64-bit before clamping.
  - `HMREngine::X3DReader`: nesting depth 512 / node count 100k; recursive parser
    depth-guarded.
  - HMREngine scene traversal (`Traverse`/`TraverseNode`/`EnumerateRecursive`):
    depth cap 512 to avoid stack exhaustion.
  - `WLXFaceRecognition`: downscales to 640px max dimension, bounded samples.
- **Credentials:** never store plaintext. `AuthProvider` keeps an in-memory cache
  only; `PSAAuthenticationStore` persists to
  `%LOCALAPPDATA%\Microsoft\WL\MovieMaker\auth.dat` as a single `CryptProtectData`
  blob (current-user DPAPI), magic/version-header-gated, size-bounded (64 MB),
  length-prefixed fields with bounds checks. Extend, don't bypass.
- **Telemetry/SQM:** stubs must return `S_OK` and never send data off-machine.
- **Never print, log, or commit secrets/tokens.**

## Quirks Preservation

`QUIRKS.md` documents 23 faithfully-reproduced original bugs (VEH+SEH double
exception, `Global\WindowsLiveMovieMaker_Sundance_SingleInstance` mutex, null
CLSID, process-heap exceptions, D3D11 single-threaded flag, dead code paths,
etc.). Recreating, keeping, and extending the codebase to match these behaviors
is **correct and expected**. Only change them when the user explicitly requests
a "remix" with fixes.

## Pitfalls / Gotchas

- Contract suite **defaults to the reference/stub DLL dir** — always set
  `WMMR_DLL_DIR` to `build\bin\Debug`.
- 64-bit Python or 64-bit build will not test 32-bit DLLs. Output is x86 only.
- `build_clean\bin\Debug` holds reference-parity DLLs returning `E_NOTIMPL` —
  valid only as an *export-parity source*, never as a behavioral oracle for
  factories. Real (non-stub) implementations win where the reference is a stub;
  the contract pins the real HRESULT.
- Missing legacy DLLs (SQM/Bici/wlidcli/uxcore) must not crash startup.
- `MovieMaker.exe` runs as the app entry; `WLXTranscode.exe`/`WLXCodecHost.exe`
  are supporting exes, not launchers.
- Do not touch `.github/dependabot.yml` (empty ecosystem, intentionally inert) or
  `labeler.yml`/`label.yml` unless asked.

## Definition of Done

1. Builds clean: `cmake --build build --config Debug` — no warnings as errors, 0 errors.
2. `WMMR_DLL_DIR` set to the real build; contract suite green (`PASS=116 FAIL=0`).
3. `ctest --test-dir build -C Debug --output-on-failure` passes (2/2).
4. Export parity enforced/preserved for any touched DLL.
5. No secret/plaintext credential, no unbounded parser on untrusted input.
6. Submodule contract changes committed in the submodule + gitlink bump recorded.
7. Nothing committed/pushed into this repo unless the user explicitly asks.