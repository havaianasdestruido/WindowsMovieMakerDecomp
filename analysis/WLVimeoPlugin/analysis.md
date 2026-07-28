# WLVimeoPlugin.dll - Deep Static Analysis

## Overview

WLVimeoPlugin.dll is a **.NET Framework 4.0 managed assembly** (C#) implementing the Vimeo video publish plugin for Windows Live Photo Gallery / Movie Maker. It provides OAuth 1.0-authenticated video upload to Vimeo's API v2.

---

## PE Headers

| Field | Value |
|-------|-------|
| **Machine** | x86 (0x14C) |
| **Linker** | MSVC 11.00 (VS 2012) |
| **Subsystem** | Windows CUI (3) — standard for .NET assemblies |
| **Image Base** | 0x00400000 |
| **Image Size** | 0x3A000 (237,568 bytes) |
| **File Size** | 228 KB on disk |
| **Entry Point** | 0x0042501E (`_CorDllMain` from mscoree.dll) |
| **Timestamp** | 2014-04-01 01:17:17 UTC (533A3DCD) |
| **DLL Characteristics** | Dynamic Base, NX Compatible, No SEH, Terminal Server Aware |
| **Stack Reserve/Commit** | 0x100000 / 0x1000 |
| **Heap Reserve/Commit** | 0x100000 / 0x1000 |

### CLR Header

| Field | Value |
|-------|-------|
| **COM Descriptor RVA** | 0x2008 (size 0x48) |
| **CLR Runtime** | v4.0.30319 |
| **Strong Name** | Signed |
| **Target Framework** | .NETFramework,Version=v4.0 |

### Sections

| Section | VA | VirtSize | RawSize | Flags | Purpose |
|---------|------|----------|---------|-------|---------|
| .text | 0x2000 | 0x23024 | 0x23200 | Code, Execute Read | IL code + .NET metadata (143 KB) |
| .rsrc | 0x26000 | 0x11980 | 0x11A00 | Initialized, Read Only | Embedded resources (70 KB) |
| .reloc | 0x38000 | 0x0C | 0x200 | Initialized, Read Only, Discardable | Base relocations |

### Debug Info

| Field | Value |
|-------|-------|
| **Format** | RSDS (PDB 2.0) |
| **GUID** | `{C1544937-97A3-49C5-BD2D-E571840E89F8}` |
| **Age** | 1 |
| **PDB File** | WLVimeoPlugin.pdb |

---

## Assembly Metadata

- **Assembly:** WLVimeoPlugin, Version=16.4.3528.331
- **Product:** Windows Live (R)
- **Copyright:** (c) Microsoft Corporation. All rights reserved.
- **Company:** Microsoft Corp.
- **Target Framework:** .NETFramework,Version=v4.0
- **ComVisible:** Yes (COM interop exposed)
- **CLSCompliant:** Yes

---

## Exports

**No native exports.** Export Directory table is zeroed. COM interop via .NET `ComVisibleAttribute`.

## Imports

- **mscoree.dll** (1 function): `_CorDllMain` — .NET CLR loader entry point

---

## Class Architecture

```
<Module>
WLVimeoPlugin.dll
├── Account
│   ├── Nsid : string
│   ├── Name : string
│   ├── Token : string
│   ├── TokenSecret : string
│   ├── QuotaTotal : long
│   ├── QuotaUsed : long
│   ├── CanUploadSDVideos : bool
│   ├── CanUploadHDVideos : bool
│   └── MaxVideoSize : long
├── VimeoPlugin
│   ├── VimeoPublishPlugin : IPublishPlugin
│   │   ├── PublishItem()
│   │   ├── HasPublishResults()
│   │   ├── ShowSummaryInformation()
│   │   ├── HasSummaryInformation()
│   │   ├── LaunchPublishResults()
│   │   ├── ShowConfigurationSettings()
│   │   ├── GetSessionInfo()
│   │   └── DisplayResults()
│   ├── VimeoApi
│   │   ├── SetOAuth()
│   │   ├── GetRequestToken()
│   │   ├── GetOAuthUrl()
│   │   ├── GetOAuthAccount()
│   │   ├── VerifyOAuthToken()
│   │   ├── UpdateAccountInformation()
│   │   ├── GetDisplayName()
│   │   ├── PrepareMovieUpload()
│   │   ├── CreateMovieUploadTicket()
│   │   ├── UploadMovieChunk()
│   │   ├── FinalizeMovieUploaded()
│   │   ├── VideoIdToUrl()
│   │   ├── VerifyUpload()
│   │   ├── CompleteUpload()
│   │   ├── ApplyMovieMetadata()
│   │   ├── ApplyMovieTitle()
│   │   ├── ApplyMovieDescription()
│   │   ├── ApplyMovieDownloadPrivacy()
│   │   ├── ApplyMoviePrivacy()
│   │   ├── ApplyMovieTags()
│   │   ├── InvokeRestApiCheckResponse()
│   │   ├── InvokeRestApi()
│   │   ├── InvokeApi()
│   │   ├── GetWebString()
│   │   ├── OAuthEncode()
│   │   ├── SignOAuthRequest()
│   │   └── FromQueryString()
│   ├── VimeoApiException : Exception
│   │   ├── ErrorCode : string
│   │   └── FromResponse()
│   ├── UploadContext
│   │   ├── Url : string
│   │   ├── Method : string
│   │   ├── Request : HttpWebRequest
│   │   ├── HeadBuffer : byte[]
│   │   ├── MovieStream : Stream
│   │   ├── FootBuffer : byte[]
│   │   ├── RequestStream : Stream
│   │   ├── CurrentPosition : long
│   │   ├── ChunkSize : int
│   │   └── PercentComplete : float
│   ├── Item (PublishItemPropertyStore)
│   │   ├── Id, FullFilePath, OriginalFileName, OriginalFileExtension
│   │   ├── PerceivedType, Title, OriginalWidth, OriginalHeight
│   │   ├── LengthMS, FileSize, Duration
│   │   ├── KeywordSet (settable), Description (settable)
│   │   ├── Privacy (settable), Url (settable)
│   │   └── publishProperties : IPublishProperties
│   ├── LogItem
│   │   ├── Successful : bool
│   │   ├── Message : string
│   │   ├── UploadedSize : long
│   │   └── Size : long
│   ├── ConfigureForm : Form (Windows Forms)
│   │   ├── authPanel, confirmPanel, pubPanel
│   │   ├── authAuthorizeButton, authBackButton, authCancelButton
│   │   ├── authAccountLink, authPictureBox
│   │   ├── confirmNextButton, confirmCancelButton
│   │   ├── confirmVerifierTextBox, confirmInfoLink
│   │   ├── pubPublishButton, pubCancelButton
│   │   ├── pubAccountComboBox, pubRemoveLink
│   │   ├── pubTextBoxTitle, pubTextBoxDescription
│   │   ├── pubLabelPrivacy, pubLinkLabelTOS
│   │   └── pictureBoxVideoFrame
│   ├── StatusForm : Form
│   │   ├── infoTextBox
│   │   ├── okButon
│   │   └── titleLabel
│   ├── SessionData
│   │   ├── sessionXml : XmlDocument
│   │   ├── User : string
│   │   ├── ItemSet : string
│   │   └── Log : string
│   ├── PersistentData
│   │   ├── persistXml : XmlDocument
│   │   ├── Users : list
│   │   └── DefaultUser : string
│   ├── LayoutHelper
│   │   ├── FontNormal, FontHeading
│   │   ├── HeadingColor : Color
│   │   ├── IsRightToLeft : bool
│   │   ├── RTLMBOptions : MessageBoxOptions
│   │   ├── Naturalize(), NaturalizeButtonRow()
│   │   └── AdjustControl()
│   ├── Privacy (Enum)
│   │   ├── Anybody = 0
│   │   ├── Nobody = 1
│   │   └── Contacts = 2
│   ├── Glink
│   │   └── GetGlinkForId()
│   └── VimeoPluginResources (ResourceManager)
└── VimeoPlugin.Properties
```

---

## Authentication: OAuth 1.0

### Hardcoded Consumer Credentials

| Field | Value |
|-------|-------|
| **Consumer Key** | `2af6ecd35087e3efaf8dd87331e4e692` |
| **Consumer Secret** | `98a4dbc9b14e76c0` |

These are Microsoft's OAuth 1.0 credentials registered with Vimeo for the Windows Live integration.

### OAuth Endpoints

| Endpoint | URL |
|----------|-----|
| **Request Token** | `http://vimeo.com/oauth/request_token` |
| **Authorize** | `http://www.vimeo.com/oauth/authorize?oauth_token={0}&permission=write` |
| **Access Token** | `http://vimeo.com/oauth/access_token` |
| **Check Token** | Method: `vimeo.oauth.checkAccessToken` |

### OAuth Parameters

- `oauth_token`, `oauth_token_secret` — request/access tokens
- `oauth_consumer_key` — Microsoft's consumer key
- `oauth_signature_method` — `HMAC-SHA1`
- `oauth_version` — `1.0`
- `oauth_nonce` — random nonce
- `oauth_timestamp` — Unix timestamp
- `oauth_callback` — `oob` (out-of-band, desktop app pattern)

### OAuth Flow

```
1. GetRequestToken() → oauth_token + oauth_token_secret
2. GetOAuthUrl() → browser opens www.vimeo.com/oauth/authorize?oauth_token=...&permission=write
3. User enters oauth_verifier in confirmVerifierTextBox
4. VerifyOAuthToken() → exchange for access token
5. GetOAuthAccount() → populate Account (Nsid, Name, Token, TokenSecret, quotas)
6. SetOAuth() → sign all subsequent API requests
```

### HMAC-SHA1 Signing

- Key: `consumerSecret&tokenSecret`
- Input: `verb&url_encoded_base_url&param1=val1&param2=val2...`
- Algorithm: `HMACSHA1` → Base64 → URL-encode

---

## API Endpoints (Vimeo API v2 REST)

### Base URL

```
http://vimeo.com/api/rest/v2
```

All API calls are signed with OAuth 1.0 and use HTTP GET (via query string) or POST.

### API Methods Used

| Method | Purpose | Parameters |
|--------|---------|------------|
| `vimeo.oauth.checkAccessToken` | Validate auth token | `oauth_token` |
| `vimeo.test.login` | Test login (debug) | — |
| `vimeo.videos.upload.getQuota` | Check upload quota | — |
| `vimeo.people.getInfo` | Get user profile | `user_id` |
| `vimeo.videos.upload.getTicket` | Get upload ticket | — |
| `vimeo.videos.upload.verifyChunks` | Verify uploaded chunks | `ticket`, `chunks` |
| `vimeo.videos.upload.complete` | Finalize upload | `ticket`, `filename`, `video_id` |
| `vimeo.videos.setTitle` | Set video title | `video_id`, `title` |
| `vimeo.videos.setDescription` | Set video description | `video_id`, `description` |
| `vimeo.videos.setDownloadPrivacy` | Enable/disable downloads | `video_id`, `download` |
| `vimeo.videos.setPrivacy` | Set privacy level | `video_id`, `privacy` |
| `vimeo.videos.addTags` | Add tags to video | `video_id`, `tags` |

### Response Format

```xml
<!-- Success -->
<rsp stat="ok">...</rsp>

<!-- Failure -->
<rsp stat="fail">
  <err code="..." msg="..."/>
</rsp>
```

Parsed via XPath: `rsp[@stat='ok']` and `rsp[@stat='fail']/err.msg.code`

### Upload Endpoint (Chunked)

```
POST http://vimeo.com/
Content-Type: multipart/form-data; boundary={0}
```

Multipart form fields:
- `--{0}` boundary delimiter
- `Content-Disposition: form-data; name="{0}"` for each field
- Chunked upload with `ticket_id` + `chunk_id` tracking

### Quota Response Fields

- `upload_space.max` — max upload space
- `upload_space.free` — free space
- `hd_quota` — HD upload quota (1 = allowed)
- `sd_quota` — SD upload quota

### User Info Fields

- `user.id` — Vimeo user ID
- `user.person/display_name` — display name

---

## Upload Flow (Detailed)

```
1. Authorize (OAuth 1.0 flow)
   ├── GetRequestToken()
   ├── OpenBrowser(GetOAuthUrl()) → user authorizes
   ├── VerifyOAuthToken(verifier)
   └── GetOAuthAccount() → Account with quotas

2. Prepare Upload
   ├── CheckItemConstraints() — validate file size vs. quotas
   ├── CreateMovieUploadTicket() → ticket with host, endpoint, max_file_size
   └── PrepareMovieUpload() → setup UploadContext (HttpWebRequest)

3. Chunked Upload
   ├── UploadMovieChunk() — sends file chunks
   │   ├── HeadBuffer + MovieStream + FootBuffer
   │   ├── HttpWebRequest with Content-Length
   │   └── Progress via SetPublishProgress()
   └── VerifyUpload() — verify all chunks received

4. Finalize
   ├── CompleteUpload() → video_id
   ├── FinalizeMovieUploaded()
   ├── VideoIdToUrl() → http://vimeo.com/{video_id}
   └── ApplyMovieMetadata()
       ├── ApplyMovieTitle()
       ├── ApplyMovieDescription()
       ├── ApplyMoviePrivacy()
       ├── ApplyMovieDownloadPrivacy()
       └── ApplyMovieTags()

5. Results
   ├── HasPublishResults() → true
   ├── LaunchPublishResults() → StatusForm
   └── DisplayResults() → log text
```

---

## Glink (Windows Live Integration)

```
http://g.live.com/dmxpgen-us/{0}
```

The `Glink` class resolves Windows Live IDs to profile display names via this URL pattern.

---

## Persistence Model

### SessionData (XML)

```xml
<persistXml>
  <settings>
    <users>
      <user>
        <sessionData>
          <!-- Session tokens, user info -->
        </sessionData>
      </user>
    </users>
    <defaultUser>...</defaultUser>
  </settings>
</persistXml>
```

- Stored in `settingsNode`, `users`, `defaultUser`
- Methods: `Save()`, `InitializeXml()`, `GetUsers()`, `SetUsers()`, `GetDefaultUser()`

### PersistentData (XML)

Stores OAuth credentials per account:
- User name, Token, TokenSecret

---

## UI Components (Windows Forms)

### ConfigureForm — 3-panel wizard

| Panel | Purpose | Controls |
|-------|---------|----------|
| **authPanel** | OAuth authorization | authAuthorizeButton, authBackButton, authCancelButton, authPictureBox, authAccountLink |
| **confirmPanel** | Enter OAuth verifier | confirmNextButton, confirmCancelButton, confirmVerifierTextBox, confirmInfoLink |
| **pubPanel** | Publish settings | pubPublishButton, pubCancelButton, pubAccountComboBox, pubRemoveLink, pubTextBoxTitle, pubTextBoxDescription, pubLabelPrivacy, pubLinkLabelTOS |

### StatusForm — Upload progress

- `infoTextBox` — upload status log
- `okButon` — close button
- `titleLabel` — status heading

### Panel Navigation

```
ShowPanel(authPanel) → Authorize
ShowPanel(confirmPanel) → Enter verifier
ShowPanel(pubPanel) → Configure & publish
```

### LayoutHelper

- RTL (Right-to-Left) support for Arabic/Hebrew
- `FontNormal`, `FontHeading` — Segoe UI fonts
- `HeadingColor` — styled header text
- `Naturalize()`, `AdjustControl()` — DPI/layout adaptation

---

## Video Quality Levels

| Constant | Meaning |
|----------|---------|
| `Vimeo130` | SD quality level identifier |
| `Vimeo160` | HD quality level identifier |

- `CanUploadSDVideos` / `CanUploadHDVideos` — per-account capability flags
- `VideoResolutionChangeWarning` — displayed when HD video downgraded to SD due to quota

---

## Error Handling

### VimeoApiException

- `ErrorCode` — API error code from `<rsp stat="fail"><err code="..."/>`
- `FromResponse()` — factory method from XML response

### Error Strings

| Key | Purpose |
|-----|---------|
| `AuthenticateFailure` | OAuth authentication failed |
| `AuthenticateFailureTitle` | Auth failure dialog title |
| `AuthenticateFailureUnconfirmedEmail` | Email not confirmed |
| `ErrorBadResponseFormat` | Unexpected API response |
| `ErrorUploadStreamFailure` | Upload stream error |
| `ErrorXmlElementMissing` | Expected XML element not found |
| `ExceededVideoUploadLimitError` | Video exceeds upload limit |
| `ExceededVideoUploadLimitWarning` | Warning about upload limits |
| `InitializePubPanelFailureFormat` | Failed to init publish panel |
| `InitializePubPanelFailureTitle` | Init failure title |
| `UploadFailureLogFormat` | Upload failure format string |
| `UploadRejectedError` | Upload rejected by Vimeo |
| `UploadSuccessLogFormat` | Upload success format string |
| `UserCanceledUploadMessage` | User cancelled |
| `VideoResolutionChangeWarning` | HD→SD downgrade warning |

### Error Constants

- `SERVICE_ERROR`
- `INVALID_AUTH_TOKEN`
- `MAX_FILE_SIZE_EXCEEDED`

---

## Embedded Resources

| Resource | Content |
|----------|---------|
| `VimeoPlugin.ConfigureForm.resources` | Configure form layout and strings |
| `VimeoPlugin.StatusForm.resources` | Status form layout and strings |
| `VimeoPlugin.Properties.VimeoPluginResources.resources` | Localized UI strings |
| Icon resources | `PluginIcon` (Vimeo icon), `Vimeo130`, `Vimeo160` (quality icons) |
| Bitmap resources | `Vimeo130`, `Vimeo160` — quality indicator images |

---

## Privacy Model

| Value | Meaning |
|-------|---------|
| `Anybody` (0) | Public — anyone can view |
| `Nobody` (1) | Private — only owner |
| `Contacts` (2) | Contacts only |

Additional: `allowDownloads` flag controls whether download is permitted.

---

## Security Notes

1. **OAuth Consumer Credentials Hardcoded:** Consumer key (`2af6ecd...`) and secret (`98a4dbc...`) are embedded in the binary. These are Microsoft's registered credentials with Vimeo.
2. **HMAC-SHA1 Signing:** Standard OAuth 1.0 signing with `consumerSecret&tokenSecret` as the key.
3. **OAuth Callback = `oob`:** Desktop app pattern — verifier displayed in browser, user copies back.
4. **HTTP (not HTTPS) endpoints:** OAuth endpoints use `http://` not `https://` — consistent with 2012-era Vimeo API.
5. **No certificate pinning** — relies on system trust store.

---

## Comparison with Flickr Plugin

WLVimeoPlugin and WLFlickrPlugin share nearly identical architecture:
- Both use OAuth 1.0 with HMAC-SHA1
- Both have consumer key/secret hardcoded
- Both use `oob` callback for desktop auth
- Both use chunked upload with `HttpWebRequest`
- Vimeo adds: quota management, privacy settings, download permissions, HD/SD quality levels
- Flickr adds: photo sets, friends/family permissions, photo-specific handling

---

## Comparison with WLFacebookPlugin

- Facebook uses custom REST API key/secret (not OAuth 1.0)
- Facebook supports both photos and videos; Vimeo is videos only
- Facebook has subscribe functionality; Vimeo is publish-only
- Vimeo has richer privacy model (3 levels + download permission)
