# WindowsMovieMakerDecomp

## Star History

<a href="https://www.star-history.com/?repos=havaianasdestruido%2FWindowsMovieMakerDecomp&type=date&legend=top-left">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/chart?repos=havaianasdestruido/WindowsMovieMakerDecomp&type=date&theme=dark&legend=top-left" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/chart?repos=havaianasdestruido/WindowsMovieMakerDecomp&type=date&theme=light&legend=top-left" />
   <img alt="Star History Chart" src="https://api.star-history.com/chart?repos=havaianasdestruido/WindowsMovieMakerDecomp&type=date&legend=top-left" />
 </picture>
</a>


Full decompilation/recreation of **Microsoft Windows Live Movie Maker 2012**
(codename Sundance, 16.4.3528.0331), extracted from a 2012 Live Essentials
setup and rebuilt from binary analysis as C++14 / ATL / WTL source.

Please read [TODO.md](TODO.md), [THINKING_PROCESS.md](THINKING_PROCESS.md),
[ROADMAP.md](ROADMAP.md) and [QUIRKS.md](QUIRKS.md).

**Work plan:** [**`TODO.md`**](TODO.md) at the project root is the master phased checklist for
reaching a faithful **1:1 clean-room decompilation & reconstruction** of the original
application and its companion service/plugin surfaces — 13 gated phases (`0`–`12`) plus four
cross-phase trackers (unreconstructed modules, `E_NOTIMPL` stub burn-down, PE binary-diff gaps,
and the per-module parity scorecard).

## 1:1 Clean-Room Reconstruction Plan ([`TODO.md`](TODO.md))

| Phase | Focus | Exit Gate |
|---|---|---|
| [**Phase 0**](TODO.md#phase-0--clean-room-charter-provenance--legal-hygiene) | Clean-room charter, provenance & legal hygiene | `analysis/PROVENANCE.md` (SHA-256 ledger), raw-dump remediation, CI leak guard |
| [**Phase 1**](TODO.md#phase-1--reference-freeze--module-inventory) | Reference freeze & module inventory | `analysis/inventory.json`, doc contradictions reconciled, Tier 1A/1B/2/3 surface cards |
| [**Phase 2**](TODO.md#phase-2--static-specification-extraction-per-module) | Static specification extraction | `analysis/<Module>/spec.md` per Tier-1A target, COM/GUID census, `UIFILE`/`.duxt` spec |
| [**Phase 3**](TODO.md#phase-3--dynamic-analysis--golden-oracles) | Dynamic analysis & golden oracles | 24/24 Tier-1A dynamic traces, `analysis/GOLDEN_MANIFEST.md`, scenario → contract map |
| [**Phase 4**](TODO.md#phase-4--build-system--binary-surface-parity) | Build system & binary-surface parity | 100% export/ordinal parity (`MovieMakerMain @1`), delay-load/import parity, CI gates on PR |
| [**Phase 5**](TODO.md#phase-5--surface-by-surface-re-implementation-app--service-stack) | Surface-by-surface re-implementation | Engine (`MovieMakerCore`), 20 supporting DLLs/EXEs, service/plugin stack, Tier-2 satellites |
| [**Phase 6**](TODO.md#phase-6--resources-localization--ui-assets) | Resources, localization & UI assets | `.rsrc` type/ID/version parity across all targets with clean-room substituted payloads (`NOTICE`) |
| [**Phase 7**](TODO.md#phase-7--real-media--service-pipeline-decode--render--encode--publish) | Real media & service pipeline | Decode → D3D11/MF render → sink-writer encode → golden-comparable video & loopback publish |
| [**Phase 8**](TODO.md#phase-8--contract--regression-hardening) | Contract & regression hardening | Expanded `mmr-python` & CTest suites, `mmr-gui --selftest`, parser fuzzing, 23/23 quirk tests |
| [**Phase 9**](TODO.md#phase-9--11-verification--diff-closure) | 1:1 verification & diff closure | `analysis/PARITY_SCORECARD.md`, `DEVIATIONS.md`, visual/`.wlmp`/trace diffs, 1:1 claim statement |
| [**Phase 10**](TODO.md#phase-10--installation-registration--deployment-parity) | Installation, registration & deployment | Clean-VM installer, COM/`.wlmp`/`wlpg://`/AutoPlay registration, 10 exercise apps, clean uninstall |
| [**Phase 11**](TODO.md#phase-11--documentation--clean-room-deliverables) | Documentation & clean-room deliverables | Docusaurus/Jekyll sync, recreation-notes audit, per-module `analysis/AUDIT_TRAIL.md` |
| [**Phase 12**](TODO.md#phase-12--remix--modernization-explicitly-out-of-scope-for-11) | Remix / modernization (*out of scope for 1:1*) | Kept on separate branches so the 1:1 parity baseline never drifts |

See also the cross-phase trackers in [`TODO.md`](TODO.md#cross-phase-trackers):
[Tracker A — Unreconstructed Modules](TODO.md#tracker-a--analyzed-modules-with-no-src-target-yet) ·
[Tracker B — Stub Burn-Down](TODO.md#tracker-b--stub--annotation-burn-down) ·
[Tracker C — Binary-Surface Gaps](TODO.md#tracker-c--binary-surface-gaps-from-analysisbinarydiffdiff_analysismd) ·
[Tracker D — Parity Matrix](TODO.md#tracker-d--parity-matrix-template-populate-in-analysisparity_scorecardmd-in-phase-9).

## Quick Start

```
& "C:\Program Files\CMake\bin\cmake.exe" --build build --config Debug
build\bin\Debug\MovieMaker.exe
```

## Verification

```
python32\python.exe tests\mmr-python\run_tests.py     # contract suite
ctest --test-dir build --output-on-failure             # CTest
```

Contract suite should report `PASS=116 FAIL=0`. It is the behavioral
regression oracle for the DLL surfaces (see `tests/mmr-python/README.md`).

## Documentation

Full codebase documentation (every module, subsystem, test harness, and tool) is
available as a [Docusaurus](https://docusaurus.io/) site:

> **Read it online:** https://havaianasdestruido.github.io/WindowsMovieMakerDecomp/

Run it locally:

```
cd website
npm install
npm start
```

The docs source lives in [`website/docs/`](website/docs/) — see
[`website/README.md`](website/README.md) for the structure and how to contribute pages.

## Getting Oriented

- [`TODO.md`](TODO.md) — **phased work plan** for the 1:1 clean-room decompilation & reconstruction, with exit gates and trackers.
- [`ROADMAP.md`](ROADMAP.md) — build state, subsystem inventory, SDK fixes, gotchas.
- [`StubDesign.md`](StubDesign.md) — stub conventions, decision rules, hardening notes.
- [`QUIRKS.md`](QUIRKS.md) — 23 original bugs/quirks deliberately preserved.
- [`TODO_AUDIT.md`](TODO_AUDIT.md) — index of inline `TODO(reconstruction)` source annotations.
- [`THINKING_PROCESS.md`](THINKING_PROCESS.md) — reconstruction methodology and history.
- [`analysis/`](analysis/) — binary analysis output, one directory per module.
- [`src/`](src/) — reconstructed source, 29 CMake module directories (see `ROADMAP.md`).
- [`tests/`](tests/) — `mmr-python` (contracts), `mmr-gui` (GUI harness), `mmr-cli`, and per-module C++ tests.

## Requirements

- VS2022 Build Tools with the C++ ATL component
- CMake ≥ 3.20
- Windows SDK 10.0.26100.0
- WTL 10 headers under `src/WTL/` (from NuGet `WTLCSPkg`)
