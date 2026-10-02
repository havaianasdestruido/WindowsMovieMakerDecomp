---
sidebar_position: 4
title: Calling Conventions & Exports
description: stdlib-vs-exports rules, name decoration, and the .def-file pattern that keeps parity enforced.
---

# Calling Conventions & Exports

## The rule

> Every public entry point is `__stdcall` (WINAPI) **except**:
> - `MovieMakerMain` in `MovieMakerCore.dll` — `__cdecl`, undecorated
> - the UXCore resource/registry helpers in `Resources.cpp` — `__cdecl`

This is not stylistic: the calling convention determines the **export decoration**, and
export parity with the reference binary is mandatory. A `__stdcall` `int f(int)` exports as
`_f@4`; a `__cdecl` one exports as `f`. Get it wrong and the DLL's export table no longer
matches the original — `tools/diff_exports.py` will (correctly) fail the change.

## Decoration cheat sheet

| Convention | 32-bit decoration | Example |
|---|---|---|
| `__stdcall` (WINAPI) | `_Name@N` (N = bytes of params) | `_CreateVideoPlayer@4` → exported as `CreateVideoPlayer@4` |
| `__cdecl` | `Name` (undecorated) | `MovieMakerMain` |
| C++ mangled (class methods) | `?Method@Class@@YG...Z` | DmxBici / WLXPhotoSqm / WLXPhotoBase exports |

## How parity is achieved

### 1. `.def` files (most modules)

Each module carries `<Module>.def` listing exports with ordinals. Two patterns appear:

```text
LIBRARY WLXVideoTrim
EXPORTS
    CreateAVICopierDirect @1
    CreateVideoCopierFromMediaType @2
    ...
```

```text
LIBRARY DmxBici
EXPORTS
    ; C++ mangled name aliased to a stable C-style undecorated export
    ?AddStringToDataPoint@BiciWrapper@@YG_NKKPB_W@Z = _BiciWrapper_AddStringToDataPoint@12 @1
    ...
```

The second pattern keeps the **original mangled name** (as found in the reference binary)
while providing a forwarder to the implementation symbol — both names appear in the export
table, exactly as in the original.

### 2. `__declspec(dllexport)` (UXCore)

`UXCore` exports directly from headers via `__declspec(dllexport)` on the DirectUI classes
(`Element`, `HWNDElement`, `GPURenderer`, ...) rather than a `.def` file.

### 3. Ordinals matter

Aliases carry explicit ordinals (`@1`, `@2`, ...). Ordinal-only imports by legacy consumers
rely on these numbers, so they are treated as part of the contract.

## Checking parity

```powershell
# One DLL
python tools/diff_exports.py undecomp\WLXVideoTrim.dll build\bin\Debug\WLXVideoTrim.dll

# Whole directory pair
python tools/diff_exports.py --batch undecomp build\bin\Debug
```

Run this whenever an export is added, removed, renamed, or its convention changes. See
[Analysis Tools](../tools/analysis-tools.md) for the full tool guide.

## COM entry points

COM DLLs (`WLMFDS`, `WLMFReadWrite`, `WLXMP4Parser`, `WLXCodecHost`, `MetadataSys`,
`WLXMediaPublishSubscribe`, ...) export the standard quartet — `DllCanUnloadNow`,
`DllGetClassObject`, `DllRegisterServer`, `DllUnregisterServer` — alongside any API
exports. Factory behavior is pinned by the contract suite; where the reference
parity build returned `E_NOTIMPL` from a factory, our real implementation wins and the
contract pins the real HRESULT (see [Stub Design](../methodology/stub-design.md#ground-rules)).
