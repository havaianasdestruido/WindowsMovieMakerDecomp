---
sidebar_position: 1
title: Reconstruction Methodology
description: How 10 MB of 2012 machine code became readable C++14 — phases, decisions, and lessons.
---

# Reconstruction Methodology

This page condenses [`THINKING_PROCESS.md`](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/blob/main/THINKING_PROCESS.md),
the project's phase-by-phase narrative. The methodology in one sentence: **analyze the
binaries exhaustively, rebuild contract-compatible source, and pin behavior with an
executable oracle.**

## Phase 1 — Binary analysis

Starting point: `MovieMaker.exe` (~122 KB), `MovieMakerCore.dll` (~10.5 MB, single export
`MovieMakerMain`), and 17 supporting DLLs, extracted from the original
`wlsetup-all.exe` (Live Essentials 2012).

Extracted from every binary:

- **RTTI class names** — 1360 unique type names across 48 namespaces (the architecture
  map fell out of this)
- **Strings** — 5721 ASCII + 2560 UTF-16 (feature discovery, codename "Sundance",
  version 16.4.3528.0331)
- **Imports/exports**, PE structure, and PDB GUIDs
- Registry scripts (`.rgs`), COM GUIDs, and resource inventories

Key insight: the EXE is a thin launcher; all real code is in `MovieMakerCore.dll`;
supporting DLLs are utility libraries. → 18 CMake targets (now 29 with the engine-layer
targets).

## Phase 2 — Build system

- CMake + `Visual Studio 17 2022`, **Win32** — matching the 32-bit originals.
- ATL ships with VS Build Tools (needs the "C++ ATL" component); **WTL 10** headers came
  from the NuGet `WTLCSPkg` package into `src/WTL/`.
- Lesson: WTL's `atlapp.h` requires `extern CAppModule _Module;` before including control
  headers.

## Phase 3–4 — Skeleton, then depth

**Round 1** created all module skeletons (CMakeLists + stubs) shaped by the RTTI/strings
analysis: SundanceApp, StoryboardManager, HMREngine, HMRAVSource, UI, Serialization,
Theme, Background, Transport.

**Round 2** implemented depth per subsystem — the X3D field/node system on COM
reference-counting, the `X3DMath` library over DirectXMath, the MF source/sink/reader
strategies, the Ribbon application model, the `.wlmp` reader/writer.

## Phase 5–6 — Build fixes to a clean build

Two dedicated fix passes resolved dozens of SDK-era incompatibilities — the raw list is
the "23 known gotchas" in `ROADMAP.md` (from `winmm.h` not existing in the Win10 SDK to
MSVC 14.44 ATL `BEGIN_COM_MAP` conflicts). The pass ended with **all 18 targets building
clean: 0 compile errors, 0 link errors, 0 RC errors.**

## Phase 7 — First launch

Runtime initialization (D3D11/WARP, WIC, MF, single-instance mutex, delay-load
survival) — MovieMaker.exe launches and shows the "Windows Live Movie Maker" window.

## Phase 8–10 — Stub replacement, by subagent waves

Two **15-subagent passes** replaced 200+ stub methods with real implementations across
serialization, UI behaviors, publishing, audio DSP, the MF pipeline, and undo/redo —
each landing with build + contract verification. The 2026-09 hardening pass followed
(bounded parsers, DPAPI store, error-surfacing fixes).

## Key technical decisions

| Decision | Rationale |
|---|---|
| **Behavioral re-implementation, not code transcription** | Original machine code can't be legally/mechanically transcribed; contracts capture the observable truth |
| **C++14** | MSVC 11.0-era idiom; avoids modernizing drift |
| **`.def`-based export parity** | Ordinals and decorations are part of the observable contract |
| **Contract suite as oracle** | Executable parity beats documentation; regression-safe |
| **Quirks preserved & cataloged** | "Faithful" includes the bugs — a remix can strip them later |
| **Reference binaries are not factory oracles** | Where the reference returns `E_NOTIMPL`, the real implementation defines the contract |

## User preferences that shaped the work

- Match original behavior **exactly**, including preserved bugs, unless a "remix/fix"
  pass is explicitly requested.
- Keep telemetry inert and credentials protected — never ship behavior that phones home
  or persists secrets in plaintext.
- The contract suite must stay green; never weaken it to land a change.

## Where to dig deeper

- [Stub Design](stub-design.md) — the rules for what may be a stub.
- [Preserved Quirks](quirks.md) — the 23 faithful bugs.
- [Binary Analysis](binary-analysis.md) — the tooling that produced `analysis/`.
- `THINKING_PROCESS.md` at the repo root — the full word-for-word narrative.
