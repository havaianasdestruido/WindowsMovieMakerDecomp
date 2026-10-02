---
sidebar_position: 1
title: Repository Map
description: Every directory in the repository and what lives in it.
---

# Repository Map

## Root documents

| File | Contents |
|---|---|
| `README.md` | Project overview + quick start |
| `AGENTS.md` | **The** canonical instruction file for AI coding agents (CLAUDE.md / AI.md point here) |
| `ROADMAP.md` | Build state, subsystem inventory, SDK fixes, 23 gotchas |
| `StubDesign.md` | Stub conventions, decision rules, hardening notes |
| `QUIRKS.md` | 23 original bugs/quirks deliberately preserved |
| `THINKING_PROCESS.md` | Reconstruction methodology narrative |
| `TODO_AUDIT.md` | Maintained index of intentionally incomplete work |
| `CMakeLists.txt` | Root build: 29 module subdirectories, common libs, definitions |

## Directories

| Path | Contents |
|---|---|
| `src/` | Reconstructed source — 30 module dirs + vendored `WTL/`; `MovieMakerCore/` is the engine (349 files); most other modules are 1–4 files. See [Module Reference](../modules/overview.md) |
| `src/WTL/` | Vendored WTL 10 headers (from NuGet `WTLCSPkg`) — third-party, keep pristine |
| `analysis/` | Binary-analysis output, one directory per module (65 entries). See [Binary Analysis](../methodology/binary-analysis.md) |
| `tests/` | Harnesses + per-module tests. See [Testing](../testing/overview.md) |
| `apps/` | 10 consumer/exercise apps (all submodules) |
| `tools/` | Build, verification, analysis tooling. See [Tools](../tools/overview.md) |
| `website/` | This Docusaurus documentation site |
| `resources/extracted/` | Extracted resource staging area |
| `build64/` | Alternate (x64 experiment) build directory — gitignored scratch |
| `.github/workflows/` | CI: per-project build matrix (`build-projects.yml`), PR labeler, sync, docs deploy |

## Submodules (13)

| Path | Repository |
|---|---|
| `tests/mmr-python` | `havaianasdestruido/mmr-python` — contract suite |
| `tests/mmr-gui` | `havaianasdestruido/mmr-gui` — WinForms harness |
| `tests/mmr-cli` | `havaianasdestruido/mmr-cli` — console harness |
| `apps/photoviewer` | exercise app |
| `apps/transition-plugin` | exercise app |
| `apps/moviemaker-launcher` | exercise app |
| `apps/mediapublisher` | exercise app |
| `apps/video-trimmer` | exercise app |
| `apps/metadata-editor` | exercise app |
| `apps/slideshow-studio` | exercise app |
| `apps/face-tagger` | exercise app |
| `apps/movielibrary` | exercise app |
| `apps/pipeline-graph` | exercise app |

Initialize with `git submodule update --init --recursive`.

## Gitignored (never commit)

| Path | Why |
|---|---|
| `undecomp/` | **Original copyrighted binaries.** Analysis-only, never commit, never un-ignore |
| `python32/` | Embedded 32-bit CPython 3.11.9 (test tool download) |
| `build/`, `build_clean/`, `out/`, `cool/`, `intro/` | Build outputs; `build_clean` holds reference/parity-stub DLLs |
| `tools/thirdparty/` | Ghidra/VBox scripts |
| `*.exe`, `*.dll`, `*.pdb`, `*.lib`, `*.exp`, `*.obj`, ... | Binaries never enter git |
