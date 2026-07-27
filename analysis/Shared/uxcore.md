# UXCore.dll — DirectUI Framework Core

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Timestamp | Tue Apr 1 01:26:04 2014 (0x533A3FDC) |
| Image Base | 0x10000000 |
| Size of Image | 0x27E000 (~2.5 MB) |
| Code Size | 0x242000 (~2.3 MB) |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | Dynamic base, NX compatible |
| Linker | 11.00 (VS2012) |
| PDB | `uxcore.pdb` — GUID `{F22C0AFB-AFAF-4806-B729-A18804EE256E}` |
| C/C++ Functions | 603 |

## Exports Summary
- **Total exports:** 1946 functions, 1889 named
- **DLL entry points:** `DllCanUnloadNow`, `DllGetClassObject`, `DllMain`, `DllRegisterServer`, `DllUnregisterServer`
- **Core init:** `UXCoreInitProcess`, `UXCoreInitThread`, `UXCoreUnInitProcess`, `UXCoreUnInitThread`
- **Class factory:** `UxGetClassObject`, `DuiCreateObject`
- **Gadget API:** `GetGadget`, `GetGadgetRect`, `GetGadgetSize`, `GetGadgetStyle`, `SetGadgetStyle`, `InvalidateGadget`, `DrawGadgetTree`, `FindGadgetFromPoint`, `MapGadgetPoints`, `GetGadgetFocus`, etc.

## DirectUI Element Classes (RTTI / IClassInfo)
The DLL registers a comprehensive set of UI element classes via `Class@*@@2PAUIClassInfo`:

### Core Elements
| Class | Description |
|-------|-------------|
| `Element` | Base element — all UI elements derive from this |
| `HWNDElement` | Top-level HWND-backed element (main window host) |
| `HWNDHost` | Hosts native HWND child controls |
| `NativeHWNDHost` | Creates and hosts native HWND windows |
| `HWNDContainer` | Container for HWND-based elements |

### Controls
| Class | Description |
|-------|-------------|
| `Button` | Push button |
| `ButtonText` | Button with text content |
| `ButtonIcon` | Button with icon |
| `ButtonArrow` | Arrow button |
| `ButtonDropDown` | Drop-down button |
| `ButtonList` | List of buttons |
| `Checkbox` | Check box (has `CheckedProp`, `StateProp`) |
| `CheckboxGroup` | Group of checkboxes |
| `Combobox` | Drop-down combo box (has `SelectionChange` event) |
| `Edit` | Text edit control |
| `Hyperlink` | Clickable link (has `Navigate` event) |
| `Label` | Static text label |
| `Menu` | Menu container |
| `MenuButton` | Menu trigger button |
| `MenuItem` | Menu item |
| `MenuItem2` | Alternate menu item |
| `MenuItemText` | Text-only menu item |
| `Progress` | Progress bar |
| `RepeatButton` | Auto-repeating button |
| `ScrollBar` | Scroll bar |
| `SimpleScrollBar` | Simplified scroll bar |
| `ScrollViewer` | Scrollable viewport |
| `Selector` | Selection management |
| `Slider` | Slider control |
| `SplitButton` | Split button |
| `TabButton` | Tab strip button |
| `TabControl` | Tab container |
| `TabPage` | Individual tab page |
| `Thumb` | Draggable thumb |

### Toolbars
| Class | Description |
|-------|-------------|
| `Toolbar` | Toolbar container |
| `ToolbarButton` | Toolbar button |
| `ToolbarChevron` | Overflow chevron |
| `ToolbarControls` | Toolbar control area |
| `ToolbarDropDown` | Toolbar dropdown |
| `ToolbarSeparator` | Toolbar separator |

### Text / Rich Content
| Class | Description |
|-------|-------------|
| `FormattedText` | DirectWrite-based formatted text |
| `TextChunk` | Inline text chunk (for hyperlinks etc.) |
| `EmbeddedChunk` | Embedded object in text |
| `EmoticonText` | Emoticon rendering (has `CF_EMOTICONNAME` format) |

### Layout & View
| Class | Description |
|-------|-------------|
| `Viewer` | Image/content viewer |
| `VirtualListView` | Virtualized list (lazy element creation) |
| `VirtualLayout` | Virtualized layout manager |
| `MultiColumnSplitter` | Multi-column splitter |
| `RefPointElement` | Reference point for positioning |

### Popup & Dialog
| Class | Description |
|-------|-------------|
| `PopupMenu` | Context menu |
| `PopupMenu2` | Extended popup menu |
| `Dialog` | Modal dialog |
| `DialogHost` | Dialog hosting element |
| `PopupWindow` | Popup window |

### Document Object
| Class | Description |
|-------|-------------|
| `DocObjHWNDHost` | OLE document object host |
| `DocObjHost` | OLE document hosting container |

### Animation & Image
| Class | Description |
|-------|-------------|
| `Animator` | Animation controller |
| `AsyncImage` | Asynchronous image loader (has `URNProp`) |
| `BorderSplitter` | Splittable border |

## Layout Engine
| Class | Description |
|-------|-------------|
| `BorderLayout` | Border-based layout |
| `FillLayout` | Fill parent layout |
| `FlowLayout` | Flow/wrap layout |
| `GridLayout` | Grid layout |
| `NineGridLayout` | 9-patch grid layout |
| `RowLayout` | Row-based layout |
| `TableLayout` | Table layout |
| `Layout` | Base layout class |

## Key Element Properties (PropertyInfo)
Properties are stored as static `PropertyInfo*` globals on each class. Key properties on `Element`:

| Property | Description |
|----------|-------------|
| `ActiveProp` | Active state |
| `AccNameProp` | Accessibility name |
| `AccDescProp` | Accessibility description |
| `AccRoleProp` | Accessibility role |
| `AccStateProp` | Accessibility state |
| `AccValueProp` | Accessibility value |
| `AccDefActionProp` | Accessibility default action |
| `AccessibleProp` | Accessible flag |
| `AlphaProp` | Transparency |
| `AllowDrop` | Drag-drop support |
| `AnimationProp` | Animation assignment |
| `BackgroundProp` | Background brush/color |
| `BackgroundIndexProp` | Background image index |
| `BorderColorProp` | Border color |
| `BorderStyleProp` | Border style |
| `BorderThicknessProp` | Border thickness |
| `ChildrenProp` | Child elements |
| `ClassProp` | CSS-like class assignment |
| `Click` | Click event handler |
| `CmdContextProp` | Command context |
| `CmdGroupProp` | Command group GUID |
| `CmdIDProp` | Command ID |
| `ColorizeProp` | Colorize tint |
| `ContentAlignProp` | Content alignment |
| `ContentIndexProp` | Content image index |
| `ContentProp` | Content text |
| `ContextMenuResIDProp` | Context menu resource ID |
| `CursorProp` | Mouse cursor |
| `DataContextProp` | Data context (for bindings) |
| `DataTransformProp` | Data transform |
| `DataSourceProp` | Data source (on VirtualLayout) |
| `DesiredSizeProp` | Desired layout size |
| `DirectionProp` | Layout direction |
| `EnabledProp` | Enabled state |
| `ExtentProp` | OLE extent |
| `FontSizeProp` | Font size |
| `FontWeightProp` | Font weight |
| `ForegroundProp` | Foreground color |
| `HeightProp` | Height |
| `IDProp` | Element ID |
| `KeyboardNavigate` | Keyboard navigation event |
| `KeyFocusedProp` | Key focus state |
| `KeyWithinProp` | Key focus within subtree |
| `LayoutPosProp` | Layout position |
| `LayoutProp` | Layout assignment |
| `MaxSizeProp` | Maximum size |
| `MinSizeProp` | Minimum size |
| `MouseFocusedProp` | Mouse focus state |
| `MouseWithinProp` | Mouse hover state |
| `PaddingProp` | Inner padding |
| `ParentProp` | Parent element |
| `SelectedProp` | Selected state |
| `TagProp` | User data tag |
| `VisibleProp` | Visibility |
| `WindowActiveProp` | Window active state |

## Resource Management (CRM Classes)
| Class | Description |
|-------|-------------|
| `CRMColorResource` | Named color resource (ARGB, colorize) |
| `CRMImage` | Image resource (HBITMAP wrapper, DIB, alpha, GDI+) |
| `CRMResource` | Generic binary resource loader |
| `CRMStringResource` | Localized string resource |
| `CRMDUIParser` | **UIFILE parser** — parses DirectUI markup into element trees |
| `CRMDUIPhoto` | Photo-specific UI element (has `CachingStyleProp`) |

### CRMDUIParser — The UIFILE Engine
This is the DirectUI UI definition parser. Key methods:
- `Create@CRMDUIParser@@SGJPAPAV1@@Z` — Static factory
- `Load@CRMDUIParser@@QAEJPBDIK@Z` — Load from binary resource
- `LoadAndCreateElement@CRMDUIParser@@QAEJPBDIPB_WPAPAVElement@DirectUI@@PAV23@K@Z` — Parse and instantiate
- `CreateElement@CRMDUIParser@@UAEJPB_WPAPAVElement@DirectUI@@PAV23@@Z` — Create element from name
- `CreateStringValue`, `CreateInt`, `CreateRectValue`, `CreateColorValue`, `CreateFloatValue`, `CreateImageValue`, `CreateGraphicValue`, `CreateIconValue`, `CreateCursorValue`, `CreateStringValue`, `CreateAnimationValue`, `CreateColorizeValue` — Value factory methods for parsing UIFILE attributes

## Value System
`Value@DirectUI` is a tagged union type used throughout the framework. Static singleton constants:
- `pvNull`, `pvUnset`, `pvUnavailable` — sentinel values
- `pvBoolTrue`, `pvBoolFalse`
- `pvIntZero` through `pvIntFour`, `pvIntNegOne`, `pvIntNegThree`
- `pvStringNull`, `pvStringEmpty`
- `pvElementNull`, `pvElListNull`, `pvLayoutNull`, `pvGraphicNull`
- `pvPointZero`, `pvRectZero`, `pvSizeZero`
- `pvDoubleZero`, `pvFloatZero`, `pvDword64Zero`
- `pvColorTrans`, `pvColorizeGrayText`
- `pvAnimationNull`, `pvCursorNull`, `pvGUIDNull`, `pvRLENull`, `pvSheetNull`, `pvUnknownNull`, `pvValueListNull`

Factory methods: `CreateInt`, `CreateBool`, `CreateString`, `CreateDouble`, `CreateFloat`, `CreateColor`, `CreateAtom`, `CreateSize`, `CreatePoint`, `CreateRect`, `CreateGraphic`, `CreateCursor`, `CreateGUID`, `CreateRLE`, `CreateLayout`, `CreateElementList`, `CreateElementRef`, `CreateValueList`, `CreateVariant`, `CreateUnknown`, `CreateDFC`, `CreateDTB`, `CreateAnimation`, `CreateLinearAnimation`, `CreateCompositeAnimation`, `CreateCycleAnimation`, `CreateConstAnimation`, `CreateInverseAnimation`, `CreateFramesAnimation`, `CreateColorize`

## Animation System
- `AnimationManager` — Manages active animations
- `Animator` — Per-element animator
- `DuiElementTimerHandler` — Timer-driven animation support
- `SingleTimerHandler` — Shared timer

## Data Source / VirtualList
- `IDuiDataSourceImpl` — Data source implementation for virtual lists
- `VirtualListView` — Virtual list with lazy element creation
- `ElementRecycler` — Recycles elements for performance
- `ItemRange` — Represents a range of items in virtual lists

## Accessibility
- `DuiAccessible` — IAccessible implementation
- `HWNDElementAccessible` — HWND element accessibility
- `HWNDHostAccessible` — HWND host accessibility
- `CRichEditAccessible` — Rich edit accessibility
- `FormattedAccessible` — Formatted text accessibility
- `ButtonAccessible` — Button accessibility
- `ChunkAccessible` — Text chunk accessibility

## Frameless Window Support
- `CFramelessHost` — Frameless (chromeless) window host
- `UXFramelessManager` — Manages frameless windows
- `UXColorizer` — Color scheme management for frameless windows

## Dialog System
- `CDUIDialog` — DirectUI-based dialog (extends `CFramelessHost`)
  - `OnCreate`, `OnInitDialog`, `OnClose`, `OnDestroy`, `OnSize`, `OnCommand`, `OnActivate`, `OnSysCommand`, `OnGetMinMaxInfo`, `OnInitIcon`, `OnNCDestroy`, `OnShowWindow`, `OnSaveHiddenState`, `OnLoadHiddenState`
  - `DoModal`, `DoModeless`, `ShowDialog`, `ExitDialog`, `FilterMessage`
  - `DIALOG_CLASS` — Window class name constant

## Rich Edit / Text
- `WLEditT` — Windows Live edit control (extends Element with rich edit support)
- `CRichEditOleCallback` — OLE callback for rich edit
- `TextHostT` — DWrite text host
- `EmoticonText` — Emoticon rendering in text

## OLE / Drag-Drop
- `DocObjHost` — OLE document hosting (IOleClientSite, IOleInPlaceSite, etc.)
- `DocObjHWNDHost` — HWND-backed OLE document host
- `CBaseOleSvr` — Base OLE server
- Built-in drag-drop: `DoDragDrop`, `RegisterDragDrop`, `RevokeDragDrop`

## URL Download
- `CUrlDownload` — HTTP download utility (wraps WININET)
  - `AddHttpHeader`, `SetPostString`, `Abort`, `SetDownloadCompleteSite`, `RevokeDownloadCompleteSite`

## Image Processing (GDI+)
- `CRMImage` wraps GDI+ for image manipulation
  - `Load`, `LoadFromResource`, `Save`, `BitBlt`, `AlphaBlend`, `Composite`, `Resize`, `CopyAlpha`, `ConvertToARGB`, `ConvertToDDB`, `GetSize`, `GetWidth`, `GetHeight`, `HasAlphaChannel`, `Detach`

## RLE (Run-Length Encoded) Image Support
- `RLEDrawToDIB`, `RLEDrawToHDC`, `RLESystemColorsChanged` — RLE image decoding/rendering

## UxTheme Integration
- Delay-loaded: `OpenThemeData`, `CloseThemeData`, `DrawThemeBackground`, `GetThemePartSize`, `IsThemeActive`, `IsAppThemed`
- DWM: `DwmIsCompositionEnabled`
- GDI+: Full GDI+ image loading, matrix transforms, rendering
- D2D1/DWrite: `DWriteCreateFactory`, D2D1 ordinals 1-2
- D3D10_1: `D3D10CreateDevice1`, `D3D10CreateEffectFromMemory`
- DXGI: `CreateDXGIFactory1`

## External Dependencies (WLDCore.dll)
Imports 13 ordinals from WLDCore.dll — likely the Windows Live shared logging/config/cryptography layer.

## Imports Summary
| DLL | Purpose |
|-----|---------|
| MSVCR110.dll | C runtime (VS2012) |
| KERNEL32.dll | Win32 core |
| USER32.dll | Window management, input |
| GDI32.dll | GDI drawing |
| ADVAPI32.dll | Registry, WMI tracing, crypto |
| ole32.dll | COM/OLE |
| OLEAUT32.dll | Automation |
| SHLWAPI.dll | Shell utility (path, string) |
| MSIMG32.dll | AlphaBlend, GradientFill, TransparentBlt |
| IMM32.dll | Input method manager (IME) |
| ntdll.dll | SList, interlocked ops |
| d2d1.dll | Direct2D |
| d3d10_1.dll | Direct3D 10.1 |
| dxgi.dll | DXGI |
| DWrite.dll | DirectWrite |
| USP10.dll | Uniscribe text shaping |
| WLDCore.dll | Windows Live core services |

## Delay-Loaded
| DLL | Purpose |
|-----|---------|
| SHELL32.dll | DragQueryFile, SHGetKnownFolderPath |
| urlmon.dll | URL monikers, bind status callback |
| WININET.dll | URL cache |
| OLEACC.dll | Accessibility |
| UxTheme.dll | Visual styles |
| dwmapi.dll | Desktop Window Manager |
| gdiplus.dll | GDI+ |
| SETUPAPI.dll | Device enumeration, cabinet iteration |
| CRYPT32.dll | Certificate chain verification |
| WINTRUST.dll | WinVerifyTrust |

## Key Architecture Notes
1. **DirectUI is a custom retained-mode UI framework** — elements form a tree, layout is computed via Layout objects, and rendering uses GDI/GDI+ with optional D2D/DWrite for text.
2. **UIFILEs** are binary resource files parsed by `CRMDUIParser` to instantiate element trees declaratively.
3. **The framework is WRL-free** — uses raw COM with manual vtable wiring (`QI`, `AddRef`, `Release`).
4. **Property system** uses static `PropertyInfo` pointers with string-based names — enables XAML-like data binding.
5. **Animation** is built-in via `Animator`/`AnimationManager` with linear, composite, cycle, and frames animation types.
6. **VirtualListView** provides virtualized list rendering for large datasets (contacts, etc.).
7. **Frameless windows** are supported via `CFramelessHost` / `UXFramelessManager` for custom window chrome.
