# WindowsMovieMakerDecomp

Full decompilation/recreation of **Microsoft Windows Live Movie Maker 2012**
(codename Sundance, 16.4.3528.0331), extracted from a 2012 Live Essentials
setup and rebuilt from binary analysis as C++14 / ATL / WTL source.

Please read [THINKING_PROCESS.md](THINKING_PROCESS.md),
[ROADMAP.md](ROADMAP.md) and [QUIRKS.md](QUIRKS.md).

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

## Getting Oriented

- `ROADMAP.md` — build state, subsystem inventory, SDK fixes, gotchas.
- `StubDesign.md` — stub conventions, decision rules, hardening notes.
- `QUIRKS.md` — 23 original bugs/quirks deliberately preserved.
- `THINKING_PROCESS.md` — reconstruction methodology and history.
- `analysis/` — binary analysis output, one directory per module.
- `src/` — reconstructed source, 18 CMake targets (see `ROADMAP.md`).
- `tests/` — `mmr-python` (contracts) and `mmr-gui` (GUI harness).

## Requirements

- VS2022 Build Tools with the C++ ATL component
- CMake ≥ 3.20
- Windows SDK 10.0.26100.0
- WTL 10 headers under `src/WTL/` (from NuGet `WTLCSPkg`)
