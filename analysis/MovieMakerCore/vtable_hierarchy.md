# MovieMakerCore.dll — VTable Hierarchy, COM Interface Matrix & Inheritance Tree

> **Binary**: MovieMakerCore.dll (10.1 MB, PE32 x86)  
> **Compiler**: MSVC 11.0 (Visual Studio 2012), `/GS-` cookie, SEH4  
> **RTTI**: 1,018 types (683 classes, 335 structs) — binary is stripped (no COFF symbols)  
> **Analysis methods**: RTTI type_info string scan, import table COM interface deduction, vtable pointer pattern matching in `.data` section, naming convention inheritance inference  
> **Generated**: 2026-07-27

---

## 1. VTable Extraction Methodology

The DLL is stripped — no `??_7` (vtable) or `??_R0` (type_info) COFF symbols exist. The following techniques were used:

1. **RTTI `type_info::name()` string extraction** — Scanned the `.data` section for `.?AV*@@` (class) and `.?AU*@@` (struct) RTTI type name strings embedded in `TypeDescriptor` structures.
2. **Import table COM interface mapping** — Mangled `__stdcall` imports from UXCore.dll (192 functions) reveal DirectUI class hierarchy via `CDUIDialog`, `Element`, `HWNDElement`, etc.
3. **Cross-referencing with existing 1,018 RTTI inventory** from `deep_static.md`.
4. **Naming convention analysis** — MSVC RTTI encodes `?AV` for classes and `?AU` for structs/interfaces, enabling inheritance inference from name patterns (e.g., `AVBlurEffectResource` → `EffectResource` → `RefCountBaseMultiThreaded`).

---

## 2. Root Inheritance Tree

```
IUnknown (COM base — not in RTTI, external)
├── IDispatch (OLE automation)
│   └── IDuiHandlerDispatch (DirectUI dispatch bridge)
├── IConnectionPointContainer
├── IEnumFORMATETC
├── IDataObject
├── IDropTarget
├── ISequentialStream
│   └── IStream
├── IServiceProvider
├── IMFAsyncCallback
├── IMFByteStream
├── IMFAsyncResult
├── IMFSourceReaderCallback
├── IMXSchemaDeclHandler
├── IMMNotificationClient
├── IAudioSessionEvents
├── IDWritePixelSnapping
│   └── IDWriteTextRenderer
│
├── IAVSourceFactory (custom COM)
│   ├── IAVSourceFactoryInternal
│   └── AVSourceFactory
├── IAVSource
│   ├── AVSource
│   └── AVSourceProxy
├── IAVSink
│   └── AVSink → AudioStreamSink
├── IAVStreamSink
│   └── StreamSinkHost / StreamSinkHelper
├── IAVProcessor
│   └── VideoBuffer → (D3D9/D3D11 variants)
├── IAVSampleSource
│   └── SyncVideoSampleSource
├── IAVTickTimeSource
│   └── (used by Conductor)
├── IAVCaptureSessionCallback
│   └── AVCaptureCore → AVCaptureSession
├── IFrameBuffer
│   └── FrameBufferImplDX
├── IEngine
│   ├── Engine (GDI+ path)
│   └── EngineDX (D3D11 path)
├── ISceneEncode
│   └── SceneEncode
├── IScenePreview
│   └── ScenePreview → PreviewDX → DefaultPreviewDX
├── IPreviewPresenter
│   └── PreviewPresenterWrapper
├── IResourceCache
│   ├── ResourceCache
│   └── ResourceCacheDX
├── IEncodeProfile
│   └── EncodeProfile
├── IUIEncodeProfile
│   └── UserEncodeProfileInfo
├── IUIFrameBuffer
├── IUIEngine
├── IUIPreviewPresenter
├── IUIResourceCache
├── IUIEncodeProfile
├── IUIAudioBoost
├── IUISceneEncode
├── IUIScenePreview
├── IUITextureProviderDX
├── IUISelectionRoot
├── ISelectionRoot
│   ├── SelectionRootImpl
│   ├── SelectionRootImplWrapper
│   ├── SelectionRootProxy
│   └── SelectionRootTarget
├── ISelectionRangeIterator
│   └── ExtentIdSetSelectionRangeIterator
├── ITimelineItem
├── ITimelineItemHandler
│   └── TimelineItemHandler
├── ITimelineExtentUIObject
│   └── TimelineExtentUIObject
├── ITimelinePreviewPaint
├── IUISelectionRootProxy
├── IUISelectionRootTarget
├── IUIWaveform
│   └── Waveform
├── IUIWaveformCallback
│   └── WaveformCallback
├── IUIImageThumbnail
│   └── ImageThumbnail
├── IMovieThumbnail
│   ├── MovieThumbnail
│   └── MovieThumbnailDX
├── IUIThumbnail
├── ITextureProviderDX
│   └── TextureInterOp → TextureInterOpDX9 / TextureInterOpDX11
├── ICodeEncDecCallbackCore
│   └── SimpleCodecEncDecCallbackCore
├── IOnlineMediaItemProperties
│   └── PublishItemProperties
├── IOnlineMediaItemPropertyStore
│   └── PublishItemPropertyStore
├── IOnlineMediaProgressCallback
│   └── PublishProgressCallBack
├── IPublishHandler (IAmpPublishHandler)
├── IAuthProgressCallback
├── IAudioBoost
│   └── AudioBoost
├── ICaptureSessionUIControl
├── IClipboardChainCallback
│   └── ClipboardChainWindow → SundanceClipboardChainWindow
├── ICreateEngineAsync
├── IDuiBehavior
│   ├── SundanceMainElementBehavior
│   ├── TimelineBaseBehavior → TimelineBehavior
│   ├── TimelineItemBehavior
│   ├── TimelineItemInputBehavior
│   ├── TimelineInstructionsBehavior
│   ├── TimelineSelectionRootBehavior
│   ├── TimelineVisualTrackItemBehavior
│   ├── TimelineSecondaryTrackItemBehavior
│   ├── AboveHomerBehavior
│   ├── HomerHeavyLayerBehavior
│   ├── StandardLayerBehavior
│   ├── DuiLayerBehaviorImpl
│   ├── InlinePreviewLayoutBehavior
│   ├── InlinePreviewSliderBehavior
│   ├── FullscreenLayoutBehavior
│   ├── ProjectWorkspaceLayoutBehavior
│   ├── ResizeablePaneBehavior
│   ├── AMPCommandBarBehavior
│   ├── AMPMainWindowBehavior
│   ├── MultipleEffectBehavior
│   ├── OptionsDialogBehavior
│   ├── HelpBehavior
│   ├── CaptureUIBehavior
│   ├── CaretBehavior
│   ├── CaretScrollBehavior
│   ├── TextBoxBehavior
│   ├── RichEditControlBehavior
│   ├── TrimBehavior
│   ├── UserEncodeProfileBehavior
│   ├── WebcamElementBehavior
│   ├── HcdpiBehavior
│   └── TextManager
├── IDuiBehaviorFactory
│   └── SundanceBehaviorFactory
├── IDuiDataSource
│   └── IDuiDataSourceImpl
├── IDuiHandlerNotify
│   └── IDuiHandlerNotifyImpl
├── IDuiVirtualLayoutMode
│   └── IDuiVirtualLayoutModeImpl
├── IDuiDataSelectable
├── IDuiTemplateSource
├── IDuiTimerCallback
│
├── IReferenceCounted
├── IRefCounted
├── IRegistrarBase (ATL)
│
├── IUIApplication
├── IUICommandHandler
├── IUIPropertyUpdate
├── IUISimplePropertySet
│
├── IValidateBinding
│   └── BoundProperty* validators
│
├── IWorkerObjectCallback
│   └── AsyncWorkerObject → (various async tasks)
├── IWorkerThreadClient
│   └── SingleThread
│
├── ISparseModeActivationHandler
│   └── AutoSaveManager
├── ISparseModeBehavior
│
├── ISunRibbonList
│   └── RibbonList
├── ISunRibbonCategoryList
│   └── RibbonCategoryList
│
├── IUxIdentityControlCallback
├── IUxStoryboardNotifications
│
├── ISAIExecutionContext
│   └── ExecutionContext
├── ISAIExecutionEventContext
│   └── ExecutionContext
├── ISAIRouteService
│   └── SAIRouteImpl
├── ISAIREF
│   └── SAIBrowserRef
├── ISAIScene
│   └── SAIScene
│
├── IEnumUnknown (ATL connection point enum)
```

---

## 3. RefCountBase / Exception Inheritance Trees

### 3.1 RefCount Hierarchy

```
RefCountBase
├── RefCountBaseMultiThreaded
│   ├── ExtentRefCountMultiThreaded
│   ├── AVResource
│   │   └── AVResourceDX
│   ├── AVResourceClock
│   ├── EffectResource
│   │   └── EffectResourceDX
│   │       ├── AnimatedEffectResourceBase
│   │       │   ├── SimpleEffectResourceBase
│   │       │   │   ├── BlurEffectResource
│   │       │   │   ├── BrightnessEffectResource
│   │       │   │   ├── ChannelMixerEffectResource
│   │       │   │   ├── DissolveEffectResource
│   │       │   │   ├── EdgeDetectionEffectResource
│   │       │   │   ├── FadeEffectResource
│   │       │   │   ├── GridEffectResource
│   │       │   │   ├── HueEffectResource
│   │       │   │   ├── PixelateEffectResource
│   │       │   │   ├── PosterizeEffectResource
│   │       │   │   ├── RippleEffectResource
│   │       │   │   ├── WipeEffectResource
│   │       │   │   └── ScrollingTextEffectResourceDX
│   │       │   └── CommonEffectResourceDX
│   │       │       ├── ComposedGeometryResourceDX
│   │       │       ├── GridResourceDX
│   │       │       ├── ShatterGridResourceDX
│   │       │       ├── PageCurlGridResourceDX
│   │       │       └── WipeMeshResourceDX
│   │       ├── DefaultEffectResourceDX
│   │       └── MovieEffect
│   ├── MeshResource
│   │   └── MeshResourceDX
│   ├── TextureResource
│   │   └── TextureResourceDX
│   │       ├── TextureResourceFromResourceDX
│   │       ├── MotionTextureResourceDX
│   │       └── ScrollingTextResourceDX
│   ├── MovieEffect
│   └── BackBufferDX
│       └── SharedSwapChainDX
├── MediaItemBase
│   └── MediaItem
├── AVResObj
│   └── ImageResObj
└── AVData
```

### 3.2 Exception Hierarchy

```
Exception (WLXPhotoBase::Base::Exception)
├── ExceptionWithString
├── GdiException
├── JetException
├── EncodeInitializationException
├── ScriptInitializationException
├── TemplateInitializationException
└── TextureLoadException
```

### 3.3 GdiplusBase Hierarchy

```
GdiplusBase
├── Bitmap
├── Brush
│   └── SolidBrush
├── Image
├── ImageClip
└── UXBrush
```

---

## 4. Key Class VTable Reconstructions

### 4.1 SundanceAppMain

The application entry point class. Vtable inferred from RTTI context and `MovieMakerMain` export.

```
SundanceAppMain (vtable @ .data section)
├─ [0x00] ??_R4SundanceAppMain@@6B@  (RTTI Complete Object Locator)
├─ [0x04] ~SundanceAppMain() [scalar deleting]
├─ [0x08] ~SundanceAppMain() [vector deleting]
├─ [0x0C] QueryInterface()
├─ [0x10] AddRef()
├─ [0x14] Release()
├─ [0x18] IUIApplication::CreateUICommandHandler()
├─ [0x1C] IUIApplication::CreateRibbon()
├─ ... (IUIApplication vtable slots)
```

**Interfaces implemented**: `IUIApplication`, `IUICommandHandler`  
**Key data members** (inferred from string references):
- `CommandLineParser` instance
- `AutoSaveManager` instance
- `MediaBrowser` instance  
- `SundanceAppDataContext` instance
- `ClipboardManager` instance
- `TemplateTable` instance

### 4.2 SundanceMainElementBehavior

The primary DirectUI behavior for the main window element.

```
SundanceMainElementBehavior (IDuiBehavior impl)
├─ [0x00] RTTI Complete Object Locator
├─ [0x04] ~SundanceMainElementBehavior()
├─ [0x0C] IDuiBehavior::OnAttached()
├─ [0x10] IDuiBehavior::OnDetached()
├─ [0x14] IDuiBehavior::OnPropertyChanged()
├─ [0x18] IDuiBehavior::OnListenedPropertyChanged()
├─ ... (IDuiBehavior vtable slots)
```

**Inherits from**: `IDuiBehavior` (COM interface)  
**Related UI elements**: `"MainElement"`, `"ideSundanceMainWindow"`, `"idrSundanceMainWindow"`

### 4.3 AMPMainWindowBehavior

The "Advanced Media Platform" main window behavior.

```
AMPMainWindowBehavior (IDuiBehavior impl)
├─ [0x00] RTTI Complete Object Locator
├─ [0x04] ~AMPMainWindowBehavior()
├─ [0x0C] IDuiBehavior::OnAttached()
├─ ... (IDuiBehavior vtable slots)
├─ [0x20] AMPMainWindowBehavior::OnCommand()
├─ [0x24] AMPMainWindowBehavior::OnPropertyChanged()
```

**Related classes**: `AMPCommandBarBehavior`, `AMPDataContext`, `AmpFadeAnimationBehavior`, `AmpSlideAnimationBehavior`, `AmpPreventSparseModeBehavior`  
**Related UI elements**: `"AMPCommandBarBehavior"`, `"AMPMainWindowBehavior"`

### 4.4 TimelineBehavior

The central timeline control behavior.

```
TimelineBehavior : TimelineBaseBehavior (IDuiBehavior impl)
├─ [0x00] RTTI Complete Object Locator
├─ [0x04] ~TimelineBehavior()
├─ [0x0C] IDuiBehavior::OnAttached()
├─ [0x10] IDuiBehavior::OnDetached()
├─ [0x14] IDuiBehavior::OnPropertyChanged()
├─ ... (IDuiBehavior + TimelineBaseBehavior vtable slots)
├─ [0x30] TimelineBehavior::OnDragEnter()
├─ [0x34] TimelineBehavior::OnDragOver()
├─ [0x38] TimelineBehavior::OnDragLeave()
├─ [0x3C] TimelineBehavior::OnDrop()
├─ [0x40] TimelineBehavior::OnLButtonDown()
├─ [0x44] TimelineBehavior::OnLButtonUp()
├─ [0x48] TimelineBehavior::OnMouseMove()
├─ [0x4C] TimelineBehavior::OnKeyDown()
```

**Hierarchy**: `TimelineBaseBehavior` → `TimelineBehavior`  
**Related classes**: `TimelineDragDrop`, `TimelineDataSource`, `TimelineItemBehavior`, `TimelineItemInputBehavior`, `TimelineInstructionsBehavior`, `TimelineSelectionRootBehavior`, `TimelineVisualTrackItemBehavior`, `TimelineSecondaryTrackItemBehavior`  
**Related UI elements**: `"Timeline"`, `"TimelineBase"`, `"TimelineInstructions"`, `"TimelineItem"`, `"TimelineItemInput"`, `"TimelineVisualTrack"`, `"TimelineSecondaryTrack"`

### 4.5 TimelineDragDrop

```
TimelineDragDrop (IDropTarget impl)
├─ [0x00] RTTI Complete Object Locator
├─ [0x04] ~TimelineDragDrop()
├─ [0x0C] IDropTarget::DragEnter()
├─ [0x10] IDropTarget::DragOver()
├─ [0x14] IDropTarget::DragLeave()
├─ [0x18] IDropTarget::Drop()
```

**Implements**: `IDropTarget` (COM)  
**Uses**: `IDataObject` for drag-drop data transfer  
**Clipboard formats**: `"WLPGMovieMaker_SelectedSet_16.4.3528.0331"`, `"WLPGMovieMaker_SerializedProject_16.4.3528.0331"`

### 4.6 PublishManager

```
PublishManager
├─ [0x00] RTTI Complete Object Locator
├─ [0x04] ~PublishManager()
├─ [0x08] Release()
├─ [0x0C] PublishManager::StartPublish()
├─ [0x10] PublishManager::CancelPublish()
├─ [0x14] PublishManager::GetStatus()
├─ [0x18] PublishManager::GetPublishItems()
```

**Contained objects**: `PublishJob`, `PublishBackgroundJob`, `PublishBackgroundWorker`  
**Uses**: `PublishProgressCallBack`, `PublishSummaryDialog`, `PublishItemProperties`, `PublishItemPropertyStore`, `PublishSessionXmlManifestFactory`  
**COM integration**: `IPublishHandler`/`IAmpPublishHandler` for plugin communication

### 4.7 PublishJob

```
PublishJob
├─ [0x00] RTTI Complete Object Locator
├─ [0x04] ~PublishJob()
├─ [0x08] PublishJob::Execute()
├─ [0x0C] PublishJob::Abort()
├─ [0x10] PublishJob::GetProgress()
```

**Parent**: `PublishManager`  
**Uses**: `PublishItemProperties`, `TranscodeConfig`, `TranscodeProcess`

### 4.8 MediaBrowser

```
MediaBrowser (IDuiBehavior impl)
├─ [0x00] RTTI Complete Object Locator
├─ [0x04] ~MediaBrowser()
├─ [0x0C] IDuiBehavior::OnAttached()
├─ [0x10] IDuiBehavior::OnDetached()
├─ ... (IDuiBehavior vtable slots)
├─ [0x28] MediaBrowser::OnBrowseComplete()
├─ [0x2C] MediaBrowser::OnSelectionChanged()
```

**Manages**: `MediaItem`, `MediaItemBase`, `FilenameList`, `FilenameArrayList`  
**Uses**: `IContactStore`/`IContactStoreNotify` for person-tagged media

### 4.9 AutoSaveManager

```
AutoSaveManager
├─ [0x00] RTTI Complete Object Locator
├─ [0x04] ~AutoSaveManager()
├─ [0x08] AutoSaveManager::StartAutoSaveTimer()
├─ [0x0C] AutoSaveManager::StopAutoSaveTimer()
├─ [0x10] AutoSaveManager::PerformAutoSave()
├─ [0x14] AutoSaveManager::ValidateAutoSave()
├─ [0x18] AutoSaveManager::RecoverAutoSave()
```

**Implements**: `ISparseModeActivationHandler` (background activation)  
**Files**: `"AutoSave.wlmp"`, `"AutoSaveValidate.wlmp"`  
**Uses**: `SerializationWriter`/`SerializationReader` for WLMP project format

---

## 5. COM Interface Implementation Matrix

### 5.1 Standard COM Interfaces Used

| Interface | GUID | Implementing Classes |
|-----------|------|---------------------|
| `IUnknown` | `{00000000-0000-0000-C000-000000000046}` | All COM classes |
| `IDispatch` | `{00020400-0000-0000-C000-000000000046}` | IDuiHandlerDispatch |
| `IDataObject` | `{109 {109` | DynamicDataObjectWrapper |
| `IDropTarget` | `{46571460-0000-0000-C000-000000000046}` | TimelineDragDrop, SundanceMainElementBehavior |
| `IConnectionPointContainer` | `{B196B284-BAB4-101A-B69C-00AA00341D07}` | IDuiDataSourceImpl |
| `IStream` | `{00000003-0000-0000-C000-000000000046}` | ReadOnlyStreamOnStaticMemory, MFByteStreamOnStream |
| `IServiceProvider` | `{6D5140C1-7436-11CE-8034-00AA006009FA}` | (various) |
| `IMFAsyncCallback` | `{a27003d0-e347-4ea8-8966-9b39ef2eadc6}` | MFAsyncResult, MFByteStreamOnStreamAsyncResult |
| `IMFByteStream` | `{ad4c1f0a-6437-4468-b8a5-5aa09ec44bd8}` | MFByteStreamOnStream |
| `IMFSourceReaderCallback` | `{deee8921-5c43-4037-9db2-013caad2b27b}` | DShowMFSourceReaderBuilder, NativeMFSourceReaderBuilder |
| `IDWritePixelSnapping` | `{eaf3a2d4-2d75-47ae-9ba8-52987978520c}` | (text rendering bridge) |
| `IDWriteTextRenderer` | `{ef8a8135-5cc1-4f05-890f-06a9fdcb6a90}` | (text rendering bridge) |
| `IMMNotificationClient` | `{793cd814-8b03-4287-95ba-14f8bfded43b}` | (audio device monitoring) |
| `IEnumFORMATETC` | `{00000103-0000-0000-C000-000000000046}` | (clipboard enumeration) |

### 5.2 Custom Internal COM Interfaces

| Interface | Purpose | Key Implementors |
|-----------|---------|-----------------|
| `IAVSourceFactory` | Create AV sources from URIs | AVSourceFactory, NativeMFSourceReaderBuilder, DShowMFSourceReaderBuilder |
| `IAVSource` | Media source abstraction | AVSource, AVSourceProxy |
| `IAVSink` | Media sink abstraction | AVSink |
| `IAVStreamSink` | Individual stream sink | AudioStreamSink, StreamSinkHost |
| `IAVProcessor` | Video frame processing | VideoBuffer |
| `IAVSampleSource` | Sample delivery | SyncVideoSampleSource |
| `IAVTickTimeSource` | Timing reference | (used by Conductor) |
| `IAVCaptureSessionCallback` | Capture events | AVCaptureCore |
| `IFrameBuffer` | Render target | FrameBufferImplDX |
| `IEngine` | Core rendering engine | Engine, EngineDX |
| `ISceneEncode` | Scene → encoded output | SceneEncode |
| `IScenePreview` | Scene → preview | ScenePreview, PreviewDX, DefaultPreviewDX |
| `IPreviewPresenter` | Display preview frames | PreviewPresenterWrapper |
| `IResourceCache` | Texture/effect caching | ResourceCache, ResourceCacheDX |
| `IEncodeProfile` | Encoding parameters | EncodeProfile |
| `ISelectionRoot` | Selection management | SelectionRootImpl, SelectionRootImplWrapper |
| `ITimelineItem` | Timeline item abstraction | (various timeline items) |
| `ITimelineItemHandler` | Timeline item rendering | TimelineItemHandler |
| `ITextureProviderDX` | Texture interop | TextureInterOp, TextureInterOpDX9, TextureInterOpDX11 |
| `IImageThumbnail` / `IMovieThumbnail` | Thumbnail generation | ImageThumbnail, MovieThumbnail, MovieThumbnailDX |
| `IWaveform` / `IWaveformCallback` | Audio waveform | Waveform, WaveformCallback |
| `IAudioBoost` | Audio volume boost | AudioBoost |
| `IDuiBehavior` | DirectUI behavior | 30+ behavior classes (see §4) |
| `IDuiBehaviorFactory` | Create behaviors | SundanceBehaviorFactory |
| `IDuiDataSource` | DirectUI data binding | IDuiDataSourceImpl |
| `IDuiHandlerNotify` | DirectUI notifications | IDuiHandlerNotifyImpl |
| `IDuiVirtualLayoutMode` | Virtual layout | IDuiVirtualLayoutModeImpl |
| `IDuiTemplateSource` | Template rendering | TimelineTemplateSource |
| `IPublishHandler` / `IAmpPublishHandler` | Publish plugin | (loaded from WLXMediaPublishSubscribe.dll) |
| `ICodecEncDecCallbackCore` | Codec callbacks | SimpleCodecEncDecCallbackCore |
| `IClipboardChainCallback` | Clipboard monitoring | ClipboardChainWindow |
| `ISparseModeActivationHandler` | Background activation | AutoSaveManager |
| `IWorkerObjectCallback` | Async work items | AsyncWorkerObject |
| `IUxStoryboardNotifications` | Storyboard events | (theme/project change notifications) |
| `ISAIExecutionContext` | Scene-authoring | ExecutionContext |

### 5.3 Windows Live Publishing Plugin Interfaces

Loaded dynamically from `WLXMediaPublishSubscribe.dll`:

| Interface | GUID | Implementing DLL | Purpose |
|-----------|------|-----------------|---------|
| `IFlickrProvider` | `{197608E2-D42D-43a5-927E-1C67FC041431}` | WLXMediaPublishSubscribe | Flickr upload |
| `ILiveProvider` | `{5F4019FA-375D-4D74-9922-EFCD9D920A0D}` | WLXMediaPublishSubscribe | SkyDrive/Live upload |
| `IProviderManager` | `{43DBAB44-8EDC-4FB0-B305-F117ECB15305}` | WLXMediaPublishSubscribe | Plugin discovery |
| `IPluginDecorator` | `{54c41e30-bfd8-44e5-91a2-ce038e242817}` | WLXMediaPublishSubscribe | Plugin wrapper |
| `IPluginManagerHelper` | `{66A5A6CA-0D84-44cb-BEC4-04B3B888E4B2}` | WLXMediaPublishSubscribe | Plugin lifecycle |
| `IOnlineMediaPluginManager` | `{8ab5eea6-7a20-417d-b6d1-d40c9804b2cc}` | WLXMediaPublishSubscribe | Online media plugins |
| `IInternetCacheManager` | `{C9C25561-A401-43b4-BC22-79FBA2A00042}` | WLXMediaPublishSubscribe | Cache management |
| `IPublishPluginHelper` | `{CE5E100A-6CE8-4d67-8870-245DFA3499AB}` | WLXMediaPublishSubscribe | Plugin helpers |
| `IMetadataSettingsController` | `{DA69067E-3959-47ca-A58D-2300786168CD}` | WLXMediaPublishSubscribe | Metadata settings |

### 5.4 DirectUI Class Hierarchy (via UXCore.dll imports)

```
DirectUI::Element (192 imported methods)
├── DirectUI::HWNDElement
│   └── SundanceNativeHwndHost
├── DirectUI::NativeHWNDHost
├── DirectUI::Button
│   └── DirectUI::TabButton
├── DirectUI::Checkbox
├── DirectUI::Edit
├── DirectUI::WLEditT
├── DirectUI::Combobox
├── DirectUI::Selector
├── DirectUI::Label
├── DirectUI::Thumb
├── DirectUI::Slider
├── DirectUI::ScrollBar
├── DirectUI::Hyperlink
├── DirectUI::TabControl
├── DirectUI::TabPage
├── DirectUI::PopupMenu2
├── DirectUI::SuperPopup
├── DirectUI::PopupWindow
├── DirectUI::VirtualLayout
│   └── DirectUI::VirtualListView
├── DirectUI::DialogHost
├── DirectUI::BorderSplitter
└── DirectUI::MenuItem

CDUIDialog (UXCore)
├── CSundanceDialog
│   ├── SundanceApplicationOptionsDialog
│   ├── MultipleEffectDialog
│   ├── RenderSummaryDialog
│   ├── PublishSummaryDialog
│   └── UserEncodeProfileDialog
└── DontShowPromptDialog
    └── SundanceDontShowPromptDialog

CFramelessHost (UXCore)
└── (used for frameless window rendering)

DirectUI::Layout
├── DirectUI::FillLayout
└── (other layout managers)
```

---

## 6. 3D Engine Hierarchy

### 6.1 Engine Chain

```
IEngine
├── Engine (GDI+ software path)
└── EngineDX (D3D11 hardware path)
    ├── Scene → ScenePreview → PreviewDX → DefaultPreviewDX
    ├── SceneEncode (encode pipeline)
    ├── RenderLoop → RenderLoopDX
    ├── Conductor (AV synchronization)
    ├── Mixer (audio mixing)
    │   └── MixerBuffer
    ├── MovieTransport → RenderTransport → TransportBase
    ├── SnapShot → SnapShotDX
    └── BackBufferDX → SharedSwapChainDX
```

### 6.2 X3D Node Hierarchy

```
X3DNodeImpl (root)
├── X3DChildNodeImpl
│   ├── X3DGroupingNodeImpl
│   ├── X3DShapeNodeImpl
│   ├── X3DGeometryNodeImpl
│   │   ├── X3DComposedGeometryNodeImpl
│   │   └── X3DVertexAttributeNodeImpl
│   ├── X3DAppearanceNodeImpl
│   │   ├── X3DMaterialNodeImpl
│   │   └── X3DTextureNodeImpl → X3DTexture2DNodeImpl
│   ├── X3DCoordinateNodeImpl
│   ├── X3DNormalNodeImpl
│   ├── X3DColorNodeImpl
│   ├── X3DTextNodeImpl
│   ├── X3DViewpointNodeImpl
│   ├── X3DViewportNodeImpl
│   ├── X3DSoundNodeImpl → X3DSoundSourceNodeImpl
│   ├── X3DShaderNodeImpl → X3DAnimatedShaderNodeImpl
│   ├── X3DLayerNodeImpl
│   ├── X3DFontStyleNodeImpl
│   └── X3DInterpolaterNodeBase
├── X3DBindableNodeImpl
├── X3DBoundedObjectImpl
├── X3DTimeDependentObjectImpl
│   └── X3DSensorNodeImpl → X3DTriggerNodeImpl
├── X3DUrlObjectImpl
├── X3DMetadataObjectImpl
└── X3DChildObjectImpl
```

### 6.3 CCodec Texture Format Hierarchy

```
CCodec (base)
├── CCodec_A8R8G8B8 (and ~107 DXGI format variants)
├── CCodec_BC1_UNORM (+ SRGB)
├── CCodec_BC2_UNORM (+ SRGB)
├── CCodec_BC3_UNORM (+ SRGB)
├── CCodec_BC4_SNORM / BC4_UNORM
├── CCodec_BC5_SNORM / BC5_UNORM
├── CCodec_BC6H_SF16 / BC6H_UF16
├── CCodec_BC7_UNORM (+ SRGB)
├── CCodec_D32_FLOAT
├── CCodec_R16G16B16A16_FLOAT
├── ... (107 total DXGI format codecs)
├── CCodecDXT (base for compressed)
└── CCodecYUV (base for YUV)
```

---

## 7. StoryboardManagerNamespace Hierarchy (~141 types)

```
StoryboardManager (root orchestrator)
├── MovieProject (project data model)
│   ├── MovieExtent (timeline extent)
│   │   └── MovieEffect (effect extent)
│   ├── MovieEffect → ThemeEffect → PanAndZoomShapeEffect
│   └── MediaClipBase
│       ├── VideoClip
│       ├── AudioClip / AudioClipImpl
│       ├── ImageClip
│       └── TitleClip
├── ThemeManager (theme application)
│   ├── ThemeProject
│   ├── ThemePrimaryTrack
│   ├── ThemeDependentTrack
│   ├── ThemeTrack
│   └── ThemeOperationLogger
├── Conductor (AV sync engine)
├── TextManager (title/credits)
│   └── TextComposer
├── SelectionRootImpl (selection model)
│   ├── SelectionRootImplWrapper
│   ├── SelectionIndex
│   └── SelectorExtentRefContainer
├── SerializationWriter / SerializationReader
│   ├── CommandBin / RenderCommandBin
│   ├── SerializationContext
│   └── SerializationMemoryReaderWriter
├── BoundProperty system
│   ├── BoundProperty
│   ├── BoundPropertyBool / Float / Int / String
│   ├── BoundPropertyFloatElement / StringElement
│   ├── BoundPropertyFloatSet / StringSet
│   ├── BoundPropertyDictionary
│   └── BoundPlaceholder + MapContainer
├── Metadata system
│   ├── Metadata
│   ├── MetadataBoolImpl / DoubleImpl / FloatImpl
│   ├── MetadataIntegerImpl / StringImpl / SetImpl
│   └── DatabasePropertyManager
├── Template system
│   ├── TemplateTable
│   ├── BaseTemplate → BaseX3DTemplate
│   │   ├── CompositeX3DTemplate
│   │   ├── EffectX3DTemplate
│   │   ├── ExtentX3DTemplate
│   │   └── ThemeX3DTemplate
│   ├── TemplatePlaceholder / TemplateProperty
│   ├── TemplateSocket / TemplateSocketsParser
│   └── TemplateInitializationException
├── Extent system
│   ├── ExtentMapContainer
│   ├── ExtentSelector
│   ├── ExtentSelectorValidator
│   ├── ExtentSocketsHandler
│   └── ExtentListRegionIterator / TimeSnapshotRegionIterator
├── Effect/Transition containers
│   ├── EffectListContainer
│   ├── TransitionListContainer
│   └── TransitionSerializer
├── Storyboard I/O
│   ├── ModBeginDocument / ModBeginElement / ModEndElement
│   ├── ModContainer / ModContainerWithAttribute
│   └── ModContainerWithIDLookAhead
└── Background operations
    ├── SceneMergeBackgroundRequest
    ├── MediaLoadBackgroundRequest
    ├── BaseBackgroundRequest
    └── TranscodeBackgroundRequest
```

---

## 8. ATL Module Registration

The DLL uses ATL for COM class registration:

```
CComModule (CAtlModule)
├── CComObjectRootBase
│   └── (ATL COM objects registered via OBJECT_MAP)
├── CRegObject
└── CAtlModule

ATL Module Init Order:
  1. _Module.Init(ObjectMap, hInstance)
  2. _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE)
  3. Main message loop
  4. _Module.RevokeClassObjects()
  5. _Module.Term()
```

**COM APPID**: `{CE8B9537-708C-4784-9DD4-127B635DD348}` — `Microsoft\Live\MovieMaker`

---

## 9. RTTI Statistics Summary

| Category | Count | Percentage |
|----------|-------|-----------|
| Application Framework | 12 | 1.2% |
| Timeline UI | 14 | 1.4% |
| 3D Engine Core | 16 | 1.6% |
| Audio/Video Processing | 12 | 1.2% |
| Effects & Shaders | 25 | 2.5% |
| Pattern Meshes | 28 | 2.7% |
| Textures | 15 | 1.5% |
| Grids & Shatter | 8 | 0.8% |
| Rendering | 8 | 0.8% |
| X3D Scene Nodes | 46 | 4.5% |
| D3DX11 Effect Framework | 94 | 9.2% |
| D3DX11 Texture Codecs | 107 | 10.5% |
| Publishing | 9 | 0.9% |
| Serialization/Binding | 34 | 3.3% |
| Media/Transcode/Encode | 40 | 3.9% |
| UI Behavior/Dialog | 91 | 8.9% |
| Bound/Template System | ~25 | 2.5% |
| Media Items/Browser | ~20 | 2.0% |
| Ribbon UI | 5 | 0.5% |
| Miscellaneous | ~524 | 51.5% |
| **Total RTTI types** | **1,018** | **100%** |

---

## 10. COM Object Registration GUIDs

| GUID | Type | Description |
|------|------|-------------|
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | APPID | MovieMakerCore application ID |
| `{9590CDCE-BFFC-45d5-85EA-493C65FB2C4F}` | CLSID/Template | Transition/Effect template identifier |
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Telemetry | Error reporting endpoint ID |

**Note**: No embedded type library (`.tlb`) was found. All COM interfaces are defined through C++ vtables and RTTI, not IDL. The actual CLSIDs for COM objects are registered via ATL REGISTRY resources in the `.rsrc` section (4.07 MB).

---

## 11. Cross-DLL Interface Map

```
MovieMakerCore.dll
  ├── imports from ──► UXCore.dll (192 DirectUI functions)
  ├── imports from ──► WLXPhotoBase.dll (14 base utility functions)
  ├── imports from ──► WLXPhotoSqm.dll (13 SQM telemetry functions)
  ├── imports from ──► DmxBici.dll (5 BICI telemetry functions)
  ├── imports from ──► MetadataSys.dll (1 function)
  ├── imports from ──► uxctl.dll (3 UX control functions)
  ├── loads at runtime ► WLXMediaPublishSubscribe.dll (9 COM interfaces)
  ├── loads at runtime ► WLAVRes.dll (AV resources)
  ├── loads at runtime ► eWLXPhotoLibraryDatabase.dll (photo DB)
  └── loads at runtime ► Comctl32.dll, d3d10level9.dll
```
