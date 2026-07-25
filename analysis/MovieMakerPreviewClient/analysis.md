# MovieMakerPreviewClient.dll Analysis

## Overview

**MovieMakerPreviewClient.dll** is the COM proxy/stub DLL for Windows Live Movie Maker 2012's timeline/storyboard preview system. It is a lightweight marshaling DLL (~28 KB image, ~5.9 KB code) that enables cross-thread/cross-process COM communication for the `IPreviewClientStatusCallback` interface.

| Field | Value |
|-------|-------|
| **Image** | 28,672 bytes (0x7000) |
| **Code (.text)** | 5,892 bytes (0x1704) |
| **Linker** | MSVC 11.00 (VS 2012) |
| **Image Base** | 0x10000000 |
| **Subsystem** | Windows GUI (2) |
| **OS Ver** | 6.02 (Windows 8) |
| **ASLR/DEP** | Dynamic Base + NX Compatible |
| **Timestamp** | 2014-04-01 01:17:17 (exports), 2014-04-01 01:28:27 (PE header) |
| **PDB** | `MovieMakerPreviewClient.pdb` GUID `{ABE6F6C4-02E6-47EA-B7FF-6A44A32264DD}` age 1 |
| **Build** | `16.4.3528.0331_ship.client.main.w5m4 (ship)` |
| **Description** | "Photo Gallery Preview Client" |

## PE Structure

### Sections

| Section | VA | VirtSize | RawSize | Characteristics | Purpose |
|---------|-------|----------|---------|-----------------|---------|
| `.text` | 0x10001000 | 0x1704 | 0x1800 | Code, Execute, Read | Application code + CRT init |
| `.orpc` | 0x10003000 | 0x41 | 0x200 | Code, Execute, Read | OLE proxy/stub (MIDL-generated) |
| `.data` | 0x10004000 | 0x3B8 | 0x200 | InitData, Read, Write | Globals, vtables, IID table |
| `.rsrc` | 0x10005000 | 0x420 | 0x600 | InitData, Read Only | VERSION_INFO resource |
| `.reloc` | 0x10006000 | 0x466 | 0x600 | InitData, Discardable, Read Only | Base relocations for ASLR |

### Notable PE Details
- **No TLS directory**, **No bound imports**, **No delay-load imports**
- **No COM descriptor directory** — standard COM DLL, not a .NET assembly
- Entry point at RVA 0x17D8 (standard CRT `_DllMainCRTStartup` wrapper)
- `/GS` buffer security: 17 functions protected (from debug FEAT data)
- Certificate directory present at RVA 0x2C00 (3EC0 bytes) — Authenticode signature

## Exports (4 functions)

| Ordinal | Hint | RVA | Name |
|---------|------|------|------|
| 1 | 0 | 0x14E1 | `DllCanUnloadNow` |
| 2 | 1 | 0x14A5 | `DllGetClassObject` |
| 3 | 2 | 0x1535 | `DllRegisterServer` |
| 4 | 3 | 0x1561 | `DllUnregisterServer` |

All four are standard COM DLL self-registration exports. No application-specific named exports. The DLL acts as both a COM server and a proxy/stub marshaling DLL.

## COM Architecture

### Custom Interface: `IPreviewClientStatusCallback`

The single custom interface found in the binary is `IPreviewClientStatusCallback`. This interface allows the preview client to report status changes back to the host (MovieMakerCore.dll or MovieMaker.exe).

The interface name string `IPreviewClientStatusCallback` is stored at RVA 0x137C.

Based on the proxy/stub dispatch table at RVA 0x135C (containing 6 function pointers), the interface has **6 methods** (3 IUnknown + 3 custom):

| RVA | Name |
|------|------|
| 0x10001F02 | `IPreviewClientStatusCallback_Proxy` stub (likely `QueryInterface`) |
| 0x10001F0E | `IPreviewClientStatusCallback_Proxy` stub (likely `AddRef`) |
| 0x10001F1A | `IPreviewClientStatusCallback_Proxy` stub (likely `Release`) |
| 0x10001F26 | Custom method 1 |
| 0x10001F3E | Custom method 2 |
| 0x10001F4A | Custom method 3 |

### COM GUIDs

**Interface IID (inferred):** `{DBFFDF24-FBB1-42D1-719A-EC305FBF765F}`
- Found at .data section offset 0x10001210
- This is the IID for `IPreviewClientStatusCallback`

### Class Object (Proxy/Stub)

The DLL uses MIDL-generated standard marshaling for the `IPreviewClientStatusCallback` interface. The proxy/stub infrastructure is visible via the RPCRT4.dll imports:

- **19 functions** from RPCRT4.dll: `CStdStubBuffer_*`, `IUnknown_*Proxy`, `NdrOleAllocate/Free`, `NdrDllCanUnloadNow/GetClassObject/RegisterProxy/UnregisterProxy`
- **4 functions** from ole32.dll: `HWND_UserMarshal/Unmarshal/Free/Size` — the interface uses `HWND` as a parameter type, requiring custom marshaling for the `HWND` type
- **4 ordinal imports** from OLEAUT32.dll (ordinals 283-286): These are `SysAllocString` (283), `SysFreeString` (284), `SysStringLen` (285), and `SysStringByteLen` (286) — indicating `BSTR` usage in the interface

### .orpc Section

The `.orpc` section (0x41 bytes at RVA 0x3000) contains the MIDL-generated **NdrProxyInitialize** stub function. This tiny function:
1. Sets up the stack frame
2. Calls `NdrProxyInitialize` from RPCRT4
3. Returns with `NdrProxySendReceive`

This is the standard pattern for MIDL-generated proxy DLLs.

## Imports (5 DLLs, ~59 functions)

### MSVCR110.dll (18 functions)
CRT core: `_calloc_crt`, `_malloc_crt`, `free`, `memcmp`, `_lock`/`_unlock`, `_initterm`/`_initterm_e`, C++ exception handling (`__CppXcptFilter`, `__crtUnhandledException`, `__crtTerminateProcess`), CRT init/exit (`__dllonexit`, `_onexit`, `_amsg_exit`), debug (`_crt_debugger_hook`, `__clean_type_info_names_internal`), operator delete (`??3@YAXPAX@Z`)

### KERNEL32.dll (9 functions)
`DisableThreadLibraryCalls`, `EncodePointer`, `DecodePointer`, `QueryPerformanceCounter` (for COM apartment checks), `GetCurrentThreadId`, `GetSystemTimeAsFileTime`, `GetTickCount64`, `IsDebuggerPresent`, `IsProcessorFeaturePresent`

### ole32.dll (4 functions)
`HWND_UserMarshal`, `HWND_UserUnmarshal`, `HWND_UserFree`, `HWND_UserSize` — custom HWND marshaling for COM interface

### OLEAUT32.dll (4 ordinal imports)
Ordinals 283-286 — `SysAllocString`, `SysFreeString`, `SysStringLen`, `SysStringByteLen` — BSTR support

### RPCRT4.dll (19 functions)
Full proxy/stub infrastructure: `CStdStubBuffer_AddRef/Connect/CountRefs/Disconnect/DebugServerQueryInterface/DebugServerRelease/Invoke/IsIIDSupported/QueryInterface`, `IUnknown_AddRef_Proxy/QueryInterface_Proxy/Release_Proxy`, `NdrCStdStubBuffer_Release`, `NdrDllCanUnloadNow/GetClassObject/RegisterProxy/UnregisterProxy`, `NdrOleAllocate/Free`

## Strings

### Version Info (from .rsrc)
- **CompanyName:** Microsoft Corporation
- **FileDescription:** Photo Gallery Preview Client
- **FileVersion:** 16.4.3528.0331_ship.client.main.w5m4 (ship)
- **InternalName:** MovieMakerPreviewClient
- **LegalCopyright:** © 2012 Microsoft Corporation. All rights reserved.
- **OriginalFilename:** MovieMakerPreviewClient.dll
- **ProductName:** Photo Gallery
- **ProductVersion:** 16.4.3528.0331

### Interface/Code Strings
- `IPreviewClientStatusCallback` — the sole custom COM interface
- `MovieMakerPreviewClient.pdb` — debug symbols (GUID: {ABE6F6C4-02E6-47EA-B7FF-6A44A32264DD})
- `MovieMakerPreviewClient.dll` — module name in export section

### Licensing Strings
- `$Windows Essential`
- `Legal_policy_statement`
- `Legal_Policy_Statement`

### Certificate Strings (Authenticode)
Standard Microsoft code signing certificate chain strings embedded in the certificate directory:
- Microsoft Corporation (Redmond, Washington)
- Microsoft Code Signing PCA
- Microsoft Time-Stamp PCA
- Microsoft Root Certificate Authority
- CRL/CTL URLs: `crl.microsoft.com`, `www.microsoft.com`
- MOPR identifiers, nCipher DSE ESN

## Function Categories

1. **COM Registration** (4 exports) — `DllCanUnloadNow`, `DllGetClassObject`, `DllRegisterServer`, `DllUnregisterServer`
2. **Proxy/Stub Dispatch** (19 RPCRT4 functions) — Standard marshaling for `IPreviewClientStatusCallback`
3. **Interface Custom Methods** (3 proxy stubs + 3 custom stubs) — `IPreviewClientStatusCallback` vtable
4. **HWND Marshaling** (4 ole32 functions) — Cross-apartment HWND passing
5. **BSTR Support** (4 OLEAUT32 ordinals) — String parameter marshaling
6. **CRT Initialization** (18 MSVCR110 functions) — Standard CRT bootstrap

## Architecture Summary

MovieMakerPreviewClient.dll is a **minimal COM proxy/stub DLL** generated by MIDL from an IDL file defining the `IPreviewClientStatusCallback` interface. Its role is:

1. **Client-side proxy**: When MovieMakerCore.dll (or another component) uses `IPreviewClientStatusCallback` across COM apartment boundaries, this DLL provides the transparent proxy that marshals method calls (including HWND and BSTR parameters) to the actual implementation.

2. **Server-side stub**: When the implementing object (likely in MovieMakerCore.dll or a UI component) receives calls, this DLL provides the stub that unmarshals parameters and invokes the real implementation.

3. **COM registration**: The DLL self-registers the proxy/stub CLSID so COM can locate it when marshaling the `IPreviewClientStatusCallback` interface.

### Typical Flow
```
Caller (e.g., MovieMakerCore)
  → IPreviewClientStatusCallback::Method()
    → Proxy stub (this DLL)
      → RPCRT4 marshaling
        → Stub (this DLL)
          → Real implementation
```

### Key Design Observations
- The interface uses `HWND` as a parameter (requires custom marshaling via ole32's `HWND_User*` functions)
- The interface uses `BSTR` strings (requires OLEAUT32 `Sys*` functions)
- 6 methods total (3 IUnknown + 3 custom status callback methods)
- The DLL is both a COM server (DllGetClassObject for the proxy CLSID) and a proxy/stub DLL (NdrDllGetClassObject for the interface)
- No dependencies on WLXPhotoBase.dll or other Windows Live DLLs — fully self-contained
- Extremely small code footprint (~5.9 KB): almost entirely MIDL-generated boilerplate
