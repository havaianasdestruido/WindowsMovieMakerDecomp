---
sidebar_position: 1
title: Testing Overview
description: Five testing layers — contracts, CTest, GUI self-test, CLI twin, and the TODO audit.
---

# Testing Overview

Testing in this project answers one question: **"does the reconstruction still behave
like the original?"** Because the source is a re-implementation, unit tests alone prove
nothing about parity — the tests must observe the DLLs from *outside the process*, the
way a real consumer would.

| Layer | Where | What it proves | Gate |
|---|---|---|---|
| [**Contract suite**](contract-suite.md) | `tests/mmr-python` (submodule) | HRESULTs, GUIDs, decorations, factory semantics, file-format details, side effects | `PASS=116 FAIL=0` |
| [**CTest**](module-tests.md#ctest) | root `CMakeLists.txt` | Build sanity (outputs exist) | 2/2 |
| [**GUI harness**](gui-harness.md) | `tests/mmr-gui` (submodule) | 128 headless checks across 21 DLLs; managed self-check | green |
| [**CLI twin**](cli-harness.md) | `tests/mmr-cli` (submodule) | Console mirror of the GUI harness checks | green |
| [**Per-module tests**](module-tests.md) | `tests/<Module>/` | Classic C++ exe tests per module | builds + passes |
| [**TODO audit**](todo-audit.md) | `tools/todo_audit.py` | Every intentional stub is classified and locatable | clean or triaged |

## The quick loop

```powershell
# 1. Build
cmake --build build --config Debug

# 2. Contract suite — point at the REAL build output!
$env:WMMR_DLL_DIR = "$PWD\build\bin\Debug"
python32\python.exe tests\mmr-python\run_tests.py     # PASS=116 FAIL=0

# 3. CTest
ctest --test-dir build -C Debug --output-on-failure    # 2/2
```

## Design principles

1. **The contract suite is the regression oracle.** Do not weaken a contract to make a
   failure pass — fix the code or, if the contract itself was wrong, change it in the
   `mmr-python` submodule with justification (and bump the gitlink in this repo).
2. **Reference binaries are not the behavioral oracle for factories** — the parity-stub
   build returns `E_NOTIMPL` from several factories. Where the reference is stub-like,
   our real implementation wins and the contract pins the real HRESULT.
3. **32-bit only.** All DLLs are x86; only the embedded `python32` interpreter can load
   them. 64-bit Python fails with `WinError 193`.
4. **A new behavior fix that could regress should get a contract** — factory semantics,
   HRESULT changes, file-format details, registry side effects.

## Where the harnesses live

The three submodules host the harness code:

```text
tests/mmr-python/   # ctypes contract suite (run_tests.py, wmmr/contracts/*.py, wmmr/workflows.py)
tests/mmr-gui/      # WinForms browser + --selftest gate
tests/mmr-cli/      # .NET 10 console twin
```

They are separate repositories (`github.com/havaianasdestruido/mmr-*`) — changes there
require a submodule commit **plus** a gitlink bump in this repo.
