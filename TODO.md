# TODO.md — Phased Plan for a Faithful 1:1 Clean-Room Reconstruction

> **Target:** Microsoft Windows Live Movie Maker 2012 (codename **Sundance**), `16.4.3528.0331`,
> x86 PE32, MSVC 11.0 (VS2012), Windows 6.2+ — as shipped in Windows Live Essentials 2012.
> **Scope:** `MovieMaker.exe` + `MovieMakerCore.dll` + the supporting DLL/EXE set, plus the
> satellite modules that the install registered (Photo Gallery / shell / publish plugins).
> **Last reviewed:** 2026-10-14
> **Companion docs:** [`AGENTS.md`](AGENTS.md) (rules) · [`StubDesign.md`](StubDesign.md)
> (stub policy) · [`QUIRKS.md`](QUIRKS.md) (preserved bugs) · [`ROADMAP.md`](ROADMAP.md)
> (build state) · [`TODO_AUDIT.md`](TODO_AUDIT.md) (open source annotations) ·
> [`THINKING_PROCESS.md`](THINKING_PROCESS.md) (methodology narrative) ·
> [`analysis/`](analysis/) (binary evidence)

This file is the **work plan**: what still has to happen, in which order, for the
reconstruction to be defensibly 1:1 with the original application. It does not restate
build state (`ROADMAP.md`), stub policy (`StubDesign.md`), or the annotation index
(`TODO_AUDIT.md`) — it sequences them and defines the gates between them.

---

## How to use this file

- **Phases are gates, not walls.** Work inside a phase may overlap with the next one, but a
  phase is only `[x]` when its *Exit gate* is met with evidence committed to the repo.
- **Checkbox legend:** `[ ]` not started · `[~]` in progress · `[x]` done (evidence linked) ·
  `[!]` blocked (state the blocker inline).
- **Evidence rule.** Never tick a box on the strength of "it builds". A done task points at
  a contract, an analysis file, a scorecard row, or a CI run.
- **Spec-first rule.** No `src/` implementation starts before its behavior spec exists under
  `analysis/<Module>/spec.md` (Phase 2). Implementers work from specs + contracts, not from a
  disassembler window (Phase 0 firewall).
- **Any change to externally observable behavior lands with a contract** in `tests/mmr-python`
  (submodule commit + parent gitlink bump), per `AGENTS.md` → Definition of Done.

### Status snapshot (2026-10-14)

| Metric | Value | Source of truth |
|---|---|---|
| CMake targets building | 29 (`src/` module dirs: 30 incl. vendored `WTL/`) | `find src -name CMakeLists.txt` |
| Authored source files under `src/` (excl. `src/WTL/`) | ~455 | `find src -type f -not -path 'src/WTL/*'` |
| `MovieMakerCore/` files | ~350 | `find src/MovieMakerCore -type f` |
| Contract checks (ctypes suite) | 116 expected pass, 0 fail | `tests/mmr-python/run_tests.py` |
| CTest entries | 2 | `ctest --test-dir build` |
| GUI harness self-test | 128 checks / 21 DLLs | `tests/mmr-gui --selftest` |
| Open `TODO(reconstruction)` annotations | 0 (audit clean) | `python3 tools/todo_audit.py --check` |
| Remaining `E_NOTIMPL` stubs inventoried | 90 (0 CRITICAL / 4 HIGH / 19 MEDIUM / 67 LOW) | `analysis/E_NOTIMPL/inventory.md` |
| Documented original quirks preserved | 23 | `QUIRKS.md` |
| Analysis directories | 62 (+3 top-level synthesis docs) | `analysis/` |
| Modules analyzed with **no** `src/` counterpart | ~30 (see Tracker A) | table below |
| Submodules (tests + apps) | 13 | `.gitmodules` |

### Phase map

| Phase | Name | One-line goal | Exit gate | Status |
|---|---|---|---|---|
| 0 | Clean-room charter & provenance | Make the clean-room claim auditable and the repo legally safe | Provenance ledger + binary-leak guard in CI | `[~]` |
| 1 | Reference freeze & inventory | One authoritative module/binary inventory with scope tiers | `analysis/inventory` published, counts reconciled | `[~]` |
| 2 | Static specification extraction | Behavior specs an implementer can code from, per module | `analysis/<Module>/spec.md` for every Tier-1 module | `[~]` |
| 3 | Dynamic analysis & golden oracles | Observable truth: traces, golden files, scenario definitions | Golden corpus hashed + scenario→contract map | `[~]` |
| 4 | Build system & binary-surface parity | Exports, imports, linkage, sections, CI gates match reference | 29/29 export parity + CI gate on PR | `[~]` |
| 5 | Surface-by-surface re-implementation | Real behavior for every subsystem and supporting module | Per-module contracts green, stub inventory burned down | `[~]` |
| 6 | Resources, localization & UI assets | `.rsrc`, dialogs, strings, DUI/Ribbon markup, satellites | Resource ID/type parity (with substitution policy) | `[ ]` |
| 7 | Real media pipeline | Decode → render → encode → publish produces real output | Golden-comparable export from a full scenario | `[ ]` |
| 8 | Contract & regression hardening | Nothing can silently regress | CI green on PR, quirk + fuzz + workflow coverage | `[~]` |
| 9 | 1:1 verification & diff closure | Prove or formally accept every remaining difference | Parity scorecard + accepted-deviation register | `[ ]` |
| 10 | Installation, registration & deployment | Install/register/associate/uninstall like the original | Clean-VM install → launch → clean uninstall | `[ ]` |
| 11 | Documentation & clean-room deliverables | Third party can audit the whole claim | Docs site + audit trail per module | `[~]` |
| 12 | Remix / modernization (**out of scope**) | Kept separate so the 1:1 baseline stays clean | n/a — separate branch + doc | `[ ]` |

---

## Phase 0 — Clean-room charter, provenance & legal hygiene

**Goal:** make the clean-room claim *auditable*, and guarantee no Microsoft binary, listing,
or asset ever enters this repository.

- [ ] Record reference provenance in `analysis/PROVENANCE.md`: where the install came from
      (Live Essentials 2012 `wlsetup-all.exe`), product/version strings, extraction date, and
      **SHA-256 of every reference PE** — hashes and metadata only, never the files.
- [ ] Verify `undecomp/` stays gitignored and un-ignorable by accident; add a CI/pre-commit
      guard that rejects `undecomp/**`, `*.dll`, `*.exe`, `*.pdb`, and disassembler output
      (`*.asm`, `*.i64`, `*.gpr`, `*.ghidra`) from being staged.
- [ ] Write down the role split: **spec writers** read the binaries and produce
      `analysis/<Module>/spec.md`; **implementers** write `src/` from specs, contracts, and
      golden data only. Record who did which for each module (audit trail, Phase 11).
- [ ] Codify the transcription rule: no verbatim disassembly, decompiler output, or
      machine-code listing inside `src/` or its comments. Describe behavior in prose or
      neutral pseudocode under `analysis/`, and cite it as "recreation notes" (existing style).
- [ ] Asset policy: Microsoft-owned art, icons, bitmaps, string tables, help content, DUI
      markup, and Ribbon XML are **not** redistributed here. Phase 6 defines how resource
      *parity* (IDs, types, dimensions, languages) is achieved with authored/substituted
      payloads; capture that decision in a root `NOTICE` alongside `LICENSE`.
- [ ] Confirm `LICENSE` covers the reconstructed source only, and that vendored
      `src/WTL/` (NuGet `WTLCSPkg`) keeps its own upstream license text.
- [ ] State the purpose line used across the project (research / interoperability /
      preservation, not affiliated with or endorsed by Microsoft) in `README.md`, `NOTICE`,
      and the docs site footer — they must not drift.

**Exit gate:** provenance ledger committed; leak guard running in CI; role split + asset
policy written down and referenced from `AGENTS.md`.

---

## Phase 1 — Reference freeze & module inventory

**Goal:** stop arguing about counts. One authoritative, regenerable inventory of every
binary in scope, with an explicit scope tier.

- [ ] Consolidate `analysis/CROSS_BINARY_MASTER.md` (35+ binaries) and
      `analysis/FINAL_SYNTHESIS.md` into a machine-readable inventory
      (`analysis/inventory.json` + generated `.md` table): name, type, image size, `.text`
      size, export count/list, RTTI class count, PDB GUID, load-time vs delay-load deps,
      registration model, in-scope tier.
- [ ] Reconcile documented contradictions and fix them at the source:
      `ROADMAP.md` says 18 targets (actual: 29); image sizes differ between the two synthesis
      docs (e.g. `MovieMakerCore.dll` 10.1 MB vs 6.5 MB; `MovieMaker.exe` 54 KB vs 1.7 MB) —
      re-measure from the reference set and mark the stale numbers.
- [ ] Fix scope tiers explicitly:
      - **Tier 1** — launcher, engine, and the DLL/EXE set already in `src/` (29 targets).
      - **Tier 2** — satellites analyzed but not built (Tracker A): Photo Gallery / shell /
        acquisition / scheduling modules.
      - **Tier 3** — Microsoft or third-party shared infrastructure (`Imaging.dll`,
        `D3DCOMPILER_46.dll`, `RSCMFT`/`MPG4DEMUX`, the `ux*`/`wl*` shared family):
        stub-compatible surface only, no re-implementation.
- [ ] For each Tier-1 module capture the *surface card*: exact exports with ordinals and
      decoration (`Name@N` vs undecorated vs C++ mangled), calling convention, COM
      CLSID/IID/AppID list, threading model, registration entries, delay-load set, and the
      SDK-era APIs it needs that no longer exist (see the SDK table in `ROADMAP.md`).
- [ ] Add every newly scoped module to **Tracker A** at the bottom of this file, and add its
      `analysis/<Module>/` page if missing.
- [ ] Record the x86-only decision (original is PE32; `build64/` is scratch) as an explicit
      out-of-scope note so it is not re-litigated.

**Exit gate:** one inventory file, generated not hand-maintained; all doc counts reconciled;
every Tier-1 module has a surface card.

---

## Phase 2 — Static specification extraction (per module)

**Goal:** produce specs complete enough that an implementer never needs to open a
disassembler — this is what makes the reconstruction clean-room rather than a transcription.

- [ ] RTTI/vtable mapping for all Tier-1 modules: class hierarchy, base classes, vtable slot
      order, and which classes are COM-aggregated. Extend the existing
      `analysis/MovieMakerCore/vtable_hierarchy.md` pattern module by module
      (`tools/rtti_extractor.py`).
- [ ] Refresh exports/imports/cross-DLL correlation (`analysis/CrossDllImports/*`,
      `tools/pe_analyzer.py`, `tools/diff_exports.py`) and stamp each generated file with
      tool + version + date + exact command so it is regenerable.
- [ ] String and resource census per module: ASCII/UTF-16 strings (`tools/string_analyzer.py`),
      dialog/string/menu/accelerator/version resources, and the `DUXT`/UIFILE `RCDATA` blobs —
      modeled on `analysis/MovieMakerLang/resource_analysis.md`.
- [ ] **Recover the UIFILE / `.duxt` binary format** field by field into
      `analysis/WLXPhotoLibraryDuiResources/uifile_format.md`. This is the single biggest
      blocker for real DirectUI hosting: `SundanceMainElementBehavior::LoadLayout` finds and
      bound-checks the blob but cannot populate the element tree yet (`TODO_AUDIT.md`).
      If recovery fails, record it as an accepted deviation with the owner-drawn shell in
      `src/MovieMakerCore/UI/SundanceMainVisual.*` as the documented substitute.
- [ ] Complete the COM census (`analysis/COMGuids/guids.md`): CLSID/IID/AppID, threading
      model, ProgID, registration keys, and which module registers what.
- [ ] Extend the registry/file-I/O surface map
      (`analysis/CrossDllImports/registry_fileio_patterns.md`): keys read/written,
      `%LOCALAPPDATA%\Microsoft\WL\...` paths, AutoPlay handlers, shell extensions, protocol
      handlers, file associations, uninstall entries.
- [ ] Re-verify the thread-model audit (`analysis/ThreadModel/thread_model_audit.md`) against
      the surface cards and flag every mismatch as a Phase-5 task.
- [ ] Write `analysis/<Module>/spec.md` for each Tier-1 subsystem: interfaces, per-method
      contracts (args, HRESULTs including `S_FALSE` semantics), side effects, ordering and
      lifetime constraints, error paths, bounds, and the quirks from `QUIRKS.md` that the
      implementation must reproduce.
- [ ] Enumerate the dead-but-shipped surfaces (legacy `.wlmp` v1 reader, add-in host,
      telemetry/SQM, Live ID sign-in, service publish endpoints) and mark each
      *spec-documented* so nobody "finishes" them by accident.

**Exit gate:** every Tier-1 module has `spec.md` + resource/GUID/registry/threading census;
UIFILE format either documented or formally declared out of scope.

---

## Phase 3 — Dynamic analysis & golden oracles

**Goal:** observable ground truth. Contracts can only be as good as the reference behavior
they were measured against.

- [ ] Instrumented reference runs in a VM (API/COM tracing, debug output, registry and file
      monitoring) per scenario; results land as `analysis/<Module>/dynamic_analysis.md`.
      Eight modules already have one — extend to all Tier-1 modules.
- [ ] Build the **golden corpus** (files stay out of git; commit a hash manifest at
      `analysis/GOLDEN_MANIFEST.md`): sample `.wlmp` projects covering every theme,
      transition, effect, text/credit style, and extent type; exported videos per publish
      profile; generated thumbnails; auto-save and recovery files; registry/MRU snapshots.
- [ ] Define the scenario list as executable scripts (input media + user actions + expected
      observations): import (image/video/audio), storyboard edits, split/trim, transition and
      effect application, text/caption editing, theme apply/remove, AutoMovie, preview
      play/pause/scrub/frame-step, save → reopen round-trip, publish to file, publish to
      service (mocked), undo/redo across all of the above, crash/auto-save recovery,
      cold start with the legacy DLLs (SQM/Bici/wlidcli/uxcore) absent.
- [ ] For each scenario record the expected observations: HRESULT sequence, window/UI state,
      files written (names, sizes, formats), resource creation counts, and timing tolerances.
- [ ] Produce the **scenario → contract map** (`tests/mmr-python/` coverage table): which
      scenario is pinned by which contract, and which are GUI-only (covered by
      `tests/mmr-gui --selftest` or the visual diff in Phase 9).
- [ ] List scenarios that cannot be observed on modern Windows (Live ID auth,
      Facebook/Flickr/Vimeo/YouTube endpoints, D3DX11 effect framework) and pin the
      documented-parity fallback instead of leaving them ambiguous.

**Exit gate:** golden manifest committed; ≥1 dynamic analysis per Tier-1 module; scenario →
contract map published and referenced from `tests/mmr-python/README.md`.

---

## Phase 4 — Build system & binary-surface parity

**Goal:** the rebuilt PE must present the same surface as the reference: same exports, same
imports, same linkage story, same registration — with every unavoidable deviation written down.

- [ ] Keep MSVC-only + Win32-only enforcement (root `CMakeLists.txt` already `FATAL_ERROR`s
      otherwise); keep the VS2012→VS2022 SDK compatibility table in `ROADMAP.md` as the single
      list, and mirror each entry with a short comment at its use site.
- [ ] **Export hygiene for all 29 targets:** exactly the reference names, ordinals, and
      decorations, via `.def` (20 `.def` files today; audit the rest — `UXCore` uses
      `__declspec(dllexport)`). The export table's internal module name must read
      `MovieMakerCore.dll`, not the historical `MOVIECORE.dll`
      (`analysis/BinaryDiff/diff_analysis.md` §3 gap 4).
- [ ] **Launcher linkage:** `MovieMaker.exe` delay-loads `MovieMakerCore.dll` and imports
      `MovieMakerMain` by name; import the correct `WLXPhotoBase` symbol
      (`?Delete@BasePrivate@@YAXPAX@Z`) rather than a locally invented one; keep the VEH +
      delay-load-survival pattern (`QUIRKS.md` #1).
- [ ] **Import-table parity:** drive `target_link_libraries` from the Phase-1 inventory so the
      rebuilt import DLL set matches the reference — the diff lists 18 missing import DLLs and
      missing GDI+/ole32/GDI32 functions (`analysis/BinaryDiff/diff_analysis.md` §2.4–2.5, §3).
- [ ] **Toolchain/ABI decision record:** MSVCR110 (VS2012) vs MSVCP140/VCRUNTIME140 (VS2022),
      linker version stamp, Control Flow Guard, image checksum, and section layout
      (`.text`/`.data`/`.rsrc`/`.reloc` vs the extra `.rdata`/`.fptable`). True 1:1 here needs
      the v110 toolset — either add an optional VS2012 build lane or sign off the deviation in
      the register (Phase 9). Do not leave it implicit.
- [ ] Resource sections: wire real `.rc`/`.rsrc` into every target that has one in the
      reference (Phase 6 does the content; this phase does the build plumbing).
- [ ] **CI:** re-enable push/PR triggers in `.github/workflows/build-projects.yml` (currently
      `workflow_dispatch` only) and add jobs for: configure+build all 29 targets on
      `windows-2022`, `tools/diff_exports.py` as a hard gate, `ctest`, and the contract suite
      with `WMMR_DLL_DIR` pointed at `build/bin/Debug` (never `build_clean`).
- [ ] Add a repo-hygiene job: run `python3 tools/todo_audit.py --check` and the Phase-0 leak
      guard on every PR.

**Exit gate:** all 29 targets build clean; export parity 100% per `tools/diff_exports.py`;
import-table delta either closed or listed as accepted deviations; CI runs on PR.

---

## Phase 5 — Surface-by-surface re-implementation

**Goal:** replace safe fallbacks with real behavior, subsystem by subsystem, each change
pinned by a contract. Burn down `analysis/E_NOTIMPL/inventory.md` (90 entries: 4 HIGH,
19 MEDIUM first) and keep `tools/todo_audit.py` clean.

### 5.1 Engine core — `src/MovieMakerCore/`

- [ ] **SundanceApp:** guarantee controller construction (import, export, undo, clipboard,
      playback) so the null-controller `E_NOTIMPL` paths become unreachable; command-line
      parsing; single-instance mutex quirk; auto-save/recovery; shutdown ordering.
- [ ] **StoryboardManager + Serialization:** byte-stable `.wlmp` round-trip against the golden
      projects (element/attribute order, whitespace, default elision, version stamp); extent
      and timeline math; templates (23 effects / 7 themes); theme apply/remove; legacy v1
      reader kept as documented dead path.
- [ ] **HMREngine:** X3D-style scene graph and field system, pattern meshes, DWrite text
      pipeline, DX11 resource management, and the D3DX11 replacement policy
      (`HMREngine/d3dx11compat.*`) — keep the depth/count bounds (`StubDesign.md`).
- [ ] **HMRAVSource:** MF source/stream-sink/host, `TextureInterOp` (DX9 + DX11 shared
      handles), video processing (DXVA2 path), transcode manager, audio chain (fade, ducking,
      channel mapping, mixer, waveform, output/capture queues).
- [ ] **UI:** real DirectUI element-tree hosting from the recovered `.duxt` (blocked on
      Phase 2), Ribbon command surface (67 commands, `UpdateState`,
      `ComputeCommandEnabled`, MRU site registry), the 50+ behavior `OnMessage` handlers, and
      an explicit decision — recorded in the module doc — on where the owner-drawn shell
      (`SundanceMainVisual`) ends and DUI hosting begins.
- [ ] **Preview/Transport:** presenter, data context, transport command bin, playback timing,
      seek/frame-step accuracy, and the preview-client out-of-process boundary.
- [ ] **Publishing:** publish job lifecycle, provider/plugin manager, summary dialogs,
      background encode + upload state machine (endpoints mocked, credentials DPAPI-only).
- [ ] **DataStructs / utility containers:** parity of the container semantics the rest of the
      engine relies on (copy/move, growth, invalidation).

### 5.2 Supporting modules already in `src/` (per-module burn-down)

Each item = exports verified behaviorally (not just linkable) + contracts + module doc updated.

- [ ] `WLXPhotoBase` — the 56 `Base*` exports: exceptions, memory, version/OS checks, string
      and GDI+ helpers; this is the foundation every other module imports.
- [ ] `WLXPipeline` — factory surface (`GetPipelineCreateFunctions`), graph construction.
- [ ] `WLXPipetran` — `GetTFXCreateFunctions` plus visual parity for all 22 generic
      transitions against golden frames.
- [ ] `WLXSlideshow`, `WLXPhotoCinematic` (Ken Burns), `WLXMovieLibrary` (D3D9 video
      processor), `WLXVideoTrim` (custom `FACILITY_WLXVIDEOTRIM` / `AVS_E_*` HRESULTs).
- [ ] `WLXMP4Parser` — box walker, demux behavior, bounds kept (`StubDesign.md`).
- [ ] `WLXFaceRecognition` — detection/recognition pipeline, downscale bounds, COM objects.
- [ ] `WLMFDS`, `WLMFReadWrite`, `WLXTranscode.exe`, `WLXCodecHost.exe` — MF/DS bridge,
      source reader/sink writer wrappers, CLI surface and exit codes, codec isolation host.
- [ ] `MovieMakerPreviewClient`, `WLXMediaPublishSubscribe` (22 exports), `MetadataSys`
      (property handler), `MovieMakerLang` (resource satellite).
- [ ] Inert-by-design surfaces stay inert but correct: `DmxBici`, `WLXPhotoSqm` (telemetry
      returns `S_OK`, sends nothing), `UXCore`/`uxctl`/`wlidcli` (survive absence).
- [ ] Reconstruction-side modules with no reference twin (`MediaCatalog`, `PlaybackEngine`,
      `ProjectManager`, `TimelineEngine`, `GPURenderer`) — label them clearly as
      reconstruction scaffolding in their module docs so they are never mistaken for 1:1
      targets, or fold them into the module they emulate.

### 5.3 Tier-2 satellites (analyzed, not yet reconstructed — Tracker A)

For each: scope decision → `spec.md` → CMake target + `.def` → contracts → docs page.

- [ ] Photo/shell family: `WLXPhotoAcq`, `WLXPhotoClassic`, `WLXPhotoViewer`,
      `WLXPhotoVoyager`, `WLXPhotoGallery` (EXE), `WLXPhotoGalleryRepair`,
      `WLXPhotoLibraryMain`, `WLXPhotoLibraryDatabase`, `WLXPhotoLibraryDuiResources`,
      `PhotoViewerShim` (+x64), `NPWLPG`, `wlsoxe`, `wlxclip`.
- [ ] Acquisition/media helpers: `WLXVideoAcquireWizard.exe`,
      `WLXVideoCameraAutoPlayManager.exe`, `WLXImageTranscode`, `WLXDSPA`, `WLXQuickTime`
      (control host + shell ext), `WLXVAFilt`, `WLXSendMail`.
- [ ] Scheduling/services: `WLXGrinderScheduler`, `WLXCodecHostPS` (proxy/stub),
      `AlbumDownloadProtocolHandler`, `WLXAlbumDownloadWizard`.
- [ ] Shared infra (Tier 3, surface-only): `Imaging.dll`, `RSCMFT`/`MPG4DEMUX`,
      `WLAVRes`, `sqmapi`, `wlbici`, `wldcore`, `wldlog`, `wlidux`, `uxcalendar`,
      `uxcontacts`, `uxctlloc`, `D3DCOMPILER_46` (reflection-only use).

### 5.4 .NET publish plugins

- [ ] Decide and record: reconstruct `WLFacebookPlugin`, `WLFlickrPlugin`, `WLVimeoPlugin`,
      `WLYouTubePlugin` (+ `PublishPluginsInterop` / `SubscribePluginsInterop`) as managed
      twins, or declare them out of scope because their service endpoints are dead.
      `analysis/WLFacebookPlugin/` already holds reflection output to spec from.
- [ ] If reconstructed: match assembly identity, public type/method signatures, and the
      native↔managed interop contract; keep credentials out of the repo and off disk in
      plaintext (`StubDesign.md`), and drive them from the Phase-7 mock provider.

**Exit gate (per module):** builds clean · export parity · contracts green · quirks preserved
· `analysis/E_NOTIMPL/inventory.md` and `TODO_AUDIT.md` updated · module doc page current.

---

## Phase 6 — Resources, localization & UI assets

**Goal:** `.rsrc` parity. Today the gap is stark: the reference `MovieMaker.exe` carries
~97 KB of resources and `MovieMakerCore.dll` ~4 MB (UI definitions, strings, XML templates),
while the rebuilt launcher has ~128 bytes (`analysis/BinaryDiff/diff_analysis.md` §1.5, §3).

- [ ] Add/complete resource scripts for every target that has resources in the reference
      (`src/MovieMaker/moviemaker.rc` and `src/MovieMakerLang/MovieMakerLang.rc` exist; the
      rest need them), staged through `resources/extracted/`.
- [ ] Resource-ID/type/language parity map per module: icon, cursor, bitmap, dialog, menu,
      accelerator, string table, version info, `RCDATA`/`DUXT`, manifest — IDs and dimensions
      match the reference even where payloads are substituted.
- [ ] **Clean-room asset substitution:** author or license replacement art/strings; never copy
      Microsoft bitmaps, icons, or string payloads into the repo. Record every substitution in
      `NOTICE` and in the Phase-9 deviation register.
- [ ] DUI/Ribbon markup: `.duxt`/UIFILE payloads and Ribbon XML. Where originals cannot be
      redistributed, keep the measured layout spec as source of truth —
      `website/replica/moviemaker.html` is already the pixel-verified visual spec for the
      16.4 ribbon shell, filmstrip storyboard, and status bar.
- [ ] Localization: satellite model, LCID coverage, string-ID mapping, and fallback behavior
      for missing languages (`analysis/MovieMakerLang/`).
- [ ] Manifests: `requestedExecutionLevel`, DPI awareness, `supportedOS` GUIDs, COM
      registration model (registry vs reg-free), dependent assemblies — compare against the
      captured pattern in `analysis/WLXAlbumDownloadWizard/manifest.xml`.
- [ ] Version info stamped `16.4.3528.0331` from `src/common.h` macros (`WMMR_VERSION_*`) —
      never hardcoded per target.
- [ ] Verification: `tools/resource_extractor.cpp` as the checker plus a contract that
      enumerates expected resource types/IDs per DLL and fails on drift.

**Exit gate:** rebuilt resource tables match the inventory (modulo documented substitutions);
resource-load contracts pass; `NOTICE` lists every substitution.

---

## Phase 7 — Real media pipeline (decode → render → encode → publish)

**Goal:** the last big functional gap. Export/publish currently produces verified test
frames, not real video (`ROADMAP.md` → "What's Still Stub/Placeholder").

- [ ] Real encode path: `ExportController` / `PublishJob` write a playable file through the
      MF sink writer / transcode path for every documented publish profile (container, video
      codec, resolution, frame rate, bitrate, audio codec/channels/rate).
- [ ] Profile parity: reproduce the reference profile list and its metadata exactly; compare
      output against the golden exports with declared tolerances (duration, stream metadata,
      PSNR/SSIM bands).
- [ ] Decode/import matrix: the container and codec set the reference accepted
      (WMV/AVI/MP4/MOV/M4V/3GP, WMA/MP3/AAC, JPEG/PNG/BMP/GIF/TIFF and the `Imaging.dll` RAW
      path), including identical rejection behavior/HRESULTs for unsupported input.
- [ ] Render-graph parity: compositing order for transitions/effects, alpha and blend modes,
      aspect-ratio/letterbox rules, frame-rate conversion, pan/zoom (Ken Burns), text and
      credits rendering, audio fade/ducking curves.
- [ ] Out-of-process hosts: `WLXCodecHost.exe` isolation (timeouts, crash recovery, restart
      policy) and `WLXTranscode.exe` CLI/exit-code surface.
- [ ] Publish-to-service: provider enumeration, plugin load, auth handshake, chunked upload
      state machine, retry/resume — implemented against a local mock provider; credentials
      stay in memory or in the DPAPI blob only.
- [ ] Add-in/plugin host: decide whether the inert add-in surface stays inert
      (`StubDesign.md`) or becomes a working plugin API — the latter belongs to Phase 12.
- [ ] Performance parity: preview scrub latency, thumbnail generation, background render
      thread priorities, measured against the Phase-3 traces, with tolerances documented.

**Exit gate:** one scripted "import → edit → publish to file" run produces a golden-comparable
video; every profile's metadata matches; no test-frame fallback remains on the main path.

---

## Phase 8 — Contract & regression hardening

**Goal:** the oracle covers the whole product, so 1:1 claims stay true over time.

- [ ] Grow `tests/mmr-python` past 116 checks: per-export contracts for all 29 targets
      (HRESULT, GUID, decoration, observable side effects), not just the factory entry points.
- [ ] Add cross-DLL workflow contracts (`tests/mmr-python/wmmr/workflows.py`) for every
      Phase-3 scenario in the map.
- [ ] CI hygiene: set `WMMR_DLL_DIR=build/bin/Debug` explicitly; fail loudly if
      `build_clean/bin/Debug` (the `E_NOTIMPL` parity stubs) is used as the oracle.
- [ ] GUI gate: `mmr-gui --selfcheck` + `--selftest <binDir>` (128 checks / 21 DLLs) in CI;
      keep `tests/mmr-cli` (the .NET twin) in step with it.
- [ ] Wire per-module C++ tests (`tests/<Module>/test_core.cpp`) into CTest for all 29 targets
      — today CTest reports 2 entries against 45 test directories.
- [ ] Fuzz + bounds regression for every untrusted-input parser (`WLXMP4Parser`,
      `HMREngine::X3DReader` and scene traversal, UIFILE, `.wlmp`, metadata/property stores):
      assert the documented depth/count/size caps and the absence of crashes on hostile input.
- [ ] Quirk regression tests: one test per `QUIRKS.md` entry proving it still reproduces
      (VEH + SEH double exception, `Global\WindowsLiveMovieMaker_Sundance_SingleInstance`,
      null CLSID, process-heap exceptions, D3D11 single-threaded flag, `S_FALSE` semantics,
      dead code paths).
- [ ] Security invariants as tests: no plaintext credentials on disk, DPAPI blob
      header/size gating, telemetry never opens a socket (network-capture assertion).
- [ ] Keep `python3 tools/todo_audit.py --check` clean; every new intentional gap gets a
      `TODO(reconstruction)` annotation stating the missing behavior and its compatibility
      constraint.
- [ ] Nightly matrix: Debug + Release, plus a run on a clean Windows VM with the legacy DLLs
      (SQM/Bici/wlidcli/uxcore) absent to prove startup survival.

**Exit gate:** CI green on PR; contract/CTest/self-test counts recorded in the snapshot table
above and in `README.md`; zero un-annotated intentional gaps.

---

## Phase 9 — 1:1 verification & diff closure

**Goal:** every remaining difference between reference and reconstruction is either closed or
formally accepted, in writing, with evidence.

- [ ] Re-run the binary diff against the current build and refresh
      `analysis/BinaryDiff/diff_analysis.md`; walk its 12 numbered gaps (Tracker C) and mark
      each **closed** or **accepted deviation + rationale**.
- [ ] Publish `analysis/PARITY_SCORECARD.md` (template in Tracker D): per module — export
      parity, import parity, COM/registration parity, resource parity, section/layout,
      behavioral contracts, visual diff, quirk reproduction, with percentages and links to the
      evidence.
- [ ] Visual diff harness: capture the rebuilt UI per tab/dialog/state and compare against
      reference captures and `website/replica/moviemaker.html`; automate with a pixel-diff
      threshold and store the report next to the harness.
- [ ] File-format diff: byte-level `.wlmp` round-trip against golden projects; enumerate the
      non-deterministic fields (timestamps, generated GUIDs) and how they are normalized.
- [ ] Behavioral diff runner: replay the Phase-3 scenario scripts against a reference VM and
      the rebuilt build; diff HRESULT sequences, side effects, and produced files.
- [ ] Accepted-deviation register (root `DEVIATIONS.md` or a scorecard section): CRT/linker
      ABI, D3DX11 effect framework, telemetry/SQM, dead service endpoints, substituted art and
      strings, x86-only, out-of-scope Tier-2/3 modules — each with reason, impact, and owner.
- [ ] Write the overall **1:1 claim statement**: what is byte/surface/behavior-identical, what
      is behavior-equivalent, and what is intentionally different. Link it from `README.md`.

**Exit gate:** scorecard + deviation register published; no open gap without a decision.

---

## Phase 10 — Installation, registration & deployment parity

**Goal:** installing, registering, and removing the reconstruction behaves like the 2012
Live Essentials install.

- [ ] Reproduce install-time effects: file placement (32-bit program files layout), Start-menu
      and desktop shortcuts, `.wlmp` file association, AutoPlay handler
      (`WLXVideoCameraAutoPlayManager`), shell extensions (`PhotoViewerShim`,
      `AlbumDownloadProtocolHandler`), COM registration (`.rgs`-equivalent keys /
      `DllRegisterServer`), and uninstall entries.
- [ ] Build an installer for reconstruction artifacts only (WiX/MSI or Inno — record the
      choice); verify install → launch → uninstall on a clean Windows VM leaves no residue
      beyond documented keys.
- [ ] Elevation and per-user behavior: non-admin install, per-user registration, UAC prompts —
      match the reference or record the deviation.
- [ ] Coexistence rules: behavior when a genuine 2012 install is present (file/registry
      conflicts, the `Global\` single-instance mutex, shared COM registrations).
- [ ] Offline-first guarantee: no network traffic at startup, telemetry inert, verified by the
      Phase-8 network-capture test.
- [ ] Launcher entry points (`apps/moviemaker-launcher` and the other 9 exercise apps) stay
      green against the installed layout, not just the build tree.

**Exit gate:** clean-VM install → launch → scenario → uninstall all pass; registration and
association parity documented in the scorecard.

---

## Phase 11 — Documentation & clean-room deliverables

**Goal:** a third party can audit the reconstruction claim end to end.

- [ ] Keep the Docusaurus site (`website/docs/`) in sync: a page per module (most exist), plus
      new pages for this TODO, the parity scorecard, and the deviation register; keep the
      Jekyll root site (`index.html`, `_includes/footer.html`, `_config.yml`) consistent with
      the root doc set.
- [ ] Audit "recreation notes" coverage at non-obvious call sites (existing convention) for
      everything added in Phases 5.3, 5.4, 6, and 7.
- [ ] Update `ROADMAP.md` counts (targets, subsystems, stub totals) whenever Phase 4/5/7
      changes them, and correct the stale 18-target figure.
- [ ] Extend `THINKING_PROCESS.md` with the clean-room process actually used (spec writer →
      spec → implementer → contract → parity evidence), so the methodology is reproducible.
- [ ] Contributor guide: how to claim a task from this file, the spec-first workflow, contract
      requirements, submodule + gitlink rules, and the Definition of Done from `AGENTS.md`.
- [ ] Publish the per-module audit trail (spec author, implementation commit range, contract
      IDs, scorecard row) — this is the artifact that substantiates "clean room".

**Exit gate:** docs site builds and reflects current state; audit trail complete for all
Tier-1 modules; root doc set internally consistent (no contradictory counts).

---

## Phase 12 — Remix / modernization (**explicitly out of scope for 1:1**)

Tracked here only so it never contaminates the parity baseline. Each item requires an explicit
request, its own branch, and its own document:

- [ ] Fix the 23 preserved quirks in `QUIRKS.md` (that file is the checklist).
- [ ] x64 port / modern SDK & toolchain, ARM64.
- [ ] Working replacements for dead services (publish endpoints, Live ID sign-in).
- [ ] Real plugin/add-in API instead of the inert host.
- [ ] Modern UI stack, portable/non-Windows targets, packaged store build.

---

## Cross-phase trackers

### Tracker A — Analyzed but not yet reconstructed

Regenerate with:

```bash
for d in analysis/*/; do n=$(basename "$d"); [ -d "src/$n" ] || echo "$n"; done
```

Non-module analysis dirs (`ArchitectureSynthesis`, `BinaryDiff`, `COMGuids`, `CrossDllImports`,
`E_NOTIMPL`, `RegRes`, `Shared`, `ThreadModel`) are evidence, not targets. Note the renames:
`analysis/MovieMakerExe` → `src/MovieMaker`; `analysis/Shared/uxcore.md` → `src/UXCore`;
`analysis/Shared/wlbici.md` → `src/DmxBici`. Everything else below is a real Phase 5.3/5.4
candidate:

| Group | Modules | Tier |
|---|---|---|
| Photo/shell | `WLXPhotoAcq`, `WLXPhotoClassic`, `WLXPhotoViewer`, `WLXPhotoVoyager`, `WLXPhotoGallery`, `WLXPhotoGalleryRepair`, `WLXPhotoLibraryMain`, `WLXPhotoLibraryDatabase`, `WLXPhotoLibraryDuiResources`, `PhotoViewerShim` (+x64), `NPWLPG`, `wlsoxe`, `wlxclip` | 2 |
| Acquisition/media | `WLXVideoAcquireWizard`, `WLXVideoCameraAutoPlayManager`, `WLXImageTranscode`, `WLXDSPA`, `WLXQuickTime`, `WLXVAFilt`, `WLXSendMail` | 2 |
| Scheduling/services | `WLXGrinderScheduler`, `WLXCodecHostPS`, `AlbumDownloadProtocolHandler`, `WLXAlbumDownloadWizard` | 2 |
| Publish plugins (.NET) | `WLFacebookPlugin`, `WLFlickrPlugin`, `WLVimeoPlugin`, `WLYouTubePlugin`, `PublishPluginsInterop` | 2 |
| Shared infra | `Imaging`, `SharedMFDlls` (`RSCMFT`, `MPG4DEMUX`), `D3DCOMPILER_46`, `WLAVRes`, `sqmapi`, `uxctlloc`, `uxcalendar`, `uxcontacts`, `wldcore`, `wldlog`, `wlidux` | 3 |

### Tracker B — Stub burn-down

| Source | Count | Where |
|---|---|---|
| `E_NOTIMPL` inventory (0 CRITICAL / 4 HIGH / 19 MEDIUM / 67 LOW) | 90 | `analysis/E_NOTIMPL/inventory.md` |
| Open `TODO(reconstruction)` annotations | 0 (clean) | `TODO_AUDIT.md` + `tools/todo_audit.py --check` |
| Documented-parity fallbacks (do **not** "fix") | see policy | `StubDesign.md`, `QUIRKS.md` |

Order of attack: HIGH → MEDIUM → the export-observable LOWs → internals resolved with a
recreation note. Never delete a documented quirk or telemetry no-op to reduce the count.

### Tracker C — Binary-surface gaps (from `analysis/BinaryDiff/diff_analysis.md`)

| # | Gap | Class | Status |
|---|---|---|---|
| 1 | 18 missing import DLLs in `MovieMakerCore.dll` | critical | `[ ]` Phase 4 |
| 2 | ~4 MB of embedded resources missing | critical | `[ ]` Phase 6 |
| 3 | Over-exporting (4,224 symbols vs 1) | critical | `[~]` `.def` exists — re-verify |
| 4 | Export module name `MOVIECORE.dll` vs `MovieMakerCore.dll` | critical | `[ ]` Phase 4 |
| 5 | Static vs delay-loaded EXE→DLL linkage | significant | `[ ]` Phase 4 |
| 6 | `WLXPhotoBase` imported symbol mismatch | significant | `[ ]` Phase 4 |
| 7 | CRT mismatch MSVCR110 vs MSVCP140/VCRUNTIME140 | significant | `[ ]` decision record |
| 8 | Missing GDI+/ole32/GDI32 import functions | significant | `[ ]` Phase 4 |
| 9 | Extra sections (`.rdata`, `.fptable`) | cosmetic | `[ ]` accept or close |
| 10 | Linker version 11.00 vs 14.44 | cosmetic | `[ ]` accept or close |
| 11 | CFG enabled in rebuild, not in original | cosmetic | `[ ]` accept or close |
| 12 | No image checksum in rebuild | cosmetic | `[ ]` accept or close |

### Tracker D — Parity matrix template (copy per module into `analysis/PARITY_SCORECARD.md`)

| Module | Exports | Imports | COM/reg | Resources | Behavior (contracts) | Visual | Quirks | Deviations |
|---|---|---|---|---|---|---|---|---|
| `<Module>` | n/m | n/m | n/m | n/m | n/m | pass/fail/n-a | n/23 | links |

---

## Maintenance

- Update the **status snapshot** and **phase map** whenever a gate is passed; keep the counts
  regenerable (commands are listed inline) rather than hand-copied.
- A phase moves to `[x]` only when its Exit gate has committed evidence.
- This file plans the work; `TODO_AUDIT.md` indexes intentionally incomplete *source*;
  `StubDesign.md` decides whether a gap is even a defect. When they disagree, resolve the
  disagreement here and link the decision.
- Never commit reference binaries, disassembly, or Microsoft-owned assets — see Phase 0.
