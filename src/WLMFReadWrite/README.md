# WLMFReadWrite

Media Foundation read/write bridge DLL.

## Fixes

- `DllCanUnloadNow` returns `S_OK` (was a leftover `S_FALSE` stub value).
  There are no outstanding COM objects, so unloading is always safe; this
  matches the behavior a reference DllMain exposes when idle.

## Contract Coverage

Contract `comstubs.py` / `wlmfreadwrite.py` pins the COM quartet and the
`S_OK` unload semantics.