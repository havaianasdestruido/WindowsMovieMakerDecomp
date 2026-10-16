# Arena audit batch — 2026-10-16

## Inventory and provenance

All 18 open issues (#15–#32) were inspected before implementation, including full descriptions and evidence. GitHub attributes them to `havaianasdestruido`, not a bot. They form a contiguous batch created within 45 seconds, use the same Summary/Evidence/Root cause/Impact/Recommended fix/Acceptance criteria/Verification limitations structure, and share the same Linux extraction audit environment. Together with the task identifying the prior Arena audit, this is the attribution evidence; author identity alone is not used. No other open issues were returned (PR #33 is unrelated and excluded). Base: `main` at `a0eccb3`; working tree initially clean. No AGENTS files exist below the root.

## Dependency plan

- #15/#16: harden box walkers first, then field readers; verify actual ISO-BMFF offsets rather than copy proposed constants blindly.
- #17: bound scene tokenization and reject truncated JSON.
- #18/#23: join workers before replacement/destruction/handle cleanup.
- #19/#20/#22: preserve ring, history, and project-path invariants.
- #21/#25: remove unused competing project model; defer disconnected preview ownership/linkage explicitly. #24 still hardens the standalone playback library.
- #26–#29: repair analysis/test/build tools with synthetic fixtures and portable tests.
- #30/#31: restore automatic Windows checks and use ephemeral scoped mirror credentials.
- #32: refresh the coherent Docusaurus dependency set and verify install/audit/production build.

## Complete target inventory

### [#15: security: WLXMP4Parser box walkers accept 64-bit-overflowing sizes -> giant payloads and wrapped offsets (OOB reads)](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/15)

**Severity / subsystem:** Security — high. A crafted MP4/MOV file (e.g. opened via `IsMP4FilePlayable` / `GetTrackInfo`, reachable from import and property-display paths) causes out-of-bounds heap reads: crash at minimum, potential information disclosure. Also contradicts the documented hardening contract for this module.

**Acceptance criteria (original):**

- [ ] `BoxReader::Next` and `MP4File::Open` reject any box whose size exceeds the remaining bytes, including the wrap case shown above.
- [ ] A regression test parses a file/box with extended size `0xFFFFFFFFFFFFFFFF` and returns a parse failure (not a huge payload).
- [ ] Existing contract suite (`PASS=116 FAIL=0`) stays green.

### [#16: security: WLXMP4Parser fixed-offset box field reads exceed bounds by up to 8 bytes (ParseTkhd/ParseMvhd/ParseMdhd/ParseStsd)](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/16)

**Severity / subsystem:** Security — high (bounded). Crafted MP4/MOV files reaching `MoovBox`/`TrakBox` parsing (track info, `IsMP4FilePlayable`) trigger small out-of-bounds heap reads: reliable crash potential, limited info-leak potential. This is inside a module whose README advertises "All box reads validate `m_pos + sizeof >= m_cbData` before dereferencing".

**Acceptance criteria (original):**

- [ ] No fixed-offset read exceeds the length its guard guarantees (audit all `Parse*` helpers in the file).
- [ ] Regression tests feed each listed box (tkhd v0/v1, mvhd v1, mdhd v1, stsd) at boundary sizes and observe clean rejection, not OOB access.
- [ ] Contract suite stays green (`PASS=116 FAIL=0`).

### [#17: security: X3DReader::ParseJSON reads past end of input buffer (heap OOB read)](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/17)

**Severity / subsystem:** Security — high. X3D/JSON scene content is loaded from disk (`LoadFromFile` → `ReadFile` → `LoadFromMemory`), i.e. untrusted input per the project's own security requirements. A truncated file crashes the process (denial of service); as a heap OOB read it is in the same class the repo's fuzz/bounds plan (TODO.md Phase 8) is meant to eliminate.

**Acceptance criteria (original):**

- [ ] No `data[pos]` access in `ParseJSON` (and sibling `ParseX3DV1`/`ParseXML` tokenizers) occurs without `pos < length`.
- [ ] Truncated-scene regression test passes under ASan/UBSan or the project's chosen fuzz harness.
- [ ] Contract suite stays green.

### [#18: bug: HMREngine::ReloadImage never joins its worker thread -> std::terminate crash on restart/destruction](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/18)

**Severity / subsystem:** High (functional/reliability). Any code path that reloads an image twice in a row, or destroys a `ReloadImage`/`ImageLoaderWrapper` (which owns a `ReloadImage` via `std::unique_ptr`) before the worker finishes, aborts MovieMakerCore's host process. Because `WorkerThread` currently completes almost instantly the crash is timing-dependent, which makes it an intermittent "app randomly crashes" class of bug.

**Acceptance criteria (original):**

- [ ] `ReloadImage` can be constructed → started → destroyed repeatedly without aborting.
- [ ] `StartReload` may be called twice in sequence without terminating.
- [ ] A test exercises start/stop/restart under the contract suite or a native unit test.

### [#19: bug: AudioQueue::SetMaxItems leaves m_dwTail == m_dwMaxItems; Enqueue then indexes out of bounds (heap corruption)](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/19)

**Severity / subsystem:** High — memory safety. The audio queue sits between the source reader and audio output in `HMRAVSource`; any resize of a full queue (e.g. runtime quality adaptation) corrupts the heap: undefined behavior, crashes, exploitable write primitive in the worst case. Also reachable via the follow-up `SetMaxItems` calls themselves (the copy loop reads `srcIndex = (m_dwHead + dwDropped + i) % m_dwMaxItems`, which is correct, but leaves the broken tail).

**Acceptance criteria (original):**

- [ ] `m_dwTail < m_dwMaxItems` and `m_dwHead < m_dwMaxItems` hold after every `SetMaxItems`/`Enqueue`/`Dequeue`/`Flush`.
- [ ] Shrink-while-full regression test passes (enqueue after shrink lands in slot 0).
- [ ] Contract suite stays green.

### [#20: bug: UndoManager::Undo/Redo permanently discard the action when the callback fails](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/20)

**Severity / subsystem:** Medium-high (functional / data integrity). Any failing undo (callback returns an error — allocation failure, missing media file, locked output) silently eats that history entry: the document state and the undo stack diverge, and repeated failures drain the whole stack. Not covered by any `QUIRKS.md` entry, so it cannot be intentional parity behavior.

**Acceptance criteria (original):**

- [ ] A failed `Undo`/`Redo` leaves the corresponding count unchanged and the entry retryable.
- [ ] Successful `Undo`/`Redo` behavior is unchanged (existing semantics preserved).
- [ ] Regression tests cover both failure and success paths.

### [#21: bug: src/Project ProjectManager::Save writes a 0-byte file and returns false; dead target duplicates struct Clip](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/21)

**Severity / subsystem:** Medium (latent). Today it is dead weight that wastes build time and misleads contributors; the moment someone consumes `ProjectManager` — or includes both project headers — they inherit data loss (empty save files) and an immediate build break. The MovieMakerCore engine itself uses `StoryboardManager::MovieProject` (XmlLite-based), which is unaffected.

**Acceptance criteria (original):**

- [ ] Either `src/Project` is removed from the build, or `Save()` round-trips a project and returns `true` with a non-empty, correctly encoded file.
- [ ] No `struct Clip` redefinition: a TU including both project headers compiles (or the duplicate is gone).
- [ ] The target is either linked by a consumer or deleted; no orphan targets remain in `cmake --build build` output.

### [#22: bug: MovieProject::RecoverAutoSave makes the autosave file the project path, so saves never update the original .wlmp](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/22)

**Severity / subsystem:** Medium-high (functional, user data confusion / effective data loss). After a crash-recovery session, all subsequent saves land in the hidden autosave file (and then delete it on success), while the visible project file stays stale. Users lose the expectation that Ctrl+S persists their work; closing without noticing leaves changes only in a file that `DeleteAutoSaveFile` may have removed.

**Acceptance criteria (original):**

- [ ] After `RecoverAutoSave`, `GetFilePath()` still returns the original `.wlmp` path.
- [ ] `Save()` after recovery writes the original file and no longer deletes the recovery target mid-session.
- [ ] Regression test covers the recover → edit → save sequence.

### [#23: bug: AutoSaveManager::OnProjectClosed can abandon the auto-save thread, which spins on closed handles / freed memory](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/23)

**Severity / subsystem:** Medium-high (reliability). Crash-on-close (UAF) or a permanent 100%-CPU zombie thread after closing a project while an auto-save is in flight; intermittent by nature. Static-analysis finding — the timing window depends on disk latency, so it was not reproduced live (see limitations).

**Acceptance criteria (original):**

- [ ] `OnProjectClosed`/`Shutdown` never return while the worker thread is still runnable.
- [ ] No code path closes `m_hStopEvent`/`m_hTimerThread` while the worker can reference them; `WAIT_FAILED` exits the loop.
- [ ] Rapid open/close cycling test shows one live worker thread and stable handle counts.

### [#24: bug: PlaybackEngine::CopySampleToTexture maps a D3D11_USAGE_DEFAULT texture (always fails); row-copy underflow and IMFMediaType leak](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/24)

**Severity / subsystem:** High (functional) + medium (memory safety). The video preview path owned by `SundanceAppMain` (`m_pPlaybackEngine`, constructed at `src/MovieMakerCore/SundanceApp/SundanceAppMain.cpp:493`) can never display a frame, and any future fix that merely switches the texture to a mappable usage without fixing line 207 turns the latent over-read into a live one. The per-open leak grows with every file opened. (Today the engine is constructed but not yet driven by a render loop — see the companion wiring issue — which is why the breakage has gone unnoticed.)

**Acceptance criteria (original):**

- [ ] `CopySampleToTexture` returns a real `HRESULT` and `Map` no longer targets an unmappable resource.
- [ ] Row copy can never read beyond `cbData` (fuzz: samples smaller than one row).
- [ ] No `IMFMediaType` leak per `OpenFile` (verify with a refcount/debug-layer run).
- [ ] `ReadNextSample` terminates on pathological streams.

### [#25: bug: DirectUI preview stack (GPURenderer/PlaybackEngine/TimelineDispatcher) is constructed but never driven](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/25)

**Severity / subsystem:** Medium (incomplete feature / dead code). Users get no timeline video preview from this engine; the core DLL carries unused code that also contains live memory-safety and correctness defects (see the `PlaybackEngine` rendering issue), which will surface the day someone wires it up. It also double-initializes Media Foundation (`PlaybackEngine` ctor calls `MFStartup`) at startup for nothing.

**Acceptance criteria (original):**

- [ ] Either a UI/timer code path demonstrably calls `TimelineDispatcher::Tick` with a non-null catalog in a build, or the unwired objects/link deps are removed.
- [ ] `TODO_AUDIT.md` / roadmap reflect the chosen state.
- [ ] No double `MFStartup`-without-use remains at startup (if removed).

### [#26: tools: diff_exports.py misreads export/import directories (empty results for valid PEs) and exits 0 on failure - parity gate ineffective](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/26)

**Severity / subsystem:** Medium-high (build/release integrity). The repository's core quality claim — "Export parity is mandatory… `tools/diff_exports.py` enforces this" (`StubDesign.md`, `AGENTS.md`) — is enforced by a tool that cannot see any exports or imports and cannot fail. Any regression in the `.def` surface would pass unnoticed.

**Acceptance criteria (original):**

- [ ] `parse_exports`/`parse_imports` return results identical to `pefile` for the fixture (and for any real DLL supplied in CI).
- [ ] `diff_exports.py` exits non-zero on missing/extra exports, unparseable input, and missing files.
- [ ] Documented in `AGENTS.md`/`StubDesign.md` only after the tool actually gates.

### [#27: tools: pe_analyzer.py reports wrong Image Base for PE32 binaries (reads BaseOfData)](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/27)

**Severity / subsystem:** Low-medium (analysis correctness). `pe_analyzer.py` feeds the `-Analyze` mode of `tools/build.ps1` and writes `tools/reports/*_pe.json` snapshots used for binary comparison during reconstruction; a wrong image base skews any RVA math based on it and misleads parity analysis. No production/runtime impact.

**Acceptance criteria (original):**

- [ ] `pe_analyzer.py` reports the correct Image Base for PE32 binaries (fixture test included).
- [ ] PE32+ parsing unchanged and still correct.

### [#28: tools: test_all.py checks DLLs in .../build/build/bin/Debug, exits 0 when the target exe is missing, ignores --launch](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/28)

**Severity / subsystem:** Medium (tooling/QA integrity). The harness is the repository's runtime smoke test; a wrong DLL directory produces false negatives, and the exit-code defect produces false *positives* in any automated pipeline. Both combine to make "runtime tests pass" a meaningless statement.

**Acceptance criteria (original):**

- [ ] `test_dlls_present` finds the 15 DLLs in a standard `build/bin/Debug` layout.
- [ ] Missing/non-runnable target exe → exit code 1 and a clear report.
- [ ] `--launch` runs only launch tests and affects the exit code.

### [#29: build: tools/build.ps1 resolves $ProjectRoot to the parent of the repo - configures/builds the wrong tree](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/29)

**Severity / subsystem:** Medium (build tooling). The documented one-command build is broken on a clean machine: it either errors immediately or silently creates a stray `build/` directory in the user's parent folder, after which "build succeeded!" style messaging can mislead. Directly contradicts the documented verification flow (build → contract suite → ctest).

**Acceptance criteria (original):**

- [ ] `.\tools\build.ps1` on a clean clone configures `<repo>` and produces `<repo>\build\bin\Debug\*.dll` (Debug default).
- [ ] `-Clean` removes and recreates only `<repo>\build`.
- [ ] A guard fails fast if `$ProjectRoot` doesn't contain the root `CMakeLists.txt`.

### [#30: build: build-projects.yml automatic triggers (push/PR) are commented out - build gate only runs manually](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/30)

**Severity / subsystem:** Medium (CI/regression prevention). Every module — including the 30 CMake targets and any new ones — can silently stop compiling on `main`. Combined with the fact that the project builds only on Windows/MSVC (most contributors' local checks may differ), the disabled gate removes the only platform-correct check.

**Acceptance criteria (original):**

- [ ] A push to `main` and any PR touching `src/` triggers the matrix build.
- [ ] The workflow completes for all 29 matrix entries (proving the trigger change didn't surface pre-existing breakage — any failures found get their own issues).
- [ ] Workflow history shows automatic runs, not only `workflow_dispatch`.

### [#31: security: sync.yml embeds CODEBERG_PAT in the remote URL (persists in .git/config on the runner)](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/31)

**Severity / subsystem:** Security — low-medium (secret-hygiene). The PAT grants push access to the mirror repository; the practical risk is bounded by the ephemeral runner, but the pattern violates least-exposure and will trip any secret-scanning/audit of contributor forks. The `--mirror` breadth adds unrelated risk: force-updating all Codeberg refs means an accidental branch/tag on the mirror is silently deleted to match GitHub.

**Acceptance criteria (original):**

- [ ] No step of the workflow writes the token into `.git/config` or any other workspace file (verify by `git config --local --list` and `grep -r <token>`-style check in a test run with a dummy secret).
- [ ] The mirror still updates `main` (and tags, if intended) on push.
- [ ] Scope of mirrored refs is an explicit, documented decision.

### [#32: build: website lockfile carries 49 npm audit findings (16 critical) in the Docusaurus toolchain](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/issues/32)

**Severity / subsystem:** Low-medium (supply chain / developer-tooling). The published site is static output, so end-user exposure is limited; the real surfaces are (a) developers running `npm start` locally on a vulnerable dev server, and (b) the CI docs build pulling known-vulnerable packages. No runtime impact on the C++ reconstruction itself.

**Acceptance criteria (original):**

- [ ] `npm ci && npm audit` in `website/` reports zero critical/high findings (or every remainder is listed with a documented waiver).
- [ ] `npm run build` still succeeds with `WMMR_DOCS_BASE_URL=/WindowsMovieMakerDecomp/docs/` and `onBrokenLinks: throw`.
- [ ] Lockfile change is committed together with the version bump.


## Implementation reconciliation

All 18 defects were present at the baseline; none were already fixed. All 18 issues
have implemented resolutions (including the explicit deletion/deferral options and
the permitted #32 waiver). The final automatic Windows matrix and portable job
passed in run 38916223230. No issue is silently excluded. The portability suite
has **34 new test methods**, including **12 native sanitizer harnesses**.

Every row also has regression coverage in commit `7f709d3` under `tests/portable/`.
Subsequent regression follow-ups are recorded in the CI section below.
Source paths abbreviated below are under `src/MovieMakerCore/` unless qualified.

| Issue | Title / subsystem | Relevant implementation commits / files | Fix | Tests | Status |
|---|---|---|---|---|---|
| #15 | MP4 box-size overflow | 4dfcdff; `src/WLXMP4Parser/WLXMP4Parser.cpp` | Compare whole box size to remaining bytes in both walkers (not the issue suggestion’s undersized payload comparison); reject malformed opens and clear stale parse state. | Native overflow, exact/truncated/trailing box tests; actual MP4File::Open with file shims. | Resolved; platform limitations below apply |
| #16 | MP4 fixed-offset reads | 4dfcdff; `src/WLXMP4Parser/WLXMP4Parser.cpp` | Correct final-field guards; also correct verified tkhd v0 track-id offset to 12. | All fixed parser lengths 0–100 under ASan/UBSan; known track fields. | Resolved; platform limitations below apply |
| #17 | Truncated scene input | 4dfcdff; `src/MovieMakerCore/HMREngine/X3DReader.cpp` | Bound every token access, handle escaped quotes and nested arrays, reject incomplete/depth-exceeding JSON; guard empty XML tags and invalid input. Array fields remain unsupported/ignored as before. | Truncated JSON prefixes, nested arrays/quotes/depth under ASan/UBSan. | Resolved; platform limitations below apply |
| #18 | Reload worker termination | 2a4e44b, b0a6f38; `HMREngine/AsyncWorker.{h,cpp}` | Join before restart/destruction; atomic cancellation/result, synchronized callback copy and completion after callback. Owner-thread lifecycle required. | 100 repeated start/restart/cancel/destruct cycles under sanitizers. | Resolved; platform limitations below apply |
| #19 | Audio ring tail corruption | 2a4e44b; `HMRAVSource/Audio/AudioQueue.cpp` | Normalize tail modulo new capacity. | Shrink-full/grow/wrap/FIFO/drop with checked array shim. | Resolved; platform limitations below apply |
| #20 | Undo/redo loses failed action | 2a4e44b; `SundanceApp/UndoManager.cpp` | Restore popped action to its original stack on HRESULT failure; retain existing callback invocation behavior. | Failure/retry/order/empty history tests. | Resolved; platform limitations below apply |
| #21 | Unused broken project model | a80df7d, 1931998; `src/Project` (deleted), root CMake, CI matrix, website docs | Take explicit deletion option; preserve authoritative MovieProject and SundanceApp ProjectManager. | Structural removal/matrix checks; strict-link docs build. | Resolved; platform limitations below apply |
| #22 | Autosave retargets project path | 2a4e44b; `StoryboardManager/MovieProject.cpp`, `SundanceApp/AutoSaveManager.cpp` | Recovery restores path even on failure and marks recovered content dirty. AutoSave/WriteAutoSave and manager recovery writes serialize to streams, never Save/SaveAs. | Actual recovery control flow with loader/save shims verifies recovery/edit/save destination and dirty state. | Resolved; platform limitations below apply |
| #23 | Autosave abandons worker | 2a4e44b, a80df7d; `SundanceApp/AutoSaveManager.{h,cpp}`, `SundanceAppMain.cpp` | Wait for worker termination before closing/resetting handles; exit on failed wait; atomic worker-visible flags; shut down autosave before releasing app subsystems. | Join/close/reset ordering and WAIT_FAILED loop tests with handle shims. | Resolved; platform limitations below apply |
| #24 | Broken playback upload | 0f3982d; `src/Playback/PlaybackEngine.{h,cpp}`, `FrameCopy.h` | Negotiate decoded RGB32, own COM types/resources with WRL, stage CPU upload before GPU copy, validate complete signed-stride frames, propagate failures, bound null-sample and timestamp loops. | Native row-copy short/padded/inverted/overflow and sample error/null/EOF/refcount tests; GPU/WARP remains unverified. | Resolved; platform limitations below apply |
| #25 | Unwired DirectUI preview | a80df7d; `SundanceAppMain.cpp`, core CMake, TODO_AUDIT.md, website docs | Take explicit deferral option: remove construction and link dependencies, preserve null optional getters; MF PlaybackController authoritative. | Structural ownership/linkage checks; docs build. | Resolved; platform limitations below apply |
| #26 | Ineffective PE parity gate | 2e446bb; `tools/diff_exports.py` | Bounded PE reader, correct exports/ordinals/holes/import descriptors, nonzero CLI for errors/differences, strict batch checks; import differences informational. | Synthetic PE32/PE32+ tests and independent pefile oracle; malformed/CLI/batch cases. | Resolved; platform limitations below apply |
| #27 | PE32 ImageBase offset | 2e446bb; `tools/pe_analyzer.py` | Skip BaseOfData before reading ImageBase. | PE32 distinct BaseOfData/ImageBase and PE32+ fixture checks. | Resolved; platform limitations below apply |
| #28 | Runtime harness false results | 2e446bb; `tools/test_all.py` | Resolve EXE and adjacent DLL paths; propagate test failures; implement launch-only dispatch and argument validation. | DLL trees for Debug/Release/custom, missing EXE process exit, launch dispatch/status. | Resolved; platform limitations below apply |
| #29 | Build wrapper wrong root | 2e446bb, 07bef3d; `tools/build.ps1` | Resolve one parent of tools, guard root CMakeLists before any deletion, and configure explicit VS2022/Win32. | Root/generator/guard regression; Windows MediaCatalog job executes -Clean wrapper from outside the repository. | Resolved; platform limitations below apply |
| #30 | Automatic build triggers disabled | 1931998, 07bef3d, c68faa7, e2b765a; `.github/workflows/build-projects.yml`, `TranscodeMetadata.cpp`, core CMake | Restore push-main and pull_request; retain dispatch, Windows2022/Win32 matrix (28 after #21 deletion); add Ubuntu portable suite. | Trigger/matrix/dependency assertions; automatic run history verified. Final remote matrix outcome is recorded in PR #34. | Resolved; all 28 Windows matrix targets and portable CI pass |
| #31 | Persisted mirror PAT | 1931998; `.github/workflows/sync.yml` | Per-command credential helper from env, no credential persistence; explicit main/tag refspecs, unrelated refs preserved, read-only GitHub permissions. | Dummy helper credential fill, workspace scan, real local bare push and unrelated-ref preservation. | Resolved; platform limitations below apply |
| #32 | Website dependency advisories | 7d8ad9a; `website/package*.json`, `website/README.md` | Align Docusaurus 3.10.2, patched overrides, regenerate lock. Explicit time-bounded braces waiver lists all 29 remaining propagated high findings; no critical findings. | Clean npm ci, audit JSON (exit 1, documented waiver), production strict-link build. | Resolved via documented exception |

## Actual verification results

- `python -m unittest discover -s tests/portable -v`: **34/34 pass**, including
  12 production-algorithm C++14 ASan/UBSan harnesses. Latest run: 11.598 seconds.
- `git archive a0eccb3 src tools | tar -x -C /home/user/audit-baseline` followed by
  `WMMR_TEST_SOURCE_ROOT=/home/user/audit-baseline python -m unittest discover -s tests/portable -p test_native.py -v`
  (at the nine-native-test stage): expected failure, **8 runtime regressions
  reproduced**, plus #24 new-helper compile failure (header absent at baseline).
  The new helper failure is not represented as execution of the old GPU path.
- Isolated venv `pip install pefile PyYAML` then
  `/home/user/verification-venv/bin/python tests/portable/verify_pe_oracle.py`:
  **pass**, independent export/import oracle for both PE32 and PE32+.
- `yaml.safe_load` over `.github/workflows/*.yml` using that venv: **pass**.
- `python -m py_compile tools/diff_exports.py tools/pe_analyzer.py tools/test_all.py tests/portable/test_tools.py tests/portable/test_native.py tests/portable/test_configuration.py`:
  **pass** for all changed Python code.
- `python -m compileall -q tools tests/portable`: **fails** on unchanged
  `tools/analysis/extract_strings_resources.py:644` (Python 3.11 disallows a
  backslash in an f-string expression). This pre-existing analysis-script issue
  is unrelated to the target batch; no lint/type-check tooling is configured.
- In `website/`: `npm install @docusaurus/core@^3.10.2 @docusaurus/preset-classic@^3.10.2 @docusaurus/theme-mermaid@^3.10.2 @docusaurus/module-type-aliases@^3.10.2 @docusaurus/types@^3.10.2`,
  `npm audit fix`, and `npm install` after targeted overrides: completed.
  Unconstrained audit fix alone could not resolve the batch; no forced downgrade.
- In `website/`: final `npm ci`: **pass**, 1,376 audited packages.
- In `website/`: final `npm audit --json`: **exit 1**, 29 high, 0 critical,
  0 moderate/low. Every remaining finding is the unpatched braces advisory or a
  propagated dependent; all are explicitly listed/waived in `website/README.md`
  with review deadline **2026-11-16**. Not a clean audit.
- In `website/`: `WMMR_DOCS_BASE_URL=/WindowsMovieMakerDecomp/docs/ npm run build`:
  **pass**, server/client compiled and static output generated; `onBrokenLinks:
  'throw'` unchanged. Repeated after removal/deferral documentation changes.
- `git submodule update --init tests/mmr-python`: **pass**; pinned submodule
  unchanged. `WMMR_DLL_DIR="$PWD/build/bin/Debug" python tests/mmr-python/run_tests.py`:
  **cannot run**, exits 2 because the real DLL directory does not exist. No claim
  of PASS=116 is made; x86 Windows DLL loading needs Windows and 32-bit Python.
- CMake/CTest were initially absent. After installing CMake into the isolated venv:
  `/home/user/verification-venv/bin/cmake -S . -B /home/user/wmmr-cmake-verification`:
  **blocked**, correctly rejects Linux with "only builds on Windows with MSVC".
  `.../cmake --build build --config Debug` and
  `.../ctest --test-dir build -C Debug --output-on-failure`: **blocked**, no Windows
  build tree exists. No Windows, ATL/SDK, PowerShell, or GPU/WARP environment here.
- `git diff --check`: **pass**. Full tracked diff reviewed; no generated docs,
  Python caches, reference binaries, secrets, submodule modifications, or unrelated
  user changes included. The worktree was clean before this run.

## Remaining risks / release checks

- #30 is verified by automatic run 38916223230: all 28 remaining Windows matrix
  targets and the portable job pass, including MovieMakerCore and the clean wrapper.
- Real x86 contracts, CTest, export parity against copyrighted reference DLLs,
  slow-disk autosave behavior, and D3D11/Media Foundation debug-layer/refcount checks
  require the Windows environment. Portable shims are explicitly not substitutes.
- Autosave shutdown now waits for in-flight I/O instead of abandoning the thread;
  this may delay close on slow storage. Reload callbacks must not destroy their
  own worker; lifecycle methods remain owner-thread operations.
- #25 is intentionally deferred, not a claim that timeline preview now works.
  #21 removes only unused scaffolding, not the core project manager.
- The Codeberg secret and live service were never used; dummy/local integration
  validates credential hygiene and ref scope, not remote authentication.
- #32 retains a time-bounded, fully enumerated high-severity exception; updated
  cross-range overrides need continued upstream compatibility review. The static
  production build was tested, not a browser/HMR test.

## Automatic CI follow-up (same PR #34)

The automatic `pull_request` event is verified in run
[38914408495](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/actions/runs/38914408495).
It built 27 of the 28 Windows targets successfully, but exposed a pre-existing core
compile failure and a portable harness compilation failure. We did not suppress
or skip either failure:

- `b0a6f38` (#18/#30): explicitly include `<system_error>` (newer GCC no longer
  provides it transitively); fixed both production source and extracted harness.
  A separate fixture lifetime correction (`2f73583`) keeps callback captures alive
  until the joined worker is destroyed. Final portable CI is green at `c68faa7`.
- `c68faa7` (#30): replace nonexistent `PKEY_DateTaken` with SDK
  `PKEY_Photo_DateTaken` and avoid taking `CComPtr::operator&` when constructing an
  array of borrowed metadata readers. Both defects are present in the baseline.
- The follow-up Windows build compiled all changed core code, then found one
  unresolved DynamicDataObjectWrapper constructor. `e2b765a` (#30) includes the
  already-existing `External/DragDropStub.cpp` implementation in the core target;
  no replacement stub or weakened linker option was introduced.
- `07bef3d` (#29/#30): root guard added before `-Clean`, and the Windows
  MediaCatalog job now executes `tools/build.ps1 -Clean -Target MediaCatalog
  -Verbose` from `$RUNNER_TEMP`. That real PowerShell integration check passed in
  run [38915530274](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/actions/runs/38915530274).
- `1394bfc` (#23/#30): additional 100-cycle real-thread autosave regression, with
  live-worker and handle-count assertions. Latest local suite: **34/34 pass**,
  including 12 sanitizer harnesses; pefile oracle and changed-Python compilation
  were rerun successfully.

GitHub job logs redirect to Azure Blob storage, which this sandbox cannot access.
The workflow therefore preserves actionable failure tails as check annotations
without changing failure exit status. Earlier CI failures are part of the record,
not passed runs. Attempts to cancel superseded runs were denied by the integration
(HTTP 403); no permission workaround was attempted. The final automatic run and
final #30 disposition are recorded in the PR's Verification section. #30 must not
be closed unless all remaining Windows targets pass.

### Final reconciliation

[Automatic run 38916223230](https://github.com/havaianasdestruido/WindowsMovieMakerDecomp/actions/runs/38916223230)
completed **successfully** for code commit `caa0b78`: **28/28 Windows/MSVC Win32
matrix builds**, plus **34/34 portable regressions** in the Ubuntu job. The
MediaCatalog job's real PowerShell `-Clean` wrapper check also passed. #30 is now
fully resolved, including the newly exposed compile/link blockers; no follow-up
build failure remains to track. This final documentation-only update records the
already-verified code and does not change it.

Final original-inventory disposition: **18 resolved, 0 already fixed, 0 blocked
or partially resolved**. #21 is resolved by deletion, #25 by explicit deferral,
and #32 by patched updates plus the acceptance criteria's documented waiver.
Windows DLL contracts/CTest/GPU runtime remain unexecuted as detailed above; a
successful matrix build is not represented as a contract or runtime test pass.
The PR contains closing references for all 18 original issues.

Additional baseline tool check using the same synthetic fixture against the
`git archive` source: #26 returned `{}`/empty imports and exited 0 for missing
inputs; #27 reported `0x3000` instead of `0x400000`. The fixed fixture/oracle
checks pass. Final diff and clean working-tree checks passed after removing only
generated Python caches.
