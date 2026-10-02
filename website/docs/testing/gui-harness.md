---
sidebar_position: 3
title: GUI Harness (mmr-gui)
description: The WinForms browser and its 128-check headless self-test across 21 DLLs.
---

# GUI Harness (`tests/mmr-gui`)

## What it is

A **C# WinForms application** with two jobs:

1. **Browser** — an interactive explorer that loads the built DLLs and exercises their
   surfaces, useful when investigating behavior by hand.
2. **Headless self-test** — `--selftest <binDir>` runs **128 checks across 21 DLLs**
   without any UI, suitable as a CI gate.

- **Location:** `tests/mmr-gui` (git submodule → `github.com/havaianasdestruido/mmr-gui`)
- **Target:** `.NET 10`, `PlatformTarget=x86`

## Running it

```powershell
dotnet build -warnaserror

# Managed-only self-check — no DLLs needed (toolchain sanity)
mmr-gui.exe --selfcheck

# Headless DLL gate — needs the x86 .NET runtime
mmr-gui.exe --selftest build\bin\Debug
```

Set the x86 runtime location first:

```powershell
$env:DOTNET_ROOT_X86 = "C:\Program Files (x86)\dotnet"
```

## Modes

| Mode | What it needs | What it proves |
|---|---|---|
| `--selfcheck` | nothing (managed only) | the harness itself is healthy |
| `--selftest <binDir>` | x86 .NET runtime + built DLLs | 128 behavioral checks across 21 DLLs |
| Interactive | runtime + DLLs | manual exploration |

## Relationship to the contract suite

The GUI harness overlaps the [contract suite](contract-suite.md) deliberately: different
runtime (managed vs. ctypes), different loading path, and a human-usable browser on top.
A behavior that passes in both is robust to consumer differences; a behavior that passes
in only one usually reveals a loading/decoration assumption worth investigating.

## Build discipline

- Build with `-warnaserror` — the submodule keeps a zero-warning bar.
- If checks fail only in `--selftest`, verify the runtime is x86 (`DOTNET_ROOT_X86`)
  before suspecting the DLLs.
