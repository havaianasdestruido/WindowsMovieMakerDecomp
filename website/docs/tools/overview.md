---
sidebar_position: 1
title: Tools Overview
description: What lives in tools/ and which tool to reach for.
---

# Tools Overview

The `tools/` directory holds the project's automation. Everything is either a **build
wrapper**, a **verification harness**, or a **binary-analysis extractor**.

| Tool | Type | One-liner |
|---|---|---|
| [`build.ps1`](build-tools.md#buildps1) | Build | Configure/build wrapper with `-Config`, `-Target`, `-Clean`, `-Analyze` |
| [`test_all.py`](build-tools.md#test_allpy) | Verification | Launch MovieMaker.exe, observe process state, check window creation, measure stability |
| [`todo_audit.py`](build-tools.md#todo_auditpy) | Verification | Scan every authored source line for `TODO(reconstruction):` annotations |
| [`pe_analyzer.py`](analysis-tools.md#pe_analyzerpy) | Analysis | PE structure analysis, single / `--compare` / `--batch` |
| [`rtti_extractor.py`](analysis-tools.md#rtti_extractorpy) | Analysis | RTTI class-name extraction (the architecture map source) |
| [`diff_exports.py`](analysis-tools.md#diff_exportspy) | Analysis / gate | Export/import table comparison — **the export-parity gate** |
| [`string_analyzer.py`](analysis-tools.md#string_analyzerpy) | Analysis | ASCII/UTF-16 string extraction and classification |
| `resource_extractor.cpp` | Analysis | DUI resource extraction tool (own CMake project under `tools/CMakeLists.txt`) |
| `tools/analysis/` | Analysis | Deep-dive scripts: `analyze_moviemaker_exe.py`, `analyze_moviemakercore_dll.py`, `analyze_supporting_dlls.py`, `analyze_pe.py`, `disasm_key_funcs.py`, `extract_rtti.py`, `extract_strings_resources.py`, `full_dump.py`, ... |
| `tools/thirdparty/` | (gitignored) | Ghidra/VBox scripts — never committed |

## Which tool for which job?

- **"Did my build break parity?"** → `diff_exports.py`
- **"Is the app still launchable/stable?"** → `test_all.py`
- **"What did the original do here?"** → the `analysis/<Module>/` artifacts first;
  regenerate with `pe_analyzer.py` / `rtti_extractor.py` / `string_analyzer.py` only if
  needed
- **"What's still intentionally unfinished?"** → `todo_audit.py`
- **"Just build it"** → `build.ps1`

:::note No lint/format tooling

The repo deliberately has no clang-format/clang-tidy/.editorconfig at the root. Match
the existing style by eye (MSVC-era conventions — see
[Conventions](../reference/conventions.md)). Do not add lint tooling unless asked.

:::
