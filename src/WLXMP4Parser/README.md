# WLXMP4Parser

MP4/MOV atom reader + filter graph builder. Bespoke `BoxReader` walks atoms
with strict bounds checks.

## Hardening

- `MoovBox::Parse` caps track count at **1024** (unbounded `push_back` fixed).
- `ParseStts` accumulates sample counts into a 64-bit value, clamped to
  `0xFFFFFFFF` before assignment (32-bit overflow fixed).
- All box reads validate `m_pos + sizeof >= m_cbData` before dereferencing.

## Contract Coverage

- `BuildMP4FilterGraph` / `BuildMP4PlayBack` on a nonexistent file → `FAILED`.
- `AddMP4SourceFilter` with a NULL graph → `E_INVALIDARG` (0x80070057).
- `IsMP4FilePlayable` → `FALSE` for missing file.
- COM quartet (`DllGetClassObject`/`DllCanUnloadNow`/reg) pinned.

Reference `build_clean` returns `E_NOTIMPL` for the graph-builder exports;
contracts pin our real parser behavior.