# MovieMakerCore

Main engine DLL (single export: `MovieMakerMain`). Houses the Sundance app
framework, storyboard/project model, HMREngine (X3D/DX11), HMRAVSource (MF
pipeline), full Ribbon UI, serialization, publishing, preview, and transport.

## Fixes & Hardening (2026-09 pass)

- **Clipboard** (`SundanceApp/ClipboardManager.cpp`): `GetClipboardData`
  failure now returns `E_FAIL` instead of masking as `S_FALSE`.
- **Ribbon** (`UI/Ribbon/RibbonSites.cpp`): `RegQueryValueExW("Count")`
  failure now returns `HRESULT_FROM_WIN32(lRes)` (or `E_FAIL`), not `S_FALSE`.
- **HMREngine scene traversal**: `Scene::TraverseNode`,
  `X3DChildNodeImpl::Traverse`, `GroupNodeImpl::Traverse`,
  `SwitchNodeImpl::TraverseActive`, and `EnumerateNodes::EnumerateRecursive`
  are depth-capped at **512** to prevent stack exhaustion on hostile graphs.
- **HMREngine X3DReader**: parse context caps nesting depth (512) and node
  count (100000); recursive JSON object parsing is depth-guarded. Limits live
  in `X3DParseContext` (`X3DReader.h`).
- **PSA auth store** (`External/PSAStub.cpp`): `Save()`/`Load()` now use
  **DPAPI** (`CryptProtectData`/`CryptUnprotectData`, current-user scope) to
  persist tokens to `%LOCALAPPDATA%\Microsoft\WL\MovieMaker\auth.dat`; file
  has magic/version header, 64 MB size cap, per-field length bounds, and a
  4096-entry token cap. `crypt32` added to `CMakeLists.txt`.
- **AuthProvider** (`HMRAVSource/AuthProvider.cpp`): credentials remain
  in-memory only (cache cleared on shutdown); nothing plaintext touches disk.

See `StubDesign.md` at the repo root for the guard/anomaly policy.