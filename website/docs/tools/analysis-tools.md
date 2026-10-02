---
sidebar_position: 3
title: Analysis Tools
description: pe_analyzer, rtti_extractor, diff_exports, string_analyzer, and the tools/analysis deep-dive scripts.
---

# Analysis Tools

These tools produced (and maintain) the `analysis/` artifacts — see
[Binary Analysis Artifacts](../methodology/binary-analysis.md).

## pe_analyzer.py

Advanced **Portable Executable analysis**:

```powershell
python tools\pe_analyzer.py <path_to_exe_or_dll>          # one binary
python tools\pe_analyzer.py --compare <original> <ours>   # original vs recreation
python tools\pe_analyzer.py --batch <directory>           # whole directory
```

Reports PE structure: sections, imports, exports, timestamps, debug directory (PDB
GUIDs).

## rtti_extractor.py

**RTTI class-name extraction** — the single most important tool for architecture
reconstruction. Walks `.?AV...` type descriptors to recover the original class names
(1360 unique names across 48 namespaces for the full binary set). The recovered names
shaped every subsystem: `CSundanceApp`, `X3DFieldNode`, `PublishDialogSkyDriveBehavior`,
...

## diff_exports.py

**The export-parity gate** — compares export and import tables between binaries:

```powershell
python tools\diff_exports.py <original.exe> <recreation.dll>
python tools\diff_exports.py --batch <orig_dir> <recon_dir>
```

Because [export parity is mandatory](../methodology/stub-design.md#ground-rules), run
this whenever a DLL's export surface changes. It compares names, ordinals, and
decorations.

## string_analyzer.py

**String extraction and classification** — dumps ASCII and UTF-16 strings from binaries
(5721 + 2560 in the original set), classifying them (dialog text, registry paths, format
strings, URLs) to map features and behavior.

## tools/analysis/ deep-dive scripts

| Script | Purpose |
|---|---|
| `analyze_moviemaker_exe.py` | focused launcher analysis |
| `analyze_moviemakercore_dll.py` | focused engine analysis |
| `analyze_supporting_dlls.py` | sweep of the supporting DLL set |
| `analyze_pe.py` | generic PE walkthrough |
| `disasm_key_funcs.py` | disassembly of key functions (needs a disassembler) |
| `extract_rtti.py` | RTTI extraction variant used for the master sweep |
| `extract_strings_resources.py` | strings + resource extraction |
| `full_dump.py` | everything, everywhere (outputs archived as `.7z` alongside) |

## resource_extractor.cpp

A **DUI resource extraction tool** (its own CMake project under `tools/CMakeLists.txt`)
for pulling DirectUI resources out of binaries — feeding the UIFILE/resource
reconstruction in `MovieMakerLang.dll`.

## tools/thirdparty/

Gitignored on purpose — Ghidra and VirtualBox scripts used during interactive analysis.
Never commit it.
