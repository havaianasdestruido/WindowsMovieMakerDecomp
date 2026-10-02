---
sidebar_position: 2
title: Stub Design & Re-implementation Guide
description: The ground rules, stub categories, S_FALSE semantics, hardening bounds, and when to add a contract.
---

# Stub Design & Re-implementation Guide

This page condenses [`StubDesign.md`](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/blob/main/StubDesign.md) —
the load-bearing design rules for every DLL surface. **Read this before touching any
stub.**

## Why stubs exist

The project recreates source from binary analysis. Many entry points are known by name
and signature from the original PE exports and RTTI, but the original MSVC-11 machine
code cannot be transcribed wholesale. The method is **behavioral re-implementation**:
rebuild each surface to be *contract-compatible*, then pin it with the automated
contract suite.

## Ground rules

1. **Export parity is mandatory.** Every DLL must export exactly the same names (and
   ordinal aliases where applicable) as the reference binary. `tools/diff_exports.py`
   enforces this.
2. **Behavior parity is graded.** Surfaces observable from outside the process
   (registry, file formats, published COM interfaces) MUST behave like the reference.
   Internal purity is not required.
3. **Reference binaries are NOT the behavioral oracle for factories.** The
   `build_clean/` reference DLLs return `E_NOTIMPL` for several factory entry points
   (e.g. `GetPipelineCreateFunctions`, `CreateAVICopierDirect`,
   `BuildMP4FilterGraph`). They are only reliable as an **export-parity source**. Where
   the reference is stub-like, our real implementation wins and the contract pins the
   real HRESULT.
4. **The contract suite is the regression oracle.** `tests/mmr-python` must stay green
   (`PASS=116 FAIL=0`).

## Stub categories

| Category | Rule | Example |
|---|---|---|
| Telemetry/SQM | No data leaves machine. Stub returns `S_OK`, never sends. | `WLXPhotoSqm`, `DmxBici` |
| Marketing/promo panels | No-op or inert placeholder. | keep-alive panels, trayfry |
| Legacy project format | Shipped but unused; dead code paths may be stubs. | `.wlmp` v1 reader |
| Add-in host | Loads DLLs, but plugin API is inert. | `MovieMakerCore::LoadAddIns` |
| Render/encode pipeline | Produces verified output frames (test frames), not real video. | `PublishJob`, `ExportController` |
| Auth/storage | Inert but **SECURE**: nothing plaintext, nothing sent. | `PSAStub` (DPAPI), `AuthProvider` (memory-only) |
| COM factories | Real implementations, contract-pinned. | `WLXVideoTrim`, `WLXPipeline`, `WLXMP4Parser` |
| Registry ops | Real writes guarded; elevation-dependent results documented. | `WLXPhotoCinematic` DllRegisterServer |

## S_FALSE semantics

`S_FALSE` is used **intentionally** for COM "no-op / state-already-correct" outcomes
throughout the codebase — do not change it arbitrarily. The audit in `analysis/E_NOTIMPL/`
and the S_FALSE pass (2026-09) fixed the two genuine masking bugs:

- `ClipboardManager`: `GetClipboardData` failure now returns `E_FAIL` (was masked as
  `S_FALSE`).
- `RibbonSites`: registry "Count" query failure now returns the Win32 error (was
  `S_FALSE`).

Do NOT change other `S_FALSE` uses unless a contract proves a caller observes the
difference.

## Untrusted-input bounds (hardening)

Loaders that ingest files from disk must bound their resource consumption — extend these
patterns, don't bypass them:

- **`WLXMP4Parser`**: bounds-checked box walker; `MoovBox` caps traks at 1024;
  `ParseStts` accumulates sample counts in 64-bit before clamping.
- **`HMREngine::X3DReader`**: parse context caps nesting depth (512) and node count
  (100000); recursive JSON parsing is depth-guarded.
- **HMREngine scene traversal** (`Traverse`/`TraverseNode`/`EnumerateRecursive`): depth
  cap 512 against stack exhaustion on hostile graphs.
- **`WLXFaceRecognition`**: analysis downscaled to a 640 px max dimension, bounded
  sample counts.

## Credential handling

Credentials are never stored in plaintext:

- `AuthProvider` keeps an **in-memory cache only** (cleared on shutdown).
- `PSAAuthenticationStore::Save/Load` persists to
  `%LOCALAPPDATA%\Microsoft\WL\MovieMaker\auth.dat` as a single `CryptProtectData` blob
  (current-user DPAPI scope). The file is magic/version-header-gated, size-bounded
  (64 MB), with length-prefixed fields and bounds checks.

## When to add a contract

Add a contract when a behavior fix matters but is easy to regress: **new factory
semantics, a changed HRESULT, file-format details, or registry side effects.**

Contracts live in `tests/mmr-python/wmmr/contracts/` (scenarios in
`wmmr/workflows.py`) — a submodule, so changing one is a submodule commit **plus** a
gitlink bump in the parent repo. See [Contract Suite](../testing/contract-suite.md).
