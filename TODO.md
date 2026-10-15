# TODO.md — Phased Plan for a Faithful 1:1 Clean-Room Decompilation & Reconstruction

> **Target:** Microsoft Windows Live Movie Maker 2012 (codename **Sundance**), `16.4.3528.0331`
> (`.ship.client.main.w5m4`), x86 PE32, MSVC 11.0 (VS2012), Windows 6.2+ — as shipped in
> Windows Live Essentials 2012 (`wlsetup-all.exe`), together with its companion client/service
> and plugin stack.
> **Scope:** `MovieMaker.exe` + `MovieMakerCore.dll` + the supporting native DLL/EXE set, plus
> the satellite modules, COM servers, protocol handlers, and managed publish/subscribe service
> plugins that the original installer registered.
> **Last reviewed:** 2026-10-15
> **Companion docs:** [`AGENTS.md`](AGENTS.md) (canonical rules) · [`StubDesign.md`](StubDesign.md)
> (stub/parity/security policy) · [`QUIRKS.md`](QUIRKS.md) (23 preserved original bugs) ·
> [`ROADMAP.md`](ROADMAP.md) (build state & SDK fixes) · [`TODO_AUDIT.md`](TODO_AUDIT.md)
> (source-level annotation index) · [`THINKING_PROCESS.md`](THINKING_PROCESS.md) (methodology
> narrative) · [`analysis/`](analysis/) (binary analysis evidence)

This file is the **authoritative phased work plan**: what is required, in what sequence, for
the decompilation and clean-room reconstruction of the original application and its service
surfaces to be **defensibly 1:1** with the reference binaries. It does not duplicate build
state (`ROADMAP.md`), stub policy (`StubDesign.md`), or the inline source-annotation index
(`TODO_AUDIT.md`) — it sequences them and defines the verification gates between them.

---

## How to use this file

- **Phases are gates, not walls.** Work inside a phase may overlap with the next one, but a
  phase only moves to `[x]` when its *Exit gate* is met with committed, reproducible evidence.
- **Checkbox legend:** `[ ]` not started · `[~]` in progress · `[x]` done (evidence linked) ·
  `[!]` blocked (state the blocker inline).
- **Evidence rule.** Never tick a box on the strength of "it compiles". A completed item must
  point to a contract check in `tests/mmr-python`, a CTest entry, an analysis spec, a parity
  scorecard row, or a CI run.
- **Spec-first clean-room firewall.** No `src/` implementation starts before its behavioral
  specification exists under `analysis/<Module>/spec.md` (Phase 2). Implementers work from
  specifications, contracts, and golden data — never by transcribing disassembly into `src/`
  (Phase 0).
- **Contract-per-behavior rule.** Any change to externally observable behavior (exports,
  HRESULTs, GUIDs, file formats, registry side effects, service/plugin contracts) lands with a
  regression contract in `tests/mmr-python` (submodule commit + parent gitlink bump), per
  `AGENTS.md` → *Definition of Done*.

### Status snapshot (2026-10-15)

| Metric | Value | Source of truth / regenerate command |
|---|---|---|
| Module `CMakeLists.txt` files building | 29 (`30` `src/` dirs incl. vendored `src/WTL/`; `30` CMake targets incl. `BaseTypes` `OBJECT` lib) | `find src -name CMakeLists.txt \| wc -l` |
| Target breakdown | 3 EXEs · 21 reference-mapped shared DLLs · 5 scaffolding targets (4 `STATIC` + `GPURenderer.dll`) | root `CMakeLists.txt` + `src/*/CMakeLists.txt` |
| Authored files under `src/` (excl. `src/WTL/`) | 492 | `find src -type f -not -path 'src/WTL/*' \| wc -l` |
| `src/MovieMakerCore/` files | 352 | `find src/MovieMakerCore -type f \| wc -l` |
| Tracked source files audited for `TODO(reconstruction)` | 608 files (`0` open annotations; note `--check` exits 1 when count is 0) | `python3 tools/todo_audit.py` |
| `.def` export files in `src/` | 20 on disk (19 wired in CMake; `WLXPhotoBase.def` unwired) | `find src -name "*.def" \| wc -l` |
| `.rc` resource scripts in `src/` | 2 (`moviemaker.rc`, `MovieMakerLang.rc`; 27 targets lack `.rc`) | `find src -name "*.rc"` |
| Contract checks (32-bit `ctypes` suite) | 116 expected pass, 0 fail | `python32\python.exe tests\mmr-python\run_tests.py` |
| CTest entries wired in root `CMakeLists.txt` | 2 (`sanity_MovieMaker_exe_exists`, `sanity_MovieMakerCore_dll_exists`) | `ctest --test-dir build` |
| GUI harness headless self-test | 128 checks across 21 DLLs | `mmr-gui.exe --selftest <binDir>` |
| Per-module test directories under `tests/` | 45 (3 submodules + 42 native/C# probe dirs; 127 hardcoded `undecomp\` refs) | `find tests -mindepth 1 -maxdepth 1 -type d \| wc -l` |
| Remaining `E_NOTIMPL` stubs inventoried | 90 (0 CRITICAL / 4 HIGH / 19 MEDIUM / 67 LOW) | `analysis/E_NOTIMPL/inventory.md` |
| Documented original quirks preserved | 23 | `QUIRKS.md` |
| Analysis directories | 62 (+3 top-level synthesis docs; 16 modules have `dynamic_analysis.md`) | `find analysis -mindepth 1 -maxdepth 1 -type d \| wc -l` |
| Modules analyzed with **no** `src/` counterpart | ~30 (see Tracker A) | Tracker A below |
| Git submodules (`tests/mmr-*` + `apps/*`) | 13 | `.gitmodules` |

### Phase map

| Phase | Name | One-line goal | Exit gate | Status |
|---|---|---|---|---|
| [0](#phase-0--clean-room-charter-provenance--legal-hygiene) | Clean-room charter, provenance & legal hygiene | Make the clean-room claim auditable and purge raw dumps from git | Provenance ledger + raw-dump cleanup + CI leak guard | `[~]` |
| [1](#phase-1--reference-freeze--module-inventory) | Reference freeze & module inventory | One authoritative, regenerable binary/service inventory with scope tiers | `analysis/inventory.json` published, doc contradictions reconciled | `[~]` |
| [2](#phase-2--static-specification-extraction-per-module) | Static specification extraction | Clean-room behavioral specs an implementer can code from, per module | `analysis/<Module>/spec.md` for every Tier-1 module + UIFILE decision | `[~]` |
| [3](#phase-3--dynamic-analysis--golden-oracles) | Dynamic analysis & golden oracles | Observable runtime truth: traces, golden corpus, scenario definitions | Golden corpus hashed + 24/24 Tier-1 traces + scenario→contract map | `[~]` |
| [4](#phase-4--build-system--binary-surface-parity) | Build system & binary-surface parity | Exports, imports, delay-load linkage, sections, and CI gates match reference | 100% export parity + import delta closed/registered + CI on PR | `[~]` |
| [5](#phase-5--surface-by-surface-re-implementation-app--service-stack) | Surface-by-surface re-implementation | Real behavior for engine, supporting DLLs, satellites, and service plugins | Per-module contracts green, `E_NOTIMPL` inventory burned down | `[~]` |
| [6](#phase-6--resources-localization--ui-assets) | Resources, localization & UI assets | `.rsrc`, dialogs, strings, DUI/Ribbon markup, manifests, satellites | Resource ID/type/lang parity (with clean-room substitution policy) | `[ ]` |
| [7](#phase-7--real-media--service-pipeline-decode--render--encode--publish) | Real media & service pipeline | Decode → render → encode → publish produces real media & loopback uploads | Golden-comparable video export + end-to-end mock-service publish | `[ ]` |
| [8](#phase-8--contract--regression-hardening) | Contract & regression hardening | Nothing can silently regress across DLLs, workflows, parsers, or quirks | CI green on PR, CTest wired, quirk + fuzz + security + workflow coverage | `[~]` |
| [9](#phase-9--11-verification--diff-closure) | 1:1 verification & diff closure | Prove or formally accept every remaining difference against the reference | `analysis/PARITY_SCORECARD.md` + `DEVIATIONS.md` + 1:1 claim statement | `[ ]` |
| [10](#phase-10--installation-registration--deployment-parity) | Installation, registration & deployment | Install, register COM/shell/protocols, associate `.wlmp`, and uninstall cleanly | Clean-VM install → launch → scenario → clean uninstall | `[ ]` |
| [11](#phase-11--documentation--clean-room-deliverables) | Documentation & clean-room deliverables | Third party can independently audit the entire 1:1 clean-room claim | Docs site + per-module spec→impl→contract audit trail | `[~]` |
| [12](#phase-12--remix--modernization-explicitly-out-of-scope-for-11) | Remix / modernization (**out of scope**) | Kept strictly separate so the 1:1 baseline never drifts | n/a — separate branch + explicit user request | `[ ]` |

---

## Phase 0 — Clean-room charter, provenance & legal hygiene

**Goal:** make the clean-room claim *auditable*, eliminate raw disassembly/binary dumps from the
tracked tree, and guarantee no Microsoft binary or proprietary asset ever enters git.

- [ ] **Record reference provenance in `analysis/PROVENANCE.md`:** installer source (Windows
      Live Essentials 2012 `wlsetup-all.exe`, build `16.4.3528.0331`, branch
      `ship.client.main.w5m4`, PE timestamp `2014-04-01`), extraction procedure, and the
      **SHA-256, file size, and PDB GUID of every reference PE** — metadata and hashes only,
      never the binaries.
- [ ] **Remediate tracked raw disassembly & raw section dumps currently in the repo:** replace
      verbatim machine-code / raw-bytes dumps with clean-room prose specifications and add
      their patterns to `.gitignore`:
      - `analysis/WLXAlbumDownloadWizard/disasm_full.txt` (69,051 lines of raw disassembly)
      - `analysis/WLXPhotoCinematic/disasm.txt` (raw disassembly)
      - `analysis/MovieMakerCore/rdata_raw.txt` (2,641,305 lines of raw `.rdata` dump),
        `analysis/MovieMakerCore/data_raw.txt`, `analysis/MovieMakerCore/rawdata_data.txt`
      - `tools/analysis/disasm_output_full.7z` and `tools/analysis/full_dump_output.7z`
      - `analysis/WLFacebookPlugin/dotnet_temp/bin/` and `obj/` (committed .NET build caches
        from the reflection helper) and `tests/OtherDlls/test_npwlp` (duplicate extensionless
        source file alongside `test_npwlp.c`).
- [ ] **Add a CI and pre-commit leak guard:** reject `undecomp/**`, `*.dll`, `*.exe`, `*.pdb`,
      `*.lib`, `*.exp`, `*.obj`, `*.res`, `*.7z`, and raw disassembler/decompiler dumps
      (`*disasm*.txt`, `*_raw.txt`, `*.asm`, `*.i64`, `*.idb`, `*.gpr`, `*.ghidra`, `*.bndb`)
      from being staged or merged.
- [ ] **Formalize the clean-room role split:**
      - **Spec writers (dirty room):** inspect reference binaries in `undecomp/`, run dynamic
        tracing in a VM, and author behavioral specifications under `analysis/<Module>/spec.md`
        plus black-box test contracts.
      - **Implementers (clean room):** write C++14/ATL/WTL code under `src/` working strictly
        from `analysis/<Module>/spec.md`, public Windows SDK headers, `StubDesign.md`,
        `QUIRKS.md`, and contract results — never from a disassembler window.
- [ ] **Codify the non-transcription rule:** no verbatim assembly, decompiler pseudocode, or
      hex byte-blobs copied from reference `.text`/`.rdata` inside `src/`. Document behavior
      in neutral prose or mathematical/state-machine descriptions and cite via "recreation
      notes" comments at non-obvious call sites.
- [ ] **Asset & resource copyright policy:** Microsoft-owned icons, bitmaps, cursors, string
      tables, help content, binary `.duxt` (UIFILE) blobs, and Ribbon XML are **not**
      redistributed verbatim. Phase 6 defines how resource *structural parity* (resource types,
      IDs, dimensions, LCIDs, control IDs) is achieved using clean-room authored replacements,
      documented in a root `NOTICE` file alongside `LICENSE`.
- [ ] **License & attribution hygiene:** confirm root `LICENSE` covers authored reconstruction
      code, preserve upstream licensing for vendored `src/WTL/` (`WTLCSPkg`), and keep the
      research/interoperability disclaimer identical across `README.md`, `NOTICE`,
      `_includes/footer.html`, and `website/docs/intro.md`.

**Exit gate:** `analysis/PROVENANCE.md` and `NOTICE` committed; tracked raw disassembly/section
dumps replaced by prose specs and removed from the working tree; CI leak guard enforcing binary
and dump exclusions on every PR; clean-room firewall referenced from `AGENTS.md`.

---

## Phase 1 — Reference freeze & module inventory

**Goal:** one authoritative, machine-readable inventory of every binary and service surface in
scope, reconciling all historical count and size contradictions across existing docs.

- [ ] **Generate `analysis/inventory.json` (+ generated markdown summary):** consolidate
      `analysis/CROSS_BINARY_MASTER.md` (39 binaries), `analysis/FINAL_SYNTHESIS.md`, and
      `analysis/CrossDllImports/*` into a single schema recording per binary: exact filename,
      PE type (`EXE` / `DLL` / `.NET DLL` / resource-only `DLL`), file size, virtual image
      size, `.text` and `.rsrc` sizes, PDB name + GUID, export list (name, ordinal, decoration,
      calling convention), static vs delay-loaded import DLLs, RTTI class count, registered
      COM CLSIDs/IIDs/TypeLibs, and scope tier.
- [ ] **Reconcile documented contradictions across the repo and fix them at the source:**
      - *Target count:* `ROADMAP.md` still states `18` CMake targets (`375+` files); root
        `CMakeLists.txt` actually builds **29** module subdirectories (`30` CMake targets
        counting the `BaseTypes` `OBJECT` library in `src/WLXPhotoBase/CMakeLists.txt`, `492`
        authored files under `src/`).
      - *Binary types:* `analysis/CROSS_BINARY_MASTER.md` lists `MovieMakerPreviewClient.exe`
        (EXE) and `WLXTranscode.dll` (DLL), whereas `src/` and `ROADMAP.md` build
        `MovieMakerPreviewClient.dll` (4 COM exports) and `WLXTranscode.exe` (`WIN32` EXE, 0
        exports) — verify against the reference PEs and correct the synthesis tables.
      - *Image sizes:* `analysis/BinaryDiff/diff_analysis.md` measures `MovieMaker.exe` at
        `122,048` bytes (`0x1D000` virtual, `.text` `4,848` B, `.rsrc` `97,632` B) and
        `MovieMakerCore.dll` at `10,583,240` bytes (`10.1 MB`, `.text` `5.5 MB`, `.rsrc`
        `4.0 MB`), whereas `analysis/MovieMakerExe/deep_analysis.md` says `54 KB (115,200 B)`
        and `analysis/CROSS_BINARY_MASTER.md` says `MovieMakerExe.exe` `1.7 MB` and
        `MovieMakerCore.dll` `6.5 MB`. Re-measure from the reference set and mark stale figures.
- [ ] **Lock the 4-tier scope taxonomy:**
      - **Tier 1A — Reference-mapped targets already in `src/` (24 targets):**
        - *3 EXEs:* `MovieMaker.exe`, `WLXTranscode.exe`, `WLXCodecHost.exe`.
        - *21 Shared DLLs:* `MovieMakerCore.dll`, `MovieMakerLang.dll` (resource-only),
          `MovieMakerPreviewClient.dll`, `WLXPhotoBase.dll` (+ `BaseTypes`), `WLXPipeline.dll`,
          `WLXPipetran.dll`, `WLXSlideshow.dll`, `WLXPhotoCinematic.dll`,
          `WLXMovieLibrary.dll`, `WLXVideoTrim.dll`, `WLXMP4Parser.dll`,
          `WLXFaceRecognition.dll`, `WLMFDS.dll`, `WLMFReadWrite.dll`,
          `WLXMediaPublishSubscribe.dll`, `MetadataSys.dll`, `DmxBici.dll`,
          `WLXPhotoSqm.dll`, `UXCore.dll`, `uxctl.dll`, `wlidcli.dll`.
      - **Tier 1B — Reconstruction-side scaffolding targets in `src/` (5 targets, no reference PE twin):**
        - `MediaCatalog` (`STATIC`, `src/Media`), `PlaybackEngine` (`STATIC`, `src/Playback`),
          `ProjectManager` (`STATIC`, `src/Project`), `TimelineEngine` (`STATIC`,
          `src/Timeline`), and `GPURenderer.dll` (`SHARED`, `src/Renderer`). Decide per target
          whether to fold its code into `MovieMakerCore.dll` (so `MovieMakerCore.dll` does not
          import a non-reference `GPURenderer.dll`) or retain as internal static helpers only.
      - **Tier 2 — Analyzed satellite & service/plugin modules not yet in `src/` (~25 modules, Tracker A):**
        Photo Gallery / shell extensions / acquisition wizards / background scheduler / COM
        proxy-stub / protocol handler / `.NET 4.0` social publish plugins.
      - **Tier 3 — Shared Microsoft / third-party runtime infrastructure (Tracker A):**
        `Imaging.dll`, `RSCMFT.dll`, `MPG4DEMUX.dll`, `D3DCOMPILER_46.dll`, `WLAVRes.dll`,
        `sqmapi.dll`, `wlbici.dll`, `wldcore.dll`, `wldlog.dll`, `wlidux.dll`,
        `uxcalendar.dll`, `uxcontacts.dll`, `uxctlloc.dll` — documented surface compatibility
        or delay-load survival only.
- [ ] **Produce a per-module Surface Card for all 24 Tier-1A targets:** exact exports with
      ordinals and C/C++ decorations (`Name@N` vs undecorated vs MSVC mangled), calling
      conventions (`__stdcall` default vs `__cdecl` exceptions in `MovieMakerMain` and `UXCore`
      `Resources.cpp`), COM CLSID/IID/TypeLib/ProgID table, apartment threading model, static
      vs delay-loaded imports, and Win8→Win10 SDK shims required.
- [ ] **Record the x86 (PE32) architecture lock:** original binaries are 32-bit x86 with
      32-bit pointer layout and `__thiscall`/`__stdcall` decorations; `build64/` is gitignored
      scratch and x64 is explicitly deferred to Phase 12.

**Exit gate:** `analysis/inventory.json` committed and regenerable via script; all target/size/type
contradictions across `ROADMAP.md` and `analysis/*.md` resolved; every Tier-1A target has a
complete surface card.

---

## Phase 2 — Static specification extraction (per module)

**Goal:** produce clean-room behavioral specifications complete enough that an implementer
never needs to open a disassembler or decompiler.

- [ ] **Complete RTTI & vtable hierarchy specs for all Tier-1A modules:** class inheritance,
      multiple-inheritance / COM interface offsets, vtable slot ordering, and local functor
      classes (`@?1??...` lambdas, `QUIRKS.md` #14), extending
      `analysis/MovieMakerCore/vtable_hierarchy.md` across all modules via
      `tools/rtti_extractor.py`.
- [ ] **Stamp and refresh PE import/export/cross-DLL correlation artifacts:** regenerate
      `analysis/CrossDllImports/*` with `tools/pe_analyzer.py` and `tools/diff_exports.py`,
      stamping each output with tool version, date, and command line.
- [ ] **String, constant & resource census per module:** extract string literal tables,
      error/debug trace formats, registry paths, XML schema element/attribute names, and
      resource trees (`tools/string_analyzer.py`, `tools/resource_extractor.cpp`) modeled on
      `analysis/MovieMakerLang/resource_analysis.md`.
- [ ] **Recover the binary `DUXT` / `UIFILE` layout format (`analysis/WLXPhotoLibraryDuiResources/uifile_format.md`):**
      document the binary token/opcode stream read by `CRMDUIParser::Load` and
      `SundanceMainElementBehavior::LoadLayout` (which currently locates and bound-checks the
      `DUXT` `RCDATA` resource but defers tree population — see `TODO_AUDIT.md`). If full
      binary-schema recovery is infeasible because `directui.dll`/`uxcore.dll` is an external
      dead-path surface, formally record the owner-drawn `SundanceMainVisual` shell
      (`src/MovieMakerCore/UI/SundanceMainVisual.{h,cpp}`) as the accepted clean-room
      equivalent in Phase 9.
- [ ] **Complete the COM & GUID specification (`analysis/COMGuids/guids.md`):**
      - Map all `77+` registered CLSIDs (`54+` in `MovieMakerCore.dll`, `7` in
        `WLXFaceRecognition.dll`, `2` in `WLXPhotoCinematic.dll`, `6` in `WLXPhotoAcq.dll`,
        `8` in `WLXMediaPublishSubscribe.dll`, `8` Voyager theme/transform CLSIDs in
        `WLXPhotoVoyager.dll`, `CLSID_AlbumDownloadLauncher` in
        `AlbumDownloadProtocolHandler.dll`, etc.), their implemented IIDs, `ThreadingModel`,
        and `.rgs` registration scripts (`analysis/RegRes/analysis.md`).
      - **Distinguish true original quirks from reconstruction placeholders:** `QUIRKS.md` #3
        (null `CLSID_MovieMakerCore` `{00000000-...}`) is an original quirk, whereas
        `QUIRKS.md` #4 (`{A1B2C3D4-E5F6-4829-9B01-234567890ABC}` sequential pseudo-GUIDs in
        `src/MovieMakerCore/ComFactory.h:33-47`) is a *reconstruction placeholder* noted as
        "the originals were never extracted". Map the real CLSIDs/IIDs from
        `analysis/COMGuids/guids.md` so Phase 5 can replace the placeholder GUIDs (updating
        contracts in lockstep).
- [ ] **Complete the registry, file-system & IPC specification (`analysis/CrossDllImports/registry_fileio_patterns.md`):**
      `HKCU\Software\Microsoft\Windows Live\Movie Maker` and `Photo Gallery` keys, MRU lists,
      `%LOCALAPPDATA%\Microsoft\WL\MovieMaker\auth.dat` DPAPI store, auto-save/recovery paths,
      file associations (`.wlmp`), protocol handlers (`wlpg://`), and out-of-process RPC/named-pipe
      contracts (`WLXCodecHost.exe`, `WLXTranscode.exe`, `MovieMakerPreviewClient`).
- [ ] **Re-verify the apartment & thread-safety audit (`analysis/ThreadModel/thread_model_audit.md`):**
      cross-check STA vs MTA vs Both COM apartments, `D3D11_CREATE_DEVICE_SINGLETHREADED`
      (`QUIRKS.md` #9), background grinder/orchestrator worker threads, and lock hierarchies.
- [ ] **Author `analysis/<Module>/spec.md` for every Tier-1A module and `MovieMakerCore` subsystem:**
      specify public/internal interfaces, method pre/post-conditions, exact `HRESULT` return
      matrix (including intentional `S_FALSE` no-op semantics per `StubDesign.md` and custom
      facilities such as `FACILITY_WLXVIDEOTRIM` / `AVS_E_*`), memory ownership (e.g.,
      `WLXPhotoBase` process-heap `Exception::operator new` vs `BasePrivate::Delete`), bounds
      on untrusted inputs, and preserved quirks from `QUIRKS.md`.
- [ ] **Spec the client/online service & plugin protocols (`analysis/PublishPlugins/spec.md`):**
      document the native↔managed interop interfaces (`PublishPluginsInterop.dll`,
      `SubscribePluginsInterop.dll`), `wlidcli.dll` Windows Live ID RPS/Passport token flow,
      `WLXMediaPublishSubscribe.dll` provider state machine, and the wire/API contracts of
      `WLFacebookPlugin`, `WLFlickrPlugin` (OAuth 1.0), `WLVimeoPlugin` (OAuth 1.0),
      `WLYouTubePlugin` (ClientLogin/GData), and SkyDrive/OneDrive so Phase 5/7 can implement
      them against a local loopback mock service.
- [ ] **Fence off dead-but-shipped surfaces in the specs:** legacy `.wlmp` v1 project reader
      (`QUIRKS.md` #19), 118 dummy D3DX11 texture codec classes (`QUIRKS.md` #13), 22 unimplemented
      stub transitions (`QUIRKS.md` #18), gated `UXCore` `g_*_mark` paths (`19` dead stubs),
      inert add-in host, and SQM/BICI telemetry (`WLXPhotoSqm`, `DmxBici`) — mark each in its
      `spec.md` so no contributor "completes" dead code and breaks 1:1 parity.

**Exit gate:** every Tier-1A module has `analysis/<Module>/spec.md` + COM/resource/registry/threading
census; placeholder vs genuine quirks separated; `UIFILE`/`.duxt` format either fully specified
or formally assigned to the `SundanceMainVisual` deviation record.

---

## Phase 3 — Dynamic analysis & golden oracles

**Goal:** establish observable runtime ground truth from the reference binaries so contracts
test real 1:1 behavior rather than guesses.

- [ ] **Complete dynamic trace coverage across all 24 Tier-1A modules:**
      16 modules already have `analysis/<Module>/dynamic_analysis.md` (`MovieMakerPreviewClient`,
      `SharedMFDlls`, `WLMFDS`, `WLMFReadWrite`, `WLXCodecHost`, `WLXFaceRecognition`,
      `WLXMP4Parser`, `WLXMediaPublishSubscribe`, `WLXMovieLibrary`, `WLXPhotoBase`,
      `WLXPhotoCinematic`, `WLXPipeline`, `WLXPipetran`, `WLXSlideshow`, `WLXTranscode`,
      `WLXVideoTrim`). Capture and commit `dynamic_analysis.md` for the remaining 9 Tier-1A
      targets: `MovieMakerExe`, `MovieMakerCore`, `MovieMakerLang`, `MetadataSys`, `DmxBici`,
      `WLXPhotoSqm`, `UXCore`, `uxctl`, and `wlidcli`.
- [ ] **Build the Golden Corpus (files stored outside git; SHA-256 + metadata manifest committed at `analysis/GOLDEN_MANIFEST.md`):**
      - `.wlmp` project files exercising every extent type (video, photo, audio/music,
        narration, title/caption/credits), all 7 AutoMovie themes, all 23 visual effects, and
        all 70 active + 22 stub transitions in `WLXPipetran`.
      - Golden exported MP4/WMV media files and stream metadata for every built-in publish
        profile (resolution, frame rate, video/audio codec, bitrate, container boxes/headers).
      - Golden WIC thumbnails (`160x120`), waveform cache sections (`CachedWFSection`),
        auto-save recovery files, and registry/MRU snapshots.
- [ ] **Define executable end-to-end scenario scripts:**
      1. Cold start (`MovieMaker.exe` → `SetDllDirectoryW(L"")` → delay-load `MovieMakerCore.dll`
         → `MovieMakerMain` → single-instance mutex → D3D11/WARP + MF + WIC init → main window).
      2. Cold start with legacy delay-loaded DLLs (`WLXPhotoSqm.dll`, `DmxBici.dll`,
         `wlidcli.dll`, `UXCore.dll`) absent (`QUIRKS.md` #11).
      3. Media import (video/photo/audio, supported vs unsupported container rejection HRESULTs,
         thumbnail & waveform generation, proxy transcode trigger).
      4. Storyboard editing (add, reorder, split, trim start/end, speed, volume/fade/ducking,
         drag-and-drop `IDataObject` / `CF_HDROP`).
      5. Transitions, visual effects, pan/zoom (Ken Burns via `WLXPhotoCinematic`), and
         DirectWrite text/caption/credit overlays.
      6. Theme application/removal and AutoMovie composition (`AutofitProcess` / `Conductor`).
      7. Preview transport (play, pause, stop, scrub, frame-step, full-screen preview via
         `MovieMakerPreviewClient`).
      8. `.wlmp` save, Save As (untitled redirect), reopen, and byte-level round-trip stability.
      9. Undo/redo transaction stack + system/internal clipboard cut/copy/paste across all edits.
      10. Crash/auto-save recovery (`AutoSaveManager`) and VEH+SEH double-exception survival
          (`QUIRKS.md` #1).
      11. Publish to local file (WMV / H.264 MP4 across profiles) and publish to online service
          via local loopback mock provider.
- [ ] **Publish the Scenario → Contract Map (`analysis/SCENARIO_CONTRACT_MAP.md` & `tests/mmr-python/README.md`):**
      map every scenario and Tier-1A export to the exact contract check in `tests/mmr-python`
      or GUI check in `tests/mmr-gui --selftest`, highlighting uncovered gaps for Phase 8.
- [ ] **Document unobservable/sunset runtime surfaces:** Windows Live ID (`wlidcli`), live
      Facebook/Flickr/Vimeo/YouTube/SkyDrive endpoints, PlayReady DRM (`AuthProvider::AcquireLicense`),
      and `D3DX11CreateEffectFromMemory` (`d3dx11compat.cpp`) — pin their exact expected
      offline/mock or `E_NOTIMPL` behavior in the oracle.

**Exit gate:** `analysis/GOLDEN_MANIFEST.md` and `analysis/SCENARIO_CONTRACT_MAP.md` committed;
all 24 Tier-1A modules have `dynamic_analysis.md`; every sunset/external surface has a pinned
oracle rule.

---

## Phase 4 — Build system & binary-surface parity

**Goal:** make every rebuilt PE match the reference binary's external surface — exports,
ordinals, decorations, internal module name, static and delay-load import tables, and
resources — enforced automatically in CI.

- [ ] **Fix `MovieMakerCore.dll` over-exporting (4,224 exports → 1 export `MovieMakerMain @1`, Tracker C #3):**
      `src/MovieMakerCore/MovieMakerCore.def` already lists only `MovieMakerMain @1`, but MSVC
      `link.exe` union-merges `.def` entries with every symbol marked `__declspec(dllexport)` in
      constituent object files. Strip `__declspec(dllexport)` from internal engine macros
      (`MOVIECORE_API`, `RIBBON_API`, `STORYBOARD_API`, `DATASTRUCT_API`, `AVSOURCE_API`,
      `SUNDANCE_ADDIN_API`) in `src/MovieMakerCore/exports.h`, `src/exports.h`,
      `src/MovieMakerCore/HMRAVSource/HMRAVSourceTypes.h`,
      `src/MovieMakerCore/Preview/PreviewPresenter.h`, and
      `src/MovieMakerCore/AddIn/SundanceAddInContract.h`, and remove `__declspec(dllexport)`
      from static-library classes `PlaybackEngine` (`src/Playback/PlaybackEngine.h`) and
      `TimelineDispatcher` (`src/Timeline/TimelineDispatcher.h`) so `MovieMakerCore.dll`
      exports **only** `MovieMakerMain` at ordinal 1.
- [ ] **Verify PE export directory internal module name (Tracker C #4):** ensure `MovieMakerCore.def`
      `LIBRARY MovieMakerCore` produces `MovieMakerCore.dll` (not `MOVIECORE.dll`) in the PE
      export directory header, and check all other 20 shared DLLs for exact case-sensitive
      `LIBRARY <Name>` parity.
- [ ] **Wire `WLXPhotoBase.def` and audit all 21 shared DLL export surfaces:**
      - `src/WLXPhotoBase/WLXPhotoBase.def` exists on disk (56 mangled exports `@1..@56`),
        **but `src/WLXPhotoBase/CMakeLists.txt` omits `WLXPhotoBase.def` from
        `add_library(WLXPhotoBase SHARED ...)`** and sets `VERSION 2012.1.0` instead of
        `16.4.3528.331`. Wire `WLXPhotoBase.def` into `src/WLXPhotoBase/CMakeLists.txt` and
        unify its version stamp.
      - Audit all 20 `.def` files (`DmxBici`, `MetadataSys`, `MovieMakerCore`,
        `MovieMakerPreviewClient`, `UXCore`, `WLMFDS`, `WLMFReadWrite`, `WLXFaceRecognition`,
        `WLXMP4Parser`, `WLXMediaPublishSubscribe`, `WLXMovieLibrary`, `WLXPhotoBase`,
        `WLXPhotoCinematic`, `WLXPhotoSqm`, `WLXPipeline`, `WLXPipetran`, `WLXSlideshow`,
        `WLXVideoTrim`, `uxctl`, `wlidcli`) so every ordinal, calling convention, and
        decoration matches the Phase-1 surface cards (`MovieMakerLang.dll` has 0 exports via
        `/NOENTRY`).
- [ ] **Fix `MovieMaker.exe` launcher linkage & import parity (`src/MovieMaker/CMakeLists.txt` & `src/MovieMaker/main.cpp`, Tracker C #5 & #6):**
      - In `src/MovieMaker/CMakeLists.txt`, remove the broken configure-time check
        `if(EXISTS "${CMAKE_BINARY_DIR}/lib/MovieMakerCore.lib")` (which evaluates to `FALSE`
        on a fresh `cmake -S . -B build` configure because `MovieMakerCore.lib` has not been
        built yet!) and link the CMake target `MovieMakerCore` + `delayimp` directly with
        `/DELAYLOAD:MovieMakerCore.dll`.
      - In `src/MovieMaker/main.cpp`, match the reference launcher (`analysis/MovieMakerExe/deep_analysis.md`):
        call `SetDllDirectoryW(L"")` for DLL search-order hardening, import
        `?Delete@BasePrivate@@YAXPAX@Z` from `WLXPhotoBase.dll` instead of the locally invented
        `_WLXPhotoBase_Init@0`, invoke the delay-loaded `MovieMakerMain` directly (preserving
        the VEH + SEH `0xe06d7363` double-exception filter, `QUIRKS.md` #1), and drop the extra
        `SHELL32.dll` (`CommandLineToArgvW`), `COMDLG32.dll`, and `COMCTL32.dll` imports.
- [ ] **Fix `MovieMakerCore.dll` static and delay-load import tables (`src/MovieMakerCore/CMakeLists.txt`, Tracker C #1 & #8):**
      - `src/MovieMakerCore/CMakeLists.txt` passes `/DELAYLOAD` for `WLXPhotoSqm.dll`,
        `DmxBici.dll`, `MetadataSys.dll`, `wlidcli.dll`, `UXCore.dll`, and `uxctl.dll`, **but
        does not link their import targets in `target_link_libraries(MovieMakerCore PRIVATE ...)`**,
        causing `link.exe` to ignore `/DELAYLOAD` (`LNK4199`) and omit them from the PE
        delay-load directory. Link those 6 targets in `target_link_libraries` and ensure the
        engine references their entry points so the delay-load descriptors are emitted.
      - Reconcile the remaining import DLL delta against the reference 34-DLL import list
        (`d3d9.dll`, `D3DCOMPILER_46.dll`, `dwmapi.dll`, `dxva2.dll`, `ESENT.dll`, `OLEACC.dll`,
        `PROPSYS.dll`, `UxTheme.dll`, `VERSION.dll`, `WindowsCodecs.dll`, `WINMM.dll`) and
        remove or internalize the non-reference `GPURenderer.dll` (`src/Renderer`),
        `COMCTL32.dll`, `COMDLG32.dll`, `MFReadWrite.dll`, and `WININET.dll` imports.
- [ ] **Upgrade `tools/diff_exports.py` into a standalone CI gate:**
      - Fix the bug in `parse_imports()` (`tools/diff_exports.py:185-209`) where seeking to
        `dll_off` to read the imported DLL name never restores the file cursor to the next
        20-byte `IMAGE_IMPORT_DESCRIPTOR`.
      - Add verification of export ordinals, PE export header module name, and delay-load
        import descriptors (`IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT`).
      - Add `--manifest analysis/inventory.json` mode so CI can verify export/import parity on
        every PR without requiring the gitignored `undecomp/` directory, and return a non-zero
        exit code on any mismatch.
- [ ] **Record the Toolchain / ABI / PE Header Decision Record (Tracker C #7, #9–#12):**
      document the disposition of `MSVCR110.dll` (VS2012) vs `MSVCP140.dll`/`VCRUNTIME140.dll`/UCRT
      (VS2022), linker version `11.00` vs `14.44`, OS/subsystem version stamps (`6.02`),
      Control Flow Guard (`/guard:cf` enabled in root `CMakeLists.txt` lines 97/105 vs absent
      in original), `/RELEASE` PE checksum computation, and `.rdata`/`.fptable` section merging.
      Either disable `/guard:cf` + add `/RELEASE` + `/MERGE` flags in CMake to close gaps #9,
      #11, #12, or log them in `DEVIATIONS.md` (Phase 9).
- [ ] **Re-enable CI on push/PR (`.github/workflows/build-projects.yml`):**
      uncomment `push` and `pull_request` triggers, build all 29 targets in one shared build
      tree on `windows-2022`, and run: (1) `python tools/diff_exports.py --manifest analysis/inventory.json build/bin/Debug`,
      (2) `ctest --test-dir build -C Debug --output-on-failure`, (3) `tests/mmr-python/run_tests.py`
      with `WMMR_DLL_DIR=build/bin/Debug`, and (4) `python3 tools/todo_audit.py` (adding a
      `--max 0` / `--require-clean` flag, since `--check` currently fails when 0 annotations
      exist) + the Phase-0 leak guard.

**Exit gate:** all 29 targets build clean; `MovieMakerCore.dll` exports only `MovieMakerMain @1`
under module name `MovieMakerCore.dll`; `WLXPhotoBase.def` and launcher/engine delay-loads
wired; `tools/diff_exports.py --manifest` passes 100% in CI on every PR.

---

## Phase 5 — Surface-by-surface re-implementation (app & service stack)

**Goal:** replace remaining stubs and placeholders with faithful 1:1 implementations across the
core engine, the 20 supporting DLLs/EXEs, the Tier-2 satellites, and the client/online service
plugins — burning down `analysis/E_NOTIMPL/inventory.md` (`90` entries: `4` HIGH, `19` MEDIUM,
`67` LOW) while preserving all intentional quirks and dead-code stubs.

### 5.1 Engine core — `src/MovieMakerCore/` (39 inventoried stubs: 4 HIGH, 11 MEDIUM, 24 LOW)

- [ ] **`SundanceApp/` (16 stubs — 2 HIGH, 12 MEDIUM, 2 LOW):**
      - Guarantee instantiation of `m_pImportController`, `m_pExportController`,
        `m_pUndoManager`, `m_pClipboardManager`, and `m_pPlaybackController` during
        `SundanceAppMain::Initialize()` so the 13 null-controller guards in
        `SundanceAppMain.cpp` (`ImportFiles`, `PublishMovie`, `PublishToService`, `Undo`,
        `Redo`, `CutSelection`, `CopySelection`, `PasteFromClipboard`, `StartPlayback`,
        `StopPlayback`, `PausePlayback`, `OnRibbonCommand(kRibbonCmdDelete)`,
        `OnRibbonCommand(kRibbonCmdSaveMovie)`) are never hit in normal operation, and match
        the reference error `HRESULT` when invoked uninitialized.
      - Verify `MediaBrowser::CreateDataObject` (`MediaBrowser.cpp`) and
        `DynamicDataObjectWrapper` (`DragDropStub.cpp`) across shell drag-and-drop and
        clipboard `CF_HDROP` / internal clip formats, and refresh `analysis/E_NOTIMPL/inventory.md`
        for items already resolved in the `2026-10-08` pass (`TODO_AUDIT.md`).
      - Verify command-line parsing (`CommandLineParser.cpp`), single-instance mutex
        `Global\WindowsLiveMovieMaker_Sundance_SingleInstance` (`QUIRKS.md` #2),
        `AutoSaveManager`, and deterministic shutdown ordering.
- [ ] **`StoryboardManager/` + `Serialization/` + `Theme/` + `Background/`:**
      - Byte-identical `.wlmp` XML serialization round-trip against the Phase-3 golden project
        corpus (`SerializationReader.cpp`, `SerializationWriter.cpp`,
        `BoundPropertyDictionary.cpp`, `DatabasePropertyManager.cpp`, `ModElements.cpp`,
        `SnapShot.cpp`), preserving the `IXmlWriter` error-path leak (`QUIRKS.md` #23) and the
        dead v1 legacy reader (`QUIRKS.md` #19).
      - Verify extent timeline math (`MovieExtent.cpp`, `Extents.cpp`, `TimelineTrack.cpp`),
        all 23 effect templates and 7 theme structures (`Templates.cpp`, `Theme.cpp`,
        `ThemeTemplates.cpp`, `TextComposition.cpp`), and background `AutofitProcess` /
        `Conductor` / `BackgroundOrchestrator`.
- [ ] **`HMREngine/` (3D X3D/VRML scene graph & Direct3D 11 renderer):**
      - Verify X3D node/field graph (`X3DNodes.cpp`, `X3DGroupingImpls.cpp`,
        `X3DAppearanceImpls.cpp`, `X3DLayerImpls.cpp`, `X3DSensorImpls.cpp`,
        `InterpolatorImpl.cpp`), pattern meshes (`PatternMesh.cpp`, `PatternMeshFactory.cpp`),
        DirectWrite text pipeline (`TextRenderPipeline.cpp`), and DX11 resource cache
        (`EngineDX.cpp`, `RenderLoopDX.cpp`, `ResourceCacheDX.cpp`).
      - Preserve `HMREngine.h` SDK error-macro redefinition zone (`QUIRKS.md` #5), the 118
        dummy texture codec classes (`TextureCodecs.h`, `QUIRKS.md` #13), WARP fallback order
        (`QUIRKS.md` #20), silent HRESULT swallowing (`QUIRKS.md` #21), and the 512-depth /
        100k-node security caps (`StubDesign.md`).
- [ ] **`HMRAVSource/` (Media Foundation & audio/video pipeline — 13 stubs):**
      - Complete `TranscodeMetadata::ExtractThumbnail` and `ExtractMetadata` WIC/MF fallback
        paths (`TranscodeMetadata.cpp:172,205` — 2 MEDIUM stubs) and synchronize
        `analysis/E_NOTIMPL/inventory.md` with the `2026-10-08` `TextureInterOp` DX9/DX11
        shared-handle implementation.
      - Verify `MFSource`, `DShowSource`, `StreamSink`, `StreamSinkHost`, `VideoProc`,
        `SyncVideoSource`, `SAXProfileBuilder`, `EncodeProfile`, and the `Audio/` DSP chain
        (`AudioFadeProcessor`, `AudioDuckingProcessor`, `AudioChannelMapper`, `AudioResampler`,
        `AudioBoost`, `AudioAnalysis`, `Mixer`, `Waveform`, `AudioOutput`, `AudioCapture`).
      - Keep `AuthProvider.cpp` DRM/PlayReady/OAuth/Token stubs (`#28–#33`) and `dxva2stubs.cpp`
        (`#34–#35`) aligned with `StubDesign.md`.
- [ ] **`UI/` & `Ribbon/` (DirectUI behaviors, owner-drawn shell, 67 Ribbon commands):**
      - Complete the 50+ `OnMessage` handlers across 37 behavior classes in
        `UIBehaviorClasses.cpp`, `EditingBehaviors.cpp`, and `TimelineDataSources.cpp`, plus
        `RibbonApp.cpp` (`GetCommandLabelDescription`) and `RibbonSites.cpp`.
      - Wire `SundanceMainElementBehavior::LoadLayout` to the Phase-2 `.duxt` parser (or
        finalize the boundary with `SundanceMainVisual.{h,cpp}`).
      - Replace sequential placeholder CLSIDs in `ComFactory.h` (`QUIRKS.md` #4) with the
        extracted reference CLSIDs from `analysis/COMGuids/guids.md`.
- [ ] **`Preview/` & `Transport/`:**
      - Verify `PreviewPresenter.cpp`, `PreviewDataContext.cpp`, `TransportBase.cpp` (preserving
        the uninitialized `RenderTransport::Seek` null-deref quirk, `QUIRKS.md` #22), and
        `CommandBin.cpp`.
- [ ] **`Publishing/` (in-engine publish orchestration):**
      - Complete `PublishJob.cpp`, `PublishItem.cpp`, `PublishClasses.cpp`,
        `PublishDialogs.cpp` (preserving `PublishDialogSkyDriveBehavior`, `QUIRKS.md` #7), and
        `XmlManifest.cpp` so local and service publish jobs run end-to-end against
        `WLXMediaPublishSubscribe.dll`.

### 5.2 Supporting Tier-1A modules already in `src/`

Each checkbox requires: full export & ordinal parity + real behavioral implementation (or
documented parity stub per `StubDesign.md`) + contract coverage in `tests/mmr-python` + updated
module doc under `website/docs/modules/`.

- [ ] **`WLXPhotoBase.dll` (`src/WLXPhotoBase/`):** all 56 `Base` / `BasePrivate` exports
      (process-heap `Exception::operator new` + `EXCEPTION_NONCONTINUABLE_EXCEPTION` per
      `QUIRKS.md` #8, `?Delete@BasePrivate@@YAXPAX@Z`, `Win32ErrorToHresult`,
      `GdiplusStatusToHresult`, OS version checks, string/GDI+ helpers). Keep the 4 accurate
      `E_NOTIMPL` error-mapping entries (`inventory.md` #78–#81).
- [ ] **`WLXPipeline.dll` (`src/WLXPipeline/`):** `GetPipelineCreateFunctions` (7 pipeline
      factory functions, replacing `inventory.md` #91 stub) and `DllRegisterServer`.
- [ ] **`WLXPipetran.dll` (`src/WLXPipetran/`):** `GetTFXCreateFunctions` (`inventory.md` #90)
      and all 92 transition/effect RTTI classes (70 active pattern-mesh/shader transitions
      verified against golden frames + 22 intentional stub transitions per `QUIRKS.md` #18).
- [ ] **`WLXSlideshow.dll` & `WLXPhotoCinematic.dll`:** 4 COM exports each; slideshow timeline
      generator and Ken Burns (`CinematicFullScreen`, `PanZoomTransform`) D3D9 transforms.
- [ ] **`WLXMovieLibrary.dll` (`src/WLXMovieLibrary/`):** COM factory + implement the MF
      video-thumbnail fallback in `GetThumbnail` (`WLXMovieLibrary.cpp:177`, `inventory.md`
      #93 MEDIUM).
- [ ] **`WLXVideoTrim.dll` (`src/WLXVideoTrim/`):** implement or contract-pin the 5 DirectShow
      trimmer/transcoder factories (`CreateAVICopierDirect`, `CreateVideoCopierFromMediaType`,
      `CreateVideoFormatContextTranscoder`, `CreateVideoPlayer`, `CreateVideoWMVTranscoder`,
      `inventory.md` #73–#77) with custom `FACILITY_WLXVIDEOTRIM` / `AVS_E_*` HRESULTs.
- [ ] **`WLXMP4Parser.dll` (`src/WLXMP4Parser/`):** close the 4 MEDIUM stubs (`GetTrackInfo`,
      `AddMP4SourceFilter`, `BuildMP4FilterGraph`, `BuildMP4PlayBack`, `inventory.md` #82–#85)
      while keeping the 1,024-trak and 64-bit `stts` sample-count bounds (`StubDesign.md`).
- [ ] **`WLXFaceRecognition.dll` (`src/WLXFaceRecognition/`):** 7 COM classes (`CFaceDetector`,
      `CFaceRecognizor`, `CNeuralNet`, `CClassifier`, LBP/PCA/Texton extractors) with 640px
      downscale and sample bounds (`StubDesign.md`).
- [ ] **`WLMFDS.dll`, `WLMFReadWrite.dll`, `WLXTranscode.exe`, `WLXCodecHost.exe`:**
      - `WLMFReadWrite.dll`: implement `WriteFrame` (`WLMFReadWrite.cpp:226`, `inventory.md`
        #92 MEDIUM) wrapping buffers in `IMFMediaBuffer`/`IMFSample` across all 9 exports.
      - `WLMFDS.dll`: `CreateMFSourceFromDShowFilter` and EVR filter connection (`inventory.md`
        #88–#89).
      - `WLXTranscode.exe` & `WLXCodecHost.exe`: out-of-process CLI flags, IPC/shared-memory
        marshaling, timeout/crash isolation, and exit codes.
- [ ] **`MovieMakerPreviewClient.dll` (`src/MovieMakerPreviewClient/`):** close
      `HandleCommand(PreviewCmd_LoadFile)` (`inventory.md` #87 MEDIUM) and `PreviewCmd_Snapshot`
      (#86 LOW) across the 4 COM exports.
- [ ] **`MetadataSys.dll` (`src/MetadataSys/`):** implement `WLXPSGetItemPropertyHandler`
      (`MetadataSys.cpp:16`, `inventory.md` #94 MEDIUM) and property store readers/writers.
- [ ] **`MovieMakerLang.dll` (`src/MovieMakerLang/`):** resource-only satellite DLL (`/NOENTRY`,
      0 exports, 201 `RT_STRING` tables, icons, cursors, PNGs, `UIFILE` resources — Phase 6).
- [ ] **Inert-by-design & legacy shared surfaces (`DmxBici.dll`, `WLXPhotoSqm.dll`, `UXCore.dll`, `uxctl.dll`, `wlidcli.dll`):**
      - `DmxBici.dll` (19 `BiciWrapper` exports) & `WLXPhotoSqm.dll` (44 `Sqm::` exports):
        keep 100% export/ordinal parity, return `S_OK`/`TRUE`, and never open network sockets.
      - `UXCore.dll` & `uxctl.dll`: preserve the 19 `g_*_mark`-gated dead stubs (`inventory.md`
        #42–#60) and `__cdecl` resource helpers in `Resources.cpp`.
- [ ] **Resolve the 5 Tier-1B scaffolding targets (`MediaCatalog`, `PlaybackEngine`, `ProjectManager`, `TimelineEngine`, `GPURenderer`):**
      fold their functionality into `MovieMakerCore.dll` (or keep them strictly as internal
      static libraries not exposed as standalone DLL dependencies) so the runtime DLL graph is
      1:1 with the reference install.

### 5.3 Client & online service stack (`WLXMediaPublishSubscribe`, `wlidcli`, `.NET` plugins, protocol handlers)

Because the original application was tightly integrated with **Windows Live Essentials 2012
services**, a faithful 1:1 clean-room reconstruction covers both the client-side service
plumbing and a local loopback service-emulation harness:

- [ ] **`WLXMediaPublishSubscribe.dll` (`src/WLXMediaPublishSubscribe/` — 22 exports, 8 COM classes, 12 stubs):**
      implement the provider manager (`CLSID_OnlineMediaPluginManager`,
      `CLSID_MediaPublishSubscribeProviderManager`, `CLSID_FlickrProvider`,
      `CLSID_LiveProvider`, `CLSID_DatabasePublishProperties`,
      `CLSID_MetadataSettingsController`) and the 12 publish/upload/status/callback/token
      functions (`inventory.md` #61–#72) against configurable local/mock endpoints while
      enforcing DPAPI-only credential storage (`PSAAuthenticationStore`, 64 MB cap).
- [ ] **`wlidcli.dll` (`src/wlidcli/` — Windows Live ID client):** reproduce the exported
      identity/ticket acquisition state machine against an offline/loopback token provider so
      the Ribbon "Sign in" button and publish auth handshakes work end-to-end without contacting
      retired Microsoft Passport servers.
- [ ] **Managed `.NET 4.0` publish/subscribe plugins & interop (`analysis/PublishPlugins/`, `analysis/PublishPluginsInterop/`):**
      reconstruct (or formally scope in `DEVIATIONS.md`) the 4 managed plugin assemblies and
      their interop bridges:
      - `PublishPluginsInterop.dll` & `SubscribePluginsInterop.dll` (COM↔CLR interop types)
      - `WLFacebookPlugin.dll` (`analysis/WLFacebookPlugin/`)
      - `WLFlickrPlugin.dll` (`analysis/WLFlickrPlugin/`, OAuth 1.0 signing + REST XML)
      - `WLVimeoPlugin.dll` (`analysis/WLVimeoPlugin/`, OAuth 1.0 + chunked upload)
      - `WLYouTubePlugin.dll` (`analysis/PublishPlugins/`, ClientLogin / GData upload)
      Ensure no consumer keys or secrets are hardcoded in plaintext (`StubDesign.md`).
- [ ] **URI protocol & mail/album service helpers (`AlbumDownloadProtocolHandler.dll`, `WLXAlbumDownloadWizard.exe`, `WLXSendMail.dll`):**
      reconstruct `CLSID_AlbumDownloadLauncher` (`IInternetProtocol` / `IInternetProtocolInfo`
      for `wlpg://` URLs), the album download wizard EXE, and the MAPI / Windows Live Mail
      attachment helper (`WLXSendMail.dll`).

### 5.4 Tier-2 Photo Gallery / shell / media satellites (analyzed in `analysis/`, Tracker A)

For each Tier-2 module required for suite-level parity: author `spec.md` → add `src/<Module>/`
CMake target + `.def` → migrate `tests/<Module>/` probe to `tests/mmr-python` → add module doc:

- [ ] **Acquisition & camera AutoPlay:** `WLXPhotoAcq.dll` (WIA/WPD/FS/DV acquisition, 6
      CLSIDs + TypeLib), `WLXVideoAcquireWizard.exe`, `WLXVideoCameraAutoPlayManager.exe`.
- [ ] **Media & background helpers:** `WLXGrinderScheduler.dll` ("Grinder" background job
      scheduler, directly imported by `MovieMakerCore.dll` in `CROSS_BINARY_MASTER.md`),
      `WLXCodecHostPS.dll` (COM proxy/stub DLL for `WLXCodecHost.exe`),
      `WLXImageTranscode.dll`, `WLXDSPA.dll` (shot detection / TOC DMOs, 93 RTTI classes),
      `WLXVAFilt.dll` (DirectShow filter), `WLXQuickTime.dll`, `wlxclip.dll` (6 exports, OLE
      clipboard).
- [ ] **Photo Gallery suite & shell extensions:** `WLXPhotoGallery.exe`,
      `WLXPhotoGalleryRepair.exe`, `WLXPhotoLibraryMain.dll`, `WLXPhotoLibraryDatabase.dll`
      (ESENT database), `WLXPhotoLibraryDuiResources.dll`, `WLXPhotoViewer.dll`,
      `WLXPhotoClassic.dll`, `WLXPhotoVoyager.dll` (8 Voyager theme/transform CLSIDs),
      `PhotoViewerShim.dll` (+ `PhotoViewerShimx64.dll`), `NPWLPG.dll` (NPAPI browser plugin),
      `wlsoxe.dll`.

**Exit gate:** all 4 HIGH and 19 MEDIUM stubs in `analysis/E_NOTIMPL/inventory.md` resolved and
pinned by contracts; `MovieMakerCore` controllers always initialized; client/service plugin
stack operational against local mock provider; `python3 tools/todo_audit.py` stays at `0`.

---

## Phase 6 — Resources, localization & UI assets

**Goal:** close the `.rsrc` parity gap (`MovieMaker.exe` `97,632 B` vs `128 B` rebuilt;
`MovieMakerCore.dll` `4,240,504 B` vs `480 B` rebuilt — Tracker C #2) using clean-room
substituted assets with 1:1 resource types, IDs, dimensions, and language tables.

- [ ] **Complete the resource inventory per target (`analysis/<Module>/resource_analysis.md`):**
      use `tools/resource_extractor.cpp` and `tools/analysis/extract_strings_resources.py` to
      catalog every resource entry (`RT_ICON`, `RT_GROUP_ICON`, `RT_CURSOR`, `RT_BITMAP`,
      `RT_DIALOG`, `RT_MENU`, `RT_ACCELERATOR`, `RT_STRING`, `RT_RCDATA` / `DUXT` / `UIFILE`,
      `RT_VERSION`, `RT_MANIFEST`, XML templates) across all 24 Tier-1A binaries.
- [ ] **Wire `.rc` resource scripts into all 24 Tier-1A CMake targets:**
      currently only `src/MovieMaker/moviemaker.rc` and `src/MovieMakerLang/MovieMakerLang.rc`
      exist. Add `<Module>.rc` + `resource.h` to `src/MovieMakerCore/` and the remaining 21
      Tier-1A targets, stamping `16.4.3528.0331` from `src/common.h` (`WMMR_VERSION_*` macros)
      in every `VS_VERSION_INFO` block.
- [ ] **Populate clean-room replacement payloads in `resources/extracted/` (currently empty):**
      - *String tables (`RT_STRING`):* author clean-room English (`0x0409`) strings matching
        all 201 `RT_STRING` tables and string IDs in `MovieMakerLang.dll` and
        `MovieMakerCore.dll`.
      - *Icons, cursors, bitmaps, PNGs:* author clean-room vector/raster replacements with
        identical resource IDs, pixel dimensions, bit depths, and alpha channels; log every
        substituted asset in root `NOTICE`.
      - *Dialogs, menus & accelerators:* reconstruct dialog templates (control IDs, styles,
        dialog units) and accelerator tables (`ID_APP_IMPORT=0xE101` .. `ID_APP_EXPORT=0xE203`
        non-contiguous IDs per `QUIRKS.md` #10).
- [ ] **DUI (`DUXT` / `UIFILE`) and Windows Ribbon markup resources:**
      author clean-room layout payloads and Ribbon command definitions (67 commands, contextual
      `Video Tools > Edit` and `Text Tools > Format` tab groups) whose geometry matches the
      pixel-verified reference specification in `website/replica/moviemaker.html`.
- [ ] **Application & assembly manifests (`RT_MANIFEST`):**
      ensure `src/MovieMaker/resources/MovieMaker.exe.manifest` is embedded as resource ID 2
      (matching `analysis/MovieMakerExe/deep_analysis.md`: `Microsoft.Windows.personalMedia.MovieMaker`
      v5.1.0.0, `processorArchitecture="x86"`, `<dpiAware>true</dpiAware>`, Common Controls
      6.0.0.0, `level="asInvoker"`, and the intentional absence of `<supportedOS>` GUIDs).
- [ ] **Localization & MUI satellite loading:**
      verify `UXCore` `RMFindModuleForResource` / `RMLoadString` / `RMLoadStringBSTR` /
      `RMLoadImage` / `RMLoadMenu` resolution against `MovieMakerLang.dll` across fallback
      LCIDs.
- [ ] **Add automated resource-table contracts in `tests/mmr-python`:**
      enumerate `EnumResourceTypesW` / `EnumResourceNamesW` + `FindResourceW` / `LoadStringW` /
      `GetFileVersionInfoW` on every built DLL/EXE and assert 100% ID/type/version parity.

**Exit gate:** all 24 Tier-1A targets embed `.rsrc` sections with 100% resource-type/ID/version
parity; `MovieMakerLang.dll` and `MovieMakerCore.dll` resource contracts pass; all asset
substitutions cataloged in `NOTICE`.

---

## Phase 7 — Real media & service pipeline (decode → render → encode → publish)

**Goal:** replace the test-frame export fallback (`ROADMAP.md` → *What's Still Stub/Placeholder*)
with a real Media Foundation / Direct3D 11 decode → composite → encode → publish pipeline that
produces playable, golden-comparable video files and completes local-loopback service uploads.

- [ ] **Real Media Foundation sink-writer & transcode encode path:**
      wire `ExportController` → `PublishJob` → `TranscodeManager` / `StreamSinkHost` /
      `WLMFReadWrite::WriteFrame` / `WLXTranscode.exe` so saving a movie encodes real video
      (H.264 / WMV9) and audio (AAC / WMA) streams from the composed timeline rather than
      synthetic test frames.
- [ ] **Encode profile parity (`EncodeProfile.cpp`, `SAXProfileBuilder.cpp`):**
      reproduce every built-in output profile (HD 1080p, HD 720p, Widescreen 480p, Standard
      480p, device/mobile presets, audio-only WMA/MP4) with exact container attributes, frame
      rates, bitrates, pixel aspect ratios, and stream metadata matching the Phase-3 golden
      exports within declared PSNR/SSIM and duration tolerances.
- [ ] **Decode & media-import matrix:**
      verify `NativeMFSourceReaderBuilder`, `DShowMFSourceReaderBuilder`, `WLXMP4Parser`,
      `WLMFDS`, and WIC image loaders across all reference input formats (WMV, ASF, AVI, MP4,
      MOV, M4V, 3GP, MPG/MPEG-2, WMA, MP3, WAV, AAC, M4A, JPEG, PNG, BMP, GIF, TIFF, ICO) and
      assert identical error `HRESULT`s on corrupt or unsupported files.
- [ ] **Render-graph & compositor parity (`HMREngine` + `HMRAVSource` + `WLXPipetran` + `WLXPhotoCinematic`):**
      verify D3D11/D3D9 shared-texture interop (`TextureInterOp`), DXVA2 video processing,
      frame-accurate transition blending across all 70 active `WLXPipetran` transitions, visual
      effects, Ken Burns pan/zoom trajectories, aspect-ratio letterboxing (4:3 vs 16:9),
      DirectWrite text/title/credit animation, and audio fade-in/fade-out/ducking/mixing curves.
- [ ] **Out-of-process worker isolation (`WLXCodecHost.exe` & `WLXTranscode.exe`):**
      verify process spawn, COM/RPC marshaling (`WLXCodecHostPS.dll`), hang timeouts, crash
      recovery when a 3rd-party codec faults, and clean child-process termination.
- [ ] **End-to-end local loopback service publish & subscribe:**
      build a lightweight local loopback HTTP/HTTPS mock server (`tools/mock_live_services.py`)
      emulating Windows Live ID RPS auth, SkyDrive/OneDrive, Facebook, Flickr, Vimeo, and
      YouTube upload endpoints; verify the full `PublishToService` UI → `wlidcli` auth →
      `WLXMediaPublishSubscribe` →plugin upload → progress/cancel/completion callback flow with
      DPAPI-encrypted token persistence (`%LOCALAPPDATA%\Microsoft\WL\MovieMaker\auth.dat`).

**Exit gate:** scripted "import video + photo + audio → apply transition + title + theme →
save `.wlmp` → export MP4/WMV → publish to loopback service" produces a playable video matching
the golden corpus metadata and PSNR/SSIM thresholds; zero test-frame fallbacks on the main path.

---

## Phase 8 — Contract & regression hardening

**Goal:** expand automated verification so every binary surface, workflow, parser bound,
security invariant, and preserved quirk is continuously enforced in CI.

- [ ] **Expand `tests/mmr-python` past 116 checks to full Tier-1A surface coverage:**
      add per-export contract checks for all 24 Tier-1A modules (every export in `WLXPhotoBase`,
      `WLXMediaPublishSubscribe`, `WLMFReadWrite`, `WLXMP4Parser`, `WLXVideoTrim`, `UXCore`,
      `uxctl`, `wlidcli`, `DmxBici`, `WLXPhotoSqm`, `MetadataSys`, etc.), plus cross-DLL
      workflow tests in `tests/mmr-python/wmmr/workflows.py` covering all 11 Phase-3 scenarios.
- [ ] **Fix the `WMMR_DLL_DIR` default footgun (`tests/mmr-python/run_tests.py`):**
      `run_tests.py` currently defaults to `build_clean\bin\Debug` (the `E_NOTIMPL` parity-stub
      directory). Change the default or require explicit `--dll-dir` / `WMMR_DLL_DIR` and fail
      loudly in CI if pointed at `build_clean`.
- [ ] **Decouple the 42 per-module test directories (`tests/*/`, `tests/OtherDlls/`) from `undecomp\` and wire them into CTest:**
      currently root `CMakeLists.txt` defines only 2 trivial `compare_files` CTest entries, and
      the 42 test directories contain `127` hardcoded references to `undecomp\Photo Gallery` /
      `undecomp\Shared`. Refactor them to resolve the target directory from `WMMR_DLL_DIR`
      (defaulting to `$<TARGET_FILE_DIR:MovieMaker>`) and register all Tier-1A C++ test
      executables in `CMakeLists.txt` via `add_test()`.
- [ ] **Run `tests/mmr-gui` (`--selfcheck` and `--selftest <binDir>`, 128 checks / 21 DLLs) and `tests/mmr-cli` in CI:**
      add an x86 .NET test step in `.github/workflows/build-projects.yml` so the managed
      harnesses stay in lockstep with `tests/mmr-python`.
- [ ] **Fuzz & untrusted-input bounds regression suite:**
      add hostile/malformed input tests for every file parser to verify the caps in
      `StubDesign.md`:
      - `WLXMP4Parser`: truncated atoms, cyclic box sizes, `>1024` traks, 64-bit `stts` overflow.
      - `HMREngine::X3DReader` & scene graph traversal: nesting depth `>512`, node count `>100,000`.
      - `.wlmp` `SerializationReader` & `XmlManifest`: malformed XML, billion-laughs entities,
        deep element trees, missing placeholder bindings.
      - `SundanceMainElementBehavior::LoadLayout`: truncated/oversized `DUXT` RCDATA blobs.
      - `WLXFaceRecognition`: `>640px` oversize bitmaps and zero-dimension images.
      - `PSAAuthenticationStore`: corrupted DPAPI blobs, bad magic/version header, `>64 MB` file.
- [ ] **Dedicated Quirk Regression Suite (23 tests for the 23 entries in `QUIRKS.md`):**
      write one explicit test per documented quirk (VEH+SEH `0xe06d7363` subcodes
      `0x19930520..22` and `0x01994000`, `Global\WindowsLiveMovieMaker_Sundance_SingleInstance`
      mutex blocking second instance, null `CLSID_MovieMakerCore`, `HMRError` macro zone,
      `Base::Exception` process-heap allocation + `EXCEPTION_NONCONTINUABLE_EXCEPTION`,
      `D3D11_CREATE_DEVICE_SINGLETHREADED`, non-contiguous command IDs, missing delay-loaded
      DLL survival, 118 dummy texture codecs, `IntSet`/`StringSet` semantics, 22 stub
      transitions, dead legacy v1 project classes, WARP fallback, etc.).
- [ ] **Security & privacy invariant tests:**
      assert zero outbound network sockets during startup/editing/telemetry calls (`DmxBici`,
      `WLXPhotoSqm`) and zero plaintext credential strings written under `%LOCALAPPDATA%` or
      the registry.

**Exit gate:** CI runs on every push/PR and gates on: all 29 targets building (Debug + Release),
`diff_exports.py --manifest`, `mmr-python` contract suite, expanded CTest suite, `mmr-gui --selftest`,
fuzz/bounds suite, 23/23 quirk checks, and `python3 tools/todo_audit.py`.

---

## Phase 9 — 1:1 verification & diff closure

**Goal:** prove or formally disposition every remaining binary, resource, file-format, visual,
and behavioral difference between the reference install and the clean-room reconstruction.

- [ ] **Re-run the full PE binary diff and refresh `analysis/BinaryDiff/diff_analysis.md`:**
      extend the diff from `MovieMaker.exe` + `MovieMakerCore.dll` to all 24 Tier-1A targets,
      and walk all 12 numbered gaps in Tracker C to **closed** or **accepted deviation**.
- [ ] **Publish `analysis/PARITY_SCORECARD.md` (using the Tracker D schema):**
      score every Tier-1A module across 8 dimensions: Export parity, Import/delay-load parity,
      COM/registry parity, Resource (`.rsrc`) parity, Section/PE-header parity, Behavioral
      contract coverage, Visual parity, and Quirk reproduction — with links to evidence.
- [ ] **Automated visual parity diff against `website/replica/moviemaker.html` and reference captures:**
      render the reconstructed UI shell (`SundanceMainVisual` / DirectUI + Ribbon tabs,
      contextual bands, storyboard filmstrip, preview transport, dialogs, status bar zoom
      slider) across standard window sizes and DPI settings, and assert pixel-diff thresholds.
- [ ] **Byte-level `.wlmp` & media output diff:**
      verify that opening and saving each golden `.wlmp` project produces byte-identical XML
      (after normalizing non-deterministic timestamps/GUIDs if any), and that exported media
      streams match the reference container box layout and codec parameters.
- [ ] **Side-by-side behavioral trace diff:**
      replay the 11 Phase-3 scenario scripts in both the reference VM and the reconstructed
      build, diffing API/COM call traces, returned `HRESULT` sequences, registry writes, and
      created files.
- [ ] **Publish root `DEVIATIONS.md` (Accepted-Deviation Register):**
      formally document every intentional or toolchain-mandated difference from the 2012
      binaries:
      1. MSVC 19.44 (VS2022) / UCRT (`MSVCP140`/`VCRUNTIME140`) vs MSVC 11.0 (`MSVCR110.dll`)
         and PE linker metadata (or document the optional `v110` toolset lane).
      2. Windows 10 SDK `10.0.26100.0` compatibility shims (`d3dx11compat`, `dxva2stubs`,
         removed Windows Ribbon SDK definitions — `ROADMAP.md`).
      3. Clean-room substituted icons, bitmaps, strings, and UI markup (`NOTICE`).
      4. Inert telemetry (`WLXPhotoSqm`, `DmxBici`) and DPAPI-hardened credential storage
         (`StubDesign.md`).
      5. Retired online services (Live ID, SkyDrive, Facebook/Flickr/Vimeo/YouTube) routed to
         offline/loopback mock providers.
      6. Untrusted-input parser bounds (`WLXMP4Parser`, `X3DReader`, scene traversal,
         `WLXFaceRecognition`, `PSAAuthenticationStore`) added for memory safety.
      7. Disposition of Tier-2 Photo Gallery satellites and Tier-3 3rd-party/shared binaries.
- [ ] **Publish the formal 1:1 Clean-Room Claim Statement in `README.md` and `PARITY_SCORECARD.md`:**
      state concisely what is byte/surface-identical (exports, ordinals, decorations, COM
      CLSIDs/IIDs, `.wlmp` format, resource IDs), what is behaviorally equivalent (C++14
      re-implementation pinned by contracts + preserved quirks), and what is intentionally
      substituted (`DEVIATIONS.md`).

**Exit gate:** `analysis/PARITY_SCORECARD.md` and `DEVIATIONS.md` committed; all 12 Tracker C
gaps closed or registered; visual, `.wlmp`, and behavioral trace diffs green.

---

## Phase 10 — Installation, registration & deployment parity

**Goal:** installing, registering, launching, associating files/protocols, and uninstalling the
reconstructed suite works on a clean Windows VM just like the 2012 Live Essentials setup.

- [ ] **Reproduce the reference install layout & registration scripts (`analysis/RegRes/analysis.md`):**
      - 32-bit layout under `%ProgramFiles(x86)%\Windows Live\Photo Gallery\` and
        `%ProgramFiles(x86)%\Windows Live\Shared\` (plus language satellite subfolders).
      - `DllRegisterServer` / `DllUnregisterServer` and `.rgs`-equivalent registry trees for
        all COM servers (`MovieMakerPreviewClient`, `WLXSlideshow`, `WLXPhotoCinematic`,
        `WLXMovieLibrary`, `WLXMP4Parser`, `WLXFaceRecognition`, `WLMFDS`,
        `WLXMediaPublishSubscribe`, `MetadataSys`, `AlbumDownloadProtocolHandler`).
      - `.wlmp` file association, shell verb handlers, `wlpg://` pluggable protocol handler,
        and camera AutoPlay registration (`WLXVideoCameraAutoPlayManager.exe`).
- [ ] **Build a clean-room installer (WiX MSI or Inno Setup):**
      package only reconstructed binaries and clean-room assets from `build/bin/Release`;
      support silent install, repair, and clean uninstall that removes all registered keys and
      files without leaving orphans.
- [ ] **Verify UAC, elevation, and side-by-side coexistence:**
      confirm `MovieMaker.exe` runs `asInvoker` without elevation, handles non-admin `HKLM`
      registration probes gracefully (`StubDesign.md`), and behaves predictably when a genuine
      2012 Windows Live Essentials install is present on the same machine (shared mutex
      `Global\WindowsLiveMovieMaker_Sundance_SingleInstance`, COM CLSIDs).
- [ ] **Verify all 10 consumer/exercise apps (`apps/*` submodules) against the installed layout:**
      run `moviemaker-launcher`, `photoviewer`, `transition-plugin`, `mediapublisher`,
      `video-trimmer`, `metadata-editor`, `slideshow-studio`, `face-tagger`, `movielibrary`,
      and `pipeline-graph` against the installed directory.

**Exit gate:** automated clean-VM test passes `install → regsvr32/COM check → launch MovieMaker.exe → open/export .wlmp → run 10 exercise apps → uninstall with zero residue`.

---

## Phase 11 — Documentation & clean-room deliverables

**Goal:** enable any third-party engineer or reviewer to audit the 1:1 clean-room
reconstruction from binary evidence → specification → C++14 source → contract → parity proof.

- [ ] **Synchronize the Docusaurus site (`website/docs/`) and Jekyll landing site (`index.html`, `_includes/footer.html`):**
      - Ensure every built target has an up-to-date reference page under `website/docs/modules/`.
      - Add Docusaurus pages for this Phased TODO Plan, the Parity Scorecard
        (`analysis/PARITY_SCORECARD.md`), and the Accepted-Deviation Register (`DEVIATIONS.md`).
      - Verify `.github/workflows/docs.yml` builds both Jekyll and Docusaurus cleanly with zero
        broken links.
- [ ] **Reconcile all numbers across root documentation (`README.md`, `ROADMAP.md`, `AGENTS.md`, `TODO_AUDIT.md`, `THINKING_PROCESS.md`):**
      update `ROADMAP.md`'s stale `18 targets` / `July 22, 2026` figures to match the canonical
      inventory (`29` CMake module directories, `492` authored source files, `90` inventoried
      `E_NOTIMPL` stubs) so no two root documents disagree.
- [ ] **Audit "recreation notes" comments across `src/`:**
      verify that every non-obvious algorithm, HRESULT quirk, vtable layout, and SDK shim in
      `src/` carries a concise recreation note citing its `analysis/<Module>/spec.md` section
      and `QUIRKS.md` entry.
- [ ] **Publish the per-module Clean-Room Audit Trail (`analysis/AUDIT_TRAIL.md`):**
      table mapping each reconstructed module to: (1) reference PE SHA-256 (`PROVENANCE.md`),
      (2) specification document (`analysis/<Module>/spec.md`), (3) reconstructed source path
      (`src/<Module>/`), (4) contract file (`tests/mmr-python/wmmr/contracts/*.py`), and
      (5) parity scorecard row (`analysis/PARITY_SCORECARD.md`).

**Exit gate:** `analysis/AUDIT_TRAIL.md` published; Jekyll + Docusaurus docs build with 0
warnings/broken links; zero contradictory counts across root markdown files.

---

## Phase 12 — Remix / modernization (**explicitly out of scope for 1:1**)

Tracked here solely to keep modernization and bug-fix ideas from contaminating the 1:1
clean-room baseline. Any item below requires an explicit user request, a dedicated remix
branch, and its own design doc:

- [ ] Fix the 23 preserved original bugs in `QUIRKS.md` (starting with critical items #1, #5,
      #8, #21, #22, #23).
- [ ] Native x64 / ARM64 port and modern C++20/23 refactoring.
- [ ] Modern OAuth 2.0 / REST publish plugins for current YouTube, OneDrive, Vimeo, or local
      FFmpeg/AV1/HEVC export presets.
- [ ] Public C/COM add-in SDK replacing the inert `MovieMakerCore::LoadAddIns` host.
- [ ] Per-Monitor DPI v2 awareness, dark theme, and hardware-accelerated multithreaded D3D11/12
      rendering (removing `D3D11_CREATE_DEVICE_SINGLETHREADED`).

---

## Cross-phase trackers

### Tracker A — Analyzed modules with no `src/` target yet

Regenerate the raw directory diff with:

```bash
for d in analysis/*/; do n=$(basename "$d"); [ -d "src/$n" ] || echo "$n"; done
```

Non-module evidence directories (`ArchitectureSynthesis`, `BinaryDiff`, `COMGuids`,
`CrossDllImports`, `E_NOTIMPL`, `PublishPlugins`, `RegRes`, `Shared`, `ThreadModel`) are
cross-cutting analysis artifacts, not standalone binaries. Note the existing directory mappings:
`analysis/MovieMakerExe` → `src/MovieMaker`; `analysis/Shared/uxcore.md` → `src/UXCore`;
`analysis/Shared/uxctl.md` → `src/uxctl`; `analysis/Shared/wlbici.md` → `src/DmxBici`;
`src/wlidcli` corresponds to the Live ID client (`analysis/Shared/wlidux.md`). All remaining
analyzed binaries are tracked below:

| Group | Analyzed Modules (`analysis/<Module>/`) | Scope Tier | Owning Phase |
|---|---|---|---|
| **Photo Gallery & shell** | `WLXPhotoAcq`, `WLXPhotoClassic`, `WLXPhotoViewer`, `WLXPhotoVoyager`, `WLXPhotoGallery` (EXE), `WLXPhotoGalleryRepair` (EXE), `WLXPhotoLibraryMain`, `WLXPhotoLibraryDatabase`, `WLXPhotoLibraryDuiResources`, `PhotoViewerShim`, `PhotoViewerShimx64`, `NPWLPG`, `wlsoxe`, `wlxclip` | Tier 2 | Phase 5.4 |
| **Acquisition & media helpers** | `WLXVideoAcquireWizard` (EXE), `WLXVideoCameraAutoPlayManager` (EXE), `WLXImageTranscode`, `WLXDSPA`, `WLXQuickTime`, `WLXVAFilt`, `WLXSendMail` | Tier 2 | Phase 5.3 / 5.4 |
| **Scheduling, IPC & protocols** | `WLXGrinderScheduler`, `WLXCodecHostPS` (proxy/stub), `AlbumDownloadProtocolHandler` (`wlpg://`), `WLXAlbumDownloadWizard` (EXE) | Tier 2 | Phase 5.3 / 5.4 |
| **Publish/subscribe plugins (.NET 4.0)** | `WLFacebookPlugin`, `WLFlickrPlugin`, `WLVimeoPlugin`, `WLYouTubePlugin` (in `analysis/PublishPlugins/`), `PublishPluginsInterop` (+ `SubscribePluginsInterop`) | Tier 2 (service) | Phase 5.3 / 7 |
| **Shared / 3rd-party infrastructure** | `Imaging` (CaptureOne RAW), `SharedMFDlls` (`RSCMFT.dll`, `MPG4DEMUX.dll`), `D3DCOMPILER_46`, `Shared/WLAVRes`, `Shared/sqmapi`, `Shared/uxcalendar`, `Shared/uxcontacts`, `Shared/uxctlloc`, `Shared/wldcore`, `Shared/wldlog`, `Shared/wlidux` | Tier 3 (surface only) | Phase 1 / 4 |

### Tracker B — Stub & annotation burn-down

| Source | Current Count | Breakdown / Policy | Where Tracked |
|---|---|---|---|
| `E_NOTIMPL` inventory | **90** across 13 DLLs | `0` CRITICAL · `4` HIGH · `19` MEDIUM · `67` LOW (incl. `19` dead `UXCore` stubs + `4` valid `WLXPhotoBase` error mappings) | `analysis/E_NOTIMPL/inventory.md` |
| Open `TODO(reconstruction)` source annotations | **0** across 608 audited files | Resolved 14 annotations in `2026-10-08` pass; scanned by `python3 tools/todo_audit.py` | `TODO_AUDIT.md` |
| Documented-parity stubs & preserved quirks | **23** quirks + telemetry/DRM/dead-code stubs | Must **not** be "fixed" or deleted on the 1:1 baseline | `StubDesign.md`, `QUIRKS.md` |

**Burn-down order:**
1. Reconcile `analysis/E_NOTIMPL/inventory.md` against the `2026-10-08` pass (`MediaBrowser::CreateDataObject`, `DynamicDataObjectWrapper::GetDataHere`/`EnumFormatEtc`, `TextureInterOp::CreateTextureInternal`/`CreateSurfaceInternal`, `TranscodeMetadata::ExtractThumbnail`).
2. Fix `SundanceAppMain::Initialize()` controller construction (`ImportFiles` HIGH + 12 MEDIUM null-controller guards in `SundanceAppMain.cpp`).
3. Close the remaining MEDIUM feature gaps: `WLXMP4Parser` (`GetTrackInfo`, `AddMP4SourceFilter`, `BuildMP4FilterGraph`, `BuildMP4PlayBack`), `WLMFReadWrite::WriteFrame`, `WLXMovieLibrary::GetThumbnail`, `MovieMakerPreviewClient` (`PreviewCmd_LoadFile`), and `MetadataSys::WLXPSGetItemPropertyHandler`.
4. Implement or contract-pin export-observable LOWs (`WLXVideoTrim` 5 factories, `WLXPipeline::GetPipelineCreateFunctions`, `WLXPipetran::GetTFXCreateFunctions`, `WLXMediaPublishSubscribe` 12 functions).
5. Leave the 19 `UXCore` dead-code stubs, 4 `WLXPhotoBase` error-code mappings, `d3dx11compat`, and `dxva2stubs` intact as documented parity.

### Tracker C — Binary-surface gaps (from `analysis/BinaryDiff/diff_analysis.md`)

| # | Gap | Severity | Root Cause in Current Tree & Resolution | Status |
|---|---|---|---|---|
| 1 | 18 missing import DLLs in `MovieMakerCore.dll` | Critical | `src/MovieMakerCore/CMakeLists.txt` passes `/DELAYLOAD` for `WLXPhotoSqm`, `DmxBici`, `MetadataSys`, `wlidcli`, `UXCore`, `uxctl` without linking their import libs (`LNK4199`), and omits symbols from `d3d9`, `D3DCOMPILER_46`, `dwmapi`, `dxva2`, `ESENT`, `OLEACC`, `PROPSYS`, `UxTheme`, `VERSION`, `WindowsCodecs`, `WINMM` | `[ ]` Phase 4 |
| 2 | ~4 MB `.rsrc` missing in `MovieMakerCore.dll` & ~97 KB in `MovieMaker.exe` | Critical | Only 2 targets have `.rc` files (`moviemaker.rc`, `MovieMakerLang.rc`); `resources/extracted/` is empty | `[ ]` Phase 6 |
| 3 | Over-exporting in `MovieMakerCore.dll` (4,224 symbols vs 1) | Critical | `MovieMakerCore.def` exists, but `MOVIECORE_API` / `STORYBOARD_API` / `RIBBON_API` / `DATASTRUCT_API` / `AVSOURCE_API` expand to `__declspec(dllexport)` and `link.exe` union-merges them | `[~]` Phase 4 |
| 4 | Export module name `MOVIECORE.dll` vs `MovieMakerCore.dll` | Critical | Verify `LIBRARY MovieMakerCore` in `src/MovieMakerCore/MovieMakerCore.def` and check with `diff_exports.py` | `[~]` Phase 4 |
| 5 | Static/manual `LoadLibraryExW` vs delay-loaded `MovieMaker.exe` → `MovieMakerCore.dll` | Significant | `src/MovieMaker/CMakeLists.txt` checks `if(EXISTS "${CMAKE_BINARY_DIR}/lib/MovieMakerCore.lib")` at configure time (false on clean configure) and `main.cpp` uses manual `LoadLibraryExW`/`GetProcAddress` | `[ ]` Phase 4 |
| 6 | `WLXPhotoBase.dll` imported symbol mismatch in `MovieMaker.exe` | Significant | `src/MovieMaker/main.cpp` imports `_WLXPhotoBase_Init@0` instead of `?Delete@BasePrivate@@YAXPAX@Z`; `WLXPhotoBase.def` is not wired in `src/WLXPhotoBase/CMakeLists.txt` | `[ ]` Phase 4 |
| 7 | CRT mismatch `MSVCR110.dll` (VS2012) vs `MSVCP140`/`VCRUNTIME140`/UCRT (VS2022) | Significant | Toolchain decision record: add optional `v110` build lane or register in `DEVIATIONS.md` | `[ ]` Phase 4 / 9 |
| 8 | Missing individual GDI+ (8), ole32 (7), and GDI32 (11) imports in `MovieMakerCore.dll` | Significant | Closes naturally as Phase 5 UI/clipboard/OLE/GDI+ code paths replace simplified Win32 calls | `[ ]` Phase 4 / 5 |
| 9 | Extra PE sections (`.rdata`, `.fptable`) vs 4-section reference layout | Cosmetic | Evaluate linker `/MERGE:.rdata=.text` or register in `DEVIATIONS.md` | `[ ]` Phase 4 / 9 |
| 10 | Linker version `11.00` vs `14.44` | Cosmetic | Inherent to VS2022 linker; register in `DEVIATIONS.md` | `[ ]` Phase 9 |
| 11 | Control Flow Guard (`/guard:cf`) enabled in rebuild, absent in original | Cosmetic | Root `CMakeLists.txt` lines 97/105 add `/guard:cf` and `/GUARD:CF`; remove for 1:1 PE parity or register in `DEVIATIONS.md` | `[ ]` Phase 4 / 9 |
| 12 | PE header checksum `0` in rebuild vs `0xA23F3E` in original | Cosmetic | Add `/RELEASE` linker flag in `CMakeLists.txt` so `link.exe` computes the PE image checksum | `[ ]` Phase 4 |

### Tracker D — Parity Matrix Template (populate in `analysis/PARITY_SCORECARD.md` in Phase 9)

| Module | Tier | Exports (name/ord/dec) | Imports (static + delay) | COM / Registry | Resources (`.rsrc`) | Behavioral Contracts | Visual / Output Diff | Quirks Preserved | Registered Deviations |
|---|---|---|---|---|---|---|---|---|---|
| `MovieMaker.exe` | 1A | 0/0 | n/3 DLLs | manifest + `.wlmp` | n/m IDs | n/m pass | pass/fail | #1 | links |
| `MovieMakerCore.dll` | 1A | 1/1 (`MovieMakerMain @1`) | n/34 DLLs | n/54+ CLSIDs | n/m IDs | n/m pass | pass/fail | #2–#23 | links |
| `<Supporting DLL/EXE>` | 1A | n/m | n/m DLLs | n/m CLSIDs | n/m IDs | n/m pass | pass/fail/n-a | n/a | links |

---

## Maintenance rules

1. **Keep counts regenerable.** Every number in the *Status snapshot* table includes the shell
   command that computes it; re-run those commands whenever a phase gate is updated.
2. **Evidence before checkboxes.** A task or phase moves to `[x]` only in the same commit (or
   after the commit) that lands its verification evidence.
3. **Single source of sequencing.** This file sequences the work; `ROADMAP.md` records current
   build state; `TODO_AUDIT.md` indexes inline `TODO(reconstruction)` source comments;
   `StubDesign.md` defines when a stub is the final 1:1 behavior.
4. **Never commit copyrighted binaries, disassembly listings, or Microsoft assets** — see
   Phase 0.
