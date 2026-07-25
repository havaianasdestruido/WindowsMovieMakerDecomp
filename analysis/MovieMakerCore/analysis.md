# MovieMakerCore.dll - Complete Static Analysis

## 1. Binary Overview

| Property | Value |
|----------|-------|
| **File** | MovieMakerCore.dll |
| **Type** | DLL (PE32, x86) |
| **Image Base** | 0x10000000 |
| **Entry Point** | 0x104C8E3D |
| **Subsystem** | Windows GUI (2) |
| **Linker Version** | 11.00 (MSVC 11.0 / VS2012) |
| **OS Version** | 6.02 (Windows 8) |
| **Timestamp** | 0x533A3FDC (Tue Apr 1 01:26:04 2014) |
| **DLL Characteristics** | Dynamic base (ASLR), NX compatible (DEP) |
| **Stack Reserve/Commit** | 0x40000 / 0x1000 |
| **Heap Reserve/Commit** | 0x100000 / 0x1000 |
| **Checksum** | 0xA23F3E |
| **PDB GUID** | D5217874-B614-477C-B45B-E0CE638C6496 |
| **PDB Name** | MovieMakerCore.pdb |
| **Total Size of Image** | 0xA22000 (10,626,048 bytes, ~10.1 MB) |

## 2. Section Layout

| Section | Virtual Address | Virtual Size | Raw Size | Flags |
|---------|----------------|-------------|----------|-------|
| **.text** | 0x1000 | 0x574654 (5,719,636) | 0x574800 | Code, Execute Read |
| **.data** | 0x576000 | 0x3929C (234,140) | 0x2E200 | Initialized Data, Read Write |
| **.rsrc** | 0x5B0000 | 0x40B478 (4,240,504) | 0x40B600 | Initialized Data, Read Only |
| **.reloc** | 0x9BC000 | 0x6593E (415,550) | 0x65A00 | Initialized Data, Discardable, Read Only |

**Key observations:**
- The `.text` section is ~5.46 MB — massive code size indicating this DLL contains the entire application logic.
- The `.rsrc` section is ~4.07 MB — contains extensive UI resources (DirectUI layouts, icons, strings, templates).
- The `.data` section is ~228 KB — global variables, vtables, RTTI data.
- The `.reloc` section is ~400 KB — supports ASLR relocation.

## 3. Export Table

### Original Binary Exports

| Ordinal | Hint | RVA | Name | Calling Convention |
|---------|------|-----|------|--------------------|
| 1 | 0 | 0x000E96C0 | `MovieMakerMain` | `__cdecl` |

**This is the ONLY exported function.** The DLL exports a single entry point:
```c
extern "C" int __cdecl MovieMakerMain(int argc, wchar_t** argv);
```

### Reconstructed Source Exports (exports.h)

The reconstructed `exports.h` defines **four** export macro families:
- `MOVIECORE_API` — general DLL exports
- `RIBBON_API` — ribbon-related exports
- `STORYBOARD_API` — storyboard/timeline exports
- `DATASTRUCT_API` — data structure exports

**However, none of these macros are used in the exports.h file itself** — it only defines the macro declarations. The actual `MovieMakerMain` function is declared in `MovieMakerCore.h` using `MOVIECORE_API`.

### Comparison: Original vs Reconstructed

| Aspect | Original Binary | Reconstructed Source |
|--------|----------------|---------------------|
| **Exported functions** | 1 (`MovieMakerMain`) | 1 (`MovieMakerMain`) |
| **Export macro families** | N/A (single function) | 4 defined (MOVIECORE_API, RIBBON_API, STORYBOARD_API, DATASTRUCT_API) |
| **Internal functions** | Not exported (accessed via COM/vtables) | `MovieCore_Initialize`, `MovieCore_Shutdown`, `MovieCore_GetInstance`, etc. — marked `extern "C"` but NOT exported |
| **COM objects** | Via internal COM registration | 4 ATL COM classes registered via OBJECT_MAP |

**Key difference:** The original DLL has a single exported function. The reconstructed source adds several internal helper functions and defines additional export macros (`RIBBON_API`, `STORYBOARD_API`, `DATASTRUCT_API`) that don't correspond to any actual exports in the original binary. This suggests the reconstructed source anticipated exposing more API surface than the original DLL actually did.

The original DLL's architecture is entirely based on **internal class vtables and COM interfaces** rather than flat C exports. All 125 RTTI classes and their functionality are accessed through COM/ATL registration and internal dispatch.

## 4. Import Table (Complete)

### System DLLs

| DLL | Function Count | Key Functions |
|-----|---------------|---------------|
| **KERNEL32.dll** | 141 | CreateFileW, CreateThread, GetModuleHandleW, LoadLibraryW, VirtualAlloc, CreateIoCompletionPort, etc. |
| **USER32.dll** | 105 | CreateWindowExW, GetMessageW, SendMessageW, RegisterClassExW, DefWindowProcW, etc. |
| **GDI32.dll** | 18 | CreateDIBSection, BitBlt, DeleteObject, GetDeviceCaps, etc. |
| **ADVAPI32.dll** | 25 | RegOpenKeyExW, RegQueryValueExW, CryptAcquireContextW, TraceMessage, RegisterTraceGuidsW, etc. |
| **SHELL32.dll** | 19 | SHGetDesktopFolder, SHCreateItemFromParsingName, ShellExecuteW, etc. |
| **SHLWAPI.dll** | 24 | PathFileExistsW, PathCanonicalizeW, StrCmpIW, SHCreateStreamOnFileW, etc. |
| **ole32.dll** | 23 | CoCreateInstance, CoInitializeEx, CreateStreamOnHGlobal, StgOpenStorage, etc. |
| **OLEAUT32.dll** | 22 (ordinals) | SysAllocString, SysFreeString, VariantInit, SafeArray functions, etc. |

### Windows Live / Microsoft DLLs

| DLL | Function Count | Purpose |
|-----|---------------|---------|
| **UXCore.dll** | 180+ | DirectUI UI framework — CDUIDialog, Element, HWNDElement, VirtualLayout, etc. |
| **WLXPhotoBase.dll** | 14 | Base utility library — Exception handling, OS version checks, memory management |
| **WLXPhotoSqm.dll** | 13 | SQM telemetry — Sqm::Startup, AddToStream, ReportAppLaunchStatus |
| **MetadataSys.dll** | 1 | WLXPSGetItemPropertyHandler |
| **DmxBici.dll** | 5 | BiciWrapper telemetry — StartExperience, EndExperience, AddToDataPoint |
| **wlidcli.dll** | 7 (ordinals) | Windows Live ID client |
| **uxctl.dll** | 3 | UxControlsInitProcess, UxControlsCreateObject, UxControlsUninitProcess |

### Media Framework DLLs

| DLL | Function Count | Purpose |
|-----|---------------|---------|
| **MF.dll** | 2 | MFGetService, MFTranscodeGetAudioOutputAvailableTypes |
| **MFPlat.DLL** | 18 | MFStartup, MFCreateMediaType, MFCreateSample, MFCreateAttributes, etc. |

### Graphics DLLs

| DLL | Function Count | Purpose |
|-----|---------------|---------|
| **gdiplus.dll** | 24 | GdipCreateFromHDC, GdipDrawImageRectRect, GdipCreateFont, GdiplusStartup, etc. |
| **d3d11.dll** | 1 | D3D11CreateDevice |
| **d3d9.dll** | 2 | Direct3DCreate9, Direct3DCreate9Ex |
| **d2d1.dll** | 1 (ordinal 1) | D2D1CreateFactory (via ordinal) |
| **DWrite.dll** | 1 | DWriteCreateFactory |
| **dxva2.dll** | 2 | DXVA2CreateDirect3DDeviceManager9, DXVA2CreateVideoService |
| **dwmapi.dll** | 1 | DwmExtendFrameIntoClientArea |
| **UxTheme.dll** | 3 | OpenThemeData, GetThemeMetric, CloseThemeData |
| **D3DCOMPILER_46.dll** | 2 | D3DGetInputSignatureBlob, D3DReflect |
| **WindowsCodecs.dll** | 1 | WICConvertBitmapSource |

### Other DLLs

| DLL | Function Count | Purpose |
|-----|---------------|---------|
| **VERSION.dll** | 3 | GetFileVersionInfoSizeW, GetFileVersionInfoW, VerQueryValueW |
| **WINMM.dll** | 1 | PlaySoundW |
| **XmlLite.dll** | 4 | CreateXmlReader, CreateXmlWriter, etc. |
| **PROPSYS.dll** | 8 | PSGetPropertyKeyFromName, PropVariantChangeType, etc. |
| **OLEACC.dll** | 2 | AccessibleChildren, AccessibleObjectFromWindow |
| **ESENT.dll** | 28 | Jet database engine — JetCreateInstanceW, JetOpenDatabaseW, etc. |

**Total imported functions: ~630+**

## 5. COM GUIDs Found in Binary

### Confirmed GUIDs (from binary strings)

| GUID | Context | Purpose |
|------|---------|---------|
| `{9590CDCE-BFFC-45d5-85EA-493C65FB2C4F}` | Near transition/effect names (Wheel, Circles, Heart, etc.) | **Transition/Effect type GUID** — likely identifies a specific transition template |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Near `//ItemSet/Item/ErrorCode` XML path, `http://g.live.com` | **Telemetry/error reporting endpoint ID** — used for error code reporting to Microsoft servers |
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | Near `Microsoft\Live\MovieMaker`, `APPID`, `Module`, `REGISTRY` | **Application/module identifier GUID** — appears to be the COM APPID for MovieMakerCore |

### Certificate-related GUIDs (from PE Authenticode signature)

| GUID | Purpose |
|------|---------|
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Microsoft Code Signing PCA certificate ID |
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Microsoft Code Signing PCA 2011 certificate ID |

### GUIDs in Reconstructed Source (NOT in original binary)

| GUID | Source File | Purpose |
|------|------------|---------|
| `{A1B2C3D4-E5F6-4829-9B01-234567890ABC}` | ComFactory.h | CLSID_StoryboardManagerObj (placeholder) |
| `{B2C3D4E5-F6A7-4930-A112-34567890ABCD}` | ComFactory.h | CLSID_MovieProjectObj (placeholder) |
| `{C3D4E5F6-A7B8-4041-B223-4567890ABCDE}` | ComFactory.h | CLSID_TimelineTrackObj (placeholder) |
| `{D4E5F6A7-B8C9-4152-C334-567890ABCDEF}` | ComFactory.h | CLSID_SerializationWriterObj (placeholder) |

**Important:** The reconstructed CLSIDs are clearly placeholder/dummy values (sequential hex pattern). The original binary does NOT export any COM class factories — instead, the COM objects are registered internally via ATL's OBJECT_MAP and `_Module.RegisterClassObjects()`. The real COM CLSIDs are likely embedded in the resource section (.rsrc) as REGISTRY resources.

## 6. DllMain Entry Point Analysis

The reconstructed `dllmain.cpp` shows the expected flow:

```
DllMain(hModule, DLL_PROCESS_ATTACH):
  1. g_hInstance = hModule
  2. DisableThreadLibraryCalls(hModule)  // Performance optimization
  // Full init deferred to MovieMakerMain

ShutdownSubsystems() (in DLL_PROCESS_DETACH):
  1. Release WIC factory
  2. Release DirectWrite factory
  3. Release Direct2D factory
  4. Release D3D11 device/context
  5. Uninit DirectUI
  6. Uninit UXCore
  7. MFShutdown()
  8. GdiplusShutdown()
  9. _Module.RevokeClassObjects() + _Module.Term()
  10. CoUninitialize()
```

The initialization order is: COM → ATL Module → GDI+ → Media Foundation → UXCore → DirectUI → D3D11 → D2D → DirectWrite → WIC.

## 7. RTTI Class Inventory

**125 unique C++ classes** identified via RTTI type_info names:

### Application Framework Classes
- `SundanceAppMain` — Main application object
- `CommandLineParser` — Command-line parsing
- `AutoSaveManager` — Auto-save functionality
- `MediaBrowser` — Media file browser
- `TemplateTable` — Template management
- `ClipboardManager` — Clipboard operations

### Timeline Classes
- `TimelineBaseBehavior` — Base timeline UI behavior
- `TimelineBehavior` — Main timeline behavior
- `TimelineDataSource` — Timeline data binding
- `TimelineDragDrop` — Drag-drop in timeline
- `TimelineItemBehavior` — Timeline item rendering
- `TimelineItemInputBehavior` — Timeline item interaction
- `TimelineInstructionsBehavior` — Help text overlay
- `TimelineSelectionRootBehavior` — Selection management
- `TimelineVisualTrackItemBehavior` — Visual track rendering
- `TimelineSecondaryTrackItemBehavior` — Audio/secondary tracks
- `TimelineTemplateSource` — Template rendering
- `TimelineLayoutMode` — Layout mode management
- `TimelineExtentUIObject` — Extent rendering

### Text/Title Classes
- `TextBoxBehavior` — Text editing
- `LegacyTextExtent` — Legacy text support
- `LegacyParagraph` — Legacy paragraph support
- `CFontSite`, `CColorPickerSite` — Font/color pickers
- `CaretBehavior`, `CaretScrollBehavior` — Caret management
- `RichEditControlBehavior` — Rich text editing

### Media/Encoding Classes
- `AudioCaptureSession` — Audio recording
- `AVCaptureCore`, `AVCaptureSession` — Webcam capture
- `WebcamElementBehavior`, `WebcamUI` — Webcam UI
- `NarrationUI` — Narration recording
- `TrimBehavior` — Video trimming
- `UserEncodeProfileBehavior` — Encode profile selection
- `UserEncodeProfileDialog` — Encode settings dialog
- `UserEncodeProfileInfo` — Encode profile data
- `UserEncodeProfileRecommended` — Recommended profiles
- `UserEncodeProfileWLVS` — WLVS profile support
- `ChunkMediaFileList` — Chunked media files
- `CommandLineMediaFileList` — CLI file list

### Publishing Classes
- `PublishManager` — Publish orchestration
- `PublishJob` — Individual publish job
- `PublishBackgroundJob` — Background publishing
- `PublishBackgroundWorker` — Worker thread
- `PublishSummaryDialog` — Summary dialog
- `PublishProgressCallBack` — Progress reporting
- `PublishItemProperties` — Publish item properties
- `PublishItemPropertyStore` — Property storage

### UI Behavior Classes
- `SundanceMainElementBehavior` — Main element
- `AboveHomerBehavior` — Above preview area
- `HomerHeavyLayerBehavior` — Heavy preview layer
- `StandardLayerBehavior` — Standard layer
- `DuiLayerBehaviorImpl` — DirectUI layer
- `InlinePreviewLayoutBehavior` — Inline preview
- `InlinePreviewSliderBehavior` — Preview slider
- `FullscreenLayoutBehavior` — Fullscreen layout
- `FullscreenBackgroundWindow` — Fullscreen background
- `ProjectWorkspaceLayoutBehavior` — Workspace layout
- `ResizeablePaneBehavior` — Resizable panes
- `AMPCommandBarBehavior` — AMP command bar
- `AMPMainWindowBehavior` — AMP main window
- `AMPDataContext` — AMP data context
- `AmpFadeAnimationBehavior` — Fade animation
- `AmpPreventSparseModeBehavior` — Sparse mode prevention
- `AmpSlideAnimationBehavior` — Slide animation
- `MultipleEffectBehavior` — Multiple effects
- `MultipleEffectDialog` — Effects dialog
- `OptionsDialogBehavior` — Options dialog
- `HelpBehavior` — Help display
- `CaptureUIBehavior` — Capture UI
- `PopUpSlider` — Popup slider control
- `ProgressBase`, `ProgressDialog`, `ProgressStatusBar` — Progress UI

### Dialog/Host Classes
- `CDUIDialog` — DirectUI dialog (from UXCore.dll)
- `CFramelessHost` — Frameless window host
- `CSundanceDialog` — Sundance dialog base
- `CMsgFilter` — Message filter
- `SundanceNativeHwndHost` — Native HWND host
- `SundanceClipboardChainWindow` — Clipboard chain
- `DontShowPromptDialog` — "Don't show again" dialog
- `SundanceDontShowPromptDialog` — Sundance version
- `SundanceApplicationOptionsDialog` — Options

### Data/Model Classes
- `SundanceAppDataContext` — Application data context
- `PreviewDataContext` — Preview data
- `PreviewPresenterWrapper` — Preview presenter
- `AMPDataContext` — AMP data context
- `SelectionRootImpl`, `SelectionRootImplWrapper` — Selection
- `SelectionIndex` — Selection index
- `ComplexProperty`, `SingleProperty`, `TransformProperty` — Properties
- `FilenameList`, `FilenameArrayList` — Filename lists
- `LegacyExtent`, `LegacyTransform` — Legacy support
- `LegacyProjectSupport` — Legacy project loading
- `CachedWFSection` — Cached waveform section
- `SFTime` — Time representation
- `ContactStore` — Contact store

### Site/Factory Classes
- `CMRUSite` — MRU (Most Recently Used) site
- `CSpinnerSite` — Spinner site
- `CGenericSite` — Generic site
- `CGroupSite` — Group site
- `CGallerySite` — Gallery site
- `ToggleSite` — Toggle site
- `CFontSite` — Font site
- `CColorPickerSite` — Color picker site
- `SundanceBehaviorFactory` — Behavior factory

### Ribbon Classes
- `RibbonApp` — Ribbon application
- `RibbonCategoryItem` — Ribbon category item
- `RibbonCategoryList` — Ribbon category list
- `RibbonList` — Ribbon list
- `RibbonListItem` — Ribbon list item

### Other
- `CMRUItem` — MRU item
- `ParsePhotoGalleryTransferFile` — Photo Gallery transfer
- `UXBrush` — UX brush wrapper
- `ExtentIdSetSelectionRangeIterator` — Extent range iterator
- `type_info` — C++ RTTI

## 8. String Literals Found

### Application Identity
- `"Windows Live Movie Maker"` — Product name
- `"WindowsLiveMovieMakerMain"` — Main window class name
- `"WindowLiveMovieMakerMainWindowClass"` — Window class name
- `"WindowLiveMovieMakerFwdCmdMapping"` — Forward command mapping class
- `"MovieMakerCore"` — DLL internal name
- `"Movie Maker"` — Product name (short)
- `"16.4.3528.0331"` — Full version string
- `"Sundance"` — Internal codename
- `"WLPGMovieMaker_SelectedSet_16.4.3528.0331"` — Clipboard format
- `"WLPGMovieMaker_SerializedProject_16.4.3528.0331"` — Clipboard format
- `"_SerializedProject"` — Serialization marker

### Serialization / XML Paths
- `"//MovieMaker/Project/DataStr"` — Project data structure XPath
- `"//TiEffectArr[@UID="%s"]/UID"` — Effect array XPath
- `"//TiEffect[@UID="%s"]/TiEffectPtr"` — Effect pointer XPath
- `"//TiTransition[@UID="%s"]/TTFrom"` — Transition from XPath
- `"//TiTransition[@UID="%s"]/TiTransitionPtr"` — Transition pointer XPath
- `"//TIArr/UID[@UID="%i"]"` — Track item array XPath
- `"//Track[@TrackTyp="0"]"` — Track type XPath
- `"//ItemSet/Item/ErrorCode"` — Error code XPath
- `" version='1.0' encoding='UTF-8'"` — XML declaration
- `"AutoSave.wlmp"` — Auto-save filename
- `"AutoSaveValidate.wlmp"` — Auto-save validation

### Registry Paths
- `"Software\Microsoft\Windows Live\Movie Maker"` — Main app settings
- `"Software\Microsoft\Windows Live\Movie Maker\Post"` — Post-operation settings
- `"Software\Microsoft\Windows Live\Movie Maker\Suppressed"` — Suppressed prompts
- `"Software\Microsoft\Windows Live\Movie Maker\Recent"` — Recent files
- `"Software\Microsoft\Windows Live\Movie Maker\RecentWLVS"` — Recent video profiles
- `"Software\Microsoft\Windows Live\Photo Gallery"` — Photo Gallery settings
- `"Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs"` — Raw codec settings
- `"Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs"` — Codec settings
- `"Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes"` — Suppressed file types
- `"Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions"` — QuickTime support
- `"Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions"` — Raw photo support
- `"Software\Microsoft\Homer"` — Homer component settings
- `"Software\Microsoft\Windows Live\Installer"` — Installer settings
- `"Software\Microsoft\Windows Live\Common"` — Common WL settings
- `"Software\Microsoft\Windows Live\Environment\PhotoGallery"` — Gallery environment
- `"Software\Microsoft\Windows Media Foundation\ByteStreamHandlers"` — MF byte stream handlers
- `"Software\RegisteredApplications"` — Registered applications
- `"SOFTWARE\Classes"` — COM class registrations
- `"%LOCALAPPDATA%\Microsoft\Windows Live Movie Maker"` — Local app data path

### COM / Module Strings
- `"Microsoft\Live\MovieMaker"` — COM namespace
- `"Microsoft\Live\MovieMaker.MovieMakerLang"` — Language DLL reference
- `"APPID"` — Application ID marker
- `"Module"` / `"Module_Raw"` — Module type markers
- `"REGISTRY"` — Registry storage type
- `"HKCR"` / `"HKCU"` / `"HKLM"` / `"HKU"` / `"HKPD"` / `"HKDD"` / `"HKCC"` — Registry hive abbreviations

### Timeline Types
- `"TmlnVideoItem | TmlnVideoOnlyItem | TmlnStillItem | TmlnAudioItem | TiTitleSource | TiTitleOverlay"` — Timeline item types
- `"ProducerData"`, `"ShellLink.Producer"`, `"Dat"` — Producer types

### Property IDs
- `"PropertyID_AppPaneEnabled"`
- `"PropertyID_AppItemCountAndPositionString"`
- `"PropertyID_TimelineIsEmpty"`
- `"PropertyID_TimelineIsNotEmpty"`
- `"PropertyID_PreviewPlayPosition"`
- `"PropertyID_PreviewSliderPosition"`
- `"PropertyID_PreviewSliderMax"`

### UI Element Names
- `"ideStoryboardPane"`, `"idePreviewPlusSplitter"`, `"ideDropPreview"`
- `"ideTimelineScrollbar"`, `"ideThumbZoomButton"`, `"ideWebCamPreview"`
- `"idePreviewPane"`, `"ideHomerHeavyPreview"`, `"ideHomerAbove"`
- `"idrSundanceMainWindow"`, `"idrAutoMoviePlayerWindow"`, `"idrAboveHomerWindow"`
- `"idrOptionsDialog"`, `"ProgressDialogBox"`
- `"ideInlinePreviewSliderParent"`, `"ideTimelineInstructionsText"`
- `"ideTimeline"`, `"ideSundanceMainWindow"`
- `"MovieMaker_PopUpSlider"`
- `"MicrosoftTabletPenServiceProperty"`

### Resource IDs
- `"IDR_BRIGHTNESSCONTROL_BRIGHTERICON_"`, `"IDR_BRIGHTNESSCONTROL_DARKERICON_"`
- `"IDR_MIXCONTROL_MOVIEICON_"`, `"IDR_MIXCONTROL_NARRATIONICON_"`
- `"IDR_MIXCONTROL_SOUNDTRACKICON_"`, `"IDR_MIXCONTROL_VOLUMEMINUSICON_"`
- `"IDR_MIXCONTROL_VOLUMEPLUSICON_"`, `"IDR_TRANSPARENCY_MINUSICON_"`, `"IDR_TRANSPARENCY_PLUSICON_"`

### Template/Effect Names
- `"Video Profiles"` — Video profile template category
- `"TemplateEffectKind"`, `"TransitionCompatibility"`, `"TemplateTitle"`, `"TemplateCategory"`, `"TemplateThumbnailPath"`
- `"FanOut"`, `"MIRRORED_REPEAT"`, `"CLAMP_TO_EDGE"`, `"ITALIC"`, `"Rectangles"`, `"BOLDITALIC"`
- `"Wheel"`, `"Circles"`, `"PLAIN"`, `"CLAMP"`, `"BOLD"`
- `"Heart"`, `"Split"`, `"SweepUp"`, `"Keyhole"`
- `"TextEffectTemplate"`, `"TwoToneDissolve"`

### DirectUI Layout Element Names
- `"MainElement"`, `"ProjectWorkspaceLayout"`, `"Help"`
- `"InlinePreviewSliderBehavior"`, `"InlinePreviewLayoutBehavior"`, `"ResizeablePane"`
- `"Timeline"`, `"TimelineBase"`, `"TimelineInstructions"`, `"TimelineItem"`, `"TimelineItemInput"`
- `"TimelineVisualTrack"`, `"TimelineSecondaryTrack"`
- `"RichEditControl"`, `"AboveHomer"`, `"TextBox"`, `"Hcdpi"`, `"CaretScroll"`, `"MultipleEffect"`
- `"WebcamElementBehavior"`, `"HomerHeavyLayerBehavior"`, `"StandardLayerBehavior"`
- `"AMPCommandBarBehavior"`, `"AMPMainWindowBehavior"`, `"UserEncodeProfile"`
- `"FullscreenBackground"`, `"FullscreenLayout"`

### Media Format Strings
- `".mp4"`, `".wmv"`, `".m4a"`, `".wma"` — Media output formats
- `".wlmp"` — Movie Maker project format
- `".wlvs"` — Video profile format
- `"A.*.wlvs"` — WLVS file pattern
- `".mpg"`, `".wmv"`, `".asf"`, `".avi"`, `".mpv"`, `".m1v"`, `".m2v"`, `.mpeg"`, `".qt"`, `".mov"`, `".wm"`, `".mpe"`, `".3g2"`, `".3gpp"`, `".3gp"`, `".mqv"`, `".rle"`, `".gif"`, `".ico"`, `".3gp2"`, `".mpv2"`, `".mp2v"`, `".mp2"`, `".dib"`, `".mod"`, `".vob"` — Supported input formats

### Error/Status Strings
- `"<Failed: 0x%08X>"` — DXVA mode failure
- `"Unsupported Tag: "%s""` — XML parsing error
- `"error"`, `"ignorableWarning"`, `"fatalError"` — XML severity levels
- `"Error"`, `"Warning"`, `"Fatal"` — Profile error levels
- `"profile.audio.title.props.video.attribute.container.label.high32..low32.UINT64..UINT32..GUID..DOUBLE..BLOB..STRING"` — Profile data type descriptors
- `"AVS_E_NON_SEEKABLE_FILE"`, `"AVS_E_SAMPLE_OVERSEEK"` — AV Stream error codes
- `"S_FALSE"`, `"S_OK"`, `"E_INVALIDARG"`, `"E_FAIL"`, `"MF_E_PLATFORM_NOT_INITIALIZED"`, `"E_UNEXPECTED"`, `"MF_E_INVALIDREQUEST"`, `"MF_E_BUFFERTOOSMALL"` — HRESULT string representations
- `"%02X"`, `" %02X"` — Hex formatting for data
- `"<NULL>"`, `"NULL"` — Null representation strings
- `"<Too Long>"` — String truncation indicator
- `"<IUnknown*>"` — Unknown COM object representation
- `"<BLOB>"` — Binary data representation

### Telemetry / URLs
- `"http://g.live.com"` — Production telemetry endpoint
- `"http://g.live-int.com"` — Internal/test telemetry endpoint
- `"%s?version=%u.%u.%04u.%04u&ErrorCode=%x"` — Error report URL template
- `"Learn more....<A HREF="%s?version=%u.%u.%04u.%04u&ErrorCode=%x">%s</A>"` — Learn more link template
- `"WindowsLive.PublishPlugins.ThumbnailImage"` — Publishing plugin thumbnail

### Version Info (from PE resources)
- `"FileVersion"`: `"16.4.3528.0331_ship.client.main.w5m4 (ship)"`
- `"InternalName"`: `"MovieMakerCore"`
- `"LegalCopyright"`: `"(c) 2012 Microsoft Corporation. All rights reserved."`
- `"OriginalFilename"`: `"MovieMakerCore.DLL"`
- `"ProductName"`: `"Movie Maker"`
- `"ProductVersion"`: `"16.4.3528.0331"`

### Animation Direction Enums
- `"outALittle"`, `"right"`, `"left"`, `"up"`, `"down"`
- `"yawRight"`, `"yawLeft"`, `"pitchUp"`, `"pitchDown"`, `"rollCw"`, `"rollCCw"`
- `"upLeft"`, `"upRight"`, `"downLeft"`, `"downRight"`, `"placeholder"`

### Misc Strings
- `"Content Type"`, `"FileAssociations"` — MIME type handling
- `"TypeLib"`, `"none"`, `"in"`, `"out"` — Type library markers
- `"Mime"`, `"SAM"`, `"SECURITY"`, `"SYSTEM"`, `"Software"` — Section headers
- `"ShortCatalogRescan"`, `"ShortCatalogTimeStamp"` — Catalog management
- `"ProductStatus"`, `"TOUVersion"` — `"16.0.0.0"` — Installer/product status
- `"SuiteLanguage"`, `"FREDate"`, `"UserLanguage"`, `"InstalledLanguages"` — Localization settings
- `"%s/%s%s/%u"` — URL path template
- `"INT"` — Integer format marker
- `"%g"` — Float format
- `"(%g,%g)-(%ux%u)"` — Rect format
- `"%0.2f"`, `"-%.2f"` — Float formatting

## 9. DXVA/Media Profile GUID Strings

Found as string representations (not binary GUIDs):
- `"GUID_NULL"` — Null GUID reference
- `"DXVA_ModeMPEG2_A"`, `"DXVA_ModeMPEG2_B"`, `"DXVA_ModeMPEG2_C"`, `"DXVA_ModeMPEG2_D"` — DXVA decode modes
- `"S_OK"`, `"S_FALSE"`, `"E_INVALIDARG"`, `"E_FAIL"`, `"E_UNEXPECTED"` — Standard HRESULT strings
- `"MF_E_PLATFORM_NOT_INITIALIZED"`, `"MF_E_INVALIDREQUEST"`, `"MF_E_BUFFERTOOSMALL"` — Media Foundation errors

## 10. Embedded Type Libraries

No embedded type library (`.tlb`) resources were found in the binary. The only `TypeLib` reference is a metadata keyword string `"TypeLib"` appearing in a data section, likely as part of a property descriptor table. All COM interfaces are defined through C++ vtables and RTTI, not through IDL/TypeLib.

## 11. Calling Conventions

- **Exported function (`MovieMakerMain`):** `__cdecl` (default for x86 C/C++)
- **UXCore.dll imports:** `__stdcall` (STDAPICALLTYPE — standard Win32/DLL convention)
- **All other imports:** Standard calling conventions per their respective APIs
- **COM vtable methods:** `__stdcall` (STDMETHODCALLTYPE)
- **Internal classes:** `__thiscall` (default C++ member function convention for MSVC x86)

## 12. Estimated Class Count

**125 classes** confirmed via RTTI data in the `.data` section. This is a significant C++ codebase.

Breakdown by category:
- **UI Behavior classes:** ~40 (timeline, preview, effects, dialogs)
- **Data/Model classes:** ~20 (properties, selections, file lists)
- **Site/Factory classes:** ~10 (UI site wrappers)
- **Dialog/Host classes:** ~12 (window hosting, dialogs)
- **Media/Encoding classes:** ~12 (capture, encoding, trimming)
- **Publishing classes:** ~8 (upload, export)
- **Timeline classes:** ~15 (tracks, items, drag-drop)
- **Text/Title classes:** ~6 (text editing, carets)
- **Ribbon classes:** ~5 (ribbon UI)
- **Application framework classes:** ~5 (main, options, save)
- **Other:** ~7 (clipboard, brush, templates)

## 13. Differences Between Original Exports and Reconstructed exports.h

| Aspect | Original | Reconstructed |
|--------|----------|---------------|
| **Number of exports** | 1 (`MovieMakerMain`) | 1 (`MovieMakerMain`) |
| **Export macros defined** | None (implicit dllexport) | 4 (`MOVIECORE_API`, `RIBBON_API`, `STORYBOARD_API`, `DATASTRUCT_API`) |
| **COM registration** | Internal only (via resource section) | Explicit ATL COM wrappers with dummy GUIDs |
| **Additional extern "C"** | None | `MovieCore_Initialize`, `MovieCore_Shutdown`, `MovieCore_GetInstance`, etc. |
| **Header file style** | N/A (header not separately exported) | Separate public header (`MovieMakerCore.h`) |

The reconstructed source correctly identifies that the DLL has a single export. However, it over-engineers the export infrastructure by defining four export macro families that are never actually used for exports. The additional `extern "C"` functions are internal helpers that exist in the source but are not part of the DLL's public API.

The reconstructed COM factory (ComFactory.h) creates 4 ATL COM objects with placeholder GUIDs. The original binary likely registers COM objects through ATL's resource-based registration mechanism (REGISTRY resources in .rsrc section), but the actual CLSIDs are not visible in the export table — they're embedded in the resource data.

## 14. Architecture Summary

MovieMakerCore.dll is a monolithic DLL containing the entire Windows Live Movie Maker application logic:

1. **Single export architecture** — The DLL exposes one function (`MovieMakerMain`) that serves as the application entry point. This is an unusual pattern for a DLL — it functions more like an EXE that was packaged as a DLL for modularity.

2. **Monolithic design** — 5.46 MB of code, 125+ C++ classes, all in one binary. The DLL handles everything from UI rendering to media processing to file I/O.

3. **DirectUI-based UI** — Extensive use of UXCore.dll's DirectUI framework for the entire application UI, including timeline, preview, properties, and publishing dialogs.

4. **Media Foundation pipeline** — Uses MF/MFPlat for media decoding, encoding, and transcoding, with DXVA2 for hardware acceleration.

5. **GDI+ and Direct3D 11 rendering** — Dual rendering pipeline: GDI+ for 2D UI elements, D3D11 for hardware-accelerated preview/rendering.

6. **ESENT database** — Uses Extensible Storage Engine (Jet) for internal data storage (likely thumbnail cache, project index, or MRU data).

7. **Telemetry integration** — Three telemetry systems: SQM (WLXPhotoSqm), BICI (DmxBici), and custom error reporting via HTTP.

8. **Windows Live ecosystem integration** — Deep integration with Photo Gallery, Windows Live ID, and the broader Windows Live Essentials suite.
