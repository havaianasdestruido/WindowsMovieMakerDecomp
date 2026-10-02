---
sidebar_position: 6
title: FAQ
description: Frequently asked questions about the reconstruction.
---

# FAQ

### Is this a decompilation of the original machine code?

No — it's a **behavioral re-implementation**. The original MSVC-11 machine code is not
transcribed. Every public surface was rebuilt to be contract-compatible with the
reference binaries, then pinned by an automated contract suite
([Stub Design](../methodology/stub-design.md)). The original binaries are not in this
repository.

### Why does the app launch but publishing/export produces test frames?

Export rendering is a **documented stub category**: the transcode pipeline runs
end-to-end and reports correct progress, but the encoder writes deterministic test
frames rather than a real codec stream. It's the largest remaining reconstruction gap —
see the [TODO audit](../testing/todo-audit.md) priorities.

### Why are there bugs nobody fixed?

Because they're **the original's bugs**, faithfully preserved — 23 of them, cataloged in
[Preserved Quirks](../methodology/quirks.md). "Faithful" includes the bugs. A "remix"
pass can strip them later if explicitly requested.

### Can I build this on Linux/macOS/clang?

No. The project is **MSVC-only and Win32 (x86)-only**, enforced with `FATAL_ERROR` at
configure time. This matches the original binaries' architecture, export decorations,
and the 32-bit contract suite.

### Can I use 64-bit Python for the tests?

No — the DLLs are 32-bit. Only the gitignored embedded 32-bit CPython
(`python32\python.exe`) can load them; 64-bit Python fails with `WinError 193`.

### Why do my contract tests report `E_NOTIMPL` everywhere?

You're testing against `build_clean\bin\Debug` — the reference/parity-stub DLLs (the
suite's default). Set `$env:WMMR_DLL_DIR` to `build\bin\Debug` first. See
[Verification](../getting-started/verification.md).

### What's the difference between `build/` and `build_clean/`?

`build/` is the real reconstruction output. `build_clean/` holds reference/parity-stub
DLLs used as an **export-parity source only** — they return `E_NOTIMPL` from several
factories and are never a behavioral oracle.

### Are the COM GUIDs the original ones?

Some are (extracted from binary strings + `.rgs` scripts — see `analysis/COMGuids/`).
Reconstructed COM class GUIDs are hand-crafted placeholders because the originals were
never extracted ([Quirk #4](../methodology/quirks.md#4-deterministic-pseudo-guids-for-com-objects)).

### Does it send telemetry or store my credentials?

No, by rule. Telemetry (`WLXPhotoSqm`, `DmxBici`) is inert — `S_OK` and nothing leaves
the machine. Credentials live in memory only or in a DPAPI-encrypted, bounds-checked
store. See [Security Policy](security.md).

### Where did the architecture knowledge come from?

Mostly **RTTI extraction**: 1360 class names across 48 namespaces, cross-referenced with
strings, imports/exports, and registry scripts — all preserved under `analysis/`. See
[Binary Analysis](../methodology/binary-analysis.md).

### How do I run just one contract?

```powershell
$env:WMMR_DLL_DIR = "$PWD\build\bin\Debug"
python32\python.exe tests\mmr-python\run_tests.py --filter <name>
```

### How do I preview or edit this documentation site?

```powershell
cd website
npm install
npm start        # dev server with hot reload
npm run build    # production build → website/build
```

See `website/README.md` for details.
