# Stub Design & Re-implementation Guide

## Purpose

This project recreates the source of Windows Live Movie Maker 2012 (codename
Sundance, 16.4.3528.0331) from binary analysis. Many entry points are known by
name and signature from the original PE exports and RTTI, but the original
MSVC-11 machine code cannot be transcribed wholesale. The project therefore
uses **behavioral re-implementation**: each surface is rebuilt to be
contract-compatible with the reference binaries, then pinned by an automated
contract suite.

This file documents the stub conventions, the testing oracle, and the
decision rules for when a stub is acceptable vs. when real behavior is
required.

## Ground Rules

1. **Export parity is mandatory.** Every DLL must export exactly the same
   names (and ordinal aliases where applicable) as the reference binary.
   `tools/diff_exports.py` enforces this.

2. **Behavior parity is graded.** Surfaces whose behavior can be observed from
   outside the process (registers, file formats, published COM interfaces)
   MUST behave like the reference DLL. Internal purity is not required.

3. **Reference binaries are NOT the behavioral oracle for factories.** The
   `build_clean/` reference DLLs return `E_NOTIMPL` (0x80004001) for several
   factory entry points (e.g. `GetPipelineCreateFunctions`,
   `CreateAVICopierDirect`, `BuildMP4FilterGraph`). They are only reliable as
   an **export-parity source**. Where the reference is stub-like, our real
   implementation wins and the contract is pinned to the real HRESULT.

4. **The contract suite is the regression oracle.** It lives in
   `tests/mmr-python` (a git submodule). It loads our built DLLs and asserts
   HRESULTs, GUIDs, return values, and observable side effects. It must stay
   green (`PASS=116 FAIL=0` at last run). See `tests/mmr-python/README.md`.

## Stub Categories

| Category | Rule | Example |
|---|---|---|
| Telemetry/SQM | No data leaves machine. Stub returns S_OK, never sends. | `WLXPhotoSqm`, `DmxBici` |
| Marketing/promo panels | No-op or inert placeholder. | keep-alive panels, trayfry |
| Legacy project format | Shipped but unused; dead code paths may be stubs. | `.wlmp` v1 reader |
| Add-in host | Loads DLLs, but plugin API is inert. | `MovieMakerCore::LoadAddIns` |
| Render/encode pipeline | Produces verified output frames (test frames), not real video. | `PublishJob`, `ExportController` |
| Auth/storage | Inert but SECURE: nothing plaintext, nothing sent. | `PSAStub` (DPAPI), `AuthProvider` (memory-only) |
| COM factories | Real implementations, contract-pinned. | `WLXVideoTrim`, `WLXPipeline`, `WLXMP4Parser` |
| Registry ops | Real writes guarded; elevation-dependent results documented. | `WLXPhotoCinematic` DllRegisterServer |

## S_FALSE Semantics

`S_FALSE` is used intentionally for COM "no-op / state-already-correct"
outcomes throughout the codebase. The audit in `analysis/E_NOTIMPL/` and the
S_FALSE pass (2026-09) fixed the two genuine masking bugs:

- `ClipboardManager`: `GetClipboardData` failure now returns `E_FAIL`.
- `RibbonSites`: registry "Count" query failure now returns the Win32 error.

Do NOT change other `S_FALSE` uses unless a contract proves a caller observes
the difference.

## Untrusted-Input Bounds (hardening)

Loaders that ingest files from disk must bound their resource consumption:

- `WLXMP4Parser`: box walker is bounds-checked; `MoovBox` caps traks at 1024;
  `ParseStts` accumulates sample counts in 64-bit before clamping.
- `HMREngine::X3DReader`: parse context caps nesting depth (512) and node
  count (100000); the recursive JSON parser is depth-guarded.
- `HMREngine` scene traversal: `Traverse`/`TraverseNode`/`EnumerateRecursive`
  cap recursion depth at 512 to avoid stack exhaustion on hostile graphs.
- `WLXFaceRecognition`: analysis is downscaled to a 640px max dimension and
  bounded sample counts.

## Credential Handling

Credentials are never stored in plaintext:

- `AuthProvider` keeps an in-memory cache only (cleared on shutdown).
- `PSAAuthenticationStore::Save/Load` persists to
  `%LOCALAPPDATA%\Microsoft\WL\MovieMaker\auth.dat` as a single
  `CryptProtectData` blob (current-user DPAPI scope). The file is
  magic/version-header-gated and size-bounded to 64 MB; token strings are
  length-prefixed with per-field bounds checks.

## When To Add a Contract

Add a contract when a behavior fix matters but is easy to regress:
new factory semantics, changed HRESULT, file-format details, or registry
side effects. Contracts for `tests/mmr-python` live under
`tests/mmr-python/wmmr/contracts/` and workflows under
`tests/mmr-python/wmmr/workflows.py`. Changing a contract is a submodule
commit plus a parent-repo gitlink bump.

## Committing Method State

- Rebuild with: `& "C:\Program Files\CMake\bin\cmake.exe" --build build --config Debug`
- Verify: `python32\python.exe tests\mmr-python\run_tests.py` (expect `116/0`),
  then `ctest`.
- Never commit until the user explicitly asks.