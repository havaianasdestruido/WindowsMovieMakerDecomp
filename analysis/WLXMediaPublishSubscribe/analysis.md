# WLXMediaPublishSubscribe.dll Analysis

## Overview
Publish/subscribe framework DLL for Windows Live Movie Maker 2012. PE32 x86 DLL, linker v11.0 (MSVC 2012), image base 0x10000000, ASLR+DEP. Timestamp: 2014-04-01 01:25:56 (file header) / 01:11:52 (export). Build: 16.4.3528.0331_ship.client.main.w5m4. PDB: `WLXMediaPublishSubscribe.pdb` {17F284FA-930A-4DA2-9649-93B296009330}.

## PE Headers
- **Machine**: x86 (0x14C)
- **Subsystem**: Windows GUI (2)
- **Entry Point**: 0x1006E053
- **Image Size**: 0x16A000 (1,486,848 bytes)
- **DLL Characteristics**: Dynamic Base, NX Compatible
- **Stack**: 256KB reserve / 4KB commit
- **Heap**: 1MB reserve / 4KB commit

## Section Layout
| Section | VA | VirtSize | RawSize | Purpose |
|---------|-------|----------|---------|---------|
| `.text` | 0x1000 | 0x8A0B7 | 0x8A200 | Code (568 KB) |
| `.data` | 0x8C000 | 0x6F0C | 0x5E00 | Globals, vtables, RTTI |
| `.rsrc` | 0x93000 | 0xC85B0 | 0xC8600 | Resources (821 KB -- largest) |
| `.reloc` | 0x15C000 | 0xD1A0 | 0xD200 | ASLR relocations |

## Export Table (22 functions)

### COM Infrastructure (4)
| Ordinal | Name |
|---------|------|
| 19 | `DllCanUnloadNow` |
| 20 | `DllGetClassObject` |
| 21 | `DllRegisterServer` |
| 22 | `DllUnregisterServer` |

### MediaPublishSubscribeHelper (9 methods)
| Ordinal | Name | Signature |
|---------|------|-----------|
| 1 | `??0MediaPublishSubscribeHelper@@QAE@XZ` | ctor |
| 3 | `??1MediaPublishSubscribeHelper@@UAE@XZ` | dtor (virtual) |
| 5 | `?CreateContainerAndPublishItems@MediaPublishSubscribeHelper@@QAEXPAUIGrinderTaskScheduler@@PAUIGrinderJobEventSink@@KPAKPBU__MIDL___MIDL_itf_mediapublishsubscribe_0000_0005_0002@@PAUIMediaPublishSubscribeItemSet@@@Z` | Create container and publish items |
| 9 | `?GetLiveSignInProvider@MediaPublishSubscribeHelper@@SGJPAPAUILiveSignInProvider@@@Z` | Static: get Live Sign-In provider |
| 10 | `?GetMediaPublishSubscribeProviderManager@MediaPublishSubscribeHelper@@QAEXPAPAUIMediaPublishSubscribeProviderManager@@@Z` | Get provider manager |
| 11 | `?Initialize@MediaPublishSubscribeHelper@@QAEXABU_GUID@@@Z` | Initialize with GUID |
| 14 | `?LaunchAuthBrowser@MediaPublishSubscribeHelper@@SGXPB_WW4SiteIDType@1@_N2PAUILiveSignInProvider@@@Z` | Static: launch auth browser (5-param) |
| 15 | `?LaunchAuthBrowser@MediaPublishSubscribeHelper@@SGXPB_WW4SiteIDType@1@_NPAUILiveSignInProvider@@@Z` | Static: launch auth browser (4-param) |
| 16 | `?PluginPublishItems@MediaPublishSubscribeHelper@@QAEXPAUIGrinderTaskScheduler@@PAUIGrinderJobEventSink@@KPAKPB_W3PAUIMediaPublishSubscribeItemSet@@PAUIUnknown@@PAUIXMLDOMDocument@@@Z` | Plugin-based publish items |
| 17 | `?PublishItems@MediaPublishSubscribeHelper@@QAEXPAUIGrinderTaskScheduler@@PAUIGrinderJobEventSink@@KPAKPBU__MIDL___MIDL_itf_mediapublishsubscribe_0000_0000_0008@@PAUIMediaPublishSubscribeItemSet@@II@Z` | Publish items (core method) |

### VideoTranscoder (6 methods)
| Ordinal | Name | Signature |
|---------|------|-----------|
| 2 | `??0VideoTranscoder@@QAE@XZ` | ctor |
| 4 | `??1VideoTranscoder@@UAE@XZ` | dtor (virtual) |
| 6 | `?GetInputFileAverageBandwidth@VideoTranscoder@@QAEJPAK@Z` | Get input file avg bandwidth |
| 7 | `?GetInputFileDisplaySize@VideoTranscoder@@QAEJPAUtagSIZE@@@Z` | Get display size |
| 8 | `?GetInputFileDuration@VideoTranscoder@@QAEJPAK@Z` | Get duration |
| 12 | `?Initialize@VideoTranscoder@@QAEJPB_W0KKKK@Z` | Initialize (input/output paths + flags) |
| 18 | `?Transcode@VideoTranscoder@@QAEJPAUIVideoTranscodeProgressCB@@@Z` | Run transcode with progress callback |

### Utility (1)
| Ordinal | Name | Signature |
|---------|------|-----------|
| 13 | `?IsHResultDRMProtected@@YG_NJ@Z` | Static: check if HRESULT indicates DRM protection |

## Import Table

### System DLLs (20)
| DLL | Functions | Purpose |
|-----|-----------|---------|
| **KERNEL32.dll** | 100+ | Process/thread, memory, file I/O, synchronization, module loading, registry, time, string |
| **USER32.dll** | 70+ | Window management, dialog boxes, message pump, GDI UI, accessibility |
| **GDI32.dll** | 20+ | Fonts, DC management, DIB sections, regions |
| **ADVAPI32.dll** | 20+ | Registry (open/close/query/enum), ETW tracing (RegisterTraceGuids), crypto (Crypt*) |
| **SHELL32.dll** | 7 | SHBindToParent, SHParseDisplayName, SHCreateItemFromParsingName, ShellExecuteW, ShellExecuteExW, SHGetFolderPathAndSubDirW |
| **SHLWAPI.dll** | 22+ | Path manipulation, string comparison, URL unescape, AssocQueryStringW |
| **ole32.dll** | 15+ | CoCreateInstance, CoInitialize(Ex), StringFromGUID2/CLSID, OleInitialize, PropVariant, CLSIDFromString, CreateStreamOnHGlobal |
| **OLEAUT32.dll** | 24+ (ordinals) | Variant types, string conversions |
| **WINHTTP.dll** | 16 | HTTP networking: WinHttpOpen/SendRequest/ReceiveResponse/ReadData/WriteData/QueryHeaders/SetStatusCallback, proxy (GetProxyForUrl, GetIEProxyConfigForCurrentUser), URL cracking |
| **WININET.dll** | 5 | URL cache: RetrieveUrlCacheEntryStream, CreateUrlCacheEntryW, CommitUrlCacheEntryW, etc. |
| **CRYPT32.dll** | 2 | CryptProtectData, CryptUnprotectData (DPAPI) |
| **PROPSYS.dll** | 3 | Property system: PSGetPropertyKeyFromName, VariantToPropVariant, InitPropVariantFromStringAsVector |
| **gdiplus.dll** | 40+ | GDI+: image manipulation, text rendering, bitmap operations, matrix transforms |
| **UxTheme.dll** | 5 | Visual styles: Open/CloseThemeData, GetThemeColor/Font, DrawThemeBackground, SetWindowTheme |
| **UXCore.dll** | 12 | Windows Live resource management: CRMResource, CRMStringResource, RMInitialize/Terminate |
| **OLEACC.dll** | 2 | Accessibility: AccessibleObjectFromWindow, LresultFromObject |
| **VERSION.dll** | 3 | File version info |
| **MSVCR110.dll** | 50+ | MSVC 2012 CRT: memory, exceptions, string, math, locale, threading |

### Windows Live / Partner DLLs (6)
| DLL | Functions | Purpose |
|-----|-----------|---------|
| **WLXPhotoSqm.dll** | 11 | SQM telemetry: Start, Increment, AddToStream, AddToStreamTimer, IsEnabled, Set, Shutdown |
| **DmxBici.dll** | 7 | BICI telemetry: Start/EndExperience, TransferExperienceToWeb, AddToStream, SetAnid, SetString, AddStringToDataPoint |
| **wlidcli.dll** | 18+ (ordinals) | Windows Live ID authentication/sign-in |
| **wlxpnp.dll** | ? | (Delay load) Base library |

**Delay Load:**
| DLL | Functions | Purpose |
|-----|-----------|---------|
| **WLXPhotoBase.dll** | 10 | Base types: Exception, Throw, ThrowLastError, New/Delete, GdiplusStatusToHresult, String manager, ATL::BaseAtlThrow |
| **MetadataSys.dll** | 1 | `WLXPSGetItemPropertyHandler` -- metadata property handler |
| **msi.dll** | 2 (ordinals) | Windows Installer |

## COM GUIDs & Interfaces

Binary RTTI/mangled names reveal the following COM/hierarchy types:

### CLSIDs referenced in binary
| Name | Description |
|------|-------------|
| `CLSID_MediaPluginManager` | COM class for media plugin manager |
| `CLSID_MediaPluginDecorator` | COM class for media plugin decorator |
| `CLSID_FlickrProvider` | COM class for Flickr publishing provider |
| `CLSID_LiveProvider` | COM class for Windows Live publishing provider |
| `CLSID_MediaPublishSubscribeProviderManager` | Provider manager factory |
| `CLSID_On...Manager` | Another manager class |
| `CLSID_MetadataManager` | Metadata property manager |
| `CLSID_LivePlugin` | Base plugin template |
| `CLSID_LivePluginT` | Templatized plugin |
| `CLSID_LiveGr...ugin` | (partial, possibly LiveGroupPlugin) |
| `CLSID_International...` | Possibly internationalization |

### IIDs referenced in binary
| Name | Description |
|------|-------------|
| `IID_IEnumString` | COM enumerator interface |
| `IID_IEnumVARIANT` | OLE enumerator variant |

### Interface types found in exports/mangled names
| Interface | Description |
|-----------|-------------|
| `IMediaPublishSubscribeProviderManager` | Provider manager interface |
| `IMediaPublishSubscribeItemSet` | Item set interface |
| `ILiveSignInProvider` | Windows Live authentication interface |
| `IGrinderTaskScheduler` | Background task scheduler (Grinder = internal code name) |
| `IGrinderJobEventSink` | Job event notifications |
| `IVideoTranscodeProgressCB` | Transcode progress callback |
| `IUnknown` | Standard COM base |
| `IXMLDOMDocument` | XML document interface |
| `ILiveProvider` | Live provider public interface |
| `ILiveProviderPrivate` | Live provider private interface |
| `IPublishSubscribeProvider` | Base publish/subscribe provider |
| `IPubSubProviderManager` | Pub/Sub variation of provider manager |
| `IServiceProvider` | COM service provider |

### Internal RTTI classes
| Class | Description |
|-------|-------------|
| `MediaPublishSubscribeHelper` | Main helper class (publish/subscribe operations) |
| `VideoTranscoder` | Video transcoding (bandwidth, size, duration analysis) |
| `MediaPluginManager` | Plugin discovery/management |
| `MediaPluginDecorator` | Plugin decoration/wrapping |
| `FlickrProvider` | Flickr publishing provider |
| `FlickrMoniker` / `FlickrMonikerAttachment` | Flickr COM moniker |
| `LiveProvider` | Windows Live provider |
| `PublishSubscribeMoniker` / `PublishSubscribeUser` | General pub/sub moniker |
| `PublishTaskProvider` | Task-based publishing |
| `DatabasePublishItemProperties` | Database-backed publish item properties |
| `DatabasePublishProvider` | Database-backed publish provider |
| `BasePublishProperties` | Base publish properties |
| `OnlineMediaPlugin` | Online media plugin |
| `MediaPluginInlineManager` | Inline plugin manager |
| `PublishItemPropertyStore` | Property store for publish items |
| `MVTranscoder` | Movie/Vista transcoder variant |

## Architecture

### Key Architecture Patterns

1. **Publish/Subscribe Model**: The DLL provides a framework allowing media items to be published to various online services via a plugin architecture. Plugins (Flickr, Live, etc.) implement the `IPublishSubscribeProvider` interface.

2. **Grinder Task Scheduler**: Publishing operations use an internal task scheduler codenamed "Grinder" (`IGrinderTaskScheduler`) with job event notifications (`IGrinderJobEventSink`). This suggests async background publish jobs with progress reporting.

3. **Provider Manager Pattern**: `MediaPublishSubscribeProviderManager` enumerates installed publishing plugins. `MediaPublishSubscribeHelper` is the main entry point for client code.

4. **Authentication**: Via `ILiveSignInProvider` (from wlidcli.dll -- Windows Live ID). `LaunchAuthBrowser` opens a browser-based OAuth/login flow for different `SiteIDType` values.

5. **Video Transcoding**: `VideoTranscoder` handles pre-publish transcoding with bandwidth/duration/size analysis and progress callbacks.

6. **Telemetry (dual system)**:
   - **WLXPhotoSqm.dll**: SQM (Service Quality Monitoring) counters for feature usage tracking
   - **DmxBici.dll**: BICI (Business Intelligence Customer Insights) A/B testing/experimentation telemetry
   - Both have Start/AddToStream/End patterns

7. **Data Persistence**: Database-backed properties (`DatabasePublishItemProperties`, `DatabasePublishProvider`) suggest a local cache/database of publish settings.

8. **DRM Detection**: `IsHResultDRMProtected` utility checks whether a media file is DRM-protected.

### Provider Ecosystem (from binary strings)
- **FlickrProvider** -- Flickr photo sharing
- **LiveProvider** -- Windows Live/SkyDrive publishing (likely later renamed to OneDrive)
- **LivePlugin** / **LivePluginT** -- Templatized plugin pattern
- **LiveGroupPlugin** -- Possibly group/shared album publishing

### Data Flow
```
Client Code
  |
  v
MediaPublishSubscribeHelper::PublishItems()
MediaPublishSubscribeHelper::PluginPublishItems()
MediaPublishSubscribeHelper::CreateContainerAndPublishItems()
  |
  +---> VideoTranscoder::Transcode() (if video needs transcoding)
  |
  +---> IGrinderTaskScheduler (async job scheduling)
  |
  +---> MediaPublishSubscribeProviderManager
           |
           +---> LiveProvider (ILiveProvider/ILiveProviderPrivate)
           +---> FlickrProvider
           +---> [Other plugins via COM]
  |
  +---> ILiveSignInProvider (authenticate via wlidcli.dll)
  +---> WinHTTP (HTTP upload to provider endpoints)
  +---> WININET (URL cache for downloaded thumbnails)
  +---> CRYPT32 (DPAPI protect/unprotect credentials/tokens)
  +---> PROPSYS (Windows property system integration)
```

## Registry Integration
- COM registration via `DllRegisterServer`/`DllUnregisterServer`
- Registry entries: `eGuidsW`, `CLSID` keys
- Standard ATL COM module with `CAtlModule::m_libid`

## Key String Literals Found in Binary
### Provider/Moniker Names
- `FlickrProvider`
- `FlickrMoniker` / `FlickrMonikerAttachment`
- `LiveProvider` / `LiveProviderPrivate`
- `LivePlugin` / `LivePluginT`
- `PublishSubscribeMoniker` / `PublishSubscribeUser`
- `MediaPluginManager` / `MediaPluginDecorator`
- `MediaPublishSubscribeProvider`
- `PublishTaskProvider`
- `OnlineMediaPlugin`
- `PublishItemPropertyStore`

### Database/Cache
- `DatabasePublishItemProperties`
- `DatabasePublishProvider`
- `BasePublishProperties`

### Other
- `MVTranscoder` (Movie/Vista Video Transcoder)

## Telemetry Integration
- **SQM (WLXPhotoSqm.dll)**: Start, Increment, AddToStream (3 overloads), AddToStreamTimer (2 overloads), IsEnabled, Set, Shutdown
- **BICI (DmxBici.dll)**: StartExperience, EndExperience, TransferExperienceToWeb, AddToStream, SetAnid, SetString, AddStringToDataPoint
- Both track feature usage for publishing operations

## Inter-DLL Dependencies
| DLL | Type | Role |
|-----|------|------|
| WLXPhotoBase.dll | Delay-load | Base exception, memory, string, GDI+ status |
| MetadataSys.dll | Delay-load | Metadata property handler |
| WLXPhotoSqm.dll | Load-time | SQM telemetry |
| DmxBici.dll | Load-time | BICI telemetry |
| wlidcli.dll | Load-time | Windows Live ID authentication |
| UXCore.dll | Load-time | Windows Live resource/string management |
| msi.dll | Delay-load | Windows Installer (COM registration cleanup?) |
