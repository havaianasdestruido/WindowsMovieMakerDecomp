# Portable audit regressions (#15–#32)

Run from the repository root on Linux (Python 3.11+, g++ with C++14 and ASan/UBSan):

```sh
python -m unittest discover -s tests/portable -v
```

There are no external Python dependencies for this suite. `test_native.py`
extracts actual production classes/methods into temporary translation units,
replacing only platform facilities with inert shims. It compiles with
`-fsanitize=address,undefined` and leak detection. No generated binaries or
copyrighted reference DLLs are stored in the repository.

- #15/#16: both MP4 walkers, overflowing extended sizes, exact/trailing/truncated
  boxes, every fixed parser length 0–100 and known v0 track fields.
- #17: every truncated prefix of a JSON document, arrays, escaped quotes, depth cap.
- #18: 100 start/restart/cancel/destroy cycles; real std::thread and atomic state.
- #19: bounds-checked ATL array stand-in, shrink-full/grow/wrap/FIFO/drop sequence.
- #20: failed undo/redo retains retryable entries and successful operations preserve order.
- #22: actual recovery control flow with fake loader/state and save destination.
- #23: actual close/shutdown/loop methods with handle shims enforcing join-before-close
  and terminating on WAIT_FAILED. A second harness runs 100 actual-thread open/close
  cycles with event shims and handle/live-worker counters. Not a Windows disk test.
- #24: production row-copy helper, short/padded/inverted/overflow inputs; actual
  ReadNextSample with null/error/format-change/EOF and ref-counted sample shims.
- #26/#27: synthetic PE32/PE32+ export/import tables, ordinal holes, malformed
  lengths/RVAs/signatures, ImageBase, batch mode and process exit behavior.
- #28/#29: DLL discovery, CLI failure/launch dispatch, build-root/generator assertions.
- #21/#25/#30: structural ownership/linkage/trigger/matrix checks.
- #31: actual helper with a dummy secret, workspace credential scan, and a scoped
  push to a local bare repository preserving unrelated refs. No Codeberg access.
- #32: strict-link and documented time-bounded audit exception guards.

Optional independent PE oracle (not needed in CI):

```sh
python -m venv /tmp/wmmr-verification
/tmp/wmmr-verification/bin/pip install pefile
/tmp/wmmr-verification/bin/python tests/portable/verify_pe_oracle.py
```

`WMMR_TEST_SOURCE_ROOT=/path/to/baseline` points native extraction at an older
source tree. Before-fix runs should fail; the #24 frame helper did not exist
before this change, so its baseline failure is a missing-header compile failure,
not evidence of executing the old upload path.

## Boundaries

These tests do **not** load the real x86 DLLs, execute XmlLite, model Win32 thread
scheduling, or exercise D3D11/Media Foundation decoding. Run the Windows/MSVC build,
`WMMR_DLL_DIR=<real-build>/bin/Debug` contract suite, CTest, and GPU/WARP checks
before a release. Autosave close now deliberately waits for outstanding I/O,
which may delay shutdown on slow disks. ReloadImage lifecycle/destruction remain
owner-thread operations; completion callbacks must not destroy their own worker.

The scheduled Windows matrix is the platform compile gate; the Ubuntu CI job runs
this portable suite. No contract submodule changes or reference binaries are needed.
