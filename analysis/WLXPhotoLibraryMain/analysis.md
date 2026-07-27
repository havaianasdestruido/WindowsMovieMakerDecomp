# WLXPhotoLibraryMain.dll Analysis

## Overview

**File:** WLXPhotoLibraryMain.dll (Windows Live Photo Gallery 2012 — Photo Library Main Logic)
**Role:** Main UI and application logic DLL for Photo Gallery — gallery views, editing, publishing, search, filters, slideshow, video, DirectUI
**Image size:** 0x350000 (~3.3 MB)
**PDB:** `WLXPhotoLibraryMain.pdb` `{08153282-41D5-43F3-8971-A8D9A632C905}`
**Build:** 16.4.3528.0331 (timestamp: 2014-04-01 01:26:33)

## PE Structure

| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Subsystem | Windows GUI (2) |
| Linker | MSVC 11.00 |
| Image Base | 0x10000000 |
| Image Size | 0x350000 |
| Entry Point | 0x10268855 |
| OS/Image Version | 6.02 / 6.02 |
| DLL Characteristics | Dynamic Base (ASLR), NX Compatible |
| Stack reserve/commit | 256 KB / 4 KB |
| Heap reserve/commit | 1 MB / 4 KB |
| Checksum | 0x35BCD3 |

### Sections

| Section | VA | VirtSize | RawSize | Characteristics |
|---------|------|----------|---------|-----------------|
| `.text` | 0x1000 | 0x2E4A78 (2.8 MB) | 0x2E4C00 | Code, Execute Read |
| `.data` | 0x2E6000 | 0x2B640 (173 KB) | 0x29E00 | Init Data, Read Write |
| `.rsrc` | 0x312000 | 0x3F0 (1 KB) | 0x400 | Init Data, Read Only |
| `.reloc` | 0x313000 | 0x30768 (194 KB) | 0x3C200 | Init Data, Discardable, Read Only |

**Key observation**: Massive code section (2.8 MB) — the largest binary analyzed. `.rsrc` is minimal (1 KB) — UI resources are stored in external DRM/DUI resource files (`WLXPhotoLibraryDuiResources`, `WLXPhotoLibraryDuiResourcesLocalized`). The large `.data` section (173 KB) contains DirectUI property info tables, vtables, and string constants.

## Exports (3 functions)

| Ordinal | Name | RVA | Purpose |
|---------|------|-----|---------|
| 1 | `DisplayRepairPromptDialog` | 0x58DA1 | Database repair dialog |
| 2 | `DllCanUnloadNow` | 0x5636E | COM unload check |
| 3 | `RunAsStandAlone` | 0x5667D | Standalone launch mode |

Only 3 exports — the DLL is not COM-registered. `DllCanUnloadNow` is used for COM lifetime management. `RunAsStandAlone` launches the gallery as a standalone app. `DisplayRepairPromptDialog` shows the database corruption repair dialog.

## Import Table (76 dependencies)

### CRT / Runtime
- **MSVCR110.dll** (100+ functions): CRT core — memory (malloc/free/calloc/realloc/aligned_malloc/aligned_free/memcpy/memcmp/memset/memmove), SSE2 math (acos/asin/atan/cos/sin/exp/log/pow/sqrt/tan), string (wcschr/wcscat_s/wcscpy_s/wcsncpy_s/wcslen/wcsnlen/wcsstr/wcsrchr/wcspbrk/wcscspn/wcsspn/wcstok_s/wcstol/wcstoul/_wcsicmp/_wcsicmp_l/_wcsnicmp/_wcslwr_s/_wcsnset_s/towlower/towupper/iswspace/iswupper/iswdigit/isgraph/swprintf_s/swscanf_s/_swscanf_s_l/_vsnwprintf/_vsnwprintf_l/_vscwprintf/vswprintf_s), exceptions (CxxThrowException/CxxFrameHandler3/__RTDynamicCast/terminate), file I/O (fopen/fclose/fread/fwrite/fprintf/fwprintf/fputc/fputs), qsort/qsort_s/bsearch, locale (_create_locale/_free_locale), threading (_beginthreadex), math (ceil/floor/_CIatan2/_CIfmod), time (_time64), random (srand/rand), debug (_crt_debugger_hook/__crtTerminateProcess/__crtUnhandledException), format (_itoa_s/_i64tow_s/_gcvt/_wtof/_wtoi/_wtoi64/_wfopen_s)
- **MSVCP110.dll** (6 functions): C++ stdlib — `_Syserror_map`, `_Winerror_map`, `_Xout_of_range`, `_Xlength_error`, `_Xbad_alloc`, `std::cout`

### OS
- **KERNEL32.dll** (100+ functions): File I/O (CreateFileW/ReadFile/WriteFile/CopyFileW/MoveFileExW/DeleteFileW/FindFirstFileW/FindFirstFileExW/FindNextFileW/FindClose/SetFileAttributesW/GetFileAttributesW/GetFileAttributesExW/GetFileTime/SetFileTime), directory (CreateDirectoryW/GetFullPathNameW/GetLongPathNameW), volume (GetDriveTypeW/GetVolumeInformationW/GetLogicalDriveStringsW/GetDiskFreeSpaceExW), module (LoadLibraryW/LoadLibraryExW/FreeLibrary/GetModuleHandleW/GetModuleHandleA/GetModuleFileNameW/GetProcAddress), resources (FindResourceW/FindResourceExW/LoadResource/LockResource/SizeofResource), sync (CreateEventW/CreateMutexW/CreateIoCompletionPort/GetQueuedCompletionStatus/PostQueuedCompletionStatus/WaitForSingleObject/CRITICAL_SECTION/SListHead), threading (CreateThread/GetCurrentThreadId/SetThreadPriority/GetThreadPriority/GetExitCodeThread/SwitchToThread), memory (LocalAlloc/LocalFree/HeapAlloc/HeapFree/HeapReAlloc/HeapSize/VirtualAlloc/VirtualFree/GlobalAlloc/GlobalFree/GlobalLock/GlobalUnlock/GlobalSize), time (GetSystemTime/GetSystemTimeAsFileTime/FileTimeToSystemTime/GetLocalTime/GetTickCount/GetTickCount64/QueryPerformanceCounter/QueryPerformanceFrequency), encoding (MultiByteToWideChar/WideCharToMultiByte), process (CreateProcessW/IsWow64Process/GetCurrentProcess), ASLR (EncodePointer/DecodePointer), debug (IsDebuggerPresent/RaiseException/OutputDebugStringA/FlushInstructionCache), environment (ExpandEnvironmentStringsW/GetEnvironmentVariableW), temp (GetTempPathW/GetTempFileNameW), misc (DisableThreadLibraryCalls/SetErrorMode/IsProcessorFeaturePresent/Sleep/InterlockedIncrement/InterlockedDecrement/InterlockedExchange/InterlockedCompareExchange/InitializeCriticalSectionAndSpinCount/CompareStringW/CompareFileTime/GetVersion/GetVersionExW/BackupRead/BackupSeek/GetACP/GetThreadUILanguage/GlobalAddAtomW/GlobalDeleteAtom)
- **USER32.dll** (50+ functions): Window management (CreateWindowExW/DestroyWindow/RegisterClassExW/DefWindowProcW/CallWindowProcW), message loop (GetMessageW/TranslateMessage/DispatchMessageW/PeekMessageW/PostMessageW/PostQuitMessage/PostThreadMessageW/MsgWaitForMultipleObjects/MsgWaitForMultipleObjectsEx/CallMsgFilterW), UI (SetWindowTextW/GetWindowTextW/MessageBoxW/FindWindowExW/SendMessageW/SendNotifyMessageW/SetWindowLongW/GetWindowLongW/SetTimer/KillTimer/GetDC/ReleaseDC), text (CharNextW/CharLowerBuffW/RegisterWindowMessageW/GetClassInfoExW/LoadCursorW/IsWindow)
- **ADVAPI32.dll** (12 functions): Registry (RegCreateKeyExW/RegOpenKeyExW/RegSetValueExW/RegQueryValueExW/RegDeleteKeyW/RegDeleteValueW/RegEnumKeyExW/RegQueryInfoKeyW), ETW (RegisterTraceGuidsW/TraceEvent/EventRegister/EventUnregister/EventWrite/GetTraceLoggerHandle/GetTraceEnableLevel/GetTraceEnableFlags/UnregisterTraceGuids), security (GetNamedSecurityInfoW/SetNamedSecurityInfoW), file encryption (EncryptFileW/DecryptFileW)
- **GDI32.dll** (45+ functions): Graphics (BitBlt/CreateDIBSection/CreateCompatibleDC/GetDIBits/SelectObject/DeleteObject/CreateFontIndirectW/CreateSolidBrush/CreatePen/CreateBrushIndirect/CreateRectRgn/CreateRectRgnIndirect/CombineRgn/EqualRgn/PtInRegion/OffsetRgn/GetRgnBox/GetRegionData/ExtCreateRegion/GetClipRgn/SetROP2), text (GetTextMetricsW/GetTextExtentPoint32W/ExtTextOutW/SetTextColor/SetBkColor/SetBkMode), transform (GetWorldTransform/SetWorldTransform/ModifyWorldTransform/CombineTransform/SetGraphicsMode/SetViewportOrgEx/OffsetViewportOrgEx), palette (CreateHalftonePalette/SelectPalette/RealizePalette), DC (DeleteDC/GetDeviceCaps/GetLayout/SetLayout), pen/brush (SetDCPenColor/SetDCBrushColor/GetCurrentObject/GetStockObject/GetOutlineTextMetricsW/RoundRect), misc (PatBlt/GetBitmapBits)

### COM
- **ole32.dll** (13 functions): COM core (CoInitialize/CoUninitialize/CoCreateInstance/CoGetClassObject/CoInitializeEx), clipboard (OleInitialize/OleUninitialize/OleGetClipboard/OleSetClipboard/OleFlushClipboard/OleIsCurrentClipboard), drag-drop (RegisterDragDrop/RevokeDragDrop), memory (CoTaskMemAlloc/CoTaskMemRealloc/CoTaskMemFree), stream (CreateStreamOnHGlobal), GUID (CoCreateGuid/StringFromCLSID/CLSIDFromString/StringFromGUID2), context (CreateBindCtx/GetRunningObjectTable/CreateObjrefMoniker), security (CoAllowSetForegroundWindow)
- **OLEAUT32.dll** (33 ordinal-only): Extensive COM automation — SysAllocString/SysFreeString/SysStringLen/SysStringByteLen/SysAllocStringByteLen/SysReAllocString/SysReAllocStringLen/SysFreeString, VariantInit/VariantClear/VariantCopy/VariantChangeType/VariantToPropVariant/PropVariantToVariant, SafeArrayCreate/SafeArrayDestroy/SafeArrayAccessData/SafeArrayUnaccessData/SafeArrayGetElement/SafeArrayPutElement/SafeArrayGetLBound/SafeArrayGetUBound/SafeArrayGetDim, SysDateTimeToVarDateTime/VarDateFromDateTime, UIAddTypeLibDispInvoke, etc.
- **PROPSYS.dll** (8 functions): Property system (InitPropVariantFromBuffer/InitPropVariantFromCLSID/PSCreateMemoryPropertyStore/PropVariantChangeType/PropVariantToGUID/PropVariantToStringAlloc/PSFormatForDisplay/VariantCompare)
- **OLEACC.dll** (3 functions): Accessibility (AccessibleObjectFromWindow/LresultFromObject/CreateStdAccessibleProxyW)

### Shell / UI
- **SHLWAPI.dll** (38 functions): Path utilities (PathFileExistsW/PathFindFileNameW/PathFindExtensionW/PathIsRelativeW/PathIsRootW/PathAppendW/PathAddBackslashW/PathRemoveBackslashW/PathRemoveFileSpecW/PathStripToRootW/PathStripPathW/PathIsDirectoryW/PathIsFileSpecW/PathIsNetworkPathW/PathIsUNCW/PathMatchSpecW/PathParseIconLocationW/PathRenameExtensionW/PathRemoveExtensionW), string (StrCmpIW/StrCmpLogicalW/StrCmpNIW/StrStrIW/StrChrIW/StrRChrW/StrRetToBSTR/StrRetToBufW/StrFormatByteSizeW/SHStrDupW), registry (SHRegGetValueW/SHRegOpenUSKeyW/SHRegGetUSValueW/SHRegEnumUSValueW/SHRegCloseUSKey/SHDeleteValueW/SHOpenRegStream2W), stream (SHCreateStreamOnFileW/SHCreateStreamOnFileEx), misc (AssocGetPerceivedType/AssocQueryStringW)
- **gdiplus.dll** (100+ functions): Full GDI+ rendering — bitmap (GdipCreateBitmapFromHBITMAP/GdipCreateBitmapFromStream/GdipCreateBitmapFromScan0/GdipCreateBitmapFromGraphics/GdipCloneBitmapAreaI/GdipBitmapLockBits/GdipBitmapUnlockBits/GdipGetImageWidth/GdipGetImageHeight/GdipGetImagePixelFormat/GdipCloneImage/GdipDisposeImage/GdipImageRotateFlip), graphics (GdipCreateFromHDC/GdipCreateFromHWND/GdipGetImageGraphicsContext/GdipReleaseDC/GdipGetDC), drawing (GdipDrawImageI/GdipDrawImageRectI/GdipDrawImagePointRectI/GdipDrawImageRectRectI/GdipDrawImageRectRect/GdipDrawCachedBitmap/GdipDrawLineI/GdipDrawLinesI/GdipDrawRectangleI/GdipDrawString/GdipDrawString/GdipFillRectangleI/GdipFillEllipseI/GdipFillPolygonI), pen/brush (GdipCreatePen1/GdipCreatePen2/GdipDeletePen/GdipCreateSolidFill/GdipCreateHatchBrush/GdipCreateLineBrush/GdipCreateLineBrushI/GdipCreateLineBrushFromRect/GdipCreateLineBrushFromRectI/GdipDeleteBrush/GdipCloneBrush), path (GdipCreatePath/GdipAddPathArcI/GdipAddPathLineI/GdipAddPathLine/GdipClosePathFigure/GdipDeletePath/GdipFillPath/GdipDrawPath), string format (GdipCreateStringFormat/GdipDeleteStringFormat/GdipCloneStringFormat/GdipSetStringFormatAlign/GdipSetStringFormatLineAlign/GdipSetStringFormatFlags/GdipSetStringFormatTrimming/GdipSetStringFormatHotkeyPrefix/GdipSetStringFormatDigitSubstitution/GdipStringFormatGetGenericTypographic/GdipMeasureString), font (GdipCreateFont/GdipCreateFontFamilyFromName/GdipDeleteFont/GdipDeleteFontFamily/GdipCreateFontFromDC/GdipCreateFontFromLogfontA/GdipGetFontHeight/GdipGetFontStyle/GdipGetFontSize/GdipGetFontUnit/GdipGetFamily/GdipGetFamilyName/GdipGetLogFontW/GdipGetGenericFontFamilySansSerif/GdipGetCellAscent/GdipGetLineSpacing), matrix (GdipCreateMatrix/GdipCreateMatrix2/GdipDeleteMatrix/GdipGetMatrixElements/GdipSetWorldTransform/GdipGetWorldTransform/GdipTranslateWorldTransform/GdipRotateMatrix/GdipMultiplyWorldTransform), region (GdipCreateRegion/GdipDeleteRegion/GdipGetClip/GdipSetClipRegion/GdipSetClipRectI/GdipSetClipHrgn/GdipGetClipBoundsI), rendering hints (GdipSetSmoothingMode/GdipSetRenderingOrigin/GdipSetTextRenderingHint/GdipSetInterpolationMode/GdipSetPixelOffsetMode/GdipSetPageUnit/GdipSetCompositingMode), misc (GdipAlloc/GdipFree/GdipGetDC/GdipReleaseDC/GdipFlush/GdipCreateCachedBitmap/GdipDeleteCachedBitmap/GdipCreateImageAttributes/GdipDisposeImageAttributes/GdipSetImageAttributesWrapMode/GdipCreateHalftonePalette/GdipSetSolidFillColor/GdipSetLineBlend/GdipSetLineSigmaBlend/GdipSetPenMode), startup (GdiplusStartup/GdiplusShutdown)
- **UxTheme.dll** (9 functions): Visual styles (OpenThemeData/CloseThemeData/DrawThemeBackground/GetThemeColor/GetThemeFont/GetThemePartSize/GetThemeMargins/SetWindowTheme/EnableThemeDialogTexture/IsThemePartDefined)
- **WININET.dll** (1 function): `InternetGetConnectedState` — network connectivity check

### Windows Media
- **MF.dll** (1 function): `MFGetService` — Media Foundation service
- **MFPlat.DLL** (11 functions): Media Foundation platform (MFStartup/MFShutdown/MFGetPluginControl/MFCreateAttributes/MFCreateCollection/MFCreateMediaType/MFCreateSample/MFCreateMemoryBuffer/MFCreateSourceResolver/MFFrameRateToAverageTimePerFrame/MFAverageTimePerFrameToFrameRate/MFLockPlatform/MFUnlockPlatform/MFPutWorkItemEx/MFInvokeCallback)
- **d3d11.dll** (1 function): `D3D11CreateDevice` — Direct3D 11 device creation
- **dxva2.dll** (2 functions): `DXVA2CreateDirect3DDeviceManager9/DXVA2CreateVideoService` — video acceleration
- **d3d9.dll** (delay, 2 functions): `Direct3DCreate9/Direct3DCreate9Ex` — Direct3D 9 fallback

### Network
- **WINHTTP.dll** (14 functions): HTTP client (same as WLXPhotoLibraryDatabase.dll)
- **wlidcli.dll** (delay, 1 ordinal): Windows Live ID client (ordinal 3)

### Publishing
- **WLXMediaPublishSubscribe.dll** (5 functions): `MediaPublishSubscribeHelper` (ctor/dtor), `GetLiveSignInProvider`, `LaunchAuthBrowser`, `PluginPublishItems` — SkyDrive/OneDrive publishing

### Windows Live DLLs
- **WLXPhotoBase.dll** (32 functions): `Base::Exception` (ctor/copy/dtor/operator bool), `Base::Throw/ThrowLastError`, `Base::IsOutOfMemoryError`, `Base::IsWin7OrGreater/IsWin8OrGreater`, `Base::GdiplusStatusToHresult`, `Base::GetBaseStringManager`, `BasePrivate::New/Delete`, `ATL::BaseAtlThrow`, `Base::Version` (ctor/copy/operator==/operator</operator>/IsValid/Invalidate/Set/AsString) — extensive version comparison and exception infrastructure
- **WLXPhotoSqm.dll** (30+ ordinal imports): Comprehensive SQM telemetry — `Sqm::Startup/Shutdown/IsEnabled`, `Sqm::AddToStream` (5 overloads), `Sqm::Set/Increment/SetIfMax`, `Sqm::DeferReportMedian/DeferAddToMedian`, `Sqm::AddToAverage`, `Sqm::StartStreamTimer/StopStreamTimer/AbortStreamTimer/IsStreamTimerActive/AddStreamTimerData`, `Sqm::SetApplicationMode/SetAppDefinedValue`, `Sqm::ReportAppLaunchStatus/ReportAppCloseStatus/ReportUserExecutedAction/InitializeUserExecutedActionReporting`, `Sqm::EnableShipAsserts`, `Sqm::GetOptInState/SetOptInPreference`
- **UXCore.dll** (150+ imports): DirectUI UI framework — `Element` (50+ property infos and methods), `HWNDElement`, `NativeHWNDHost`, `HWNDHost`, `Button`, `Checkbox`, `Label`, `Slider`, `Thumb`, `Hyperlink`, `VirtualLayout`, `VirtualListView`, `SimpleScrollBar`, `PopupWindow`, `FillLayout`, `TableLayout`, `Value`, `CRMResource`, `CRMImage`, `CRMStringResource`, `CRMStringManager`, `CRMDUIParser`, `CDUIDialog`, `CFramelessHost`, `IDuiDataSourceImpl`, `NavReference`, resource manager (`RMFindModule/RMLoadMenu/RMLoadImage/RMLoadString/RMLoadIcon/RMLoadAccelerators/RMLoadColorARGB/RMLoadRect/RMLoadInt/RMUpdateResourceSet`)
- **uxctl.dll** (3 functions): `UxControlsInitProcess/UxControlsCreateObject/UxControlsUninitProcess`
- **WLXPhotoLibraryDatabase.dll** (delay, 17 functions): All exports from the database DLL
- **WLXMovieLibrary.dll** (delay, referenced): Movie library integration

### Other
- **VERSION.dll** (3 functions): File version info
- **SETUPAPI.dll** (5 functions): Device enumeration (same pattern as WLXPhotoAcq)
- **dwmapi.dll** (delay, 1 function): `DwmIsCompositionEnabled` — Desktop Window Manager
- **urlmon.dll** (delay, 1 function): `URLDownloadToCacheFileW` — URL download
- **WindowsCodecs.dll** (delay, 1 function): `WICMapGuidToShortName` — WIC
- **XmlLite.dll** (delay, 2 functions): `CreateXmlWriter/CreateXmlWriterOutputWithEncodingName` — XML writing
- **msi.dll** (delay, 3 ordinals): MSI installer integration

## DirectUI Framework

### XML Resource Files

| Resource | Purpose |
|----------|---------|
| `ReadOnlyGraph.xml` | Main gallery graph layout |
| `ExclusionList.xml` | Excluded media items |
| `WLRawCodec.xml` | RAW codec definitions |
| `WLXPhotoLibraryDuiResources` | DUI resource set (English) |
| `WLXPhotoLibraryDuiResourcesLocalized` | Localized DUI resources |

### UI Element Hierarchy

#### Navigation
| Class | Property/Method | Purpose |
|-------|----------------|---------|
| `Photos_NavTree` | SysTreeView32 | Folder navigation tree |
| `WLPGPhotoLibrary_NavTreeNodes_16.4.3528.0331` | Tree nodes | Navigation tree data |

#### Gallery Views
| Class | Property/Method | Purpose |
|-------|----------------|---------|
| `Photos_PicListView` / `WLPGListViewDragDropID` | SysListView32 | Main photo list view |
| `GalaView` | View mode | Gallery view container |
| `DaliView` | ResId, TemplateSize, ThumbMaxHeight/Width | Dali timeline view |
| `Photos_LoadingCanvas` / `Photos_LoadingFrameCover` | Loading | Loading state UI |

#### Search
| Class | Property/Method | Purpose |
|-------|----------------|---------|
| `SearchBar` | Pin/Unpin | Search bar container |
| `ideSearchBarWordwheel` | WordwheelEdit | Search text input |
| `ideSearchBarFilter*` | Filters | Filter buttons |
| `idrSearchBarRatingStar` / `idrSearchBarRatingRange` | Rating | Star rating filter |
| `FilterString/FilterRating/FilterPerson/FilterLocation/FilterDateRange/FilterMonthAnyYear/FilterFlag/FilterFolder/FilterLabel/FilterText/FilterAllObjects` | Filters | Filter types |
| `FilterContainer/FilterIntersection/FilterUnion` | Boolean | Filter composition |

#### Info/Tag Pane
| Class | Property/Method | Purpose |
|-------|----------------|---------|
| `TagsSection` / `TagsListContainer` | Tag editing | Tag section |
| `LabelTagList` / `LocationTagList` / `PersonTagList` | Tag lists | Tag categories |
| `Photos_StaticTextEx` | Extended text | Info text |
| `btnInfoPaneEditTitleButton` | Button | Edit title |
| `btnInfoPaneAddTagButton` | Button | Add tag |
| `Photos_CueEdit` | Cue text | In-place editing |

#### Face Tagging
| Class | Property/Method | Purpose |
|-------|----------------|---------|
| `Photos_ContactPickerWindow` / `Photos_ContactPickerListView` | Contact picker | People tag picker |
| `FabFiveListView` / `FabFiveListViewFactory` | FabFive | Top 5 people |
| `FabFiveListItemWithEmail` / `ContactPickerListItemEmail` | Email | Contact email |
| `PromptForNameDialog` | Dialog | Name prompt |

#### Toolbar/Actions
| Class | Property/Method | Purpose |
|-------|----------------|---------|
| `btnRotateCCW/CW` | Rotate | Rotation buttons |
| `btnSlideshow` | Slideshow | Slideshow button |
| `btnViewThumbnails/Tiles` | View mode | View mode buttons |
| `LIBRARY_RIBBON` | Ribbon | Library ribbon |

#### Hover Preview
| Class | Property/Method | Purpose |
|-------|----------------|---------|
| `HoverPreview` / `HoverPreviewBehavior` | Preview | Hover thumbnail preview |
| `HoverPreviewThumbRenderer` | Renderer | Thumbnail renderer |
| `IDP_GalaHoverPreview_*` | Parameters | Preview parameters |

## Application Architecture

### Standalone Mode
`RunAsStandAlone()` launches the gallery as a standalone application:
- Main window class: `WLX_Photo_Library_MainWnd`
- Supported media types: `SupportedMediaTypes`
- Max photo dimensions: `MaxPhotoWidth/MaxPhotoHeight`
- Video limits: `MaxVideoBitrate/MaxVideoFramerate`

### Database Integration
- `WLXPhotoLibraryDatabase.dll` (delay-loaded) — all database operations
- `WLXPhotoBase.dll` — memory/exception infrastructure
- `MetadataSys.dll` (delay-loaded) — metadata property access

### DirectUI/UXCore Framework
Massive DirectUI integration (150+ imports from UXCore.dll):
- `Element` — base UI element with 50+ property infos (Background, Foreground, Width, Height, Padding, Margin, LayoutPos, Visible, Enabled, etc.)
- `HWNDElement` — HWND-hosted DirectUI element
- `NativeHWNDHost` — HWND hosting
- `HWNDHost` — child HWND hosting within DirectUI
- `Button/Checkbox/Label/Slider/Thumb/Hyperlink` — standard controls
- `VirtualLayout/VirtualListView` — virtualized list rendering
- `PopupWindow` — popup/tooltip support
- `CDUIDialog` — DUIDialog base (DoModal, OnInitDialog, OnCommand, OnClose, etc.)
- `CFramelessHost` — frameless window support
- `CRMDUIParser` — DirectUI XML parser
- `CRMResource/CRMImage/CRMStringResource` — resource management
- `Value` — DirectUI value type (bool, int, string, graphic, layout, unknown)

### Video Processing
- **MFPlat.DLL**: Media Foundation for video decode/encode
- **d3d11.dll / dxva2.dll**: GPU-accelerated video rendering
- **d3d9.dll** (delay): Direct3D 9 fallback
- Video rotate via `WLXVideoTrim` / `MMRotate.prx`
- Media types: YV12, NV12, Y41P, YUY2, YVYU, UYVY, IYUV, YUYV, YVU9, Y411, Y211, MPEG2_AUDIO, DOLBY_AC3

### Publishing (SkyDrive/OneDrive)
- `WLXMediaPublishSubscribe.dll` — publish plugin
- `PluginPublishItems()` — upload items
- `LaunchAuthBrowser()` — authentication
- `GetLiveSignInProvider()` — Live ID sign-in
- `wlidcli.dll` (delay) — Windows Live ID
- `PhotoGalleryPublishSession` / `PublishParameters` — session objects
- Album management: `AlbumId/AlbumName/AlbumURL/AlbumThumbnail/AlbumPeopleSyncManager`

### Panoramic Stitch
- `PanoramicStitchCameraMotion` — motion detection
- `PanoramicStitchMapType` — stitching map
- `PanoramicStitchTheta0/1/Phi0/1` — projection angles

### Effects/Editing
- `Microsoft.Imaging.Effects.CropEffect` — crop (from Imaging.dll)
- `CropEffect.Mode / CropArea` — crop parameters
- `StraightenEffect.AutoCrop` — auto-crop after straighten
- `AutoAdjustEditsEnabled` — auto-enhance

### Email Integration
- `SMAPI.DLL` — Simple MAPI for email sharing

### Codec Support
Same as WLXPhotoAcq.dll — DownloadableCodecs, DownloadableWLRawCodecs, RawPhotoExtensions, QuickTimeMovieExtensions, SuppressFileTypes

## Key Registry Paths

| Registry Path | Purpose |
|---------------|---------|
| `Software\Microsoft\Windows Live\Photo Gallery` | Main settings |
| `Software\Microsoft\Windows Live\Photo Gallery\Library` | Library paths |
| `Software\Microsoft\Windows Live\Photo Gallery\Library\AddFolders` | Added folders |
| `Software\Microsoft\Windows Live\Photo Gallery\Library\Suppressed` | Suppressed items |
| `Software\Microsoft\Windows Live\Photo Gallery\Library\MDSyncInFlight\*` | Metadata sync |
| `Software\Microsoft\Windows Live\Photo Gallery\Viewer` | Viewer settings |
| `Software\Microsoft\Windows Live\Photo Gallery\Slideshow` | Slideshow settings |
| `Software\Microsoft\Windows Live\Photo Gallery\QuickPublishMru\*` | Quick publish MRU |
| `Software\Microsoft\Windows Live\Photo Gallery\PublishPluginsMru` | Publish plugins MRU |
| `Software\Microsoft\Windows Live\Photo Gallery\SuppressedDialogs` | Suppressed dialogs |
| `Software\Microsoft\Windows Live\Photo Gallery\FileSupport\*` | File type support |
| `Software\Microsoft\Windows Live\Photo Gallery\WLRaw\*` | RAW codec version |
| `Software\Microsoft\Windows Live\Photo Gallery\Extensibility\*` | Plugin extensibility |
| `Software\Microsoft\Windows Live\Environment\PhotoGallery` | Environment paths |
| `Software\Microsoft\Windows Live\Common` | Common settings |
| `SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\%ws` | Property handlers |
| `SOFTWARE\Microsoft\Windows\CurrentVersion\Media Center\Decoder` | MCE decoder |
| `Software\Microsoft\Windows Media Foundation\ByteStreamHandlers` | MF stream handlers |
| `Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers` | Autoplay |
| `Software\Microsoft\Windows Live\Communications Clients\Shared` | Communication clients |
| `SOFTWARE\Classes\Applications\WLXPhotoViewer.dll\shell\open\command` | Photo viewer |

## Key Strings

- `WLX_Photo_Library_MainWnd` — main window class name
- `WLXPhotoGallery_Exe` — main executable reference
- `PhotoGallery` — module name
- `WLX_Photo_Library` — library identifier
- `SupportedMediaTypes` / `MaxPhotoWidth` / `MaxPhotoHeight` / `MaxVideoBitrate` / `MaxVideoFramerate` — media limits
- `GalleryScopedFolders` — scoped folder list
- `SearchStatus` / `CommitEdits` — UI state
- `WindowsLive.PhotoGallery.16.4` — registered application name
- `WLXPhotoViewer.dll` / `PhotoViewer.dll` / `shimgvw.dll` — viewer DLL chain
- `WLXPhotoGallery.exe` — main executable
- `WLXPGSS.scr` — screensaver
- `MovieMaker.exe` — Movie Maker reference
- `rundll32.exe` — rundll32 for DLL loading
- `Microsoft Research AutoCollage Touch 2009 Module` — AutoCollage plugin
- `https://profile.live.com/Services/?view=manage` — SkyDrive management URL

## Dependencies Summary

| DLL | Functions | Role |
|-----|-----------|------|
| MSVCR110.dll | 100+ | CRT core + math |
| MSVCP110.dll | 6 | C++ stdlib |
| KERNEL32.dll | 100+ | OS kernel |
| USER32.dll | 50+ | Window/message |
| ADVAPI32.dll | 12 | Registry + ETW |
| GDI32.dll | 45+ | Graphics |
| ole32.dll | 13 | COM core + clipboard |
| OLEAUT32.dll | 33 ordinals | COM automation |
| PROPSYS.dll | 8 | Property system |
| OLEACC.dll | 3 | Accessibility |
| SHLWAPI.dll | 38 | Path/string utilities |
| gdiplus.dll | 100+ | Full GDI+ rendering |
| UxTheme.dll | 9 | Visual styles |
| WININET.dll | 1 | Network connectivity |
| MF.dll | 1 | Media Foundation |
| MFPlat.DLL | 11 | MF platform |
| d3d11.dll | 1 | Direct3D 11 |
| dxva2.dll | 2 | Video acceleration |
| WINHTTP.dll | 14 | HTTP client |
| WLXPhotoBase.dll | 32 | Memory/exception/version |
| WLXPhotoSqm.dll | 30+ | SQM telemetry |
| UXCore.dll | 150+ | DirectUI framework |
| uxctl.dll | 3 | UxControls |
| WLXMediaPublishSubscribe.dll | 5 | SkyDrive publishing |
| WLXPhotoLibraryDatabase.dll (delay) | 17 | Database operations |
| STI.dll (delay) | 1 | WIA/scanner |
| d3d9.dll (delay) | 2 | Direct3D 9 fallback |
| MetadataSys.dll (delay) | 1 | Metadata properties |
| WindowsCodecs.dll (delay) | 1 | WIC |
| XmlLite.dll (delay) | 2 | XML writing |
| dwmapi.dll (delay) | 1 | DWM composition |
| urlmon.dll (delay) | 1 | URL download |
| msi.dll (delay) | 3 ordinals | MSI installer |

## Architecture Summary

WLXPhotoLibraryMain.dll is the main application logic DLL for Windows Live Photo Gallery 2012, weighing in at 3.3 MB with 2.8 MB of code — the largest binary in the suite. It implements the complete gallery UI using the DirectUI framework (via UXCore.dll) with 150+ UI element types, navigation trees, list views, search/filter system, tag editing, face tagging, hover previews, and slideshow. The DLL handles photo/video browsing, editing (crop, rotate, auto-enhance), video processing via Media Foundation (MFPlat + D3D11/DXVA2), panoramic stitching, and publishing to SkyDrive/OneDrive via WLXMediaPublishSubscribe.dll. It exports only 3 functions: `DllCanUnloadNow`, `RunAsStandAlone` (standalone launch), and `DisplayRepairPromptDialog` (database repair). The DLL delay-loads WLXPhotoLibraryDatabase.dll for all database operations, STI.dll for scanner access, d3d9.dll for fallback rendering, and various other DLLs for extensibility. The DirectUI XML resource system stores UI definitions in external resource files rather than embedded resources, explaining the tiny 1 KB `.rsrc` section.
