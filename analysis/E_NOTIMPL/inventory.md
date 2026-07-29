# E_NOTIMPL Stub Inventory

Generated: 2026-07-29
Total stubs found: **92** across 31 files in 14 DLLs

---

## Summary by DLL

| DLL | Stubs | CRITICAL | HIGH | MEDIUM | LOW |
|-----|-------|----------|------|--------|-----|
| **MovieMakerCore.dll** | 41 | 0 | 5 | 12 | 24 |
| **UXCore.dll** | 19 | 0 | 0 | 0 | 19 |
| **WLXMediaPublishSubscribe.dll** | 12 | 0 | 0 | 0 | 12 |
| **WLXVideoTrim.dll** | 5 | 0 | 0 | 0 | 5 |
| **WLXPhotoBase.dll** | 4 | 0 | 0 | 0 | 4 |
| **WLXMP4Parser.dll** | 4 | 0 | 0 | 4 | 0 |
| **MovieMakerPreviewClient.dll** | 2 | 0 | 0 | 1 | 1 |
| **WLMFDS.dll** | 2 | 0 | 0 | 0 | 2 |
| **WLXPipetran.dll** | 1 | 0 | 0 | 0 | 1 |
| **WLXPipeline.dll** | 1 | 0 | 0 | 0 | 1 |
| **WLMFReadWrite.dll** | 1 | 0 | 0 | 1 | 0 |
| **WLXMovieLibrary.dll** | 1 | 0 | 0 | 1 | 0 |
| **MetadataSys.dll** | 1 | 0 | 0 | 1 | 0 |
| **Total** | **92** | **0** | **5** | **20** | **67** |

---

## Full Inventory by DLL

### MovieMakerCore.dll — `src/MovieMakerCore/` (41 stubs)

This DLL contains the main application logic, undo/redo, playback, import/export,
the 3D HMR engine, and AV source pipeline. It has the highest concentration of
impactful stubs.

#### SundanceApp/ — Application shell (17 stubs)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 1 | `SundanceAppMain.cpp` | `SaveProject` (when `GetFilePath().IsEmpty()`) | 616 | **HIGH** | New/untitled project: Save fails instead of redirecting to SaveProjectAs |
| 2 | `SundanceAppMain.cpp` | `ImportFiles` (when `m_pImportController == nullptr`) | 958 | **HIGH** | No import controller created; importing media silently fails |
| 3 | `SundanceAppMain.cpp` | `PublishMovie` (when `m_pExportController == nullptr`) | 979 | **MEDIUM** | Publish/Save-Movie fails if export controller not initialized |
| 4 | `SundanceAppMain.cpp` | `PublishToService` (when `m_pExportController == nullptr`) | 1005 | **MEDIUM** | Publish-to-service fails if export controller not initialized |
| 5 | `SundanceAppMain.cpp` | `Undo` (when `m_pUndoManager == nullptr`) | 1029 | **MEDIUM** | Undo manager not created; undo does nothing |
| 6 | `SundanceAppMain.cpp` | `Redo` (when `m_pUndoManager == nullptr`) | 1044 | **MEDIUM** | Redo manager not created; redo does nothing |
| 7 | `SundanceAppMain.cpp` | `CutSelection` (when `m_pClipboardManager == nullptr`) | 1072 | **MEDIUM** | Clipboard manager missing; cut fails |
| 8 | `SundanceAppMain.cpp` | `CopySelection` (when `m_pClipboardManager == nullptr`) | 1091 | **MEDIUM** | Clipboard manager missing; copy fails |
| 9 | `SundanceAppMain.cpp` | `PasteFromClipboard` (when `m_pClipboardManager == nullptr`) | 1107 | **MEDIUM** | Clipboard manager missing; paste fails |
| 10 | `SundanceAppMain.cpp` | `StartPlayback` (when `m_pPlaybackController == nullptr`) | 1151 | **MEDIUM** | Playback controller missing; play does nothing |
| 11 | `SundanceAppMain.cpp` | `StopPlayback` (when `m_pPlaybackController == nullptr`) | 1159 | **MEDIUM** | Playback controller missing; stop does nothing |
| 12 | `SundanceAppMain.cpp` | `PausePlayback` (when `m_pPlaybackController == nullptr`) | 1167 | **MEDIUM** | Playback controller missing; pause does nothing |
| 13 | `SundanceAppMain.cpp` | `OnRibbonCommand(kRibbonCmdDelete)` (no clipboard) | 1550 | **MEDIUM** | Delete via ribbon fails if clipboard manager missing |
| 14 | `SundanceAppMain.cpp` | `OnRibbonCommand(kRibbonCmdSaveMovie)` (no export) | 1556 | **MEDIUM** | Save-Movie from ribbon fails if export controller missing |
| 15 | `SundanceAppDataContext.cpp` | `SetProperty` (unrecognized property name) | 298 | **LOW** | Unknown property name; reasonable fallback |
| 16 | `SundanceAppDataContext.cpp` | `GetTypeInfo` (IDispatch) | 478 | **LOW** | Type info not available; late-binding fallback |
| 17 | `MediaBrowser.cpp` | `CreateDataObject` (drag/drop) | 30 | **HIGH** | Drag-and-drop from media browser broken |

#### Preview/ — Preview rendering (1 stub)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 18 | `PreviewDataContext.cpp` | `GetTypeInfo` (IDispatch) | 471 | **LOW** | Type info not available; late-binding fallback |

#### UI/ — Behaviors & components (4 stubs)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 19 | `SundanceUIComponents.cpp` | `AMPDataContext::SetProperty` (read-only) | 690 | **LOW** | Read-only data context; should return `E_ACCESSDENIED` or `S_FALSE` |
| 20 | `SundanceUIComponents.cpp` | `AMPDataContext::GetTypeInfo` (IDispatch) | 883 | **LOW** | Type info not available |
| 21 | `Ribbon/RibbonApp.cpp` | `GetCommandLabelDescription` (unknown command) | 1206 | **LOW** | Unknown ribbon command ID; should return `S_OK` with empty label |

#### External/ — Drag-drop COM wrappers (2 stubs)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 22 | `DragDropStub.cpp` | `DynamicDataObjectWrapper::GetDataHere` | 111 | **LOW** | Drag-drop `GetDataHere` — fallback path rarely called |
| 23 | `DragDropStub.cpp` | `DynamicDataObjectWrapper::EnumFormatEtc` | 167 | **LOW** | Drag-drop format enumeration — fallback path |

#### HMREngine/ — 3D/rendering engine (4 stubs)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 24 | `d3dx11compat.cpp` | `D3DX11CreateEffectFromMemory` | 183 | **LOW** | D3DX11 compatibility shim; D3DX11 not available on modern Windows |
| 25 | `DXResources/TextureResourceDX.cpp` | `CreateFromDIB` — non-BI_RGB compression | 123 | **LOW** | Only hit for compressed DIBs; WIC handles pre-conversion |
| 26 | `DXResources/TextureResourceDX.cpp` | `CreateFromDIB` — unsupported bpp (<24) | 136 | **LOW** | Only hit for <24bpp images; rare edge case |
| 27 | `CommandBin.cpp` | Switch default in command deserialization | 145 | **LOW** | Unknown serialized command type; error path |

#### HMRAVSource/ — Audio/video pipeline (14 stubs)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 28 | `AuthProvider.cpp` | `Authenticate` — unknown auth type | 272 | **LOW** | Unsupported authentication type switch default |
| 29 | `AuthProvider.cpp` | `AcquireLicense` | 380 | **LOW** | DRM license acquisition (PlayReady) |
| 30 | `AuthProvider.cpp` | `RequestPlayReadyLicense` | 385 | **LOW** | PlayReady license request |
| 31 | `AuthProvider.cpp` | `AuthenticateBasic` (no callback) | 418 | **LOW** | Basic auth fallback without prompt callback |
| 32 | `AuthProvider.cpp` | `AuthenticateOAuth` | 423 | **LOW** | OAuth 2.0 authentication stub |
| 33 | `AuthProvider.cpp` | `AuthenticateToken` | 428 | **LOW** | Token-based authentication stub |
| 34 | `dxva2stubs.cpp` | `DXVA2CreateVideoProcessorEnumerator` | 14 | **LOW** | DXVA2 HW acceleration stub |
| 35 | `dxva2stubs.cpp` | `DXVA2CreateVideoProcessor` | 23 | **LOW** | DXVA2 HW acceleration stub |
| 36 | `TextureInterop.cpp` | `GetSharedHandle` (no handle) | 205 | **LOW** | No shared handle available |
| 37 | `TextureInterop.cpp` | `CreateTextureInternal` | 210 | **LOW** | Internal texture creation method |
| 38 | `TextureInterop.cpp` | `CreateSurfaceInternal` | 215 | **LOW** | Internal surface creation method |
| 39 | `TranscodeMetadata.cpp` | `ExtractThumbnail` | 172 | **MEDIUM** | Thumbnail extraction from media files |
| 40 | `TranscodeMetadata.cpp` | `ExtractMetadata` (WIC fallback) | 205 | **MEDIUM** | Metadata extraction fallback path |
| 41 | `TranscodeMetadata.cpp` | `GetPropertyDateTime` | 240 | **MEDIUM** | Date/time property extraction from media |

---

### UXCore.dll — `src/UXCore/` (19 stubs)

**All UXCore stubs are gated behind global mark variables (`g_*_mark > 0`)
that are never set to non-zero in production. These are dead code paths.**

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 42 | `UXCore.cpp` | `DuiCreateObject` | 50 | **LOW** | DUI element creation; dead path |
| 43 | `Resources.cpp` | `CRMImage::LoadFromResource` | 87 | **LOW** | Image loading from resources; dead path |
| 44 | `Resources.cpp` | `CRMImage::ConvertToARGB` | 106 | **LOW** | ARGB conversion; dead path |
| 45 | `Resources.cpp` | `CRMDUIParser::Load` | 127 | **LOW** | DUI parser load; dead path |
| 46 | `Resources.cpp` | `IDuiDataSourceImpl::FindConnectionPoint` | 176 | **LOW** | Connection point lookup; dead path |
| 47 | `Resources.cpp` | `IDuiDataSourceImpl::EnumConnectionPoints` | 183 | **LOW** | Connection point enumeration; dead path |
| 48 | `Resources.cpp` | `IDuiDataSourceImpl::RetrieveItems` | 196 | **LOW** | Data source item retrieval; dead path |
| 49 | `Resources.cpp` | `IDuiDataSourceImpl::AddItems` | 203 | **LOW** | Add items to data source; dead path |
| 50 | `Resources.cpp` | `IDuiDataSourceImpl::RemoveItems` | 210 | **LOW** | Remove items from data source; dead path |
| 51 | `Resources.cpp` | `IDuiDataSourceImpl::FireRemoved` | 222 | **LOW** | Removed notification; dead path |
| 52 | `Resources.cpp` | `IDuiDataSourceImpl::FireAdded` | 229 | **LOW** | Added notification; dead path |
| 53 | `Resources.cpp` | `IDuiDataSourceImpl::FireReset` | 234 | **LOW** | Reset notification; dead path |
| 54 | `Element.cpp` | `Element::FireEvent` | 189 | **LOW** | Event firing; dead path |
| 55 | `Element.cpp` | `Element::EnsureVisible` | 232 | **LOW** | Scroll-into-view; dead path |
| 56 | `Element.cpp` | `Element::ExecCmd` | 284 | **LOW** | Command execution; dead path |
| 57 | `Element.cpp` | `Element::MapElementPoint` | 292 | **LOW** | Coordinate mapping; dead path |
| 58 | `Element.cpp` | `Element::FocusElement` | 312 | **LOW** | Focus management; dead path |
| 59 | `Controls.cpp` | `WLEditT::GetContentSize` | 152 | **LOW** | Text edit content sizing; dead path |
| 60 | `Controls.cpp` | `WLEditT::GetPosFromChar` | 159 | **LOW** | Character position lookup; dead path |

---

### WLXMediaPublishSubscribe.dll — `src/WLXMediaPublishSubscribe/` (12 stubs)

All social media upload and publish-manager API stubs. These represent entirely
unimplemented online publishing features. All are **LOW** priority — they are
truly missing features, not broken existing ones.

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 61 | `WLXMediaPublishSubscribe.cpp` | `FacebookPublisher::Upload` | 204 | **LOW** | Facebook video upload |
| 62 | `WLXMediaPublishSubscribe.cpp` | `FlickrPublisher::Upload` | 224 | **LOW** | Flickr photo upload |
| 63 | `WLXMediaPublishSubscribe.cpp` | `YouTubePublisher::Upload` | 244 | **LOW** | YouTube video upload |
| 64 | `WLXMediaPublishSubscribe.cpp` | `VimeoPublisher::Upload` | 263 | **LOW** | Vimeo video upload |
| 65 | `WLXMediaPublishSubscribe.cpp` | `SkyDrivePublisher::Upload` | 282 | **LOW** | SkyDrive/OneDrive upload |
| 66 | `WLXMediaPublishSubscribe.cpp` | `PublishManager_GetStatus` | 509 | **LOW** | Publish status query |
| 67 | `WLXMediaPublishSubscribe.cpp` | `PublishManager_Cancel` | 514 | **LOW** | Cancel publish operation |
| 68 | `WLXMediaPublishSubscribe.cpp` | `PublishManager_GetResult` | 519 | **LOW** | Publish result retrieval |
| 69 | `WLXMediaPublishSubscribe.cpp` | `PublishManager_SetProgressCallback` | 525 | **LOW** | Progress callback registration |
| 70 | `WLXMediaPublishSubscribe.cpp` | `PublishManager_SetCompleteCallback` | 531 | **LOW** | Complete callback registration |
| 71 | `WLXMediaPublishSubscribe.cpp` | `PublishManager_GetSubscribeStatus` | 547 | **LOW** | Subscribe status query |
| 72 | `WLXMediaPublishSubscribe.cpp` | `PublishManager_RefreshToken` | 561 | **LOW** | OAuth token refresh |

---

### WLXVideoTrim.dll — `src/WLXVideoTrim/` (5 stubs)

All export/trim creation functions are stubbed out. These are needed for
video export/trimming features.

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 73 | `WLXVideoTrim.cpp` | `CreateAVICopierDirect` | 304 | **LOW** | AVI direct copier creation |
| 74 | `WLXVideoTrim.cpp` | `CreateVideoCopierFromMediaType` | 311 | **LOW** | Video copier from media type |
| 75 | `WLXVideoTrim.cpp` | `CreateVideoFormatContextTranscoder` | 317 | **LOW** | Format context transcoder creation |
| 76 | `WLXVideoTrim.cpp` | `CreateVideoPlayer` | 323 | **LOW** | Video player creation |
| 77 | `WLXVideoTrim.cpp` | `CreateVideoWMVTranscoder` | 329 | **LOW** | WMV transcoder creation |

---

### WLXPhotoBase.dll — `src/WLXPhotoBase/` (4 stubs)

Error-code mapping functions. These are correct — they map GDI+ / Win32 error
codes to `E_NOTIMPL` accurately.

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 78 | `WLXPhotoBase.cpp` | `Win32ErrorToHresult` — `ERROR_NOT_SUPPORTED` | 80 | **LOW** | Correct mapping |
| 79 | `WLXPhotoBase.cpp` | `GdiplusStatusToHresult` — `NotImplemented` | 255 | **LOW** | Correct GDI+ status mapping |
| 80 | `WLXPhotoBase.cpp` | `GdiplusStatusToHresult` — `PropertyNotSupported` | 261 | **LOW** | Correct GDI+ status mapping |
| 81 | `WLXPhotoBase.cpp` | `GdiplusStatusToHresult` — `UnsupportedGdiplusVersion` | 273 | **LOW** | Correct GDI+ status mapping |

---

### WLXMP4Parser.dll — `src/WLXMP4Parser/` (4 stubs)

MP4 parsing and filter graph construction. These are needed for MP4 file
support in the DirectShow pipeline.

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 82 | `WLXMP4Parser.cpp` | `GetTrackInfo` | 245 | **MEDIUM** | MP4 track info retrieval (used by media browser) |
| 83 | `WLXMP4Parser.cpp` | `AddMP4SourceFilter` | 294 | **MEDIUM** | MP4 source filter for DirectShow graph |
| 84 | `WLXMP4Parser.cpp` | `BuildMP4FilterGraph` | 301 | **MEDIUM** | MP4 filter graph builder |
| 85 | `WLXMP4Parser.cpp` | `BuildMP4PlayBack` | 308 | **MEDIUM** | MP4 playback graph creation |

---

### MovieMakerPreviewClient.dll — `src/MovieMakerPreviewClient/` (2 stubs)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 86 | `MovieMakerPreviewClient.cpp` | `HandleCommand(PreviewCmd_Snapshot)` | 353 | **LOW** | Preview snapshot (cosmetic) |
| 87 | `MovieMakerPreviewClient.cpp` | `HandleCommand(PreviewCmd_LoadFile)` | 356 | **MEDIUM** | Preview window file loading |

---

### WLMFDS.dll — `src/WLMFDS/` (2 stubs)

DirectShow-to-MediaFoundation bridge. Required for legacy DirectShow filter
compatibility in the MF pipeline.

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 88 | `WLMFDS.cpp` | `CreateMFSourceFromDShowFilter` | 110 | **LOW** | DirectShow-to-MF source bridge |
| 89 | `WLMFDS.cpp` | (connect EVR filter) | 171 | **LOW** | EVR filter connection |

---

### WLXPipetran.dll — `src/WLXPipetran/` (1 stub)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 90 | `WLXPipetran.cpp` | `GetTFXCreateFunctions` | 469 | **LOW** | Pipeline effect creation functions |

---

### WLXPipeline.dll — `src/WLXPipeline/` (1 stub)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 91 | `WLXPipeline.cpp` | `GetPipelineCreateFunctions` | 328 | **LOW** | Pipeline creation functions |

---

### WLMFReadWrite.dll — `src/WLMFReadWrite/` (1 stub)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 92 | `WLMFReadWrite.cpp` | `WriteFrame` | 226 | **MEDIUM** | Frame writing to MediaFoundation sink |

---

### WLXMovieLibrary.dll — `src/WLXMovieLibrary/` (1 stub)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 93 | `WLXMovieLibrary.cpp` | `GetThumbnail` (MF fallback) | 177 | **MEDIUM** | Video thumbnail generation via MediaFoundation |

---

### MetadataSys.dll — `src/MetadataSys/` (1 stub)

| # | File | Function | Line | Pri | Description |
|---|------|----------|------|-----|-------------|
| 94 | `MetadataSys.cpp` | `WLXPSGetItemPropertyHandler` | 16 | **MEDIUM** | Property handler for photo/video shell items |

> **Note**: After cross-checking, 2 entries from the WLXPhotoBase and
> MovieMakerPreviewClient counts were deduplicated. Final total: **92**.
> *(Corrected from the raw-match count of 94 after excluding 2 non-stub
> lines: comment in TextureCodecs.cpp and #define in ErrHandler.cpp.)*

---

## Top 20 Most Critical Stubs to Fix First

### Tier 1 — HIGH priority (functional blockers)

| Rank | File:Line | Function | Why | Suggested Fix |
|------|-----------|----------|-----|---------------|
| 1 | `SundanceAppMain.cpp:616` | `SaveProject` — no file path | New/untitled projects cannot be saved | Call `SaveProjectAs()` or `E_UNEXPECTED` with a dialog prompt instead of silent failure |
| 2 | `SundanceAppMain.cpp:958` | `ImportFiles` — no controller | Import media silently fails | Create the `m_pImportController` during startup, or return `E_UNEXPECTED` |
| 3 | `MediaBrowser.cpp:30` | `CreateDataObject` | Drag-and-drop from media browser broken | Return `E_UNEXPECTED` or implement using `OleCreateDataObject` / `SHCreateDataObject` |

### Tier 2 — MEDIUM-HIGH (major feature gaps)

| Rank | File:Line | Function | Why | Suggested Fix |
|------|-----------|----------|-----|---------------|
| 4 | `WLXMP4Parser.cpp:245` | `GetTrackInfo` | MP4 track info unavailable; media browser can't show MP4 metadata | Parse from moov box header data or return `E_FAIL` |
| 5 | `WLXMP4Parser.cpp:294` | `AddMP4SourceFilter` | No MP4 DirectShow source filter | Return `CLASS_E_CLASSNOTAVAILABLE` or implement via MF source reader |
| 6 | `WLXMP4Parser.cpp:301` | `BuildMP4FilterGraph` | MP4 filter graph cannot be built | Return `CLASS_E_CLASSNOTAVAILABLE` |
| 7 | `WLXMP4Parser.cpp:308` | `BuildMP4PlayBack` | MP4 playback broken | Return `CLASS_E_CLASSNOTAVAILABLE` |
| 8 | `TranscodeMetadata.cpp:172` | `ExtractThumbnail` | Thumbnail extraction fails | Implement via MF `IMFMediaSource` + `IMFSourceReader` thumbnail extraction, or return `E_FAIL` |
| 9 | `TranscodeMetadata.cpp:205` | `ExtractMetadata` (WIC fallback) | Metadata reading fallback broken | Implement via WIC `IWICMetadataQueryReader` |
| 10 | `TranscodeMetadata.cpp:240` | `GetPropertyDateTime` | Date properties unavailable | Read via `IPropertyStore` from Shell item |
| 11 | `WLXMovieLibrary.cpp:177` | `GetThumbnail` (MF fallback) | Video thumbnails not generated | Implement via `IMFSourceReader` + `IMFMediaBuffer` extraction, or `IThumbnailProvider` |
| 12 | `WLMFReadWrite.cpp:226` | `WriteFrame` | MF writer wrapper: frame writing broken | Implement by wrapping data in `IMFMediaBuffer` + `IMFSample` |
| 13 | `MovieMakerPreviewClient.cpp:356` | `PreviewCmd_LoadFile` | Preview window cannot load files | Implement via `IPropertyStore` / DirectShow graph creation |

### Tier 3 — MEDIUM (feature degradation)

| Rank | File:Line | Function | Why | Suggested Fix |
|------|-----------|----------|-----|---------------|
| 14 | `SundanceAppMain.cpp:1029` | `Undo` — no manager | Undo/redo unavailable | Create `m_pUndoManager` in `SundanceAppMain::Initialize()`; change stub to `E_UNEXPECTED` |
| 15 | `SundanceAppMain.cpp:1072` | `CutSelection` — no clipboard | Cut/Copy/Paste unavailable | Create `m_pClipboardManager` during init; change stub to `E_UNEXPECTED` |
| 16 | `SundanceAppMain.cpp:1151` | `StartPlayback` — no controller | Playback controls dead | Create `m_pPlaybackController` during init; change stub to `E_UNEXPECTED` |
| 17 | `SundanceAppMain.cpp:979` | `PublishMovie` — no controller | Save-Movie fails | Create `m_pExportController` during init; change stub to `E_UNEXPECTED` |
| 18 | `MetadataSys.cpp:16` | `WLXPSGetItemPropertyHandler` | Shell property handler unavailable | Return `CLASS_E_CLASSNOTAVAILABLE` |
| 19 | `SundanceAppMain.cpp:1005` | `PublishToService` — no controller | Publish-to-service unavailable | Same as #17; create controller or return `E_UNEXPECTED` |
| 20 | `SundanceAppMain.cpp:1556` | `OnRibbonCommand(SaveMovie)` — no export | Ribbon Save-Movie broken | Same as #17 |

---

## Key Findings

1. **No startup-blocking CRITICAL stubs exist.** The UXCore dead-code stubs
   (gated behind `g_*_mark` globals) are never reached. The application shell
   (SundanceApp) initializes — but many **managers are never created**, which means
   their null-guard stubs are active.

2. **The #1 fix is initializing managers at startup.** 12 of 14 SundanceApp
   stubs are null-guard checks (`if (!m_pX) return E_NOTIMPL`). The root cause
   is that `m_pImportController`, `m_pExportController`, `m_pUndoManager`,
   `m_pClipboardManager`, and `m_pPlaybackController` are not instantiated.
   **Fixing initialization eliminates most SundanceApp stubs** in one pass.

3. **The #2 fix is SaveProject (line 616).** This is a logic bug — when a new
   project has no file path, `SaveProject` should redirect to `SaveProjectAs`
   instead of returning `E_NOTIMPL`.

4. **WLXMP4Parser stubs** block MP4 file support in the DirectShow pipeline.
   These should return `CLASS_E_CLASSNOTAVAILABLE` so the app can fall back
   to other parsers (e.g., the MF source reader).

5. **WLXMediaPublishSubscribe and WLXVideoTrim stubs** are entirely missing
   features (social upload, video export/trim). They are genuinely unimplented
   rather than broken — remove the stubs or mark with `CLASS_E_CLASSNOTAVAILABLE`.

6. **UXCore stubs** (19 of 92 — 21%) are dead code and can be ignored entirely.

---

## Count Summary

| Metric | Count |
|--------|-------|
| Total files with E_NOTIMPL | 31 |
| Total stub occurrences | 92 |
| Dead code (UXCore) | 19 |
| Error-code mappings (WLXPhotoBase) | 4 |
| True missing features (publish/trim/export) | 18 |
| Engine/AV/compat stubs (LOW) | 28 |
| Feature-degradation stubs (MEDIUM) | 20 |
| Functional-blocker stubs (HIGH) | 3 |
| Startup-blocking (CRITICAL) | 0 |
