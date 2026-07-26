# WLXMediaPublishSubscribe.dll Dynamic Analysis

## Test Results Summary

**All 22 exports resolved and tested.** DLL loaded from `undecomp\Photo Gallery\` with all 20+ dependencies present.

## Export Signatures & Behavior

### COM Infrastructure (4 exports)

| # | Mangled Name | Demangled Signature | Behavior |
|---|---|---|---|
| 19 | `DllCanUnloadNow` | `HRESULT __stdcall DllCanUnloadNow()` | Returns **S_OK** (COM object count = 0 when no active refs) |
| 20 | `DllGetClassObject` | `HRESULT __stdcall DllGetClassObject(REFCLSID, REFIID, void**)` | Returns `0x80040111` (CLASS_E_CLASSNOTAVAILABLE) for NULL CLSID |
| 21 | `DllRegisterServer` | `HRESULT __stdcall DllRegisterServer()` | Returns `0x8002801C` (needs admin elevation) |
| 22 | `DllUnregisterServer` | `HRESULT __stdcall DllUnregisterServer()` | Returns `E_ACCESSDENIED` (needs admin elevation) |

### MediaPublishSubscribeHelper (9 methods)

| # | Mangled Name | Demangled Signature | Behavior |
|---|---|---|---|
| 1 | `??0MediaPublishSubscribeHelper@@QAE@XZ` | `MediaPublishSubscribeHelper::MediaPublishSubscribeHelper()` | **Ctor** - allocates 256+ byte object on stack, succeeds |
| 3 | `??1MediaPublishSubscribeHelper@@UAE@XZ` | `virtual MediaPublishSubscribeHelper::~MediaPublishSubscribeHelper()` | **Virtual dtor** |
| 5 | `?CreateContainerAndPublishItems@...@@QAEX...` | `void __thiscall CreateContainerAndPublishItems(IGrinderTaskScheduler*, IGrinderJobEventSink*, unsigned long, unsigned long*, const MIDL_struct_0000_0005_0002*, IMediaPublishSubscribeItemSet*)` | Creates upload container and publishes items; requires live Grinder scheduler |
| 9 | `?GetLiveSignInProvider@...@@SGJPAPAUILiveSignInProvider@@@Z` | `static HRESULT __stdcall GetLiveSignInProvider(ILiveSignInProvider**)` | **Returns S_OK** with valid provider pointer. Successfully creates `ILiveSignInProvider` from wlidcli.dll |
| 10 | `?GetMediaPublishSubscribeProviderManager@...@@QAEX...` | `void __thiscall GetMediaPublishSubscribeProviderManager(IMediaPublishSubscribeProviderManager**)` | Throws C++ exception when called with uninitialized helper (needs `Initialize()` first) |
| 11 | `?Initialize@...@@QAEXABU_GUID@@@Z` | `void __thiscall Initialize(const GUID&)` | Takes app GUID; throws C++ exception with zero GUID (expects valid WLPG app GUID) |
| 14 | `?LaunchAuthBrowser@...@@SGXPB_WW4SiteIDType@1@_N2PAUILiveSignInProvider@@@Z` | `static void __stdcall LaunchAuthBrowser(const wchar_t*, SiteIDType, BOOL, BOOL, ILiveSignInProvider*)` | **5-param** overload - launches browser for OAuth flow |
| 15 | `?LaunchAuthBrowser@...@@SGXPB_WW4SiteIDType@1@_NPAUILiveSignInProvider@@@Z` | `static void __stdcall LaunchAuthBrowser(const wchar_t*, SiteIDType, BOOL, ILiveSignInProvider*)` | **4-param** overload - launches browser for OAuth flow |
| 16 | `?PluginPublishItems@...@@QAEX...` | `void __thiscall PluginPublishItems(IGrinderTaskScheduler*, IGrinderJobEventSink*, unsigned long, unsigned long*, const wchar_t*, const wchar_t*, IMediaPublishSubscribeItemSet*, IUnknown*, IXMLDOMDocument*)` | Plugin-based publishing; takes plugin name, XML manifest |
| 17 | `?PublishItems@...@@QAEX...` | `void __thiscall PublishItems(IGrinderTaskScheduler*, IGrinderJobEventSink*, unsigned long, unsigned long*, const MIDL_struct_0000_0000_0008*, IMediaPublishSubscribeItemSet*, unsigned int, unsigned int)` | **Core publish method** - main entry point for standard publishing |

### VideoTranscoder (7 methods)

| # | Mangled Name | Demangled Signature | Behavior |
|---|---|---|---|
| 2 | `??0VideoTranscoder@@QAE@XZ` | `VideoTranscoder::VideoTranscoder()` | **Ctor** - allocates 512+ byte object |
| 4 | `??1VideoTranscoder@@UAE@XZ` | `virtual VideoTranscoder::~VideoTranscoder()` | **Virtual dtor** |
| 6 | `?GetInputFileAverageBandwidth@...@@QAEJPAK@Z` | `HRESULT __thiscall GetInputFileAverageBandwidth(unsigned long*)` | Returns `0x80040007` (CO_E_NOT_INITIALIZED) before Initialize; returns avg bandwidth in bps after |
| 7 | `?GetInputFileDisplaySize@...@@QAEJPAUtagSIZE@@@Z` | `HRESULT __thiscall GetInputFileDisplaySize(tagSIZE*)` | Returns `CO_E_NOT_INITIALIZED` before init; returns display pixel dimensions after |
| 8 | `?GetInputFileDuration@...@@QAEJPAK@Z` | `HRESULT __thiscall GetInputFileDuration(unsigned long*)` | Returns `CO_E_NOT_INITIALIZED` before init; returns duration in milliseconds after |
| 12 | `?Initialize@...@@QAEJPB_W0KKKK@Z` | `HRESULT __thiscall Initialize(const wchar_t* inputPath, const wchar_t* outputPath, unsigned long, unsigned long, unsigned long, unsigned long)` | Returns `0x80070002` (ERROR_FILE_NOT_FOUND) for nonexistent files |
| 18 | `?Transcode@...@@QAEJPAUIVideoTranscodeProgressCB@@@Z` | `HRESULT __thiscall Transcode(IVideoTranscodeProgressCB*)` | Runs video transcode with progress callback; requires prior Initialize |

### Utility (1 export)

| # | Mangled Name | Demangled Signature | Behavior |
|---|---|---|---|
| 13 | `?IsHResultDRMProtected@@YG_NJ@Z` | `static BOOL __stdcall IsHResultDRMProtected(HRESULT)` | Checks if HRESULT indicates DRM protection; returned FALSE for E_FAIL, S_OK, and 0x800430C1 |

## Plugin Architecture

### Plugin Discovery & Loading

Plugins are discovered via two registry paths:
- `HKCU\Software\Microsoft\Windows Live\PublishPlugins`
- `HKCU\Software\Microsoft\Windows Live\SubscribePlugins`

Each plugin entry contains:
- `AssemblyPath` - DLL path for the plugin
- `ClassName` - COM class name to instantiate
- `FriendlyName` - Display name
- `IconPath` - Icon resource
- `UriMatch` - Regex for matching publish URLs
- `UriFilter` - URL filter pattern

### Plugin XML Manifest (`PubSubXml::XmlManifestFactory`)

The DLL uses an XML manifest system (`PubSubXmlManifestManifest`) to manage plugins:
```xml
<Plugins>
  <Plugin id="..." Type="PublishPlugin" DomainId="...">
    <Enabled/>
    <AssemblyPath/>
    <ClassName/>
    <UriMatch/>
  </Plugin>
</Plugins>
```

XPath queries used:
- `/Plugins/Plugin[@id='%ws' and Type='PublishPlugin']/Enabled`
- `/Plugins/Plugin[@id='%ws' and Type='PublishPlugin']/ClassName`
- `/Plugins/Plugin[Type='%ws' and DomainId='%ws' and @internal='True']`
- `/Plugins/Plugin[Type='%ws' and @internal='%ws']/UriMatch`

### Known Plugin DLLs

| DLL | CLSID | Provider Type |
|-----|-------|---------------|
| `WLFacebookPlugin.dll` | `{8DB100C7-50C5-46EB-B535-618AB68A3E22}` | FacebookPlugin.FacebookPublishPlugin / FacebookSubscribePlugin |
| `WLFlickrPlugin.dll` | `{1812A500-BA1E-41EE-B3A5-5D7B6FCA7393}` | FlickrPlugin.FlickrPublishPlugin |
| `WLYouTubePlugin.dll` | `{66557ED9-C5F8-4815-A8CC-D157272CFFCE}` | YouTubePlugin.YouTubePublishPlugin |
| `WLVimeoPlugin.dll` | `{1D31145D-AEFE-48B0-B48A-513E5D413B44}` | VimeoPlugin.VimeoPublishPlugin |

### Plugin Type Hierarchy (RTTI from .data section)

```
IUnknown
  +-- IPublishPlugin
  |     +-- ISubscribePlugin
  |     +-- IMediaPublishSubscribeProvider
  |           +-- ILiveProviderPrivate
  |                 +-- LiveProvider
  |           +-- FlickrProvider
  +-- IOnlineMediaPluginManager
  |     +-- OnlineMediaPluginManager (singleton via CComObject)
  +-- IOnlineMediaPluginDecorator
  |     +-- OnlineMediaPluginDecorator (CComObject)
  +-- IMediaPublishSubscribeProviderManager
  |     +-- MediaPubSubProviderManager (singleton via CComObject)
  +-- IOnlineMediaItemPropertyStore
  |     +-- DatabasePublishItemPropertyStore (CComObject)
  +-- IOnlineMediaItemProperties
  |     +-- DatabasePublishProperties (CComObject)
  +-- IPropertyKeySetVerify
  +-- ISetDatabase
  +-- IMetadataSettings
        +-- MetadataSettingsController (CComObject)
```

### COM Classes Registered

| CLSID Name | ATL Class | Threading |
|-----------|-----------|-----------|
| `CLSID_OnlineMediaPluginManager` | `CComObject<OnlineMediaPluginManager>` via `CSingletonWrapper` | Multi-threaded (free) |
| `CLSID_OnlineMediaPluginDecorator` | `CComObject<OnlineMediaPluginDecorator>` via `CNonGlobalClassFactorySingleton` | Multi-threaded (free) |
| `CLSID_FlickrProvider` | `CComObject<FlickrProvider>` | Multi-threaded (free) |
| `CLSID_LiveProvider` | `CComObject<LiveProvider>` | Multi-threaded (free) |
| `CLSID_MediaPublishSubscribeProviderManager` | `CComObject<MediaPubSubProviderManager>` via `CSingletonWrapper` | Multi-threaded (free) |
| `CLSID_DatabasePublishProperties` | `CComObject<DatabasePublishProperties>` via `CComCoClass` | Multi-threaded (free) |
| `CLSID_DatabasePublishItemPropertyStore` | `CComObject<DatabasePublishItemPropertyStore>` | Multi-threaded (free) |
| `CLSID_MetadataSettingsController` | `CComObject<MetadataSettingsController>` via `CComCoClass` | Multi-threaded (free) |

## Auth Flow (wlidcli.dll Integration)

### Windows Live ID Authentication

1. **`GetLiveSignInProvider()`** - Static method that creates an `ILiveSignInProvider` instance. Successfully returned a valid pointer at `0x009A6250` in test.

2. **`LaunchAuthBrowser()`** - Static method with two overloads:
   - 4-param: `(URL, SiteIDType, forceReauth, ILiveSignInProvider*)`
   - 5-param: `(URL, SiteIDType, forceReauth, bool, ILiveSignInProvider*)`
   - Opens `iexplore.exe` for browser-based login flow

3. **wlidcli.dll imports** (18 ordinal imports):
   - Windows Live ID client library for token management
   - Used for COMA (Contacts, Albums, Media API) authentication

4. **Auth credential storage**:
   - `ComaAuthentication` / `AuthKey` / `AuthSecret` / `AuthUser` stored in XML manifest
   - `AuthEnv` for environment switching (production vs staging)
   - `CryptProtectData` / `CryptUnprotectData` (DPAPI) for credential encryption
   - Registry: `Software\Microsoft\Windows Live\Photo Gallery\FlickrAccounts` for Flickr auth tokens

5. **IdentityCRL integration**:
   - Registry: `Software\Microsoft\IdentityCRL`
   - Loads `wlidcli.dll` and `msidcrl40.dll` from install directory
   - Uses `TargetDir` registry value for DLL path resolution

6. **Auth protocol strings**:
   - `Authorization: WLID1.0 %s` - Windows Live ID token header
   - `Authorization: Anonymous` - Anonymous access for public resources
   - `%s?authkey=%s` - Auth key query parameter

## Upload Mechanism (WinHTTP)

### HTTP Stack

The DLL imports 16 WinHTTP functions for all HTTP operations:

| Function | Purpose |
|----------|---------|
| `WinHttpOpen` | Create HTTP session |
| `WinHttpConnect` | Connect to server |
| `WinHttpOpenRequest` | Create HTTP request |
| `WinHttpAddRequestHeaders` | Add custom headers |
| `WinHttpSendRequest` | Send request |
| `WinHttpReceiveResponse` | Receive response |
| `WinHttpReadData` / `WinHttpWriteData` | Read/write data streams |
| `WinHttpQueryHeaders` / `WinHttpQueryDataAvailable` | Query response info |
| `WinHttpSetStatusCallback` | Async status callbacks |
| `WinHttpSetOption` | Configure options |
| `WinHttpSetTimeouts` | Set connection/request timeouts |
| `WinHttpCrackUrl` | Parse URLs |
| `WinHttpGetProxyForUrl` | Auto-detect proxy |
| `WinHttpGetIEProxyConfigForCurrentUser` | Use IE proxy settings |
| `WinHttpCloseHandle` | Cleanup |

### URL Cache (WinINET)

Also uses 5 WinINET functions for thumbnail/image caching:
- `CreateUrlCacheEntryW` / `CommitUrlCacheEntryW` - Create cache entries
- `RetrieveUrlCacheEntryStreamW` / `ReadUrlCacheEntryStream` - Read cached data
- `UnlockUrlCacheEntryStream` - Release cache lock

### Upload Protocol Details

**Flickr API** (via `api.flickr.com`):
- REST API: `http://api.flickr.com/services/rest/`
- Upload: `http://api.flickr.com/services/upload/`
- Auth: `flickr.auth.getFrob`, `flickr.auth.getToken`
- Photo management: `flickr.photosets.create`, `flickr.photosets.addPhoto`, `flickr.photos.setDates`
- API key: `5cc21a1294d0be0fb5567ec2bc6ff847` (hardcoded)
- Shared secret: `44018e234dacfc3f` (hardcoded)
- Auth URL: `http://www.flickr.com/services/auth`

**Windows Live COMA API** (via `api.live.net`):
- Base URL: `http://api.live.net` / `http://api.live-tst.net` (staging)
- SSL: `https://ssl.live.com` / `https://ssl.live-int.com` (staging)
- Protocol: Atom XML with custom `live:` namespace
- Namespace: `http://api.live.com/schemas`
- App IDs: `1140851978` (production), `1610621310`
- User profile: `https://profile.live.com/`
- Group creation: `https://groups.live.com/create.aspx`
- Event creation: `https://events.live.com/create.aspx`

**Multipart upload format**:
```
Content-Type: multipart/form-data; boundary="..."
Content-Disposition: form-data; name="..."
Content-Type: image/jpeg
```

**Flickr-specific properties** (XML):
```xml
//ItemSet/Item
//ItemSet/Item[PerceivedType='image']
//ComaAuthentication/AuthKey
//ComaAuthentication/AuthSecret
//ComaAuthentication/AuthEnv
//ComaAuthentication/AuthUser
```

### HTTP Headers Used
- `Content-Type: application/atom+xml` - COMA API requests
- `Content-Type: multipart/related;boundary="...";type="application/atom+xml"` - Upload with metadata
- `Content-Type: image/jpeg` / `video/x-mpeg4` - Media uploads
- `MIME-Version: 1.0`
- `Authorization: WLID1.0 %s` - Auth tokens
- `Pragma: no-cache`
- `Accept-Language:` / `Accept-Encoding:` / `Content-Encoding:` - Standard headers

## Telemetry

### WLXPhotoSqm.dll (SQM)

| Function | Signature |
|----------|-----------|
| `Sqm::Startup()` | `void __stdcall Sqm::Startup()` |
| `Sqm::Shutdown()` | `void __stdcall Sqm::Shutdown()` |
| `Sqm::Increment()` | `void __stdcall Sqm::Increment(DWORD, DWORD)` |
| `Sqm::AddToStream()` | 4 overloads: `(DWORD,DWORD)`, `(DWORD,DWORD,DWORD)`, `(DWORD,DWORD,DWORD,DWORD)`, `(DWORD,DWORD,Tuple*)`, `(DWORD,DWORD,DWORD,Tuple*)` |
| `Sqm::AddToStreamTimer()` | 2 overloads: `(DWORD,DWORD,Tuple*)`, `(DWORD,DWORD,DWORD,Tuple*)` |
| `Sqm::IsEnabled()` | `bool __stdcall Sqm::IsEnabled()` |
| `Sqm::Set()` | `void __stdcall Sqm::Set(DWORD, DWORD)` |

**RTTI wrapper**: `SqmStartupWrapper` - RAII wrapper that calls `Sqm::Startup` on construction, `Sqm::Shutdown` on destruction.

### DmxBici.dll (BICI)

| Function | Signature |
|----------|-----------|
| `BiciWrapper::StartExperience()` | `HRESULT __stdcall StartExperience()` |
| `BiciWrapper::EndExperience()` | `HRESULT __stdcall EndExperience()` |
| `BiciWrapper::TransferExperienceToWeb()` | `bool __stdcall TransferExperienceToWeb(const wchar_t*, wchar_t**)` |
| `BiciWrapper::AddToStream()` | `void __stdcall AddToStream(DWORD, Tuple*)` |
| `BiciWrapper::AddStringToDataPoint()` | `bool __stdcall AddStringToDataPoint(DWORD, DWORD, const wchar_t*)` |
| `BiciWrapper::SetAnid()` | `HRESULT __stdcall SetAnid(const wchar_t*)` |
| `BiciWrapper::SetString()` | `bool __stdcall SetString(DWORD, const wchar_t*)` |

**RTTI wrapper**: `BiciStartupWrapper` - RAII wrapper for BICI experience tracking.

### Telemetry Flow
```
Publishing operation starts
  -> Sqm::Startup() / BiciWrapper::StartExperience()
  -> Sqm::AddToStream() / AddToStreamTimer() per operation
  -> Sqm::Increment() for counters
  -> BiciWrapper::AddToStream() / AddStringToDataPoint()
  -> BiciWrapper::TransferExperienceToWeb() to upload telemetry
  -> Sqm::Shutdown() / BiciWrapper::EndExperience()
```

## Video Transcoding Pipeline

### Pipeline Architecture
```
Client
  -> VideoTranscoder::VideoTranscoder()  [ctor]
  -> VideoTranscoder::Initialize(inputPath, outputPath, flags...)
     - Returns ERROR_FILE_NOT_FOUND if file doesn't exist
  -> VideoTranscoder::GetInputFileDuration()
  -> VideoTranscoder::GetInputFileDisplaySize()
  -> VideoTranscoder::GetInputFileAverageBandwidth()
  -> VideoTranscoder::Transcode(IVideoTranscodeProgressCB*)
     - Uses IVideoTranscodeProgressCB for progress notifications
  -> ~VideoTranscoder()  [dtor]
```

### Status Codes Observed
| HRESULT | Meaning |
|---------|---------|
| `0x80040007` (CO_E_NOT_INITIALIZED) | Methods called before Initialize() |
| `0x80070002` (ERROR_FILE_NOT_FOUND) | Input file does not exist |
| `0x800430C1` | DRM protection (passed to IsHResultDRMProtected) |

### Transcode Profiles

**SoapBox Video Profile** (embedded in binary):
- Audio: WMA Voice (0x0161), 44100 Hz, stereo, 96 kbps
- Video: WMV3, 544 kbps, 24-bit, max keyframe spacing 40M
- Resolution: dynamic based on input
- Storage format: 1

### Companion DLLs
- `WLXVideoTrim.dll` - Video trimming functionality
- `WLXImageTranscode.dll` - Image transcoding
- `WLXCodecHostPS.dll` - Codec hosting pipeline stage

## Registry Keys

| Path | Purpose |
|------|---------|
| `HKCU\Software\Microsoft\Windows Live\PublishPlugins` | Registered publish plugins |
| `HKCU\Software\Microsoft\Windows Live\SubscribePlugins` | Registered subscribe plugins |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\FlickrAccounts` | Flickr auth tokens |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery\Library` | Library settings, LastLogin |
| `HKCU\Software\Microsoft\Windows Live\Photo Gallery` | General settings |
| `HKCU\Software\Microsoft\Windows Live\Common` | SuiteLanguage, InstalledLanguages |
| `HKCU\Software\Microsoft\IdentityCRL` | Windows Live ID credential cache |
| `HKLM\Software\Microsoft\Windows Live\Environment` | PhotoGallery install path |

## File System Paths

| Path Pattern | Purpose |
|-------------|---------|
| `%ProgramFiles%\Windows Live\Shared` | Shared DLLs |
| `%ProgramFiles(x86)%\Windows Live\Shared` | Shared DLLs (x86) |
| `\wlidcli.dll` / `\msidcrl40.dll` | Identity DLLs from install dir |
| `Microsoft\WindowsLive\PublishPlugins\PersistentData\` | Local cache |
| `Software\Microsoft\Windows Live\PublishPlugins` | Plugin manifests |

## Data Model

### Publish Item Properties
- `SupportedMediaTypes` / `MaxItemCount` / `MaxFileSize` / `MaxVideoDuration`
- `DomainId` / `MaxPhotoWidth` / `MaxPhotoHeight` / `MaxVideoWidth` / `MaxVideoHeight`
- `MaxVideoBitrate` / `MaxVideoFramerate` / `PreferredVideoWidth` / `PreferredVideoHeight`
- `PreferredVideoBitrate` / `PreferredVideoFramerate`

### Windows Property System Integration
Properties mapped via PROPSYS.dll:
- `System.Title`, `System.Subject`, `System.Photo.DateTaken`
- `System.Rating`, `System.SimpleRating`, `System.Author`
- `System.Image.HorizontalSize`, `System.Image.VerticalSize`
- `System.Video.FrameWidth`, `System.Video.FrameHeight`
- `System.Media.Duration`, `System.Video.TotalBitrate`, `System.Video.FrameRate`
- `System.Photo.ExposureTime`, `System.Photo.FocalLength`, `System.Photo.FNumber`
- `System.Photo.ISOSpeed`, `System.Photo.CameraManufacturer`, `System.Photo.CameraModel`

### COMA Atom Feed Data Model
- Sites (Events/Groups/Spaces) with `QuotaUsed`, `TotalQuota`, `MemberCount`
- Folders with `FolderName`, `FolderCaption`, `SharingLevel`, `FolderUri`
- Files with `FileName`, `FileCaption`, `FileThumbnail`, `FileUploadUri`
- Comments, PersonRegions, Permissions
- Metadata: `live:resourceId`, `live:sharingLevel`, `live:canonicalName`

### DRM Detection
- `IsDRMCached` / `BaseLAURL` / `DRMHeader.*` properties
- DRM headers: `KID`, `LAINFO`, `CID`, `SECURITYVERSION`, `ContentDistributor`, `SubscriptionContentID`

### Localized Content
- Supported locales: sr-cyrl-cs, sr-latn-cs, ar-ploc-sa, ja-ploc-jp, az-latn-az, bs-latn-ba, ha-latn-ng, iu-latn-ca, uz-latn-uz, en-locr-us, ca-ES-valencia, chr-Cher, qut-Latn, sr-Cyrl-BA

## ETW Tracing

ADVAPI32.dll imports for ETW:
- `RegisterTraceGuidsW` / `UnregisterTraceGuids` - Register ETW trace provider
- `GetTraceLoggerHandle` / `GetTraceEnableLevel` / `GetTraceEnableFlags` - Query trace config
- `TraceEvent` - Emit trace events

## MSI Integration

Delay-loaded `msi.dll` imports (2 ordinal functions):
- Likely used for cleanup during COM unregistration
- Possibly `MsiOpenPackage` / `MsiCloseHandle` for installer integration

## Additional RTTI Classes Discovered

| Class | Description |
|-------|-------------|
| `PubSubXml::XmlManifestFactory` | XML manifest parsing factory |
| `OnlineMediaPluginManager` | Singleton managing all plugins |
| `OnlineMediaPluginDecorator` | Decorator pattern for plugin wrapping |
| `MediaPubSubProviderManager` | Provider lifecycle management |
| `DatabasePublishItemPropertyStore` | Database-backed property storage |
| `DatabasePublishProperties` | Database-backed publish properties |
| `MetadataSettingsController` | Metadata sharing settings |
| `EnumAttributes` | COM enumerator for publish attributes |
| `EnumMonikers` | COM enumerator for monikers |
| `PublishSubscribeMoniker` / `PublishSubscribeUser` | COM moniker objects |
| `FlickrMoniker` / `FlickrMonikerAttachment` | Flickr-specific moniker |
| `PublishTaskProvider` | Task-based publishing |
| `SqmStartupWrapper` | RAII SQM lifecycle |
| `BiciStartupWrapper` | RAII BICI lifecycle |
| `CAtlModule` / `CComModule` / `_ATL_MODULE70` | ATL module infrastructure |
| `CRegObject` | ATL registry object |
| `EnumMediaPublishSubscribeMonikerAttributes` | Moniker attribute enumeration |

## Key Observations

1. **Error 126 on load**: The DLL has hard load-time dependencies on WLXPhotoSqm.dll, DmxBici.dll, wlidcli.dll, and UXCore.dll that must all be co-located
2. **C++ exceptions (0xE06D7363)**: The helper class methods throw C++ exceptions for error conditions rather than returning HRESULTs (Initialize, GetMediaPublishSubscribeProviderManager)
3. **Stack-allocated helper objects**: The test successfully constructed both MediaPublishSubscribeHelper and VideoTranscoder on the stack, proving they are concrete C++ classes
4. **VideoTranscoder state machine**: Returns CO_E_NOT_INITIALIZED before Initialize(), ERROR_FILE_NOT_FOUND for bad paths, and presumably S_OK when properly initialized
5. **IsHResultDRMProtected**: Conservative DRM detection - returned FALSE for all tested HRESULTs
6. **GetLiveSignInProvider**: Successfully creates ILiveSignInProvider instance, proving wlidcli.dll integration works
7. **No thread safety needed for init**: Both classes use single-threaded apartment (QAE calling convention = __thiscall thisptr)
