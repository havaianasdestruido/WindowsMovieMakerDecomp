# WLXVideoTrim

Video trim/copy factory DLL. Exports four COM factory entry points plus
`DllGetClassObject`/`DllCanUnloadNow`/`DllRegisterServer`.

## Ported Fixes

- **DllMain COINIT tolerance** (`dllmain.cpp`): the module no longer aborts
  DLL load (`return FALSE` → error 1114) when the host thread already
  initialized COM in a different concurrency mode (`RPC_E_CHANGED_MODE`).
  `g_bComInit` is set only on `CoInitializeEx` success; all other mode
  conflicts are tolerated so the DLL always loads.

## Contract Coverage

`tests/mmr-python/wmmr/contracts/wlxvideotrim.py` pins:

- Both factory entry points → `S_OK`.
- `CreateVideoCopierDirect` returns a real copier → `S_OK`.
- `CreateVideoCopierFromMediaType` with a MEDIATYPE whose major GUID is zeroed
  → `AVS_E_UNSUPPORTED_FILE_TYPE` (0x80520005).

The reference binary returns `E_NOTIMPL` for these factories;
`build_clean` is export-parity source only.