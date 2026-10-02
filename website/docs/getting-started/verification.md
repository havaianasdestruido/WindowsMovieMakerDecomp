---
sidebar_position: 5
title: Verification
description: The contract suite, CTest, the GUI harness, and the TODO audit — how a change is proven correct.
---

# Verification

Every change is verified with the same three-step gauntlet: **build → contract suite →
CTest**. The details of each harness live in [Testing](../testing/overview.md); this page is
the quick operational reference.

## 1. Contract suite (the behavioral regression oracle)

```powershell
# CRITICAL: point the suite at the REAL build output, not build_clean
$env:WMMR_DLL_DIR = "C:\path\to\WindowsMovieMakerDecomp\build\bin\Debug"

python32\python.exe tests\mmr-python\run_tests.py
```

Expected result:

```text
PASS=116 FAIL=0
```

The suite is a ctypes harness that loads the built 32-bit DLLs and asserts HRESULTs, GUIDs,
calling-convention decorations, factory semantics, and observable side effects. Contracts
live in `tests/mmr-python/wmmr/contracts/*.py` (submodule); cross-DLL scenarios in
`wmmr/workflows.py`.

:::danger The `WMMR_DLL_DIR` trap

`run_tests.py` **defaults** the DLL directory to `build_clean\bin\Debug` — the
reference/parity-stub DLLs that return `E_NOTIMPL` from many factories. If you forget to set
`WMMR_DLL_DIR`, contracts fail or silently report stub behavior. Always export the variable
first.

:::

Useful flags:

```text
run_tests.py [DLL-DIR] [--list] [--filter NAME] [--skip NAME] [--retries N]
                  [--junit PATH] [--json PATH] [--tap PATH] [--quiet]
```

## 2. CTest

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

Expected: **2/2 pass** (sanity checks that `MovieMaker.exe` and `MovieMakerCore.dll` exist
in the build output — compare-file self tests wired through `include(CTest)`).

## 3. GUI harness (optional, deeper)

```powershell
# Managed-only self-check (no DLLs needed)
dotnet run --project tests\mmr-gui -- --selfcheck

# Headless DLL gate: 128 checks across 21 DLLs (needs x86 .NET runtime)
dotnet run --project tests\mmr-gui -- --selftest build\bin\Debug
```

Requires `.NET 10`, `PlatformTarget=x86`, and `$env:DOTNET_ROOT_X86` pointing at the x86
runtime.

## 4. Export parity

If you touched any DLL's export surface:

```powershell
python tools\diff_exports.py <original.dll> build\bin\Debug\<ours>.dll
python tools\diff_exports.py --batch <original_dir> build\bin\Debug
```

Export parity is **mandatory** — names, ordinals, and decorations must match the reference
exactly. See [Stub Design → Ground Rules](../methodology/stub-design.md#ground-rules).

## 5. TODO audit

```powershell
python tools\todo_audit.py --check
```

Reports all `TODO(reconstruction):` annotations across authored source. Use it before
committing to ensure you have not introduced unclassified stubs. See
[TODO Audit](../testing/todo-audit.md).

## Definition of done

From `AGENTS.md`, a change is complete when:

1. `cmake --build build --config Debug` is clean (0 errors).
2. `WMMR_DLL_DIR` set to the real build; contract suite green (`PASS=116 FAIL=0`).
3. `ctest --test-dir build -C Debug --output-on-failure` passes (2/2).
4. Export parity preserved for any touched DLL.
5. No plaintext credentials, no unbounded parser on untrusted input.
6. Submodule contract changes committed in the submodule + gitlink bump in the parent repo.
