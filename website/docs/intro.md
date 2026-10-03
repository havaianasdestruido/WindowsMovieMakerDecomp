---
slug: /
sidebar_position: 1
title: Introduction
description: What WindowsMovieMakerDecomp is, why it exists, and how this documentation is organized.
---

# Introduction

**WindowsMovieMakerDecomp** is a full decompilation/recreation of **Microsoft Windows Live
Movie Maker 2012** — codename **Sundance**, version **16.4.3528.0331** — extracted from a 2012
Windows Live Essentials setup and rebuilt from binary analysis as C++14 / ATL / WTL source.

The original binaries were compiled with MSVC 11.0 (Visual Studio 2012) for Windows 8. This
project reconstructs them with a modern VS2022 toolchain while preserving the original
architecture, export surfaces, observable behavior, and — deliberately — the original bugs.

## Key facts

| | |
|---|---|
| **Product** | Windows Live Movie Maker 2012 |
| **Codename** | Sundance |
| **Version** | 16.4.3528.0331 (`.ship.client.main.w5m4`) |
| **Original toolchain** | MSVC 11.0 (VS2012), x86, Windows 6.2+ |
| **Reconstruction toolchain** | MSVC 19.44 (VS2022), C++14, CMake ≥ 3.20, Windows SDK 10.0.26100.0 |
| **Build targets** | 29 CMake targets (launcher EXE, engine DLL, supporting DLLs/EXEs) |
| **Source files** | 375+ (`src/MovieMakerCore/` alone has 349) |
| **RTTI classes mapped** | 1360 across 48 namespaces |
| **Contract tests** | 116 checks (`tests/mmr-python`), expected `PASS=116 FAIL=0` |
| **Preserved quirks** | 23 documented original bugs (see [QUIRKS](./methodology/quirks.md)) |

## What "reconstruction" means here

This is not a transcription of the original machine code. It is **behavioral
re-implementation**: every public surface is rebuilt to be contract-compatible with the
reference binaries, then pinned by an automated contract suite. Three rules govern the work
(full detail in [Stub Design](./methodology/stub-design.md)):

1. **Export parity is mandatory.** Every DLL exports exactly the same names, ordinals, and
   decorations as the reference binary, enforced by `tools/diff_exports.py`.
2. **Behavior parity is graded.** Anything observable from outside the process (registry,
   file formats, published COM interfaces, HRESULTs) must behave like the reference.
   Internal purity is not required.
3. **The contract suite is the oracle.** `tests/mmr-python` loads the built DLLs and asserts
   HRESULTs, GUIDs, return values, and side effects. It must stay green.

Where the reference DLL itself is a stub (several factory entry points in the parity build
return `E_NOTIMPL`), the real implementation wins and the contract pins the real HRESULT.

## Repository layout at a glance

| Path | Contents |
|---|---|
| `src/` | Reconstructed source: 30 module directories + vendored `src/WTL/` headers |
| `analysis/` | Binary-analysis output, one directory per module (65 entries) |
| `tests/` | Contract suite, GUI/CLI harnesses (submodules), per-module C++ tests |
| `apps/` | 10 consumer/exercise apps (submodules) |
| `tools/` | Build, verification, and binary-analysis tooling |
| `website/` | This Docusaurus documentation site |
| `README.md`, `ROADMAP.md`, `StubDesign.md`, `QUIRKS.md`, `THINKING_PROCESS.md`, `AGENTS.md` | Root engineering documents |

A complete map lives in [Repository Map](./reference/repository-map.md).

## How to read this documentation

- **New here?** Start with [Getting Started](./getting-started/overview.md) to build and run
  the app, then skim the [Architecture Overview](./architecture/overview.md).
- **Looking for a specific DLL?** Every build target has a page in the
  [Module Reference](./modules/overview.md).
- **Working on the code?** Read [Stub Design](./methodology/stub-design.md) and
  [Preserved Quirks](./methodology/quirks.md) first — they explain what may and may not be
  changed. [Conventions](./reference/conventions.md) covers code style and CMake patterns.
- **Verifying a change?** See [Testing](./testing/overview.md).
- **Curious how 10 MB of machine code became readable C++?** See
  [Reconstruction Methodology](./methodology/reconstruction.md).

## Legal note

This is an independent, clean-room-style reconstruction for research and interoperability
purposes. It is not affiliated with or endorsed by Microsoft. The original binaries are
copyrighted and are **not** part of this repository (they live in the gitignored `undecomp/`
directory on the author's machine and must never be committed).
