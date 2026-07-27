# UXCtl.dll — Windows Live UI Controls

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Timestamp | Tue Apr 1 01:24:20 2014 (0x533A3F74) |
| Image Base | 0x10000000 |
| Size of Image | 0x2FD000 (~3.0 MB) |
| Code Size | 0x63A00 (~400 KB) |
| Resources | 0x27F670 (~2.5 MB) — **very large** |
| Subsystem | Windows GUI (2) |
| PDB | `uxctl.pdb` — GUID `{6612D194-2A96-4F55-A405-68F1833D6FED}` |
| C/C++ Functions | 156 |

## Exports (8 total)
| Ordinal | Name | Description |
|---------|------|-------------|
| 1 | `DllCanUnloadNow` | COM unload check |
| 2 | `DllGetClassObject` | COM class factory |
| 3 | `DllMain` | Entry point |
| 4 | `DllRegisterServer` | COM registration |
| 5 | `DllUnregisterServer` | COM unregistration |
| 6 | `UxControlsCreateObject` | **Creates control objects** — main factory |
| 7 | `UxControlsInitProcess` | Process initialization |
| 8 | `UxControlsUninitProcess` | Process cleanup |

## COM Registration
- **Server name:** `UxControls` (registered as out-of-process server)
- **Uninit name:** `UxControlsUninit`

## Key Classes & Strings
- `UXControlsHttpDo` — HTTP download control
- `CRMDUIParser` — Inherits/reuses UXCore's parser
- `CRMImage`, `CRMStringResource` — Reuses UXCore resource classes

## Imports from UXCore.dll
This is the critical dependency — UXCtl imports extensively from UXCore:

### Element API
- `Add@Element`, `Remove@Element`, `Insert@Element`, `Destroy@Element`, `DestroyAll@Element`
- `Initialize@Element`, `FindDescendent@Element`, `FindDescendentByClass@Element`
- `GetValue@Element`, `SetValue@Element`, `RemoveLocalValue@Element`
- `SetDataContext@Element`, `GetDataContext@Element`, `EvaluateSubtreeBindings@Element`
- `FireEvent@Element`, `PostEvent@Element`
- `AddHandler@Element`, `RemoveHandler@Element`, `ResolveProperty@Element`
- `AddBehavior@Element`, `RemoveBehavior@Element`, `QIBehaviors@Element`
- `SetDataTransform@Element`, `SetBinding@Element`
- `StartDefer@Element`, `EndDefer@Element`
- `ShowElement`, `FocusElement@DirectUI`
- `StrToID@DirectUI`, `FindFirstDescendentByClass@DirectUI`

### Layout
- `FillLayout@DirectUI::Create`

### Value System
- `CreateInt`, `CreateBool`, `CreateString`, `CreateValueList`, `CreateSize`, `CreateRect`, `CreateDouble`, `CreateUnknown`, `CreateAtom`, `CreateLayout`, `CreateGraphic`, `CreateColor`

### CRMDUIParser (inherited from UXCore)
- `??0CRMDUIParser@@QAE@XZ`, `??1CRMDUIParser@@UAE@XZ`
- `CreateStringValue`, `CreateColorValue`, `CreateInt`, `CreateRectValue`

### CRMImage
- `Load@CRMImage`, `Detach@CRMImage`, `Destroy@CRMImage`, `Create@CRMImage`, `Attach@CRMImage`
- `CopyAlpha@CRMImage`, `Resize@CRMImage`, `HasAlphaChannel@CRMImage`, `GetSize@CRMImage`, `Composite@CRMImage`, `LoadFromResource@CRMImage`, `IsNull@CRMImage`, `ConvertToARGB@CRMImage`

### CRMStringResource
- `Load@CRMStringResource`, `Length@CRMStringResource`, `ToString@CRMStringResource`

### CUrlDownload
- `Abort`, `AddHttpHeader`, `SetDownloadCompleteSite`, `RevokeDownloadCompleteSite`, `SetPostString`

### Dialog System
- `CDUIDialog` — full lifecycle: `OnCreate`, `OnInitDialog`, `OnClose`, `OnDestroy`, `OnSize`, `OnCommand`, `OnActivate`, `OnSysCommand`, `OnGetMinMaxInfo`, `OnInitIcon`, `OnNCDestroy`, `OnShowWindow`, `OnSaveHiddenState`, `OnLoadHiddenState`, `OnDetach`, `OnUpdateFrame`, `OnPostCreateDialog`
- `CFramelessHost` — `OnCreateRegion`, `OnDefaultFrameColorChanged`, `OnUpdateFrame`
- `DoModal`, `DoModeless`, `ShowDialog`, `ExitDialog`, `FilterMessage`

### VirtualList / VirtualLayout
- `VirtualListView::Register`, `VirtualListView::SetData`, `VirtualListView::Refresh`
- `VirtualLayout::PrepareEnterStage`, `VirtualLayout::PlayExitStage`, `VirtualLayout::SetKeyFocus`
- `ElementRecycler` — `Add`, `Clear`, `Get`

### WLEditT
- `Initialize@WLEditT`, `SetMaxTextLength@WLEditT`, `SetReadOnly@WLEditT`, `SetRichEditStyle@WLEditT`, `SetURLDetect@WLEditT`
- `OnMessage@WLEditT`, `OnInput@WLEditT`, `OnHosted@WLEditT`, `OnPropertyChanged@WLEditT`
- `PaintText@WLEditT`, `GetTextLineMetrics@WLEditT`, `GetContentSize@WLEditT`
- `SyncText@WLEditT`, `SyncLabel@WLEditT`, `SyncMaxTextLength@WLEditT`

### Binding
- `Binding::Create`, `Binding::SetTwoWay`

### PopupWindow
- `PopupWindow` — `Initialize`, `Show`, `Hide`, `TrackPopup`, `TrackPopupModal`, `SetContent`, `CopyContext`, `SetSelectAnchor`

### Button
- `Initialize@Button`, `OnInput@Button`, `OnHosted@Button`, `OnPropertyChanged@Button`, `GetTextElement@Button`, `DefaultAction@Button`
- `Click@Button` event, `TextProp@Button`, `DialogStyleProp@Button`

### Selector / TabControl
- `Selector::Class`
- `TabControl::Class`, `TabPage::Class`, `SetFocusOnChild@TabControl`

### Combobox
- `Combobox::Class`, `AddString@Combobox`, `InsertString@Combobox`, `GetSelectedIndex@Combobox`, `GetSelectedString@Combobox`
- `SelectionChange@Combobox` event

### Checkbox
- `Checkbox::Class`, `CheckedProp@Checkbox`, `StateProp@Checkbox`

### DialogHost
- `DialogHost::Class`, `FrameTitleProp@DialogHost`

### RM (Resource Manager) API
- `RMInitialize@@YGXXZ` — Initialize resource manager
- `RMTerminate@@YGXXZ` — Terminate resource manager
- `RMUpdateResourceSet@@YG_NPBDPB_WK11PAUHINSTANCE__@@@Z` — Update resource sets
- `RMLoadString@@YGIPBDIPA_WIK@Z` — Load localized string
- `RMLoadRect@@YG_NPBDPB_WPAUtagRECT@@K@Z` — Load rectangle from resource
- `RMLoadIcon@@YGPAUHICON__@@PBDPB_WK@Z` — Load icon from resource
- `RMLoadInt@@YGHPBDIHK@Z` — Load integer from resource
- `RMLoadImage@@YGPAXPBDPB_WIHHIK@Z` — Load image from resource
- `RMLoadMenu@@YGPAUHMENU__@@PBDPB_WK@Z` — Load menu from resource
- `RMFindModule@@YGPAUHINSTANCE__@@PBDK@Z` — Find resource module
- `RMLoadStringBSTR@@YGPA_WPBDIK@Z` — Load string as BSTR

### Helper Functions
- `SetElementProperty@DirectUI` — Set element property by name
- `EnableElement@DirectUI` — Enable/disable element
- `ShowElement@@YGJPAVElement@DirectUI@@H@Z` — Show/hide element
- `SetElementContentFromStringResource`, `SetElementPropertyFromStringResource`, `SetElementPropertyFromImage`, `SetElementPropertyFromImageResource`, `SetElementPropertyFromHICON`, `SetElementPropertyFromStringResource`, `SetElementContentFromIconResource`, `SetElementContentFromImageResource`, `SetAnimatorAnimationFromAnimatedGIF`, `SetAnimatorAnimationFromAnimatorResource`, `SetElementBackgroundFromImageResource`
- `GetWorkAreaRect@@YGXPBUtagRECT@@PAU1@@Z`, `FitRectToScreen@@YGXPAUtagRECT@@KPBU1@@Z`
- `CalculateAutoColorizeColor@@YGKPBVCRMImage@@H@Z`, `MaxContrast@@YGKKKK@`
- `GetGdiplusEncoderClsid@@YGJPB_WPAU_GUID@@@Z`

### Property Globals (imported from UXCore)
All `*Prop@Element@DirectUI@@2PAUPropertyInfo@2@A` statics — BackgroundProp, ContentProp, VisibleProp, EnabledProp, LayoutPosProp, etc.

## Imports from uxcontacts.dll
- Ordinal 10 (from `uxcontacts.dll`) — likely `UXContactsCreateObject` or similar factory

## Resource Section
- **2.5 MB of resources** — contains UIFILE templates, string tables, images, icons, menus
- This is the main resource DLL for Windows Live shared UI controls

## Delay-Loaded
| DLL | Functions |
|-----|-----------|
| WININET.dll | `InternetOpenA`, `InternetConnectA`, `HttpOpenRequestA`, `InternetReadFile`, `InternetWriteFile`, `HttpSendRequestExA`, `HttpEndRequestA`, full HTTP client stack |
| gdiplus.dll | `GdipCreateBitmapFromFile`, `GdipCreateFont`, `GdipDrawString`, `GdipCloneBitmapAreaI`, `GdipCreateSolidFill`, `GdipCreateFontFamilyFromName` — text rendering and bitmap manipulation |

## Key Architecture Notes
1. **Thin wrapper DLL** — only 156 C/C++ functions but 2.5 MB of resources
2. **All UI logic lives in UXCore** — UXCtl adds Windows Live-specific controls and resource templates
3. **UXCtl imports UXContacts** (ordinal 10) for contact-related UI
4. **HTTP download** built-in via `CUrlDownload` + delay-loaded WININET
5. **Rich text rendering** via GDI+ (delay-loaded) for `GdipDrawString`/`GdipCreateFont`
6. **RM (Resource Manager) API** provides a resource abstraction layer for loading strings, images, icons, menus, and rects from a centralized resource system
7. **WLEditT** is the primary text input control — wrapping Rich Edit with DirectUI integration
