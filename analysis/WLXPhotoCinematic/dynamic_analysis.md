# WLXPhotoCinematic.dll - Dynamic Analysis

## Test Harness
- `tests/WLXPhotoCinematic/test_cinematic.cpp` - compiled with MSVC 2022 x86
- Loads DLL via `LoadLibraryW`, calls `DllGetClassObject` directly, probes COM interfaces

---

## Key Findings

### 1. Real CLSIDs (Object Map) vs. RGS Script CLSIDs

**The RGS scripts in the resource section do NOT match the actual object map CLSIDs.**

| Purpose | CLSID | Source |
|---------|-------|--------|
| RGS CinematicFullScreen | `{B1CACF91-6F51-4533-BB28-B22D4E8A9C65}` | .rsrc RGS script |
| RGS CinematicTransform | `{5409AB48-D8D3-40E6-A1EB-23489DC422DE}` | .rsrc RGS script |
| **Actual Entry 0** | `{557B4CD8-C1EA-4A46-84EE-BA1AF9AA67D4}` | .text @ VA 0x10001D74 |
| **Actual Entry 1** | `{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}` | .text @ VA 0x100018EC |

### 2. COM Object Instantiation

Both objects instantiate successfully via `DllGetClassObject`:

```
Entry 0: S_OK -> IClassFactory -> CreateInstance -> S_OK (object at 0x011ABFE8)
Entry 1: S_OK -> IClassFactory -> CreateInstance -> S_OK (object at 0x011CB948)
```

### 3. Object Map Layout (ATL _ATL_OBJMAP_ENTRY32)

Static array at VA `0x10010348` (.data), set into `[10010FB8]` during DllMain.

```
struct _ATL_OBJMAP_ENTRY32 {  // 0x24 bytes per entry
    DWORD pclsid;             // +0x00: ptr to CLSID in .text
    DWORD pfnCreateInstance;  // +0x04: creator function
    DWORD m_pCF;              // +0x08: cached class factory (non-zero = initialized)
    DWORD dwRegister;         // +0x0C: registration cookie
    DWORD reserved1;          // +0x10
    DWORD reserved2;          // +0x14
    DWORD pfnGetClassObject;  // +0x18
    DWORD pfnGetClassObject2; // +0x1C
    DWORD pfnInit;            // +0x20: called during DllRegisterServer
};
```

### 4. IID Table Format (QI Dispatch)

Generic QI handler at `0x100065D4` uses 12-byte entries:

```
struct IID_ENTRY {     // 0x0C bytes
    DWORD pIID;        // +0x00: ptr to IID in .text (NULL = end sentinel)
    DWORD vtblOffset;  // +0x04: offset from 'this' pointer to sub-interface
    DWORD pfnHandler;  // +0x08: creator/dispatch function (1 = IUnknown-style direct)
};
```

### 5. Interface Table Results

#### Entry 0 (`{557B4CD8-C1EA-4A46-84EE-BA1AF9AA67D4}`)
IID table at VA `0x10001D3C`:

| # | IID | vtblOffset | Handler | Description |
|---|-----|-----------|---------|-------------|
| 0 | `{70E8E77F-8721-46B6-B746-335E-BF85-7704}` | 0x00 | IUnknown-style | IUnknown identity |

- Vtable at VA `0x1000164C`, 13 function slots (0-12)
- **IUnknown only** - no domain-specific interfaces exposed
- This is the **CinematicFullScreen** object

#### Entry 1 (`{0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}`)
IID table at VA `0x100018A8`:

| # | IID | vtblOffset | Handler | Description |
|---|-----|-----------|---------|-------------|
| 0 | `{C2DB9835-1146-4B10-A7BB-7361-9125B026}` | 0x00 | IUnknown-style | IUnknown identity |
| 1 | `{6D5140C1-7436-11CE-8034-00AA006009FA}` | 0x04 | IUnknown-style | Sub-interface |

- Main vtable at VA `0x10001624`, 23 function slots (0-22)
- Sub-interface vtable at VA `0x10001610` at `this+4`
- Sub-interface returned via QI at `this+4` = different pointer from base IUnknown
- This is the **CinematicTransform / PanZoomTransform** object

### 6. Object Construction Details

Entry 1 (PanZoomTransform) constructor at `0x1000373F`:
- Initializes object starting at `[esi]`
- Sets vtable to `0x10001624` (base) and `0x10001610` (sub-interface)
- Calls `[10010ED8]->vtable[1]` for global state initialization
- Object layout at offset +0x20 has a 0x19-byte embedded structure with a lock byte

Entry 0 constructor at `0x100037B6`:
- Similar but simpler, only one vtable (`0x10001624`)
- Sets `[esi+8]` to `0xC0000001` (status code?)
- Calls `[10010ED8]->vtable[2]` then `0x10008C45`

### 7. DllMain Initialization Chain

```
DllMain (0x1000BE24)
  -> DLL_PROCESS_ATTACH check
  -> 0x1000C576: security cookie init (RDTSC-based)
  -> 0x1000BE4C: SEH-protected main init
    -> 0x1000BC2A: thread pool / lock manager (uses lock cmpxchg at 0x10011028)
    -> 0x100031A4: ATL module init
      -> 0x1000411D: Set object map pointer [10010FB8] = 0x10010348
        -> Call pfnInit (0x10004574) for each entry in object map
      -> 0x10003128: Register entries (calls [10001024] per entry)
```

### 8. DllGetClassObject Code Flow

```
DllGetClassObject (0x1000327A -> 0x10003F38)
  1. Read [10010FB8] (object map pointer)
  2. If NULL -> fallback to 0x10003E51 (returns CO_E_NOT_INITIALIZED or searches CLSID array)
  3. If non-NULL -> iterate 0x24-byte entries:
     a. Check entry.pclsid != NULL (end sentinel)
     b. Check entry.m_pCF != 0 (skip uninitialized entries)
     c. Compare requested CLSID with entry.pclsid using 0x100065AA
     d. If match: create CComObject<CCinematic*> via entry.pfnCreateInstance
  4. If no match: return CLASS_E_CLASSNOTAVAILABLE (0x80040111)
```

**Key insight**: Step 3b means DllGetClassObject only returns class factories for entries that have already been initialized (`m_pCF != 0`). Since both entries have `m_pCF = 0x100032C1` (CComClassFactory vtable address) at load time, they pass this check.

### 9. DllCanUnloadNow

```asm
1000328A: xor eax,eax
1000328C: cmp [10010F94], eax  ; check ATL lock count
10003292: setne al              ; return TRUE if count != 0
```

### 10. DllRegisterServer / DllUnregisterServer

- `DllRegisterServer` (0x32AA -> 0x39CD): Returns `E_ACCESSDENIED` without admin rights
- `DllUnregisterServer` (0x329B -> 0x3DB8): Returns `S_OK` (deletes registry keys)
- Both iterate the same object map for registration/unregistration

### 11. Standard IUnknown QI Bypass

The generic QI handler at `0x100065D4` has a fast path for standard IUnknown:
```asm
; Check: [edi+0..3] == 0, [edi+4..7] == 0, [edi+8] == 0xC0, [edi+C] == 0x46000000
; If so, return this + table[0].vtblOffset without searching the table
```
This explains why both objects respond to IUnknown even though their real IID tables have different identity IIDs.

### 12. Thread Safety

The DLL uses a global lock at VA `0x10011028` with `lock cmpxchg` (CAS-based synchronization) during DllMain. The lock state is tracked at `0x1001102C` (0 = unlocked, 2 = locked).

---

## Vtable Summary

### Entry 0 (CinematicFullScreen) Vtable @ VA 0x1000164C

| Slot | RVA | Function |
|------|-----|----------|
| 00 | 0x371E | QueryInterface (IID table at 0x10001D3C) |
| 01 | 0x36CF | AddRef |
| 02 | 0x36EA | Release |
| 03 | 0x6C35 | Unknown method |
| 04 | 0x6CB3 | Unknown method |
| 05 | 0x6CF7 | Unknown method |
| 06 | 0x6D44 | Unknown method |
| 07 | 0x6D79 | Unknown method |
| 08 | 0x6D9F | Unknown method |
| 09 | 0x6EA6 | Unknown method |
| 10 | 0x6E85 | Unknown method |
| 11 | 0x6F02 | Unknown method |
| 12 | 0x386C | Release (destructor variant) |

### Entry 1 (CinematicTransform) Vtable @ VA 0x10001624

| Slot | RVA | Function |
|------|-----|----------|
| 00 | 0x384B | QueryInterface (IID table at 0x100018A8) |
| 01 | 0x37FC | AddRef |
| 02 | 0x3817 | Release |
| 03 | 0x8C9E | Unknown method |
| 04 | 0x8CF2 | Unknown method |
| 05 | 0x9CB6 | Unknown method |
| 06 | 0x9BA8 | Unknown method |
| 07 | 0x99DB | Unknown method |
| 08 | 0x38B9 | Unknown method |
| 09 | 0x2A54 | Unknown method |
| 10 | 0x371E | QueryInterface (reused from Entry 0) |
| 11 | 0x36CF | AddRef (reused) |
| 12 | 0x36EA | Release (reused) |
| 13-21 | 0x6C35-0x6F02 | Same methods as Entry 0 slots 03-11 |
| 22 | 0x386C | Release (destructor variant) |

### Entry 1 Sub-interface Vtable @ VA 0x10001610

| Slot | RVA | Function |
|------|-----|----------|
| 00 | 0x38FE | Unknown method |
| 01 | 0x38EF | Unknown method |
| 02 | 0x38E0 | Unknown method |
| 03 | 0x33B6 | Unknown method |
| 04 | 0x2A40 | Unknown method |
| 05-19 | various | Cascade into main vtable (offset adjustment thunks) |
