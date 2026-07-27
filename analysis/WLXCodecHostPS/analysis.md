# WLXCodecHostPS.dll — COM Proxy/Stub Analysis

## Overview

**DLL**: `WLXCodecHostPS.dll` — MIDL-generated COM proxy/stub DLL for the `ICodecHost` interface, part of Microsoft Photo Gallery (Windows Live Photo Gallery).

| Property | Value |
|---|---|
| **Architecture** | x86 (PE32) |
| **Linker** | MSVC 11.0 (Visual Studio 2012) |
| **Timestamp** | Tue Apr 1 01:28:27 2014 (0x533A406B) |
| **Image Base** | 0x10000000 |
| **Image Size** | 0x8000 |
| **Subsystem** | Windows GUI (2) |
| **DLL Characteristics** | Dynamic base, NX compatible |
| **PDB GUID** | `{2B17A5B2-998C-4959-A615-53BB3DFC7401}` |
| **PDB Name** | `WLXCodecHostPS.pdb` |

## PE Section Layout

| Section | VA Range | File Offset | Size | Purpose |
|---|---|---|---|---|
| `.text` | `0x10001000–0x10003174` | `0x400–0x25FF` | 0x2200 | Code, IAT, INR, NDR format strings |
| `.orpc` | `0x10004000–0x10004040` | `0x2600–0x27FF` | 0x200 | ORPC proxy helper function |
| `.data` | `0x10005000–0x1000538F` | `0x2800–0x29FF` | 0x200 | CLSID map, PS factory data |
| `.rsrc` | `0x10006000–0x10006407` | `0x2A00–0x2FFF` | 0x600 | VS_VERSION_INFO resource |
| `.reloc` | `0x10007000–0x10007451` | `0x3000–0x35FF` | 0x600 | Base relocations |

## Exports (5)

Standard COM proxy/stub DLL entry points:

| Ordinal | Name | RVA | Purpose |
|---|---|---|---|
| 1 | `DllCanUnloadNow` | `0x1F01` | Checks if PS can be unloaded (ref counting) |
| 2 | `DllGetClassObject` | `0x1EC5` | Returns class factory for PS CLSID |
| 3 | `DllRegisterServer` | `0x1F55` | Self-registration via `NdrDllRegisterProxy` |
| 4 | `DllUnregisterServer` | `0x1F81` | Self-unregistration via `NdrDllUnregisterProxy` |
| 5 | `GetProxyDllInfo` | `0x1E95` | Returns proxy DLL info to RPC runtime |

## Imports

### RPCRT4.dll (19 imports — core proxy/stub infrastructure)

**CStdStubBuffer vtable** (standard `IRpcStubBuffer` implementation):

| IAT VA | Ordinal | Function |
|---|---|---|
| `0x10001094` | 0 | `CStdStubBuffer_AddRef` |
| `0x10001098` | 1 | `CStdStubBuffer_Connect` |
| `0x1000109C` | 2 | `CStdStubBuffer_CountRefs` |
| `0x100010A0` | 3 | `CStdStubBuffer_DebugServerQueryInterface` |
| `0x100010A4` | 4 | `CStdStubBuffer_DebugServerRelease` |
| `0x100010A8` | 5 | `CStdStubBuffer_Disconnect` |
| `0x100010AC` | 7 | `CStdStubBuffer_IsIIDSupported` |
| `0x100010B0` | 6 | `CStdStubBuffer_Invoke` |
| `0x100010B4` | 8 | `CStdStubBuffer_QueryInterface` |

**IUnknown proxy helpers**:

| IAT VA | Ordinal | Function |
|---|---|---|
| `0x100010B8` | 0xF | `IUnknown_AddRef_Proxy` |
| `0x100010BC` | 0x10 | `IUnknown_QueryInterface_Proxy` |
| `0x100010C0` | 0x11 | `IUnknown_Release_Proxy` |

**NDR runtime**:

| IAT VA | Ordinal | Function |
|---|---|---|
| `0x100010C4` | 0x96 | `NdrCStdStubBuffer_Release` |
| `0x100010C8` | 0xCA | `NdrDllCanUnloadNow` |
| `0x100010CC` | 0xCB | `NdrDllGetClassObject` |
| `0x100010D0` | 0xCC | `NdrDllRegisterProxy` |
| `0x100010D4` | 0xCD | `NdrDllUnregisterProxy` |
| `0x100010D8` | 0x104 | `NdrOleAllocate` |
| `0x100010DC` | 0x105 | `NdrOleFree` |

### ole32.dll — HBITMAP marshaling

| IAT VA | Function |
|---|---|
| `0x100010E4` | `HBITMAP_UserSize` |
| `0x100010E8` | `HBITMAP_UserMarshal` |
| `0x100010EC` | `HBITMAP_UserFree` |
| `0x100010F0` | `HBITMAP_UserUnmarshal` |

Indicates `ICodecHost` has `HBITMAP` parameters requiring cross-apartment/user-mode marshaling.

### OLEAUT32.dll — Automation types (ordinal imports)

| Ordinal | Function | Purpose |
|---|---|---|
| 283 | `SysFreeString` | Free BSTR |
| 284 | `SysAllocString` | Allocate BSTR |
| 285 | `SysStringLen` | Query BSTR length |
| 286 | `VariantInit` | Initialize VARIANT |
| 291 | `SafeArrayCreate` | Create SAFEARRAY |
| 292 | `SafeArrayDestroy` | Destroy SAFEARRAY |
| 293 | `SafeArrayAccessData` | Lock SAFEARRAY data |
| 294 | `SafeArrayUnaccessData` | Unlock SAFEARRAY data |

Indicates `ICodecHost` is automation-compatible with `BSTR`, `VARIANT`, and `SAFEARRAY` parameters.

### MSVCR110.dll — CRT runtime (17 imports)

Standard MSVC 2012 CRT initialization: `_initterm`, `_initterm_e`, `_malloc_crt`, `_calloc_crt`, SEH support (`_except_handler4_common`, `__CppXcptFilter`), and C++ cleanup (`__clean_type_info_names_internal`).

### KERNEL32.dll (9 imports)

Process/thread: `DisableThreadLibraryCalls`, `GetCurrentThreadId`. Security: `IsDebuggerPresent`, `IsProcessorFeaturePresent`. Timing: `GetTickCount64`, `GetSystemTimeAsFileTime`, `QueryPerformanceCounter`. Pointer obfuscation: `EncodePointer`, `DecodePointer`.

## COM Interface: ICodecHost

### Identifiers

| Type | GUID |
|---|---|
| **IID** | `{BB40E64E-19B1-44BF-6811-001000000000}` |
| **PS CLSID** | `{AC46E1E4-9981-49E8-A5E1-90A07142EDC3}` |

### .data Section Structure (PSFactoryBuffer data)

```
VA 0x10005000: FF FF FF FF                    ; Flags (0xFFFFFFFF = proxy/stub marker)
VA 0x10005004: BB 40 E6 4E B1 19 BF 44 ...  ; IID_ICodecHost (16 bytes)
VA 0x10005014: 00 00 00 00                    ; Reserved/padding
VA 0x10005018: EC 1C 00 10                    ; → pProxyFileInfo (0x10001CEC in .text)
VA 0x1000501C: 58 11 00 10                    ; → pClsid (0x10001158, PS CLSID)
VA 0x10005020: BE 29 00 10                    ; → pStublessClsid (0x100029BE)
VA 0x10005024: 12 2A 00 10                    ; → pVtblClsid (0x10002A12)
VA 0x10005028: EE 29 00 10                    ; → pReserved (0x100029EE)
VA 0x1000502C: FF FF FF FF FF FF FF FF        ; End-of-chain sentinel
```

### ProxyFileInfo Structure (at VA 0x10001CEC)

```
VA 0x10001CEC: 90 11 00 10                    ; ppClsid → IID array (0x10001190)
VA 0x10001CF0: EA 1B 00 10                    ; pNext (0x10001BEA, likely CLSID mapping)
VA 0x10001CF4: 76 1D 00 10                    ; pStublessIndex
```

### Global Proxy Table (at VA 0x10001D04)

Referenced by `.orpc` helper function for proxy initialization:

```
VA 0x10001D04: 04 50 00 10  → 0x10005014 (→ .data section IID/CLSID map)
VA 0x10001D08: 00 00 00 00  → NULL
VA 0x10001D0C: 84 1D 00 10  → 0x10001D84 ("ICodecHost" interface name)
VA 0x10001D14: 58 11 00 10  → 0x10001158 (PS CLSID)
VA 0x10001D18: E8 11 00 10  → 0x100011E8 (NDR type info)
VA 0x10001D1C: 07 00 00 00  → 7 methods in ICodecHost vtable
VA 0x10001D24: 06 2A 00 10  → Proxy thunk
VA 0x10001D28: B2 29 00 10  → Proxy thunk
VA 0x10001D2C: 12 1F 00 10  → NDR format string ref
VA 0x10001D30: 4E 2A 00 10  → CStdStubBuffer thunk
VA 0x10001D34: 2A 2A 00 10  → CStdStubBuffer thunk
VA 0x10001D38: 42 2A 00 10  → CStdStubBuffer thunk
VA 0x10001D3C: 36 2A 00 10  → CStdStubBuffer thunk
VA 0x10001D40: E2 29 00 10  → Proxy thunk
```

**Key insight**: The value `0x07` at `VA 0x10001D1C` indicates **7 methods** in the `ICodecHost` vtable (including `IUnknown::QueryInterface`, `IUnknown::AddRef`, `IUnknown::Release`).

## .orpc Section — ORPC Helper

The `.orpc` section (VA 0x10004000–0x10004040) contains a single helper function:

```
10004004: 8B FF          mov  edi, edi       ; hotpatch preamble
10004006: 55             push ebp
10004007: 8B EC          mov  ebp, esp
10004009: 51             push ecx            ; 1 local variable
1000400A: 6A 10          push 0x10
1000400C: 6A 04          push 4
1000400E: 58             pop  eax            ; eax = 4
1000400F: 6B C0 00       imul eax, eax, 0   ; eax = 0 (array index)
10004012: 8B 80 04 1D 00 10  mov eax, [eax + 0x10001D04]  ; load global table ptr
10004018: FF 70 04       push [eax + 4]     ; push table entry[1]
1000401B: FF 75 08       push [ebp + 8]     ; push first arg (REFCLSID)
1000401E: E8 36 EA FF FF call 0x10002A5A    ; → IAT thunk (CRT helper)
10004023: 83 C4 0C       add  esp, 0x0C     ; clean 3 args
10004026: 89 45 FC       mov  [ebp - 4], eax ; store result
1000402A: 83 7D FC 00    cmp  dword [ebp - 4], 0
1000402E: 75 0B          jne  0x1000403B
10004030: 8B 45 0C       mov  eax, [ebp + 0xC]  ; output param
10004033: 83 20 00       and  dword [eax], 0    ; *output = NULL
10004036: 33 C0          xor  eax, eax
10004038: 40             inc  eax               ; return 1 (success)
10004039: EB 02          jmp  0x1000403D
1000403B: 33 C0          xor  eax, eax          ; return 0 (failure)
1000403D: C9             leave
1000403E: C2 08 00       ret  8                 ; stdcall, 2 params
```

This is the `GetProxyDllInfo` implementation. It queries the global proxy table at `0x10001D04` and returns proxy/CLSID information via the output parameter.

## NDR Format Strings

### Interface Name String

```
VA 0x10001D84: "ICodecHost" (11 bytes, ASCII)
```

### Procedure Format Descriptors (VA 0x1000128C)

The NDR procedure format area defines marshaling for 7 interface methods:

```
02 80 11 00 00 00  ; Method 0: IUnknown::QueryInterface  (handle = 0x11)
02 80 02 00 00 00  ; Method 1: IUnknown::AddRef          (handle = 0x02)
06 80 12 00 00 00  ; Method 2: IUnknown::Release         (handle = 0x12)
06 80 03 00 00 00  ; Method 3: (custom, handle = 0x03)
08 80 13 00 00 00  ; Method 4: (custom, handle = 0x13)
08 80 16 00 00 00  ; Method 5: (custom, handle = 0x16)
08 80 17 00 00 00  ; Method 6: (custom, handle = 0x17)
08 80 0E 00 00 00  ; Method 7: (custom, handle = 0x0E)
```

Format flag byte breakdown:
- `0x02` = `FC_IN` (input-only parameter)
- `0x06` = `FC_IN | FC_OUT` (bidirectional)
- `0x08` = `FC_IN | FC_OUT | FC_RETURN` (complex return)

### Type Format Area (VA 0x100012C2)

Contains complex type definitions for marshaled parameters:

```
02 02 14 00 00 00  ; Simple type ref (ULONG?)
FC 01 04 00 00 00  ; FC_BOGUS (complex embedded structure)
0A 80 05 00 00 00  ; Conformance descriptor
0C 80 0B 00 00 00  ; Array/safearray descriptor
06 80 FF FF 00 00  ; Pointer (FC_FF)
06 80 0A 00 00 00  ; Pointer descriptor
08 80 06 00 00 00  ; Complex pointer type
```

These descriptors encode marshaling rules for the `BSTR`, `VARIANT`, `SAFEARRAY`, and `HBITMAP` parameters identified in the import analysis.

## RTTI / Debug Information

No C++ RTTI present (binary is C-style MIDL-generated code). CodeView debug info present:

```
Signature:  RSDS
PDB GUID:   {2B17A5B2-998C-4959-A615-53BB3DFC7401}
Age:        1
PDB Name:   WLXCodecHostPS.pdb
```

## Version Resource

| Field | Value |
|---|---|
| CompanyName | Microsoft Corporation |
| FileDescription | Photo Gallery Codec Host Proxy |
| FileVersion | 16.4.3528.0331_ship.client.main.w5m4 (ship) |
| InternalName | WLXCodecHostPS |
| LegalCopyright | 2012 Microsoft Corporation. All rights reserved. |
| OriginalFilename | WLXCodecHostPS.dll |
| ProductName | Photo Gallery |
| ProductVersion | 16.4.3528 |
| Translation | Lang=0x0409 (US English), CharSet=0x04B0 (Unicode) |

## Notable Strings

| VA | String |
|---|---|
| `0x10001D84` | `ICodecHost` — COM interface name |
| `0x1000310A` | `WLXCodecHostPS.dll` — module name |
| `0x1000311D` | `DllCanUnloadNow` |
| `0x1000312D` | `DllGetClassObject` |
| `0x10003140` | `DllRegisterServer` |
| `0x10003151` | `DllUnregisterServer` |
| `0x10003165` | `GetProxyDllInfo` |
| `0x10002036` | `__CppXcptFilter` |
| `0x10002084` | `MSVCR110.dll` |
| `0x10002218` | `KERNEL32.dll` |
| `0x10002490` | `RPCRT4.dll` |
| `0x10001767` | `%d` — printf format (debug output) |

## IAT Thunk Table (RPCRT4 → proxy vtable)

The `.text` section contains thunks that bridge the CStdStubBuffer vtable to the NDR runtime:

```
10002A18: jmp [0x100010A8]  → CStdStubBuffer_Disconnect
10002A1E: jmp [0x100010A4]  → CStdStubBuffer_DebugServerRelease
10002A24: jmp [0x1000109C]  → CStdStubBuffer_CountRefs
10002A2A: jmp [0x100010B4]  → CStdStubBuffer_QueryInterface
10002A36: jmp [0x100010A0]  → CStdStubBuffer_DebugServerQueryInterface
10002A42: jmp [0x10001094]  → CStdStubBuffer_AddRef
10002A4E: jmp [0x10001098]  → CStdStubBuffer_Connect
10002A54: jmp [0x10001098]  → CStdStubBuffer_Connect
```

## Architecture Summary

```
┌──────────────────────────────────────────────────────┐
│                    RPC Runtime                        │
│  (NdrDllGetClassObject, NdrDllRegisterProxy, etc.)   │
└──────────────┬───────────────────────────┬───────────┘
               │                           │
    ┌──────────▼──────────┐    ┌───────────▼──────────┐
    │   DllGetClassObject │    │  CStdStubBuffer      │
    │   DllRegisterServer │    │  _Connect/_Invoke    │
    │   DllCanUnloadNow   │    │  _IsIIDSupported     │
    │   GetProxyDllInfo   │    │  _AddRef/_Release    │
    └──────────┬──────────┘    └───────────┬──────────┘
               │                           │
    ┌──────────▼───────────────────────────▼──────────┐
    │              NDR Marshaling Engine               │
    │  Format strings at VA 0x1000128C                │
    │  Type info at VA 0x10001D84 ("ICodecHost")     │
    │  Handles: HBITMAP, BSTR, VARIANT, SAFEARRAY    │
    └──────────────────────┬──────────────────────────┘
                           │
    ┌──────────────────────▼──────────────────────────┐
    │              ICodecHost Interface                │
    │  IID: {BB40E64E-19B1-44BF-6811-001000000000}  │
    │  PS CLSID: {AC46E1E4-9981-49E8-A5E1-90A0..}   │
    │  Methods: 7 (3 IUnknown + 4 custom)             │
    │  Parameters: image/codec related                │
    └─────────────────────────────────────────────────┘
```

## Git

Commit hash: (pending commit)
