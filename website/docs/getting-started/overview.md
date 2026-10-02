---
sidebar_position: 1
title: Overview
description: The path from clone to a running Windows Live Movie Maker 2012 reconstruction.
---

# Getting Started Overview

This section takes you from a fresh clone to a **running MovieMaker.exe** and a **green
contract suite**. Work through the pages in order:

1. [**Requirements**](requirements.md) — the exact toolchain (VS2022 Build Tools + ATL,
   CMake, Windows SDK, WTL 10).
2. [**Building**](building.md) — configure with CMake (Win32 / VS2022 generator) and build
   all 29 targets, or use the `tools/build.ps1` wrapper.
3. [**Running**](running.md) — launch the app, understand the launcher → engine DLL flow,
   and what the supporting executables are for.
4. [**Verification**](verification.md) — run the contract suite (`PASS=116 FAIL=0`), CTest,
   the GUI harness, and the TODO audit.
5. [**Troubleshooting**](troubleshooting.md) — the classic failure modes: wrong DLL dir,
   missing ATL component, 64-bit Python, and more.

## The 60-second version

```powershell
# 1. Clone (submodules are used for tests/apps)
git clone https://github.com/havaianasdestruido/WindowsMovieMakerDecomp.git
cd WindowsMovieMakerDecomp
git submodule update --init --recursive

# 2. Configure + build
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Debug

# 3. Launch
build\bin\Debug\MovieMaker.exe

# 4. Verify (contract suite) — always point it at the REAL build output!
$env:WMMR_DLL_DIR = "$PWD\build\bin\Debug"
python32\python.exe tests\mmr-python\run_tests.py    # expect PASS=116 FAIL=0
```

:::info Windows-only

The project is **MSVC-only** — CMake fails configuration with `FATAL_ERROR` on
non-Windows or non-MSVC toolchains. The architecture must be selected as
**Win32 (x86)** with `-A Win32` (as in the commands above): the original 2012
binaries are 32-bit, and export decorations and the contract suite assume x86.

:::
