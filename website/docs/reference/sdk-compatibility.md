---
sidebar_position: 2
title: SDK Compatibility
description: Every fix applied to build VS2012-era code against the Windows 10 SDK (10.0.26100.0).
---

# SDK Compatibility

The original targeted the VS2012/Win8 SDK; the reconstruction builds against VS2022 /
Windows SDK 10.0.26100.0. These are the fixes that made that possible — reproduced from
`ROADMAP.md`. **Don't undo them**; the removed APIs will not come back.

## Replaced or stubbed APIs

| Original (VS2012/Win8 SDK) | Fix applied (VS2022/Win10 SDK) |
|---|---|
| `d3dx11.h` / D3DX11 effect API | Stub: `HMREngine/d3dx11compat.h/.cpp` (returns `E_NOTIMPL` for unused paths) |
| `d3dx9.h` / `d3dx9.lib` | Removed from all targets |
| `d3dcompiler_46.lib` | Renamed to `d3dcompiler.lib` |
| `dxva2.h` DXVA2 video-processor APIs | Stub: `HMRAVSource/dxva2stubs.cpp` + stub interface in `pch.h` |
| `mferror.lib` | Removed (MF error codes live in headers) |
| `MF_ENABLE_HARDWARE_TRANSFORMS` | Removed (not in Win10 SDK) |
| `INTERNET_OPTION_ENABLE_FEATURE` | Removed (not in Win10 SDK enum) |
| `uiribbon.lib` / `IID_IUICommandHandler` | GUID manually defined |
| `MF_OBJECT_UNKNOWN` | Defined as `((MF_OBJECT_TYPE)0)` |
| `XmlWriterProperty_ProcessNamespaces` | Defined as `((XmlWriterProperty)1)` |
| `IXmlReader::GetAttribute` | Replaced with `MoveToAttributeByName` + `GetValue` |
| `winmm.h` | Replaced with `mmsystem.h` |

## The 23 known gotchas

Lessons learned during the build-fix passes (full detail in `ROADMAP.md`):

1. `winmm.h` does NOT exist in SDK 10.0.26100.0 — use `mmsystem.h`
2. `IAtlStringMgr` is in `<atlstr.h>`, not `<atlbase.h>`
3. ATL 14+ defines `BaseAtlThrow` natively
4. `CAtlArray` copy constructor is private — use `.Copy()`
5. `CComObject<T>` does NOT expose T's members through `->`
6. D3DX11 is completely removed — must use the stub
7. `CComPtr<T>` requires `AddRef/Release` — not for non-COM types
8. Circular includes broken by extracting enums into a separate header
9. `RIBBON_API` macro was never defined — added to `exports.h`
10. `ATL::CStringMap` does NOT exist — use `std::map`
11. `IXmlReader::GetAttribute` does NOT exist — use `MoveToAttributeByName` + `GetValue`
12. MF enums removed from Win10 SDK
13. `IDXVA2VideoProcessor` interface removed
14. `DrawTextLayout` takes 3 or 4 args (NOT `D2D1_RECT_F` as 4th)
15. `_itow_s` requires 4 args (value, buf, bufSize, radix)
16. `WriteEndElement()` / `WriteEndDocument()` take 0 args
17. Circular include fixed by heap-allocating `ProjectTimeline`
18. Forward-declared structs must be moved INTO the namespace in headers
19. `DEFINE_STUB_TRANSITION` macro `L##stringId` — remove the `L##` prefix
20. `mferror.lib` does not exist in the Win10 SDK
21. `Gdiplus::Graphics::DrawImage` takes `Gdiplus::Image*`, not `const Bitmap*`
22. ATL `BEGIN_COM_MAP`/`COM_INTERFACE_ENTRY(IUnknown)` fails with MSVC 14.44 — use
    `COM_INTERFACE_ENTRY_IID(IID_IUnknown, ClassName)`
23. `CComQIPtr<IUnknown>` has a template specialization conflict in MSVC 14.44 — use
    `CComPtr<IUnknown>`

## Version macros

From `src/common.h` — reference these, never hardcode elsewhere:

```cpp
WMMR_VERSION_MAJOR    = 16
WMMR_VERSION_MINOR    = 4
WMMR_VERSION_BUILD    = 3528
WMMR_VERSION_REVISION = 331
WMMR_VERSION_STRING   = "16.4.3528.331"
```
