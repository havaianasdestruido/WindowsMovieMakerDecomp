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

1. **Rendering and UI reconstruction:** DirectUI `.duxt` loading, the D3DX11
   effect compatibility layer, and D3D9/D3D11 texture interop.
2. **Media behavior:** source-image thumbnails, per-transition rendering,
   metadata writes/thumbnails, and shell-property fallback behavior.
3. **Interop and serialization:** drag/drop format enumeration and the
   unresolved serialization-writer scratch artifact.

Do not add a TODO merely because a method returns `S_OK` or `E_NOTIMPL`.
Classify it against `StubDesign.md`, preserve documented quirks, and add a
contract before replacing an externally observable fallback.
