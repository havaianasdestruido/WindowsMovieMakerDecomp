---
sidebar_position: 2
title: Build & Verification Tools
description: build.ps1, test_all.py, and todo_audit.py — the day-to-day tooling.
---

# Build & Verification Tools

## build.ps1

The **build automation wrapper** for WMMR:

```powershell
.\tools\build.ps1                            # Full Debug build
.\tools\build.ps1 -Config Release            # Release build
.\tools\build.ps1 -Target MovieMakerCore     # Single target
.\tools\build.ps1 -Clean                     # Clean build
.\tools\build.ps1 -Analyze                   # Build + analyze binaries
```

Parameters:

| Parameter | Values | Default |
|---|---|---|
| `-Config` | `Debug`, `Release` | `Debug` |
| `-Target` | any CMake target | all |
| `-Clean` | switch | — |
| `-Analyze` | switch | run binary analysis after build |

It wraps the same `cmake -S . -B build -G "Visual Studio 17 2022" -A Win32` +
`cmake --build` sequence documented in [Building](../getting-started/building.md).

## test_all.py

The **runtime test harness** — tests MovieMaker.exe behavior by launching it and
observing the process:

```powershell
python tools\test_all.py                  # Run all tests
python tools\test_all.py --launch         # Just test launch
python tools\test_all.py --timeout 30     # Custom timeout
```

What it checks:

- process launches and stays alive (configurable timeout)
- window creation (the "Windows Live Movie Maker" window)
- process stability (no crash during observation window)

## todo_audit.py

The **TODO audit scanner** — reads every line of every tracked, authored source file and
reports `TODO(reconstruction):` annotations:

```powershell
python tools\todo_audit.py --check
```

- Scans `.bat .c .cc .cmake .cpp .cs .cxx .def .h .hpp .ps1 .py .rc` + all
  `CMakeLists.txt`.
- Excludes vendored `src/WTL/` and submodules (not authored here).
- Output is location-based, so it stays accurate as code moves.

Full policy on [TODO Audit](../testing/todo-audit.md).
