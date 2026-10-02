---
sidebar_position: 2
title: Contract Suite (mmr-python)
description: The 116-check ctypes oracle that pins DLL behavior — usage, contract layout, and how to add contracts.
---

# Contract Suite (`tests/mmr-python`)

## What it is

A **ctypes-based contract suite** that loads the built 32-bit DLLs and asserts their
observable behavior: HRESULTs, GUIDs, return values, calling-convention decorations, and
side effects (registry, files). It is the project's regression oracle — the definitive
answer to "did behavior change?".

- **Location:** `tests/mmr-python` (git submodule → `github.com/havaianasdestruido/mmr-python`)
- **Contracts:** `wmmr/contracts/*.py` — one file per module surface
- **Cross-DLL scenarios:** `wmmr/workflows.py`
- **Expected result:** `PASS=116 FAIL=0`

## Running it

```powershell
# REQUIRED: point at the real build output
$env:WMMR_DLL_DIR = "$PWD\build\bin\Debug"

python32\python.exe tests\mmr-python\run_tests.py
```

:::danger Default DLL dir is a trap

`run_tests.py` defaults `WMMR_DLL_DIR` to `build_clean\bin\Debug` — the
**reference/parity-stub DLLs** that return `E_NOTIMPL` from several factories. Always
set `WMMR_DLL_DIR` to `build\bin\Debug` first, or you will test the wrong binaries and
see mysterious stub behavior.

:::

## CLI

```text
run_tests.py [DLL-DIR] [--list] [--filter NAME] [--skip NAME]
                  [--retries N] [--junit PATH] [--json PATH]
                  [--tap PATH] [--quiet]
```

| Flag | Use |
|---|---|
| `--list` | enumerate contract names |
| `--filter NAME` | run a subset (fast iteration while implementing) |
| `--skip NAME` | exclude a known-failing contract temporarily |
| `--retries N` | retry flaky contracts (e.g. timing-sensitive) |
| `--junit/--json/--tap` | machine-readable output for CI |

## Requirements

- The **gitignored embedded 32-bit CPython 3.11.9** at `python32\` — 64-bit Python cannot
  load the x86 DLLs (`WinError 193`).
- A completed Debug build in `build\bin\Debug`.

## What a contract looks like

Contracts assert the *externally observable* surface of a DLL. For example
(`wmmr/contracts/wlxpipeline.py`): `GetPipelineCreateFunctions` must return `S_OK` and a
real 6-pointer table (`uVersion`, `uStructSize`, `pfnCreate`, `pfnDestroy`, `pfnProcess`,
`pfnGetInfo`) — pinning the *real* implementation, because the reference parity build
returns `E_NOTIMPL` there.

## Adding or changing contracts

1. A behavior fix matters and could regress (new factory semantics, HRESULT change,
   file-format detail, registry side effect) → write a contract.
2. Contracts live in the **submodule**: commit there first.
3. Then bump the gitlink in this repo (`tests/mmr-python` pointer) in the same logical
   change set.
4. Never weaken a contract to make a failure pass — the suite is the oracle, not the
   obstacle.

See also [Stub Design → When To Add a Contract](../methodology/stub-design.md#when-to-add-a-contract).
