# Reconstruction TODO Audit

This is the maintained index for work that is intentionally incomplete in the
reconstruction. It complements, rather than overrides, `StubDesign.md`:
telemetry no-ops, documented parity behavior, and intentionally returned
`E_NOTIMPL` values are **not** automatically defects.

## Coverage

`tools/todo_audit.py` reads every line of each tracked, authored source file
with a supported source extension (`.bat`, `.c`, `.cc`, `.cmake`, `.cpp`,
`.cs`, `.cxx`, `.def`, `.h`, `.hpp`, `.ps1`, `.py`, and `.rc`) plus all
`CMakeLists.txt` files. Vendored `src/WTL/` code and Git submodules are
excluded because they are not maintained by this project. The script finds
source annotations in the following uniform format:

```cpp
// TODO(reconstruction): Describe the missing behavior and its compatibility constraint.
```

Run the audit with:

```bash
python3 tools/todo_audit.py --check
```

The output is location-based, so it stays accurate as source moves. A TODO
should state the missing behavior and why the current safe fallback must remain
until binary analysis and a contract establish the replacement.

## Current priorities

The audit is **clean**: the 14 annotations from the 2026-09 pass were all
resolved in the 2026-10-08 pass, grouped by area:

1. **Rendering and UI reconstruction:**
   - `SundanceMainElementBehavior::LoadLayout` now locates and bound-checks
     the binary `.duxt` (UIFILE) resource under the `DUXT` RCDATA type. Tree
     population stays deferred: the UIFILE binary format is not yet recovered
     and the DirectUI element tree lives in `directui.dll`/`uxcore.dll`
     (dead-path stub surface) — recorded as a recreation note at the call site.
   - `D3DX11CreateEffectFromMemory` keeps its `E_NOTIMPL` fallback as
     documented parity: the D3DX11 effect framework is an external Microsoft
     runtime that no longer ships with modern Windows, and no in-process
     replacement can match the reference interface — recorded as a recreation
     note (see `ROADMAP.md`'s SDK-compatibility table).
   - `TextureInterOp` gained its D3D9 device and real
     `CreateTextureInternal`/`CreateSurfaceInternal` implementations (the
     texture is created with the shared-handle out-parameter when the desc's
     `fShared` flag is set); `TextureInterOpDX9` now delegates to them and
     `TextureInterOpDX11` keeps its D3D11 path.
2. **Media behavior:**
   - `MediaCatalog::CreateThumbnail` decodes and scales the source image with
     WIC into a bounded 160x120 thumbnail box (deterministic 1x1 fallback
     kept for undecodable files).
   - All 22 generic WLXPipetran transitions now have distinct render
     algorithms matching their pattern-mesh geometries (wipes, pushes,
     slides, rotate/flip/spin/zoom, pixelate, dissolve, blur, shrink, expand).
   - `TranscodeMetadataParser::WriteMetadata`/`SetMetadataAttribute` persist
     through the shell property-store path (`IShellItem2::GetPropertyStore`),
     and `ExtractThumbnail` pulls the first video frame via the MF source
     reader and encodes it as a JPEG through WIC.
   - `GetPropertyString` falls back to the WIC metadata query readers for
     formats without an OLE property store.
3. **Interop and serialization:**
   - `DynamicDataObjectWrapper` implements `GetDataHere` (caller-provided
     HGLOBAL storage) and `EnumFormatEtc` (snapshot `IEnumFORMATETC` over the
     entry list), and `SetData`/`GetData` now handle the HGLOBAL-only storage
     contract correctly.
   - `MediaBrowser`'s `CreateDataObject` builds a real `IDataObject` from the
     caller's format/medium pairs (this also fixed the `CF_HDROP` medium
     allocation, which used `sizeof(CF_HDROP)` — the 4-byte format constant —
     instead of `sizeof(DROPFILES)`).
   - The unreferenced `SerializationWriter_tmp.cpp` scratch file was removed;
     the recovered writer is implemented in `SerializationWriter.cpp`.

Remaining stub work that is intentionally **not** annotated with
`TODO(reconstruction)` — telemetry no-ops, documented parity `E_NOTIMPL`s, and
dead code paths — is triaged separately in `analysis/E_NOTIMPL/inventory.md`
(92 stubs; the MEDIUM/HIGH entries there are the next candidates for
reconstruction passes).

Do not add a TODO merely because a method returns `S_OK` or `E_NOTIMPL`.
Classify it against `StubDesign.md`, preserve documented quirks, and add a
contract before replacing an externally observable fallback. Behaviors that
are not externally observable (internal classes, static-library helpers, and
`__thiscall` member functions the ctypes harness cannot bind) are resolved by
implementation plus a recreation note at the call site; when such a behavior
becomes export-observable, pin it with a contract in `tests/mmr-python`.
