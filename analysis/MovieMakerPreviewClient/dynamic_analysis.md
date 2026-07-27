# MovieMakerPreviewClient.dll — Dynamic Analysis

## Test Harness Results

**Test file**: `tests/MovieMakerPreviewClient/test_preview.cpp`
**Compiled**: MSVC 19.44.35228 (x86), `/EHsc /MD /Zi`
**Date**: 2026-07-27

### Export Resolution

| Export | RVA | Resolved Address | Status |
|--------|-----|-----------------|--------|
| `DllCanUnloadNow` | 0x14E1 | 0x719014E1 | OK |
| `DllGetClassObject` | 0x14A5 | 0x719014A5 | OK |
| `DllRegisterServer` | 0x1535 | 0x71901535 | OK |
| `DllUnregisterServer` | 0x1561 | 0x71901561 | OK |

All 4 exports resolved successfully. Offsets match PE headers exactly (image base 0x10000000 + base 0x71900000 = DLL loaded at 0x71900000).

### DllCanUnloadNow Behavior

```
DllCanUnloadNow() → S_OK (0x00000000)        // Before any usage
DllCanUnloadNow() → S_OK (0x00000000)        // After CoCreateInstance attempt
```

Both calls return `S_OK`, indicating:
- The DLL's reference count is zero (no COM objects active)
- The DLL is safe to unload at any point during this test
- This is expected: we never instantiated any objects, so the lock count stays at 0

### DllGetClassObject Behavior

```
DllGetClassObject(CLSID_NULL,  IID_IUnknown, ...) → 0x80040111  // CLASS_E_CLASSNOTEXPECTED
DllGetClassObject(proxy_IID,   IID_IUnknown, ...) → 0x80040111  // CLASS_E_CLASSNOTEXPECTED
```

**Interpretation**: This DLL does **not** register any COM class factories. `DllGetClassObject` is implemented but always returns `CLASS_E_CLASSNOTEXPECTED`. This is significant — despite having the standard 4-export COM DLL signature, this DLL is **purely a proxy/stub** and does not create COM objects.

The actual class factory lookup is handled by `NdrDllGetClassObject` (from RPCRT4.dll), which is used internally for proxy/stub registration. The exported `DllGetClassObject` is the standard CRT wrapper that delegates to the RPCRT4 proxy infrastructure.

### CoCreateInstance Results

```
CoCreateInstance(IID_IPreviewClientStatusCallback_as_CLSID,
                 NULL, CLSCTX_INPROC_SERVER, IID_IUnknown) → 0x80040154  // CLASS_E_CLASSNOTAVAILABLE
```

**Interpretation**: The interface IID `{DBFFDF24-FBB1-42D1-719A-EC305FBF765F}` is not registered as a CLSID. This is correct — it's an interface IID, not a class ID. The proxy/stub CLSID would need to be looked up differently (typically via the `HKCR\Interface\{DBFFDF24-...}\ProxyStubClsid32` registry key after running `DllRegisterServer`).

## COM Interface Architecture

### IPreviewClientStatusCallback

**IID**: `{DBFFDF24-FBB1-42D1-719A-EC305FBF765F}`

Based on the MIDL-generated proxy/stub dispatch table (6 function pointers at .data:0x1000135C):

| Vtable Index | Method (Inferred) | Stub RVA | Notes |
|-------------|-------------------|----------|-------|
| 0 | `QueryInterface` | 0x10001F02 | IUnknown standard |
| 1 | `AddRef` | 0x10001F0E | IUnknown standard |
| 2 | `Release` | 0x10001F1A | IUnknown standard |
| 3 | Custom method 1 | 0x10001F26 | Status callback method |
| 4 | Custom method 2 | 0x10001F3E | Status callback method |
| 5 | Custom method 3 | 0x10001F4A | Status callback method |

### Parameter Types (from import analysis)

- **HWND** — Requires `HWND_UserMarshal/Unmarshal/Free/Size` from ole32.dll (4 imports)
- **BSTR** — Requires `SysAllocString/SysFreeString/SysStringLen/SysStringByteLen` from OLEAUT32.dll (4 ordinal imports)

### Cross-Apartment Marshaling

The DLL imports 19 RPCRT4 functions for full COM proxy/stub infrastructure:
- `CStdStubBuffer_Invoke` — Main dispatch for incoming calls
- `CStdStubBuffer_Connect/Disconnect` — Channel binding
- `NdrDllRegisterProxy` — Self-registration of proxy/stub CLSID
- `IUnknown_*_Proxy` — Standard IUnknown marshaling

### .orpc Section

The `.orpc` section (RVA 0x3000, 0x41 bytes) contains the `NdrProxyInitialize` stub that:
1. Sets up stack frame
2. Reads vtable index from the NdrServerInfo structure at .data:0x10001220
3. Calls `NdrProxyInitialize` → `NdrProxySendReceive`

## Key Findings

1. **Pure proxy/stub DLL** — Despite having `DllGetClassObject`, it never creates objects. The class factory is only for COM infrastructure registration.

2. **Single interface** — Only `IPreviewClientStatusCallback` is marshaled. The DLL exists solely for cross-apartment communication of this one interface.

3. **HWND usage** — The interface takes HWND parameters (likely for preview window handles), requiring custom COM marshaling that can't use standard wire format.

4. **BSTR usage** — String parameters are passed as BSTRs, requiring OLEAUT32 marshaling.

5. **No runtime dependencies on other WMM DLLs** — Self-contained proxy/stub; imports only system DLLs (MSVCR110, KERNEL32, ole32, OLEAUT32, RPCRT4).

6. **Tiny code footprint** — 0x1704 bytes of code (5,892 bytes), almost entirely MIDL-generated boilerplate.

7. **PDB available** — `MovieMakerPreviewClient.pdb` GUID `{ABE6F6C4-02E6-47EA-B7FF-6A44A32264DD}` age 1, for source-level debugging if symbols are available.
