# Publishing Plugins Analysis

All four publishing plugins (Facebook, Flickr, Vimeo, YouTube) are **.NET Framework 4.0 managed assemblies**, not native C++ DLLs. This distinguishes them from every other WLX/WL DLL in the Photo Gallery suite.

---

## 1. WLFacebookPlugin.dll

### PE Headers
| Field | Value |
|-------|-------|
| **Machine** | x86 (0x14C) |
| **Linker** | MSVC 11.00 |
| **Subsystem** | Windows CUI (3) — typical for .NET assemblies |
| **Image Base** | 0x00400000 |
| **Image Size** | 0x8E000 (570,368 bytes, 558 KB on disk) |
| **Entry Point** | 0x00484DFE (`_CorDllMain` from mscoree.dll) |
| **Timestamp** | 2014-04-01 01:17:17 (533A3DCD) |
| **Characteristics** | Dynamic Base, NX Compatible, Terminal Server Aware |
| **CLR Header** | COM Descriptor at RVA 0x2008, size 0x48 (.NET metadata) |
| **PDB** | `WLFacebookPlugin.pdb` `{4A5CD2FE-C24E-4448-B54D-8D547584291F}` |

### Sections
| Section | VA | VirtSize | RawSize | Purpose |
|---------|------|----------|---------|---------|
| .text | 0x2000 | 0x82E04 | 0x83000 | IL code + metadata (537 KB) |
| .rsrc | 0x86000 | 0x4558 | 0x4600 | Embedded resources (17 KB) |
| .reloc | 0x8C000 | 0x0C | 0x200 | Base relocations |

### Exports
**No native exports.** The Export Directory table is zeroed out. The DLL entry point is `_CorDllMain` imported from `mscoree.dll`.

### Imports
- **mscoree.dll** (1 function): `_CorDllMain` — .NET runtime loader

### Version Info
- **FileVersion:** 16.4.3528.331
- **ProductVersion:** 16.4.3528.331
- **ProductName:** Windows Live (R)
- **FileDescription:** WLFacebookPlugin
- **CompanyName:** Microsoft Corp.
- **LegalCopyright:** (c) Microsoft Corporation. All rights reserved.

### .NET Framework Target
`.NETFramework,Version=v4.0` (from assembly metadata)

### COM Interface
The plugin exposes .NET classes with `ComVisibleAttribute` for COM interop, consumed by the native `WLXMediaPublishSubscribe.dll` framework:

- **Implements:** `IPublishPlugin`, `ISubscribePlugin` (from WLXMediaPublishSubscribe)
- **Classes:** `FacebookPlugin`, `FacebookPublishPlugin`, `FacebookSubscribePlugin`
- **Internal interface:** `IFacebook`

### Class Architecture
```
FacebookPlugin (base plugin factory)
  +-- FacebookPublishPlugin (IPublishPlugin implementation)
  |     - PublishItem(), PublishPhoto(), PublishVideo()
  |     - PrepareUpload(), UploadChunk(), GetUploadResponse()
  |     - CreateAlbum(), GetProfileAlbumId()
  |     - GetAuthenticationSessionEnv/Key/Secret/User()
  +-- FacebookSubscribePlugin (ISubscribePlugin implementation)
  |     - Subscribe()
  +-- FacebookService (HTTP API helper)
  |     - GetResponse(), GetResponseStream()
  +-- FacebookParser (XML response parser)
  |     - DeserializeAuthToken(), DeserializeAccount()
  |     - DeserializeAccountLink(), DeserializeAccountSession()
  |     - DeserializeLoggedInUser(), DeserializePermissions()
  +-- FacebookException (error class)
  +-- FacebookFriend (data class)
  |     - Name, Id, ProfileUrl, SquarePictureUrl, Confidence
  +-- FacebookPluginResources (localized strings)
  +-- FacebookAccountLinkPanel (UI)
  +-- FacebookVideoPanel (UI)
```

### Authentication
- **Method:** Custom API Key + Secret + Session-based auth (Facebook REST API, pre-Graph era)
- **Constants:** `FACEBOOK_REST_URL`, `FACEBOOK_VIDEO_REST_URL` (API endpoints)
- **Tokens:** `DefaultAPIKey`, `DefaultAPISecret`, `APIKey`, `Secret`
- **Session:** `SessionKey`, `SessionSecret`, `SessionEnv`, `SessionUser`
- **OAuth-style flow:** `GetAuthenticationSessionEnv()` → `GetAuthenticationSessionKey()` → `GetAuthenticationSessionSecret()` → `GetAuthenticationSessionUser()`
- **Account links:** `SerializeAccountLink()`, `DeserializeAccountLink()`, `SetAccountLinks()`, `GetAccountLinks()`

### API Constants
```
FACEBOOK_REST_URL       — Base REST API endpoint (likely http://api.facebook.com/restserver.php)
FACEBOOK_VIDEO_REST_URL — Video-specific REST endpoint
PROFILE_PICTURES_ALBUM_ID — Special album ID for profile pictures
BUFFER_SIZE             — Upload buffer size constant
```

### API Methods (from reflection)
- `GetProfileAlbumId()`, `CreateAlbum()`
- `GetAuthenticationSessionEnv/Key/Secret/User()`
- `GetPermissions()`, `GetUploadedPhoto()`, `GetUploadedVideo()`
- `GetVideoUploadLimits()`
- `UploadChunk()`, `PrepareUpload()`, `PublishPhoto()`, `PublishVideo()`
- `ShouldContinueVideoUpload()`

### String Resources (error messages)
- `FacebookGeneralAuthError`, `FacebookCreateAlbumError`, `FacebookCreateTokenError`
- `FacebookGetSessionError`, `FacebookResponseError`, `FacebookGenericError`
- `FacebookNeedsOfflineAccessPermission`, `FacebookNeedsPhotoUploadPermission`, `FacebookNeedsVideoUploadPermission`
- `DefaultBrowserError`, `InvalidAuthToken`, `SERVICE_ERROR`, `UploadFailure`, `UploadSuccess`
- `VideoUploadDurationLimitWarning`, `VideoUploadSizeLimitWarning`, `VideoUploadLimitsWarning`
- `PhotoTermsOfUseMessage`, `VideoTermsOfUseMessage`

### UI Components (Windows Forms)
- `ConfigurationSettingsPanel`, `FacebookAccountLinkPanel`, `FacebookVideoPanel`
- `ChooseAccountHeaderLabel`, `ChooseAccountLabel`, `LinkAccountsHeaderLabel`
- `facebookAlbumNameTextBox`, `facebookAlbumDescriptionTextBox`, `facebookAlbumVisibilityComboBox`
- `facebookVideoTitleTextBox`, `facebookVideoDescriptionTextBox`
- `facebookAlbumRadioButton`, `facebookFriendsComboBox`, `facebookFriendSquarePictureBox`

### Upload Flow
1. Authenticate (API key + secret → session key/secret)
2. If publishing photos: `GetProfileAlbumId()` or `CreateAlbum()`
3. `PrepareUpload()` → `UploadChunk()` → `GetUploadResponse()`
4. Handle photo/video permissions, size limits, duration limits
5. Display results via `LaunchPublishResults()`

---

## 2. WLFlickrPlugin.dll

### PE Headers
| Field | Value |
|-------|-------|
| **Machine** | x86 (0x14C) |
| **Linker** | MSVC 11.00 |
| **Subsystem** | Windows CUI (3) |
| **Image Base** | 0x00400000 |
| **Image Size** | 0x44000 (278,528 bytes, 265 KB on disk) |
| **Entry Point** | 0x0043195E (`_CorDllMain` from mscoree.dll) |
| **Timestamp** | 2014-04-01 01:17:19 (533A3DCF — slightly different from others) |
| **CLR Header** | COM Descriptor at RVA 0x2008, size 0x48 |
| **PDB** | `WLFlickrPlugin.pdb` `{E4AC91BB-B3FB-4694-BF4F-1532161825B5}` |

### Sections
| Section | VA | VirtSize | RawSize | Purpose |
|---------|------|----------|---------|---------|
| .text | 0x2000 | 0x2F964 | 0x2FA00 | IL code + metadata (195 KB) |
| .rsrc | 0x32000 | 0xE518 | 0xE600 | Embedded resources (57 KB) |
| .reloc | 0x42000 | 0x0C | 0x200 | Base relocations |

### Exports
**No native exports.** Export Directory table is zeroed.

### Imports
- **mscoree.dll** (1 function): `_CorDllMain`

### Version Info
- **FileVersion:** 16.4.3528.331
- **ProductVersion:** 16.4.3528.331
- **FileDescription:** WLFlickrPlugin

### COM Interface
- **Implements:** `IPublishPlugin`
- **Classes:** `FlickrPlugin`, `FlickrPublishPlugin`
- **Internal API:** `FlickrApi`

### Class Architecture
```
FlickrPlugin (base plugin factory)
  +-- FlickrPublishPlugin (IPublishPlugin implementation)
  |     - PublishItem(), PreparePhotoUpload()
  |     - GetOAuthAccount(), GetOAuthUrl(), GetRequestToken()
  |     - VerifyOAuthToken(), SignOAuthRequest(), OAuthEncode()
  |     - GetSessionInfo(), GetAccountUploadStatus()
  |     - CreatePhotoSet(), GetUploadedPhotoId()
  |     - PersistInitializeDocument()
  |     - SessionLoadOAuth/SessionStoreOAuth
  |     - SessionLoadCreatePhotoSet/SessionStoreCreatePhotoSet
  +-- FlickrApi (Flickr API client)
  +-- FlickrApiException (error class)
  +-- FlickrPluginResources (localized strings)
  +-- ConfigureForm (Windows Forms UI)
  |     - authAuthorizeButton, authBackButton, authCancelButton
  |     - authAccountLink, authAccountLabel
  |     - pubPublishButton, pubAccountComboBox
  |     - pubPermissionsComboBox (public/friends/family/private)
```

### Authentication
- **Method:** OAuth 1.0 (full OAuth flow)
- **Classes:** `consumerKey`, `consumerSecret`, `Token`, `TokenSecret`
- **Flow:** `GetOAuthUrl()` → browser opens → `GetOAuthAccount()` → `VerifyOAuthToken()` → `SignOAuthRequest()`
- **OAuth helpers:** `OAuthEncode()`, `SignOAuthRequest()`
- **Session persistence:** `SessionLoadOAuth()`, `SessionStoreOAuth()`, `PersistInitializeDocument()`
- **Users:** `PersistLoadUsers()`, `PersistStoreSelectedUser()`, `PersistRemoveUser()`

### API Methods
- `GetOAuthUrl()`, `GetRequestToken()`, `VerifyOAuthToken()`, `SignOAuthRequest()`
- `GetSessionInfo()`, `GetAccountUploadStatus()`
- `CreatePhotoSet()`, `GetUploadedPhotoId()`, `GetUploadedImagesUrl()`
- `ValidateResult()`, `GetErrorInfo()`

### String Resources (error messages)
- `AuthenticateFailure`, `AuthenticateFailureTitle`, `AuthenticateFrobFailureFormat`
- `ErrorBadResponseFormat`, `ErrorXmlElementMissing`, `ErrorCode`
- `ExceededPhotoUploadLimit`, `ExceededVideoUploadLimit`
- `ExceededPhotoUploadLimitButNotVideos`, `ExceededVideoUploadLimitButNotPhotos`
- `UploadFailureLogFormat`, `UploadSuccessLogFormat`
- `AddPhotoFailureLogFormat`, `AddPhotoSuccessLogFormat`
- `CreatePhotoSetFailureLogFormat`, `CreatePhotoSetSuccessLogFormat`
- `UploadFileSizeLimitsWarning`, `UploadVideoSizeLimitWarning`
- `UserCanceledUploadMessage`, `UploadVideoFailureMessage`

### Upload Flow
1. OAuth authentication → browser-based authorization
2. Persist OAuth tokens (consumerKey, consumerSecret, Token, TokenSecret)
3. Load/create photo set (`SessionLoadCreatePhotoSet()` / `SessionStoreCreatePhotoSet()`)
4. `PreparePhotoUpload()` → `UploadChunk()` (chunked via `FootBuffer`/`HeadBuffer`)
5. `GetUploadedPhotoId()`, `GetSessionInfo()`
6. Session logging: `SessionGetItemCount()`, `SessionGetVideoCount()`, `SessionGetErrorLogCount()`

### Permissions Model
- `PermissionPublic`, `PermissionPrivate`, `PermissionFamily`, `PermissionFriends`, `PermissionFriendsAndFamily`

### Notable Patterns
- Larger `.rsrc` section (57 KB) — likely contains resource strings and icons
- Has different timestamp (533A3DCF vs 533A3DCD) — compiled separately from the other three
- Session flag system: `SessionFlagItemsInternal`, `SessionFlagOverLargeItems`, `SessionFlagVideos`

---

## 3. WLVimeoPlugin.dll

### PE Headers
| Field | Value |
|-------|-------|
| **Machine** | x86 (0x14C) |
| **Linker** | MSVC 11.00 |
| **Subsystem** | Windows CUI (3) |
| **Image Base** | 0x00400000 |
| **Image Size** | 0x3A000 (237,568 bytes, 228 KB on disk) |
| **Entry Point** | 0x0042501E (`_CorDllMain` from mscoree.dll) |
| **Timestamp** | 2014-04-01 01:17:17 (533A3DCD) |
| **CLR Header** | COM Descriptor at RVA 0x2008, size 0x48 |
| **PDB** | `WLVimeoPlugin.pdb` `{C1544937-97A3-49C5-BD2D-E571840E89F8}` |

### Sections
| Section | VA | VirtSize | RawSize | Purpose |
|---------|------|----------|---------|---------|
| .text | 0x2000 | 0x23024 | 0x23200 | IL code + metadata (143 KB) |
| .rsrc | 0x26000 | 0x11980 | 0x11A00 | Embedded resources (70 KB) |
| .reloc | 0x38000 | 0x0C | 0x200 | Base relocations |

### Exports
**No native exports.**

### Imports
- **mscoree.dll** (1 function): `_CorDllMain`

### Version Info
- **FileVersion:** 16.4.3528.331
- **ProductVersion:** 16.4.3528.331
- **FileDescription:** WLVimeoPlugin

### COM Interface
- **Implements:** `IPublishPlugin`
- **Classes:** `VimeoPlugin`, `VimeoPublishPlugin`
- **Internal API:** `VimeoApi`

### Class Architecture
```
VimeoPlugin (base plugin factory)
  +-- VimeoPublishPlugin (IPublishPlugin implementation)
  |     - PublishItem()
  |     - GetOAuthAccount(), GetOAuthUrl(), GetRequestToken()
  |     - VerifyOAuthToken(), SignOAuthRequest(), OAuthEncode()
  |     - CreateMovieUploadTicket(), CompleteUpload()
  |     - FinalizeMovieUploaded(), VerifyUpload()
  |     - UploadMovieChunk(), PrepareMovieUpload()
  |     - UpdateAccountInformation(), GetSessionInfo()
  |     - InvokeRestApiCheckResponse()
  +-- VimeoApi (Vimeo API client)
  +-- VimeoApiException (error class)
  +-- VimeoPluginResources (localized strings)
  +-- ConfigureForm (Windows Forms UI)
  |     - authAuthorizeButton, authBackButton, authCancelButton
  |     - authAccountLink, pubPublishButton
  |     - pubAccountComboBox, pubTextBoxDescription
  +-- StatusForm (upload status)
```

### Authentication
- **Method:** OAuth 1.0 (same pattern as Flickr)
- **Classes:** `consumerKey`, `consumerSecret`, `Token`, `TokenSecret`
- **Flow:** Identical to Flickr — `GetOAuthUrl()` → `GetRequestToken()` → `VerifyOAuthToken()` → `SignOAuthRequest()`

### Video Quality Constants
- `Vimeo130` (SD), `Vimeo160` (HD) — quality level identifiers
- `CanUploadSDVideos`, `CanUploadHDVideos`

### API Methods
- `CreateMovieUploadTicket()`, `CompleteUpload()`, `FinalizeMovieUploaded()`
- `UploadMovieChunk()`, `VerifyUpload()`
- `InvokeRestApiCheckResponse()`
- `ApplyMovieDescription()`, `ApplyMovieDownloadPrivacy()`
- `VideoIdToUrl()`, `CheckItemConstraints()`

### String Resources (error messages)
- `AuthenticateFailure`, `AuthenticateFailureTitle`, `AuthenticateFailureUnconfirmedEmail`
- `ErrorBadResponseFormat`, `ErrorCode`, `ErrorUploadStreamFailure`, `ErrorXmlElementMissing`
- `ExceededVideoUploadLimitError`, `ExceededVideoUploadLimitWarning`
- `UploadFailureLogFormat`, `UploadSuccessLogFormat`
- `UploadRejectedError`, `UserCanceledUploadMessage`
- `VideoResolutionChangeWarning`
- `PublishWizardDescription`, `PublishWizardPrivacyInformation`
- `PublishWizardTitle`, `PublishWizardTOS`, `PublishWizardViewTOS`

### Upload Flow
1. OAuth 1.0 authentication (identical flow to Flickr)
2. Populate accounts via `PopulateAccounts()`
3. `CreateMovieUploadTicket()` → `UploadMovieChunk()` → `CompleteUpload()` → `FinalizeMovieUploaded()`
4. `VerifyUpload()` → `VideoIdToUrl()`
5. Privacy settings: `allowDownloads`, permissions `settingsNode`
6. `UpdateAccountInformation()` for account management

### Notable Patterns
- Largest `.rsrc` section (70 KB) — the TOS/privacy text likely embedded
- Contains "PublishWizard" UI components (Wizard-style dialog flow)
- Adobe XMP metadata namespace URLs embedded (likely for video metadata serialization)
- `InitializeXml()`, `PopulatePublishFields()`, `publishParametersNode` for XML-based parameter passing

---

## 4. WLYouTubePlugin.dll

### PE Headers
| Field | Value |
|-------|-------|
| **Machine** | x86 (0x14C) |
| **Linker** | MSVC 11.00 |
| **Subsystem** | Windows CUI (3) |
| **Image Base** | 0x00400000 |
| **Image Size** | 0x22000 (139,264 bytes, 133 KB on disk) |
| **Entry Point** | 0x0041977E (`_CorDllMain` from mscoree.dll) |
| **Timestamp** | 2014-04-01 01:17:17 (533A3DCD) |
| **CLR Header** | COM Descriptor at RVA 0x2008, size 0x48 |
| **PDB** | `WLYouTubePlugin.pdb` `{F189A253-5E60-4230-9A46-CB103F3928AE}` |

### Sections
| Section | VA | VirtSize | RawSize | Purpose |
|---------|------|----------|---------|---------|
| .text | 0x2000 | 0x17784 | 0x17800 | IL code + metadata (94 KB) |
| .rsrc | 0x1A000 | 0x57A8 | 0x5800 | Embedded resources (22 KB) |
| .reloc | 0x20000 | 0x0C | 0x200 | Base relocations |

### Exports
**No native exports.**

### Imports
- **mscoree.dll** (1 function): `_CorDllMain`

### Version Info
- **FileVersion:** 16.4.3528.331
- **ProductVersion:** 16.4.3528.331
- **FileDescription:** WLYouTubePlugin

### COM Interface
- **Implements:** `IPublishPlugin`
- **Classes:** `YouTubePlugin`, `YouTubePublishPlugin`
- **Internal API:** `YouTubeApi`

### COM GUID Found
- `860ED1B9-FDFE-4930-A609-77B12B8B5610` — likely `CLSID_YouTubePlugin` or `GUID_YouTubePlugin`

### Class Architecture
```
YouTubePlugin (base plugin factory)
  +-- YouTubePublishPlugin (IPublishPlugin implementation)
  |     - PublishItem()
  |     - PrepareUpload(), UploadChunk(), GetUploadResponse()
  |     - GetAuthenticationError(), GetUserToken()
  |     - ParseAuthenticationResponse(), UserTokenIsValid()
  |     - GetClientId(), GetDeveloperKey(), SetClientId(), SetDeveloperKey()
  |     - GetUploadedVideoUrl(), GetUploadResponseError()
  |     - GetPermission(), SetPermission()
  |     - SanitizeKeywords(), JoinKeywords(), GetValidKeywords()
  |     - SetLog(), GetLog()
  +-- YouTubeApi (YouTube Data API client)
  +-- YouTubeApiException (error class)
  +-- YouTubePluginResources (localized strings)
  +-- YouTubeConfigureForm (Windows Forms UI - Wizard style)
  |     - SignInWizardCreateAccount, linkLabelCreateAccount
  |     - textBoxDescription, comboBoxPermission
  |     - pictureBoxLogo
```

### Authentication
- **Method:** Google ClientLogin (username/password/token-based legacy auth) + Developer Key
- **Credentials:** `clientId` (username), `DeveloperKey` (API key), `Token` (auth token)
- **Flow:** `SetClientId()` → `SetDeveloperKey()` → `GetUserToken()` → `ParseAuthenticationResponse()` → `UserTokenIsValid()`
- **CAPTCHA:** `https://www.google.com/accounts/DisplayUnlockCaptcha` (URL for CAPTCHA challenge)
- **Auth errors:** `BadAuthenticationError`, `AuthenticationTokenExpiredError`, `CaptchaRequiredError`, `NotVerifiedError`, `AccountDeletedError`, `AccountDisabledError`, `AccountMigratedError`, `AccountNotLinkedError`, `ServiceDisabledError`, `ServiceUnavailableError`, `TermsNotAgreedError`

### API Methods
- `GetUserToken()`, `UserTokenIsValid()`, `ParseAuthenticationResponse()`
- `GetUploadedVideoUrl()`, `GetUploadResponse()`, `GetUploadResponseError()`
- `GetClientId()`, `GetDeveloperKey()`, `SetClientId()`, `SetDeveloperKey()`
- `GetPermission()`, `SetPermission()`
- `PrepareUpload()`, `UploadChunk()`

### String Resources (error messages)
- `BadAuthenticationError`, `AuthenticationTokenExpiredError`, `CaptchaRequiredError`
- `YouTubeAuthenticationExpiredError`, `YouTubeAuthenticationInvalidError`
- `YouTubeResponseError`, `YouTubeQuotaEntriesError`, `YouTubeQuotaRecentCallsError`
- `YouTubeValidationDeprecatedError`, `YouTubeValidationInvalidCharacterError`
- `YouTubeValidationInvalidFormatError`, `YouTubeValidationInvalidValueError`
- `YouTubeValidationRequiredError`, `YouTubeValidationTooLongError`, `YouTubeValidationTooShortError`
- `AccountDeletedError`, `AccountDisabledError`, `AccountMigratedError`, `AccountNotLinkedError`
- `ServiceDisabledError`, `ServiceUnavailableError`, `FailedToLoadCategoriesError`
- `FindKeywordError`, `ItemSelectionError`, `TermsNotAgreedError`
- `UnexpectedError`, `UnknownError`, `UploadRejectedError`

### Video Quality Constants
- `YouTube130` (130p?), `YouTube160` (160kbps audio?), `YouTube180` (180p?)

### PublishWizard UI
- `PublishWizardTitle`, `PublishWizardHeading`, `PublishWizardSubHeading`
- `PublishWizardDescription`, `PublishWizardCategory`, `PublishWizardTags`
- `PublishWizardPermission`, `PublishWizardPrivate`, `PublishWizardPublic`
- `PublishWizardSafety`, `PublishWizardTOS`, `PublishWizardViewTOS`
- `PublishWizardCancel`, `PublishWizardPublish`
- `PublishWizardCategoryText`, `PublishWizardTagText`, `PublishWizardFormText`

### Upload Flow
1. ClientLogin auth (username/password → auth token)
2. Validate via `UserTokenIsValid()` and `ParseAuthenticationResponse()`
3. Show PublishWizard dialog (video metadata: title, description, category, tags, permissions)
4. Validate keywords via `SanitizeKeywords()`, `JoinKeywords()`, `GetValidKeywords()`
5. `PrepareUpload()` → `UploadChunk()` → `GetUploadResponse()`
6. Handle quota errors, validation errors, CAPTCHA challenges

---

## Common Patterns (All Four Plugins)

### PE Structure
| Feature | Pattern |
|---------|---------|
| **Image type** | PE32 x86 .NET assembly (managed code) |
| **Linker** | MSVC 11.00 (VS 2012) — same as all other WLX DLLs |
| **Subsystem** | Windows CUI (3) — all .NET DLLs |
| **Import** | Only `mscoree.dll` / `_CorDllMain` |
| **Export** | None — COM interface via .NET interop |
| **CLR Header** | COM Descriptor at RVA 0x2008, size 0x48 |
| **Sections** | 3 sections: .text, .rsrc, .reloc (identical layout pattern) |
| **Characteristics** | 0x8540: Dynamic Base + NX + No SEH + Terminal Server Aware |
| **Timestamp** | All 533A3DCD (Apr 1 2014), except Flickr 533A3DCF |
| **Version** | All 16.4.3528.331, "Windows Live (R)", Microsoft Corp. |

### Plugin Interface Architecture
All four implement the same COM interface pattern consumed by `WLXMediaPublishSubscribe.dll`:

```
IPublishPlugin (from WLXMediaPublishSubscribe)
  - PluginTitle, Key, KeywordSet
  - PublishItem(IPublishItemPropertyStore, IPublishProgressCallback)
  - CreatePlugin(), ParsePlugin(), Initialize()
  - HasPublishResults, LaunchPublishResults()
  - ShowConfigurationSettings()

ISubscribePlugin (Facebook only)
  - Subscribe(ISubscribeProgressCallback)
  - ShowSubscribe()
```

The native `WLXMediaPublishSubscribe.dll` discovers plugins via COM and calls:
1. `CreatePlugin()` — instantiate the plugin
2. `Initialize()` — setup with auth/session data
3. `ShowConfigurationSettings()` — show auth dialog
4. `PublishItem()` — upload media
5. `HasPublishResults()` / `LaunchPublishResults()` — show results

### Authentication Comparison
| Plugin | Method | Key Fields | Flow |
|--------|--------|-----------|------|
| **Facebook** | Custom REST API key/secret | APIKey, Secret, SessionKey, SessionSecret | API key → session → upload |
| **Flickr** | OAuth 1.0 | consumerKey, consumerSecret, Token, TokenSecret | OAuth URL → authorize → verify |
| **Vimeo** | OAuth 1.0 | consumerKey, consumerSecret, Token, TokenSecret | OAuth URL → authorize → verify |
| **YouTube** | Google ClientLogin + Dev Key | clientId, DeveloperKey, Token | Login → token → upload |

### Upload Mechanism (Common)
- **Transport:** `HttpWebRequest` / `HttpWebResponse` (System.Net)
- **Method:** Chunked upload via `UploadChunk()` with buffer management
- **Buffering:** `BUFFER_SIZE`, `buffer`, `bufferSize`, `FootBuffer`, `HeadBuffer`
- **Stream handling:** `GetRequestStream()`, `GetResponseStream()`
- **Error handling:** `WebException`, `GetResponseError()`, `FromResponse()`

### Session/Persistence (Common)
- **Persistence format:** XML (`sessionXml`, `persistXml`, `sessionData`)
- **Serialization:** `SerializeAccount()` / `DeserializeAccount()` pattern
- **Caching:** `UpdateCachedItems()` / `QueryCachedItems()`, `useCache`

### Error Handling (Common)
All plugins have rich exception hierarchies:
- Base: `Exception`, `WebException`, `COMException`
- Plugin-specific: `FacebookException`, `FlickrApiException`, `VimeoApiException`, `YouTubeApiException`
- Validation: Multiple `YouTubeValidation*Error` types (YouTube has the most comprehensive validation)
- Error messages: `get_*()` resource accessor pattern for localized error strings

### UI Pattern (Common)
- Windows Forms dialogs embedded in .resources
- `ConfigurationSettingsPanel` / `ConfigureForm` / `ConfigurationSettings_Load`
- Auth panels with `authAuthorizeButton`, `authBackButton`, `authCancelButton`
- `pubPublishButton` for initiating upload
- `BackgroundWorker` for async operations (`DoWork`, `RunWorkerCompleted`)
- `Timer` for polling/loading states

### Windows Live Integration (Common)
- No direct dependency on `wlidcli.dll` (these are .NET, so Live ID auth is handled by WLXMediaPublishSubscribe)
- Product name: "Windows Live (R)"
- Company: Microsoft Corp.
- No telemetry (SQM/BICI) directly — that's handled by the native framework

### File Size Order
1. Facebook: 558 KB (largest — photo + video album management)
2. Flickr: 265 KB
3. Vimeo: 228 KB
4. YouTube: 133 KB (smallest — but has most validation error types)

### Key Differences
| Aspect | Facebook | Flickr | Vimeo | YouTube |
|--------|----------|--------|-------|---------|
| **Interface** | IPublishPlugin + ISubscribePlugin | IPublishPlugin only | IPublishPlugin only | IPublishPlugin only |
| **Auth** | API key/secret + session | OAuth 1.0 | OAuth 1.0 | ClientLogin + Dev Key |
| **Media type** | Photos + Videos | Photos only | Videos only | Videos only |
| **Quality levels** | — | — | Vimeo130, Vimeo160 | YouTube130, YouTube160 |
| **Windows Forms UI** | AccountLinkPanel, VideoPanel | ConfigureForm | ConfigureForm + StatusForm | YouTubeConfigureForm (Wizard) |
| **Permissions** | Friends list | Public/Friends/Family/Private | Downloads, privacy settings | Public/Private/Safety |
| **Error granularity** | Medium | Medium | Medium | Very high (10+ validation errors) |
| **Resource section** | 17 KB | 57 KB | 70 KB | 22 KB |
| **Compiled timestamp** | 533A3DCD | 533A3DCF (different) | 533A3DCD | 533A3DCD |
