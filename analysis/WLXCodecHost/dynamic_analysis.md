# WLXCodecHost.exe — Dynamic & Deep Static Analysis

## 1. RTTI Class Inventory (16 types)

### Concrete Classes (AV — C++ vtable implementations)

| Class | Category | Purpose |
|-------|----------|---------|
| `CodecHost` | Codec Host | Main COM object implementing `ICodecHost` interface |
| `CodecHostModule` | ATL Module | `CAtlExeModuleT<CodecHostModule>` — EXE module managing COM registration and lifetime |
| `CAtlModule` | ATL Framework | Base ATL module |
| `CComClassFactory` | ATL COM | Standard ATL class factory for creating `CodecHost` instances |
| `CComObjectRootBase` | ATL COM | Base class for ATL COM object roots |
| `CRegObject` | ATL COM | ATL registry object for `.rgs` script processing |
| `GdiplusStartupWrapper` | GDI+ | RAII wrapper for `GdiplusStartup`/`GdiplusShutdown` lifetime |
| `Exception` | WLXPhotoBase | `Base::Exception` — structured exception wrapper |
| `type_info` | CRT/RTTI | C++ RTTI type_info metadata |

### Interface Types (AU — abstract vtable interfaces)

| Interface | Purpose |
|-----------|---------|
| `ICodecHost` | Custom interface — primary API exposed by the COM server for codec operations |
| `IRegistrarBase` | ATL registrar base interface |
| `IUnknown` | Standard COM IUnknown |
| `IClassFactory` | Standard COM class factory |

### ATL Template Instantiations (from mangled names)

```
CComObject<CodecHost>                      — COM wrapper for CodecHost
CComCoClass<CodecHost, &CLSID_CodecHost>   — CoClass association
CAtlExeModuleT<CodecHostModule>            — EXE module type
```

---

## 2. COM Activation Pattern

### Registration (LocalServer32)

```
HKCR\CLSID\{E30A45E6-1916-4659-95EE-035E62DB9AB0}
  ProgID = Microsoft.WLXCodecHost.CodecHost.1
  VersionIndependentProgID = Microsoft.WLXCodecHost.CodecHost
  LocalServer32 = %MODULE%
  AppID = {87A9DFB0-BA04-45F3-85EB-C33727ECEF22}
```

### Activation Flow

```
MovieMakerCore.dll
  → CoCreateInstance(CLSID_CodecHost, IID_ICodecHost, &pCodecHost)
  → Windows SCM launches WLXCodecHost.exe (LocalServer32)
  → CodecHostModule registers class object via CoRegisterClassObject
  → CoResumeClassObjects signals readiness
  → ICodecHost vtable calls cross process boundary via LPC/ALPC
```

### Lifetime Management

- `CoAddRefServerProcess()` — increment when new client connects
- `CoReleaseServerProcess()` — decrement when client disconnects
- When last client releases, server process exits naturally
- `SetPriorityClass` — manages process priority (likely IDLE or BELOW_NORMAL for sandbox)

---

## 3. Command-Line Analysis

### Expected Arguments

WLXCodecHost.exe uses a **GUI subsystem** with a message pump. It is activated exclusively via **COM LocalServer32** — no command-line arguments are expected or parsed. The `GetCommandLineW` import is standard CRT initialization, not application-level argument parsing.

### IPC Mechanism

- **Primary:** COM LPC/ALPC via LocalServer32 activation
- **Cross-thread:** `PostThreadMessageW` for intra-process thread communication
- **No named pipes, shared memory, or WM_COPYDATA** — pure COM remoting

---

## 4. GDI+ Codec Pipeline

### Initialization
1. `GdiplusStartup` — initialize GDI+ with token
2. `GdiplusShutdown` — cleanup on exit

### Image Decoding
- `CreateDIBSection` (GDI32) — create device-independent bitmap for decoded output
- `DeleteObject` — GDI object cleanup
- `SHCreateItemFromParsingName` (SHELL32) — parse file path to IShellItem for codec selection

### Property System Integration
- `PSCoerceToCanonicalValue` (PROPSYS) — coerce photo properties to canonical form
- `WLXPSGetItemPropertyHandler` (MetadataSys.dll) — get Windows Property System handler for photo item

### Property Names Found in Binary
- `System.Photo.Orientation` — image EXIF orientation

---

## 5. Registry Isolation

The binary imports a full set of registry operations (8 ADVAPI32 registry functions):
- `RegCreateKeyExW` / `RegOpenKeyExW` — create/open keys
- `RegSetValueExW` — set values
- `RegQueryInfoKeyW` / `RegEnumKeyExW` — enumerate subkeys
- `RegDeleteValueW` / `RegDeleteKeyW` — clean up

This registry access is for **per-user codec registration and sandbox configuration** — the codec host maintains its own registry state independent of the parent WLX process, preventing codec failures from corrupting the parent's settings.

---

## 6. Threading Model

- **STA (Single-Threaded Apartment):** Uses `CoInitialize` (not `CoInitializeEx`)
- **Message pump thread:** `GetMessageW`/`DispatchMessageW`/`TranslateMessage`
- **Worker thread:** `CreateThread` with `CreateEventW`/`SetEvent`/`WaitForSingleObject` synchronization
- **Critical sections:** `InitializeCriticalSectionAndSpinCount` for thread-safe COM object access
- **Cross-thread messaging:** `PostThreadMessageW` for posting work to the UI/message thread

---

## 7. Security & Sandboxing

### Fault Tolerance
- **SEH:** `__CxxFrameHandler3`, `_except_handler4_common`, `_XcptFilter`
- **Watson dumps:** `_invoke_watson`, `__crtSetUnhandledExceptionFilter`
- **Heap corruption detection:** `HeapSetInformation`
- **Process termination protection:** `__crtTerminateProcess`, `__crtUnhandledException`

### Process Hardening
- **ASLR:** Dynamic base (`0x400000` randomized)
- **DEP:** NX compatible
- **Stack canaries:** `/GS` compiler protection (46 buffer security checks noted in PDB feature flags)
- **Pointer obfuscation:** `EncodePointer`/`DecodePointer` for vtable pointer protection

### Debug Detection
- `IsDebuggerPresent` — anti-debugging check
- `IsProcessorFeaturePresent` — CPU feature detection for optimization

---

## 8. Cross-DLL Import Summary

| Source DLL | Imports Used |
|------------|-------------|
| `WLXPhotoBase.dll` | `Base::Throw`, `BasePrivate::New`, `BasePrivate::Delete`, `Base::Exception::~Exception`, `Base::OS::IsWin7OrGreater`, `ATL::BaseAtlThrow` |
| `MetadataSys.dll` | `WLXPSGetItemPropertyHandler` |
| `ole32.dll` | Full COM lifecycle (13 functions) |
| `OLEAUT32.dll` | BSTR/SysString operations (7 ordinals) |
| `gdiplus.dll` | `GdiplusStartup`, `GdiplusShutdown` |
| `GDI32.dll` | `CreateDIBSection`, `DeleteObject` |
| `SHELL32.dll` | `SHCreateItemFromParsingName` |
| `PROPSYS.dll` | `PSCoerceToCanonicalValue` |

---

## 9. Comparison: WLXCodecHost vs WLXTranscode

| Aspect | WLXCodecHost.exe | WLXTranscode.exe |
|--------|-----------------|-----------------|
| **Subsystem** | Windows GUI | Windows CUI (console) |
| **Activation** | COM LocalServer32 | Direct command-line |
| **Image Size** | 53 KB | 311 KB |
| **Code Size** | 32 KB | 272 KB |
| **RTTI Classes** | 9 concrete | 25+ concrete |
| **COM CLSID** | `{E30A45E6-...}` | None (no export) |
| **Primary API** | ICodecHost vtable | Command-line args |
| **Media Stack** | GDI+ | Media Foundation + DXVA2 |
| **GPU Access** | None | D3D9 + D3D11 + DXVA2 |
| **ETW Tracing** | No | Yes (full ETW) |
| **Crypto** | No | Yes (CryptSignHashW) |
| **Dependencies** | 11 DLLs | 13 DLLs |

---

## 10. Key Findings

1. **Pure COM server** — WLXCodecHost.exe is a classic ATL LocalServer32 with no command-line interface. It exists solely to sandbox codec execution via COM remoting.

2. **Minimal binary** — At only 53 KB (32 KB of code), this is an extremely lean COM server focused exclusively on GDI+ image decoding and metadata extraction.

3. **Crash isolation** — The primary purpose is crash isolation. If a GDI+ codec crashes, only the out-of-process host dies, not the main Photo Gallery/Movie Maker process.

4. **Registry sandboxing** — Maintains independent registry state via ADVAPI32, preventing codec corruption from affecting the parent application.

5. **No GPU access** — Unlike WLXTranscode, this binary has no D3D/DXVA dependencies, making it suitable for pure 2D image decoding.

6. **Windows 7+ targeting** — `OS::IsWin7OrGreater` check suggests conditional code paths for pre-Win7 vs Win7+ GDI+ capabilities.
