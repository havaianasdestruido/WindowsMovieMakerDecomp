---
sidebar_position: 6
title: Troubleshooting
description: Known failure modes when building, running, and testing the reconstruction.
---

# Troubleshooting

## Build-time failures

### `fatal error C1083: Cannot open include file: 'atlbase.h'`

The VS2022 Build Tools are installed **without the C++ ATL component**. Open the Visual
Studio Installer → *Modify* → *Individual components* → check **"C++ ATL for latest v143
build tools (x86 & x64)"** → rebuild.

### `This project only builds on Windows with MSVC.` / `This project requires the MSVC compiler.`

CMake configure-time guards. Use the supported toolchain: Windows + VS2022 + generator
`"Visual Studio 17 2022"` with `-A Win32`. Clang, gcc, and non-Windows hosts are
unsupported by design.

### Link errors about missing `d3dx11.lib`, `dxva2.lib`, or `mferror.lib`

These are **expected to be absent** on the Windows 10 SDK — the project ships compatibility
shims instead (`src/MovieMakerCore/HMREngine/d3dx11compat.h`, `dxva2stubs.cpp`) and must not
link the removed import libraries. If you see these errors, a module's `CMakeLists.txt` was
modified to link them — revert. See [SDK Compatibility](../reference/sdk-compatibility.md).

### CMake configures but nothing builds / wrong architecture

Use `-A Win32` (x86). A Win64 build produces binaries whose export decorations and
contract-suite behavior do not match the reference set.

## Test-time failures

### Contract suite reports `E_NOTIMPL` from factories

You ran the suite against `build_clean\bin\Debug` (the default) — the reference/parity-stub
binaries. Set the environment variable first:

```powershell
$env:WMMR_DLL_DIR = "$PWD\build\bin\Debug"
python32\python.exe tests\mmr-python\run_tests.py
```

### `OSError: [WinError 193] %1 is not a valid Win32 application` when loading DLLs

You are running **64-bit Python**. The DLLs are 32-bit; only the gitignored embedded
32-bit CPython (`python32\python.exe`) can load them.

### Contract suite cannot find DLLs

`WMMR_DLL_DIR` must point at the directory *containing* the DLLs
(`build\bin\Debug`), not at the repo root, and the build must have produced output there.

### GUI harness fails to start

`mmr-gui` needs `.NET 10` with the **x86** runtime and `PlatformTarget=x86`. Set
`$env:DOTNET_ROOT_X86` to the x86 runtime location. The `--selfcheck` mode runs
managed-only and needs no DLLs — use it to isolate toolchain issues from DLL issues.

## Runtime issues

### MovieMaker.exe exits immediately

Check, in order:

1. Is `MovieMakerCore.dll` next to `MovieMaker.exe` (`build\bin\Debug`)? The launcher
   `LoadLibrary`s it from its own directory.
2. Is something else holding a single-instance mutex? Startup creates two:
   `Global\WindowsLiveMovieMaker_Sundance_SingleInstance` (in `MovieMakerMain`) and
   `Global\WindowsLiveMovieMaker_SundanceApp` (in `SundanceAppMain` initialization).
   (Any program creating either name blocks launch — preserved original behavior,
   [Quirk #2](../methodology/quirks.md#2-sundance-codename-leaked-into-runtime-objects).)
3. A silently swallowed exception during bootstrap is *original behavior* (the VEH + SEH
   double layer, [Quirk #1](../methodology/quirks.md#1-double-exception-protection-pattern-veh--seh))
   — attach a debugger and break on `0xe06d7363` to see what is really failing.

### Video preview is blank / renders in software

The engine falls back to **WARP** when no usable hardware D3D11 device exists, and the
original defaulted to a single-threaded device flag
([Quirk #9](../methodology/quirks.md#9-d3d11-single-threaded-flag-for-a-video-editor),
[Quirk #20](../methodology/quirks.md#20-warp-fallback-as-default-rendering-path)). Check
`d3d11` availability and the DX log output.

### The app tries to write `%LOCALAPPDATA%\Microsoft\WL\MovieMaker\auth.dat`

That is the DPAPI-encrypted PSA authentication store — expected, and safe: a single
`CryptProtectData` blob, current-user scope, magic/version-gated, size-bounded. See
[Security](../reference/security.md).

## Documentation site issues

### Building these docs

```powershell
cd website
npm install
npm run build      # static site → website/build
npm run serve      # local preview of the production build
npm start          # hot-reloading dev server
```

Requires Node.js ≥ 18. See `website/README.md`.
