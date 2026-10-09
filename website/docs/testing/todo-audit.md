---
sidebar_position: 6
title: TODO Audit
description: The maintained index of intentionally incomplete work — how TODO(reconstruction) annotations are scanned and triaged.
---

# TODO Audit

## What it is

The **TODO audit** is the project's maintained index of work that is intentionally
incomplete. It exists because in a parity reconstruction, "returns `E_NOTIMPL`" or
"stubbed" is often *correct*. The scanner, `tools/todo_audit.py`, is a **search**: it
finds and reports `TODO(reconstruction):` annotations across authored source. Deciding
whether a given annotation is an intentional stub or a genuine gap — and whether the
index is complete — is a **manual review** step; the tool does not verify or classify
what it finds. With `--check`, the scan only fails when **no** annotations are found
(i.e. it guards against the audit going empty, not against unclassified stubs).

Maintained index: [`TODO_AUDIT.md`](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/blob/main/TODO_AUDIT.md)
at the repo root. Scanner: `tools/todo_audit.py`.

## The annotation format

```cpp
// TODO(reconstruction): Describe the missing behavior and its compatibility constraint.
```

Every incomplete area must carry one of these, stating (a) what behavior is missing and
(b) why the current safe fallback must remain until binary analysis and a contract
establish the replacement.

## Running it

```powershell
python tools\todo_audit.py --check
```

Output is **location-based**, so it stays accurate as source moves.

## Coverage rules

The scanner reads every line of each **tracked, authored** source file with a supported
extension:

```text
.bat .c .cc .cmake .cpp .cs .cxx .def .h .hpp .ps1 .py .rc   + all CMakeLists.txt
```

**Excluded** (not authored by this project):

- vendored `src/WTL/` code
- Git submodules

## Policy

- Do **not** add a TODO merely because a method returns `S_OK` or `E_NOTIMPL` — classify
  it against [Stub Design](../methodology/stub-design.md) first.
- A TODO must state the missing behavior **and** why the fallback is required.
- Do not remove a TODO without implementing the behavior *and* adding a contract for it.
  For externally observable behavior, the contract is mandatory. Behaviors that are not
  externally observable (internal classes, static-library helpers, and `__thiscall`
  member functions the ctypes harness cannot bind) are resolved by implementation plus a
  recreation note at the call site; when such a behavior becomes export-observable, pin
  it with a contract in `tests/mmr-python`.

## Current state (from `TODO_AUDIT.md`)

The audit is **clean** — the 14 annotations from the 2026-09 pass were resolved in the
2026-10-08 pass across the three former priority areas (rendering/UI, media behavior,
interop/serialization). See [`TODO_AUDIT.md`](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/blob/main/TODO_AUDIT.md)
for the per-item resolution notes. Remaining stub work that is intentionally not annotated
— telemetry no-ops, documented parity `E_NOTIMPL`s, and dead code paths — is triaged in
`analysis/E_NOTIMPL/inventory.md` (92 stubs; its MEDIUM/HIGH entries are the next
candidates for reconstruction passes).
