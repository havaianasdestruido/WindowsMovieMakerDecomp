---
sidebar_position: 4
title: CLI Harness (mmr-cli)
description: The .NET 10 console twin of the GUI harness checks.
---

# CLI Harness (`tests/mmr-cli`)

## What it is

The **console twin** of the [GUI harness](gui-harness.md): the same DLL checks, packaged
as a .NET 10 console application with no WinForms dependency. Useful where a GUI-capable
session is unavailable (CI agents, SSH, logging-friendly pipelines).

- **Location:** `tests/mmr-cli` (git submodule → `github.com/havaianasdestruido/mmr-cli`)
- **Target:** .NET 10, x86 process (to load the 32-bit DLLs)

## Using it

Build and run from the submodule directory:

```powershell
dotnet build
# then run against the built DLLs (see the harness's CLI help for exact arguments)
mmr-cli --help
```

Point it at `build\bin\Debug` — the same `WMMR_DLL_DIR`-style trap as the contract suite
applies: never test against `build_clean\bin\Debug` (the parity-stub DLLs).

## Notes

- The submodule ships **no README** (as of this writing); treat it as a thin re-host of
  the mmr-gui check set.
- Results should agree with `mmr-gui --selftest`. Disagreements are bugs in one harness
  or the other — file them against the submodule, not the parent repo.

## When to prefer it

| Situation | Use |
|---|---|
| CI on a headless agent | mmr-cli |
| Manual investigation with browsing | mmr-gui |
| Fast pre-commit behavioral check | mmr-python (contract suite) |
