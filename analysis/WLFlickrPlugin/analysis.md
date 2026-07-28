# WLFlickrPlugin.dll — Deep Static Analysis

## Overview

WLFlickrPlugin.dll is a **.NET Framework 4.0 managed assembly** (C#) implementing the Flickr photo/video publish plugin for Windows Live Photo Gallery. It provides OAuth 1.0a-authenticated photo and video upload to Flickr's REST API.

---

## PE Headers

| Field | Value |
|-------|-------|
| **Machine** | x86 (0x14C) |
| **Linker** | MSVC 11.00 (VS 2012) |
| **Subsystem** | Windows CUI (3) — standard for .NET assemblies |
| **Image Base** | 0x00400000 |
| **Image Size** | 0x44000 (278,528 bytes) |
| **File Size** | 267 KB on disk |
| **Entry Point** | 0x0043195E (`_CorDllMain` from mscoree.dll) |
| **Timestamp** | 2014-04-01 01:17:19 UTC (533A3DCF) |
| **DLL Characteristics** | Dynamic Base, NX Compatible, No SEH, Terminal Server Aware |
| **Stack Reserve/Commit** | 0x100000 / 0x1000 |
| **Heap Reserve/Commit** | 0x100000 / 0x1000 |

### CLR Header

| Field | Value |
|-------|-------|
| **COM Descriptor RVA** | 0x2008 (size 0x48) |
| **CLR Runtime** | v4.0.30319 |
| **Strong Name** | Signed (PublicKeyToken=31bf3856ad364e35) |
| **Target Framework** | .NETFramework,Version=v4.0 |

### Sections

| Section | VA | VirtSize | RawSize | Flags | Purpose |
|---------|------|----------|---------|-------|---------|
| .text | 0x2000 | 0x2F964 | 0x2FA00 | Code, Execute Read | IL code + .NET metadata (195 KB) |
| .rsrc | 0x32000 | 0xE518 | 0xE600 | Initialized, Read Only | Embedded resources (57 KB) |
| .reloc | 0x42000 | 0x0C | 0x200 | Initialized, Read Only, Discardable | Base relocations (512 B) |

### Debug Info

| Field | Value |
|-------|-------|
| **Format** | RSDS (PDB 2.0) |
| **GUID** | `{E4AC91BB-B3FB-4694-BF4F-1532161825B5}` |
| **Age** | 1 |
| **PDB File** | WLFlickrPlugin.pdb |

---

## Assembly Metadata

- **Assembly:** WLFlickrPlugin, Version=16.4.3528.331
- **Product:** Windows Live (R)
- **Copyright:** (c) Microsoft Corporation. All rights reserved.
- **Company:** Microsoft Corp.
- **Target Framework:** .NETFramework,Version=v4.0
- **Strong Name:** Signed (PublicKeyToken=31bf3856ad364e35)

---

## Exports

**No native exports.** Export Directory table is zeroed.

## Imports

- **mscoree.dll** (1 function): `_CorDllMain` — .NET CLR loader entry point

---

## Class Architecture

```
WLFlickrPlugin.dll
├── FlickrPlugin
│   ├── FlickrPublishPlugin : IPublishPlugin
│   │   ├── ShowConfigurationSettings(parent, sessionXml, persistXml, publishProperties) → bool
│   │   ├── PublishItem(parent, mediaObjectId, stream, sessionXml, publishProperties, callback, cancelEvent) → bool
│   │   ├── HasSummaryInformation(sessionXml) → bool
│   │   ├── ShowSummaryInformation(parent, sessionXml)
│   │   ├── HasPublishResults(sessionXml) → bool
│   │   └── LaunchPublishResults(sessionXml)
│   ├── FlickrApi
│   │   ├── consumerKey : string = "5cc21a1294d0be0fb5567ec2bc6ff847"
│   │   ├── consumerSecret : string = "44018e234dacfc3f"
│   │   ├── token : string (runtime)
│   │   ├── tokenSecret : string (runtime)
│   │   ├── parameters : SortedDictionary (OAuth params cache)
│   │   ├── unreservedChars : string (RFC 3986 unreserved chars)
│   │   ├── SetOAuth(token, tokenSecret)
│   │   ├── GetRequestToken() → fetches oauth_token + oauth_token_secret
│   │   ├── GetOAuthUrl() → authorization URL string
│   │   ├── GetOAuthAccount(verifier) → Account
│   │   ├── VerifyOAuthToken(token, tokenSecret) → bool
│   │   ├── GetPhotoSetsList(user) → List<PhotoSet>
│   │   ├── CreatePhotoSet(title, description, primaryPhotoId) → photoset ID
│   │   ├── AddPhotoToPhotoSet(photosetId, photoId)
│   │   ├── GetAccountUploadStatus(user) → Account (quota refresh)
│   │   ├── PreparePhotoUpload(photoStream, photoAttributes, photoPermissions) → UploadContext
│   │   ├── UploadChunk(cookie) → bool
│   │   ├── GetUploadedPhotoId(cookie) → string
│   │   ├── GetUploadedImagesUrl(photoids) → flickr URL string
│   │   ├── StatusOk(response XmlDocument) → bool
│   │   ├── GetErrorInfo(response, &msg, &code)
│   │   ├── InvokeRestApi() → XmlDocument
│   │   ├── InvokeApi(url) → response string
│   │   ├── GetWebString(url, &result) → bool
│   │   ├── OAuthEncode(data) → URL-encoded string
│   │   ├── SignOAuthRequest(verb, url) — HMAC-SHA1 signing
│   │   ├── FromQueryString(queryString) → SortedDictionary
│   │   └── CalculateQueryString() → string
│   ├── FlickrApiException : Exception
│   │   ├── SERVICE_ERROR = -2147209457 (0x80072A0F)
│   │   ├── INVALID_AUTH_TOKEN = -2147209421 (0x80072A33)
│   │   ├── MAX_FILE_SIZE_EXCEEDED = -2147209397 (0x80072A4B)
│   │   └── FromResponse(response XmlDocument) → FlickrApiException
│   ├── UploadContext
│   │   ├── Url, Method, Request : HttpWebRequest
│   │   ├── HeadBuffer : byte[], PhotoStream : Stream, FootBuffer : byte[]
│   │   ├── RequestStream : Stream
│   │   ├── CurrentPosition : long, ChunkSize : int
│   │   └── PercentComplete : int (read-only)
│   ├── Account
│   │   ├── Nsid, Name, Token, TokenSecret : string
│   │   ├── QuotaTotalKb, QuotaUsedKb, MaxPhotoSizeKb : int
│   │   ├── CreatedPhotoSets, RemainingPhotoSets : int
│   │   ├── CanUploadVideos : bool
│   │   ├── MaxVideoSizeKb : int
│   │   └── VideosUploaded, VideosRemaining : int
│   ├── PhotoSet
│   │   ├── Id, Title, Description : string
│   ├── PhotoAttributes
│   │   ├── Id, Filename, Title, Description : string
│   │   └── Tags : List<string>
│   ├── Permissions
│   │   ├── IsPublic, IsFriend, IsFamily : bool
│   ├── ConfigureForm : Form (3-panel wizard)
│   │   ├── ConfigurePanel enum: Authorize=0, Confirm=1, Publish=2
│   │   ├── ValidateResult enum: Continue=0, Exit=1, DoNothing=2
│   │   ├── authPanel — OAuth authorization (authorize button, back, cancel, account link)
│   │   ├── confirmPanel — OAuth verifier entry (next, cancel, verifier textbox, info link)
│   │   └── pubPanel — Publish settings (account, permissions, photo size, photo set)
│   ├── PhotoSetForm : Form
│   │   └── GetNameAndDescription(parent, &title, &description) → bool
│   ├── StatusForm : Form
│   │   ├── infoTextBox, okButon, titleLabel
│   │   └── DisplayResults(parent, sessionXml)
│   ├── XmlHelper (static utility)
│   │   ├── SessionLoadOAuth, SessionStoreOAuth
│   │   ├── SessionLoadPermissions, SessionStorePermissions
│   │   ├── SessionLoadCreatePhotoSet, SessionStoreCreatePhotoSet
│   │   ├── SessionLoadUsePhotoSet, SessionStoreUsePhotoSet
│   │   ├── SessionStoreDimensions, SessionLoadItemInfo
│   │   ├── SessionStorePhotoId, SessionStoreErrorCode
│   │   ├── SessionLoadLog, SessionLog, SessionFlagVideos
│   │   ├── SessionGetItemCount, SessionGetVideoCount
│   │   ├── PersistInitializeDocument, PersistLoadUsers
│   │   ├── PersistUpdateUser, PersistLoadSelectedUser
│   │   └── PersistStoreSelectedUser, PersistRemoveUser
│   ├── LayoutHelper : (abstract)
│   │   ├── FontNormal, FontHeading : Font
│   │   ├── HeadingColor : Color
│   │   ├── IsRightToLeft : bool
│   │   ├── RTLMBOptions : MessageBoxOptions
│   │   ├── GetFont(), Naturalize(), NaturalizeButtonRow()
│   │   └── AdjustControl()
│   └── Properties.Resources (ResourceManager)
└── FlickrPlugin.Properties
```

---

## Authentication: OAuth 1.0a (3-legged)

### Hardcoded Consumer Credentials

| Field | Value |
|-------|-------|
| **Consumer Key** | `5cc21a1294d0be0fb5567ec2bc6ff847` |
| **Consumer Secret** | `44018e234dacfc3f` |

These are Microsoft's OAuth 1.0 credentials registered with Flickr for the Windows Live integration.

### OAuth Endpoints

| Endpoint | URL |
|----------|-----|
| **Request Token** | `http://www.flickr.com/services/oauth/request_token` |
| **Authorize** | `http://www.flickr.com/services/oauth/authorize?oauth_token={0}&perms=write` |
| **Access Token** | `http://www.flickr.com/services/oauth/access_token` |

### OAuth Parameters

- `oauth_token`, `oauth_token_secret` — request/access tokens
- `oauth_consumer_key` — `5cc21a1294d0be0fb5567ec2bc6ff847`
- `oauth_signature_method` — `HMAC-SHA1`
- `oauth_version` — `1.0`
- `oauth_nonce` — random nonce (System.Guid.NewGuid().ToString("N"))
- `oauth_timestamp` — Unix timestamp (DateTime.UtcNow epoch seconds)
- `oauth_callback` — `oob` (out-of-band, desktop app pattern)

### OAuth Flow

```
1. GetRequestToken()
   ├── HTTP GET to request_token endpoint with OAuth 1.0 signed header
   ├── Response: oauth_token=xxx&oauth_token_secret=xxx&oauth_callback_confirmed=true
   └── Store token + token_secret

2. GetOAuthUrl()
   └── Returns: http://www.flickr.com/services/oauth/authorize?oauth_token={token}&perms=write

3. User opens browser to authorize URL → authorizes → gets 7-digit verifier code

4. GetOAuthAccount(verifier)
   ├── HTTP GET to access_token endpoint with OAuth 1.0 signed header + oauth_verifier
   ├── Response: oauth_token=xxx&oauth_token_secret=xxx&user_nsid=xxx&username=xxx
   └── Returns Account with full name + token + token_secret

5. VerifyOAuthToken(token, tokenSecret)
   ├── SetOAuth(token, tokenSecret)
   ├── Calls flickr.test.login API
   └── Check response stat='ok'
```

### HMAC-SHA1 Signing

- Key: `consumerSecret&tokenSecret`
- Input: `verb&url_encoded_base_url&param1=val1&param2=val2...` (sorted parameters)
- Nonce: 32-char hex from `Guid.NewGuid().ToString("N")`
- Timestamp: seconds since Unix epoch
- Signature algorithm: `HMACSHA1` → Base64 → URL-encode (custom encoder using unreserved chars)

---

## REST API

### Base URL

```
http://api.flickr.com/services/rest/
```

All API calls are signed with OAuth 1.0a and use `flickr.xxx` method format. Parameters are sent in the query string or URL depending on HTTP verb.

### API Methods Used

| Method | Purpose | Parameters |
|--------|---------|------------|
| `flickr.test.login` | Validate auth token | — |
| `flickr.people.getUploadStatus` | Check user quota | — |
| `flickr.photosets.getList` | List photo sets | — |
| `flickr.photosets.create` | Create new set | `title`, `description`, `primary_photo_id` |
| `flickr.photosets.addPhoto` | Add photo to set | `photoset_id`, `photo_id` |

### Response Format

```xml
<!-- Success -->
<rsp stat="ok">...</rsp>

<!-- Failure -->
<rsp stat="fail">
  <err code="..." msg="..."/>
</rsp>
```

Parsed via XPath: `/rsp[@stat='ok']` and `/rsp[@stat='fail']/err`

### Upload Endpoint

```
POST http://api.flickr.com/services/upload/
Content-Type: multipart/form-data; boundary={boundary}
```

Multi-part form upload with:
- `Content-Disposition: form-data; name="{field_name}"` for auth/meta fields
- `Content-Disposition: form-data; name="photo"; filename="{filename}"` for photo binary
- `Content-Type: image/jpeg` for photo content
- Chunked upload via `HttpWebRequest` with `HeadBuffer` + `PhotoStream` + `FootBuffer`

### Upload Chunk Size

The chunk size (`UploadContext.ChunkSize`) controls how much data is sent per `HttpWebRequest`. The upload is split into head/middle/foot buffer sections.

---

## Upload Flow (Detailed)

```
1. Authorize (OAuth 1.0a flow)
   ├── GetRequestToken()
   ├── OpenBrowser(GetOAuthUrl()) → user authorizes
   ├── GetOAuthAccount(verifier)
   └── VerifyOAuthToken() → confirm token works

2. Configure Publish
   ├── ShowPanel(pubPanel)
   ├── Select account from pubAccountComboBox
   ├── PopulateAccounts() — load persisted OAuth accounts
   ├── PopulateSets(user) — flickr.photosets.getList
   ├── PopulateQuota(user) — flickr.people.getUploadStatus
   ├── Permissions: Public / Private / Friends / Family
   ├── Photo Size: 640 / 1024 / 1600 / 2048 / Original
   ├── Photo Set: existing set or create new
   └── ValidateItemSizes() — check file size vs. account limits

3. Publish
   ├── For each item:
   │   ├── PreparePhotoUpload(stream, attributes, permissions)
   │   │   └── Builds multipart request boundary
   │   ├── UploadChunk(cookie) — sends chunk, tracks progress
   │   └── GetUploadedPhotoId(cookie) — extracts <photoid> from response
   ├── If creating photo set: CreatePhotoSet() + AddPhotoToPhotoSet()
   └── Log success/failure per item

4. Results
   ├── HasPublishResults() → true if any items uploaded
   ├── LaunchPublishResults() → opens:
   │   http://www.flickr.com/tools/uploader_edit.gne?ids={comma-separated photoids}
   └── ShowSummaryInformation() → StatusForm with log
```

---

## Supported Photo File Sizes

| Label | Max Dimension |
|-------|---------------|
| **640 pixels** | 640px |
| **1024 pixels** | 1024px |
| **1600 pixels** | 1600px |
| **2048 pixels** | 2048px |
| **Original size** | No resize |

---

## Permission Model (Privacy)

| Setting | IsPublic | IsFriend | IsFamily |
|---------|----------|----------|----------|
| Public | true | — | — |
| Private (Just Me) | false | false | false |
| Private (Friends only) | false | true | false |
| Private (Family only) | false | false | true |
| Private (Family, Friends only) | false | true | true |

---

## Session Persistence (XML)

### Session XML Paths

| Path | Purpose |
|------|---------|
| `/PhotoGalleryPublishSession` | Root |
| `/PhotoGalleryPublishSession/OAuth` | OAuth token + secret |
| `/PhotoGalleryPublishSession/PublishParameters/Permissions` | is_public, is_friend, is_family |
| `/PhotoGalleryPublishSession/PublishParameters/CreatePhotoSet` | Create new set (title, description) |
| `/PhotoGalleryPublishSession/PublishParameters/UsePhotoSet` | Existing set (id, title) |
| `/PhotoGalleryPublishSession/ItemSet` | Uploaded items |
| `/PhotoGalleryPublishSession/ItemSet/Item/PhotoId` | Per-item uploaded photo ID |
| `/PhotoGalleryPublishSession/Log` | Error/success log |

### Persist XML Paths

| Path | Purpose |
|------|---------|
| `/Settings/Defaults` | Default nsid |
| `/Settings/Users` | Saved accounts |
| `/Settings/Users/User[@Nsid='{0}']` | Per-user token, secret, name |

---

## UI Components (Windows Forms)

### ConfigureForm — 3-panel wizard

| Panel | Purpose | Controls |
|-------|---------|----------|
| **authPanel** | OAuth authorization | authAuthorizeButton, authBackButton, authCancelButton, authPictureBox, authAccountLink, authAccountLabel |
| **confirmPanel** | Enter OAuth verifier | confirmNextButton, confirmCancelButton, confirmVerifierTextBox, confirmInfoLink, confirmInfoLabel |
| **pubPanel** | Publish settings | pubPublishButton, pubCancelButton, pubAccountComboBox, pubRemoveLink, pubPermissionsComboBox, pubPhotoSizeComboBox, pubPhotoSetComboBox, pubInfoLink, pubStatusLabel |

### Panel Navigation

```
ShowPanel(Authorize=0) → OAuth auth page
ShowPanel(Confirm=1) → Enter verifier code
ShowPanel(Publish=2) → Configure & publish
```

### PhotoSetForm — Dialog

- GetNameAndDescription(parent, &title, &description) → bool
- Controls: nameTextBox (title), descriptionTextBox, createButton, cancelButton

### StatusForm — Results

- infoTextBox — scrollable log of upload results
- okButon — close
- titleLabel — "Detailed Publish Information"

---

## Embedded Resources

| Resource | Content |
|----------|---------|
| `FlickrPlugin.ConfigureForm.resources` | Configure form layout, icons |
| `FlickrPlugin.PhotoSetForm.resources` | Photo set form layout |
| `FlickrPlugin.StatusForm.resources` | Status form layout |
| `FlickrPlugin.Properties.FlickrPluginResources.resources` | Localized UI strings (en, etc.) |

---

## Error Handling

### FlickrApiException

| Constant | Value (HRESULT) | Meaning |
|----------|-----------------|---------|
| `SERVICE_ERROR` | `-2147209457` (0x80072A0F) | Generic API service error |
| `INVALID_AUTH_TOKEN` | `-2147209421` (0x80072A33) | OAuth token invalid/expired |
| `MAX_FILE_SIZE_EXCEEDED` | `-2147209397` (0x80072A4B) | Photo or video exceeds size limit |

### Error Strings

| Key | Purpose |
|-----|---------|
| `AuthenticateFailure` | OAuth authentication failed |
| `AuthenticateFailureTitle` | Auth failure dialog title |
| `AuthenticateFrobFailureFormat` | Frob (request token) failure format |
| `ErrorBadResponseFormat` | Unexpected API response |
| `ErrorXmlElementMissing` | Expected XML element not found |
| `ExceededPhotoUploadLimit` | Photo monthly limit reached |
| `ExceededPhotoUploadLimitButNotVideos` | Photo limit reached but videos OK |
| `ExceededVideoUploadLimit` | Video monthly limit reached |
| `ExceededVideoUploadLimitButNotPhotos` | Video limit reached but photos OK |
| `InitializePubPanelFailureFormat` | Publish panel init failure |
| `InitializePubPanelFailureTitle` | Init failure dialog title |
| `UploadFailureLogFormat` | Per-item upload failure log |
| `UploadSuccessLogFormat` | Per-item upload success log |
| `UploadFileSizeLimitsWarning` | Photo exceeds account size limit |
| `UploadFileSizeLimitsWarningAll` | All photos exceed size limit |
| `UploadVideoSizeLimitWarning` | Video exceeds size limit |
| `UploadVideoFileSizeLimitsWarningAll` | All videos exceed size limit |
| `UploadVideoFailureMessage` | Video upload failure message |
| `UserCanceledUploadMessage` | User cancelled upload |

---

## Format Size Display

The `StrFormatByteSizeW` native method is imported from `shlwapi.dll` (via P/Invoke) for human-readable file size formatting.

---

## LayoutHelper

- Supports RTL (Right-to-Left) layout for Arabic/Hebrew
- FontNormal = `Segoe UI` at `FontSizeNormal`
- FontHeading = `Segoe UI` at `FontSizeHeading`
- HeadingColor — styled header text color
- Naturalize() / NaturalizeButtonRow() — layout adaptation

---

## Security Notes

1. **OAuth Consumer Credentials Hardcoded:** Consumer key (`5cc21a1294d0be0fb5567ec2bc6ff847`) and secret (`44018e234dacfc3f`) are embedded in the binary. These are Microsoft's registered Flickr credentials.
2. **HMAC-SHA1 Signing:** Standard OAuth 1.0a signing with `consumerSecret&tokenSecret` as the key.
3. **OAuth Callback = `oob`:** Desktop app pattern — verifier displayed in browser, user copies back.
4. **HTTP (not HTTPS) endpoints:** Flickr upload endpoint uses `http://` not `https://` — consistent with 2012-era Flickr API.
5. **No certificate pinning** — relies on system trust store.
6. **Photo Stream not encrypted** — plaintext HTTP upload.

---

## Comparison with WLFacebookPlugin

- Flickr uses OAuth 1.0a; Facebook uses custom REST API key/secret (not OAuth)
- Both support photos and videos
- Flickr has photo set management; Facebook has album management
- Flickr supports both friends/family privacy; Facebook uses per-album privacy
- Facebook uses Windows Live Glink integration; Flickr does not
- Both share the same `LayoutHelper` and `XmlHelper` pattern

## Comparison with WLVimeoPlugin

- Both use OAuth 1.0 with HMAC-SHA1
- Both have consumer key/secret hardcoded
- Both use `oob` callback for desktop auth
- Both use chunked upload with `HttpWebRequest`
- Flickr supports photos + videos; Vimeo is videos only
- Flickr adds: photo sets, friends/family permissions, photo size resizing
- Vimeo adds: HD/SD quality levels, download permissions, privacy levels

---

## Categorization

- **File:** `WLFlickrPlugin.dll`
- **Classification:** Windows Live Photo Gallery / Movie Maker — Flickr Publish
- **Type:** .NET Framework 4.0 managed assembly (C#), 32-bit x86
- **Size:** 267 KB (0x44000 bytes)
- **PDB:** Yes (WLFlickrPlugin.pdb)
- **Signed:** Yes (Microsoft Authenticode)
- **COM Visible:** Yes (ComVisibleAttribute)
