# MovieMakerCore.dll — Deep Static Analysis

## 1. Binary Overview

| Property | Value |
|----------|-------|
| **File** | MovieMakerCore.dll |
| **Type** | DLL (PE32, x86) |
| **Image Base** | 0x10000000 |
| **Image Size** | 0xA22000 (10,626,048 bytes, ~10.1 MB) |
| **Entry Point** | 0x104C8E3D (DllMain CRT init) |
| **Subsystem** | Windows GUI (2) |
| **Linker** | 11.00 (MSVC 11.0 / Visual Studio 2012) |
| **OS Version** | 6.02 (Windows 8) |
| **Timestamp** | 0x533A3FDC (Tue Apr 1 01:26:04 2014) |
| **DLL Characteristics** | Dynamic base (ASLR), NX compatible (DEP) |
| **Stack** | Reserve 0x40000 / Commit 0x1000 |
| **Heap** | Reserve 0x100000 / Commit 0x1000 |
| **Checksum** | 0xA23F3E |
| **PDB** | MovieMakerCore.pdb ({D5217874-B614-477C-B45B-E0CE638C6496}) |
| **Product Version** | 16.4.3528.0331 |
| **Copyright** | (c) 2012 Microsoft Corporation |
| **Internal Codename** | Sundance |

## 2. Section Layout

| Section | VA | Virtual Size | Raw Size | Flags |
|---------|----|-------------|----------|-------|
| `.text` | 0x1000 | 0x574654 (5.46 MB) | 0x574800 | Code, Exec Read |
| `.data` | 0x576000 | 0x3929C (229 KB) | 0x2E200 | Init Data, Read Write |
| `.rsrc` | 0x5B0000 | 0x40B478 (4.07 MB) | 0x40B600 | Init Data, Read Only |
| `.reloc` | 0x9BC000 | 0x6593E (406 KB) | 0x65A00 | Discardable, Read Only |

- `.text` = 5.46 MB — entire application logic in one section
- `.rsrc` = 4.07 MB — DirectUI layouts, icons, strings, templates, COM REGISTRY
- `.data` = 229 KB — global variables, vtables, RTTI data, vtable pointers
- `.reloc` = 406 KB — ASLR relocation table

## 3. Export Table

| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | 0x000E96C0 | `MovieMakerMain` |

**Single export function.** The DLL exposes exactly one entry point.

### MovieMakerMain Signature (from disassembly)

```
100E96C0: mov  edi,edi         ; hotpatch prologue
100E96C2: push ebp
100E96C3: mov  ebp,esp
100E96C5: push 0FFFFFFFFh     ; SEH frame
100E96C7: push 104CC469h       ; SEH handler
100E96D4: mov  eax,1268h       ; stack frame size = 0x1268 (4712 bytes!)
```

The function allocates a 4712-byte stack frame, indicating extensive local variable usage. It sets up SEH (Structured Exception Handling) with handler at 0x104CC469. The first parameter `argc` is accessed at `[ebp+8]`. The function returns a 32-bit int (observed: `0xC945001A` when called with minimal args — an internal error code).

### Calling Convention

`__cdecl` (x86 default) — caller cleans the stack. The prologue follows MSVC hotpatch convention (`mov edi,edi` / `push ebp` / `mov ebp,esp`).

## 4. Import Table — 33 DLLs, ~630 Functions

### 4.1 Core Windows APIs (5 DLLs, ~312 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **KERNEL32.dll** | 121 | CreateFileW, CreateThread, VirtualAlloc, CreateIoCompletionPort, LoadLibraryExW, GetModuleHandleW, QueryPerformanceCounter, SleepConditionVariableCS, CreateFileMappingW, MapViewOfFile, InterlockedCompareExchange, GetQueuedCompletionStatus, CreatePipe, CreateProcessW, HeapSetInformation, etc. |
| **USER32.dll** | 105 | CreateWindowExW, RegisterClassExW, DefWindowProcW, GetMessageW, TranslateMessage, DispatchMessageW, SendMessageW, SetWindowPos, SystemParametersInfoW, TrackPopupMenuEx, RegisterDeviceNotificationW, MonitorFromPoint, etc. |
| **GDI32.dll** | 18 | CreateDIBSection, BitBlt, SelectObject, GetDIBits, GetDeviceCaps, GetStockObject, ExtCreateRegion, GetRegionData, GetLayout/SetLayout |
| **ADVAPI32.dll** | 25 | RegCreateKeyExW, RegQueryValueExW, RegSetValueExW, CryptAcquireContextW, CryptSignHashW, CryptHashData, RegisterTraceGuidsW, TraceMessage, TraceEvent |
| **SHELL32.dll** | 19 | SHGetDesktopFolder, SHCreateItemFromParsingName, SHGetKnownFolderPath, ShellExecuteExW, CommandLineToArgvW, SHOpenFolderAndSelectItems, SHGetPropertyStoreFromParsingName |

### 4.2 COM/OLE (3 DLLs, ~47 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **ole32.dll** | 23 | CoCreateInstance, CoInitializeEx, CoSetProxyBlanket, CreateStreamOnHGlobal, StgOpenStorage, OleInitialize, OleSetClipboard, OleGetClipboard, PropVariantCopy/Clear, CLSIDFromString, StringFromCLSID/GUID2, CoTaskMemAlloc/Free |
| **OLEAUT32.dll** | 22 (ordinals) | SysAllocString, SysFreeString, SysStringLen, VariantInit, VariantClear, VariantCopy, VariantChangeType, SafeArrayCreateVector, SafeArrayDestroy, SafeArrayCopy, SafeArrayPutElement, LoadTypeLib, LoadRegTypeLib, DispCallFunc, SystemTimeToVariantTime |
| **OLEACC.dll** | 2 | AccessibleChildren, AccessibleObjectFromWindow |

### 4.3 Media Foundation (2 DLLs, ~20 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **MF.dll** | 2 | MFGetService, MFTranscodeGetAudioOutputAvailableTypes |
| **MFPlat.DLL** | 18 | MFStartup, MFShutdown, MFCreateMediaType, MFCreateSample, MFCreateAttributes, MFCreateMemoryBuffer, MFCreateAlignedMemoryBuffer, MFCreateSourceResolver, MFCreateCollection, MFPutWorkItemEx, MFLockPlatform/MFUnlockPlatform, MFFrameRateToAverageTimePerFrame |

### 4.4 DirectX/Graphics (8 DLLs, ~33 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **d3d11.dll** | 1 | D3D11CreateDevice |
| **d3d9.dll** | 2 | Direct3DCreate9, Direct3DCreate9Ex |
| **d2d1.dll** | 1 (ordinal) | D2D1CreateFactory |
| **DWrite.dll** | 1 | DWriteCreateFactory |
| **dxva2.dll** | 2 | DXVA2CreateDirect3DDeviceManager9, DXVA2CreateVideoService |
| **D3DCOMPILER_46.dll** | 2 | D3DGetInputSignatureBlob, D3DReflect |
| **gdiplus.dll** | 24 | GdiplusStartup/Shutdown, GdipCreateFromHDC, GdipDrawImageRectRect, GdipCreateFont/FontFamily, GdipCreateSolidFill, GdipCreateBitmapFromStream, GdipSetClipHrgn, GdipFillRectangle, etc. |
| **dwmapi.dll** | 1 | DwmExtendFrameIntoClientArea |
| **UxTheme.dll** | 3 | OpenThemeData, GetThemeMetric, CloseThemeData |

### 4.5 Windows Live / Microsoft (7 DLLs, ~223 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **UXCore.dll** | ~180 | CDUIDialog, Element, HWNDElement, VirtualLayout, NativeHWNDHost, SuperPopup, PopupWindow, FillLayout, DuiCreateObject, UXCoreInitProcess/Thread, CRMImage, CRMDUIParser, CRMResource, CRMStringResource, IDuiDataSourceImpl, etc. |
| **WLXPhotoBase.dll** | 14 | Exception (Base), BasePrivate::New/Delete, OS::IsWin7OrGreater/IsWin8OrGreater, Base::GetBaseStringManager, Base::Throw/ThrowLastError, Base::GdiplusStatusToHresult |
| **WLXPhotoSqm.dll** | 13 | Sqm::Startup, Shutdown, AddToStream, Set, IsEnabled, ReportAppLaunchStatus, ReportAppCloseStatus, EnableShipAsserts, SetOptInPreference, GetOptInState |
| **MetadataSys.dll** | 1 | WLXPSGetItemPropertyHandler |
| **DmxBici.dll** | 5 | BiciWrapper::StartExperience, EndExperience, AddToDataPoint, AddStringToDataPoint, TransferExperienceToWeb |
| **wlidcli.dll** | 7 (ordinals) | Windows Live ID client (ordinal imports) |
| **uxctl.dll** | 3 | UxControlsInitProcess, UxControlsCreateObject, UxControlsUninitProcess |

### 4.6 Storage/Database (1 DLL, ~28 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **ESENT.dll** | 28 | JetCreateInstanceW, JetInit2, JetBeginSessionW, JetCreateDatabaseW, JetOpenDatabaseW, JetCreateTableW, JetOpenTableW, JetAddColumnW, JetCreateIndex2W, JetBeginTransaction, JetCommitTransaction, JetRollback, JetRetrieveColumn, JetSetColumn, JetSeek, JetMakeKey, JetMove, JetUpdate, JetDelete, JetTerm |

### 4.7 Other Libraries (7 DLLs, ~45 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **SHLWAPI.dll** | 24 | PathFileExistsW, PathCanonicalizeW, PathFindFileNameW, PathAppendW, StrCmpIW, SHCreateStreamOnFileW, SHRegGetUSValueW, PathRemoveBackslashW, etc. |
| **PROPSYS.dll** | 8 | PSGetPropertyKeyFromName, PropVariantChangeType, PSCreateMemoryPropertyStore, VariantToPropVariant |
| **XmlLite.dll** | 4 | CreateXmlReader, CreateXmlWriter, CreateXmlReaderInputWithEncodingName, CreateXmlWriterOutputWithEncodingName |
| **VERSION.dll** | 3 | GetFileVersionInfoSizeW, GetFileVersionInfoW, VerQueryValueW |
| **WINMM.dll** | 1 | PlaySoundW |
| **WindowsCodecs.dll** | 1 | WICConvertBitmapSource |

## 5. RTTI Class Inventory — 1,023 Unique Types

### 5.1 Namespace Distribution

| Namespace | Type Count | Purpose |
|-----------|-----------|---------|
| **(global)** | 327 | Application classes, interfaces, X3D node types |
| **HMREngine** | 232 | 3D rendering engine (X3D, effects, shaders, textures) |
| **StoryboardManagerNamespace** | 141 | Timeline/project/storyboard management |
| **D3DX11Effects** | 94 | D3DX11 effect framework internal types |
| **D3DX11Texture** | 86 | D3D11 texture codec classes (BC, R, G, B formats) |
| **HMRAVSource** | 36 | Audio/video source processing |
| **D3DXOldTexture** | 34 | Legacy D3D9 texture codecs |
| **(complex/lambda)** | 12 | Lambda closures (e.g., `?1??GetFirstInvalidAssetOnTrack@...`) |
| **Base** | 11 | Base utility library (exceptions, strings, threads) |
| **ATL** | 9 | Active Template Library framework types |
| **Gdiplus** | 5 | GDI+ wrapper classes |
| **CodecUtil** | 4 | Codec encode/decode utilities |
| **Other** | 46 | Misc namespaces (PSA, DragDrop, GdipUtil, HMRTranscode, etc.) |

### 5.2 Application Framework Classes (global namespace)

```
SundanceAppMain              — Main application object (codename "Sundance")
CommandLineParser            — Command-line argument parsing
AutoSaveManager              — Auto-save (.wlmp) management
MediaBrowser                 — Media file browser
TemplateTable                — Effect/transition template registry
ClipboardManager             — Clipboard operations
ContactStore                 — Contact store (PhotoContact integration)
ParsePhotoGalleryTransferFile — Photo Gallery transfer file parser
```

### 5.3 Timeline Classes

```
TimelineBaseBehavior          — Base timeline UI behavior
TimelineBehavior              — Main timeline behavior
TimelineDataSource            — Timeline data binding
TimelineDragDrop              — Drag-drop in timeline
TimelineItemBehavior          — Timeline item rendering
TimelineItemInputBehavior     — Timeline item interaction
TimelineInstructionsBehavior  — Help text overlay
TimelineSelectionRootBehavior — Selection management
TimelineVisualTrackItemBehavior — Visual track rendering
TimelineSecondaryTrackItemBehavior — Audio/secondary tracks
TimelineTemplateSource        — Template rendering
TimelineLayoutMode            — Layout mode management
TimelineExtentUIObject        — Extent rendering
TimelineItemHandler           — Timeline item handler
```

### 5.4 3D Engine Classes (HMREngine, 232 types)

Core engine hierarchy:
```
Engine → EngineDX               — Base/DX rendering engines
Scene                            — 3D scene graph
SceneEncode / ScenePreview       — Encode/preview scenes
RenderLoop → RenderLoopDX        — Render loops
PreviewDX                        — Preview renderer
EncodeDX                         — Encoder
DefaultPreviewDX                 — Default preview
Mixer → MixerBuffer              — Audio/video mixer
AudioQueue / MixStream           — Audio processing
AudioBoost / AudioOutput         — Audio enhancement/output
Waveform                         — Audio waveform
```

Effect/Shader system:
```
EffectResource → EffectResourceDX
  ├── BlurEffectResource → BlurShaderImpl
  ├── BrightnessEffectResource → BrightnessShaderImpl
  ├── ChannelMixerEffectResource → ChannelMixerShaderImpl
  ├── DissolveEffectResource → DissolveShaderImpl
  ├── EdgeDetectionEffectResource
  ├── FadeEffectResource → FadeShaderImpl
  ├── GridEffectResource → GridShaderImpl
  ├── HueEffectResource
  ├── PixelateEffectResource → PixelateShaderImpl
  ├── PosterizeEffectResource → PosterizeShaderImpl
  ├── RippleEffectResource
  ├── WipeEffectResource → WipeShaderImpl
  └── ScrollingTextEffectResourceDX → ScrollingTextShaderImpl
```

Pattern meshes (transition shapes):
```
BowTiePatternMesh, CheckerboardPatternMesh, CirclePatternMesh,
CirclesPatternMesh, DiagonalBoxPatternMesh, DiagonalCrossPatternMesh,
EyePatternMesh, FanAndSweepPatternMesh, FanInPatternMesh,
FanOutPatternMesh, FanUpPatternMesh, FillVPatternMesh,
HeartPatternMesh, IrisPatternMesh, KeyholePatternMesh,
PageCurlGrid, RectanglePatternMesh, RectanglesPatternMesh,
RevealPatternMesh, SplitPatternMesh, StarPatternMesh,
StarsPatternMesh, SweepInPatternMesh, SweepOutPatternMesh,
SweepUpPatternMesh, WheelPatternMesh, ZigzagPatternMesh
```

Texture management:
```
TextureProvider → TextureResource → TextureResourceDX
ImageTexture → ImageTextureImpl
MotionTexture → MotionTextureImpl → MotionTextureResourceDX
MovieTexture → MovieTextureImpl
TextureTransform / TextureTransformContainer
```

X3D node implementations (VRML/X3D scene graph):
```
X3DNodeImpl, X3DGroupingNodeImpl, X3DShapeNodeImpl,
X3DGeometryNodeImpl, X3DComposedGeometryNodeImpl,
X3DMaterialNodeImpl, X3DAppearanceNodeImpl,
X3DTexture2DNodeImpl, X3DTextureNodeImpl,
X3DCoordinateNodeImpl, X3DNormalNodeImpl, X3DColorNodeImpl,
X3DViewpointNodeImpl, X3DViewportNodeImpl,
X3DTimeDependentObjectImpl, X3DSensorNodeImpl,
X3DShaderNodeImpl, X3DSoundNodeImpl, X3DSoundSourceNodeImpl,
X3DAnimatedShaderNodeImpl, X3DTextNodeImpl, X3DUrlObjectImpl,
X3DVertexAttributeNodeImpl, X3DLayerNodeImpl, X3DBoundedObjectImpl
```

### 5.5 Storyboard/Project Classes (StoryboardManagerNamespace, 141 types)

Core project:
```
StoryboardManager              — Top-level storyboard manager
MovieProject                   — Project data model
MovieExtent                    — Project extent (clip range)
Conductor                      — Orchestration
MovieTransport / TransportBase — Transport/playback control
RenderTransport                — Render transport
MediaItem → MediaItemBase      — Media items
AudioClip / VideoClip / TitleClip / ImageClip — Clip types
AudioVideoMediaClip            — AV media clips
MediaClipBase                  — Base media clip
```

Theme system:
```
Theme, ThemeManager, ThemeProject
ThemeIntro → ThemeOutro → ThemeMid → ThemeInterior
ThemeTitle → ThemeSimpleTitle → ThemeComplexTitle
ThemeTrack → ThemePrimaryTrack → ThemeDependentTrack
ThemeEffect → ThemeEffectTemplate → ThemeFirstEffect → ThemeLastEffect
ThemeTransition → ThemeFirstTransition → ThemeLastTransition
ThemeSimpleElement, ThemeX3DTemplate, ThemeComplexType
MonolithicThemeOperation
```

Template system:
```
BaseTemplate, BaseX3DTemplate, CompositeX3DTemplate
EffectX3DTemplate, ExtentX3DTemplate, ThemeX3DTemplate
TemplatePlaceholder, TemplateProperty, TemplateSocket
TemplateSocketsParser, TemplateInitializationException
```

Serialization:
```
SerializationReader / SerializationWriter / SerializationContext
SerializationContainer / SerializationContainerValidator
SerializationMemoryReaderWriter / SerializationModifier
BoundProperty (+ Bool/Float/Int/String variants)
BoundPropertyDictionary / BoundPropertyFloatSet / BoundPropertyStringSet
BoundPlaceholder / BoundPlaceholderMapContainer / BoundPlaceholderSerializer
BoundPropertiesDictionaryContainer
SerializableObject, ValueVariantRef
CommandBin, RenderCommandBin
ModBeginDocument / ModBeginElement / ModEndElement
ModContainer / ModContainerWithAttribute / ModContainerWithIDLookAhead
```

Selection:
```
ExtentSelector / ExtentSelectorValidator
SelectorFinder / SelectorExtentRefContainer / SelectorExtentRefSerializer
```

### 5.6 Audio/Video Source Classes (HMRAVSource, 36 types)

```
AVSource → AVSourceProxy
AVSourceFactory → AVSourceFactoryInternal
AVSink → AudioStreamSink / StreamSinkHost / StreamSinkHelper
IAVProcessor → VideoBuffer
IAVSampleSource → SyncVideoSampleSource
IVideoProcessor → DXVA2VideoProc / XVideoProc
TextureInterOp → TextureInterOpDX9 / TextureInterOpDX11
MFSourceReaderBuilder → NativeMFSourceReaderBuilder → DShowMFSourceReaderBuilder
MFByteStreamOnStream (+ MFByteStreamOnStreamAsyncResult, OnReadAsyncCallback, OnWriteAsyncCallback)
MFAsyncResult / MFRateControlHelper
EncodeProfile → SAXProfileBuilder
AudioResamplerHelper
AuthProvider / AuthCredentials
AsyncSourceResolver
```

### 5.7 UI Behavior Classes (global)

```
SundanceMainElementBehavior     — Main ribbon/toolbar
AboveHomerBehavior              — Above preview area
HomerHeavyLayerBehavior         — Heavy preview layer
StandardLayerBehavior           — Standard layer
DuiLayerBehaviorImpl            — DirectUI layer
InlinePreviewLayoutBehavior     — Inline preview
InlinePreviewSliderBehavior     — Preview slider
FullscreenLayoutBehavior        — Fullscreen layout
FullscreenBackgroundWindow      — Fullscreen background
ProjectWorkspaceLayoutBehavior  — Workspace layout
ResizeablePaneBehavior          — Resizable panes
AMPCommandBarBehavior           — AMP command bar
AMPMainWindowBehavior           — AMP main window
AMPDataContext                  — AMP data context
AmpFadeAnimationBehavior        — Fade animation
AmpPreventSparseModeBehavior    — Sparse mode prevention
AmpSlideAnimationBehavior       — Slide animation
MultipleEffectBehavior          — Multiple effects
MultipleEffectDialog            — Effects dialog
OptionsDialogBehavior           — Options dialog
HelpBehavior                    — Help display
CaptureUIBehavior               — Capture UI
PopUpSlider                     — Popup slider control
ProgressBase / ProgressDialog / ProgressStatusBar — Progress UI
TextBoxBehavior                 — Text editing
RichEditControlBehavior         — Rich text editing
CaretBehavior / CaretScrollBehavior — Caret management
CaptureUIBehavior               — Webcam capture UI
WebcamElementBehavior / WebcamUI — Webcam UI
TrimBehavior                    — Video trimming
NarrationUI                     — Narration recording
UserEncodeProfileBehavior       — Encode profile selection
UserEncodeProfileDialog         — Encode settings dialog
```

### 5.8 Dialog/Host Classes

```
CDUIDialog                      — DirectUI dialog (from UXCore)
CFramelessHost                  — Frameless window host
CSundanceDialog                 — Sundance dialog base
CMsgFilter                       — Message filter
SundanceNativeHwndHost          — Native HWND host
SundanceClipboardChainWindow    — Clipboard chain
DontShowPromptDialog            — "Don't show again" dialog
SundanceDontShowPromptDialog    — Sundance version
SundanceApplicationOptionsDialog — Options dialog
RenderSummaryDialog             — Render summary
PublishSummaryDialog            — Publish summary
MultipleEffectDialog            — Effects dialog
```

### 5.9 Publishing Classes

```
PublishManager                   — Publish orchestration
PublishJob                       — Individual publish job
PublishBackgroundJob             — Background publishing
PublishBackgroundWorker          — Worker thread
PublishProgressCallBack          — Progress reporting
PublishItemProperties            — Publish item properties
PublishItemPropertyStore         — Property storage
PublishSessionXmlManifestFactory — XML manifest
XmlManifestFactory               — XML manifest factory
SharedBitmap                     — Shared bitmap (PublishHelpers)
```

### 5.10 Ribbon Classes

```
RibbonApp                        — Ribbon application
RibbonCategoryItem               — Ribbon category item
RibbonCategoryList               — Ribbon category list
RibbonList                       — Ribbon list
RibbonListItem                   — Ribbon list item
```

### 5.11 D3DX11 Texture Codec Classes (120 types)

Comprehensive DXGI format support via CCodec_* pattern:
```
CCodec_B8G8R8A8_UNORM, CCodec_R8G8B8A8_UNORM, CCodec_R16G16B16A16_FLOAT,
CCodec_BC1_UNORM through CCodec_BC7_UNORM_SRGB (compressed),
CCodec_R32G32B32A32_FLOAT (high-precision),
CCodec_D32_FLOAT, CCodec_D24_UNORM_S8_UINT (depth),
CCodecDXT (DXT compressed), CCodecYUV (YUV formats)
CDdsLoader, CImageSlice, CWinCodecLoader, CTexture
CAsyncTextureProcessor, CAsyncMemoryLoader
```

### 5.12 D3DX11 Effects Framework (94 types)

Complete D3DX11 effect system embedded:
```
CEffect, SGroup, SPassBlock, STechnique, SType
SConstantBuffer, SBaseBlock
Variable types: SFloat/SBool/SInt/SShort scalar/vector variants
Global variables: SBlend/Sampler/Shader/Rasterizer/RenderTarget/DepthStencil
Annotation types: SNumeric/SString/Matrix
Invalid/null objects: SEffectInvalid* (19 types)
```

### 5.13 X3D Node Hierarchy (46 types)

Complete VRML/X3D 9777 node implementation:
```
X3DNode → X3DGroupingNode, X3DShapeNode, X3DGeometryNode
X3DAppearanceNode, X3DMaterialNode, X3DTexture2DNode
X3DCoordinateNode, X3DNormalNode, X3DColorNode
X3DViewpointNode, X3DViewportNode, X3DLayerNode
X3DTimeDependentNode, X3DSensorNode, X3DTriggerNode
X3DShaderNode, X3DSoundNode, X3DSoundSourceNode
X3DAnimatedShaderNode, X3DTextNode, X3DUrlObject
X3DVertexAttributeNode, X3DBindableNode, X3DBoundedObject
X3DInterpolatorNodeBase, X3DComposedGeometryNode
X3DFontStyleNode, X3DGeometricPropertyNode, X3DMetadataObject
```

## 6. COM GUIDs Found in Binary

### Confirmed GUIDs

| GUID | Purpose |
|------|---------|
| `{9590CDCE-BFFC-45d5-85EA-493C65FB2C4F}` | Transition/Effect type GUID |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Telemetry/error reporting endpoint |
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | COM APPID for MovieMaker |

### PE Authenticode Signatures

| GUID | Certificate |
|------|-------------|
| `{4faf0b71-ad37-4aa3-a671-76bc052344ad}` | Microsoft Code Signing PCA |
| `{2860b52e-c4a3-454d-bc1e-32c5add17e90}` | Microsoft Code Signing PCA 2011 |

## 7. MovieMakerMain Execution Flow

From disassembly analysis:

```
MovieMakerMain(argc, argv):
  1. Sets up 4712-byte stack frame with SEH handler
  2. Calls internal init function at 100E9570 (likely CrtInit)
  3. Checks global at [1057CE84] — ATL module state
  4. If module is initialized (flag 0x10 set):
     - Calls 100E9120 (likely DoCommandLineParse)
  5. Sets error code 0xC945001A at [ebp-11D0h]
  6. Calls 100E9DB0 (likely InitializeSubsystems)
  7. Calls [100011CC] (GetProcAddress thunk → GetCurrentProcess?)
  8. Continues initialization or returns error
```

Return value `0xC945001A` when called without proper parent process context — suggests the function validates it's running as a child of a known process.

## 8. String Constants (Selected)

### Application Identity
- `"Windows Live Movie Maker"` / `"Movie Maker"` / `"Sundance"` (codename)
- `"WindowsLiveMovieMakerMain"` / `"WindowLiveMovieMakerMainWindowClass"` — Window classes
- `"16.4.3528.0331"` — Version string
- `"WLPGMovieMaker_SelectedSet_16.4.3528.0331"` — Clipboard format

### Serialization Paths
- `"//MovieMaker/Project/DataStr"` — Project data root
- `"//TiEffectArr[@UID="%s"]/UID"` — Effect lookup
- `"//TiTransition[@UID="%s"]/TTFrom"` — Transition lookup
- `"//TIArr/UID[@UID="%i"]"` — Track item lookup
- `"AutoSave.wlmp"` / `"AutoSaveValidate.wlmp"` — Auto-save files

### Registry
- `"Software\Microsoft\Windows Live\Movie Maker"` — Main settings
- `"Software\Microsoft\Windows Live\Photo Gallery"` — Shared WL settings
- `"Software\Microsoft\Windows Media Foundation\ByteStreamHandlers"` — MF handlers
- `"%LOCALAPPDATA%\Microsoft\Windows Live Movie Maker"` — App data

### Telemetry
- `"http://g.live.com"` — Production endpoint
- `"http://g.live-int.com"` — Internal/test endpoint

### Supported Input Formats
`.mp4, .wmv, .m4a, .wma, .wlmp, .wlvs, .mpg, .asf, .avi, .mpv, .m1v, .m2v, .mpeg, .qt, .mov, .wm, .mpe, .3g2, .3gpp, .3gp, .mqv, .rle, .gif, .ico, .3gp2, .mpv2, .mp2v, .mp2, .dib, .mod, .vob`

### DXVA Error/Mode Strings
- `"DXVA_ModeMPEG2_A/B/C/D"` — Hardware decode modes
- `"<Failed: 0x%08X>"` — DXVA failure reporting

## 9. Test Harness Results

```
=== MovieMakerCore.dll Test Harness ===

[*] Loading DLL
[+] DLL loaded at base: 0x6ACD0000
[+] MovieMakerMain at: 0x6ADB96C0 (RVA 0x000E96C0)
[*] Calling MovieMakerMain(1, testArgv)...

[*] MovieMakerMain returned: -918224870 (0xC945001A)
[*] Last error: 0
[+] DLL unloaded
```

The function executed without crashing (SEH handled gracefully) and returned `0xC945001A`. This value was pre-set at `[ebp-11D0h]` in the prologue before any real initialization. This is the "uninitialized/failure" return code, indicating the function detected it wasn't properly hosted (needs parent process context, COM apartment, or specific initialization state).

## 10. Architecture Summary

### Dependency Graph (33 imported DLLs)

```
MovieMakerCore.dll
├── Core Windows (5 DLLs)
│   ├── KERNEL32.dll (121 fns) — Process, thread, memory, file I/O
│   ├── USER32.dll (105 fns) — Window management, message loop
│   ├── GDI32.dll (18 fns) — Device contexts, regions, bitmaps
│   ├── ADVAPI32.dll (25 fns) — Registry, crypto, WPP tracing
│   └── SHELL32.dll (19 fns) — Shell items, known folders
├── COM/OLE (3 DLLs)
│   ├── ole32.dll (23 fns) — COM initialization, clipboard, storage
│   ├── OLEAUT32.dll (22 fns) — BSTR, VARIANT, SafeArray, type libs
│   └── OLEACC.dll (2 fns) — Accessibility
├── Media Foundation (2 DLLs)
│   ├── MF.dll (2 fns) — MF services, transcode output types
│   └── MFPlat.DLL (18 fns) — MF platform, media types, samples
├── DirectX/Graphics (8 DLLs)
│   ├── d3d11.dll + D3DCOMPILER_46.dll — D3D11 rendering + HLSL
│   ├── d3d9.dll + dxva2.dll — D3D9 fallback + hardware video decode
│   ├── d2d1.dll + DWrite.dll — Direct2D + DirectWrite
│   ├── gdiplus.dll (24 fns) — GDI+ 2D rendering
│   ├── dwmapi.dll — Desktop Window Manager
│   └── UxTheme.dll — Visual styles
├── Windows Live (7 DLLs)
│   ├── UXCore.dll (~180 fns) — DirectUI framework
│   ├── WLXPhotoBase.dll (14 fns) — Base utilities
│   ├── WLXPhotoSqm.dll (13 fns) — SQM telemetry
│   ├── DmxBici.dll (5 fns) — BICI telemetry
│   ├── MetadataSys.dll (1 fn) — Photo metadata
│   ├── wlidcli.dll (7 fns) — Windows Live ID
│   └── uxctl.dll (3 fns) — UX controls
├── Storage (1 DLL)
│   └── ESENT.dll (28 fns) — Extensible Storage Engine (Jet DB)
├── Path/String (1 DLL)
│   └── SHLWAPI.dll (24 fns) — Path/string utilities
├── Properties (1 DLL)
│   └── PROPSYS.dll (8 fns) — Property system
├── XML (1 DLL)
│   └── XmlLite.dll (4 fns) — XML reader/writer
├── Versioning (1 DLL)
│   └── VERSION.dll (3 fns) — File version info
├── Audio (1 DLL)
│   └── WINMM.dll (1 fn) — PlaySound
└── Imaging (1 DLL)
    └── WindowsCodecs.dll (1 fn) — WIC bitmap conversion
```

### Initialization Order

```
DllMain (DLL_PROCESS_ATTACH):
  1. Store hInstance
  2. DisableThreadLibraryCalls

MovieMakerMain:
  3. CrtInit (SEH frame, stack guard)
  4. ATL module state check
  5. Command-line parsing
  6. CoInitializeEx (STA)
  7. ATL _Module.Init / RegisterClassObjects
  8. GdiplusStartup
  9. MFStartup
  10. UXCoreInitProcess / UXCoreInitThread
  11. DirectUI initialization
  12. D3D11CreateDevice
  13. D2D1CreateFactory / DWriteCreateFactory / WIC factory
  14. Main message loop

ShutdownSubsystems (DLL_PROCESS_DETACH):
  Release WIC → DWrite → D2D → D3D11 → DirectUI → UXCore
  MFShutdown → GdiplusShutdown → _Module.Term → CoUninitialize
```

### Key Architectural Patterns

1. **Single-export monolith** — 5.46 MB of code, one export. The DLL is functionally an EXE packaged as a DLL.

2. **Three-layer rendering** — GDI+ (2D UI) + D3D9 (legacy) + D3D11 (primary preview/encode)

3. **Embedded D3DX11 framework** — Complete D3DX11 effect system (94 types) compiled into the DLL, not loaded from external .fx files.

4. **VRML/X3D engine** — Full X3D 9777 node hierarchy (46 node types) for 3D transitions and effects.

5. **Pattern mesh system** — 28+ transition patterns (heart, star, iris, page curl, etc.) for video transitions.

6. **ESENT database** — 28 Jet API imports for internal structured storage (likely thumbnail cache, project index, or MRU).

7. **Dual telemetry** — SQM (Microsoft Quality Metrics) + BICI (Behavioral Instrumentation) + custom HTTP error reporting.

8. **Storyboard namespace** — 141 types managing project serialization, theme application, template instantiation, and media item lifecycle.

9. **Extensive COM integration** — ATL module, OLE clipboard, COM class registration via resource section (REGISTRY resources in .rsrc).

10. **Multi-threaded architecture** — IoCompletionPort, condition variables, thread pool, interlocked operations indicate heavy async processing.
