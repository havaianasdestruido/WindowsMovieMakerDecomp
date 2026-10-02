---
sidebar_position: 2
title: Requirements
description: The exact toolchain needed to build the Windows Live Movie Maker 2012 reconstruction.
---

# Requirements

## Toolchain

| Component | Version | Notes |
|---|---|---|
| **Visual Studio 2022 Build Tools** | MSVC 19.x | **Must include the C++ ATL component** (installed via the VS Installer's "C++ ATL for latest v143 build tools" workload item) |
| **CMake** | ≥ 3.20 | The root `CMakeLists.txt` declares 3.15, but 3.20+ is what the project is validated with |
| **Windows SDK** | 10.0.26100.0 | Older Win10 SDKs workable but untested; the project patches several APIs removed from this SDK |
| **WTL 10** | 10.x | Vendored under `src/WTL/` (from the NuGet package `WTLCSPkg`) — already in the repo, no install needed |
| **CMake generator** | `Visual Studio 17 2022` | Architecture **Win32 (x86)** — required |
| **PowerShell** | 5.1+ | For `tools/build.ps1` and setting environment variables |

## Test tooling (optional but recommended)

| Component | Purpose |
|---|---|
| **32-bit Python 3.11** (`python32/`, gitignored) | The contract suite loads 32-bit DLLs — a 64-bit Python **cannot** run it. Download embedded CPython 3.11.9 win32 and unpack it to `python32/`. |
| **.NET 10 SDK + x86 runtime** | The `tests/mmr-gui` WinForms harness and `tests/mmr-cli` console twin. Set `$env:DOTNET_ROOT_X86`. |
| **Ghidra / disassembler** | Only needed for new binary analysis. See [Binary Analysis](../methodology/binary-analysis.md). |

## Hard constraints

The root `CMakeLists.txt` fails configuration with a fatal error if either of these is violated:

```text
This project only builds on Windows with MSVC.   ← non-Windows / non-MSVC
This project requires the MSVC compiler.         ← clang/gcc on Windows
```

- **Architecture is Win32 (x86)** — configure with `-A Win32`. The original binaries are
  32-bit; export decorations (`Name@N`), calling conventions, and the contract suite all
  assume x86.
- **No third-party runtime dependencies.** Modules link only system import libraries
  (`kernel32`, `user32`, `ole32`, `gdiplus`, `mf`, `d3d11`, ...). The only vendored
  third-party code is the WTL 10 header set under `src/WTL/`.

## Cloning with submodules

The repository uses 13 git submodules (3 test harnesses + 10 `apps/*` exercise apps), all
hosted under `github.com/havaianasdestruido/`:

```bash
git clone --recurse-submodules https://github.com/havaianasdestruido/WindowsMovieMakerDecomp.git
# or, in an existing clone:
git submodule update --init --recursive
```

The core build does **not** require the submodules — only the test harnesses and demo apps
live there. See [Repository Map](../reference/repository-map.md) for the full list.
