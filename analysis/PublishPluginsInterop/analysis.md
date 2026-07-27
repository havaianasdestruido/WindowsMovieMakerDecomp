# PublishPluginsInterop.dll & SubscribePluginsInterop.dll Analysis

## Overview

These are **mixed-mode C++/CLI interop assemblies** that bridge native C++ COM callers (specifically `WLXMediaPublishSubscribe.dll`) to managed .NET plugin implementations (`ManagedPluginWrapper.dll` / `ManagedSubscribePluginWrapper.dll`). They are **not** pure native DLLs nor pure .NET assemblies — they combine native x86 code with embedded .NET CLR metadata and managed types.

**Key distinction:** Unlike the pure .NET publish plugins (`WLFacebookPlugin.dll`, `WLFlickrPlugin.dll`, etc.) which have *only* `_CorDllMain` as a native import, these interop DLLs have substantial native import tables (MSVCR110, ole32, ADVAPI32, USER32, OLEAUT32, KERNEL32) AND CLR metadata. They are C++/CLI (managed extensions for C++) assemblies compiled with `/clr`.

---

## 1. PublishPluginsInterop.dll

### PE Headers

| Field | Value |
|-------|-------|
| **Machine** | x86 (0x14C) |
| **Linker** | MSVC 11.00 (VS 2012) |
| **Subsystem** | Windows CUI (3) |
| **Image Base** | 0x10000000 |
| **Image Size** | 0x1E000 (122,880 bytes) |
| **Entry Point** | 0x100191D6 |
| **Timestamp** | 2014-04-01 01:17:04 (533A3DC0) |
| **PDB** | `PublishPluginsInterop.pdb` `{E96C1C59-4E0C-4688-A58B-CC047AACFFBA}` |
| **CLR Runtime** | v4.0.30319 (.NET 4.0) |
| **CLR Flags** | Strong Name Signed + Native Entry Point |

### CLR Header

| Field | Value |
|-------|-------|
| **MetaData RVA** | 0x1E80 (size 0xD2A8 — 54 KB of .NET metadata) |
| **StrongNameSignature** | RVA 0x1E00 (0x80) |
| **VTableFixups** | RVA 0xF1D0 (0x68 — 13 entries) |
| **ManagedNativeHeader** | None (mixed-mode, not pure IL) |

### Sections

| Section | VA | VirtSize | RawSize | Purpose |
|---------|------|----------|---------|---------|
| .text | 0x1000 | 0x19181 | 0x19200 | Native + managed code (100 KB) |
| .data | 0x1B000 | 0xB28 | 0x600 | Data, vtables, gcroots |
| .rsrc | 0x1C000 | 0xD30 | 0xE00 | Resources (3.5 KB) |
| .reloc | 0x1D000 | 0xA72 | 0xC00 | Base relocations |

### Exports (4 — standard COM DLL server)

All exports are forwarded to `ManagedPluginWrapper.dll`:

| Ordinal | Name | Purpose |
|---------|------|---------|
| 1 | `DllCanUnloadNow` | COM: check if DLL can be unloaded |
| 2 | `DllGetClassObject` | COM: retrieve class factory |
| 3 | `DllRegisterServer` | COM: self-registration |
| 4 | `DllUnregisterServer` | COM: self-unregistration |

### Native Imports

| DLL | Key Functions | Purpose |
|-----|---------------|---------|
| **MSVCR110.dll** | 36 functions | CRT: memory, exceptions, C++ runtime |
| **ole32.dll** | 6 functions | `CoTaskMemAlloc/Realloc/Free`, `CoCreateInstance`, `StringFromGUID2`, `PropVariantClear` |
| **ADVAPI32.dll** | 8 functions | Registry operations (CRUD on keys/values) |
| **KERNEL32.dll** | 35 functions | Thread locale, module loading, resource loading, critical sections, PE metadata |
| **USER32.dll** | 2 functions | `CharNextW`, `MessageBoxW` |
| **OLEAUT32.dll** | 7 ordinals | VARIANT/OLE automation support |
| **SHLWAPI.dll** | 1 ordinal | Path/string helper |
| **mscoree.dll** | 1 function | `_CorDllMain` (.NET CLR bootstrap) |

### Assembly References

| Assembly | Version |
|----------|---------|
| mscorlib | 4.0.0.0 |
| Microsoft.VisualC | 10.0.0.0 |
| **Microsoft.WindowsLive.PublishPlugins** | **1.0.0.0** |
| System | 4.0.0.0 |
| System.Xml | 4.0.0.0 |
| System.Drawing | 4.0.0.0 |
| System.Windows.Forms | 4.0.0.0 / 2.0.0.0 |

### RTTI Classes (from string analysis)

| Mangled Name | Demangled | Role |
|--------------|-----------|------|
| `.?AVManagedPublishPlugin@@` | `ManagedPublishPlugin` | Managed wrapper (holds `gcroot<IPublishPlugin*>`) |
| `.?AVManagedPublishPluginImpl@@` | `ManagedPublishPluginImpl` | Implementation (calls into managed code) |
| `.?AVCreateManagedPublishPlugin@@` | `CreateManagedPublishPlugin` | COM coclass — the CLSID factory |
| `.?AUICreateManagedPlugin@@` | `ICreateManagedPlugin` | COM interface for factory |
| `.?AUPublishPluginComWrapper@@` | `PublishPluginComWrapper` | COM wrapper around managed plugin |
| `.?AVPublishPluginComWrapper@@` | `PublishPluginComWrapper` | VTable implementation |
| `.?AVManagedPluginWrapperModule@@` | `ManagedPluginWrapperModule` | ATL DLL module |

### Key Managed Types (from .NET reflection)

#### `ManagedPublishPlugin` (class, sealed)
Native C++/CLI wrapper that holds a `gcroot<Microsoft.WindowsLive.PublishPlugins::IPublishPlugin^>` to the managed plugin instance.

#### `ManagedPublishPluginImpl` (class, sealed)
The implementation class that bridges native COM calls to managed methods:
```
public .ctor()
public Void LoadPlugin(String pluginPath, String assemblyName)
public Boolean ShowConfigurationSettings(IWin32Window parentWindow, ...)
public Boolean PublishItem(IWin32Window parentWindow, String mediaObjectId, Stream stream, ...)
public Boolean HasPublishResults(XmlDocument sessionXml)
public Void LaunchPublishResults(XmlDocument sessionXml)
public Boolean HasSummaryInformation(XmlDocument sessionXml)
public Void ShowSummaryInformation(IWin32Window parentWindow, XmlDocument sessionXml)
```

#### `ManagedPublishProgressCallback` (class, sealed)
Implements `IPublishProgressCallback`. Wraps a native `IOnlineMediaProgressCallback*` pointer and a cancel event handle:
```
public .ctor(IOnlineMediaProgressCallback* pProgressCallback, UInt32 hCancelEvent)
public virtual Void SetPublishProgress(Int32 percentComplete)
public virtual Boolean Canceled()
```

#### `ManagedPublishItemPropertyStore` (class, sealed)
Implements `IPublishItemPropertyStore`. Wraps `IOnlineMediaItemPropertyStore*`:
```
public .ctor(IOnlineMediaItemPropertyStore* pItemPropertyStore)
public virtual Boolean GetValue(String key, T& result)
```

#### `ManagedPublishProperties` (class, sealed)
Implements `IPublishProperties`. Wraps `IOnlineMediaItemProperties*`:
```
public .ctor(IOnlineMediaItemProperties* pItemProperties)
public virtual IPublishItemPropertyStore GetItemProperties(String mediaObjectId)
```

#### `ReadOnlyComStream` (class, sealed)
Extends `System.IO.Stream`. Wraps a native `IStream*` for read-only access:
```
public .ctor(IStream* pStream)
public virtual Int32 Read(Byte[] bufferArray, Int32 nOffset, Int32 nCount)
public virtual Int64 Seek(Int64 nOffset, SeekOrigin eOrigin)
```

#### Helper classes
- `SmartSetRestoreThreadCultureInfo` — RAII culture info save/restore
- `XmlHelpers` — `XmlDocumentFromXmlDomDocument`, `UpdateXmlDomDocumentFromXmlDocument`
- `PropVariantManagedHelpers` — PROPVARIANT ↔ .NET type conversions (DateTime, Bitmap, String, arrays)
- `CultureHelper` — `GetBestCulture`
- Multiple `auto_handle<T>` — RAII smart handles for managed objects

### COM Registration

The DLL registers as a standard COM in-process server. Description string: *"Creates the managed plugin wrapper class."*

### Strong Name

Signed with key `35MSSharedLib1024.snk` (Microsoft shared library key), public key token `31bf3856ad364e35`.

---

## 2. SubscribePluginsInterop.dll

### PE Headers

| Field | Value |
|-------|-------|
| **Machine** | x86 (0x14C) |
| **Linker** | MSVC 11.00 (VS 2012) |
| **Subsystem** | Windows CUI (3) |
| **Image Base** | 0x10000000 |
| **Image Size** | 0x1E000 (122,880 bytes) |
| **Entry Point** | 0x10019226 |
| **Timestamp** | 2014-04-01 01:17:08 (533A3DC4) |
| **PDB** | `SubscribePluginsInterop.pdb` `{8D12B774-35A4-4901-B674-476BB6F22497}` |
| **CLR Runtime** | v4.0.30319 (.NET 4.0) |
| **CLR Flags** | Strong Name Signed + Native Entry Point |

### CLR Header

| Field | Value |
|-------|-------|
| **MetaData RVA** | 0x1EC0 (size 0xC5E8 — 50 KB of .NET metadata) |
| **StrongNameSignature** | RVA 0x1E40 (0x80) |
| **VTableFixups** | RVA 0xE554 (0xB8 — 23 entries) |

### Sections

| Section | VA | VirtSize | RawSize | Purpose |
|---------|------|----------|---------|---------|
| .text | 0x1000 | 0x1919A | 0x19200 | Native + managed code (100 KB) |
| .data | 0x1B000 | 0xC48 | 0x800 | Data, vtables, gcroots |
| .rsrc | 0x1C000 | 0xD40 | 0xE00 | Resources (3.5 KB) |
| .reloc | 0x1D000 | 0xB5E | 0xC00 | Base relocations |

### Exports (4 — standard COM DLL server)

All exports forwarded to `ManagedSubscribePluginWrapper.dll`:

| Ordinal | Name |
|---------|------|
| 1 | `DllCanUnloadNow` |
| 2 | `DllGetClassObject` |
| 3 | `DllRegisterServer` |
| 4 | `DllUnregisterServer` |

### Native Imports

Nearly identical to PublishPluginsInterop.dll. Same set of DLLs (MSVCR110, ole32, ADVAPI32, KERNEL32, USER32, OLEAUT32, mscoree.dll). Minor difference: no `PropVariantClear` import, no `SHLWAPI.dll`.

### Assembly References

Same as PublishPluginsInterop, except:
- References **Microsoft.WindowsLive.SubscribePlugins** v1.0.0.0 (instead of PublishPlugins)

### RTTI Classes

| Mangled Name | Demangled | Role |
|--------------|-----------|------|
| `.?AVManagedSubscribePlugin@@` | `ManagedSubscribePlugin` | Managed wrapper |
| `.?AVManagedSubscribePluginImpl@@` | `ManagedSubscribePluginImpl` | Implementation bridge |
| `.?AVCreateManagedSubscribePlugin@@` | `CreateManagedSubscribePlugin` | COM coclass factory |
| `.?AUISubscribePluginInternal@@` | `ISubscribePluginInternal` | Internal COM interface |
| `.?AVSubscribePluginComWrapper@@` | `SubscribePluginComWrapper` | COM wrapper |
| `.?AVManagedStream@@` | `ManagedStream` | Managed stream wrapper |
| `.?AVManagedStreamImpl@@` | `ManagedStreamImpl` | Stream implementation |
| `.?AVStreamComWrapper@@` | `StreamComWrapper` | Stream COM wrapper |
| `.?AVManagedSubscribePluginWrapperModule@@` | `ManagedSubscribePluginWrapperModule` | ATL DLL module |

### Key Managed Types

#### `ManagedSubscribePluginImpl` (class, sealed)
The implementation bridge for subscribe operations:
```
public .ctor()
public Void LoadPlugin(String pluginPath, String assemblyName)
public Boolean ShowConfigurationSettings(IWin32Window parentWindow, ...)
public XmlDocument GetChildItems(XmlDocument sessionXml, String itemId, Int32 offset, Int32 limit, Boolean useCache, ...)
public XmlDocument GetItemPropertiesById(XmlDocument sessionXml, String itemId, String itemType, String propertyName, Boolean useCache, ...)
public XmlDocument GetItemPropertiesByUri(XmlDocument sessionXml, String uri, String itemType, String propertyName, Boolean useCache, ...)
public Stream GetItemDataStream(XmlDocument sessionXml, String itemId, String dataStreamId, Boolean async, ...)
public Void AddComment(XmlDocument sessionXml, String itemId, String comment, ...)
public Void AddPersonRegion(XmlDocument sessionXml, String itemId, String name, String contactId, Double left, Double top, Double width, Double height, ...)
public XmlDocument GetCapabilities(XmlDocument sessionXml)
```

#### `ManagedSubscribeProgressCallback` (class, sealed)
Implements `ISubscribeProgressCallback`:
```
public .ctor(IOnlineMediaProgressCallback* pProgressCallback, UInt32 hCancelEvent)
public virtual Void SetSubscribeProgress(Int32 percentComplete)
public virtual Boolean Canceled()
```

#### `ManagedStream` / `ManagedStreamImpl`
Wraps managed `System.IO.Stream` as a COM `IStream`:
```
ManagedStream:
  public Void Init(Stream stream)
  public virtual Int32 Read(Byte[] bufferArray, Int32 nOffset, Int32 nCount)
  public virtual Int64 Seek(Int64 nOffset, SeekOrigin eOrigin)
  public virtual Void CopyTo(Stream destination, Int32 bufferSize)

ManagedStreamImpl:
  public .ctor(Stream stream)
  public virtual Int32 Read(...)
  public virtual Int64 Seek(...)
  public virtual Void CopyTo(...)
```

#### `AssemblyLoader` (Subscribe only)
Handles dynamic assembly resolution:
```
public .ctor()
public Assembly AssemblyResolveHandler(Object sender, ResolveEventArgs args)
```
Fields: `_publishAssembly`, `_subscribeAssembly` — cached references to the publish and subscribe plugin assemblies.

#### Other types
Same helper classes as PublishPluginsInterop: `SmartSetRestoreThreadCultureInfo`, `XmlHelpers`, `CultureHelper`, `auto_handle<T>` smart pointers.

---

## 3. Managed Plugin Interface Contracts

These are the .NET interfaces defined in the separate managed assemblies (`Microsoft.WindowsLive.PublishPlugins.dll` and `Microsoft.WindowsLive.SubscribePlugins.dll`), which the interop DLLs reference.

### IPublishPlugin (Microsoft.WindowsLive.PublishPlugins)

```
public interface IPublishPlugin
{
    Boolean ShowConfigurationSettings(IWin32Window parentWindow, XmlDocument sessionXml, XmlDocument persistXml, IPublishProperties publishProperties);
    Boolean PublishItem(IWin32Window parentWindow, String mediaObjectId, Stream stream, XmlDocument sessionXml, IPublishProperties publishProperties, IPublishProgressCallback callback, EventWaitHandle cancelEvent);
    Boolean HasPublishResults(XmlDocument sessionXml);
    Void LaunchPublishResults(XmlDocument sessionXml);
    Boolean HasSummaryInformation(XmlDocument sessionXml);
    Void ShowSummaryInformation(IWin32Window parentWindow, XmlDocument sessionXml);
}
```

### IPublishProperties

```
public interface IPublishProperties : IDisposable
{
    IPublishItemPropertyStore GetItemProperties(String mediaObjectId);
}
```

### IPublishItemPropertyStore

```
public interface IPublishItemPropertyStore : IDisposable
{
    Boolean GetValue<T>(String propertyName, ref T propertyValue);
}
```

### IPublishProgressCallback

```
public interface IPublishProgressCallback : IDisposable
{
    Void SetPublishProgress(Int32 percentComplete);
    Boolean Canceled();
}
```

### ISubscribePlugin (Microsoft.WindowsLive.SubscribePlugins)

```
public interface ISubscribePlugin
{
    Boolean ShowConfigurationSettings(IWin32Window parentWindow, String applicationName, Boolean reconfigure, XmlDocument persistXml, XmlDocument sessionXml);
    XmlDocument GetChildItems(XmlDocument sessionXml, String itemId, Int32 offset, Int32 limit, Boolean useCache, ISubscribeProgressCallback callback, EventWaitHandle cancelEvent);
    XmlDocument GetItemPropertiesById(XmlDocument sessionXml, String itemId, String itemType, String propertyName, Boolean useCache, ISubscribeProgressCallback callback, EventWaitHandle cancelEvent);
    XmlDocument GetItemPropertiesByUri(XmlDocument sessionXml, String uri, String itemType, String propertyName, Boolean useCache, ISubscribeProgressCallback callback, EventWaitHandle cancelEvent);
    Stream GetItemDataStream(XmlDocument sessionXml, String itemId, String dataStreamId, Boolean async, ISubscribeProgressCallback callback, EventWaitHandle cancelEvent);
    Void AddComment(XmlDocument sessionXml, String itemId, String comment, ISubscribeProgressCallback callback, EventWaitHandle cancelEvent);
    Void AddPersonRegion(XmlDocument sessionXml, String itemId, String name, String contactId, Double left, Double top, Double width, Double height, ISubscribeProgressCallback callback, EventWaitHandle cancelEvent);
    XmlDocument GetCapabilities(XmlDocument sessionXml);
}
```

### ISubscribeProgressCallback

```
public interface ISubscribeProgressCallback : IDisposable
{
    Void SetSubscribeProgress(Int32 percentComplete);
    Boolean Canceled();
}
```

---

## 4. Architecture — The Interop Bridge Pattern

```
┌─────────────────────────────────────────────────────────────────────┐
│  Native C++ Consumer (WLXMediaPublishSubscribe.dll)                 │
│                                                                     │
│  Calls COM interfaces via CoCreateInstance / IClassFactory          │
└─────────────────────┬───────────────────────────────────────────────┘
                      │  COM (IUnknown / IClassFactory / vtable)
                      ▼
┌─────────────────────────────────────────────────────────────────────┐
│  PublishPluginsInterop.dll  /  SubscribePluginsInterop.dll          │
│  (C++/CLI Mixed-Mode Assembly)                                      │
│                                                                     │
│  COM coclass: CreateManagedPublishPlugin / CreateManagedSubscribe   │
│  COM wrappers: PublishPluginComWrapper / SubscribePluginComWrapper  │
│  Native IStream wrapper: ReadOnlyComStream (publish)               │
│                          ManagedStream (subscribe)                  │
│  Callback adapters: ManagedPublishProgressCallback                 │
│                     ManagedSubscribeProgressCallback                │
│  Property adapters: ManagedPublishProperties                       │
│                     ManagedPublishItemPropertyStore                 │
│                                                                     │
│  Uses gcroot<T^> to hold references to managed objects             │
│  Uses C++/CLI #pragma managed/unmanaged for interop                │
└─────────────────────┬───────────────────────────────────────────────┘
                      │  Managed calls (through CLR VTableFixups)
                      ▼
┌─────────────────────────────────────────────────────────────────────┐
│  ManagedPluginWrapper.dll / ManagedSubscribePluginWrapper.dll       │
│  (.NET managed assemblies — NOT present in repo)                    │
│                                                                     │
│  Loads actual plugin DLLs (Facebook, Flickr, etc.) via reflection  │
│  via AssemblyLoader.AssemblyResolveHandler()                        │
└─────────────────────┬───────────────────────────────────────────────┘
                      │  .NET interface implementation
                      ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Actual Plugin DLLs (.NET managed)                                  │
│                                                                     │
│  WLFacebookPlugin.dll   — IPublishPlugin + ISubscribePlugin        │
│  WLFlickrPlugin.dll     — IPublishPlugin only                      │
│  WLVimeoPlugin.dll      — IPublishPlugin only                      │
│  WLYouTubePlugin.dll    — IPublishPlugin only                      │
│                                                                     │
│  Microsoft.WindowsLive.PublishPlugins.dll  (interface definitions)  │
│  Microsoft.WindowsLive.SubscribePlugins.dll (interface definitions) │
└─────────────────────────────────────────────────────────────────────┘
```

### COM Native Interface (IOnlineMedia* types)

The interop DLLs bridge to these native COM interfaces from `WLXMediaPublishSubscribe.dll`:

| Native Interface | Direction | Managed Equivalent |
|-----------------|-----------|-------------------|
| `IOnlineMediaProgressCallback*` | Native → Managed callback | `IPublishProgressCallback` / `ISubscribeProgressCallback` |
| `IOnlineMediaItemPropertyStore*` | Native → Managed read | `IPublishItemPropertyStore` |
| `IOnlineMediaItemProperties*` | Native → Managed read | `IPublishProperties` |
| `ISharedBitmap*` | Used in PropVariant helpers | `System.Drawing.Bitmap` |
| `IStream*` | COM stream | `System.IO.Stream` |

### Data Conversion Layer

`PropVariantManagedHelpers` handles type marshaling between native PROPVARIANT and .NET types:

| Native Type | Managed Type |
|-------------|-------------|
| VT_FILETIME | `System.DateTime` |
| VT_BSTR / VT_LPWSTR | `System.String` |
| VT_BOOL | `System.Boolean` |
| VT_UNKNOWN | `ISharedBitmap` (via WIC) |
| VT_UI1..VT_R8 arrays | Typed .NET arrays |
| VT_CY | System.Decimal |

### XML Bridge

`XmlHelpers` converts between:
- Native `IXMLDOMDocument` (COM) ↔ `System.Xml.XmlDocument` (managed)

This is critical because the plugin interface passes all configuration/session data as XML documents.

---

## 5. VTable Fixups

The CLR VTableFixups directories indicate native-to-managed thunks:

- **PublishPluginsInterop.dll**: 13 VTable fixup entries (0x68 bytes) at RVA 0xF1D0
- **SubscribePluginsInterop.dll**: 23 VTable fixup entries (0xB8 bytes) at RVA 0xE554

These thunks allow the COM vtable entries in the native wrapper classes to call into managed code through the CLR.

---

## 6. Key Differences Between the Two DLLs

| Aspect | PublishPluginsInterop | SubscribePluginsInterop |
|--------|----------------------|------------------------|
| **Referenced managed assembly** | Microsoft.WindowsLive.PublishPlugins | Microsoft.WindowsLive.SubscribePlugins |
| **Managed target** | ManagedPluginWrapper.dll | ManagedSubscribePluginWrapper.dll |
| **Plugin interface** | IPublishPlugin (6 methods) | ISubscribePlugin (8 methods) |
| **Progress callback** | SetPublishProgress | SetSubscribeProgress |
| **Stream support** | ReadOnlyComStream (native → managed) | ManagedStream + ManagedStreamImpl (managed → native IStream) |
| **Special classes** | ManagedPublishProperties, ManagedPublishItemPropertyStore | AssemblyLoader, StreamComWrapper |
| **VTable fixups** | 13 entries | 23 entries (more managed vtable slots) |
| **Timestamp** | 01:17:04 | 01:17:08 (4 seconds later) |
| **PDB GUID** | `{E96C1C59-...}` | `{8D12B774-...}` |

---

## 7. Build Information

| Property | Value |
|----------|-------|
| **Build system** | MSBuild with `/clr` (C++/CLI) |
| **Source path** | `e:\bt\1105173\public\...` (Windows build lab) |
| **Strong name key** | `35MSSharedLib1024.snk` (fake/test key) |
| **Security permission** | `System.Security.Permissions.SecurityPermissionAttribute` from mscorlib |
| **Version** | 16.4.0.0 |
| **Copyright** | Microsoft Corporation. All rights reserved. |
| **Timestamp** | Tue Apr 1 01:17:04-08 2014 (build 16.4.3528.0331) |

---

## 8. Missing Components

The following assemblies are referenced but **not present** in the repo:

| Assembly | Notes |
|----------|-------|
| **ManagedPluginWrapper.dll** | Target of PublishPluginsInterop exports |
| **ManagedSubscribePluginWrapper.dll** | Target of SubscribePluginsInterop exports |
| **Microsoft.VisualC** v10.0.0.0 | C++/CLI runtime support |
| **System.Windows.Forms** v4.0.0.0 | WinForms (for IWin32Window parameter) |

The wrapper DLLs (`ManagedPluginWrapper.dll`, `ManagedSubscribePluginWrapper.dll`) are the intermediate managed layer that dynamically loads the actual plugin DLLs (Facebook, Flickr, etc.) via reflection and the `AssemblyLoader` class.

---

## 9. Security Observations

- Both DLLs are **Strong Name Signed** but with a **fake/test key** (`35MSSharedLib1024.snk` from `e:\bt\1105173\public\internal\strongnamekeys\fake\`), meaning the strong name signature provides no security guarantee — it's only for assembly identity.
- `SecurityPermissionAttribute` is applied from mscorlib, indicating the assembly demands some CAS (Code Access Security) permissions at load time.
- The DLLs are Microsoft Authenticode signed (timestamped signatures visible in PE data).
